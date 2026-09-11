# XLive named-pipe transport owner

This packet reconstructs the seven complete transport-owner functions in
`src/xlive_pipe_transport.cpp`: `00A5DE34`, `00A5DE68`, `00A5E145`,
`00A5E710`, `00A5E844`, `00A5E3D0`, and `00A5E557`. The API exposes real
Win32 transport operations through `Win32XLivePipeSystemHost`. Protocol
initialization and destruction remain substantive required dependencies.
No successful protocol context or OS handle is supplied by a default stub.

`PIPEIPC_*` names in Ghidra came from analyst-assigned medium-confidence
`block_pipe_ipc` tags. They are not recovered vendor symbols. The direct bodies,
named-pipe path, ACL construction, and Win32 calls establish named-pipe behavior;
they do not establish library authorship or exempt those bodies from future
reconstruction. The new descriptive names are hypotheses too.

## Canonical state and initialization

`XLivePipeNativeState` is exactly `0x30` (48) bytes in the required Win32 build.
It is the single shared field projection for transport, I/O, and framing work.

| Offset | Field | Native behavior |
| --- | --- | --- |
| `+00` | Protocol pointer | `A5F416` initializes this before own constructor stores |
| `+04` | Mode DWORD | Constructor leaves preimage; create writes caller mode |
| `+08` | Pipe handle | Constructor sets `INVALID_HANDLE_VALUE` |
| `+0C..+1F` | Actual `OVERLAPPED` | Constructor leaves all bytes untouched |
| `+20` | I/O state DWORD | Constructor sets 3 |
| `+24` | Completion event | Constructor sets null |
| `+28` | Borrowed stop event | Constructor sets null |
| `+2C` | Wait handle count | Constructor sets 0 |

Successful create stores the completion event into both `+24` and
`OVERLAPPED.hEvent` at `+1C`. It sets the wait count to 1, or stores the caller's
nonnull stop event at `+28` and sets count 2. Other `OVERLAPPED` bytes retain their
preimage. The later I/O implementation must initialize them only where its own
assembly does. No independent event or overlap state should be introduced.

`XLivePipeTransport` appends borrowed protocol/system host pointers after the
native prefix. Its standard allocator allocates this typed projection with
ordinary throwing `new`; the original requests `0x30` bytes from `00BF681B` and
also contains a null-result branch. The constructor calls protocol initialization
before it stores any of the fields listed above. It neither tests protocol
success nor invents a native initialized flag. Host context must remain valid
until destruction. This C++ interface is not a binary replacement.

## Full open path

`00A5DE34` and `00A5E844` are native `__stdcall(mode, stopEvent, output*)`,
returning a signed HRESULT in EAX and using `RET 0Ch`. Null output returns
`80070057`. The wrapper uses local output storage, then publishes null for a
negative result. The implementation writes null for returned failures after
cleanup; positive raw errors become `(error & FFFFh) | 80070000h`, while zero
and signed-negative values are preserved. A failed API with `GetLastError()==0`
uses `507h`. There is no added cleanup scope for allocator/protocol exceptions;
already-performed operations and output preimages remain observable.

Mode zero opens the verified parent process's pipe; any nonzero mode creates
the current process's pipe. The fixed format at `A5E8DE` is
`\\.\pipe\%08x`, with an 18-wide-character destination. Its maximum DWORD
format is exactly 17 characters plus NUL. The source uses the SDK safe formatter
for this fixed invocation; the generic `A5E6CD`/`A5E0E7` CRT formatting bodies
are not claimed as reconstructed functions.

The server path allocates the NT-authority `NETWORK` SID and copies the first
logon SID from the current process token. It passes two zero-initialized
`EXPLICIT_ACCESS_W` entries to `SetEntriesInAclW`: deny `1FFFFFh` to the network
well-known group, then set `C0000000h` for the logon group. Both use SID trustees
and no inheritance. It allocates a `14h` security descriptor, initializes
revision 1, and calls `SetSecurityDescriptorDacl(TRUE, acl, TRUE)`, retaining the
native defaulted-DACL argument. The non-inheritable security attributes feed
`CreateNamedPipeW(path, 40080003h, 6, 1, 400h, 400h, 5000, attributes)`.

The client path uses `CreateFileW` with `C0000000h` access, share zero,
`OPEN_EXISTING`, and `FILE_FLAG_OVERLAPPED`, then requests pipe mode 2 through
`SetNamedPipeHandleState`. Both paths create an unnamed, manual-reset,
initially-signaled completion event. No connect, I/O, worker thread, or framing
operation is invented by this packet.

Temporaries are released in native order: network SID, copied logon SID, ACL,
security descriptor. Failed create then closes a pipe unless it is `-1`, closes
a nonnull completion event, destroys protocol state, frees the owner, and clears
the returned pointer. These failed-create closes **do not rewrite owner fields**.

## Helpers and normal destruction

`00A5E557` is native `__stdcall(DWORD*)`, `RET 4`, returning raw error.
It uses `CreateToolhelp32Snapshot(2, 0)` and `PROCESSENTRY32W.dwSize=22Ch`,
scans for the current process, opens its recorded parent with access `400h`,
and obtains both creation times. Only `CompareFileTime(parent, current)==-1`
accepts the parent and writes the output; other comparisons yield `E9h`.
Failures leave output untouched. It closes the parent before the snapshot.
The API thunks `A607E2`, `A607D6`, `A607D0` are retained Toolhelp imports.

`00A5E3D0` is native `__stdcall(HLOCAL*)`, `RET 4`, returning raw error.
It opens the current process token with `TOKEN_QUERY` and requests token groups.
The sizing call must fail with `ERROR_INSUFFICIENT_BUFFER`; unexpected success
returns `Dh`. The allocated groups are scanned for the first attributes whose
`C0000000h` mask is fully set. It allocates/copies that SID. No match returns
`545h`. Groups are freed before the token is closed. Success alone writes the
caller output; failure frees any copied SID without publishing it.

`00A5E145` receives ECX=owner, returns EAX=owner with bare `RET`.
`00A5E710` is native `__stdcall(owner)`, `RET 4`: null is ignored; pipe handles
other than `-1` (including null) are closed and replaced with `-1`; nonnull
completion events are closed and cleared; then protocol and owner are freed.
The borrowed stop event and aliased `OVERLAPPED.hEvent` are not separately
closed. Wrapper `00A5DE68`, also `RET 4`, ignores both null and `-1` owner values.

## Evidence and remaining work

Read-only Ghidra queries used the existing `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, verified live count 63026. Capped disassembly,
exports, and raw bytes establish branches, field widths, imports, and final
instructions. All seven entries exist. `reports/xlive_pipe_transport.json`
records terminal instruction addresses/lengths and the two false-no-return
free-call gaps. Parent repair is `bsp.py ghidra flow 00a5e710 00a5e844 --apply`
under the tools' write lock, followed by export refresh. The worker made no
Ghidra mutations and requests no invented interior function entries.

Protocol lifetime remains `XLivePipeProtocolHost`'s two required operations.
`A5F416` creates an `A8h` context and uses CryptoAPI plus substantial arithmetic;
`A5F371` frees its provider/subobjects/allocation without clearing the owner's
slot. A bounded follow-up should own `A5F416/A5F336/A5F371/A5F1D8`, extend
after leases for `A5ED9F/A5EECB` and their required arithmetic, and produce
`xlive_pipe_protocol.hpp/.cpp` plus a matching document/report. I/O should own
`A5DEAA/A5DEE8/A5DF20/A5DF5E/A5E1F7/A5E28B/A5E31F/A5E7E6/A5E815`
while reusing this header. Framing remains the separately mapped packet in
`reports/xlive_ipc.json`. None of these dependencies is represented by success
stubs here.

Validation: MSVC Win32 Release strict build, two existing CTests, all eight
native seed comparisons, and one ignored synchronous fixture passed. The
fixture checks constructor preimages, parent age/output handling, ACL fields,
event aliasing, temporary cleanup order, failed-create versus normal-destroy
field stores, null allocation, and error normalization. OS endpoints and
process operations were scripted in the fixture; production adapters use
actual APIs. No pipe/thread/process endpoint, DLL, cryptographic protocol,
gameplay, or binary ABI validation is claimed.
