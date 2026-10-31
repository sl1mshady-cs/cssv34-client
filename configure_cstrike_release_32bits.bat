@echo off

set INCLUDE=
set LIB=
set LIBPATH=

set VSCMD_VER=
set VSINSTALLDIR=
set VCINSTALLDIR=
set VisualStudioVersion=

waf.bat configure -T release -4 --prefix=../game/win32_release --build-games=cstrike --disable-warns --enable-speex --enable-opus %*
