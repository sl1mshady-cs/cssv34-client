# CS:S v34 Engine (new)

Information from [wikipedia](https://wikipedia.org/wiki/Source_(game_engine)):

Source is a 3D game engine developed by Valve.
It debuted as the successor to GoldSrc with Half-Life: Source in June 2004,
followed by Counter-Strike: Source and Half-Life 2 later that year.
Source does not have a concise version numbering scheme; instead, it was released in incremental versions

Source code is based on TF2 2018 leak and patched to work with CS:S v34 servers.
Don't use it for commercial purposes.

This project is using waf buildsystem. If you have waf-related questions look https://waf.io/book

Releases can be found [here](https://github.com/sl1mshady-cs/cssv34-client-releases/releases)

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
- prediction (or collision) bug, when standing on entities, moving into entities, etc
- rare "unknown shaders" bug (will crash), partially fixed
- unknown crashes

# How to Build?

Windows (with Visual Studio):
- Run waf_configure_cstrike_release.bat or waf_configure_cstrike_debug.bat
- If you want target 32 bit, add `-4` at the end
- If you need voice support, add `--enable-speex --enable-opus` at the end
- Run waf_build.bat
- Output files will be copied to `./__build__`

Android, Linux, other platforms coming soon, just wait.

# How to Run?

- You need to download content. Download it from: [here](https://drive.google.com/file/d/1Jtc9HfyoX88ENAxPzjHopECaX9wsiaWI/view)
- Unpack this content into `__build__` folder
- Run hl2_launcher.exe with `-game cstrike` (you can create .bat file for it)
