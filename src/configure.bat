@echo off
@setlocal ENABLEDELAYEDEXPANSION
title WAF - Configure

rem // Display info
echo WAF - Configure
echo 8 options available. Choose one and configuration will continue.
echo.
echo  -- Game --
echo 1. Win64 Release
echo 2. Win64 FastNative
echo 3. Win64 Debug
echo.
echo 4. Win32 Release
echo 5. Win32 FastNative
echo 6. Win32 Debug
echo.
echo -- Other --
echo 7. Dedicated Server (Win64 Release)
echo 8. Tools (Win64 Release)
echo 9. MANUAL CONFIGURATION
echo.

rem // Display choice
choice /C 123456789 /N
echo %ERRORLEVEL%
if %ERRORLEVEL%==0 echo Aborted: CTRL+C was pressed. && pause && exit /b 0
if %ERRORLEVEL%==255 echo Aborted: Unknown Error && pause && exit /b -1

cls
set waf_cmd=waf.bat configure --build-games=cstrike --disable-warns
set waf_buildmode=
set waf_buildmode_alt=
set waf_archprefix=
set waf_voicemodules=
set waf_other=
set waf_manual=0

if %ERRORLEVEL%==9 goto setManual

goto setup

:setManual
set waf_manual=1
set /p "waf_manual_cmd=%waf_cmd% "
if "%waf_manual_cmd%"=="" cls && goto setManual

rem // Runs main command
:runCommand

if %waf_manual%==1 (
set "waf_cmd=%waf_cmd% %waf_manual_cmd%"
) else (
set "waf_cmd=%waf_cmd% -T %waf_buildmode% --prefix %waf_archprefix% %waf_voicemodules% %waf_other%"
)

cls
echo %waf_cmd%
call %waf_cmd%
choice /M "Create Visual Studio solution?"

if %ERRORLEVEL%==1 call waf.bat msvs
pause
exit /b 1

:setup
:setBuildmode
if %ERRORLEVEL%==1 set waf_buildmode=release
if %ERRORLEVEL%==4 set waf_buildmode=release
if %ERRORLEVEL% GTR 6 set waf_buildmode=release
set waf_buildmode_alt=%waf_buildmode%

if %ERRORLEVEL%==2 set waf_buildmode=fastnative && set waf_buildmode_alt=release_fast
if %ERRORLEVEL%==5 set waf_buildmode=fastnative && set waf_buildmode_alt=release_fast

if %ERRORLEVEL%==3 set waf_buildmode=debug && set waf_buildmode_alt=%waf_buildmode%
if %ERRORLEVEL%==6 set waf_buildmode=debug && set waf_buildmode_alt=%waf_buildmode%

:setArchitecture
if %ERRORLEVEL% LEQ 3 set waf_archprefix=../game/win64_%waf_buildmode_alt% && goto setVoiceModules
if %ERRORLEVEL% LEQ 6 set waf_archprefix=../game/win32_%waf_buildmode_alt% && goto setVoiceModules
if %ERRORLEVEL% GTR 6 goto setOther

goto runCommand

:setOther
if %ERRORLEVEL%==7 set waf_archprefix=../game/windows_srcds && set waf_other=--dedicated
if %ERRORLEVEL%==8 set waf_archprefix=../game/windows_tools && set waf_other=--tools

goto runCommand

:setVoiceModules
set waf_voicemodules=--enable-speex --enable-opus

goto runCommand