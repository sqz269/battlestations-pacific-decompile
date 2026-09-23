# Aimdive response: the image's dive law after the roll, and why binding it does not help yet

Addresses: 009C58D0, 009C5B43, 009C5C9F, 009C5D8E, 009C5DB8, 009C5DE0, 009C5E01, 009C5E5D, 009C5F19, 009C5F9E, 009C5FCB, 009C6071, 009C6080, 00B63D50, 004142E0, 00419010, 009C62B0, 009C6453, 009C6AD9, 009C6BB4, 009C6F97, 009C6FB0, 009C6FFB, 007C4810

Packet `cc9_aimdive_response`, 2026-09-22. It follows `docs/HULL_AIM_TURNDOWN.md`. The report is
`reports/aimdive_response.json`. All names are hypotheses, not recovered symbols.

**Result.**
- **No guard at the pass.** The image does not guard the singular bearing when the predicted
  impact passes the point.
- **An unbound tail.** After the roll, the image writes three commands this host never wrote: a yaw
  toward the aim point in the aircraft's own frame, a throttle, and an air brake. So the host dived
  at full power and zero brake.
- **Binding it makes USN04 worse.** The tail is read whole and bound as a pure rule. With it on,
  releases fall from 23 to 4. The host's aim error saturates the pitch command, and the tail then
  holds minimum power and maximum brake for the whole dive.
- **Both switches stay false.** The tail is committed behind `kAimDiveTailBound = false`.
- **Fly-over still unread.** Its desired-speed arm is read but not bound: the frame slot that
  scales it is still unpaired.

## 1. The image's aimdive law, 009C58D0

Settled in `docs/HANDOFF_DIVE_BOMB_AIM.md` (a) and earlier binds, not re-derived:
- **Pitch.** 009C5C9F-009C5CF7 computes pitch as error x (approach+64h or +68h) and clamps it to
  [-1, 1]. Below 30° nose-down the steep gate at 009C5BD4 forces -1.0.
- **Roll.** 009C5D0E-009C5DA3 maps one of two bearings with 00419010.

**Roll, with its band choice.** The wide band, 00CE69D0 = -0.5 to 00CE3800 = +0.5 (floats), is
used only while the error slot [ESP+5Ch] is above 0.0 and |bank| [ESP+20h] is below 00D05AAC,
which is 60°. In that band the roll input is [ESP+18h]: heading less the bearing from the predicted
impact to the aim point. Otherwise the band is 00D1F400 = -0.4 to 00CE7804 = +0.4, and the input is
[ESP+24h], the unit-to-point bearing.

**No guard at the pass.** Nothing in 009C5C9F-009C6080 gates, holds or rate-limits the roll input
when the impact passes the point. The only change of source is the band switch above, keyed on
the sign of the error. So the singular input the previous packet traced is the image's own law.

**The tail, 009C5DB8-009C6080, read whole and unbound before this packet:**

* **Yaw, cmd+284h.** 009C5DC8 refreshes the aircraft's pose. 009C5DE0 then builds its orthogonal
  scaled inverse with 00B63D50 into unit+110h, cached by the byte unit+10Ch. At 009C5E01, 004142E0
  carries the `vtable[0]` aim point into the aircraft's frame, giving x at [ESP+40h] and z at
  [ESP+48h]. The command is `yaw = x / |z| x 8.0`, with the gain at 00CE3DB0 (double). It is
  stored as a float at 009C5E5D, with +288h = 1 at 009C5E3F and +2D4h = 0 at 009C5E63.
* **Pull-out margin, [ESP+20h].** It starts at 0.0 (009C5E49). With `t = -pitch` above 0
  (009C5E6F-009C5E80):
  - If |bank| is above 00CE3830 = π/2 (double), then t = 00CE3D28 - t, which is π - t.
  - Then `M = speed x (t / class+1ACh) x (1 - cos t) + 00CE3938 (50.0)`.
  - Here speed is unit->vtable[38h] (009C5EDE) and class+1ACh is PitchSpd (009C5EE4).
* **Below the release floor** (approach+A8h above the height [ESP+14h], 009C5F20-009C5F2A):
  throttle cmd+278h = 00CE7638 = 0.05 and air brake cmd+2A8h = 1.0.
* **Otherwise**, with a = |pitch command| (009C5F4C) and the PilotBot row at approach+14h:
  - `I1 = interp(0 -> row+44h, 1 -> row+48h, row+54h x a)` (009C5F9E) is the power cap, from
    MaxPowerCtrl down to MinPowerCtrl.
  - `I2 = interp(0 -> row+50h, 1 -> row+4Ch, a)` (009C5FCB) is the brake floor, from
    MinBrakeCtrl up to MaxBrakeCtrl.
  - `v = clamp((h - M) / 00D1F3F8 (120.0), 00D7A238 (0.01), 00D7A24C (1.0))`.
  - throttle = min(v, I1) (009C6016-009C602A); air brake = max(I2, 1 - v) (009C6030-009C604E;
    009C6044 is JA).
  - The stores are at 009C605F, 009C6071, 009C606A, 009C6079 and 009C6080.
* **The row values.** They come from this installation's `robots.lua`, PilotBot SPNormal: 0.7,
  0.2, 0.5, 0.0 and 3.0.

**The abort, 009C5B01-009C5B3E.** It fires only when all three hold:
1. approach+D4h + approach+50h is above the height [ESP+14h].
2. pose+C64h is above 00D20338 = -60° (float).
3. 0.3 x height + 150 is above [ESP+1Ch], the planar distance to the aim point. The 0.3 is
   00CE3DC8 and the 150 is 00CE3DD8, both doubles.

| image term | host before this packet |
|---|---|
| pitch loop, steep gate | bound, same |
| roll bands and sources, no guard at the pass | bound, same |
| yaw x/\|z\| x 8 | **absent**: yaw slot never written in the aimdive |
| throttle min(v, power cap), 0.05 below the floor | **absent**: holds the attackrun's 1.0 |
| air brake max(brake floor, 1 - v), 1.0 below the floor | **absent**: holds the attackrun's 0.0 |
| abort thresholds | bound, same; host feeds approach+50h as 0 where the image has the aim point's y (0-2 m) |

The traced control confirms the last three rows: `thr=1.000 brk=0.000` on every aimdive tick
of movieval|.-2.

## 2. The fly-over's exit state, 009C62B0

The heading arm and the altitude arm are bound by earlier packets. The bank arm, 009C69B1/009C69B9,
is a constant wings-level target, cmd+2C4h = 0.0 with mode 1, and the heading's mode 2 overrides
it when both run.

The desired speed, 009C6F97-009C6FFB, reads as follows:
- **Stores:** cmd+2B4h = `approach+A4h + d`, with cmd+2B0h = 0 and cmd+2D8h = 1.
- **The term d:** if the frame slot [ESP+2Ch] is above 0, `d = [ESP+2Ch] x 00D7A378`. Otherwise
  `d = (approach+A4h - 007C4810(class)) x [ESP+2Ch]`, where 007C4810 is called at 009C6FB0.
- **Unresolved slot.** [ESP+2Ch] has three writers in the body: 009C6453, 009C6AD9 and
  009C6BB4/009C6BC4. The last two sit inside the heading arm's argument windows. Pairing them with
  the 009C6FB9 read needs an ESP-depth sweep, which I did not complete.

So **the fly-over's speed is not bound**, and **why the fly-over hands the dive a 72-270 m
sideways offset is still open**. The candidate this read leaves is the unbound speed command: the
host flies the fly-over at whatever speed the attackrun left.

## 3. Predictions made before the runs

With the tail bound and the hull switch off:
- The dive slows, from brake up to 0.5 and power capped at 0.7 falling to 0.2.
- The yaw holds the nose on the point.
- The sideways swing at the pass shrinks, and the abort count falls.
- Turndown entry is unchanged by construction, since the tail runs only in the aimdive.

With the hull switch on, releases should partly recover.

## 4. The four runs

All four use the before-log parameters, with the trace switch on. Logs are in this tree's `local\`.

```
./tools/run_game.ps1 -Exe local\bin_<v>\bsp_game.exe -Log local\<v>_usn04.log -- --frames 5000 --press-start-frame 30 --menu-select USN04 --mission-frames 4800 --mission-frame-seconds 0.05
```

| log | tail | hull | releases | impacts | aborts | torpedo swims |
|---|---|---|---|---|---|---|
| `coff_usn04.log` | off | off | 23 | 20 | 2 | 12 |
| `boff_usn04.log` | **on** | off | **4** | 2 | 8 | 10 |
| `con_usn04.log` | off | on | 8 | 6 | 8 | 12 |
| `bon_usn04.log` | **on** | on | **0** | 0 | 2 | 11 |

The control off-run matches `feedoff_usn04.log` from `docs/HULL_AIM_TURNDOWN.md` on all 3,743
census lines, so this tree's default equals main.

The four criteria:
- **(a) Turndown entry.** Identical control against bound, as predicted: the same 837 turndown
  lines, and the same first line for movieval|.-2.
- **(b) The pass.** The prediction failed. Bound, movieval|.-2 is still 265 m short at 385 m, at
  pitch -0.58 and banked 1.3-1.7 rad. The pitch command sits at ±1, because the error still swings
  -200 to +224 m. So the tail holds power 0.2 and brake 0.5, and the aircraft sinks at about 40 m/s
  instead of 100 m/s. Aborts rise from 2 to 8.
- **(c) Justified term by term.** Fails. The release rows move far, and torpedo swims move too
  (12 to 10). Those move through a changed mission, not through any bound term, so this was not
  shown term by term.
- **(d) Hull switch on.** Nothing to judge: 0 releases.

## 5. Decision

**`kAimDiveTailBound` stays false and `kHullAimOffsetEnabled` stays false.** The tail rule and its
wiring are committed behind the switch because the read is complete and bounded. They must not be
switched on until two things hold:
- The host's aim error is small enough that |pitch command| is not saturated through the dive.
- The fly-over's speed command is bound.

The image's dive keys its power and brake on |pitch command|. So in the image the dive only works
if that command is modest. This host's error of ±200 m is the term still out of line with the
image. Its known unread input is the fly-over's speed, and with it the dive entry speed.

Whether the image's dive is as sensitive as this host's cannot be settled from the listing. The
image has no guard at the pass. But it runs the pass slower, braked and power-capped, and with a
rudder on the point. This host has never flown it that way with a sane pitch command.

**Next read:** pair the fly-over's [ESP+2Ch] with an ESP-anchored sweep of 009C62B0-009C6FB9, and
bind the desired speed. Then rerun this four-run table.

## Correction from docs/FLYOVER_SPEED.md (2026-09-22, packet cc9_flyover_speed)

Section 2 says 009C6AD9 and 009C6BB4 "sit inside the heading arm's argument windows", which implies
they might be argument pushes. They are not. An ESP sweep anchored on the epilogue (true depth 152
before 009C7062 `POP EDI`) puts all three writers, 009C6453, 009C6AD9 and 009C6BB4/009C6BC4, at
depth 152. They all write frame slot entry-108, the slot the speed arm reads at 009C6FB9.
009C6AD9 zeroes it on every tick. Only the 007F0280 near-field avoidance arm overwrites it, so the
speed arm's frame slot is 0.0 without a neighbour. The desired speed is then 0.95 x MaxSpd in speed
mode. It is now bound, and it is not the term that saturates the aimdive's pitch command.

## Correction, 2026-09-23 (packet cc9_dive_throttle)

* Section 4(b) was measured with the throttle frozen at 1.0 (`docs/PILOT_THROTTLE_SLOT.md`). The
  fly-over and turndown could not slow the aircraft, and it entered the dive at about 113 m/s.
  With the speed hold working (`docs/DIVE_THROTTLE.md` run T1), the dive starts at 54 m/s. The
  entry error still swings about 540 m peak to peak, but it settles within about 40 ticks and
  stays inside 25 m from 532 m down. The tail then lifts the brake and adds power. So "holds power
  0.2 and brake 0.5 ... through the dive" does not hold on a throttling host. Releases are still 0
  there, for a different reason: the dive-abort and aimglide-ceiling gates. See
  `docs/DIVE_THROTTLE.md` section 4.
