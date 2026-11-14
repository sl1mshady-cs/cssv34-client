#!/usr/bin/env python3
# generator
from pathlib import Path
import logging
from waftocmake.parser import ProjectType

def quote(v):
    return '"' + v.replace("\\", "/") + '"'

def emit_block(f, command, target, lists):
    if not lists:
        return

    opened = []

    for lst in lists:
        common = 0
        while (
            common < len(opened)
            and common < len(lst.conditions)
            and opened[common].expression == lst.conditions[common].expression
        ):
            common += 1

        while len(opened) > common:
            f.write("endif()\n")
            opened.pop()

        while len(opened) < len(lst.conditions):
            cond = lst.conditions[len(opened)]
            f.write(f"if({cond.expression})\n")
            opened.append(cond)


        f.write(f"{command}({target} PRIVATE\n")
        for item in lst.items:
            f.write(f"    {item.replace('\\', '/')}\n")

        f.write(")\n\n")

    while opened:
        f.write("endif()\n")
        opened.pop()

    f.write("\n")

logger = logging.getLogger(__name__)

def lists2array(lists):
    array = []
    for lst in lists:
        for item in lst.items:
            array.append(item)

    return array

# generate
def generate(project, outfile):

    Path(outfile).parent.mkdir(parents=True, exist_ok=True)

    with open(outfile, "w", encoding="utf-8") as f:

        f.write("# ------------------------------------------------- #\n")
        f.write("# waf2cmake automatically generated CMakeLists.txt,\n")
        f.write("# Fix if needed before using.\n")
        f.write("# ------------------------------------------------- #\n\n")

        #
        # create target
        #

        if project.type == ProjectType.EXECUTABLE:
            f.write(f"add_executable({project.name})\n\n")

        elif project.type == ProjectType.SHARED:
            f.write(f"add_library({project.name} SHARED)\n\n")

        elif project.type == ProjectType.STATIC:
            f.write(f"add_library({project.name} STATIC)\n\n")

        else:
            raise RuntimeError(project.type)

        #
        # properties
        #

        logger.info(f" --SRC: {lists2array(project.sources)}")
        logger.info(f" --INCLUDES: {lists2array(project.includes)}")
        logger.info(f" --LIBS: {lists2array(project.libs)}")


        emit_block(
            f,
            "target_sources",
            project.name,
            project.sources
        )

        emit_block(
            f,
            "target_include_directories",
            project.name,
            project.includes
        )

        emit_block(
            f,
            "target_compile_definitions",
            project.name,
            project.defines
        )

        emit_block(
            f,
            "target_link_libraries",
            project.name,
            project.libs
        )