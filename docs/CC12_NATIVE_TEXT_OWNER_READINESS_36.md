# Native Text owner readiness 36

Read-only Source assessment at `1f3bc258e116cadc3601c94af230bed5045f4bfb`.
Receipt: `reports/cc12_native_text_owner_readiness_36.json`. This packet changes
only its two published evidence files. No C++, ledger, Ghidra, installed asset,
build or runtime was changed. `bsp.py brief` reported the live
`/battlestationspacific.exe`; `bsp.py ghidra count` returned 76773, equal to the
snapshot. The configured project is `C:/Users/sqz269/bsp.gpr`.

The renderer already supplies the canonical node/resource domain. The missing
normal-host composition is **not another renderer, node tree, resource registry,
string pool, material pool or model pool**. It is one persistent GUI owner bundle,
two callers that still demand semantic names, and authoritative mutable GUI/Text
state. Constructing the four-argument `GameNativeTextApplication` alone does not
connect the normal layout loader.

## Reusable authoritative providers

| Required domain | Exact existing provider and admission/lifetime |
| --- | --- |
| Node destruction, scenes, actual attachment identities | `renderer.camera_environment().nodes`, exposed by `game_native_renderer_application.cpp:563-565`. `game_native_renderer_camera.inc:42-46` uses `DestructionGraph::nodes`; `game_native_renderer_lifetime.inc:27-30,64-65` composes one `LightTypeBootstrap`, `ActualSceneRuntime`, `GeneratedModelLifetimeRuntime` and **raw-name** node runtime. Borrow these; do not instantiate another scene/attachment domain. |
| Model/group pools and type cells | `game_native_resource_pool_process().model_pool_01090054()` and `.group_pool_010902f4()`. Normal `game_main.cpp:1138-1149` already starts both pools. Borrow `vfs.native_owners().types()` and `vfs.native_types().light_types()/model_types()/group_types()`. `game_native_vfs_application.cpp:90,109` already initializes the shared descriptor family; `game_native_type_storage.cpp:84-94` initializes model and group through the same counter/common cells. Persistent lightweight `LightTypeBootstrap`, `ModelTypeBootstrap`, `NativeGroupTypes` wrappers can borrow these same cells without initializing a second set. |
| Model environment | Compose `NativeModelEnvironment` from that model pool, the renderer node runtime, borrowed model bootstrap, `renderer.actual_owners()`, mapped `CE4ADC/CE4970/D7A208/D7A260`, mapped `D62DE8/D62C88` tables. Interface: `native_model_owner.hpp:21-41`. `renderer.node_constants()` already supplies the exact `CE4970/D7A24C/CE4ADC` cells for the raw constructor. No production instance of this environment exists at the assessed revision. |
| Widget native calls and parenting | Existing `GuiNativeScene` is the concrete `GuiWidgetNativeCalls` (`gui_native_scene.hpp:14-35`). It creates `NativeNodeParentingRuntime` using the same nodes and live group descriptor, and resolves current model/group companions (`gui_native_scene.cpp:23-68`). Compose its `NativeGroupEnvironment` with the canonical group pool/nodes/types, mapped `D634F8/D62C88` and the same `D7A24C/CE4970` constants. It permits only one process callback binding; keep it until all groups and node notifications retire. |
| Actual materials and parameters | `game_native_material_process()` already owns F8D3AC and the **distinct** F8D3E4 pool on E188B4 (`game_native_material_process.hpp:7-23`, `.cpp:14-18`); normal startup initializes both at `game_main.cpp:1090-1091`. Compose one stable `NativeMaterialDestructionAccess{renderer.actual_owners(), grid.strings, parameter_pool, material_pool}` and `NativeMaterialParameterAccess{same_parameter_pool, same_strings, current_name_comparer, mapped_D61A00}`. Definitions: `native_material_owner.hpp:53-58`, `native_material_parameters.hpp:21-30`. |
| Geometry, mappings, declarations, layouts, image owners | `renderer.game_grid_context().graphics` and `.strings`, `renderer.section_layout_services()`, `renderer.vertex_half_import()`, `renderer.actual_owners()`. Use `renderer.borrow_gui_material_destruction_profile_00d5e520()` for the exact canonical material profile, not an independently copied table (`game_native_renderer_application.cpp:609-637`). Effect admission checks these identities and the existing process pools at `game_native_gui_effect_application.cpp:164-189`. |
| Text raw pool | `NativeGuiTextPool` is implemented and explicitly initialized/destroyed (`gui_text_factory.cpp:86-128,253-259`). No production `NativeGuiTextPoolStorage`/process owner or startup call exists. Add one F8BDF0 Source storage/companion on `game_native_physical_pool_process().allocator_list_domain_00e188b4()`, with explicit one-time startup and explicit teardown after every Text slot. Its destructor only unbinds trim dispatch. `GUI_TEXT_FACTORY.md` records original CD71F0/CE0A80; direct AB77C0/AB7420 composition must not pretend it recovers original CRT registration/order. |
| Style | `GuiMaterialBindingServices` borrows the same widgets, actual owners and actual aspect cell (`gui_material_binding.hpp:30-43`). `gui_clip_box_parameter_sources` already resolves real type16 fields. Native Text content/clip registration uses `register_native_gui_clip_parameters_00aa9f10` directly (`gui_text_content.cpp:317,360`, `gui_text_clip_refresh.cpp:71`), so it does not require the semantic `retain_widget` callback. If `bind_gui_material_callbacks` is used for other types, a real page/widget owning token is required (`gui_material_binding.cpp:132-146`); current `unique_ptr` page ownership cannot supply it. Never substitute a no-op shared pointer. |

## Two real name-domain seams

`GuiWidgetOwnerRuntime::create_model` unconditionally asks
`nodes.require_semantic_name_pool()` at `gui_widget_owner.cpp:404`, creates a
`PooledStringStorage`, then constructs the primary model at lines 420-422. This
rejects the existing renderer's raw-name runtime **before allocation**. Likewise,
`GuiNativeScene::create_group` at `gui_native_scene.cpp:84` and the prepared-owner
group constructor at `native_group_owner.cpp:230` require semantic names.

The underlying raw bodies are ready: model's companion overload at
`native_model_owner.cpp:204-242` and group's complete raw constructor in
`gui_page_root.hpp:36-41`. The latter can also be followed by the existing
constructed-storage `NativeGroupOwner` adoption constructors
(`native_group_owner.hpp:45-60`). A bounded repair should preserve the existing
canonical maps, actual pool slots, publication/temporary cleanup timing and
failure retention; reuse those raw calls instead of introducing new registries.
Alternatively the ordinary primary model caller can use the already-supported
actual-name adapter route, with its explicitly narrower exception domain.

There is an important exception: AB8530's **tracked auxiliary** model path is
already different. At `gui_widget_owner.cpp:474-572` it borrows the caller's
actual `NativeStringStorage`, retains its one-shot frame and publishes +188
before consuming the temporary. With `models.actual_names = &grid.strings`, the
typed model overload at `native_model_owner.cpp:173-184` skips the semantic getter
and uses that same actual string wrapper. Its cleanup also uses that wrapper
(`native_model_owner.cpp:104-111`). This is a supported Source path for normal
returning getters; `NativeStringStorage::release` is noexcept
(`native_string_pool_storage.hpp:31-37`). It does not establish raw throwing-getter,
FH3 or SEH parity. Setting `actual_names=nullptr` while borrowing raw nodes
instead makes this tracked auxiliary call reject: it currently calls the typed,
not raw, model overload. Choose and document one domain; do not silently mix them.

## Constants that are ready, and globals that are not

`GameNativeReadOnlyData::data_at` provides verified original **read-only** data
only within CE2000..E07B23 (`game_native_readonly_data.hpp:13-17,39-42`). It can
supply the referenced typed cells in these existing aggregates:

* `GuiTextConstructorConstants`: D5C5C0, CE3E18, D7A24C, CE3800.
* `GuiTextPropertyConstants`: the same D5C5C0/D7A24C cells plus CEE07C.
* `GuiTextBoundsConstants`: D7A280, CEC380, CEF1B8, D5C5C8.
* `GuiTextGlyphPositionConstants`: D7A358, D5C7A8, D7A220, D06880, CEC380.
* D7A260 default width, D62D60 mesh profile and the terminated CEFD78
  `GuiFontBilinear.mshd` literal (established in `GUI_TEXT_GLYPH_CHILD_RUNTIME.md:36`).

Aggregate declarations are at `gui_text_lifetime.hpp:16-22`,
`gui_text_properties.hpp:6-15`, `gui_text_runtime_factory.hpp:17-22`, and
`gui_text_glyph_reference.hpp:35-41`. Use stable original mapped references;
preserve shared-cell identity checked by `game_hosts_text.cpp:545-563`.

No production provider was found for mutable E12FC0 aspect, E12FD4 vertical scale,
E12FD8..E12FE4 shadow quartet, F8BE38/F8BE48 shared lazy-white state, F8BE54 blend,
or the **initialized** F8BE28[3] Text lineage. They cannot come from
`GameNativeReadOnlyData`. The semantic bridge's `kVerticalScale00e12fd4 = 0.75f`
(`game_hosts_text.cpp:35-40`) is not an owner for the mutable cell.
`GUI_TEXT_LIFETIME.md:34-38` explicitly records that lineage is zero in the
uninitialized image; a zero array does not authorize correct shadow retirement.
Recover/verify their producer/startup/writer contracts, then give the one GUI
process owner stable mutable cells. Do not fill them with plausible defaults.

The remaining comparison/locale bindings also need deliberate ownership:
`GameNativeTextApplicationBindings` requires live CRT locale-changed state,
current-locale wide comparison, name comparison and the shared SSE2 selector.
`gui_text_content.hpp:66-75` distinguishes ASCII-only zero mode from current
locale mode. The frontend already owns a stable queried SSE2 decision
(`game_hosts_frontend.cpp:159-161`), but it is private. The existing private
`GameLocaleTextRuntime` supports uppercase and explicitly leaves Lua marker
context unsupported (`game_hosts_text.cpp:83-110`). Reuse/expose those bounded
services or compose the existing proper Lua-context adapter; claiming complete
localization from that bridge would exceed its contract.

## Exact next Original packet, ahead of raw loader routing

Live xrefs and the existing exported assembly identify a bounded next packet:

| Original entry | Exact next work and ready dependency |
| --- | --- |
| `00CD71F0..00CD7205` (22 bytes) | Canonical Text-pool CRT wrapper. MOV ECX,F8BDF0; CALL AB77C0 at CD71F5; PUSH CE0A80; CALL BF6FF5 atexit at CD71FF; POP ECX; RET. Implement an explicit one-attempt Source process binding using the existing pool/list, preserving actual atexit result and initialized-on-registration-failure behavior. |
| `00CE0A80..00CE0A89` (10 bytes) | Matching atexit thunk: MOV ECX,F8BDF0; JMP AB7420. The companion/storage/list must survive this callback, and all Text slots must already be gone. This is destruction, not another initializer. |
| `00CD7210..00CD725E` (79 bytes) | Text lineage static initialization. Guard F8BDEC is tested then set; name F8BE34 receives D5C5DC (`cGuiText`); ECX=F8BC88 is passed to AA8200; current parent IDs F8BC88/F8BC8C are copied to F8BE2C/F8BE30; 6FAC20 supplies the shared counter. Assembly is decisive: CD7255 writes counter+4 = captured+1 **before** CD7258 publishes captured own ID to F8BE28. The pseudocode reverses these final stores. Preserve guard/name/earlier stores on parent/counter failure; do not reset state to retry. |
| `00AA8200` named parent prerequisite | Parent Element descriptor F8BC88[2], guard F8BC80, name D5C124, root0109DB84 through BEA780. `gui_widgets_cc13.hpp:123-132` already records this contract; `LightTypeBootstrap::initialize_root` supplies the established root family. Audit the parent's listing when implementing its concrete same-counter adapter. |

Reuse `gui_widgets_cc13.hpp:42-93` (`GuiTypeDescriptor`, `GuiTypeCounterHost`,
`gui_type_initialize`) rather than rebuilding type machinery. There is no
Text-specific CD7210/process-backed wrapper in current Source. The helper's
`target.ids[0] = counter.next_id_04++` needs compiled/order review against the
explicit CD7255/CD7258 schedule before accepting it for this native wrapper.
This packet is independent of repairing normal primary-model/group routing;
it supplies the real pool/lineage owner those callers will eventually borrow.

For the other mutable cells, live bounded xrefs give a different next action:
F8BE38 writes and F8BE48 read/writes all occur in ABB630, whose existing
`gui_text_properties.cpp:83-86` already implements shared lazy-white state.
They need one backing owner, not another property-reader reconstruction.
E12FC0, E12FD4, E12FD8 and F8BE54 xrefs show only reads/data references in their
reported result sets. Verify exact initialized `.data` bytes in the original
image and any indirect/global initialization contract before adding those
backing cells. Lack of direct write xrefs is not proof that they are immutable.
The ready CD71F0/CD7210 owner packet can proceed while that scalar-preimage
audit remains a separate bounded dependency; do not manufacture those values.

## Minimal next integration sequence

1. Add one persistent native GUI application bundle (new dedicated host pimpl,
   registered in `cmake/startup.cmake`; owned by `GameStartupHost`). Borrow all
   domains above. Add the actual Text pool owner/startup, authoritative mutable
   GUI/Text cells and comparison providers. Keep descriptor initialization on the
   existing VFS counter/type cells. Repair the primary model/plain-group name
   seams in `gui_widget_owner.cpp` and `gui_native_scene.cpp` with ready raw
   construction/adoption APIs, or document the narrower actual-name adapter.
2. Construct `GuiWidgetOwnerRuntime` first with a dispatch closure that has a
   not-yet-published Text application slot. Compose stable style/material access,
   then construct four-argument `GameNativeTextApplication` and publish that slot
   **before any Text allocation**. The Text arm calls `text.make_type(owner)`;
   other arms delegate to `GuiTypeDispatchFactory`. That factory currently has
   no Text arm (`gui_type_dispatch.cpp` final switch). No body copy is needed.
3. Bind the same Text factory to the persistent native font producer before
   exporting/using its font owners; keep its actual registry publication and
   `SingletonLifetimeManager` binding stable. Do not mix semantic-loaded records
   with native registry records (`font_registry_startup.hpp:73-75`). The root
   integrator owns font activation; this packet does not duplicate that work.
4. Adapt the **existing** normal `GameGuiLayoutHost` to its retained owner runtime
   and `GuiNativeScene`: create/attach actual root and primary models, then
   preserve derived construction -> node binding/parenting -> current74 -> base
   properties -> derived18 -> current78. Current methods still synthesize IDs
   and report unimplemented work at `game_hosts_frontend.cpp:371-427`.
   Plain page roots require `GuiTypeDispatchFactory::prepare_script_page` and a
   genuine Screen service. `GuiScreenSceneRuntime` is ready composition, but its
   actual scene/store/directional-light native calls must be supplied
   (`gui_screen_scene_runtime.hpp:14-45`). This is a normal-page integration
   prerequisite, not a reason to replace a Screen with a Group fixture.
5. Promote the probe's sampler/effect **parent ownership**, not its eager test
   acquisition schedule. `game_native_gui_effect_probe.cpp:457-466` composes
   `GameNativeGuiSamplerApplication` and `GameNativeGuiEffectApplication`; the
   probe owns both through shared drain (`:242-275,590-598`). The normal host
   currently creates this only under probe flags (`game_hosts.cpp:2605-2623`).
   Bind the same persistent effect application through the Text constructor.
   Default AB9650/AB8530 will request `guidefault.mshd` before later AB8CE0 font
   selection; do not run eager GuiFont probe acquisitions first and treat their
   successful ordering as proof of the cold Text path. Preserve the recovered
   cache-create/release bracket and existing nonreplayable operation frames.

Normal retirement must be explicit: stop entry/frames; retire page/Text owners
and settle child/copy/mapping operations while all providers exist; retire native
fonts with the same Text factory still alive; release remaining GUI models/groups
and queued uses; perform the **one** shared renderer/singleton drain; close and
destroy effect metadata while Text services still exist; close/destroy sampler
metadata; destroy now-unborrowed Text metadata; then destroy the empty widget/
scene/environment metadata and Text pool before its allocator list. Both normal
and exceptional shutdown paths in `game_hosts.cpp:1819-1858,2868-2882` need this
ordering. A default C++ member destruction order is insufficient because the
effect destructor clears Text's consumer slot after the native drain
(`game_native_text_application.hpp:84-98`, `CC12_NATIVE_TEXT_EFFECT_COMPOSITION_33.md`).
Retain the whole entered graph on unresolved failure; do not replay or unwind it
by clearing maps and pointers.

## Existing runtime evidence and limit

The checked receipt `cc12_native_gui_font_effect_startup_integration_14.json`
records historical GUI07 Source `d4edc832430c5e501a5f29c7115e8fc1d2d42da3`:
six fonts/1060 glyphs/three actual images, genuine GuiFont and GuiFontBilinear
effects, 119 presents, caller-credit retirement and the shared drain. It explicitly
sets `native_text_draw_validated=false`; its visible screen is the frontend
bridge. Native fonts retire inside the current probe at
`game_native_font_probe.cpp:208`, before the normal frame loop. These demonstrate
reusable providers and probe parent lifetimes; they do not demonstrate native
Text construction, native buffers, pass submission, drawing, visual parity,
original ABI compatibility or gameplay. No runtime was repeated by this packet.
