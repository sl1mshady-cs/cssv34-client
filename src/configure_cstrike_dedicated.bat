@echo off

set INCLUDE=
set LIB=
set LIBPATH=

set VSCMD_VER=
set VSINSTALLDIR=
set VCINSTALLDIR=
set VisualStudioVersion=

waf.bat configure -T release --prefix=../game/windows_srcds --dedicated --build-games=cstrike --disable-warns --enable-speex --enable-opus %*
