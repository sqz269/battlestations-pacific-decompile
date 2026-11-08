@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars32.bat" >nul
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++17 /EHsc /O2 /MD /fp:strict /I include /Gy /Fo:local\ /Fe:local\c832-leg-owner-probe.exe tests\wire_g5_c832_leg_owner_probe.cpp src\leg_altitude_manager_cc13.cpp build\win32\Release\bsp_core.lib build\win32\Release\bsp_lua511.lib build\win32\Release\bsp_zlib121.lib user32.lib d3d9.lib PowrProf.lib shell32.lib /link /OPT:REF /MANIFEST:EMBED
if errorlevel 1 exit /b %errorlevel%
local\c832-leg-owner-probe.exe
