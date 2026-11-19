#!/usr/bin/env python3
# parser
import ast
from pathlib import Path

from waftocmake.model import (  # noqa: F401  (re-exported for old imports)
    Blocks, Condition, ConditionalList, Project, ProjectType, rebase,
)
from waftocmake.conditions import convert_condition, dotted, env_name, negate

# values of conf.env.X that are not simply "${X}"
ENV_SPECIAL = {
    'DEST_OS': '${WAF_DEST_OS}',
    'DEST_CPU': '${WAF_DEST_CPU}',
    'PREFIX': '${CMAKE_INSTALL_PREFIX}',
}

# calls whose result we deliberately do not model (no warning)
QUIET_CALLS = ('add_option_group', 'run_test', 'check_cc', 'check_cxx', 'check', 'load')


class BaseWafParser(ast.NodeVisitor):
    """Tiny abstract interpreter for the subset of python used by wscripts.

    Values are either python scalars (str/int/bool) or Blocks (list of ConditionalList,
    each remembering under which `if` conditions it was created).
    """

    def __init__(self):
        self.variables = {}
        self.condition_stack = []
        self.warnings = []
        self.filename = ""
        self.functions = {}

    # ------------------------------------------------------------ helpers

    def warn(self, node, msg):
        self.warnings.append(f"{self.filename}:{getattr(node, 'lineno', '?')}: {msg}")

    def current(self):
        return tuple(self.condition_stack)

    def convert(self, node):
        return convert_condition(node)

    def as_blocks(self, value):
        if isinstance(value, Blocks):
            return value
        if value is None:
            return Blocks()
        if isinstance(value, (str, int)):
            return Blocks([ConditionalList([str(value)], self.current())])
        return Blocks()

    def rebased(self, value):
        """Blocks of `value` with conditions as seen from the current position."""
        return [
            ConditionalList(list(b.items), rebase(b.conditions, self.current()))
            for b in self.as_blocks(value)
        ]

    # ------------------------------------------------------------ evaluation

    def evaluate(self, node):
        if isinstance(node, ast.Constant):
            return node.value

        if isinstance(node, (ast.List, ast.Tuple, ast.Set)):
            items = []
            blocks = Blocks()
            for e in node.elts:
                v = self.evaluate(e)
                if isinstance(v, Blocks):
                    blocks.extend(v)
                elif v is not None:
                    items.append(str(v))
            if items:
                blocks.insert(0, ConditionalList(items, self.current()))
            return blocks

        if isinstance(node, ast.Name):
            if node.id in self.variables:
                return self.variables[node.id]
            self.warn(node, f"unknown variable '{node.id}'")
            return None

        if isinstance(node, ast.BinOp):
            return self.eval_binop(node)

        if isinstance(node, ast.Attribute):
            return self.eval_attribute(node)

        if isinstance(node, ast.Call):
            return self.eval_call(node)

        if isinstance(node, ast.IfExp):
            self.warn(node, "conditional expression is not supported")
            return None

        self.warn(node, f"unsupported expression: {ast.unparse(node)}")
        return None

    def eval_binop(self, node):
        left = self.evaluate(node.left)
        right = self.evaluate(node.right)

        if isinstance(node.op, ast.Add):
            if isinstance(left, Blocks) or isinstance(right, Blocks):
                return Blocks(list(self.as_blocks(left)) + list(self.as_blocks(right)))
            if isinstance(left, str) and isinstance(right, str):
                return left + right
            self.warn(node, f"cannot evaluate: {ast.unparse(node)}")
            return None

        if isinstance(node.op, ast.Mod) and isinstance(left, str) and isinstance(right, (str, int)):
            try:
                return left % right
            except (TypeError, ValueError):
                pass

        self.warn(node, f"cannot evaluate: {ast.unparse(node)}")
        return None

    def eval_attribute(self, node):
        ref = env_name(node)
        if not ref:
            self.warn(node, f"unsupported attribute: {ast.unparse(node)}")
            return None

        name = ref[1]
        if name.startswith('INCLUDES_'):
            return Blocks([ConditionalList([f"${{{name[len('INCLUDES_'):]}_INCLUDE_DIRS}}"], self.current())])

        return ENV_SPECIAL.get(name, f"${{{name}}}")

    def eval_call(self, node):
        fn = dotted(node.func)

        if fn.endswith(QUIET_CALLS):
            return None

        args = [self.evaluate(a) for a in node.args]

        if fn == 'list' and args:
            v = args[0]
            if isinstance(v, Blocks):
                return Blocks(ConditionalList(list(b.items), b.conditions) for b in v)
            return v

        if fn in ('os.path.abspath', 'os.path.normpath') and args and isinstance(args[0], str):
            rel = args[0].replace('\\', '/').strip()
            if rel in ('', '.', './'):
                return '${CMAKE_SOURCE_DIR}'
            return '${CMAKE_SOURCE_DIR}/' + rel.strip('/')

        if fn == 'os.path.join' and args and all(isinstance(a, str) for a in args):
            return '/'.join(a.strip('/') if i else a.rstrip('/') for i, a in enumerate(args))

        if fn.endswith(('.filter_cflags', '.filter_cxxflags')) and args:
            # waf keeps only flags the compiler accepts; '?' marks "check before use"
            return Blocks(
                ConditionalList(["?" + i for i in b.items], b.conditions)
                for b in self.as_blocks(args[0])
            )

        if fn.endswith('.ant_glob'):
            self.warn(node, "ant_glob() is not supported, list the sources explicitly")
            return None

        self.warn(node, f"unsupported call: {ast.unparse(node)}")
        return None

    # ------------------------------------------------------------ statements

    def visit_FunctionDef(self, node):
        return  # bodies are visited explicitly by subclasses

    def visit_body(self, stmts):
        """Visit statements; `if X: return` makes the rest of the body conditional on NOT X."""
        pushed = 0
        for stmt in stmts:
            self.visit(stmt)
            if (
                isinstance(stmt, ast.If)
                and not stmt.orelse
                and any(isinstance(s, ast.Return) for s in stmt.body)
            ):
                cond = self.convert(stmt.test)
                self.condition_stack.append(Condition(negate(cond.expression), cond.note))
                pushed += 1
        for _ in range(pushed):
            self.condition_stack.pop()

    def visit_If(self, node):
        cond = self.convert(node.test)

        self.condition_stack.append(cond)
        self.visit_body(node.body)
        self.condition_stack.pop()

        if node.orelse:
            # an `elif` is just an If nested in orelse, so it nests naturally
            self.condition_stack.append(Condition(negate(cond.expression), cond.note))
            self.visit_body(node.orelse)
            self.condition_stack.pop()

    def visit_For(self, node):
        if not isinstance(node.target, ast.Name):
            self.warn(node, "unsupported for loop")
            return
        items = [i for b in self.as_blocks(self.evaluate(node.iter)) for i in b.items]
        for item in items:
            self.variables[node.target.id] = item
            self.visit_body(node.body)

    def visit_Expr(self, node):
        if isinstance(node.value, ast.Call):
            self.handle_call(node.value)

    def handle_call(self, call):
        pass

    def visit_Assign(self, node):
        if len(node.targets) != 1:
            return
        target = node.targets[0]

        if isinstance(target, ast.Name):
            self.assign_name(target.id, node.value)
        elif isinstance(target, ast.Tuple):
            # cflags, linkflags = conf.get_optimization_flags()
            for elt in target.elts:
                if isinstance(elt, ast.Name):
                    self.variables[elt.id] = Blocks()
        elif isinstance(target, ast.Attribute):
            self.assign_attr(target, node.value, node)

    def assign_name(self, name, value_node):
        value = self.evaluate(value_node)
        old = self.variables.get(name)
        if isinstance(value, Blocks) and self.condition_stack and isinstance(old, Blocks):
            # assignment inside if/else: keep both alternatives, each with its own conditions
            self.variables[name] = Blocks(list(old) + list(value))
        else:
            self.variables[name] = value

    def assign_attr(self, target, value_node, node):
        pass

    def visit_AugAssign(self, node):
        if not isinstance(node.op, ast.Add):
            return

        if isinstance(node.target, ast.Name):
            rhs = self.as_blocks(self.evaluate(node.value))
            old = self.variables.get(node.target.id)
            # new list on purpose: `cxxflags = list(cflags)` must stay independent
            self.variables[node.target.id] = Blocks(list(self.as_blocks(old)) + list(rhs))
        elif isinstance(node.target, ast.Subscript):
            self.aug_subscript(node)

    def aug_subscript(self, node):
        pass


# ====================================================================
# regular (non-root) wscript
# ====================================================================

# conf.env.X values that are flag lists: X -> (directory property, language)
FLAG_ENVS = {
    'CFLAGS': ('COMPILE_OPTIONS', 'C'),
    'CXXFLAGS': ('COMPILE_OPTIONS', 'CXX'),
    'LINKFLAGS': ('LINK_OPTIONS', 'ALL'),
}
# conf.env.append_unique('X', ...) in a child configure() -> Project attribute
ENV_TO_FIELD = {
    'DEFINES': 'defines',
    'CFLAGS': 'cflags',
    'CXXFLAGS': 'cxxflags',
    'LINKFLAGS': 'linkflags',
    'INCLUDES': 'includes',
}


class WafParser(BaseWafParser):

    TYPE_BY_METHOD = {
        'shlib': ProjectType.SHARED,
        'stlib': ProjectType.STATIC,
        'program': ProjectType.EXECUTABLE,
    }

    def __init__(self):
        super().__init__()
        self.projects = []
        self.default_name = ""
        self.conf_blocks = {f: [] for f in ENV_TO_FIELD.values()}   # from configure()
        self.prelude = []

    def parse(self, filename, default_name=""):
        """Returns list[Project] (a wscript may declare several targets)."""
        self.filename = str(filename)
        self.default_name = default_name
        source = Path(filename).read_text(encoding="utf-8")
        tree = ast.parse(source, self.filename)

        # module level first (PROJECT_NAME etc), then configure() (conf.define, ...), then build()
        for stmt in tree.body:
            if not isinstance(stmt, ast.FunctionDef):
                self.visit(stmt)
        for fname in ("configure", "build"):
            for stmt in tree.body:
                if isinstance(stmt, ast.FunctionDef) and stmt.name == fname:
                    self.visit_body(stmt.body)

        for p in self.projects:
            if not p.name:
                p.name = self.variables.get("PROJECT_NAME") or default_name
            # waf keeps configure() results in the shared env, so they apply to every target
            # built by this wscript (and, in waf, leak to later ones; we do not model that)
            for fld, blocks in self.conf_blocks.items():
                getattr(p, fld).extend(ConditionalList(list(b.items), b.conditions) for b in blocks)
            p.prelude = list(self.prelude)
            self.check_conditions(p)

        if not self.projects:
            self.warnings.append(f"{self.filename}: no shlib/stlib/program target found")

        return self.projects

    def check_conditions(self, project):
        seen = set()
        for lists in (project.sources, project.includes, project.defines, project.libs,
                      project.libpath, project.cflags, project.cxxflags, project.linkflags):
            for lst in lists:
                for c in lst.conditions:
                    if c.note and c.note not in seen:
                        seen.add(c.note)
                        self.warnings.append(
                            f"{self.filename}: unsupported condition '{c.note}' "
                            f"(target '{project.name}'), its items are disabled with if(FALSE)")

    def handle_call(self, call):
        ptype = None

        if isinstance(call.func, ast.Attribute):
            owner = dotted(call.func.value)
            if call.func.attr == "define" and owner == "conf":
                return self.conf_define(call)
            if call.func.attr == "append_unique" and owner == "conf.env":
                return self.conf_append_unique(call)

        if isinstance(call.func, ast.Name) and call.func.id == "bld":
            ptype = self.type_from_features(call)
        elif isinstance(call.func, ast.Attribute):
            ptype = self.TYPE_BY_METHOD.get(call.func.attr)

        if ptype is not None:
            self.parse_project_arguments(call, ptype)

    def conf_define(self, call):
        """conf.define('KEY'), conf.define('KEY', 1), conf.define('KEY', 'str'), conf.define('KEY', conf.env.CFLAGS)"""
        key = call.args[0].value if call.args and isinstance(call.args[0], ast.Constant) else None
        if not isinstance(key, str):
            return self.warn(call, f"unsupported define: {ast.unparse(call)}")

        if len(call.args) < 2:
            text = key
        else:
            vnode = call.args[1]
            ref = env_name(vnode) if isinstance(vnode, ast.Attribute) else None
            if ref and ref[0] == 'env' and ref[1] in FLAG_ENVS:
                # the string of flags that are active at this point of the directory
                prop, lang = FLAG_ENVS[ref[1]]
                var = f"_waf_{key}"
                line = f"waf_flags_string({var} {prop} {lang})"
                if line not in self.prelude:
                    self.prelude.append(line)
                text = f'{key}="${{{var}}}"'
            else:
                v = self.evaluate(vnode)
                if isinstance(v, bool) or isinstance(v, int):
                    text = f"{key}={int(v)}"
                elif isinstance(v, str):
                    text = f'{key}="{v}"'     # waf quotes string values
                else:
                    self.warn(call, f"cannot evaluate value of define: {ast.unparse(call)}")
                    text = key

        self.conf_blocks['defines'].append(ConditionalList([text], self.current()))

    def conf_append_unique(self, call):
        if len(call.args) < 2 or not isinstance(call.args[0], ast.Constant):
            return
        fld = ENV_TO_FIELD.get(call.args[0].value)
        if fld:
            self.conf_blocks[fld].extend(self.rebased(self.evaluate(call.args[1])))

    def type_from_features(self, call):
        for kw in call.keywords:
            if kw.arg == "features" and isinstance(kw.value, ast.Constant) and isinstance(kw.value.value, str):
                f = kw.value.value
                if "shlib" in f:
                    return ProjectType.SHARED
                if "stlib" in f:
                    return ProjectType.STATIC
                if "program" in f:
                    return ProjectType.EXECUTABLE
        if any(kw.arg == "source" for kw in call.keywords):
            return ProjectType.EXECUTABLE
        return None

    def parse_project_arguments(self, call, ptype):
        project = Project(type=ptype)

        for kw in call.keywords:
            if kw.arg is None:
                continue

            if kw.arg == "target":
                value = self.evaluate(kw.value)
                if isinstance(value, Blocks):
                    value = value[0].items[0] if value and value[0].items else ""
                project.name = str(value or "")
                continue

            target = {
                "source": project.sources,
                "includes": project.includes,
                "defines": project.defines,
                "use": project.libs,
                "uselib": project.libs,
                "lib": project.libs,
                "libpath": project.libpath,
                "cflags": project.cflags,
                "cxxflags": project.cxxflags,
                "linkflags": project.linkflags,
            }.get(kw.arg)

            if target is None:
                continue

            target.extend(self.rebased(self.evaluate(kw.value)))

        self.projects.append(project)

    def assign_name(self, name, value_node):
        if name == "PROJECT_NAME" and isinstance(value_node, ast.Constant):
            self.variables[name] = value_node.value
            return
        super().assign_name(name, value_node)
