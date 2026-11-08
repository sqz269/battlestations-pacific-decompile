@echo off
setlocal
cd /d "%~dp0.."
for /f "usebackq tokens=*" %%v in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "BSP_C835_VS=%%v"
if not defined BSP_C835_VS exit /b 1
call "%BSP_C835_VS%\Common7\Tools\VsDevCmd.bat" -arch=x86 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /MD /O2 /Gy /Iinclude tests\wire_g5_c835_leave_map_probe.cpp /Fo:local\c835-leave-map-probe.obj /Fe:local\c835-leave-map-probe.exe build\win32\Release\bsp_core.lib build\win32\Release\bsp_lua511.lib build\win32\Release\bsp_zlib121.lib /link /MANIFEST:EMBED /OPT:REF user32.lib d3d9.lib PowrProf.lib shell32.lib
if errorlevel 1 exit /b 1
local\c835-leave-map-probe.exe
exit /b %errorlevel%
