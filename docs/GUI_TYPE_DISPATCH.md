# Concrete GUI type dispatch

`GuiTypeDispatchFactory` constructs Group2, Icon6, FrameBox18 and script-backed
Screen1 companions over the same `GuiWidgetOwner::layout()`, scene flags, native
node and `extra_fields().overbright_94`. It creates no second base object or tree.
The C++ names are hypotheses and the interfaces are semantic reconstructions,
not binary replacements for native100h/124h/138h/11Ch objects.

## Integration and callback order

Construct the factory with actual resource services, put `make_factory()` in
`GuiWidgetOwnerEnvironment`, and retain the factory while preparing page inputs.
Inside the host's root creation, call `prepare_script_page(root, screen_flag)`
before `runtime.construct_root(root, actual_node)`. The exact byte is consumed
for that layout only. Cancel an unconsumed preparation if root allocation fails.
The callable itself retains its factory state. There is no global current-page
flag, and default/copy Screen construction without a script input fails.

The resource-service factories run during type construction, before node binding.
Their geometry/model callbacks must resolve the owner lazily when called. Icon
and FrameBox base loaded, position and recompose callbacks are bound directly to
the same owner; real geometry, renderer, texture, material, clip and color services
remain required. Unsupported widget types fail explicitly.

The retained host routes `on_widget_before_properties` to
`runtime.before_properties`, `on_widget_base_properties_bound` to
`runtime.base_properties_bound`, and the existing constructed, derived-property
and loaded hooks to their corresponding runtime methods. Ordinary children run
constructor, bind/parent, +74, pre-base hook, base properties, base publication,
children, derived properties, +78 in that order. Group's reader00AC6FD0 is simply
a jump to00AAA710, so it has no extra derived continuation.

Screen's reader00AC4C50 has a different composition: read Priority into both+FC
and+118 and RenderOrder into+11C; call current+78; bind the actual root node to the
scene; set base position to zero; then read base properties and children. Its
pre-base hook performs that prefix. Page loading already omits a trailing child
+78 call. Calling `runtime.loaded78(screen)` explicitly still invokes its actual
camera-store slot; it is not silently suppressed. Screen base Visible is skipped
by native00AAA710's +5C type check, as in the existing loader/runtime.

Use `owner.set_visible34()` for current-type visibility. The original base
`set_visible_00aa8530()` remains separately callable. Screen34 uses+F5 rather
than the node's+ACh factor: on an edge it changes the SAME store's+20 count, then
always propagates visibility, binds/unbinds the actual node, notifies an installed
hook only on an edge, and writes+F5 last. It preserves native32-bit count wrap.
The type's derived retirement runs before the base scene-node/tree release.
Root binding00AA6720 is explicitly nonthrowing: it only stores the binding and
identity and clears native node flag bits. Failed type construction still removes
the layout callback and owner association. A duplicate native model-pool address
is rejected before the allocation unwind, preserving the already-live slot.

## Vtable and instruction evidence

Live reads used `bsp.py ghidra`, whose client verifies project `bsp`, program
`/battlestationspacific.exe`, language and image base before each query. Local
configuration points to `C:/Users/sqz269/bsp.gpr`. Analysis and disk decoding were
read-only; no Ghidra names, comments, function bodies or flow overrides changed.

| Type / table | +34 | +38 | +3C | +60 | +74 | +78 |
|---|---|---|---|---|---|---|
| Group2 /00D5CB80 |00AA8530|00A9E0D0|00A9E100|00AA6A30|00A9AC00|00AA7170|
| Icon6 /00D5C4C0 |00AA8530|00A9E0D0|00AB6120|00AA6A30|00AB2540|00AB10F0|
| FrameBox18 /00D5D130 |00AA8530|00A9E0D0|00A9E100|00AA6A30|00ACF8F0|00ACEB50|
| Screen1 /00D5BE38 |00AC4450|00AA38E0|00A9E100|00AA6A30|00A9AC00|00AC59A0|

Icon3C is not the base RET4. Undefined Ghidra bytes00AB6120..00AB639F (640 bytes)
decode through RET4 at00AB639D. Per record,00AB6189/6194 test+3C and visible for
load;00AB6327/6332 test+3D and hidden for placeholder replacement/rebuild. If
neither branch applies, only loop bookkeeping occurs. The existing runtime
rejects DelayedTextureLoad before applying properties; normal add-state records
have both flags clear. The adapter implements this proven supported branch and
throws if a delayed load/unload is needed. AutoRotate remains rejected by the
existing runtime. No async loader callback is invented.

Undefined00AC6FD0..00AC6FD4 (five bytes) is a tail JMP00AAA710, followed by INT3.
The Group constructor00AC6F50..00AC6F63 passes type2 to00AA9390 and stamps00D5CB80.
Allocator00AA12F0..00AA1373 selects this constructor or copy00AC6FA0; physical
widget-pool allocation/copy ABI is not reproduced by this logical factory.

Assembly corrects older gui_layer documentation:00AC6600 is ECX=destination,
stack `(NativeString* name, native_node, byte screen_flag)`, RET0C. At00AC66A4 the
third argument byte loads from ESP+510 and at00AC66EE stores+120; ESP+50C loads
the node, which is passed to00AA6720. At00AC6705/670A the stores to base+20/+24
set SIZE to(1,1), not scale. Its script constructor also zeros derived+EC,+F0,
+F5,+F8,+FC and+121. It does not initialize+F4; the real acquire78 callback does.
Default00AA3840..00AA38A8 leaves scene/store/ownership/visibility/+120 unwritten,
so this factory does not fabricate that preimage.

| Address / inclusive end | Original convention | Implemented boundary |
|---|---|---|
|00AA12F0..00AA1373|ECX optional source, EAX object, RET|logical fresh Group dispatch only|
|00AC6F50..00AC6F63|ECX destination, EAX same, RET|base type2 and derived selection|
|00AC6600..00AC6878|ECX destination, stack name/node/flag, RET0C|derived script-constructor fields; existing loader executes scripts|
|00AC4C50..00AC4D27|ECX layer, stack visitor, RET4|ordering and pre-base prefix using evaluated Lua values|
|00AC4450..00AC44CE|ECX layer, stack bool, RET4|visibility count/propagation/bind/hook/order|
|00AA38E0..00AA38E6|ECX layer, AL byte+F5, RET|layer visibility reader|
|00AC59A0..00AC5F5A|ECX layer, RET|required actual acquire callback|

Live flow reports found zero listing gaps in00AA12F0,00AC6600,00AC4C50,
00AC4450 and00AC59A0. The reviewed free calls in the script constructor have
normal listed fallthrough; no false-free/no-return repair was applied. The two
undefined callback ranges above were decoded from the installed read-only PE.

## Required services and fidelity limits

Screen acquisition still requires the actual camera, outer scene, lights and
camera-store owners. The callback must implement00AC59A0 including its shared
store early return, before the owned-scene tail. The same `GuiCameraStore` and
`GuiCameraStoreKey` definitions from `gui_render_order.hpp` are reused; the old
`GuiLayerImage::base` diagnostic projection is not populated or synchronized.
Scene binding, optional-hook dispatch and partial/complete derived release are
also mandatory services. The release callback must distinguish owned-store
removal from shared-scene release and must not throw.

This packet binds startup type slots and synchronous Icon/FrameBox behavior.
It does not implement every per-frame virtual, native widget allocation/copy ABI,
screen camera/light acquisition itself, or establish renderer/game parity.
Validation details are recorded in `reports/gui_type_dispatch.json`.
MSVC Win32 Release and the two existing CTests passed after native seed-byte
verification. The ignored `local/gui_type_probe.cpp` reuses the existing native
model owner fixture, executes a Lua5.1.1 page through the real loader, and checks
Group construction, missing-service/input rejection, exact0x81 Screen input,
pre-base acquisition order, layer-versus-node visibility, edge counts/hooks and
derived-before-base release. Camera/scene operations in that probe are explicitly
fixture callbacks. Icon/FrameBox rendering was not exercised by this probe.

# Integration correction: manager page disposal

`docs/GUI_NATIVE_SCENE.md` establishes `00AA31F0`: current virtual20 runs
before deleting virtual04(1). Derived `00AC5480` still precedes base `00AA9730`
inside destruction, but widgets are already logically unbound at that point.
The retained runtime now keeps these two operations separate. The original
isolated callback probe was adapted to assert a null node in derived cleanup;
an actual outer-scene/group/model fixture verifies the complete disposal order.
This does not supply camera acquisition or native weak-base services.
