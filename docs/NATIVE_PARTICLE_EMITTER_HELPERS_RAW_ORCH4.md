# Raw particle emitter helpers

## Scope and concrete interfaces

Four complete bodies now have additional raw-domain overloads in the existing
`native_particle_definition_loading.*` files. Existing host overloads and all
derived definition parsers remain unchanged.

| Address | Bytes | Native inputs/result | Raw providers |
| --- | ---: | --- | --- |
| AF9D00 | 399 | ECX definition; stack 4h name, 10h builder, float; RET0C/AL | runtime context, borrowed D7A358 double |
| AFA370 | 365 | ECX definition; stack nonnull C string; RET4 | raw string context |
| AFA4E0 | 365 | ECX definition; stack nonnull C string; RET4 | raw string context |
| AFA650 | 858 | ECX definition; stack 4h pooled suffix; RET4/AL | same raw string context |

`NativeParticleParameterRuntimeRawContext` supplies the actual F8D344 parameter
pool and existing coefficient/integral providers. `NativeStringRawPoolContext`
supplies the actual published pool, manager and return-gate cells. No aggregate
of unused services, fake legacy bindings, allocator callback or replacement
parser is introduced. Each size-based string return invokes the current
419CC0 getter and BD1510 using the native captured buffer/size.

## Base parameter and floating-point schedule

BornRatio calls genuine AFC1B0 and stores its ST0 directly to definition+18.
Lifetime, ParticleEmission, EmitterEmission, Speed, VerticalSpeed,
InheritedSpeed and WindSensitivity call the genuine AFBF60 raw conversion,
then store scalar times current D7A358 into curve+0 and publish the pointer at
definition+20/+24/+28/+2C/+30/+34/+38. Repeated fields overwrite without
disposing the previous parameter. Unknown names return false. There is no
null-result repair, type conversion or additional ownership guard.

The scalar remains the incoming binary32 C++ argument; its address is passed
to the helper, so no unrelated C++ floating-point return is introduced. After
conversion returns, assembly performs FLD32 scalar, FMUL64 scale and FSTP32
directly to curve+0, before pointer publication. The borrowed scale pointer
is retained for the invocation; its volatile pointee is loaded late. This
distinguishes the pointer's lifetime contract from the current numeric value.
The source does not preserve aliases to the original caller's physical stack.

## Enum strings and true unwind

AFA370 and AFA4E0 compare PerSec then PerMeter, storing 0 or 1 at +74 and +78.
Unknown strings leave the field untouched. The first literal uses an actual
8h header initialized to zero, raw 41DD40 resize to six characters, then the
captured data pointer and current length+1 for its copy. Its pointer is captured
before constructing the input string; its length is read again at return.
The input C-string pointer is captured after the first literal construction
and retained for the second comparison, as native EDI is.

The second literal and both input strings use genuine raw 41E870 construction
and 435C40 comparison. Normal input cleanup uses 41DD20's captured data and
length before the getter. Native 41DD20 leaves the header untouched. Neither
input string is unwind-owned. First and second expected strings are owned
only after their construction completes. Normal expected cleanup drops the
state before returning the captured buffer, and does not clear its header.

| Body | FuncInfo / map | States |
| --- | --- | --- |
| AFA370 | DF2F20 / DF2F10 | 0 first expected -> -1; 1 second expected -> -1 |
| AFA4E0 | DF2F54 / DF2F44 | same |
| AFA650 | DF2F78 / DF2F9C | 0 token -> -1; 1 copy -> 0; 2 copy -> -1; 3 token -> -1; 4 copy -> 3; 5 copy -> -1 |

The three compiler handlers CBB030/CBB050/CBB080 remain explicit raw-only
EH evidence. The eight existing unwind funclets are recorded separately.
Normal getter exceptions propagate. True-unwind guards are noexcept, so a
secondary cleanup exception terminates rather than replacing the exception.

## Flag reader

Token0 is regenerated for each comparison. PartEmissionType and
EmitEmissionType take token1, enter token-owned state0 or3, construct a pooled
copy with raw AF5660, then move directly to copy-only state2 or5 BEFORE the
token's normal return. The token header is cleared after capturing the copy's
pointer and before calling its enum setter. After the setter, state becomes
-1 before returning that captured copy pointer. The copy header is not cleared
on this normal path. The unused intermediate map states1/4 are retained in the
true-unwind model, rather than broadening normal ownership.

Looping, FollowDirection and Stops use signed `atol(token1)>0` and store bytes
at +15, +1D and +1C. Looping true additionally reloads definition+10 and writes
byte+66 on that actual resource. False does not clear the resource byte. The
first three keyword temporaries use inline returns without header clearing;
FollowDirection and Stops keywords use AEE2A0. All keyword and numeric-token
temporaries remain unowned by the outer FH3 frame. A recognized enum keyword
returns true even when its enum value is unknown. No short-token validation,
sanitization, rollback, private scratch or bounds policy is added.

## Evidence and verification boundary

Fourteen complete spans match the installed PE and live `bsp.gpr` /
`battlestationspacific.exe`: 1,987 body bytes, all EH code/maps/FuncInfo,
literals and percentage constant. All 668 decoded instructions are listed;
there are no gaps and no repair was needed. The report retains prior names,
comments, documentation and ledger evidence. Correct existing descriptive
names are retained; no library names are changed and this worker makes no
Ghidra mutations. All 85 body calls and eight unwind tails are explicit rows;
three raw compiler-handler tails are distinct from function-owned calls.

Strict Win32 `scripts/build.ps1` and all three existing CTests pass after eight
seed checks. The copied-original/source fixture passes 27 comparisons with
exit code zero: both enum fields and all values, all five flags and unknown
input, resource+66, all eight base properties and unknown input, untouched
owner bytes, string-pool counters, genuine runtime payloads and domain drain.
All four x87 rounding modes pass; multiplier bits round down/up to
3F9E060F/3F9E0610. The late-scale alias case also passes.

The fixture uses genuine raw string and parameter pools and builder providers
from this worktree's completed library. Internal original branches remain
intact; original AFA650 calls the copied original enum bodies. Fifteen numeric
constants are independently verified against PE/live bytes. The relocation
used for the late-scale alias is a deliberate fixture condition, not an
assertion that the original game's constant overlaps parameter storage.

Native getter/allocation failures, native FH3/SEH, hardware faults, secondary
exceptions, installed CRT identity/locale, caller-stack aliases, drop-in
register ABI and gameplay are not established by this packet.
