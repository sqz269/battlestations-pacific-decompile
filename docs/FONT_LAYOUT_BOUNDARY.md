# Bounded text layout reconstruction boundary

The smallest useful next port is the **single-line measurement, alignment and
glyph-placement fragment of `00ab9fd0`**, consuming the existing `FontData` and
feeding the existing `00ab98f0` quad prefix. It needs no archive or stream work:
the input font has already been loaded. The surrounding native renderer and
text-context lifetime remain separate contracts.

This read-only audit selected five routines: `00ab9fd0`, `00aba270`,
`00ab6bd0`, `00ab8f00`, and the already reconstructed selector `00ad4480`.
The first four form a bounded single-line/wrapping/height/ellipsis family; the
selector is their existing data dependency. The wrapping and ellipsis algorithms
are sufficiently different that they should not share an invented general
measurement helper.

The live `bsp` project and `/battlestationspacific.exe`, x86 little-endian,
base `00400000`, were verified before each analysis batch. Live code inspection
stayed within `00ab6000..00ad89ff`; the parent approved the specific constant
reads listed in the report. Ignored raw evidence is under
`exports/bsp/parallel_font_layout/`. Every selected complete function was
compared byte-for-byte with the installed PE. No source implementation, test,
build, Ghidra annotation, saved-project mutation, or game validation occurred.

After reviewing this audit, the primary agent applied the four proposed
descriptive annotations, preserving existing comments and the established
wrapped-geometry name. The later C++ implementation and focused validation are recorded in
[FONT_SINGLE_LINE_IMPLEMENTATION.md](FONT_SINGLE_LINE_IMPLEMENTATION.md).
The body below preserves the original analysis boundary.

## Selected functions and original ABI

| Address | Established role | Original ABI and result |
|---|---|---|
| `00ab9fd0` | Single-line glyph placement and geometry population | ECX context; two stack slots, RET 8. Caller passes UTF-16 string wrapper then draw section. This body ignores the first slot and reads context `+EC/+F0`. Side effects, no established return value. |
| `00aba270` | Wrapped/aligned glyph placement and geometry population | ECX context; stack UTF-16 string wrapper, draw section; RET 8. Copies string to context `+EC/+F0` if wrappers differ, then writes geometry and metrics. |
| `00ab6bd0` | Normalized text height query | ECX context; no stack arguments, RET. Float32-rounded value returned through x87 ST0. |
| `00ab8f00` | Width-limited UTF-16 copy with trailing three dots | ECX context; stack destination string wrapper, source string wrapper, normalized float width; RET 0C, EAX destination. Allocates/copies native strings. |
| `00ad4480` | Existing glyph selection | ECX font; low-16 key in one stack slot; RET 4, EAX borrowed glyph payload. Existing descriptive name is retained. |

`00aba8d0` branches on context byte `+FC`: zero calls `00ab9fd0`, nonzero
calls `00aba270` (`00abab59..00abab7c`). Both receive the same two arguments.
Before this, `00aba955..00aba991` resets width `+114`, clears optional children,
and invokes the string assignment helper for context `+EC`. It handles empty
text before either geometry path. Consequently, the single-line function's
ignored string argument is intentional observable behavior; a host projection
can accept an explicit text span but must not claim native object ABI identity.

## Single-line fragment: implementation-ready behavior

Text is null-terminated **16-bit code units**, advanced by two bytes. A null
data pointer uses the saved-image empty word at `00f8be50`. There is no surrogate
pair decoding, Unicode shaping or pair-kerning lookup in either placement loop.
The selector handles each individual code unit. Space and LF select embedded
font payload `+4C`; CR selects `+8C`; missing ordinary keys select the copied
`0091` fallback at `+6C`. LF and CR do not cause a line break in this path.

The context's font pointer is `+108`, its additional horizontal scale is
float `+1D8`, normalized container width is float `+20`, alignment mode is
DWORD `+100`, accumulated width is float `+114`, and first emitted origin is
float `+18C`. These are field roles, not a recovered class layout.

1. Reset context `+110` to zero and `+114` to float zero. The line counter
   remains zero even for nonempty single-line text.
2. Multiply container width by double 960 using x87. Temporarily set rounding
   control to truncation, `FISTP` to a signed 64-bit temporary, keep its low
   DWORD, then restore the control word (`00aba055..00aba0a9`). Subsequent
   conversion treats this DWORD as unsigned: `FILD` followed by addition of
   float `2^32` if the sign bit is set.
3. Scan the entire text. For each selected glyph, zero-extend payload word
   `+12`, multiply by context scale, add current width, and spill to float32
   (`00aba0c3..00aba0ea`). Glyph bearing `+10` and quad width `+14` are absent
   from measurement. Font DAT scaling has already happened; `+1D8` is an
   additional scale.
4. Start x is zero for modes other than 1/2. Mode 2 subtracts measured width
   from the converted container width. Mode 1 additionally multiplies the
   difference by double 0.5. Spill the result to float32
   (`00aba100..00aba148`). No width clamp or wrapping occurs.
5. Scan again, emitting every nonzero code unit at `(x, 0)`. Advance x with
   the same unsigned-advance, multiply, add, float-spill sequence. Supply
   unsigned font height word `+14` to the quad writer. Quad count increments
   by one, first vertex by four, and index destination by twelve bytes.

Preserve the x87 operation/spill sequence when numerical parity matters.
Single-line accumulation is not the per-character integer truncation used by
ellipsis. Neither summing unscaled advances then multiplying once nor using
quad widths gives this algorithm.

The first emitted glyph alone binds payload `+18/+1C` into the supplied draw
section's material `+20`, slots 0/1, and stores the starting x at context `+18C`
(`00aba189..00aba1b8`). Later glyphs do not rebind textures. This is evidence
for a single font resource pair, not automatic texture-page batching.

## Wrapped path: exact features and retained quirks

`00aba270` scans lines using the same unsigned advance times context scale and
float32 accepted-width spills. It converts normalized container width through
the same truncating 64-bit/low-DWORD route. However, it saves alignment widths
with `CVTTSS2SI` and uses different arithmetic when emitting spaces.

* LF (`000A`) terminates a scan; CR (`000D`) does not. The selected emitted
  interval includes the terminating LF, because the end pointer is advanced
  one code unit at `00aba51e`. LF therefore reaches the quad writer and selects
  the space/LF glyph. This is not a conventional newline-stripping layout API.
* Left alignment preserves leading spaces at the initial string position and
  directly after LF. Other alignments skip leading spaces; left alignment
  skips spaces after a soft wrap (`00aba3e3..00aba427`). There is no generic
  whitespace-class check: only literal space is used here.
* On overflow, the scan may rewind from an overflowing space. If an accepted
  space exists, the chosen emission end can be one past the last accepted
  space; without an accepted space, it includes the overflowing code unit.
  An over-wide unbroken word therefore is not simply clipped to fitted glyphs.
  `00aba4c2..00aba534` is the decisive endpoint/width sequence.
* Modes 0/1/2 are left/center/right; mode 3 adds justification on soft wraps.
  Natural spaces in the emission loop use the **unscaled** space advance;
  nonspace glyphs use advance times scale (`00aba73d..00aba777`). The scanning
  loop scaled spaces too. Do not silently make these two loops agree.
* Justification computes natural space advance plus unsigned
  `(container_width - saved_width) / (accepted_space_count - 1)` in x87.
  It only guards a zero count, not a count of one
  (`00aba560..00aba599`). This can divide by zero; no corrected denominator
  should be introduced under a native-behavior claim.
* Context `+114` is compared against a second emitted-line accumulator. For
  nonspaces that accumulator adds the **unscaled** advance; for spaces it adds
  the chosen spacing. The wrapper resets `+114` before calling; this function
  only raises it (`00aba785..00aba79b`). It is not generally the final pen x.

Each emitted glyph participates in min-y/max-y tracking. Font height is
sign-extended when preparing line geometry at `00aba284`, although the quad
writer consumes its low unsigned word. Per-line y advances by
`((DistanceBetweenLines + 1.0) - 0.15000000596046448) * signed_height`,
then adds previous y and spills to float32 (`00aba7a3..00aba7ca`). The subtraction
constant is an exact double encoding of the indicated float value; it is not
exact decimal 0.15. Context `+110` increments once per processed line.

After layout, context `+178` receives `max_y - min_y`. Vertical modes 1/2 compute
center/bottom offsets from context normalized height `+24`, this extent divided
by 720, and mutable vertical scale `00e12fd4`. A final vertex pass adds the
normalized offset to y. The quad count is narrowed to low16 before final
section vertex and triangle counts are written (`00aba843..00aba859`).
The empty-input path has sentinel-based extent behavior, while the ordinary
wrapper avoids empty geometry calls. A future safe host interface should state
its supported domain rather than imply every invalid/overflowing native input
has meaningful layout.

## Height and ellipsis must remain distinct

`00ab6bd0` returns `context+178 / 720` when multiline byte `+FC` is nonzero.
Otherwise it returns sign-extended font height / 720, or zero if the font pointer
is null. Each branch explicitly stores and reloads float32 before returning in
ST0. It does **not** apply mutable vertical scale, unlike the quad geometry and
the wrapped vertical-alignment pass.

`00ab8f00` first constructs/copies the destination from the supplied source.
Font flag `+48` can invoke transformation helper `00a9ec30` before measurement;
the existing font registry identifies this as `uppercase_only`. The helper's
cached export confirms in-place per-code-unit transformation, but its full
mapping and native string helper ownership are outside this audit. Do not
substitute the host locale's uppercase operation without separate evidence.

The ellipsis width is `low16(trunc_i32(normalized_width * double(960)))`.
For every code unit the routine computes `low16(trunc_i32(previous_u16_width +
unsigned_advance * scale))` (`00ab8fb0..00ab9004`). If the resulting unsigned
width exceeds the target, it subtracts three scaled dot advances from the
target, again truncating to integer and retaining low16. It then removes suffix
code units while the accumulated unsigned width exceeds that target, applying
the same truncation/low16 after every subtraction. Finally it concatenates
the retained prefix and literal `...`. The three-dot reserve can wrap modulo
65536 for a too-small target. This routine does not count lines or apply the
wrapping path's space rules.

## Output ownership and independent host boundary

The native callers obtain geometry from context `+4C` through `00b74640(0)`,
the vertex stream through `00b73260(0)`, and the index object from geometry
`+60`. They invoke vertex virtual `+10` and index virtual `+0C` with capacities
`text_length * 4` and `text_length * 6`, then unlock through vertex `+14` and
index `+10`. The supplied draw section receives vertex count at `+10` and
triangle count at `+18`; those are distinct from byte capacities.

Upstream `00ab8400` obtains the vertex format and creates vertex/index objects
through renderer virtual `+38/+5C/+60`, assigns them through `00b73bb0` and
`00b73b70`, then releases its local references. Its assembly is essential:
pseudocode incorrectly invents an `unaff_retaddr` input. The actual two stack
arguments are glyph capacity and geometry, RET 8. This establishes the
allocation/assignment/release sequence, not the complete lifetime of those
external objects. The allocator, setters, external helper
`00b865a0`, and complete text-context teardown are external contracts, not
permitted stubs.

`00aba8d0` subsequently shares the populated vertex/index objects with a
second geometry route and copies section counts. The single-line builder also
calls `00b865a0` after unlocking. Full native ownership and cache identity require
those external helpers, so the initial scalar layout port should return an
owned host list of `{code_unit, x, y}` placements and scalar metrics, with an
explicit loaded `FontData` input. The existing host caller keeps `FontResources`
alive and uses its real resources, existing material owner, buffer allocation,
and quad writer to consume that list. A layout result need not retain native
glyph pointers or any file/stream provider.

The proposed first implementation is therefore:

1. Reconstruct the two single-line scalar loops and modes 0/1/2 from
   `00ab9fd0`, preserving float spills and container conversion. Accept explicit
   already-transformed UTF-16 code units and decoded font data; state the
   finite/capacity domain of the new host interface.
2. Reuse `select_font_glyph_00ad4480` and
   `write_font_quad_00ab98f0_fragment` to feed the existing installed-font draw
   path with multiple placements. Keep resources at font/material level and
   expose metrics separately from geometry buffer ownership.
3. Add the tiny normalized-height query if the caller needs it. Implement
   wrapped layout and ellipsis as subsequent address-labeled fragments,
   retaining their arithmetic differences and explicit preprocessing boundary.

This adds useful multi-glyph output without requiring MPKG, compression, native
preload policy, or stream changes. It does not recover optional per-character
child UI, full native text context, clipping, renderer cache lifetime or original
game rendering parity. Those are separate from the existing successful
single-glyph installed-shader diagnostic.

## Proposed Ghidra annotations and evidence

The accompanying [audit report](../reports/font_layout_boundary_audit.json)
records current names, decompiler-visible original comments, and proposed
append-only comments.
Suggested new descriptive names are
`BSP_TextContext_BuildSingleLineGlyphGeometry` (`00ab9fd0`),
`BSP_TextContext_GetNormalizedTextHeight` (`00ab6bd0`), and
`BSP_TextContext_EllipsizeUtf16ToWidth` (`00ab8f00`). Retain the existing
`BSP_TextContext_BuildGlyphGeometry` and `BSP_Font_SelectGlyph` names. These
names are hypotheses describing observed behavior, not recovered source symbols.
No library symbol is renamed. Parent review/integration must preserve existing
comments, record old values, save the project, and refresh affected exports.

| Complete function | Bytes | SHA-256 |
|---|---:|---|
| `00ab6bd0..00ab6c27` | 88 | `15ff2522be095170c10d36732bb1e556efaefd2b3168b3d70be68c577177b1b8` |
| `00ab8f00..00ab923f` | 832 | `3b7ea64623a1834efe8beeab7fb6028a15c920c8ed04affacfd5cb78aea3e6eb` |
| `00ab9fd0..00aba263` | 660 | `a0e2dd07ffac72765efcbb1242ec201f77f66202a88d649051f3424859a3c52b` |
| `00aba270..00aba8c6` | 1623 | `4c435d42b8c60f1eeab6987c2530b1bd54216c74c9d1b325f0f491f8d114d54b` |
| `00ad4480..00ad44fd` | 126 | `fe786a41f924b7a9ef37af1db008c658c9d494a19f10a90d2adf400579efd1ce` |

The constants and additional caller/allocator windows are in the report.
`00e12fd4` contains **0.75 in the saved image and disk**, which is not evidence
of its runtime value. `00f8be50` is PE zero-fill rather than disk-backed bytes.
Only the listed code and data spans were identity-checked; this is not a
whole-Ghidra-image comparison. The selected family contains no false no-return
truncation in its returning paths. Unrelated exploratory destructor pseudocode
does stop incorrectly after `_free`; it was not used to infer ownership.
