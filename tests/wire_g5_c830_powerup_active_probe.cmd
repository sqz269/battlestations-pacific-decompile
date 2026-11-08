@echo off
setlocal
pushd "%~dp0.."
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -host_arch=x64 -arch=x86 >nul
if errorlevel 1 exit /b %errorlevel%
if not exist local\output mkdir local\output
(for %%F in (build\win32\bsp_game.dir\Release\*.obj) do if /I not "%%~nxF"=="game_hosts_powerups.obj" if /I not "%%~nxF"=="game_hosts_mission_result.obj" if /I not "%%~nxF"=="game_main.obj" echo "%%~fF") > local\output\c830-game-objects.rsp
cl /nologo /std:c++20 /EHsc /MD /O2 /Gy /fp:strict /DNDEBUG /Iinclude tests\wire_g5_c830_powerup_active_probe.cpp @local\output\c830-game-objects.rsp /Fo:local\output\c830-powerup-active-probe.obj /Fe:local\output\c830-powerup-active-probe.exe /link /MANIFEST:EMBED /OPT:REF build\win32\Release\bsp_core.lib build\win32\Release\bsp_lua511.lib build\win32\Release\bsp_zlib121.lib user32.lib d3d9.lib PowrProf.lib shell32.lib kernel32.lib gdi32.lib winspool.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib
if errorlevel 1 exit /b %errorlevel%
local\output\c830-powerup-active-probe.exe
