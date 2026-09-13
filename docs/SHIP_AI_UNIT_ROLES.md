# Canonical unit current-role storage

Addresses: 00928630, 00928713..0092874D; read-only writer evidence 009281C0,
0077F480. Packet `orch6_unit_roles_k`.

`GameUnitsHost` now owns one nine-element signed dword array in each existing
`GameUnitSlot`. Its constructor fills every entry with the existing
`kUnitRoleTableFill` (8), using `kUnitRoleTableEntries` (9) from
`unit_instance_layout.hpp`. No role copy is added to ShipAI, Commands, the Lua
host, or `UnitInstanceState`.

The public read is:

```cpp
bool unit_current_role_slot(std::size_t index, std::int32_t role_index,
    std::int32_t& out) const;
```

A created unit and role index 0 through 8 return `true` and the stored value.
A missing unit, negative role, or role above 8 returns `false` without changing
`out`. The value 8 is an available **unassigned role**, not an unavailable read.
The lookup does not infer an assignment from Party, selected-unit publication,
script intent, or participant state.

## Native producer and ownership

`00928630..0092878F` is the game-entity constructor. ESI receives ECX at
`0092864A` and remains the owner through the role stores. After virtual
`148h(1FFh,9)` at `00928711`, `00928713` loads EAX=8. Nine six-byte stores at
`00928718`, `0092871E`, `00928724`, `0092872A`, `00928730`, `00928736`,
`0092873C`, `00928742`, and `00928748` write native offsets `+1ACh..+1CCh`.
The complete immediate-plus-stores fragment is `00928713..0092874D`, 59 bytes.
The constructor's `RET 8` at `0092878D` confirms two stack arguments; this
packet does not reconstruct its other calls or ownership.

These current assignments are distinct from the nine-word policy table at
`+188h..+1A8h` and the separate `+180h` owner-policy slot. The process array is
an explicit constructor-state projection, not a raw native-layout overlay.
It lives with its `GameUnitSlot` and is initialized before that slot is
published by `create_units`. The getter returns a copy; it exposes no mutable
alias. All created slots represent this constructor fragment, so an extra
availability flag is unnecessary. No external allocation or borrowed storage
is introduced.

[SHIP_AI_ROLE_OWNERS.md](SHIP_AI_ROLE_OWNERS.md) establishes that the proven
ship-bot attachment resolves `[[unit+740h]+50h]` back to the same ship. Future
cruise bindings must therefore read roles 0 and 1 from this canonical owner.
Assigned values require the actual session participant owner; this packet
supplies neither that owner nor a replacement AI-held predicate.

## Assignment remains pending

No setter is exposed. The observed `009281C0..009281FE` receiver handles role
0 by clearing `+184h` when assigning 8 and calling `00927F60` before the store
at `009281E2`. Other roles store at `009281F3`; both return with `RET 8`.
`0077F480..0077F50B` calls that receiver at `0077F48F` and then performs class,
policy, active-participant and Party comparisons, with a further `0077EDF0`
call and byte store on its accepting path. Its ABI also uses `RET 8`.

The earlier role-owner assessment additionally identifies the acquisition and
release gates, observer/control work and relay in kind `4Bh` of `00780120`.
An unconditional local store would omit those contracts. Until actual delivery
and these side effects are reconstructed, all represented current roles retain
their constructor value. Neither `set_controlled_unit_004c0890` nor a sender
request is treated as delivery.

## Coverage and verification

| Surface | Coverage |
| --- | --- |
| `GameUnitSlot` current roles | Complete process storage for the nine constructor-produced values |
| `unit_current_role_slot` | Complete process getter and unavailable-output contract |
| `00928630` | Partial: only `00928713..0092874D` is projected here; all surrounding constructor behavior remains outside this packet |
| `009281C0`, `0077F480`, kind `4Bh` receiver | No new writer implementation; read-only evidence explains the pending boundary |
| Cruise and participant bindings | Integrator-owned follow-up, not implemented here |

Win32 Release and the two existing CTests are the build checks. The one ignored
`local/unit_roles_probe.cpp` executes the original 59-byte store fragment on
explicit scratch storage, then constructs the **actual** process VFS, Lua and
Units host objects and calls `create_units` with two supplied creation records
and one skipped record. No VFS manager, Lua state, native class constructor,
game startup, participant pool or mission is fabricated or launched. The
fixture compares all nine public reads on each created owner with the original
fragment, verifies the unchanged output on unavailable reads, and checks that
selecting one unit does not assign any role. Different Party inputs remain
irrelevant to the constructor state.

The report records the completed results, exact source/object/executable
hashes and native-call verification. Reproduction files and build logs are
indexed by `local/unit_roles_artifact_manifest.json`. This is constructor
fragment and actual-host fixture evidence; it is not original full-constructor,
assignment-delivery, mission-runtime or network-session validation. Ghidra was
read-only through the BSP wrappers against `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; there are no new native function definitions,
names, ledger records or tracked tests.
