# Ship navigation bindings

Project: `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
Descriptive names are hypotheses; process interfaces are not original object ABI.

The executable now runs complete `009E0270` before navigation consumers. It
loads actual class `+570h` from the mission Lua owner before `009E4330`, using
the actual session mode and VehicleClass Type/variant selection documented in
[GAME_SHIP_DEPTH_INPUT.md](GAME_SHIP_DEPTH_INPUT.md). The constructor callback
uses its local navigation fields before assigning the completed block.

The pre-step borrows persistent geometry, twelve sectors, reference speed and
65 profile bytes. Flags `+3E8h/+3E9h/+3EAh` and profile `+45h` now have byte
storage; any nonzero profile flag survives exactly. The constructor supplies
the native cleared bins and flag before calling the pre-step. Its stack
argument is one unused word, not a float delta.

Actual unit reference speed, turn circle at 0.5, full width and cached world
pose supply the core. The represented modifier lists are empty. Native
`009E0270` floors reference speed at the widened `1.4f` comparison while
preserving unordered values. Later obstacle and throttle-ceiling inputs now
retain the produced `+3C4h` value, including the separate explicit input read
at `009EC9AB` that review caught still using raw unit speed.

The dimension producer is documented in [UNIT_HULL_EXTENTS.md](UNIT_HULL_EXTENTS.md).
Class `+A0h/+A4h` supply actual Length/Width for the absent-model path. The
model-local box branch uses twice the maximum absolute extent about the origin,
not max-minus-min. Historical `unit_half_width_09cc` returns the native full
width. Hydro retains the separate raw class width its own producer requires.
Native extent fixtures cover both branches; runtime covers the represented
absent-model owner.

Navigation blocks are now constructed only for resolved ship descriptor kinds
7 through 14. Their actual vtable `+210h` entries reach `00810DD0`, which calls
`009F3F20` to allocate the `2268h` brain through `009F3BA0`. Abstract kind6 is
not a concrete class row. This restricts navigation to the 14 ship instances
among USN01's 77 loaded units. The other 63 retain generic command and director
updates. Original allocation prefixes and resource/session gates remain
separate from this process-owned representation.

The retained `009DE2F0` geometry supplies goal position `+184h`, sector/throttle
position and axes, and clearance shoulders `+18Ch/+194h`. Cached-pose access
rejects an unsupported dirty cache; absence of a model is established by the
represented owner and native `0087BCC0`/`006D1E30` contract. The distance constant
at `00CF1748` is widened `0.45f`, bits `3FDCCCCCC0000000`; the constructor's
`009E45F3` square root uses the recovered CRT without a positive-only guard.

Earlier corrections remain: `009ED902` adds controlled-unit `+9C8h` to braking
distance; `009E58D4` subtracts it from target `+7A0h` range. That target range
producer remains unresolved. Passing-corner diagnostics record the
`009EBF67 ->009D84E0` call only when a blocking neighbour exists.

Win32 Release and both existing CTests passed. The integrated executable
completed 120 USN01 frames, including the Enterprise MoveTo request, with 14
navigation blocks, 3,374 full pre-steps, 2,400 controls extent reads and 48
goal-position reads. All runtime depth inputs selected Single mode0 and the
shipped scalar0. All 77 rows logged loaded dimensions. Shutdown was clean,
with no FMOD errors. Commands, source/executable hashes, exact input rows and
prior verification history are in `reports/game_ship_navigation_binding.json`.

The integrator also reproduced 1,449 original-byte extent comparisons, four
existing original-byte full pre-step fixtures, and the compiled Lua reader
excerpt over 160 supported rows and 473 rejected non-ship rows. The reader
fixture checks ten Single/Multi keys and boundary/error behavior; it is not
original-byte execution of the entire settings loader. No new tracked tests
were added.

Zero corner arms occurred. These checks do not establish model-present runtime
behavior, neighbour avoidance, collision-world integration or gameplay parity.
The pre-step's `+168h` is also not the adaptive planner layer `+30Ch`.

## Follow-up packets

Bind the stateful `009ECA20` producer for `+308h/+30Ch` before changing PathPick's
layer input. It starts with owner virtual `+214h`, which reads distinct class
`+560h`; submarines use the actual `+1268h` selector into that four-value array.
Do not substitute the now-loaded `+570h` scalar. Formation and RNG owners are
under other active packets; recheck leases before expanding this work.

Main's new `unit_kind_query` core supplies native class-test behavior. Its
GameUnitsHost consumers still need a separate binding to remove the Destroyer
ancestry assumption and preserve unresolved class identity explicitly. Keep
the explicit resolved ship guard until that runtime binding is complete.
