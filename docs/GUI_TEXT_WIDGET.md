# GUI Text widget (class id 3, 1F4h)

Addresses: 00AB9650, 00AB7B60, 00ABB630, 00AB8C30, 00AB72D0, 00ABAED0, 00AB6AD0,
00AB6B50, 00AB6D70, 00ABBF30, 00AB7A40, 00AB7700, 00AB87D0, 00AB79E0, 00AB78A0,
00AC2090, 00AC2DE0 (read only: 00AA9390, 00AAAED0, 00AAA710, 00AA6870, 00AA6980,
00AA7970, 00ABA8D0, 00AB9FD0, 00ABA270, 00A9FAD0, 00A9F4B0, 00AC3570)

## Correction to the proposed packet

`docs/GUI_LAYOUT_LOADER.md` and `reports/gui_layout_loader.json` propose
`gui_text_properties` at `00AC2090` / `00AC2DE0`. Those two addresses are **not**
the Text class. They are the describe/read pair of a different widget class whose
only property is an integer-keyed `Points` array of tag-6 (vec2) entries held in
a `begin/end` pointer pair at `+F0h`/`+F4h`; `00AC2090` walks the pair, `00AC2DE0`
probes ascending integer keys until the visitor's `+14h` predicate fails. That is
the `Curve` class, not `Text`. Both were read and are named accordingly, but they
carry no `Font`, `Align`, `Multiline`, `DefaultText` or `DefaultShadow`.

The real Text class was reached from the class table instead: the id-3
constructor stub `00AA1380` allocates through `00AB79E0` and calls `00AB9650`,
which passes the literal `3` to the base constructor `00AA9390` and installs the
vtable at `00D5C6C8`. Its virtual `+18h` is the property reader `00ABB630` (one
of the 13 callers of `00AAA710`) and its virtual `+1Ch` is the describer
`00AB7B60` (one of the 13 callers of `00AAAED0`).

The class is the same object the font packets call the "text context": the
existing docs `FONT_CONTEXT_OWNERSHIP.md`, `FONT_CONTEXT_MATERIAL_BINDINGS.md`,
`FONT_GEOMETRY.md` and `FONT_LAYOUT_BOUNDARY.md` describe its geometry side.
This packet supplies the authoring side, the field names and the tie between them.

## Vtable 00D5C6C8

32 slots, `+00h`..`+7Ch`. The slots this packet establishes:

| Slot | Target | Role |
| --- | --- | --- |
| `+00h` | `00BD30E0` | shared deleting destructor |
| `+18h` | `00ABB630` | read properties from the page table |
| `+1Ch` | `00AB7B60` | describe properties back out |
| `+4Ch` | `00AB6AD0` | set alpha; also scales the shadow alpha |
| `+50h` | `00AB6B50` | set colour; also scales the shadow alpha |
| `+58h` | `00ABBF30` | set size, then force a text rebuild |
| `+60h` | `00AB87D0` | forward a state to `__Active_Left_Icon` / `__Active_Right_Icon` |
| `+64h` | `00AB6D70` | adjust a bounds rectangle for `Align` / `VerticalAlign` |
| `+70h` | `00AB7A40` | push clip parameters into the main and shadow drawables |
| `+74h` | `00AB7700` | post-load hook: build a placeholder mesh and reset the local bounds |

Slots `+20h`, `+24h`, `+28h`, `+2Ch`, `+30h`, `+34h`, `+38h`, `+3Ch`, `+40h`,
`+44h`, `+48h`, `+54h`, `+5Ch`, `+68h`, `+6Ch`, `+78h`, `+7Ch` point at base
implementations in `00A9Exxx`/`00AA6xxx`-`00AA8xxx` and were not analysed here.

## Layout, 1F4h bytes

`00AA9390` builds `+00h`..`+EBh` (the `Group` size, `0ECh`); the Text extension is
`+ECh`..`+1F3h`. Every default below is the store in `00AB9650`. Strings are the
repository's `NativeString`, a `{ size_t length; char* data; }` pair, so a name
that ends in a single offset covers two dwords.

| Offset | Type | Lua key | Default | Evidence |
| --- | --- | --- | --- | --- |
| `+ECh` | NativeString (UTF-16) | (runtime) current text | empty | `00ABA270+108`: `[+ECh]*4` is the vertex count |
| `+F4h` | NativeString (narrow) | `DefaultText` | empty | `00ABAED0` compares and caches it; `00AB7CD3` emits `*(+F8h)` |
| `+FCh` | bool | `Multiline` | **true** | reader tag 3; `00ABA8D0` picks `00ABA270` when nonzero |
| `+100h` | int32 | `Align` | 0 (Left) | reader `stricmp` chain; describer inverse |
| `+104h` | int32 | `VerticalAlign` | 0 (Top) | reader `stricmp` chain; `00ABA270` vertical offset |
| `+108h` | FontDescriptor* | (from `Font`) | null | `00AB8C30` stores the registry lookup |
| `+10Ch` | float | `DistanceBetweenLines` | 0 | reader tag 2; `00ABA6xx` line advance |
| `+110h` | int32 | (runtime) line count | 0 | `00ABA270` increments per line |
| `+114h` | float | (runtime) measured width | 0 | both builders; `/960` to normalise |
| `+118h` | bool | `MISColors` present | false | reader stores 1 after the sub-table |
| `+11Ch` | float[4] | `MISColors.Normal` | (0.7,0.7,0.7,1) | ctor `00CE3E18`/`00D7A24C` |
| `+12Ch` | float[4] | `MISColors.Focus` | (1,1,1,1) | ctor |
| `+13Ch` | float[4] | `MISColors.Selected` | (1,1,1,1) | ctor |
| `+14Ch` | float[4] | `MISColors.Disabled` | (0,0,0,0.5) | ctor `00CE3800` |
| `+15Ch` | bool | `Shadowed` | false | reader tag 3; `00AB72D0` |
| `+160h` | int32 | `ShadowPos` | 0 | 1 when the string matches `Front` |
| `+164h` | float | `ShadowOffset` | 0.05 | default literal `00D5C5C0` |
| `+168h` | float[4] | `ShadowColor` | (0,0,0,0.75) | default literal `00E12FD8` |
| `+178h` | float | (runtime) wrapped vertical extent | 0 | `00ABA270` |
| `+17Ch` | int32 | unidentified | 2 | ctor only |
| `+188h` | node* | shadow drawable | null | `00AB6AD0`, `00AB6B50`, `00AB8530` |
| `+18Ch` | ptr | (runtime) first emitted origin | 0 | `FONT_LAYOUT_BOUNDARY.md` |
| `+1C0h` | NativeString | `ShaderName` | empty | reader tag 0; describer guards on the length |
| `+1C8h` | NativeString | `Font` | empty | `00AB8C30` copies into it |
| `+1D0h` | int32 | `DefaultShadow` | -1 | reader tag 1 |
| `+1D4h` | float | alpha-texture scale | 1.0 | `00AB8C30` copies font `+1Ch` |
| `+1D8h` | float | `FontScale` | 1.0 | reader tag 2; the extra layout scale |
| `+1ECh` | refcounted* | cached font shader | null | `00AB8C30` releases it |
| `+1F0h` | bool | unidentified | true | ctor only |

`+180h`, `+184h`, `+190h`..`+1BFh`, `+1DCh`..`+1EBh` are zeroed by the
constructor and belong to the geometry side documented in
`FONT_CONTEXT_OWNERSHIP.md` and `FONT_CONTEXT_MATERIAL_BINDINGS.md`.

**Value tag 1.** `gui_layout_loader.hpp` lists tags 1, 4 and 7 as unrecovered.
`DefaultShadow` recovers tag **1 = signed int32**: the reader pushes tag 1 with
the default `0xFFFFFFFF` and the describer pushes tag 1 with the same default.

## The Lua keys

`00ABB630`, `__thiscall(this, visitor*)`, `RET 4`. It calls
`00AAA710` for the base properties first, then, in source order:

1. `Font`: read as tag 0 into a temporary, then `00AB8C30(this, name)`.
2. `FontScale` -> `+1D8h`, tag 2, default 1.0.
3. `Multiline` -> `+FCh`, tag 3, default **1**.
4. `DistanceBetweenLines` -> `+10Ch`, tag 2, default 0.
5. `DefaultText` -> a temporary, tag 0, default empty.
6. `ShaderName` -> `+1C0h`, tag 0, default empty.
7. `Align` -> a temporary, tag 0. `__stricmp(s,"Left")` yields 0; otherwise the
   case-insensitive helper `00425850` is tried against `Right` (2), `Center` (1)
   and `Justified` (3); an unmatched or absent value leaves 0.
8. `VerticalAlign` -> a temporary, tag 0. `Top` yields 0, then `Bottom` (2) and
   `Center` (1); anything else leaves 0.
9. `MISColors`: if the visitor's `+14h` predicate reports the key present, the
   visitor descends (`+4h`), four tag-8 colours `Normal`, `Focus`, `Selected`,
   `Disabled` are read into `+11Ch`, `+12Ch`, `+13Ch`, `+14Ch` with the
   lazily-built white default at `00F8BE38`, `+118h` is set to 1 and the visitor
   ascends (`+8h`).
10. `DefaultShadow` -> a temporary, tag 1, default -1. If it came back different
    from -1 the value goes to `00AB72D0`; only when it is still -1 does the
    reader continue with `Shadowed` (`+15Ch`, tag 3, default false), and only
    when `Shadowed` is true with `ShadowColor` (`+168h`, tag 8, default
    `(0,0,0,0.75)`), `ShadowPos` (tag 0, default empty; `+160h` becomes 1 exactly
    when it matches `Front`) and `ShadowOffset` (`+164h`, tag 2, default 0.05).
    `DefaultShadow` and the explicit shadow keys are therefore mutually
    exclusive, with `DefaultShadow` winning.
11. `DefaultText`, if non-null, goes to `00ABAED0(this, text, 1)`. The literal
    `push 1` at `00ABBD90` is the localisation flag, so an authored `DefaultText`
    is always run through the localisation table.
12. The temporaries are returned to the sized pool.

`00AB7B60`, `__thiscall(this, visitor*)`, `RET 4`, is the inverse. It emits a
property only when `00BD5680` reports the field differs from the same default,
except `Align` and `VerticalAlign`, which are always emitted. `Align` maps back
0 -> `Left`, 1 -> `Center`, 2 -> `Right`, 3 -> `Justified`; `VerticalAlign` maps
0 -> `Top`, 1 -> `Center`, 2 -> `Bottom`.

**Describer defect.** In the `ShadowPos` emit the compare `CMP dword ptr
[EDI+160h], 1` at `00AB8028` is followed only by flag-preserving `MOV`s and then
the call at `00AB804E`; no conditional consumes it. The describer always writes
the literal `Behind` (`00D5C604`), so a widget authored with `ShadowPos =
"Front"` does not round-trip. The reader is unaffected.

## Font resolution

`00AB8C30`, `__thiscall(this, const NativeString* name)`, `RET 4`. It returns
immediately when the name matches `+1C8h` case-insensitively. Otherwise it copies
the name into `+1C8h`, takes the font registry singleton (`BSP_FontSystem_GetRegistry`),
looks the name up with `BSP_FontRegistry_FindByName` (`00AC3570`, the routine
`find_font_00ac3570` already models: length equality then a case-insensitive
compare, first match), stores the borrowed record at `+108h`, copies the record's
`+1Ch` float into `+1D4h` (1.0 when the lookup failed), and releases and clears
the cached shader at `+1ECh`. It does **not** re-run the layout; the text is only
re-laid-out on the next text or size change. Because the reader sets `Font`
before `DefaultText`, an authored page always gets the right font.

`00AB72D0`, `__thiscall(this, int preset)`, `RET 4`, is the `DefaultShadow`
expander. It stores the preset at `+1D0h` and returns early when unchanged.
Preset -1 clears `Shadowed`. Any other preset sets `Shadowed`, `ShadowColor` =
`(0,0,0,0.75)`, `ShadowPos` = 0 and `ShadowOffset` = 0.05, and preset 1
additionally overwrites the colour with `(1,1,1, 00CEE07C)` = `(1,1,1,0.75)`.
Presets 0 and 2 are therefore identical.

## Setting the text at runtime

`00ABAED0` `BSP_GuiText_SetLocalisedSource`,
`__thiscall(this, const NativeString* source, bool localise)`, `RET 8`, 97 callers,
all screen code.

1. Return immediately when `source` matches the cached narrow text at `+F4h`
   case-insensitively. The comparison is on the **source key**, not the resolved
   text, so a language change alone does not invalidate a widget.
2. Copy `source` into `+F4h`.
3. `localise == 0`: widen the bytes directly with `004C5E60`.
   `localise != 0`: `00A9FAD0` splits the key on `|` with `_strcspn` and resolves
   each piece with `00A9F4B0`, which strips one leading `^`, treats a leading `.`
   as a marker that suppresses the map lookup, resolves the remainder and then
   substitutes the `#` markers. What the `^` prefix means is still open; it is
   `docs/APP_INIT_LOCALE.md`'s open question 5 and this packet did not settle it.
   No shipped page uses it (see the installed-file check).
4. Hand the UTF-16 result to `00ABA8D0`, then free the temporary.
5. Call virtual `+50h` with `this + 50h`. `+50h` is the base widget's four-float
   `Color` property, so the tail is `SetColor(this, this->Color)`: it re-pushes
   the colour and, through `00AB6B50`, re-derives the shadow alpha. That resolves
   the "virtual `+50` with `context+50`" note in `FONT_CONTEXT_OWNERSHIP.md`.

The re-layout triggers are therefore: a changed source key (this routine),
`00AB6AB0` for a UTF-16 caller, `00ABB000` for the ellipsis path, and virtual
`+58h`. `00ABBF30`, `__thiscall(this, size)`, `RET 4`, calls the base size setter
`00AA7970` and then `00ABB1D0`, which blanks `+ECh` and re-submits the saved copy
so the wrap is recomputed against the new width.

## Layout and drawing, as a contract

The layout itself belongs to the `font_single_line_layout` and
`font_wrapped_layout` packets. What this class contributes:

- `00ABA8D0` selects the builder from `+FCh`: **zero selects the single-line
  builder `00AB9FD0`, nonzero the wrapped builder `00ABA270`**. Because `+FCh`
  defaults to true, `Multiline` is an opt-**out** of wrapping.
- The container width is the base widget's `Size.x` at `+20h`, multiplied by the
  double 960 at `00CEC380` and truncated to an integer. The container height is
  `Size.y` at `+24h`.
- The horizontal scale is `+1D8h` (`FontScale`), applied on top of the font's own
  DAT scaling.
- Single-line placement uses `+100h`: mode 0 and mode 3 start at x = 0, mode 2
  subtracts the measured width from the container width, mode 1 halves that
  difference. Mode 3 has no effect without line breaks.
- Wrapped placement uses `+100h` per line, with mode 3 distributing the slack
  across the line's spaces and falling back to left on the last line and on LF.
  It accumulates the per-line maximum into `+114h`, counts lines into `+110h`,
  and advances the pen by `(+10Ch + k1 - k2) * lineHeight` with the constants at
  `00D7A210` and `00CE6618`. Those two are read as x87 quantities of ambiguous
  width in the pseudocode; the exact advance was not confirmed and belongs to the
  wrapped-layout packet.
- Vertical placement uses `+104h` against the extent stored at `+178h`: mode 0
  leaves the origin, mode 1 centres it in `Size.y`, mode 2 bottom-aligns it.
- `00AB6D70`, `__thiscall(this, float* left, float* top, float* right, float* bottom)`,
  `RET 10h`, applies the same two enums to a caller-supplied rectangle so that
  hit-testing and bounds match the drawn text. Horizontally: mode 0 sets
  `*right = *left + w + p`, mode 2 sets `*left = *right - w - p`, mode 1 centres
  the pair around their midpoint, where `w = +114h / 960` and `p` is the double
  at `00D5C5C8`, about 0.012. Vertically it dispatches on `+104h` and calls
  `00AB6BD0`.
- Colour, glyph quads, the shadow section copy and the material bindings are
  already recorded in `FONT_CONTEXT_OWNERSHIP.md` (shadow byte `+15Ch` gates the
  attachment, `+168h`..`+174h` reach the shadow material, `+164h` is scaled by
  the double 720 and placed by `+160h`) and in
  `FONT_CONTEXT_MATERIAL_BINDINGS.md`. This packet does not restate them.
- `00AB6AD0` (`+4Ch`) and `00AB6B50` (`+50h`) both call the base implementation
  (`00AA6980`, `00AA6870`) and then, when `+188h` holds a shadow drawable,
  multiply `+174h` (the shadow alpha) by the incoming alpha or by the incoming
  colour's `w` lane and write the product into the shadow material parameter at
  `+0Ch`. The shadow alpha is therefore always relative to the widget alpha.
- `00AB7A40` (`+70h`) pushes the clip parameters of the main drawable `+4Ch` and
  the shadow drawable `+188h` into `BSP_UIContext_RegisterClipParameters`, then
  recurses into every child's `+70h`.

## Installed-file check

All 97 `interface/*.lua` files in the installation were scanned read-only for
keys whose suffix after the last underscore names a widget class. Only the
`["Key"] = literal` form is counted; a value written as an expression or supplied
by a `_common.lua` helper is not, and those are the same pages the layout
loader's subset parser rejects.

| Measure | Count |
| --- | --- |
| `*_Text` keys | 416 |
| files containing at least one | 67 |
| `Font` keys | 394 |
| `Align` keys | 388 |
| `VerticalAlign` keys | 385 |
| `Multiline` keys | 141 |
| `DefaultShadow` keys | 348 |
| `DefaultText` string literals | 315 |
| of those beginning with `^` | 0 |
| `MISColors` keys | 60 |
| `ShaderName` keys on any class | 68 |
| `Shadowed` / `ShadowPos` keys | 5 / 5 |
| `DistanceBetweenLines` keys | 4 |
| `FontScale` keys | 2 |

Text is the second most common class after Icon (701) and ahead of Group (211)
and FrameBox (123). `docs/GUI_LAYOUT_LOADER.md` reports 248 Text widgets; that
figure counts only the 70 pages its subset parser accepts, so the two numbers
agree in kind.

Distinct authored values:

| Key | Values |
| --- | --- |
| `Font` | `Arial16` 184, `Arial15` 80, `Viper19` 72, `Arial18` 50, `Arial20` 8 |
| `Align` | `Left` 213, `Center` 101, `Right` 62, `Justified` 10, `left` 1, `right` 1 |
| `VerticalAlign` | `Center` 203, `Top` 155, `Bottom` 24, `top` 3 |
| `Multiline` | `false` 139, `true` 2 |
| `DefaultShadow` | `0` 345, `2` 2, `1` 1 |

Three consequences. The lower-case `left`, `right` and `top` spellings prove the
case-insensitive comparison is load-bearing: five widgets would lose their
alignment under a case-sensitive reader. `Multiline` is only ever written to turn
wrapping **off**, so 275 of the 416 Text widgets take the wrapped builder by
default. `DefaultShadow` is authored on 348 widgets and the explicit `Shadowed`
key on 5, which matches the reader's mutual exclusion; since presets 0 and 2 are
identical, 347 of the 348 get the black `(0,0,0,0.75)` shadow and one gets white.
Only five fonts are referenced in total, all resolvable through the registry
`find_font_00ac3570` already models.

## Callers and callees

- `00AB9650`: called by the id-3 constructor stub `00AA1380` and by
  `00AB98F0`. Calls `00AA9390` with the literal 3 and `00AB8530`.
- `00ABB630`: single caller `00AAA710`, through vtable `+18h`. Calls `00AAA710`,
  `00AB8C30`, `00AB72D0`, `00ABAED0`, `__stricmp`, `00425850`,
  `BSP_SizedStoragePool_*`.
- `00AB7B60`: single caller `00AAAED0`, through vtable `+1Ch`. Calls
  `00AAAED0`, `00BD5680` and the visitor's `+4h`/`+8h`/`+0Ch`.
- `00AB8C30`: callers `00ABB630` and the font-context code. Calls
  `BSP_NativeString_EqualsInsensitive`, `BSP_NativeString_Resize`, `_memcpy`,
  `BSP_FontSystem_GetRegistry`, `BSP_FontRegistry_FindByName`,
  `InterlockedDecrement`.
- `00ABAED0`: 97 callers, all screen code. Callees as listed in the ledger.
- `00AB79E0` -> `00AB78A0`: a fixed-size pooled allocator guarded by a critical
  section at `+0Ch`, with a free list and 0x7E84-byte chunks. It is tagged
  `cg_static_dtor_stub` in the Ghidra database; the tag is wrong, `00AA1380`
  uses the returned pointer as the new object.

## Uncertainties

- `+17Ch` (constructor value 2) and `+1F0h` (constructor value true) have no
  reader or writer in the routines examined.
- Where `MISColors` is consumed is unknown. No routine in the Text class reads
  `+11Ch`..`+158h`; the four names (`Normal`, `Focus`, `Selected`, `Disabled`)
  and the 60 authored occurrences suggest a screen-side state selector.
- The wrapped line advance constants at `00D7A210` and `00CE6618` are read at
  ambiguous width by the decompiler and were not confirmed in the listing.
- Virtual `+60h` (`00AB87D0`) finds `__Active_Left_Icon` and
  `__Active_Right_Icon` children and, when both exist, calls their virtual `+34h`
  with its own argument. Its role is provisional.
- Virtual `+74h` (`00AB7700`) builds a mesh object through
  `BSP_GeneratedModel_SetGeometry` and then calls
  `BSP_GuiWidget_SetLocalPositionAndBounds` with a zero vector. Why a Text widget
  needs a placeholder mesh at load time was not established.
- The `^` prefix rule in `00A9F4B0` remains the locale packet's open question.
  This packet only establishes that the reader always requests localisation and
  that no shipped page uses the prefix.

## What remains

- The `Curve` class at `00AC2090` / `00AC2DE0` was only skimmed; its `Points`
  array and the `+F0h`/`+F4h` pair deserve their own packet.
- The `MISColors` consumer.
- Whether `00AB8C30` failing to trigger a re-layout is observable: changing
  `Font` at runtime without changing the text leaves stale glyph geometry.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `gui_text_mis_colors` | 00AB7B60, 00ABB630 callers, `+11Ch`..`+158h` readers | docs/GUI_TEXT_MIS_COLORS.md | Who selects among `MISColors.Normal/Focus/Selected/Disabled` and how the chosen colour reaches virtual `+50h`. |
| `gui_curve_widget` | 00AC2090, 00AC2DE0 | docs/GUI_CURVE_WIDGET.md, include/bsp/gui_curve.hpp | The `Points` class mis-identified as Text: the tag-6 point vector at `+F0h`/`+F4h`, the integer-key probe loop, and its instance size and class id. |
| `gui_text_active_arrows` | 00AB87D0, 00AA8530 | docs/GUI_TEXT_ACTIVE_ARROWS.md | Virtual `+60h`: what state the `__Active_Left_Icon` / `__Active_Right_Icon` children receive through their virtual `+34h`, and which screens author them. |

## State reached

| Address | State |
| --- | --- |
| `00AB9650` | analyzed, reconstructed (layout and defaults), build-tested, installed-file-checked |
| `00ABB630` | analyzed, reconstructed, build-tested, installed-file-checked |
| `00AB7B60` | analyzed, reconstructed (the enum inverses and the defect), build-tested |
| `00AB8C30` | analyzed, reconstructed, build-tested |
| `00AB72D0` | analyzed, reconstructed, build-tested |
| `00ABAED0` | analyzed, reconstructed (the caching and flag rules), build-tested |
| `00AB6AD0` | analyzed (listing only) |
| `00AB6B50` | analyzed (listing only) |
| `00AB6D70` | analyzed, reconstructed (the horizontal case), build-tested; listing only in Ghidra |
| `00ABBF30` | analyzed (listing only) |
| `00AB7A40` | analyzed |
| `00AB7700` | analyzed |
| `00AB87D0` | analyzed (provisional) |
| `00AB79E0`, `00AB78A0` | analyzed |
| `00AC2090`, `00AC2DE0` | analyzed (identified as a different class) |
