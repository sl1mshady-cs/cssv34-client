#!/usr/bin/env python3
# helpers shared by the generators
import re

INDENT = "    "

_NEEDS_QUOTES = re.compile(r'[\s"()#;\\\']')


def cm_arg(value: str) -> str:
    """CMake argument: quoted only when it has to be. `$` is left alone on purpose,
    so ${VAR} and $<genex> keep working."""
    if value == "" or _NEEDS_QUOTES.search(value):
        return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'
    return value


def cm_quoted(value: str) -> str:
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def cm_path(value: str) -> str:
    return value.replace("\\", "/")


def emit_conditional(f, entries, render):
    """Write `entries` (objects with .conditions) wrapped in if()/endif(), sharing common
    condition prefixes between neighbours. render(entry) -> list of lines."""
    opened = []

    def close_to(n):
        while len(opened) > n:
            opened.pop()
            f.write(INDENT * len(opened) + "endif()\n")

    for entry in entries:
        conds = list(entry.conditions)

        common = 0
        while (
            common < len(opened)
            and common < len(conds)
            and opened[common].expression == conds[common].expression
        ):
            common += 1
        close_to(common)

        while len(opened) < len(conds):
            c = conds[len(opened)]
            note = f"  # TODO(waf2cmake): unsupported condition: {c.note}" if c.note else ""
            f.write(INDENT * len(opened) + f"if({c.expression}){note}\n")
            opened.append(c)

        for line in render(entry):
            f.write((INDENT * len(opened) + line if line else "") + "\n")

    close_to(0)
