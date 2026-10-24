# October 24 native GUI texture binding

Step 2 replaces the executable host's `GuiManagerResources::renderer_load_texture`
stand-in only when environment variable `BSP_NATIVE_GUI_RESOURCES=1`. The runtime
switch is read once and defaults OFF; both arms always compile. The primary GUI
lane owns the CMake source addition and application startup/drain hooks.
Step 1 text rendering must land first. This component creates no GUI singleton,
page, widget, projection state or gameplay object.

## Evidence and scope

The existing saved `bsp.gpr`, program `/battlestationspacific.exe`, was live
with 77,629 functions on October 24. `config/target.json` identifies
`C:/Users/sqz269/bsp.gpr`; the live AA5E20 query confirmed the program name.
The existing export and assembly for AA5E20 were inspected before implementation.

- AA5E20 is ECX manager, no stack arguments, RET. It copies
  `data/interface/textures/whiteGui.tga`, calls actual VFS BDF4C0 at AA5E88,
  and only on success calls current renderer `*(00F8D394)` virtual +64h at
  AA5EA2, passing the same native string header and callback word zero.
- The returned reference is assigned directly to manager+28h at AA5EA4.
  Transparent uses `interface/textures/common/transparent.tga`, calls the same
  virtual at AA5EF0 with word zero, then replaces manager+2Ch at AA5F1A.
  Existing +2Ch is decremented at AA5EFF and destroyed through current virtual0
  only at zero. Both initial frontend slots are empty for this bounded binding.
- The concrete observed renderer profile D5F0A8 has +64h = B319B0. Its existing
  reconstructed loader has ECX renderer, two DWORD stack arguments, RET8.
  The AA5E60 host log label lies inside AA5E20; it is not a separate function.

`GameNativeGuiResourceApplication` uses the renderer's existing texture cache,
VFS/string domains, canonical actual-owner registry and D5F0A8 profile. It uses
the SAME actual BDF4C0 resolver factory for whiteGui's outer guard. Each entered
resolver/cache/native-string operation has address-stable retained storage.
No native numeric address is executed as a host function pointer.

The component owns exactly each returned caller reference; the frontend's
existing projected manager fields borrow those identities. B319B0's existing
cache owns its separate native cache credit. No COM AddRef, substitute image,
private cache, second registry, counter or pool is introduced. The native raw
resource is never inserted in the bridge's `IDirect3DTexture9*` ownership vector.

## Integration contract

Add `src/game_native_gui_resource_application.cpp` to `bsp_game`. After creating
the frontend, before `run_font_and_gui_startup_0073bae0`, call
`frontend->bind_native_gui_resources(renderer)`; this returns immediately while
the runtime switch is OFF, before touching the renderer/VFS. ON binding
requires an unconstructed, empty frontend manager. Only the two initial entries
are admitted, in whiteGui then transparent order. This does not support reload,
transparent replacement from an existing nonnull initial slot, or the native
GUI manager destructor.

Before renderer drain, end all users of the manager texture fields, then call
`frontend->retire_native_gui_resources()`. It clears the two borrowing manager
slots and calls `release_native_render_actual_owner` once per returned nonnull
reference, retaining its decrement/terminal observation in the same frame.
Ownership is marked consumed before entering any terminal callback;
interrupted retirement cannot be retried. Native cache cleanup remains the
renderer owner's work. Retain the frontend/component, renderer, original VFS,
string pool, operation records and canonical owners on any interrupted native
load/retirement. `native_gui_resources_require_process_retention()` exposes this
obligation; it is also true during a healthy run while owned references remain.
The component destructor rejects destruction with outstanding retained work.

OFF preserves the existing host texture loader and UNIMPLEMENTED log entry.
ON replaces only the two manager texture acquisitions and that log entry. Page,
widget and sprite-bridge texture ownership are unchanged, so this step alone
does not activate the parked native GUI draw chain. New names are integration
descriptions, not recovered symbols; this API is not original ABI compatible.
Each texture call has its own native-string scope; the original enclosing
AA5E20 stack/SEH lifetime and exception unwind timing are not reproduced.

## Validation

Source review confirms the two reached assembly calls, zero callback words,
the whiteGui resolution guard and the initial +28/+2C slot constraints.
Build validation and OFF/ON game evidence are recorded by the primary lane in
`docs/OCT24_GUI_RENDER.md`. Compilation, OFF identity, ON runtime retirement and
screenshots remain separate gates; no game run or visual proof is claimed here.
