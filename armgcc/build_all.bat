@echo off
setlocal
call "%~dp0build_debug.bat"
if errorlevel 1 exit /b %errorlevel%
call "%~dp0build_release.bat"
if errorlevel 1 exit /b %errorlevel%
if "%1"=="" pause
exit /b 0
