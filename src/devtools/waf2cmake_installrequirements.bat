@echo off
@setlocal
chcp 65001 >nul

where python.exe /Q
if %ERRORLEVEL% neq 0 exit /b 1
where pip.exe /Q
if %ERRORLEVEL% neq 0 exit /b 1

pip install argparse alive_progress logging pathlib dataclasses typing ast
pause