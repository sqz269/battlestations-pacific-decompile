# Ship AI hull geometry and full pre-step

Packet `orch6_ship_ai_hull_axes_d` reconstructs `009DE2F0..009DE5A0`, the
previously partial `009E0270..009E04D9`, and bounds copier
`0098A8E0..0098A91E`. All ends are inclusive. The new public C++ interfaces
are semantic projections, not binary replacements. Descriptive names are
hypotheses. Ghidra analysis/export was read-only against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; proposed names are in
the repository ledger for the integrator to apply. Live flow checks found
150, 148, and 15 instructions respectively, with zero flow gaps.

`ship_ai_hull_geometry_009de2f0` takes the existing block's stored distance
`+3E4`, shoulder offset `+1B8`, actual unit pose/model access, and the existing
`CameraAxesCrtAccess`. Its output records only the fields this body writes:

| Native block field | Produced value |
| --- | --- |
| `174/178` | X/Z of transformed local `(0,0,distance3E4)` |
| `17C/180` | X/Z of transformed local `(0,0,-0.0-distance3E4)` |
| `184/188` | world matrix translation X/Z |
| `1AC/1B0` | normalized binary32 difference `bow-position` |
| `19C/1A0` | `(-0.0-forwardZ, forwardX)` |
| `1A4/1A8` | `(-0.0-beamX, -0.0-beamZ)` |
| `18C/190`, `194/198` | position plus/minus the float-stored beam times `1B8` |
| `1BC`, `1C0` | cached model world maximum/minimum Y, or native `50/-10` |

The native entry receives its block in ECX, has no stack arguments, and
returns with plain RET. It resolves the owner at block `+3FC` separately for
each of three world reads. At each read it checks unit `+C8` and calls
`refresh_pose_00414db0` if dirty, then uses the matrix at `+CC`. The new
world-read contract preserves these three accesses. Hosts with canonical
borrowed pose fields can call `ship_ai_hull_world_pose(PoseRefreshView&)`,
which performs that refresh. A host exposing only an actual cache-valid
matrix may supply it and reject dirty unsupported state; creating fake
parent/local/cache records is not a valid binding.

The two transforms reuse the complete x87 `transform_point_004142e0`.
Normalization reuses `native_vector2_reciprocal_length_00419260` and its
required actual CRT dispatch/exception access. The local planar assembly
retains `009DE3EE..009DE524`, including every binary32 store, x87 operation,
and SSE subtraction from the actual negative-zero constant `00D7A208`
(`80000000`). A heading shortcut or direct copy of the pose's forward row
does not reproduce this producer: translation rounding, projection, and
normalization occur before the beam axes are formed. A zero projection is
left to the existing native reciprocal-length behavior; no fallback axis,
finite repair, or normalization guard is introduced.

The constructor is the input producer, not this per-frame function:
`009E44DB` multiplies unit `+9C8` (full hull length) by double `00CF1748`,
whose bits `3FDCCCCCC0000000` are widened `0.45f`, and stores `+3E4`.
`009E45D7..009E4606` produces `+1B8` from the square root of cruise turn
radius squared minus full length squared times `0.25`, without a guard.
Existing `SHIP_AI_NAV_BLOCK_CTOR.md` documents that constructor; the
integrator owns its constant correction. The geometry output does not
include untouched `+1B4` or duplicate input `+1B8`.

The optional model is a real native object, not an inferred class box.
MDestroyer constructor `006FE460` installs vtable `00CFC3D0`; slot `+20`
at `00CFC3F0` contains `006D1E30`. That seven-byte getter is
`MOV EAX,[ECX+360]; RET`, with inclusive end `006D1E36`. It was initially
undefined in Ghidra; the integrator defined/saved/exported it under the
write lock (see its separate function-definition report). It is now a
queryable function, not a pending analysis gap.

Producer `0087BCC0..0087BF73` loads descriptor `unit+354`, then descriptor
`+50` at `0087BDFB`. It zeroes EBX at `0087BDFE`; when that model pointer is
null, branch `0087BE02` reaches `0087BF5B`, storing that zero into unit
`+360`. This establishes the model-absent path. For a present model the
body selects a part set, allocates `1AC` bytes, calls `007135C0` at
`0087BE58` or `0087BE9B`, then publishes the returned node at `0087BEA4`.
The constructor stores the owner/part set and calls shape producer
`00712440`. That producer reaches local bounds setter `0098A920` at
`007128A4`; the separate existing `0098A750` world-bounds producer uses
the node matrix `+50`, local center `+124`, local extents `+130`, and frame
stamp `+154` to store world minimum `+13C/+140/+144` and maximum
`+148/+14C/+150`. Its producer is already covered by `SPATIAL_INDEX.md`.
This packet does not reconstruct the model construction pipeline.

Geometry calls unit virtual `+20` once to test presence and again when
present. Both native callers of `0098A8E0` were inspected: `009DE554`
inside `009DE2F0`, and `009EB009` inside `009EAFC0..009EB651`. Both push
maximum/minimum stack buffers before their second virtual getter; those
arguments belong to the subsequent bounds copier. The getter's RET has
no cleanup. The copier receives ECX=node and stack minimum,maximum, then
RET8; it performs six x87 loads/stores, not integer memcpy. The typed
copier reuses `HitQueryBounds`/`HitQueryPoint`, without casting a semantic
object to a native node. It does not refresh the node's cached bounds.
A nonnull first model read followed by null on the second is native-invalid;
the C++ interface throws after the earlier geometry stores.

`ship_ai_hull_pre_step_009e0270` completes the head and tail previously
omitted by the shape-only helper in `SHIP_AI_SECTOR_SCAN.md`. Original ABI
is ECX=block, one **unused stack word**, RET4 at `009E04D7`. The earlier
float-argument interpretation is superseded. Constructor call `009E46A9`
passes literal 1; controller call `009F5156` writes a boolean low byte to a
stack temporary and pushes the whole word with its remaining bytes from a
float temporary. No instruction in the callee reads the argument.
`009F5156` belongs to `009F50E0..009F5253`.

The full pre-step reads `unit+538 -> class+570` into block `+168`, obtains
`0080FC30` reference speed, and stores `+3C4` using the original x87
comparison against double `00D045F0` (`3FF6666660000000`, widened `1.4f`).
Below the floor it selects float `00D06874` (`3FB33333`); an unordered
comparison keeps the returned float. It then runs full hull geometry,
requests `00811A30(unit,0.5f)`, and writes the twelve sector shapes from
that returned float, existing stored `+3E4`, and actual full beam `unit+9CC`.
`0080FC30` applies modifier channel 4; `00811A30` divides the class curve
`0082E960` by modifier channel 5. Supplying unmodified values is valid only
when the actual represented modifier state proves the native empty path.

The existing shape helper remains callable for compatibility; its legacy
`half_length_9c8` and `half_width_9cc` field names represent full length and
full beam. Its new overload accepts the exact stored reach, which full
pre-step uses directly. Both now share the original x87/SSE shape loop,
with sector stride `2C` and six-record group stride `108`. Only sector
`+0`, `+4`, `+C`, and `+10` are written. The six radii are
`r, 2.5r, -1, -1, 2.5r, r`; reaches are
`E/10, E/5, E/2, E/2, E/5, E/10`; lateral offsets are
`w/d, w/4, w/d, -w/d, -w/4, -w/d`, where `d` is the actual double at
`00D05AC8` (`40019999A0000000`, widened `2.2f`). The old literal double
`2.2` approximation and compatibility reach literal double `0.45` are
corrected. x87 FCHS precedes negative divisions/multiplications, and the
original constants stay on the x87 stack across both groups.

Finally pre-step clears byte flags `3EA`, `3E9`, `3E8` in that order. If
byte `45` is zero it stores 1 there **before** clearing 65 bytes at `+4`.
A pre-existing nonzero byte, including `FF`, and its existing profile
bytes are preserved. The view takes references to that actual storage and
does not default-initialize a replacement block.

Class `+570` is not an unknown pointer or a nullable model reference:
`ShipLeafTuning.scalar` and `ShipLeafTuningSource.scalar_source` in the
existing vehicle-class Lua loader already record its selected `70h`-byte
AvoidZoneDepth block producer. For example the destroyer consumes
offset `08h`; its array consumes `0Ch`. Full pre-step requires the actual
produced scalar bits. An unbound constructor zero is not recovered depth.
Runtime width currently remains upstream unless the actual `+9CC` producer
is represented; a placeholder getter zero is not validated hull width.

Validation: standalone MSVC Win32 Release build passed, existing CTest
`reconstructed_math` and `native_math_differential` passed (2/2), and native
seed verification matched disk. One ignored `/MANIFEST:EMBED` probe compared
four full pre-step results against copied original bytes of `009E0270`,
`009DE2F0`, `004142E0`, `00419260`, `0098A8E0`, and `006D1E30` with
relocated calls/constants. It compared the entire synthetic block,
including untouched fields, profile bytes, all sectors, and geometry;
all four comparisons passed. Cases cover translated nonunit/rolled poses,
zero projected forward and signed zero, NaN reference speed, and both
model branches. Pose/model callback counts were checked. Reference-speed
and turn-circle dependencies are explicit fixture adapters; both sides
use the existing reconstructed CRT sqrt access. Dirty pose refresh and
exception dispatch were not exercised by this probe. No game execution
or binary ABI compatibility is claimed by this packet.
