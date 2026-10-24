# Milestone D: borrow the mission's current units

`GameNativeMissionUnits` is a new C++ render binding over the already constructed
`GameUnitsHost`, its existing `GameMissionLuaHost`, and the mounted `GameVfsHost`.
It owns only borrowed pointers. It creates no units, Application, Lua state,
mission load, physics state, or preload. It retains no simulation matrix.

`GameMissionHost::native_render_unit_source()` forwards through the current
`GameMissionFrameHost`. The frame accessor returns its existing `units.get()` and
`lua` reference. Before that owner exists the source is empty. The renderer must
query this chain each render frame, after simulation, and must not retain the
source across mission/frame-host destruction. These two accessors contain no
gameplay stores. The corresponding four files were unleased when claimed;
cc9 and cc15 were notified through coordination messages 830 and 831.

## Resource identity

`count()` reads `GameUnitsSummary::units`, which `create_units` refreshes from
`slots.size()` after every batch, including runtime spawns. `descriptor(index)`
uses `unit_row` only for instance/class/type identity. It reads the same installed
`VehicleClass[type_id].Mesh` string that the existing unit host uses for model
Aux and buoyancy reads (`class_buoyancy_list_0082fe30`). It verifies that logical
asset through the existing mounted VFS and preserves the original logical name
for `load_installed_unit(logical_name, unit_index)`. Missing rows, missing Mesh
strings, and missing installed assets fail explicitly. There is no Fletcher or
ship-only asset table and no alternate asset chosen on failure.

This is the existing live Lua table reader, not a second parser. Model selection
follows the host's loaded class table and its existing load/purge policy. Resource
loading remains the model renderer's responsibility. A hidden unit can still
yield its descriptor, allowing the same resource to appear when later revealed.
The existing shared field schema records the original Mesh read at `0087CB44`
inside `0087CA80`: `__thiscall` descriptor receiver, stack Lua-object row, `RET 4`.
`00960230` calls that common loader before its vehicle-specific fields. The
adapter uses the existing C++ Lua reader rather than invoking that native ABI.

## Pose and visibility evidence

| Existing provider | Native evidence and original ABI | Binding use |
| --- | --- | --- |
| `GameUnitsHost::unit_pose` | The same canonical slot `world` whose rows correspond to unit `+CCh` right, `+DCh` up, `+ECh` forward, `+FCh` translation; read by `0063A6C0` and `00427EB0`. `004142E0` is a `__thiscall` affine point transform: ECX source vector, stack output and matrix. | Reads all 12 xyz lanes into a temporary `CameraMatrix` on each query; affine fourth lanes are `0,0,0,1`. No heading-only reconstruction or stored world pose. |
| `unit_alive_and_visible` | `0043F080`: ECX unit receiver, no stack arguments, EAX boolean. Tests `+5Ch != 0`, `+5Dh == 0`, `+60h == 0`, `+5Eh == 0`. Assembly inspected. | Reuses the host's current lifecycle cells. |
| `unit_hit_node_detached` | `00951F40`: ECX receiver, stack visibility byte, `RET 4`. With part `+4A4h`, attaches/detaches `+360h` through `00710AD0`/`00710B80`, then calls `00B6DA70` with visibility factor 1/0. Assembly inspected. The host represents this state with `plane_hit_node_detached`. | Suppresses host-hidden hangar/elevator/SetVisible planes as well as lifecycle-hidden units. |

The renderer calls `read_transform(index, temporary)` and `visible(index)` each
render frame. Ships, planes and submarines share this canonical world matrix.
The transform query refuses an unset current `+C8h` world-valid byte rather than
refreshing or drawing a stale pose; the existing host remains its only producer.
The renderer must pass the temporary transform directly to the native model draw
adapter; it must never write it back into gameplay or keep it as simulation state.

Do not use `units()` or `GameUnitRow::position`/`heading_degrees` for drawing:
`units()` rebuilds a diagnostic flat copy, and the plane step updates diagnostic
position only under `kPlaneRowPositionBound`, leaving ship-only row fields alone.
The current affine basis can contain plane pitch/roll and ship/submarine motion
that those summary fields do not express.

## Evidence limits and integration

The live Ghidra process was verified as `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 Windows, image base `00400000`. The original
ABI notes describe those existing native operations. This adapter and the
borrowing accessors are new C++ interfaces, not binary replacements. No Ghidra
annotation or ledger claim is added for this wiring.

Visibility is limited to state the host exposes: lifecycle gating and the
existing binary plane visibility operation. It does not claim recovered
continuous alpha, native culling, LOD, or a complete original render scene graph.
No active simulation-camera accessor was found in the two mission-host headers;
any whole-scene fit camera remains diagnostic until a real camera owner is wired.

Primary integration registers `src/game_native_mission_units.cpp` in
`cmake/startup.cmake`, supplies a fresh source through the existing menu/mission
owner, and handles the Win32 build and OFF/ON comparisons. This worker performed
source/assembly inspection and diff review only; no full build or game run was
requested from this packet. Build, visual and gameplay claims remain separate.
