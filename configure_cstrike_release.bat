@echo off

set INCLUDE=
set LIB=
set LIBPATH=

set VSCMD_VER=
set VSINSTALLDIR=
set VCINSTALLDIR=
set VisualStudioVersion=

waf.bat configure -T release --prefix=../game/win64_release --build-games=cstrike --disable-warns --enable-speex --enable-opus %*
