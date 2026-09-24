# The unit instance's presentation sub-updates, run in the host

Packet `cc9_unit_instance_step11` (worker cc9-platform, 2026-09-23).
Addresses: 00815AA0 0081C050 008227E0 008252C0 00956600 00834E90 00834820 00834CC0 00834A70
0083B5E0 008405D5 00822CBE 0077F030 00779C60 00865FE0 0081B010

These are the six routines `008255B0` (`update_unit_instance_008255b0`) calls that were the
largest unowned family in `docs/UNIMPLEMENTED_AUDIT.md`: 94,500 calls each in USN04 4500, one per
unit per frame. Three step-11 routines and 00815AA0 already had reconstructions
(`docs/UNIT_TIMED_SUBUPDATES.md`, `docs/UNIT_TIMERS.md`); none was wired.

## 1. What the host's units carry

The host creates its units without the image's instance construction: 006FE590
(`SceneUnit::create_instance_from_descriptor`) and 00928860 (`SceneUnit::place_instance`) are
records. So a host unit has:
- **no scene model:** no scene node at +4A4h, no steering nodes at +107Ch, no propeller nodes at +106Ch;
- **no effect handles:** nothing at +9E8h, +9ECh (bow and stern wave), +A00h..+A0Ch (spray),
  +B44h.. (cavitation), no effect array at +FFCh, no parts at +A14h;
- **no sound emitters** at +BB4h, +BB8h, +BBCh and no object at +BC0h;
- **no descriptor** at +354h, so no damage table.

Every routine below tests each of these for null before use. With the handles null, the image
takes its own skip, so running a routine against the host state is the law, not a stub.

## 2. The six routines

| routine | what the image does | host state it needs | exists today | this packet |
| --- | --- | --- | --- | --- |
| 00815AA0 publish effect intensity (step 4) | 006FF270's intensity times the gate, pushed into every effect group; +9D0h latches the last value | effect groups at +9F0h/+9F4h, +9E8h.., parts, attachments | none; +2F0h/+2F4h are on `UnitInstanceState` | **bound**; only the latch moves |
| 0081C050 prune finished effects (step 6) | walks the pointer array at +FFCh (count +1000h), erases each element 00865FE0 calls finished through 0081B010 | the +FFCh array | empty | **bound**; new reconstruction `unit_prune_finished_effects_0081c050` |
| 008227E0 damage smoke (step 7) | a sub-object (its +28h is the unit) counts down effect slots at +0Ch and respawns expired ones at random points inside the hull extents while its own clock +30h is below [00CE7630] and the unit is above [00CFBC84], drawing `BSP_Random_UniformFloatRange` and `BSP_RandomThreads_NextU32` | the smoke controller object (`this+28h` is the unit), its slot vector and effect definitions at +1Ch | no | **record**: not reconstructed, and binding it would draw from the random streams, and 00BD2F10 is one process-wide generator, so a new draw would shift every later draw in the run |
| 008252C0 engine audio (step 11) | smooths `|throttle|` (+980h) into +BC4h at the class record's EngineSoundSmoothRate, then pushes it to up to three emitters | settings+5BCh/+5E0h/+604h records, the emitters | the records are now read from Lua; no emitters | **bound**; the smoother runs, the pushes are skipped by the null tests |
| 00956600 timers (step 11) | age +524h, countdown +728h, the 0.2 s damage scan (+6D8h) over the descriptor table, the animation chain, the +2F4h fade toward +2F8h and the node visibility | the descriptor table, the animation owner, +2F8h, the scene node | no table, no owner, no node | **bound**, with the +2F8h substitution below |
| 00834E90 propellers (step 11) and its tails 00834820, 00834CC0, 00834A70 | samples 0092D730, chases the steering angle +1088h, spins propeller nodes and drives cavitation; the tails move the bow wave, stern wave and spray effects | nodes, handles, class +69Ch/+6A0h/+6A4h, load +1030h, spray points class+650h | none | **bound**; the steering angle and the timers run, the node and handle work is skipped |

### The engine-sound records

0083B5E0 reads `ShipGlobals["Sounds"]` (key 00D0A640) and, through the jump table at 00842954,
one record table per index: 0 `Ship` (00CEB79C), 1 `TBoat` (00D0A780), 2 `Submarine` (00CEB7B0),
3 `Plane` (00D0A638). Each record is 24h bytes at settings+5BCh + i*24h. `EngineSoundSmoothRate`
(00D0A5F8) is read through 00B66330 with the default 0.2f (00CE54A0) and stored at 008405D5 into
record+8h, which is what 008252C0 reads. This installation's
`scripts\datatables\shipglobals.lua` (dated 2024-07-13, untouched bulk) authors 0.5 for all four.
008252C0 picks TBoat for IsKindOf(0Eh), Submarine for IsKindOf(8), and Ship otherwise, planes
included. `GameMissionLuaHost::read_engine_sound_smooth_rates_0083b5e0` reads them once in
`load_gameplay_settings_0083b5e0`.

### The fade target: a labelled substitution

+2F4h is the same cell as `intensity_scale`, and 00956600 chases it toward +2F8h. The writers of
+2F8h found by scanning `.text` are:
- 0077F030 in `BSP_UnitOwnerEntity_Construct`, which stores 0.0 into both +2F8h and +2F4h;
- the setter 00779C60, called from 006D3200 and 007BC550;
- 00953B55 in `BSP_Unit_BindDummyObjectId`.

None is reconstructed, and the host's `intensity_scale` starts at 1.0. **The host holds the target
equal to the current scale**, so 009569F8 takes its "equal, untouched" arm. The fade does not move
in this packet.

### Other substitutions

- **+61h.** Held clear; no writer exists (`GameUnitsHost::unit_flag_0061`).
- **+1030h and the class fields +69Ch/+6A0h/+6A4h.** Left zero. They reach only the
  per-propeller loop, which no node enters.
- **The spray point count, class+654h.** Left zero. With every slot null the walk only skips.
- **+364h, the damage-scan mark.** Left zero. It is read only behind a non-empty table.

## 3. Switch

`kUnitInstanceStep11Bound` in `src/game_hosts_units.cpp`. The runners are in
`src/unit_instance_step11.cpp`, registered in `cmake/startup.cmake`. OFF keeps the six records.

## 4. Predictions, written before the pair

The pair is one tree on main `ed1b14b5d`, built twice differing only in the switch, with
`BSP_GUNNERY_RNG_STREAMS=1`.

* **OFF total** equals the last ON total, 2,793,139, unless main moved a row since.
* **ON total** is OFF minus 472,500: five UNIMPLEMENTED rows of 94,500 leave the table. They are
  00815AA0, 0081C050, 008252C0, 00956600 and 00834E90.
* **Rows that change:**
  * `UnitInstance::smooth_intensity` (008227E0) stays at 94,500.
  * New done rows at 94,500 each: `prune_finished_effects`, `update_bow_wave`,
    `update_stern_wave` and `update_propeller_spray`.
  * `UnitInstance::controller_body_axis_speed` (0092D730) doubles to 189,000, because 00834EB7
    samples it.
  * A single `GameSettings::load_engine_sound_records` done row appears, and the log gains one
    line with the four rates, 0.5 each.
* **Rows expected flat:** every other table row and all summary lines, including damage, deaths,
  queued hits, releases and mission end. The cells these routines write have no reader in the
  host: +524h, +6D8h, +728h, +BC4h, +1088h, +9D0h and the wave and spray timers. +2F4h does not
  move. None of the routines feeds damage or physics.

## 5. The pair

Logs `local\s11_off_usn04.log` (OFF) and `local\s11_on_usn04.log` (ON, run from a copy of the ON
build under `build\on`). Both use `--frames 4700 --press-start-frame 30 --menu-select USN04
--mission-frames 4500 --mission-frame-seconds 0.05` with `BSP_GUNNERY_RNG_STREAMS=1`.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,797,750 | **2,325,250** (-472,500) |
| 00815AA0, 0081C050, 008252C0, 00956600, 00834E90 | 94,500 UNIMPLEMENTED each | 94,500 done each |
| 00834820, 00834CC0, 00834A70 | absent | 94,500 done each |
| 0092D730 controller_body_axis_speed | 94,500 | 189,000 |
| 008227E0 smooth_intensity | 94,500 UNIMPLEMENTED | 94,500 UNIMPLEMENTED |
| every other table row | | identical |
| all 150 `summary` lines | | identical |

**Against the predictions:**
- **ON minus OFF.** Every row prediction holds.
- **The OFF total** is 4,611 above the last pair's ON, 2,793,139. The rows that moved are plane-AI
  and ship-AI rows: `AvoidZoneLayer::sample_0041bc20` and the `Bot*` states. Main merged
  cc9-dogfight-engaged's `cc9_yorktown_order_split` and cc9-aa-targeting's `cc9_gun_barrel_count`
  between the two runs, which accounts for them. My prediction said "unless main moved a row", and
  it did.
- **Wording error in the prediction.** The engine-sound load is not behind the switch, so its
  `GameSettings::load_engine_sound_records` done row and its log line appear on both sides. The
  line shows 0.5 for all four records, as predicted.
- **The startup callback count** `PlatformLoopCallbacks::pretranslate` differs, 14 against 19. It
  differed between identical runs in the last pair too.

**Result.** The pair is explained by the reads. **The switch lands ON.** The rows left from the
family are 008227E0 at 94,500, a record for the reason in section 2.
