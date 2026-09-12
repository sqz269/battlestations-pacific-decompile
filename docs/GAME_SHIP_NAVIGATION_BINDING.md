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

The executable now calls recovered `009DE2F0` during navigation construction
and each controller pre-step. Construction passes its local fields before
assigning them to the controller. Each call reads the unit's actual cached
world transform, rejecting an unsupported dirty cache. The represented owner
creates no model/parts: native `0087BCC0` and getter `006D1E30` establish the
absent-model branch. This is a scoped binding, not a claim about every model.

The persistent output supplies goal position `+184h`, sector/throttle position
and axes, and clearance shoulders `+18Ch/+194h`. The constant at `00CF1748`
is widened `0.45f`, with double bits `3FDCCCCCC0000000`; the constructor now
uses that exact value. Its `009E45F3` square-root adapter invokes the recovered
CRT kernel with actual application CRT access. Review caught and removed the
old positive-only guard, which incorrectly converted negative/NaN inputs to zero.

Win32 Release and both existing CTests passed. The corrected executable then
completed120 USN01 mission frames with18557 hull-geometry updates,17520 concrete
controls extent reads,48 goal-position reads, no FMOD errors and clean shutdown.
The earlier existing-instance attempt remains recorded separately; the other
orchestrator's process was left untouched. Commands, hashes and logs are in
`reports/game_ship_navigation_binding.json`.

The complete `009E0270` core implementation exists, but the executable still
records its parent pre-step as partial: actual class `+570h` depth, canonical
profile/flag inputs and real width remain unbound. The width getter currently
returns a placeholder `1.0f`. Model-present and dirty-pose branches were not
exercised by this runtime, which also produced zero corner arms. These checks
do not establish corner-detour, collision-world or original gameplay parity.

## Follow-up packet: actual avoidance-depth and complete pre-step binding

`GameMissionLuaHost` already owns the loaded ShipGlobals tables. A typed reader
can use actual `GameUnitRow.type_id` to select the VehicleClass row, its
`HeavyCruiser`/`BigLandingShip` flags, and the existing
`kShipLeafTuningSources` mapping. Native `0083B5E0` loads the flat
`ShipGlobals.AvoidZoneDepthsSingle/Multi` arrays; `00837DE0` selects Single
only for actual session mode zero. Retain the selected first array value as
class `+570h` before AI construction. Do not use the later controller's legacy
hardcoded session value or wait for the later avoid-zone load.

Existing unit APIs provide reference speed and turn circle at0.5 under the
represented empty modifier lists. Complete binding also requires real `+9CCh`
width, canonical byte flags/profile storage, and removal of the later raw
reference-speed assignment that would overwrite the pre-step's floor. The
existing bool fields must not be reinterpreted as borrowed uint8 references.
The65 profile bins and12 sectors already have process-owned storage. Recheck
leases before editing: the Units files belonged to another active packet
when this follow-up was assessed.
