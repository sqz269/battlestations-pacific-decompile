# Damageable class construction and allocation-failure cleanup

## Result and boundaries

`src/native_damageable_class_construction.cpp` reconstructs complete ordinary
`0087C640` and library `std_Tree_Buynode` (`00877FA0`), with the five container
cleanup entries reached by constructor unwind. All interfaces borrow actual
Win32 storage. Class and field names are hypotheses; the library name is retained.
The interfaces are source compositions, not native binary replacement entrypoints.

The constructor receives stable actual vtable DWORD identities and the existing
`NativeStringRawPoolContext`. It uses `singleton_lifetime_allocate/free` and the
existing `reserve_native_class_point_array_0074d190`; there is no substitute class,
synthetic vtable, allocator callback, or new storage domain in production source.
The two vector cleanup methods call the records' current actual vtable entries.

## Complete bodies and original calling conventions

| Entry | Inclusive end | Bytes | Original interface |
| --- | --- | ---: | --- |
| `0087C640` | `0087C6E9` | 170 | ECX class; EAX same class; RET |
| `00877FA0` | `00877FD6` | 55 | No used input; EAX raw node; RET |
| `0087C260` | `0087C29C` | 61 | ECX vector header; RET |
| `0087AB30` | `0087AB70` | 65 | ECX first, EDX end, two ignored stack words; RET8 |
| `00879830` | `00879834` | 5 | Tail JMP `00879240` |
| `00879240` | `00879288` | 73 | ECX vector header; RET |
| `0081B0A0` | `0081B0DC` | 61 | ECX counted point header; RET |

These seven bodies total 490 bytes. The report contains individual disk/live
SHA-256 parity for them, six unwind funclets and their handler, the FH3 metadata,
and the existing `00BD30F0` vptr store: ten spans, 654 bytes. Each live byte query
uses `bsp.py ghidra`, which verifies configured project `bsp`, program
`/battlestationspacific.exe`, language, and image base before reading. The
configured project file is `C:/Users/sqz269/bsp.gpr`.

## Constructor writes and sentinel

Write order is grandparent vptr (`00CEB130` identity), refcount `+4=1`, then base
vptr (`00D0E13C` identity). Zero DWORDs at `+0C,+10,+14,+1C,+20,+24,+28,+2C,+30,
+38,+3C`; store `+40=-1`, byte `+44=0`, then zero DWORDs `+50,+58,+5C`.
The remaining class bytes are preserved, including `+8,+18,+34,+48,+4C,+54`,
padding `+45..47`, and the comparator/padding DWORD `+60`.

`00877FA0` allocates exactly `58h` bytes. It tests node, node+4, and node+8
separately before their zero link stores, using Win32 wrapping address arithmetic.
It writes byte `+54=1` and `+55=0`; node value bytes `+0C..53` and tail padding
`+56..57` remain untouched. The canonical allocator retries the CRT new handler
and throws on exhaustion; it does not add a nullable allocation fallback.

After allocation the constructor stores head at class+64, writes head+55=1,
reloads current head for each self-link store in order `+4,+0,+8`, then zeros
class+68. No old head is released. `+64/+68` remain untouched if allocation fails.

## FH3 state5 cleanup

Handler `00C9686F` loads `00DC8CC0` then jumps to `00BF6B43`. Metadata has magic
`19930522`, maxState 6, unwind map `00DC8CE4`, and no try blocks. The constructor
sets state0 after the two vptr writes and state5 immediately before its only call.

| State -> next | Funclet | Action |
| --- | --- | --- |
| 5 -> 4 | `00C96864` | `0041DD20(class+58)` |
| 4 -> 3 | `00C96859` | `0041DD20(class+38)` |
| 3 -> 2 | `00C9684E` | `0081B0A0(class+28)` |
| 2 -> 1 | `00C96843` | `00879830(class+18)` |
| 1 -> 0 | `00C96838` | `0087C260(class+8)` |
| 0 -> -1 | `00C96830` | `00BD30F0(class)` |

Cleanup reads current embedded headers, including changes made by a failing new
handler. Strings use actual raw-pool composition and retain their header values.
The point destructor calls existing reserve(0) only for signed negative capacity,
decrements positive count to zero through observable DWORD stores, captures data,
zeros count, and frees data without nulling data/capacity.

The effect vector captures begin/end, calls each inline `30h` record's actual
vslot0 with flags0, reloads begin after callbacks, frees it, and clears +4/+8/+C.
The owner vector invokes its range helper on `10h` records: for nonnull record+0C,
atomically decrement owner+4, invoke owner's current vslot0 if the result is zero,
then clear record+0C. It likewise reloads/free begin and clears vector fields.
The leading header DWORD is preserved by both vector destructors.

`00BD30F0` is exactly `MOV [ECX],00CEB130; RET`. This source interface stores the
borrowed actual refcount-table identity at that step rather than calling the
existing naked entrypoint that embeds the original executable's absolute VA.
No count decrement, class free, sentinel destruction, or rollback is added.

The source catches C++ allocation exceptions, executes the six cleanup actions,
and rethrows. A second exception during this cleanup terminates. Original FH3
registration, hardware faults, arbitrary SEH, and native exception equivalence
are outside the proof; the ordinary post-allocation stores are not protected by
a newly invented ownership scope.

## Listing defects and validation

The original live listing omitted returning-free fallthrough at `0081B0D8`
(`ADD ESP,4; POP ESI; RET`, through `0081B0DC`), `0087C282` (`ADD ESP,4`), and
`0087926D` (`ADD ESP,4; POP EDI`, through `00879270`). Disk disassembly and
live byte parity establish those tails. The integrator applied, saved, and
refreshed these coordinated repairs; this worker re-read the complete point
destructor and repaired instruction contexts. This worker performs no Ghidra mutations.

The ignored `local/damageable_ctor_probe.cpp` fixture compares copied original
ordinary base/helper bodies with source against nonzero-filled storage, normalizing
only their allocated head addresses. Both complete object bytes, preserved bytes,
and node bytes agree. Native direct allocator calls are rebound to the fixture's
size-checking allocator; original FH3 handling is not executed on a failure.

The same fixture injects a source allocation exception after mutating both string
headers, the point array, and both empty vector allocations. It checks cleanup
order, header preservation, the restored refcount vptr, unchanged head/count,
and propagation of the original exception. This portion substitutes the generic
allocation/free and raw-string boundaries; it is not original-machine-code EH,
real string-pool validation, nonempty virtual-record coverage, or gameplay proof.
The negative-capacity point-reserve branch uses the existing source dependency
but is not exercised by this fixture. Final build and verifier results are in
`reports/native_damageable_class_construction_orch4.json`.
