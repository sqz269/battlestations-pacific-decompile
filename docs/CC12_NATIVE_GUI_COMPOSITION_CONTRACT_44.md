# Canonical native GUI composition contract 44

Read-only contract map at Main `3df346c0ab1cf87f138bc91fa0bec0b4ecf220a3`,
which contains the requested published prerequisite baseline `7bbd1e404`.
The review worktree was fast-forwarded from Main before analysis. Root's separate
model-name routing change `d623a5c283e60ffda037e19e9f8fd6194427ae63` was also
reviewed below. Worker 43 owns group/raw-scene routing; this review neither
duplicates nor certifies that pending work. No Source, Ghidra, installed game,
audio, settings, build, or runtime was changed or executed.

Readiness 36 is useful routing history, but its missing pool/lineage/scalar
provider findings are superseded by process 37, state 38, and host 41. Native
Text rendering and the normal native GUI owner composition remain unactivated.

## Existing canonical providers

| Domain | Existing production identity to borrow |
| --- | --- |
| Nodes, scenes, attachment identities | `renderer.camera_environment().nodes`. `DestructionGraph` owns one common bootstrap, `ActualSceneRuntime`, `GeneratedModelLifetimeRuntime`, and raw-name `NativeNodeDestructionRuntime` (`game_native_renderer_lifetime.inc:27-30,58-65`). Camera and GUI must share it; do not create another hierarchy or attachment registry. |
| Raw node constants | `renderer.node_constants()` (`game_native_renderer_application.cpp:566-567`): the exact CE4970/D7A24C/CE4ADC cell references composed at `game_native_renderer_camera.inc:40`. |
| Actual string adapter | `renderer.game_grid_context().strings`, ultimately the VFS's `ActualNativeStringPoolStorage` (`game_native_renderer_application.cpp:398-422`; `game_native_renderer_grid.inc:13-18`). It uses the same publication01090AA8, return gate01090AA4, and raw manager domain. The raw node runtime separately borrows that same application's raw string context. |
| Model/group pools | `game_native_resource_pool_process().model_pool_01090054()` and `.group_pool_010902f4()`, initialized by the existing normal startup. These are the existing physical pool/allocator-list owners, not fresh GUI-local pools. |
| Counter and type cells | `vfs.native_owners().types()` and `vfs.native_types().light_types()/model_types()/group_types()`. `GameNativeVfsApplication` already initializes model/group through its common cells; lightweight wrappers may borrow them without reinitializing or copying IDs. |
| Actual geometry/render owners | `renderer.game_grid_context().graphics`, its existing declaration/stream/mapping contexts and `geometry.actual_owners()`, equal to `renderer.actual_owners()`. Ready-only `renderer.section_layout_services()` and `vertex_half_import()` supply native Text dependencies. |
| Material and parameter pools | `game_native_material_process().material_pool_00f8d3ac()` and the distinct `.parameter_pool_00f8d3e4()`. Both share the process allocator list and must survive their payloads and CRT cleanup. |
| Text pool and live lineage | `game_native_gui_text_process().pool_00f8bdf0()`, `.text_lineage_00f8be28()` and `.element_lineage_00f8bc88()`, only after explicit host-41 startup. Pool teardown remains its registered CRT callback after every Text slot is settled. |
| Mutable GUI/Text cells | The one `game_native_gui_text_state()` owner: aspect, vertical scale, shadow quartet, lazy-white quartet/mask, and blend. No per-Text or GUI-local copy is another authoritative mutable provider. |
| Original numeric data | The retained `GameNativeReadOnlyData::data_at(address, bytes)` service verifies mapped read-only spans CE2000..E07B23. Its tables contain numeric Original tokens, never callable host vtables. Mutable cells and headers cannot be sourced from it. |

The ordinary model route may use the actual adapter and the group route may
use the genuine raw pool while sharing one underlying VFS domain. Their C++
exception contracts differ; matching publication values alone does not prove
that a different string adapter/context is canonical.

## Exact environment fields

`NativeModelEnvironment` (`native_model_owner.hpp:21-40`) needs:

| Field | Binding |
| --- | --- |
| `pool_01090054` | Existing resource-pool process model pool |
| `nodes` | Renderer camera environment's raw-name node runtime |
| `types` | Persistent `ModelTypeBootstrap` over the same VFS counter, a borrowed common bootstrap, and `vfs.native_types().model_types()` |
| `retained_owners` | `renderer.actual_owners()` |
| `constants` | Exact CE4ADC, CE4970, D7A208, D7A260 DWORD views |
| `vtable_00d62de8`, `vtable_00d62c88` | Verified current table views, at least 23 and 22 DWORDs respectively |
| `actual_names` | For the ordinary typed/actual-adapter route, `&renderer.game_grid_context().strings`; keep this binding stable through destruction |

`NativeGroupEnvironment` (`native_group_owner.hpp:34-42`) needs the same node
runtime, the process group pool, persistent `NativeGroupTypes` over the VFS
counter/common/group cells, D7A24C/CE4970 constants, and verified D634F8/D62C88
tables of at least 22 DWORDs each. Worker 43's genuine raw constructor route also
needs the same `renderer.node_constants()` and raw string context; its final
admission/retention implementation must be reviewed as that packet closes.

`GuiNativeScene(group_environment)` supplies the concrete `GuiWidgetNativeCalls`
and owns the matching `NativeNodeParentingRuntime`. Its group-type reference
must be live before construction. It publishes one process notification binding,
rejects a second binding, and cannot die while group companions remain
(`gui_native_scene.cpp:23-34`).

`GuiWidgetOwnerEnvironment` has exactly four fields
(`gui_widget_owner.hpp:80-85`): the stable model environment; that same scene as
`native`; a real per-type `make_type` dispatch closure; and a clip-refresh service
pointer configured after the same widget/material domain exists. Construct one
`GuiWidgetOwnerRuntime` over it. The runtime destructor rejects retained widgets,
models, or frame bindings (`gui_widget_owner.cpp:346-352`).

This requires a persistent owner bundle, not references to temporary aggregate
initializers. A Text-dispatch slot may initially be unpublished while the widget
runtime is composed, but no Text admission may occur until the real application
and its same factory are bound. Non-Text arms need the actual dispatch services
for Icon, FrameBox, Screen, Section, and Listbox (`gui_type_dispatch.hpp:119-129`).

## Material, style, and Text constructor inputs

Compose `NativeMaterialDestructionAccess` from the same actual owner registry,
`grid.strings`, canonical parameter pool, and canonical material pool.
`NativeMaterialParameterAccess` borrows that same parameter pool/string adapter,
the current name comparator, and the actual D61A00 table (including slot+10).
Use `renderer.borrow_gui_material_destruction_profile_00d5e520()` for the ready
two-word D5E520 profile; do not manufacture a destruction table.

`GuiMaterialBindingServices` and `GuiWidgetClipRefreshServices` share that widget
runtime and the state-38 aspect cell. The former also needs `clip_box_sources`
returning stable fields of the actual type-16 ancestor, and a genuine
`retain_widget` token whose `get()` is the same widget and whose ownership keeps
its layout/borrowed fields alive. The existing general material-callback path
requires both (`gui_material_binding.cpp:131-144`). A no-op shared pointer over
unique page ownership does not satisfy it. Native Text's own raw factory
retention is a separate owner/count contract, not that generic shared token.

The 28 fields in `GameNativeTextApplicationBindings`
(`game_native_text_application.hpp:20-49`) map as follows:

| Fields | Required provider or remaining composition |
| --- | --- |
| `widgets`, `parenting`, `pool` | Same widget runtime, scene parenting, and initialized process-37 Text pool |
| `materials`, `parameters`, `style` | Stable shared aggregates above; no second material/string/actual-owner domain |
| `fonts` | Existing normal font producer's canonical `NativeFontResourceOwners`; needs a bounded host accessor/export and same-factory binding |
| `published_registry_00f8bf44`, `singleton_lifetime` | Stable publication of the exact normal `GameFontHost::registry()` and a real lifetime-interface adapter to the canonical manager, or a separately reviewed bounded accessor contract; neither is exposed by the current font host |
| `locale`, `locale_runtime` | Existing locale tables plus deliberate stable locale/context adapter; the sprite's private runtime leaves Lua marker context unsupported |
| `crt_locale_changed_0109de1c`, `compare_current_locale_00c0392a`, `compare_names_00bf7fbf`, `crt_sse2_conversion` | Live Source CRT comparison/locale contract and shared SSE2 alias; no fabricated zero flag or ASCII substitute. The existing frontend's queried SSE2 bool is private and must be exposed/owned deliberately. |
| `configuration_0109cf04` | Exact cell identity from `renderer.borrow_platform_publication_0109cf04()`, with the live actual platform receiver supporting the signed +28 height read; not the legacy focus-only projection |
| `material_vtable_00d5e520`, `mesh_vtable_00d62d60` | Existing verified mapped profiles; mesh clone also validates current slot+10/B742A0 |
| `glyph_strings`, `glyph_shader_name_00cefd78` | Same VFS `ActualNativeStringPoolStorage` and the verified terminated CEFD78 literal |
| `vertical_scale_00e12fd4`, `blend_factor_00f8be54` | Same state-38 cells |
| `default_width_00d7a260` | Mapped float cell; identity shared with the existing constant consumers |
| `constructor`, `properties`, `bounds`, `glyph_position` | Exact mapped/state cells listed below; preserve aliases, especially the shared shadow quartet |
| `lineage_00f8be28` | Process-37 live array, not loader-zero bytes or copied numeric IDs |

`GameFontHost` currently exposes its registry and retirement only. The registry
getter's null arm allocates/registers a new registry
(`font_registry_startup.cpp:185-199`), so the new publication must never silently
select another registry.
`SingletonLifetimeManager` is an interface (`game_entry.hpp:81-85`); the current
`GameSingletonHost` exposes raw lifetime services, while the only existing direct
implementation found is the separate semantic manager. Build a deliberate
same-manager adapter; do not add an unrelated semantic lifetime just to fill a
constructor reference.

By final review, Main `150f4210c37eb45c6a843415958b49d6e671cce2` also contains
the bounded BF7EB7 ASCII child (`native_crt_ascii_stricmp.hpp:7-16`). It explicitly
supplies no parent locale/TLS/errno dispatch. That leaf is a useful dependency,
but does not supply the required BF7FBF parent comparison provider in this table.

## Actual cells, tables, and headers

| Consumer | Required live numeric data |
| --- | --- |
| Model/group/node | DWORD CE4ADC/CE4970/D7A208/D7A260; group additionally D7A24C. Raw constructor bound cells must alias the model/group environment's mapped cells. |
| Text constructor | Floats D5C5C0, CE3E18, D7A24C, CE3800; shadow E12FD8[4] from state 38 |
| Text properties | Same D7A24C, D5C5C0 and E12FD8 quartet; float CEE07C; state-38 F8BE38[4]/F8BE48 |
| Bounds | Doubles D7A280, CEC380, CEF1B8, D5C5C8 |
| Glyph positions | Doubles D7A358, D5C7A8, D7A220, D06880, CEC380 |
| Profiles/literal | D62DE8, D62C88, D634F8, D5E520, D61A00, D62D60 and terminated CEFD78 literal, with each existing consumer's required extent |

Original numeric addresses are evidence identities, not permission to cast and
invoke Original code. Keep the mapped data owner through all callbacks and raw
drain. State 38 supplies mutable cells outside that mapping; process 37 supplies
live lineage. Empty or plausible local values do not substitute for either.

Names passed to raw effect/material/resource factories are actual initialized
eight-byte headers. Caller operation records must retain header storage and
unconsumed bytes across entered failure; after native cleanup consumes a header,
saved addresses become diagnostic-only. A C++ `std::string` object is not such a
header. The registry and configuration bindings likewise require persistent
publication cells, not a local copy of a currently equal pointer value.

One existing geometry preimage limit is bounded: the grid index-creation
context supplies a Source-selected `native_pool_stack_bits=0`, explicitly not
recovered native residue. B4BF30 consumes it only for flags low nibble >3
(`native_logical_index_owner.cpp:163-170`). AB8400's Text index call uses flags1
(`gui_text_buffers.cpp:66-67`), so this residue is not a new blocker for that
specific call. Do not generalize this to other flags or undocumented headers.

## Effect ownership and remaining normal-page services

Normal Text needs a persistent `GameNativeGuiSamplerApplication` and
`GameNativeGuiEffectApplication` over the existing renderer/VFS/manager/input/
platform/shadow publications, as the probe already composes. Promote the parent
ownership, not the probe's eager acquisition schedule. AB9650/AB8530 can request
`guidefault.mshd` before later font-shader selection; cold Text order is not
proved by eagerly loading GuiFont shaders first.

The four-argument Text constructor binds that effect application to its exact
private buffer services before allocations. It validates cell/service identities
before publication and has no throwing work after the binding publishes
(`game_hosts_text.cpp:691-709`). The effect owner must survive actual shared
cache/compiler/effect drain. Close/destroy effect metadata while Text buffer
services still exist, then destroy the Text application; its retention predicate
includes even an idle live effect-consumer binding (`774-777`). Sampler metadata
also survives drain and verifies its closure afterward.

Normal layout loading still needs the actual owner runtime and type factory,
the derived-construction -> primary model/parent -> current74 -> base properties
-> derived18 -> current78 order, and real page-root preparation. Screen pages
require `GuiScreenSceneEnvironment`: existing stores, actual GUI scenes, camera
environment, directional-light resolver, and real native calls for directional
allocation/radius/root bounds/page registration/visibility hook
(`gui_screen_scene_runtime.hpp:14-40`). These contracts are not supplied by
constructing a Group fixture. No complete normal-page provider is instantiated
by either routing repair alone.

## Admission versus entered failure

- Pure same-domain validation can reject before a native operation starts. The
  old semantic-name seam is such a pre-allocation rejection; it is not evidence
  that a new native allocation failed.
- Once names, pooled slots, actual constructors, reference registration,
  publication, shader compilation, mapping, or retained child/copy work enter,
  preserve the exact existing operation's acquired state and native cleanup
  boundary. Do not reset a failed frame or unwind live companions merely to
  empty the GUI bundle. Text application entry failure is sticky after its
  guarded operation enters; its `requires_process_retention()` also covers live
  raw allocations, child storage, and the effect consumer.
- **Font export changes retirement admission:** `native_fonts()` sets
  `fonts_exported=true`. A later Text-constructor/preflight failure before the
  SAME empty real factory is bound prevents ordinary font retirement. Preflight
  providers first; retain the complete font/registry/GUI/VFS/renderer graph on
  interruption between export and factory binding. Do not reset that flag or
  bind an unrelated empty factory.
- Retire Texts, pending work and completed raw storage while all providers live;
  retire fonts while the same bound factory still exists; then settle remaining
  GUI models/groups/material references before shared raw drain. Keep provider
  metadata through drain and close effect/sampler bindings in their documented
  order. All Text slots must be gone before process-37 CRT pool cleanup.

## Bounded packet-45 Source verdict

PASS for `d623a5c283e60ffda037e19e9f8fd6194427ae63`: ordinary `create_model`
selects `models.actual_names` when bound, otherwise the existing semantic pool,
then passes that storage to a private helper. The helper's allocation/map/
constructor/reference/publication/unwind body is unchanged. Model construction
and destruction already select the same `actual_names` adapter. Semantic
rejection remains before allocation; the local semantic adapter cannot escape
the synchronous helper. Reviewed blobs: `gui_widget_owner.cpp`
`79a11550d42126d8009bc506d566faf559f514fe`, header
`b051343705b24c9014e82bc2efdd105577d70e62`.

This closes that ordinary name-domain seam for returning getters and
`NativeStringStorage::release`'s existing noexcept boundary. It adds no direct
raw overload, throwing-getter temporary-name frame, native FH3/SEH equivalence,
new pool/constants, normal activation, or rendering proof. The parent's build
result is separate; this reviewer ran no build/test/runtime check. Worker 43's
group route, the required persistent composition, comparison/ownership services,
and native drawing remain independent acceptance work.
