# CC16 Source active-cause /MD probe

One fresh Win32 `/MD /EHsc` probe passed the SAME fifteen checks and SDK key reader as `/MT` receipt `423fd389c41bffe8e353f677bf56c41db31a9f7b`. Root reported `bsp_core` Release uses `MultiThreadedDLL` and `/EHsc`; this follow-up checks that runtime configuration in a fresh local executable. It does not execute `bsp_core`, the game or a production caller. Changed Text96 remains held, and active-object identity provides no Native-origin permission.

Only runtime-module provenance logging, configuration banners and provenance failure in the exit status were added to the previous local Source. Mechanical normalization reproduces the `/MT` Source byte-for-byte; all fifteen check bodies, SDK reader, bare-rethrow, distinct-secondary and diagnostic-capture sequences are identical. The `/MT` executable was not rerun. The new Source/exe/logs/map remain ignored under `local/output/source_cause_md_probe_20261030/`; [the report](../reports/cc16_source_active_cause_md_probe.json) pins their exact bytes and full commands.

MSVC toolset folder `14.51.36231`, compiler `19.51.36244.0`, STL `202604`, Windows SDK `10.0.26100.0` built with `/std:c++20 /EHsc /MD /Od /W4 /D_VCRT_ALLOW_INTERNALS`. Existing SDK `trnsctrl.h`, `ehdata_forceinclude.h`, `EHExceptionRecord` and `PER_PEXCEPTOBJ/PER_PTHROW` are used; no redeclared API, private offset/layout, vendor replacement or numeric function cast was added. Embedded `asInvoker` manifest and PE32 x86 were checked before the sole run. Compile/run were normal priority, bounded to 45/10 seconds, completed in about 1.96/0.061 seconds and returned 0.

PE imports and map bind `__current_exception`, `_CxxThrowException` and `__CxxFrameHandler3` to the SAME `VCRUNTIME140.dll`; diagnostic exception-pointer functions bind to `MSVCP140.dll`. The three selected VCRUNTIME exports are not forwarded. The actual loaded files were:

| Logged module | Physical x86 backing corroborated | File version |
| --- | --- | --- |
| `VCRUNTIME140.dll` | `C:/Windows/SysWOW64/VCRUNTIME140.dll` | `14.51.36247.0` |
| `MSVCP140.dll` | `C:/Windows/SysWOW64/MSVCP140.dll` | `14.51.36247.0` |
| `ucrtbase.dll` | `C:/Windows/SysWOW64/ucrtbase.dll` | `10.0.26100.9711` |

`vcruntime140_1.dll` was not loaded. GetModuleFileName logged System32 paths from the 32-bit process. A separate file-metadata-only 32-bit reader opened those reported paths, and each hash/size/PE014C exactly matched the physical SysWOW64 file. This avoids hashing a 64-bit System32 DLL from a 64-bit shell. The first metadata read lacked `Get-FileHash`; builtin .NET SHA256 completed that file-hash operation. This was no behavioral probe rerun. Versions, hashes, commands and import-library pins are in the report. Actual DLL build `14.51.36247` is recorded separately from compiler/SDK build `36244`.

All fifteen checks passed: seven SDK reader calls only within supported active catches, no reader-induced exception copy/destruction or tested payload mutation, actual caught-object match, repeated/bare-rethrow object identity, separate live secondary object, separate call-edge identities, and two `current_exception()` captures producing two copies and comparing unequal. The EH-record address happened to remain equal on rethrow; its equality is not required as identity. All four exception objects ended once. This checks current own-runtime C++ identity; the secondary event is not Native cleanup FH3 validation.

No general nonthrow, arbitrary-CRT/module/managed/SEH, Original gameCRT/ABI or activation proof follows. This fresh executable's runtime resolution does not establish the production application's module loader/provider graph. Private actual cause issuers, edge freshness, sticky committed host/unknown denial, stale sibling/reentry rejection and cleanup-secondary permission still require their coupled Source contracts. No production API, CMake, ledger, Ghidra, application, owner or process-control change occurred, and no other probe/framework was added.
