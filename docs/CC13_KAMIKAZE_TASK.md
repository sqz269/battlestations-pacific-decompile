# The kamikaze and drop-kamikaze bot tasks (packet cc13-kamikaze1)

Addresses: 009AB740-009AF830 (window), 009AF720 009AEEF0 009AED20 009AB920 009AB850 009ABF60 009ABA30
009AF0A0 009AC160 009AC190 009AEFC0 009AC8D0 009AC910 009AF310 009AF480 009AF510 009AF560 009AEBE0
009ADBF0 009AD720 009ABAB0 009ADE00 009AE9D0 009AE610 009AE530 009AE1B0 009AE8A0 009ACFC0 009AD180
009AD1F0 009AD430 009AD480 009AD5E0 009AD6A0 009AE210 006FEC00 007F4BA0 007F15F0.

Worker cc13-kamikaze1, branch `agent/cc13-kamikaze1`, 2026-10-12. Project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`; instruction evidence from the installed executable on disk
(`bsp.py disasm-raw`), constants read at their load width (`local/kz1_pe.py`, `local/kz1_consts.py`).
Names are hypotheses, not recovered symbols.

**Status.** Exported, reconstructed and **build-tested** only: `include/bsp/kamikaze_task_cc13.hpp`,
`src/kamikaze_task_cc13.cpp` (target `bsp_game`). Nothing is wired into a host, nothing was run, so
nothing is game-validated. The suggested host glue is in section 6 and was not applied.

## 1. Short answers

- **Kamikaze task (kind 0Ch).** Five states: moveto and follow (the shared formation states),
  prepare (registered, never chosen by the rule), **gotowards** and **aim**. Gotowards flies at the
  target with a pitch law and a turnaround latch; aim is the terminal dive, steering the yaw and
  pitch plan slots from the target's body-frame position. There is **no detonation code** in the
  task: the plane flies into the target and the collision path (`00819A20`
  `BSP_UnitInstance_KamikazeDetonate`, hosted as `kamikaze_detonate_00819a20` in cc9's gunnery host)
  does the rest.
- **Drop-kamikaze task (kind 5).** Seven states: moveto, follow, attackrun, prepare, done, release,
  goAway. The carrier (BettyOhka 32, FunryuCarrier 811) flies a glide approach (attackrun), levels
  out and releases its `DummyKamikazePlane` round through `007BBBA0` (release or prepare), then turns
  away (goAway) or rejoins (done).
- **The released Ohka / Funryu do get a kamikaze task, indirectly** (section 4). The dummy's tick
  `006FEC00` spawns the class-156/370 unit `OpenAfterTime` seconds after the drop, with
  `AutoAttackTarget` = the dummy's target. The new squadron's SEntity init (`007F4BA0`) reads that
  property and orders the attack through `007F15F0 -> 007EEC50`, the ordinary chooser, which gives
  class `00E08F50` to a kind-17h unit; the pilot bot's `0099A170` then builds the task with
  `009AF720`. Nothing in the window or the dummy names the kamikaze class directly.
- **RNG.** Eight draw sites, all `00BD2F10` with ECX = 1 (stream 1). The reconstruction routes each
  through `KamikazeRandom::uniform_00bd2f10(key, lo, hi)`; section 5 lists keys and distributions.
  Nothing here draws from stream 0.

## 2. The kamikaze task

### 2.1 Layout

| object | offset | evidence |
| --- | --- | --- |
| task vtables | `00D1FD40` (+0), `00D1FD3C` (+3F8h), `00D1FD38` (+4D4h) | `009AEF31`-`009AEF4D` |
| approach base | `009AB920`, vtable `00D1FAF4` then `00D1FD34`/`00D1FD30` | `009AB9A1`, `009AED20` |
| target ref | approach+68h (task+460h); sub+18h = approach+80h = task+478h is the target | `009AB99B LEA ECX,[EBP+68h]` before `009FB200` |
| states | moveto +4E8h, follow +524h, prepare +5BCh, gotowards +654h (`00D1FB84`), aim +678h (`00D1FBA0`) | `009AED20` registrations, `009AF310` |
| initial state | not leader: follow; leader and not rocket-like: moveto; leader and rocket-like: gotowards | `009AEF5x`-`009AEF69` (the "extra LEA ESI+654h" of docs/BOT_TASKS.md is gotowards) |

Task vtable `00D1FD40` slots read from `.rdata`: +0Ch `009AF5D0` (no Ghidra function; copies
approach vtable[0]), +10h `009AF070` (returns 8), +14h `009AF080` ("Kamikaze"), +18h `009AF670`,
+3Ch `009AF060` (`00E08F50`), +40h `009AF560`, +4Ch `009AF2F0` (follow query when on +524h, else
`0099B720`), +54h `009AF480`, +64h `009AF510`. State vtables: +4 enter, +8 exit (`007B3DC0`, a bare
RET, for gotowards and aim), +0Ch tick, +14h a debug draw.

### 2.2 The approach

**Rocket-like vs fighter-like** (`009AB93E`-`009AB959`, again at `009AC1A3` and `009AEFF7`): the unit
answers `IsKindOf(17h)` and its byte +C24h (PilotFires) is clear. Rocket-like takes the
`Pilot/Kamikaze/RocketLike` block (tuning +45Ch..+474h), fighter-like `FighterLike` (+478h..+490h).
The seven floats land at approach+2Ch..+44h by REP MOVSD (`009ABA09`): CruisingAlt, DropDist,
AttackRange, AttackAlt, TurboRange, TurboAngle, ReferenceSpeed. The ReferenceSpeed goes to `009F9CE0`
(speed ratio +24h).

**`009AB850` (redraw):** +48h = AttackRange x ratio; +4Ch = ctl+398h + U(-40, 80), floored at 15
(compare against the qword 15.0 at `00CF3F20`, store of the float at `00CE5380`); +50h = ctl+394h +
U(-80, 80), floored at 20. Called by the constructor, every gotowards enter and every cruise-profile
call.

**`009ABF60` (aim error):** reads the robots row Kamikaze* fields through `[approach+14h]`
(`robot_config.hpp` `_08c`..`_0b4`): spread = TargetPrecision on all three axes; dh = U(-HError,
HError), dv = U(-VError, VError), approach+64h = U(-ProjTimeError, ProjTimeError) (lower bounds formed
as `-0.0 - x` by SUBSS); the centre sub+54h..5Ch = **(dv, 0, dh)**; section chance and the three
weights at sub+64h..70h; then `009FA380(sub, (dh, 0, dv))` (the bias) and a tail jump to `009FA260`
(the pick). The centre and the bias take the two draws in opposite orders; both were checked in the
listing (`009AC020`-`009AC08C`). approach+64h has no reader in the window (only the stores
`009AB9B4`, `009AC01D`); the draw is consumed for its stream position only.

**`009ABA30` (approach vtable 0, the point flown at):** the ref's world point, y raised to 1.0, plus
`00419010(8.0, 40.0, 2.0, 0.0, eta)`: 40 m above the target while the eta is 8 s or more, 0 at 2 s.

**`009AF0A0` (update, first in the arm):** gun block (approach+1Ch)+34h/+38h = 0; `009FADA0` on the
ref; band +58h = 1; +5Ch = +5Dh. With a target: a target answering `IsKindOf(0Eh)` has its spread
zeroed and the dirty byte set (aim at the centre); +54h = planar distance to the 009ABA30 point
(`0042B2F0` on (dx, 0, dz)); +60h = +54h / max(unit vtable[204h], 10) (`007B8E70` FLD unit+B2Ch, the
body forward speed); projtime sub+44h (approach+ACh) = clamp(+60h, 2, 5) then clamp [0, 30]; a
fighter-like plane has +5Ch = (+48h > +54h).

### 2.3 gotowards (`009AC190`, listing read in full, `local/kz1_ac190.asm`)

- d = 3-D distance to the point (`009AC0B0`), floored at 10, kept in +18h. H = pi/2 - atan2(dz, dx)
  wrapped to [0, 2pi); err = `00438B10`(H, heading).
- **Turnaround latch +1Dh.** R = (k |err| + 3.0) x class+188h with k = 1.2 rocket-like, 0.8 otherwise.
  When latched, the heading is H + pi until d > R + 50. When not latched, it latches when
  d < min(R, (h - 100) x f), with h = unit y - point y and f = 1.5 / 1.0; a rocket-like plane latches
  only when h >= 0.2 d. Plan +2C0h = heading (mode 2), +2E8h = 1.2.
- **Pitch.** P = atan2(-h, d), kept in +20h. Latched: atan2(+4Ch - h, +48h) clamped to [-30, 20]
  degrees (a climb back to +4Ch). Beyond TurboRange + 10: e = h - d / +48h x +4Ch, p2 = atan2(-(h + k e),
  d) with k = 0.6 for e >= 0 and 0.8 below, clamped by `00415690` into [P - w, P + a], a =
  `00419010`(0.8, 0, 1.5, 15 deg, d / TurboRange), w = `00419010`(1, 0, 2, 20 deg, same). Inside
  TurboRange + 10: P. Rocket-like: min with `00419010`(0.5, -25 deg, 1.1, 5 deg, |v| / LevelFlight).
  Plan +2BCh = pitch, +2D0h = 1.
- **Aligned +1Ch** = not latched, |err| < 25 deg and wrap(pitch - unit+C64h) < 20 deg (signed).
- Throttle slot 1.0, air brake 0 (plan+2D8h = 0); fighter-like and latched: boost plan+2E4h = 1.
- Band +58h: 1 below +48h, 3 below 3000 (rocket-like) or 2000, else FFh.
- The ambiguous `FSUBR ST(1)` at `009AC389` is `D8 E9` (ST0 = ST1 - ST0 = h - 100) and `009AC594` is
  `DE C2` (FADDP ST(2)), decoded from the bytes.

**`009AEFC0` (ready for aim):** aligned, d < +48h + 2000.0 (qword), and either d < TurboRange or
(rocket-like and +20h > -TurboAngle).

### 2.4 aim (`009AC910`, `local/kz1_ac910.asm`)

- Enter `009AC8D0`: lock +18h = 0, window time +1Ch = 0, precision timer +20h = 0, multiplier +24h =
  1.0, bank side +19h = (roll < 0).
- The point is moved into the body frame (`004142E0` with unit+110h); u = x / max(z, 30), v = y /
  max(z, 30). Band +58h = 0.
- Precision timer: +20h -= dt; below 0 it is redrawn to U(0.8, 1.25) x ManeuverPrecisionTimer and
  +24h to U(ManeuverPrecisionMul[1], [2]) (robots.lua: 2.5 s and {0.8, 1.1} in the SP blocks read).
- Unlocked: yaw slot +284h = clamp(u x 20 x m, +-1), pitch slot +29Ch = clamp(v x 15 x m, +-1), bank
  +2C4h = 0. Locked: gains 36 and 24, and bank = max(b - eta, 1) x side x 0.06 with b = 12 when the
  plane has no guns (unit+C24h clear), else 5. Plan +2CCh = 1, +2D0h = 0, +2D4h = 0; throttle 1.0.
- Bank side latch: set below -20 degrees of roll, cleared above +20.
- Lock: the window w = 0.025 (no guns) or 0.035, scaled by 1000 / max(d, 200) under 1000 m. Inside
  TurboRange with |u| < w and |v| < w for more than 3.0 s locks +18h; while locked and inside, plan
  +2E4h = 1 (boost) every tick. Leaving the window zeroes +1Ch; nothing unlocks except the enter.

### 2.5 The rule `009AF310` and the arm `009AF510`

`abandon` = not rocket-like and (no target, or target+5Dh, or (not in range +454h and ctl+370h != 2)).
From aim, gotowards or prepare: abandon goes to the formation state (leader moveto, else follow);
gotowards goes to aim when `009AEFC0` holds; aim and prepare otherwise stay. From any other state:
abandon goes to formation, otherwise straight to **gotowards** (prepare is skipped). A rocket-like
plane never abandons. The arm: `009AF0A0(dt)` on task+3F8h, `009AF310`, current state vtable[0Ch](dt),
task+2E4h = task+450h (the band).

**`009AF480` (vtable +54h):** the altitude channels (bot_tasks.hpp `PilotAltitudeChannel`): ctl+394h
= task+424h (CruisingAlt), ctl+398h = task+430h (AttackAlt), then `009AB850`, `009AF0A0(0)` and
`0099EBB0`. **`009AF560` (+40h):** 2 without a control block; 1 when ctl vtable[114h] holds a command
of class `00E08F50` or `00E08F78` whose resolved target is task+478h (or none is latched).

## 3. The drop-kamikaze task

### 3.1 Layout

Task vtables `00D1FC70`, `00D1FC68` (+3F8h), `00D1FC64` (+4BCh). Approach base `009ABAB0` (vtable
`00D1FB00`), target ref at approach+30h (`009ADE0B LEA ECX,[ESI+30h]`), the target at approach+48h =
task+440h. States: moveto +4D0h, follow +50Ch, attackrun +5A4h (`00D1FBEC`), prepare +5C0h and done
+664h (both `00D1FC08`, a follow variant), release +708h (`00D1FC40`), goAway +72Ch (`00D1FC24`).
Field names come from the property-bag pair `009ABBE0`/`009AE2D0`: startDist +ACh, attackDist +B0h,
targetDist +B4h, targetHdg +B8h, currentAvoidFlag +BCh, insideAttackRange +C0h. Task vtable slots:
+1Ch `009AE1B0`, +24h `009AE210`, +3Ch `009ADDC0` (`00E08F30`), +40h `009AE140`, +4Ch `009AE260`,
+50h `009ADDB0` (2 while approach+C1h), +54h `009AE9D0`, +58h `009AE0D0`, +64h `009AE8A0`.

### 3.2 Approach and profile

- `009ABAB0`: ratio from tuning+458h **Pilot/LevelBomb/ReferenceSpeed**; +A4h = class+18Ch; alt +A8h =
  1200; startDist = ratio x 2000 (qword); attackDist = startDist x 1.4 (qword); targetDist 99999;
  avoid flag FFh; ordnance byte +C1h = 1; bias 0 (`009FA380` on the zero-fill at `00F87574`), spread 0.5.
- `009AE9D0` (+54h): a flight leader writes both altitude channels from tuning+45Ch
  (RocketLike/CruisingAlt); everyone redraws startDist = U(0.9, 1.2) x ratio x tuning+460h
  (RocketLike/DropDist) and attackDist = ratio x 500 (qword) + startDist; then `009ADE00(0)`.
- `009ADE00` (update): alt +A8h = ctl+398h; +C1h = `007B93E0`; with a target, planar targetDist and
  targetHdg; insideAttackRange latches below startDist, holds below startDist + 100, holds for good
  when `007B58D0` (ctl+369h and `[00E17BF2]`, "reload enabled") is set; an empty follower without
  reload keeps it only while its leader (ctl+3D0h) is within attackDist of the aim point.

### 3.3 States

| state | enter | tick |
| --- | --- | --- |
| attackrun | `009ACFB0` (+18h = 0) | heading = targetHdg; `009FBA50`(alt, startDist, targetDist, `00419010`(0.1, 0.4, 0.35, 1.0, max(1400 - y, 50) / min(targetDist, 2000))); throttle |
| prepare, done | `009AD180`: follow enter, ctl+3E4h = 3, formation indices, timer +98h = -1 | `009AD1F0`: without a countdown, the follow base `009C1FD0` (avoid flag 3 when ctl+3D0h); with one (+98h > 0, set to 3.0 by `009AE060`): wings level, 5 deg pitch (mode 2), speed hold, release when \|roll\| and \|pitch\| < 15 deg or the timer ends; while not level the timer is held at max(t, 1) (`00415550`) |
| release | `009AD6A0`: +18h = unit+C25h, timer 3.0, speed max(\|v\|, level) | `009AD5E0`: release once (`007BBBA0`, approach+2Ch -= 1), wings level, 5 deg (mode 1), speed hold; the timer runs down once released and unit+C25h is clear |
| goAway | `009AD430`: side = +1 when `[00F876B0]` is odd, else -1 | `009AD480`: re-engage +18h = reload and ordnance; `009FB800`(alt, 1.0); heading from `009FD570`(point, unit, startDist, &side, targetDist, 1.0); turn scale 0.5; exit `009AD460` restores 1.0 |

### 3.4 Rule `009AE610`, entry `009AE530`, break-off `009AE1B0`, arm `009AE8A0`

`engage` = inside, or (ctl+370h == 2 and a target). Outside the five attack states: engage enters
`009AE530`, else the formation state. Inside them: no engage, formation; ctl+370h == 0, prepare; done:
leader to goAway, follower re-enters when reload and ordnance; goAway: re-enters on its +18h; others:
`009AE1B0` sends the leader to goAway and a follower to done; prepare: released to release, no
countdown to the entry; attackrun: inside to release; release: timer below 0 to goAway/done.
`009AE530`: unit+C25h, release; mode 0, prepare; with ordnance, inside ? release : attackrun; else
leader ? goAway : done. `009AE1B0`: `BSP_BotTask_BreakOffAllowedForUnit` and (no target, target+5Dh,
or without reload: not in an attack state or no ordnance). The arm sets the avoid flag to FFh, runs
`009ADE00`, sets the moveto ranges (alt - 100, alt, startDist) through `009BDE80`, the rule, the state
tick, task+2E4h = avoid flag, and releases one owed round (task+424h, raised by the manual drop
`009AE210`) per tick while (unit+72Ch) vtable[38h] holds and the state is not attackrun, release or
prepare.

## 4. Do the released Ohka (156) and Funryu (370) get a kamikaze task?

**Yes, by an ordinary attack order, not by a direct task install.** The chain, each step from the
listing:

1. The carrier's release `007BBBA0` fires the `DummyKamikazePlane` round (ordnance kind 2Fh, stored at
   instance+C4h by `006FE950`). Its tick element vtable `00CFC610` has slot +8h = **`006FEC00`** (no
   Ghidra function; the only reference to it is the `.rdata` slot `00CFC618`, scan `00 ec 6f 00`).
2. `006FEC00` calls the bomb tick `006E1300`, then, on a single-player or host world
   (`[00E188A8]+1FE4h != 2`), detached from its carrier, once `now - launch time (+9Ch) >
   desc+D8h` (**OpenAfterTime**, read by `006FE780` from `006FF170`), builds a property bag
   (`008F41A0`) and spawns `[desc+DCh]` (**KamikazePlaneClass**, `006FF1EE`) through `004F0AD0` at
   the dummy's matrix with: Type = class+70h, WingCount 1, Skill, Race, Party, OwnerPlayer, State 7,
   VelocitySI, Vel3D, Invincible 0.0, FireStance 2, **AutoAttackTarget** = the dummy's target (+98h)
   handle +174h when set, and **DummyObjectID** = dummy+A0h (`006FED4A`-`006FEE74`, the property
   names are the pushed string literals).
3. The new plane's squadron runs `007F4BA0` (SEntity init slot +A4h); on world mode 0 or 1 it reads
   `AutoAttackTarget` (`007F4EC0 PUSH 00CF87EC`, `008F2260`), resolves the handle (`00521E30`) and calls
   `007F15F0` (`007F4EE5`), whose callees are `007EEC50 BSP_Unit_ChooseAttackCommand` and `0071ECF0
   BSP_WeaponDirector_IssueCommand`.
4. Classes 156 (MXY7 Ohka) and 370 (Funryu) are `Type "Kamikaze"` in this installation's
   vehicleclasses.lua and answer kind 17h (docs/CC13_ROCKET_KAMIKAZE_SCOPE.md section 2), so the
   chooser's arm `007EEB36` gives class `00E08F50` against a ship (subject to the small-ship gate).
   The pilot bot (`0099ACD0` -> `0099A170`) builds the task with `009AF720`.
5. Being gunless (unit+C24h clear), the Ohka runs the **RocketLike** profile: it starts in
   gotowards when it leads its one-plane squadron, never abandons, and caps its climb by speed.

**Byte scans, with positive controls.**
- `50 8f e0 00` (`00E08F50` as a literal): 18 hits. The known users appear (the chooser `007EED45`,
  the installer `0099A37D`, PilotGunFire `008A529A`, the task's own `009AF061`/`009AF586`). None is in
  the dummy, the release states or the squadron init: no direct kamikaze order exists on this path.
- `callsite_census 009AF720`: one CALL, `0099A3A2` in `0099A170`; no absolute reference (`20 f7 9a
  00`: none). Same for `009AEBE0` (`0099A2DD`). `0099A170` itself: `0099ADAA` (pilot bot tick) and the
  JMP `0099A5EB`.
- `ec 87 cf 00` (the `AutoAttackTarget` literal): writer `006FEE57` (the dummy spawn, also `006FFCD3`
  in the DummySubmarine twin) and readers `007F4EC1`, `006C525A`, `0094BD9C`, `0094C12E`, `0085406A`.
  `callsite_census 007F15F0`: one CALL, `007F4EE5`.
- `40 c8 cf 00` (`DummyObjectID`): writer `006FEE6C`, readers `0095E8AD` (the plane property binder,
  docs/UNIT_NEIGHBOUR_FIELDS.md) and `007F4D53`; `GetKamikazeByDummy` (`008C3880`) maps the dummy id
  to the unit through unit+6B8h; no script in this installation calls it.

Not proven: that the spawned squadron's init runs before its first think (pass order is
docs/SENTITY_INIT_PASSES.md's subject) and that the spawned unit carries a target that is still
alive; a dummy without a target (+98h null) spawns without `AutoAttackTarget` and so without an order.

## 5. RNG draw sites (stream 1, ECX = 1)

| site | function | distribution | lands in | key |
| --- | --- | --- | --- | --- |
| `009AB885` | `009AB850` | U(-40, 80) + ctl+398h, floor 15 | approach+4Ch | `4c` |
| `009AB8DF` | `009AB850` | U(-80, 80) + ctl+394h, floor 20 | approach+50h | `50` |
| `009ABFB2` | `009ABF60` | U(-HError, HError) | bias x, centre z | `h` |
| `009ABFE5` | `009ABF60` | U(-VError, VError) | bias z, centre x | `v` |
| `009AC018` | `009ABF60` | U(-ProjTimeError, ProjTimeError) | approach+64h (no reader) | `64` |
| `009ACA03` | `009AC910` | U(0.8, 1.25) x ManeuverPrecisionTimer | aim+20h | `t` |
| `009ACA2F` | `009AC910` | U(Mul[1], Mul[2]) | aim+24h | `mul` |
| `009AEA82` | `009AE9D0` | U(0.9, 1.2) x ratio x DropDist | startDist | `ac` |

Order per call: `009AB850` draws `4c` then `50`; a gotowards enter draws `4c`, `50`, `h`, `v`, `64`;
the aim timer draws `t` then `mul`. The image draws all of them from one stream in tick order; a host
keyed stream (`unit name + "#kz" + key`, the H-U convention of `release_altitude_draw_00bd2f10`) is
a labelled substitution for that order, faithful in distribution. These are the census's eight
HOST-OMITS-FUNCTION rows of section 11 (CC13_RNG_STREAM_CENSUS.md); their host status changes only
when cc9 wires the tasks.

## 6. Host glue for cc9 (suggested, not applied)

The reconstruction offers two drivers, `kamikaze_task_arm_009af510` and `drop_kamikaze_task_arm_009ae8a0`,
each over a small host interface, plus the pure per-state functions if cc9 prefers its inline style.
The minimal wiring in `src/game_hosts_units.cpp`, behind OFF switches:

```diff
+#include "bsp/kamikaze_task_cc13.hpp"
+// Packet cc13-kamikaze1 (docs/CC13_KAMIKAZE_TASK.md): committed OFF.
+inline constexpr bool kKamikazeTaskBound = false;
+inline constexpr bool kDropKamikazeTaskBound = false;

 struct GameUnitSlot {
+    bool kamikaze_task_installed{false};
+    bsp::KamikazeTask kz;                  // task kind 0Ch
+    bool drop_kamikaze_task_installed{false};
+    bsp::DropKamikazeTask dkz;             // task kind 5
+    std::size_t kz_target_plus_one{0};
 };

+// One adapter per task over the slot (sketch; every read below already exists in the host):
+struct KamikazeHostAdapter final : bsp::KamikazeTaskHost, bsp::KamikazeRandom {
+    float uniform_00bd2f10(const char* key, float lo, float hi) override {
+        return owner_.release_altitude_draw_00bd2f10(unit_.row.name + "#kz" + key, lo, hi);
+    }
+    bool update_target_ref_009fada0(bsp::ApproachTargetRefState&, float, std::array<float,3>& p) override {
+        const GameUnitSlot* t = target();            // command_target_plus_one, as st_target()
+        if (t == nullptr) return false;
+        float q[3] = {t->motion.position[0], t->motion.position[1], t->motion.position[2]};
+        owner_.aim_point_009fada0(unit_, *t, unit_.command_target_plus_one, q);
+        p = {q[0], q[1], q[2]};
+        return true;
+    }
+    std::array<float,3> world_to_body(const std::array<float,3>& p) override {
+        float out[3]; follow_to_body(unit_, p.data(), false, out); return {out[0], out[1], out[2]};
+    }
+    float heading_50() override { return pose_heading_radians(unit_); }
+    float pitch_c64() override { return unit_.plane_pitch_angle_c64; }
+    float roll_c68() override { return unit_.plane_bank_angle_c68; }
+    bool pilot_fires_c24() override { return unit_.plane_pilot_fires_c24; }
+    bool unit_kind_17() override { return bsp::unit_is_kind_of(unit_.class_id, 0x17); }
+    bool flight_leader_007b8ad0() override { return owner_.unit_is_flight_leader_007b8ad0(unit_.process_index); }
+    int attack_mode_370() override { return unit_.st_attack_mode_370; }   // the strafe arm's copy
+    float level_flight_speed_007c47f0() override { return bot_desired_speed_007c47f0(unit_); }
+    // speed_b1c: |motion.linear_velocity|; forward_speed_b2c: the kTerrainAvoidForwardSpeedBound dot;
+    // class_speed_188: unit_.plane_max_spd; robot_row(): the PilotBot level's Kamikaze* fields;
+    // ctl_alt_*: the squadron's sq_alt_394/398 as the strafe glide seed reads them.
+    void apply_plan(const bsp::KamikazePlanCommand& c) override;   // below
+    void enter/exit/tick_formation_state(...) override;           // the existing moveto/follow arms
+};

+void apply_plan(GameUnitSlot& u, const bsp::KamikazePlanCommand& c) {
+    if (c.writes_heading_2c0) { u.plan_heading_2c0 = c.heading_2c0; u.plan_heading_mode_2cc = 2; }
+    if (c.writes_bank_2c4) { u.plan_state.bank_target_2c4 = c.bank_2c4; u.plan_heading_mode_2cc = 1; }
+    if (c.writes_turn_scale_2e8) u.plan_state.turn_scale_2e8 = c.turn_scale_2e8;
+    if (c.writes_pitch_target_2bc) { u.plan_state.pitch_target_2bc = c.pitch_target_2bc;
+                                     u.plan_state.pitch_mode_2d0 = c.pitch_mode_2d0; }
+    if (c.writes_yaw_slot_284) { u.plan_slots[bsp::kPilotSlotYaw].desired = c.yaw_284;
+                                 u.plan_slots[bsp::kPilotSlotYaw].active = 1; u.gl_yaw_mode_2d4_zero = true; }
+    if (c.writes_pitch_slot_29c) { u.plan_slots[bsp::kPilotSlotPitch].desired = c.pitch_29c;
+                                   u.plan_slots[bsp::kPilotSlotPitch].active = 1; u.plan_state.pitch_mode_2d0 = 0; }
+    if (c.writes_throttle_slots) { u.plan_slots[bsp::kPilotSlotThrottle].desired = 1.0f;
+                                   u.plan_slots[bsp::kPilotSlotThrottle].active = 1;
+                                   u.plan_slots[bsp::kPilotSlotAirBrake].desired = 0.0f;
+                                   u.plan_slots[bsp::kPilotSlotAirBrake].active = 1;
+                                   u.plane_air_brake_mode_2d8 = 0; }
+    if (c.writes_speed_2b4) { u.plane_desired_speed_2b4 = c.speed_2b4; u.plane_air_brake_mode_2d8 = 1; }
+    // c.boost_2e4: plan+2E4h -> cmd+14h -> unit+A10h (PILOT_PLAN_SLOT_PIPELINE.md); no host field yet.
+    // c.calls_cruise_altitude_009fba50 / calls_pitch_command_009fb800: the host's
+    // cruise_altitude_command_009fba50 and pitch_command_009fb800 (plane_flight.hpp).
+}

 // In the per-think dispatcher, next to run_strafe_task_arm_009cd170 (line ~29994):
+if constexpr (kKamikazeTaskBound) run_kamikaze_task_arm_009af510(elapsed);
+if constexpr (kDropKamikazeTaskBound) run_drop_kamikaze_task_arm_009ae8a0(elapsed);

+void run_kamikaze_task_arm_009af510(float dt) {
+    if (unit_.attack_command_class != bsp::kAttackCmdKamikaze) { unit_.kamikaze_task_installed = false; return; }
+    if (unit_.command_target_plus_one == 0) return;
+    KamikazeHostAdapter h{owner_, unit_};
+    if (!unit_.kamikaze_task_installed || unit_.kz_target_plus_one != unit_.command_target_plus_one) {
+        unit_.kz = bsp::KamikazeTask{};          // 0099A170 builds a new task per order
+        /* seed: unit_kind_17, pilot_fires_c24, plane_max_spd, the two tuning profiles, ctl alts */
+        bsp::kamikaze_task_construct_009aeef0(unit_.kz, h, seed, h);
+        unit_.kamikaze_task_installed = true; unit_.kz_target_plus_one = unit_.command_target_plus_one;
+    }
+    bsp::kamikaze_task_arm_009af510(unit_.kz, h, h, dt);
+}
```

The same shape serves the drop task (`DropKamikazeTaskHost`): `request_ordnance_release_007bbba0`
maps to the gunnery host's drop of the carried round (cc9's `release_bomb_drop` path, which must spawn
the dummy and, for faithfulness, later the class-156/370 unit of section 4); `manual_control_72c` is
`unit_.plane_control_mode_900 == 7`; `reload_enabled_007b58d0` is the loadout byte and
`lua_device_reload_enabled_00e17bf2()`; `step_odd_00f876b0` is the host's step-counter parity (as the
dive-bomb goaway enter takes it); `fly_to_heading_009fd570` is `plane_fly_to_solver.hpp`. The host
must also pass the plane gun controller's gate: the arm clears gun block +34h/+38h every tick and the
goAway/prepare states clear +40h. Also needed: an `ApproachTargetRefState` per task (the reconstruction
writes its spread, centre, bias, weights and dirty byte), and the cruise profile slot +54h on the
host's cadence (the strafe host records that cadence as unread).

## 7. Verification and uncertainty

- x87: the gotowards, aim, approach update, drop update and attackrun bodies were walked by hand
  over the full listings (stack slots traced back to their stores); the two short-form opcodes were
  decoded from bytes. `tools/x87_sym.py` confirmed two forced paths: `009AC9B4` (aim, unlocked:
  u = x / zc, v = y / zc, timer - dt, yaw = u x 20.0 x +24h, pitch = v x 15.0 x +24h) and
  `009AC4BF` (gotowards turbo arm: q = d / +48h x +4Ch, e = h - q, -(h + e x 0.6), d / +3Ch, and the
  two `00419010` argument lists). The walker does not model `_CIatan2`'s two-in, one-out stack, so
  its expressions after `00BF701A` were not used.
- Precision: intermediates are computed in double and rounded at the image's float stores; at the
  main thread's 24-bit control word (CC13_FPU_PRECISION.md) the image rounds intermediates to float,
  which this code does not reproduce bit for bit.
- unit+C25h (the drop entry and the release timer) is the release-pending byte: its only writer
  (scan `c6 ?? 25 0c 00 00`, positive control: the readers `009AD668`, `009AE539`, `009B5AC9` of
  `80 ?? 25 0c 00 00 00`) is `007C0EE2` in `007C0D90 BSP_Plane_TickReleaseOrderIssue`
  (docs/TORPEDO_RELEASE_ORDERS.md; `release_pending_c25` in plane_level_bomb_task.hpp). So the
  release state's 3 s timer runs only once the release order has gone out. ctl+370h (attack mode)
  is the strafe arm's `st_attack_mode_370`. (unit+72Ch) vtable[38h] is the free-flight test
  (`plane_control_mode_900 == 7` in the host).
- `009AF2B0` (set gotowards) has no reader traced; `009AF030` (the kamikaze attack-state test, no
  Ghidra function) was read but not named. The property-bag (save/load) twins are named, not
  reconstructed.
- The follow base, the moveto state, `009FADA0`, `009FA260`, `009FBA50`, `009FB800`, `009FD570` and
  `007BBBA0` are host-side and cited, not re-derived.

## 8. Ledger and Ghidra

50 names added (`python tools/bsp.py ledger add-name`) and applied through
`tools/ghidra_annotate.py --apply` (prior annotations recorded in
`local/ghidra-annotations-20261012T093649Z.json`, project saved). Existing names (`009AB740`,
`009ADBF0`, `009AE1B0`, `009AEBE0`, `009AEEF0`, `009AF480`, `009AF720`, `009AD1E0`) were kept.
`006FEC00` (the dummy tick that spawns the Ohka/Funryu) lies outside this packet's lease and is
recommended as `BSP_DummyKamikazePlane_TickSpawnKamikaze` for its owner.

## 9. Handoff

Open: decide with cc9 whether the
gunnery host's drop spawns the dummy at all (without it the drop path ends at the release and no
Ohka exists). Scripts: `local/kz1_cat.py`, `kz1_pe.py`, `kz1_consts.py`, `kz1_names.py`; listings
`local/kz1_*.asm`.
