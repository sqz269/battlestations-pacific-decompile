# FrameBox widget (type 18)

`GuiFrameBoxWidget` and the functions in `gui_framebox.hpp` reconstruct the
derived state, evaluated property reader, startup hooks, state selection and
nine-slice geometry for `FrameBox`. The native type is 11Ch bytes, with a
0ECh-byte base. These are semantic C++ interfaces, not binary replacements.
Every descriptive name is a hypothesis. Ghidra was kept read-only for this
packet; the reviewed name ledger is ready for the integrator's annotation batch.

## Evidence and original ABI

All live queries verified project `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe` through `tools/bsp.py ghidra`. Exports are in the
shared ignored `exports/bsp/functions/<address>/`. Ranges below include the
**last instruction address**, rather than the last instruction's final byte.

| Address range | Native interface | Recovered behavior |
| --- | --- | --- |
| 00AA1A40..00AA1AC3 | ECX optional source; EAX result; RET | Requests 11Ch, then default/copy constructor |
| 00AD0280..00AD0285 | RET through tail target | Loads pool00F8C188 into ECX and jumps00AD0050 |
| 00AD2600..00AD2697 | ECX destination; EAX same; RET | Base(type18), vtable00D5D130, derived defaults |
| 00AD27A0..00AD2873 | ECX destination, stack source; RET4 | Base clone, state copy, clear shader, rebuild, clear+74, constructed hook |
| 00AD08E0..00AD0C3A | ECX widget, stack reader; RET4 | Base properties/children, then derived properties/states |
| 00ACF8F0..00ACF9A7 | ECX widget; RET; virtual+74 | Associate new mesh if+74 is clear, then set position/bounds to zero |
| 00ACEB50..00ACEB67 | ECX widget; RET; virtual+78 | Base loaded78, then virtual+84(0) |
| 00ACF070..00ACF0BD | ECX widget, stack state; RET4; virtual+84 | Select only changed valid state, or changed untextured state |
| 00AD0D80..00AD157E | ECX widget, stack signed16 state; RET4; virtual+80 | Stream/material/section rebuild |
| 00ACF9B0..00ACFF66 | ECX widget, stack stream,size; RET8; virtual+7C | Nine rectangles,54 vertices |
| 00ACF260..00ACF483 | Stack stream,index pointer,position rectangle,UV rectangle; RET10h | TL,TR,BR,TL,BR,BL triangle-list vertices |
| 00ACEB70..00ACEBD7 | ECX out-size, EDX texture; EAX out; RET | Full pixel dimensions divided by960,720 |
| 00AD2B30..00AD2D4C | ECX widget, stack string,pivot,size; RET0Ch | Add 2Ch-byte texture/pivot/size/UV state; low16 EAX index |

`00AD2B30` has **no Ghidra function** in the verified program. Its complete
inclusive instruction range above was decoded from the installed PE using
`bsp.py disasm-raw`; it must remain a fragment until an authorized function
creation/annotation batch. The first state getter also has no Ghidra function:
`00AD26D0..00AD26D7`, ECX widget, returns signed16 bits in AX. The checked
state-record accessor at `00AD2750..00AD2792` is likewise a raw range, ECX widget,
stack signed16 index, EAX record, RET4. `00AD2A80` (virtual+8C record insertion)
and `00ACEB00` (virtual+9C stream getter) were only inspected as dependencies;
they are not claimed reconstructed.

The export's `CG_static_dtor_stub_00ad0280` label is incorrect: its two
instructions load the FrameBox allocation pool and tail-call its allocator.
The pool routine's decompiler has a suspect `_free` no-return edge. Pool
allocation, SEH/OOM and native intrusive-container ABI remain external; no
allocator pseudocode was compiled. Base allocation/factory, child ownership
and scene binding belong to `GuiWidgetOwner`/the integrator.

## Fields and property reader

The constructor sets signed16 current state+ECh to -1, clears the vector
pointers+F4h/+F8h/+FCh and shader string+104h/+108h, and sets HasTexture+100h.
FrameSizesX+10Ch/+110h receive bits3D6EEE95 (`0.05833299830555916`);
FrameSizesY+114h/+118h receive bits3D9F498C (`0.07777699828147888`).
The copy constructor copies state records/texture ownership, current state,
HasTexture and frame sizes, but starts with an **empty ShaderName**. Its base
clone and rebuild-before-clearing+74 order are not implemented by the pure
derived-state copy helper.

`read_properties_00ad08e0` runs after the owner finishes the base reader. It
reads HasTexture(true), ShaderName(empty), FrameSizesX and FrameSizesY(default
pairs), then visits States indices1,2,... until the first nil. Each entry reads
Texture(empty), Size(base size) and Pivot(base pivot), in that order, then adds
one state. It appends, rather than clearing the existing vector. FrameBox does
not read Icon's UV_LURB, DynamicVB, delayed-load or partial-display properties.

Both overloads consume evaluated values: one takes the actual `GuiTable`
snapshot; the other drives `GuiLuaReader` directly. Neither parses Lua syntax.
The snapshot overload reuses00BD63B0 conversion rules. Invalid non-table state
scopes throw a host error; malformed-table/SEH parity is not claimed.

Add-state always resolves the texture through existing00AA2660 with scale1
and initial UV(0,0,1,1), including when HasTexture is false. It adopts the one
returned reference with a required release callback. If resolution leaves both
size components zero,00ACEB70 supplies the full texture dimensions/(960,720).
That fallback does not multiply by the atlas UV extent. Real atlas resolution
can already have supplied the size. No placeholder textures are fabricated.

## Startup, material and geometry

The class constructed+74 hook creates/associates a fresh mesh on a generated
model only when native widget+74 (`transform.bounds_enabled`) is clear. The
two00B75170 thresholds are **-1.0**, verified at00D7A260 asBF800000. It then
calls the owner's real00AA7DC0 setter with(0,0,0). Loaded+78 calls the base hook
and selects state0 through00ACF070. A same-index selection does nothing; an
invalid signed16 index is ignored when textured. The untextured branch may
select an invalid index, but its geometry writer still requires a valid state
texture if it actually writes vertices. The reconstruction exposes that
failure instead of returning blank geometry.

The shared `rebuild_gui_geometry_00ab3cb0_fragment` performs actual logical
stream locking/unlocking, declaration-based writes, material and texture
registration, retained section/layout publication and base transform/color
callbacks. FrameBox supplies SimpleColor.mvfm, stream flags1,54 vertices,
primitive4 (triangle list), range words(0,54,0,18), and its recovered writer.
Its first section uses ShaderName when nonempty, otherwise GuiDefault.mshd
when textured or guifade.mshd otherwise. Subsequent rebuilds keep that section's
existing material; FrameBox has no Icon point-filter choice. It registers
cOverbrightFactor(widget+94h), cLowColor(+A4h), cHighColor(+B4h),
cBlendFactor(+C4h), base clip bindings and widget ownership, and sets texture0.
Textured rebuilds copy the selected state's size/pivot back into the same base
layout. A set+74 suppresses all position/UV/color writes. Other writes use
white vertex colors. Native recompose occurs after unlock, before material
assignment/layout rebuild; color virtual+50 runs last.

Nine rectangles are written in order: top-left, top-right, bottom-left,
bottom-right, top, left, right, bottom, center. X frame extents are authored
directly. Y frame extents and total height are multiplied by global00E1301C,
whose installed value is0.75. The UV edge extents use those borders divided by
the selected texture's normalized dimensions and atlas spans. The algebraic
cancellations are intentionally not applied: native float stores and x87
division/multiply order are retained. Center extents can be negative, and
zero/invalid dimensions can produce IEEE infinities/NaNs; there is no clamp.
The rectangle writer is a triangle list and therefore does not reuse Icon's
four-vertex strip crop writer.

## Validation and remaining boundaries

`verify-seeds` passed. `scripts/build.ps1` passed MSVC Win32 Release compilation
and both existing CTests. The build supplied the sibling Icon-runtime header
through an ignored compiler include overlay; that source/header must be merged
before this packet. No permanent tests were added.

Ignored `local/framebox_probe.cpp` executes installed `_highlight.lua` in the
real repository Lua5.1.1 host. Its snapshot and direct-reader paths agree on all
four FrameBox records. A relocated original-byte reference for00ACF9B0 and
00ACF260 matches all270 float words for each of the four generated geometries:
zero differences, also for a second nonunit atlas-UV fixture(0.11,0.23,0.87,0.93).
Texture dimensions were explicit128x64 fixture inputs; this is numeric/reader
evidence, not an installed texture, material/D3D9, visibility or game-render
validation. The game installation and saved analysis were not modified.

Required runtime dependencies are the real texture/atlas VFS services, native
texture/COM lifetime projection, compiled material/effect creation, stream
creation, clip/owner binding, generated-model association, and same-owner base
loaded/position/transform/color calls. The shared geometry helper carries
those dependencies explicitly and throws when missing. Native generic copy
allocation, material serialization, state mutation virtuals outside startup,
full binary ABI and gameplay/render parity remain unclaimed.
