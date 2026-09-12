# Offline XLive stand-in and the installed DLL's crash

Addresses: 00A40DF0, 00A40110, 00A3ED60, 00A3EF20, 00A3ED10, 00A3EBD0, 00A40510, 00A3FA70, 00A3FF20, 00B29670, 00BEC1A0, 00BECCD0, 004C0170, 0073D410, 008F81F0, 00A66030, 004254B0 (existing recovered bodies and their import thunks; no address was named or annotated by this packet).

`tools/xlive_stub/xlive_stub.cpp` is an ordinal-compatible Win32 DLL that answers the
thirty `xlive.dll` ordinals `bsp_game.exe` resolves, the way an absent Live service
answers. It is a harness component, not a reconstruction of Games for Windows Live
and not a reimplementation of the installed `xlive.dll`. It exists because the
installed DLL cannot be loaded by any host but the original executable, for the
reason set out below.

## Why the installed xlive.dll faults

The installed `I:/SteamLibrary/steamapps/common/Battlestations Pacific/xlive.dll` is
not a Games for Windows Live runtime. Its debug directory names the PDB
`C:\Users\runsv\Desktop\big_projects\BSP\xlive\alterbattlestations\out\xlive\Debug\xlive.pdb`,
it carries a `.msvcjmc` section (a Debug build), and its `.rdata` holds
`void __cdecl InitMod(void)`, `int __cdecl log_detour(const char *,...)` and
`int __cdecl fputs_detour(const char *,struct _iobuf *)`. The log file beside it is
prefixed `XLLN:`, so it is a Battlestations-specific fork of XLiveLessNess with a
mod and debug harness bolted on. It imports only IPHLPAPI, KERNEL32, USER32, GDI32,
SHELL32 and WS2_32; it requires no Live runtime, no Live-signed executable and no
manifest, and nothing about the process name, the working directory or the
`XLiveInitialize` arguments is what breaks it.

What breaks it is that during `DllMain` it **binary-patches the host executable**.
At DLL RVA `0016b76c` it calls `GetModuleHandleA(NULL)` and stores the result in its
data cell `104d8450`. Six sites then add a fixed offset to that cell and write over
the result:

| DLL RVA | destination | bytes | what it writes | containing game function |
| --- | --- | --- | --- | --- |
| `0018c195` | base + `33d7f2` = `0073D7F2` | 4 | the DWORD `1041bf24`, which points at the DLL's own string `"data\"` | `0073D410` `BSP_Application_Initialize` |
| `0018c264` | base + `640f5e` = `00A40F5E` | 5 | `90 90 90 90 90`, five NOPs over a 5-byte `CALL` | `00A40DF0` `BSP_XLiveSystem_Initialize` |
| `0018c27f` | base + `4f841f` = `008F841F` | 4 | a DWORD produced by DLL RVA `0015279f` | `008F81F0` `BSP_WinMain` |
| `0018c5b3` | base + `0254b0` = `004254B0` | n/a | a detour install, not a `memcpy`: the target, the detour `1014ee51` and a third value go to DLL RVA `00158451`. `1014ee51` is an incremental-link thunk to `0018c610`, whose body pushes the signature string `int __cdecl log_detour(const char *,...)` at `0018c6d2` | `004254B0`, no reviewed name |
| `0018cb0c` | base + `6660a6` = `00A660A6` | 5 | five bytes | `00A66030` `luaB_print` |
| `0018cb27` | base + `6660e2` = `00A660E2` | 5 | five bytes | `00A66030` `luaB_print` |

Those offsets are RVAs of the 12 MB `battlestationspacific.exe`. `bsp_game.exe` is a
1.8 MB image, so `base + 0x640f5e` lands well past the end of its mapping, on an
unmapped page, and the store faults. The faulting instruction is the dword store in
the DLL's own SSE `memcpy` at RVA `00319049`:

```
00319047  8b16    mov edx, dword ptr [esi]
00319049  8917    mov dword ptr [edi], edx    <-- faults
```

`local/xlive_load_probe.cpp` (scratch, not committed) reproduces it directly. It
loads the installed DLL under a vectored exception handler and records:

```
code    c0000005     op write        module base 6c8e0000
address 6cbf9049  =  xlive.dll + 00319049
target  01220f5e     edx 90909090    esi 00a1ec20 (a stack buffer)
return address rvas  0018c276  0017bf30  00180bb3  00153069 (the DLL entry point)
```

`0x01220f5e - 0x640f5e = 0x00be0000`, the host's ASLR load base, and `edx` is the
four NOP bytes. The return-address chain ends at the DLL entry point, so this runs
inside `LoadLibraryExW` on the loading thread, before any ordinal is called. The RVA
matches the fault offset `0x00319049` in the Windows Application Error log quoted by
milestone 2s.

### The second fault offset, 0x0033dbeb

It is not an independent failure. `0033dbd0` opens `mov edi,edi / push ebp /
mov ebp,esp / sub esp,0x4c8` with no `__chkstk`, and `0033dbeb` is
`mov dword ptr [ebp-0x474], 1`, its first store into that frame. It faults once the
first exception has started a recursive dispatch that eats the thread's stack. The
probe reproduced that shape: the fault at `00319049` is followed by a repeating
fault in `ntdll` with the frame pointer descending about `0x6e88` per iteration. A
concurrent worktree's crash of the same executable logged the pair as
`xlive.dll+0x00319049` then `ntdll.dll+0x0006abc6`, one second apart, from a single
process id. **Provisional**: the cascade is observed; the routine at `0033dbd0` is
not named here.

### Consequence

No argument, manifest, working directory, process name or `XLiveInitialize` value
changes this. The DLL writes into whatever image `GetModuleHandleA(NULL)` returns.
It is usable only under the original `battlestationspacific.exe`, and any host that
is not that image needs a stand-in.

## The ordinals the stand-in exports

`battlestationspacific.exe` imports 106 ordinals from `xlive.dll`. The stand-in
exports the thirty that `bsp_game.exe` itself resolves with
`GetProcAddress(module, MAKEINTRESOURCEA(n))`, across five loaders:
`src/xlive_library.cpp`, `src/xlive_startup_adapter.cpp`, `src/xlive_sdk_adapter.cpp`,
`src/native_xlive_device_adapter.cpp` and `src/xlive_application_callbacks.cpp`.
Every export name below is the installed DLL's own export name for that ordinal;
all thirty pairs were checked against its export table. The call site is the `CALL`
instruction in the original executable and the native column is the import thunk it
calls; `tools/verify_report_calls.py reports/xlive_stub.json` checks all thirty.

| Ordinal | Export | IAT | Thunk | Call site | Containing function | Stack | No-op contract |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | XWSAStartup | `00ce2708` | `00a4d5d2` | `00a40f72` | `00a40df0` `BSP_XLiveSystem_Initialize` | 8 | returns 0 and echoes the requested version into a zeroed `WSADATA`; the host rejects anything but 2.2 and throws on a nonzero result |
| 2 | XWSACleanup | `00ce2704` | `00a4d5cc` | `00a40f84` | `00a40df0` | 0 | returns 0; result discarded |
| 38 | XSocketNTOHS | `00ce262c` | `00a4d494` | `00a40f8e` | `00a40df0` | 4 | the real byte swap; the result is passed straight to ordinal 84 |
| 84 | XNetSetSystemLinkPort | `00ce2700` | `00a4d5c6` | `00a40f94` | `00a40df0` | 4 | returns 0; no link port exists offline |
| 651 | XNotifyGetNext | `00ce26f0` | `00a4d5ae` | `00a40161` | `00a40110` `BSP_XenonSystemManager_DrainNotifications` | 16 | returns FALSE (queue empty) and writes no output |
| 1082 | XGetOverlappedExtendedError | `00ce2678` | `00a4d422` | `00a3eefa` | `00a3ed60` `BSP_XLiveSystem_DownloadStorage` | 4 | returns `ERROR_IO_INCOMPLETE` |
| 1083 | XGetOverlappedResult | `00ce2670` | `00a4d42e` | `00a3eeb8` | `00a3ed60` | 12 | returns `ERROR_IO_INCOMPLETE` |
| 5005 | XLiveOnCreateDevice | `00ce25d0` | `00c2f1c0` | `00b29972` | `00b29670` `BSP_D3D9Renderer_RecreateDevice` | 8 | returns `S_OK`; no overlay attaches to the D3D9 device |
| 5006 | XLiveOnDestroyDevice | `00ce25d4` | `00c2f1c6` | `00b29859` | `00b29670` | 0 | returns `S_OK` |
| 5022 | XLiveGetUpdateInformation | `00ce26e4` | `00a4d59c` | `00a3ff5c` | `00a3ff20` `BSP_XLiveSystem_GetTitleUpdatePath` | 4 | returns `E_FAIL`; `title_update_path_00a3ff20` reads a negative HRESULT as "no update" |
| 5024 | XLiveUpdateSystem | `00ce26e8` | `00a4d5a2` | `00a404e9` | `00a40110` | 4 | returns `E_FAIL`; no update package is launched |
| 5030 | XLivePreTranslateMessage | `00ce25dc` | `00c2f1d2` | `00bec1d8` | `00bec1a0` `BSP_Win32Platform_RunLoop` | 4 | returns FALSE (not handled), leaving the host's own message pump in charge |
| 5260 | XShowSigninUI | `00ce26f8` | `00a4d5ba` | `00a4087a` | `00a40510` `BSP_XLiveSystem_PumpSigninUi` | 8 | returns `ERROR_NOT_SUPPORTED`; a success would make the pump wait for an overlay that never appears |
| 5261 | XUserGetXUID | `00ce26c4` | `00a4d56c` | `00a3ec07` | `00a3ebd0` `BSP_XLiveManager_RefreshCachedLocalUser_Provisional` | 8 | returns `ERROR_NO_SUCH_USER`; the caller substitutes XUID 0 |
| 5262 | XUserGetSigninState | `00ce26c8` | `00a4d572` | `00a3ebde` | `00a3ebd0` | 4 | returns 0, `eXUserSigninState_NotSignedIn`; a state, not an error code |
| 5263 | XUserGetName | `00ce26c0` | `00a4d566` | `00a3ebf4` | `00a3ebd0` | 12 | returns `ERROR_NO_SUCH_USER`; the caller substitutes an empty name |
| 5265 | XUserCheckPrivilege | `00ce2638` | `00a4d482` | `00a3ec39` | `00a3ebd0` | 12 | returns `ERROR_NO_SUCH_USER`; the caller substitutes privilege 0 |
| 5266 | XShowMessageBoxUI | `00ce26fc` | `00a4d5c0` | `00a4072a` | `00a40510` | 36 | returns `ERROR_NOT_SUPPORTED`; no overlay, so no choice is produced |
| 5267 | XUserGetSigninInfo | `00ce26bc` | `00a4d560` | `00a4090f` | `00a40510` | 12 | returns `ERROR_NO_SUCH_USER`; the `0x28`-byte block is left untouched |
| 5270 | XNotifyCreateListener | `00ce26f4` | `00a4d5b4` | `00a40fd4` | `00a40df0` | 8 | returns a stable non-NULL, non-`(-1)` identity so the host's validity test at `00a40fd9`/`00a40fe0` passes; the queue behind it stays empty |
| 5277 | XUserSetContext | `00ce2698` | `00a4d3f2` | `004c0280` | `004c0170` `BSP_Game_UpdatePresenceContext` | 12 | returns nothing and stores no context |
| 5278 | XUserWriteAchievements | `00ce26e0` | `00a4d596` | `00a3fbf5` | `00a3fa70` `BSP_XLiveSystem_PumpAchievements` | 12 | returns `ERROR_NO_SUCH_USER` |
| 5297 | XLiveInitializeEx | `00ce2710` | `00a4d5de` | `00a40f46` | `00a40df0` | 8 | returns `S_OK`; the host stores the HRESULT and branches on nothing |
| 5304 | XStorageUploadFromMemoryGetProgress | `00ce26d8` | `00a4d58a` | `00a3f0a5` | `00a3ef20` `BSP_XLiveSystem_UploadStorage` | 16 | returns `ERROR_IO_PENDING` |
| 5305 | XStorageUploadFromMemory | `00ce26dc` | `00a4d590` | `00a3f039` | `00a3ef20` | 20 | returns `ERROR_NO_SUCH_USER` |
| 5307 | XStorageDownloadToMemoryGetProgress | `00ce26d0` | `00a4d57e` | `00a3eeaa` | `00a3ed60` | 16 | returns `ERROR_IO_PENDING` |
| 5310 | XOnlineStartup | `00ce270c` | `00a4d5d8` | `00a40f63` | `00a40df0` | 0 | returns `S_OK`; result discarded at `00a40f63` |
| 5315 | XInviteGetAcceptedInfo | `00ce26ec` | `00a4d5a8` | `00a40328` | `00a40110` | 8 | returns `ERROR_NOT_FOUND` and writes no payload; unreachable while the notification queue is empty |
| 5344 | XStorageBuildServerPath | `00ce26cc` | `00a4d578` | `00a3ed44` | `00a3ed10` `BSP_XLiveSystem_BuildStoragePath` | 28 | returns `ERROR_NO_SUCH_USER`; `path_bytes` is untouched |
| 5345 | XStorageDownloadToMemory | `00ce26d4` | `00a4d584` | `00a3ee60` | `00a3ed60` | 28 | returns `ERROR_NO_SUCH_USER` |

"Stack" is the `RET imm16` each export must use. It is pinned twice: by the
`__stdcall` signature in `xlive_stub.cpp` and by the decorated internal name
`_Name@<bytes>` in `xlive_stub.def`, so a changed parameter count fails the link
rather than shifting a caller's stack silently.

The remaining 76 imported ordinals are not exported. They are reached only through
the original executable's import table, which `bsp_game.exe` does not have, so
exporting them would mean guessing a stack cleanup for each. Ordinal 38 is the one
export that keeps the original's arithmetic: it is a pure byte swap whose result the
host feeds to ordinal 84, and returning the argument unswapped would be a different
value, not a neutral one. Ordinal 5030 has a second call site, `00becd0d` in
`00beccd0` `BSP_Win32Platform_PumpLoadMessages`; the table cites `00bec1d8` because
that is the one the executable's own run log names. Both are in the report.

Setting `%BSP_XLIVE_STUB_LOG%` to a path makes the stand-in write `<ordinal> <name>
<count>` for every export reached, at `DLL_PROCESS_DETACH`, with plain Win32 calls.

## Building it

`tools/build_xlive_stub.ps1` compiles it with `cl` to
`build/win32/Release/xlive_stub.dll`, linking `/MANIFEST:EMBED` with `/MT`. The name
carries none of `install`, `setup`, `update` or `patch`, which Windows UAC installer
detection would otherwise treat as an elevation request. `cmake/startup.cmake` also
registers the same target through three deferred calls (`add_library bsp_xlive_stub
SHARED`, `set_target_properties` for `OUTPUT_NAME` and the `/DEF:` link flag, and
`target_compile_options /W4 /WX`), so `scripts/build.ps1` produces the same DLL at
the same path; that build links the shared CRT, like every other target.

## Validation

`scripts/build.ps1` Release Win32: every target built, no warnings under `/W4 /WX`.
`ctest -C Release`: `reconstructed_math` passes, 1 of 1. **No test cases were added.**
All thirty ordinal-to-name pairs match the installed DLL's export table, the stand-in
imports only `KERNEL32` plus the shared CRT, and its manifest is embedded.

| Run | Exit | XLive calls |
| --- | --- | --- |
| `--frames 60 --xlive-dll build/win32/Release/xlive_stub.dll` | 0 | 5030 x3 |
| `--frames 60 --xlive-dll "<install>/xlive.dll"` | `0xC0000005` | none; the log stops at 77 lines |
| milestone 2s acceptance line with the stand-in | 0 | 5030 x3 |
| `--frames 60` again, against the `tools/build_xlive_stub.ps1` build | 0 | 5030 x3 |

Both builds of the stand-in were run: the CMake target's and the script's. They
differ only in the CRT link (`/MD` against `/MT`) and produce the same call counts.

The call count is corroborated inside the executable's own log by
`PlatformLoopCallbacks::pretranslate [00bec1d8] calls=3`. Ordinal 5030 is the only
one the host reaches: `bsp_game.exe` wires `XLiveLibrary` into the platform loop but
does not yet drive `XLiveStartupAdapter`, so `00a40df0`'s sequence is reconstructed
in `src/audio_online_startup.cpp` without an executable caller. The real-DLL run
stops after the `settings resolution=` line, the line before the `SoundServices`
construction where `XLiveLibrary` loads the DLL, exactly as milestone 2s recorded.

The acceptance run reproduces milestone 2s's published figures to the digit for
`units=32 ai_owned=31 steps=15680 gated=0 replans=5048`,
`state_steps{concrete=4803 records=245}`, `cruise=13 stop=12 attackmove=6
movetopos=1`, `arm tail bodies=3426 latched=3425 stops=6 arrival_latches=0`,
`sector_scans=47040 sector_marks=0 ring_scans=588` and `path search ticks=3421
swaps=12 points=3419 units_with_point=7`. Three motion totals differ:
`total_path` is 5007.14 against 5047.64, DeRuyter `moved` 33.43 against 34.80 and
Kortenaer `moved` 215.02 against 258.16. The branch base is `f51654b8`, many merges
past milestone 2s's `95f39aa4`, and the motion work landed in between. The stand-in
cannot account for it: the only ordinal the run reaches is 5030, three times,
returning FALSE, which is the state the executable was in before commit `2c11966a`.

These are build, export and run results. No Live account, service, context or SDK
call was exercised, and no Ghidra address was written or named by this packet.

## Proposed patch for the host owner

`src/game_hosts.cpp` and `include/bsp/game_hosts.hpp` are leased to
`agent/orch6-20260912` and were not touched. One change is proposed.

**Degrade to the stand-in instead of throwing.** `src/game_hosts.cpp` line 53:

```cpp
std::wstring selected_library_path(const std::wstring& selected, const wchar_t* name) {
    return selected.empty() ? std::filesystem::absolute(name).wstring() : selected;
}
```

and line 894:

```cpp
: xlive(selected_library_path(app.options_.xlive_dll, L"xlive.dll"),
        app.options_.xlive_dependencies),
```

`XLiveLibrary`'s constructor throws when `LoadLibraryExW` fails, so an absent
`xlive.dll` aborts startup; and when the installed one is present it does not fail
the load at all, it faults inside it, which no `try` can catch. Both cases want the
same answer: the stand-in beside the executable.

Proposed, for the owner to place in the same file:

```cpp
// The installed xlive.dll patches its host executable at offsets valid only for
// battlestationspacific.exe and access-violates under any other image
// (docs/XLIVE_STUB.md). Default to the stand-in and take the real DLL only when
// --xlive-dll names it explicitly.
std::wstring selected_xlive_path(const std::wstring& selected) {
    if (!selected.empty()) return selected;
    const auto stub = std::filesystem::absolute(L"xlive_stub.dll");
    if (std::filesystem::exists(stub)) return stub.wstring();
    return std::filesystem::absolute(L"xlive.dll").wstring();
}
```

with line 894 calling `selected_xlive_path(app.options_.xlive_dll)`. This keeps
`--xlive-dll` authoritative, needs no `try`/`catch`, and makes an unattended run
work without an explicit flag. It does not make the executable tolerant of a
faulting DLL, which is not achievable in-process: the fault happens inside the
loader, before the constructor returns.

**The dangling-pointer item is already fixed and needs no patch.** Milestone 2s
correction 7 said `src/game_hosts.cpp` line 1230 passes a braced temporary to
`bind_legacy_crt_math_runtime`, whose stored address `src/legacy_crt_math.cpp`
line 271 keeps. On this branch's base, line 48 defines `application_math_runtime`
as a namespace-scope `const` object with static storage duration and line 1252
passes that object, so the stored pointer is valid for the process lifetime.
Commit `dc6b8b10` introduced the hoisted definition; `git log -L 46,50:src/game_hosts.cpp`
shows it.
