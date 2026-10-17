# cc13-gui2: GUI Text geometry update and builders

Packet cc13-gui2 (2026-10-17). Code: `src/cc13_gui2.cpp`, `include/bsp/cc13_gui2.hpp`.
Builds on cc13-gui1 (`include/bsp/cc13_gui1.hpp`): the three text setters there
(00ABAED0, 00ABB000, 00AB6AB0) reach 00ABA8D0 through
`GuiTextHooks::update_utf16_geometry_00aba8d0`; a binder routes that hook to
`text_update_utf16_geometry_00aba8d0` below. Nothing is wired. Status of all three:
reconstructed and compiled (strict Win32, /W4 clean); not fixture-, ABI- or game-validated.
Names are hypotheses, not recovered symbols.

Earlier work on the same addresses is logical, not native: `src/gui_text_single_line.cpp`,
`src/gui_text_wrapped.cpp`, `src/gui_text_content.cpp`, `src/gui_text_runtime_content.cpp`
and `src/font_geometry_owner.cpp` model the text through `GuiTextWidget`/`FontData`
projections. These routines work on the raw Text object and call every callee
through `GuiTextGeometryHooks` at the binary's call point. The previous projections
were used only as a cross-check; every statement here was read from the listing and the bytes.

## Routines

| Address | C++ | ABI | Notes |
|---|---|---|---|
| 00AB9FD0 | `text_build_single_line_geometry_00ab9fd0` | `__thiscall(text; source, section)` RET 8, SUB ESP,38h | arg1 never read |
| 00ABA270 | `text_build_wrapped_geometry_00aba270` | `__thiscall(text; source, section)` RET 8, SUB ESP,64h | one deviation, see below |
| 00ABA8D0 | `text_update_utf16_geometry_00aba8d0` | `__thiscall(text; source)` RET 4, SEH 00CB8130 | unwind path not reproduced |

### 00AB9FD0, single-line builder

Clears text+110h, takes geometry 00B74640(text+4Ch, 0), vertex stream 00B73260(geometry, 0),
calls vertex_stream->vtable[+10h](4n, 0, 0) (result unread) and index_stream (geometry+60h)
->vtable[+0Ch](6n, 0, 0), the index write pointer; n = text+0ECh. Sets section+10h = 4n,
section+18h = 2n. The width is trunc_i64(text+20h * 960.0 [00CEC380]) under a
truncating control word, low DWORD (inline asm keeps the FNSTCW/FLDCW/FISTP sequence).
Measures text+114h as the per-unit sum of glyph+12h (unsigned) * text+1D8h, stored per
code unit. Alignment text+100h: 2 right (unsigned width minus text+114h), 1 centre (times
0.5 [00D7A280]), other values pen 0. In the emit loop 00AB9FD0 selects each glyph (00AD4480),
binds material slots 0/1 to glyph+18h/+1Ch and stores the pen at text+18Ch on the first
glyph. 00AB9FD0 calls 00AB98F0(glyph, &pos, vertex stream, index pointer, &pos, &pos, quad index,
font height (MOVZX font+14h), first vertex, code unit), with the same float[3] for the
three position slots (checked against ESP after each push at 00ABA1DE/1E3/1EA). pos.y/z are
zeroed once before the loop, pos.x is the pen per glyph. After the loop: index_stream
->vtable[+10h](), vertex_stream->vtable[+14h](), 00B865A0(section, geometry).
A null text+0F0h reads the image's empty wide string 00F8BE50.

### 00ABA270, wrapped builder

Calls 00AB80C0, then copies arg1 into text+0ECh unless it is that header (004C53E0 resize
with preserve, then a code-unit copy including the terminator, skipped for length 0).
Same stream setup as the single-line builder; it does not call 00B865A0. Font height is
MOVSX font+14h here. Per line: leading spaces are skipped except for left alignment at
the text start or right after a line feed; units are measured until a line feed, the
end, or an overflow (`FCOMI` then `JA`: candidate = text+1D8h * advance + width-so-far,
compared unrounded against the unsigned container width; a NaN does not overflow). A
fitting space records the count, CVTTSS2SI of the width so far and its pointer; a
fitting non-space clears the all-spaces flag. An overflow at a space steps back one unit.
When a non-space fitted and the stop unit is a space, trailing spaces are backed over and
the raw (unscaled) space advance is subtracted from the break width each time. The line
ends at the terminator, after the stop unit (no spaces, or a line feed), or after the
last space. Alignment: 0 left; 3 justifies an overflow-broken line (space step =
(width - break) / (spaces - 1) + space advance, FIDIV with no zero guard; 0 without
spaces); 2 right; 1 centre. Alignments outside 0..3 keep the pen and space step from the
previous line. Each glyph updates min top and max bottom (FCOMIP then JA, so NaN
assigns) and calls 00AB98F0 with the same argument pattern; a non-space moves the pen by
advance * text+1D8h (D8 C9 at 00ABA755) and the line sum by the raw advance, a space moves
both by the space step. After each line: text+114h = line sum when ordered-greater
(JBE), text+110h += 1, top += ((text+10Ch + 1.0) - 0.15000000596) * height.
At the end text+178h = max - min; vertical alignment text+104h (1 centre, 2 bottom) gives
an offset from 0.75f [00E12FD4] * text+24h and span / 720 * 0.75 (DE CA FMULP ST2 at
00ABA812 checked from the bytes); section+10h/18h are rewritten from the low 16 bits of the
quad count and every vertex is read back with 004768D0 and written with y + offset at
stream+8 + stream+10h + stride * i.

Deviation: the space step at frame S+28h is written only for alignments 0..3, so an
alignment outside that range reads an uninitialised stack slot on its first line.
The reconstruction starts it at 0.

### 00ABA8D0, geometry update

00AB8530, then a 004C8DD0 copy of the source, upper-cased in place by 00A9EC30 on the
locale table [00F8BC4C] when font+48h is set. Unchanged text is skipped: an empty
text+0ECh changes only when the copy is non-empty, an empty copy always changes, and
otherwise __wcsicmp (00C03A39) decides. On a change: text+114h = 0, 00AB80C0, geometry and
section 00B732C0(geometry, 0), 004C5E20 assigns the copy to text+0ECh. An empty copy
zeroes section+18h/+10h of the main and the shadow (text+188h) sections. A non-empty copy
calls 00AB8400(length, geometry), resets section+8 = 4 and +0Ch/+14h/+10h/+18h = 0,
and when 00AB8CE0 reports a shader change sets the shader (text+1ECh), copies
text+94h/+1D4h to +1DCh/+1E0h and registers cOverbrightAlpha (float2, +1DCh), cLowColor
(float4, +0A4h), cHighColor (float4, +0B4h) and cBlendFactor (float, +0C4h) before
00AA9F10. Then 00B865A0(section, geometry), the wrapped builder when text+0FCh is set,
else the single-line one (both get the copy and the section), and text->vtable[+50h](text+50h).
The shadow shares vertex stream 0 (00B73BB0) and the index stream (00B73B70), copies
section +8/+0Ch/+14h/+10h/+18h, rebuilds its layout, takes texture slot 0 from the main
material when its signed slot count +34h is positive, and on a shader change takes the
main shader (+7Ch), clip parameters and five registrations (cBlendFactor from the zero
block 00F8BE54 first, then the four above). Finally the shadow diffuse colour is text+168h..
+174h, the shadow model's vtable[+2Ch] gets {h/720 * text+164h, same, text+160h ? -0.5 : 0.5},
00B6E680 parents it to text+4Ch when text+15Ch is set (else 0), and 00B6D890(0) runs when
text+15Ch is clear. The copy is released (length*2+2 bytes) on every path.

### 00AA8610, widget alpha (wiring blocker, segment 77)

`gui_widget_set_alpha_recursive_00aa8610`, `__thiscall(widget; float alpha)` RET 4, through
`GuiWidgetAlphaHooks`. Stores widget+5Ch; when 00B74650 reports geometry on model +4Ch
and 00B72B40 (geometry+58h) is nonzero, a fresh 00B74640 geometry gives element 0
(00B732C0) whose material diffuse colour (00B179F0) +0Ch gets the alpha. Then each child
in the std::list at +64h (head +68h) gets the same alpha recursively; the checked-iterator
end tests around the recursive call compare the node with the head re-read from +68h
and report through 00BF6713. Unblocks the HUD fade callers 00611AE0, 00612B70, 0061AFF0,
0065BD90 and 006723F0 (hook `widget_set_alpha_recursive_00aa8610` in
`include/bsp/ingame_gui_cc13.hpp` and `include/bsp/seg25_cc13.hpp`).

### 00AAA710, widget base property read (wiring blocker, segment 77)

`gui_widget_read_properties_and_children_00aaa710`, `__thiscall(widget; visitor)` RET 4,
SEH 00CB7571, through `GuiWidgetReadPropertiesHooks` (cc13-gui1's `GuiWidgetHooks` plus the
pool pair and the visitor slots). It supersedes, for wiring, the partial projections
`read_native_gui_widget_property_prefix_00aaa710` (`src/native_gui_widget_property_prefix.cpp`,
[AAA710, AAABD6) only, through a caller-owned stack preimage) and
`continue_native_gui_widget_property_interaction_00aaabd6`; neither covers the child loop.
Each property is visitor->vtable[+0Ch](key, field, fallback), three 8-byte {tag, payload}
pairs by value: Pos (5, +0Ch, {0,0,0}), Size (6, +20h, {0,0}), Pivot (6, +18h, {0,0}),
Scale (6, +28h, {1,1}), Rotate (2, +48h, 0.0f), Color (8, +50h, static white
[00F8BCE0] initialised on first use under [00F8BCF0] bit 0), LowColor (8, +0A4h,
static {0,0,0,1} [00F8BCCC] under [00F8BCDC]), HighColor (8, +0B4h, white),
BlendFactor (2, +0C4h, 0.0f), WideScreenAlign (0, a NativeString temporary, "").
"Left" (__stricmp, non-null buffer only) sets +0E0h = 1, "Right" (00425850) 2, else 0;
the temporary is released. +8 = +0Ch, and when +0E0h is set and the platform
[0109CF04]+0Dh byte is set, +0Ch moves by 0.14166666567 (double [00D5C118]) left (1) or
right (2). Unless widget->vtable[+5Ch]() is 1, Visible (3, +0E4h, true) then
vtable[+34h](+0E4h byte). MouseBlock (3, +84h, false); set forces +78h = 1, clear reads
MouseHit (3, +78h, false). Then 00AA7220. Children: a 250-pair block with tags preset to
-1 and count 0 goes to visitor->vtable[+18h]; for each tag-0 entry the name becomes a
NativeString (0041DD40 resize, memcpy length+1), 00AA2490 gives the type (0 skips to the
release), cc13-gui1's 00AA6560 creates the child with a null source (no null check
follows), a 184h model block from 00B74EB0 is constructed by 00B75030 with the name (0
when the block is null) into child+4Ch with +138h &= ~3, a current parent is detached
(00AA83A0), the child is inserted at the end of the +64h list (00A9B790, 00A9D480, then
the head/prev links), child+70h = widget, 00B6E680 attaches the models, then
child->vtable[+74h](), visitor->vtable[+4](entry), child->vtable[+18h](visitor),
visitor->vtable[+8](), child->vtable[+78h](), and the name is released.
Deviations: the keys are this file's literals with the image's text, not the image's
literal addresses; the bool fallbacks set only the payload's low byte in the image (the
other three bytes are stale stack) and are zero-extended here.
Unblocks the type-specific readers that call it first: 00A9E400 listbox, 00AABD70
button, 00AAD560 movie, 00AB3310 icon, 00ABB630 text, 00ABE1D0 curve, 00AC0280 section,
00AC2DE0 line, 00AC4C50 layer, 00AC6FD0 group, 00AC85A0 scrollbar, 00AC9900 grid and
the other four callers `bsp.py lookup 00aaa710` lists.

Other GUI-band blockers in `local/output/wire_readiness.json` that were checked and not taken:
00A966E0 BSP_TextEditor_SetActive and 00A92C40 BSP_InputManager_Update already have
faithful typed implementations (`src/text_editor_activation.cpp`,
`src/native_input_action_tick.cpp`; 00A966E0 re-read against the listing: +4 byte,
+24h, 00A965A0, the 0.0f argument pushed before 004BEC00 and consumed by 00A92C40,
resize(0, 0), the conditional memcpy from 00CE3A0C, +20h = 0), so they need binding, not a
second reconstruction. 00A91020 and 00A95CB0 are in cc13-front2's packet. 00A9D9D0 is
an STL instantiation.

## Arithmetic

Each x87 step is written as one double operation and each FSTP as a float conversion.
That matches the image at the CRT's default 53-bit precision for normal-range values
(not for denormal or overflowing intermediates, where x87's wider exponent differs).
The text+94h to +1DCh copies go through FLD/FSTP in the image, which would quiet a
signalling NaN; the C++ copies the float.

## Unblocked callers

- 00AB9FD0 and 00ABA270: 00ABA8D0 (their only caller).
- 00ABA8D0: cc13-gui1's 00ABAED0 BSP_GuiText_SetLocalisedSource, 00ABB000
  BSP_GuiText_SetEllipsizedSource and 00AB6AB0 BSP_GuiText_SubmitUtf16; its other callers
  00ABB1D0 BSP_TextContext_RebuildCurrentText and 00AB98F0 BSP_TextContext_WriteGlyphQuad
  (the glyph-child tail) are not reconstructed natively.
- Still blocking a native Text path: 00AB98F0, 00AB8400, 00AB8530, 00AB8CE0, 00AB80C0
  (hooks here).

## Shared RNG

None of the three routines draws from 00BD2F10.
