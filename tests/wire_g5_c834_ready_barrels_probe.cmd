@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars32.bat" >nul
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++17 /EHsc /MD /O2 /fp:strict /I include tests/wire_g5_c834_ready_barrels_probe.cpp /Fo:local/c834-ready-barrels-probe.obj /Fe:local/c834-ready-barrels-probe.exe /link /MANIFEST:EMBED build/win32/bsp_game.dir/Release/director_geommesh_seg38_39_argon.obj build/win32/bsp_game.dir/Release/cmake_pch.obj build/win32/Release/bsp_core.lib build/win32/Release/bsp_lua511.lib build/win32/Release/bsp_zlib121.lib d3d9.lib user32.lib gdi32.lib ole32.lib advapi32.lib shell32.lib PowrProf.lib oleaut32.lib uuid.lib comdlg32.lib winspool.lib
if errorlevel 1 exit /b %errorlevel%
local\c834-ready-barrels-probe.exe
