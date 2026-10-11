# GUI widget render kernels: Curve, Line, Scrollbar, Progbar (cc13-render1)

Packet `cc13_gui_render`, partition segment 78 (`00AACF10..00AC9640`). Code:
`include/bsp/gui_render.hpp`, `src/gui_render.cpp`. Every name is a hypothesis, not a
recovered symbol.

Status vocabulary: **exported** (pseudocode in ignored `exports/`), **reconstructed**
(C++ in `src/`), **build-tested** (compiles in the MSVC Win32 build). Nothing here is
fixture-tested, ABI-compatible or game-validated.

## Segment survey

Segment 78 already had 122 reconstructed functions under existing docs: Movie
(`docs/GAME_MOVIE_PLAYER.md`), Icon (`docs/GUI_ICON_WIDGET.md`, `docs/GUI_ICON_FRAME_RUNTIME.md`),
Text (`docs/GUI_TEXT_*.md`), Section (`docs/GUI_SECTION_RUNTIME.md`), font registry
(`docs/FONT_REGISTRY_STARTUP.md`), Layer/Group pools (`docs/GUI_LAYER_MANAGER.md`). This
packet took the four widget classes that had no doc. The class of each comes from the
loader chain: the key suffix goes through `00AA2490` (`__stricmp` chain) to a type id,
`00AA6560` switches on the id to an allocator stub, the stub runs the constructor, and the
constructor stores the vtable.

| Key suffix | Type | Size | Allocator | Constructor | Vtable |
|---|---|---|---|---|---|
| `Line` | 4 | 104h | `00AA1530` | `00AC2D00` | `00D5C9E8` |
| `Progbar` | 5 | 140h | `00AA14A0` | `00AC77C0` (over the Icon ctor `00AB5C60`) | `00D5CC18` |
| `Scrollbar` | 7 | 0FCh | `00AA15C0` | `00AC7C20` | `00D5CCF8` |
| `Curve` | 0Dh | 140h | `00AA1800` | `00ABE030` | `00D5C7F8` |

Correction: `00AC2090`/`00AC2DE0` were recorded as `BSP_GuiPoints_*`. They are the Line
widget's +1Ch/+18h slots (`00D5CA04`/`00D5CA00`); the `Points` string is the property key.
Both were renamed `BSP_GuiLine_*`.

Every class has the same three type accessors: +08h `MOV EAX,[chain]; RET`, +14h
`MOV EAX,[own id]; RET`, +0Ch a `RET 4` scan of a three-dword chain. Twenty such bodies
and other vtable targets had no Ghidra function; they were defined with
`tools/ghidra_define_function.py` (record `reports/cc13_gui_render_defined_functions.json`)
after reading their bounds from disk (`ret`/`jmp` followed by `INT3` padding).

## Curve (type 0Dh)

Fields: `+ECh/+F0h/+F4h` padded control array (ptr, count, capacity, Vec2), `+F8h/+FCh/+100h`
per-point widths (float), `+104h` texture NativeString (`+108h` c_str), `+10Ch/+110h/+114h`
gameplay point array (Vec2), `+118h` cached vertex count, `+11Ch..+128h` atlas UV rect
(left, top, right, bottom), `+12Ch` material, `+130h..+13Ch` std::vector<Vec2> of the
layout `PosArray`.

- `00ABE1D0` ReadProperties (+18h, `__thiscall(visitor)`, `RET 4`). `PosArray[1..]` entries are
  read as GuiValueTag 5 (three floats): x, y go to the `+130h` vector through `004222C0`, z to
  the width list `+F8h` (`00ABE2CA..00ABE31C`). The padding block that follows is guarded by
  `CMP EAX,2; JNC 00ABE59C` at `00ABE349`: it runs only when **fewer than two** points were
  read, and then indexes `[0]` and `[1]` under the checked-iterator guard `00BF6713`
  (`_invalid_parameter_noinfo`). With two or more layout points `+ECh` is left alone, so
  a layout-only Curve draws nothing until gameplay feeds `00ABCA10`. This reads like an
  inverted test in the original; it is recorded as observed. Then `Texture` (`+104h`) is
  read, `00ABDD80` applied, and virtual +7Ch called.
- `00ABC7A0` DescribeProperties (+1Ch): `PosArray` from the `+130h` vector, then `Texture`.
- `00ABDD80` SetTexture (`__thiscall(NativeString*)`, `RET 4`): case-insensitive compare;
  on change copies the name, resets the UV rect to (0,0,1,1) (`00D7A24C` = 1.0f), resolves
  through `00AA2660` and binds material slot 0 (`00B189F0`). Gameplay callers: `0053A4D0`,
  `005A3830`, `005A7550`.
- `00ABC920` ClearPoints, `00ABC970` AddPoint(`Vec2*`, `float width`), `RET 8`: the gameplay
  feed into `+10Ch` and `+F8h`. Same three callers (`0053A4D0`'s sites such as `0053B824` lie outside its current Ghidra body, so the checked rows in `reports/cc13_gui_render.json` use the `005A3830`/`005A7550` sites) (rel32 scan of the PE: `0053B521`,
  `005A3955`, `005A7755`; `0053B63C`, `0053B66E`, `005A39A3`, `005A39F1`, `005A77D2`, `005A784B`).
- `00ABCA10` RebuildControlPoints (`__fastcall`): empties `+ECh`; when `+110h > 1` writes
  `p0 - (p1 - p0)`, the points, `pn + (pn - pn-1)`, then tail-jumps to +7Ch
  (`00ABCBBD`). Reconstructed as `gui_curve_control_points_00abca10`.
- `00ABCC70` CreateGeometry (+74h): mesh plus `guidefault.mshd` material with
  `cOverbrightFactor`, `cLowColor`, `cHighColor`, `cBlendFactor` (strings `00D5C25C`,
  `00D5C47C`, `00D5C470`, `00D5C464`, `00D5C454`); material stored at `+12Ch` (`00ABCCFC`).
- `00ABCED0` Tessellate (+7Ch, `__fastcall`), reconstructed as
  `gui_curve_tessellate_00abced0`:
  - Requires `+F0h > 3` (`00ABCF00`). Samples per span `S` = 10 and vertices across `C` = 3,
    or 4 and 2 when exactly four control points (`00ABCF0A..00ABCF24`). Rows
    `(n-3)*S + 1`; the last span takes `S+1` samples (`00ABD1EE`).
  - The buffers are rebuilt only when `rows*C` differs from `+118h` (`00ABCF4E`): vertex
    format from `SimpleColor.mvfm` (`00CF0078`), 16-bit index buffer of
    `(C-1)*(rows-1)*6` entries (`00ABD025..00ABD047`, flags 1, 65h), zig-zag strips with
    the last two indices swapped on odd triangles (`00ABD0D0..00ABD0FD`), draw section
    type 4 with vertex count and triangle count `(2C-2)*(rows-1)` (`00ABD129..00ABD146`).
  - Position: Catmull-Rom with the constants `00D7A2B0` (3.0), `00D7A328` (4.0),
    `00D7A370` (5.0), `00D7A308` (2.0), `00D7A280` (0.5), evaluated at `t` and `t + 1/S`.
    The difference is normalised by `00419260` (reciprocal or zero). The width is
    `w[i-1] + t*(w[i] - w[i-1])`; the normal is `(-dy, dx) * w`; cross vertex `j` sits at
    `(j/(C-1) - 0.5) * 2` along it. `y` is scaled by `00CEC9D8` (0.75) after the offset;
    `z` is `00CE3800` (0.5). Colour is packed `0xFFFFFFFF` or four float 1.0 depending on
    the stream's colour offset sign.
  - U: arc length so far, V: `j/(C-1)`; afterwards U is divided by the total and both are
    mapped into the UV rect (`00ABDCB8..00ABDD41`).
  - Arc length: after every sample except a span's last, `|P(t+dt)-P(t)| * [ESP+18h]` is
    added (`00ABDC73`). `[ESP+18h]` is written only on the four-sample path
    (`00ABD2D1..00ABD302`): `(chord/4.78) * (w_k - 1) + 1` clamped at 0, `chord` =
    `|ctrl[0] - ctrl[3]|`, `w` = {1.9, 0.0975, 0.1, 10.0, 0} (`00D21A94`, `00D5C7F0`,
    `00D7A2F0`, `00CE38B8`). On the ten-sample path the slot is stale: in a call that rebuilt
    the buffers it holds the index-strip countdown (`00ABD0AA`, `00ABD10B`), which ends at
    0, so the arc length is 0 and U is 0/0. The C++ takes that value as an explicit
    argument. Whether `SimpleColor.mvfm` samples a texture at all is not established.
  - Ends with stream unlock (+14h) and the widget's virtual +50h on `this+50h`.

## Line (type 4)

Fields: `+ECh..+F4h` std::vector<Vec2> of pending points, `+FCh` committed vertex count,
`+100h` capacity (min 4).

- `00AC2DE0` ReadProperties: `Points[1..]` as GuiValueTag 6 (Vec2) into `+ECh`.
- `00AC2430` CreateGeometry (+74h): `SimpleColor.mvfm` / `guidefault.mshd`.
- `00AC2950` Reset (+7Ch): erase `+ECh`, `+FCh = 0`, `00AC2690(4)`, element `+10h/+18h = 0`.
- `00AC2690` EnsureCapacity (`RET 4`): requests below 5 become 4; on change reallocates
  `2*cap` vertices and indices and copies `min(committed, cap)` of the old data.
- `00AC29D0` Flush (+80h, `xref 00D5CA68`), reconstructed as `gui_line_flush_00ac29d0`:
  element vertex count `n` and primitive count `n-1` (`00AC2A35..00AC2A3B`); locks
  `n - committed` vertices from `committed` (lock offset semantics from
  `BSP_LogicalVertexStream_Lock` `00B49980`: the data pointer starts at offset*stride) and
  the index range from `committed*2`. Loop `k`: index pair `(committed+k-1, committed+k)`
  when `committed+k >= 1`; vertex `committed+k` = `(points[k].x - [this+18h],
  points[k].y * 0.75 - [this+1Ch], 0)` (`00E12FF4` = 0.75), UV 0, white. Then the vector is
  emptied (`00AC2C21..00AC2C79`) and `+FCh = n` (`00AC2C1B`). `+18h/+1Ch` are the base
  widget's Pivot fields (`include/bsp/gui_widget.hpp`), subtracted unscaled. Because the
  vector is emptied, a later flush appends only when the vector has grown past the old
  count, and then from its front; reproduced as observed.

## Scrollbar (type 7)

Fields: `+ECh` value, `+F0h` MinValue, `+F4h` MaxValue, `+F8h` horizontal (1) / vertical (0).

- `00AC85A0` ReadProperties: `Alignment` (`00D5CDB0`, String, default `""` at `00CE3A0C`)
  compared to `Vertical` (`00D5CDA4`) with `__stricmp` (`00AC8623`): equal stores 0, else 1;
  `MinValue` and `MaxValue` floats default 0 (`00AC8646..00AC86DC`); then
  `SetMinimum(min)`, `min = max` when `min > max`, `SetValue(max)` when `value > max`,
  virtual +7Ch. Reconstructed as `gui_scrollbar_apply_properties_00ac85a0`.
- `00AC8770` DescribeProperties: writes `Alignment` when `+F8h == 0`, the two floats when
  they differ from 0 (`00BD5680`).
- `00AC7D00` SetValue / `00AC7D80` SetMinimum / `00AC7DE0` SetMaximum: clamp and keep
  `min <= max`; each ends in virtual +7Ch. Callers of SetMaximum/SetValue include
  `00ACA290` (segment 79, list box).
- `00AC8890` OnLoaded (+78h): looks up the four children and discards the results, then
  `SetValue(min)` and `00AA7170`.
- `00AC8A80` LayoutChildren (+7Ch), reconstructed as `gui_scrollbar_layout_00ac8a80`.
  Children are found with `00AAA2B0` (case-insensitive name-suffix search of the child
  list): `_BackArrow_Icon` (EDI), `_ForwArrow_Icon` (EBP), `_Background_Icon` (EBX),
  `_Slider_Icon` (`[ESP+14h]`). The arrows are not null-checked. Horizontal
  (`00AC8C6B`): back at 0, forward at `W - fwd.w` (W = `+20h`), background at `back.w`,
  slider at `back.w` when `max == min` (`FUCOMIP/LAHF/TEST AH,44h/JP`), else
  `back.w + (W - (slider.w + 2*back.w)) * (value-min)/(max-min)` (`00AC8D1A..00AC8D75`,
  doubles on the stack). Vertical uses `+24h` and heights (`00AC8D88..`; read to
  `00AC8DFA`, the rest follows the same shape in the pseudocode).
- `00AC7C90` is a lone `RET` in the +74h slot; `00AC8320` (+80h) was not analysed.

## Progbar (type 5)

Icon-derived; `+138h` current value, `+13Ch` DefaultValue.

- `00AC77C0` ctor: Icon ctor, `+138h = 1.0`, `+60h = 5`.
- `00AC7A90` ReadProperties: Icon reader, then `DefaultValue` (Float, default 1.0) into
  `+13Ch`, copied to `+138h`. `00AC7B00` writes it back when it differs from 1.0.
- `00AC7870` SetValue (+A8h, `RET 4`): `v' = (width*v)/width` on the x87 stack, rounded to
  float; when it differs (`FUCOMPI/LAHF/TEST AH,44h/JNP`) stores `+138h`, calls +8Ch
  (`00AB10D0` RebuildCurrentState) and +88h(0, 3, v') (`00AB1710` SelectState).
  Reconstructed as `gui_progbar_set_value_00ac7870`.
- `00AC7820` GetValue (+ACh): `FLD [ECX+138h]; RET`.
- `00AC78E0` OnLoaded (+78h): `00AA7170`, then +88h(0, 0, 1.0f).
- `00AC7930` WriteQuad (+B0h, `__thiscall(stream, uv[4], unused, size[2])`): slots
  TL (0,0), TR (w*v,0), BL (0,0.75h), BR (w*v,0.75h) with `00E13004` = 0.75; U lerped to
  `v` across the rect. Reconstructed as `gui_progbar_quad_00ac7930`.
- `00AC7B80` TypeIdInit: type record named `cGuiProgressbar`, parent chain from the Icon
  record (`00AB3180`, `cGuiIcon`).

## Unreferenced code in the segment

A rel32 and absolute-dword scan of the PE (`local/r1_rel32.py`) finds no caller of:

- `00AC0C90`, a nine-slice frame template (`UL/U/UR/L/R/BL/B/BR_Icon` children found with
  `BSP_GuiWidget_FindChildByName`, base vtable `00D5C980` {`__purecall`, `00AC0BD0`},
  derived `00D5C9C8` {`00AC10A0`}). The vtables are referenced only by this constructor.
- `00AC6BE0`, a recursive Lua-table preloader (`Texture` through `00AA2660`, `ModelName`
  through the resource manager, `SoundEfx` through `00A83FD0`); its only call is to itself.

An absent rel32/absolute reference does not exclude a computed call; these are recorded
as unreferenced, not as dead.

## Evidence boundary and open items

- Arithmetic follows the native operation order. The C++ runs at the build's float
  precision; the x87 extended intermediates (scrollbar slider, progbar value round trip,
  Catmull-Rom sums) are not reproduced bit for bit, and there is no fixture.
- Renderer objects (buffer creation, locks, draw sections, materials) are host contracts,
  not reproduced.
- Open: `00AB0520` (Movie subtitles, 2448 bytes of Lua and STL) remains evidence-only, as in
  `docs/GAME_MOVIE_PLAYER.md`; `00AC8320`; the ten-sample arc weight when the buffers are
  not rebuilt; whether layouts in this installation use `Curve` with `PosArray` at all.
