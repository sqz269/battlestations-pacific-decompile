# Mission observer runtime binding

Addresses: none (source application composition).

`GameMenuHost` now binds each newly allocated mission to the existing
`GameSingletonHost::observers()` runtime before it can load units. The mission
and frame forward the same borrowed runtime to stable `GameUnitSlot` owners.
The actual native endpoint prefixes and teardown behavior are documented in
[GAME_UNIT_OBSERVER_BINDING.md](GAME_UNIT_OBSERVER_BINDING.md).

The menu owns the mission after its HUD member, so the mission and its unit
borrowers are destroyed first. Existing application shutdown deletes the menu
before draining the raw singleton manager. The unit teardown log records the
live dispatch owner and callback-before-observed order; the combined T run
checks that line occurs before the manager-drain line.

This source wiring does not establish complete native unit construction,
nonempty gameplay notification, repeated mission loading or native ABI parity.
Validation is recorded in `reports/game_observer_mission_binding.json`.
