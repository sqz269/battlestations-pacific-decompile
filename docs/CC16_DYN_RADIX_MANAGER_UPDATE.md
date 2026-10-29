# Dyn radix manager endpoint sort and sweep

This packet recovers only `00C5A8B0..00C5B175`, the RTTI-attributed slot 3
of `Dyn::SAPRadixBroadPhaseManager`: 2246 contiguous bytes, 654 listed
instructions, 52 branches and eleven calls. The descriptive public name is
`bsp::cc16_dyn_radix_manager_update::update_radix_manager_00c5a8b0`.
The undefined `00C5B180` pointer-retirement wrapper is a separate body and
receives no coverage here. There is one new in-house normal body; reused
profile/string/CRT implementations and compiler support receive no credit.

All live queries used the verifying `bsp.py ghidra` client for project `bsp`,
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 LE32,
image base `00400000`. Full pseudocode and all assembly pages were inspected.
Six bounded live reads cover all 2246 bytes and match the installed PE.
Three unreachable alignment gaps at `C5AB2B` (5B), `C5ABC8` (8B), and
`C5AFAD` (3B) have no incoming native edge. Their bytes are included in the
native span hash, without copying them into the instruction schedule.

## Interfaces and real provider custody

Native ABI is ECX = actual raw manager, no stack arguments, plain RET;
EBP/EBX/ESI/EDI are preserved. EDX is not an input. Native EAX residue is the
last profile-owner reload, not an asserted semantic return value. The private
MSVC x86 kernel adds a borrowed context stack argument and uses RET4. The
public function is an ordinary C++ void interface. Neither is an original
binary ABI/vtable adapter, hook, owner, activation or runtime binding.

The caller must provide the genuine manager, `DynLiveCrt`,
`DynProfileScopeContext`, canonical feature cell `0109EEA4`, double cell
`00D7A280`, and all five actual name pointers. No native class, replacement
profile, strings, globals, constant defaults, allocators or callback stubs are
invented. The profile context retains the actual publication cell, initialized
9Ch owner, actual 48h nodes, and the existing `AvoidZoneDynHullMemory` services.
The feature cell address remains stable; its value is not frozen. All borrowed
inputs, allocation domains and provider tables must outlive normal execution
and cleanup. Reentrancy/concurrent profile mutation remain outside the existing
profile-provider contract.

Calls compose the existing real implementations:

| Native dependency | Actual source/interface used |
| --- | --- |
| BF55BE, twice | Genuine `DynLiveCrt::operator_new_00bf55be`; live BF55BE is JMP BF681B |
| BF6989, twice | Genuine `DynLiveCrt::free_00bf6989`, same allocator custody |
| C50390, five times | `dyn_profile_node_append_child_00c50390`, actual parent/name/id/memory |
| BF79F0, once | `fill_native_crt_bytes_00bf79f0`, actual feature cell passed by reference |
| BF9410, once | Actual MSVC `__alloca_probe` compiler support |
| C38040, four unwind actions | `cc13_dyn1::profile_scope_leave_00c38040`, real scope and profile context |

The memset bridge forwards the feature **reference**, without reading the word.
The existing qualified provider reads it at its native zero-fill/count gate;
the actual call here is zero fill of 4096 bytes. Its CPU/OS SSE2, DF=0, writable
extent and frame-disjointness requirements still apply. Its real vector engine
is reused, without vendor credit. The real child append composes the existing
C44000 profile constructor and legacy string implementation. No leaf call is
replaced with a returning placeholder or a supplied result.

## Frame, register and call adaptation

The native initial EBP frame, 8-byte alignment, fixed `10A0h` probe and four
register saves remain. Let S be ESP after those saves: the histogram occupies
`S+B0..S+10AF`; original scope/scratch slots retain their exact addresses.
The original saved-ECX slot `S+10B0` now holds the borrowed context. The state
word remains `S+10BC`. The original handler slot is reserved as zero, and its
two FS registration/restoration writes become NOPs. No original image handler
or invented unmapped FH registration is installed. The read of incoming FS[0]
and reserved previous-record slot remain inert native frame bookkeeping.

Each of the ten ordinary native calls gains `PUSH ESP` immediately before its
relocated call, preserving every register and flags. That value is the original
argument-stack address. The shims pass its original arguments to genuine C++
bridges and remove only the added word; the append shim also retains C50390's
original RET8 cleanup. All original caller ADDs remain. This preserves the pair
allocation return spill at `C5AF99`: before caller ADD4, `[ESP+18]` means
`S+14`. No live stack or heap scratch is replaced with a C++ container.

Fourteen absolute profile-owner loads become borrowed-cell loads into their
original destination register. The five name pushes use EDX, which is dead
before the real append call. The half-cell load also uses dead EDX while
preserving the signed CMP flags from C5AD8D through C5AE23. The final owner
load at C5B154 occurs after POP EDI and therefore uses `ESP+10AC`, rather than
the body's `ESP+10B0`. Full CFG stack propagation checks every such epoch,
each original argument area, and the restoring MOV ESP,EBP epilogue.

All ordinary call live-outs require at most EAX; ECX/EDX/arithmetic flags are
dead. The fixed probe must preserve ECX for the original manager input.
Archive inspection of current libcmt/libcmtd/msvcrt/msvcrtd definitions finds
the actual 43 BF9410 bytes exactly, with `__chkstk` at the same definition:
ECX restored, EDX/nonvolatiles/FP untouched and actual stack probing/allocation.
The original `10A0h` request and stack histogram are retained.

## Data, sort, sweep and floating-point epochs

Manager offsets are consumed as raw native storage: input entries +4/+8,
scratch vector +10/+14/+18, pair vector +1C/+20/+24, selected axis +28.
The input count is captured once. Sort and sweep loops use its native signed
tests, while scratch growth/copy and capacity comparisons retain unsigned
tests. Entry stride is eight: key then actual endpoint pointer. All native
uint32 wrapping, signed branches, null computed-destination checks, repeated
field loads and callback-visible publication order remain literal assembly.

Keys read the selected endpoint minimum. In each four-entry group the first
three use FLD/FSTP and the fourth uses MOVSS; tails also use MOVSS. They are not
interchangeable for signaling NaNs, denormals, exception flags or float spills.
Each key then undergoes its exact sign-bit sortable transform. Scratch capacity
is published before new; live size/data are reloaded during copy, old data are
freed before replacement publication, and scratch size becomes captured count.
The fixed interleaved 256-by-four histogram becomes four exclusive-prefix
tables. Four stable low-to-high-byte passes alternate original/scratch storage
and finish in the original input buffer; no comparison sort is substituted.

Sweep conditionally clears a nonzero pair count, tags each endpoint+20 with the
low16 outer index, and accumulates centers and squares. The selected-axis
comparison breaks only on ordered greater candidate minimum. Six full AABB
comparisons use the same ordered-greater rejection; unordered values continue.
Overlaps append the two unsigned pointer-ordered endpoints, with native pair
capacity growth `2*capacity+2`, wrapping byte size, live copy/reloads, old free
before publication, zero computed-destination suppression and count increment.
No missing/empty/malformed guards, rollback or normalization are added.

There are exactly 101 x87 instructions and 27 FSTP sites, all retained with
actual operand directions. Live bytes `DC C9` at C5ADB7 mean
`FMUL ST1,ST0`; `D8 C9` at C5ADC3 means `FMUL ST0,ST1`. The first updates the
retained x sum while preserving half on top; FXCH/FSTP then spill x, and the
remaining half is reused for y/z. Both self-multiply `DC C8` encodings remain.
No division, count scaling, algebraic simplification or approximate C++ double
arithmetic is introduced. The real half cell contains double 0.5.

| FSTP sites | Storage epoch retained |
| --- | --- |
| C5A902 / A92B / A957 | First three float keys per group, before integer transform |
| C5AD95 / AD9F / ADA9 | Three coordinate sums spilled to float before multiplication |
| C5ADBB / ADC5 / ADCD | Three half-scaled centers spilled to float |
| C5ADDB / ADE9 / ADFB | Updated float center sums |
| C5AE0B / AE15 / AE1F | Updated second moments, with native unspilled products/additions |
| C5AE4E / AEA6 / AEB6 / AEC6 / AED5 / AEE5 / AEF5 | Pop the remaining comparison operand after each FCOMIP |
| C5B0D7 / B0E7 / B0F7 | Float `secondMoment - sum*sum` variances, with no count normalization |
| C5B105 / B11F | Final axis-comparison stack pops |

The axis starts at zero and changes only for ordered greater variance. Ties
and unordered comparisons retain the earlier axis. All FCOMIP/FSTP/JA or JBE
sequences, float spills, x87 extended intermediates and status effects remain.
PC/rounding/masks/MXCSR are inherited, without FLDCW, FNCLEX, FINIT or LDMXCSR.
CFG analysis finds at most four additional x87 registers and zero added depth
at every call/return, relative to valid ABI entry. Bridges and cleanup contain
no FP instructions. The public wrapper has two MOVUPS data copies for name
pointers; these perform no FP conversion/arithmetic or FP-state change.

## Profiling and exception boundary

The five cold cache entries are owner+30/34/38/3C/40 for numeric IDs
9/10/11/12/13 and actual names SAPSort/SAPSweep/SAPObjectNum/
SAPPossiblePairNum/SAPPairNum at D79C80/88/94/A4/B8. Every native cache
publication/reload ordering, parent-link write, active-owner publication,
unserialized RDTSC and normal modulo64 elapsed/total and modulo32 count update
remains. Exit pops the current owner through its actual current-node parent,
not a substituted scope node. Physical elapsed values differ with code/frame
and bridge overhead; no cycle-for-cycle timing claim is made.

The live CC8506 handler points to FuncInfo E04498, maxState4, with unwind map
E04478: `{-1,CC84DA}`, `{0,CC84E5}`, `{1,CC84F0}`, `{2,CC84FB}`.
Those funclets tail-jump to C38040 with native scope offsets Sweep `S+80`,
Object `S+68`, PossiblePair `S+50`, Pair `S+98`. Sort remains state -1 and has
no cleanup. Original state activation/demotion instructions remain in place.
Cold entry failure cleans only scopes already active. Pair new/free failure
cleans Pair, PossiblePair, Object, then Sweep.

Each real call bridge uses compiler-generated /EHsc try/catch. It reads the
actual state, demotes it before each action, and calls the real C38040 source
with the still-live original scope storage, then rethrows. This is static
evidence for those four C++ cleanup actions, not original FH3 transport proof.
No manager/allocation rollback or Sort cleanup is invented. Original fault
locations, native SEH/filter/translator behavior, asynchronous/unmasked FP
fault unwinding, stack-overflow transport and secondary cleanup exceptions
are not supplied or runtime-validated.

The actual reused C50390 provider explicitly supports complete **normal**
valid-storage behavior. Its existing documentation excludes native unwind,
OOM and concurrent mutation. Native C50390 has its own CC8383/E041FC state0
partial-node cleanup through CC8378 -> BF65AC; its current real source does
not recreate that cleanup. This caller packet cannot upgrade that dependency
to full failed-construction equivalence. It is an inherited, named exception
closure limit, with no substitute implementation or hidden stub.

## Verification and remaining integration

Strict optimized Win32 compile succeeds with MSVC 19.51.36244.0, C++17,
`/permissive- /EHsc /W4 /WX /fp:strict /O2`. PE/COFF inspection matches all
654 native address rows to 704 emitted kernel instructions: 50 added
instructions, 25 explicitly adapted rows, eleven correct dependency relocations,
52 correct branch destinations, and zero unexplained encoding differences.
All 654 native rows are reachable; all stack/x87 joins agree. Call rows are
independently checked by the repository's actual-call verifier. Exact hashes,
scope map, provider rows and retained local receipts are in the JSON report.

No native fixture, probe executable, game launch, new tests, shared CMake,
ledger, name or Ghidra changes were made. The common build and registration
belong to the integrator. Genuine caller/vtable/profile/allocator binding,
binary ABI replacement, nested failure/SEH equivalence and gameplay remain
open. This is normal-path reconstruction and compile/static proof only,
with no activation or runtime-equivalence credit.
