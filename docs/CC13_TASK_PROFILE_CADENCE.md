# CC13 task cruise-profile cadence: vtable +54h (packet cc13-cadence2)

Addresses: 0099ACD0, 0099AE1C, 0099AE2B, 0099AE72-0099AEB3, 0099A4C0, 0099A5EB, 0099A170, 0099A020,
0099ADAA, 009998A0, 009C8920, 009B8C90, 009B3C60, 009C3650, 009CD020, 009D4A70, 009A2D60, 009A6500,
009AAF30, 009AE9D0, 009AF480, 007B7AF0, 009B9420, 009BD6E0, 0099B660, 00BD2F10

## Verdict

**The host calls four of the profiles too often.** The fix is one OFF switch,
`kTaskProfileActivationOnlyBound` (section 6).

In the image a task's `vtable[+54h]` "cruise profile" runs **once, when the task becomes the
head of the bot's task list**. The per-think slot is `vtable[+64h]` (`docs/CC13_BOT_TASK_CADENCE.md`
section 1). The host runs the profile bodies of the dive bomb, level bomb and land tasks on every
think. It runs the moveto profile's altitude gate on every 0.5 s refresh. That changes two things:

1. **Lua altitude overrides.** `SquadronSetTravelAlt` (`0089F550`) and `SquadronSetAttackAlt`
   (`008A22B0`) are called without `force` 428 times, in 86 files, in this installation's loose scripts
   (section 7). In the image such a value lasts until the second profile run after the call. That
   is normally the next task change, so in practice it lasts for the whole task. The host replaces
   it with the tuning default 0.5 s after the call.
2. **The dive profile's draw.** `009C899B` draws `U(0, 15)` on stream 1. The image draws it once
   per activation. The host draws it on every think of every dive-bomb flight leader
   (`kDiveProfileDrawBound`, ON), so the squadron's `+398h` changes every think.

With the switch OFF nothing changes. Status: exported and reconstructed. Build-tested with the
switch OFF (`./scripts/build.ps1`, section 6). The diff was applied in this tree, built and
reverted; the copy below passes `git apply --check` against origin/main (section 6). It is not
pair-tested or game-validated. `docs/CC13_TASK_PROFILE_CADENCE.calls.json` checks 21 call rows
with `tools/verify_report_calls.py`, and 0 fail.

## 1. Where vt[+54h] is dispatched, image-wide

**Direct references.** A rel32 scan of `.text` covers `E8`/`E9` and `0F 8x` Jcc. An
absolute-dword scan covers every section. Both were run for each of the 15 distinct `+54h` bodies
(`local/cad2_refs.py`):

| body | class | references |
| --- | --- | --- |
| `009A2D60` | closetoship | `.rdata 00D1F53C` only |
| `009A6500` | depthcharge | `00D1F78C` only |
| `009AAF30` | dogfight | `00D1FA04` only |
| `009B3C60` | land | `00D1FFF4` only |
| `009B8C90` | levelbomb | `00D20264` only |
| `009AE9D0` | dropkamikaze | `00D1FCC4` only |
| `009C8920` | divebomb | `00D20E6C` only |
| `009CD020` | strafe | `00D21134` only |
| `007B7AF0` | rocket | `00D0596C` only |
| `009AF480` | kamikaze | `00D1FD94` only |
| `009D4A70` | torpedo | `00D2141C` only |
| `009B9420` | type-F parked task | `00D206AC` only |
| `009BD6E0` | class `00D20910` | `00D20964` only |
| `009C3650` | moveto (`00D20B68`, `00D20BE0`) | `00D20BBC`, `00D20C34` only |
| `0099B660` | base (`RET`) | twelve vtables, plus the eleven `CALL 0099B660` that open the bodies above |

So every profile is reached only through its vtable slot. None is called directly.

**Indirect dispatch.**
- **`CALL/JMP [reg+54h]`, every encoding** (disp8, disp32, SIB): one hit in all of `.text`,
  `00C03B1B` in `__callthreadstartex` (CRT). MSVC does not dispatch tasks that way.
- **`MOV r,[r+54h]` then `CALL r`/`JMP r` within 40 bytes**: a linear capstone sweep of all of
  `.text` (`local/cad1_slotsweep.py`) finds 105 sites. It is not vacuous: it finds both known
  `+64h` calls in `009998A0`. Each of the 105 was assigned to its enclosing function
  (`local/cad2_group.py`):
  - **Two are on a task:** `0099AE2B` and `0099AEB3`, both in `0099ACD0 BSP_PilotBot_Tick`,
    both with `ECX` = the head of `bot+58h`.
  - **Two are in the plane code but not on a task:**
    - `007EE104`: `[edi+4]` was first tested with `vt[5Ch](29h)`, the entity `IsKindOf`, and its
      `vt[54h]` returns ST0 (`007EE106 FADD`). Task `+54h` bodies return nothing.
    - `007F4E04` (`BSP_PlaneSquadron_SEntityInitSlotA4`): `[esi+348h]`'s `vt[54h]` takes one
      argument (`007F4E02 PUSH 0`). Every task `+54h` body ends in a bare `RET` or a tail into
      `0099B740` (a bare `RET`).
  - **The other 101** are in GUI, HUD, renderer, shader cache, stream writer, coastline, scoring,
    ship AI and similar code. None of them loads from a `bot+58h` list.

**Coverage.** Direct references are complete (all sections). Indirect dispatch is complete for the
`CALL [r+54h]` forms. For the load-then-call form it is complete up to what a linear sweep can
decode: a sweep that desyncs in data inside `.text` could miss a site. **No third task dispatch was
found.**

## 2. When the two sites fire

From the listing of `0099ACD0` (disk bytes):

```
0099ADAA  (bot+7Ch) 00999E40 retire-all, 00999EE0 drain, 0099A170 install   ; no vt[54h]
0099ADC7  (list non-empty, unit+184h, 007B8AD0, head->vt[38h]()) 009BBFC0 parked task,
0099AE1C  00999F50 prepend, 0099AE2B CALL head->vt[54h]                    ; the parked task
0099AE6C  edi = count ? bot+58h[0] : 0
0099AE7E  0099A4C0 retire finished heads; when the list empties, 0099A5EB JMP 0099A170 installs
0099AE83  ebx = count ? bot+58h[0] : 0
0099AE94  if (edi != ebx && ebx) 0099AEB3 CALL ebx->vt[54h]                 ; the new head
0099AF1C  009998A0(head, acc)  ->  vt[64h] every think
```

A command task is built only when the list is empty (`docs/SQUADRON_LAND_TASK.md`, `0099A020`
appends). It is built by one of two routes:
- the tail of `0099A4C0`, after which `0099AEB3` fires because the head changed (from 0 or from
  the retired head); or
- `0099ADAA` under `bot+7Ch`. That route builds the task with **no** profile call: `edi` is
  already the new head at `0099AE6C`. Only `009C1BA8` sets `bot+7Ch`, from a moveto state that has
  run out of target (`docs/PILOT_BOT_TICK_GATES.md`).

A task also gets `0099AEB3` again whenever a task prepended over it retires. The prepending tasks
are the parked task `009BBFC0`, the strafe go-away (`009CB8B0`, `009CBB30`) and the rocket
(`007B5F10`, `007B6240`) through `00999F50`.

So in the image a profile runs **once per install, plus once each time its task resurfaces as
head**. It never runs per think.

## 3. What each profile does on a call (image) and what the host does

The `[task+404h]+37Ch` block is the squadron's countdown block. The countdowns are `+37Ch..+388h`
(`007F3BA0` counts them down) and the freeze bytes are `+38Ch..+38Fh`. The altitudes are `+394h`,
`+398h` and `+39Ch`; the locks are `+3A9h..+3ABh`, and `+3ADh` is a dirty byte
(`docs/LUA_BINDING_MISSION.md`). Each altitude channel works the same way:
`if (!freeze) { if (countdown < 0 && !lock) { alt = default; dirty = 1 } lock = 0 }`.

| profile | image body on one call (leader = `007B8AD0` true) | host call | host frequency |
| --- | --- | --- | --- |
| dive `009C8920` | leader: `+394h` = CruisingAlt (`+4C0h`); **draw** `U(0, [00CE5380]=15)` stream 1 at `009C899B`, `+ BeginAltRange/1` (`+4CCh`) -> `+398h`; `+39Ch` = `[00CE4C04]`; always: `+4B0h = max(+4B0h, AttackDist * +41Ch)`, `+4BCh = [00CFDEB0]`, `009C7A80(approach, 0, 0)` (`009C8A7C`), tail `0099B740` | `update_dive_bomb_approach` (`game_hosts_units.cpp` 3875; the `kDiveProfileDrawBound` block 3892-3952 and the 3992-3996 clamp) from 18859 | **every think** |
| level bomb `009B8C90` | leader: `+394h` = CruisingAlt (`+444h`), `+398h` = DropAlt (`+448h`); always: `+4B0h` clamp (`+44Ch`), `009B7C90` (`009B8D66`), tail `0099B740` | `lb_cruise_profile_009b8c90` (21586) from the arm (22033) | **every think** |
| land `009B3C60` | leader: `+394h` and `+398h` = Landing/CruisingAlt; no draw, no tail | `land_cruise_profile_009b3c60` (28384) from 29350 | **every think** |
| moveto `009C3650` | all units: **draws** `+460h = U(FollowDist/1, /2)` (`009C367C`) and `+424h` (approach+2Ch) `= U(0, TravelAltRandom)` (`009C36A3`), `+464h = 0`, `009BEBA0` refresh (`009C36BF`); leader: the `+394h` gate with Small/LargePlaneTravelAlt + 0.6 x TravelAltRandom | the gate inside `moveto_refresh_009beba0` (26411), from the install (26374) and the 0.5 s refresh (29386) | **gate every 0.5 s**; the two draws **never** |
| strafe `009CD020` | leader: the alt gate with Strafe/CruisingAlt; always `009CA3B0` glide seed (`009CD0AA`), `009CCED0` (`009CD0BB`), tail `0099B740` | not called. `st_glide_seed_009ca3b0` runs from the go-towards enter sites (20738, 20795), which the image also has | never |
| torpedo `009D4A70` | the engage-range clamp, the plan reset, the `+398h` row, tail `0099B740` | the install block (24308-24330); `read_control_block` constants | **once per install** (matches) |
| dogfight, closetoship, depthcharge, rocket, kamikaze, dropkamikaze, parked, `00D20910` | - | not hosted | - |

The `0099B740` tails are idempotent at any cadence (`docs/CC13_BOT_TASK_CADENCE.md`).

## 4. What the extra calls change, field by field

**The altitude gate (dive, level bomb, land, moveto).**
- In the host the squadron's script override is
  `sq_alt_394`/`sq_alt_398` with `sq_timer_expiry_380`/`_37c`, `sq_lock_3a9`/`_3aa` and
  `sq_freeze_38d`/`_38c`. The natives write it (`set_squadron_travel_alt_0089f550`,
  `set_squadron_attack_alt_008a22b0`, 15848-15874): countdown 0.5 s, `lock = 1`.
- **Image.** The next profile run (the activation that follows the order) sees `lock = 1`. It
  skips the write and clears the lock. The script value then holds **until the profile runs
  again**, which is the next head change.
- **Host.** The first think clears the lock. The first think 0.5 s after the call writes the
  default (dive, level bomb, land), or clears `sq_alt_set`/`sq_attack_alt_set` (moveto, dive).
- A forced call (`force = true`) holds in both.
- **Fields:**
  - `sq_alt_394`, `sq_alt_398`, `sq_lock_3a9`, `sq_lock_3aa`, `sq_alt_set` and
    `sq_attack_alt_set`;
  - the readers `db_begin_alt_ac` (3947, 3951), the moveto `cruise` (26450) and
    `st_glide_seed_009ca3b0`'s `alt`.

**The dive draw.**
- Draw site: `009C899B`, stream 1, `U(0.0, 15.0)` added to `BeginAltRange/1`.
- In the host: `release_altitude_draw_00bd2f10(name + "#dp", 0, 15)` (3923-3926), on every think
  of a flight leader with a dive task. It writes `sq_profile_398` (3941) whenever the gate is open,
  which is nearly always, and every member's `db_begin_alt_ac` copies it on that think (3951).
- With keyed streams (`BSP_GUNNERY_RNG_STREAMS=1`) the extra draws only advance the `#dp` key.
  Unset, every units-host draw shares one sequence, so each extra draw shifts every later draw in
  the process.
- If the pending audit7 and audit9 diffs land, `db_begin_alt_ac` also feeds the flyabove
  altitude, so the per-think jitter reaches the flight path (section 8).

**The clamps** (`+4B0h` on dive and level bomb): idempotent. Nothing lowers `lb_ap.in_range_b8` or
`db_in_range_b8` after install. Their only other writers are 21484 and 22510, both at install.
So the clamp's cadence makes no difference. The diff gates the level-bomb clamp only to stay
next to the body it belongs to, and leaves the dive clamp (3992-3996) as it is.

**Moveto: the host under-calls.**
- The image's single activation redraws approach+2Ch as `U(0, TravelAltRandom)` (50 in this
  installation's tuning), replacing the constructor's `U(0, GeneralMaxAltOffset)` (5).
- It also draws approach+68h `= U(FollowDist/1, FollowDist/2)`, which the host substitutes with
  50.0 (7138).
- Neither draw is in the host. This is not part of the switch: it would add draws, and it changes
  the moveto steering altitude by up to 50 m. It is listed as a follow-up (section 9).

## 5. Activation in the host

For the host, an activation is the class's install: `dive_bomb_task_installed` going true (22396),
`lb_install_009b7990` (21451, which also runs on a new target, as the image builds a new task),
the land install (11627) and `run_moveto_task_install_009c3000` (26344).

LABELLED:
- The host has none of the prepends (parked, strafe go-away, rocket), so it has no
  re-activation.
- A dive or land task that the `bot+7Ch` route rebuilds would have no activation in the image,
  but gets one here.
- The moveto flag is consumed by the install's own refresh (`009C1D9D`), one refresh earlier than
  the image's (`009C36BF`, inside the profile). The gate's result is the same.

## 6. The diff

`kTaskProfileActivationOnlyBound` (OFF), in `include/bsp/game_hosts_units.hpp` next to the
squadron-altitude switches. Four pending flags on `GameUnitSlot` are set at the four installs and
consumed by the first profile call:
- dive: the leader's draw and gate run only on a pending call;
- level bomb: the altitude block and the clamp run only on a pending call, and the `0099B740`
  part still runs every think (`0099993C`);
- land: the whole body runs only on a pending call;
- moveto: on a non-pending refresh, a set override is read as it is (`009BEBA0` just reads
  `+394h`), and the gate runs only for a pending flight leader.

Draws stay on the existing `#dp` key; no stream-0 draw is added.

Build: **pass.** Applied in this tree on top of origin/main `210dabf10`, then
`./scripts/build.ps1` (exit 0; bsp_game.exe linked, 2/2 tests passed, no warning in game_hosts_units), then reverted with `git checkout`.
`git apply --check` of the copy below against origin/main: **pass**. The diff is against
`210dabf10` and stacks with the pending audit7+audit9 and planner-roll diffs (section 8).

```diff
diff --git a/include/bsp/game_hosts_units.hpp b/include/bsp/game_hosts_units.hpp
index 44f37c5cd..a4452f30f 100644
--- a/include/bsp/game_hosts_units.hpp
+++ b/include/bsp/game_hosts_units.hpp
@@ -451,6 +451,14 @@ inline constexpr bool kDepartedWingmanTaskBlockBound = true;  // ON: pairs (docs
 inline constexpr bool kDiveProfileDrawBound = true;  // ON: pairs (docs/DIVE_BOMB_TASK.md)
 inline constexpr bool kSquadronAttackAltBound = true;  // ON: pairs (docs/LUA_BINDING_MISSION.md)
 inline constexpr bool kSquadronTravelAltBound = true;  // ON: pairs held (docs/LUA_BINDING_MISSION.md)
+// Packet cc13-cadence2 (docs/CC13_TASK_PROFILE_CADENCE.md). A task's vtable +54h
+// cruise profile runs only when the task becomes the bot's head: 0099AEB3 after
+// 0099A4C0 changed the head (the 0099A170 install), and 0099AE2B for the parked
+// task. No other vt[54h] dispatch on a task exists. True: the dive (009C8920),
+// level-bomb (009B8C90), land (009B3C60) and moveto (009C3650) profile bodies run
+// once per install; the dive draw is made then. False: they run per think (the
+// moveto gate per 0.5 s refresh), as before.
+inline constexpr bool kTaskProfileActivationOnlyBound = false;
 
 inline constexpr bool kPlaneRowPositionBound = true;  // ON: mechanism held, one premise miss (docs/CONTROLLED_UNIT.md)
 
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index f8634b115..f062503ca 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -529,6 +529,12 @@ struct GameUnitSlot {
     // attack order, which is what every aircraft in IJN01 and USN01 has.
     unsigned int attack_command_class{0};
     bool dive_bomb_task_installed{false};
+    // kTaskProfileActivationOnlyBound: set at each task install, cleared by the
+    // profile call it stands for (0099AEB3 -> task vt[54h]).
+    bool db_profile_activation_pending{false};
+    bool lb_profile_activation_pending{false};
+    bool moveto_profile_activation_pending{false};
+    bool land_profile_activation_pending{false};
     bsp::DiveBombState dive_bomb_state{bsp::DiveBombState::kNone};
     // Packet cc9_dogfight_task: the dogfight task (kind 2), installed when
     // 007EEC50 chose class 00E08F58. docs/DOGFIGHT_TASK.md.
@@ -3919,7 +3925,13 @@ struct GameUnitsHost::Impl {
                 }
             }
             GameUnitSlot& block = sq != nullptr ? *sq : slot;
-            if (leader) {
+            bool profile_runs = true;
+            if constexpr (kTaskProfileActivationOnlyBound) {
+                // 0099AEB3: 009C8920 runs once, when the task becomes the head.
+                profile_runs = slot.db_profile_activation_pending;
+                slot.db_profile_activation_pending = false;
+            }
+            if (leader && profile_runs) {
                 const float value = static_cast<float>(
                     static_cast<double>(release_altitude_draw_00bd2f10(
                         slot.row.name + "#dp", 0.0f, 15.0f))
@@ -11625,6 +11637,7 @@ bool GameUnitsHost::Impl::install_land_task_core_009b41c0(std::size_t unit_index
     unit.land_deck_plus_one = deck + 1u;
     unit.land_explicit_site = explicit_site;
     unit.land_task_installed = true;
+    unit.land_profile_activation_pending = true;    // 0099AEB3
     unit.land_installed_at = summary.simulated_seconds;
     // 009B32C2-009B32DD: a flight leader (007B8AD0) starts in moveto (land),
     // a wing member in follow (land); then the state's enter, vtable[4].
@@ -21451,6 +21464,7 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                     void lb_install_009b7990() {
                         using namespace bsp::level_bomb;
                         unit_.level_bomb_task_installed = true;
+                        unit_.lb_profile_activation_pending = true;   // 0099AEB3
                         unit_.lb_target_plus_one = unit_.command_target_plus_one;
                         bsp::LevelBombApproach& a = unit_.lb_ap;
                         a = bsp::LevelBombApproach{};
@@ -21585,7 +21599,14 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                     // profile re-runs is the arm's own update a moment later.
                     void lb_cruise_profile_009b8c90(bool leader) {
                         bsp::LevelBombApproach& a = unit_.lb_ap;
-                        if (leader) {
+                        bool profile_runs = true;
+                        if constexpr (kTaskProfileActivationOnlyBound) {
+                            // 0099AEB3: the body runs once, when the task becomes
+                            // the head; the 0099B740 below stands for 0099993C.
+                            profile_runs = unit_.lb_profile_activation_pending;
+                            unit_.lb_profile_activation_pending = false;
+                        }
+                        if (leader && profile_runs) {
                             GameUnitSlot* sq = owner_.squadron_slot_of(unit_.process_index);
                             if (sq != nullptr) {
                                 const double now = owner_.summary.simulated_seconds;
@@ -21608,7 +21629,7 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         // 009B8D47-009B8D69: +4B0h = max(AttackDist x +41Ch, +4B0h).
                         const float floor_b8 = lb_tuning(
                             &bsp::GameTuningBlock::pilot_level_bomb_attack_dist, 2000.0f) * a.speed_ratio_24;
-                        if (!(floor_b8 < a.in_range_b8)) a.in_range_b8 = floor_b8;
+                        if (profile_runs && !(floor_b8 < a.in_range_b8)) a.in_range_b8 = floor_b8;
                         // 0099B740: the leader authorises the attack (vt[38h] is
                         // 0099B710, true), and the squadron's +370h is what every
                         // member reads.
@@ -22394,6 +22415,7 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         }
                         if (!unit_.dive_bomb_task_installed) {
                             unit_.dive_bomb_task_installed = true;
+                            unit_.db_profile_activation_pending = true;   // 0099AEB3
                             // 009C7710 leaves +310h on the moveto/follow pair
                             // 007B8AD0 picks: 009C7777 CALL / 009C777C TEST AL,AL
                             // / 009C777E LEA ECX,[ESI+4F0h] / 009C7784 JNZ over
@@ -26345,6 +26367,7 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         if (unit_.attack_command_class != bsp::kPilotOrderClassMoveTo) return;
                         if (unit_.moveto_task_installed) return;
                         unit_.moveto_task_installed = true;
+                        unit_.moveto_profile_activation_pending = true;   // 0099AEB3
                         unit_.moveto_arrived_454 = false;       // 009C1D0A
                         unit_.moveto_timer_548 = 1.0f;          // 009C307C
                         unit_.moveto_stagger_54c = -0.5f;       // 009C3089, midpoint
@@ -26435,8 +26458,21 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 const double now = owner_.summary.simulated_seconds;
                                 const bool below_zero = !sq->sq_freeze_38d
                                     && now >= sq->sq_timer_expiry_380;
+                                bool profile_runs = true;
+                                if constexpr (kTaskProfileActivationOnlyBound) {
+                                    // 0099AEB3: the gate is 009C3650's, run once at
+                                    // the head change and by the leader only
+                                    // (009C36CA 007B8AD0); 009BEBA0 itself just
+                                    // reads +394h. LABELLED: consumed by the
+                                    // install's refresh (009C1D9D), one call early.
+                                    profile_runs = unit_.moveto_profile_activation_pending
+                                        && owner_.unit_is_flight_leader_007b8ad0(
+                                               unit_.process_index);
+                                }
                                 if (sq->sq_freeze_38d) {
                                     cruise = sq->sq_alt_394;             // 009C372A JNZ, return
+                                } else if (!profile_runs) {
+                                    cruise = sq->sq_alt_394;             // no profile call
                                 } else if (below_zero && !sq->sq_lock_3a9) {
                                     sq->sq_alt_set = false;             // the profile overwrites
                                 } else {
@@ -26446,6 +26482,9 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 ++owner_.summary.squadron_travel_alt_refreshes;
                                 sq->sq_last_cruise = cruise;
                             }
+                            if constexpr (kTaskProfileActivationOnlyBound) {
+                                unit_.moveto_profile_activation_pending = false;
+                            }
                         }
                         unit_.moveto_point[1] = cruise - unit_.moveto_alt_offset_2c;
                         const float dx = unit_.moveto_point[0] - unit_.motion.position[0];
@@ -28382,6 +28421,11 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                     // arm stands for the profile's call, the block is the
                     // squadron's slot, and the dirty byte +3ADh is not modelled.
                     void land_cruise_profile_009b3c60() {
+                        if constexpr (kTaskProfileActivationOnlyBound) {
+                            // 0099AEB3: once, when the land task becomes the head.
+                            if (!unit_.land_profile_activation_pending) return;
+                            unit_.land_profile_activation_pending = false;
+                        }
                         if (!owner_.unit_is_flight_leader_007b8ad0(unit_.process_index)) return;
                         GameUnitSlot* sq = owner_.squadron_slot_of(unit_.process_index);
                         if (sq == nullptr) return;
```

## 7. Missions likely to move with the switch ON

A census of the loose scripts in this installation (`scripts/`, `.lua`) for calls to
`SquadronSetTravelAlt(`/`SquadronSetAttackAlt(` that pass no literal `true`:
- 428 such calls in 86 files (664 live calls in all; 236 pass `true`; commented-out lines excluded, `local/cad2_altcensus.py`).
- In the pair missions:
  - **IJN01** (`ijn/ijn_1_pearl.lua`): 2 live (1009, 1011). Lines 1009-1011 are
    `NavigatorAttackMove(unit, strafe ship)`, then `SquadronSetTravelAlt(unit, 300)` and
    `SquadronSetAttackAlt(unit, 300)`; the other four are commented out.
  - **USN04** (`usn/usn_19_coralus.lua` 2182): 1, `SquadronSetTravelAlt(Mission.moviefisher,
    1100)`.
  - **JM08** (`prcpjm08.lua`): 6 non-forced (1075, 1076, 1169, 1170, 1267, 1268) and the forced 574 (the Mavis at 750).

The expected directions, ON against OFF:
- **IJN01.** The strafers ordered at 1009 keep 300 m travel and attack altitude for the whole task,
  instead of reverting to the defaults (moveto travel, dive BeginAltRange, strafe CruisingAlt)
  half a second after the order. Expect lower approaches, and a change in the AA exposure and
  attack timing of those aircraft.
- **USN04.** The movie aircraft holds 1100 m during its moveto, instead of reverting to
  Small/LargePlaneTravelAlt.
- **USN04 and any dive-bomb mission.** Each dive leader draws `#dp` once per install instead of
  every think. With keyed streams, only the begin altitude moves: it is fixed per task instead of
  re-drawn each think, within the same 15 m band. Unkeyed, the units-host sequence moves for every
  later draw.
- **Land.** Landing squadrons keep any non-forced script altitude through the land task. No pair
  mission script calls the setters in a landing phase that was traced, so little movement is
  expected beyond the draw effect.
- **JM08.** Its six non-forced calls hold for their task.

## 8. Dependence on the pending gunnery36-era landings

None of the three is on origin/main `210dabf10`:
- **The planner-roll fix** `kRollArmBaseNumeratorBound` (`docs/CC13_AUDIT_PLANNER_ROLL.md`): its
  diff applies cleanly, so it has not landed. It changes the planner roll arm (5636, 30437) and
  touches no profile field. **Its presence or absence makes no difference to the verdict.**
- **The audit7 plane-state diff** (`docs/CC13_AUDIT_PLANE_BOT_STATES_2.md`): applies cleanly,
  not landed.
- **The audit9 flyabove diff** (`docs/CC13_AUDIT_PLANE_BOT_STATES_3.md`): applies cleanly on top of
  audit7, and only there, so it is stacked on audit7 and not landed.
- **audit7+audit9 and this verdict.** Both make the flyabove altitude command use
  `db_begin_alt_ac` (ctl+398h), the value the dive profile writes. Without them, the per-think dive
  redraw only moves the begin altitude that aimdive/aimglide read. With them, it also moves the
  flyabove altitude every think. **The verdict holds either way.** With them the switch matters
  more for dive bombers.
- The diff in section 6 was checked to apply on origin/main alone and on origin/main plus
  audit7+audit9 plus the roll diff (section 6 result: pass on both).

## 9. Open items

- **Moveto activation draws** (section 4; done in section 10): `+424h`/approach+2Ch `U(0, TravelAltRandom)` and
  `+460h`/approach+68h `U(FollowDist/1, /2)`. This is a separate switch and pair.
- **Strafe profile** `009CD020`: not hosted. Its altitude gate would replace a script attack
  altitude at the strafe task's second activation (after a go-away prepend retires). The host
  never replaces it.
- **Torpedo** `009D4A70`: the host applies it at install, as the image does, but reads constant
  altitudes. A script override of `+394h`/`+398h` never reaches the torpedo host.
- `docs/BOT_TASK_STATES.md` line 113 says the two `+54h` sites "sit in an undefined region" and
  that their push/per-tick nature is unread. Sections 1 and 2 settle that: both are in
  `0099ACD0`, and both fire on activation.

## 10. The moveto activation draws (packet cc13-cadence3, `kMoveToActivationDrawsBound`)

This section closes the first open item of section 9, where the host makes too few calls. Status:
exported and reconstructed. The diff below is build-tested with the switch OFF, alone and on top
of the section 6 diff. It is not pair-tested or game-validated.

### 10.1 The draws, read from the listing

The moveto task class `00D20B68` (also `00D20BE0`) has `+54h` = `009C3650`. Its approach sits at
`task+3F8h` and is built by `009C1C30`. All draws below are `00BD2F10` with `ECX = 1`, so stream 1;
`[ESP]` is the low end and `[ESP+4]` the high end.

| # | site | when | field | distribution (this installation, `planeglobals.lua`) |
| --- | --- | --- | --- | --- |
| 1 | `009C1C8F` | construction (`009C1C30`) | approach `+2Ch` | `U(0, tuning+540h MaxAltOffset)` = `U(0, 5)` (line 378) |
| 2 | `009C1CEE` | construction | approach `+58h` | `-U(0, 0.5)` |
| - | `009C1D0D` | construction | approach `+68h` | constant `50.0` (`00CEB4D4`), no draw |
| 6 | `009C367C` | **activation** (`009C3650`, every unit) | `task+460h` = approach `+68h` | `U(tuning+368h, tuning+36Ch)` = `U(FollowDist[1], FollowDist[2])` = `U(20, 200)` (line 464) |
| 7 | `009C36A3` | **activation** (every unit) | `task+424h` = approach `+2Ch` | `U(0, tuning+364h TravelAltRandom)` = `U(0, 50)` (line 463) |
| - | `009C36B7` | activation | `task+464h` = approach `+6Ch` | `0.0` |

Draws 3-5 are the follow, circle and task constructor draws in `docs/PILOT_MOVETO_TASK.md`. After
draw 7, `009C36BF` calls the refresh `009BEBA0`. Only then does `009C36CA 007B8AD0` gate the
leader's `ctl+394h` write. The profile has no other `00BD2F10` call (`009C3650`-`009C394A`, from
disk bytes). `009BEBA0` itself makes no draw.

So in the image, after its one activation (section 2), a moveto task flies with approach `+2Ch`
drawn from `U(0, 50)`, not `U(0, 5)`, and with approach `+68h` drawn from `U(20, 200)`, not 50.

### 10.2 What the values feed

A linear sweep of `009BEB00`-`009BEE00` and `009C1000`-`009C3B00` (`local/cad3_reads.py`) for
`[reg+2Ch]` and `[reg+68h]`:
- **approach `+2Ch`:**
  - The refresh `009BEBA0` zeroes it at `009BEBB7` when `ctl+3ADh == 0`. `ctl+3ADh` is the
    squadron dirty byte. The block init `007F2BD0` sets it to 1 (`007F2C1A`), the Lua altitude
    natives clear it, and a profile write sets it again.
  - At `009BEBF1`-`009BEBFD` the steer point's altitude is `ctl+394h - approach+2Ch`.
  - `009BEDF5` is the follow state's own `+2Ch`, not the approach's.
- **approach `+68h`:**
  - Read at `009C23DD` (`FADD`) in `009C23B0`, the target-speed override:
    `reach = TurnCircleRadius + approach+68h`, and within reach of a faster target the moveto
    takes the target's speed.
  - The other stores in that range (`009C14B1`, `009C16CF` in `009BFEE0`, the follow state's
    geometry, and `009C1E57` in the sibling constructor `009C1DC0`, also `00CEB4D4`) belong to
    other objects or constructors.
  - Outside these ranges the approach's readers were not swept: `coverage: partial`.

### 10.3 The host now

- `moveto_alt_offset_2c` is draw 1, `U(0, pilot_general_max_alt_offset)` on key `#mt2c`, at
  install (`run_moveto_task_install_009c3000`). Nothing redraws it.
- `moveto_refresh_009beba0` steers at `cruise - moveto_alt_offset_2c` every 0.5 s.
- `moveto_target_speed_009c23b0` uses the constant `50.0f` for approach `+68h`
  (`kMoveToTargetSpeedOverrideBound`, ON).

### 10.4 Who sets the inputs

A scan of every `.lua` under the game root finds `TravelAltRandom`, `FollowDist` and
`MaxAltOffset` only in `scripts/datatables/planeglobals.lua`:
- line 378, `MaxAltOffset = 5`;
- line 463, `TravelAltRandom = 50`;
- line 464, `FollowDist = {20, 200}`.

That file's mtime is 2024-10-29, so it is this installation's value and not necessarily retail.
No mission script and no `robots.lua` or vehicle-class row sets them: `robots.lua` has only
`DogfightFollowDist`, a different key. So every mission uses the same values.

### 10.5 The diff

`kMoveToActivationDrawsBound` is OFF, and it is an `Impl` static next to
`kMoveToTargetSpeedOverrideBound`. It adds `moveto_follow_dist_68` (50.0) to `GameUnitSlot`, and
`009C23B0`'s reach reads it instead of the literal. That is the same value while the switch is OFF.

At the end of the install, the diff first sets `moveto_follow_dist_68 = 50.0` (`009C1D0D`). With the
switch ON it then makes draw 6 on key `#mt68`, then draw 7 on key `#mt424` (into
`moveto_alt_offset_2c`), sets `moveto_approach_6c = 0` and refreshes again (`009C36BF`). Each draw is
made once per install. Nothing draws on stream 0.

**Order dependency with section 6.**
- The draws use no pending flag. The host install stands for the activation, as section 5 labels
  it, and the draws come after the install's own refresh (`009C1D9D`), as in the image.
- With both switches ON, section 6's pending flag is consumed by that first refresh, so the
  travel-altitude gate runs there, one refresh early (labelled in section 6). The second refresh,
  added here, only reads `+394h`.
- With only this switch ON, the second refresh also applies the per-refresh gate. That gate is the
  one the host already runs every 0.5 s, so it adds nothing new.
- The two diffs touch disjoint hunks.

**Checks:**
- `./scripts/build.ps1` passes with this diff alone (exit 0, `bsp_game.exe` linked, 2/2 tests,
  no warning in `game_hosts_units`). It also passes stacked on the section 6 diff (exit 0, 2/2 tests, no warning in game_hosts_units).
- Both builds were reverted.
- `git apply --check` against origin/main `e7e2dd1e8` passes alone, and also after applying the
  section 6 diff.

```diff
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index f8634b115..361d3c8c2 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -667,6 +667,9 @@ struct GameUnitSlot {
     // fields, named by their offset in that object.
     std::size_t moveto_target_plus_one{0};   // approach+44h, the order's target
     float moveto_alt_offset_2c{0.0f};        // U(0, tuning+540h MaxAltOffset), 009C1C30
+    // approach+68h: 50.0 (00CEB4D4) from 009C1C30, redrawn U(FollowDist/1,
+    // FollowDist/2) by the activation's 009C3650 (kMoveToActivationDrawsBound).
+    float moveto_follow_dist_68{50.0f};
     float moveto_refresh_58{0.0f};           // countdown; period +54h = 0.5 (009C1CE0)
     float moveto_dwell_60{0.0f};             // the arrival dwell
     float moveto_distance_64{0.0f};          // planar distance, 009BEBA0
@@ -7138,6 +7141,14 @@ struct GameUnitsHost::Impl {
     // is within TurnCircleRadius + approach+68h (50.0) and faster than 007C47F0.
     // False: squadron+3A0h always (a record).
     static constexpr bool kMoveToTargetSpeedOverrideBound = true;  // ON: five rows identical (PILOT_MOVETO_TASK)
+// Packet cc13-cadence3 (docs/CC13_TASK_PROFILE_CADENCE.md section 10). The
+// moveto task's activation (0099AEB3 -> vt[54h] 009C3650) draws approach+68h =
+// U(FollowDist/1, FollowDist/2) at 009C367C and approach+2Ch = U(0,
+// TravelAltRandom) at 009C36A3, both stream 1, then refreshes (009C36BF).
+// True: the install makes both draws on "#mt" keys after the construction
+// draws and refreshes again. False: +68h stays 50.0 and +2Ch keeps its
+// construction draw U(0, MaxAltOffset).
+static constexpr bool kMoveToActivationDrawsBound = false;
     unsigned long long ts_override_calls{0}, ts_override_target{0};
     unsigned long long ts_override_within{0}, ts_override_applied{0};
     float ts_override_max_speed{0.0f};
@@ -7158,7 +7169,7 @@ struct GameUnitsHost::Impl {
         const bsp::OceanVec3& v = t.motion.linear_velocity;
         const float speed = static_cast<float>(std::sqrt(static_cast<double>(v.x) * v.x
             + static_cast<double>(v.y) * v.y + static_cast<double>(v.z) * v.z));
-        const float reach = unit.plane_turn_circle_radius + 50.0f;          // 009C23E3 FSTP
+        const float reach = unit.plane_turn_circle_radius + unit.moveto_follow_dist_68;  // 009C23E3 FSTP
         if (!(reach > unit.moveto_distance_64)) return base;                 // 009C23F8 JBE
         ++ts_override_within;
         if (!(static_cast<double>(speed) >
@@ -26387,6 +26398,29 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                             ? (unit_.moveto_arrived_454 ? GameUnitSlot::MoveToTaskState::kCircle
                                                         : GameUnitSlot::MoveToTaskState::kMoveTo)
                             : GameUnitSlot::MoveToTaskState::kFollow;   // 009C3097-009C30BF
+                        unit_.moveto_follow_dist_68 = 50.0f;            // 009C1D0D, 00CEB4D4
+                        if constexpr (GameUnitsHost::Impl::kMoveToActivationDrawsBound &&
+                                      bsp::kMoveToTaskTickBound) {
+                            // 0099AEB3 -> 009C3650, the task's activation right
+                            // after this install (the head changed). Draws 6 and 7
+                            // on stream 1 in the image; keyed here. Every unit,
+                            // before 009C36CA's leader gate. LABELLED: the host
+                            // install stands for the head change.
+                            float fd1 = 20.0f, fd2 = 200.0f, rnd = 50.0f;
+                            if (owner_.lua.plane_globals_loaded()) {
+                                const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
+                                fd1 = g.pilot_move_to_follow_dist_1;     // tuning+368h
+                                fd2 = g.pilot_move_to_follow_dist_2;     // tuning+36Ch
+                                rnd = g.pilot_move_to_travel_alt_random; // tuning+364h
+                            }
+                            const std::string& n = unit_.row.name;
+                            unit_.moveto_follow_dist_68 = owner_.release_altitude_draw_00bd2f10(
+                                n + "#mt68", fd1, fd2);                 // 009C367C, +460h
+                            unit_.moveto_alt_offset_2c = owner_.release_altitude_draw_00bd2f10(
+                                n + "#mt424", 0.0f, rnd);               // 009C36A3, +424h
+                            unit_.moveto_approach_6c = 0.0f;            // 009C36B7, +464h
+                            moveto_refresh_009beba0();                  // 009C36BF
+                        }
                         owner_.log.notef("  moveto task %s: installed kind 7, leader=%d state=%s "
                             "range=%.1f (009C3000)", unit_.row.name.c_str(), leader ? 1 : 0,
                             unit_.moveto_state == GameUnitSlot::MoveToTaskState::kFollow
```

### 10.6 Missions likely to move with the switch ON

Every mission in which a plane gets a kind-7 moveto task. That covers authored moveto orders,
`PilotMoveTo` and the moveto route of `0099A170`. USN04 does: its escort Zeros and script-spawned waves are ordered into moveto tasks (`docs/PILOT_MOVETO_TASK.md` lines 3 and 386). Other pair missions were not traced.
- **Steering altitude:** without a script altitude, the moveto flies `U(0, 50)` below its travel
  altitude instead of `U(0, 5)`. That is 22.5 m lower on average, and the squadron's members spread
  over 50 m.
- **Target speed:** the override's reach grows from `TurnCircleRadius + 50` to
  `TurnCircleRadius + U(20, 200)`, which averages 110. So a moveto that follows a faster unit
  switches to the target's speed earlier, on average.
- **Draws:** two keyed draws per moveto install (`#mt68`, `#mt424`). Unkeyed
  (`BSP_GUNNERY_RNG_STREAMS` unset), every later units-host draw shifts by two per install.

### 10.7 A related gap, not in this diff

`009BEBA0` zeroes approach `+2Ch` whenever `ctl+3ADh == 0`, which is the state after a Lua
`SquadronSetTravelAlt`/`SquadronSetAttackAlt` call (the natives clear `+3ADh`). In the image a
moveto under a script altitude therefore flies exactly at that altitude. The host always subtracts
`moveto_alt_offset_2c`, including under `sq_alt_set`. A fix would set the offset to 0 in
`moveto_refresh_009beba0` while the squadron's override is live, as one more switch. It was not
written, because it is not a draw.
