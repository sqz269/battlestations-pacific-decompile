# Binding the recon sensor pass

Addresses: `008073C0` (BSP_Recon_RebuildSlotLists, the driver), `008048A0` (the per-observer
sensor evaluation), `009623A9` (the `ReconModifier` field read inside `00960230`
BSP_VehicleClass_ReadLuaFields), `008079B0` (BSP_Recon_ServicePeriodicRefresh, the cadence),
`00805BE0` (BSP_Recon_ResetDetection, the pre-pass zeroing), `008053C0` (the side's published
contact list), `009DFBE0` (the union-list membership read), `008082A0` (the ReconClass record
loader), `00922DC0` (the surface-target thunk), `00852B90` (the submarine category getter).

Packet `cc8_recon_sensor_pass_binding`, on `3512280b0`. The rules were reconstructed by
`cc7_gunnery_recon_detection` and the driver by `cc8_recon_sensor_pass_rule_c`; this packet
is the host wiring that gives them a caller, and the measurement that shows what they do.

## What each contract point became

| point | in code |
| --- | --- |
| 1 | `GameMissionLuaHost::read_vehicle_class_number(int, const char*, float)` in `include/bsp/game_hosts_lua.hpp` and `src/game_hosts_lua.cpp`, the float twin of `read_vehicle_class_integer`. The gunnery host reads `ReconModifier` with it and squares the result, because `009623CF` FMUL ST0,ST0 squares before `009623D9` stores at class+B8h |
| 2 | `GameGunneryHost::Impl::ReconClassRecord`, one per distinct `ReconClass` id, built once with `build_sensor_class_table_008082a0` and converted field for field to the 56 `GunneryReconSensorRow` entries. Held by `unique_ptr` so adding a class never moves a live entry array |
| 3 | `Impl::step_recon_sensor_pass_008073c0`, called from `GameGunneryHost::fixed_step` before the per-unit pass loop. Not per firing unit, and **not per frame**: see Corrections |
| 4 | `recon_contact_count_008053c0` drops a target whose published level is `none`, after the existing side, visibility, dead and kind rejections, counting `GameGunnerySummary::contact_reject_recon_level` |
| 5 | `recon_knows_target_009dfbe0` takes `GameGunneryHost::recon_sensor_pass_state().level(own_side, target)`. The `rule (c) absent` comment and the PARTIAL note above it are gone |
| 6 | Census rows `Recon::sensor_pass` at `0x008073c0` and `Recon::evaluate_sensors` at `0x008048a0`, plus `summary mission recon sensor_pass`, one `recon side` line per observing side and one `recon class` line per resolved id |

The ship AI reaches the state through `GameShipAiHost::bind_gunnery`, which
`GameGunneryHost::set_ship_ai` already calls for the firepower host's category gun lists. No
new binding path was added and `src/game_hosts.cpp` is untouched.

## Host methods

`GunneryReconSensorPassHost` in `src/game_hosts_gunnery.cpp` implements
`bsp::ReconSensorPassHost`, one method per native read.

| method | answered from |
| --- | --- |
| `unit_present` | `unit_alive_and_visible` and `unit_active` and not dead, the same rule (b) gate the contact sweep uses |
| `unit_side` | `unit_side_0054` |
| `unit_is_observer_class`, `unit_is_unit_base` | `unit_is_kind_of(kUnitKindQueryUnit)`, the `05h` of `008068C2` and `008048D1` |
| `unit_is_submarine` | `unit_is_kind_of(kUnitKindQuerySubmarine)`, the `08h` of `0080497F` |
| `unit_is_surface_target` | `bsp::entity_is_surface_target_00922c80` on the same facts `GameShipAiHost::target_is_surface_00922dc0` fills; the `008DDF90` set branch is not built, so its tail is taken, as there |
| `unit_sensor_category` | the installed getters are constants, so this is the unit's kind: `0074E190` air for the plane base, `006DFD20` surface for the ship base, the airfield `45h` and the land fort `1Bh`, `004F1740` unclassified otherwise. A submarine takes `submarine_periscope_sensor_state(false)` and records `00852B90` as unimplemented |
| `unit_world_xz`, `unit_heading` | `unit_position_00fc` and `unit_heading_radians` |
| `unit_recon_modifier_sq` | the squared `ReconModifier`, resolved once per unit |
| `unit_sensor_rows` | the cached `ReconClassRecord`'s 56 rows, or null, which is `008048C1`'s early false |
| `unit_environment_factor` | 1.0f. This process builds no gameplay-modifier list, the same empty-list answer the hit path already takes at `008E6430` |
| `unit_detection_forced`, `unit_forced_level` | false and `none`. Nothing in this process sets det+10h |
| `simplified_recon_multiplier`, `simplified_sonar_multiplier` | 1.0f. Neither mission script calls `008B24A0` or `008B2770` |
| `network_role` | 0, the single originating session |

## Corrections

Appended, not rewritten. Nothing in another packet's document is edited in place.

### docs/RECON_SENSOR_PASS.md, "State of the packet"

The contract is landed. Two of its statements needed correcting before it would run.

**The cadence.** The contract said "step it once per tick". `008079B0`
BSP_Recon_ServicePeriodicRefresh is the only periodic caller of `008073C0` (a callsite census
gives five callers; the other four are `004C9E82` scene apply, `004E059B` mission load, and
`006EDB99` and `007C631C` plane placement). It holds a countdown at `[00F874B8]`: `008079B6`
subtracts the frame delta, `008079C8` returns while the result is still positive, `008079CE`
reloads it by adding the double at `00D7A2B0`, which is 3.0, clamped at zero by the `008079EA`
arm, and only then does `00807A00`..`00807A08` walk the slot list at `[00F874BC]` and call the
pass on each.

The pass's dt is the measured gap, not the frame delta: `008073C1` loads the clock at
`[00F876A4]`, `008073CC` subtracts the slot's last-pass stamp at slot+2Ch and `008073D6`
stores the difference into slot+30h.

This is decisive because the detection value does not carry across passes. `00807480`..
`00807490` calls `00805BE0` BSP_Recon_ResetDetection on every record first, and `00805BF9`
zeroes the value at record+0Ch while `00805BFE` clears the level at +4h. One pass has to reach
a threshold on its own: the shipped `Gain` of 1.0 per second is halved to 0.5 by `00808626`,
and 0.5 * 3.0 = 1.5 clears the 0.25 blip threshold and saturates the 1.0 cap, while 0.5 * 0.05
is 0.025 and can never clear anything.

Measured, not argued. The per-frame attempt reported `blip=0 identified=0 none=94880` on
USN02, and recon-sourced gun assignments fell from 879 to 0: every side blind to every target
for the whole mission. The host now reproduces the countdown, and the same run reports
`blip=19 identified=1532 none=32`.

The 3-second period was **already on record**: the ledger's `008079B0` evidence from packet
`cc_unit_lists` states it, and `00805BE0`'s evidence from `cc2_recon_slot_lists` states the
zeroing. The driver packet did not carry either into its contract. `kReconSensorPassRefreshPeriod`
is now a constant in `include/bsp/recon_sensor_pass.hpp` with the listing behind it, so the next
caller cannot make the same choice.

**The class index.** The contract's `ReconClass` is "a `VehicleClass` integer field, reachable
through `GameMissionLuaHost::read_vehicle_class_integer`". True, but the index is
`GameUnitRow::type_id` and **not** `GameUnitsHost::unit_class_id`: the latter is the entity
class id the IsKindOf chain walks (`06h` ship base, `0Fh` plane base), so indexing the authored
table with it resolves every unit to one wrong row. `build_guns` already takes `type_id`. Found
at run time: the first corrected-cadence-less run reported `classes=1` with `class_missing=0`
across 32 units.

### docs/SHIP_AI_GOAL_VECTOR_VISIBILITY.md, the partial label

`recon_knows_target_009dfbe0`'s "rule (c) absent" is no longer accurate. The sensor pass runs
in the gunnery host's tick and the ship AI takes its published level. The permissive fallback
is unchanged in spirit: a side the pass never covered keeps `kReconDetectionUnknownLevel`,
which is `identified`, so binding the pass can only ever make a target less visible where a
sensor judged it.

Measured on USN02: each of the twelve ship rows moved from `recon=75 surface=0` to
`recon=74 surface=1`, one of the 75 union-list reads per ship. `flag_true` is unchanged at 600.

### include/bsp/game_hosts_gunnery.hpp, the file header

"Rule (c), the detection value, is not applied: no sensor pass runs in this process, so every
enemy in class scope is a contact" is now false and is corrected in place, because it is this
packet's own file and the sentence describes what the file does.

## Installed data

* **`ReconModifier`**: 14 of the 637 `VehicleClass` rows that carry `ReconClass` also carry
  `ReconModifier`, and every one of the 14 is exactly 1. Every unit in both missions therefore
  falls back to `kGunneryReconModifierSqDefault` = 1.0f, which is the same number `009623AE`
  FLD1 supplies natively. Both runs report `modifier_absent` equal to the whole unit count.
  The reader is correct and the contract needs it; it changes no value in this installation.
* **`GameMode`**: `scripts/datatables/autoload/reconclasses.lua` takes `RealisticTable` only
  when the global `GameMode` is 1, and this installation carries no `gamemode.lua` for that
  autoload's `DoFile` to define it from, so the arcade table is the one this process can reach.
  The host uses `ReconTableVariant::arcade`.
* **Coverage**: `class_missing` and `no_sensor_table` are both 0 in both runs. Every id
  resolved is inside 1..12 and has authored rows, so no unit fell through `008048C1`.
* This installation is modded; `vehicleclasses.lua` carries a local modification date while
  `reconclasses.lua` does not. The `ReconModifier` census above is this installation's.

## no_ghidra_function

none. `008073C0`, `008048A0`, `008079B0`, `00805BE0`, `00960230`, `008082A0`, `00922DC0` and
`00852B90` all have Ghidra functions.

## Validation

`./scripts/build.ps1` Win32 `/W4 /WX`: clean. `ctest --test-dir build/win32 -C Release`: 2/2
pass. No test added: the behaviour this packet changes is only observable against the installed
data, and both missions measure it.

Both missions were run on this worktree, before and after, through `tools/run_game.ps1`. The
before runs are from a binary built with the packet's changes stashed.

### USN02

| | before | after |
| --- | --- | --- |
| shots / hull / deaths / damage | 872 / 125 / 3 / 13673.7 | 872 / 125 / 3 / 13673.7 |
| first shot / first hit | 1.40 s / 41.85 s | 1.40 s / 41.85 s |
| gun assigns, arm / recon | 5188, 4309 / 879 | 5188, 4309 / 879 |
| contacts admitted, ship / plane | 108498 / 0 | 105490 / 0 |
| rule (c) rejections | n/a | 3008 |
| ship AI visible, per ship | recon=75 surface=0 | recon=74 surface=1 |
| standoff choices / curve refreshes | 8400 / 2450 | 8400 / 2450 |
| ring scans / bearings | 8400 / 8400 | 8400 / 8400 |

Sensor pass: `passes=100 observers=1583 targets=1583 blip=19 identified=1532 none=32
classes=2 no_table=0 class_missing=0`. Side 0: `none=18 blip=19 identified=856`. Side 1:
`none=14 blip=0 identified=676`. Classes 2 (3 lists, 5 entries) and 3 (5 lists, 8 entries).

Attribution: rule (c) rejected 3008 contact considerations, which is exactly the admit_ship
fall of 3008. It moved one of each ship's 75 union-list reads out of the recon arm. It changed
no gun assignment, no shot, no hit and no death. At 3-second granularity the USN02 sides are
inside each other's authored sensor ranges for all but the first pass, which is what
`identified=1532` against `none=32` says.

### USN01

Aircraft against ships, where the sensor categories differ.

| | before | after |
| --- | --- | --- |
| shots / hull / deaths / damage | 8992 / 23 / 1 / 220.0 | 8711 / 23 / 1 / 220.0 |
| first shot / first hit | 0.25 s / 0.60 s | 4.35 s / 4.70 s |
| gun assigns, arm / recon | 1191, 0 / 1191 | 1067, 0 / 1067 |
| candidates / rejected | 1048 / 43985 | 928 / 28159 |
| contacts admitted, ship / plane | 43533 / 59569 | 15789 / 46801 |
| rule (c) rejections | n/a | 40700 |
| ship AI visible rows | none emitted | none emitted |
| standoff choices / ring scans | 0 / 0 | 0 / 0 |

Sensor pass: `passes=150 observers=3802 targets=7604 blip=60 identified=5398 none=2146
classes=8 no_table=0 class_missing=0`. Side 0: `none=284 blip=60 identified=2306`. Side 1:
`none=435 blip=0 identified=2017`. Side 2: `none=1427 blip=0 identified=1075`. Classes 1, 2,
3, 5, 6, 8, 9 and 12.

Attribution: rule (c) rejected 40700 contact considerations, `admit_ship` falling 27744 and
`admit_plane` 12768. Gun assignments fell 124 and shots 281, all of them recon-sourced because
USN01's arm source is zero. The first shot moved from 0.25 s to 4.35 s: nothing is detected
until the first 3-second refresh publishes a level, so the guns no longer open fire on a
contact no sensor has judged. Damage, hits and deaths are unchanged. Side 2 is the side rule
(c) blinds most, 1427 `none` against 1075 `identified`, and it is the side carrying aircraft.

The ship AI's goal-vector path does not run on USN01 (`ring_scans=0`, `firepower=0`, no
`visible` rows), so contract point 5 is measured on USN02 only.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `recon_sensor_category_producer` | `00852B90`, `0085EA40` | The three submarine category states, which depend on periscope and depth. `src/submarine_model.cpp` has the periscope half; the depth thresholds are unread, and the host answers periscope-in unconditionally. Neither mission has a submarine, so the arm is unmeasured |
| `recon_slot_object` | `008073C0`, `00806B10` | This driver walks sides because the process builds no recon slot object, and it folds the drain per target. A slot object would let the drain run once at `00807634` as `00807615` flattens, and would carry slot+2Ch per slot instead of one host-wide stamp |
| `recon_forced_detection` | `00806883` | det+10h, the forced arm the pass skips the sensor test for. No producer in this process sets it, so `targets_skipped_forced` is 0 in both runs |
| `vehicle_class_number_reader` | `009623A9` | Landed for `ReconModifier`. `src/vehicle_class_fields.cpp` lists other float fields at sibling offsets that `GameVehicleClassRow` does not carry; the reader now exists for them |
