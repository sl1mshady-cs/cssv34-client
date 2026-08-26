# CS:S v34 Engine (new)
[![GitHub Actions Status](https://github.com/sl1mshady-cs/cssv34-client/actions/workflows/win64_build.yml/badge.svg)](https://github.com/sl1mshady-cs/cssv34-client/actions/workflows/win64_build.yml)
[![GitHub Actions Status](https://github.com/sl1mshady-cs/cssv34-client/actions/workflows/win32.yml/badge.svg)](https://github.com/sl1mshady-cs/cssv34-client/actions/workflows/win32.yml)
[![GitHub Actions Status](https://github.com/sl1mshady-cs/cssv34-client/actions/workflows/linux-amd64_build.yml/badge.svg)](https://github.com/sl1mshady-cs/cssv34-client/actions/workflows/linux-amd64_build.yml)
[![GitHub Actions Status](https://github.com/sl1mshady-cs/cssv34-client/actions/workflows/linux-i386_build.yml/badge.svg)](https://github.com/sl1mshady-cs/cssv34-client/actions/workflows/linux-i386_build.yml)
[![GitHub Actions Status](https://github.com/sl1mshady-cs/cssv34-client/actions/workflows/android-armv7a_build.yml/badge.svg)](https://github.com/sl1mshady-cs/cssv34-client/actions/workflows/android-armv7a_build.yml)

Information from [wikipedia](https://wikipedia.org/wiki/Source_(game_engine)):

Source is a 3D game engine developed by Valve.
It debuted as the successor to GoldSrc with Half-Life: Source in June 2004,
followed by Counter-Strike: Source and Half-Life 2 later that year.
Source does not have a concise version numbering scheme; instead, it was released in incremental versions

Source code is based on [nillerusr's source engine](https://github.com/nillerusr/source-engine) and patched to work with CS:S v34 servers.

**Don't use it for commercial purposes.**

This project is using waf buildsystem. If you have waf-related questions look https://waf.io/book

Releases can be found [here](https://github.com/sl1mshady-cs/cssv34-client/releases)

---

# Features:
- Android, OSX, FreeBSD, Windows, Linux( glibc, musl ) support
- Arm support( except windows )
- 64bit support
- Modern toolchains support
- Fixed many undefined behaviours
- Touch support( even on windows/linux/osx )
- VTF 7.5 support
- PBR support
- bsp v19-v21 support( bsp v21 support is partial, portal 2 and csgo maps works fine )
- mdl v46-49 support
- Removed useless/unnecessary dependencies
- Achivement system working without steam
- Fixed many bugs
- Serverbrowser works without steam

Known issues:
- rare "unknown shaders" bug (will crash), partially fixed
- unknown crashes

---

Currently, stable support is only available for Windows.<br>
Android, Linux, other platforms coming soon.

# How to Build?

Windows:
- Run configure_cstrike_release.bat or configure_cstrike_debug.bat
- If you want target 32 bit, add `-4` or `--32bits` at the end
- OPTIONAL: Run create_visualstudio_solution.bat to create the visual studio solution
- Run build_projects.bat
- Output files will be copied to `./__build__`

---

# How to Run?

- You need to download content. Download it from: [here](https://drive.google.com/file/d/1wzovBhjmJ_mDTpS3Lq89x57zhHTykgcN/view)
- Unpack this content into `__build__` folder
- Run hl2_launcher.exe with `-game cstrike` (you can create .bat file for it)

---

# Debugging the engine
- Set hl2_launcher as the startup project (if it isn't already) by right clicking it and pressing "Set as Startup Project".
- Right click launcher_main, go to properties and click on the debugging section. Set "Command" to point to your compiled `hl2_launcher.exe` (in the `__build__` folder).
- Set "Command Line Arguments" to `-game cstrike -insecure -sw -dev -allowdebug` (feel free to add more such as `+sv_cheats 1`).
- Press "Local Windows Debugger" at the top of Visual Studio to then launch the game and debug it.
