# Registered type 4 effect behavior

Addresses: `00872020`, `00872060`, `00872790`, `00BA9820`, `00BA9900`,
`00BA9920`, `00BA9940`, `00BA99C0`, `00BA99E0`, `00BA9A00`, `00BA9A20`,
`00BA9A40`, `00BA9A60`, `00BA9A70`, `00BA9A90`.

These entries implement the complete type 4 completion, deactivation and update
control flow over the existing `NativeRegisteredType4EffectStorage`. The update
uses its real borrowed `PointEffectInstanceStorage`, current canonical node110,
and the existing event/reference interfaces. The actual prefix04 remains the
sole event reference count. No family companion, alternate point/node owner,
default virtual implementation or new ownership domain is supplied.

| Routine | Inclusive native body | Original ABI | Coverage |
|---|---|---|---|
| completion872020 | 872020..87205B, 60 bytes | ECX event, AL result; RET at28/55/5B | complete |
| deactivation872060 | 872060..872073, 20 bytes | ECX event, RET; tail JMP BA9820 | complete |
| update872790 | 872790..872BC0, 1073 bytes | ECX event, stack(float delta, reference), 872BBE RET8 | complete control/state behavior with required real bindings below |
| tracer deactivationBA9820 | BA9820..BA984E, 47 bytes | ECX actual tracer, RET | complete |
| reciprocalBA9900 | BA9900..BA9910, 17 bytes | ECX tracer, float, BA990E RET4 | complete |
| texture rateBA9920 | BA9920..BA9930, 17 bytes | ECX tracer, float, BA992E RET4 | complete |
| alpha scaleBA9940 | BA9940..BA9950, 17 bytes | ECX tracer, float, BA994E RET4 | complete |
| width scaleBA99C0 | BA99C0..BA99D0, 17 bytes | ECX tracer, float, BA99CE RET4 | complete |
| width scalerBA99E0 | BA99E0..BA99F0, 17 bytes | ECX tracer, float, BA99EE RET4 | complete |
| width offsetBA9A00 | BA9A00..BA9A10, 17 bytes | ECX tracer, float, BA9A0E RET4 | complete |
| bone lengthBA9A20 | BA9A20..BA9A30, 17 bytes | ECX tracer, float, BA9A2E RET4 | complete |
| height scaleBA9A40 | BA9A40..BA9A50, 17 bytes | ECX tracer, float, BA9A4E RET4 | complete |
| local spaceBA9A60 | BA9A60..BA9A6C, 13 bytes | ECX tracer, low argument byte, BA9A6A RET4 | complete |
| fade inBA9A70 | BA9A70..BA9A80, 17 bytes | ECX tracer, float, BA9A7E RET4 | complete |
| width endpointsBA9A90 | BA9A90..BA9AAE, 31 bytes | ECX tracer, TWO float words, BA9AAC RET8 | complete |

Descriptive names are hypotheses derived from the parser and callee bodies.
New C++ interfaces are not original MSVC vtable, binary-call, or SEH replacements.
Unimplemented required callees are not counted as reconstructed bodies.

## Producer and storage evidence

`8744A0` produces the actual event3Ch and `868D70` allocates it. The established
tail has pending1C, direction20 (untouched by constructor, written on creation),
age24, completion delay28, signed start delay2C, stop delay30, actual tracer34,
and separately owned38. This packet uses the canonical header's existing
neutral field names rather than declaring a conflicting owner.

The `86D180` Lua producer writes ShowForward20, ShowBackwards24,
TextureRate44, AlphaScale58, WidthScale5C, HeightScale60, FadeIn74,
StartTime78, StartSpeed7C, StopTime80, StopSpeed84, WidthSpeed curve8C,
texture98/9C, LocalSpace byteA0, generated tracer definitionA4,
WidthOffsetA8, WidthScalerAC, BoneLengthB0 and LengthB8. The FitToWater branch
can replace60 from DisplacementScale and deriveB0 from Length; the update
uses current words rather than cached parser values. The curve creation call
at86DC46 and publication at86DC82 establish8C. The subject fields50/54/58 and
vector68/6C/70 belong to the existing actual114h point producer and advancement.

`BAD6F0..BADC6D` calls the generated-model constructor, publishes `D63FA0`,
uses literal `SkinedWaterTracer`, creates native material/geometry resources,
sets byte200=1 and bytes201/202=0, and initializes linked1B8=null. The eleven
direct setters are the producers of the consumed values at184/188,1D8,
1E0..1FC and210 and establish their complete write footprints. BA9820 writes
byte200=0, float7A8=1, then reloads1B8 and writes linked24=0.1 if nonnull.
7A8 is also consumed/updated by BAABB0/BAA510. `RegisteredType4TracerView`
borrows the complete existing owner; it does not construct a7ACh byte surrogate.

## Behavior and required calls

Completion returns false while pending1C is nonzero. Otherwise it returns true
when delay28 exceeds definition80, or when tracer34 exists, active0C is zero,
and tracer byte201 is zero. Equality alone does not complete. Deactivation
clears1C and deactivates current34 when present; it does not clear0C, unlink,
release, unregister, or clear34/38.

Update always advances age24, captures the subject vector68 and speed54, and
then evaluates its pending phase. Forward/backward gates accumulate positive
or negative start delay2C; crossing directions resets the old sign first.
StartSpeed7C suppresses accumulation. Once abs(delay2C)>StartTime78, it writes
direction20 and clears1C before entering the actual F87684 critical section and
incrementing actual F8769C. Successful construction publishes current34 before
decrementing depth and leaving the section. It then applies all setters and
the two curve results, in native order.

| Function / site | Native target | Contract grounded in callee |
|---|---|---|
| 872790 / 8728B2 | imported EnterCriticalSection viaCE2218 | actual F87684 OS operation; depth increment follows |
| 872790 / 8728C8 | BAC660 | overwrite incomingECX=7AC with pool0109049C; BAC4A0 returns actual slot, stride7B0 and trailing pool ID7AC |
| 872790 / 87290D | BAD6F0 | actual raw slot; five borrowed args: current game19F0.A8, game19EC, definition98,9C,A4; BADC6B RET14 |
| 872790 / 87292F | imported LeaveCriticalSection viaCE2210 | depth decremented and34 published first |
| 872790 / 87295C | BA9900 | x87 reciprocal into actual1EC; argument is Length/(1/subject50)+0.5, with native spill |
| 872790 / 87296C | BA9A60 | byteA0 to byte202 |
| 872790 / 87297B | BA9920 | float44 to1F0 |
| 872790 / 87298A | BA9940 | float58 to1F4 |
| 872790 / 872999 | BA99C0 | float5C to1E0 |
| 872790 / 8729AB | BA99E0 | floatAC to1E4 |
| 872790 / 8729BD | BA9A00 | floatA8 to1E8 |
| 872790 / 8729CF | BA9A20 | floatB0 to1D8 |
| 872790 / 8729DE | BA9A40 | float60 to1FC |
| 872790 / 8729ED | BA9A70 | float74 to210 |
| 872790 / 872A05,872A1A | captured curve's current virtual08 | sample1 then sample0, each one float/RET4; slot reloaded on same captured owner |
| 872790 / 872A23 | BA9A90 | sample0 to184, previously spilled sample1 to188; RET8 |
| 872790 / 872A90,872AB7 | B6DB70 | canonical current subject node world refresh, only if actual flag5C bit2 clear |
| 872790 / 872AF8 | BAABB0 | actual tracer, second world matrix, age24, first world's translation, captured vector68, min(subject50*speed,1), captured subject58; BAB3E4 RET18 |
| 872790 / 872B61,872B75 | event's current virtual30 | capture slot, clear actual0C, invoke via canonical event/child interface |
| 872790 / 872B82 | BAA510 | live tracer flags200/201/202, actual1B8 clock/fade, or7A8; AL predicate |
| 872060 / 87206E | tail JMP BA9820 | current actual34 after1C store |
| C96170 / C96176 | unwind tail JMP BAC2B0 | actual raw allocation returned to pool0109049C via BABF70 |

All saved direct xrefs of the eleven setters, BA9820, BAABB0 and BAA510 are
the sites above. Setter bodies were inspected through their RET cleanup.
BAABB0 consumes six words even though its current body never reads argument1
(matrix) or argument6 (subject58); neither argument is dropped from the host
interface. It reads translation/vector and maintains native geometry, bounds,
visibility, segment and fade state. BAA510 includes BF7030/4155B0 clock/fade
calculation; it is not replaced by a boolean flag shortcut.

The curve producer869E40 allocates1Ch and callsBACAA0, which publishesD63FFC.
Its slot08 isBA9DA0. Read-only disk/live inspection established complete
BA9DA0..BA9E4A: cached piecewise linear interpolation with endpoint handling;
every return consumes one float word. At872A0C, `SUB ESP,8` reserves both
the second sample argument and the saved first result. Second virtual08 RET4
leaves that saved result for BA9A90's second argument. The immediate single
PUSH before BA9A90 does not imply a one-word setter.

After pending1C clears, update caps subject50*captured speed at1, refreshes and
copies current node world twice with DWORD semantics, reloading subject110
between copies, and supplies the first translation and second full matrix.
It advances/resets stop delay30 only when active0C is exactly1. Timeout or
direction change captures current30, clears0C and invokes. Afterwards current34
is reloaded before the predicate; a nonzero predicate clears completion delay28,
otherwise delta is added. Native NaN branch behavior and x87 arithmetic spills
are preserved for the reconstructed comparisons/arithmetic. This is not a
claim about exact global x87 status flags across arbitrary host bindings.

The BAABB0 call explicitly marshals all three scalar words through x87
FLD/FSTP32: captured subject58 (`872ABC/872ADC`), cached scaled speed
(`872AE0/872AE4`), then current event24 (`872AE8/872AF1`). Current tracer34 is
reloaded afterwards at872AF5. This ordering is explicit in the C++ statements,
so argument evaluation order cannot move the current age/tracer loads ahead
of the spills. In particular, an earlier constructor/curve callback can replace
age24 with a signaling NaN after its entry-time addition; the native argument
spill quiets that word before passing it to the required update binding.

Integration review corrected the original C++ raw age24 argument and made the
existing subject58 spill plus scaled-speed spill explicit. Strict MSVC assembly
now contains the three ordered FLD/FSTP pairs before the current34 load.
The ignored fixture includes the production translation unit solely to access
its internal spill helper without a public test hook, and checks
`7F812345 -> 7FC12345` with the input word unchanged. The translation unit is
also compiled independently. This helper check does not validate the nonnull
actual node/tracer update path or introduce a successful tracer implementation.

## Exception and integration boundaries

The update's native handlerC9617B selects EH infoDC8190. Its one unwind entry
DC8188 is state0 -> C96170 -> BAC2B0, using saved raw allocation at EBP-9C.
The state is armed afterBAC660 returns and disarmed on the successful leave
path. Constructor failure first completes BAD6F0's own member unwind, then
returns the raw pool slot. It does **not** restore1C/direction20, publish34,
decrement F8769C or leave F87684. The C++ catch preserves that boundary;
allocator failure is outside the state0 cleanup. Original SEH dispatch is
not reconstructed or tested here.

`RegisteredType4TracerRuntime` requires real pool allocation/return,
constructor, current curve, update and predicate bindings. Construction must
install the canonical actual node, generated-model lifetime and actual-owner
associations including terminal virtual18/00, using the same owner/counts.
No raw-C allocation, plain174h node, generated-owner substitute or success
stub is supplied. Family frame dispatch and CMake composition remain the
integrator's work; these functions do not make an incomplete family runnable.

## Verification

Strict standalone MSVC Win32 `/std:c++17 /MD /O2 /W4 /WX /EHsc /fp:strict`
compilation passes. `scripts/build.ps1` passes for the existing repository;
this worker does not edit CMake, so the new translation unit is verified by
the explicit strict compile and linked ignored fixture. After seed verification,
both reconstructed_math and native_math_differential pass.

`local/type4_probe_aj.cpp` executes original60-byte completion,20+47-byte
deactivation and all eleven setter bodies, comparing complete actual3Ch/7ACh
and linked storage. It executes the original full1073-byte update on13 selected
paths that do not construct/update a tracer, covering signed start delay,
pending/inactive flags, quiet NaN, negative delta, stop delay, direction and
current30 deactivation. Full actual3Ch equality and native RET cleanup pass.
Original code is copied into isolated executable memory; fixed constant
operands and the deactivation tail jump are relocated. Its invocation wrapper
is noinline because the original code clobbers XMM registers invisible to an
inlined MSVC assembly CALL. No original game globals or startup are executed.

The same ignored fixture injects a throw through the required constructor
binding, after a fixture raw7B0 allocation boundary. No successful tracer owner
is created. It verifies all five captured arguments, raw-slot return before
propagation, unchanged actual04, no34 publication, cleared1C/set direction20,
and the real OS critical section still held with actual depth1. Fixture cleanup
releases that hold only after checking it. This proves the reconstructed
caller's exception ordering, not the real allocator, BAD6F0 member unwind,
pool return internals, or original SEH.

`local/type4_body_evidence_aj.json` records16 disk spans; live/disk SHA256
matches are checked for update1073, completion60 and consumed curve171 bytes.
The ignored fixture log is `local/type4_probe_aj.log`; the repository build
log is `local/type4_build_aj.log`. Real tracer construction/update/predicate,
nonnull current110 update, original EH and game behavior remain unvalidated.
No reachable family host is installed, so no runtime frame claim is made.

Ghidra work was read-only against `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, verified by the repository client's target check
for every query batch. No names/comments/signatures/definitions/saves changed.
Missing live definitions are872020..87205B (last RET87205B, length1) and
consumed curveBA9DA0..BA9E4A (last RET4 BA9E48, length3). The integrator owns
any annotation/definition repair and refreshed exports.

## AJ combined integration verification

The source is registered in bsp_core. The combined strict MSVC Win32 build and
both existing CTests passed. The report records the focused fixture replay, exact
call/tail checks, saved Ghidra name/signature preimages and comment readback.
Required external runtime bindings, original exception ABI and gameplay remain
limited as described above; this integration does not extend the fixture coverage.
