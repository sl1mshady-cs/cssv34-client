@echo off

set INCLUDE=
set LIB=
set LIBPATH=

set VSCMD_VER=
set VSINSTALLDIR=
set VCINSTALLDIR=
set VisualStudioVersion=

waf.bat configure -T fastnative --prefix=../game/win64_release_fast --build-games=cstrike --disable-warns --enable-speex --enable-opus %*
