# Native cone definition generation

Addresses: `00B03970`, `00B03B60`, `00B03AC0`.

| Routine | Native contract | Coverage |
| --- | --- | --- |
| `00B03970..00B039E0` | ECX definition; stack(model,time); RET8; EAX allocated108h record or null | complete through real allocation/free and common AFE0A0 |
| `00B03B60..00B03EBE` | ECX definition; stack(record,position,velocity); RET0C | complete |
| `00B03AC0..00B03B38` | ECX definition; stack(record,position,velocity,matrix); RET10 | complete through canonical captured current virtual0C |

The proposed C++ names are descriptive hypotheses. EDX adds a required borrowed
`NativeParticleConeDefinitionAccess`; these are new interfaces over actual native
storage, not drop-in binary replacements. Void generation interfaces do not
promise the incidental native EAX value. No game or application binding is claimed.

## Producer and caller evidence

`AF9FB0` selects `ConeEmitter`, allocates94h and calls `B03940`; that constructor
calls `AFA280` then installs `D5DEBC`. Live `D5DEC4/8/C` point respectively to
`B03970/B03B60/B03AC0`. All three live xref queries returned only those DATA
references; no direct native caller is recorded.

The known canonical indirect consumers were separately read in assembly:
`AF6E6C` captures definition slot08 and supplies model/time; `AFDB0D` captures
recordA0 definition slot0C and supplies record/position/velocity; `AFDC12`
captures recordA0 definition slot10 and supplies record/position/velocity/matrix.
`B03AD5` is the cone's additional current-slot0C dispatch. This lists static
evidence, not an assertion that runtime dispatch has no other caller.

The producer `B03EC0` maps named parsed curves to the following words. Native
stores, rather than inferred geometry names, establish these offsets:

| Offset | Producer token | Store instruction |
| --- | --- | --- |
| 80h | InnerEmitSpeed | B0422E |
| 84h | OuterEmitSpeed | B042B8 |
| 88h | MaxAngle | B042F0 |
| 8Ch | InnerDistance | B0434C |
| 90h | OuterDistance | B0437F |

The parser is not reconstructed here. Its pseudocode has existing `_free`
no-return artifacts, so those apparent early returns are not used as evidence.
`AFA32C` stores the constructor's supplied word into definition70h. Existing
canonical record declarations establish modelA4, definitionA0 and record60h;
no competing record or model layout is introduced.

## Preserved behavior

Slot08 allocates108h through the required original-domain `BF681B` service
(`ADD ESP,4` at B03993). It preserves the null branch and forwards time through
x87 float32 before common `AFE0A0` (three stack words, RET0C). This is the sparse
temporary constructor, not the separate initialized-child entry `AFE1A0`.
The C++ cleanup uses the same required `BF65AC` domain and propagates exceptions.
The native EH table DF3748 points to CBB670; CBB674 calls BF65AC, followed by
CBB679 POP ECX and CBB67A RET. Current live CBB670's body ends at CBB678 because
of the existing no-return flow artifact. That ancillary tail repair belongs to
the integrator and does not leave an unread branch in B03970.

Slot0C spills record3C*record34 to float32, compares against CURRENT double
D7A220, and uses CURRENT float CE3D08 when the native JBE selects the fallback.
Curve mode0 reads curve04 directly; mode1 calls AFFA70; other modes call AFFAE0.
All calls retain their native RET4/ST0 contracts and unbounded segment walks.

MaxAngle is scaled by CURRENT double D5DAF8. The first primary random draw is
unconditional and scaled by CURRENT CE3828. A second draw occurs exactly when
the native FUCOMIP/LAHF parity sequence selects it; ordered zero skips it and
unordered angle does not. The common `NativeParticleUnitRandomAccess` uses the
same `RandomThreads` owner and preserves the BD2F40 guard, signed FILD plus
unsigned correction, CURRENT scaling and float32 result. No replacement random
state or cached scalar value exists.

FSIN/FCOS are the original x87 instructions, including their original spills
and exceptional-input behavior. Direction uses record60h when the CURRENT
recordA0 definition70h is nonzero. Otherwise it captures recordA4 model, selects
modelB0 when model1B0 is set, or refreshes modelF0 if flag5C bit2 is clear.
The existing exact normalize=false branch of `0042D0D0` is reused through a
thin ABI adapter; it is not a new general normalization implementation.

The native scalar/time scratch lifetimes, x87 stack order, float64 complement
spill, output order and aliases remain explicit in the assembly kernel. In
particular, the OuterEmitSpeed pointer is captured before position writes,
dereferenced after those writes, and the InnerEmitSpeed pointer is loaded later.
No vector input snapshot or reordered curve evaluation hides those aliases.

Slot10 captures its own CURRENT vtable0C once and passes that target to the
canonical `NativeParticleEmissionSpawnAccess::definition_virtual0c`. That
binding must dispatch the actual captured target, including another override.
After dispatch it reloads recordA0/definition70 and recordA4 as reached, then
copies the actual selected matrix with existing sequential x87 `004134F0`.
This permits callback mutations and native forward-overlap effects.

Every direct call site is machine-readable in the accompanying report. Callee
bodies were read before selecting contracts: BF681B/BF65AC/BF9DC8, AFE0A0,
AFFA70/AFFAE0, BD2F40, B6DB60/B6DB70 and 004134F0/0042D0D0. Library allocation
and free remain required real services, not ports of CRT implementation code.

## Verification and limits

Configured project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, bridge8089 were verified by the read-only command
client. Worker made no Ghidra mutation. The integrator created the missing
B03AC0 function through B03B38 after an early address-lease handoff; raw bytes
match before and after definition.

All three complete routine ranges match live Ghidra bytes and the installed
PE bytes; exact lengths and SHA256 hashes are in the report. Seed verification
passed. Win32 MSVC `/W4 /WX /fp:strict` compiled the cone translation unit.

Scratch fixture `C:/Users/sqz269/bsp-an-cone/probe.cmd` builds and runs
`cone_probe.exe` with `/MANIFEST:EMBED`. `native_data.py` verifies live/disk
bytes and emits the relocated native bodies. Seven differential scenarios
passed: zero/nonzero cone angle; constant/linear/cubic curves; overlapping
position/velocity; record/local/world selection with world refresh; and matrix
output in native record storage. Each compares the full record/definition/model
and output storage, x87/MXCSR status and next canonical random draw.

The reference relocates external calls to the same verified shared kernels;
it independently checks these cone bodies, not those shared dependencies or
original CRT identity. The allocator/EH path was source/assembly inspected and
compiled, not executed by this fixture. Exceptional inputs, callback mutation,
all floating control modes and game reachability were not exhaustively tested.
The fixture linked the then-current common AFE0A0 source and existing main
`bsp_core.lib`; final CMake registration/full build belongs to the integrator.

## AN combined integration

The combined strict MSVC Win32 build and both seeded CTests passed. Focused
original-byte probes were relinked to the current combined library. All26
reconstruction names/signatures and six analyzed FH3 dispatcher comments were
saved and read back, preserving prior annotations; affected exports refreshed.
Four reports verify132 direct call rows with zero failures. The integration
report records exact per-probe limits and supersedes earlier worker pending
notes; application composition, native throwing ABI and gameplay are unvalidated.
