# Single-line font layout scalar fragment

`build_font_single_line_00ab9fd0_fragment` produces owned glyph placements and
metrics from a loaded `FontData`, already-transformed UTF-16 input, normalized
container width, horizontal scale and alignment. Each placement contains only
the code unit and its x/y origin, so the existing glyph selector and quad writer
can consume it without retaining pointers into a font or a native text context.
Font resource and vertex/index buffer ownership stay with the caller.

This implements the scalar portions of native `00ab9fd0`, whose complete
660-byte body matches the installed executable with SHA-256
`a0e2dd07ffac72765efcbb1242ec201f77f66202a88d649051f3424859a3c52b`.
Its native ABI is ECX context, two stack slots, RET 8. The first stack slot is
unused by that body; it reads the context's string at `+EC/+F0`. The second
slot is a draw section. The new C++ interface explicitly supplies text and
returns scalar output; it is not that native ABI or the whole function.
The read-only recovery and full ownership boundaries are in
[FONT_LAYOUT_BOUNDARY.md](FONT_LAYOUT_BOUNDARY.md) and
[font_layout_boundary_audit.json](../reports/font_layout_boundary_audit.json).

## Inputs and results

`FontSingleLineParameters` contains `normalized_width`, `width_scale`,
`alignment`, and `max_glyphs`. Modes 1 and 2 center and right-align the text;
all other DWORD values start at zero, as the native branch does. The additional
width scale corresponds to context `+1D8`; DAT metric scaling is already
reflected in `FontData`.

The input `std::u16string_view` is processed through its first NUL or its end.
It need not contain a terminator. Each 16-bit code unit selects a glyph through
`select_font_glyph_00ad4480`; surrogate pairs are not combined. LF and CR use
the selector's existing special records and do not break the line. The input
must already have undergone any requested uppercase-only transformation.

`FontSingleLineLayout` owns a vector of `{code_unit, x, y}` placements and:

| Field | Meaning |
|---|---|
| `measured_width` | The scaled advance sum corresponding to context `+114`. |
| `initial_x` | Aligned initial pen position; native `+18C` is updated only if a glyph is emitted. |
| `final_x` | Pen after the last advance; it is not the rightmost quad bound. |
| `container_width` | Unsigned low DWORD of the native truncating int64 conversion. |
| `height` | Unsigned font-height word supplied to the quad writer. |

All y origins are positive zero. Vector size is the glyph count; four vertices
and six indices per placement can feed the existing buffer/quad path. This
component does not bind texture slots, allocate native buffers, implement
optional child UI or register geometry. It also does not claim that native
context `+110` has become one: the native single-line path leaves that counter
at zero. No normalized-height getter was added because this API does not need
it.

## Arithmetic and supported domain

MSVC Win32 inline x87 instructions preserve the audited operation order:

* Container width: float input multiplied by double 960, kept in x87 through
  signed-64-bit `FISTP`; retain its low DWORD. The conversion temporarily ORs
  rounding-control bits with `0C00h`, then restores the saved control word.
* Measurement: zero-extend glyph word `+12`, `FILD`, multiply by float scale,
  add the previous float width, then spill once to float32 per code unit.
* Alignment: `FILD` the signed interpretation of the low DWORD and add float
  `2^32` when its sign bit is set. Subtract the measured width; mode 1 then
  multiplies by double 0.5. Spill once to float32.
* Placement: start at that aligned x and repeat the native unsigned-advance,
  multiply/add/float-spill sequence after every emitted code unit.

These correspond to `00aba055..00aba0a9`, `00aba0cf..00aba0ea`,
`00aba100..00aba148`, and `00aba1fd..00aba22c`. Width and final pen are computed
in separate passes; the implementation does not replace either with an
unscaled sum multiplied once. Bearings and quad widths do not affect advance
measurement. No kerning, wrapping or ellipsis algorithm is introduced.

The caller's x87 precision and rounding remain active for the floating
operations. Host validation compares the actual extended container product
against `[-2^63, 2^63)` before integer conversion, avoiding indefinite integer
results. It compares each extended width/origin/pen result against
`[-FLT_MAX, FLT_MAX]` before the native float spill. These checks retain the
accepted value on the x87 stack; they do not insert a rounding spill. Results
outside that finite intermediate domain are unsupported even if a particular
rounding mode might have rounded one back to a finite float.

Input width and scale must be finite; finite negative values are accepted.
Glyph count through the first NUL must be at most both the caller's
`max_glyphs` and 16,384. The hard cap permits quad indices starting at zero
without wrapping the native 16-bit vertex indices. Empty input is supported
as a host projection with an empty placement vector and zero measured width;
the native rendering wrapper normally bypasses empty geometry generation.

Unsupported inputs return false with an error string and leave the supplied
layout unchanged. Work occurs in a temporary result and is published only after
all checks pass; allocation exceptions propagate with the old result intact.
Error text is cleared at entry. Floating exception flags and trap timing are
not promised to match the native function: validation adds comparisons, and
the caller's enabled floating exceptions still apply. The control word is
restored after every performed integer conversion.

## Integration and validation boundary

The component adds only `include/bsp/font_layout.hpp` and
`src/font_layout.cpp`. It does not open a stream or reference mounts, archives,
preload selection, shaders or devices. A caller keeps `FontResources` alive,
selects each placement's glyph again by `code_unit`, and passes placement x/y,
the result height and supplied width/vertical scales to
`write_font_quad_00ab98f0_fragment`.

The integrated MSVC Win32 build and both existing CTests pass. One added case
in the existing installed-font probe lays out `A <LF><CR>A<NUL>X` with width
0.125, scale 1.25 and center alignment: five placements, container 120, measured
width 37.5, first x 41.25 and final x 78.75. It checks each origin and feeds all five
placements to the existing quad writer. This checks scalar/geometry output;
the multi-glyph case is not rendered. No new test target or framework was added.

The existing real A draw now obtains its pen from this layout, then adds the
explicit host origin 64,64. It retains the same 74 lit pixels, zero pixels outside
the expected region, and restored device state. Full output and limits are in
[integration evidence](../reports/parallel_implementation_validation.json) and
[probe output](../reports/parallel_implementation_probe.txt). The other component
worker independently reviewed this implementation against the native assembly.
The new scalar layout has no native differential execution or original-game
text comparison; the earlier native single-quad fixture covers the quad writer.
