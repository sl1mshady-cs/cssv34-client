#!/usr/bin/env python3
# python (waf) condition -> CMake if() expression
import ast
import re

from waftocmake.model import Condition

_ATOM = re.compile(r'[A-Za-z_][A-Za-z0-9_]*')
_NOT_ATOM = re.compile(r'NOT ([A-Za-z_][A-Za-z0-9_]*)')


def _outer_parens(s):
    """True if s is '(...)' and the first '(' closes at the very end."""
    if not (s.startswith('(') and s.endswith(')')):
        return False
    depth = 0
    for i, ch in enumerate(s):
        if ch == '(':
            depth += 1
        elif ch == ')':
            depth -= 1
            if depth == 0 and i != len(s) - 1:
                return False
    return depth == 0


def negate(expr):
    if _ATOM.fullmatch(expr):
        return f"NOT {expr}"
    m = _NOT_ATOM.fullmatch(expr)
    if m:
        return m.group(1)
    if expr.startswith("NOT ") and _outer_parens(expr[4:]):
        return expr[5:-1]
    return f"NOT ({expr})"


def paren(expr):
    if _ATOM.fullmatch(expr) or _NOT_ATOM.fullmatch(expr):
        return expr
    return f"({expr})"


class Unsupported(Exception):
    pass


OS_MAP = {
    'win32': 'WIN32',
    'darwin': 'APPLE',
    'linux': 'LINUX',
    'android': 'ANDROID',
}
BSD_NAMES = {
    'freebsd': 'FreeBSD',
    'openbsd': 'OpenBSD',
    'netbsd': 'NetBSD',
    'dragonflybsd': 'DragonFly',
}
# ARCH_* variables are defined by the generated root CMakeLists.txt
CPU_MAP = {
    'x86': 'ARCH_X86', 'i386': 'ARCH_X86', 'i686': 'ARCH_X86',
    'x86_64': 'ARCH_X86_64', 'amd64': 'ARCH_X86_64',
    'arm': 'ARCH_ARM',
    'aarch64': 'ARCH_AARCH64', 'arm64': 'ARCH_AARCH64',
}
# bare python names that appear in conditions of the root wscript
NAME_FLAGS = {
    'arch32': 'ARCH_32BIT',
    'arch64': 'ARCH_64BIT',
}


def dotted(node):
    if isinstance(node, ast.Name):
        return node.id
    if isinstance(node, ast.Attribute):
        base = dotted(node.value)
        return f"{base}.{node.attr}" if base else ""
    return ""


def env_name(node):
    """'conf.env.X' / 'bld.options.X' -> ('env' | 'options', 'X'), otherwise None."""
    parts = dotted(node).split('.')
    if len(parts) == 3 and parts[0] in ('conf', 'bld', 'opt', 'ctx') and parts[1] in ('env', 'options'):
        return parts[1], parts[2]
    return None


def _os_expr(v):
    if v in OS_MAP:
        return OS_MAP[v]
    if v in BSD_NAMES:
        return f'CMAKE_SYSTEM_NAME STREQUAL "{BSD_NAMES[v]}"'
    raise Unsupported


def _cpu_expr(v):
    if v in CPU_MAP:
        return CPU_MAP[v]
    raise Unsupported


def _compiler_expr(v):
    if v == 'msvc':
        return 'MSVC'
    if v == 'gcc':
        return 'CMAKE_C_COMPILER_ID STREQUAL "GNU"'
    if v == 'clang':
        return 'CMAKE_C_COMPILER_ID MATCHES "Clang"'
    raise Unsupported


def _build_type_expr(v):
    return f'CMAKE_BUILD_TYPE STREQUAL "{str(v).capitalize()}"'


class ConditionConverter:

    def convert(self, node):
        if isinstance(node, ast.BoolOp):
            op = " AND " if isinstance(node.op, ast.And) else " OR "
            return op.join(paren(self.convert(v)) for v in node.values)

        if isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.Not):
            return negate(self.convert(node.operand))

        if isinstance(node, ast.Compare):
            return self.compare(node)

        if isinstance(node, ast.Constant) and isinstance(node.value, bool):
            return "TRUE" if node.value else "FALSE"

        if isinstance(node, ast.Attribute):
            ref = env_name(node)
            if ref:
                return ref[1]

        if isinstance(node, ast.Name) and node.id in NAME_FLAGS:
            return NAME_FLAGS[node.id]

        raise Unsupported

    def compare(self, node):
        if len(node.ops) != 1:
            raise Unsupported

        op = node.ops[0]
        right = node.comparators[0]

        if isinstance(right, ast.Constant):
            values = [right.value]
        elif isinstance(right, (ast.List, ast.Tuple, ast.Set)) and all(isinstance(e, ast.Constant) for e in right.elts):
            values = [e.value for e in right.elts]
        else:
            raise Unsupported

        if not isinstance(op, (ast.Eq, ast.NotEq, ast.In, ast.NotIn)):
            raise Unsupported
        if not values or not all(isinstance(v, str) for v in values):
            raise Unsupported

        ref = env_name(node.left)
        if dotted(node.left) == 'sys.platform':
            mapper = _os_expr
        elif ref and ref[1] == 'DEST_OS':
            mapper = _os_expr
        elif ref and ref[1] == 'DEST_CPU':
            mapper = _cpu_expr
        elif ref and ref[1] == 'COMPILER_CC':
            mapper = _compiler_expr
        elif ref and ref[1] == 'BUILD_TYPE':
            mapper = _build_type_expr
        elif ref:
            name = ref[1]
            mapper = lambda v: f'{name} STREQUAL "{v}"'
        else:
            raise Unsupported

        exprs = list(dict.fromkeys(mapper(v) for v in values))
        expr = exprs[0] if len(exprs) == 1 else " OR ".join(paren(e) for e in exprs)

        if isinstance(op, (ast.NotEq, ast.NotIn)):
            return negate(expr)
        return expr


_converter = ConditionConverter()


def convert_condition(node) -> Condition:
    try:
        return Condition(_converter.convert(node))
    except Unsupported:
        # never silently enable code under a condition we do not understand
        return Condition("FALSE", note=ast.unparse(node))
