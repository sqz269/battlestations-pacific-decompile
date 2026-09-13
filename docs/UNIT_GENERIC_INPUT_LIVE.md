# Generic unit tick runtime binding

Addresses: 00953CC0, 00953CFD, 0095DC40, 0095CC90, 006D1F20.

GameUnitsHost now runs the reconstructed 00953CC0 sequence for the three
proven generic leaf dispatches: LandVehicle, LandFort and CommandBuilding.
Their +1F0h target is 0095DC40 and their +1D8h target is 006D1F20. The
existing class/allocator dispatch selects these identities; speed and pose
values do not select a motion class.

Each runtime unit owns the constructor-established +528h=-9, +634h=0,
+6F8h/+6FCh=0, +520h=0 and +63Ch=1 cells. Native stores and their register
provenance are documented in UNIT_GENERIC_MOTION_PHASE.md and
UNIT_GENERIC_INPUT_PHASE.md. A temporary pure-routine view borrows their
values and the canonical current role0; mutated fields are written back.
The runtime does not introduce another current-role owner.

Before each call, current roles0 and4 must both equal8 and +634h must be0.
These conditions establish that 00953CC0 does not read unresolved byte+61h
or call the participant role predicate, and that 0095DC40 takes its complete
false-role path regardless of the local-player index. That path stores1 at
+63Ch, compares the just-stored value and returns without any input, view,
settings or timestamp access. Its dedicated M helper preserves that sequence.
The temporary view's unused gate member is explicitly not a native value.

The final virtual executes the verified RET4 no-op's semantics. The discarded
+5Ch query binds the existing class predicate if its native notify gate is
ever reached. Any frame outside the supported role/flag domain records an
unavailable generic tick and retains the unresolved-phase report. This is
conditional runtime coverage, not a complete ownership or role-message port.

The existing fixed-step motion pass supplies the schedule. Its position
relative to director, gunnery and ship-AI passes remains the documented
process adaptation. No original-game ABI or gameplay/visual parity is claimed.

L's combined executable at88dc00d4 passed Win32 and both existing CTests and
ran120 frames before this binding:18557 finite trajectory rows,2400 avoidance
queries,1080 cruise-owner reads and zero unavailable reads. Participant pools
used authored scene count8, player0 plus mission1..7, retaining device bytes.
The binding's own combined build/runtime evidence is recorded separately in
reports/unit_generic_input_live.json when verified.
