# Avoid-zone layer grid normal

Address: `0041BD10`, complete native body `0041BD10-0041BE81`.

`avoid_zone_layer_normal_0041bd10` is a complete control-flow semantic projection
through required layer sampling bindings and existing native CRT/math kernels.
Proposed name `BSP_AvoidZoneLayer_GridNormal` is a hypothesis. Original ABI:
ECX layer, stack float3 output / float2 world point, EAX output, `RET 8` at
`0041BE7F`. The C++ API adds semantic layer/host/CRT arguments and is not an ABI
replacement. No other function is reconstructed or renamed by this packet.

The function transforms the world point to cell coordinates, samples four corners
in order, and adds two triangle gradients. For stored heights `h00`, `h10`,
`h01`, `h11` and grid spacing `s`, the gradients are
`(h00-h10, s, h00-h01)` and `(h01-h11, s, h10-h11)`. Every difference and
component sum is stored to float in native order; the code does not collapse
these expressions algebraically. It normalizes the resulting vector through
the existing x87 vector-length kernel and an explicitly stored reciprocal.

| Step | Site | Native callee | Contract established from complete callee body/listing |
| --- | --- | --- | --- |
| World-to-cell | `0041BD31` | `00417FA0` | ECX layer; stack out/x/z; `RET 0Ch`; float-stored `(world+half_extent)/cell_size` followed by native clamps to `[0, dimension-2]` |
| Height h00 | `0041BD44` | `0041BAE0` | ECX layer; stack x0/z0; ST0 float height; `RET 8` |
| Neighbor x1 | `0041BD57` | `00BF7420` | Consumes extended ST0 = cell.x + double 1.0; actual mutable `0109EEA4` selects SSE2 or x87 conversion; low EAX word is the index |
| Height h10 | `0041BD62` | `0041BAE0` | x1/z0 |
| Neighbor z1 | `0041BD75` | `00BF7420` | Consumes extended ST0 = cell.z + double 1.0; mode is read again |
| Height h01 | `0041BD82` | `0041BAE0` | x0/z1 |
| Height h11 | `0041BDB4` | `0041BAE0` | x1/z1; the first gradient is already visible in the output |
| Vector length | `0041BE14` | `00419440` | ECX summed vector; ST0 float length; bare `RET`; existing `camera_vector_length_00419440` with borrowed actual CRT access |

The base indices are hardware `CVTTSS2SI` results: z0 in ESI at `0041BD36`,
x0 in EBX at `0041BD3C`. The first converter result becomes EBP at `0041BD5C`;
the second is saved at `0041BD7E` for the fourth sample. ESI becomes the output
pointer at `0041BD94`, after its base-z use is complete. Neighboring indices are
not integer increments of the base indices. There is no float store between the
native x87 `FADD` of 1.0 and either CRT conversion. `00D7A210` bytes
`00 00 00 00 00 00 F0 3F` establish that double constant. The decompiler's
pointer/int/float casts obscure this register-input contract.

The native output first receives the first gradient at `0041BDA4-0041BDB1`,
before the fourth height call. The second gradient's differences and spacing
are stored at `0041BDCD`, `0041BDD4`, `0041BDDC`; the three sums are stored at
`0041BDE6`, `0041BDF1`, `0041BDFC`, then copied to output before the length call.
The new source preserves these phases, including partial output if the fourth
sampling binding throws.

Length is float-stored at `0041BE19`. `FCOMI` / `JBE` at
`0041BE23-0041BE27` selects a positive-zero reciprocal for nonpositive or
unordered length. The positive arm computes `1/length` and stores it to float.
All three component products still run, including multiplication of nonfinite
components by zero. This is not an early return of a zero vector. Original
`DE F1` at `0041BE2B` establishes the reverse division; `D8 C9` at
`0041BE57` establishes ST0 multiplication. The candidate's DC-form scan found
no DC register-form sites.

The existing `TerrainGridLayerRecord::cell_size` and
`TerrainGridLayerOffsets::kCellSize` describe native +0Ch. Producer
`00423960` loads dimension and divides double 24000.0 by it, storing the
result at +0Ch at `00423993`. No competing layout or offset constant is added.
Layer and sampling data must remain stable, borrowed CRT pointers must stay
live, and the typed point/output/layer storage must not overlap.

World-to-cell and cell-height remain required bindings. Their existing game
host implementations live in the executable target; cell-height also explicitly
returns zero on invalid storage where native calls `00BF6713` and then resumes
its access. That silent substitute is not accepted as an exact binding here.
The existing native converter and vector-length implementations are reused
directly; no CRT mode, handler, grid data or error fallback is invented. The
existing `00419440` qualification still applies: its finite nonnegative sqrt
core is reconstructed, while native exceptional diagnostics require the actual
borrowed CRT state and exception handler. This packet supplies that required
boundary and does not independently validate the diagnostic behavior.

Every live query verified the configured `bsp` project, program
`/battlestationspacific.exe`, x86 language and image base through `bsp.py`.
The configuration names `C:/Users/sqz269/bsp.gpr`. Live `proto` confirmed the
body and listing return; live xrefs reported no references to the entry. The
worker performed no Ghidra mutations; primary owns annotation/save/export.

Validation results are in `reports/oct15_layer_normal.json`. The required Win32
build passed with all three existing suites (`reconstructed_math`,
`native_math_differential`, `tool_tests`); the native suite covers existing seeds,
not this new routine. All eight direct call rows passed. Compilation and
existing checks establish source/build evidence. No new tests, native-byte
differential execution of this routine, native ABI replacement, game wiring or
gameplay validation is claimed. Full exception/status equivalence across the
new C++ binding boundary is not claimed.
