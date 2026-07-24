@echo off
set PATH=C:\mingw64\bin;C:\SDL2\bin;%PATH%
cd /d "%~dp0simulator\build"
start "" "LabBuddy_Simulator.exe"
