#!/usr/bin/env python3
# generator
from pathlib import Path
import logging

CMAKE_Minimum_Required = "3.27"

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

logger = logging.getLogger(__name__)

def write_rootproject(outfile, name, projects):
    Path(outfile).parent.mkdir(parents=True, exist_ok=True)

    print(f"{bcolors.OKBLUE}Writing root project '{name}' with: {bcolors.OKCYAN}{bcolors.BOLD}", end="")
    logger.info(f" Writing root project '{name}' with:")

    with open(outfile, "w", encoding="utf-8") as f:
        f.write(f"cmake_minimum_required (VERSION {CMAKE_Minimum_Required})\n\n")

        f.write(f"project({name})\n\n")

        # for AMD64
        f.write(
        "if(CMAKE_SIZEOF_VOID_P EQUAL 8)\n"
        "\tset(AMD64 TRUE)\n"
        "else()\n"
        "\tset(AMD64 FALSE)\n"
        "endif()\n\n"
        )

        f.write("# Add all projects\n")
        for project in projects:
            f.write(f"add_subdirectory({project})\n")
            
            # TODO: add parsing for root wscript
            # defines, uselib, etc.......

            print(f"{project}", end=" ")
            logger.info(f"{project}")

        print(f"{bcolors.ENDC}")