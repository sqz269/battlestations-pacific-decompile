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

Pending.
