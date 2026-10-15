# Instruction-backed avoid-zone layer sampling

Addresses: `00417FA0`, `0041BAE0`. Existing reviewed names and older game-host
implementation records are preserved. This packet adds complete Source kernels
and a separately admitted concrete adapter for `AvoidZoneLayerNormalHost`.
Names are hypotheses; the new calling conventions are not native ABI replacements.

| Native body | Original ABI | New Source kernel ABI | Coverage |
| --- | --- | --- | --- |
| `00417FA0-0041803E` | ECX layer; stack out/x/z; EAX out; `RET 0Ch` | `layer_world_to_cell_kernel_00417fa0`: ECX layer, EDX out, stack x/z; EAX out; `RET 8` | complete instruction schedule with incoming/outgoing ABI adaptation |
| `0041BAE0-0041BB48` | ECX layer; stack x/z; ST0 float; `RET 8` | `layer_cell_height_kernel_0041bae0`: ECX mutable layer, EDX x, stack z; ST0 float; `RET 4` | complete instruction schedule through qualified current-CRT invalid-parameter entry |

World-to-cell retains native x87 addition/division and each float store. The
dead Source x/z argument slots supply the same u/upper scratch values that the
native body stores in its dead argument slots. Double 2.0 remains on the x87
stack between the two upper-bound computations; z's bound uses native `FISUBR`.
`00D7A308` bytes `00 00 00 00 00 00 00 40` settle the constant. The DC-form scan
found `DC E9` at `00417FD6`: ST1 = ST1 - ST0, dimension minus 2.0.

For each stored coordinate c, ordered `0 > c` selects positive zero; otherwise
ordered `c > upper` selects `float(dimension-2)`; equality/unordered retain c.
The `COMISS` / `JBE` decisions and later `FCOMIP` / `JBE` decisions are retained.
There is no replacement with double arithmetic, `std::clamp`, dimension repair
or finite-value filtering. Original output stores and later layer reads remain
interleaved. The Source ABI supplies a different call frame and return cleanup.

Cell-height keeps signed index clamps, the wrapped DWORD multiplication/addition
`dimension*z+x`, a nonnull begin test and the unsigned comparison against
`end-begin`. It performs the native byte read even after a returning invalid-span
report. Begin is reloaded after that report; the previously formed index remains
in EDI and is not recomputed. `file_scalar` is loaded at the late `FMUL`, after
the byte read and integer `FILD`; the result is stored to float and reloaded.
No second check, zero result, invented cells or repair callback is inserted.

| Containing routine | Call site | Native callee | Source boundary |
| --- | --- | --- | --- |
| `0041BAE0` | `0041BB25` | `00BF6713` | existing `invoke_native_invalid_parameter_00bf6713` |

The invalid-parameter entry's complete body and existing Source implementation
were read. Its current SDK zero-argument `_invalid_parameter_noinfo` import is
a qualified Source policy: it does not reproduce original encoded-global,
debugger, dynamic-handler/Watson or fault/return-address identity. This packet
does not rename or replace that service. If a current Source handler returns,
it must leave the reloaded begin plus the cached index readable. It can repair
the mutable raw backing and change the late scale; no exception/return policy
or `noreturn` assumption is added. Unrepaired backing can still fault.

Both raw kernels use the existing `Cc13TerrainGridLayerNative` declaration,
with compile-time checks against the existing `TerrainGridLayerOffsets`.
Producer `00423960` establishes half extent, dimension, cell size, scale and
the byte vector; `004239E0` reads these scalars in file order, resizes the
native byte vector at +18h to the wrapped dimension square and fills its
actual +1Ch begin storage. The existing Source parser in
`scene_contents_hosts.cpp` reads the same scalar fields and assigns those file
bytes to `TerrainGridLayerRecord::grid`; `scenes_seg1_cc13.cpp` has the same
stream-field mapping. The records are related by fields, not by C++ object
layout or by interpreting `std::vector` internals as a native vector.

`AvoidZoneLayerSamplingHost::admit` returns an optional concrete host only for
a valid C++ record with dimension >= 2, exact dimension-squared vector size
and a nonnull data pointer, with the size representable as a DWORD span.
The square check uses unsigned 64-bit arithmetic. Rejected input receives no
adapter; it is not repaired or sampled. Scalar bits are copied into private
native-shaped Source backing. The cells remain at the actual `grid.data()`
address: no cell values are synthesized or copied. Private unused vtable,
proxy and capacity fields do not establish a native owner/container identity.
The backing is used only by the two kernels and has no byte ownership.

Admission is separate from the raw kernels' contract. For the admitted positive
dimension, either signed clamp arm produces an index in [0,n-1], and every row
index lies in [0,n*n-1] without DWORD overflow. Thus every cell read is in the
borrowed exact byte span even if a world/CRT conversion returned an exceptional
integer. The raw invalid-parameter path is unreachable under the adapter's
stable-data contract. Those preconditions are not imposed on direct raw calls.

The caller keeps the same semantic record object, all scalar bits, vector
storage/size and cells stable for the adapter lifetime/calls. A method given a
different record object throws Source `std::invalid_argument`; this is an adapter
identity check, not a recovered game branch. Point/output must not overlap the
record or bytes. This const semantic adapter does not admit handler-driven
repair, mutable raw-header identity, malformed vectors or native owner services.
The mutable raw boundary preserves the applicable reload/repair behavior instead.

The admitted host can be passed directly to `avoid_zone_layer_normal_0041bd10`
with its `admitted_layer()`. The outer routine's actual CRT state/handler and
finite-sqrt-core versus native-diagnostics qualifications still apply. No game
host or existing normal file is edited, and no runtime binding is enabled.

Live `bsp.py` queries verified project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base. Configuration names
`C:/Users/sqz269/bsp.gpr`. All original bodies/callees were read; caller audit
finds `0041BC20` and `0041BD10`, with two world calls and eight cell calls total.
Primary owns all Ghidra mutations. Validation is in the corresponding report.
The Win32 build and all three existing suites passed; these existing suites
cover existing seeds. Primary also ran one focused paired WorldToCell probe
against the full installed 159-byte body, independently matched to saved Ghidra
bytes, with only its double-2.0 data operand relocated to an isolated executable
page. It linked this worker's actual compiled `bsp_core.lib` and passed 4/4
inputs: finite arithmetic, combined lower/upper clamps, two distinct unordered
NaN payloads, and signed zero. Both binary32 outputs and each returned output
pointer's identity matched under masked default x87/MXCSR state.

Primary evidence is in main checkout `local/oct15_layer_sampling_probe/`:
`result.json`, `original_input.json`, and `probe.cpp`; the executable was linked
with `/MANIFEST:EMBED`. This is bounded WorldToCell output/return evidence.
CellHeight, FP status/traps, other control states, native ABI, the adapter, outer
normal and gameplay remain outside that probe. No new worker fixture, game
execution or runtime binding is claimed.
