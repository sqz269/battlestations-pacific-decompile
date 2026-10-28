# CC13 mode3steer: the blk+3E0h cruise-distance seed

This is packet cc13-mode3steer. The defect is rev4's, read from the bytes, and the mechanism is
wire1's correction. The switch is `kShipAiCruiseDistanceSeedBound` in
`include/bsp/ship_ai_seg67_switches_cc13.hpp`. It is committed OFF and is not game-validated.

## 1. The defect (rev4)

- **The image seeds the cruise distance.**
  - The nav-block constructor `009E4330` calls `009DFCB0` at `009E43CC`, with `ECX = blk+1C4h`
    (`009E43A4 LEA ECX,[ESI+1C4h]`).
  - `009DFD49 MOVSS XMM2,[00CE3804]` loads 1000.0f.
  - `009DFEA8 MOVSS [ESI+21Ch],XMM2` stores it at blk+3E0h. XMM2 is not written in between.
- **The constructor keeps it.** Neither the constructor (to its `RET 4` at `009E46C0`) nor
  `009E0270` touches blk+3E0h.
- **Its only reader is the prologue.** `009ED918 MOVSS XMM0,[ESI+3E0h]` copies it to 32Ch for any
  non-Stopped direction, and 330h = 32Ch + 2000 (`009ED92A FADD [00CF0DD8]`). There is no store to
  `+3E0h` in `009ED6B0..009EE760`.
- **The host never writes it.**
  - `ShipAiControlBlock::cruise_distance_3e0` (ship_ai_states.hpp) starts at 0.
  - `ship_ai_nav_block_steering_defaults_009dfcb0()` computes the 1000, but the nav-block build
    copied only 3C8h, 340h and 3D0h into the controller.
  - Hence d32c = 0 / d330 = 2000 on every mode-3 row.
- **This is what cc9-lua70 saw behind the interim OFF of `kShipAiNavigateAsternSkipBound`:**
  every attack-move unit hunting in mode 3, d32c 0.00, the throttle swinging 0.5 / -0.625.

## 2. What 32Ch = 0 does (wire1, from bytes)

**No arrival effect.** Nothing on the mode-3 path reads 32Ch for throttle or arrival.

**What the -0.625 is.** It is the drive's 364h ahead/astern flip in the 009F3F80 sector escape:
- `009EC9DB` negates the ceiling, and `009EC9E5` compares it with the double -0.625 at
  `[00D21A80]` and floors it at the float -0.625 at `[00D21A78]`. I rechecked both constants from
  the bytes.
- The flip has a 1 s dwell on 368h (`009F4AA0..009F4B95`).

**32Ch = 0 keeps the flip going through four readers.** These feed the escape its
blocked/escape inputs:
- **`009F0100`'s neighbour clear.** At `009F06D4..009F06EC` it compares a neighbour's distance
  with own 32Ch plus `[EDI+48h]` (`009F06D4 FLD [ESI+32Ch]`, `009F06DA FADD [EDI+48h]`), and
  clears the neighbours beyond that.
  - EDI there is the settings record: `009F02BF CALL 00424C40` (GameSettings), `009F02CF MOV EDI,EAX`,
    `009F02E6 ADD EDI,180h`, saved at `009F02FD` and restored at `009F09DC` before the back edge.
    Only the behind arm repoints EDI at the node (`009F07C6`). So `[EDI+48h]` is settings+1C8h,
    ShipAvoidance.NearbyShip_NextCornerReachDistAddOn (default 100), and the threshold is
    own 32Ch + 100 (wire1, docs/CC13_DRIVE32C.md §3).
  - Withdrawn: an earlier revision inferred `[EDI+48h]` was the neighbour's published 330h, from
    the 009F3F80 publish (`009F4D5B` / `009F4D62`). That publish uses a different EDI (the traffic
    slot); the matching offset is a coincidence (180h + 48h = 1C8h).
  - With own 32Ch = 0, every neighbour that reaches this comparison and is more than 100 ahead
    is cleared; the seed widens that window to 1100. Own-group nodes for a group leader never
    reach this comparison: the preceding `009D8010` gate rejects them and holds node+88h at 0.
    For Ashigara's escorts, that gate explains `pass_posts=0`; the distance window applies to
    other ships. See `CC13_DRIVE32C.md` section 3 and the correction in `7c26febef`.
- **The arm-final query range.** It starts at `009DE61D` (`FLD [ESI+32Ch]`, rechecked); wire1
  traces it on to `009DE633` and the `009DC313` exit.
- **The leader corridor cap** at `009DEEBF` (`FADD [ESI+32Ch]`, rechecked).
- **The slot+40h crossing negotiation** `009D8CE0`.

rev4 also lists 32Ch/330h readers in the arm final (`009DE77A`), the tail (`009F0022`) and the
publish (`009F4D4D`, `009F4D5B`).

## 3. The binding

- **ON:** at the nav-block build in `register_ship_ai_unit` (game_hosts_ship_ai.cpp, right after
  the 3C8h/340h/3D0h copies), it sets
  `ctl.blk.cruise_distance_3e0 = ship_ai_nav_block_steering_defaults_009dfcb0().cruise_distance_3e0`
  (1000.0f).
- **OFF:** no change, because the block is behind `if constexpr`.
- **Scope: 3E0h only.** The other 009DFCB0 mismatches (section 5) go behind goaway's separate
  switch, so the mode-3 pair stays clean.

**It is not neutral with the skip OFF.** Mode 0 and mode 1 ships with a non-Stopped direction
also take `009ED918`, so their 32Ch/330h move whatever `kShipAiNavigateAsternSkipBound` says.

**Expected ON signature and pass criteria:**
1. Mode-3 rows show d32c=1000.00 / d330=3000.00.
2. The `l70_astern.py` d32c0 count falls to 0. Today it is LOMP10 Ashigara 3113/3113; IJN18 Gear3
   406/406, Oma4 401/401, Gear1 366/366.
3. pass_posts and traffic_writes are > 0.
4. The free bearing is > 263.
5. There are fewer -0.625 steps.
6. LOMP10 completes near 1072 s. This is not guaranteed.

**Pairing plan:**
1. This switch ON and `kShipAiNavigateAsternSkipBound` ON, against both OFF, on LOMP10 and IJN18.
2. This switch ON against OFF, with the skip OFF.

Re-flip the skip only after (1) passes. cc9-lua52's `local\l70_astern.py` does the d32c census.

## 4. ON self-check: the data lifetime

- **Every controller is seeded, including ones created after load.** The seed is written once
  per controller, in `register_ship_ai_unit`, where `nav_block_built` is set.
  - `register_units` calls it for every unit at load.
  - `register_generated_units` calls the same function for each controller created after load.
- **The prologue cannot run on an unseeded block.** `009ED6B0`'s model
  (`ship_ai_direct_control_arm_009ed6b0`) runs only through `ship_ai_controller_step_009f50e0`,
  and the step loop gates that on `ctl.nav_block_built`.
- **Nothing resets the block afterwards.**
  - `register_units` clears and resizes `controllers` before any build.
  - `register_generated_units` only appends (`resize` moves the existing controllers).
  - There is no `ctl.blk =` assignment in game_hosts_ship_ai.cpp.
- **Nothing else writes the field.** The host has no other writer of `cruise_distance_3e0`; the
  prologue only reads it.
- **No new container is looked up.** `ship_motion_probe.cpp` builds its own block and is
  untouched.

## 5. Audit of the other 009DFCB0 stores (reported only)

Source: the whole-body reconstruction `cc13_seg67d_nav_block_seed_defaults_009dfcb0`
(src/cc13_seg67d.cpp). The stores cited by address were rechecked from disk bytes. blk offset =
1C4h + the store's displacement. Non-zero stores:

| blk+ | image store | host field and start | readers found |
| --- | --- | --- | --- |
| 1E4h | byte 1 (`+20h`) | `goal.final_leg_1e4` = false | `009EE6DE`. `009DE050` stores it unconditionally (`009DE14C`), and `009F3240` calls `009DE050` before it writes mode 3 |
| 214h | byte 1 (`+50h`) | none found on blk | none in `009ED6B0..009EE760` or `009DE5B0` |
| 224h.. | the plan pair `009DBDF0` (`+60h`) | `plan_a` / `plan_b` member defaults | not compared |
| 340h | 1000.0 (`+17Ch`) | at the build, `nav.look_ahead_340` = the constructor's `009E45BB` value | `009ED769` re-seeds it from 3C8h every tick |
| 344h | 1.0 (`+180h`) | `obstacle.throttle_limit_344` = 1.0 | `009ED6F9` rewrites it |
| 348h | 1.0 (`+184h`) | `obstacle.rudder_limit_348` = 1.0 | |
| 354h | -1.0 (`+190h`, `009DFD71`) | `blk.clamp_354`, `throttle_profile.hold_354`, `clearance.hold_354`, all 0 | `009DE8FF` (0 > 354h) in `009DE5B0`; the raises |
| 364h | byte 1 (`+1A0h`, `009DFDB7`) | `blk.yaw_rate_subtracts_364`, `clearance.committed_ahead_364`, both false | `009ED9D1` (mode 0); `009F40CA` drive-order ring (from `009F4DA0`, every mode); `009EFFDC` / `009EFFEC` |
| 380h | -1.0 (`+1BCh`) | `obstacle.backoff_timer_380` = -1.0 | |
| 384h | -1.0 (`+1C0h`, `009DFD79`) | `blk.direction_counter_384` (int) = 0, `obstacle.stall_time_384` = 0.0 | the stall threshold 10.0 (`009F489E`) via `009F3F80` |
| 3A4h | byte 1 (`+1E0h`) | `follow_land making_way` = true | `009EDB28`, `009EDF93` (station arm) |
| 3C0h | byte 1 (`+1FCh`) | none found | none in the scanned bodies |
| 3C4h-3D8h, 3E4h | the window | overwritten by `009E4330` / `009E0270`; the host copies the results | |
| 3DCh | 400.0 (`+218h`, `009DFEA0`) | no copy | none in the scanned bodies |
| **3E0h** | **1000.0 (`+21Ch`, `009DFEA8`)** | `blk.cruise_distance_3e0` = 0 | **`009ED918`: this switch** |

**Not checked.** The zero stores were not compared field by field, and neither were the plan
pair's defaults.

**Against goaway's independent audit:**
- **goaway found these, and I did not.** All are zero stores or plan-pair fields, which I did not
  compare:
  - the station enables 3A5h/3C1h (image 0, host `station_request.enable` true);
  - 300h = -1.0 (the plan pair's `+DCh`), with no host storage;
  - 37Ch = 0 against the host sentinel 9999.
- **I found this, and goaway did not:** 3C0h, byte 1, with no host field found.
- **We disagree on 340h.** goaway has image 1000 against host 0. I found the build copies the
  constructor's own 340h (`009E45BB`), which overwrites the 1000, into `nav.look_ahead_340`, and
  `009ED769` rewrites it each tick. goaway should confirm which host copy it read.
- **goaway pairs 1E4h with 214h as final_leg.** I have 214h as unmapped.
- **The rest agree:** 364h, 384h, 3E0h, 354h and 3DCh.

## 6. Stale comments fixed (no code change)

- **game_hosts_ship_ai.cpp, `wrap_brain_heading_00605070`.**
  - brain+1E0h is blk+1D8h, and it has a reader: `009ED947` copies it to 324h in modes 1 and 3.
  - The `00605070` wrap is not applied, only recorded. The `cc9_heading_wrap_census` summary in
    JM08 `p20e_off_jm08.log` shows `out_of_range=0` of 57068 stores.
  - Applying the wrap would change behaviour where that count is non-zero, so it needs its own
    switch. It is listed for later and not added here.
- **ship_ai_states.cpp, the `009ED7D8` latch.** It has been modelled since 996696b4d.
  `[ESP+4Fh]` is written at `009ED7F3`, and its only access in `009ED6B0..009EF220` is `009EE756`.
  The host's skip reads `blk.mode` at that point.
- **`NavigateAstern` is not renamed** in this patch.

## Uncertainty

- **The four readers in section 2 are wire1's.** I rechecked only the loads at `009F06D4`,
  `009DE61D`, `009DEEBF` and `009EC9DB`..`009EC9F1`.
- **The pass criteria come from the cc9/rev4 logs.** No pair has been run here.
- **Unknown readers.** The readers of 3DCh, 3C0h and 214h were not found in the scanned bodies,
  and no program-wide displacement scan was done.
- **Constant widths.** No header constant was added; the 1000.0f comes from the existing
  `ShipAiNavBlockSteeringDefaults` (`MOVSS` from `00CE3804`).
- **Builds.** Built OFF and ON (ctest 2/2 each), committed OFF.

## 7. The mode-3 name (packet cc13-astern3a6)

The host calls blk+1C4h = 3 `ShipAiSteeringMode::NavigateAstern`, and the switch is
`kShipAiNavigateAsternSkipBound`. That name is a misnomer. The provisional hypothesis is
**NavigateCallerSteered**: the caller publishes a path goal and then steers the ship itself.

### Writers

A disp32 scan of 009D0000..00A00000 for stores to +1C4h finds two stores of 3:
- `009DE2D2 MOV [ESI+1C4h],3` in 009DE280. 009DE280's only caller is `009E7FB8` in 009E7FA0, and
  009E7FA0 has no rel32 or absolute reference, so this writer is not live.
- `009F3375 MOV [ESI+1C4h],EBP` in 009F3240. Here `009F3330 MOV EBP,3`, and ESI = blk from
  `009F32F7 LEA ESI,[ECX+8]`. 009F3240 is reached from vtable `00D219A0` and is the attackmove
  approach sub-state step. This is the live writer.

The other stores to +1C4h write 0, 1, 2 or a register:
- 009DE050 / 009DE1B0 / 009DFF40 / 009DFFB0 / 009E0040;
- the restore 009DA0D0;
- the land and kamikaze steps 009E1950 / 009E2020.

Their register sources were not traced.

### What both writers do

Both writers run the same sequence:
1. Call 009DE050(goal, keep = 1, final = 0).
2. Zero 368h/360h unless the mode is already 3.
3. Set 1D8h = heading, wrapped by 00605070.
4. Set 1CCh = 0 (direction Stopped).
5. Set 1C4h = 3.
6. Set 250h/2B8h = throttle.

009ED6B0 then latches mode 3 at 009ED7E6 and skips its own output block and tail at 009EE756
(section 1 and the switch comment).

So in mode 3, the heading and throttle are the caller's. The ahead/astern flip that gave this mode
its name is the 364h escape outcome (section 2), not a property of the mode.

### Applied

Names are unchanged in Ghidra and in the ledger:
- `BSP_ShipAi_SetNavigationGoalWithPenalty` 009DE280;
- `BSP_ShipAi_AttackMoveApproachSubStateStep` 009F3240.

Both records gained appended evidence (`ledger add-name --append-evidence`). `ghidra_annotate.py`
re-applied their plate comments, saving the prior plates to `local/`.

Interior labels were added under the write lock with `tools/ghidra_label_interior.py`
(`reports/cc13_astern3a6_labels.json`):
- `BSP_ShipAi_EnterNavigateCallerSteered_009de2d2`;
- `BSP_ShipAi_EnterNavigateCallerSteered_009f3375`.

Neither function had any labels before. The project is saved.

The C++ enum value and the switch keep their names, because cc9 flips the switch by name. Both now
carry a comment that names the hypothesis. The log string `navigate_astern` is unchanged, so logs
stay comparable.
