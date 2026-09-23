# The AI ships' torpedo response (packet cc9_ship_torpedo_response)

Packet `cc9_ship_torpedo_response`, 2026-09-23. Offsets are relative to blk = brain+8h unless
written as brain+N or record+N, and names are hypotheses. The reconstruction is
`src/ship_ai_torpedo_response.cpp`. The host binding is `kShipTorpedoResponseBound` in
`src/game_hosts_ship_ai.cpp`, landed **ON** after the pairs in section 5. The secondary
`kShipTurnRadiusSitesBound` also landed ON. docs/TORPEDO_EVASION.md
explains why none of this applies to the player-controlled Lexington.

## 1. What the image does

### Detection: the brain pre-pass, 009F158A..009F1855

- **The timer.** brain+0B48h counts down against the period brain+0B44h with 009F15CC's rule: a
  step at least as large as the countdown makes it due and adds one period minus the step. The
  brain constructor 009F1160 seeds both on stream 1:
  - B48h = -U(0, 1) at 009F1316..009F1330;
  - B44h = U(settings+1ECh, settings+1F0h) = U(1.5, 2.0) at 009F139E..009F13BE, from this
    installation's shipglobals.lua line 279, `TorpedoAvoidance.CollectTimer = { 1.5, 2 }`.
- **The walk.** The walk covers the world torpedo list (count world+21Ch, head +220h; 00856360
  registers each MTorpedo). A candidate needs:
  - `slot38` non-zero and `slot2C` non-zero;
  - its dead byte +5Dh clear;
  - its shooter +4F8h not being this ship.
- **The admission test** (docs/SHIP_AI_BRAIN_PREPASS_SCHEDULE.md). Take the relative planar
  velocity (torpedo minus own forward axis times 0092D730) and d = own position minus torpedo
  position. The ship admits the torpedo when |d|^2 < (0.6 * length)^2. Failing that, it admits
  it when (|v_rel| * H)^2 > |d|^2 and d . v_rel > 0. The horizon is
  H = TorpedoPredict[2] + TorpedoObservation[2] + (B44h + 3.0).

### Admission: 009F0AD0 (body 009F0AD0-009F0D1C, whole pseudocode read)

- With 80h tracks already held, nothing happens.
- A track whose +48h is this torpedo only gets its lifetime refreshed: +14h = settings+1F0h + 1.0
  (00D7A210, a double 1.0).
- Otherwise it makes three stream-1 draws from the NavigatorBot row the ship's skill selects
  (config at F8A688, header 0Ch, stride 24h; row = unit+390h):
  - predict = U(+10h, +14h), times length / +18h when the hull is longer than +18h;
  - observation = U(+1Ch, +20h), plus +24h when record+49Ch is set (a submarine launch);
  - spd_err = U(+28h, +2Ch).
- An own-side torpedo (entity+54h equal) gets observation = 0, and predict = 25.0 when it drew
  25.0 or less (00CE3880, 00CE89CC).
- It then allocates 68h and calls 009EACA0(5.0, 12.0, (spd_err + [[record+314h]+E4h]) * predict,
  torpedo, observation, spd_err, 1). The class +E4h is WaterTravelSpeed.

The robots.lua comments (this installation, mtime 2025-06-01, lines 490-549) state the intent:
- TorpedoPredict is how many seconds before impact the ship notices the torpedo and starts to
  evade;
- TorpedoObservation limits evasion to torpedoes that have already existed that long;
- TorpedoSpdErr is how many m/s the ship misjudges the torpedo's speed by.

| row | predict | ref length | observation | sub addon | spd err |
| --- | --- | --- | --- | --- | --- |
| 0 Stun | 4.0-5.0 | 150 | 3.5-5.0 | 1.0 | -0.0-1.0 |
| 1 SPNormal | 3.5-5.0 | 100 | 3.5-5.0 | 4.0 | -3.0-1.0 |
| 2 SPVeteran | 20-22 | 150 | 0.0-1.0 | 0.0 | -0.1-0.1 |
| 3 MPNormal | 6.0-8.0 | 150 | 1.0-3.0 | 3.0 | -2.0-1.0 |
| 4 MPVeteran | 8.0-10.0 | 150 | 0.5-2.0 | 1.5 | -1.5-1.0 |
| 5 Elite | 20-22 | 150 | 0.0-1.0 | 0.0 | -0.1-0.1 |

Row indices are robot_config's (`src/robot_config.cpp:773`). The host's skill defaults to 1
(docs/GAME_DIFFICULTY.md).

### The track: 009EACA0 (body 009EACA0-009EADDF, whole pseudocode read)

| offset | value |
| --- | --- |
| +0h | 5.0 (the half length 009E04E0 uses) |
| +4h | 12.0 * 0.5 (00D7A280) |
| +8h | 12.0 |
| +0Ch | (spd_err + WaterTravelSpeed) * predict, the horizon cap |
| +10h | observation |
| +14h | settings+1F0h + 1.0 = 3.0 for kind 1; kind 0, a ship contact, uses +194h |
| +1Ch | spd_err |
| +34h..+48h | an observer on the torpedo (+48h); +4Ch..+60h the ship observer (unused for kind 1) |
| +64h | 0 |

It ends with 009DC060.

### The refresh: 009DC060 (body 009DC060-009DC2DD, whole pseudocode read)

- **Torpedo arm.** While +48h is live it sets:
  - +2Ch/+30h = the entity's world X/Z (+FCh/+104h);
  - +24h/+28h = the vtable[34h] velocity's X/Z, normalized by 00414C60;
  - +18h = that length + spd_err;
  - +20h = record+46Ch, the commanded heading (docs/TORPEDO_TICK.md).
- **Ship arm.** The +60h arm serves ship contacts.
- **Neither observer live.** The routine sets +64h and returns false.

### The consumers, already reconstructed

- **009E04E0**, `ship_ai_build_throttle_profile_009e04e0`. Each step it counts +14h down,
  destroys dead or expired tracks, and skips a track while record+488h (the run timer 0085748A
  advances) is below +10h. Behind 009DA1D0 it refreshes the track and builds the 65-bin throttle
  profile and the avoidance vector blk+34Ch/+350h. Destroying a track moves the last one into
  the hole (009E1057) and drops the count.
- **009DA1D0**, the gate (docs/TORPEDO_EVASION.md). It refuses a torpedo boat, a unit deeper
  than -15, blk+3ECh = 0, and the director's torpedoAvoidance +240h = 0.
- **009DE5B0 section 5**, 009DE8F1..009DE96C (docs/SHIP_AI_ARM_FINAL_STEP.md). After the early
  out |0092D730| <= 1.0, if the gate is open, blk+354h < 0 and |vector|^2 > 1e-4, then blk+324h
  becomes the vector's heading, plus pi when astern.

## 2. The host binding (switch ON builds)

| stage | host |
| --- | --- |
| world torpedo list | `GameGunneryHost::live_torpedoes()`: alive rounds of a class with WaterTravelSpeed > 0. Each projectile row now carries a run-unique `serial` (the track's +48h identity) and `swim_seconds`, which advances by dt each swim step (record+488h) |
| slot38 / slot2C / +5Dh | alive. SUBSTITUTION: a round in the air passes too, as slot2C's `2` answer does above the water |
| record+46Ch | atan2(vx, vz) of the velocity. SUBSTITUTION: host rounds run straight on their launch heading |
| record+49Ch | false: no host submarine launches underwater |
| unit+390h | `units.skill_level`, default 1 |
| the timer seeds | two draws on the ship's first re-plan, not seven at construction. SUBSTITUTION |
| 009DE5B0 | section 5 only, after the 009DE5DE early out, from the arm tail's 009EF213 hook and the station arm's 009EE57B exit. Sections 3, 4, 6 and 7 stay records, so blk+354h is never raised to 3.0 by section 4 |
| cruise arm 2's request for the controlled unit | applied in the ship AI host whatever the director holds (see below) |

**A host gap the first treatment exposed.** 009F3DF3 forces the controlled unit into `cruise`,
and 009E1170's arm 2 stores blk+3ECh = 0 at 009E11D6 without testing the director's command. The
commands host's `cruise_step` returns before that store when the director holds another command.
Once the Lexington took its scripted `attackmove` at 125.80 s, blk+3ECh stayed at the pre-pass 1.
The first treatment build (`trT`) therefore opened 009DA1D0 for the player's ship 682 times. Its
torpedo tracks then drove its throttle profile, and the idle Lexington steamed at throttle 1.000
and later astern at -0.562. The ship AI host now applies arm 2's request itself, behind the same
switch, and the second treatment (`trT2`) keeps the Lexington's gate shut.

The per-ship log line reads:

```
torpedo <unit> scans admits tracks_built tracks_max gate_open vector_steps overrides max_turn first
```

## 3. Random streams

The three admission draws and the two timer seeds are stream-1 draws (00BD2F10, ECX = 1). They
now go through `GameGunneryHost::ship_ai_draw`:
- by default, the one shared generator every gunnery draw uses, in call order, which is the
  image's coupling shape;
- with `BSP_GUNNERY_RNG_STREAMS=1`, a generator keyed by (`ship_ai_torpedo` = 7, unit), so a
  treatment's extra draws leave every gunnery key's sequence where it was.

See docs/RANDOM_STREAMS.md section 8.

## 4. Predictions, written before any run

The pairs are control `build/win32/trC` (both new switches off) against treatment
`build/win32/trT` (torpedo response on), same tree, `BSP_GUNNERY_RNG_STREAMS=1` on both.

**USN04 4700/4500** (the control numbers are from local/sk_ctl_usn04.log):
1. **Yorktown-class01.** It is the only AI ship a Kate aims at. Kate #4's four rounds pass it
   at 18-28 m after runs of 5.2-5.75 s. Kate #8's four pass at 166-167 m after runs of
   11.9-20.1 s. Expected:
   - tracks are built for all eight;
   - Kate #4's tracks are consumed only in their last 0.5-2 s, because observation is
     U(3.5, 5.0) s at skill 1, so its hits are unchanged;
   - Kate #8's tracks are consumed for several seconds, so the Yorktown gets overrides and a
     heading deviation of tens of degrees;
   - its damage taken (4599) is unchanged or lower by at most one torpedo.
2. **Fletcher-class04 and the escorts near the Lexington.** Kate #2.1's lead round passes
   Fletcher-class04 at 28.3 m after a 4.1 s run, so a track may form but is barely consumed. An
   escort under 1 m/s never overrides (009DE5DE).
3. **Flat rows:**
   - the Lexington: its gate is shut and it is stationary;
   - every aircraft row until the Yorktown's track moves;
   - the unimplemented census, apart from the newly concrete torpedo rows.
4. **Deaths.** The control has none for ships and 19 in total. No ship death is expected to
   appear or vanish.

**E2 9200/9000** (the control numbers are from the dogfight worker's DC_9000.log):
5. **Fletcher-class01** dies at 255.41 s to Kate #6.1|.-3's round, one of three rounds that
   passed the Lexington and ran about 28 s. That round is observed well before impact, so
   Fletcher-class01 should evade. Expected: it survives, or dies later to a different round.
6. **The Lexington** still dies at 225.81 s (its gate is shut).
7. The Yorktown behaves as in USN04 until about 225 s.

**Turn-radius sites** (`kShipTurnRadiusSitesBound`, treatment `build/win32/rsT` against `trC`):
8. The follow station latch and back-off radius double, so followers leave their latch less
   often. Path-follower corners use twice the radius, and the arm tail's astern threshold widens
   from 0. Expected on USN04:
   - some navigate-mode rows move;
   - no ship death appears or vanishes;
   - the Lexington stays flat.

## 5. Runs (2026-09-23, from 13:01, when a 120-frame probe passed)

All pairs used `BSP_GUNNERY_RNG_STREAMS=1` on both sides. Controls `trC2` and treatments `trT2` /
`rsT2` are built from one tree, after the controlled-unit fix above.

**Torpedo response, USN04 4500** (`local\tr2_ctl_usn04.log` / `local\tr2_trt_usn04.log`):

| row | control | treatment |
| --- | --- | --- |
| Yorktown tracks built / overrides / first / largest turn | - | 8 / 179 / 124.65 s / 1.426 rad |
| Yorktown damage taken | 4599 | 4599 |
| Yorktown AA shots / dealt | 525 / 1158 | 424 / 1245 |
| Fletcher-class01 vector steps / overrides | - | 66 / 0 (under 1 m/s) |
| Lexington damage taken / moved | 7858 / 100.51 m | 7858 / 100.51 m |
| deaths / hits / damage | 18 / 220 / 16504.6 | 20 / 254 / 16974.2 |
| unimplemented calls | 3448445 | 3435047 |

18 ships build tracks, and every track is consumed behind the gate. Only the Yorktown and
Fletcher-class01 ever leave a non-zero vector. Against the predictions:

1. **Yorktown: met.** The eight tracks form, the overrides start while Kate #4's rounds run, and
   the damage it takes is unchanged. The largest heading change is 82 degrees.
2. **Escorts near the Lexington: met.** Fletcher-class01 builds a vector but cannot override
   below 1 m/s.
3. **Flat rows: met for the Lexington.** The aircraft rows move after 124 s, as the Yorktown's
   turn moves its AA: two more aircraft die (`D3A Val #3.1|.-2`, `B5N Kate #8.1`) and
   `D3A Val #3.1` dies 15 s later.
4. **Ship deaths: met.** None appears or vanishes.

**Torpedo response, E2 9000** (`local\tr2_ctl_e2.log` / `local\tr2_trt_e2.log`):

5. **Fletcher-class01: not testable.** The control no longer loses it. Station keeping, landed
   since the DC_9000 reference, keeps the escorts in their latches, and Fletcher-class01 takes
   0 damage in both runs.
6. **Lexington: met.** It dies at 225.81 s on both sides, to B5N Kate #6.1.
7. **Yorktown: met.** Its rows repeat USN04's: 179 overrides, the first at 124.65 s, and 4599
   damage taken. Deaths go from 30 to 31 and damage from 19256.2 to 19594.2, all aircraft. The
   unimplemented calls go from 6944285 to 6908333.

**Turn-radius sites, USN04 4500** (`local\tr2_ctl_usn04.log` / `local\rs2_trt_usn04.log`):

8. **Mostly met.**
   - Fletcher-class03's arm runs rise from 2866 to 3876 and Fletcher-class04's from 1702 to 3876.
     They now stay in their station latch.
   - Plan requests fall from 48569 to 45391, arrival latches go from 0 to 3, and stops from 7 to 9.
   - Deaths stay 18, with the same aircraft at moved times.
   - The Lexington does not move (100.51 m), but its AA does: shots 283 to 289, hits 26 to 30.
     That part of the "Lexington flat" prediction was wrong.
   - The unimplemented calls go from 3448445 to 3395630.

**Decision.** Both switches land ON:
- The torpedo response matches the read at every stage the rows can show.
- The turn-radius sites give the image's value at sites that were using half of it.

**Torpedo-boat exemption** (`tbC` against `trC`, the tree before the controlled-unit fix): every
simulation line is identical, and only pointer values in environment lines differ. This matches
the one-line prediction; neither mission has a torpedo boat.

## 6. no_ghidra_function

None. Every address above has a Ghidra function.

**Default runs (option off).** With both switches on, the USN04 4500 reference loses the
Lexington at 182.86 s to a dive-bomb hit. Main's own reference leaves it at 15 of 8000, and each
switch alone leaves it at 66 or 84. This is a shared-generator coupling flip on a knife edge, not
a torpedo-response behaviour. docs/GAME_EXECUTABLE.md, "Mission reference baselines, 2026-09-23
(after station keeping, and with the torpedo response)", has the rows.

**Update, packet cc9_heading_target_sections.** `kShipTorpedoResponseImageTerms` (on) replaces the
liveness and timer-seed substitutions of section 2 with the image's terms. Once only swimming
rounds count, the heading row becomes an equivalence. docs/HEADING_TARGET_SECTIONS.md reads
009DE5B0's sections 3, 4, 6 and 7, which are inert in this host behind 009ECA20, 009F0D20 and the
unread 009DC2E0.
