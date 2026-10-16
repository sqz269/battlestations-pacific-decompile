# Whole raw traffic DWORD insertion

GOAL15X reconstructs only 00493120, all436 Native bytes and181 instructions.
The Source is src/native_traffic_dword_insert.cpp; the physical interface is include/bsp/native_traffic_dword_insert.hpp. The existing ledger
library name STL_Vector_InsertN_00493120 remains a descriptive library
instantiation hypothesis. No game class is inferred.

## Actual physical entry and mutable argument cells

Original input is ECX pointing to a borrowed16-byte header. Word0 is opaque;
begin/end/capacity words are at+4/+8/+0C. Four actual stack DWORDs are present:
stack+4 opaque first preimage, +8 position, +0C count, +10 value-cell address.
The opaque word is not directly read; indirect aliases can still reach it.
Original consumes no incoming EDX input. All normal exits use RET10.

Source uses a naked fastcall envelope with a Source-only unused EDX parameter
to keep all four actual stack DWORDs in place. That parameter must never be
invented in Original metadata. This API exposes physical EAX for inspection,
without claiming a recovered Original semantic result or class. The only
actual Native caller ignores EAX. Count0 leaves the original value-cell
address; allocation leaves new end; both in-place paths leave final fill end.

Before any count check, the entry loads its fourth argument, dereferences that
address, and overwrites the actual fourth argument cell with the DWORD value.
It then reads actual begin and conditionally actual capacity. Count0 retains
all those reads/writes. There is no added null/value/count guard.

The in-place route overwrites the actual third/count cell with wrapped count*4.
All four Fill/FillN sites borrow the address of the actual overwritten fourth
cell. Late count-byte-span reads occur after preceding child calls. Replacing
these cells with locals, a constant, an early pointer snapshot, or an ordinary
typed container would narrow the observed caller-frame alias contract.

Whole readable assembly retains all181 instructions and actual physical
argument cells. No byte embedding, padding for counts, provider shim, callback,
global, unresolved-body stub, repaired guard or RAII cleanup was added.
The initial strict compilation rejected named parameters unused by inline
assembly; the definition now uses unnamed parameters with explanatory comments.
The public declaration retains names. Strict W4/WX compilation then passed.

## Branches, arithmetic and publication schedule

Capacity and size use the actual null-begin branches, wrapped32 subtraction
and arithmetic SAR2. Count is compared unsigned against wrapped3FFFFFFF-size.
Capacity growth uses logical SHR1, the original unsigned overflow comparison,
and repeated actual end loads. Signed/unsigned behavior and wrapping remain
literal; malformed ranges are not repaired.

Allocation reloads begin/position only after actual allocation. It reloads
position and header end after FillN, then rereads begin/end after the final
copy before capturing old size and conditionally freeing actual begin.
It computes wrapped new capacity/end, stores begin+4, pops EBP and EDI, stores
capacity+0C, then stores end+8. There is no added release-on-copy-error guard.

The short-tail branch copies the captured tail, reloads current end, recomputes
the remaining count, performs FillN, then reloads actual count-slot byte span.
It increments end and reloads end before the final fill.

The long-tail branch copies the old last count values, pushes all backward-copy
arguments, publishes new end, then performs backward copy. It reloads actual
count-slot byte span only after that copy, and preserves both pending cdecl
argument groups until ADD ESP,18. Flags survive MOV/LEA/PUSH where Native uses
them; all arithmetic, branch encodings and physical stack offsets match.

## All nine copy/fill setups

Each row describes the actual three stack arguments in order.
The two copy routines with RET0C use their genuine Source stdcall interfaces;
Fill and backward-copy use genuine cdecl interfaces. Incoming ECX/EDX are not
invented child inputs.

| Native site | Parent path | Actual stacked values | Cleanup |
| --- | --- | --- | --- |
| 004931DE | allocation | begin reloaded from header+4 after allocation; position reloaded from actual parent stack+8; new storage captured in EBP | child RET0C |
| 004931EC | allocation | preceding copy EAX; original captured count EBX; address of actual overwritten parent stack+10 value cell | child RET0C |
| 004931FD | allocation | position reloaded after fill; header+8 reloaded after destination push; fill EAX | child RET0C |
| 00493260 | capacity sufficient, tail<count | position captured in EDI; old end captured in EBP; wrapped position+count*4 | child RET0C |
| 0049327A | capacity sufficient, tail<count | current header+8 reloaded after copy; captured count minus recomputed SAR2(current end-position); address of actual parent stack+10 value cell | child RET0C |
| 00493292 | capacity sufficient, tail<count | captured position; newly reloaded published end minus late-loaded actual count-slot byte span; address of actual parent stack+10 value cell | child plain RET; parent ADD ESP,0C |
| 004932A8 | capacity sufficient, tail>=count | captured old end minus captured count-byte span; captured old end; captured old end | child RET0C |
| 004932B3 | capacity sufficient, tail>=count | captured position; captured cut=end-byte span; captured old end | child plain RET; three arguments retained |
| 004932C5 | capacity sufficient, tail>=count | captured position; position plus actual count-slot byte span reloaded after backward copy; address of actual parent stack+10 value cell | child plain RET; parent ADD ESP,18 cleans both cdecl groups |

The complete matrix with register preimages is also in reports/goal15x_traffic_insert_n.json.

## True Native transfers and actual caller

At 00493169 it calls 00490240.

At 004931CB it calls 0047E970.

At 004931DE it calls 0048D660.

At 004931EC it calls 0048E4A0.

At 004931FD it calls 0048D660.

At 0049321C it calls 00BF65AC.

At 00493260 it calls 0048D660.

At 0049327A it calls 0048E4A0.

At 00493292 it calls 00486800.

At 004932A8 it calls 0048D660.

At 004932B3 it calls 00486820.

At 004932C5 it calls 00486800.

At 00495B2C it calls 00493120.

The whole actual caller 00495AE0 is179 bytes/78 instructions. Its growth setup
uses LEA EAX,[ESP+20] to borrow the caller's actual original stack+8 value word,
pushes that address, subtracts captured old size from requested size, pushes
count/end/header, and places actual header in ECX. It supplies all four stack
cells. This is not a zero-fill constant or a synthesized local snapshot.
The whole caller was inspected/captured; it has no new Source implementation
or closure claim in this packet.

## Genuine providers and declared boundaries

The allocation and length failure Source entries are exactly T pin653908afe.
T was pending independent max review at X start. The primary has explicitly
accepted the review READY with no blockers under its current CRT/owning
payload/EH boundaries. The acceptance and exact review hash are retained in
T_review_acceptance.json. Main integration/build/annotation was still pending
at that acceptance; X does not turn it into a Main or Original ABI claim.

Allocation uses the existing canonical actual malloc/_callnewh retry service
and its accepted current std::bad_alloc overflow policy. Length failure uses
the actual existing owning28h NativeSingletonVectorLengthError, real raw
legacy owner/SBO construction/copy/destruction and CompletedMessage cleanup.
It does not substitute std::length_error, fake a Native catch type, or dispatch
the D69260 data word as a host vtable.

All four R helpers use their complete registered Source and actual current
SDK memmove_s service where present. Free binds to existing genuine
singleton_lifetime_free, whose actual implementation uses std::free in the
same declared current allocation domain. Native BF65AC is a five-byte JMP to
BF9DC8. Rebuilding the private BF9DC8 free implementation is not claimed.

Current allocator/new-handler/copy/free/errno/handler/owning-host-EH policies
remain qualified external boundaries. Native private CRT/FH3/RTTI/ThrowInfo/
SEH/throw identity and arbitrary hardware-fault/unwind-frame behavior remain
unclaimed. The parent adds no guards or cleanup between these boundaries.

## Whole emission, archive and one focused runtime case

Fresh live and installed bytes match for all nine captured extents. Both
strict and registered own COFF contain one whole436-byte entry,181 instructions:
388 identical fixed bytes and48 bytes of twelve actual zero-addend CALL
rel32 operands. Every nonempty executable section, symbol offset/extent,
instruction and genuine call binding is retained. No NativeSize prefix was
used. Internal branches, memory operands, registers and stack cleanup match.

All nine genuine provider objects are retained as complete CODE/data/symbol/
relocation evidence, totaling8882 executable bytes. The linked fixture checks
the entire own436B code, all12 actual archive child bindings, and77 whole
linked provider contributions; unused provider functions are still accounted
in complete COFF rather than claimed as executed. Actual map origins identify
bsp_core objects. Source/provider hashes and exact archive hash are recorded.

Actual scripts/build.ps1 ran once after settled Source: PASS. Both available
existing CTests reconstructed_math and tool_tests pass. Native seed reference
was not enabled. Strict MD/O2/Gy/W4/WX/fp:strict/EHsc compilation passes.
The existing unrelated LNK4006 spawn duplicate warning is retained in the log.

Exactly one focused alias case uses genuine public registered Source children,
the actual public Source parent, and the complete captured Native436 body.
Only the twelve true CALL operands are rebound; every other Native byte is
retained and the Golden literal/hash is independently checked.

Its runtime route is specifically capacity sufficient and tail>=count. The
actual value-cell address points at the actual outgoing count cell initialized
to2. The parent overwrites the value cell with2 and count cell with byte span8.
The case executes the long-tail copy/backward-copy/fill sites, obtains
10,2,2,20,30,40, preserves the opaque word/capacity, publishes end+6, exposes
EAX+3 and observes RET10. Both Source and Native pass the same focused case.

Parent allocation/free, short-tail, overflow throw, unwind and invalid-copy
error-service paths were NOT exercised. They retain whole-body static
instruction/call/provider evidence only. T's previous standalone allocation/
owning-transport runtime fixture is separate and is not parent-path proof.
In particular it does not prove current C++ exception propagation through
this naked parent.
No new tracked test/framework or broad suite was added.

## Preservation, remaining gates and reproduction

Before any rebuild, all63 original T artifact hashes, Source/header/doc/report,
actual registered archive/own object/seven canonical objects and provider
inputs were copied and verified:98 saved copies under prior_goal15t.
Original R artifacts remain intact. Shared CMake gains one append-only line;
only the owned address's additive0049 name/reconstruction records are added.
The frozen base remains T653908afe, per primary instruction.

No Ghidra, game or deployment mutation occurred. Old names/prototypes/comments/
xrefs were captured for primary annotation under the write lock. Preserve
the correct library ledger name/comments; use actual ECX pointer plus four
stack words and RET10 without a dummy Original EDX or inferred class.

Whole495AE0 resize,49E560 traffic tail,4C71C0 layout,4D5BD0 parent and authored
mutable-global producers remain OPEN. No whole-world adoption, drop-in binary,
startup, visual or gameplay validation follows from this packet.

Full qualification/build/reject/docs/commit/release/handoff wall time starts
at 2026-10-16T04:36:00+00:00 and is pinned in ignored receipt.json at handoff.
Reproduce static audit with python local/goal15x_traffic_insert_n/audit.py.
The build_probe.ps1 driver supports CompileOnly or FixtureOnly; the default
performs strict compile and actual registered archive fixture. The actual
registered build is scripts/build.ps1. verify_receipt.py verifies frozen
artifact/Source/provider/commit/object/archive hashes without execution.
