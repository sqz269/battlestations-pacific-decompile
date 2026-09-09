# Wrapped font layout scalar fragment

Addresses: `00aba270`

`build_font_wrapped_00aba270_fragment` now produces owned glyph placements,
processed-line records and measured/aligned extents from loaded font data. It
reproduces the selected native scan and emission rules, including their
different treatment of spaces and LF. The existing quad writer consumes the
raw placement coordinates; a separate helper applies vertical alignment to
each vertex **after** that writer normalizes coordinates.

This is a new C++ scalar interface. Native `00aba270` takes ECX text context,
UTF-16 string wrapper and draw section on the stack, and returns with RET 8.
The native routine also copies strings, clears optional child objects, locks
buffers, binds material resources and unlocks buffers. Those ownership and
renderer operations are outside this implementation.

## Fresh evidence and relationship to the earlier audit

The existing `bsp` project and `/battlestationspacific.exe` were verified with
`tools.ghidra_export.Client` and `config/target.json` before this export batch.
Language is x86 little-endian Win32 and image base is `00400000`. The complete
body `00aba270..00aba8c6`, 1,623 bytes, again matches the installed PE:
`4c435d42b8c60f1eeab6987c2530b1bd54216c74c9d1b325f0f491f8d114d54b`.
The final vertical-adjustment loop `00aba860..00aba8a8`, 73 bytes, is a slice
of that fresh comparison with SHA-256
`f74a27e4963074980ae1b0ac480bb1e2624c37e9750270a09ccd9e1980080e17`.

Raw assembly, decompilation, state mapping, target identity and comparison
records are under `exports/bsp/parallel_font_wrapped/`. The tracked
[audit report](../reports/font_wrapped_layout_audit.json) identifies each
implemented instruction group. Constants use the earlier parent-approved,
byte-checked records in `parallel_font_layout/constants.json`; mutable vertical
scale is supplied explicitly, not taken from its saved-image value of 0.75.
No Ghidra mutation was performed for this packet.

Reinspection confirmed the earlier [layout boundary](FONT_LAYOUT_BOUNDARY.md)
but refined its whitespace interpretation: rewinding the scanner changes the
space count and saved width without changing the last-accepted-space pointer.
The eventual emission interval can therefore still contain those spaces.
Calling that adjustment generic trailing-space removal would be incorrect.

## API and ownership

`FontWrappedParameters` supplies normalized width/height, additional horizontal
scale, vertical scale, distance between lines, horizontal mode 0 through 3,
vertical mode 0 through 2, and glyph/line capacities. Modes are respectively
left/center/right/justify and top/center/bottom. Inputs must already have
undergone any requested uppercase-only transformation. The function never
changes the input string or font data.

`FontWrappedLayout` owns these outputs:

| Field | Meaning and consumer |
|---|---|
| `placements` | Existing `FontGlyphPlacement` values `{code_unit,x,y}`. Raw origins before quad normalization; no borrowed glyph pointers. |
| `lines` | One entry per native processed-line iteration, including a possible zero-placement iteration after skipping terminal spaces. |
| `lines[].first_placement/placement_count` | Interval within `placements`, including LF when the chosen source interval includes it. |
| `lines[].initial_x/y` | Initial raw pen coordinates for that processed line. |
| `lines[].emitted_width` | Unscaled nonspace advances plus the chosen space increments; distinct from scan width and final x. |
| `container_width` | Low DWORD of the truncating signed-64-bit width conversion. |
| `height` | Unsigned font-height word for the quad writer. |
| `measured_width` | Maximum emitted-line metric, beginning at zero as native caller `00aba8d0` initializes context `+114`. |
| `measured_height` | Native `max_y-min_y` float extent, context `+178`. |
| `normalized_height` | Float-spilled `(measured_height / 720) * vertical_scale`; this is not the unscaled height-query `00ab6bd0`. |
| `normalized_vertical_offset` | Final normalized y translation applied after quad writing. |

The first emitted placement's x supplies the native first-origin metric
written at context `+18C`. `lines.size()` supplies the native processed-line
count written at `+110`. The scalar interface starts the width metric at zero;
it does not preserve an arbitrary preexisting native `+114` value.

Generation uses a private candidate. Unsupported inputs return false with an
error string and leave the supplied output unchanged. Allocation exceptions
propagate with the old output intact. Error text is cleared on entry. Successful
publication moves owned values into the output. Font textures, native string
allocators, draw sections, and vertex/index buffers are not retained or invented.

## Exact scan and interval selection

Input ends at its first NUL or the supplied span end. A span need not contain
a terminator. Code units are processed individually; no surrogate pairing,
shaping, kerning, tab stops or general whitespace classification is introduced.
Only literal space (`0020`) and LF (`000A`) receive the special layout tests.
CR (`000D`) does not end a line. The existing glyph selector still supplies
special space/LF and CR records and the missing-key fallback.

For each processed line, `00aba3e3..00aba427` skips leading spaces except when
left alignment begins at the original string start or immediately after LF.
Thus left alignment preserves indentation after an explicit LF but skips it
after a soft wrap. The other supported alignments skip leading spaces.

The scanner starts with float width zero, saved integer width zero, zero
accepted spaces, no last-space pointer, and an all-spaces flag. At
`00aba465..00aba4c6`, it zero-extends the selected glyph advance, multiplies it
by context scale and adds the previous float scan width in x87. It compares
that **extended candidate before spilling** against the unsigned-converted
container width. Accepted candidates spill to float32. On an accepted space,
the previous scan width is converted through `CVTTSS2SI`, and the last-space
pointer and count are updated. An accepted nonspace clears the all-spaces flag.

The scan stops at NUL, LF or an overflowing candidate. If the overflowing unit
is a space, it first moves the scan pointer back one unit. When an accepted
nonspace exists and the resulting current unit is a space, it repeatedly moves
back, decrements the accepted-space count and subtracts the **unscaled** space
advance from the saved DWORD width. It does not update the last-space pointer
(`00aba4cd..00aba4f4`). DWORD width subtraction retains modulo-32-bit bits.

`00aba4fb..00aba534` chooses the exclusive emitted end:

* At NUL, use the current scan pointer.
* At LF or with zero accepted spaces, use one past the scan pointer.
* Otherwise, use one past the last accepted space, even if the scan pointer
  has subsequently moved backward.

Consequently, LF reaches the glyph writer, and an over-wide unbroken word can
include its overflowing code unit. The saved alignment width is replaced by
`CVTTSS2SI(full_accepted_scan_width)` when its saved value is zero or the current
stop unit is NUL/LF. This is not a standard word-wrapping algorithm.

## Alignment and emitted metrics

The normalized container width uses x87 multiply by double 960, temporary
truncation control, signed-64-bit `FISTP`, and its low DWORD
(`00aba32a..00aba39d`). Each later unsigned conversion uses signed `FILD` plus
float `2^32` when the sign bit is set. Integer alignment slack is the low-DWORD
subtraction `container_width - saved_width` **before** that conversion.

Left alignment uses x zero and natural unscaled space advance. Center/right
use the converted unsigned slack, additionally multiplied by double 0.5 for
center. Justification uses left/natural spacing when the stop is NUL or LF.
For a soft wrap, zero accepted spaces produce a zero space increment;
otherwise it computes:

```text
space_increment = natural_unscaled_space_advance
                + unsigned_slack / signed(accepted_spaces - 1)
```

Native only guards zero accepted spaces. The count-one division has no finite
meaning and is explicitly rejected by the host rather than changed to a
different justification formula (`00aba560..00aba599`).

During emission (`00aba73d..00aba777`), a literal space advances both pen x and
the emitted-width metric by the chosen space increment. Every other unit
advances x by unsigned advance times scale, but adds the **unscaled** advance
to the emitted-width metric. LF uses the same selected payload as space but
falls into this nonspace branch: its pen advance is scaled. The x87 code keeps
the unscaled metric beneath the scaled x increment and spills each result once.
Scanning, pen advancement and the exported width metric therefore intentionally
differ. The implementation does not force them into one shared width formula.

## Vertical geometry and instruction order

Font height is sign-extended then converted exactly to float for line spacing
and extent calculation (`00aba284/00aba294`), while the quad writer receives
its unsigned low word. Each emitted line contributes raw y and float-spilled
`y + signed_height` to the min/max bounds, whose native sentinels are
`+1e10/-1e10`. The y step is performed after each processed line, including the
last and a zero-placement iteration:

```text
next_y = float32(((distance + double(1))
                  - double(0.1500000059604644775390625)) * signed_height + y)
```

The code retains this x87 sequence, not a simplified `distance + 0.85`
expression (`00aba7a3..00aba7ca`). Raw extent spills to float32. Its division
by double 720 remains extended through multiplication by vertical scale,
then the normalized height spills. Center/bottom subtract that spilled height
from `vertical_scale * normalized_container_height`; center also multiplies
by double 0.5 (`00aba7dc..00aba841`).

`apply_font_wrapped_vertical_offset_00aba860_fragment` implements the y portion
of the final vertex pass: load normalized vertex y, add the normalized offset,
spill float32, reload and store float32 (`00aba86d..00aba896`). Apply it to
each vertex y after `write_font_quad_00ab98f0_fragment`. Adding an equivalent
raw offset to placement y before division/scaling can round differently.
The helper leaves its scalar output unchanged on nonfinite input/result.

The caller's x87 precision/rounding and SSE environment remain active. Only
the integer-width conversion temporarily selects x87 truncation and restores
the prior control word. Scan-width integer conversions remain `CVTTSS2SI`.
Floating computations retain the native float spills; host checks reject
nonfinite spilled results before publishing the layout. Floating exception
flags, enabled traps and trap timing are not an equivalence claim.

## Explicit supported domain

The host requires finite scalar inputs, a container product in signed-int64
range, float results that remain finite, and scan-width conversions in the
signed-int32 range. Finite negative scalars and signed-negative font heights
are not silently clamped. Horizontal modes above 3 are rejected because the
native fallthrough can reuse an uninitialized space increment; vertical modes
outside 0 through 2 are excluded from this interface.

Input code units through the first NUL are capped at both `max_glyphs` and
16,384. Processed lines are capped at both `max_lines` and 16,384. Forward source
intervals ensure emitted count cannot exceed accepted input count. This avoids
the native final low16 quad-count wrap and keeps quad indices starting at zero
within 16-bit vertex indexing. Endpoints that precede the input, invalidate the
space counter, or fail to make forward progress are rejected. The native
justified soft-wrap count-one division is also rejected explicitly.

Empty input and text producing no placements are unsupported here. Native
caller `00aba8d0` has an empty-geometry route; invoking the wrapped body directly
without emitted glyphs instead uses its sentinel extent. The host caller must
use its empty route rather than treat that sentinel result as a normal height.
A terminal-space skip after earlier emitted text can still add a zero-placement
processed-line record, matching the audited iteration behavior.

## Validation boundary

The scalar component is implemented and independently reviewed against the
complete byte-verified assembly. It is integrated into the existing installed
font probe and passes the MSVC Win32 build and both existing CTests. One
`A A\nA` case emits five glyphs across three lines, checking soft wrapping, LF
emission, asymmetric spacing and alignment. The installed shader draw produces
900 lit pixels, none outside the generated bounds, with ink on all three lines
and restored device state. The original single-A draw still produces 74 pixels.
See [integration results](../reports/parallel_entry_validation.json).

D3D9 leaves x87 control word `007F` active in this probe. Its 24-bit precision
produces measured height `62.1000023` and normalized offset `0.163749993`.
The fixture allows the observed rounding around the decimal expectations;
the source preserves the native arithmetic and caller floating environment.
No test target or broad wrapping suite was added.

Native differential execution, original-game wrapping/render parity, optional
child UI, material rebinding/caching and full native context lifetime remain
unverified. A successful host draw alone must not be reported as those results.
