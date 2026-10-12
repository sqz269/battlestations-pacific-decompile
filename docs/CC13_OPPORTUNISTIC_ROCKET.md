# The pilot bot's opportunistic rockets (packet cc13-oprocket1)

`009FD0E0` `BSP_PilotBot_OpportunisticRocketTick` is the third rocket path in the image. The rocket task
(docs/CC13_ROCKET_TASK.md) and its homing (docs/CC13_ROCKET_HOMING.md) are the other two. Every AI pilot bot
owns this sub-object at `task+38Ch`. While a **dogfight** or **strafe** task has handed it a target, it
fires small rockets (HVAR, R4M) at that target in short salvos. It also fires at **planes**, which the rocket
task never does.

Status:
- **Reconstructed and build-tested:** `include/bsp/opportunistic_rocket_cc13.hpp`,
  `src/opportunistic_rocket_cc13.cpp` (bsp_core).
- **Not wired:** `kOpportunisticRocketBound = false`. The host glue in section 7 is a suggested diff that was
  applied, built and reverted.
- **Not tested:** no fixture, no game run.
- Every name here is a hypothesis, not a recovered symbol.

## 1. The object and who drives it

| piece | address | evidence |
| --- | --- | --- |
| sub-object | `task+38Ch`, 5Ch bytes | the base bot-task ctor runs `009FCEF0(this+38Ch, this)` at `0099C79C` (docs/BOT_TASKS.md) |
| ctor | `009FCEF0` (cc13-legmgr1's range, not renamed here) | `+0 = -1`, `+4 = task`, `+8 = task+2F4h` (the plane), `+0Ch = 0`, `+10h = +14h = [00D7A260]` (-1.0), the observer nodes `+24h` (vtable `00CE75CC`, pointer `+38h`) and `+44h` (vtable `00D21CCC`, pointer `+58h`), `+3Ch = 0`, `+40h = 0`. `+18h..+20h` are **not** written |
| target setter | `007B7870` `BSP_PilotBot_OpportunisticRocketSetTarget` | `__thiscall(this; target, x, y, z)`, `RET 10h`: re-registers `+24h` with the target, copies the point to `+18h..+20h`, `+3Ch = target vt[5Ch](0Fh)` (0 for null) |
| setter callers | `009A76D3` in `009A7650` `BSP_BotTaskDogfight_SetTarget` (point = zero vector `[00F87574]`); `009CA80E` in `009CA780` `BSP_BotApproachStrafe_RearmTargetPoint` (point = `approach+80h`) | rel32 and absolute-dword scan of the PE (`local/opr_rel32.py`): these two only |
| clear | inlined in the Stops (slot `+58h`) of dogfight `009AA9E0` (vtable `00D1F9B0`), strafe `009CD0D0` (`00D210E0`) and rocket `007B7BA0` (`00D05918`) | target 0, point `[00F87574]`, `+3Ch = 0`. A byte scan for `+3B0h`/`+3C4h`/`+3C8h` (`task+38Ch+24h/+38h/+3Ch`) finds only these three; a capstone sweep of `.text` for `LEA r,[r+24h]` next to `006952A0`/`00694A60` finds no other setter |
| tick | `009FD0E0`, `__thiscall(this = task+38Ch, float dt)`, `RET 4`, body `009FD0E0-009FD54E` | listing read instruction by instruction |
| range slot | `009F99B0` `BSP_PilotBot_OpportunisticRocketRangeSlot`, `__fastcall(ECX = target)`, int in `EAX` | Ghidra types the return as `bool`; the values are 0-5 |

Fields:

| offset | meaning |
| --- | --- |
| `+0` | class row of the last decision: -1 none, 0 small plane, 1 large plane, 2 ship, 3 landfort |
| `+0Ch` | rockets left in the salvo (int) |
| `+10h` | decision timer (`RocketCheckTime`) |
| `+14h` | gap timer (`RocketDelay`) |
| `+18h..+20h` | the point handed to `0080AA20` (target body frame) |
| `+38h` | the target (observer-tracked) |
| `+3Ch` | the target is a plane (picks AA rockets) |
| `+40h` | cached `desc` = robots row + 0Ch |
| `+58h` | the rocket picked for the next shot (observer-tracked) |

**Cadence.** `BSP_PilotBot_Update` (`009998A0`) calls it at `0099998C` on the AI branch (`0099C270` false:
the unit is not under human control in that slot), every think. The order is: task `vt[64h]`, then the gun
tick `009FC7C0` (only with `unit+C24h`), then `009FD0E0` (no gate of its own there), then `009A17D0`, then
`0099D300` (docs/PILOT_BOT_TICK_GATES.md). The other caller, `00997340`, runs the same pair. It has no rel32
call and no absolute reference anywhere in the image, so it is dead code.

**What disables it.**
- **No target.** Only the dogfight and strafe tasks set one. Under every other task kind (torpedo, dive-bomb,
  level-bomb, rocket, moveto, land, ...) the target is null. Every task change runs the old task's Stop, and the
  dogfight, strafe and rocket Stops clear it.
- **No suitable rocket.** `BSP_Unit_FindAimRocket(plane, want_small = 1, anti_air = +3Ch)` (`007C1EB0`) must
  find a rocket with `IgnitionDelay <= 0` whose `AntiAir` matches the target: AA rockets for planes,
  non-AA rockets for ships and land targets (`[00F874FD]`, 0 after scene load, would accept either).
- The plane's own live bytes (`+5Ch` set, `+5Dh`/`+60h`/`+5Eh` clear) and a target that passes `0043F080`.

## 2. The robots rows

The class row is `row+148h + class * 34h` (`desc+13Ch`). `RocketCheckTime` is `row+140h/+144h`. The row
comes from `[task+3F4h]`: `00F8A30C`-based, `+30h` base and `+34h` index times `248h`. Field names follow
`include/bsp/robot_config.hpp`.

| class row | +0 | +4..+18h | +1Ch | +20h | +24h | +28h | +2Ch |
| --- | --- | --- | --- | --- | --- | --- | --- |
| meaning | RocketChance | AttackDist[6], indexed by `009F99B0` | AttackTime | NumRockets (int) | RocketDelay | AttackAngle | HomingAngle |

`AttackDist` is one six-slot array: 0 small plane, 1 large plane, 2 BB/CV (kind 9 or 0Dh), 3 CA/CL (0Bh or 0Ah),
4 other ships, 5 everything else. robots.lua fills only its own class's slots: the scalar of Rocket_SmallPlane
goes to slot 0, Rocket_LargePlane to slot 1, the `{BB/CV, CA/CL/AK, DD}` table of Rocket_Ship to slots 2-4, and
Rocket_Landfort to slot 5 (`robot_config.hpp` offsets `14Ch`, `184h`, `1BCh..1C4h`, `1FCh`). The other slots
stay 0.0, because the config is zero-filled BSS. Section 8 notes the one case where an unfilled slot is read.

This installation's `scripts/datatables/robots.lua` (mtime 2025-06-01 16:03, modified relative to the
2024-07-13 bulk). `kOpportunisticRocketRows` carries all six levels. Read by `local/opr_robots.py`. Angles are
`DEG()`. Columns: chance / dist / time / N / delay / angle / homing.

| level (index) | CheckTime | SmallPlane | LargePlane | Ship (dist BB-CV, CA-CL, DD) | Landfort |
| --- | --- | --- | --- | --- | --- |
| Stun (0) :1301 | 2-3 | 0.1/1000/4/2/0.6/30/0.2 | 0.1/1000/3.5/2/0.6/10/0.2 | 0.1/1000,700,500/8/2/0.6/10/0.2 | 0.1/1000/6/2/0.6/10/0.2 |
| SPNormal (1) :611 | 1-2 | 1/1000/4/4/0.2/30/0.6 | 0.5/1000/3.5/2/0.2/6/0.5 | 1/1800,1200,800/3.5/4/0.2/10/0.2 | 0.8/1200/3.5/3/0.15/4/0.2 |
| SPVeteran (2) :778 | 2-3 | 1/1500/3.5/1/0.3/3/2 | 1/1800/3.5/1/0.3/2/2 | 1/1800,1600,1400/3.5/2/0.3/2/2 | 1/1200/3.5/1/0.3/4/2 |
| MPNormal (3) :916 | 2-3 | 0.5/1000/4/1/0.3/9/0.5 | 0.75/1200/3.5/1/0.3/6/1 | 0.9/1300,1100,900/3.5/2/0.3/6/1.5 | 0.9/1200/3.5/2/0.3/4/1.5 |
| MPVeteran (4) :1065 | 2-3 | 0.5/1200/4/1/0.3/4/2 | 1/1200/3.5/1/0.3/6/2 | 1/1600,1300,1000/3.5/2/0.3/4/1.75 | 1/1500/3.5/2/0.3/4/1.75 |
| Elite (5) :1192 | 2-3 | 1/1500/3.5/1/0.3/3/2 | 1/1800/3.5/1/0.3/2/2 | 1/1800,1600,1400/3.5/2/0.3/2/2 | 1/1200/3.5/1/0.3/4/2 |

The Hungarian comments state the intent:
- RocketCheckTime: "decides this often whether to rocket";
- RocketChance: "the chance at each decision";
- AttackDist: "only at targets closer than this";
- AttackTime: "only if the rocket reaches the target within this time";
- AttackAngle: "only if the target's direction differs from the rocket's by at most this";
- NumRockets: "at most this many per decision";
- RocketDelay: "at least this much time between rockets";
- HomingAngle: "the rocket may turn this many degrees per second toward the target".

## 3. The tick, step by step (`009FD0E0`)

1. `009FD0E7`: if the target's `+5Dh` is set, the target node is unregistered (`+38h = 0`).
2. `009FD10A`: the plane (`+8h`) must have `+5Ch` set and `+5Dh`, `+60h`, `+5Eh` clear. Otherwise return.
3. `009FD131`: `+10h -= dt` and `+14h -= dt`, each as a float store.
4. `009FD156`: if `+40h` is null, it is fetched from the task's robots ref. If it is still null, return.
5. **Decision** (`009FD187`, when `0 > +10h`):
   - `+0Ch = 0`;
   - `+10h = U(CheckTime[1], CheckTime[2])` (`009FD1B3`);
   - `+0 = -1`;
   - with no target, return;
   - `r = U(0, 1)` (`009FD1DD`);
   - the class (section 4);
   - with no class, return;
   - if `RocketChance > r` (`FCOMIP`/`JBE`), `+0Ch = NumRockets`.
6. `009FD284`: with class -1, return.
7. `009FD29D`: continue only if `0 > +14h`, `+0Ch != 0`, there is a target and `0043F080(target)` passes.
8. `009FD2CB`: if `+58h` is null, or `00923810(rocket, 1)` is null, then `+58h = 007C1EB0(plane, 1, +3Ch)`.
   `00923810` is the rocket's `+3Ch` parent link, so null means it has left the plane.
9. `009FD2F5`: if `+58h` holds an attached rocket:
   - `d0 = |plane - target|` (`00427EB0` x2, `00427C90`, `0042B2F0`);
   - `t0 = 00809E10(rocket, d0)`, the flight time (`rocket_flight_time_00809e10`);
   - `lead = t0 * target vt[34h]` (world velocity), `aim = target + lead`, each a float store;
   - `body = 00414D10(00414E10(plane), aim)`: right, up, forward;
   - `d1 = |body|`, `t1 = 00809E10(rocket, d1)`;
   - `dist = AttackDist[009F99B0(target)]`;
   - **the gates**, in order. A failed gate returns at `009FD548` and keeps the rocket ref:

     | gate | site | pass when |
     | --- | --- | --- |
     | in front | `009FD3EB` COMISS | `body.z > 1.0` (`00D7A24C`) |
     | range | `009FD40E` FCOMIP | `dist > d1` |
     | time | `009FD41F` FCOMIP | `AttackTime > t1` |
     | not too close | `009FD42F` COMISS | `t1 > 0.25` (`00CE3868`) |
     | angle | `009FD4B2`, `009FD4D9` | `L < 5.0` (`00CE3850`), else `AttackAngle > L / clamp(d1, 0.4*dist, dist)` |

     Here `L = sqrt(body.x^2 + body.y^2) * s` and `s = 00419010(0, 0.66, NumRockets, 1.0, +0Ch)`.
     `s` is 1.0 for the first rocket of a full salvo and falls toward 0.66 as the salvo empties, so later
     rockets pass a looser cone. `0.4` is the qword at `00CE65D0`; `0.66` is `00D21CE4`.
   - **fire** (`009FD4E3`):
     - `0080AA20(rocket+468h, target, &+18h, HomingAngle)`. This is the `009FD4FB` site of
       CC13_ROCKET_HOMING 5;
     - `007BBBA0(plane)`, the release request;
     - `+0Ch -= 1`;
     - `+14h = 00415550(&RocketDelay, &0.08)`, i.e. max(RocketDelay, 0.08) (`00D05B50`).
10. `009FD529`: whenever `+58h` is non-null here (after a release, or after a fresh pick that is not attached),
    the rocket node is unregistered and `+58h = 0`.

The aim **geometry** uses the target's origin and its lead. The `+18h` point (the strafe pick, or zero for
dogfight) only goes to the homing block.

**RNG.** There are two draws: `009FD1B3` and `009FD1DD`, both `00BD2F10` with `ECX = 1`, the thread's
gameplay stream. The census (docs/CC13_RNG_STREAM_CENSUS.md) lists them as the "ordnance sub-object, 2 sites,
a contract". The reconstruction routes both through `RocketRandom` with the keys `chk` and `chance`. A host
must build them as `unit name + "#or" + key`, never on stream 0. The draws happen whenever the decision timer
expires on a live plane whose robots row resolves, **even with no target** (the first draw only), so such a plane
draws once every 1-3 s whether it has rockets or not. A host that binds this moves the image's shared stream order
for every AI plane. Keying it per unit avoids that.

## 4. Target class (`009FD1E6-009FD256`, `009F99B0`)

| target | class row | AttackDist slot |
| --- | --- | --- |
| kind 0Fh, `0047B880` true (not kind 10h/16h) | 0 SmallPlane | 0 |
| kind 0Fh, kind 10h or 16h (bombers) | 1 LargePlane | 1 |
| kind 6, kind 9 or 0Dh | 2 Ship | 2 |
| kind 6, kind 0Bh or 0Ah | 2 Ship | 3 |
| kind 6, other | 2 Ship | 4 |
| kind 1Bh, 45h or 46h | 3 Landfort | 5 |
| anything else | none: no salvo | 5 |

`0047B880` (`BSP_Unit_UsesSmallTurnRollLimit`) is `!(vt[5Ch](10h) || vt[5Ch](16h))`. At `009FD1F9`,
`NEG AL / SBB / ADD 1` gives 0 when it is true.

## 5. x87 and precision

- The two timers are `FSUB` then `FSTP dword`.
- The lead products and sums are stored as dwords.
- `x` and `y` are spilled as qwords. `x*x + y*y` is stored as a dword, then `sqrt` (`00BF7030`) as a dword,
  then the scale product as a dword.
- The ratio `L / clamp` is `FDIVR qword` against the x87 clamp result. It is compared with the float
  AttackAngle in the x87 register. The reconstruction divides in double. The main thread runs the FPU at
  24 bits (docs/CC13_FPU_PRECISION.md), so a ratio within one float ulp of the angle can differ.
- `0.4` is a qword constant (`0.4000000059604645`), multiplied at extended precision and stored as a float.

## 6. What it reaches in this installation

Carriers (docs/CC13_ROCKET_KAMIKAZE_SCOPE.md 2 and CC13_ROCKET_HOMING 5):

| rocket (bullet) | IgnitionDelay | AntiAir (arcade line) | fired by this path at |
| --- | --- | --- | --- |
| Tiny Tim (6) | 1 | false (176) | **never** (`want_small` rejects it) |
| 5" HVAR (8), HVAR DLC (51) | 0 | false (249, 1475) | ships and land targets, from **strafe** |
| R4M AA (11), R4M AA (116, arcade) | 0 | true (352, 2979) | planes, from **dogfight** |

So:
- **R4M planes in a dogfight fire air-to-air salvos.** This covers the Raiden (every one), the Shinden
  (DefaultEquipment 2, the R4M set), and the Kikka and A8M when they carry set 2.
  - Against bombers (kind 10h/16h) the LargePlane row applies. At SPNormal: chance 0.5, 1000 m, 3.5 s, 2
    rockets, a 6 deg cone, homing 0.5 deg/s.
  - Against fighters the SmallPlane row applies: chance 1, 1000 m, 4 s, 4 rockets, 30 deg, 0.6 deg/s.
  - The R4M (VMax 800, Acceleration 1000) covers 1000 m in about 1.5 s from a 150 m/s launch, inside both AttackTimes.
- **HVAR planes strafing fire salvos at ships and land targets.** This covers the Corsair (every one), the
  Mustang with set 2 and the AvengerTinyTim with set 2 (default set 1 carries only the Tiny Tim).
  - At SPNormal against ships: chance 1, 4 rockets, 10 deg; the range is 1800 m for BB/CV, 1200 m for
    CA/CL and 800 m for DD.
  - Against land targets: chance 0.8, 3 rockets, 1200 m.
- **Missions with such units in the scene** (scope table, section 3): USN13 (1 Raiden), USN07 (6 Raiden),
  IJN10, IJN17 (2 Raiden), IJN19 (1 Raiden), IJN14 and ALT02 (Corsairs and 7 Raidens each), IJN20 (2 Corsair,
  1 Shinden), IJN23 (7 Corsair), IJN29 (A8M and 1 Raiden), USNBR (4 Corsair), FINAL (6 Corsair), USN18
  (`prcp_13_iwojima`: Corsairs, 2 Shinden), USN19 (`prcp_14_okinawa`: 2 Shinden), JM08 (Kikka, Shinden),
  and the script spawns of IJN14 (2 Corsair) and IJN22 (Corsair, Mustang). The player's own Corsairs
  (USN14, USN17, USN19, FINAL) count while they are AI-flown.
- **What changes:**
  - R4M fighters (mostly Japanese, in USN missions) gain air-to-air rocket fire against US fighters and
    bombers in every dogfight. This is a new loss source for the player's air groups.
  - Corsairs gain rocket salvos during strafing runs, which adds damage to the strafe's gun damage.
  - It moves only units that are in a dogfight or strafe task **and** carry the rocket.
  - Without the fuze (CC13_ROCKET_HOMING 7), a host R4M hits a plane only on contact.

## 7. Host glue for cc9's units host (suggested, NOT applied)

**Dependencies.** The glue is inert unless both of these land with it:
- **The rocket ordnance feed:** `007ED970` / `BSP_Unit_FindAimRocket` `007C1EB0`. The host must know which
  rack holds a class-33h bullet with `IgnitionDelay <= 0` and the right `AntiAir`. Today the host knows each
  rack's bullet class (`rack_bullet_class_per_rack`) and ammo, but not `IgnitionDelay` or `AntiAir`. The glue
  reads both from Lua (`Bullets[b]`) per rack, the way `read_device_bullet_class_id` does.
- **The rocket homing glue** (CC13_ROCKET_HOMING 6, `kRocketHomingBound`): `set_homing_target_0080aa20` must
  reach the rocket record the release spawns. Until it lands, the glue releases unguided rockets and records
  the homing request.

It also needs `release_ordnance_007bbba0` to launch a **rocket** from the chosen rack. Today the host's single
rack drop is bomb/torpedo-shaped. A rack with class 33h and ammo is released the same way, so the projectile
the gunnery host spawns for it is the open part.

GLUE-DIFF-PLACEHOLDER

## 8. Uncertainty and open items

- **`+18h` frame.** The strafe hands `approach+80h`, which is `ApproachTargetRefState::centre_54` (ref at
  `approach+2Ch`). `0080AA20` treats the point as target-body-frame. Whether `centre_54` is body frame was not
  traced. The dogfight hands the zero vector (the target's origin).
- **Stale slot.** The class row is chosen at the decision, but the AttackDist slot is chosen from the target
  *now*. If dogfight or strafe re-targets between decisions (for example from a fighter to a bomber), the
  small-plane row is read at slot 1, which is 0.0, and the range gate fails until the next decision. The
  reconstruction keeps this.
- **The released rocket.** `0080AA20` arms the rocket that `007C1EB0` picked. `007BBBA0` only raises the
  plane's release request. That the request launches that same rocket is assumed, as in CC13_ROCKET_TASK.
- **`00923810`** (`n`-th `+3Ch` link, 26 callers) is not renamed. It is shared and outside this packet.
- **`009FCEF0`/`009FCF90`** (ctor, rocket-node setter) lie in cc13-legmgr1's lease and are not renamed here.
  Suggested names: `BSP_PilotBot_OpportunisticRocketConstruct` and `..._SetRocketRef`.
- **Precision:** section 5.
- **Verification:** build only. `tools/verify_report_calls.py reports/cc13_opportunistic_rocket_calls.json`:
  33 call rows, 0 failed.

## 9. Ledger and Ghidra

- Names, applied with `tools/ghidra_annotate.py --apply` and saved. The old values (`FUN_...`) are in
  `local/ghidra-annotations-20261012T113805Z.json` of the worker tree:
  - `009FD0E0` `BSP_PilotBot_OpportunisticRocketTick` (was `FUN_009fd0e0`);
  - `009F99B0` `BSP_PilotBot_OpportunisticRocketRangeSlot` (was `FUN_009f99b0`);
  - `007B7870` `BSP_PilotBot_OpportunisticRocketSetTarget` (was `FUN_007b7870`).
- Function records: `opportunistic_rocket_tick_009fd0e0`, `opportunistic_rocket_range_slot_009f99b0`,
  `opportunistic_rocket_set_target_007b7870`.
- No function was created. `Non-Returning Functions - Discovered` was left disabled.
  `ghidra proto 00BF9DC8` was `void __cdecl _free(void *)` before and after.
