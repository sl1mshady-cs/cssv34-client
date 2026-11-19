#!/usr/bin/env python3
# generator: CMakePresets.json (Visual Studio / CLion / VS Code / cmake --preset)
import json
from pathlib import Path

# (arch, VS architecture value, build type)
ARCHS = [("x64", "x64"), ("x86", "x86")]
TYPES = ["Debug", "Release"]

WINDOWS_ONLY = {"type": "equals", "lhs": "${hostSystemName}", "rhs": "Windows"}


def build_presets():
    configure = [
        {
            "name": "base",
            "hidden": True,
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/out/build/${presetName}",
            "installDir": "${sourceDir}/out/install/${presetName}",
        },
        {
            "name": "windows-base",
            "hidden": True,
            "inherits": "base",
            "condition": WINDOWS_ONLY,
            "cacheVariables": {
                "CMAKE_C_COMPILER": "cl.exe",
                "CMAKE_CXX_COMPILER": "cl.exe",
            },
        },
    ]
    build = []

    for arch, vs_arch in ARCHS:
        for btype in TYPES:
            name = f"{arch}-{btype}"
            configure.append({
                "name": name,
                "displayName": f"{arch} {btype}",
                "inherits": "windows-base",
                # "external": the IDE / vcvarsall sets up the environment for the architecture
                "architecture": {"value": vs_arch, "strategy": "external"},
                "cacheVariables": {"CMAKE_BUILD_TYPE": btype},
            })
            build.append({
                "name": name,
                "configurePreset": name,
                "condition": WINDOWS_ONLY,
            })

    return {
        "version": 6,
        "cmakeMinimumRequired": {"major": 3, "minor": 27, "patch": 0},
        "configurePresets": configure,
        "buildPresets": build,
    }


def write_presets(outfile, overwrite=False):
    """Returns True if the file was written. An existing file is kept unless overwrite=True,
    because people edit presets by hand (and the root CMakeLists.txt is regenerated, this isn't)."""
    outfile = Path(outfile)
    if outfile.exists() and not overwrite:
        return False
    outfile.write_text(json.dumps(build_presets(), indent=4) + "\n", encoding="utf-8", newline="\n")
    return True
