@echo off
setlocal
cmake --preset release
if errorlevel 1 exit /b %errorlevel%
cmake --build --preset release
exit /b %errorlevel%
