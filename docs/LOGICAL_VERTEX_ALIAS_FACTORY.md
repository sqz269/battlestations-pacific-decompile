# Logical vertex factory and registry aliases

The four newly traced renderer exception-cleanup aliases do **not** write or
retain an owner at logical-stream `+4C`. They receive the renderer's raw
pointer-array headers, shrink their counts, and free the backing arrays without
dereferencing the stored logical pointers. The nonnull `+4C` writer, incoming
owner profile and final-zero terminal remain unresolved. This is bounded native
discovery, not a source-ready owner implementation.

The [report](../reports/logical_vertex_alias_factory.json) contains **31 fresh
live-Ghidra / installed-PE spans, 2,146 bytes**, all equal, and installed executable
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Every live query used `python tools/bsp.py ghidra`; its client verifies the
configured `bsp` project, `/battlestationspacific.exe`, x86-32 language and image
base before querying. `C:/Users/sqz269/bsp.gpr` exists. Saved analysis and the game
installation were not modified. Descriptive names remain hypotheses.

This packet starts at `8ce8b94a`. The earlier
[alias discovery](NATIVE_VERTEX_STREAM_ALIAS_ESCAPE_DISCOVERY.md) already covered
factory/registration, the five nominated escape routes, renderer-band registry
readers and normal cleanup fragments. Those findings are not claimed as new.
Six selected earlier complete spans were freshly revalidated here; earlier
provider hashes and other spans are provenance only. The new investigation
extends beyond that report's `B10000..B40000` displacement scan to adjusted
registry pointers in exception-cleanup funclets.

## Exact factory and registration origins

`B287C0` takes ECX renderer and stack `(count, flags, declaration)`, returns the
constructor result or zero in EAX, and uses `RET 0C`. Although `B287DB` loads
`ECX=74h`, the call at `B287E0` reaches `B4B370`, whose complete ten bytes overwrite
ECX with pool `108FE18` and tail-jump `B4AE80`. This is a CPU-slot pool entry;
`74h` is not an argument consumed by a generic size allocator.

At `B28802` the returned CPU slot becomes unadjusted constructor ECX.
`B28804` calls `B4BC00`; `B28809` captures its result in EBP. This constructor
installs `D61D6C` and returns its original logical object. The same EBP is written
at `B28847` into the array selected by captured renderer `+1AAC`; the header has
data `+0`, count `+4`, capacity `+8`. No stream AddRef occurs in the factory.
The current renderer virtual `+58` is then invoked. If AL equals exactly one,
`B2888E` writes the same EBP into the second raw array at renderer `+19B0`.
`B2889A` returns that EBP unchanged. The null construction result still follows
the native raw-array append/count path; no null repair is inferred.

Fresh table bytes at `D5F100` establish original `D5F0A8+58 -> B1FE50` and
`+5C -> B287C0`. The complete seven-byte `B1FE50` is `MOV AL,[ECX+19AC]; RET`;
it currently lacks a Ghidra function record, which was preserved. These are the
canonical table targets, not a claim that arbitrary current tables cannot change.

`B28A40` takes ECX renderer, one stack logical pointer and `RET 4`, with no stable
return contract established. Its sole observed direct caller is alternate
constructor `B4A9B0`, which passes its logical object and the current `F8D394`
renderer. `B28A41` forms **renderer+1AAC**, passes that header and the stack pointer
cell to `B25300` swap-last removal, grows with `B22D10` if needed, and publishes
the unchanged pointer at `B28A7F`. The helpers receive raw array/header addresses;
they do not receive `logical+4C`. Neither registration nor these helpers retains
or dereferences the logical pointee. `B22D10`'s returning-free continuation is
included in the full 95-byte comparison. `B25300`'s prior 104-byte capture includes
its complete returning body and one INT3 padding byte.

The ordinary factory captures its incoming renderer before construction. The
alternate constructor reloads the global renderer for registration. Equality of
those owners across callbacks is not assumed.

## Newly resolved exception-cleanup aliases

A whole installed `.text` scan searched both memory displacements and immediate
adjustments for `1AAC`, `1AB0`, `1AB4`, `19B0`, `19B4`, `19B8`. It decoded
`00401000..00CE2000`, 9,310,208 bytes, producing 3,352,745 instruction/skipdata
records, 269 skipped bytes and 65 candidates. The earlier scan contained 47.
The additional in-band hit `B268E5` adjusts renderer ECX by `1AAC` in the already
known unregistration wrapper. Seventeen hits lie outside the earlier band.
Four are actual adjusted renderer registry aliases:

| Parent and native FH3 path | State and full funclet | Array alias passed onward |
| --- | --- | --- |
| Constructor `B32410 -> CBDEF5 -> FuncInfo DF66EC`, map `DF6710` | State 4 -> 3, entry `DF6730`, funclet `CBDD58` | Saved this `[EBP-45C] + 19B0` |
| Same constructor | State 10 -> 9, entry `DF6760`, funclet `CBDDBE` | Saved this `[EBP-45C] + 1AAC` |
| Destructor `B32920 -> CBE07B -> FuncInfo DF67F8`, map `DF681C` | State 4 -> 3, entry `DF683C`, funclet `CBDF2C` | Saved this `[EBP-14] + 19B0` |
| Same destructor | State 10 -> 9, entry `DF686C`, funclet `CBDF80` | Saved this `[EBP-14] + 1AAC` |

The native prologues save the incoming ECX at `B32430`/`B3293E` and install
`D5F0A8` at `B3243B`/`B32942`. The saved-this offsets agree with the corresponding
FH3 frame-relative funclet loads. Fresh complete handler bytes load the stated
FuncInfo into EAX and tail-jump `BF6B43`; the four selected unwind entries and
all four complete funclets are pinned. Each funclet tail-jumps `B29B20` with the
**array header** in ECX, not a stored stream in ECX. Current Ghidra caller queries
find exactly these four callers of `B29B20`; this is an observed reference set,
not proof against arbitrary indirect calls.

Complete `B29B20..B29B37` is 23 bytes, takes ECX array header, has no stack
arguments and returns with plain RET. It calls `B25940(header,0)`, reloads the
header's data pointer and calls `BF6989` free. Its current live listing stops at
that free call. The missing five-byte continuation is:

```text
B29B32  ADD ESP,4
B29B35  POP ESI
B29B36  RET
```

These bytes were compared live and on disk without repairing Ghidra. The helper
does not clear the header's data pointer or capacity after free.

Full `B25940` is 80 bytes, takes ECX array header plus a signed requested count,
and uses `RET 4`. It reserves raw storage when requested count exceeds capacity,
zeroes newly exposed raw cells, decrements the header count when shrinking, and
finally stores the requested count. No branch dereferences a pointer loaded from
a cell, performs intrusive reference operations on a stored stream, or dispatches
a stream method. For the cleanup call with requested zero and valid native array
state, it shrinks the count and then frees the raw backing allocation. These four
newly traced paths therefore add no stream-owned `+4C` assignment or terminal.

The other thirteen out-of-band constant hits are preserved as candidate records,
not classified as a global absence proof. Focused checks show `C66BB0`/`C66E60`
adjust by `19B8` and reach distinct linked-list cleanup `4C2D30`;
`525FB1`/`526375` form a list address from game-side singleton `E188A8`; and
`7693A0` pushes allocation size `1AB4`, which is not an object-pointer adjustment.
Other game-side matches were not followed as factory-derived stream aliases.

## Limits and remaining dependency

This scan is linear Capstone decoding with skipdata, not a Ghidra control-flow
or alias proof. It may decode data or lose instruction alignment and cannot
find every arithmetic-produced alias, copied header or callback mutation.
All 65 candidate instructions are preserved in the report. The four selected EH
paths are independently grounded by original native handlers, unwind records,
prologues and helper bytes; their conclusion does not rely only on an offset match.
The enclosing renderer constructor/destructor and CRT FH3/free implementations
are not reconstructed or validated as complete by this packet.

The [base-owner discovery](NATIVE_VERTEX_STREAM_OWNER_DISCOVERY.md) establishes
that `B62010` can release an independent retained object from logical `+4C`.
Knowing the logical stream's own `D61D6C` terminal does not identify that object's
runtime profile. Base-constructor zero initialization and all current direct-store
exclusions still do not establish lifetime nullness after escape. Current virtual
dispatch, allocator/free callbacks, reentrancy and other unreviewed APIs remain
outside this bounded result; distinct valid native objects/allocations are assumed.

No nonnull `+4C` writer/type/final-zero terminal was found, so no null-only owner
implementation or source-ready packet is justified. Only this document and its
report are committed. There are no source, test, shared ledger/config/CMake or
Ghidra changes. Byte identity is not build, fixture, ABI-compatibility or gameplay
validation, and no such claim is made.
