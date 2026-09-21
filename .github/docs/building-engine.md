# Building instructions:
## Linux:
### Ubuntu 18.04 (bionic) / Debian 10 (buster)
#### Preparing
- Install `git` and clone this repository with `git clone --recursive --depth 1` command. `--recursive` key for init and update submodules on clone, and `--depth 1` key for clone only one last commit from branch.
- Add `i386` architecture to dpkg with `dpkg --add-architecture i386` command.
- Install `build-essential gcc-multilib g++-multilib pkg-config ccache` packages.
#### Required dependencies
- `libsdl2-dev:i386 libfreetype6-dev:i386 libfontconfig1-dev:i386 libopenal-dev:i386 libjpeg-dev:i386 libpng-dev:i386 libcurl4-gnutls-dev:i386 libbz2-dev:i386 libedit-dev:i386` for 64bit system.
- You can use `libcurl4-openssl-dev` or `libcurl4-nss-dev` instead of `libcurl4-gnutls-dev`.
##### OPUS
- If you want a voice chat support, add `--enable-opus` argument to WAF and follow this instructions.
- **DO NOT INSTALL DEFAULT `libopus-dev` PACKAGE!** This build of opus doesnt contains a custom modes support!!
- Clone latest opus sources with `git clone --recursive --depth 1 https://github.com/xiph/opus` command and `cd` to it.
- Run `sudo apt update; sudo apt install automake autoconf libtool`.
- `export CFLAGS="-m32" CPPFLAGS="-m32"` for 64bit system, required for 32bit engine build.
- After basic configure script dependencies installation run `./autogen.sh && ./configure --enable-custom-modes && make -j$(nproc) && sudo make install`.
- Now you can continue build engine.
### Arch Linux and Arch Linux based distros
#### Required dependencies
- Install `git python gcc gcc-multilib sdl2 freetype2 fontconfig zlib bzip2 libjpeg libpng curl openal opus`.
- For 32 bit install `lib32-gcc-libs lib32-sdl2 lib32-freetype2 lib32-fontconfig lib32-zlib lib32-bzip2 lib32-libjpeg lib32-libpng lib32-curl lib32-openal lib32-opus`
- Follow [common build instructions](#common-build-instructions-for-all-os).
## Windows
- Install full version of Visual Studio 2026/2022 with MSVC v145/v143 and latest version of Windows SDK.
- Install [Python 3](https://python.org/downloads) and add it to Path.
- Follow [common build instructions](#common-build-instructions-for-all-os).
# Common build instructions (For all OS)
Using a WAF build system you need to install python. I've recommend to use Python 3.

### Configuration (Manual)
- Run `./waf configure -T BUILDMODE`, where `BUILDMODE` is `release`, `debug` or `fastnative` (fast release build).
- On Windows you need to use `.\waf.bat` or run WAF script with `python3 waf`.
- If waf says something like `/usr/bin/env: 'python': No such file or directory` use `python3 ` prefix before `./waf`.
- If you need a 32 bit build add `-4` or `--32bits` argument to WAF.
- Add `--prefix=DIRECTORY` where `DIRECTORY` is output installation directory for game binaries. 
Default `__build__` if you don't specify a prefix.
- Configure build games using `--build-games=GAME`, `GAME` can be one of those:
    > cstrike = Counter-Strike: Source // Default - This will be selected if you don't specify a game<br>
    > hl2 = Half-Life 2<br>
    > episodic = Half-Life 2 Episode 1<br>
    > hl2mp = Half-Life 2: Deathmatch<br>
    > dod = Day of Defeat<br>
    > portal = Portal<br>
- To supress all the warnings during compilation add `--disable-warns`.
- Want voice chat? Add `--enable-speex` `--enable-opus` arguments to WAF.
- **Want dedicated server?** Just add `-d` argument to WAF.

### Building (Manual)
- Run `./waf install` to build the engine and place all the binaries in directory that was specified in `--prefix`.
- Windows Alternative: simply run `build_projects.bat`.

### Simplified configuration and building on Windows:
- Run any of the `configure_cstrike_**.bat` scripts for the configuration you want .
- Run `create_visualstudio_solution.bat` to create Visual Studio solution and simplify your coding experience.
- Inside of Visual Studio you can manually build each one of the projects.
- Or use `build_all_projects` or `install_all_projects` shortcuts in `build aliases` folder.

## Android (on Linux)
### Preparing
- Download and extract Android NDK r10e from [here](https://github.com/android/ndk/wiki/Unsupported-Downloads).
- Download and extract [CLANG 11](https://github.com/llvm/llvm-project/releases/download/llvmorg-11.1.0/clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz) or use CLANG from [LLVM repository](https://apt.llvm.org) for APT-based distros.
- Run `export ANDROID_NDK_HOME="PATH/TO/NDK/android-ndk-r10e"` and `export PATH="PATH/TO/CLANG/bin:$PATH"` or `export PATH="/usr/lib/llvm-11/bin:$PATH"` if you're use llvm.sh script.
- Add to WAF args `--togles --android=armeabi-v7a-hard,host,21`. `armeabi-v7a-hard` can be replaced with `aarch64` for arm64 build, but you need to add `-8` or `--64bits` argument
