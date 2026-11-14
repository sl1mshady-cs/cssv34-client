@echo off
@setlocal
chcp 65001 >nul

where python.exe /Q
if %ERRORLEVEL% neq 0 exit /b 1
where pip.exe /Q
if %ERRORLEVEL% neq 0 exit /b 1

python %~dp0/waf2cmake %*