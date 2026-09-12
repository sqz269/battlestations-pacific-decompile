# Ship navigation bindings

Project: `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
Descriptive names are hypotheses; process interfaces are not original object ABI.

The controls step now reads the controlled unit's recovered `+9C8h` hull extent
when computing braking distance. At `009ED8DA`, EAX comes from `blk+3FCh`;
`009ED902` adds `[EAX+9C8h]` after the speed/deceleration calculation. The old
binding returned zero despite the unit owner already exposing the field.

The MoveTo step uses the same field at `009E58D4`. Its EAX comes from the
controlled unit at `brain+AA8h`, loaded at `009E58C8`; it is subtracted from
the target's integer `+7A0h` range. The existing host method's radius name
does not change the native meaning: `+9C8h` is the full hull extent.
The target range producer remains unresolved and is recorded separately.

The field producer `00810F60` stores class `+A0h` at `0081106E` when there is
no model box. `GameUnitsHost::unit_hull_length_09c8` supplies this represented
path from its actual loaded motion class. The model-box path and the class
`+A4h` width producer remain outside this binding's validation.

Passing-corner diagnostics now count the native branch correctly. Recovered
`ship_ai_scan_obstacle_sector_009eb660` calls `009D84E0` at `009EBF67` only
when a blocking neighbour exists. Previously, the process emitted an
unimplemented-call record for the opposite case, including an empty list.
It now records the concrete call only when the scan took that branch.
This correction does not create neighbours or claim corner-detour coverage.

Win32 Release and the two existing CTests passed after the code change.
A subsequent USN01 runtime attempt reached the existing-instance error path;
another orchestrator's executable was confirmed live and left untouched.
That attempt provides no mission-frame validation of this change. Exact
hashes and the attempted command are in `reports/game_ship_navigation_binding.json`.

The parallel dependency packets reconstruct polygon partitioning, Dyn hull
production, and the ship hull-geometry pre-step. Their contracts and validation
are recorded in their individual reports before runtime integration.
