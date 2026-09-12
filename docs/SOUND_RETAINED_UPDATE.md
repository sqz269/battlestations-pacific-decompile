# Retained-source channel updates

Addresses: 00A7B520, 00A7B5D0, 00A7B690.

The three retained-source channel tables dispatch vslot30 to these methods.
The C++ functions share the existing `SoundChannelInstance`,
`SoundInstanceContext` and actual sample-options getter. Source fields are
borrowed by reference; this module declares no competing source-record or
derived-channel layout. Descriptive names are hypotheses.

| Routine | Vtable and slot address | Behavior | Coverage |
| --- | --- | --- | --- |
| 00A7B520 | D5AD58, D5AD88 | Gated, clamped ramp from fields68/6C | Complete normal-return behavior in the valid domain below |
| 00A7B5D0 | D5ADA0, D5ADD0 | Same ramp, multiplied by field70 with an intermediate float spill | Complete normal-return behavior in the valid domain below |
| 00A7B690 | D5ADE8, D5AE18 | Unit gain for nonpositive or unordered coordinate, zero for positive | Complete normal-return behavior in the valid domain below |

The verified table block is D5AD58..D5AE28, end exclusive. The initial saved
analysis had functions for B520 and B5D0 but none for B690. B690 was recovered
from disk instructions and matched against saved analysis bytes through
B705 `RET8`, length3, end-exclusive B708. Its function-definition request and
current verification status are recorded in `reports/sound_retained_update.json`.

## Native inputs and publication

All three methods take ECX=this and two stack words: float dt and a listener
word. Each ends in `RET8`. ESI is assigned from ECX once and preserved across
both calls; every instance access uses ESI. EDX has no input provenance and is
unused by the already reconstructed A7AF10. These are new C++ APIs rather than
native layout or ABI replacements.

The current owner is loaded from F8BBD8 at entry. Native owner+F8 maps to the
existing `owner.listener.transform_c4[13]`. This follows the producer:
A7E817 sets ESI=owner+A4, A7E828 passes ESI+20 to the 4134F0 matrix copy at
A7E830, and A7E85E submits ESI+54 (owner+F8) as the middle listener-position
component. No independent height or velocity interpretation is introduced.
The listener stack word is only forwarded to A7AF10; it does not select the
owner coordinate read here.

For B520/B5D0, an SSE `COMISS current,+0; JBE` bypasses field68/6C reads for
nonpositive **or unordered** current, leaving positive-zero gain. Otherwise
the lower endpoint first passes through `FLD/FSTP float`. Native x87 computes
`(current-lower)/(upper-lower)` without intermediate subtraction spills, then
stores the ratio to float. The following `FLD1; FLDZ; FSUB; FMUL ratio; FADDP`
identity interpolation also stores to float. This step is retained because
rounding mode, signed zero and NaN behavior need not permit algebraic removal.
The lower `FCOMIP +0,gain; JA` zeroes ordered negative values; the upper SSE
`COMISS gain,1; JBE` retains unordered values. A NaN produced by the ratio
therefore propagates, despite the decompiler's misleading `0 <= ratio` test.
The upper constant at D7A24C is the verified float bit pattern 3F800000.

B5D0 then loads field70, multiplies by gain and **stores to float at B657
before retrieving sample options**. The final product is options_volume times
that stored value; the decompiler's left-associated product loses this rounding
point. B690 selects zero only for ordered positive coordinate and one for
everything else, including NaN. Its following lower and upper SSE clamp is
preserved literally.

Each method calls A81860 on actual instance.sample_4c; its complete body is
`LEA EAX,[ECX+8]; RET`. The float at options+4 is the existing sample volume.
An x87 product writes instance.volume_24, dirty_14 becomes1, and the original
dt passes through `FLD/FSTP float` before the unconditional A7AF10 call. A
stopped instance still receives the volume/dirty stores even though A7AF10
immediately returns. There is no additional unresolved host method here.

| Caller | Call site | Callee | Contract |
| --- | --- | --- | --- |
| A7B520 | A7B5A0 | A81860 | Return actual sample+8 |
| A7B520 | A7B5C2 | A7AF10 | Existing channel update; forwarded dt/listener |
| A7B5D0 | A7B65B | A81860 | Return actual sample+8 after scaled gain spill |
| A7B5D0 | A7B67D | A7AF10 | Existing channel update; forwarded dt/listener |
| A7B690 | A7B6DC | A81860 | Return actual sample+8 |
| A7B690 | A7B6FE | A7AF10 | Existing channel update; forwarded dt/listener |

## Valid domain and validation

The current owner, instance, actual sample options and referenced source fields
must remain live; calls are serialized and inherited A7AF10 host contracts
apply. Floating exceptions are masked. The inline x87 operations preserve the
calling precision/rounding mode and explicit native float spills; SSE branches
preserve unordered handling. Exception delivery and exact FP status-register
identity are outside this projection. Native object ABI, audible output and
gameplay behavior remain unvalidated.

All three full method spans and the supporting fixture's A81860/A7AF10 spans
match the current disk image and saved-analysis bytes; the report carries
their SHA256 evidence. The MSVC Win32 Release build passed, along with both
existing CTests after `verify-seeds`.

The focused local fixture (`local/run_retained_probe.ps1`) passed 5,040
bit-identical volume comparisons: three methods, 140 deterministic inputs,
three x87 precision settings and four rounding modes. Inputs include ordered
finite values, reversed/equal endpoints, infinities, denormals, signed zeros,
quiet/signaling NaNs and deterministic arbitrary float bits. It also checks
dirty14=1 and unchanged update count. The reference executes relocated original
method bytes, the original A81860 getter and the original stopped A7AF10 branch;
unused host callbacks abort if reached. This is numerical/stopped-path evidence,
not an audio or gameplay run. The probe embeds its manifest and exits zero.

The initial call-site verifier accepts all four B520/B5D0 rows. Its two B690
rows await the integrator's missing-function definition; the report retains
that explicit boundary instead of attributing those calls to B5D0.
