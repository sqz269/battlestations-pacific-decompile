# The Yorktown fighters' sea losses, traced: not a roll problem (packet cc9_fighter_roll_trace)

2026-09-25. Ghidra read-only. Names are hypotheses. The question
(docs/TERRAIN_ARM_US_FIGHTERS.md section 6) was whether the terrain arm's roll set, or `0099C129`'s
roll fallback, levels a sinking fighter in the image, and whether the host lacks that roll set.

## 1. The terrain arm has no roll set, in the image or the host

`0099CAB0` inserts its band at `0099D01A`-`0099D025`: `LEA ECX,[EDI*0C8h + EBP + 4]`. `EDI` is the
terrain pass index, 0 or 1, so the band goes to set 0 (`pilot+4h`) or set 1 (`pilot+CCh`). Set 2
(`pilot+194h`) is the one `0099BF30` uses for the roll command, and only the gunfire arm writes it.
`0099C129`'s roll fallback acts only when a roll-set pair covers the whole range. So nothing in the
image's terrain path levels the wings, and the host (`terrain_shape_band_0099cab0`, set = `pass`)
matches.

## 2. The trace

One diagnostic build (`local\rtd`): all five switches as landed, plus `kRateLawAttitudeTermsBound`
ON, plus `kFighterLowTraceDiag` extended with bank, roll and pitch commands and band counts
(committed OFF). Run `local\RTD_9000.log`, E2 9000, `BSP_GUNNERY_RNG_STREAMS=1`, window 1600x900.
Two US depth kills, both Yorktown sqn04.

Yorktown-class01_sqn04|.-3, the last 20 s, in aim throughout:

| t (s) | alt (m) | speed (m/s) | bank | throttle | pitch command | note |
| --- | --- | --- | --- | --- | --- | --- |
| 306.9 | 199 | 87.8 | -0.82 | 1.0 | 1.0 | turning onto the target |
| 310.9 | 202 | 82.8 | 0.17 | 1.0 | 0.02 | level |
| 311.9 | 209 | 79.1 | 0.04 | **0.21** | 0.00 | the speed arm cuts the throttle |
| 314.9 | 223 | 38.7 | 0.09 | 0.0 | 0.00 | slowing with the nose level |
| 317.9 | 229 | 23.7 | 0.00 | 0.0 | 0.25 | stalling |
| 319.9 | 219 | 20.8 | 0.05 | 0.0 | 1.0 | aim's pitch cap 0.78 commanded, full up stick |
| 323.8 | 117 | 35.3 | 0.00 | 0.0 | 1.0 | mushing |
| 327.8 | -1.7 | 29.0 | -0.28 | 0.0 | 1.0 | into the sea |

Yorktown-class01_sqn04|.-2 is the same:
- the throttle is at 0 in aim;
- the speed falls from 55 to 13 m/s by 337 s;
- the aircraft stalls and mushes at 27 m/s with bank about -0.7, to the depth kill at 345 s.

**The wings are close to level in the first loss.** This is not a spiral held by the slide term. It
is a stall at zero throttle: the aim speed arm slows the fighter below flying speed behind its target,
and then the aircraft falls.

## 3. The image's aim speed arm has no minimum-speed guard

- When not head-on, `009A76E0` writes `plan+2B4h = (d - FollowDist) + target vtable[38h]` with
  `+2B0h = 1` and `+2D8h = 1` (docs/DOGFIGHT_ENGAGED.md). The speed hold then drives the throttle
  toward that speed, to 0 when the fighter is faster.
- `+2B0h` switches off the planner's target-speed correction at `0099D924`-`0099D970`. That
  correction is the only speed-shaping term on this path:
  - with `+2B0h` clear, the multiplier `+2B8h` is raised to
    `InterpolateClamped(-TrgSpeedCorrMinPitch -> TrgSpeedCorrSpeedMul, 0 -> 1, pitch)`
    (`tuning+B0h`/`+B4h`), then decays at `TrgSpeedCorrMulDecay` (`tuning+B8h`,
    `0099D75C`-`0099D79A`);
  - the measured forward speed is divided by it (`0099D99E`-`0099D9A3`).
  It is a nose-down correction, not a stall guard, and aim disables it. The host never reads `+2B0h`
  and holds `+2B8h` at 1.0, which equals the image in aim.
- The planner's pitch arm has no speed term (docs/CLIMBOUT_SPEED_GATE.md), and the free-flight law
  has no post-stall recovery (docs/FREEFLIGHT_STALL_LAW.md).

So by the image's own laws, a fighter that closes on a target slower than its own stall-safe speed
cuts its throttle, stalls, and cannot recover at low altitude. The slide term only makes such
closures happen: without it, the fighters never come round onto a target's tail.

## 4. Verdict

- **Nothing is bound.** The host lacks no roll set, and the image has no speed guard here that the
  host misses.
- **`kRateLawAttitudeTermsBound` stays OFF pending the original-game check.** The losses come from
  the image's own laws as read, and the sea-loss gate stays 0-1.
- **The trace output** (bank, roll and pitch commands, band counts) is added to the OFF diagnostic
  `kFighterLowTraceDiag`.
- **Open.** The target's speed at the moment of closure is not logged. The chase diagnostic
  `kFighterChaseTraceDiag` would give it. A slow target is either a Kate on its run-in or a Val in a
  glide, and 72-80 m/s Kates would not explain 13-21 m/s. Which target is being chased is the first
  thing to check if this is taken further.
