# Glyph quad geometry fragment

`write_font_quad_00ab98f0_fragment` reconstructs the ordinary geometry prefix
`00ab98f0..00ab9c5d`: four positions, four UV pairs, white vertex colors and six
16-bit triangle indices. It deliberately stops before the conditional child-UI
path beginning at `00ab9c5e`. This is a typed host interface, not a replacement
for the native context/writer objects or a complete text renderer.

## Evidence and original ABI

The live `bsp` project and `/battlestationspacific.exe`, x86 little-endian Win32,
base `00400000`, were verified before the raw-byte analysis. The full function
`00ab98f0..00ab9fc2` is 1747 bytes; saved analysis bytes matched the installed PE
bytes during recovery. SHA-256:
`795b76de15e3a52d032bfca739ca1cffe9714a83d7d619fd6aae9302aa13d236`.
The 878-byte implemented prefix has SHA-256
`6a4b223b92f3da617f26cf8ede3290aee7e6f3a1f7b39cf49b00c0e5349ec685`.
The final instruction is `c2 28 00` (`RET 28h`). Assembly is in the ignored
export `exports/bsp/functions/00ab98f0/assembly.txt`.

ECX is the text context. Ten four-byte stack slots are callee-cleaned:

| Argument | Established use |
|---|---|
| 1 | Glyph payload pointer, not the containing tree node |
| 2 | Pointer to two floats: x, y |
| 3 | Vertex writer pointer |
| 4 | Pointer to six output uint16 indices |
| 5, 6 | Not consumed by the implemented prefix; no host meaning assigned |
| 7 | Quad index; index base is four times this value |
| 8 | Height, low unsigned 16 bits |
| 9 | First vertex index in the supplied writer |
| 10 | Character, low 16 bits, used by the later optional path |

The writer supplies data pointer at `+8`, stride at `+C`, float3 position offset
at `+10`, float2 UV offset at `+28`, signed packed-color offset at `+34`, and four
float-color offsets at `+38/+3C/+40/+44`. The host supplies these established
values without pretending to reproduce the rest of the writer layout.

## Coordinates and output order

Glyph payload `+10` is sign-extended (`00ab9916 MOVSX`), while payload `+14` and
height argument 8 are zero-extended (`00ab9932` and `00ab992d`). Payload `+12`
is not used by this writer. The width multiplier comes from context `+1D8`.
The vertical multiplier comes from mutable float global `00e12fd4`, supplied
explicitly through `FontGeometryParameters::vertical_scale`.

Verified constants are double 960 at `00cec380`, double 720 at `00cef1b8`, and
float 1 at `00d7a24c`. These are normalization divisors, not inferred backbuffer
dimensions. There is no additional NDC conversion or clipping in this prefix.

Conceptually left is `(x + signed_bearing) / 960`, right adds
`unsigned_width * width_scale` before normalization, top is
`y / 720 * vertical_scale`, and bottom uses the supplied unsigned height.
The implementation retains the actual x87 instruction order rather than using
these formulas directly:

- `00ab9936 FST` spills left to float without popping. The right sum therefore
  retains the extended left value until `00ab996d FSTP`.
- Bottom executes `(y + height)`, then `FLD1; FADD ST1,ST0; FSUBP`, followed by
  a float spill. Although the added and subtracted one cancel mathematically,
  these instructions are retained for rounding and exceptional values.
- Normalized coordinates spill to floats at `00ab99a8`, `00ab99c4`,
  `00ab99d0`, and `00ab99ec`. The first vertex reloads x/y through x87;
  subsequent position/UV/color stores move float bits using SSE in the native
  code. The host preserves these first-vertex x87 stores and later bit copies.

The host leaves the caller's x87 control word in effect, as does the native
prefix. It does not promise floating-point exception timing parity: native
stores the first z before all coordinate arithmetic finishes, whereas the host
computes coordinates before writing the output buffer.

| Vertex | Position | UV from payload offsets |
|---|---|---|
| 0 | left, top, +0 | +4, +0 |
| 1 | right, top, +0 | +C, +0 |
| 2 | right, bottom, +0 | +C, +8 |
| 3 | left, bottom, +0 | +4, +8 |

This establishes the four payload floats as UV coordinates in the order
v-top, u-left, v-bottom, u-right. Labels describe observed consumption, not
recovered source names. All colors are white: DWORD `FFFFFFFF` when packed
offset is nonnegative, otherwise float 1 at all four separate color offsets.
White alone cannot establish channel byte order (ARGB versus RGBA, etc.).

Indices are `[base,base+1,base+2,base,base+2,base+3]` with low-16 truncation,
where base is argument 7 times four. The vertex buffer offset independently
uses argument 9; these two inputs are intentionally not combined. The final
native stores at `00ab9c41..00ab9c5d` occur in index order 1,0,3,2,4,5.

## Explicit remaining boundary

The later branch runs only when context `+1B0` and `+1AC` are non-null, `+DC`
is zero, `+1A4` is nonzero, and the null-terminated UTF-16 list at `+1A8`
contains the character argument. It first blanks all four UV pairs and then
creates/configures child UI state through helpers including `00ab78a0` and
`00ab9650`, using `GuiFontBilinear.mshd` and text obtained through `00ab81c0`.
That object's ownership, transforms, resource bindings and callback behavior
are not reconstructed here. Calling this fragment represents the ordinary
quad or the state before this optional postprocessing, not the entire native
function when that branch is enabled. No synthetic shader or callback fills
this gap.

`FontData` supplies scalar glyph fields only; font texture/resource ownership
and the caller's line layout/batching remain separate integration dependencies.
The next complete rendering step needs the real writer/batch and resource
bindings, or recovery of the optional child-UI lifecycle when that feature is
required. Geometry generation alone is not text draw or game validation.

## Host safety and validation status

The host rejects null buffers, zero stride, wrapping four-vertex indices and
out-of-range position/UV/color stores before modifying vertices or indices.
Native code has no such guards. Bounds arithmetic is widened; overlapping
attribute locations retain the per-vertex position, UV, color write order.
Inputs and index storage must not alias the vertex buffer. Unsafe native
pointer wrapping and alias-driven input mutation are outside this interface.

The Win32 build and both existing CTests pass. The existing optional GUI native
reference generator/probe now includes one font prefix case: a negative signed
bearing, fractional coordinates/scales, distinct quad and first-vertex indices,
and sentinel padding. Every vertex byte and all six indices match. The fixture
copies only `00ab990c..00ab9c5e` (exclusive end), checks all branches and four
absolute relocations, and supplies a local stack/register wrapper instead of
the original SEH registration. It has no calls or original-process dependencies.
It excludes the later child-UI branch and exception dispatch.

The installed Arial16 DAT also produces the expected A glyph UVs, normalized
origin, white colors and indices in the existing D3D9 probe. Its GFX texture
loads and recreates through the retained 2D loading route. No font draw or
gameplay equivalence is claimed. The float-color branch remains assembly-backed;
the single native case exercises packed color. No new test target was added.
See `reports/font_geometry_texture_probe.txt` and
`reports/font_geometry_texture_audit.json`.
