@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -host_arch=x64 -arch=x86 >nul
if not exist local\output\c619_off_include\bsp mkdir local\output\c619_off_include\bsp
>local\output\c619_off_include\bsp\wire_goal4_chain619_switches.hpp echo #pragma once
>>local\output\c619_off_include\bsp\wire_goal4_chain619_switches.hpp echo namespace bsp { inline constexpr bool kWireGoal4Chain619Bound = false; }
cl /nologo /std:c++17 /EHsc /MD /O2 /W4 /WX /I include local/c619_substep_probe.cpp /Fo:local/output/c619_substep_on.obj /Fe:local/output/c619_substep_on_probe.exe /link /MANIFEST:EMBED build/win32/Release/bsp_core.lib kernel32.lib user32.lib /OPT:REF
if errorlevel 1 exit /b 1
local\output\c619_substep_on_probe.exe > local\output\c619_substep_on.txt
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /MD /O2 /W4 /WX /I local/output/c619_off_include /I include local/c619_substep_probe.cpp /Fo:local/output/c619_substep_off.obj /Fe:local/output/c619_substep_off_probe.exe /link /MANIFEST:EMBED build/win32/Release/bsp_core.lib kernel32.lib user32.lib /OPT:REF
if errorlevel 1 exit /b 1
local\output\c619_substep_off_probe.exe > local\output\c619_substep_off.txt
if errorlevel 1 exit /b 1
fc /b local\output\c619_substep_on.txt local\output\c619_substep_off.txt
