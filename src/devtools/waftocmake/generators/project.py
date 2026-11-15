#!/usr/bin/env python3
# generator
from pathlib import Path
import logging

from waftocmake.model import ProjectType
from waftocmake.generators.common import cm_arg, cm_path, emit_conditional, INDENT

logger = logging.getLogger(__name__)

HEADER = (
    "# ------------------------------------------------- #\n"
    "# waf2cmake automatically generated CMakeLists.txt,\n"
    "# Fix if needed before using.\n"
    "# ------------------------------------------------- #\n\n"
)


def lists2array(lists):
    return [item for lst in lists for item in lst.items]


def emit_block(f, command, target, lists, convert=None, wrap=None):
    """target_xxx(<target> PRIVATE ...) for every ConditionalList, grouped by conditions."""
    # merge neighbours that sit under exactly the same conditions
    merged = []
    for lst in lists:
        if not lst.items:
            continue
        if merged and [c.expression for c in merged[-1].conditions] == [c.expression for c in lst.conditions]:
            merged[-1] = type(lst)(merged[-1].items + lst.items, merged[-1].conditions)
        else:
            merged.append(type(lst)(list(lst.items), lst.conditions))
    lists = merged
    if not lists:
        return

    def render(lst):
        items, seen = [], set()
        for item in lst.items:
            item = convert(item) if convert else item
            if item not in seen:
                seen.add(item)
                items.append(wrap(item) if wrap else cm_arg(item))
        tail = [] if lst.conditions else [""]
        return [f"{command}({target} PRIVATE", *[INDENT + i for i in items], ")", *tail]

    emit_conditional(f, lists, render)


def generate(projects, outfile, uselib=None):
    """projects: Project or list[Project]; uselib: waf uselib name -> CMake expression"""
    if not isinstance(projects, (list, tuple)):
        projects = [projects]
    uselib = uselib or {}

    Path(outfile).parent.mkdir(parents=True, exist_ok=True)

    with open(outfile, "w", encoding="utf-8", newline="\n") as f:
        f.write(HEADER)

        for project in projects:
            write_target(f, project, uselib)


def write_target(f, project, uselib):
    name = project.name

    if project.type == ProjectType.EXECUTABLE:
        f.write(f"add_executable({name})\n\n")
    elif project.type == ProjectType.SHARED:
        f.write(f"add_library({name} SHARED)\n\n")
    elif project.type == ProjectType.STATIC:
        f.write(f"add_library({name} STATIC)\n\n")
    else:
        raise RuntimeError(project.type)

    logger.info(f" --SRC: {lists2array(project.sources)}")
    logger.info(f" --INCLUDES: {lists2array(project.includes)}")
    logger.info(f" --LIBS: {lists2array(project.libs)}")

    path = lambda s: cm_arg(cm_path(s))
    lib = lambda s: cm_arg(uselib.get(s, s))

    emit_block(f, "target_sources", name, project.sources, convert=cm_path)
    emit_block(f, "target_include_directories", name, project.includes, convert=cm_path)
    emit_block(f, "target_compile_definitions", name, project.defines)
    emit_block(f, "target_compile_options", name, project.cflags,
               wrap=lambda s: cm_arg(f"$<$<COMPILE_LANGUAGE:C>:{s}>"))
    emit_block(f, "target_compile_options", name, project.cxxflags,
               wrap=lambda s: cm_arg(f"$<$<COMPILE_LANGUAGE:CXX>:{s}>"))
    emit_block(f, "target_link_directories", name, project.libpath, convert=cm_path)
    emit_block(f, "target_link_options", name, project.linkflags)
    emit_block(f, "target_link_libraries", name, project.libs,
               convert=lambda s: uselib.get(s, s))
