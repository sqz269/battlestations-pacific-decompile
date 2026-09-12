# Native particle record update

Addresses: `00AFCF50`, `00AFD410`, `00AFD7A0`, `00AFE030`, `00AFE290`.

The five instruction kernels operate on the existing actual108h records, the
canonical `NativeParticleModelArraysStorage` actual18h owner and its same actual
model. No duplicate record/owner schema or allocation domain is introduced.
Descriptive C++ names are provisional hypotheses. The simulation is complete
through two required real emission bindings; their effects are never replaced
with successful no-op work.

| Inclusive span | Entry suffix | Original ABI | Coverage |
| --- | --- | --- | --- |
| AFCF50..AFD0E6,407 bytes | copy_native_particle_record_00afcf50 | ECX destination108h; stack source108h; EAX destination; RET4 | complete |
| AFD410..AFD43C,45 bytes | append_native_particle_record_00afd410 | ECX actual18h; stack source108h; RET4 | complete |
| AFD7A0..AFD81D,126 bytes | update_native_particle_records_00afd7a0 | ECX actual18h; stack time/mode; EAX signed count14; RET8 | complete |
| AFE030..AFE07A,75 bytes | copy_native_particle_model_position_00afe030 | ECX same actual model; stack destination; EAX destination; RET4 | complete |
| AFE290..AFEAD6,2119 bytes | update_native_particle_record_00afe290 | ECX actual108h; stack time/step/mode; AL alive; RET0C | complete through required emission bindings |

Every final RET is three bytes. The report records full boundaries, byte hashes,
instruction counts and live/installed equality for all2772 bytes. No missing
definition or omitted continuation was found. Ghidra was read only; proposed
names and eventual saved-analysis annotation remain integrator work.

The two update entries add borrowed per-call access in EDX and four stack bytes.
Original locals, outgoing arguments and scalar spills retain their offsets;
only incoming arguments shift. Other entries have an unused EDX slot to keep
the native stack layout. Added loads use MOV/PUSH/POP and introduce no float
spill or arithmetic. The look-at adapter additionally receives access in EAX
and borrows the original up triple. These are new integration interfaces,
not established drop-in binary/object/EH compatibility.

## Actual storage and behavior

AFCF50 copies the whole108h payload, including the final float at104. Almost
all words pass through forward x87 load/store pairs;40/44 and A0/A4 use raw
integer copies. The matrix60..9F uses existing004134F0. No reference is retained.
The two-iteration B8..E7 loop preserves its interleaved reads/stores and source
minus destination addressing. Overlapping objects and signaling NaNs therefore
must not be implemented with memcpy. A model initializer's virtual08 temporary
must support all108h bytes, correcting earlier minimum9F consumer descriptions.

AFD410 compares signed current owner14 with byte-array capacity08. It reads
the unsigned index byte from current owner04, multiplies by108h, adds current
owner0C, copies, then increments the reloaded owner14. Capacity failure is a
native no-op; negative counts and malformed indices are not newly sanitized.
AFD7A0 reloads owner00 model and model1A0 on every iteration. A dead record swaps
its index byte with the current final active byte, reloads owner04 for the
second store, decrements the iteration index when needed, and decrements the
current count14. The moved record is visited; appended records can be observed
through the native current count reloads. The model and arrays remain canonical.

AFE030 tests the same model1B0: nonzero copies localE0 via B6E0A0; otherwise it
refreshes world state if flags5C lacks bit2 and copies world120 forwards. No
independent transform cache is created.

AFE290 subtracts record30 from incoming time, spills to float32, stores age34,
and compares age against duration38. Expiry returns AL0 unless definitionA0+15
requests restart, which sets start=time and age/previous-age=0. Upper EAX is
unspecified. It computes elapsed age, copies position0..8 into previousC..14,
and clamps the float32 age*3C curve parameter using the original ordered/NaN
branches and current doubleD7A220/floatCE3D08 loads.

For definition70 nonzero, the routine evaluates actual curves2C/30/38 and
optional34, combines direction48 and54, recordAC/B0 scales, the current
D7A258 transverse term and three separately loaded E13028 vector words, then
advances position by step. Curve tags0/1/other select inline value/linear/cubic.
Every x87 lifetime, float32 rounding point, SSE copy and arithmetic operand
order remains in the listing. For70 zero, model inactivity and definition1C
can kill the record; otherwise actual model position and velocity200 are copied.

Definition1D requests a displacement-oriented matrix when vector length exceeds
current D7A268. The native look-at call receives zero eye, displacement target
and the permuted up triple(z,x,y), followed by matrix copy, explicit transposes,
self-copy and row swaps. The current CRT access and D7A24C flow through the
existing camera math implementation without replacing its runtime policy.

Nonzero mode runs the emitter stage: curve24, distance-vs-step selection from
definition74, original zero/NaN denominator6C gate, definition54 rows/count68,
and per-record B8 accumulators. Values reaching1 acquire the same emitter's
container and call B04C80. The child stage always runs: curve28, definition78,
denominator50, definition3C rows/count4C and E8 accumulators call AFD440 on the
same model190 owner. Both stages subtract the signed returned count or clear
the accumulator according to the current mode field, reloading definition and
counts after calls. No count clamp or emission-success substitute is added.

## Direct calls and required contracts

All27 direct call instructions have numeric site/target/containing-function rows
in the report. Callers are AFCF50<-AFD410, AFD410/AFD7A0<-AF6DD0,
AFE290<-AFD7A0 and AFE030<-AFE290. Original caller stack setup and final RETs
establish argument counts where pseudocode omitted parameters.

| Target | Original ABI and binding |
| --- | --- |
| 004134F0 | ECX destination, stack source, RET4; existing forward matrix-copy kernel |
| B6E0A0/B6DB70 | actual canonical model local-position copy / world refresh |
| AFFA70/AFFAE0 | ECX curve, stack time, ST0 float, RET4; existing full curve kernels |
| 00419440 | ECX vector, ST0 float, RET; existing native length with required current CRT access |
| BF7030 | operand already ST0, sqrt result ST0; existing CRT kernel, current dispatch/handler preserved |
| B63F10 | ECX matrix, EDX eye, stack target/up.x/up.y/up.z, RET10; existing full look-at kernel and current one |
| AFF690 | ECX existing emitter; RET; existing actual lazy container acquisition; no substitute container |
| B04C80 atAFE911 | ECX actual30h container; stack definition,108h record,requested,time,elapsed; RET14; required real emission body |
| AFD440 atAFEA8E | ECX actual18h owner; stack same model,definition,parent108h,requested,time,elapsed; RET18; required real child-record append |

B04C80..B04DE6 was read through its final RET14: real floor and ST0 conversion
produce a signed request, capacity clamps it, AFDAF0 interpolates, B0CA40 builds
actual6Ch states, state5C scales by parentB4, and actual model counters update.
Its EAX is the clamped requested count, which can differ from the emitted count
when a later capacity check breaks. AFD440..AFD792 was read through RET18: real
floor/conversion, AFE1A0 initialization, random/curves and AFDBF0 interpolation
populate indexed108h slots and increment owner14; EAX counts actual appends.
These bindings must preserve real current-target behavior and side effects.
Their full implementations are outside this packet, and are not CRT ports.

## Verification and limits

Strict MSVC Win32 `/W4 /WX /fp:strict /Gy /O2` compilation passed. The call-site
verifier passed27/27. A temporary original-byte fixture at
`C:/Users/sqz269/bsp-am-record-update` mapped the verified five original routines,
relocated absolute data references and rebound their external call boundaries.
It compared every record/index byte, return value, floating-point status and
boundary argument trace across40 combinations of ten scenarios and four
x87 precision/rounding settings. Scenarios cover expiry/restart, local/cached
world position, inactivity, velocity with a changed current transverse global,
constant/linear/cubic curves, orientation and both emission call boundaries.
It also covers self/forward-overlap/nonoverlap copying with a signaling NaN,
indexed swap removal, append selection and a full-owner no-op.

The fixture found an adapter error that shifted an outgoing ESP+4 step slot;
the final source preserves that outgoing slot and shifts only incoming owner
arguments. The corrected object and probe passed again. The fixture's two
controlled emission callbacks validate boundary arguments and returned-count
arithmetic; they do not validate the actual downstream emitters. Reused math
and container helpers are shared between the two paths. Original CRT exceptional
handlers, dirty-world refresh, native EH and gameplay are unvalidated here.
No permanent test suite was added. The integrator owns the combined standard
build/CMake registration and saved-analysis changes; their status is separate.

## Correction from AM concrete simulation integration

Model AF6DD0 now invokes concrete AFD410, AFF640 and AFD7A0. Record AFE290
now directly invokes B04C80 and AFD440 with typed borrowed application access.
Those kernels include AFDAF0 and AFE1A0/AFDBF0 respectively; real current
definition virtual generation, state B0CA40 initialization and CRT services
remain required. Existing assembly access offsets and original stack words
are preserved. Definition virtual08 must return readable storage through+107,
because AFCF50 copies the final float at+104; the earlier+9F extent was too small.
See NATIVE_PARTICLE_EMITTER_UPDATE.md, NATIVE_PARTICLE_RECORD_UPDATE.md,
NATIVE_PARTICLE_EMISSION_SPAWN.md and NATIVE_PARTICLE_RECORD_CHILDREN.md.
Earlier isolated fixture service descriptions are historical, not final wiring.
Full application composition, native exception ABI and gameplay remain unproven.

## AM combined validation and saved analysis

The combined strict MSVC Win32 build and both existing seeded CTests passed.
Eight call reports check200 direct CALL rows without failures. All seven focused
replays pass within their documented boundaries. Saved names, native signatures,
full body ranges and old-comment preservation were read back; affected exports
were refreshed. The report embeds the earliest annotation preimages and repair
records. Earlier worker pending notes describe isolated snapshots. No original
exception ABI, complete application composition or gameplay claim is added.
