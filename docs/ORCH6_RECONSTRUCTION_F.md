# Orchestrator 6 reconstruction batch F

Addresses: `00C41AD0`, `00C38070`, `0040A1E0`, `009ECA20`, `006DFD80`,
`00852FD0`, `00412170`, `00412180`, `004121B0`, `00417D60`, `004121A0`,
`00417B10`, `0042BB40`, `006FE460`, `0083B5E0`, `00837DE0`.

Work continues in `agent/orch6-20260912` at
`J:/PROG/battlestations-pacific-decompile-orch6-20260912`.
The full game reconstruction goal remains active. Saved Ghidra remains
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; installed data is read only.

## Integrated changes and evidence

- Actual unit class predicates now serve public, controlled-unit, hydro and hull
  queries. See `GAME_UNIT_KIND_BINDING.md` for corrected ancestry and unresolved
  listener/publication behavior.
- `DYN_SCENE_RUNTIME.md` reconstructs scene dispatch tables, SAP, manifold pool,
  intersection tasks and event reserve. Four original-code cases match 78 buffers.
- `DYN_WORLD_RUNTIME.md` completes fresh world storage, all task vectors and late
  shared motion. Four original-code cases match 118 buffers and x87 flags/TOP.
  The original constructor uses `RET8`, correcting its earlier cdecl annotation.
- `SHIP_AI_LAYER_SELECTION.md` reconstructs the full layer selector and eight
  getter/manager/group dependencies. 3,072 original selector cases and 68 helper
  cases pass; every one of 40 main external callback sites is observed.
- `GAME_SHIP_LAYER_INPUT.md` reads full installed tuning and six avoidance timers
  through protected Lua adapters. Its source-excerpt fixture passes for 160 ship
  rows and 320 session reads while preserving scalar-only depth behavior.

The combined Win32 build at `541d39cf54c7989bc955b2fdd8d14823f27cc672` and both existing CTests passed.
A final installed USN01 mission ran all 120 requested frames, kept 77 loaded
units and 14 ship navigation controllers, and exited 0 with zero FMOD errors.
Its executable hash is `9b3d028b9783b16915d0d53d5eb9f1f780e52d0e530c15d785620bf6ad5a55e4`. The exact command, source
hashes, preserved executable and logs are in `reports/orch6_reconstruction_f.json`.
The world constructor and layer selector are compiled but remain unbound to this mission.
No gameplay or visual fidelity is inferred from these checks.

## Next bounded work

Recheck current leases before actual layer runtime wiring. During this batch,
the startup host files belonged to `orch4_game_input_runtime_af` and mission
files to `orch5_picture_integration`; mission_frame.cpp had become unleased.
These are a dated ownership snapshot, not an instruction to wait after release.

Main already supplies the canonical RNG range implementation in
`native_particle_object_state.hpp/.cpp` (ea800790). Borrow the existing initialized
`GameStartupHost::random_threads_` through startup/menu/mission/frame ownership;
do not create another RNG or reset its stream history. Its access also needs
actual current scalars at `CE3978` and `D63B80`. Constructor draws use stream0;
layer-selection draws use stream1. Initial goal caches are **1e9**, distinct
from the selector's **1e10** non-navigation reset.

Use the real four-element class560 tuning, actual submarine depth1268 and
formation0070E450 aggregate, with the existing persistent navigation fields.
Class570/nav168 scalar depth is not travel layer30C. The selector report records
the exact initializer and consumer chains for308/30C/310/314 and31C/320.

For dynamics, recover the actual engine/task-manager owners at00C55F50,
00C55EA0 and00C37740, followed by task execution, destruction and collision
stepping. The world fixture's borrowed engine projection is not that runtime.
Retained fixture sources, binaries, records and hashes are under the primary
worktree's ignored `local/`; the batch report maps every copied worker artifact.
