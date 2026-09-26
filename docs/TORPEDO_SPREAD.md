# Torpedo spread: the per-ship alternating offset at unit+6D4h

Addresses: 008FFF20 00900280 009002AB 009002BF 0090094D 00900951 00951FC0 00CF8608 00D19BE8
00D7A208 0095CE11 0042D7E0

Packet `cc9_torpedo_spread`, Ghidra read-only. Every descriptive name is a hypothesis, not a
recovered symbol. `docs/TORPEDO_LAUNCH_GATE.md` (the lead point and the launch command) and
`docs/GUN_BOT_REMAINDER.md` (`00951FC0`) are cited, not restated.

## 1. The rule

* **Aim.** `008FFF20`, the torpedo bot's heading tick, takes the target through the bot's
  `vtable[44h]` and its world matrix through `0042D7E0`. At `00900280..009002BF` it adds
  `unit+6D4h * matrix+20h` to the intercept's x and `unit+6D4h * matrix+28h` to its z, where
  `unit` is the owner (`EDI`). The intercept comes from `008FBB00`. Matrix `+20h` is the third row,
  the target's forward axis, since the translation row sits at `+30h` (entity `+FCh` = `+CCh + 30h`).
  So the offset slides the aim point along the target's track. The friendly-crossing gate reads
  the same point.
* **Flip.** `0090094D` fires the gun (`vtable[1F0h]`), and `00900951` calls `00951FC0` on the
  owner:
  * a negative value becomes `-0.0 - value` (`00D7A208`);
  * a non-negative one becomes `-value - 35.0` (`00CF8608`, double);
  * a result above 80.0 (`00D19BE8`) is reset to 0.
* **The flip, from the listing.** `00951FCC COMISS value, 0` / `JB 00951FE9` sends a negative
  value to `00951FE9..00951FF5` (`-0.0 - value`, SSE). A value >= 0 goes through `FCHS` /
  `FSUB qword [00CF8608]` / `FSTP` (`00951FD6..00951FE1`). The reset test runs **after** the
  store on the new value: `00951FFD` reloads `+6D4h`, and `00952005 COMISS new, [00D19BE8]` /
  `JBE` leaves `EAX = 0` when `new <= 80.0`. Otherwise `EAX = 1`, and the `CVTSI2SS` / `fabs` /
  `UCOMISS 0` / `LAHF` / `TEST AH,44h` / `JNP` tail stores 0 at `00952039` exactly when
  `new > 80.0`, strictly.
* **Start.** The unit constructor stores 0 (`0095CE11`, `XORPS XMM0,XMM0` at `0095CD7D`).

One ship's launches therefore aim at 0, -35, +35, -70, +70 and -105 m. The next flip gives 105,
which is reset to 0, so the cycle has six launches. All of a ship's tubes share the one value.

Labelled: the host adds the offset whether or not the intercept solved, where it falls back to
the target point. `0095BD08` also reads `+6D4h` and was not followed.

## 2. Switch

`kTorpedoSpreadBound` in `src/game_hosts_gunnery.cpp`.

## 3. Predictions (written before any run)

The baseline is current main (`ar_on`), USN02 9200/9000. It fires 274 torpedoes from 23 ships for
20 hits (7.3%) and 38384 damage, most of the 59842 total. USN04 fires none.

1. **USN04:** identical.
2. **USN02 torpedo hits.** Three of every six launches aim within 35 m of the intercept, and the
   rest at 70 to 105 m along the track. Against a destroyer half-length of about 50 m, the far
   launches hit only when the intercept errs along the track by a similar amount. Hits land
   between 14 and 26, hit rate 5 to 10%. Launch counts stay within 10% unless the death table
   moves the fight.
3. **USN02 deaths:** between 17 and 27, with the order of the torpedo-killed ships changing. The
   44.60 s failure does not move.

## 4. Results

Same-tree builds `ts_off` and `ts_on`, window line and module directory checked, logs deleted
first. `BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`.

| run | torpedoes / hits / damage | deaths | hit records | total damage | end |
| --- | --- | --- | --- | --- | --- |
| USN04 OFF / ON | 0 | 31 / 31, identical | 528 | 7215.6 | - |
| USN02 OFF | 274 / 20 / 38384 | 23 | 413 | 59841.9 | failed at 44.60 s (Exeter sank at 43.45 s) |
| USN02 ON | 217 / 12 / 29986 | 20 | 487 | 53671.0 | failed at 39.65 s (Exeter sank at 35.95 s) |

`usn_2_java.lua` fails the mission when Houston or Exeter is dead (line 521), and ends it a
little over a second later.

1. **Held:** USN04 is identical.
2. **Did not hold:** hits fell to 12 (4.4% of 217), under the 14 to 26 band, and launches fell by
   21%, more than 10%. The spread moved the early torpedo kills:
   * Java sinks at 27.75 s to Yudachi (141.30 s OFF), and Exeter at 35.95 s to Tokitsukaze
     (43.45 s OFF).
   * DeRuyter still goes, at 30.25 s.
   * Alden sinks at 149.35 s instead of 34.50 s. Perth, sunk at 46.65 s OFF, survives the run.
3. **Deaths held** at 20, inside 17 to 27. **The failure time did not hold:** Exeter's earlier
   torpedo death ends the mission 4.95 s sooner, at 39.65 s instead of 44.60 s.

`kTorpedoSpreadBound` is ON. The USN02 9000 reference moves:
* deaths 23 -> 20;
* hit records 413 -> 487;
* torpedoes 274 -> 217;
* failure 44.60 -> 39.65 s.
