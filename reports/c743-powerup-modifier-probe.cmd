@echo off
setlocal
pushd "%~dp0.."
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars32.bat" >nul
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++17 /EHsc /MD /O2 /Gy /Gw /DWIN32 /D_WINDOWS /DNDEBUG /D_CRT_SECURE_NO_WARNINGS /Iinclude reports/c743-powerup-modifier-probe.cpp /Folocal/output/c743-powerup-probe.obj /Felocal/output/c743-powerup-probe.exe /link /MANIFEST:EMBED /OPT:REF /OPT:ICF build/win32/Release/bsp_core.lib build/win32/Release/bsp_lua511.lib build/win32/Release/bsp_zlib121.lib d3d9.lib user32.lib gdi32.lib ole32.lib advapi32.lib shell32.lib PowrProf.lib
if errorlevel 1 exit /b %errorlevel%
local\output\c743-powerup-probe.exe
set probe_result=%errorlevel%
popd
exit /b %probe_result%
