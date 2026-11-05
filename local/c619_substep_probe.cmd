@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -host_arch=x64 -arch=x86 >nul
cl /nologo /std:c++17 /EHsc /MD /O2 /W4 /WX /I include local/c619_substep_probe.cpp /Fo:local/output/c619_substep_probe.obj /Fe:local/output/c619_substep_probe.exe /link /MANIFEST:EMBED build/win32/Release/bsp_core.lib kernel32.lib user32.lib /OPT:REF
if errorlevel 1 exit /b 1
local\output\c619_substep_probe.exe > local\output\c619_substep_probe.txt
if errorlevel 1 exit /b 1
type local\output\c619_substep_probe.txt
