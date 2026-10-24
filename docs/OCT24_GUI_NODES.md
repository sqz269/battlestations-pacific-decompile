# Native GUI node application, 2026-10-24

`GameNativeGuiNodeApplication` composes the existing model/group owners for
render-side GUI callers. It supplies a callable standalone Text model path and
the existing plain page-root group path. The Stop section in
`CC15_OBJECT_GUI_A.md` predates the complete `NativeModelOwner`,
`GuiWidgetOwnerRuntime` and `GuiNativeScene` implementations; its statement that
the model lifetime is unported is no longer current source evidence.

This capability constructs no GUI manager, screen manager or front-end manager.
It neither activates a runtime switch nor draws a replacement projection. The
primary GUI render application owns Text/font/effect composition and execution.

## Canonical domains

The renderer's `camera_environment().nodes` is the sole node destruction,
scene-attachment and generated-model reference domain. The application borrows
that exact object and `renderer.actual_owners()`. `GuiNativeScene` supplies
parenting and actual group notification using the same node runtime. Native
transforms, reference counts, roots, child/sibling links and attachment arrays
remain their original views over the real allocations.

Model and Group pool accessors come from `GameNativeResourcePoolProcess` after
explicit CD7F00/CD8460 startup. They are the already bound 01090054 and 010902F4
owners under the existing E188B4 allocator list, with their original CRT exits.
The application does not initialize or destroy another pool. Model and Group
type companions borrow the VFS's existing counter, Light/Model/Group descriptor
storage and guards; construction does not allocate or initialize type IDs.

The model environment borrows the VFS's `ActualNativeStringPoolStorage`, as the
existing mission instance-model environment does. This is required by the
existing ordinary widget-model creation overload. The raw page-root route uses
the renderer's persistent raw string context. Admission compares all three raw
string publication cells, the renderer grid's actual string-storage identity,
and the model/node bounds-cell identities. It rejects a mixed domain.

Mapped input spans are CE4ADC, CE4970, D7A208 and D7A260 (one DWORD each),
D62DE8 (5Ch), D62C88 (58h), D634F8 (58h), and the renderer's existing
D7A24C/CE4970/CE4ADC node-constant cells. Numeric tables select existing source
dispatch; original executable code is never called.

## Construction and ownership

1. Construct the node application with a deferred, real type-factory callback.
   Construct `GameNativeTextApplication` using `nodes.widgets()` and
   `nodes.parenting()`, then make that factory's Text arm return
   `text.make_type(owner)`. No widget allocation occurs in the node constructor.
2. Supply one fresh `unique_ptr<GuiLayoutWidget>` with type Text, its key, and
   explicit constructor-unwritten `authored_x`/`visible` preimages.
   `create_standalone_text` puts this sole layout in its owning record before
   calling `create_with_scene_00aa6640`. The existing factory constructs the
   Text, the existing widget runtime allocates and constructs its actual Model,
   binds it through AA6720, and invokes current74. The returned owner's
   `model_reference()` and `node_binding()` borrow that canonical model; both
   must be nonnull. No parent, properties, current78 or scene insertion is
   invented by this API.
3. Existing Text properties/content/buffers and renderer submission operate on
   that same owner. `retain_widget(owner)` can supply a genuine alias token to
   `GuiMaterialBindingServices`: its `get()` is exactly `&owner`, while its
   ownership retains the runtime and the sole layout record. This is host
   lifetime bookkeeping, with no native reference-count store or extra owner.
4. Release those material/lifetime tokens and complete pending Text work, then
   `take_standalone_for_retirement` transfers the same layout allocation to
   `GameNativeTextApplication::retire_owned_text`. That application's original
   child transport owns AB8EE0 flags1 and the factory's AB75A0 pool return. A
   second transport is invalid because Text verifies its original child-call
   identity. Never discard the transferred wrapper or substitute `retire_tree`
   for this pooled Text path. Drain any retained render references before
   destroying node metadata, renderer metadata or the CRT pools.

The later page loader can prepare its exact cGuiLayer factory/flag and call
`create_plain_page_root_00aa5840_fragment` with its own fresh Screen layout.
The existing `GuiNativeScene` then owns one actual cGroup and binds that same
root widget. This does not construct a native page/cGuiLayer allocation or
claim that the manager tree is active. Borrowed page layouts must outlive
their native retirement. The caller uses the same scene/runtime for children.

An entered exception latches the application and keeps its owning/borrowed
associations. There is no automatic replay or rollback. Preserve all borrowed
providers and the complete graph through process exit before native drain.
Destruction rejects live layouts, model/group companions, entered calls or
failed work. Existing constructors' materialization/EH and `noexcept` callback
limits remain in force.

## Reused evidence and original ABI

| Address | Existing body/contract used |
|---|---|
| 00AA6640 / 00AA6560 | Standalone widget/type factory followed by actual Model binding and current74; current C++ interface is not native widget ABI. |
| 00B74EB0 / 00B74D00 | Static 01090054 allocator thunk / its canonical pool allocation body. Existing widget runtime calls the same pool body directly. Model allocation is 188h, with the pool's slab DWORD at +184h preserved. |
| 00B75030 | ECX actual Model, stack 8-byte name header, EAX same Model, RET4. Actual model size 184h over the B6F5A0 prefix. |
| 00B6F5A0 | ECX actual node, stack name header, EAX same node, RET4. Existing canonical storage/scene/reference companions are reused. |
| 00AA6720 | Binds the real node to this widget and clears node+138 low two bits. |
| 00B8F450 / 00B8F310 | Static 010902F4 thunk / canonical pool allocation. Group allocation is 18Ch; slab DWORD +188h is preserved. |
| 00B8F5E0 | ECX actual Group, stack name header, EAX same Group, RET4. Constructor covers 188h, with group attachment fields in the same allocation. |
| 00B6E680 | ECX child, stack requested parent, RET4. Existing actual hierarchy/scene/backlink dispatch; no substitute hierarchy. |
| 00B6DFA0 | Existing logical unlink/reference release. Final model reference reaches B75290 and canonical pool return B74750. |
| 00B75290 | ECX Model, stack flags, EAX original address, RET4; flags&1 returns the 188h slot after B750C0 destruction. |
| 00AB8EE0 / 00AB75A0 | Same canonical Text scalar deletion / Text pool return, through the original Text application's deletion transport and factory. |

Source evidence is `gui_widget_owner.cpp`, `gui_native_scene.cpp`,
`native_model_owner.cpp`, `native_model_pool.cpp`, `native_group_owner.cpp`,
`native_group_pool.cpp`, `game_native_resource_pools.cpp`,
`game_native_type_storage.cpp` and `game_native_mission_model.inc`.
Existing detailed evidence remains in `GUI_WIDGET_SCENE_VISIBILITY.md`,
`GUI_TEXT_RUNTIME_FACTORY.md` and the native node/model/group owner headers.
This is application wiring, not a new reconstruction or annotation batch;
no Ghidra symbols/comments or reconstruction ledgers changed.

## Validation and limits

The node translation unit is registered temporarily in the worker's local
`startup.cmake` for `scripts/build.ps1`; that CMake edit is restored before
delivery. The primary integrator owns permanent build registration and runtime
switches. Build result is recorded in the delivery report.

No new tests, game runs, OFF/ON comparison, screenshot, draw proof, original
widget binary-ABI proof or game validation is claimed by this worker. The typed
model-name overload retains its existing `NativeStringStorage::release`
`noexcept` boundary and fixed constructor constants; the Group root uses the
existing raw current-cell overload. Neither certifies native FH3/SEH behavior.
