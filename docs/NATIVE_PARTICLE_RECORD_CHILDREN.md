# Native particle child records

Addresses: `00AFD440`, `00AFE1A0`, `00AFDBF0`.

This packet closes the child-record append boundary used by AFE290. It operates
on the same `NativeParticleModelArraysStorage` actual18h owner, actual2DCh model,
indexed108h records and canonical primary `RandomThreads` domain. The initializer
and interpolation wrapper are concrete. Real CRT services and the captured
definition virtual10 implementation remain required; no successful generation
substitute is supplied. Descriptive names are provisional hypotheses.

| Inclusive span | C++ suffix | Original ABI | Coverage |
| --- | --- | --- | --- |
| AFD440..AFD792,851 bytes | append_native_particle_record_children_00afd440 | ECX actual18h; stack model,definition,parent108h,requested,time,elapsed; EAX actual appends; RET18 | complete through required CRT and virtual dispatch |
| AFE1A0..AFE28D,238 bytes | initialize_native_particle_record_00afe1a0 | ECX actual108h; stack model,definition,time,velocity; RET10 | complete |
| AFDBF0..AFDCEB,252 bytes | interpolate_native_particle_record_00afdbf0 | ECX parent108h; stack position,velocity,matrix,ordinal,fraction; RET14 | complete through required virtual10 |

All final RETs are three bytes. Verified live and installed bytes agree over
all1341 bytes. No missing definition/continuation was found. Ghidra was read
only; the report supplies proposed names, boundaries, original ABIs and evidence
for integrator annotation. The C++ entries add a borrowed EDX access pointer and
four stack bytes. Only incoming stack references shift; native outgoing argument
slots and local float32 spills retain their offsets. These are new integration
interfaces, not demonstrated drop-in binary/object/EH compatibility.

## Initialization and interpolation

AFE1A0 is the producer for the consumed record fields. It clears B8..E4 with
integer stores and E8..104 with MOVSS zero stores, then writes modelA4, start30,
flags40/44, definitionA0, age34 and current raw D7A24C atB4. It copies the input
velocity into54..5C through three forward x87 load/store pairs, then clears
velocity24..2C. Unwritten fields preserve the actual allocation preimage.
Current one is loaded after start30 and before flags40/44; no scalar snapshot
or new reference retain is introduced. Aliased source/destination and signaling
NaNs retain the native copy order and representation effects.

AFDBF0 captures `[parent+A0]`, its current vtable, and slot10 before calling the
exact supplied binding with `(parent,position,velocity,matrix)`. The binding
receives the already captured target in EDX and must dispatch it without
rereading the table/slot. The dynamic implementation is unread; its name is
only a slot/ABI contract, not an inferred generation policy.

After the callback returns, the wrapper rereads parent position0..8 and
previous positionC..14. Its x87 schedule implements the displacement adjustment
to the generated position, preserving every intermediate float32 spill and
forward output store. Callback mutations of parent fields are therefore visible.
The incoming matrix stack word becomes float scratch after dispatch. Ordinal
is never read; fraction is the fifth stack word. The helper retains RET14 rather
than following the decompiler's incomplete three-argument prototype.

## Append behavior and current storage

AFD440 widens the incoming float request to double for the actual CRT floor,
stores its ST0 result to float32, reloads it and calls the real register-input
BF7420 conversion. Signed values below1 return0. Otherwise signed available
capacity `(owner08-owner14)` clamps the request. The routine computes/stores
`1/clampedCount` before its nonpositive-count gate, including the original
zero-capacity floating-point effect; it does not silently skip that divide.

Every iteration checks current count14 against current capacity08, reloads
the current unsigned index byte through owner04, multiplies by108h and adds
current owner0C. It captures the same model's current velocity200..208 and calls
concrete AFE1A0 with the captured definition and incoming time. The sixth
incoming elapsed word is never read anywhere in the body.

The definition20 lifetime curve supplies a random amplitude. The original
UCOMISS/LAHF/parity branch chooses current one for equal-zero amplitude; otherwise
stream0 draws through the same `RandomThreads` object and the code uses signed
FILD, amplitude, current doubleD5DA30 and doubleD7A210 in native order. Curve
tag0/1/other selects its inline value, existing linear kernel or existing cubic
kernel at time0. It stores duration38 and current doubleD7A220/duration at3C.

The interpolation wrapper receives ordinal `i` and float32 `i/clampedCount`,
beginning at0. Its generated position is copied into previousC..14, origin18..20
and position0..8 in the native mixed MOVSS/x87 order; generated velocity48..50
and matrix60..9F are copied afterward. Definition2C/30/38 amplitudes each use
the same zero/NaN/random policy and store random multipliersA8/AC/B0.

Only after generation does the routine increment actual-appended count, ordinal
and the current owner14. A callback can change that count or capacity; the next
iteration sees the native reloads. EAX returns actual appends, distinct from
B04C80's clamped requested count. No independent owner, count, registry or record
publication mechanism is introduced.

## Call evidence and required bindings

Eleven direct call instructions have numeric site/target/containing-function
rows in the report; the verifier passes all11. AFDC12's indirect CALL EAX was
checked against the full AFDBF0 body: AFDC04 loads parentA0, AFDC0A its vtable,
and AFDC0C its10 entry. AFD440's only caller is AFEA8E; AFE1A0 and AFDBF0 each
have only their AFD440 caller atAFD51A/AFD5EC. Stack setup and final RETs were
checked rather than trusting the incomplete pseudocode parameter lists.

| Target | Original ABI / required behavior |
| --- | --- |
| BF85B0 atAFD456 | cdecl double request, ST0 result; caller ADD ESP,8 atAFD463; actual floor with current dispatch/control state and exceptional handling |
| BF7420 atAFD466 | operand already ST0; EAX signed result; actual0109EEA4 dispatch, SSE CVTTSD2SI or x87 fallback, current floating environment |
| BD2FC0 atAFD54B/689/6DE/733 | ECX stream0, EAX random bits; same existing RandomThreads domain, no independent RNG |
| AFFA70/AFFAE0 atAFD591/598 | ECX actual curve, stack float0, ST0 result, RET4; existing complete curve kernels |
| 004134F0 atAFD65E | ECX actual destination matrix, stack source, RET4; existing forward x87 copy |
| AFDC12 virtual10 | captured actual definition entry; ECX definition, four stack words parent/position/velocity/matrix, RET10; required real dispatch |

The CRT conversion's SSE body BF7420..BF743B branches to the separate x87
fallback BF7456. The required binding covers that dispatch; this packet does
not port the CRT or claim that a local cast has the same behavior.

## Verification and limitations

Strict MSVC Win32 `/W4 /WX /fp:strict /Gy /O2` compilation passed. A temporary
fixture at `C:/Users/sqz269/bsp-am-record-children` relocates the verified three
original routines and compares them with the new kernels. Twenty-one combinations
of seven scenarios and three precision/rounding settings compare all record
bytes, return/count values, floating-point status, captured-target traces and
the final canonical random state. Scenarios include early-zero requests,
zero-amplitude and random-amplitude paths, constant/linear/cubic curves,
zero-capacity reciprocal, callback-filled capacity, changed parent definition,
post-callback parent positions, next-record model velocity and current one.
An additional sparse initializer comparison uses a forward alias and signaling
NaN. The native and reconstructed outputs agree.

The fixture's controlled definition callback supplies deterministic generated
data and mutations; its finite floor/ST0-conversion boundaries are shared by
both paths. Those services validate argument composition and instruction effects,
not the actual game definition implementation or native CRT exceptional policy.
Native EH/object compatibility, full successful generation and gameplay are
unvalidated. No permanent tests were added. The integrator owns combined CMake
registration, the standard build and any saved-analysis changes.

## AM combined validation and saved analysis

The combined strict MSVC Win32 build and both existing seeded CTests passed.
Eight call reports check200 direct CALL rows without failures. All seven focused
replays pass within their documented boundaries. Saved names, native signatures,
full body ranges and old-comment preservation were read back; affected exports
were refreshed. The report embeds the earliest annotation preimages and repair
records. Earlier worker pending notes describe isolated snapshots. No original
exception ABI, complete application composition or gameplay claim is added.
