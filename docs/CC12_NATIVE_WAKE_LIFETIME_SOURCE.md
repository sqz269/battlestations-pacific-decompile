# Native wake lifetime Source

The existing raw-fill translation unit now implements the complete native
wake constructor **00815600 (115 bytes / 28 instructions)** and plain
destructor **00818100 (14 bytes / three instructions)**. The adapted private
constructor is **106 bytes / 30 instructions**; its public ordinary wrapper
is **15 bytes / six instructions**. The destructor remains 14 bytes / three
instructions, with only its actual Source release target relocated.

## Actual storage and ownership contract

```cpp
void* construct_native_unit_wake_00815600(
    void* actual_wake, game::GameNativeGeometryGlobals& actual_geometry);
void __fastcall destroy_native_unit_wake_00818100(void* actual_wake);
```

The caller supplies actual writable **988-byte native wake storage**, with
independent lifetime and no existing live section to overwrite. The method
does not allocate that storage or introduce a raw-wake class, scratch owner,
semantic copy or allocation-root permission. `ShipAiWakeTrail` starts its
samples at zero; the older `UnitPoseHistoryRing` has 20-byte slots and an
828-byte object. Neither is a valid overlay. No game host or existing typed
caller is changed, and no actual unit allocation/producer is newly admitted.

Native layout is vptr word +0, section slot +4, forty 24-byte records at +8,
head +0x3C8, full-byte flag +0x3CC and residual +0x3D0. The literal vptr value
`00D09480` is retained as an observed native word. No Source vtable or C++
virtual-dispatch binding is fabricated by storing it.

The geometry argument must be the genuine process instance returned by
`game_native_geometry_globals()`. Its actual private-constructor object is
one writable, initially zero 12-byte allocation with escaping mutable array
access. The new wrapper obtains only that array's address. Raw wake storage
must be distinct from the canonical cells, active wrapper/kernel/provider
frames and formal argument slots. Caller-owned synchronization and lifetimes
cover construction and every borrower. Destruction follows the end of all
borrowers and the real section's owning/quiescent-thread requirement.

## Exact constructor order and context adaptation

The constructor preserves the complete native order:

1. XORPS, the native ESI save and vptr store; forty ordered heading/segment
   zero pairs, advancing by 0x18. Yaw and unrelated preimage are preserved.
2. Direct call to the actual current 39-byte section-creation body.
3. Native FLDZ, PUSH ECX and FSTP to the heading argument; push the actual
   mutable seed pointer, set ECX to the wake, then publish EAX to +4.
4. Direct call to the existing **private 314-byte / 85-instruction fill** in
   the same translation unit. The section is published before this call.
5. Three fresh MOVSS reads of the real seed cells, interleaved with residual
   stores at +0x3D0/+0x3D4/+0x3D8, then return the same wake.

There are exactly two added pointer operations. `PUSH EDX` follows the native
ESI save and retains the canonical address at constructor entry ESP−8.
Original `PUSH F87574` becomes `PUSH [ESP+4]` after the native heading spill;
x86 evaluates that effective address before decrementing ESP. A `POP EDX`
immediately after raw fill both reloads the address and restores the native
ESP before the residual reads. Only the three original absolute MOVSS seed
operands change to `[EDX]`, `[EDX+4]`, `[EDX+8]`. There is no value snapshot,
literal reset substitution, dummy EDX clearing or extra FP operation.

Every original instruction maps to the complete compiled kernel: 21 exact
instruction byte sequences, one mapped loop branch, two real call
relocations, one saved-pointer PUSH operand and three fresh-cell operands.
The extra PUSH/POP are flag-neutral. All mapped x87 states agree, and ESP
differs by precisely the saved four-byte word until the fill return. The
original ESI save remains at entry ESP−4.

Raw fill sees position and heading at its original +4/+8 argument locations.
The saved pointer is at fill entry +12, beyond the final direct argument byte
at +11. Complete fill-stack inspection excludes that word; indirect wake and
seed accesses are disjoint from frames under the explicit contract. The real
create/allocation calls use their own lower frames and actual arguments.
There is no assumption about a caller's EBP. Maximum local x87 use is four
slots through fill, excluding CRT/OS interiors. No FP reset is introduced.

The whole public wrapper is integer PUSH/MOV/POP followed by a tail JMP to
the actual local kernel. It reads no seed values and adds no FP or EFLAGS
operation. Both emitted canonical-array accessors are `MOV EAX,ECX; RET`.
This ordinary constructor API adds Source context; it is not the Original
no-context thiscall entry, unchanged body or volatile-register/flags ABI.

## Genuine section services and plain destruction

The real creation service pushes **0x1C**, calls the existing allocation
wrapper, calls imported `InitializeCriticalSection` and initializes depth.
The actual allocation wrapper is **58 bytes / 20 instructions**, including
its existing security-cookie bookends. It forms request kind 2 and native /
host sizes **28 / 28**; these are allocation sizes, not wrapper code length.
The current **90-byte / 37-instruction** allocator reads `host_bytes`, calls
real malloc, and retains its current new-handler/retry/bad-alloc path. The
matching free service is a **six-byte IAT tail jump** to real free.

The complete plain destructor performs `MOV [ECX],D09480; ADD ECX,4; JMP`
to the actual **64-byte / 25-instruction** owned-section release. That body
captures the real slot/section, drains positive signed depth on the owning
thread through `LeaveCriticalSection`, calls `DeleteCriticalSection`, frees
the actual malloc-backed section, and clears its owner slot. The null arm
leaves the slot unchanged. No wake allocation-root free or unrelated wake
store is introduced.

Seven unique complete SDK code-import members are physically retained before
and after the build: Initialize/Leave/DeleteCriticalSection, malloc, free,
_callnewh and _CxxThrowException. Their actual I386 name transformations,
libraries and DLL targets are verified. The complete existing allocation,
free, create and release bodies and ordered relocations are retained; no
Original CRT interior or failure-policy equivalence is inferred from them.

## Build and physical proof

One normal `./scripts/build.ps1` passes all three existing checks:
`reconstructed_math` (0.32 s), `native_math_differential` (0.04 s) and
`tool_tests` (8.02 s). No new test, ad-hoc link/probe or new entry execution
was added. The existing LNK4006 duplicate `spawn_request_id_matches` warning
remains. Construction/destruction failure, hardware-fault resumption and
Original CRT/SEH policy are outside the normal successful domain; no rollback
or fallback lock is invented.

All **98 pre-existing complete function extents** across six selected objects
retain their entire bytes and ordered relocations. There are 103 current
extents, including the new entries and emitted accessors. Four actual EH
subfunction starts occur at nonzero section offsets (170, 13, 254 and 161);
the independent reader retains their true separate extents instead of
duplicating whole sections. Five provider objects are byte-identical across
the build. The old fill's 314-byte kernel, 29-byte bridge, four read-only
constants and every relocation remain unchanged.

Three physically retained complete core archives each contain exactly one
member equal to each selected whole object. The post-build archive has 1,905
members; the two pre-build snapshots have 1,904. The incoming main merge had
already registered `native_parent_header_clear.cpp`, which the normal build
also compiled. That aggregate growth is not credited to this packet.

Source/header baselines were retained before editing. The new fill translation
unit's actual 192-file compiler dependency set was physically captured before
the build and remains unchanged. Existing
provider records cover 189/165/181/73/127 dependencies; those providers were
retained unchanged, not recompiled this turn. Actual pre/post sources,
recipes, configured tools, libraries, whole objects and archives are retained.
Generated pre-build records describe the preceding build; current reconfigured
records are retained post-build without fabricating earlier evidence. Git
normalizes tracked text to LF separately from physical compiled-input hashes.

The required headers emit the existing four-byte SDK
`__Avx2WmemEnabledWeakValue` selectany fallback. Its actual COMDAT auxiliary
record, alternatename directive and absence of parent-function references are
verified; the baseline canonical-owner object already contains the same
symbol. It is separately accounted for and is not reset storage.

Full mappings, CFG states, provider bodies, archive members and hashes are
indexed in [the report](../reports/cc12_native_wake_lifetime_source.json) and
its sealed ignored evidence family. Admission is limited to complete Source,
normal existing checks and current object/archive bindings. No application
COMDAT/map/PDB full-body qualification, actual unit owner/producer, game
wiring, startup or gameplay validation is claimed. Shared annotation,
registration and integration remain with Root.
