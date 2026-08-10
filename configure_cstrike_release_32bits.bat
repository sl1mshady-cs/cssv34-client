@echo off

set INCLUDE=
set LIB=
set LIBPATH=

set VSCMD_VER=
set VSINSTALLDIR=
set VCINSTALLDIR=
set VisualStudioVersion=

waf.bat configure -T release -4 --prefix=__build__ --build-games=cstrike --disable-warns %*
