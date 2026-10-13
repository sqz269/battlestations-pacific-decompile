# CC13 VER 21: AI ship surface gunnery, image against host (cc13-ver21)

Packet cc13-ver21 was a read-only verification for cc9, done on 2026-10-13. It checks the AI ship
gun chain against ships (main and secondary batteries) against `src/game_hosts_gunnery.cpp` at
origin/main `760c88179`. That file belongs to cc9's gunnery lane, so this packet only read it.

Every image claim below was read from the disk bytes with `bsp.py disasm-raw` and
`tools/pe_const_read.py`, or from the Ghidra export listing where a line names one. No game was
run. The names are hypotheses, not recovered symbols. Host line numbers refer to this tree's
`src/game_hosts_gunnery.cpp`.

Addresses: 00864880 00862CD0 00862F58 00862F62 00862660 00438AA0 006DF520 006DEFF0 006DEE40
006DF833 006DF8BF 006DFAB3 006DFB8C 006DFBD6 006E98BC 006E8430 006E85D2 006E6750 006E6490
0042AC60 00BD2F10

## 1. Scope, and what was already verified

Gunnery has more than eighty earlier packets (`docs/GUNNERY_OPEN_ITEMS.md`, `docs/GUN_*.md`,
`docs/SURFACE_GUNNERY_REFERENCE.md`). This pass re-read the decisive formulas from the bytes and
did not repeat the reads that earlier documents state in full.

| stage | image | result here |
| --- | --- | --- |
| range estimate, correction between salvos | `00864880`, `00862CD0`, `00862660` | **D1, D2** |
| ranging step into the aim | `006DF66A..006DF6D2`, `0042AC60`, double 30.0 at `00CE7630` | matches the host (`ranging_offset_step`, 2135) |
| aim-error envelope | `006DEFF0`, `006DF59F..006DF651` | matches: U(3, 8) period first, then t, span and roll in that order; `t^Power` through FYL2X/F2XM1; `horz = sin(roll) r`, `vert = cos(roll) r`; interpolation old to new over the countdown |
| lead and elevation | `006DF833..006DF97D`, `00955630` at `006DFAB3` | **D3, D5** |
| fire gate | `006DFB54..006DFC61` | matches, except **N1** |
| launch and flight | `006E8430`, `006E6750`, `006E6490` | **D3, D4** |
| dispersion cone | `00730540..0073064A` | not re-derived; `docs/GUN_DISPERSION.md` section 4 holds, and the host's cone (8093..8111) follows it |
| aim point on the hull | `00816650` | not re-derived; `src/gun_bot_remainder.cpp` 144..199 |
| hit test | `00724510` chain | not re-derived; `docs/SURFACE_GUNNERY_REFERENCE.md` 8.2 |
| target choice, reload, stagger | `00864FE0`, `0072D520`, `0072D2C0` | not re-derived. The engagement range is consistent: the host's `gun.max_range` takes `finalised.engagement_range` (4566), which is `Range` for artillery in both |

RNG: the image draws every term in this chain from `00BD2F10` with `ECX = 1`. These are the
ranging error and bearing (`00864A12`, `00864A31`, `00862F46`), the envelope (`006DF5D4`,
`006DF009`, `006DF0B8`, `006DF0DC`), the aim point (`00816883..008168D5`), the fire delay
(`006DFBD6`) and the throw (`00730557`, `00730575`). They interleave with every other gameplay
draw on that stream. The host keys each purpose separately (`Draw::ranging`, `aim_error`,
`aim_point`, `artillery_fire_delay`, `bullet_throw`). None of the adoptions below adds or removes
a draw.

## 2. The divergences

### D1. The ranging bearing never turns after the record is built

- **Image.** At `00862F46`, `00862CD0` draws `U(-AngleChange, AngleChange)` and passes it with the
  old `+6Ch` to `00438AA0` (`00862F58`). `00438AA0` is pure: `FLD [ESP+4]; FADD [ESP+8]`, a wrap
  loop, `RET 8`, with the result in ST0. `00862F62 FSTP ST0` throws the sum away. `00862F80` then
  rebuilds `+74h..+7Ch` from the **old** `+6Ch`. Nothing else in the function writes `+6Ch`. So
  the error bearing is fixed at the `U(0, 2pi)` value that `00864880` drew. Only its length
  changes. The draw is still consumed.
- **Host.** At 2043..2046, `engagement_update_00862cd0` adds the draw to `e.angle` and wraps it,
  so the offset direction random-walks by up to `AngleChange` (15 deg at SPNormal, 20 deg at Stun)
  on every update where the error moved.
- **Effect on hit rate.** The mean miss distance is unchanged, because the length rules agree. The
  difference is correlation. In the image, one record's misses all fall on one side of the target
  (long, short, or one flank) until the error length decays. In the host they wander. If the
  bearing lies along the line of fire, the image's steep shells (D3) miss on every salvo, while a
  bearing across the line of fire misses or hits depending on the hull's length. The expected
  change in the mean is small, under about 5 % (estimated, not measured). The spread per
  engagement grows: some ship pairs never hit until the error decays, and others hit at once.
- **Mission effect.** This applies to every long ship-vs-ship exchange where a record lives for
  minutes (JM08, IJN08, USN13, and USN02's Java Sea fight). Stun-row groups (USN02's DeRuyter
  group) keep a 750 m-capped error on one bearing. No RNG change, since the draw stays.
- **Adoption.** Keep the `draw(Draw::ranging, ...)` call and drop the `e.angle +=` and the
  remainder. Bind it behind a switch such as `kRangingBearingFixedBound`.

### D2. The record is built with a two-second velocity extrapolation

- **Image.** `00864880` copies the target matrix into `+20h` and then, at
  `00864A7F..00864B8A`, does the following with `k = GlobalConfig+88h` (WeaponDirectorThinkTime,
  2 s, `docs/GUNNERY_TABLES.md` section 4) and `V = target->vtable[34h]()` (the world velocity):
  - `+50h..+58h -= V * k`;
  - `+60h..+68h = position + V * k`, then `+64h = 0`.
- **Host.** At 1991..1997, `engagement_00864880` seeds `prev_pos = predicted = position`, with
  `predicted.y = 0`.
- **Consequence.** The order in `00864FE0` is update first (`008650DD`), then hand-out
  (`00865860`). The first update after a build is therefore one pass (2 s) later. In the image it
  sees a motion of `2V + kV = 4V` and predicts `p1 + 4V`. The host sees `2V` and predicts `p1 + 2V`.
  The first shot-arm update (the next pass) then takes `miss = |predicted - p2|`. That is `kV`
  (30 m at 15 m/s) in the image and 0 in the host, for a target on a steady course. The image's
  first correction is therefore larger by `|V| k DeviationMul frac^2`. At SPNormal (0.8) and full
  range that is +24 m, under the cap `min(40, 0.2 D)`. For Stun (10.0) it is +300 m, capped at
  `min(750, 0.5 D)`.
- **Effect on hit rate.** For a moving target, the image's error after its first corrected salvo
  is that much larger. It decays by ApproachMul (0.15..0.25 at SPNormal) per later salvo, so it is
  gone after two or three salvos. Stationary targets are unaffected. So the host is slightly too
  accurate on the second and third salvos against ships under way.
- **Mission effect.** Openings of fights against fast ships: JM08, IJN08, USN13 and USN02.
  There are no draws.
- **Adoption.** Seed `prev_pos = p - V k` and `predicted = p + V k` (y 0) in
  `engagement_00864880`. `artillery_ranging_record_construct_00864880` is the pure form.

### D3. Artillery flies the `sqrt(g * Range)` arc on a sped-up clock; the host flies the authored V0

This is the largest divergence found.

- **Image.**
  - `006E98BC..006E9905` (already reconstructed as `weapon_class_derive_engagement_range`) does
    the following for a class of sub-type 4..7:
    - it **replaces** `+50h` with `v' = sqrt(9.81 * Range)`;
    - it stores `+5Ch = V0 / v'`.
  - Every consumer then uses `v'`:
    - the lead pre-estimate (`006DF876`: `s = D g / v'^2`; pitch `asin(s) / 2`, or pi/4 above 1);
    - the time of flight `D / (v' cos(pitch) * +5Ch)` (`006DF8E4..006DF911`);
    - the arc solve (`006DFAB3` loads `+50h` for `00955630`);
    - the launch velocity (`006E8594`).
  - The flight step scales its dt by `+5Ch` (`006E675A` and `006E649B`: `FLD [class+5Ch]; FMUL
    [ESP+..]`).
  - So the shell traces the `v'` trajectory, whose maximum range is exactly `Range` at 45 deg, and
    it does so `V0 / v'` times faster than real time.
- **Host.**
  - `gun.muzzle_speed` is the authored V0 (4511). The finaliser's `muzzle_speed` and `time_scale`
    are computed and dropped (4560..4574).
  - The lead (7134..7143), the arc (7155) and the launch (8124, 8201) use V0, and
    `run_projectiles` steps with the plain `dt` (9270).
- **Size, in this installation.** `gamemode.lua` sets `GameMode = 0`, so the active table is
  `classtables/arcade/bulletclasses.lua`, which is locally modified (mtime 2026-05-09). Its
  Artillery rows author V0 = 300 with Range 1500..3300 m, giving v' = 121..180 m/s and a time scale
  of 1.67..2.47. The full table is `local/ver21_arty_arcade.txt` from `local/ver21_arty_table.py`.

  | Range | D | pitch image / host | flight time image / host |
  | --- | --- | --- | --- |
  | 1500 | 750 | 15.0 / 2.3 deg | 2.6 / 2.5 s |
  | 1500 | 1350 | 32.1 / 4.2 deg | 5.3 / 4.5 s |
  | 3000 | 1500 | 15.0 / 4.7 deg | 5.2 / 5.0 s |
  | 3000 | 2700 | 32.1 / 8.6 deg | 10.6 / 9.1 s |

  The image's shells plunge at 15..32 deg; the host's skim in at 2..9 deg. The flight time is
  4..17 % longer in the image, because `v' cos(pitch) * k = V0 cos(pitch)`.
- **Effect on hit rate.** The shallow host trajectory has a danger space of `freeboard /
  tan(descent)`, about 50..200 m for an 8 m freeboard at 9..2 deg. So an aim point long by tens of metres
  still strikes the hull side. The image's plunging shells have 13..30 m. Range errors from the
  ranging offset (D1, D2), the hull-box aim point and the vertical error envelope are therefore
  absorbed by the host and not by the image.
  - A crude Monte Carlo is in `local/ver21_hit_mc.py` and `local/ver21_hit_mc.txt`. It uses a
    broadside box of 120 x 15 x 8 m, aim points drawn like `00816650`'s box, and a ranging error
    `e` at a uniform bearing. Per shell:
    - `e` = 10..20 m: host hit fraction 0.90..0.99, image (D3 alone) 0.52..0.97;
    - `e` = 40 m: host 0.58..0.84, image 0.18..0.33.
  - With D4 as well, the image falls to 0.19..0.76 even at `e` = 0.
  - The model has no superstructure or deck, so it overstates the image's loss: plunging shells
    hit decks and upperworks that the box omits. Read the direction and rough size from it, not
    the numbers.
- **Mission effect.** Every AI ship gun of sub-type 4..7 (the artillery categories 2, 3, 4 and 6,
  and the turrets on JM08, IJN08, USN13 and USN02). Predicted on adoption:
  - fewer hull hits per shell, perhaps a third to two thirds fewer;
  - later sinkings, and fewer sinkings in time-boxed missions;
  - longer flight times, so more lead error against turning ships.
  - Any objective that waits on a sinking moves. Hit-dependent damage draws (fires, part damage)
    shift every later stream-0 draw, so treat a pair as RNG-coupled downstream.
- **Adoption.** These pieces go together:
  - carry `finalised.muzzle_speed` and `finalised.time_scale` into the gun row for sub-types
    4..7, as a new field so the authored V0 stays readable;
  - use them in the lead (7134..7143, with `time_scale` in the divisor) and in the arc query
    (7155);
  - launch at `v'` (8124);
  - scale the shot's step and `flight_time` by `time_scale` in `run_projectiles` (9270). The
    projectile's FlyTime expiry then runs on scaled time, as `+1C4h` does.

  Other users of `gun.muzzle_speed` need checking before a switch flips: the seat artillery path
  at 12304 and the bullet-row copy at 4635/5835. `artillery_lead_push_out_006df833` and
  `projectile_scaled_step_006e675a` are the pure forms.

### D4. The artillery launch velocity loses `0.05 * 9.81` m/s of vertical speed

- **Image.** At `006E85C1..006E85E4`, `006E8430` subtracts `[00D0DE84] * [00CF9058]`
  (0.05 x 9.81 = 0.4905 m/s) from vy for class sub-types 4..7. The aim (`00955630`) does not
  allow for it. In the class frame, the flight is shortened by `2 * 0.4905 / g` = 0.1 s, so the
  shell lands `0.1 v' cos(pitch)` short: 10..17 m for this installation's arcade rows.
- **Host.** At 8201..8202, `projectile_launch_velocity_006e8430(launch_speed, dir, 0)` passes
  sub-type 0, so the reconstruction's own bias branch (`src/projectile_impact.cpp` 44..53) never
  runs.
- **Effect on hit rate.** On a plunging shell, the shell lands 10..17 m short, against a beam of
  about 15 m. In the model above this alone halves the image's hit fraction near its own maximum
  range. On the host's flat V0 = 300 arc, the same bias would land about 30 m short at a 2..9 deg
  descent, which strikes the hull lower or falls in the water. **Adopt D4 only with D3.**
- **Mission effect.** The same missions as D3, with the same magnitude band. No draws.
- **Adoption.** Pass the class's refined sub-type (`gun.bullet_sub_type`) at 8202 under the same
  switch as D3.

### D5. The lead push-out has no vertical term

- **Image.** At `006DF927`, `006DF92D` and `006DF92F`, `0.0 (00D7A258) * t` is stored for the Y of
  the push-out. A target's vertical speed is never led.
- **Host.** At 7143, `lead[i] = at[i] + v[i] * flight` for all three axes.
- **Effect.** For a ship, vy is heave and settling, a few tenths of a metre per second, so the
  vertical aim moves by a metre or two over a 3..10 s flight. This is negligible for hit rate,
  although a sinking or capsizing target (vy of order 1 m/s) is led downward by up to about
  10 m in the host.
- **Adoption.** Zero `v[1]` in the push-out. This is a cosmetic fidelity change with no draws.

### N1. The fire gate's tolerance is inclusive

`006DEE40` returns true when `|wrap(current - desired)| <= 0.1 deg` (`006DEE70 FCOMIP`, then
`JB` to false). The host's settle test at 7521..7525 uses `<`. Equality of two float angles at
exactly 0.0017453294 rad is a measure-zero event, so there is no effect, and no change is
proposed.

## 3. Pure fragments

The fragments are in `include/bsp/ship_gunnery_ver_cc13.hpp` and
`src/ship_gunnery_ver_cc13.cpp`, registered in `cmake/startup.cmake`. Each takes its draws as
arguments, and none is wired into a host. Status: reconstructed and build-tested; not
ABI-compatible, fixture-tested or game-validated.

| function | image |
| --- | --- |
| `artillery_ranging_record_construct_00864880` | `00864880` (D2) |
| `artillery_ranging_record_update_error_00862cd0` | `00862CD0` error arm `00862D44..00862FA7` (D1), without the extrapolation tail |
| `artillery_ranging_offset` | `0085C3F0` about +Y. LABELLED: the rotation sense was not re-read; the bearing is uniform, so it cannot change a distribution |
| `artillery_lead_push_out_006df833` | `006DF833..006DF97D` (D3, D5) |
| `artillery_launch_vertical_bias_006e85d2` | `006E85C1..006E85E4` (D4) |
| `projectile_scaled_step_006e675a` | `006E675A`, `006E649B` (D3) |
| `artillery_launch_bias_shortfall` | derived, not an image routine |

There are fragment ledger records for `00864880`, `00862CD0`, `006DF833`, `006E85D2` and
`006E675A`. The evidence of `00862CD0` and `00864880` was appended to their names, and their
Ghidra plate comments were refreshed through `tools/ghidra_annotate.py --apply`; the prior values
are in `local/ghidra-annotations-20261013T154842Z.json`.

## 4. Open

- Check whether any host consumer of `gun.muzzle_speed` besides the AI artillery path (the player
  seat's artillery, the AI target weight's accuracy terms) should see `v'`.
- `0085C3F0`'s rotation sense (only matters if a later packet correlates the D1 bearing with the
  hull).
- A real hull-mesh estimate of D3 and D4 needs an ON pair on USN02 and JM08. Only cc9 should run it.
