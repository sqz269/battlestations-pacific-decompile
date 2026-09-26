# Complete texture-source time and clock dispatch, CC10

`C302A0` now has a complete raw MSVC Win32 source provider for both child-count
branches. An explicit source overload of `B19A10` reads current genuine resource
profile tables and routes their current time slot to that provider. The existing
naked numeric B19A10 interface and its complete71-byte emitted body are unchanged.
The new leaf is registered in the normal CMake build. No caller is activated.

## Native callback and source ABI

The recovered C302A0 body is74 bytes. Original ABI is ECX source, one stacked
float, RET4. The source fastcall interface adds EDX pointing to the actual readable
eight-byte CE47A0 double storage. It supplies no literal fallback and dereferences
that storage only at the native FDIVR point on the nonzero branch. The native
constant bytes are00 00 00 00 00 40 8F 40, exactly1000 as a double.

Capture source+14 once. Zero count skips all input/rate/constant/FPU work and
leaves source+8 unchanged. Nonzero executes FLD incoming float, XOR EDX,EDX,
FLD source+0C, FDIVR actual double, then FDIVP. Native division order and extended
intermediates are retained; no multiplied-float expression or rounded C++ cast
replaces them. FNSTCW saves the original control word. OR only0C00 into a temporary
copy, FLDCW that copy, FISTP signed qword, load only its low DWORD into EAX, and
unsigned DIV by the captured ESI count with EDX still zero. Restore the original
CW before storing EDX remainder to source+8. The quotient and qword high half are
unused. No second count read, signed modulo, clamp, rate fallback or validation
is added. The callback neither accesses texture children nor transfers ownership.

The source saves extra EDI to retain its added scale pointer. With entry ESP=S,
SUB8/PUSH ESI/PUSH EDI gives ESP=S-16. Incoming float and later saved original CW
are at ESP+14h=S+4. Temporary CW and qword low are at ESP+8=S-8; qword high is at
ESP+0Ch=S-4. Saved EDI and ESI are at S-16/S-12; none overlaps the qword, CW or
return address. POP EDI/POP ESI/ADD8/RET4 closes the frame. Net x87 stack depth
is unchanged on normal return. Hardware faults/unmasked x87 exceptions can stop
before CW restoration; noexcept is not fault cleanup or native SEH/FH3 proof.

## Genuine current-profile dispatch

The regular C++ overload borrows three actual profile views and the actual
constant backing. Current D64478, D644B4 and base D79B54 table+18 all select
C302A0. It reads the current sink profile and the corresponding borrowed actual
table's current slot every iteration. Numeric native table entries are selectors,
not callable host function pointers. No replacement table, pointer patch, fake
sink, extra identity map or retain is used.

The initial assembly loads incoming bits, captures owner+8 before writing owner+18,
then reads count+C. Each current record+28 sink receives a fresh FLD/FSTP copy of
the original parameter. Current sink+0 and selected table+18 are captured between
that FLD and FSTP, with no call or extra floating operation. Qualified C302A0 then
invokes the genuine full callback. After return, current count is read before
current data, the captured record address advances2Ch, and the native comparison
repeats. DWORD products/address arithmetic wrap as native does. There is no null
skip, count normalization, current-time reread from+18, vector change or release.

Unknown profile, absent borrowed view or unsupported current slot is an explicit
source boundary. Complete the FLD/FSTP pair before throwing at the regular C++
function boundary; no throw/helper call runs through the old naked entry or
with a new x87 stack value pending. The already-written+18 and all borrowed
lifetimes remain with the caller. This policy does not reproduce a native bad
profile hardware fault. All profile/constant/context backing must survive use.

## Build and emitted verification

Implementation baseline is `616f96c331e8e868773703d24bfb8ff3c0c02bfa`, safely synced
after published application shutdown integration. Four code files exactly match
the reviewed ignored proposal; one required registration line is appended to
cmake/startup.cmake. Before build, eight source/config inputs, exact baseline Git
tree,27 existing tool/SDK identities and preconfigure cache were sealed. All43
previous archives and1,188 prior frozen files remained unchanged.

One fresh Release Win32 build completed2026-09-26 08:44:04.258262 to08:44:34.244796
UTC, exit0. Actual project flags are /MD, /EHsc, /O2, /W4, /WX and /fp:strict.
All three existing CTests passed. No tracked test or ad hoc executable was added.
Actual build launch and terminal wait tool-return objects are archived separately.
There is no new callback/clock/sampler fixture or application/game run.

Exact CMake objects match both new core-library members. Three complete COFF
bodies cover429 bytes: original naked B19A10[71], new C302A0 source[74], regular
source dispatch[284]. All bytes decode; no embedded-data repair was required.
The old naked body is identical to its frozen old object/member and the original
native71-byte body, with no relocation. Full callback instruction order matches
the original after explicitly accounting for EDX->savedEDI scale, added EDI
save/restore, stack+10h->+14h and+4->+8 shifts, and FDIVR absolute constant->[EDI].
FDIVP opcode DEF9 divides ST1 by ST0. The zero branch at offset0Ch targets42h,
skipping every floating access. The original CW restores at3Bh before the+8
remainder store at3Fh. The callback has no COFF relocations.

The regular overload's incoming FLD is at5Dh; current profile read60h and current
slot read8Fh precede FSTP9Eh. There is no call or additional FP operation in that
68-byte window. Spilled EBP-1Ch is loaded by MOVSS atB0h and copied to a four-byte
stack argument by MOVSS atBCh. This preserves the spilled bits without FP
reconversion. ECX receives the captured sink, EDX the actual scale pointer, and
the REL32 operand atC2h names the real source leaf (CALL atC1h, callee RET4).
Current count readCCh precedes data readD7h. Both actual source error calls are
after the spill. `compiled_schedule.json` pins these exact instructions and
`compiled_evidence.json` retains complete bodies, relocations and member hashes.

A static inspection helper initially included the DIR32 invalid_argument RTTI
data relocation in its CALL count. Its failed version and diagnosis are retained;
filtering actual calls by REL32 kind corrected the inspection. Production code,
objects and library were not changed or rebuilt for that correction.

This is complete COFF and exact archive-member evidence. No new linked entry,
original entry/register/private-stack compatibility, hardware-fault equivalence,
native FH3/SEH or callback runtime is claimed. Source naming remains descriptive.

## Separate prerequisites

Fresh BBC6F0/BBC810 resources have zero internal child count and unspecified+0C.
The recovered zero branch avoids that rate preimage, but it cannot substitute
for the full nonzero contract. Genuine positive child/rate production is the
unimplemented C30570 Lua `shaderfx/animtextures.lua`/FPS/current renderer texture
path. No FPS/count/child vector is fabricated and no C30570 source is added.

The raw clock-owner/typed vector-header lifetime boundary remains separate.
This packet performs raw assembly accesses and does not unify or placement-create
the owner, nested header or sampler view. It does not activate B1B4D0/B1A4F0,
platform5030/MSG/current online/input services, source0/W, a full sampler/material
caller, application publications, startup, drawing or the original game. The
new API creates no callback context implicitly and installs no ownership binding.

The report is `reports/native_texture_source_time_cc10.json`. Evidence is frozen
in `local/cc10_texture_source_time_evidence.zip`; member manifest and external
freeze receipt are under `local/cc10_texture_source_time_implementation`.
The earlier read-only readiness and concrete proposal archives remain immutable.
