# October 24 GUI page readiness

The selected first page is `FE_initial`. Its current installed Lua has one Screen
root, two Icons and one Text. Execution remains blocked by the Screen manager
dependency, raw Icon geometry composition, and page-owned lifetime transport.
`BSP_NATIVE_GUI_PAGE=1` is a proposed default-OFF runtime switch, not an accepted
ON page route. This packet changes documentation only. Step 1 remains the
prerequisite for page integration; the resource commits `51edfbc10` and
`fdf3951b8` and their build proof are unchanged.

## Installed page inventory

The files below were read without modifying the installation. The inventory
strips Lua comments, then counts literal typed table keys. These three files
have static child definitions; evaluating property values still needs the real
`VfsGuiPageScripts` / `GuiPageLuaEvaluation` path. `_Common` runs before each
page, and each page resets `GuiScreen = {}`.

| Installed `interface/` file | Active descendants | SHA-256 |
|---|---|---|
| `_Mouse.lua` | 10 Icons | `76efd18430c43558cccd01c239e6c581c21cc31f3ca35a5051af7947668338e5` |
| `_Highlight.lua` | 4 FrameBoxes | `394a6d568dd8fe82b986cc1d6d6f5bb60b4770d077a20319d220230adfa0d948` |
| `FE_initial.lua` | 2 Icons, 1 Text | `3bab3fc8a29569d0f7da13cc3c69d3b1232c57772ced58a2e06985700f332cfa` |

Installation root:
`I:/SteamLibrary/steamapps/common/Battlestations Pacific`.
`FE_initial` contains `logo_pacfic_Icon`, `New_Icon`, and `press_start_Text`;
the Text uses `Viper19`. Its PC `DefaultText` is an AlterBSP-specific string,
so this is current installed-source evidence, not a pristine shipped-file claim.
The commented copyright/sublogo block contributes no widgets.

No matching loose `<page>.mmod` exists at the installation root for these three
names. This does not prove the mounted VFS would reject that name: archive
precedence, provider selection, script bytes actually resolved, and the native
model-backed branch remain unverified. A production acceptance run must record
the real VFS resolution/model test before loading the selected page.

## All 18 classes

The IDs and factory entries come from `GuiWidgetType` / the class table in
`gui_layout_loader.hpp/.cpp`. Current retained admission is the switch in
`GuiTypeDispatchFactory::Shared::create`; Text is the separate
`GuiTextRuntimeFactory::make_type` arm. A present runtime is not a complete
native class, renderer binding, scalar ABI, or production readiness claim.

| ID | Class | Native factory entry | Retained runtime and remaining admission |
|---:|---|---|---|
| 1 | Screen | `00AA3BD0`; script `00AC6600` via `00AA5840` | `GuiScreenLayerImplementation`; real raw store/scene acquisition and retirement remain required |
| 2 | Group | `00AA12F0` | `GuiGroupTypeImplementation`; uses the same widget/node tree |
| 3 | Text | `00AA1380` | `GuiTextRuntimeFactory` / `GuiTextRuntimeImplementation`; must reuse Step 1's same services and actual allocation retirement |
| 4 | Line | `00AA1530` | No retained factory arm; member/vector/render/lifetime adapter required |
| 5 | Progbar | `00AA14A0` | No retained factory arm; actual Icon base plus current88/value binding required |
| 6 | Icon | `00AA1410` | `GuiIconTypeImplementation` / `GuiIconRuntime`; existing typed geometry is not the raw mesh publication route |
| 7 | Scrollbar | `00AA15C0` | No retained factory arm; base/native state and current virtual/property binding required |
| 8 | Grid | `00AA1650` | No retained factory arm; actual column/row/child ownership and teardown required |
| 9 | Model | `00AA16E0` | No retained factory arm; actual model load, scene reference, and destruction binding required |
| 10 | Movie | `00AA1770` | No retained factory arm; actual player/video textures, current slots, focus, and release required |
| 11 | Listbox | `00A9B7E0` through `*00F8BC10` | `GuiListboxTypeImplementation`; current34/60 bounded domain exists; full properties/frame/scalar ABI remain open |
| 12 | AnimIcon | `00AA2340` | No retained factory arm; Icon state/geometry plus actual animation/current88 lifetime required |
| 13 | Curve | `00AA1800` | No retained factory arm; actual point/vector/geometry and cleanup binding required |
| 14 | Sound | `00AA1890` | No retained factory arm; actual sound references/instance stop and string lifetime required |
| 15 | Button | `00AA4440` | No retained factory arm; actual button registry/global/current operations and destruction required |
| 16 | ClipBox | `00AA1920` | `GuiClipBoxTypeImplementation`; requires the same actual clip/material/widget domain |
| 17 | Section | `00AA19B0` | `GuiSectionRuntimeImplementation`; actual buffer/material/atlas services and `GuiStartupHost` singleton service required |
| 18 | FrameBox | `00AA1A40` | `GuiFrameBoxTypeImplementation`; same raw mesh/stream/material gap as Icon |

Seven classes are admitted by `GuiTypeDispatchFactory`, and Text supplies the
eighth retained implementation. The other ten hit its explicit unsupported
default. Their pool/factory/member reconstructions exist; connecting those
raw-object interfaces to one retained layout/type implementation is separate work.

### Exact ten-type lifetime dependencies

The addresses below are in `gui_widgets_cc13.hpp` factory/destructor metadata
and member declarations. These are original body identities, not callable host
addresses or proof that all current virtuals are bound. The wrappers use new
C++ service interfaces. Native fresh constructors consume ECX=self; scalar
deleting entries additionally consume flags on the stack and return the slot
to the matching class pool when bit0 is set.

| Missing retained type | Fresh constructor | Derived destruction | Concrete missing ownership/service connection |
|---|---|---|---|
| Line | `00AC2D00` | `00AC28D0`; scalar `00AC2CA0` | Point vector at +F0/+F4/+F8, actual base/node and render hooks; `GuiWidgetMemberHost`, `GuiWidgetDestructionHost` |
| Progbar | `00AC77C0` | `00AC7830` -> Icon `00AB5E70` | `GuiWidgetLifetimeHost` Icon ctor/copy and current88; loaded `00AC78E0`, reader `00AC7A90`; Icon pool/value+138 overlap must be preserved |
| Scrollbar | `00AC7C20` | `00AC7CA0` -> base `00AA9730` | Actual EC..F8 fields, base/native virtual dispatch and reader/current behavior; existing constructor/describer alone is insufficient |
| Grid | `00ACB0A0` | `00ACBBF0` | `GuiWidgetCopyHost::grid_clear_00acb9b0`, column destruction `00ACA730`, copy `00ACB290`, actual child association |
| Model | `00ACC740` | `00ACC7B0` | `GuiModelReadHost::load_model_00acccf0`, reader `00ACCE20`, scene release `00ACC6A0` on the same model/reference |
| Movie | `00AAFE20` | `00AAFEF0` | `GuiMovieHost` current78 player creation; current38 textures, current10, real node material and B189F0 sampler binding; decoder lifetime/focus |
| AnimIcon | `00AD2D50` | `00AD2DC0` | `GuiAnimIconHost` Icon reset/current88; loaded `00AD2FF0`, actual PlaybyDefault/Loop/Forward and state reference lifetime |
| Curve | `00ABE030` | `00ABDE90` | Point clear `00ABC920`, append `00ABC970`, loaded `00ABC040`, actual vector/material/geometry and member-host cleanup |
| Sound | `00ACDB30` | `00ACDC40` | `GuiWidgetCopyHost` reference assignments `00ACD8E0`/`00ACD9E0`, instance stop/current8, same native strings/base destruction |
| Button | `00AAC6F0` | `00AAC530` | `GuiButtonRegistry` list at 00F8BD38, push/remove, native F4 string, correct global00F8BCF8 cleanup and same base |

`GuiWidgetConstructionHost::construct_default/copy_construct` and
`GuiWidgetDestructionHost::run_destructor` are external service obligations;
passing an address to those interfaces does not implement that body. None of
the ten has a factory arm over `GuiWidgetOwner::layout()` in this checkout.

## Screen blocks every candidate

`NativeGuiSceneAcquisitionContext` already composes the real raw AC59A0 body,
using `NativeGuiCameraStoreMapBindings`, outer-scene, camera, resource, light,
fog, CRT-radius and fixed-owner contexts. It requires an actual124h page and
actual18Ch Group root. Its existing implementation has these reached calls:

- `00AC59C5`: unconditionally `get_native_gui_manager_004c12b0`, before
  `00AC59D3` camera-store lookup. A zero ShareScene byte does not avoid it.
- `00AC5B32`: the same getter before `00AC5B48` / `00AA5070` store creation.
- `00AC5EED`: native page registration after another getter in the reached tail.

The authorized scope prohibits constructing the 004C12B0/up chain, and no
caller-borrowed actual manager is available to this page packet. This is the
coordination blocker even though the raw acquisition bodies exist. Injecting
an already-existing actual manager and its SAME canonical context can be a
future composition API; constructing a substitute manager here is outside scope.

`GuiScreenSceneRuntime` is a different semantic interface. Its `GuiCameraStore`
has an owned `visible_layers` int, and `GuiCameraStoreMap` is a flat vector.
Neither is the native24h store / red-black tree. That typed state cannot be
cast to the raw AC59A0 state or maintained as a second authoritative count/map.
Raw destruction `00AC5480` / scalar `00AA38F0` also needs the same complete raw
base/containers and scene/store/string/terminal context.

Manager injection alone does not complete raw page loading.
`NativeGuiPageReaderContinuation` currently reaches AC6806/EH5 with
`awaiting_properties_continuation`; its borrow explicitly reserves a future
genuine property tail while the same Lua/reader stay alive. The typed
`GuiPageScriptLayoutHost` visitor cannot be relabeled as that raw tail without
the page/widget identity and lifetime composition.

## Bounded real Icon path

Recovery can be bounded to a real four-vertex Icon before broad type work.
Existing `GuiNativeGeometryOwners::construct_and_associate74_fragment` supplies
the actual AB2563..AB25C3 mesh/model+180 fragment. Existing
`acquire_gui_native_section_fragment` and
`publish_gui_native_section_fragment` cover actual section selection and
AB443D..AB4488 publication after stream unlock/AA7220. The latter sets the actual
material, consumes its temporary reference, rebuilds layout with current+40,
appends a new section when required, and consumes its section reference.

The remaining bridge must use current+38 `SimpleColor.mvfm`, current+5C
four-vertex flags1 stream, same actual lock/unlock/descriptor and stream0
publication B73BB0, actual 00535320 material/effect/cache ownership, the same
AA9F10/B18A40 clip/widget lifetime, actual texture/atlas references, and current50
color publication. Step 1's real buffer/material/layout services are reusable
prerequisites, not evidence that this Icon caller composition exists.

`GuiGeometryRuntimeServices` currently writes `GeneratedInstanceGeometry`,
`LogicalVertexStream`, `MaterialCloneState` and typed layout fields. As documented
in `GUI_NATIVE_GEOMETRY.md`, these do not publish raw mesh+64, section+20/+50,
or mesh+54[0]. `gui_native_geometry.hpp` deliberately supplies no such adapter.
An independent logical stream/material/layout would not close the raw path.

## Future single-page ownership API

After those providers are ready, a `GameNativeGuiPageApplication` can borrow
the SAME `GameNativeGuiRenderApplication` graph and own one selected page record,
its sole `GuiPageRegistry` layout, raw page/node/type companions and persistent
failure frames. Proposed entry points are `requested()`, `handles(name)`,
`load_page(name, screen_flag, add_reference)`, native visibility, and explicit
retirement. The frontend routes FE_initial before its projection loader, keeps
only a borrowed page view, and skips that page in the sprite bridge. Default
OFF leaves its existing registry/loader/draw path intact. Admission must be
complete before the frontend omits the existing page.

Required shared borrows are `nodes()` (with widgets/parenting/scene),
`text_factory().make_type(owner)`, and the same Text buffer/effect/material/clip
services. The widget runtime's factory closure must already dispatch Text and
the admitted non-Text types; getters alone do not replace its captured factory.
The actual manager and raw Screen acquisition/lifetime contexts remain explicit
borrowed prerequisites, not new owners.

Node commit `bcf052b6a9` has a further lifetime boundary: `retain_widget()` uses
`standalone.at(&owner.layout())`, so it only retains app-owned standalone Text.
Page-owned Text/Icon need a token from their one authoritative page record that
is accepted by the SAME material/clip service. It must not transfer their layout
into a second standalone owner. Text's actual pool retirement must use the same
Text deletion transport; the standalone take-for-retirement API requires a
detached layout and cannot silently retire an attached page child.

## Evidence boundary

This is a bounded source/installed-file inventory. No page app, switch, native
manager, new widget implementation or shared frontend edit was added. There
were no Ghidra writes, game launches, screenshots, ON page acceptance or gameplay
claims. Existing resource build proof is separate: `scripts/build.ps1` passed
with both runtime arms compiled and both existing tests passing. A documentation
change does not require repeating that build. Primary runtime evidence belongs
in `OCT24_GUI_RENDER.md` after Step 1 is compiled and exercised.

### Further actual-storage prerequisites

The later source batch closes raw Icon/AnimIcon storage, Text font/state/shader
operations, Grid and Curve storage, Model9 default allocation/placement copy,
actual AAA710 child enumeration, AC4C50 Page property composition and optional
full canonical Text/Icon child retirement. See the individual OCT24_GUI_* docs
and OCT24_GUI_RENDER.md for exact domains and build/runtime evidence. These
helpers add no retained factory admissions and do not expand the existing
logical factory's admitted type set.

Step4 remains incomplete. Full raw Text content/source/properties and recursive
glyph delivery, raw Icon property/GPU adoption, other derived providers and
sole actual Page retirement must be composed. Model9's copy factory requires
genuine partial-loader failure custody. Progbar's real138/13C Icon-pool trailer
and adjacent-slot alias remains unclosed; no larger pool or repaired index is
admitted. The actual warm88h manager must come from cc15 in the SAME focus/
resource context. The Page adapter now rejects a missing manager before its
slow construction arm at every reached original getter site. No GUI A or
higher chain object is instantiated here, no Page has a second owner, and no
native Page ON screenshot is claimed. Input remains the later button-dispatch
blocker after visual bring-up.
