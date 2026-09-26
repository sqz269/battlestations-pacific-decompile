# The AA fire window in the gun's own frame

Addresses: 0085A9A0 0085A9D3 0085AA27 0085AA69 0085AA8F 0085AAA3 0085AACC 0085AB32 007F60A0
0095F500 00B63D50 0042D0D0 00521370

Packet `cc9_aa_fire_window_mount`, Ghidra read-only. Every descriptive name is a hypothesis, not a
recovered symbol. `docs/AA_TARGETING.md` (section 8's open item) and
`docs/SHIP_PLATFORM_ATTACHMENT.md` (the slot frames) are cited, not restated.

## 1. What the image tests

From the disk listing of `0085A9A0`:

1. The origin is the gun node's world translation, `[gun+3CCh]+120h..+128h` (`0085A9E9..0085AA18`).
   The node is refreshed through `00B6DB70` when `+5Ch` bit 1 is clear. The direction to the
   target's pose origin (`target+FCh`, `0085AA27..0085AA51`) is stored to the caller's buffer.
2. The frame is `[gun+3Ch]`. `0085AA69..0085AA8F` refresh its pose (`00414DB0`) when `+10Ch` is
   clear, build `+110h` from `+CCh` with `00B63D50`, and set `+10Ch = 1`. `0085AAA3` then
   transforms the direction by `+110h` as a direction (`0042D0D0`, flag 0), and `00B63D50`
   normalises it. That is the entity layout of a unit: `+CCh` world matrix, `+10Ch` the cached-inverse
   flag, `+110h` the inverse. It is the same idiom `00826F10`'s R11c uses on the ship.
3. `00521370` turns it into yaw and pitch. The yaw is wrapped by 2 pi when it is at pi or more.
   `0085AB32` asks `007F60A0` with `(-yaw, pitch)` over the arc list of the class's platform row
   (`[gun+3F0h]+538h+94h`, index `gun+38Ch`).

## 2. Why the hull frame is exact here

`[gun+3Ch]` is the gun's parent entity (`gun_parent_*` in `include/bsp/gun_bot_ticks.hpp`: present
test and `+5Dh`). Whether it is the ship or a platform object, its orientation is the ship's in
this installation. Every one of the 336 slot frames the host bound on USN02 (173) and USN04 (163)
has `forward = (0, 0, +z)` in model space, aligned with the hull, and the aft turrets' windows are
authored in that frame (for example DeRuyter platform 4: `[-180..-35] [35..180]`). So the host's
test is not a substitution in the frame:
* direction from the gun's mount point (`gun_muzzle_point`, the 0095F500 slot origin carried by
  the ship pose) to the target's pose origin;
* yaw and pitch against the ship's axes;
* `gun_fire_allowed_007f60a0` with the sign of `docs/GUN_HORZ_SIGN.md`.

Labelled: the origin is the slot point plus the muzzle offsets, not the node's own translation,
a few metres at most against AA ranges.

The test was computed on every call already (`bind_aa_acceptance`) and only its application was
switched off, so the OFF runs report what it would refuse: USN04 1437 refusals, USN02 0.

## 3. Switch

`kAaFireWindowBound` in `src/game_hosts_gunnery.cpp`, OFF since packet `cc9_aa_targeting`.

## 4. Predictions (written before any run)

Current main plus this branch, `BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`.

1. **USN02 9000:** 0 refusals, so identical.
2. **USN04 4500:** the 1437 refused slots pass the plane to the next gun in the rank order, or
   leave the slot empty. Guns that cannot bear stop holding a target they cannot shoot, and guns
   that can bear pick those planes up.
   * AA shots on planes change by -10 to +25%, and AA hit records by -15 to +25% (528 now).
   * Plane deaths land between 26 and 36 (31 now).
   * The per-target rows move within a few seconds, because the AA stream draws re-key.

## 5. Results

Same-tree builds `fw_off` and `fw_on`, window line and module directory checked, logs deleted
first.

| run | deaths | hit records | total damage | window refusals | AA shots (sub-type 1 / 5) | AA hits (1 / 5) |
| --- | --- | --- | --- | --- | --- | --- |
| USN04 OFF | 31 (16 Kates, 12 Vals) | 528 | 7215.6 | 1437 (observed) | 1879 / 92 | 73 / 65 |
| USN04 ON | 30 (16 Kates, 11 Vals) | 520 | 6971.7 | 3443 (applied) | 2279 / 87 | 74 / 64 |
| USN02 OFF / ON | 20 / 20, identical | 487 | 53671.0 | 0 | - | - |

1. **Held:** USN02 is identical, and it still fails at 39.65 s.
2. **Held:**
   * AA shots on planes rose 20%, from 1971 to 2366 (inside -10 to +25%).
   * Hit records fell 1.5% (inside -15 to +25%).
   * Deaths are 30 (inside 26 to 36).

   Guns that can bear take the planes the refused guns used to hold, so sub-type 1 fires 400 more
   rounds for one more hit. The Kate torpedo squadrons still die between 199.66 s and 215.66 s,
   a few tenths of a second to five seconds later than OFF. One Val (#3.1|.-2, 193.51 s OFF)
   survives, Val #3.1 dies 9.85 s earlier (143.45 s), and the rest move by up to 2.4 s.

`kAaFireWindowBound` is ON. The USN04 reference moves from 31 to 30 deaths and from 528 to 520
hit records. USN02 does not move.

## 6. E2 (USN04 9200/9000): predictions written after the flip, before these runs

The integrator asked for E2 after the flip was committed, so these predictions follow the 4500-frame
result and precede only the two E2 runs, on the same `fw_off` and `fw_on` builds.

* AA hit records change by -10 to +10%.
* Kate deaths change by at most 2.
* Total deaths are within 4 of OFF.
* Window refusals applied are more than 2 times the OFF run's observed count.

| E2 run | deaths (Kates / Vals) | hit records | total damage | window refusals |
| --- | --- | --- | --- | --- |
| OFF | 35 (16 / 16) | 586 | 7968.1 | 1624 observed |
| ON | 35 (16 / 16) | 594 | 7845.7 | 4214 applied |

All four held: hit records +1.4%, Kates unchanged, deaths unchanged, and 4214 is 2.6 times 1624.
The E2 reference keeps 35 deaths, and its hit records move from 586 to 594.

