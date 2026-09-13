# Actual material-pass sampler removal and shader slots

Addresses: `00b5eff0`, `00b5f080`, `00b5f0c0`.

`native_material_pass_shader_slots.hpp/.cpp` reconstruct these three complete
functions against existing actual pass, sampler-array and refcount storage.
They are new C++ interfaces, not binary replacements. The earlier
`material_samplers.cpp` interpretation remains semantic evidence; this packet
does not route through that projection. Report:
`reports/native_material_pass_shader_slots.json`.

| Routine | Coverage |
| --- | --- |
| B5EFF0 | Complete removal body; existing reserve extent/failure contract |
| B5F080 | Complete setter body; concrete shader terminal provider external |
| B5F0C0 | Complete setter body; concrete shader terminal provider external |

## ABI and producer evidence

All three originals take the actual pass in ECX and one stacked DWORD, and
return with RET 4. EAX has no semantic result. B5EFF0 owns B5EFF0..B5F07F
(144 bytes, 58 instructions); its shared return is B5F022, with the final
backward jump at B5F07E. B5F080 owns B5F080..B5F0BA and B5F0C0 owns
B5F0C0..B5F0FA (59 bytes/24 instructions each). All ranges are inclusive.
Full listings and disk/live byte comparisons establish no unhandled tail.

The existing B44B10 producer calls actual B5F720 to construct the 5Ch base
inside the 88h derived material pass. Base +20 is the sampler-state owner:
vtable +00, actual atomic count +04, row pointer +08, signed count +0C and
capacity +10. Each sampler row is three DWORDs: slot, state key, state value.
Base +54 is the compiled vertex wrapper and +58 the compiled pixel wrapper.
The derived pass's +70/+74 descriptor fields are different storage.

B3B3C0 publishes the B44B10 result at its ESP+14 local (B3B557). Its B3BCC8
call sets ECX from that pass and pushes the B5FAF0 result; B3BFDE does the
same with B5F9B0. B3C060 pushes each unused sampler slot in a 0..15 scan,
with EBP holding that same pass local. The caller's separate temporary
wrapper release after each swap is outside these routines.

B5F9B0/B5FAF0 establish actual 10h shader wrappers with a vtable at +00,
atomic count initially one at +04, COM interface at +08 and a word at +0C.
Their final tables D62A60/D62A70 have virtual0 BD30E0 and virtual4
B5F6E0/B5F700. COM AddRef uses stdcall with the COM identity pushed on the
stack. These constructors are evidence dependencies, not implemented here.

## Repeated sampler removal

B5EFF0 captures the current +20 owner, its count and its row pointer for each
first-match scan. The comparison/index termination uses DWORD equality. If
the found index differs from captured count minus one, it reloads owner
count and data for the last row, reloads data for the destination, and copies
the three DWORDs in order. It does not clear the discarded tail.

It then reloads pass +20, captures that owner's current count minus one,
and compares target/capacity as signed values. B5F068 calls the unchanged
actual B40BE0 reserve helper when target exceeds capacity. The subsequent
signed shrink loop reads the current header count on each comparison,
decrements it in storage, publishes the captured target, and restarts from
the current pass +20 owner. No captured row list is reused across removals.

Ordinary readable extents require nonnegative count <= capacity and enough
12-byte rows. Existing B40BE0 overflow/extent validation and its allocation
failure boundary are borrowed unchanged. This packet does not claim the
native out-of-bounds behavior of corrupt extents rejected by that helper.

## Shader publication and lifetime

B5F080 (+58) and B5F0C0 (+54) capture the old identity before comparison.
Equality performs no slot write, counter operation or canonical lookup.
Otherwise they publish the incoming identity first, increment incoming +04
if nonnull, then decrement captured old +04 if nonnull. The original IATs
CE221C/CE2220 are KERNEL32 InterlockedIncrement/InterlockedDecrement.
The source uses sequentially consistent signed 32-bit atomic operations;
wrapping values are preserved. Only a decrement result exactly zero invokes
the terminal path: old zero underflows without dispatch, for example.

The unchanged `release_native_render_actual_owner` helper performs that
decrement on the actual +04 atomic, resolves its canonical companion only
on zero, verifies it borrows that same atomic, and invokes its nonthrowing
terminal provider. No owner or count is substituted by these setters.
No further old-owner access or slot publication occurs after the terminal.

The original terminal reads the old owner's current virtual0 and calls it
as thiscall with no stacked flags. For the observed shader tables, BD30E0
then reads current virtual4, pushes flags=1, and calls thiscall/RET4. The
scalar wrappers call B5F410/B5F490, conditionally free via BF65AC on flags
bit zero, and return the original address. Those destructors perform
renderer removal and diagnostics, invoke current COM virtual8 Release
(stdcall with stacked COM identity), clear +08 and run BD30F0 base cleanup.
The pixel/vertex diagnostic and renderer-removal order differs and is not
collapsed into a shared guessed shader destructor.

No concrete D62A60/D62A70 shader deletion provider is implemented by this
packet. A real canonical provider must preserve the current virtual profile,
BD30E0/scalar-wrapper ABI, renderer registration and COM/free lifecycle.
The existing owner interface is an explicit external dependency, not a
no-op terminal, fake shader factory, or claim of complete B3B3C0 execution.

`NativeMaterialPassShaderSlotsOperation` retains source diagnostic metadata
outside native storage. The caller keeps the operation, pass and canonical
domain alive and excludes independent retirement/replacement of referenced
arrays/owners during running or failed operations. The operation's scheduled
terminal release may destroy the captured old owner; its retained pointer is
then identity-only diagnostic data. Metadata acquires no extra reference. A borrowed
reserve or canonical lookup can throw after native writes. Failure preserves
those writes and acquisitions; it neither rolls back nor retries. Destroying
a running/failed frame terminates. After explicitly resolving the native
state, `acknowledge_diagnostic_cleanup()` retires metadata only. A completed
or retired frame cannot be replayed. This is a host failure boundary, not a
new claim about original FH3 unwinding.

## Verification and limits

The focused ignored fixture copies all three original bodies plus BD30E0.
Only B5EFF0's direct reserve CALL displacement is relocated; IAT slots are
bound to stdcall shims executing actual Windows interlocked operations.
Original and source share the unchanged actual B5F720 pass/state producer,
shared heap domain, B40BE0 reserve and canonical B5F510 state-owner cleanup.
The full B44B10 renderer-dependent constructor is inspected, not executed.

The actual producer supplies 96 sampler rows. Removing slot 4 twice, then
15 and FFFFFFFF, leaves 84 rows with identical order, capacity and every
unused tail byte. Unrelated raw 5Ch pass bytes are unchanged.

Both slots run ten original/source raw-wrapper ABI boundary variants:
terminal/nonterminal, old zero, signed and unsigned counter wrapping,
null old/new/both, equal identities and terminal mutation of the published
slot. Fixture-only 10h storage has the actual +04 atomic, copied BD30E0 in
virtual0 and a virtual4 observer that checks flags=1, publication and counts.
It deliberately does not emulate COM owners, native shader constructors,
renderer registration or real shader destruction/free. Counter-invalid
preimages are diagnostic fixtures, not balanced shader ownership examples.
Separate source-only failed canonical lookup checks publication retained,
incoming count two, old count zero and explicit diagnostic retirement;
unacknowledged frame destruction exits through the terminate guard (77).

Default MSVC Win32 Release /W4 /WX build, verified native seeds and both
existing CTests passed. The focused fixture and guard passed. The report
records all direct and indirect CALL rows, including producer/destructor
evidence and the three caller sites. Numeric rows are checked live with
`tools/verify_report_calls.py`; indirect ABI evidence is separately bounded.
Disk/live pins cover 2,257 bytes in 17 spans. Local final manifest pins
original/linked bodies, scripts, fixture inputs, source/header closure,
libraries, executables, build/test logs and call verification. No game run
or drop-in ABI compatibility is claimed.

Root repaired two saved Ghidra scalar-wrapper continuation gaps under the
official write lock and re-exported them. Refreshed B5F6E0/B5F700 each have
11 instructions and zero gaps; this worker performed only Ghidra reads.

## Integrator terminal-lifetime composition

The separately implemented actual shader lifetime providers now compose with
these routines. `NativeD3d9ShaderReference` borrows the same raw+04 atomic;
both pass-slot setters exercise actual scalar destruction, registry removal,
COM Release, singleton heap free and canonical companion retirement. Eight
original/source lifetime comparisons use B5F9B0/B5FAF0-produced wrappers and
real HAL shader COM objects. See `docs/NATIVE_D3D9_SHADER_LIFETIME.md` and
`reports/native_d3d9_shader_lifetime.json` for the independent fixture and its
input hashes. This new composition does not change the narrower historical
worker fixture or establish private FH3, fixed-address ABI, drawing or gameplay.
