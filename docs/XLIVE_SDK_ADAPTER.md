# XLive SDK forwarding adapter

`XLiveSdkAdapter` implements the eleven DLL operations in `XLiveSystemPumpHost`
and four in `XLiveSigninHost`. It remains abstract for the notification drain,
localization, indexed cached-byte reads, both clock method spellings and state
callback invocation. The integrator supplies those game operations.

Construction borrows `XLiveLibrary::module_handle()`; it never loads, initializes
or unloads a library. The `XLiveLibrary` must outlive the adapter and every
pending overlap, buffer, listener and other SDK object used with it. Each call
resolves the original ordinal with `GetProcAddress`. A missing export raises an
explicit error. Function pointers are copied from `FARPROC` with `memcpy`,
avoiding a function-pointer cast and MSVC warning C4191.

## ABI and evidence

Every entry uses Win32 `__stdcall`, a DWORD return value and four-byte stack
arguments. Pointers below are real caller output storage. The C++ host methods
project pointer parameters as references and preserve all result bits.

| Original name | Ordinal | Game thunk | Stack bytes |
| --- | ---: | --- | ---: |
| XStorageBuildServerPath | 5344 | 00a4d578 | 28 |
| XStorageDownloadToMemory | 5345 | 00a4d584 | 28 |
| XStorageUploadFromMemory | 5305 | 00a4d590 | 20 |
| XStorageDownloadToMemoryGetProgress | 5307 | 00a4d57e | 16 |
| XStorageUploadFromMemoryGetProgress | 5304 | 00a4d58a | 16 |
| XGetOverlappedResult | 1083 | 00a4d42e | 12 |
| XGetOverlappedExtendedError | 1082 | 00a4d422 | 4 |
| XUserWriteAchievements | 5278 | 00a4d596 | 12 |
| XShowMessageBoxUI | 5266 | 00a4d5c0 | 36 |
| XShowSigninUI | 5260 | 00a4d5ba | 8 |
| XUserGetSigninInfo | 5267 | 00a4d560 | 12 |
| XUserGetSigninState | 5262 | 00a4d572 | 4 |
| XUserGetName | 5263 | 00a4d566 | 12 |
| XUserGetXUID | 5261 | 00a4d56c | 8 |
| XUserCheckPrivilege | 5265 | 00a4d482 | 12 |

The pump call-site stacks were recovered in [XLIVE_SYSTEM_PUMP.md](XLIVE_SYSTEM_PUMP.md).
00a3ebd0 supplies the four sign-in call stacks: user0 at 00a3ebdb, name capacity
0x80 at 00a3ebe5, XUID output at 00a3ec01, and privilege 0xFE with a BOOL output
at 00a3ec2e. See [XLIVE_SIGNIN.md](XLIVE_SIGNIN.md). Verified `bsp.py` live byte
queries confirm the original `JMP [IAT]` thunks. Independent PE parsing maps all
fifteen IAT entries to these ordinals and finds them in both installed libraries.
The shim also exports the expected names; the Microsoft library is ordinal-only.
No recovered game address or CRT symbol is renamed by this library-boundary work.

## Output ownership

Overlapped objects, storage data, download results, achievement arrays and message
choice output are passed directly, preserving their addresses across asynchronous
completion. The adapter does not reset them. `XGetOverlappedResult` receives a
32-bit BOOL converted explicitly from the host's C++ `bool`.

`XUserGetName` clears the output's known-byte mask before the call. On SDK failure
it returns the original result with every byte unknown; the recovered sign-in
caller owns its byte-zero failure write. On success it defines only the returned
name through its NUL. The unknown tail is never presented as SDK-written padding.
The temporary buffer is poisoned with nonzero bytes to detect a success response
without a bounded terminator; that invalid response raises an explicit error.
Capacities greater than the native 128-byte storage are rejected before the call.

The installed shim at RVA 0026BD68 pushes 0x28 for the sign-in-info copy; its
copy call at RVA 0026BD7B follows two size-0x28 arguments. The adapter supplies an
aligned 0x28-byte output and reads only byte +8, only on SDK success. Failure
preserves the caller's flags byte and SDK status. XUID and privilege outputs are
forwarded directly; their native callers own failure overwrite behavior.

Progress output is unconsumed four-byte storage. The native calls pass null for
the final two outputs, so this packet does not establish their pointee types or
the progress value's numeric meaning. No SDK account, network, service, UI or
storage implementation is substituted.

## Validation boundary

Win32 Release compilation passed with `/W4 /WX /fp:strict`; both existing CTests
passed. `reports/xlive_sdk_adapter.json` records the original import operands,
stack extents, DLL hashes and export RVAs. No DLL was loaded or called by this
worker, and no installation or Ghidra state was changed. The parent's earlier
load attempts reported Microsoft error 182 and shim exception C0000005; those
are parent observations, not successful SDK runtime validation.
