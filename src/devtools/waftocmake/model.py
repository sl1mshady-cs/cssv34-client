#!/usr/bin/env python3
# shared data model
from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum


@dataclass(frozen=True)
class Condition:
    expression: str   # CMake if() expression
    note: str = ""    # non-empty => original python source that could not be converted


@dataclass
class ConditionalList:
    items: list[str]
    conditions: tuple[Condition, ...] = ()


class Blocks(list):
    """A list of ConditionalList. Distinguishes 'waf list value' from plain python lists."""


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
    libpath: list[ConditionalList] = field(default_factory=list)
    cflags: list[ConditionalList] = field(default_factory=list)
    cxxflags: list[ConditionalList] = field(default_factory=list)
    linkflags: list[ConditionalList] = field(default_factory=list)


def rebase(block_conditions, use_stack):
    """Conditions of a stored block, as seen from a use site with condition stack `use_stack`."""
    block_conditions = tuple(block_conditions)
    use_stack = tuple(use_stack)
    common = 0
    while (
        common < len(use_stack)
        and common < len(block_conditions)
        and use_stack[common].expression == block_conditions[common].expression
    ):
        common += 1
    return use_stack + block_conditions[common:]
