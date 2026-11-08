@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars32.bat" >nul
cl /nologo /std:c++20 /EHsc /MD /O2 /Gy /fp:strict /Iinclude /Ithird_party/lua/src /c tests/wire_g5_c795_ai_move_probe.cpp /Folocal/output/c795_ai_move_probe.obj
if errorlevel 1 exit /b 1
powershell -NoProfile -Command "Get-ChildItem -LiteralPath build/win32/bsp_game.dir/Release -Filter *.obj | Where-Object Name -NotIn game_main.obj,game_hosts_ai.obj | ForEach-Object { '\"' + $_.FullName + '\"' } | Set-Content -Encoding ascii local/output/c795_ai_move_probe.rsp"
link /nologo /OPT:REF /SUBSYSTEM:CONSOLE /MACHINE:X86 /MANIFEST:EMBED /OUT:local/output/c795_ai_move_probe.exe local/output/c795_ai_move_probe.obj @local/output/c795_ai_move_probe.rsp build/win32/bsp_core.dir/Release/cmake_pch.obj build/win32/Release/bsp_core.lib build/win32/Release/bsp_lua511.lib build/win32/Release/bsp_zlib121.lib d3d9.lib user32.lib gdi32.lib ole32.lib advapi32.lib shell32.lib PowrProf.lib kernel32.lib winspool.lib oleaut32.lib uuid.lib comdlg32.lib
if errorlevel 1 exit /b 1
local\output\c795_ai_move_probe.exe
