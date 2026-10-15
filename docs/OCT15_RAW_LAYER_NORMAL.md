# Actual-layout avoid-zone layer normal

`layer_normal_raw_kernel_0041bd10` covers the complete instruction schedule of
`0041BD10-0041BE81` (370 bytes, 118 listed instructions). It accepts the actual
mutable `Cc13TerrainGridLayerNative` backing and uses the accepted concrete
WorldToCell, CellHeight, ST0 conversion and vector-length Source services.
The earlier typed `avoid_zone_layer_normal_0041bd10` and its semantic host,
admission rules, name/evidence and ledger record remain separate and unchanged.

| Routine | Original ABI | New Source ABI | Coverage |
|---|---|---|---|
| `0041BD10` | ECX layer; stack float3 output, float2 point; EAX output; RET8 | stdcall(output, point, layer, actual mode pointer, CRT access); EAX output; RET14h | complete Source instruction schedule through qualified existing services |

The symbol is a descriptive hypothesis, not a recovered name. This is a new
C++ entry, not a drop-in binary member or proof of original ABI compatibility.
EBX, EBP, ESI and EDI are preserved. The first two Source argument slots retain
the native output/point positions; the dead point slot is reused for stored
heights, length and reciprocal. Extra arguments provide the actual conversion
mode address and existing CRT access instead of inventing globals or callbacks.

## Data and service boundary

The raw layer is the existing producer-shaped `Cc13TerrainGridLayerNative`,
not `TerrainGridLayerRecord`: dimension at `+08h`, spacing at `+0Ch`, late height
scalar at `+10h`, actual byte-vector begin/end at `+1Ch/+20h`. Producer evidence
and complete sampling bodies are recorded in `OCT15_LAYER_SAMPLING.md`.
No layer, grid, vector proxy, owner or service state is copied. No semantic
adapter admission guard restricts dimensions, backing shape or numeric values.

Accepted raw CellHeight preserves signed clamps, wrapped DWORD row indexing,
the original span test, invalid-parameter call and returning-handler begin
reload/read. The current Source `_invalid_parameter_noinfo` service is qualified
against original `00BF6713`/CRT encoded-global, handler, debugger, Watson and
fault/return identity. A returning handler must leave the subsequent read
accessible as the native instruction schedule requires; this kernel neither
returns zero nor fabricates a repair provider.

Neighbor conversions retain extended ST0 and load the supplied actual mutable
mode pointer into ECX at each conversion.
The existing mode-dependent instruction paths are used directly. Vector length
uses `camera_vector_length_00419440`, ECX output and EDX borrowed
`CameraAxesCrtAccess`. Its original finite sqrt core is reconstructed; exceptional
CRT diagnostics remain qualified through the existing actual state/handler
access boundary. The Source CRT access object and call/control storage do not
claim original binary frame/global/provider identity.

## Read, write and arithmetic order

Point z and then x are read before output writes. WorldToCell's stored cell
coordinates are converted with CVTTSS2SI for the base indices. Each neighbor is
instead converted from extended `cell + double 1.0` by `00BF7420`; integer base
plus one would change behavior. Original `00D7A210` is the double-1.0 byte value
`00 00 00 00 00 00 F0 3F`, represented by one Source constant.

Heights are sampled as h00, h10, h01, then h11, each stored as float. The first
live spacing read precedes all first-gradient output stores. The kernel writes
`(h00-h10, spacing0, h00-h01)` to output before the fourth height call; that call
observes any resulting mutations of aliased layer fields or bytes. A separate
spacing read follows it. The second gradient is
`(h01-h11, spacing1, h10-h11)`, with the native x87 operand/pop order and float
stores. The pair is summed through float temporaries and copied to output
before the length call. Saved sum components, rather than a post-handler output
reload, supply final scaling.

Point, output, layer fields and borrowed cell bytes may overlap. There is no
stable-record or noalias restriction among those original data ranges. Every
address must be accessible when the original read/write schedule reaches it;
Source call-frame/control slots and CRT provider/access backing must remain
valid. This does not promise arbitrary aliasing into the added Source argument
slots, private frame or a different CRT provider representation.

At `0041BE23`, FCOMI followed by JBE selects a positive-zero reciprocal for
nonpositive **or unordered** length. All three component multiplications still
execute, including zero times NaN/infinity. Positive length uses extended
`1/length` then a float store. The original raw bytes are `DE F1` at `0041BE2B`
(FDIVRP), `DE CA` at `0041BE4B` (FMULP ST2) and `D8 C9` at `0041BE57`
(ST0 multiplied by ST1). The Source object has the same three forms; a complete
native DC-form scan found zero register-form DC sites.

## Direct native calls

| Call site | Target | Concrete Source contract |
|---|---|---|
| `0041BD31` | `00417FA0` | Raw WorldToCell; ECX layer, EDX local pair, stack x/z, RET8 |
| `0041BD44` | `0041BAE0` | Raw CellHeight h00; ECX layer, EDX x, stack z, RET4 |
| `0041BD57` | `00BF7420` | Extended x neighbor; ECX actual mutable mode pointer, ST0 input, EAX index |
| `0041BD62` | `0041BAE0` | Raw CellHeight h10, same complete Source contract |
| `0041BD75` | `00BF7420` | Extended z neighbor, same existing Source converter |
| `0041BD82` | `0041BAE0` | Raw CellHeight h01, same complete Source contract |
| `0041BDB4` | `0041BAE0` | Raw CellHeight h11 after first-gradient writes |
| `0041BE14` | `00419440` | Existing vector length; ECX output, EDX actual CRT access, ST0 result |

Source height calls reserve/discard one padding word so original local/scratch
offsets and output timing remain intact. The fourth sample's z argument is
installed before the first-gradient writes, as in the original. Incoming and
outgoing Source ABI adaptation does not establish original stack/frame identity.

## Verification and limits

Read-only bsp.py queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and the complete body and eight call sites. The
accepted service files are unchanged from their integrated revisions. Assembly,
operand bytes, source object disassembly, producer mapping and callee contracts
were reviewed; `./scripts/build.ps1` passed the three existing suites:
reconstructed_math, native_math_differential and tool_tests (3/3, 7.07 seconds).
The existing linker duplicate-definition warning is unrelated to this packet.
The build log is `local/oct15_raw_layer_normal_build.log`; object disassembly is
`local/oct15_raw_layer_normal_object.txt`. Call-report and diff checks are
recorded in `reports/oct15_raw_layer_normal.json`.

Existing Original probes cover WorldToCell (four output/return cases), CellHeight
(one valid-span wrapped-row case), and bilinear sampling (one finite output-bit
case). They do not exercise this normal. No worker tests or native probe were
added. Current evidence establishes complete Source schedule and build/static
checks, not full FP status/trap/control-mode behavior, original ABI/CRT diagnostic
identity, all storage aliases, adapter binding or gameplay execution.
