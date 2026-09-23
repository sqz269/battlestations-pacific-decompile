# The dive states' throttle writes, and the dive with a working speed hold

Addresses: 009C4A40, 009C4A43, 009C4C0C-009C4CA7, 009C4CBA-009C4CE7, 009C5180, 009C534B,
009C55E5-009C567F, 009C58D0, 009C5B43, 009C5F37, 009C605F-009C6080, 009C44F0, 009C4512,
009C4524, 009C6FF1, 009C4413-009C4434, 0099D300, 0099B542.

Packet `cc9_dive_throttle`. Every name is a hypothesis, not a recovered symbol.
Background: `docs/PILOT_THROTTLE_SLOT.md` (`kPilotThrottleSlotBound`, the speed-hold multiplier).

## 1. goaway (`BSP_BotStateDiveBombGoAway_Tick` 009C4A40)

The nose-down byte (009C4A43-009C4A68) is 1 while `[00CF885C]` (-0.0873 rad, dword) exceeds
`unit+C64h` (the pitch). `EBX = 1` from 009C4A5C.

* **Nose down** (byte 1, 009C4C0C-009C4CA7):
  `t = 00419010(x0 = -0.5 [00CE69D0], y0 = -1.0 [00D7A260], x1 = 0 (FLDZ), y1 = 1.0 (FLD1),
  x = unit+C64h)`. Then:
  * throttle `+278h` = clamp(t, 0, 1) (009C4C79-009C4C91, dword);
  * air brake `+2A8h` = clamp(-0.0 [00D7A208] - t, 0, 1) (009C4C4A-009C4C9F, dword);
  * `+27Ch` and `+2ACh` = BL = 1 (bytes, 009C4C8B / 009C4C99);
  * `+2D8h = 0` (009C4CA7, dword).
  So at -28.6 degrees or steeper the plane has no power and full brake. The throttle reaches
  1 - 0 at a pitch of -0.25 and climbs to 0.65 at the -5 degree gate.
* **Otherwise** (009C4CBA-009C4CE7): throttle 1.0 [00D7A24C], air brake 0 (XORPS), both active,
  `+2D8h = 0`.

The host wrote only `+2D8h = 0` on both sides. It now writes the slots too, as
`dive_bomb_goaway_throttle_009c4c0c` behind `kGoawayThrottleBound`.

## 2. The other dive states

A census of `+278h/+27Ch/+2A8h/+2ACh/+2D8h/+2B4h` stores in each tick body, from the disk bytes:

| state | tick | stores | host before this packet |
| --- | --- | --- | --- |
| attackrun | 009C4220 | 009C4413-009C4434: throttle 1.0, air brake 0, active, mode 0 | modelled |
| turndown | 009C44F0 | 009C4512 `+2B4h` (007C47F0), 009C4524 `+2D8h = EBX = 1` | modelled |
| flyabove | 009C62B0 | 009C6FF1 `+2D8h = 1`, 009C6FFB `+2B4h` | modelled |
| aimdive | 009C58D0 | 009C5F37 or 009C605F throttle, 009C6071 air brake, both active, 009C6080 mode 0 | behind `kAimDiveTailBound` (off) |
| aimglide | 009C5180 | 009C5663 throttle, 009C5671 air brake, both active (BL), 009C567F mode 0 | **absent**: aimglide ran in the per-think speed mode |
| goaway | 009C4A40 | section 1 | mode only |

**aimglide**, 009C534B-009C567F:
* `ratio = [ESP+10h] / [ESP+14h]` (FDIV dword, 009C534F).
  * `[ESP+10h]` is the planar distance to the fed aim point (009C51D3-009C52C1). The host keeps
    it as `db_planar_bc`.
  * `[ESP+14h]` is the planar distance to the predicted impact point `approach+D8h/+E0h`
    (009C5207-009C5303). The host keeps it as `db_impact_throw_14`.
* `t = 00419010(0.8 [00CE74F8], MinPowerCtrl row+48h, 1.3 [00CEB4B4], MaxPowerCtrl row+44h,
  ratio)` (009C55EE-009C5616).
* Throttle and air brake are clamped as in goaway.

So the glide adds power when the bomb would fall short (ratio above 1) and cuts it when the bomb
would carry past. It is bound as `dive_bomb_aimglide_throttle_009c55e5` behind
`kAimGlideThrottleBound`.

## 3. Predictions, written before runs C0/G0/T1/T2

All runs are E2 USN04 with `BSP_GUNNERY_RNG_STREAMS=1` on both sides.
* C0 is main: the fix, goaway, aimglide and the tail are all off.
* G0 turns on goaway and aimglide only.
* T1 is the fix plus goaway, aimglide and the tail, with the hull and throttle traces.
* T2 is T1 plus the hull offset.

1. **C0** reproduces S0: 29 releases, and the D3A Val #1.1 dive at 134 m/s released near 300 m.
   The kill figures may differ from S0 under the separate RNG streams.
2. **G0, with the fix off.** Aimglide and goaway now command the throttle slot. In the air, mode
   0's demand arm does not run, so the slot's desired value reaches the live throttle through the
   slew.
   * Aimglide cuts power to MinPowerCtrl 0.2 when the ratio is below 0.8.
   * Goaway cuts it nose-down and restores it once the nose is above -5 degrees.
   * Expect the releases made in aimglide to move a little (slower glide) and the goaway
     climb-outs to change. **Not neutral**: some Val rows move. The drop count stays near 29.
3. **T1.** Flyabove holds 63-66 m/s and turndown 34.5 m/s, and aimdive's tail takes over at about
   50 m/s.
   * The tail commands power between MinPower 0.2 and MaxPower 0.7 from the pitch ratio, with
     brake up to 0.5, so the dive accelerates on gravity alone. Expect an aimdive speed well below
     134 m/s, around 80-110.
   * The aim error should be modest, because the dive starts slow and near the target. If the tail
     then holds a modest pitch command, releases return.
   * My central guess is a partial recovery, about 15-29 releases, at lower speed and higher
     altitude than S0.
   * Val water contacts fall from S1's 19, because goaway gives full throttle once the nose is up.
   * The Yorktown fighters survive if the Vals no longer glide into the sea. They were chasing
     them in aim.
4. **T2.** The hull offset spreads the impacts along the hull. Its release count is at or below
   T1's.

## 4. Results

The runs use `BSP_GUNNERY_RNG_STREAMS=1`. The E2 runs use `--frames 9200 --mission-frames 9000`,
and the reference runs `--frames 4700 --mission-frames 4500`. All other parameters are the E2
ones. Logs are in this tree's `local\`.

| run | log | fix | goaway + aimglide throttle | aimdive tail | hull | bomb drops | kill credits | plane water contacts | mission end |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| C0 | `C0_9000.log` | off | off | off | off | 29 | 35 | 8 | failed 186.06 s |
| G0 | `G0_9000.log` | off | **on** | off | off | 29 | 34 | 8 | failed 186.06 s |
| T1 | `T1_9000.log` | on | on | on | off | **0** | 36 | **0** | failed 342.04 s |
| T2 | `T2_9000.log` | on | on | on | on | 0 | 36 | 2 | failed 342.04 s |
| C0 ref | `C0_4500.log` | off | off | off | off | 17 | 24 | 0 | failed 186.06 s |
| T1 ref | `T1_4500.log` | on | on | on | off | 0 | 16 | 0 | none |

C0 matches S0 on drops and water contacts. The kill figures differ because of the separate
random streams. C0_4500 exits with code 1 because its mission ends at 186 s, and its log is
complete.

**G0 is release-neutral but not row-neutral.** Every drop row matches C0, and the total is 29.
The first moved row is an aimglide exit: D3A Val #3.1 leaves aimglide at tick 1180 and 158.6 m,
where C0 left at tick 1176 and 147.2 m. After that the goaway climb-outs and later passes move,
and the ship AA figures follow those paths (deaths 35 to 34). Every moved row is a post-release
aimglide or goaway path.

**T1, D3A Val #1.1.** The trace is `hull_trace` and `val_trace` in `T1_9000.log`.

| phase | ticks | height above aim | speed | aim error | tail command |
| --- | --- | --- | --- | --- | --- |
| turndown | 1283-1337 | 963 to 844 | 50-54 | none | throttle 0, brake 0.2 (speed hold toward 34.5) |
| aimdive entry | 1339-1363 | 844 to 712 | 54 to 69 | -293 to +252 | power 0.197, brake 0.504 |
| aimdive settles | 1379-1411 | 648 to 545 | 65 | 18 to 85, ccip 39-124 | same |
| aimdive, on aim | 1415-1443 | 532 to 419 | 65 to 77 | -25 to +4 | brake falls to 0.04, power up to 0.59 |
| abort, 009C5B43 | 1444 | 419 | 77 | 1.3 | range 272 < 0.3 × 419 + 150; pitch -0.71 is shallower than -60 degrees |
| aimglide | 1445-1487 | 409 levelling at 330 | 79 to 56 | none | power 0.197 (ratio below 0.8) |

**The re-measured swing.** With a working throttle the dive starts at 54 m/s and 844 m instead
of 113 m/s. The error still swings about 540 m peak to peak on entry, much like the -200 to +224
from before. That part was not an artefact of the frozen throttle. But it settles within about
40 ticks, and from 532 m down the error stays inside the 25 m release window. The tail then
lifts the brake and adds power. So the earlier finding that the tail holds minimum power and
maximum brake "through the dive" rested on the frozen throttle. Here it holds them only until
the aim settles.

**Why nothing releases.** The aimdive release needs the aircraft below the release altitude
`+A8h`. That is 350 in this host, the low end of the authored { 350, 450 }, because every random
draw is pinned low. The abort (009C5B43) fires first, at 419 m, because the slow dive is
shallower than -60 degrees and the range has closed to within 0.3 × height + 150. Aimglide should
then release (009C5777), but its ceiling gate needs the height below 0.6 × 350 + 50 = 260 m. The
host's aimglide binds only its heading. The glide's pitch target 009C5522-009C55DF (cmd+2BCh with
cmd+2D0h = 2) is absent, so the per-think seed leaves a level command, and the Val levels at
330 m and passes over the target. The ceiling gate blocks it on 36 of 43 calls.

**The fighters.** No aircraft reaches the water in T1: both Yorktown fighters and every Val
survive. So the 105.7 m/s water hits in S1T were chasing gliding Vals, as suspected. A powered
goaway removes them. The leader then ends the run in moveto, level at 1547 m and 34 m/s at full
throttle while it asks for 83.3. That is not explained.

**T2** adds nothing to judge: 0 releases. Two Vals reach the water there (D3A Val #1.1|.-2 at
50.9 m/s, movieval|.-2 at 57.4). The hull offset moves their aim point.

**Predictions.**
* 1 and 2 held: C0 reproduced S0, and G0 kept the drops.
* 3 failed on releases (0, where I guessed 15-29) but held on the dive speed regime and on the
  fighters' survival.
* 4 is moot.

## 5. Decision

* **`kGoawayThrottleBound` and `kAimGlideThrottleBound` land ON.** They are image writes the host
  lacked, and with the fix off they are release-neutral (G0). They do move rows, and every moved
  row is a post-release glide or climb-out path. Strictly, they are not neutral. If the lead wants
  row-neutral only, both are one-line flips.
* **`kPilotThrottleSlotBound` stays OFF**, and so do `kAimDiveTailBound` and
  `kHullAimOffsetEnabled`. Releases drop from 29 to 0 at E2 and from 17 to 0 at the reference
  parameters.
* **The term still missing** is the aimglide pitch target, 009C5522-009C55DF. It is max of
  `00419010(x0 [00CEE07C], -1.0, x1 [00D20CE4], min(atan((ceiling - height) / d), class+1ECh), ratio)`
  and a second curve, where ceiling is `[ESP+1Ch]` after 009C5493. It reads the path-dependent
  scratch `[ESP+6Ch]`, which is why it was left unbound. Without it a slow dive that aborts
  high cannot get under the glide's release ceiling.
* **A secondary term** is the release-altitude draw, pinned at 350 of { 350, 450 }. In T1 the
  leader was on aim at 439 m.

## 6. Corrections this packet makes elsewhere

* `docs/HANDOFF_DIVE_BOMB_AIM.md`: the aimglide ratio's divisor is |impact - aircraft|, not
  |aim - impact|.
* `docs/AIMDIVE_RESPONSE.md` section 4(b): the tail's minimum power through the dive rested on
  the frozen throttle.
* `docs/PILOT_THROTTLE_SLOT.md` section 5: aimglide also lacked its throttle writes, not only
  aimdive and goaway.
