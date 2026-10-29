# Additive qualified Dyn profile bindings

This packet implements the approved six-file portion of readiness proposal
`713c8776330c076604279ec18fa4baee982020e9`, based on accepted
`605373bf732e4bd3089e8f07ddca226ab4d635a3`. It adds a matched-domain supplier
and two explicit qualified caller interfaces. It does not construct a profile,
world or radix manager, install a table, attach a game or change an activation
guard. Native, vendor and activation body credit is zero. Existing frozen math
receipts retain their original scope.

## Actual default raw-allocation supplier

`GameNativeDynProcess::profile_partial_node_delete_00bf65ac() const` requires
completed dispatch startup and uses the existing process startup mutex. It
compares the actual owner's memory **context, allocate and release** fields
against the existing `process_memory` tuple. Only that exact tuple returns the
real unsized scalar `::operator delete(void*)` function pointer. An unmatched
custom binding throws `std::logic_error`; it receives no generic BF6989/free
fallback or global context thunk.

The recognized allocator explicitly calls global scalar `::operator new`, and
its existing release explicitly calls global scalar `::operator delete`. The
accessor returns the same matching raw-storage release function, not a delete
expression or a new native-class destructor. It neither allocates another
process owner nor modifies first-bind-wins. The returned pointer is valid for
this actual source allocation domain; it is not evidence of original-image CRT
binary identity or arbitrary custom/module compatibility.

The caller must retain the same actual process memory and publication cell
when forming `DynProfileScopeContext`. The accessor does not establish that a
9Ch profile or its 48h nodes have been constructed. A C5 caller also needs its
real raw manager, vector CRT, canonical feature reference, half cell and five
profile names. The separately supplied `DynLiveCrt` does not establish the
profile node's allocation custody.

## Old and new APIs

| Interface | Resulting contract |
| --- | --- |
| Existing four-argument C57020 | Remains normal-only; its cold path calls four-argument C50390. |
| New five-argument C57020 | Requires `DynProfilePartialNodeDelete`; rejects null before profile/scope access, then cold-calls the actual five-argument C50390. |
| Existing six-argument C5A8B0 | Explicit normal-only context selects four-argument C50390. |
| New seven-argument C5A8B0 | Requires the same matched raw-node provider; rejects null before manager/profile access, sets the explicit qualified selector and cold-calls five-argument C50390. |

The new overloads throw `std::invalid_argument` for a null provider. Their
invalid-input paths do not call the instruction kernel, append a child or mutate
manager/profile data. A missing provider never causes a qualified call to
silently use the legacy path. Non-null validity and genuine matching allocation
custody remain caller preconditions.

Scope-enter shares its storage/publication/RDTSC sequence through a private
compile-time helper. Its legacy instantiation selects normal append explicitly;
the null internal placeholder is not inspected or used as a qualified fallback.
The original cache slot is still held across cold append while the published
profile pointer is reloaded, followed by the original node/current publication
and timestamp order.

`DynProfileScopeContext` is unchanged and asserted to be 10h on Win32, with
memory at +0 and publication pointer at +0Ch. The separate scope storage stays
14h. C5's existing private context prefix stays exactly 28h: CRT +0, feature +4,
half +8, profile-cell pointer +0Ch, profile context +10h and five names +14h.
Only an explicit qualifier DWORD at +28h and provider at +2Ch are appended;
total private context is 30h. No original scratch/frame slot is repurposed.

## Instruction, call and FP evidence

Strict optimized MSVC Win32 objects were compiled for all three baseline and
changed `.cpp` files using `/MD /O2 /Gy /fp:strict /EHsc /W4 /WX`. All six
compilations passed. No executable or runtime fixture was produced by this
object-only check.

The full inherited PE/COFF/CFG audit was rerun against the changed strict C5
object, retaining its original supported-target byte checks and documented
interface adaptations. It maps all **654 original listed instructions to 704
emitted instructions**, checks **52 branches / 41 distinct targets / 11 calls**,
and reaches all 654 original rows. The 15 original body/unwind call rows pass
the actual call verifier. The reference image agrees with the accepted retained
live bytes; the project/program was verified before the read-only analysis.

The kernel still has 101 x87 instructions and 27 FSTP epochs, maximum added
x87 depth four. All modeled call/return boundaries have added x87 depth zero. Ordinary bridged
calls have only their original EAX result live where consumed; the fixed stack
probe's separate register contract remains unchanged. The new append selector
branches in the C++ bridge at that zero-added-depth boundary, before either real
append call. It introduces no FP arithmetic or control-word operation. Public
C5 wrappers only use the existing SSE data-copy instructions for names.

Baseline/new COFF comparison additionally proves identical machine bytes and
normalized symbol relocations for 15 existing functions: the C5 kernel, four
shims, four-scope leave helper, legacy C57020, C5BCD7 leave, both C50390 overloads,
both canonical process accessors/factories and the allocator/release wrappers.
The fifteenth comparison is a compiler-emitted `unique_ptr` release helper;
it receives no vendor-body credit.
Anonymous namespace symbol hashes were normalized only for comparison; actual
relocation targets remain recorded. The C5 public context wrapper and append
bridge intentionally differ because they carry/select the additive inputs.

Post-build common objects have the same kernel, four shims, append bridge and
supplier bytes/normalized relocations as the strict objects. Four public C++
wrappers and the C++ scope-leave helper use EBP-relative frames in the common
build and ESP-relative frames in the standalone build; they are not byte-identical.
Their actual context stores, selector/provider offsets, argument cleanup,
qualified append target and early null-failure paths were inspected separately.
The common compiler inlines legacy C50390 into old C57020; that path still
selects normal C44000 and no qualified child cleanup.
The helper's core operands and branch targets also match after removing its
balanced EBP frame and inert NOP padding and resolving the same argument slot.
No whole-object identity is claimed between those build modes.

The native stack/bridge checks retain `S+10B0` context custody, footer
`ESP+10AC` after EDI was popped, returned-buffer spill at `S+14`, native caller/
callee argument cleanup, and the four real C38040 scope-leave actions. The
qualified append adds exactly one C++ argument (20-byte C++ caller cleanup);
legacy append retains 16-byte cleanup. Both return the actual child in EAX to
the unchanged shim. There is no additional original native call/body credit.

The supplier object contains a direct DIR32 relocation to
`??3@YAXPAX@Z`, the real unsized scalar delete symbol, together with the exact
default allocator/release address comparisons. Final common-build map evidence
and build receipts are recorded in the JSON report. A returned function pointer
and static cleanup actions do **not** prove rethrow transport through the
unregistered naked kernel, original FH3/SEH/fault/frame identity, secondary
cleanup failures, timing, concurrency or every ambient FP exception domain.

## Required build and boundaries

The fresh named tree is configured with `BSP_CL_MP=2` and `BSP_PCH=ON`; the
required `scripts/build.ps1 -Configuration Release` runs the normal build and
existing tests. The build passed and both existing tests passed (12.94 seconds
total). Its terminal result, exact compiler/cache/map/object hashes and
test receipt are in `reports/cc16_dyn_profile_qualified_binding.json`. No new
tests were added, and no game or probe was launched. Compilation of an unused
qualified function is not runtime adoption.

Root C50310 remains outside this packet. Its native state0 extends through
profile publication and BF79F0, unlike C50390's constructor-only state; its
captured raw-node cleanup and enclosing engine scopes need their own qualified
source work. Collision, world-step and solver mode1 still use their old direct
append bridges, and world-step's enter/collision dispatch does not select these
new overloads. The existing process context/runtime construction is unchanged.

Custom memory needs its actual owner to provide a matching flat deleter or a
separately reviewed context-bearing binding. This accessor deliberately leaves
that domain unqualified. Actual game/profile/radix owner production, vtable
custody and native ABI/exception/runtime admission remain separate work. No
shared CMake, ledger, Ghidra annotations, global provider state, original game
installation, saved analysis or frozen gameplay evidence was changed.
