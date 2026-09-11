# XLive notification dispatch

Addresses: 00a40110, 00c2f1d2

`drain_xlive_notifications_00a40110` reconstructs the native manager's normal
notification loop over the existing `OnlineSystemState` and `PlatformManagerFlags`.
The original ABI is ECX=manager, no stack arguments, RET at 00A402F7. Its disjoint
update arms continue through 00A404F3. These C++ projections are not ABI replacements.

The listener is recreated only when its actual value is -1. The debounce helper
runs before the first GetNext. Each GetNext reloads the current listener, requests
filter zero, and dispatches all twelve known IDs; unknown IDs are consumed too.
Native logs call the already verified bare-RET 004254B0.

Assembly corrects the earlier notes: the F8ABEC hook receives `parameter != 0`
in CL at 00A401B6, before the same value is written to manager+3E8 at 00A401C2.
The accepted-invite arm calls the DLL, sets +31, then copies all 84 bytes to +32.
The C++ adapter rejects a failed invite query rather than inventing output bytes
for the native unchecked copy. This error boundary is not native error parity.

Title-update ID15 compares the returned NativeString against an empty temporary.
For a nonempty path it appends `\setup.exe`, invokes 00A3E560 with executable
`update.exe`, sleeps 200 milliseconds, then calls `_exit(0)`. It does not compare
the returned path against `\setup.exe`. System-update ID16 queries a separate
path, widens it through 004C5E60, invokes XLiveUpdateSystem, and exits. These game
helpers and process operations are required interfaces; no update is launched by
the reconstruction or library constructor itself. NativeString storage is reused.

Sign-in debounce, cached-user refresh, profile-name refresh, localization and
updater helpers remain separately reconstructed dependencies. Optional callback
slots represent actual optional globals, including the current online client's
friends-change virtual call. Callbacks may mutate the listener and manager flags.

`XLiveLibrary` forwards original Win32 stdcall imports by ordinal, including
5030 (XLivePreTranslateMessage), 5270 (XNotifyCreateListener), 651 (XNotifyGetNext),
5315 (XInviteGetAcceptedInfo), and 5024 (XLiveUpdateSystem). The pretranslation
thunk is 00C2F1D2 -> IAT00CE25DC; both installed DLLs implement a RET4 export.
It requires an explicit absolute DLL path and performs no hidden initialization.

Win32 Release and both existing CTests passed after the initial implementation.
The actual runtime loader probes found two distinct limitations:

- Windows' Microsoft GFWL 2.0.0673.0 DLL failed LoadLibraryEx with error182.
  Static import inspection found missing ordinal43 in the installed msidcrl40.dll.
- A hash-identical local copy of the installed Team Wolfpack replacement exited
  C0000005 during DLL loading, before pretranslation. A separate debug launch
  instead observed first/second-chance C0000008; the loader fault's cause is not
  established. No DLL was patched, and no installed game files were written.

Neither probe establishes a working XLive runtime, login, online play, overlay,
or full game. Binary hashes and probe paths are recorded in the report.
