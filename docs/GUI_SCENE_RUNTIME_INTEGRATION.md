# GUI Screen scene ownership integration

Addresses: 009236A0, 00924050, 009242F0, 00924420, 00924480,
00925430, 00925470, 00925490, 00925540, 00AC5480, 00AC59A0,
00B002C0, 00B003A0, 00B004B0, 00B7C820, 00B8E6C0, 00BF7420,
00CD8A60, 00CE1040.

The Screen acquisition/release composition now builds with concrete weak-owner,
directional-light reference and group-bounds implementations. All operate on
the existing actual scene/node storage and reference counts. The C++ companions
remain new interfaces; this is not a binary replacement or game validation.

The tested source commit is recorded in
`reports/gui_scene_runtime_integration.json`. It integrates worker commits
3e361bc (weak owner), eca67fd (directional reference), 991b51c (Screen scene),
and 356ce9d (group bounds). Integration also removed an identical duplicated
camera-store header introduced while merging its shared dependency. The actual
D62D48 scene table has two relevant slots, BD30E0 and B72580; the following
B73970 word belongs to the next pool table.

## Ownership and corrections

The weak owner uses actual +04/+08 fields, the canonical 0109CE94 pool and
0109CE90 mutex publication. An externally retained weak handle survives owner
destruction with a null target. The directional companion borrows the same
native +04 count used by scene and hierarchy operations and returns its actual
pool slot before retiring its host companions.

Screen shared-store acquisition increments the existing actual scene and
returns before owned setup. Owned acquisition uses the canonical camera-store
map and actual camera, viewport, fog, GuiLights and directional-light owners.
The final flags OR 6 and clear-color writes target the camera, correcting the
older scene interpretation in GUI_RENDER_ORDER evidence. Manager AA31F0 calls
current virtual20 before deleting virtual04; AC5480 must not add a second
logical node release. See `docs/GUI_SCREEN_SCENE_RUNTIME.md` and the preceding
`docs/NATIVE_GUI_OWNER_INTEGRATION.md` for that destructor ordering correction.

B8E6C0 clears flags138 bits30 and byte175, then performs four sequential
FLD/FSTP pairs into08..17. These are floating-point loads/stores, including
alias propagation and x87 effects, not raw DWORD copies. Radius construction
uses the existing reconstructed CRT sqrt, a float32 spill/reload, the actual
0109EEA4 conversion selector and the existing BF7456 fallback. BF7420 consumes
ST0; it is not a clock. Its existing library name was retained.

## Verification

The integrated MSVC Win32 build and both existing CTests passed. All eight
native differential seed spans matched the installed binary. Three focused
ignored fixtures were rebuilt against the integrated library:

- Weak ownership: surviving null weak handle, 4097 slots, table growth32 to66,
  trimming with relocated slot IDs, and registered mutex shutdown.
- Directional ownership: actual +04 lifecycle1/2/1/1/0, shared child and point
  light logical release, destruction reentry, pool reuse and companion retirement.
- Group bounds: original44-byte writer matched the full18Ch slot and x87 status
  in three alias/FP cases. Original radius caller and conversion matched eight
  selector/rounding cases using the same reconstructed sqrt dependency.

The first two are host ownership checks. The last does not establish equivalence
to the original CRT sqrt. No new permanent tests were added. Fifty worker-local
artifacts were preserved and hash-verified under `local/gui-scene-workers/`.

Ghidra's project/program were checked for every live batch. CD8A60..CD8A76 was
defined after installed-byte comparison. Six returning-free continuations in
five functions were repaired; all six now have decoded continuations and zero
remaining call gaps. Unrelated jump padding and global callee no-return flags
were left intact. Nineteen selected names/comments were verified after locked
annotation, five pre-existing comment fields were preserved, the project was
saved, and all nineteen exports were refreshed. Detailed mutation records are
`reports/gui_scene_runtime_definitions.json` and
`reports/gui_scene_runtime_flow_repair.json`.

## Follow-up packets

- `orch2_gui_screen_fixture_q` is validating combined owned/shared Screen
  acquisition and final teardown with these actual owners. This integration
  record does not claim that combined check passed. Actual manager registration,
  optional visibility hooks and renderer/environment bindings remain required.
- `orch2_gui_native_geometry_q` reconstructs raw mesh allocation, canonical
  model+180 association and the bounded section publication suffix. Actual
  stream, material and layout projections must exist before connecting it to
  the typed GUI geometry rebuild or claiming renderable output.
- `orch2_gui_material_binding_q` reconstructs clip/owner/color binding. The
  current unique widget ownership does not provide a retained widget token;
  callers must not manufacture one or introduce a second authoritative count.
- Follow up on 0057BEC0 in `frontend_entry.hpp/.cpp` and
  `docs/GAME_FRONTEND_ENTRY.md`: the worker's listing shows BF7420 consumes
  original progress times128 after FCOMIP leaves that value in ST0. Existing
  `kLoadingProgressDeadScale`/`last_tick` and clock commentary require their own
  leased correction and caller verification. No frontend change is claimed here.

Full GUI rendering, exceptional native SEH equivalence, cross-thread weak
promotion and gameplay remain unvalidated. See the individual worker reports
for original ABI and per-function uncertainty.
