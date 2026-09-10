# The GUI Icon widget

Addresses: 00AB5C60, 00AB5D30, 00AB2B70, 00AB3310, 00AB66C0, 00AB1710,
00AB1110, 00AB10F0, 00AB10D0, 00AB1680, 00AB17B0, 00AB24B0, 00AB2600,
00AB3B30, 00AB3CB0, 00AB1860, 00ACCE20, vtable 00D5C4C0.

`Icon` is the class the page loader instantiates for any `GuiScreen` key whose
suffix is `Icon` (docs/GUI_LAYOUT_LOADER.md's type table: id 6, allocator thunk
`00AA1410`, 138h bytes). It is the stateful image class already partly covered
by docs/GUI_GEOMETRY_DISPATCH.md, which recovered its rebuild path
(`00AB3CB0`) and its quad writer (`00AB1860`) without naming the class. This
packet recovers the rest: the layout, the state record, the property set, the
add-state path and the state/filter selection rules.

`Icon` is by far the most used widget in the shipped pages: 701 of the 1520
class-suffixed tables across the 97 installed `interface/*.lua` files.

## Correction to the packet's address list

`00ACCE20` is not part of `Icon`. It is the **Model** widget's property reader
(type id 9, 11Ch bytes): `__thiscall(this, reader*)`, RET 4, base call to
`00AAA710` and then `ModelName` into +10Ch, `ModelTextureOverride` into +114h,
`ScaleVector` (type 5, a float3, default (1,1,1)) into +0F4h and
`RotationEuler` (type 5, default (0,0,0)) into +100h, ending in
`00ACCCF0(+10Ch, +114h)`. It was analyzed only far enough to identify it and is
left to a Model packet.

## Class shape

Constructor `00AB5C60`, `__thiscall(this)`, RET 0 (the type argument is a
constant, not a parameter). It calls the base constructor `00AA9390` with 6,
installs vtable `00D5C4C0` and writes the derived defaults. Copy constructor
`00AB5D30`, `__thiscall(this, const Icon*)`, RET 4, installs the same vtable,
copies the state and calls vtable +80h. Both are reached from the allocator
thunk `00AA1410`, whose only caller is `00AA6560`
(`BSP_GuiLayout_CreateWidgetOfType`).

Derived fields, all from `00AB5C60`'s stores unless noted:

| Offset | Type | Property | Default | Evidence |
| --- | --- | --- | --- | --- |
| +0ECh | int16 | active state index | -1 | 00AB5C82; read sign-extended, compared unsigned |
| +0F0h | pad | vector allocator | - | the vector object base |
| +0F4h/+0F8h/+0FCh | ptr | states first/last/end | null | 00AB5C88; count is `(last-first) >> 6` |
| +100h | bool | `HasTexture` | true | 00AB5CA5 |
| +101h | bool | `DelayedTextureLoad` | false | 00AB5C9F |
| +104h/+108h | string | `ShaderName` | empty | 00AB5CAB; length-then-data, eight bytes |
| +10Ch | bool | `DynamicVB` | false | 00AB5CB7 |
| +110h | int32 | `PartialDisplayType` | 0 | 00AB5CBD |
| +114h | float | `PartialDisplayRatio` | 1 | 00AB5CC3 |
| +118h..+124h | float[4] | crop left/top/right/bottom | 0,0,1,1 | 00AB5CC9; no property writes it |
| +128h | float | unidentified | 0 | written with +12Ch by 00AB1110 |
| +12Ch | int16 | unidentified | 0 | 00AB5CE3 |
| +130h | float | `AutoRotate` | 0 | 00AB5CEF |
| +134h | bool | cached filter choice | false | 00AB5CE9; 00AB3CB0 compares 00AB2600 against it |

The base part, +0h..+0ECh, is docs/GUI_WIDGET_TRANSFORM.md's contract. The
fields this class touches are pivot +18h/+1Ch, size +20h/+24h, scale
+28h/+2Ch, rotate +48h, the vertex-write gate +74h, the scene node +4Ch, and
the material parameter sources +94h (`cOverbrightFactor`), +0A4h (`cLowColor`),
+0B4h (`cHighColor`) and +0C4h (`cBlendFactor`).

## The state record: 40h bytes

Built on the stack by `00AB66C0` between `[esp+2Ch]` and `[esp+6Ch]`, then
pushed into the vector. The stride is confirmed three separate ways: the `>> 6`
counts at 00AB2B84, 00AB3D6B and 00AB175B, the `index * 40h` at 00AB3D77, and
the `iStack_4 + 0x40` walk in the property writer.

| Offset | Type | Meaning | Init | Evidence |
| --- | --- | --- | --- | --- |
| +00h | ptr | resolved texture, reference counted | null | 00AB674B; 00AB6869 stores the resolve result |
| +04h/+08h | string | `Texture`, deep-copied | empty | 00AB67D8 resize then 00AB67EF memcpy |
| +0Ch..+18h | float[4] | resolved UV left/top/right/bottom | 0,0,1,1 | 00AB6757; written by 00AB1680 |
| +1Ch..+28h | float[4] | authored `UV_LURB` | 0,0,1,1 | 00AB67B5; overwritten from argument 2 at 00AB680F |
| +2Ch/+30h | float2 | `Pivot` | 0,0 | 00AB6787; from argument 3 or widget +18h |
| +34h/+38h | float2 | `Size` | 0,0 | 00AB6793; from argument 4 or widget +20h |
| +3Ch | bool | load pending | false | 00AB679F; set by 00AB3B30 |
| +3Dh | bool | unidentified | false | 00AB67A3; cleared by 00AB3B30 |

Two UV rectangles is the point of the record. +1Ch..+28h is what the page
authored and what the property writer emits again; +0Ch..+18h is what the draw
samples, and it is the *atlas* rectangle, not the authored one.

## Properties

Writer `00AB2B70` (vtable +1Ch) and reader `00AB3310` (vtable +18h) are both
`__thiscall(this, visitor*)`, RET 4, and both call the base first (`00AAAED0`
and `00AAA710`). The visitor is the loader's variant interface: +4h begin
scope, +8h end scope, +0Ch scalar property, +10h indexed element, +14h key
present. Each argument is an eight-byte `{int32 type, value}` variant pushed on
the stack; the property call takes three of them (name, destination, default)
and is RET 18h, the presence test takes one and is RET 8. Type codes seen here
are 0 string, 1 int32, 2 float, 3 bool, 5 float3, 6 float2.

Read order in `00AB3310`, with the default each is compared against:

| Key | Type | Field | Default |
| --- | --- | --- | --- |
| `DynamicVB` | bool | +10Ch | false |
| `HasTexture` | bool | +100h | true |
| `ShaderName` | string | +104h | "" |
| `PartialDisplayType` | int32 | +110h | 0 |
| `PartialDisplayRatio` | float | +114h | 1 |
| `DelayedTextureLoad` | bool | +101h | false |
| `AutoRotate` | float | +130h | 0 |
| `States` | array | see below | absent |

`States` is integer-keyed, so the base reader skips it (docs/GUI_LAYOUT_LOADER.md)
and this class walks it: presence test, begin scope, then a `[1] = {...}`,
`[2] = {...}` loop that stops at the first missing index. Each entry reads
`Texture` (default ""), `Size` (float2, default the widget's +20h), `Pivot`
(float2, default the widget's +18h) and, only when the `UV_LURB` key exists,
four float elements at indices 1..4 through visitor +10h. The entry then calls
this object's own vtable +0A4h and frees the temporary texture string.

The writer at `00AB2B70` mirrors that and adds one quirk worth recording: when
the widget has exactly one state, the active index is 0 and the widget size
equals that state's native size, it zeroes the widget's own +20h/+24h across
the base call so the base does not emit a redundant `Size` (00AB2B8B..00AB2BE7).
It also emits `ShaderName` only when the length is non-zero, and `UV_LURB` only
when the authored rectangle differs from (0,0,1,1).

## Adding a state: 00AB66C0

`__thiscall(this, const NativeString* name, const float uv[4],
const float2* pivot, const float2* size)`, RET 10h, returning the new index in
EAX. It has no Ghidra function; it is reached only through vtable +0A4h.

1. Pivot from argument 3 or, when null, widget +18h/+1Ch (00AB66E0). Size from
   argument 4 or, when null, widget +20h/+24h (00AB670F).
2. Zero the 40h-byte record, set both UV rectangles to (0,0,1,1).
3. Deep-copy the name: `BSP_NativeString_Resize` with the source length, then
   `memcpy` of that many bytes. A source pointer equal to the record's own
   string is skipped (00AB67AF).
4. Copy argument 2's four floats into the authored rectangle (+1Ch..+28h).
5. If `DelayedTextureLoad` is set, hand the record to `00AB3B30`: it sets +3Ch,
   clears +3Dh and loads `interface/textures/common/transparent.tga` through
   the texture manager `00F8D394` virtual +64h, taking a reference. The
   resolved rectangle stays (0,0,1,1) because this path never reaches
   `00AB1680`.
6. Otherwise call `00AA2660` (`BSP_GUI_ResolveTextureAndAtlasUV`) with ECX set
   to the GUI manager from `004C12B0`: the name, a *fresh* (0,0,1,1) rectangle,
   the size pair, and scale 1.0. The returned texture goes to record +00h and
   the rectangle it filled goes through `00AB1680`.
7. If the size pair is still (0,0) after the resolve, take the texture's native
   extent over the resolved rectangle through `00AB17B0`.
8. Push the record and return `count - 1`.

The atlas contract is entirely `00AA2660`'s, already reconstructed as
`bsp::resolve_gui_texture_00aa2660` in src/gui_texture.cpp over the atlas of
docs/APP_INIT_WORLD_EFFECTS.md: an atlas hit copies the item's float UVs from
+14h..+20h and, when both size components are zero, sets the size from the
texture's unsigned dimensions scaled by 960/720; a miss loads the file and
leaves UV and size alone. Nothing about that is re-derived here.

### The authored UV_LURB is a flip flag, not a crop

`00AB1680`, `__thiscall(record, float l, float t, float r, float b)`, RET 10h,
stores the four arguments into +0Ch..+18h and then applies exactly two swaps:

- authored top == 1 and authored bottom == 0 swaps the resolved top/bottom pair
  (00AB1688..00AB16CE);
- authored left == 1 and authored right == 0 swaps the resolved left/right pair
  (00AB16D3..00AB16FA).

Both tests are exact equality through the `UCOMISS`/`LAHF`/`TEST AH,44h`/`JP`
idiom, so an unordered operand takes the "not equal" branch. Any other authored
rectangle, a genuine fractional sub-rectangle included, is stored at +1Ch..+28h
and re-serialised but has no effect on what is sampled. Named `UV_LURB` for
left, up, right, bottom; the string is at 00D5C3BC. 33 of the shipped Icon
tables author it.

## Sizes and the filter choice

`00AB17B0`, `__fastcall(float2* out, texture* tex, const float uv[4])`, RET 4,
returns `out`. It reads the texture's dimensions through virtuals +48h and
+4Ch, converts them as *unsigned* (FILD plus the conditional 4294967296.0 add
at 00AB17CE and 00AB17F3), divides by the doubles 960.0 (00CEC380) and 720.0
(00CEF1B8), and multiplies by the UV extents taken through `AND 7FFFFFFFh` on
the stored float. The sign mask is why a flipped rectangle still measures
positive.

`00AB24B0`, `__thiscall(record)`, RET 4, answers "does this record differ from
its texture's native extent": true when the native width is below 0.0364583
(00D5C3B8, which is 35 texels at 960), or when either axis differs by more than
1e-4 (the double at 00D7A268); false when the record is drawn one-to-one.

`00AB2600`, `__thiscall(this)`, RET, returns the material choice byte: 0 for
`guidefault_point.mshd`, non-zero for `guidefault.mshd`. It returns 0 when the
widget has no texture, when `ShaderName` is set, or when the record is drawn at
its native extent; it returns 1 when the platform byte at +14h of `0109CF04` is
clear, when the widget is rotated, or when either scale axis is not 1. The
readable summary: point sampling is used exactly when the icon is drawn
unrotated, unscaled and at one texel per texel, above a 35-texel width.

## Selecting a state

`00AB1710`, `__thiscall(this, int16 index, int32 partialType, float ratio)`,
RET 0Ch, no Ghidra function. It clamps the ratio to [0,1] with two `COMISS`/`JA`
tests, so NaN passes through unchanged, then decides whether to rebuild:

- index differs and is in range (unsigned compare against the count), or index
  differs and the widget has no texture: rebuild;
- otherwise rebuild only when `PartialDisplayType` or the clamped ratio changed
  (the equality test at 00AB1786 treats an unordered comparison as changed, so
  a NaN ratio always rebuilds);
- on rebuild it writes +114h and +110h and then calls vtable +80h with the
  index; on no rebuild it writes nothing at all.

An index that differs, is out of range and belongs to a textured widget is not
rejected here; it only survives if the type or ratio also changed, and then the
rebuild's own range check fails. That asymmetry is in the listing, not an
interpretation.

Three vtable slots wrap it: +78h (`00AB10F0`, RET 0) recomposes the transform
through `00AA7170` and then selects state 0 with type 0 and ratio 1; +84h
(`00AB1110`, RET 0Ch) forwards its first argument with type 0 and ratio 1 and
then stores its other two arguments into +128h and +12Ch; +8Ch (`00AB10D0`,
RET 0) re-selects the *current* index, which is the "the geometry is stale,
rebuild it" entry point.

The two texture setters that reach the same rebuild from outside the class are
already named: `00AB44B0` (`BSP_GUI_SetStateTexture`) and `00AB2690`
(`BSP_GUI_SetStateTextureAndUV`), both RET 0Ch, documented in
docs/ATLAS_GEOMETRY.md.

## The rebuild

Vtable +80h is `00AB3CB0`, `__thiscall(this, int16 index)`, RET 4, covered by
docs/GUI_GEOMETRY_DISPATCH.md. What this packet adds is the state indexing and
the field copies:

- it stores the index into +0ECh first (00AB3CD8), so the index is committed
  before anything can fail;
- `HasTexture` clear: texture slot 0 is null, the UV rectangle is (0,0,1,1),
  the size is the widget's own +20h, and the material is `guifade.mshd`;
- `HasTexture` set: the record is `first + (int16)index * 40h` after an
  unsigned range check that a -1 index always fails; texture slot 0 takes
  record +00h; the UV rectangle is record +0Ch..+18h; record +2Ch/+30h is
  copied into widget +18h/+1Ch and record +34h/+38h into widget +20h/+24h
  (00AB3DBE..00AB3DD3), so **the state's pivot and size overwrite the widget's
  own on every rebuild**;
- the quad writer at vtable +7Ch takes the stream, the UV rectangle, the four
  crop floats from +118h..+124h, and the size pair; it is skipped entirely when
  widget +74h is set;
- the material name is the `ShaderName` override when it is set and differs
  from the cached one, otherwise the fade/default/point choice above; the four
  registered parameters are `cOverbrightFactor` (+94h, float), `cLowColor`
  (+0A4h, float4), `cHighColor` (+0B4h, float4) and `cBlendFactor` (+0C4h,
  float), from the string lengths 17, 9, 10 and 12 at 00AB3DF6..00AB3E0F;
- the section is primitive type 5, four vertices, two primitives.

## The installed pages

Read-only survey of `interface/*.lua` under the installed game, by
`local/icon_page_scan.py` (not committed). It tracks the enclosing widget class
with a brace stack, so every count below is attributed to the `Icon` table it
sits in. It is textual, not a Lua evaluation, so pages that build tables
through `_common.lua` helpers contribute only the keys they spell out.

| Measure | Value |
| --- | --- |
| `interface/*.lua` files | 97 |
| files containing an `Icon` table | 66 |
| `Icon` tables | 701 |
| `States` keys inside `Icon` tables | 698 |
| `UV_LURB` keys inside `Icon` tables | 33 |
| `Texture` assignments inside `Icon` tables | 1399 |
| distinct `Texture` values (separator and case folded) | 472 |
| `ShaderName` assignments inside `Icon` tables | 7 |
| distinct `ShaderName` values | 4 |

Every distinct texture value ends in `.tga`. By top-level directory the
distinct values are `gui` 245, `common` 138, `fe` 80, `bsj_textures` 6 and
three bare file names. The four `ShaderName` overrides are
`guidefault_noalpha.mshd` (3 uses), `GuiFontBilinear.mshd` (2),
`guimultiply.mshd` (1) and `minimap_terrain.mshd` (1) - so 694 of the 701
`Icon` tables take the built-in fade/default/point choice, and the override is
a rounding error in practice. Counted across every widget class the same
scanner sees 1520 class-suffixed tables, case folded: `Icon` 701, `Text` 417,
`Group` 204, `FrameBox` 125, `Listbox` 22, `ClipBox` 20, `Section` 19, `Movie` 5,
`Model` 4, `Sound` 2, `AnimIcon` 1. Those totals are larger than
docs/GUI_LAYOUT_LOADER.md's per-type figures because that doc counts widgets
actually built from the 69 statically parseable pages, not authored keys across
all 97.

## Reconstruction

`include/bsp/gui_icon.hpp` and `src/gui_icon.cpp` carry the layout as documented
structs, the selection rules as pure functions and the add-state path over an
injected host, in the style of `bsp::run_application_frame`. Reused rather than
duplicated: `bsp::GuiUvRect` and `bsp::GuiQuadParameters` from gui_geometry.hpp,
`bsp::GuiWidgetTransform` and `bsp::GuiWidgetSize` from gui_widget.hpp, and
`bsp::resolve_gui_texture_00aa2660` behind `GuiIconHost::resolve_texture`. No
texture, atlas, material or render-queue code is reimplemented.

The float rules are semantic ports under the project's strict floating-point
settings, not bitwise ports. `gui_icon_native_size_00ab17b0` does its division
in double and its multiply in float, matching the native x87 sequence in shape
but not in intermediate precision; the native uses 80-bit intermediates and a
different spill order, so last-bit equality is not claimed. The two exact
comparisons in `00AB1680`, the sign mask in `00AB17B0` and `00AB24B0` and the
NaN behaviour of the ratio clamp are reproduced deliberately because they are
behavioural, not numerical.

Per-routine state:

| Address | Name | State |
| --- | --- | --- |
| 00AB5C60 | constructor | analyzed; layout reconstructed |
| 00AB5D30 | copy constructor | analyzed |
| 00AB2B70 | property writer | analyzed |
| 00AB3310 | property reader | reconstructed (field assignment only), build-tested |
| 00AB66C0 | add state | reconstructed, build-tested |
| 00AB1680 | resolved UV setter | reconstructed, build-tested, fixture-tested |
| 00AB17B0 | native size | reconstructed, build-tested |
| 00AB24B0 | native-size test | reconstructed, build-tested |
| 00AB2600 | filter choice | reconstructed, build-tested |
| 00AB1710 | select state | reconstructed, build-tested |
| 00AB1110, 00AB10F0, 00AB10D0 | select-state wrappers | analyzed |
| 00AB3B30 | delayed placeholder | analyzed |
| 00AB3CB0 | rebuild | analyzed; its quad setup reconstructed |
| 00ACCE20 | Model property reader | analyzed only, not Icon |

All counts in the survey table are installed-file-checked. Nothing here is
game-validated and nothing is ABI-compatible.

## Routines with no Ghidra function

Named from the raw listing only. The orchestrator must create the function
before applying a name. End addresses are the last instruction, inclusive.

| Address | End | Role |
| --- | --- | --- |
| 00AB66C0 | 00AB6942 | add state, vtable +0A4h, RET 10h |
| 00AB1710 | 00AB17A7 | select state, vtable +88h, RET 0Ch |
| 00AB1110 | 00AB1147 | select state and store +128h/+12Ch, vtable +84h, RET 0Ch |
| 00AB10D0 | 00AB10E2 | re-select current state, vtable +8Ch, RET 0 |

`python tools/bsp.py ghidra flow` reports zero gaps for 00AB2B70, 00AB3310,
00AB3CB0 and 00AB5C60.

## Uncertainties and what remains

- +128h and +12Ch are written together by vtable +84h and read by nothing this
  packet reached. +3Dh in the record is cleared in two places and read nowhere.
- The consumer of `AutoRotate` (+130h) and of `DynamicVB` (+10Ch) is outside
  the class; both are only serialised here.
- The crop rectangle at +118h..+124h has no property, so whatever drives
  partial display writes it from code that was not found.
- `00AB2600`'s platform byte, +14h of `0109CF04`, was not identified. It is the
  same platform object the application frame uses.
- Who clears a record's +3Ch and finishes a `DelayedTextureLoad` state was not
  found; the placeholder assignment is established, the completion is not.
- The vtable slots this packet did not open: +20h..+74h are mostly the base's,
  but +3Ch (`00AB6120`), +40h (`00AB1150`), +44h/+48h (`00AB27F0`/`00AB2820`),
  +6Ch (`00AB4CE0`), +74h (`00AB2540`), +90h (`00AB6430`) and +0A0h
  (`00AB10A0`) are the class's own and were not analyzed.

## Follow-up packets proposed

- `gui_icon_state_accessors`: 00AB1150, 00AB27F0, 00AB2820, 00AB2540, 00AB6430;
  docs/GUI_ICON_STATE_ACCESSORS.md, include/bsp/gui_icon_accessors.hpp. The
  remaining Icon-only vtable slots, to close +128h/+12Ch and record +3Dh.
- `gui_model_widget`: 00ACCE20, 00ACCCF0, 00AA16E0 and the type-9 constructor;
  docs/GUI_MODEL_WIDGET.md, include/bsp/gui_model.hpp. The Model widget the
  packet's third address actually belongs to.
- `gui_delayed_texture_completion`: 00AB3B30's callers and whatever clears
  record +3Ch; docs/GUI_DELAYED_TEXTURE.md. Closes the delayed-load half of the
  add-state path.
- `gui_animicon_widget`: 00AA2340 (type 12, 144h bytes) and the derived writer
  and reader at 00AD3550/00AD3430, which extend this class's property set with
  `Forward` and `PlaybyDefault`; docs/GUI_ANIMICON_WIDGET.md. `00AC7B00` and
  `00AC7A90` are the same pair for the `Movie` class and extend it with
  `DefaultValue`.
