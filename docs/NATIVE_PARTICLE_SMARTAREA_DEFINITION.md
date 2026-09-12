# Native smart-area definition generation

Addresses: `00B01CE0`, `00B01EC0`, `00B01E20`.

These three current `00D5DE48` vtable methods operate on the actual definition,
108h record and canonical model. They are complete reconstructions through the
required real allocation, captured-definition and CRT services. Names describe
observed behavior and remain provisional. The added EDX access and C++ allocation
unwind are new integration interfaces, without native object/EH ABI or gameplay
validation.

| Routine | Current slot | Original ABI | Coverage |
| --- | --- | --- | --- |
| B01CE0..B01D50 | +08 | ECX definition; stack(model,time); EAX allocation/null; RET8 | complete |
| B01EC0..B0220E | +0C | ECX definition; stack(parent108h,position,velocity); RET0C | complete |
| B01E20..B01E98 | +10 | ECX definition; stack(parent108h,position,velocity,matrix); RET10 | complete |

The verified saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, bridge8089. All three full spans match the installed
PE byte for byte, including each final three-byte RET. Hashes, instruction counts,
call sites and final boundaries are in the accompanying report. The integrator
created the missing B01E20 function after this worker supplied its 121-byte span;
this worker made no Ghidra mutations.

## Producer and dispatch evidence

AF9FB0 selects `SmartAreaEmitter`, allocates90h and invokes B01CB0, which invokes
the AFA280 base constructor and publishes D5DE48. B02210 writes the derived curve
pointers: `EmittedSpeed` at80h/B0257E, `Radius` at84h/B02608, `RadiusSpeed` at88h/
B02640 and `RadiusAngle` at8Ch/B026A0. That loader remains an external producer;
this packet creates no competing definition layout. AFE0A0 and AFE1A0 produce
the actual record's A0 definition/A4 model words. Record60 is its64-byte matrix;
model1B0 selects local+B0 versus current world+F0. Existing raw-node helpers use
the same canonical parent30 and flags5C storage.

B01CE0 calls the real allocator with108h (`B01D03 ADD ESP,4`), then performs the
native x87 time load/spill before the shared AFE0A0 sparse constructor. Allocation
null returns null without construction. Constructor failure frees the same raw
allocation through the required matching CRT domain and rethrows. The native
allocation cleanup is CBB4D0: its CALL free is CBB4D4, followed by disk-verified
CBB4D9 POP ECX and CBB4DA RET. Its existing Ghidra body stops at CBB4D8; that helper
flow gap was reported to the integrator and was not silently repaired here.

B01EC0 spills `record3C * record34` to binary32, then selects that value only when
the current D7A220 comparison is ordered and greater; otherwise it loads CE3D08.
It evaluates Radius at that selected value, draws twice from the existing
primary RandomThreads domain, and evaluates RadiusAngle using the first draw
times current D7A220. Native FCOS/FSIN instructions, repeated angle spills and
the CF1448 division/CE3828 multiplication remain in their original order.

The velocity direction is transformed by parent60 when the record's current
definition70 is nonzero; otherwise model1B0 selects localB0 or refreshed worldF0.
Only velocity is transformed. Position adds the native horizontal radial offset
to the parent position, using Radius and CRT sqrt(first draw). RadiusSpeed uses
the same first draw, and EmittedSpeed uses the previously selected time value.
All three curve modes retain the original constant/linear/cubic branching and
float32 spills; no clamping, finite-value repair or substitute geometry is added.

B01E20 captures actual definition/current vtable+0C before dispatch. Its required
callback receives that exact captured target in EDX and the original three stack
words. After return, it reloads the record's current definition70/modelA4 and
selects parent60/localB0/worldF0 again, then copies sixteen ordered x87 words.
Callback mutations can therefore change the copied matrix independently of the
matrix used while generating velocity. The dispatch cannot reread or substitute
the captured target.

## Required and reused calls

| Native dependency | Binding and evidence |
| --- | --- |
| BF681B / BF65AC | Required actual matching CRT allocation/free; allocator body retries malloc/new-handler then throws. No allocation fallback is supplied. |
| AFE0A0 | Primary-owned shared sparse108h constructor, same raw allocation and current D7A24C. |
| BD2F40 | Shared `native_particle_unit_random.hpp`; canonical primary thread state, original guard/FILD/unsigned correction and final float32 spill. |
| AFFA70 / AFFAE0 | Existing actual-curve linear/cubic kernels; native ECX curve, stack time, ST0 result, RET4. |
| B6DB60 / B6DB70 | Existing raw node local getter/world refresh over the actual model and hierarchy. |
| 42D0D0 | Existing no-normalize direction kernel through a native-shaped adapter; the native call explicitly pushes normalize0 and matrix, RET8. |
| BF7030 | Existing register-input CRT sqrt helper and required actual CameraAxesCrtAccess. No library implementation is duplicated. |
| 4134F0 | Existing raw sixteen-pair x87 matrix copy, RET4. |
| current definition+0C | Required captured-target callback, ECX definition and three stack words, RET0C. |

## Verification and limits

Strict MSVC Win32 `/O2 /MD /EHsc /W4 /WX /fp:strict` compilation passed. The
integrator owns CMake registration and the full `scripts/build.ps1` build.

One scratch original-byte fixture at `C:/Users/sqz269/bsp-an-smartarea` passed:
9 combinations of constant/linear/cubic curves and parent/local/refreshed-world
selection, both allocation return branches, and18 actual captured-target calls.
Three scenarios mutate definition70 inside generation before the wrapper's
matrix reload. Position/velocity/matrix words, all model cache bytes, the full
108h constructor allocation preimage/result and canonical random advancement
matched. Final log: `cases=9 allocation_paths=2 callback_calls=18 failures=0`.

The fixture relocates the verified original kernel bytes and changes only their
external CALL rel32 targets and absolute global operands; internal branches stay
unchanged. It binds downstream math/random/curve calls to the same existing
reconstructed helpers and controlled actual fixture storage. Thus it checks
these three methods' sequencing/storage behavior, without independently proving
every shared dependency, game loading, native EH, exceptional CRT inputs, NaN
curves, or every overlapping-output combination. No permanent tests were added.

## AN combined integration

The combined strict MSVC Win32 build and both seeded CTests passed. Focused
original-byte probes were relinked to the current combined library. All26
reconstruction names/signatures and six analyzed FH3 dispatcher comments were
saved and read back, preserving prior annotations; affected exports refreshed.
Four reports verify132 direct call rows with zero failures. The integration
report records exact per-probe limits and supersedes earlier worker pending
notes; application composition, native throwing ABI and gameplay are unvalidated.
