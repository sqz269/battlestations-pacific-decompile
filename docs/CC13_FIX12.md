# CC13_FIX12: IsUnitSelectable and GetEntitySpeed answered in the mission Lua host

Packet cc13-fix12, 2026-10-17. Source: `docs/CC13_VERIFY16.md` R3 and R5. Both natives are
answered in `src/game_hosts_lua.cpp`, each under its own switch in `include/bsp/game_hosts_lua.hpp`.
Both switches are committed **OFF**. With them off the two rows still record UNIMPLEMENTED and push
nothing. The summary lines print only when a switch is on, so an OFF log is unchanged.

**Flip them together.** usn_11 and prcp_11 `CheckMontR1..R10` test `IsUnitSelectable(Monteray)` and
`GetEntitySpeed(Monteray) < 30` in the same `and` chain. With only `kLuaIsUnitSelectableBound` on, a
true answer reaches `nil < 30`, which raises a Lua error in place of today's silent short-circuit.

No image RNG draw is involved on either path.

| Switch | Native | Host entry |
|---|---|---|
| `kLuaIsUnitSelectableBound` | IsUnitSelectable 008AB580 | `GameMissionLuaHost::run_is_unit_selectable_008ab580` |
| `kLuaGetEntitySpeedBound` | GetEntitySpeed 008C0820 | `GameMissionLuaHost::run_get_entity_speed_008c0820` |

## IsUnitSelectable 008AB580

**Image.** The usual binding frame runs first. Then:
- `008AB67F` resolves argument 0 through 00888AA0 (`ESI` = the entity or null).
- `008AB696..008AB69A`: `MOV DL,1` / `MOV ECX,ESI` / `CALL 00645160`.
- 00645160 (`00645160..00645174`) is `EAX = EDX; EDX = [[00E188A8]+18ECh]; PUSH EAX; CALL 00645060`.
  So it calls `00645060(unit, team = game+18ECh, allowSpectate = 1)` and adds no logic of its own.
- `008AB6A4`: 00B66450 PushBoolean(AL). The function returns 1 (00B66400).

**Host.** The binding reads argument 0's `ID` with `objective_argument_unit`, the same resolution
SetSelectedUnit uses. It then calls `bsp::game::hud_unit_selectable_00645060`
(`src/game_hosts_hud.cpp:1999`), which routes to `GameHudHost::unit_selectable_00645060`. That
predicate's inputs (`selectable_inputs_00645060`, `:1497`) already assume team index 0 and
allowSpectate 1, which are exactly the values 00645160 supplies, so nothing is duplicated. An index
past the units table answers false, as 00645060 does at `00645082` for a null unit.

**Arms and labels.**
- If argument 0 is not an entity table with a positive `ID`, the image's 00888AA0 returns null and
  00645060 rejects it at `00645082`. The host pushes **false** and counts `unresolved`.
- If no HUD host is attached, the predicate has nothing to read. The host then pushes **nothing**
  (nil, today's behaviour) and counts `no_hud`. LABELLED: the image always answers. In a game run the
  HUD host is attached together with the world (`attach_world`, `:1440`).
- The predicate's own labelled substitutions (team = party 0, spectator door shut) carry over.
- 00645060 rejects plane-family units (`006450D7` vtable[5Ch](0Fh), then `JNZ` to false). jm12's
  `luaGetType(unit) == "plane" and ... IsUnitSelectable(unit)` therefore stays false in the image
  as well; this is not a host gap.

## GetEntitySpeed 008C0820

**Image.** The usual binding frame runs first. Then:
- `008C091F` resolves argument 0 through 00888AA0.
- `008C0936..008C093D`: `MOV EDX,[ESI]; MOV EAX,[EDX+38h]; MOV ECX,ESI; CALL EAX`. There is no null
  test.
- `008C093F FSTP float ptr [ESP+10h]`, then `FLD` / `FSTP float ptr [ESP]`, then 00B66480 (push
  number) at `008C094F`. The function returns 1 (00B66400).

**What vtable[38h] is.** Each vtable's +38h slot was read and matched to the class table in
`src/unit_kind_query.cpp`:

| Family | Slot 38h | Meaning |
|---|---|---|
| All nine ship-family vtables (00CF90B0, 00CFA778, 00CFB738, 00CFC3D0, 00CFFA30, 00D01630, 00D09678, 00D0BF80, 00D0C648; data refs to 0080E0F0) | 0080E0F0: `ECX = [unit+1018h]; JMP 0092D730` | The signed body-axis forward speed, in m/s: v . pose row 2. |
| All nine plane vtables (00D00070, 00D00308, 00D05F20, 00D06638, 00D06920, 00D0BA80, 00D19D28, 00D1A000, 00D1A2D8; data refs to 007B8E60) | 007B8E60: `FLD [ECX+0B1Ch]; RET` | The cached \|velocity\|, in m/s. |
| PlaneSquadronGen 00D087C0 (slot 00D087F8 = 007ED470) | 007ED470: `ECX = [ECX+3D0h]; JMP [[ECX]+38h]` | The flight leader's vtable[38h], so the leader plane's cached \|velocity\|. |

**Host.** The binding reads argument 0's `ID` with `air_ops_entity_id`, as the other entity
natives do, and takes the index as `ID - 1`. It then picks the first matching case:
1. **Squadron.** A plane-squadron registry record whose `squadron_unit` is the index: take that
   record's `flight_leader()` (+3D0h slot 0, "whatever the count", as 007EDA91 reads it), then the
   plane value below for that leader. usn_11's `Monteray` is this case: it is a squadron
   (`SquadronSetTravelAlt(Mission.Monteray, ...)`, usn_11 `:1531`).
2. **Ship family** (`unit_is_kind_of(index, kUnitKindQueryShip)`):
   `GameUnitsHost::unit_forward_speed_0092d730(index)`, the host's existing 0092D730 over the slot's
   linear velocity and pose row 2. Hammerhead (a submarine) and the PT boat in commandhelpers take
   this case.
3. **Plane family** (`kUnitKindQueryPlane`): `float(sqrt(vx^2 + vy^2 + vz^2))` over
   `GameUnitsHost::unit_linear_velocity(index)`. The free-flight step and the spawn seed mirror
   `plane_world_velocity` into that field (`game_hosts_units.cpp` 007F2B3C and the integration step).
   This is the same value as the units host's own `leader_live_speed_007b8e60`.

The pushed value is the float widened to a Lua number, as 00B66480 receives it.

**Arms and labels.**
- **Unresolved entity** (no units host, `ID <= 0`, or an index past the table): the image would call
  through a null vtable and fault. The host pushes **nothing** (nil, today's behaviour) and counts
  `unresolved`.
- **A squadron with no leader** (`flight_leader()` is no unit): the image reads a stale or null slot
  0. The host pushes nothing and counts it under `unresolved`.
- **Any other class** (land vehicles, structures, ordnance and so on): those vtables' slot 38h
  bodies were not read. The host pushes **nothing** and counts `unmodelled`. No shipped call site
  reaches this case; the subjects are planes, squadrons, submarines and ships.
- **LABELLED, plane value:** the image caches +B1Ch in the core step at 007D807C, whereas the host
  computes the magnitude when the native is called. The host's velocity field holds the value of the
  last step, so the two agree unless a script reads the speed between a velocity write and the next
  step.
- **Precision:** the result is a single float load or a float magnitude; no x87 chain is involved.

GetShipSpeed 00890EF0 makes the same vtable[38h] call (`00891006..0089100D`, `FSTP float`). It is
not answered here because no packet asked for it.

## Missions reached once both switches are on

- usn_11 and prcp_11 `CheckMontR1..R10` (`:2295-2415` / `:2304-2421`): the primary-4 rescue points
  clear when the Monteray squadron is within 100 of a point, selectable, below y 10 and slower than
  30. The squadron is selectable through the fused-slot liveness (`kFusedSquadronSlotSelectableBound`).
- usn_11 and prcp_11 Hammerhead (`:1363` / `:1369`).
- jm05 (`COTP-IJN :1236`, `PRCPIJN :1341`): the captured-PT switch.
- jm12 (`COTP-IJN :702`, `PRCPIJN jm12 / prcpjm12 :721`): stays false through IsUnitSelectable
  on a plane-family unit (see above). If `luaGetType` also answers "plane" for a squadron entity
  (not checked here), that case reaches GetEntitySpeed through the squadron arm.
- jm02 `:1789`, `:1810`, jm05 `:4267`, `:4302`, and chg_5_tulagi `:967-1243` use IsUnitSelectable only.

Every arm that pushes nothing also records the row UNIMPLEMENTED in the log, so a bound run
still shows where the image's answer was not given.

Nothing here was run. The switches are OFF, no game runs were made, and nothing is game-validated.
