# CC13 VER 22: AI fighter air-to-air combat, image against host (cc13-ver22)

Packet cc13-ver22 was a read-only verification for cc9, done on 2026-10-13. It extends
`docs/CC13_VER_14.md`, which covered the Japanese CAP that the host loses in USN04 and the ship AA machine
gun in USN21. Neither is repeated here.

- **Method.**
  - Image bytes were read with `bsp.py disasm-raw` (Capstone on the disk image). The listings are
    `local/output/disasm-raw-009a6a60-length-0x4900-*.txt` for the dogfight band 009A6A60-009AB360.
  - Floats and doubles were read from the PE on disk with `local/v22_consts.py`.
  - The DC-form x87 sites were checked with `tools/x87_dc_form_scan.py`. Only two exist in the code read:
    `009AA7E4 DC C9 FMUL ST(1),ST` and `009FCC82 DC C0 FADD ST,ST`. Both are transcribed as the bytes say.
  - The evidence is existing logs only. No game runs were made.
- **Host files.** `src/game_hosts_units.cpp` and `src/game_hosts_gunnery.cpp` belong to cc9 and were
  read only. Every host change below is an adoption note.
- **Status.**
  - Reconstructed and build-tested: `src/dogfight_ver_cc13.cpp` and `include/bsp/dogfight_ver_cc13.hpp`.
    These are two pure fragments with ledger records `009A797F` and `009A744E`.
  - Nothing here is game-validated.
  - The aim point's lead time is already `dogfight_latch_009aac70`'s `time_to_target`
    (`src/dogfight_task.cpp:102`). The lead itself is already `approach_target_ref_lead_tail_009faf05`
    (`include/bsp/approach_target_ref.hpp:236`). Neither was rewritten.

## 1. Where the host's fighters fight

There is no dogfight in the newest host logs for three of the four missions named:

| mission | evidence | fighter air-to-air |
| --- | --- | --- |
| USN04 | `docs/GUNNERY_OPEN_ITEMS.md` 197: "No dogfight task runs on USN04 4500, E2"; `docs/CC13_VER_14.md` 1: the Zuiho CAP crashes after lift-off | none until the climb-out is fixed |
| USN21 | `docs/CC13_VER_14.md` 0: no Japanese fighters in `usn_sibuyan.lua` | none |
| PRCP01 | `cc9\local\lead_n5b_off_PRCP01.log`: 0 `dogfight` lines, 0 `killer_cat=0` death rows | none |
| JM08 | `cc9\local\lead_n5_off_JM08.log`: `MainAirFieldEntity 01_sqn50`/`sqn51` (six Japanese fighters) take `B-17 #2.x` targets | aim 4377 ticks (`planner mode census state=dogfight:aim`); 49 bursts; B-17s die to `killer_cat=0` from 2460 s |

USNOS (`Enterprise_sqn01` against `plane #1.5`, `cc9-*\local\g40_bm*_os.log`) also dogfights. So every
divergence below acts on JM08, on USNOS, and on USN04 once its CAP flies.

## 2. Verified equal (bytes)

| term | image | host |
| --- | --- | --- |
| target score | `009AA7C7`-`009AA877`: R = row+230h x 0.8 (double `00CE3D40`). Inside R the score is interp(100 `00CE3D08`, 0.5 `00CE3800`, 0.8R, 1.0, d). Outside it is interp(1.2R `00CEC160`, 1.0, 3.0R `00D7A2B0`, 0.25 `00CE3868`, d). A target behind (local z < 0) scores x 0.1 (`00D7A2F0`) | `src/dogfight_task.cpp:60`-`:69`, `:83` |
| angle score | `009AA884`-`009AA919`: z' = max(1, z); interp(0.25, 1.0, 1.5 `00CE380C`, 0.2 `00CE54A0`, \|(x/z', y/z')\|) | `:71`-`:79` |
| wingmate share | `009AA928`-`009AA95E`: x 0.7 (`00CEFFA0`) per own-squadron member whose pilot target (`007BBC10`) is the candidate | `:86`-`:88`; `007BBC10` is stood in by the wingmate's dogfight target (`src/game_hosts_units.cpp:20123`, labelled) |
| pick | strictly greater than the best so far (`009AA998` FCOMIP / JBE). With no positive score it falls back to member 0 (`009AA9BA`). A squadron of 0 or 1 takes member 0 at once (`009AA66B`) | `:20100`-`:20137` |
| reselect | `009AACE9`-`009AAD0E`: a dead or missing target, or else d > row+230h + 200 (`00CE4D70`) with the timer below 0 | `src/dogfight_task.cpp:93`-`:100` |
| latch | `009AADB1`-`009AAE17`: Dogfight/AttackDist (tuning+644h) x approach+24h + 150 (`00CE3808`) while latched > d | `:102`-`:119`, but see D3 |
| on target | `009A9BD0`: z > 1 and \|tan\| < 0.8 (double `00CE3D40`) | `:203`-`:208` |
| aim tick | `009A76E0` reads as follows: <ul><li>head-on is z > 1 with opposing vt[34h];</li><li>too-close is d < +20h;</li><li>boredom is -4 (`00CF1430`) while +48h, else interp(0.25, -1, 2, 2, \|tan\|) inside ShootDistance;</li><li>speed is (d - row+20Ch) + target vt[38h]();</li><li>the head-on throttle is interp(+20h, 0.3 `00CE69C8`, ShootDistance, 1, d).</li></ul> | `:218`-`:239` |
| gun envelope | `009FCC35`-`009FCC9E`: lateral < d x +38h, 1 < z < +34h, then lateral > +3Ch (/1.8 `00D049A8` with an auto target) or lateral > 2 x +0Ch x +08h x d. With +3Ch = 0, the area arm alone is lateral > 0 | `:288`-`:293`, the same answer |
| gun range | max(+30h, +34h + 200) > d > 1 | `:280`-`:282` |
| burst clock | `009FCE4F`-`009FCED9` | `:303`-`:312`, the draws keyed under `kDogfightGunBurstDrawBound` |
| shot spread | `00730348`-`00730498`: for a plane owner with a category-0 gun, the throw x the pilot bot's BulletThrowMul (role 1, unit+DF4h vt[24h]) | `src/game_hosts_gunnery.cpp:3516`-`:3517`, `:3538` |
| hit test, damage | the plane mesh test and the per-hit chain, already verified in `docs/AA_LETHALITY_AUDIT.md` 9-10 and `docs/CC13_AUDIT_MG_AA_LETHALITY.md` 2. Category-0 rounds use the same projectile path (`docs/FIGHTER_GUNFIRE_CONSUMER.md` 1) | `kPlaneMeshHitTestBound`, `kPlaneBlastElementEntriesBound` ON |

## 3. Divergences

### D1: the dogfight aim point has no lead (lead pursuit)

**Image.**

- `009AAE25`-`009AAE35` stores the latch's time-to-target in approach+70h, then calls
  `009FADA0` on the target reference at approach+2Ch (`LEA ECX,[ESI+2Ch]`). So approach+70h is that
  sub-object's +44h, the projtime.
- The time-to-target is computed at `009AADDE`-`009AAE17`: d / `007C2610`() (the slowest fixed-gun muzzle
  speed), clamped to [0, 30] (`00CE7630`, `00CE38C8`). It is 0 while unlatched (`009AAE1B`-`009AAE22`).
- `009FAF05`-`009FAF7D` then adds `target->vt[48h](projtime) - target origin` to the aim point
  approach+48h. For a plane, vt[48h] is `00954650`.
- That aim point (`009A6B70` returns +48h) feeds:
  - the aim state's heading and pitch (`009A76E0` `009F9E40`/`009F9ED0`);
  - d (+D4h), with the reselect, latch, too-close, boredom, speed and head-on throttle behind it;
  - the local frame +ECh and its tangents +F8h/+FCh, used by the on-target test and the boredom rate.

**Host.**

- `src/game_hosts_units.cpp:20055`-`:20058` takes the target's origin. The comment calls this "the
  009FADA0 target reference's world point". That is true only for projtime 0.
- `:20067`-`:20069` passes `min_muzzle_speed = 1.0` to the latch and discards `time_to_target`.

**Effect on air-to-air kill rates.**

- The image steers the aim state at the point where the target will be when a round arrives. The lead
  angle is about v_perp / v_muzzle, roughly 0.1 rad for a 90 m/s crossing target.
- That is larger than the gun envelope's lateral cap (+38h = 0.09 for SPNormal). So the host's nose sits
  outside the envelope for a crossing target until the fine aim (`009F9FC0`, 1 - cos < Angle_Strafe,
  about 42 degrees) pulls it in.
- In a tail chase the image's d is d(1 + v_t / v_muzzle). Its speed command is therefore higher, so the
  image closes harder. Its too-close and boredom distances also trip earlier.
- Prediction: the image has more fire ticks per aim second and more category-0 hits. The host
  under-kills, mainly on crossing and deflection passes.
  - JM08: more B-17 losses to the Japanese fighters, and earlier ones.
  - USNOS: more `plane #1.5` losses to `Enterprise_sqn01`.
  - USN04, USN21, PRCP01: none today (section 1).

**RNG.** The latch, projtime and vt[48h] use no draw. The re-arm draw is D5.

**Adoption.**

- Behind a new OFF switch, pass `min_fixed_gun_muzzle_speed_007c2610` to the latch at `:20067`.
- Then set `df_aim` = origin + (`df_predict_00954650`(target, projtime) - origin), with the projtime of the
  previous tick. The image's +48h is built after this tick's d (`009AAD34` reads +48h before `009AAE35`
  rewrites it), so d lags the point by one tick.
- The pieces exist already: `df_predict_00954650`, `approach_target_ref_lead_tail_009faf05`.

### D2: the aim state adopts the gun's target (`009A797F`-`009A79A9`)

**Image.** At the end of every aim tick, the gun controller's auto target (gun+74h, left by the previous
gun tick's `007E2090` search) replaces the dogfight target through `009A7650` when all of these hold:

- it is not null;
- it is not the current target;
- it has a squadron;
- that squadron is the one the task was ordered against (approach+CCh).

`009A7650` also re-binds the target reference and the opportunistic rocket target. Reconstructed as
`dogfight_aim_adopts_gun_target_009a797f`.

**Host.**

- The aim branch `src/game_hosts_units.cpp:21596`-`:21666` ends without this.
- The only other retargets are the reselect (`:20146`) and the early edge (`:21292`), which is asked only
  off aim (`:21474`).
- So in the host an aim-state fighter keeps steering at its picked target. Meanwhile its gun
  (`kPlaneFinderBound`, `:21033`-`:21036`) fires at whatever the finder put in the cone.

**Effect.**

- In the image, a fighter in aim switches to the bomber its gun is actually tracking. Steering and the
  gun then agree, and fire concentrates on that one.
- In the host, steering and gun can disagree for whole aim periods, which wastes fire ticks.
- Only squadrons of two or more targets are affected. In JM08 that is each B-17 flight; in USNOS it is the
  `plane #1.5` flight.
- Prediction: more hits per burst and quicker B-17 kills in the image; the host under-kills.
- It also changes the reselect cadence. A target that is adopted while within row+230h + 200 is kept
  (`009AACE9`).

**RNG.** None: `009A7650` draws nothing. Its `009FA350` only re-arms the pick, which for a plane is the
origin (`0042D810`).

**Adoption.**

- Behind an OFF switch at the end of the aim branch, call `dogfight_aim_adopts_gun_target_009a797f` with
  last tick's finder result (`nb_found_plus_one` or the gun's +74h stand-in).
- On true, run the `009A7650` effects as `df_select_009aa630` does at `:20146`-`:20155`.

### D3: approach+24h is pinned at 1.0

**Image.**

- `009F9CE0` stores max(1, class+188h MaxSpd / Pilot/Dogfight/ReferenceSpeed) at approach+24h
  (`009A6C10` passes tuning+650h, which is KMH(300) = 83.33 m/s, `planeglobals.lua:591`).
- It multiplies three things:
  - the latch range (`009AADC5`);
  - the aim state's too-close distance (`009A7605`);
  - the maneuver-expiry pursuit range (`009A87C4`).

**Host.** All three use 1.0:

- `src/game_hosts_units.cpp:20064`-`:20069`;
- `:21531`-`:21532`;
- `:20226`-`:20228`.

The strafe and level-bomb bindings already compute the ratio from `plane_max_spd` (`:21920`-`:21923`,
`:22682`-`:22684`).

**Effect.**

- Zero for the slowest fighters, whose MaxSpd equals the reference. The strafe log's `+24h` shows 1.000 for
  most squadrons, 1.071 for `Zuiho_sqn29` and 1.143 for the `F4U`.
- For an F4U-class fighter, the image engages 286 m earlier (2286 m against 2000 m). It also breaks off at a
  14 % larger too-close distance and pursues 14 % further after a maneuver.
- Prediction: small. Slightly earlier merges and slightly fewer head-on overshoots for the fast fighters;
  no effect on the Japanese JM08 fighters at ratio 1.

**RNG.** None.

**Adoption.** Behind an OFF switch, compute the ratio once at the dogfight install with
`bot_task_speed_ratio` (`include/bsp/bot_tasks.hpp:153`), and pass it to the three sites.

### D4: the gun cone is 0 outside aim (minor, inert where measured)

**Image.** Each state of the dogfight task writes its cone into gun+40h (`dogfight_cone_tuning_offset`):

| state | cone |
| --- | --- |
| aim | Angle_Strafe 15 degrees |
| attackrun, move-to, follow's fly-to arm | Angle_MoveTo 2 degrees |
| follow's hold arm | Angle_Prepare 0 |

**Host.** `:21342`-`:21352` and `:20763`-`:20773` take Angle_Strafe in aim and 0 for every other state
while the dogfight task is installed. `kTaskGunConeBound` applies only to the other tasks.

**Effect.**

- The 2-degree cone opens the fine aim within about 15 degrees of the commanded direction, for any enemy
  plane the finder returns inside 1500 m.
- The dogfight move-to ends at the 2000 m latch, so the move-to case acts only on other aircraft met on the
  way.
- Attackrun is unreachable at mode 1 (`docs/DOGFIGHT_ENGAGED.md`).
- No measurable effect on the four missions.

**Adoption.** Drop the `!dogfight_task_installed` condition once the dogfight moveto and follow write
`gun_cone_40` per the table. Low priority.

### D5 (RNG only): the target reference's re-arm draw

**Image.** `009FAE38`-`009FAE61`: while the dogfight's target reference holds a target, its timer counts
up by dt, and above 2.0 (`00CE3958`) it is re-armed to U(-0.5, 0.5) on stream 1. That is one draw every
1.5-2.5 s per fighter, from the first target pick until the task stops. The pick it can trigger is the
origin for a plane, so there is no behaviour.

**Host.** The dogfight never runs the target reference, so these draws are missing. The torpedo,
dive-bomb and depth-charge bindings take them (`src/approach_target_ref.cpp:109`).

**Effect.** In the image, every later stream-1 draw shifts. In JM08 that is about 6 fighters x (mission
time with a target) / 2 s. The ship fire stagger and AA error rolls come after it.

**Adoption.** Only with the RNG-stream pairing in place, behind its own OFF switch. Expect pair noise, not
kill-rate changes.

## 4. Handoff

- No leases remain after this commit; the packet's lease is released.
- **Open.** `007E2090`'s candidate noise (`007E21AA`, `docs/CC13_DOGFIGHT_DRAWS.md` 5) is the remaining
  air-to-air behaviour draw. The maneuver and avoid bodies were bound and keyed by earlier packets and
  were not re-read here.
- **Order suggested to cc9.** D1 first (largest), then D2, both paired on JM08 and USNOS with
  `BSP_GUNNERY_RNG_STREAMS=1`. Judge on the per-B-17 death table, not totals.
