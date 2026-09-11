# Actual single-line Text geometry

Address: `00AB9FD0`. Names are hypotheses. Read-only Ghidra wrappers verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` before live batches.

| Routine | Original ABI and boundary | Coverage |
| --- | --- | --- |
| `build_gui_text_single_line_00ab9fd0` and its post-child continuation | ECX Text; unused UTF16 wrapper and actual section stack arguments; `RET 8` at `00ABA261`, length 3; full body `00AB9FD0..00ABA263` | partial projection: complete ordinary path; suspends at `00ABA1F8` when the called glyph writer needs its child tail `00AB9D33..00AB9FAD` |

The builder uses the same `GuiTextLifetime`, actual primary model/mesh/section,
`NativeFontResourceOwners`, native logical mappings and actual material texture
setter. It creates no placement vector, alternate Text state, font metric copy,
texture wrapper, resource factory or default callback.

The native first argument is unused. The new entry accepts it as
`unused_source` so the content caller can pass its retained transformed-string
temporary, but both loops read the current stored Text string. The sole caller
at `00ABAB78` pushes section, then that temporary wrapper, after choosing the
nonwrapped branch. The wrapped builder has different source-copy semantics and
is not edited here.

Entry writes the canonical `line_count` (`+110`) to zero, captures the actual
primary geometry, reads actual stream zero (`mesh+64`) and locks
`(current Text length*4, 0, false)`. It then reloads `mesh+60` and current Text
length for index lock `(length*6, 0, false)`. Native length is the full stored
length, including code units after an embedded NUL; the later loops stop at
the first NUL. Current length is read again for actual section `+10 = length*4`
and `+18 = length*2`. All count arithmetic wraps as native DWORD arithmetic.

The x87 control word is saved after index lock. Width conversion multiplies
current normalized widget width by double 960, temporarily selects truncation,
executes signed64 `FISTP`, keeps the low DWORD and restores the control word.
No finite/range comparison is introduced; masked invalid/overflow conversion
retains the native indefinite integer result. Font height is captured from the
current associated font's sole unsigned lowword. The builder resets and writes
the canonical `measured_width` (`+114`) directly, using unsigned glyph advance,
current Text scale and one x87 float32 store for each code unit.

Alignment reads the current enum and measured width after the measure pass.
Center/right convert the width DWORD using signed `FILD` plus float `2^32`
when negative, subtract measured width, and optionally multiply by double 0.5.
Other modes start at zero. There is no shaping, kerning or LF/CR line break in
this native single-line routine. The older scalar kernel's finite/16384 guards
and placement-vector output are not substituted for this body.

The emission pass reloads stored-string backing and selects each glyph from
the currently associated font, borrowing the same actual `FontGlyphData`
record. The first glyph's raw `gfx_texture_18` is bound to current section
material slot zero. After its release callbacks, the builder reloads the same
glyph's `alpha_texture_1c` and current section material for slot one. Only then
does it store the initial pen into canonical field `+18C`. Empty input leaves
that field untouched. Current code unit is reread after the texture callbacks.

The mapped glyph helper writes actual vertex/index storage. On an ordinary
return, the builder reads the still-selected glyph's current advance, moves
the vertex/index/quad cursors by `4/12/1`, advances the text cursor and uses
current Text scale for the next pen. A callback changing the Text font does
not replace the already-selected glyph used for that advance. The next
iteration resolves the current font again.

An optional child returns a `GuiTextSingleLineContinuation` containing borrowed
native call-frame locals: current glyph/string cursor/pen, height, counters,
actual mesh/section/streams/index address and the same lifetime/services.
It also owns one stable float3 argument allocation. Native `ABA168/ABA16E`
initialize y/z once. With steady ESP `S`, the three LEAs at `ABA1DE`, `ABA1E3`
and `ABA1EA` evaluate respectively to `(S-10h)+4Ch`, `(S-14h)+50h` and
`(S-20h)+5Ch`: all are `S+3Ch`, supplied as glyph-writer arguments2/5/6.
`ABA1F2` stores pen x at `(S-28h)+64h`, the same float3's first word. Pen x
itself remains at `S+18h` and advances at `ABA228/ABA22C`.

The move-only frame's `native_position->data()` is the SAME pointer for all
three arguments and remains stable across moves into optional/outer frames.
Before each glyph call only native-position x is overwritten from the separate
pen; current native-position y is projected into the ordinary writer, while z
is retained for the full call contract (the ordinary writer stores vertex z=0).
The full child continuation must use this allocation, not `&placement.x` or a
temporary float3. Possible changes to it must survive suspension; they never
change the independent pen accumulator. This correction preserves caller
storage without implementing or declaring completion of the missing child tail.

Subsequent full AB98F0 inspection confirms that arguments5/6 are never read.
Only argument2 supplies the position, including the late child-position x read.
The stable allocation and caller alias evidence remain valid; the extra slots
must not be modeled as output buffers or hidden child-state storage. See
`GUI_TEXT_GLYPH_CHILD.md` and `GUI_TEXT_GLYPH_REFERENCE.md` for the callee evidence.

Both mappings remain active at the suspension point. The caller must retain
this frame and its enclosing content continuation; it must execute the real
glyph-child tail before calling `resume_gui_text_single_line_after_child_00ab9fd0`.
That entry starts at native `00ABA1FD`, and can suspend for another child.
Neither entry invokes a fake child handler or automatically treats the child as
completed. Frames must be consumed once by moving them, and active string backing/selected
glyphs must stay alive as the original native pointers require.

Only full completion unlocks the originally captured index stream, then vertex
stream, then calls existing actual `00B865A0` with the original section and
captured mesh. Existing `NativeMeshSectionLayoutServices` must bind the actual
descriptor and current renderer layout factory. There is no implicit cleanup
when an unfinished frame is destroyed, nor new native failure rollback.
`nullopt` is the explicit completed result that permits the content caller's
post-builder color/shadow tail.

Producer evidence is the canonical Text/lifetime constructors for `+110/+114`
and `+18C`, the DAT/font owner for the sole height/glyph/resource fields,
`00B73D70` for mesh stream identities, and `00B857F0` for section range words
and material `+20`. Existing setters preserve actual retain/release ordering.
Every native call and the sole caller are listed in the report; full-function
register filters confirm ESI Text, EBX vertex stream, EDI captured index then
selected glyph/restored index, and EBP initial zero then string cursor. Native
lock cleanup is `RET 0Ch`, texture setter `RET 8`, glyph writer `RET 28h`,
layout rebuild `RET 4`; the builder itself removes both argument words.

Validation: strict MSVC Win32 compile with `/std:c++17 /W4 /WX /O2 /fp:strict`
passes. Coordinated font/lifetime/material headers are not yet merged into this
worker; compilation used an ignored, byte-identical copy of the committed
`1445d2a1` font header plus sibling include paths. The report records live exact
call verification. No new tests, native differential or runtime/visual result
is claimed. Parent performs the combined standard build. Remaining prerequisites
are actual font/image loading and registration, Text factory/remaining virtuals,
glyph-child continuation, real layout factory and renderer/draw composition.
Original SEH/stack ABI, floating trap timing and concurrent mutation are outside
this semantic C++ interface.
