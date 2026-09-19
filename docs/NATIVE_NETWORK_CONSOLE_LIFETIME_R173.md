# Native network-console packet bookkeeping and destruction (R173)

Addresses: 00a3aab0, 00a3ab30, 00a3bb30, 00a3b6e0, 00a3d1a0, 00a3d260,
00a3c5e0, 00a3caf0, 00737f30

## Implemented scope

Six complete normal-path bodies, 699 original bytes, are in
`native_network_console_runtime.hpp/.cpp`. They use R172's actual 49Ch channels,
820h/82Ch queue records and 4003F0h owner. Names describe recovered behavior;
they are not recovered source symbols or a claim of binary ABI compatibility.

| Entry | Original ABI | Behavior |
| --- | --- | --- |
| A3AAB0 (114B) | ECX channel, stack header, RET4 | Rebuild the selective ACK cache only when byte29 is zero. Scan stream1's list against the once-captured counter, admitting unsigned deltas1..24. Write header bytes4,2,7,6,5; OR byte2 instead of clearing its existing bits. |
| A3AB30 (98B) | ECX channel, stack record/index, RET8 | Append to one of three receive lists without clearing the incoming next pointer. Advance contiguous sequence numbers through repeated scans of the captured, unsorted head; DWORD arithmetic wraps. No unlink/free or cache invalidation. |
| A3BB30 (121B) | ECX owner, stack sockaddr16, RET4/EAX index | Under once-captured lock+148, return the first active channel whose IPv4 DWORD matches. Port and family are not compared. On miss, preserve the ordinal38 XSocketNTOHS call, discarded result, then leave the original lock and return FFFFFFFF. |
| A3B6E0 (152B) | ECX owner, RET | Set quit under captured lock+14C; always Sleep(10), reload the lock, test send/receive flags, repeat if active. Capture Sleep once. Then capture CloseHandle once, close+154, reload+158 and close it. Ignore close results; retain handle fields. |
| A3D1A0 (184B) | ECX owner, RET | Stamp D23F48, release owned locks+148/+14C, reverse-destroy 1000x820h records, 1000x82Ch records, 16x49Ch channels, then run the full R172 base destructor. |
| A3D260 (30B) | ECX owner, stack flags, RET4/EAX captured identity | Full derived destructor followed by free only when flags bit0 is set. |

The socket resolver uses ordinal38 from an already loaded module. The thread
import defaults use real Win32 Sleep/CloseHandle; borrowed import cells retain
the original capture epochs. No successful SDK or worker substitute was added.

## Shutdown correction and call-site evidence

The earlier “stop-and-join” description in `APP_SHUTDOWN.md` is too strong.
A3B6E0 polls activity flags and closes handles; it never waits for either thread
handle to signal. It has no timeout, does not clear handles, and does not reset
quit for a restart. Flags may be zero before a newly created worker sets them.
Callers must establish actual worker quiescence before destroying owned storage.
The destructor itself neither stops workers nor closes their handles.

Application shutdown calls A3B6E0 at 737FDA before the online-owner deletion.
The send worker calls A3AAB0 at A3C905 and A3BB30 at A3C93C. The receive worker
calls A3BB30 at A3CC90/A3CCC9 and A3AB30 at A3CDA0. These are dependency fragments,
not implementations of the complete worker loops.

Ghidra's A3D260 listing omitted ADD ESP,4 at A3D275 after returning free. The
locked repair restored that instruction and retained RET4 at A3D27B without
changing the callee's global no-return flag. Prior names/comments, save receipt,
readback, refreshed exports and byte evidence are preserved in local artifacts.

## Validation and limits

Strict MSVC Win32 build and all three existing CTests pass. One local differential
fixture executes all six copied original bodies plus five previously verified
destructor dependencies: 14 original/source cases, 8,461,224 matching bytes.
It covers cached/rebuilt/empty/wrapping ACKs, unsorted/wrapping receive queues and
retained incoming links, first IP match despite different ports, miss-time lock
replacement, captured imports, per-poll lock replacement, late second-handle
reload, real event handles, actual malloc-backed tracked-lock release, current
singleton replacement, retained flags2 and freeing flags101. Raw pointers are
checked before normalization of archived observations.

The fixture uses recording SDK/Sleep/Close boundaries and a normal-path CRT array
iterator; it does not run native network workers or establish a network peer.
Native parent/callee evidence covers 3,071 live/PE-matched bytes and 19 direct
calls. No permanent test suite was added. The report and immutable local archives
pin source, object, executable and fixture provenance before and after integration.

Full derived constructor A3D060, worker wrappers A3CEC0/A3CEE0 and complete
A3C5E0/A3CAF0 loops remain unbound. Original FH3/SEH, hardware faults, original
allocator/new-handler identity, binary ABI, concurrent workers, ordinary startup,
network exchange and gameplay are not proved by these normal-path comparisons.

## Follow-up packet

Trace the remaining send/receive helpers from both worker call graphs, including
pool release, socket operations, clock/x87 scheduling and logging dependencies.
Compose the complete constructor only after the genuine worker bodies exist;
do not create placeholder threads to make it return.
