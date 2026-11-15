#!/usr/bin/env python3
# root wscript parser
import ast
from dataclasses import dataclass, field
from pathlib import Path

from waftocmake.model import Blocks, ConditionalList, Condition
from waftocmake.conditions import convert_condition, dotted, env_name
from waftocmake.parser import BaseWafParser


@dataclass
class OptionDef:
    dest: str
    kind: str = "bool"            # 'bool' | 'string'
    default: str = "OFF"          # 'ON'/'OFF' for bool, literal for string
    default_cond: str = ""        # CMake condition, when the default depends on the platform
    help: str = ""
    flags: list = field(default_factory=list)


@dataclass
class Directive:
    """One thing the root CMakeLists.txt has to do, under `conditions`.

    kind: define | cflag | cxxflag | linkflag | include | set | set_expr | pkg | lib | raw
    """
    kind: str
    value: str
    conditions: tuple = ()
    args: tuple = ()


@dataclass
class RootInfo:
    app_name: str = ""
    version: str = ""
    options: list = field(default_factory=list)
    directives: list = field(default_factory=list)
    project_sets: dict = field(default_factory=dict)      # set name -> [dir, ...]
    additions: list = field(default_factory=list)         # (set name, dir, conditions)
    selections: list = field(default_factory=list)        # (conditions, set name)
    uselib: dict = field(default_factory=dict)            # waf uselib name -> CMake expression
    warnings: list = field(default_factory=list)

    @property
    def needs_git(self):
        return any("${GIT_VERSION}" in d.value for d in self.directives)

    @property
    def needs_flag_checks(self):
        return any(d.kind in ("cflag_opt", "cxxflag_opt") for d in self.directives)

    @property
    def needs_pkgconfig(self):
        return any(d.kind == "pkg" for d in self.directives)

    def set_items(self, name):
        """Base items + conditional additions of a project set: [(dir, conditions)]"""
        out, seen = [], set()
        for item in self.project_sets.get(name, []):
            if item not in seen:
                seen.add(item)
                out.append((item, ()))
        for set_name, item, conds in self.additions:
            if set_name == name and (item, tuple(c.expression for c in conds)) not in seen:
                seen.add((item, tuple(c.expression for c in conds)))
                out.append((item, conds))
        return out

    def all_projects(self):
        out, seen = [], set()
        for name in self.project_sets:
            for item, _ in self.set_items(name):
                if item not in seen:
                    seen.add(item)
                    out.append(item)
        return out


IDEMPOTENT = {"define", "cflag", "cxxflag", "cflag_opt", "cxxflag_opt", "linkflag", "include"}
# conf.env values that are strings/paths, not booleans
NON_BOOL = {"PREFIX", "LIBDIR", "BINDIR", "TESTDIR", "GIT_VERSION", "DEST_OS", "DEST_CPU", "COMPILER_CC"}
FLAG_TARGETS = {
    "DEFINES": "define",
    "CFLAGS": "cflag",
    "CXXFLAGS": "cxxflag",
    "LINKFLAGS": "linkflag",
    "INCLUDES": "include",
}


def _const(node):
    return node.value if isinstance(node, ast.Constant) else None


class RootWafParser(BaseWafParser):

    def __init__(self):
        super().__init__()
        self.info = RootInfo()
        self.expanding = []
        self.warned_notes = set()

    # ------------------------------------------------------------ entry

    def parse(self, filename) -> RootInfo:
        self.filename = str(filename)
        tree = ast.parse(Path(filename).read_text(encoding="utf-8"), self.filename)
        self.functions = {n.name: n for n in tree.body if isinstance(n, ast.FunctionDef)}

        for stmt in tree.body:
            if not isinstance(stmt, ast.FunctionDef):
                self.visit(stmt)

        # build() is intentionally skipped: it repeats configure() (same project additions
        # and subproject selection), only install_files() is unique to it
        for name in ("options", "configure"):
            if name in self.functions:
                self.visit_body(self.functions[name].body)

        self.finish()
        self.info.warnings = self.warnings
        return self.info

    # ------------------------------------------------------------ module level

    def assign_name(self, name, value_node):
        if name == "projects" and isinstance(value_node, ast.Dict):
            for k, v in zip(value_node.keys, value_node.values):
                key = _const(k)
                if isinstance(key, str) and isinstance(v, (ast.List, ast.Tuple)):
                    self.info.project_sets[key] = [_const(e) for e in v.elts if isinstance(_const(e), str)]
            return
        if name in ("VERSION", "APPNAME") and isinstance(value_node, ast.Constant):
            setattr(self.info, "version" if name == "VERSION" else "app_name", str(value_node.value))
            return
        super().assign_name(name, value_node)

    # ------------------------------------------------------------ directives

    def add(self, kind, value, args=()):
        conds = self.current()
        for c in conds:
            if c.note and c.note not in self.warned_notes:
                self.warned_notes.add(c.note)
                self.warnings.append(
                    f"{self.filename}: unsupported condition '{c.note}', everything under it is disabled with if(FALSE)")
        self.info.directives.append(Directive(kind, value, conds, tuple(args)))

    def assign_attr(self, target, value_node, node):
        ref = env_name(target)
        if not ref:
            return
        name = ref[1]

        # conf.env.FRAMEWORK_APPKIT = "AppKit"
        if name.startswith("FRAMEWORK_") and isinstance(_const(value_node), str):
            store = name[len("FRAMEWORK_"):]
            self.add("lib", _const(value_node), (store, True, True))
            return

        if isinstance(value_node, ast.Constant):
            v = value_node.value
            if isinstance(v, bool) or v in (0, 1) and isinstance(v, int):
                self.add("set", name, ("ON" if v else "OFF",))
            return

        if isinstance(value_node, (ast.BoolOp, ast.UnaryOp, ast.Compare, ast.Attribute)):
            ref2 = env_name(value_node)
            if ref2 and (ref2[1] == name or ref2[1] in NON_BOOL):
                return  # conf.env.X = conf.options.X  /  paths
            cond = convert_condition(value_node)
            if cond.note:
                self.warn(node, f"cannot convert '{cond.note}'")
                return
            self.add("set_expr", name, (cond.expression,))

    def aug_subscript(self, node):
        t = node.target
        if dotted(t.value) != "projects" or not isinstance(_const(t.slice), str):
            return
        for b in self.rebased(self.evaluate(node.value)):
            for item in b.items:
                self.info.additions.append((_const(t.slice), item, b.conditions))

    # ------------------------------------------------------------ calls

    def handle_call(self, call):
        func = call.func

        if isinstance(func, ast.Name):
            fn = self.functions.get(func.id)
            if fn and func.id not in self.expanding and func.id not in ("options", "configure", "build"):
                self.expanding.append(func.id)
                self.visit_body(fn.body)
                self.expanding.pop()
            return

        if not isinstance(func, ast.Attribute):
            return

        name = func.attr
        base = dotted(func.value)
        kw = {k.arg: k.value for k in call.keywords if k.arg}
        args = call.args

        if name == "append_unique" and len(args) >= 2:
            kind = FLAG_TARGETS.get(_const(args[0]))
            if kind:
                for b in self.rebased(self.evaluate(args[1])):
                    for item in b.items:
                        self._add_with(b.conditions, kind, item)

        elif name == "define" and args and isinstance(_const(args[0]), str):
            key = _const(args[0])
            val = self.evaluate(args[1]) if len(args) > 1 else None
            if isinstance(val, bool) or isinstance(val, int):
                text = f"{key}={int(val)}"
            elif isinstance(val, str):
                text = f'{key}="{val}"'
            else:
                text = key
            self.add("define", text)

        elif name == "check_cfg" and "package" in kw:
            package = _const(kw["package"])
            store = _const(kw.get("uselib_store")) or str(package).upper()
            mandatory = _const(kw.get("mandatory")) is not False
            self.add("pkg", package, (store, mandatory))

        elif name == "check_pkg" and len(args) >= 2:
            self.add("pkg", _const(args[0]), (_const(args[1]), True))

        elif name in ("check_cc", "check", "check_lib_msvc"):
            lib = None
            if name == "check_lib_msvc" and args:
                lib = self.evaluate(args[0])
            elif "lib" in kw:
                lib = self.evaluate(kw["lib"])
            if isinstance(lib, str) and lib:
                store = self.evaluate(kw["uselib_store"]) if "uselib_store" in kw else lib.upper()
                mandatory = _const(kw.get("mandatory")) is not False
                self.add("lib", lib, (store, mandatory, False))
                if "define_name" in kw and isinstance(_const(kw["define_name"]), str):
                    self.add("define", f"{_const(kw['define_name'])}=1")

        elif name == "add_subproject" and args:
            a = args[0]
            if isinstance(a, ast.Subscript) and dotted(a.value) == "projects" and isinstance(_const(a.slice), str):
                self.info.selections.append((self.current(), _const(a.slice)))

        elif name == "add_option":
            self.add_option(call)

        elif name == "insert" and base in ("conf.env.CC", "conf.env.CXX") and len(args) == 2 and _const(args[1]) == "ccache":
            lang = "C" if base.endswith("CC") else "CXX"
            self.add("raw", f"ccache-{lang}", (
                "find_program(CCACHE_PROGRAM ccache)",
                "if(CCACHE_PROGRAM)",
                f"    set(CMAKE_{lang}_COMPILER_LAUNCHER \"${{CCACHE_PROGRAM}}\")",
                "endif()",
            ))

    def _add_with(self, conditions, kind, value):
        if value.startswith("?") and kind in ("cflag", "cxxflag"):
            kind, value = kind + "_opt", value[1:]
        for c in conditions:
            if c.note and c.note not in self.warned_notes:
                self.warned_notes.add(c.note)
                self.warnings.append(
                    f"{self.filename}: unsupported condition '{c.note}', everything under it is disabled with if(FALSE)")
        self.info.directives.append(Directive(kind, value, tuple(conditions)))

    def add_option(self, call):
        flags = [a.value for a in call.args if isinstance(_const(a), str)]
        kw = {k.arg: k.value for k in call.keywords if k.arg}

        dest = _const(kw.get("dest"))
        if not dest:
            longs = [f for f in flags if f.startswith("--")]
            if not longs:
                return
            dest = longs[0][2:].replace("-", "_").upper()

        dnode = kw.get("default")
        is_string = _const(kw.get("type")) == "string" or isinstance(_const(dnode), str)
        opt = OptionDef(
            dest=dest,
            help=(_const(kw.get("help")) or "").replace("[default: %default]", "").strip(),
            flags=flags,
        )

        if is_string:
            opt.kind = "string"
            opt.default = _const(dnode) if isinstance(_const(dnode), str) else ""
        elif dnode is None:
            opt.default = "OFF"
        elif isinstance(dnode, ast.Constant):
            opt.default = "ON" if dnode.value else "OFF"
        else:
            cond = convert_condition(dnode)
            if cond.note:
                self.warn(call, f"cannot convert default of option {dest}: {cond.note}")
            else:
                opt.default_cond = cond.expression

        self.info.options.append(opt)

    # ------------------------------------------------------------ post-processing

    def finish(self):
        info = self.info

        # drop duplicates and flags that are already implied by a less conditional copy
        # (configure() appends the same flags twice, once under `compiler != msvc`)
        def exprs(d):
            return frozenset(c.expression for c in d.conditions)

        out, seen = [], set()
        for d in info.directives:
            if d.kind in IDEMPOTENT and any(
                e.kind == d.kind and e.value == d.value and exprs(e) < exprs(d)
                for e in info.directives
            ):
                continue
            key = (d.kind, d.value, d.args, tuple(c.expression for c in d.conditions))
            if key in seen:
                continue
            seen.add(key)
            out.append(d)
        # same flag for C and C++ -> one add_compile_options() without generator expression
        def ckey(d):
            return (d.value, tuple(c.expression for c in d.conditions))

        both = {ckey(d) for d in out if d.kind == "cflag"} & {ckey(d) for d in out if d.kind == "cxxflag"}
        merged, done = [], set()
        for d in out:
            if d.kind in ("cflag", "cxxflag") and ckey(d) in both:
                if ckey(d) not in done:
                    done.add(ckey(d))
                    merged.append(Directive("flag", d.value, d.conditions))
                continue
            merged.append(d)
        info.directives = merged

        # uselib names that child wscripts may reference in use=[...]
        for d in out:
            # one variable per uselib name, whatever provides it on this platform
            # (SDL2 is a pkg-config module on Linux but a plain library on Android/Windows)
            if d.kind in ("pkg", "lib"):
                info.uselib[d.args[0]] = f"${{{d.args[0]}_LIBRARY}}"
