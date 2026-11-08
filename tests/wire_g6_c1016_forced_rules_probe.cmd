@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x86 -host_arch=x64
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++20 /EHsc /MD /O2 /Gy /Gw /fp:strict /D_CRT_SECURE_NO_WARNINGS /Iinclude tests\wire_g6_c1016_forced_rules_probe.cpp /Folocal\g6-c1016-probe.obj /Felocal\g6-c1016-probe.exe build\win32\Release\bsp_core.lib /link /MANIFEST:EMBED /OPT:REF /OPT:ICF
if errorlevel 1 exit /b %errorlevel%
local\g6-c1016-probe.exe
