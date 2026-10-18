# CC13 fix17: torpedo and submarine Lua natives

Packet cc13-fix17, 2026-10-17. Four Lua natives in cc9's mission hosts had no answer. Two are now
bound, each behind its own switch that is OFF; two are skipped. There is also one doc erratum. With
the switches OFF the build behaves exactly as before, and each row still records UNIMPLEMENTED.

| Native | Address | Result | Switch |
| --- | --- | --- | --- |
| SetSubmarineAirSupply | 00893A60 | bound | `kLuaSetSubmarineAirSupplyBound` (include/bsp/game_hosts_units.hpp), OFF |
| ForceFireDepthCharge | 0089B9D0 | bound | `kLuaForceFireDepthChargeBound` (include/bsp/game_hosts_gunnery.hpp), OFF |
| GetShipTorpedoes | 008BED80 | skipped | none |
| GetSquadTorpedoes | 008BEF80 | skipped | none |

## 0. Erratum: CC13_VERIFY16 R6

R6 said AAEnable's 0071E050 sends sub-kind 2. It sends sub-kind 4: `0071E09A MOV dword ptr [ESP+24h],4`.
ArtilleryEnable's 0071DFD0 sends 3 (0071E01A). The sub-kind arms of 0071C1E0 are:

| Sub-kind | Effect |
| --- | --- |
| 0 | vtable[64h] |
| 1 | vtable[68h] |
| 2 | +1CCh |
| 3 | +220h |
| 4 | +221h |
| 5 | +222h |
| 6 | +223h |

The decompile shows the +221h, +222h and +223h stores. A dated note now sits under R6.

## 1. SetSubmarineAirSupply 00893A60

**Image (bytes and decompile).** The function takes no class test and no null test:

- Argument 0 goes through 00888AA0.
- Argument 1 goes through 00B66270 (GetNumber, which reads nil as 0). It is stored with
  `FSTP dword [esp+10h]` (00893B95), then `FLD` and `FSTP dword [esi+127Ch]` (00893BA1).
- So the value is rounded once to float. The rounding mode does not matter here because
  there is no arithmetic.
- There is no clamp and no RNG draw, and the native returns 00B66400 (no results).

A separate setter, 00888510 (`movss [ecx+127Ch]`, ret 4), also writes the field. Nothing
references it by absolute address. This tool cannot do a rel32 caller census, so treat that as
unchecked.

**Sites of +127Ch.** A disp32 scan for `7C 12 00 00` found these readers and writers:

| Address | Role |
| --- | --- |
| 00855250 | Air model, 11 sites |
| 0064DD30, 00650450, 00683020 | HUD |
| 00852D30 | Describe state |
| 00852F10 | Constructor |
| 00853E10 | Load state |
| 008938E0 | GetSubmarineAirSupply |

The ship-AI hits (009E7FC0, 009F1BC0) are a different object (nested+127Ch). 007E2A20 is the
plane-globals block.

**Host.** The script orders host routes the row to
`GameUnitsHost::set_submarine_air_00893a60` (src/game_hosts_units.cpp). That function writes
`slot.sub_air.air`, which the host's air model `submarine_step_air_00855250` reads on every force
step under `kSubmarineAirBound` (ON).

- No new field was added. The setter mirrors the existing `set_unlimited_air_00893c00`.
- LABELLED: a unit without seeded submarine air state stores nothing. The image writes +127Ch
  of any entity.
- The HUD oxygen value (`game_hosts_hud.cpp` `submarine_127c`) stays the existing substitution,
  and GetSubmarineAirSupply 008938E0 stays unanswered. Neither is in this packet.

**Missions and predicted ON effect.** These are from this installation's Lua.

| Script | Call | Predicted ON effect |
| --- | --- | --- |
| LOMP `08_lightning_strike.lua:392` | `SetSubmarineAirSupply(Mission.SS, 10.0)` at load | 00855250 clamps 10.0 to 1.0 on the next force step (0085537A), and a fresh boat already starts at 1.0. Expected effect: none. |
| `jm12.lua:1783` | `luaJM12AddAirSupply` refills to 1 | The boat's air resets to full when that function runs. |
| `jm10.lua:1862` | checkpoint restore with `Mission.Checkpoint.Air` | That value comes from GetSubmarineAirSupply (`:1764`), which is unanswered. GetNumber reads the nil as 0, so ON would set the air to 0, and the boat would then start low or drown. **Divergence to watch:** the image reads a real value there. Leave this OFF until GetSubmarineAirSupply is answered, or check that jm10's checkpoint path is not reached in a run. |

COTP-IJN has copies of jm10 and jm12.

## 2. ForceFireDepthCharge 0089B9D0

**Image (bytes 0089BAC0..0089BB2B).**

- `unit = 00888AA0(arg 0)`, with no null test.
- `child = [unit+48h]`. While `child != 0`:
  - if `child->vtable[5Ch](24h)` and `[[child+3F4h]+80h] == 8`, call `child->vtable[1F0h]()`.
  - Then `child = [child+44h]`, read after the call.
- The native returns 00B66400 (no results).

The class test is IsKindOf 24h, MSTGun, which covers MDepthChargeLauncher 27h. Weapon type 8 is
"DEPTHCHARGE" (`bsp::GunneryCategory::kDepthCharge`). Type 9, DEPTHCHARGELAUNCHER (the Hedgehog),
does not match.

**The slot.** MDepthChargeLauncher's vtable is 00CFC190, so slot +1F0h is the dword at 00CFC380,
which is 006FDC50. 006FDC50 makes the indirect call `vtable[1E8h](1)` (`CALL EDX` at 006FDC5A);
that slot, the dword at 00CFC378, holds 006FDCD0. 006FDCD0 does
two things:

1. Calls 0072D2C0(1), the fire request.
2. Sets `+4D4h = 0` when `+454h` is 0.

All seven DEPTHCHARGE devices in this installation's realistic `deviceclasses.lua` are
`Single_Turning_Gun` (MSTGun). Their slot +1F0h is also 006FDC50 (docs/GUNNERY_OPEN_ITEMS.md
row 7).

**What happens to the request after that:**

- MSTGun's tick 006FE0D0 drops the request after the last barrel fires. This is the salvo test,
  modelled as `mst_salvo_test`.
- DepthChargeBot 008FC080 rewrites the byte only on a think that has a fire target. Without a
  target it returns at 008FC13C, before 008FC39A.
- So one call gives one salvo, unless the bot has a target.

**RNG.** The native itself draws nothing. 0072D2C0 draws 00BD2F10 for the fire stagger on the
request's rising edge (gun_platform_arc.cpp `random_stagger_00bd2f10`). Each round fired also draws
in the depth-charge activate 006FD9B0: the drag scale U(0.9, 1.1) and the depth U(DiveMinDepth,
DiveMaxDepth). **ON therefore moves the shared RNG stream** in any mission that calls the native.

**Host.**

- The script orders host queues the call through `queue_force_fire_depth_charge_0089b9d0`.
- At the next fixed step, `GameGunneryHost::fixed_step` raises `depth_charge_trigger_by_gun[g]`
  on every category-8 gun of the unit. That is the existing trigger the gun pass sends through
  vtable[1E8h].
- In `run_gun_aim_and_fire` the depth-charge block normally drops the trigger when the bot has no
  target. That is an existing labelled substitution standing in for the retarget release.
- Under the switch, a forced trigger is kept while the bot has no target. It ends in one of two ways:
  - The gun has sent the request once and `gun.fire.fire_requested` has gone false. That means
    the salvo test dropped it, or 0072D2C0 refused it for a suppressed gun.
  - The bot has a target again, so its think owns the byte.
- When the switch is OFF, the queue is cleared and the original line runs unchanged.

LABELLED substitutions:

1. Category 8 stands in for the IsKindOf(24h) test (see the device list above).
2. The raise lands at the next fixed step, not in the script phase.
3. A gun the player's seat holds keeps its seat trigger, because the depth-charge block is skipped
   for seat guns.
4. A null entity is dropped. The image would fault on it.

cc9-gunnery51's blocks are untouched.

**Missions.** The only live call is `usn_03_battle_of_santa_cruz.lua:593`:
`if Mission.Puffin.Dead == false then ForceFireDepthCharge(Mission.Puffin)` in the mission think.
Puffin is `FindEntity("Puffin")` on a ping-pong path (`:104-105`). The PRCPUS copy has the same
line. The calls in `usn_06_guadalcanal_2nd_battle.lua:1839/1843` (and the PRCPUS
`prcp_06_2ndbattle.lua` copy) are commented out: one with `--`, the other inside a `--[[` block.

**Predicted ON effect.** Puffin's DC racks fire one salvo per mission think, at the rate reload
allows, and the depth charges sink and detonate at their drawn depths. That is decorative ASW fire
along its path. RNG-coupled deltas in usn_03 are expected, so judge them through pair_diff with the
depth-charge counters.

## 3. GetShipTorpedoes 008BED80: skipped

**Image.**

- `unit = 00888AA0(arg 0)`, then NewTable.
- It walks `[game+19CCh]`, count `+21Ch` and node `+224h`, with element `[node+8]`.
- An element is added (`00927B40` self object, at keys 1..n) when `byte [e+5Eh] == 0` and
  `[e+3BCh] == unit`. That is every live torpedo the unit launched.
- There is no RNG draw.

An existing reconstruction is in `src/unit_weapons.cpp` (`lua_get_ship_torpedoes_008bed80`).

**Why skipped.**

- The host's torpedoes are gunnery projectile rows (`GameProjectileRow`), not mission entities.
  They have no Lua self object to put in the table, and Kill and the other natives cannot address
  them.
- Pushing an empty table would be a wrong answer whenever a torpedo is in the water.
- The only caller, `jm10.lua:1086` (`torp = GetShipTorpedoes(Mission.PlayerUnit)`), never reads
  `torp`. So the unanswered row costs nothing in this installation.

## 4. GetSquadTorpedoes 008BEF80: skipped (consequence for cc9 noted)

**Image.**

- `squadron = 00888AA0(arg 0)`. The flag is `GetBoolean(arg 1)` only when the argument count is 2.
- It walks the same list.
- An element qualifies when two conditions hold:
  1. `[e+5Eh] == 0`.
  2. `(e+310h)->vtable[38h]()` is true, or the flag is set. The vcall is made first.
- It is then added when one of the squadron's plane slots `[sq+3D0h+4i]`, for `i < [sq+3CCh]`
  with i >= 5 reading 0, equals `[e+3BCh]`. The slot scan then stops.
- There is no RNG draw. The existing reconstruction is in `src/cc13_left55_seg56c.cpp`.

**Why skipped.** With the flag set, the table includes torpedoes still on the planes' racks: the
vcall is false for those, and the flag admits them. The host models a plane's payload as an
ordnance set (`ordnance_has_torpedo_2bh`), not as entities, so it has nothing to return.

**Torpedo AI finding (for cc9).** `usn_11_protecting_iowa.lua:1296` and `:1533` call this right
after `GenerateObject` of the Hammerhead and Monteray squadrons. The PRCPUS copies are
`usn_11_protecting_iowa.lua` and `prcp_11_patrol.lua:1302/1539`. The calls look like this:

```lua
local torptable = GetSquadTorpedoes(Mission.Hammerhead, true)
for idx, unit in pairs(torptable) do Kill(unit, true) end
```

- **In the image**, this kills the torpedoes on the racks, so those squadrons fly in disarmed.
- **In the host**, the unanswered row returns nothing. `pairs(nil)` then raises a Lua error, so
  the rest of each function never runs. The skipped calls are:
  - `SetDeviceReloadEnabled(false)`
  - `SquadronSetTravelAlt`
  - `PilotMoveTo` or `SetShipSpeed`
  - `SetRoleAvailable` and `SetForcedReconLevel`
  - the `S1Phase4Movie` / `S2Phase4Movie` delay
- On top of that, the squadrons keep their torpedoes.
- A faithful answer needs payload torpedoes as entities with Lua objects, plus a Kill route that
  removes them from the plane's ordnance. That is outside this packet.
