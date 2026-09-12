# Native particle emission and spawn vectors

Addresses: `00B04C80`, `00AFDAF0`.

B04C80 now executes its complete desired-count clamp, indexed6Ch state emission
loop and actual model-statistic updates. AFDAF0 concretely generates the two
vectors through the captured definition virtual0C and performs the original
x87 position adjustment. The existing actual `NativeParticleEmitterContainer`
from `native_particle_emitter_lifetime.hpp` is reused. No container, particle
state, model reference, owner, or layout is synthesized. Names are descriptive
hypotheses, not recovered symbols.

| Routine, inclusive span | Original ABI | Coverage |
| --- | --- | --- |
| B04C80..B04DE6,359 bytes | ECX30h container; stack(definition,record108h,requested,time,elapsed), RET14h; EAX clamped desired integer | complete through required actual CRT/state services |
| AFDAF0..AFDBE6,247 bytes | ECX108h record; stack(position,direction,unused sequence,fraction), RET10h | complete through required captured definition virtual0C |

B04C80's final RET14h startsB04DE4 and is3 bytes; its early RET14h atB04CB5 is
also3 bytes. AFDAF0's final RET10h startsAFDBE4 and is3 bytes. Both live bodies
already contain their complete flow; no function repairs or Ghidra writes were
performed. Each source entry adds borrowed EDX access and one saved local DWORD.
Original stack arguments, cleanup sizes, x87 instructions and binary32 spills
remain intact. These are new source interfaces, not original binary replacements.

## Actual storage and consumers

The checked B04E30/B053D0 producers establish container08 as the borrowed
emitter, rows0C as8h index rows with signed-compared capacity10, states14 as
uninitialized6Ch slots, and active count1C. B04E30 initializes each row's low
WORD to its state ID. B04C80 uses that WORD at `rows[count]*8` to choose the
state at `states + id*6Ch`. It does not clamp that ID or use states18 capacity.
The source retains native unchecked spans, signed comparisons and wrapping
DWORD ADD/SUB operations. Model statistics are reached through the CURRENT
container08 emitter's model08, rather than container04.

The only B04C80 direct callers are AFE911 inAFE290 and AFF782 inAFF700.
AFE290 acquires the real emitter container throughAFF690 and pushes three
float words plus record and definition. It uses returned EAX in its accumulated
emission subtraction. AFF700 first lazily constructs the same actual30h
container when needed, then forwards all five stack words. Both callees clean
14h. AFDAF0 has only one caller, B04D41, with four words andRET10h. The report
records both external consumer call sites with their actual containing bodies.
The full surrounding consumer routines are separate packets.

## Exact behavior

B04C80 widens requested float32 to a double stack argument for actual BF85B0
floor, spills its ST0 result to float32, reloads it, then invokes actual BF7420
conversion. It keeps EAX and returns0 immediately if that signed integer is
nonpositive. This early path does not access container storage or update stats.
The required conversion preserves the CURRENT0109EEA4 branch: the SSE2 path
spills ST0 to double and usesCVTTSD2SI; the BF7456 path has its own signed64
conversion and low32 result. Neither is replaced with a C++ cast.

For a positive desired integer, clamp it to signed `(capacity10 - count1C)`.
Even if that produces zero or a negative value, compute x87 `1/count`, spill
the reciprocal to float32, multiply it by elapsed and spill that step to
float32. Thus capacity exhaustion can raise the native masked divide-by-zero
status; the source does not move this operation behind the loop condition.

For each iteration, recheck CURRENT active count and CURRENT capacity. Capture
the indexed state address before vector generation. The interpolation fraction
is `(iteration+1)*spilled_reciprocal`, spilled to float32. AFDAF0 captures
CURRENT recordA0 definition and CURRENT vtable0C and dispatches
`virtual0C(record,position,direction)`. Its sequence argument is unused. After
the callback, it reloads record00..14 and adjusts only position, leaving the
callback-generated direction unchanged. In conceptual scalar form the position
is `record0C + (generated - record00) + fraction*(record00 - record0C)`, but
each original binary32 intermediate and x87 instruction order is retained;
the implementation does not fold this expression or use fused arithmetic.

After vector generation, B04C80 captures CURRENT container08 emitter and calls
the required complete B0CA40 initializer for the already captured6Ch state.
Its time is `iteration*spilled_step + input_time`, with the original x87
operation sequence and final float32 spill. After initialization returns,
form CURRENT recordB4 times CURRENT captured-state5C on x87, increment actual
emission count and store the next iteration, spill the product to state5C,
then increment CURRENT container count1C.
Callbacks may change the subsequent capacity, count, emitter or record fields.

Finally inspect CURRENT stack-definition65. If nonzero, add actual emissions
to CURRENT emitter08->model08+1F0. Independently reload emitter/model for+1F4
and then reload again for+1F8. All additions wrap as DWORD operations. Return
the earlier clamped desired integer, even when callback changes ended the loop
early. A positive initial request with count1C already above capacity can
therefore return a negative clamped result while adding0 to statistics.

## Required services and evidence

| Containing routine/site | Native target | Checked contract and cleanup |
| --- | --- | --- |
| B04C80:B04C90 | BF85B0 | actual CRT floor including current mode/error/control effects; double stack argument; ADD ESP8 atB04C9D after float spill/reload |
| B04C80:B04CA0 | BF7420 | actual current conversion mode; consumes ST0, EAX result, RET |
| B04C80:B04D41 | AFDAF0 | concrete vector generation/interpolation; four stack words, RET10h |
| B04C80:B04D74 | B0CA40 | complete state initialization; stack(definition,time,emitter,position,direction,record), RET18h |
| AFDAF0:AFDB0D | captured recordA0 virtual0C | three stack words(record,position,direction), RET0C; dynamic target remains a required dispatch binding |

BF85B0/BF7420 bodies were checked before assigning CRT contracts. B0CA40's
complete body was also checked: it stores state64 definition/state68 emitter,
zeros60 and40, initializes positions/direction and record24..2C, then invokes
CURRENT definition virtual18. If state60 becomes nonnull, it positions the
actual point light using model translation when requested, applies the native
definition8C/D7A238 clamp, gets72B740, enters its actual tracked critical
section, and performs locked B7B090 registration against the actual modelA4.
The required initializer must include all these effects and its own unwind.
No successful empty callback or partial initializer is supplied. Virtual0C's
actual target remains dynamic; its contract is captured dispatch, and the
individual definition implementations are outside this packet.

All four direct calls inside the reconstructed functions and both external
consumer calls have numeric address/native rows. AFDB0D is explicitly marked
indirect; the verifier checks the six direct rows and does not prove this
dynamic binding. There are no fixed absolute game data operands in the new
instruction kernels. Native CRT globals remain the actual service's concern.

Both complete byte spans matched the verified `C:/Users/sqz269/bsp.gpr`
program `/battlestationspacific.exe` and installed PE. SHA256 values and live
prototypes are in `reports/native_particle_emission_spawn.json`; the binary
hash is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Validation and limits

Strict production MSVC Win32 `/std:c++20 /EHsc /MD /W4 /WX /O2 /Gy /fp:strict
/permissive-` compilation passed. One local differential probe executed both
relocated original byte bodies against the new source. All24 comparisons
passed over24/53/64-bit x87 precision modes, checking complete state bytes,
callback arguments, returned count, statistics, FP status and a preexisting
lower x87 stack value. Cases covered early floor rejection, normal and clamped
emission, exhausted/overfull capacities, NaN conversion, callback changes to
capacity/definition flags, and current emitter/record/vtable reloads. One case
confirmed desired return3 with only1 actual emission.

The probe redirects native CRT calls to explicit fixture floor/SSE2 conversion
services and state initialization/virtual0C to observable fixture callbacks.
It executes actual AFDAF0 interpolation, not a fixture version of that math.
These checks do not validate BF7420's alternate mode, the game's CRT, actual
definition implementations, B0CA40 point-light side effects, native exception
unwind, full model execution, or gameplay. Artifacts remain under
`C:/Users/sqz269/bsp-am-spawn`; no permanent test suite was added. The primary
integrator performs the separate combined repository build.
