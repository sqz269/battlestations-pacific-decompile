# Screen scene acquisition and derived retirement

`GuiScreenSceneRuntime::make_services` supplies the existing
`GuiScreenLayerServices` callbacks over the same `GuiWidgetOwner` and
`GuiScreenLayerState`. It composes actual owners with the canonical
`GuiCameraStoreMap`. Its store-to-companion associations borrow identities only:
they duplicate no native fields, reference counts, visibility counts or trees.
Externally created canonical stores can be registered with `bind_existing_store`.
`camera_for_store` and `scene_for_store` return the same borrowed canonical
references for the existing orthographic and render-queue adapter. Queue
retention remains explicit; removed stores cannot be looked up.
The environment, association service and light lifetime provider must survive
their native owners and any queued references. This is a new C++ interface;
descriptive names are hypotheses, not recovered symbols or binary replacements.

## Native evidence and corrected interpretation

| Body, inclusive span | Original ABI | Implemented boundary |
|---|---|---|
|00AC59A0..00AC5F5A (1467 bytes)|ECX layer, no consumed EDX argument, RET, void|Successful acquire78 composition with precise supplied native services|
|00AC5480..00AC5554 (213 bytes)|ECX layer, no consumed EDX argument, RET, void|Derived store/scene/name retirement; existing runtime owns base destruction|

Live read-only batches verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Both bodies have zero flow gaps. Assembly was
required because the decompiler loses ECX receivers and the x87 conversion.
The report retains the old Ghidra names/comments and byte evidence. No Ghidra
mutation or correction to historical comments was performed in this packet.

At00AC59D3 the native lookup uses the descriptor slice+108. Its result is
published to+F0 even when the exact+120 byte disables reuse. The script
constructor initializes+EC/+F0 to null and leaves+F4 unwritten; acquire establishes
its meaning. With a nonzero+120 and a matching store,00AC59EA..00AC5A19 writes
owns+F4=0, copies store.scene to+EC, increments that actual outer scene's+04,
and **returns**. It does not configure lights, camera flags, priority or bounds.
The descriptor's Priority is excluded from matching; RenderOrder remains the
single map's ordered key. Acquire changes no visible-layer count.

The owned branch sets+F4=1 before allocating the outer24h scene. It constructs
the actual camera using `GuiCam_` plus the name, publishes a24h store, constructs
the distinct3Ch `GuiLights` resource and installs it in outer scene+1C. It drops
the lights allocation reference, binds the camera into the same root chain,
sets camera clear flags7/color0, then installs a newly allocated viewport and
fog owner, releasing their temporary allocation references. Fog+68 and+78
receive positive zero before publication. The camera constructor's original
viewport is replaced through the existing retain/publish/release setter.

`GuiDirectionalLight` uses the actual pooled directional-light owner and its
canonical lifetime provider. Current native vtable00D62FB0+34 is00B6E870 and
+40 is00B6DBE0. The adapter checks this profile and uses the existing transform
implementation with a matrix whose diagonal comes from live00D7A24C. It writes
direction+1E0=(0,0,one), applies base diffuse(0,0,0,one) through004B62E0, then
binds the light into the same outer root chain. There is no extra light/camera
retain: root logical release later consumes each constructor's initial self
reference. The actual GuiLights+10 ambient owner receives(half,half,half,one),
using live00CE3800 and00D7A24C and the existing00B7AF20 setter.

**Correction to older GUI layer/render-order comments:** at00AC5EA0 and
00AC5EC6, ECX comes from **store+18 (camera)**. Flags `key.flags | 6` go to camera
+188 through00B6FE10, and color zero goes to camera+190 through00B6FE50.
Neither operation writes outer-scene flags. The adapter reloads the canonical
camera binding for both calls. When+FC differs from+118, it writes+FC first,
then invokes the real page-priority registration service00AA52A0.

If base+74 (`bounds_enabled`) is clear,00AC5EF8..00AC5F3F loads center half/half/0,
loads double2.0 at00D7A308, calls CRT sqrt00BF7030, spills/reloads float32,
converts ST0 to EAX through00BF7420, converts that signed integer to float, then
loads the current root+4C and invokes00B8E6C0. The latter clears node+138 mask
0x30, clears the **group** byte+175, and writes sphere words+08..17.
00BF7420 is a truncating float-to-int helper, not a clock: its inspected SSE2
path uses FSTP double/CVTTSD2SI; its fallback begins00BF7456. Exact CRT/FPU
effects remain in the required service; no constant radius1 is substituted.

## Retirement and caller ordering

Derived00AC5480 removes/frees the owned store first, then reloads+EC and calls
00B72250 (a decrement, not forced scene destruction). Shared retirement directly
decrements the same outer-scene+04. Both clear+EC only after the potentially
terminal callback. Native leaves+F0 dangling; the adapter does not clear it or
change+F5/counts. It releases the semantic name before returning to base cleanup.
An owner-created store can disappear while another layer still retains the
scene; native does not repair the other layer's borrowed store pointer.

The native disposal caller00AA31F0 explicitly invokes current+20 at00AA326A,
then current deleting+04(1) at00AA3276. Thus recursive00AA8320 releases/nulls the
GUI node tree **before**00AC5480. The latter eventually calls base00AA9730,
which calls00AA8320 again on the already-null tree. The outer scene's final
destructor releases lighting then consumes every current root through
00B6DFA0/current+18. The integrator's separate GUI runtime correction supplies
this manager disposal sequence. Calling derived retirement first on a live GUI
tree can leave dangling node bindings; this adapter does not conceal that with
an invented retain or an extra release inside00AC5480.

## Required services and fidelity limits

- `NativeGuiSceneWeakBase` must implement the actual00925490/00925540 weak-handle
  owner/pool. The outer scene dependency intentionally supplies no fake handle.
- `GuiScreenSceneNativeCalls::allocate_directional_light_00b7c6b0` must return
  one actual DirectionalLightOwner and its canonical GeneratedModelNodeLifetime
  bound to the same runtime and the same native+04 RenderCommandReference. It
  must use real pool allocation/constructor/terminal virtual18/54/00 dispatch.
- The root-bounds service must target the actual cGroup tail, and the radius
  service must execute the supplied CRT sqrt/spill/conversion policy. A model
  tail or a synthetic bounding-volume object is not interchangeable.
- Page registration and optional visibility-hook dispatch still require their
  real manager/global bindings. An absent hook is represented by the actual
  installed query returning false; the adapter adds no unconditional hook.
- The supplied existing NativeCameraEnvironment/NativeGuiSceneEnvironment,
  lighting resolver and live constants retain their own documented requirements.
  Orthographic view/projection setup and queue submission occur later in the
  existing GUI render-order path; acquire does not perform them.

The native function guards individual allocation/constructor and string
temporaries with EH states0/1/2/3/6/7/9/10/11/12; it does not maintain an owning
RAII stack for every already-published resource. Existing native owner factories
retain their constructor cleanup contracts. This semantic adapter uses temporary
pooled strings for its std::string name projection. Its lifecycle validity bits
avoid deleting a pre-existing lookup result when a host allocation throws before
the layer creates its own store. This is explicit partial-construction recovery,
not a claim of binary SEH equivalence. An insertion failure and a failure after
an unattached camera/light construction can retain the same unresolved native
allocation obligations; no general rollback or fabricated success is supplied.
Repeated successful acquire78 retains native overwrite/no-old-release behavior.

MSVC Win32 Release `/W4 /WX` and both existing CTests passed after seed-byte
verification. These checks do not execute this new Screen composition or prove
its required providers, native differential parity, rendering or game behavior.
