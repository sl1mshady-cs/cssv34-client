#!/usr/bin/env python3
# parser
import ast
import argparse
from pathlib import Path
from dataclasses import dataclass, field
from typing import List, Optional
from enum import Enum

@dataclass
class Condition:
    expression: str


@dataclass
class ConditionalList:
    items: List[str]
    conditions: tuple[Condition, ...] = ()

# project type enum
class ProjectType(Enum):
    EXECUTABLE = "exe"
    STATIC = "static"
    SHARED = "shared"


@dataclass
class Project:
    name: str = ""
    type: ProjectType | None = None
    sources: list[ConditionalList] = field(default_factory=list)
    includes: list[ConditionalList] = field(default_factory=list)
    defines: list[ConditionalList] = field(default_factory=list)
    libs: list[ConditionalList] = field(default_factory=list)


class WafParser(ast.NodeVisitor):

    def __init__(self):
        self.project = Project()
        self.variables = {}
        self.condition_stack = []

    def parse(self, filename):
        source = Path(filename).read_text(encoding="utf-8")
        tree = ast.parse(source, filename)
        self.visit(tree)

        return self.project

    def visit_Module(self, node):
        for stmt in node.body:
            self.visit(stmt)

    def visit_Assign(self, node):
        if len(node.targets) != 1:
            return

        target = node.targets[0]

        if not isinstance(target, ast.Name):
            return

        name = target.id

        if name == "PROJECT_NAME":
            if isinstance(node.value, ast.Constant):
                self.project.name = node.value.value
                self.variables["PROJECT_NAME"] = node.value.value
                return

        self.handle_variable_assignment(name, node.value)
        
    def normalize(self, value):
        if value is None:
            return []

        if not value:
            return []

        if isinstance(value[0], ConditionalList):
            return value

        return [
            ConditionalList(
                items=value,
                conditions=self.condition_stack.copy()
            )
        ]

    def handle_variable_assignment(self, name, value):
        blocks = self.normalize(self.evaluate(value))

        if not self.condition_stack:
            self.variables[name] = blocks
        else:
            self.variables.setdefault(name, []).extend(
                ConditionalList(
                    items=b.items,
                    conditions=self.condition_stack.copy()
                )
                for b in blocks
            )

    def evaluate(self, node):
        if isinstance(node, ast.Constant):
            return node.value

        if isinstance(node, ast.List):
            return [self.evaluate(i) for i in node.elts]

        if isinstance(node, ast.Name):
            return self.variables.get(node.id, [])

        if isinstance(node, ast.BinOp):

            if isinstance(node.op, ast.Add):

                left = self.normalize(self.evaluate(node.left))
                right = self.normalize(self.evaluate(node.right))

                return left + right

        if isinstance(node, ast.Attribute):

            text = self.attribute_to_string(node)

            if text == "bld.env.INCLUDES_SDL2":

                return ["${SDL2_INCLUDE_DIRS}"]

            return []

    def visit_AugAssign(self, node):
        if not isinstance(node.target, ast.Name):
            return

        if not isinstance(node.op, ast.Add):
            return

        rhs = self.evaluate(node.value)

        if not rhs:
            return

        if isinstance(rhs[0], ConditionalList):
            blocks = rhs
        else:
            blocks = [
                ConditionalList(
                    items=rhs,
                    conditions=self.condition_stack.copy()
                )
            ]

        self.variables.setdefault(node.target.id, []).extend(blocks)

    def visit_FunctionDef(self, node):
        if node.name != "build":
            return

        for stmt in node.body:
            self.visit(stmt)

    def attribute_to_string(self, node):
        if isinstance(node, ast.Name):
            return node.id

        if isinstance(node, ast.Attribute):
            return self.attribute_to_string(node.value) + "." + node.attr

        return ""

    def visit_Expr(self, node):
        if isinstance(node.value, ast.Call):

            self.handle_call(node.value)

    def handle_call(self, call):
        if isinstance(call.func, ast.Name):
            if call.func.id == "bld":
                self.project.type = ProjectType.EXECUTABLE
                self.parse_project_arguments(call)

        if not isinstance(call.func, ast.Attribute):
            return

        name = call.func.attr

        if name == "shlib":
            self.project.type = ProjectType.SHARED

        elif name == "stlib":
            self.project.type = ProjectType.STATIC

        elif name == "__call__":
            self.project.type = ProjectType.EXECUTABLE

        elif name == "program":
            self.project.type = ProjectType.EXECUTABLE

        else:
            return

        self.parse_project_arguments(call)
        
    def parse_project_arguments(self, call):
        for kw in call.keywords:
            value = self.evaluate(kw.value)

            #print(kw.arg, value)

            if kw.arg == "source":
                self.project.sources.extend(self.normalize(value))

            elif kw.arg == "includes":
                self.project.includes.extend(self.normalize(value))

            elif kw.arg == "defines":
                self.project.defines.extend(self.normalize(value))

            elif kw.arg == "use":
                self.project.libs.extend(self.normalize(value))

            elif kw.arg == "target":
                self.project.name = value

    def visit_If(self, node):
        #
        # if
        #

        self.condition_stack.append(
            Condition(self.convert_condition(node.test))
        )

        for stmt in node.body:
            self.visit(stmt)

        self.condition_stack.pop()

        #
        # else / elif
        #

        if not node.orelse:
            return

        #
        # elif
        #

        if len(node.orelse) == 1 and isinstance(node.orelse[0], ast.If):
            self.visit(node.orelse[0])
            return

        #
        # else
        #

        self.condition_stack.append(
            Condition(f"NOT ({self.convert_condition(node.test)})")
        )

        for stmt in node.orelse:
            self.visit(stmt)

        self.condition_stack.pop()

    def convert_condition(self, node):
        #
        # not ...
        #
        if isinstance(node, ast.UnaryOp):
            if isinstance(node.op, ast.Not):
                return f"NOT ({self.convert_condition(node.operand)})"

        #
        # and/or
        #
        if isinstance(node, ast.BoolOp):

            op = "AND"

            if isinstance(node.op, ast.Or):
                op = "OR"

            return f" {op} ".join(
                f"({self.convert_condition(v)})"
                for v in node.values
            )

        #
        # compare
        #
        if isinstance(node, ast.Compare):

            left = self.attribute_to_string(node.left)
            right = self.evaluate(node.comparators[0])
            op = node.ops[0]

            #
            # DEST_OS
            #

            if left == "bld.env.DEST_OS":

                if right == "win32":
                    right = "WIN32"
                else:
                    right = str(right).upper()

                if isinstance(op, ast.Eq):
                    return right

                if isinstance(op, ast.NotEq):
                    return f"NOT ({right})"

            #
            # DEST_CPU
            #

            if left == "bld.env.DEST_CPU":

                if right == "amd64":
                    right = "AMD64"
                else:
                    right = str(right).upper()

                if isinstance(op, ast.Eq):
                    return right

                if isinstance(op, ast.NotEq):
                    return f"NOT {right}"

        #
        # bld.env.GL
        #

        if isinstance(node, ast.Attribute):
            if self.attribute_to_string(node) == "bld.env.GL":
                return "GL"

        return "UNKNOWN"