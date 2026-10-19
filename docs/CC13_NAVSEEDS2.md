# CC13 navseeds2: the 009DFCB0 nav block seeds

Packet cc13-navseeds2. The packet adds one OFF switch, `kShipAiNavBlockCtorSeedsCc13Bound`, in
`include/bsp/ship_ai_seg67_switches_cc13.hpp`. When ON, it seeds three blk fields that 009DFCB0 sets
and the host starts at zero:
- 364h
- 384h
- 354h

The seeding is in `register_ship_ai_unit` in `src/game_hosts_ship_ai.cpp`, right after hitq's
`kShipAiCruiseDistanceSeedBound` (3E0h) block.

Status:
- Exported and reconstructed.
- Built OFF and ON.
- Not game-validated. No runs were made for this packet, and a pair is needed.

## 1. The seeds

009DFCB0 is `__fastcall` with ECX = blk+1C4h. Its one rel32 caller is 009E43CC, in the nav block
ctor 009E4330.
A store at [ECX+n] lands at blk+1C4h+n.

| blk | image store | value | host field (default) | switch |
|---|---|---|---|---|
| 364h | 009DFDB7 `MOV byte [ECX+1A0h],1` | 1 | `blk.yaw_rate_subtracts_364` (false) | yes |
| 384h | 009DFD79 | -1.0f | `obstacle.stall_time_384` (0) + `blk.direction_counter_384` (0) | yes |
| 354h | 009DFD71 | -1.0f | `blk.clamp_354` (0), `throttle_profile.hold_354` (0) | yes |
| 3E0h | 009DFEA8 | 1000.0f | `blk.cruise_distance_3e0` | hitq, `kShipAiCruiseDistanceSeedBound` |

The excluded seeds are in section 5.

### 364h: committed ahead

The image writes 364h in only three places:
- the seed;
- the escape manoeuvre, at 009F4B0F (0) and 009F4B74 (1).

The two setters that also write it, 009D51F0 and 009D5240, have no rel32 or absolute references, so
they are dead.

That claim was re-scanned for rev2's nit (b):
- The scan covers every section, looking for E8/E9 rel32 and absolute dwords. It found no hits for
  either address.
- As a positive control, the same scan finds 009EF910's caller 009F4D87 and 009DFCB0's caller
  009E43CC.

The readers are:
- 009ED9D1 in 009ED6B0. When 364h is set, heading_target_324 comes from 00438B10 (subtract);
  otherwise it comes from 00438AA0 (add).
- 009EC9CD, the throttle ceiling. With 364h clear and throttle_hold_1c8 != 0 (set by 009DE067),
  the ceiling is negated and floored at -0.625 ([00D21A78]).
- 009F40F8..009F415A, 009F4441, 009F44C3, 009F4933..009F4ABC and 009EFFDC/009EFFEC.

In the host, `yaw_rate_subtracts_364` starts false, and only the escape writes it:
`ship_ai_obstacle_tables.cpp:678` (false) and `:692` (true). So a ship that never runs the escape
keeps the astern ceiling for the whole mission.

The evidence is the "ship ai step" throttle in `p24_off_u21f`:
- The Iowa group (Iowa, DD_1/2/5/6/12/13, Massa, NewOrleans) spawns at step 5030. From step 5050
  its throttle is -0.5 or -0.625, and it is never positive after that (pos_after = 0).
- Kongo, Haruna, Nagato and Suzuya (movetopos, step 54030) start negative and turn positive later.
- There are 346 lines at -0.625 and 1769 at -0.5.

In u04 there are 541 lines at -0.625, starting at step 130.

### 384h: one float in the image

The image holds 384h as a single float:
- The direction-change paths zero it, at 009ED86F, 009ED8AD and 009EDB73. The dead 009D5277 also
  zeroes it.
- The stall 009F47EE adds up to 2.0 per back-off.
- The total is compared with the threshold 10.0 ([00CE38B8]).

The host splits it in two:
- the int marker `blk.direction_counter_384`, which the direction-change writers zero;
- the float `obstacle.stall_time_384`, which the stall accumulates.

The marker block at `:11988` (`kShipAiStallSharedBound`, ON) zeroes the float whenever the marker is 0.
The marker starts at 0, so the first drive zeroes the float.

With the switch ON, the build seeds `stall_time_384 = -1` and sets the marker to 1. The first drive
then keeps the -1, and the float runs from -1 until the first direction-change zero, as in the image.

Long term, the host should hold blk+384h as one float. The marker would then go away
(`ship_ai_obstacle_tables.hpp:194` already says the int type is wrong).

### 354h: the hold

The image seeds 354h with -1.0f at 009DFD71. The host holds two copies of this field:
- `throttle_profile.hold_354`, which 009E0591 counts down and 009DE8FF tests;
- `blk.clamp_354`.

The switch seeds both.

With OFF, the hold starts at 0. 009E0591 counts it down on the first 009E04E0 pass even when nothing
raised it, because 0 >= 0 gives -dt. So OFF differs from ON only on the steps before that first
countdown.

There is a separate, pre-existing split, which this switch does not change:
- In the image, the 009F00E3 raise (in 009EF910) lands on the same blk+354h that 009DE8FF tests.
- The host sends that raise to `ShipAiClearanceBlock::hold_354` (`ship_ai_clearance_profile.cpp:278`),
  and nothing copies it back or reads it.

Routing that raise to `throttle_profile.hold_354` / `blk.clamp_354` is a behaviour change with its own
reach. It should be its own switch and pair, not part of a seed fix.

### 364h in the clearance refresh (rev2 nit a)

009EF910 reads blk+364h directly. The prologue sets ESI = ECX = blk at 009EF91B, ESI is not rewritten
before the reads, and the function stores nothing to +364h. The reads are:
- 009EFFDC `CMP byte [ESI+364h],BL` / `SETE`, which gives "committed" = !364h in steering mode 1;
- 009EFFEC `MOV BL,[ESI+364h]`, which gives "committed" = 364h in mode 2.

The host models this at `ship_ai_clearance_profile.cpp:286/288` with
`ShipAiClearanceBlock::committed_ahead_364`. Nothing in `src/` writes that field, so it stays false.
The host's copy-in before the call (`game_hosts_ship_ai.cpp`, beside the 324h/330h/35Ch/3F0h/3F4h/
A3Ch/374h copies) does not include 364h. With the seeds ON, the block and the clearance copy
therefore disagree from step 0. Even with the seeds OFF, they disagree after any escape sets 364h.

The second OFF switch, `kShipAiClearance364SyncCc13Bound`, copies `blk.yaw_rate_subtracts_364` into
`clearance.committed_ahead_364` just before each 009EF910 call.
- Since the image reads the block byte live and 009EF910 never writes it, a copy at call entry is
  faithful in time.
- It is separate from the seed switch because it also mirrors later escape flips, so it changes
  behaviour on its own.
- It needs its own pair, plus one with the seeds ON.

## 2. ON data lifetime

The seeds are written once, inside the controller build in `register_ship_ai_unit`:
- `register_units` clears and resizes `controllers` and builds every unit. This happens before any
  tick, so before the first 009ED6B0.
- `register_generated_units` runs the same `register_ship_ai_unit` for units created after load, so
  the build seeds those too.

After the build, each field changes only as follows.

364h:
- Only the escape writes it (`ship_ai_obstacle_tables.cpp:678`/`:692`).
- No host path resets it before the first escape. A grep for `yaw_rate_subtracts_364` over `src/` and
  `include/bsp/` finds no other writer.
- Nothing reassigns `ctl.blk`, `ctl.obstacle` or `ctl.throttle_profile` wholesale. There is no
  `Controller{}` assignment and no `blk =` / `obstacle =` / `throttle_profile =` store.

384h:
- The float changes through the stall accumulation (`:544`).
- The escape and back-off zero it (`:677`, `:691`, `:721`).
- The marker block zeroes it after a direction-change writer zeroes the marker. Those writers are
  `ship_ai_states.cpp:121/151`, `ship_ai_station_keeping.cpp:103` (via `:10912`) and
  `ship_ai_navigation_arm_tail.cpp:58`.

354h:
- 009E0591 counts it down. With -1, it does not count, since -1 is not >= 0.
- The arm final escape raise (`:2332`/`:2333`, 009DE763) raises it.
- So do the obstacle and states writers (`ship_ai_obstacle_tables.cpp:478`, `ship_ai_states.cpp:174/193`)
  and the neighbour clip (`ship_ai_neighbour_clips.cpp:601`).

## 3. Reach and pass signature

Reach: every mission with navigating ships, because 364h steers every ship that has not yet run the
escape. A pair is needed before any flip.

Pass signature for cc9:
- In `p24_off_u21f`, the Iowa group (steps 5050+, pos_after = 0 with OFF) should show positive
  throttle with ON.
- The count of throttle -0.625 lines should fall (346 in u21f OFF; 541 in u04 OFF).
- Check with `BSP_SHIP_ESCAPE_TRACE=Iowa`. It prints `364=a->b` and `384=` per drive.

## 4. The 009DBDF0 plan pair

The element ctor is 009D9CC0, and its defaults match `ShipAiPathPlanBlock`. The +48h links are modelled
by `plan_front`.

The one store with no host counterpart is +DCh = -1.0f (009DBE1D, blk+300h). It has no host storage
and no host reader, so it is a later item.

## 5. Excluded seeds and settled items

- **3E0h:** hitq's `kShipAiCruiseDistanceSeedBound` (main c05b1db7f) covers it.
- **3A5h/3C1h:**
  - The live readers use `blk.flag_3a5` and `Snapshot.flag_3a5` (both false); the
    `enable{true}` copies are not what they read.
  - `station_request.enable{true}` and `Snapshot.station.enable{true}` are dead copies.
  - Tidying them is a later item.
- **37Ch:** 009EF910 refreshes it on its first call (timer 0). The host copies it at `:11921` before
  the drive.
- **340h (settled with hitq):** the build copies `nav_block.look_ahead_340` (the ctor's 009E45BB
  value) at `:12306`, and 009ED769 rewrites it every tick (`:10823`). The `{0}` default in
  `ShipAiNavState` is overwritten before any read. Excluded.
- **214h (settled):** 009DDBC0 saves 1C4..1F0 into 1F4..220. 009DDC8E/009DDC95 copy [ECX+1E4h] to
  [ECX+214h], so 214h is the saved final_leg.
  - The host field is `Snapshot.final_leg_1e4{false}`, while the image seeds 1.
  - 009DA0D0 restores it only after a snapshot is taken.
  - Low priority, not in the switch.
- **3C0h (settled):** 009DDC08/009DDC0F copy [ECX+3A4h] to [ECX+3C0h]. The host's
  `Snapshot.station.making_way{true}` is already 1.
  Its one image reader is the restore 009DA0D0, which copies it back to 3A4h, so the host already
  matches. No host field is needed.
- **3DCh = 400.0f (no reader):** 009DFCB0 stores it at [ECX+218h]. A disp32 scan
  (`disp_scan.py`) of 009D0000..00A00000 found no access to it from any other ship AI function:
  - There is no `[blk+3DCh]` access. There are 89 `+3DCh` hits across the whole image, none of
    them in this segment.
  - There is no `[brain+3E4h]` read, since blk = brain+8h. The `+3E4h` hits are blk+3E4h itself
    (009E0270, 009E4330, 009F0100) and a bomber state field (009D254E).
  - The `+218h` hits are other bases: blk+218h in 009DA0D0 and 009DDBC0, the follow state
    (009DF2D0), and the ring slot (009E6640, 009E76D0).

  It is outside the 009DDBC0 save ranges. Listed as no-reader; no host field was added.

  Scope (rev2 nit c): the `+3DCh` scan covered the whole image, 00401000..00CA0000. The
  `+3E4h`/`+218h` alias scans and their base attribution covered only the ship AI band
  009D0000..00A00000. A blk reader outside that band through another base was not ruled out.
- **300h:** this is the plan pair's +DCh (section 4), not a 009DFCB0 store. hitq agrees.

## 6. Uncertainty

- The pass signature is a prediction from the image readers and the OFF log. No ON run exists.
- 384h with ON: the host still keeps two fields. The seed reproduces the image only while the marker
  model holds, meaning every image zero of blk+384h is mirrored by a marker zero.
- No arithmetic in this packet needs x87 modelling. The seeds are exact constants (1, -1.0f).

## 7. The 3A5h/3C1h dead copies (packet cc13-deadcopy)

The image holds blk+3A5h as one byte, and blk+3C1h is its saved copy. The host holds each twice:
- blk+3A5h is `ShipAiControlBlock::flag_3a5` (live) and `Controller::station_request.enable`, which
  the 009DA3B0 copy writes.
- blk+3C1h is `Snapshot.flag_3a5` (live) and `Snapshot.station.enable`, which the whole-struct copy
  of `station_request` writes.

Nothing in `src/` reads either `.enable` copy. The only `.enable` reads are of the 009E1610 stack
request, at `game_hosts_ship_ai.cpp` (the 009DA3F7 store into `flag_3a5`) and
`ship_ai_follow_land.cpp`.

### Image scan

A disp32 scan of 009D0000..00A00000 looked for +3A5h, +3C1h, and brain+3ADh/+3C9h (blk = brain+8h).

| address | function | access | host model |
|---|---|---|---|
| 009DA3F7 | 009DA3B0 publish | write | `blk.flag_3a5 = request.enable` |
| 009DA12B | 009DA0D0 restore | write from +3C1h (read at 009DA125) | `blk.flag_3a5 = s.flag_3a5` |
| 009DDC15 | 009DDBC0 save | read, stored to +3C1h at 009DDC1B | `s.flag_3a5 = blk.flag_3a5` |
| 009DF074 | 009DE5B0 | read | `ship_ai_arm_final_step.cpp` `blk.flag_3a5` (009DF065..009DF097) |
| 009ED730 / 009ED753 | 009ED6B0 | read, then clear | `ship_ai_states.cpp` `blk.flag_3a5` |
| 009EDA28 | 009ED6B0 station-keeping arm | read | the `blk.flag_3a5 && !flag_3a6` gate in the host |
| 009EEF45, 009EF0A8 | 009ED6B0 arm tail | read | `ship_ai_navigation_arm_tail.cpp` `blk.flag_3a5` |
| 009F145E | 009F1420 pre-pass | brain+3ADh = 0 | host `blk.flag_3a5 = false` (kShipStationKeepingBound) |
| 009F4F8C | 009F4DA0 | brain+3ADh read | host `ctl.blk.flag_3a5` (kShipStationKeepingBound) |
| 009D5C80 | (undefined, 8 bytes) | write 0 | none: the stub `MOV byte [ECX+3A5h],0 / RET` has no rel32 or absolute reference |

Excluded hits:
- 009D3433 `[ECX+3ADh]` is the torpedo approach state (`torpedo_approach_update.cpp`), not a brain.
- 009FD181 is a misdecode.

+3C1h has one reader in the image, the restore at 009DA125. The host models that through
`Snapshot.flag_3a5`.

The reference scan covers every section, for rel32 E8/E9 and absolute dwords. It found:
- no hits for 009D5C80;
- positive controls: 009DA3B0 called from 009E18B6, and 009DA0D0 called from 009F51B7.

### Result

Every image access to the byte is modelled on `flag_3a5`. The `.enable` copies are host-only and
have no image counterpart beyond the same byte.

Removing them would change `ShipAiStationRequest`, whose `enable` field is the live 009E1610 request
value. So the tidy labels the two dead copies in place: comments at the `Controller` members and at
the 009DA3B0 copy. There is no code change, so OFF behaviour and output are identical.

Scope:
- The +3A5h/+3C1h/+3ADh/+3C9h scans covered only the ship AI band.
- The sibling +3A6h (`station_request.suppress` against `Controller::flag_3a6`) looks like the same
  pattern, but it was not scanned.

## 8. The 3A6h/3C2h dead copies (packet cc13-astern3a6)

3A6h follows the same pattern as section 7:
- blk+3A6h is `Controller::flag_3a6` (live) and `station_request.suppress`, which the 009DA3B0 copy
  writes.
- blk+3C2h is `Snapshot.flag_3a6` (live) and `Snapshot.station.suppress`.

Nothing in `src/` reads either `.suppress` copy. The only reads of `.suppress` are of the 009E1610
stack request.

### Image scan

A disp32 scan of 009D0000..00A00000 looked for +3A6h, +3C2h, and brain+3AEh/+3CAh. Every hit maps
onto `flag_3a6`:

| address | function | access | host model |
|---|---|---|---|
| 009DA400 | 009DA3B0 publish | write | `ctl_.flag_3a6 = request.suppress` |
| 009DA138 | 009DA0D0 restore | write from +3C2h (read at 009DA131) | `ctl_.flag_3a6 = s.flag_3a6` |
| 009DDC21 | 009DDBC0 save | read, stored to +3C2h at 009DDC28 | `s.flag_3a6 = ctl_.flag_3a6` |
| 009EDA3A | 009ED6B0 station-keeping arm | read | the `blk.flag_3a5 && !flag_3a6` gate |
| 009EF0B1 | 009ED6B0 arm tail | read | `tail.leader_snapshot_3a6`, copied from `ctl_.flag_3a6` before the tail |

There were no brain+3AEh or +3CAh hits.

Positive controls:
- The disp scan finds the known 009DA400 store.
- The reference scan, over every section for rel32 E8/E9 and absolute dwords, finds 009DA3B0 called
  from 009E18B6 (rechecked in section 7).

### Result

The `.suppress` copies are labelled in place, as in section 7, with comments only. OFF behaviour and
output are unchanged.

Scope: the scan covered only the ship AI band.
