# XInput activation import (CC10)

Addresses: `00BED3FA`, `00BED45F`, `00C2F172`, `00CE2398` in
`/battlestationspacific.exe` from `C:/Users/sqz269/bsp.gpr`.

The native `00C2F172` body is only `JMP dword ptr [00CE2398]`.
The original EXE's PE import table maps that IAT slot to ordinal 5 of
`XINPUT1_3.dll`; slots `00CE2394` and `00CE239C` import ordinals 3 and 2.
The selected default path resolves through the 32-bit process's
`GetSystemDirectoryW` as `C:/Windows/system32/XInput1_3.dll`; WOW64 maps
this to the 32-bit `C:/Windows/SysWOW64/XInput1_3.dll`. That DLL exports
ordinal 5 as `XInputEnable`, ordinal 3 as `XInputSetState`, and ordinal 2
as `XInputGetState`. Its SHA-256 is
`f76df8e21eab5f5ad4fd7efd8de24050236efd1d668c1309e393f136b6d6c14a`.
The installed SDK's `Xinput.h` declares `void WINAPI XInputEnable(BOOL)`.

| Routine / call site | Coverage and evidence |
| --- | --- |
| `00C2F172` | Complete native import thunk only. The external DLL body is not reconstructed. |
| `00BED3FA` | `BSP_Win32Platform_HandleMessage` pushes 0 then calls the thunk for inactive `WM_ACTIVATE`. No caller stack adjustment follows. |
| `00BED45F` | The same handler pushes 1 then calls the thunk for active `WM_ACTIVATE`. No caller stack adjustment follows. |

`XInputLibrary::enable(BOOL)` uses a `void (WINAPI*)(BOOL)` pointer
resolved as ordinal 5 from its already selected and owned module. Loading
fails if any required ordinal (2, 3, or 5) is missing; the failed module
is released. The host retains the library across window creation, activation
messages, and input devices. This typed wrapper is not a binary replacement
for the native import thunk.

The strict Win32 build and both existing CTests passed. The 32-bit probe
linked the built core, requested and loaded the same system path, found
the 145,280-byte DLL (the 64-bit System32 DLL is 182,656 bytes), called
disable followed by enable through `XInputLibrary`, and exited zero. Native DLL
internal state, controller response, and game behavior require separate
runtime evidence; no controller or game installation is changed here.
