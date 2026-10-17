# Rule (c), the sensor pass driver (packet `cc8_recon_sensor_pass_rule_c`)

Addresses: 00806840, 008048A0, 008073C0, 008082A0; read as evidence 0074E190, 006DFD20,
004F1740, 00852B90, 0085EA40, 00805AF0, 00805BE0, 009623A9.

Worker `agent/cc8-ship-rudder-hop`, 2026-09-18. Ghidra read-only: no renames, comments,
prototypes or saves.

## The headline: the rules were already reconstructed and had no caller

`008048A0` and `00806840` were both projected before this packet, twice:
`recon_evaluate_sensor_row_008048a0` in `src/recon_slot_lists.cpp` (packet
`cc2_recon_slot_lists`) and `gunnery_recon_range_scale_008048a0` /
`gunnery_recon_observe_008048a0` / `gunnery_recon_detect_00806840` in
`src/gunnery_recon_detection.cpp` (packet `cc7_gunnery_recon_detection`). The second set is
thorough: `docs/GUNNERY_RECON_DETECTION.md` has the range rule, the row index, the bearing
arc, the accumulate and reset calls, the publish at `0080695C` and the drain.

**Nothing in the tree called any of it.** A search for either entry point across `src` and
`tests` finds only the file that defines them. That is why
`src/game_hosts_ship_ai.cpp`'s `recon_knows_target_009dfbe0` carries

```cpp
facts.level = bsp::ReconDetectionLevel::identified;  // rule (c) absent
```

and why `docs/RECON_SLOT_LISTS.md` rule (c) is labelled absent. The packet's premise is
therefore right about the effect and wrong about the cause: rule (c) is not unanalysed, it
is undriven.

So this packet did not re-read `008048A0`. It wrote the driver those rules were missing and
traced every input to a producer this process can actually supply.

## The driver

`include/bsp/recon_sensor_pass.hpp` and `src/recon_sensor_pass.cpp`:

* `ReconSensorPassHost`, one method per native read, nothing defaulted.
* `ReconSensorPassState`, the answer indexed by (side, target), with a census.
* `recon_sensor_pass_step_008073c0(state, dt, host)`, `008073C0`'s loop: for each side with
  a present observer-class unit, build that side's observer array and run every present
  unit of a different side through `gunnery_recon_detect_00806840`.

`kReconDetectionUnknownLevel` is `identified`, deliberately. A side the pass never covered
keeps the permissive answer the tree used before, so wiring the driver can only ever make a
target less visible where a sensor actually judged it, never blind a side by omission.

Two composition notes, both differences from the native control flow and neither a rule
change:

* Natively the drain is not per target. `008073C0` runs all 97 class buckets through
  `00806840` first (`00807556` enemy, `00807574` neutral), flattens them at `00807615` and
  drains the list once at `00807634`. `gunnery_recon_detect_00806840` already folds the
  drain in per target, and this driver keeps that.
* The native pass is per recon slot and walks the slot's own triple at `+DD8h`. This driver
  walks sides instead, because this process builds no recon slot object. `env.slot_index`
  is set to the observing side, which is what `00804B2F` indexes the target's detection
  record with.

## Every input, traced to a producer

| input | native read | producer in this process |
| --- | --- | --- |
| observer admission | `008068C2` `IsKindOf(05h)` | `GameUnitsHost::unit_is_kind_of` |
| sensor category | `unit->[+1E4h]->vtable[1]()` | constants, verified: `0074E190 XOR EAX,EAX` air, `006DFD20 MOV EAX,1` surface, `004F1740 MOV EAX,6` unclassified, `00852B90` the three submarine states, `0085EA40` underwater. Derivable from the unit's kind plus `submarine_periscope_sensor_state` |
| world x/z | `unit+FCh`, `unit+104h` | `GameUnitsHost::unit_position_00fc` |
| heading | `vtable[50h]()` | `GameUnitsHost::unit_heading_radians`, concrete `006DFD60` (`docs/SHIP_AI_RUDDER_HOP.md`) |
| sensor rows | `[[unit+538h]+B4h]+8` | `build_sensor_class_table_008082a0(variant, reconClassId)` on the authored data in `src/sensor_table_data.cpp`; `ReconClass` is a `VehicleClass` integer field, reachable through `GameMissionLuaHost::read_vehicle_class_integer` |
| signature | `[[unit+538h]+B8h]`, the SQUARE of `ReconModifier` | **the one gap**: the field table in `src/vehicle_class_fields.cpp` has it at `+B8h` with evidence `009623A9`, but `GameVehicleClassRow` does not carry it and the Lua host has an integer reader only. `kGunneryReconModifierSqDefault` = 1.0f until a float reader exists |
| environment | `008E6430(0Ch, observer)` under two flags, else 1.0f | `GameplayModifiers`, already bound elsewhere in the gunnery host |
| multipliers | `[game+21C4h]+74h` / `+78h`, `SimplifiedReconMultiplier` / `SimplifiedSonarMultiplier` | 1.0f from `00444D20` unless a mission script's Lua setter (`008B24A0` / `008B2770`) changed it; USN02 does not |
| network role | `[00E188A8]+1FE4h` | the session role the gunnery host already reads |
| dt | `[slot+30h]` | the tick delta |

## State of the packet

**Landed**: the driver, built clean at Win32 `/W4 /WX`, with the existing tests passing.

**Not landed**: the host wiring, and this is a stopping decision rather than a blocker. The
contract below is complete and every input above is reachable; the packet stopped at a clean
commit because its context was long, which is the standing instruction. Nothing is
half-wired: `recon_sensor_pass_step_008073c0` has no caller yet, so no run-time path changed
and no census moved.

### The binding contract

1. `include/bsp/game_hosts_lua.hpp` and `src/game_hosts_lua.cpp`: add
   `float read_vehicle_class_number(int index, const char* key, float fallback)`, the twin
   of the existing `read_vehicle_class_integer`, and read `ReconModifier` with it. Square it
   before handing it to `unit_recon_modifier_sq`, because `008048E5` reads `+B8h` already
   squared.
2. `src/game_hosts_gunnery.cpp`: hold one `bsp::ReconSensorPassState` and one
   `bsp::SensorClassTable` per distinct `ReconClass` id seen, built once with
   `build_sensor_class_table_008082a0`. Convert each `SensorTableEntry` to a
   `ReconSensorEntry` field for field: `dist_sq` to `max_normalized_distance_sq`,
   `gain_per_second` across, `max_value` to `cap`, and the bearing field to
   `max_bearing_error`.
3. Step it once per tick, before the gunnery pass, not per firing unit: the pass is
   O(sides * observers * targets) and running it inside
   `recon_contact_count_008053c0` would repeat it per unit.
4. `recon_contact_count_008053c0`: after the existing rule (a) and rule (b) rejections, drop
   a target whose level is `none`, and count the rejection separately so the census
   attributes it.
5. `src/game_hosts_ship_ai.cpp`, `recon_knows_target_009dfbe0`: replace
   `facts.level = ReconDetectionLevel::identified` with the state's level for
   (own side, target), and delete the `// rule (c) absent` comment and the PARTIAL note
   above it.
6. Census rows: `Recon::sensor_pass` at `0x008073c0` and `Recon::evaluate_sensors` at
   `0x008048a0`, plus a summary line carrying `ReconSensorPassState`'s counters.

## Corrections

| was | is | evidence |
| --- | --- | --- |
| the packet brief and `docs/SENSOR_TABLE_DATA.md`'s follow-up: "drive rule (c) from this data", implying the rule still needs projecting | The rule was projected twice before this packet and is complete. What was missing is a caller | `src/gunnery_recon_detection.cpp`; no reference to either entry point anywhere else in `src` or `tests` |
| `src/game_hosts_ship_ai.cpp`'s PARTIAL note, "the sensor pass 00806840/008048A0 ... does not run in this process" | Accurate, and now explained: the reconstruction exists and is simply never called. The note stays until the contract lands | the same |

No text in another packet's document is rewritten. `docs/RECON_SLOT_LISTS.md` and
`docs/GUNNERY_RECON_DETECTION.md` are cited, not edited: nothing in them is wrong.

## Also in this commit

`driven=` is retired from the ship AI ring census line, as
`docs/SHIP_AI_HEADING_TO_RUDDER.md` recommended. `units_driven` was never incremented
anywhere in the tree, so `driven=0` read as a gate on a chain that in fact runs;
`live_pair_changes` is the field that measures it.

## no_ghidra_function

none. `00806840`, `008048A0`, `008073C0`, `008082A0`, `0074E190`, `006DFD20`, `004F1740`,
`00852B90` and `0085EA40` all have Ghidra functions.

## Validation

* `./scripts/build.ps1` Win32 `/W4 /WX`: clean.
* `ctest --test-dir build/win32 -C Release`: 2/2 pass. No test added: the driver has no
  caller, so there is no behavioural regression to guard, and the rules it composes already
  have coverage in `tests/math_tests.cpp`.
* No game run was taken and no census is attributed. `recon_sensor_pass_step_008073c0` is
  not called, so USN02 and USN01 are unchanged from this tree's earlier runs by
  construction. The measurement the packet asks for belongs with the contract above.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `recon_sensor_pass_binding` | `008073C0`, `008048A0` | Land the six-point contract and take the USN02 before/after the packet asks for: detected counts by level per side, the visibility flag true count per ship, the standoff and gunnery censuses, every change attributed to the detection rule |
| `vehicle_class_number_reader` | `009623A9` | The float twin of `read_vehicle_class_integer`. `ReconModifier` is the first caller; the field table in `src/vehicle_class_fields.cpp` lists others at float offsets that `GameVehicleClassRow` does not carry |
| `recon_sensor_category_producer` | `00852B90`, `0085EA40` | The three submarine category states, which depend on periscope and depth. `src/submarine_model.cpp` has the periscope half; the depth thresholds are unread |

## Correction appended by cc8_recon_sensor_pass_binding

The binding contract above is landed on the same branch. Two of its statements needed
correcting before the pass would publish anything, and both are in
`docs/RECON_SENSOR_PASS_BINDING.md` with the listing behind them. The text above is left as it
was written.

**"State of the packet" is superseded.** The host wiring is no longer "not landed": points 1
to 6 are in `src/game_hosts_lua.cpp`, `src/game_hosts_gunnery.cpp` and
`src/game_hosts_ship_ai.cpp`, and both missions were run before and after.

**Contract point 3, "step it once per tick", is wrong.** `008079B0`
BSP_Recon_ServicePeriodicRefresh is `008073C0`'s only periodic caller and runs it every 3.0
seconds (the double at `00D7A2B0`, against the countdown at `[00F874B8]`). The pass computes
its own dt as `[00F876A4]` minus slot+2Ch, the measured gap. This matters because `00805BE0`
zeroes each record's value at +0Ch before the pass, so one pass must reach a threshold
unaided: the shipped `Gain` of 1.0/s halved by `00808626` gives 0.5 * 3.0 = 1.5, which clears
the 0.25 blip threshold, while 0.5 * 0.05 gives 0.025, which never does. A per-frame step was
measured on USN02 and published `none` for all 94880 target evaluations.

Both facts were already in the ledger, under `008079B0` from packet `cc_unit_lists` and under
`00805BE0` from `cc2_recon_slot_lists`. They are now also
`bsp::kReconSensorPassRefreshPeriod` in `include/bsp/recon_sensor_pass.hpp`, with the listing
in the comment above it, so the next caller cannot step the pass per frame by accident.

**The input table's `ReconClass` row needs one word.** It is reachable through
`GameMissionLuaHost::read_vehicle_class_integer`, but the index is `GameUnitRow::type_id`, not
`GameUnitsHost::unit_class_id`, which is the entity class id the IsKindOf chain walks.

**The one gap is closed.** `GameMissionLuaHost::read_vehicle_class_number` exists and reads
`ReconModifier`. In this installation only 14 of the 637 `ReconClass`-carrying rows have the
key and all 14 are 1, so every unit still takes `kGunneryReconModifierSqDefault`.

## Records (packet cc9_g51_recon_records, cc9-gunnery51, 2026-10-17)

`kReconDetectionRecordsBound` (`include/bsp/recon_sensor_pass.hpp`), committed OFF. It replaces the per-target
model with the whole of `00806840` over persistent records (`recon_run_sensor_pass_00806840`,
`recon_record_accumulate_00805af0`, `recon_record_reset_00805be0` in `src/gunnery_recon_detection.cpp`) and a driver
`recon_sensor_pass_rebuild_008073c0` that runs slots 0, 1 and 2.

**What the image does that the OFF path does not.**
1. **The value carries across passes.** The reset loop `00807462..0080749A` walks `[[[00E188A8]+19CCh]+13Ch]`. The
   scan at `008074CA` indexes the per-class heads as `registry + id*0Ch + 1Ch`, so `+13Ch` is class `18h`, and only the
   plane squadron's world-list virtual `007F10B0` joins it (`ADD ECX,138h` at `007F10C8`; the unit's `006FE620` joins
   ids 2, 4, 5, 6, 7). Unit records are never zeroed before the pass, which corrects this doc's own claim and the
   `prior_value` comment in `gunnery_recon_detection.hpp` (docs/GAMEPLAY_LOOSE_ENDS_1.md section 2 had it right).
   Consequence: `008048A0` clamps each entry's gain against `cap - current` (`00804B45..00804B79`), which is negative
   once the record is above that entry's cap, and `00806840` keeps only a strictly larger gain than 0.0f. So a target
   identified once (value 1.0) STAYS identified while any entry of any observer still applies, even a blip-capped
   (0.25) one; it drops only when no entry applies at all (`00806957`, 00805BE0). OFF recomputes from zero and drops
   it to the applying entry's cap.
2. **Every slot is covered.** `004E0537..004E0544` builds slots 0, 1 and 2 at mission load and `008079B0` rebuilds all
   three. A party with no sensor-bearing observer publishes `none` for every scanned target. OFF leaves such a side
   uncovered, and `level()` answers `identified` (`kReconDetectionUnknownLevel`).
3. **Observers and targets come from the scan.** Own list = `slot+DD8h` (`00804E10` at `0080753A`), targets = the enemy
   buckets (`00807556`) then the neutral buckets (`00807574`); the 22 ids, four gate bytes and IsKindOf(2). An
   IsKindOf(5) own entry without a sensor table stays in the loop: `008048A0` returns false at `008048C1` without
   writing `*outGain`, and `008068EC` still compares the stale stack slot (labelled substitution: 0.0f at the start of
   a rebuild).
4. **The own arm** (`008065B0`, `00805AF0(1.0f, 1, null)` on each own record) and **the delegate**
   (`[+2Ch]->vtable[0](+30h, old, new)`, only when the effective level moved). A unit record therefore notifies on net
   changes only; `kReconListenerResetCycleBound`'s every-pass `old -> 0 -> new` cycle applies to the class-18h squadron
   records alone. ON, `dispatch_recon_listeners_00980e50` fires the records' own transitions instead (`game_hosts_lua.cpp`,
   loaned for one commit).
5. **det+28h** is kept as the winning observer's unit index; its only image reader is the HUD warning `009741E0`.
   The skip path (`00806871` / `00806883`) leaves the record as it is and publishes `+8h` / `+4h`.

**Not modelled (labelled):** the squadron records the `00807490` loop resets (the host folds squadrons from member
levels when it publishes); one dt for all three slots.

**Predictions (ON vs OFF, same tree):** not gameplay-identical. (a) `summary mission recon sensor_pass` identified
rises and blip falls, from the hysteresis; (b) a side with no sensor-bearing observer moves from uncovered to covered;
(c) `listener_recon_changes` falls sharply (no per-pass cycle on unit records), so a Lua `recon` listener waiting for
`0 -> 2` fires only on a real first identification: LOMP06's `luaSeaplaneSpotted` may go silent or fire later; (d)
gunnery contact sweeps keep identified targets longer, so fire-target picks and AutoTarget can move from the first
pass a target falls back under a blip-capped sensor. Pairs: USNOS (`usnos_s45_os_k`), ESMP08 (`g38_ord_e8`), USN04
(idle), LOMP06 (`lomp06_l50_l6_q6`).

**Pairs** (`g51_r0` / `g51_r1` at `cc0c2017d`, reference env; logs `local\g51_r{0,1}_{u4,l6,e8,os}.log` in
cc9-gunnery36):

| mission | verdict | what moved |
| --- | --- | --- |
| USN04 idle | gameplay identical | side 0 levels identical (none 401, blip 95, identified 969); side 1 (no observers) now covered: identified 3422 -> 1022, its final enemy triple 34 -> 0 |
| LOMP06 `l50_l6_q6` | moved | blip 203827 -> 654, identified 83913 -> 373044; Narwhal sunk 743.29 -> 638.32 s; **fails in both** (744.09 s OFF, 639.12 s ON) |
| ESMP08 `g38_ord_e8` | moved, deaths identical (959) | enemy_identified 31220 -> 31354, unknown 1012 -> 878; only candidate counts and plane ranges move; no end either side |
| USNOS `s45_os_k` (3000 s cap) | moved | fails at 1425.27 s both; 8 Okinawa houses no longer die to ToSpawnZao's stray blast at 1389 s; late plane deaths re-timed |

**Mechanism.** The host rebuilds about once per second, so OFF (value from zero each pass) gives 0.5/s * ~1 s, just under
the 0.5 identified threshold, and most contacts sit at blip. ON integrates the value as the image does: a target in an
applying entry's range for about two seconds is identified, and stays identified until no entry applies. That is
prediction (a); (b) shows on USN04's side 1; (c) shows on LOMP06 (`IsListenerActive` 53400 -> 3 follows the earlier
failure). `stale_gain_reads` is 0 in all four runs. **Flipped ON** by mechanism. LOMP06's keeper win is already lost
on the OFF tree (reported to the lead).
