# CC13 mode3steer: the 009DFCB0 seeds the host never made

This is packet cc13-mode3steer. The defect is rev4's, read from the bytes. The switch is
`kShipAiCruiseDistanceSeedBound` in `include/bsp/ship_ai_seg67_switches_cc13.hpp`. It is
committed OFF and is not game-validated.

## 1. The defect (rev4)

- **The image seeds the cruise distance.**
  - The nav-block constructor `009E4330` calls `009DFCB0` at `009E43CC`, with `ECX = blk+1C4h`
    (`009E43A4 LEA ECX,[ESI+1C4h]`).
  - `009DFD49 MOVSS XMM2,[00CE3804]` loads 1000.0f.
  - `009DFEA8 MOVSS [ESI+21Ch],XMM2` stores it at blk+3E0h. XMM2 is not written in between.
- **The constructor keeps it.** Neither the constructor (to its `RET 4` at `009E46C0`) nor
  `009E0270` touches blk+3E0h.
- **Its only reader is the prologue.** `009ED918 MOVSS XMM0,[ESI+3E0h]` copies it to 32Ch for any
  non-Stopped direction, and 330h = 32Ch + 2000 (`009ED92A FADD [00CF0DD8]`).
- **The host never writes it.**
  - `ShipAiControlBlock::cruise_distance_3e0` (ship_ai_states.hpp) starts at 0.
  - `ship_ai_nav_block_steering_defaults_009dfcb0()` computes the 1000.
  - But the nav-block build copied only 3C8h, 340h and 3D0h into the controller.
- **Result: d32c = 0 / d330 = 2000 on every mode-3 row.** cc9-lua70 saw exactly this behind the
  interim OFF of `kShipAiNavigateAsternSkipBound`: every attack-move unit stalled in mode 3, with
  d32c 0.00 and the throttle swinging.

## 2. The binding

ON, at the nav-block build in `register_ship_ai_unit` (game_hosts_ship_ai.cpp, right after the
3C8h/340h/3D0h copies):
- `ctl.blk.cruise_distance_3e0 = ship_ai_nav_block_steering_defaults_009dfcb0().cruise_distance_3e0`
  (1000.0f);
- `ctl.blk.clamp_354 = ctl.throttle_profile.hold_354 = -1.0f`. This is the one other mode-3-path
  seed found by the audit (section 4).

OFF: no change, because the block is behind `if constexpr`.

**It is not neutral with the skip OFF.**
- Mode 0 and mode 1 ships with a non-Stopped direction also take `009ED918`. So the switch moves
  their 32Ch/330h whatever `kShipAiNavigateAsternSkipBound` says.
- Every ship reaches `009DE5B0`, so every ship's 354h test starts at the image's value.

**Pairing plan** (also in the header):
1. This switch ON and `kShipAiNavigateAsternSkipBound` ON, against both OFF, on LOMP10 and IJN18.
2. This switch ON against OFF, with the skip OFF.

Re-flip the skip only after (1) passes. cc9-lua52's `local\l70_astern.py` does the d32c census.

## 3. ON self-check: the data lifetime

- **Written once.** The seed is written once per controller, in `register_ship_ai_unit`, at
  `nav_block_built = true`.
- **The prologue cannot run on an unseeded block.** `009ED6B0`'s model
  (`ship_ai_direct_control_arm_009ed6b0`) runs only through `ship_ai_controller_step_009f50e0`.
  The step loop gates that on `ctl.nav_block_built`, so no prologue runs before the seed.
- **Nothing resets the block afterwards.**
  - `register_units` clears and resizes `controllers` before any build.
  - `register_generated_units` only appends (`resize` moves the existing controllers).
  - No `ctl.blk =` or `throttle_profile =` assignment exists in game_hosts_ship_ai.cpp.
- **No host writer of 3E0h exists besides the seed.** The prologue only reads it, which matches
  the image (no store to `+3E0h` in `009ED6B0..009EE760`).
- **354h after the seed:** the host's writers are the raises (`009DE763`, `009ED97E`,
  `009EDA11`, `009F3FAD`, `009F45DE`, `009EF8F4`) and the countdown `009E0591`, which counts only
  while >= 0. The image starts at -1.0f, so the countdown does not run until something raises it.
  With the seed, the host starts there too.
- **The third copy is unchanged.** `ctl.clearance.hold_354` is only raised by `009EF910`'s model,
  and a raise to the hold gives the same result from -1.0f as from 0.
- **No new container is looked up.** `ship_motion_probe.cpp` builds its own block and is
  untouched.

## 4. Audit: every 009DFCB0 store against the host

Source: the whole-body reconstruction `cc13_seg67d_nav_block_seed_defaults_009dfcb0`
(src/cc13_seg67d.cpp), with the stores rechecked here from disk bytes. blk offset = 1C4h + the
store's displacement. The non-zero stores:

| blk+ | image store | host field and start | readers found | on the mode-3 path? |
| --- | --- | --- | --- | --- |
| 1E4h | byte 1 (`+20h`) | `goal.final_leg_1e4` = false | `009EE6DE` (navigation arm) | no gap: `009DE050` writes it on every goal set, and `009F3240` calls `009DE050` before it writes mode 3 |
| 214h | byte 1 (`+50h`) | none found on blk | none in `009ED6B0..009EE760`, `009DE5B0` | later packet |
| 224h.. | the plan pair `009DBDF0` (`+60h`) | `plan_a` / `plan_b` member defaults | the path pick | not compared here; later packet |
| 340h | 1000.0 (`+17Ch`) | copied from `nav_block.look_ahead_340` | `009ED769` re-seeds it from 3C8h every tick | covered |
| 344h | 1.0 (`+180h`) | `obstacle.throttle_limit_344` = 1.0 | `009ED6F9` rewrites it | matches |
| 348h | 1.0 (`+184h`) | `obstacle.rudder_limit_348` = 1.0 | | matches |
| **354h** | **-1.0 (`+190h`, `009DFD71`)** | `blk.clamp_354`, `throttle_profile.hold_354`, `clearance.hold_354`, all 0 | **`009DE8FF` (0 > 354h) in `009DE5B0`, reached at `009EF213`**; the raises | **yes: under this switch** |
| 364h | byte 1 (`+1A0h`, `009DFDB7`) | `blk.yaw_rate_subtracts_364`, `clearance.committed_ahead_364`, both false | `009ED9D1` (mode 0 only); `009F40CA` drive-order ring (from `009F4DA0`, every mode); `009EFFDC` / `009EFFEC` | not in the mode-3 arm; later packet, probably the most consequential |
| 380h | -1.0 (`+1BCh`) | `obstacle.backoff_timer_380` = -1.0 | | matches |
| 384h | -1.0 (`+1C0h`, `009DFD79`) | `blk.direction_counter_384` (int) = 0, `obstacle.stall_time_384` = 0.0 | the stall threshold 10.0 (`009F489E`), via `009F3F80` from `009F4DA0` | not in the mode-3 arm; later packet |
| 3A4h | byte 1 (`+1E0h`) | `follow_land making_way` = true | `009EDB28`, `009EDF93` (station arm) | matches |
| 3C0h | byte 1 (`+1FCh`) | none found | none in the scanned bodies | later packet |
| 3C4h-3D8h, 3E4h | the window | overwritten by `009E4330` / `009E0270`; the host copies the results | | covered |
| 3DCh | 400.0 (`+218h`, `009DFEA0`) | no copy | none in the scanned bodies | later packet; readers unknown |
| **3E0h** | **1000.0 (`+21Ch`, `009DFEA8`)** | `blk.cruise_distance_3e0` = 0 | **`009ED918`** | **yes: this switch** |

**Scope of "the mode-3 path".** It is what the image runs in mode 3 under the `009EE756` skip:
- `009ED6B0..009EE756`;
- then `009EF206`, which is `009DE5B0`.

I scanned both bodies' disk bytes for every seeded displacement. In `009DE5B0` the only seeded
field it touches is 354h; it also reads 3C8h and 3D0h, which the host copies. Of the path pick's
callees, `009DA1D0`, `009ED3E0` and `009E3C00` read none of the seeded fields; their own
callees were not scanned.

**Zero stores.** The fields the host models start at zero, as the image's zero stores do. This
was not checked field by field for the unmodelled offsets.

## 5. Stale comments fixed (no code change)

- **game_hosts_ship_ai.cpp, `wrap_brain_heading_00605070`.** brain+1E0h is blk+1D8h, and it has a
  reader: `009ED947` copies it to 324h in modes 1 and 3. The wrap is still only recorded:
  `00605070` leaves (-pi, pi] alone, and the `cc9_heading_wrap_census` summary in JM08
  `p20e_off_jm08.log` shows `out_of_range=0` of 57068 stores.
- **ship_ai_states.cpp, the `009ED7D8` latch.** It is now modelled. `[ESP+4Fh]` is written at
  `009ED7F3`, and its only access in `009ED6B0..009EF220` is `009EE756`. The host's skip reads
  `blk.mode` at the same point.

## 6. Expected movers with the switch ON

- **Mode-3 units:**
  - d32c = 1000 and d330 = 3000 instead of 0 / 2000, when the latched direction is not Stopped.
  - Those are the 32Ch/330h the arm and `009F4DA0`'s ceiling read.
- **Mode 0/1 ships with a non-Stopped direction:** the same 32Ch/330h change, in every mission.
- **The avoidance override at `009DE8F1`** can fire from a ship's first step, instead of only after
  its 354h hold has been raised and counted below zero. Its other gates (`009DA1D0`, and a
  non-zero avoidance vector) are unchanged.
- No draw changes. No new log line.

## Uncertainty

- **Readers are unknown for some seeds.** The readers of 3DCh, 3C0h and 214h were not found in the
  scanned bodies; a program-wide displacement scan was not done.
- **The plan pair's member defaults** (`009DBDF0`) were not compared with the host's
  `ShipAiPathPlanBlock` defaults.
- **The 364h and 384h seeds** are listed for a later packet. They change every ship's obstacle
  logic, not only mode 3's.
- **The constant widths** were taken from the loads (`MOVSS` from `00CE3804` and `00D7A260`), so
  `const_width_sweep` was not needed: no header constant was added.
- Built OFF and ON (ctest 2/2 each). No pair has been run.
