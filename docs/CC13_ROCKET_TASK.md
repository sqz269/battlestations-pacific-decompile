# The plane rocket task, reconstructed (packet cc13-rocket1)

Addresses: 007B7FD0 007B6A40 007B6830 007B41E0 007B3EA0 007B3C00 007B4390 007B78F0 007B58D0
007B7050 007B6FB0 007B6F50 007B6C00 007B48E0 007B4980 007B4D90 007B4F40 007B4F60 007B5300
007B5430 007B5390 007B5400 007B5780 007B5F10 007B6240 007B59D0 007B5AF0 007B5BE0 007B6D00
007B6CD0 007B7200 007B7270 007B6F90 007B7AF0 007B7BA0 007B7C40 007B7290 007B7F20 007B3D60
007B5980 007B4890 00809DE0 00809E10 0080AA20 007C1EB0 0080A000 009FD570 007F0280 009FA260
009FADA0 009FB200 009BC030 009BC0A0 00999F50 007BBBA0 00BD2F10 009CA870 009CC690 009CAC80
00D05918 00D05910 00D0590C 00D057B8 00D057E8 00D05808 00D05824

Packet `cc13-rocket1`, branch `agent/cc13-rocket1`, 2026-10-12. Reconstruct-only, for the units host
that cc9 owns: nothing in `src/game_hosts_units.cpp` is edited. New files `include/bsp/rocket_task_cc13.hpp`,
`src/rocket_task_cc13.cpp` (registered in `cmake/startup.cmake`), `reports/cc13_rocket_task.json`.
Tools in `local/` (prefix `rk1_`): `rk1_twin.py` (instruction-level twin comparison against the strafe
task), `rk1_pairs.ps1` (all 31 pairs), `rk1_pe.py` (constants from the PE at load width), `rk1_sites.py`
(the draw ranges at each `00BD2F10` site), `rk1_xref.py` (rel32 and absolute-dword references).

**Status: reconstructed and build-tested.** Not wired into any host, not fixture-tested, not game-validated,
not ABI-compatible (host shapes, not native layouts). Names are hypotheses, not recovered symbols.

## 0. Short answer

- The rocket task is the **strafe task's twin**. 31 rocket functions were compared instruction by
  instruction with their strafe counterparts (`local/rk1_pairs.ps1`). The gotowards tick, the set-state
  helper, the gotowards-ready test and the hit notice match instruction for instruction. The arm
  differs in one state offset. Most of the rest differ only in offsets and in which tuning row they
  read. Section 2 lists every difference.
- **What is the rocket's own:**
  - the Strike row of the robots table (`PilotBotParameters +104h..+13Ch`) instead of the Strafe row;
  - a fire distance chosen by the target's size class (`007B3C00`);
  - a range test with hysteresis on `Pilot/Strike/AttackDist`;
  - an **aim point raised by 9.81 x IgnitionDelay^2** (`00809DE0`);
  - a **lead time equal to the rocket's flight time** to the target, with VMax/Acceleration
    kinematics (`00809E10`, `007B5390`), written into the target ref's `projtime`;
  - a **release gate**: timer expired, inside the size-class fire distance, not too close, the point
    within `tan(FireAngle)` of the nose; it writes the rocket's **homing block** and requests release;
  - a goaway with a standoff ring flown through the shared fly-to solver `009FD570`.
- The 25 census draw sites, plus 3 sites in unreferenced functions, are all on keyed streams in the
  reconstruction (section 6). None uses stream 0.
- Section 7 is the suggested glue for `src/game_hosts_units.cpp`. It is not applied.

## 1. Where the task runs (from docs/CC13_ROCKET_KAMIKAZE_SCOPE.md)

`007EEC50` gives class `rocket` (`00E08F48`) when a weapon slot carries ordnance `33h` with
`IgnitionDelay > 0` and the target is surface. `0099A170` then calls the factory `007B7FD0`.

- **Classes (this installation):** `AvengerTinyTim` (16), `Corsair` (102), `Raiden` (151), `Kikka` (183),
  `Shinden` (189), `A8M` (312), `Mustang` (392). Every Corsair and every Raiden is rocket-armed.
- **Pairing missions:**
  - **USN13** (one Japanese `Raiden` squadron; incidental);
  - **USN22** (one `AvengerTinyTim`; incidental);
  - **USN19** (two `Shinden`; the phase scripting is unread);
  - **JM08** (one `Kikka`, one `Shinden`; incidental).
- The R4M (bullet 11) is an air-to-air rocket. The ordnance pass only runs against surface targets, so
  an R4M plane ordered against a ship takes this task. Whether it qualifies depends on the bullet's
  `IgnitionDelay` (the task needs `> 0`, `007C1EB0`) and its `AntiAir` byte; read the bullet row first.
- None of the mission rows makes the rocket task critical. A pair is the evidence that the task runs.

## 2. The twin map

`local/rk1_twin.py` normalises absolute addresses, aligns both streams with `difflib`, and then
compares the 4- or 8-byte value behind every absolute data operand of the equal stretches. "Equal" means
the same instruction with the same data value.

| rocket | strafe twin | equal / total | what differs | coverage here |
| --- | --- | --- | --- | --- |
| `007B7C40` arm | `009CD170` | 33/34 | moveto state `+4E4h` | order, section 4 |
| `007B78F0` approach update | `009CCED0` | 38/160 | own body, section 3 | complete |
| `007B7050` rule | `009CC690` | 67/122 | in-range `+444h`; no `+44Ch` term; done bytes `+6B8h`, `+690h`/`+691h` | complete |
| `007B6FB0` engaged entry | `009CC5F0` | 53/57 | offsets | complete |
| `007B6F50` set state | `009CC5B0` | 20/20 | - | in the rule |
| `007B6C00` gotowards ready | `009CC2F0` | 15/15 | - | shared kernel |
| `007B41E0` approach base | `009CA4A0` | 60/99 | Strike row; size-class fire distance; no muzzle speed; no 1.4 scaling | complete |
| `007B3EA0` glide seed | `009CA3B0` | 51/52 | cap `desc+100h` (row `+10Ch`) | complete |
| `007B4390` aim point | `009CA680` | 4/32 | the lift | complete |
| `007B48E0` gotowards enter | `009CA820` | 22/39 | Strike target-ref arming inline, tail `009FA260` | complete |
| `007B4980` gotowards tick | `009CA870` | **283/283** | - | complete |
| `007B4D90` attackrun ctor | `009CAC80` | 37/41 | - (the strafe body runs on into `009CAD00`) | complete |
| `007B4F40` attackrun enter | `009CAD90` | 5/5 | - | complete |
| `007B4F60` attackrun tick | `009CADB0` | 187/200 | `approach+54h` instead of `009CAD00`; `+664h` instead of `+658h` | complete |
| `007B5300` aim enter | `009CB0F0` | 12/15 | `U(0.2, 1)` | complete |
| `007B5430` aim tick | `009CB1B0` | 86/242 | flight-time lead, release | complete |
| `007B5780` goaway ctor | `009CB500` | 68/70 | `+4Ch = 1000.0` | complete |
| `007B5F10` goaway enter | `009CB8B0` | 164/232 | side and standoff prefix | complete |
| `007B6240` goaway tick | `009CBB30` | 249/271 | the completion test; own re-plans | complete |
| `007B59D0` pitch re-plan | `009CB650` | 75/84 | target from `approach+70h` only | complete |
| `007B5AF0` bank re-plan | `009CB780` | 35/73 | `009FD570` instead of the inline bearing | complete |
| `007B6D00` hit notice | `009CC400` | 15/15 | - | contract only |
| `007B6CD0` slot `+3Ch` | `009CC3D0` | 2/2 | `00E08F48` | complete |
| `007B7200` slot `+40h` | `009CC850` | 40/43 | no `+44Ch` term | complete |
| `007B7270` slot `+4Ch` | `009CC8D0` | 5/6 | `+520h` | contract only |
| `007B7AF0` cruise profile | `009CD020` | 43/45 | `+660h` | complete |
| `007B7F20` debug name | `009CD250` | 37/38 | - | not reconstructed |
| `007B6830` approach ctor | `009CC020` | 114/128 | gotowards seed 1400 (strafe 800); aim `+18h` 0 (strafe 0.5) | complete |
| `007B6A40` task ctor | `009CC230` | 43/47 | kind `0Bh`, offsets | complete |

Rocket-only slots: `+50h` `007B6F90` (2 in the aim state, else 0; the base `007B4140` answers 0), and
the debug draw `+04h` `007B7290` (`+35Dh`-gated, not reconstructed).

## 3. The object

`0099A170` -> `007B7FD0`: `operator_new(708h)` at `007B7FF1`, `007B6A40` at `007B800D`.
`007B6A40` -> `BSP_BotTask_ConstructBase(owner, 0Bh)`, approach `007B6830(owner, target)` at `+3F8h`,
vtables `00D05918` / `00D05910` (`+3F8h`) / `00D0590C` (`+4D0h`), state `+4E4h` (leader) or `+520h`.

**Approach** (`task+3F8h`; `007B41E0` over the shared head `009F9CE0`):

| field | value | evidence |
| --- | --- | --- |
| `+14h` | robots row + 0Ch | `009F9CE0` |
| `+24h` | `max(1, MaxSpd / Pilot/Strike/ReferenceSpeed)` | `009F9CE0(unit, tuning+668h)` at `007B4217` |
| `+34h` | `min(U(0.9, 1.05) x GoAwayDistance(row+108h), AttackDist x 0.8) x +24h` | `007B4240`-`007B4296` |
| `+38h` | `U(0.9, 1.1) x FireDist[size](row+120h + 4 x size) x +24h` | `007B4299`-`007B42ED` |
| `+3Ch` | `+24h x TooClose(row+104h)` (no draw) | `007B42FF` |
| `+2Ch`/`+30h`/`+40h` | glide seed `007B3EA0` | `007B434B` |
| `+44h` | 3600, time since hit (`007B6D00` zeroes it) | `007B42F6` |
| `+48h` | run speed `U(0.8, 1.1) x class+18Ch` | `007B4912` |
| `+4Ch` | in range (task `+444h`) | `007B78F0` |
| `+50h`/`+54h` | 3-D / horizontal distance to the aim point | `007B78F0` |
| `+58h` | target ref (`009FB200`): `+70h` target, `+74h..7Ch` world point, `+80h..88h` body offset, `+9Ch` projtime | `include/bsp/approach_target_ref.hpp` |
| `+CCh`/`+D0h` | range-test period 1.0 / countdown `-U(0, 1)` | `007B432A`, `007B4332` |
| `+D4h` | the aimed rocket, `007C1EB0(unit, 0, 0)` | `007B7918` |

`007B3C00(target)` picks the fire distance: 0 for kind 9 (carrier) or 0Dh (battleship), 1 for 0Bh
(cargo) or 0Ah (cruiser), 2 for other ships, non-ships or none.

**States** (`007B6830` registers them by name):

| state | task offset | built by | fields |
| --- | --- | --- | --- |
| `moveto (Rocket)` | `+4E4h` | `BSP_BotStateMoveTo_Construct` | ranges `+40h`, `+40h`, `+38h` |
| `follow (Rocket)` | `+520h` | `BSP_BotStateFollow_Construct` | - |
| `Rocket/prepare` | `+5B8h` | `BSP_BotStateFollow_Construct` | - |
| `Rocket/gotowards` | `+650h` | inline, vtable `00D057B8` | `+18h`/`+1Ch` = 1400, `+20h` aligned |
| `Rocket/aim` | `+674h` | inline, vtable `00D05808` | `+18h` timer = 0, `+1Ch` no rocket, `+1Dh` off |
| `Rocket/goaway` | `+694h` | `007B5780`, vtable `00D05824` | timers `{2, 5, U(0, 5)}` x 2, `+40h` 1.0, `+44h` `-U(0, 1)`, `+18h` `U(0.3, 0.9)`, `+48h` side, `+4Ch` 1000 |
| `Rocket/attackrun` | `+6E4h` | `007B4D90`, vtable `00D057E8` | `+18h` 1.0, `+1Ch` `-U(0, 1)`, `+20h` 0 |

## 4. Per tick

**The arm** `007B7C40` (slot `+64h`; `RET 4`):
1. `007B78F0(dt)` on the approach (`007B7C52`);
2. `009BDE80` on the moveto state with `+40h`, `+40h`, `+38h` (`007B7C7F`);
3. the rule `007B7050` (`007B7C8E`);
4. the current state's `vt[0Ch](dt)`;
5. `task+2E4h = FFh`.

**The approach update** `007B78F0`:
- `009FADA0(dt)` updates the target ref (its lead uses `projtime`).
- `+44h += dt`, then `+D4h = 007C1EB0(unit, 0, 0)`.
- The distances: the 3-D distance `+50h` and the horizontal distance `+54h`.
- Once per `+CCh` the range test runs:
  - out of range with no live target;
  - when already in range, it **stays** in range under the alternate loadout (`007B58D0`:
    `ctl+369h && [00E17BF2]`, false in single player per docs/BOMBER_AFTER_TASK.md 10.7) or while
    `AttackDist + 100.0 > +54h`;
  - otherwise it **enters** range when `AttackDist > +54h`.

**The rule** `007B7050`:
- "engaged" means `+444h`, or `ctl+370h == 2` with a ref target.
- In an attack state while engaged:
  - mode 0 goes to prepare;
  - prepare goes to `007B6FB0`: mode 0 stays prepare, in range goes to gotowards, else attackrun;
  - attackrun goes to gotowards when in range;
  - goaway goes to gotowards when `+6B8h` is set;
  - gotowards goes to aim when `007B6C00` answers (aligned and `+18h < 2 MaxSpd + +38h`);
  - aim goes to goaway when `+691h` (off) or `+690h` (no rocket) is set.
- Engaged outside an attack state goes to `007B6FB0`.
- Otherwise the flight leader goes to moveto and a wing member to follow.

**The states:**
- **gotowards** `007B4980`: the strafe tick, byte for byte (`rocket_gotowards_tick_007b4980`).
- **attackrun** `007B4F60`: the strafe tick with `+54h` and `+664h` (`rocket_attackrun_tick_007b4f60`).
- **aim** `007B5430`, section 5.
- **goaway** `007B6240`, the strafe tick with two differences.
  - The re-plans: the bank uses `009FD570` with the standoff `+4Ch` and the side `+48h`.
  - The completion test (`007B6370`-`007B63DE`):
    - under the alternate loadout with no rocket the goaway never completes;
    - otherwise it completes when `+54h > +34h`, scaled by 0.4 under the alternate loadout.
- **goaway enter** `007B5F10` sets two fields, then runs the strafe enter:
  - `+48h = ([00F876B0] odd) ? -1 : 1`;
  - `+4Ch = U(1.0, 1.1) x max(AttackDist x 0.8, extent)`, where `extent` is `007B5BE0`'s target extent.
- **The cruise profile** `007B7AF0` (slot `+54h`) works only for the flight leader:
  - it writes Strike CruisingAlt into ctl `+394h` and `+398h` where the base rule allows;
  - then it re-seeds the glide.

  After that every unit runs `007B78F0(0)` and `0099B740`.

## 5. The firing solution and the hand-off

**Aim point** (`007B4390`, approach `vt[0]`): the target ref's world point, with
`y += float(float(t * t) x 9.8100004196167)` where `t` is the aimed rocket's class `IgnitionDelay`
(`[[approach+D4h]+314h]+E0h`, `00809DE0`). This happens only with a rocket at `+D4h`. In effect the
plane aims at a point above the hull. The lift is applied everywhere the approach point is read: the
range test, gotowards, aim and the goaway's fly-to.

**Flight time** (`00809E10`, `__thiscall(record, range)`, `RET 4`; record = `[approach+D4h]`):
- `v0` is the launch speed: `[record+3Ch]->vt[38h]` when `+3Ch` is set, else `|record+318h|`.
- `VMax = class+D8h`, `a = Acceleration = class+DCh`.
- `v0 >= VMax`: the rocket coasts, `t = R / v0`.
- Otherwise `D = 0.5 VMax^2 / a - 0.5 v0^2 / a`:
  - `R > D`: `t = (R - D) / VMax + (VMax - v0) / a`;
  - else `t = sqrt(2 (R + 0.5 v0 (v0 / a)) / a) - v0 / a`.
- The ignition delay is not in the model.

`007B5390` clamps `t` to `[0, 30]` into `approach+9Ch`, which is the target ref's `projtime`
(`sub+44h`). `009FADA0`'s lead tail moves the aim point by the target's velocity times it on the next
update (`approach_target_ref_lead_tail_009faf05`). **The range `R` is the forward range
`f = max(local z, 0.75 |local|)`, not the distance.**

**The aim tick** (`007B5430`, `RET 4`):
1. plan speed `+48h`, heading to the point (mode 2), pitch `atan2(dy, horizontal)` (mode 1);
2. `local` = the point in the unit's frame (`004142E0` over `unit+110h`); `f` as above;
3. with a rocket: `projtime = clamp(flight_time(f))`;
4. `+1Ch = no rocket`; `+1Dh = local z < 0 or TooClose(+3Ch) > f`;
5. `+18h -= dt`;
6. **release** when `+18h < 0`, `+38h > f`, a rocket, not `+1Dh`, and
   `FireAngle(row+11Ch) > |(local x, local y)| / f`:
   - `0080AA20` on `[approach+D4h]+468h` (the rocket record's homing block):
     - `+47Ch` = target `approach+70h` (observer-registered);
     - `+480h..488h` = the body-frame aim offset `approach+80h..88h` (`007B5400`);
     - `+48Ch` = HomingAngle (row `+114h`);
   - then `007BBBA0` on the unit;
   - then `+18h = U(0.8, 1.2) x RepeatTime(row+118h)`;
7. gun cone `[approach+1Ch]+40h = 0`; `009FABE0(gun block, 009A1A20(plan), pitch)`.

Volley size is not a field of the task. The repeat timer re-arms after every release. The aim state
leaves when the plane gets too close, or when `007C1EB0` finds no qualifying rocket (out of rockets).

**Homing.** `0080A000` (the rocket tick) reads `+47Ch`, `+480h` and `+48Ch`. It transforms
`+480h` by the target's matrix when `+48Ch > 0` (`0080A000` around the
`BSP_Vector3f_TransformAffinePoint(+480h, ..., target+CCh)` call). `include/bsp/bomb_torpedo_tick.hpp`
declares only the ignition fields `+490h`/`+494h`, so **the host's rockets do not home today**. The
rocket's homing branch is the next packet if rocket hits matter (section 9).

## 6. Random draws (all `00BD2F10`, `ECX = 1`)

The census (`docs/CC13_RNG_STREAM_CENSUS.md` 10-11, `reports/cc13_rng_stream_census.json`) lists 28 sites
in `007B3D60..007B6240`; its "25" counts the reached ones. Every reached site uses the thread's gameplay
stream (stream 1) in the image. The reconstruction routes each one through
`RocketRandom::uniform_00bd2f10(key, lo, hi)`, the same shape as the kamikaze task's `KamikazeRandom`.
`kRocketDrawSites` maps each site to its key suffix. The host builds the stream key as
`unit name + "#rk" + suffix` (the table shows the full key), as the strafe arm's `#st<field>` keys do.
No site may use stream 0.

| id | site | function | distribution | key | when |
| --- | --- | --- | --- | --- | --- |
| `kGoAwayDist34` | `007B4240` | `007B41E0` | `U(0.9, 1.05)` | `#rk34` | install |
| `kFireDist38` | `007B42CB` | `007B41E0` | `U(0.9, 1.1)` | `#rk38` | install |
| `kCountdownD0` | `007B4332` | `007B41E0` | `-U(0, 1)` | `#rkd0` | install |
| `kGlideAngle2C` | `007B3F0E` | `007B3EA0` | `U(0.8, 1.2)` | `#rk2c` | install, gotowards enter, leader cruise profile |
| `kGlideDist40` | `007B3F49` | `007B3EA0` | `U(-10, 40)` | `#rk40` | same |
| `kRunSpeed48` | `007B4912` | `007B48E0` | `U(0.8, 1.1)` | `#rk48` | gotowards enter |
| `kAttackRunCd1C` | `007B4DEA` | `007B4D90` | `-U(0, 1)` | `#rkar1c` | install |
| `kAimTimer18` | `007B5322` | `007B5300` | `U(0.2, 1)` | `#rkaim18` | aim enter |
| `kAimRepeat18` | `007B5725` | `007B5430` | `U(0.8, 1.2)` | `#rkfire` | after each release |
| `kGoAwayHdgTimer30` | `007B57EE` | `007B5780` | `U(0, 5)` | `#rkga30` | install |
| `kGoAwayPitchTimer3C` | `007B5827` | `007B5780` | `U(0, 5)` | `#rkga3c` | install |
| `kGoAwayCountdown44` | `007B584F` | `007B5780` | `-U(0, 1)` | `#rkga44` | install |
| `kGoAwayProb18` | `007B5874` | `007B5780` | `U(0.3, 0.9)` | `#rkga18` | install |
| `kGoAwayPitch20` | `007B5AC0` | `007B59D0` | `U(min(p, 0), min(p + 10 deg, climb))` | `#rkga20` | goaway enter, re-plan |
| `kGoAwayBank1C` | `007B5BAB` | `007B5AF0` | `U(r - 1, r + 1)` | `#rkga1c` | goaway enter, re-plan |
| `kGoAwayStandoff4C` | `007B5FC4` | `007B5F10` | `U(1.0, 1.1)` | `#rkga4c` | goaway enter |
| `kGoAwayWindow` | `007B6053` | `007B5F10` | `U(10, 16)` | `#rkga_win` | goaway enter, not kind 10h/16h |
| `kGoAwayEnterR` | `007B607B` | `007B5F10` | `U(0, 1)` | `#rkmv_r` | goaway enter, evasive |
| `kGoAwayEnterDist` | `007B60FD` | `007B5F10` | `U(0.5, 0.7)` | `#rkmv_dist` | flikflak |
| `kGoAwayEnterSide` | `007B6180` | `007B5F10` | `U(0, 1)` | `#rkmv_side` | tightturn |
| `kGoAwayHdgRearm` | `007B6296` | `007B6240` | `U(2, 5)` | `#rkga30` | goaway tick |
| `kGoAwayPitchRearm` | `007B62D8` | `007B6240` | `U(2, 5)` | `#rkga3c` | goaway tick |
| `kGoAwayTickR` | `007B6436` | `007B6240` | `U(0, 1)` | `#rkmv_r` | within 1 s of a hit |
| `kGoAwayTickDist` | `007B6476` | `007B6240` | `U(0.5, 0.7)` | `#rkmv_dist` | flikflak |
| `kGoAwayTickSide` | `007B64F1` | `007B6240` | `U(0, 1)` | `#rkmv_side` | tightturn |
| `kUnreachedTimerCtor` | `007B3D8D` | `007B3D60` | `U(0, hi)` | - | no code reference |
| `kUnreachedRunSpeed` | `007B48B2` | `007B4890` | `U(0.8, 1.1)` | - | no code reference |
| `kUnreachedTimerTick` | `007B59B4` | `007B5980` | `U(lo, hi)` | - | no code reference |

- The ranges were read at load width by `local/rk1_sites.py`. 0.3, 0.7, 0.2 and 1.1 are float
  constants (`00CE69C8`, `00CE3E18`, `00CE54A0`, `00CE6448`), passed as floats.
- `local/rk1_xref.py 007B3D60 007B4890 007B5980` finds no rel32 call and no absolute dword for the
  last three. They look like inlined helpers whose copies remain, so a reconstruction never draws them.
- Sites outside the window that the task reaches:
  - `009FB2E7` (the target ref's first timer, between `#rk38` and `#rkd0`);
  - the `009FA260` pick;
  - the flikflak's seeds in `009BB910`.

  The host already keys them (strafe `mv_*`/`ff_*`). A rocket host should key them `#rk...` too.
- **Order at install** (`007B6A40`): `#rk34`, `#rk38`, [`009FB2E7`], `#rkd0`, `#rk2c`, `#rk40`,
  `#rkga30`, `#rkga3c`, `#rkga44`, `#rkga18`, `#rkar1c`.

## 7. Glue for the units host (suggested, NOT applied)

The interface follows the kamikaze task's pattern (`include/bsp/kamikaze_task_cc13.hpp`, packet
cc13-kamikaze1): the task is one value per unit (`RocketTask`), the host implements two interfaces
(`RocketTaskHost`, the reads and sinks; `RocketRandom`, the keyed draws), and two calls drive it:
`rocket_task_construct_007b6a40` on install and `rocket_task_arm_007b7c40` each think. The strafe arm
(`run_strafe_task_arm_009cd170`, `src/game_hosts_units.cpp`) already has almost every helper the
adapter needs, because the rocket is the same machine.

```diff
 // GameUnitSlot (src/game_hosts_units.cpp, beside strafe_task_installed)
+    // Packet cc13-rocket1: the rocket task 007B6A40, behind bsp::kRocketTaskBound.
+    bool rocket_task_installed{false};
+    std::size_t rk_target_plus_one{0};
+    bsp::RocketTask rk{};
+    unsigned long long rk_releases{0}, rk_transitions{0};

 // the arm dispatch, beside run_strafe_task_arm_009cd170(dt)
+    if constexpr (bsp::kRocketTaskBound) run_rocket_task_arm_007b7c40(dt);

+    // One adapter for both interfaces; every value below already exists for strafe.
+    struct RocketHostAdapter final : bsp::RocketTaskHost, bsp::RocketRandom {
+        UnitArm& a;
+        float uniform_00bd2f10(const char* key, float lo, float hi) override {
+            return a.owner_.release_altitude_draw_00bd2f10(a.unit_.row.name + "#rk" + key, lo, hi);
+        }
+        bool update_target_ref_009fada0(bsp::ApproachTargetRefState& ref, float,
+                                        std::array<float, 3>& p) override;   // st_refresh_aim_point on ref
+        void pick_target_ref_009fa260(bsp::ApproachTargetRefState& ref) override;  // hull_aim_seed bump
+        bool target_marked_5d() override;            // !df_slot_live(*st_target())  (LABELLED, as strafe)
+        bsp::RocketTargetKinds target_kinds() override;    // unit_is_kind_of(class, 6/9/0Ah/0Bh/0Dh)
+        bool target_is_unit_5() override;
+        float target_extent_007b5be0() override { return 0.0f; }   // LABELLED, as torpedo/dive
+        float target_y_100() override;
+        bool aimed_rocket_007c1eb0(bsp::RocketAimedRocket& out) override;  // loaded big rocket + class
+        std::array<float, 3> unit_position() override;      // motion.position
+        float heading_50() override;                         // plane_heading_c6c
+        std::array<float, 3> world_to_body(const std::array<float, 3>& p) override;  // df_local
+        float class_speed_188() override;                    // plane_max_spd
+        float class_travel_speed_18c() override;             // plane_travel_speed
+        float class_climb_1e4() override;                    // plane_climb_angle_1e4
+        float class_climb_1ec() override;                    // plane_climb_angle_1ec
+        float class_drop_1f0() override;                     // plane_drop_angle
+        bool bomber_kind() override;                         // st_bomber_kind()
+        bool flight_leader_007b8ad0() override;
+        int attack_mode_370() override;                      // the 0099B740 value, as st_attack_mode_370
+        float ctl_alt_398() override;                        // sq_alt_398, else Strike CruisingAlt
+        bool alt_loadout_007b58d0() override { return false; }   // [00E17BF2] = 0 in single player
+        int parity_f876b0() override;                        // LABELLED stand-in
+        bsp::RocketTuning tuning() override;                 // plane_globals(): pilot_strike_*, cones, ceiling
+        bsp::RocketStrikeRow robot_row() override;           // rocket_strike_row(the unit's robots row)
+        bsp::NearFieldProbeResult near_field_probe_007f0280(const float ext[3]) override {
+            const float w[3] = {0.0f, 0.0f, 0.0f};
+            return a.nf_probe_squadron_007f0280(ext, w);
+        }
+        float fly_to_heading_009fd570(const std::array<float, 3>& point, float standoff,
+                                      float& side, float range) override;   // fly_to_point_heading_009fd570
+        void set_moveto_ranges_009bde80(float n, float f, float s) override;  // the st_moveto ranges
+        void enter_formation_state(bsp::RocketState) override {}   // LABELLED, as strafe
+        void exit_formation_state(bsp::RocketState) override {}
+        void tick_formation_state(bsp::RocketState s, float) override;   // st_moveto / st_follow ticks
+        void apply_plan(const bsp::RocketPlanCommand& c) override;      // st_speed, plan_heading_2c0, ...
+        void push_manoeuvre_00999f50(const bsp::RocketManoeuvre& m) override;   // into st_mv_*
+        void release_rocket(const bsp::RocketRelease& r) override;   // homing block, then
+                                                                     // owner_.release_ordnance_007bbba0
+    };
+
+    void run_rocket_task_arm_007b7c40(float dt) {
+        if (unit_.attack_command_class != bsp::kAttackCmdRocket) { unit_.rocket_task_installed = false; return; }
+        if (unit_.command_target_plus_one == 0) return;
+        RocketHostAdapter host{*this};
+        if (!unit_.rocket_task_installed || unit_.rk_target_plus_one != unit_.command_target_plus_one) {
+            bsp::rocket_task_construct_007b6a40(unit_.rk, host, host);   // SUBSTITUTION as strafe: per target
+            unit_.rocket_task_installed = true;
+            unit_.rk_target_plus_one = unit_.command_target_plus_one;
+        }
+        // 0099B740 (attack mode) and the manoeuvre suspension exactly as the strafe arm.
+        const bsp::RocketArmResult r = bsp::rocket_task_arm_007b7c40(unit_.rk, host, host, dt);
+        if (r.after != r.before) ++unit_.rk_transitions;
+        if (r.released) ++unit_.rk_releases;
+        owner_.done("BotTaskRocket::arm", 0x007b7c40u);
+    }
```

The cruise profile (slot `+54h`) is `rocket_task_cruise_profile_007b7af0`. The strafe host does not run
its twin `009CD020` because that twin's call cadence is unread (`strafe_task.hpp`), so leave the rocket
one out until that cadence is read.

**Feeds and values the host must provide:**
- The PilotSetTarget choice must feed `has_rocket_ordnance` (`007ED970` -> `007B9480`, kind `33h` with
  `IgnitionDelay > 0`). Today it is never set (SQUADRON_LAND_TASK 5bw.1), so the class is never chosen.
  Land it together with `kRocketTaskBound`.
- `aimed_rocket_007c1eb0`: true while the plane still holds a rocket that passes
  `rocket_platform_qualifies_007c1eb0`. Fill the class's `IgnitionDelay`, `VMax` and `Acceleration`
  (`projectile_kinds.hpp` `kRocketClassOff*`). For the launch speed, use the plane's speed. Reading
  `record+3Ch` as the carrying plane is provisional (section 9).
- `release_rocket`: the host's rocket record is the bomb-family record (`bomb_torpedo_tick.hpp`), with
  the homing block at `+47Ch`/`+480h`/`+48Ch`. Then call `owner_.release_ordnance_007bbba0(unit_)`. The
  homing branch of `0080A000` is not hosted (section 5).
- `target_extent_007b5be0`: the torpedo and dive hosts leave it absent (SQUADRON_LAND_TASK 12795).
  Return 0 and label the substitution as they do.
- `parity_f876b0`: the image's global `[00F876B0]`. A deterministic stand-in, for example the unit's
  process index, must be labelled.
- The gun direction (`009FABE0`): strafe does not carry it. Carry it only when the gun gate reads it.
- The manoeuvre: copy `RocketManoeuvre` into `st_mv_*` and reuse `st_manoeuvre_tick`. One unit has one
  attack class at a time. Its draws stay on the strafe `#st` keys unless re-keyed.

## 8. For cc9: constants in the strafe host (read while comparing twins)

These came out of the twin comparison. They are cc9's to apply. Each is the strafe twin of a rocket
instruction verified here; `reports/cc13_rocket_task.json` "corrections" has them with evidence.

| strafe host line | was | the image (FLD/FMUL qword) |
| --- | --- | --- |
| `st_goaway_heading_009cb780`, `[00D05858]` | `0.3490658402442932` | **0.3490658700466156** |
| `st_goaway_pitch_009cb650`, `[00D05850]` | `0.1745329201221466` | **0.1745329350233078** |
| `st_gotowards_tick_009ca870`, `[00D057D8]` | `0.6108652353286743` | **0.6108652551968893** |
| `st_install_009cc230`, attackrun `+1Ch` | `0.0` | **`-U(0, 1)`** drawn by `009CAC80` at `009CACDA` |

The first three are double constants whose low mantissa word is `60000000h`. They are not
`float(angle)` widened, so a float literal is off by about 3e-8. The fourth is a missing draw (strafe
stream 1). The rocket reconstruction uses the image values.

## 9. Uncertainty and open items

- **x87 precision.** Intermediates are double, with a float store at each `FSTP dword`, as in
  `src/strafe_task.cpp`. The main thread runs at 24 bits (docs/CC13_FPU_PRECISION.md), so the last
  bit can differ.
- **The near-field probe products** (`-out_a.x * out_b.y * out_b.z`) follow cc9's reading of the
  strafe twin. The probe's output-pointer order was not re-derived here.
- **`007C1EB0`'s return** is `vt[218h](2Ah)` on the platform. It is read as the loaded rocket record
  because `00809E10` and `0080AA20` use rocket-record offsets on it (`+314h`, `+318h`, `+468h`) and
  `0080A000` reads the same `+47Ch`/`+480h`/`+48Ch`. The platform's `vt[210h]`, `vt[218h]` and
  `vt[220h]` bodies were not read.
- **`record+3Ch`** as the carrier entity: provisional.
- **Unread here:**
  - the consumer of slot `+50h`;
  - slot `+58h` `007B7BA0` (exits the state, clears the gun block's target `+3C4h` and `+3A4h..+3ACh`);
    read but not reconstructed;
  - the hit notice `007B6D00`, a twin of `BSP_BotTaskStrafe_OnHit`.
- **Rocket homing** (`0080A000`'s `+47Ch` branch) is not hosted. Without it a released rocket flies
  ballistic in the host.
- **Shared helpers in the window that the rocket does not call:**
  - `007B7870`: called by the strafe tail `009CA80E` and by `009A76D3`; unread;
  - `007B5350`: called by `009A617B` and `009A63FB`;
  - `007B5C70`, `007B5D30`, `007B5E20`: condition constructors shared with strafe and `0099xxxx`;
  - `007B5BE0`: target extent, shared by four goaway enters;
  - `007B7CC0`..`007B80C0`: string helpers and the AntiAir predicates `007B80A0`/`007B80C0`.

## 10. Handoff

- Done:
  - every rocket-task function read, with the twin evidence;
  - `src/rocket_task_cc13.cpp`: the whole-task calls `rocket_task_construct_007b6a40`,
    `rocket_task_arm_007b7c40` and `rocket_task_cruise_profile_007b7af0`, plus each routine as its own
    kernel. The interface is the kamikaze task's pattern. The build passes;
  - ledger names and Ghidra comments (section 11);
  - the report `reports/cc13_rocket_task.json` (`tools/verify_report_calls.py`: 69 rows, 0 failed).
- Next, for cc9's lane:
  1. the section 7 glue, behind `kRocketTaskBound`, together with the `has_rocket_ordnance` feed;
  2. pairs on USN13 (Raiden), USN22 (Tiny Tim Avenger), JM08 and USN19 (Kikka/Shinden). Check each
     class's bullet `IgnitionDelay` and `AntiAir` first: an R4M with `IgnitionDelay <= 0` never takes
     this task;
  3. the rocket homing branch of `0080A000` as its own packet;
  4. the section 8 constants in the strafe host.

## 11. Ledger and Ghidra

- **Names.** `bsp.py ledger add-name` recorded 41 new names (the section 0 address list less the
  outside contracts). It appended evidence to 5 existing names: `007B7FD0`, `007B6A40`, `007B4980`,
  `007B7AF0`, `007B5BE0`. Existing names were kept, including `_Provisional` on `007B4980`.
- **Reconstruction records.** `bsp.py ledger add-function` recorded 27 functions, all with status
  `reconstructed_build_tested`. Each record carries its coverage and the ABI from the listing's `RET`.
- **Ghidra.** `tools/ghidra_annotate.py --apply` ran in two passes. The old values are in
  `local/ghidra-annotations-20261012T094446Z.json` and `...094513Z.json`. `007B6D00` and `007B7270` have
  no Ghidra function, so their names live in the ledger only. No function was created; the
  'Non-Returning Functions - Discovered' analyzer was not touched.
- **Constants.** `tools/const_width_sweep.py include/bsp/rocket_task_cc13.hpp` passes: every header
  constant matches the width the listing loads it at.
