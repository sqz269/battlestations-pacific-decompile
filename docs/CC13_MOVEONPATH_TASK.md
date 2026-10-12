# The plane moveonpath bot task (packet cc13-moveonpath1)

Addresses: window 009BC690-009BDD70; chain 0099A24A (in 0099A170) -> 009BDBB0 -> 009BCE50; plus the
cited callees 009FD050, 007B1350, 00414C60, 0047B850, 0047B880, 009C2980.

Worker cc13-moveonpath1, branch `agent/cc13-moveonpath1`, 2026-10-12. Project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Instruction evidence is Capstone over the installed executable's
bytes (`local/mp1_dis.py`, listing `local/mp1_window.asm`); constants read at their load width
(`local/mp1_pe.py`). Names are hypotheses, not recovered symbols. Scope: docs/CC13_OMITTED_TASKS_SCOPE.md
section 1.

**Status.** Exported, reconstructed and **build-tested** only: `include/bsp/moveonpath_task_cc13.hpp`,
`src/moveonpath_task_cc13.cpp` (target `bsp_game`). The host glue of section 8 was applied in this
worktree with its switch OFF, built (`scripts/build.ps1`, both tests passed), reverted, and checked with
`git apply --check` against origin/main `d8b34c2cc`. Nothing was run, so nothing is game-validated.

## 1. Short answers

- **The task.** Kind 6 (`009BCE6D PUSH 6` into `BSP_BotTask_ConstructBase`), 520h bytes, command class
  `00E08F80` (slot +3Ch), name "moveonpath" (slot +14h). Three states registered by name: "path (path)",
  "follow (path)" and "circle (path)". The leader flies the **path** state, a wingman the shared **follow**
  state. The live code never selects **circle**: the only writers of task+310h are the constructor
  (`009BCEF6`) and the rule (`009BD1A9`), and both pick path or follow. Circle is reachable only through
  the load path's `BSP_BotStateRegistry_ReadCurrent` (`009BDA30`).
- **What the leader flies.** Each tick the approach update `009BD4D0` copies the path cursor's current
  point (`cursor->vtable[4](&out, -1)`) into approach+34h..+3Ch. The path tick `009BC8E0` then:
  - commands squadron speed ctl+3A0h;
  - sends `009FB800` the point's height;
  - sends `009F9E40` the point as the heading target.

  The task never moves the cursor. The director step's moveonpath arm `00836BF0` does that, as it does
  for ships.
- **Altitude.** A point authored at y >= 10.0 is flown at y + altOffset (approach+2Ch = U(0,
  MaxAltOffset)), provided no converging-leg weight is active. Otherwise the plane flies at the squadron
  cruise altitude ctl+394h less travelAltOffset (approach+30h). For a PATH_FM_SIMPLE path within 3000 m
  of the final point, the converging-leg manager (`009BD400`/`009FC260`) supplies the altitude instead.
  While ctl+3ADh (cruise altitude set) is clear, both offsets are zeroed.
- **Cruise profile (slot +54h, `009BD6E0`).**
  - Every call it redraws travelAltOffset = U(0, TravelAltRandom).
  - For a leader only, it picks the cruising altitude:
    - with ordnance and the class's MoveOnCruisingAlt flag set, CruisingAlt + 0.6 x TravelAltRandom
      (torpedo, then depth charge, then general bomb split into level or dive bomb);
    - otherwise Small/LargePlaneTravelAlt, chosen by `0047B880`.
  - It writes the altitude through ctl+37Ch's one-shot channel, the shape every slot +54h override shares.
- **RNG.** All draws use stream 1 (ECX = 1); none use stream 0 (section 6).
- **Boundary repair.** `009BD6E0`'s body (`009BD6E0-009BD98B`) was already correct: INT3 padding
  follows its RET at `009BD98B`. The call at `009BD9A1` belongs to a separate function, `009BD990`, the
  task's slot +64h (absolute dword `00D20974`), which Ghidra did not have. Twelve more functions (ten
  vtable slots and two constructors with no references) were missing as well. Section 2 lists them.

## 2. Ghidra boundary repair (step 0)

`_free` (`00BF9DC8`) showed `void __cdecl _free(void * _Memory)` without noreturn before and after every
create. All creates went through `tools/ghidra_define_function.py`, which takes the write lock,
disassembles the exact range and creates the function from the existing instructions without flow
discovery. The project was saved, and the record is in `local/mp1_define_record.json`.

| function | old Ghidra state | new body (end exclusive) | evidence |
| --- | --- | --- | --- |
| `009BD6E0` | `009BD6E0-009BD98B` | unchanged | RET at `009BD98B`, INT3 `009BD98C-009BD98F` |
| `009BD990` | no function, not covered | `009BD990-009BD9CE` | RET 4 at `009BD9CB`, INT3 after; dword `00D20974` |
| `009BC8E0` | none | `009BC8E0-009BC950` | RET 4 at `009BC94D`; dword `00D208C8` |
| `009BCD40` | none | `009BCD40-009BCD48` | JMP `009BCE30`; dword `00D208F4` |
| `009BCFB0`, `009BCFD0` | none | 6 bytes each | RET; dwords `00D2096C`, `00D20920` |
| `009BD090` | none | `009BD090-009BD09B` | JMP `009BDB90`; dword `00D20908` |
| `009BD120` | none | `009BD120-009BD150` | tail JMP EDX at `009BD14D`, RET at `009BD14F`; dword `00D20968` |
| `009BD280`, `009BD2E0` | none | `-009BD2DB`, `-009BD2FA` | dwords `00D20950`, `00D2095C` |
| `009BD380`, `009BD3C0` | none | `-009BD3B2`, `-009BD3F7` | dwords `00D20970`, `00D2091C` |
| `009BC870`, `009BC8A0` | none | `-009BC890`, `-009BC8C0` | state constructors, no reference found |

## 3. Layout

- **Task** (vtable `00D20910`, 26 slots). Overridden slots:

  | slot | function | role |
  | --- | --- | --- |
  | +0h | `009BDB90` | deleting destructor |
  | +4h | `009BD1D0` | debug draw |
  | +0Ch | `009BD3C0` | steer point with its valid byte |
  | +10h | `009BCFD0` | returns 3 |
  | +14h | `009BCFE0` | "moveonpath" |
  | +18h | `009BDB00` | debug text |
  | +3Ch | `009BCFC0` | `00E08F80` |
  | +40h | `009BD280` | still-valid |
  | +4Ch | `009BD2E0` | follow wait value, else base |
  | +54h | `009BD6E0` | cruise profile |
  | +58h | `009BD120` | stop |
  | +5Ch | `009BCFB0` | `&[00F8A534]`, unread |
  | +60h | `009BD380` | save |
  | +64h | `009BD990` | arm |

  The other slots are the base's (`007B40C0`, `0099C230`, ...).
- **task+3F8h**, the approach, built by `009BCC60`. Its head comes from `009BC780` -> `009F9CE0`, with
  ReferenceSpeed = tuning+37Ch Pilot/MoveTo. Fields:
  - +28h: leg weight;
  - +2Ch: altOffset;
  - +30h: travelAltOffset;
  - +34h..+3Ch: steer point;
  - +40h: the registry, view vtable `00D208F4`, also seen as task+438h with vtable `00D20900`;
  - +54h: the path state, vtable `00D208BC`;
  - +6Ch: the follow state, vtable `00D20AB8`, the same class as the moveto task's wingman state;
  - +104h: the circle state, vtable `00D208D8`.

  Approach vtables are `00D20838` (constructor), `00D208F8` (with states) and `00D20904` (in the task).
  Slot 0 of each is `009BC760` (the steer point), which the snapshot mislabelled as `Unwind@009bc760`.
- task+310h holds the current state. task+518h is the rule period, 1.0 (`[00D7A24C]`). task+51Ch is the
  rule timer, -U(0,1).
- ctl is approach+0Ch, i.e. unit+9D4h, the squadron block. It holds:
  - the pose at +FCh, refreshed through `00414DB0` when +C8h is clear;
  - the cruise channel at +37Ch: +38Dh lock, +380h current, +3A9h one-shot, +394h altitude, +3ADh dirty;
  - speed at +3A0h.
- desc is approach+8h, i.e. unit+538h: +18Ch speed, +268h turn radius.

## 4. The functions

**Construction** (`009BDBB0` -> `009BCE50`, ECX = the bot, unit = bot+50h). The steps run in this order:
1. `0099C6F0(bot, 6)`.
2. `009BCC60` on +3F8h:
   - `009BC780` draws approach+2Ch = U(0, tuning+540h MaxAltOffset) at `009BC7C0`, and sets +30h = 50.0;
   - the path state;
   - `009C2980` (arg 100.0) draws follow+74h = -U(0, 0.6) at `009C2A0A`;
   - `009BC960` draws circle+18h = (U(0,1) >= 0.5) at `009BC9AE`;
   - the three registry adds.
3. The vtables.
4. +518h = 1.0, and +51Ch = -U(0,1) drawn at `009BCECE`.
5. +310h = path when `007B8AD0`, else follow.
6. state->vtable[4]() (enter).
7. `009F9980`.

**Arm** (`009BD990`, RET 4) runs three steps: `009BD4D0(dt)` on the approach, then `009BD150(dt)`, then
the current state's vtable[0Ch](dt).

**Approach update** (`009BD4D0`):
- Without a cursor (no ctl, director+54h != `00E08F80`, or `0071BFF0(0)` null), it runs only
  `009FD050(1.0, &ctl+FCh)`.
- Otherwise:
  1. The steer point becomes the current point.
  2. It calls `007B1350`, which answers the last point only for follow mode 1.
  3. It runs `009FD050(planar |ctl - last|, &last)` (`00414C60`, cutoff 1e-10), or
     `009FD050(-1.0, &ctl+FCh)` when the path is not SIMPLE.
  4. While ctl+3ADh is clear, +2Ch = +30h = 0.
  5. If !(10.0 > y) and !(+28h > 0), the steer y becomes +2Ch + y.
  6. Otherwise the steer y becomes ctl+394h - +30h, replaced by `009BD400()->009FC260(unit, &steer,
     +28h)` when +28h > 0.
- `009FD050` (`009FD050-009FD0D8`): for 0 <= d < 3000, the manager registers the point (`009FC000`) and
  +28h = `00419010`(1200, 1.0, 3000, 0.0, d). Otherwise the manager drops the unit (`009FC1C0`) and +28h
  = -1.0.

**Rule** (`009BD150`):
- If dt < timer, the timer becomes timer - dt (`DE E9` FSUBP ST(1),ST(0), bytes read).
- Otherwise the timer becomes (period - dt) + timer and the rule asks `007B8AD0`: a leader takes path, a
  wingman follow. On a change it runs exit (vtable[8]), store, then enter (vtable[4]).
- The path and circle states inherit `007B3DB0`/`007B3DC0`, both a bare RET.

**Path tick** (`009BC8E0`), in order:
1. plan+26Ch = 1.
2. plan+2B4h = ctl+3A0h, plan+2B0h = 0, plan+2D8h = 1.
3. `009FB800(steer.y, 1.0)`.
4. `009F9E40(&steer)`.

**Circle tick** (`009BCA50`), in order:
1. plan+2B4h = desc+18Ch, with the same two stores as the path tick.
2. `009FB800(steer.y, 1.0)`.
3. `009FBB20(&(steer.x, steer.z), desc+268h, circle+18h, &out)`, result discarded.
4. If ctl->vtable[114h]() answers a director, `director->0071E430(00E08F80, 1)`, which **ends the
   moveonpath command**.
5. gun block+40h = tuning+670h (Angle_MoveTo).
6. `009FABE0(009A1A20(0099B630()))`.

**Cruise profile** (`009BD6E0`, slot +54h): section 1 and the header give the rule. The four MoveOn
flags are int32 tuning slots that the image compares with UCOMISS against 0.0 (LAHF / TEST AH,44h /
JNP), so only 0 and 80000000h count as "off". The reconstruction tests `bits & 7FFFFFFFh`.

**Still-valid** (`009BD280`, slot +40h) answers:
- 2 without ctl;
- 1 when `0071BE40` == `00E08F80` and `00521EA0(0071EB60())` equals `009BD0B0`. That function follows the
  cursor's path object +4 through vtable[28h] -> `007ACA30`, the path's entity;
- 0 otherwise.

**Stop** (`009BD120`, slot +58h): `009FD050(-1.0, &[00F87574])` drops the leg, then a tail jump to the
current state's exit.

Named, not reconstructed (presentation or persistence):
- `009BD1D0` and `009BC9F0` (debug draws) and `009BDB00` (debug text);
- `009BD380`/`009BD300` (save: altOffset, travelAltOffset, registry);
- the load path `009BDC10` -> `009BDA30` -> `009BCD50` -> `009BC7E0`;
- the destructors `009BD9D0`/`009BCB70`.

## 5. Reach in this installation

| mission | call | effect with the switch ON |
| --- | --- | --- |
| **BSM01** (`scripts/missions/bsm/bsm_01_stationed_at_pearl.lua`, 2024-07-13) | `:1877` and `:1880` `PilotMoveOnPath(P-40 Warhawk 01 / 02, PatrolPath1 / 2, PATH_FM_CIRCLE)`, each followed by `SquadronSetTravelAlt(..., 200)` | Both P-40s take the task. The leader circles its patrol path at squadron speed. Under CIRCLE `007B1350` answers invalid, so no leg weight applies, and the altitude is the point's y + altOffset, or 200 less travelAltOffset when the points lie below 10. Both planes are added to `Mission.JapFighterTrgGang`, so their positions change which targets the Japanese fighters chase, and with that the dogfight and AA numbers. |
| **JM06** (`scripts/missions/COTP-IJN/jm06.lua`, 2024-07-13) | `:118` `PilotMoveOnPath(Mission.Catalina, FindEntity("CatalinaPatrolPath"))` (follow mode defaults to 1, SIMPLE) | `PBY Catalina 01` flies the 12-point path once. Within 3000 m of the last point the leg weight is positive, and the glue's substitute leg altitude (no separation) is used. At the last point `007ADFAC` refuses the advance and `00836BF0` reports the arrival. What the director does with that answer belongs to the director, not this task, and was not read here. The task itself only keeps steering at the cursor's current point. Nothing else reads `Mission.Catalina`. |

USN13 and JM08 shipyard Mavis builds are dormant (no A9h build order in single player), and the shipyard
issue path `issue_shipyard_moveonpath_008454b4` is not covered by the glue. Every other run-list mission
has no plane `moveonpath` (scope doc 1.4).

**What changes when the switch is ON.**
- A plane that receives a `moveonpath` order gets attack_command_class `00E08F80`. The host's other arms
  key on their own class, so this replaces the task it had: the image builds one task per command.
- The plane then flies the path itself instead of drifting on its old task.
- Wingmen run the moveto task's follow body.
- `#mp`-keyed draws are added through `release_altitude_draw_00bd2f10`: construction makes four per
  plane, and one more through slot +54h at the install (its only activation in the host). Stream 0 is
  untouched.
- The squadron's sq_alt_394 / sq_alt_set / sq_lock_3a9 follow the cruise channel rule.

## 6. RNG draw sites (stream 1, ECX = 1)

| site | function | distribution | lands in | key |
| --- | --- | --- | --- | --- |
| `009BC7C0` | `009BC780` | U(0, tuning+540h MaxAltOffset) | approach+2Ch | `2c` |
| `009C2A0A` | `009C2980` (shared follow) | -U(0, [00CE3D30] 0.6) | follow+74h | `74` |
| `009BC9AE` | `009BC960` | U(0,1) >= 0.5 | circle+18h | `18` |
| `009BCECE` | `009BCE50` | -U(0,1) | task+51Ch | `51c` |
| `009BD700` | `009BD6E0` | U(0, tuning+364h TravelAltRandom) | task+428h (approach+30h) | `30` |
| `009BDAB8` | `009BDA30` (load path) | -U(0,1) | task+51Ch | not modelled |

The image draws these from one stream in tick order. A per-unit keyed host stream (`unit name + "#mp" +
key`) is a labelled substitution for that order. It is faithful in distribution only.

## 7. Verification and precision

- `tools/verify_report_calls.py reports/cc13_moveonpath_task_calls.json` checked 52 call rows with 0
  failures.
- All x87 here is short:
  - the timer sums;
  - y + altOffset;
  - ctl+394h - travelAltOffset;
  - CruisingAlt + 0.6 x TravelAltRandom, where 0.6 is a qword `[00CEFF98]` and the product is stored to
    a float slot before the FADD.
- Intermediates are computed in double and rounded at the image's float stores. At the main thread's
  24-bit control word (CC13_FPU_PRECISION.md) the image rounds intermediates to float instead, so the
  results are not bit-exact. `00419010` reuses `clamped_interpolate_00419010`.
- Host-side and cited, not re-derived:
  - the cursor's vtable[4] and its advance (`00836BF0`/`007ADD70`);
  - `009FB800`, `009F9E40`, `009FBB20`, `009FABE0`;
  - the follow state;
  - the converging-leg manager `009FC000`/`009FC1C0`/`009FC260` (a deconfliction over the manager's
    unit list; read only far enough to see the lock and the per-unit loop);
  - `0071E430`.
- Open: slot +10h returns 3 and slot +5Ch returns `&[00F8A534]`, and neither meaning is known. The
  cadence of slot +54h is settled by docs/CC13_TASK_PROFILE_CADENCE.md: the image dispatches it
  only on activation (`0099AEB3` when the task becomes the bot's head, `0099AE2B` when a task
  prepended over it retires), never per think.

## 8. Host glue for cc9 (suggested, not applied)

Applied in this worktree with `kMoveOnPathTaskBound = false`, built, reverted. `git apply --check` is
clean against origin/main `d8b34c2cc`. The pieces:
- `PilotMoveOnPath`/`NavigatorMoveOnPath` delivery installs the task on a plane (kind 0Fh) through the
  existing `bot_install_command_task_0099a170` (`ScriptOrderAttackCommandHost`) and stores the class.
- The units host constructs the task on the next think and ticks it next to the moveto task.
- Two accessors on the commands host give the cursor point with its height and `007B1350`.

Labelled substitutions are in the method comment: no leg manager, slot +54h run once at the install
(the image's `0099AEB3` activation, same think, before the arm; the host has no prepended tasks, so
the `0099AE2B` re-activation has no hook), the
moveto follow body, and the circle state kept as a record. Speed ctl+3A0h is
`retreat_squadron_speed_3a0`. ctl is the squadron slot (`squadron_slot_of`), falling back to the plane's
own position.

```diff
diff --git a/include/bsp/game_hosts_commands.hpp b/include/bsp/game_hosts_commands.hpp
index 7cbc5c851..a520c713c 100644
--- a/include/bsp/game_hosts_commands.hpp
+++ b/include/bsp/game_hosts_commands.hpp
@@ -440,6 +440,10 @@ public:
     // cursor->vtable[4h](&out, leg); leg < 0 is the sentinel both 009E5A50 and
     // 00836C89 pass, which stands for the cursor's own index.
     bool path_cursor_point(std::size_t unit_index, int leg, float& x, float& z) const;
+    // Packet cc13-moveonpath1: the same point with its height (009BD4D0 reads all
+    // three), and 007B1350's last point of a PATH_FM_SIMPLE (cursor+0Ch == 1) path.
+    bool path_cursor_point3(std::size_t unit_index, float out[3]) const;
+    bool path_final_point_007b1350(std::size_t unit_index, float out[3]) const;
 
     // 00836BF0-00836D66, the director step's `moveonpath` arm. `unit_radius` is
     // unit+9C8h and `turn_radius` is 0082E850 on the unit's class; the arm takes
diff --git a/include/bsp/pilot_order_bindings.hpp b/include/bsp/pilot_order_bindings.hpp
index 470ceda81..a8359222a 100644
--- a/include/bsp/pilot_order_bindings.hpp
+++ b/include/bsp/pilot_order_bindings.hpp
@@ -63,6 +63,11 @@ inline constexpr bool kMoveToFollowBound = true;
 // part 4 pairs (docs/PILOT_MOVETO_TASK.md). OFF: the
 // arrived leader's state stays the named record BotStateMoveToCircle::steer.
 inline constexpr bool kMoveToCircleSteerBound = true;
+// Packet cc13-moveonpath1 (docs/CC13_MOVEONPATH_TASK.md): 0099A170's `moveonpath`
+// arm (0099A24A -> 009BDBB0 -> 009BCE50) for a plane, and the kind-6 task's arm
+// 009BD990 (src/moveonpath_task_cc13.cpp). Committed OFF: the plane keeps the
+// task it already had while its director holds the path command.
+inline constexpr bool kMoveOnPathTaskBound = false;
 // 008A44EF. `retreat`.
 inline constexpr std::uint32_t kPilotOrderClassRetreat = 0x00E08F90u;
 // 008A4900. `land`, SCENE_COMMAND_TYPES row 22.
diff --git a/src/game_hosts_commands.cpp b/src/game_hosts_commands.cpp
index e70b09aee..277de8df3 100644
--- a/src/game_hosts_commands.cpp
+++ b/src/game_hosts_commands.cpp
@@ -3186,6 +3186,31 @@ bool GameCommandsHost::path_cursor_point(std::size_t unit_index, int leg,
     return true;
 }
 
+bool GameCommandsHost::path_cursor_point3(std::size_t unit_index, float out[3]) const {
+    const Impl& host = *impl_;
+    if (unit_index >= host.directors.size()) return false;
+    const GameDirector& director = host.directors[unit_index];
+    if (!director.path_built || director.path_points.empty()) return false;
+    const int count = static_cast<int>(director.path_points.size());
+    int index = director.path_cursor.index_08;
+    if (index < 0) index = 0;
+    if (index >= count) index = count - 1;
+    for (int k = 0; k < 3; ++k) out[k] = director.path_points[static_cast<std::size_t>(index)][k];
+    return true;
+}
+
+bool GameCommandsHost::path_final_point_007b1350(std::size_t unit_index, float out[3]) const {
+    // 007B1350: cursor+0Ch == 1 and a path with a positive count, else the valid
+    // byte is cleared.
+    const Impl& host = *impl_;
+    if (unit_index >= host.directors.size()) return false;
+    const GameDirector& director = host.directors[unit_index];
+    if (!director.path_built || director.path_points.empty()) return false;
+    if (director.path_cursor.follow_mode_0c != 1) return false;
+    for (int k = 0; k < 3; ++k) out[k] = director.path_points.back()[k];
+    return true;
+}
+
 bool GameCommandsHost::advance_path_cursor_00836bf0(std::size_t unit_index,
     float unit_x, float unit_z, float unit_radius, float turn_radius) {
     Impl& host = *impl_;
diff --git a/src/game_hosts_script_orders.cpp b/src/game_hosts_script_orders.cpp
index 7186a98a6..b972ebbbc 100644
--- a/src/game_hosts_script_orders.cpp
+++ b/src/game_hosts_script_orders.cpp
@@ -2732,6 +2732,19 @@ void GameScriptOrdersHost::session_route_path_order_message(void* entity,
                 "path \"%s\": 007ADC30 then answers `no legs` on the first state step "
                 "and the ship holds its heading", path_name.c_str());
         }
+        // Packet cc13-moveonpath1 (docs/CC13_MOVEONPATH_TASK.md): on a plane, the
+        // pilot bot's 0099A170 now reads `moveonpath` as the current command and its
+        // arm at 0099A24A builds the kind-6 task (009BDBB0). The units host
+        // constructs and ticks it (run_moveonpath_task_arm_009bd990).
+        if constexpr (bsp::kMoveOnPathTaskBound) {
+            if (units_.unit_is_kind_of(index, 0x0F)) {   // kUnitKindQueryPlane
+                ScriptOrderAttackCommandHost bot_host(units_, log_, bsp::kCommandMoveOnPath, 0u);
+                if (bsp::bot_install_command_task_0099a170(
+                        static_cast<std::uint32_t>(index + 1u), bot_host) != 0u) {
+                    units_.store_unit_attack_command_class(index, bsp::kCommandMoveOnPath);
+                }
+            }
+        }
     }
     });
 }
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index f8634b115..4b9ea270b 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -73,6 +73,7 @@
 #include "bsp/game_hosts_script_orders.hpp"
 #include "bsp/torpedo_release_spawn.hpp"
 #include "bsp/bot_tasks.hpp"
+#include "bsp/moveonpath_task_cc13.hpp"
 #include "bsp/game_hosts_ship_ai.hpp"
 #include "bsp/unit_hull_extents.hpp"
 
@@ -656,6 +657,10 @@ struct GameUnitSlot {
     // moveto; 009C3636 in 009C3570 sets it later.
     enum class MoveToTaskState : int { kNone = 0, kMoveTo = 0x47C, kFollow = 0x494, kCircle = 0x52C };
     bool moveto_task_installed{false};
+    // Packet cc13-moveonpath1: the kind-6 moveonpath task (009BCE50), kMoveOnPathTaskBound.
+    bool moveonpath_task_installed{false};
+    bsp::MoveOnPathTask moveonpath_task{};
+    unsigned long long moveonpath_ticks{0};
     bool moveto_stage_logged{false};   // cc9_moveto_arrival_end_command, one note per member
     MoveToTaskState moveto_state{MoveToTaskState::kNone};
     bool moveto_arrived_454{false};        // task+454h, cleared by 009C1D0A
@@ -29486,6 +29491,175 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         }
                     }
 
+                    // Packet cc13-moveonpath1 (docs/CC13_MOVEONPATH_TASK.md): the
+                    // kind-6 moveonpath task, constructed on the first think after
+                    // 0099A170's arm (0099A24A) and ticked through its vtable+64h
+                    // 009BD990. The squadron slot stands for ctl (unit+9D4h).
+                    // LABELLED substitutions: the converging-leg manager 009BD400
+                    // is not modelled (no separation; its altitude is the input);
+                    // slot +54h runs once, at the install (no re-activation hook);
+                    // the follow state is the moveto task's (same vtable
+                    // 00D20AB8); the circle state, unreachable live, is a record.
+                    void run_moveonpath_task_arm_009bd990(float dt) {
+                        struct Binding final : bsp::MoveOnPathTaskHost, bsp::MoveOnPathRandom {
+                            Binding(PlaneBinding& pb, GameUnitSlot* sq) : pb_(pb), sq_(sq) {}
+                            float uniform_00bd2f10(const char* key, float lo, float hi) override {
+                                return pb_.owner_.release_altitude_draw_00bd2f10(
+                                    pb_.unit_.row.name + "#mp" + key, lo, hi);
+                            }
+                            void leg_register_009fc000(const std::array<float, 3>&) override {
+                                pb_.owner_.record("BotPathLegManager::register", 0x009fc000u);
+                            }
+                            void leg_remove_009fc1c0() override {
+                                pb_.owner_.record("BotPathLegManager::remove", 0x009fc1c0u);
+                            }
+                            float leg_altitude_009fc260(const std::array<float, 3>& p,
+                                                        float) override {
+                                pb_.owner_.record("BotPathLegManager::altitude", 0x009fc260u);
+                                return p[1];
+                            }
+                            bsp::MoveOnPathApproachInputs approach_inputs() override {
+                                bsp::MoveOnPathApproachInputs in;
+                                const std::size_t i = pb_.unit_.process_index;
+                                float p[3] = {0.0f, 0.0f, 0.0f};
+                                in.cursor.present =
+                                    pb_.unit_.attack_command_class == bsp::kMoveOnPathCommandClass &&
+                                    pb_.owner_.commands.path_cursor_point3(i, p);
+                                in.cursor.waypoint = {p[0], p[1], p[2]};
+                                float f[3] = {0.0f, 0.0f, 0.0f};
+                                in.cursor.final_valid =
+                                    in.cursor.present && pb_.owner_.commands.path_final_point_007b1350(i, f);
+                                in.cursor.final_point = {f[0], f[1], f[2]};
+                                const GameUnitSlot& c = sq_ != nullptr ? *sq_ : pb_.unit_;
+                                in.ctl_position_fc = {c.motion.position[0], c.motion.position[1],
+                                                      c.motion.position[2]};
+                                in.ctl_alt_set_3ad = sq_ != nullptr && sq_->sq_alt_set;
+                                in.ctl_alt_394 = sq_ != nullptr ? sq_->sq_alt_394 : 0.0f;
+                                return in;
+                            }
+                            bool flight_leader_007b8ad0() override {
+                                return pb_.owner_.unit_is_flight_leader_007b8ad0(pb_.unit_.process_index);
+                            }
+                            float ctl_speed_3a0() override {
+                                return pb_.owner_.retreat_squadron_speed_3a0(pb_.unit_);
+                            }
+                            float desc_speed_18c() override { return pb_.unit_.plane_travel_speed; }
+                            float desc_radius_268() override { return pb_.unit_.plane_turn_circle_radius; }
+                            bool has_director() override { return true; }
+                            void enter_follow_009bed80() override {
+                                pb_.owner_.record("BotStateFollow::enter", 0x009bed80u);
+                            }
+                            void exit_follow_009bde40() override {
+                                pb_.owner_.record("BotStateFollow::exit", 0x009bde40u);
+                            }
+                            void tick_follow_009c1fd0(float step) override {
+                                pb_.run_moveto_follow_tick_009c1fd0(step);
+                            }
+                            void apply_plan(const bsp::MoveOnPathPlanCommand& c) override {
+                                GameUnitSlot& u = pb_.unit_;
+                                if (c.circles_009fbb20) {
+                                    pb_.owner_.record("BotStateMoveOnPathCircle::tick", 0x009bca50u);
+                                    return;
+                                }
+                                u.plane_desired_speed_2b4 = c.speed_2b4;   // 009BC912
+                                u.plane_trg_speed_corr_off_2b0 = 0;       // 009BC918
+                                u.plane_air_brake_mode_2d8 = 1;           // 009BC921
+                                ++u.plane_speed_commands;
+                                bsp::PlanePitchCommandInputs pin;         // 009BC938 009FB800
+                                pin.desired_altitude = c.pitch_altitude;
+                                pin.reference = c.pitch_gain;
+                                pin.unit_world_y = u.motion.position[1];
+                                if (pb_.owner_.lua.plane_globals_loaded()) {
+                                    const bsp::GameTuningBlock& g = pb_.owner_.lua.plane_globals();
+                                    pin.ceiling = g.dynamics_ceiling;
+                                    pin.climb_dist = g.pilot_general_climb_dist;
+                                    pin.drop_dist = g.pilot_general_drop_dist;
+                                }
+                                pin.class_climb_angle = u.plane_climb_angle_1ec;
+                                pin.class_drop_angle = u.plane_drop_angle;
+                                u.plane_commanded_altitude = c.pitch_altitude;
+                                u.plane_commanded_pitch = bsp::pitch_command_009fb800(pin);
+                                u.plan_state.pitch_target_2bc = u.plane_commanded_pitch;
+                                if constexpr (GameUnitsHost::Impl::kPitchCommandCallersBound) {
+                                    u.plan_state.pitch_mode_2d0 = 2;
+                                }
+                                if (c.heads_to_point_009f9e40) {          // 009BC944
+                                    u.plan_heading_2c0 = bsp::heading_command_009f9e40(
+                                        c.head_point[0], c.head_point[2],
+                                        u.motion.position[0], u.motion.position[2]);
+                                    u.plan_heading_2c0_written = true;
+                                    u.plan_heading_mode_2cc = 2;
+                                }
+                            }
+                            PlaneBinding& pb_;
+                            GameUnitSlot* sq_;
+                        };
+                        if (unit_.attack_command_class != bsp::kMoveOnPathCommandClass) {
+                            if (unit_.moveonpath_task_installed) {
+                                // 009BD120, slot +58h, as the task is replaced.
+                                Binding h{*this, owner_.squadron_slot_of(unit_.process_index)};
+                                bsp::moveonpath_task_stop_009bd120(unit_.moveonpath_task, h);
+                                unit_.moveonpath_task_installed = false;
+                            }
+                            return;
+                        }
+                        GameUnitSlot* sq = owner_.squadron_slot_of(unit_.process_index);
+                        Binding h{*this, sq};
+                        float max_alt_offset = 5.0f;
+                        if (owner_.lua.plane_globals_loaded()) {
+                            max_alt_offset = owner_.lua.plane_globals().pilot_general_max_alt_offset;
+                        }
+                        bool activated = false;
+                        if (!unit_.moveonpath_task_installed) {
+                            bsp::moveonpath_task_construct_009bce50(unit_.moveonpath_task, h, h,
+                                                                    max_alt_offset);
+                            unit_.moveonpath_task_installed = true;
+                            activated = true;
+                        }
+                        ++unit_.moveonpath_ticks;
+                        // Slot +54h, 009BD6E0, over the squadron's cruise block. The image
+                        // dispatches it only on activation (docs/CC13_TASK_PROFILE_CADENCE.md
+                        // sections 1-2): 0099AEB3 when the new task becomes the head, in the
+                        // same think, before the vt[64h] tick. LABELLED: the host has no
+                        // prepended tasks (0099AE2B), so there is no re-activation.
+                        if (activated && sq != nullptr && owner_.lua.plane_globals_loaded()) {
+                            const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
+                            const bsp::OrdnanceKindSet set{unit_.ordnance_mask};
+                            bsp::MoveOnPathCruiseInputs ci;
+                            ci.flight_leader = h.flight_leader_007b8ad0();
+                            ci.travel_alt_random_364 = g.pilot_move_to_travel_alt_random;
+                            ci.has_torpedo_007b93f0 = bsp::ordnance_has_torpedo_2bh(set);
+                            ci.has_depth_charge_007b94f0 = set.contains(0x2c);
+                            ci.has_general_bomb_007b9320 = bsp::ordnance_has_general_bomb_2ah(set);
+                            ci.level_bomber_0047b850 = park_bomber_class_0047b850(unit_);
+                            ci.small_turn_roll_0047b880 = !ci.level_bomber_0047b850;
+                            ci.torpedo_move_on_43c = g.pilot_torpedo_move_on_cruising_alt;
+                            ci.torpedo_cruising_alt_430 = g.pilot_torpedo_cruising_alt;
+                            ci.depth_move_on_4b4 = g.pilot_depth_charge_move_on_cruising_alt;
+                            ci.depth_cruising_alt_4a4 = g.pilot_depth_charge_cruising_alt;
+                            ci.level_move_on_454 = g.pilot_level_bomb_move_on_cruising_alt;
+                            ci.level_cruising_alt_444 = g.pilot_level_bomb_cruising_alt;
+                            ci.dive_move_on_4d4 = g.pilot_dive_bomb_move_on_cruising_alt;
+                            ci.dive_cruising_alt_4c0 = g.pilot_dive_bomb_cruising_alt;
+                            ci.small_plane_travel_alt_35c = g.pilot_move_to_small_plane_travel_alt;
+                            ci.large_plane_travel_alt_360 = g.pilot_move_to_large_plane_travel_alt;
+                            ci.cruise.locked = sq->sq_freeze_38d;
+                            ci.cruise.current = owner_.summary.simulated_seconds >= sq->sq_timer_expiry_380
+                                ? -1.0f : 1.0f;
+                            ci.cruise.one_shot = sq->sq_lock_3a9;
+                            const bsp::MoveOnPathCruiseResult cr =
+                                bsp::moveonpath_cruise_profile_009bd6e0(ci, h);
+                            unit_.moveonpath_task.ap.travel_alt_offset_30 = cr.travel_alt_offset_30;
+                            if (cr.writes_alt_394) {
+                                sq->sq_alt_394 = cr.alt_394;
+                                sq->sq_alt_set = true;
+                            }
+                            if (cr.clears_one_shot_3a9) sq->sq_lock_3a9 = false;
+                        }
+                        bsp::moveonpath_task_arm_009bd990(unit_.moveonpath_task, h, dt);
+                        owner_.done("BotTaskMoveOnPath::tick", 0x009bd990u);
+                    }
+
                     // 009C1FD0, the follow state's tick (vtable 00D20AB8 +0Ch), for
                     // the kind-7 task's wingman, packet cc9_pilot_moveto_task part 3.
                     // Its enter 009BED80 assigns the squadron's formation indices
@@ -30000,6 +30174,10 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                     run_moveto_task_tick_009c3950(elapsed);
                                 }
                             }
+                            if constexpr (bsp::kMoveOnPathTaskBound) {
+                                // Packet cc13-moveonpath1: vtable[64h] 009BD990.
+                                run_moveonpath_task_arm_009bd990(elapsed);
+                            }
                             if constexpr (GameUnitsHost::Impl::kDogfightTaskBound) {
                                 run_dogfight_task_arm_009ab1c0(elapsed);
                             }
```

## 9. Ledger and Ghidra

40 names were added with `python tools/bsp.py ledger add-name` (shard `config/names/009b0000.jsonl`,
script `local/mp1_names.py`). They were applied through `tools/ghidra_annotate.py --apply`. The prior
annotations are recorded in `local/ghidra-annotations-20261012T101150Z.json`, and the project was saved.
The window had no earlier ledger names. `009BD400` (`BSP_BotPathLegManager_Get`) is shared with the
moveto approach `009BEBA0` and `009FD050`.

## 10. Handoff

Done: the reconstruction, the names, the Ghidra repair and the glue check. Open for cc9:
- decide whether to wire the glue;
- model the leg manager `009FC260`, needed for JM06's final approach;

Scripts are in `local/`: `mp1_dis.py`, `mp1_pe.py`, `mp1_refs.py`, `mp1_ends.py`, `mp1_names.py`. The
glue diff is `local/mp1_glue.diff`.
