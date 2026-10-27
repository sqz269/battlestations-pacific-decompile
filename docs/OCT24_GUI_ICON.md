# Native Icon caller composition, oct24-gui-icon

`GameNativeGuiIconApplication::make_type` is a deferred type6 factory over the
existing `GuiWidgetOwnerRuntime`. It returns the existing
`GuiIconTypeImplementation`; current74, property18, current78, state selection,
rotation/scale/size and frame dispatch still use its one `GuiIconRuntime` and
canonical layout. The optional native geometry capability bypasses its typed
`GeneratedInstanceGeometry`, `LogicalVertexStream`, `LogicalTexture` and
`MaterialCloneState` geometry route entirely. Existing typed callers retain
their previous route when that capability is absent.

This does not activate `FE_initial`, add a switch, create a GUI manager, or
instantiate another page/widget/model/transform/reference domain.
`BSP_NATIVE_GUI_PAGE` remains a future default-OFF selection.

## Actual resource path

The renderer supplies its same `NativeGameGridContext`,
`GuiTextNativeRendererServices`, `NativeStreamCloneServices`, actual-owner
registry and `GuiTextNativeLayoutServices`. The GUI effect application supplies
the existing numeric535320 material factory and its canonical effect/cache/
compiler graph. The actual process material/parameter pools and VFS native
string service are the same objects used by Text.

* Current74 uses the existing AB2563..AB25C3 mesh construction/association
  fragment over the owner's sole actual `NativeModelOwner`, followed by the
  existing AA7DC0 zero-position call in `GuiIconRuntime`.
* AB3CB0 loads `SimpleColor.mvfm` through actual current38, creates four flags1
  vertices through actual current5C, publishes stream0 with B73BB0 and consumes
  vertex/declaration temporary references in native order. The stream uses real
  native logical/physical/HAL ownership and B49980/B49A80 mapping.
* The existing native section acquisition selects/retains mesh+54[0] or creates
  one section. Material reuse follows the saved filter byte; a replacement uses
  `GameNativeGuiEffectApplication::create_material_00535320` and an address-
  stable native name/acquisition frame.
* Parameters borrow the same overbright/low/high/blend fields. Existing raw
  AA9F10 binds the canonical ClipBox/global sources. Raw B18A40 retains the
  supplied actual Icon identity, never the C++ `GuiWidgetOwner` address.
* Slot0 texture binding uses the canonical raw texture. Atlas lookup borrows the
  existing resource provider; misses call actual renderer64/B319B0 with a
  persistent cache frame. Width/height use D61948's B3CE70/B3CE80 saved getters.
  No COM texture or atlas is duplicated.
* The existing x87-qualified AB1860 crop kernel feeds actual mapped bytes using
  the stream's producer-established stride/position/UV/color offsets. Position/
  UV order is0,1,3,2; native white color publication follows. The live E12FD0
  vertical scale is required. Bounds-enabled Icons suppress these writes.
* Unlock precedes AA7220. Existing AB443D..AB4488 publication sets the actual
  section material, consumes temporaries, calls actual current40 layout creation,
  appends a new section and consumes its selected reference. Existing current50
  writes the same raw material diffuse color.

The original Icon is a nonindexed four-vertex triangle strip: it allocates no
index buffer. Indexed Text continues to use the same renderer's real index
factory/mapping services independently.

## Required page identity and lifetime

The page allocation transport must supply a genuine existing raw Icon identity
and its canonical companion borrowing that allocation's actual+04. Admission
requires current D5C4C0, matching count/registry, the same type6 owner and the same
widget runtime. This adapter allocates no competing Icon pool slot, does not
initialize another count, and supplies no substitute terminal callback.

The same authoritative page owner must provide `retain_widget(owner)` tokens
whose `get()==&owner`, keeping the one page/tree/layout and all borrowed
parameter fields alive. A borrowed legacy registry pointer is insufficient.
Token acquisition is deferred until first rebuild, before any borrowed material
parameters. The loader's make_type/current74 occur before its child unique_ptr
is published in the parent tree, and create no material borrow.

Terminal retirement is explicit and serialized:

1. Retire real scene/model/queued-material references while the page authority
   and its parameter token remain alive.
2. Call `release_material_borrow_guard(owner)`. It requires primary node null,
   completed rebuilds and actual Icon+04==1 (only the real creator remains), then
   releases the genuine page token without touching the native count.
3. The authoritative page/type transport destroys that same implementation.
   Its native runtime callback rechecks the retirement conditions before the
   derived fields/texture references die.
4. Call `release_retired_icon(same_layout)` while the authoritative allocation
   transport still retains layout and raw identity. It removes only adapter
   metadata. The real page transport remains responsible for raw Icon
   destruction, flags1 and pool return in their established phases.

The adapter fails explicitly when those providers/phases are absent. Entered
factory/map/publication failures preserve partial actual resources and frames;
they latch process retention and cannot be replayed or normally torn down.

## Evidence and limits

Existing exports/listings inspected: AB2540 and AB3CB0, including renderer
current38/5C capture/release order, AB4114/411E clip/owner binding,
AB41A9..4266 x87 size/pivot stores and AB443D..4495 publication/current50.
The original Icon rebuild is ECX=this, one short carried in a stack DWORD,
RET4 at AB44A9; current74 is ECX=this/no stack/plain RET. Existing property18
AB3310 and loaded78 AB10F0 remain unchanged. Live original D5C4C0 begins
`BD30E0, AB6100, AB1080, AA1CC0`; D61948+48/+4C selects B3CE70/B3CE80.
`config/target.json` and live Ghidra status were verified for `bsp.gpr`,
`/battlestationspacific.exe`. No saved-analysis mutation or newly recovered
body is claimed by this composition.

These are new MSVC Win32 C++ interfaces. They do not reproduce raw Icon
containers/string layout, private stack/FH3/SEH behavior, arbitrary alias/FP
fault behavior, or the whole native widget allocation/destruction ABI. The
AB1860 kernel keeps its existing focused-fixture qualification. A temporary
`bsp_game` source registration built through `scripts/build.ps1`: the adapter
compiled, `bsp_game.exe` linked, and both existing CTest targets
(`reconstructed_math`, `tool_tests`) passed. The temporary CMake change was
restored; primary integration owns final registration. No new tests or game runs
were added. No native draw, screenshot, OFF/ON identity or game validation is
claimed.

The remaining Step4 blockers include the real Screen AC59A0 manager dependency,
one authoritative page/raw-widget allocation and terminal transport, and its
factory/lifetime integration. This adapter alone cannot admit an ON page route.
