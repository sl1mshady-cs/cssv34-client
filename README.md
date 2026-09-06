# CS:S v34 Client
[![GitHub Actions Status](https://github.com/sl1mshady-cs/cssv34-client/actions/workflows/build.yml/badge.svg)](https://github.com/sl1mshady-cs/cssv34-client/actions/workflows/build.yml)

This project is aimed to make [nillerusr's source engine](https://github.com/nillerusr/source-engine) compatible with CS:S v34 servers and allow cross-platform play.

**Don't use it for commercial purposes.**

This project is using waf buildsystem. If you have waf-related questions look https://waf.io/book

Releases can be found [here](https://github.com/sl1mshady-cs/cssv34-client/releases)

---

# Features:
- Android (armeabi-v7a), Windows (x86, x64), Linux (i386, amd64) support
- Modern toolchains support
- Fixed many undefined behaviours
- Touch support( even on windows/linux/osx )
- VTF 7.5 support
- PBR support
- bsp v19-v21 support( bsp v21 support is partial, portal 2 and csgo maps works fine )
- mdl v46-49 support
- Removed useless/unnecessary dependencies
- New code from [CS:GO 2019 leak](https://github.com/rusherr-c/csgo-src)
- Fixed many bugs
- Serverbrowser works without steam

---

# Contributing
Read [CONTRIBUTING.md](https://github.com/sl1mshady-cs/cssv34-client/blob/master/CONTRIBUTING.md)

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
