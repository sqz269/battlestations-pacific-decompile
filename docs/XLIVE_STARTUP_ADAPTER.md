# XLive startup SDK adapter

`XLiveStartupAdapter` implements the audited DLL and Win32 operations in the
existing `XLiveStartupHost`. It borrows `XLiveLibrary::module_handle()` and performs
no loading, initialization or cleanup in its constructor/destructor. The library
must outlive the adapter, startup state, listeners and other SDK objects created
through it. Missing ordinals raise explicit errors; no SDK behavior is substituted.

Renderer device/present-parameter pointers, IPC initialization at manager +3AC,
and actual state-callback invocation remain required abstract operations. The
adapter does not supply default pointers or IPC success. Its two diagnostic
methods are concrete empty bodies because the native sink 004254B0 is exactly
the single byte C3 (`RET`), independently verified against saved and disk bytes.

## Original calls and argument layout

| Name | Ordinal | Game thunk / IAT | Stack bytes | Native return use |
| --- | ---: | --- | ---: | --- |
| XLiveInitializeEx | 5297 | 00a4d5de / 00ce2710 | 8 | HRESULT discarded by native; adapter preserves it |
| XOnlineStartup | 5310 | 00a4d5d8 / 00ce270c | 0 | Discarded |
| XWSAStartup | 1 | 00a4d5d2 / 00ce2708 | 8 | Discarded before unconditional output read |
| XWSACleanup | 2 | 00a4d5cc / 00ce2704 | 0 | Discarded |
| XSocketNTOHS | 38 | 00a4d494 / 00ce262c | 4 | Low WORD becomes system-link port |
| XNetSetSystemLinkPort | 84 | 00a4d5c6 / 00ce2700 | 4 | Discarded |
| XNotifyCreateListener | 5270 | 00a4d5b4 / 00ce26f4 | 8 | HANDLE stored unchanged |

These are Win32 stdcall calls. WORD arguments still occupy DWORD stack slots;
the notification mask is one 64-bit argument. The adapter resolves the original
ordinal and copies `FARPROC` into the exact typed function pointer with `memcpy`.
It does not cast between incompatible function-pointer types.

The existing initialize structure is verified at size 0x1C, device +8, present
parameters +C and language WORD +10. Native 00a40f37..00a40f46 pushes version
0x20029900 and the stack block pointer. The adapter passes the caller's block
directly. `user_default_lang_id()` calls actual `GetUserDefaultLangID`, matching
the original import at 00ce22a8 and call at 00a40f31.

The game passes 0x0202 to XWSAStartup at 00a40f6d, 0x0C02 to XSocketNTOHS at
00a40f89, and area mask 0x2F at 00a40fc5..00a40fd4. The adapter forwards its
arguments rather than hardcoding these caller values. Listener handles are not
filtered or closed here; canonical startup/owner code owns those decisions.

## Full WSADATA and the explicit failure boundary

The adapter includes actual Win32 `winsock2.h` and allocates a complete local
`WSADATA`, not a two-byte version output passed to the DLL. Compile-time checks
prove its 0x190-byte size and offsets: wVersion0, wHighVersion2, description4,
system status105h, iMaxSockets186h, iMaxUdpDg188h, vendor pointer18Ch. This matches
the native output area at stack +28 through +1B7, before the saved exception state.
The SDK output is not prefilled with a fabricated version or success value.

Native 00a40f77 reads wVersion even after a nonzero XWSAStartup return. It tests
low byte2 and high byte equal to the low byte; otherwise it calls cleanup. The
adapter can establish a valid output under the successful SDK contract and
copies actual `WSADATA.wVersion` only after result0. For any nonzero result it
throws `XLiveWsaStartupOutputError`, retaining the exact signed code in
`sdk_result()` and leaving the caller's version unchanged.

This is a conservative, explicit divergence from the native undefined-output
path. A particular failing DLL could have written some output; this adapter
does not infer that from the return code or manufacture zero bytes. It stops
before canonical startup interprets an unestablished version. No implicit
cleanup or retry is added to that error path. A null host version-output pointer
is rejected before invoking the SDK.

## Evidence and validation limits

Verified `bsp.py` queries used the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, read-only. Static PE inspection matched all seven
game import ordinals and found them in both installed XLive libraries. The shim
exports the expected names; the Microsoft library exports these by ordinal.
The report also records the shim's argument-forwarding wrapper and Microsoft's
two-argument `RET8` XWSAStartup wrapper. No native-body ledger entries were added.

Win32 Release passed `/W4 /WX /fp:strict` compilation and both existing CTests.
An ignored local proof exported a derived-host factory, forcing the adapter
constructor and full vtable to link while keeping all four game methods required.
The proof executable only printed its success message: it never called the
factory, loaded XLive or invoked any startup, networking or account operation.

`reports/xlive_startup_adapter.json` contains byte, ordinal, structure and build
evidence. Actual XLive initialization and SDK startup remain unvalidated. The
parent separately established Microsoft DLL loading/pretranslation with its
signed legacy dependency preload; this worker did not repeat that loader work.
