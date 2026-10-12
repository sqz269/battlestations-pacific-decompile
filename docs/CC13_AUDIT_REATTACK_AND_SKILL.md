# IJN01 strafe re-attack and USN04 dive-bomber skill (cc13-strafe2 audit for cc9)

A read-only audit for the gameplay lead (cc9). Branch `agent/cc13-strafe2`, base origin/main `210dabf10`.
Nothing here changes cc9's files. The two diffs in sections 1.5 and 2.6 were applied in this tree, built with
`./scripts/build.ps1` (pass, 2 of 2 tests), reverted, and checked with `git apply --check` against origin/main.
Ghidra was only read.

Status words:
- **read**: from the listing, the decompiler or a cited doc;
- **measured**: from a run in this tree;
- **build-tested**: the diff compiles with its switch OFF.

Nothing here is game-validated.

## Summary

| item | verdict | diff |
| --- | --- | --- |
| A. IJN01: the fighters stop attacking | **Host defect (harness reach).** The strafe task re-attacks as the image does. What stops is the player's orders: once the plane that carries a squadron's name dies, the host refuses `select`/`target` on that squadron. In the image the squadron entity stays alive and selectable until its last plane dies. | `kFusedSquadronSlotSelectableBound`, OFF. Measured ON: IJN01 kills 25 of 25 StrafeShips by 612.22 s and reaches phase 2. OFF kills 13 of 25 in 1200 s. USN04 control: gameplay identical. |
| B. USN04: the US dive bombers do not release | **Faithful to this installation.** The image gives the US carrier launches row 2, SPVeteran, as the host does. This installation's `robots.lua` zeroes that row's dive-aim gains. Zero gains do not make a release impossible, but they remove all aim feedback. | A harness option, `BSP_DIFFICULTY` (no gameplay change while unset). It is **not** a usable lever for USN04: difficulty 0 gives Elite, which is also a zero row. Difficulty 2 gives SPNormal, but Lexington sank in phase 1 at 233.76 s. |

**Corrections to `docs/CC13_WIN_READINESS.md`:**
- The IJN01 row ("each A7M dives once and never strafes") predates SQUADRON_LAND_TASK 5eu/5ex (2026-10-06).
- Its USN04 lever "difficulty 0" is wrong; see 2.2.
- The "1 of 33" dive release on USN04 is a Japanese Val, not a US aircraft.
- USN19's US carriers are not SPVeteran (2.5).

## A. IJN01: why the fighters stop attacking

### 1.1 What the image does after a pass (read)

- **The strafe task cycles by itself.** It goes gotowards -> aim -> goaway -> gotowards for as long as it is
  engaged (`009CC690`, SQUADRON_LAND_TASK 5bw.4).
  - `engaged` = the in-range latch `+448h`, or `squadron+370h == 2` with a target.
  - The flight leader's abandon test drives `+370h` to 1 on every think: `0099993C CALL 0099B740`, then
    `0099B774 CALL 007ED3F0(1)` (BOMBER_AFTER_TASK 6d). So a player's squadron is engaged only while it is in
    range of a live target.
  - When the target dies, the task falls back to moveto/follow and waits for a new order.
- **Nothing re-targets a player squadron by itself.** This was read again from `009F8470`
  (`BSP_SquadronAutoTarget_Tick`, live decompile):
  - It re-chooses only when the leader's party slot is AI (`+1B0h == 8` or `BSP_PartySlot_IsAiHeld`) and not
    player-held (`+184h`), or when the controller's `+3Ch`, `+3Dh` and `sq+370h == 2` all hold.
  - For a fighter, `009F8160` sets only `+3Dh`. So the player's next SetTarget is what moves a squadron on, as
    5er says.
- **The squadron entity outlives its first plane.**
  - `007BCAEB CALL 007F3970` removes a plane from `+3D0h` at its death. The next member becomes slot 0, the
    leader (VAL_SQUADRON_REGISTRY 2).
  - `007F3A16` tests `+3CCh`. Only at zero does `007F3A2A CALL 00926D90` kill the squadron (ENTITY_DEAD_FLAG 8).
  - Until then the PlaneSquadronGen keeps its `+5Ch/+5Dh/+60h/+5Eh` bytes.
  - It is kind 18h, not 0Fh, so the player's click passes `0064A057..0064A079` and `00645060`. Its
    `vtable[124h]` (`007EE670`) reads the four bytes of the **current** `+3D0h` plane.
  - So the player can keep ordering the surviving wing.

The call sites above are checked by `tools/verify_report_calls.py` on
`reports/cc13_audit_reattack_and_skill.json` (4 rows, 0 failed).

### 1.2 What the host does (measured)

The run is IJN01 with `cc9-ships38\local\s38_i1_p1.txt`, 24200/24000 frames at 0.05 s, with
`BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`. Logs are `local\st2_i1p1.log` (OFF) and
`local\st2_i1p1_A2on.log` (ON) in this tree. The summary script is `local\st2_strafe_sum.py`.

**The strafe task re-attacks as the image does.**
- OFF has 56 `gotowards -> aim` edges, 14 `goaway -> gotowards` re-passes and 45 `follow -> gotowards` re-engagements.
- All 35 `aim -> follow/moveto` exits fall within 1.5 s of a StrafeShip's death. That is the faithful
  "target dead, so not engaged" edge, not a stall.
- The same holds in the 2026-10-06 log (`cc9-lua46\local\l46ue_off_ijn01p1.log`): 105 aims, and all 76 such
  exits are at a death.

**What stops is the orders.** Each squadron's last accepted `target` comes at its leader's death:

| squadron | named plane dies | last accepted order | orders after it |
| --- | --- | --- | --- |
| A7M_2 | 135.10 s (B-17 gunner) | frame 2403 (120 s) | every `select A7M_2` refused: `0064A057..0064A079: the four byte filters fail` |
| A7M_3 | 199.96 s | frame 3605 | refused |
| A7M_4 | 258.46 s | frame 4807 | refused |
| A7M_1 | 341.99 s | frame 6801 | refused |
| A7M_5 | 434.57 s | frame 8409 | refused (the whole wing is dead) |

- In the host, `A7M_2` is the squadron's fused slot (`squadron_unit`, unit 295), which is also its wing-0 plane.
- The select path tests that plane's flags (`game_hosts_mission_frame.cpp` select arm,
  `unit_alive_and_visible(pick)`).
- The host already holds the squadron's `Dead` until the last plane dies (`kSquadronDeadOnLastMemberBound`): the
  log has `entity dead: unit=295 "A7M_2" died=135.10 s published=597.38 s`.
- The HUD observer is held in the same way (`kSquadronObserverLivenessBound`). Only the selection filters still
  read the dead plane.
- A7M_2's four wingmen therefore fly on with no order from 140 s until they die at 246-597 s.

**The OFF outcome on this main:** 13 of 25 StrafeShips dead, the last at 422.67 s, and `MissionPhase=1` at
1200 s.

On 2026-10-06 the same order file killed all 25 by 485.75 s with only 13 A7M plane deaths. On this main 21 of the
25 ordered planes die, mostly to cat-1 MG fire from the PTs and LSTs. So leaders now die early enough for this
gap to decide the row. Why A7M losses rose since 10-06 is not examined here; it is for the gunnery/AA lane.

### 1.3 A first attempt that is wrong, kept for the record

The first diff resolved the squadron's name to its live `+3D0h` plane and selected that plane. `00645060` rejects
it, because a plane is kind 0Fh (`006450DF`). That is correct: the image never selects a plane there.

That measured run moved nothing (`local\st2_i1p1_Aon.log`: 125 accepted orders, as OFF). The faithful object to
keep selectable is the squadron's own slot, which is what the diff below does.

### 1.4 The pair (measured)

`pair_diff.py local\st2_i1p1.log local\st2_i1p1_A2on.log`:

| row | OFF | ON |
| --- | --- | --- |
| StrafeShip deaths | 13 of 25 (last 422.67 s) | **25 of 25** (last LST1 at 612.22 s) |
| MissionPhase at 1200 s | 1 | **2** |
| accepted helm orders | 125 | 262 |
| strafe aims | 56 | 107 (A7M_2 alone 54) |
| deaths / hit records | 44 / 3943 | 53 / 6678 |
| controlled unit moved | (none) | A7M_2 5453.40 |

- **Mechanism.** Every added kill from 208.36 s on is by A7M_2's orphaned wing (`A7M_2|.-2/-3/-5`). It
  accepts the re-issued orders through the squadron's slot, and 007EEC50 chooses strafe `00E08F40` for its live
  members.
- No death row before 205 s moves; A7M_2's first refused order in OFF is at frame 2802 (140 s). The other moved rows follow from the
  changed fight.

**Control.** USN04 with `s39_u4_p1.txt` (45200/45000, 0.0222 s, jitter `20,3`), base against ON: `pair_diff`
exit 1, gameplay identical. Deaths, hits, releases and all three per-entity tables are identical. The only
change is the known `ship ai free: refills` noise.

### 1.5 The diff (`kFusedSquadronSlotSelectableBound`, OFF)

Files: `include/bsp/plane_squadron_host.hpp`, `src/game_hosts_hud.cpp`, `src/game_hosts_mission_frame.cpp`.

```diff
diff --git a/include/bsp/plane_squadron_host.hpp b/include/bsp/plane_squadron_host.hpp
index 6d1147e1f..82aa51367 100644
--- a/include/bsp/plane_squadron_host.hpp
+++ b/include/bsp/plane_squadron_host.hpp
@@ -257,6 +257,27 @@ class PlaneSquadronRegistry {
 
 PlaneSquadronRegistry& plane_squadron_registry();
 
+// docs/CC13_AUDIT_REATTACK_AND_SKILL.md item A. The image's squadron entity
+// (PlaneSquadronGen, 007F2C60) is not one of its planes: 007F3970 compacts +3D0h
+// at a plane's death and kills the squadron (007F3A2A -> 00926D90) only at
+// +3CCh == 0, so its +5Ch/+5Dh/+60h/+5Eh bytes pass 0043F080 / 00645091..006450AF
+// while any plane lives. This host fuses the squadron into its wing-0 plane
+// (squadron_unit), so once that plane dies the player's select and target on
+// the squadron are refused and its surviving wing is never ordered again.
+// ON: a fused squadron slot answers those four bytes with the squadron's
+// liveness. LABELLED: the dead plane's slot stays the controlled unit.
+inline constexpr bool kFusedSquadronSlotSelectableBound = false;
+
+// The record whose fused slot is `unit`, when the switch is on and the squadron
+// still holds a plane (+3CCh > 0); nullptr otherwise.
+inline const PlaneSquadronHostRecord* plane_squadron_live_fused_slot(std::size_t unit) {
+    if (!kFusedSquadronSlotSelectableBound || unit == kPlaneSquadronNoUnit) return nullptr;
+    for (const PlaneSquadronHostRecord& r : plane_squadron_registry().records()) {
+        if (r.squadron_unit == unit) return r.live_count() > 0 ? &r : nullptr;
+    }
+    return nullptr;
+}
+
 // cc11_land_retained_hook: source-only views for 009B33F0 -> 007EFB60 ->
 // 007EEE50. These borrow a particular old task's squadron; they do not resolve
 // current plane membership, retain/refcount native objects, or supply a bot
diff --git a/src/game_hosts_hud.cpp b/src/game_hosts_hud.cpp
index 19ab0a34f..6d97f0b2f 100644
--- a/src/game_hosts_hud.cpp
+++ b/src/game_hosts_hud.cpp
@@ -1498,7 +1498,8 @@ bsp::UnitSelectableInputs GameHudHost::Impl::selectable_inputs_00645060(std::siz
     bsp::UnitSelectableInputs in{};
     // 00645091..006450B3: +5Ch set, +5Dh/+60h/+5Eh clear. The units host
     // answers the four together (its 0043F080 test, the same four bytes).
-    const bool alive = units->unit_alive_and_visible(unit);
+    const bool alive = units->unit_alive_and_visible(unit)
+        || bsp::plane_squadron_live_fused_slot(unit) != nullptr;   // CC13 item A, OFF
     in.alive_5c = alive;
     // 006450B9..006450BF: [unit+54h] == [[game+18CCh]+28h]. SUBSTITUTION
     // (labelled, as game_hosts_ai.cpp does for the same read): the local
diff --git a/src/game_hosts_mission_frame.cpp b/src/game_hosts_mission_frame.cpp
index a8ef1ca33..b3114e9a2 100644
--- a/src/game_hosts_mission_frame.cpp
+++ b/src/game_hosts_mission_frame.cpp
@@ -3165,7 +3165,8 @@ bool GameMissionFrameHost::run_mission_frame_004e4a40(float raw_delta_in) {
                 const char* why = "";
                 if (pick >= host.units->count()) {
                     why = " (no created unit has that name)";
-                } else if (!host.units->unit_alive_and_visible(pick)) {
+                } else if (!host.units->unit_alive_and_visible(pick)
+                           && bsp::plane_squadron_live_fused_slot(pick) == nullptr) {
                     why = " (0064A057..0064A079: the four byte filters fail)";
                 } else {
                     accepted = bsp::game::hud_set_selected_unit_00647300(pick, reached);
@@ -3356,6 +3357,11 @@ bool GameMissionFrameHost::run_mission_frame_004e4a40(float raw_delta_in) {
                 const std::string picked = picked_row != nullptr ? picked_row->name : order.target;
                 const bsp::PlaneSquadronHostRecord* squadron = idx < count
                     ? bsp::plane_squadron_registry().find_by_member_unit(idx) : nullptr;
+                // CC13 item A (OFF): the fused slot's own plane has left +3D0h
+                // at its death; the squadron is still the record that names it.
+                if (squadron == nullptr && idx < count) {
+                    squadron = bsp::plane_squadron_live_fused_slot(idx);
+                }
                 std::vector<std::size_t> members;
                 if (squadron != nullptr) {
                     for (const std::size_t m : squadron->member_units) {
```

**Uncertainty and labels.**
- The controlled unit stays the dead plane's slot. The image's controlled unit is the squadron object. Anything
  that reads the controlled unit's own pose (camera, HUD) sees a dead plane. The pair shows no failure from this,
  but it was not checked line by line.
- The `vtable[124h]` answer already reads the current `+3D0h` plane (`game_hosts_hud.cpp` `unit_vtable_124`), so
  no change is needed there.
- `selectable_inputs_00645060` has other callers. On USN04 the switch moved nothing, and other rows were not run.
- The 2026-10-06 log is from a different main. The OFF/ON pair is from this tree's `210dabf10` build plus this
  diff only.

## B. USN04: which skill row the US dive bombers fly, and whether it can release

### 2.1 The row the image uses (read)

- `usn_19_coralus.lua` (this installation, 2024-08-26), lines 66-82: `Mission.SkillLevelOwn` is
  `SKILL_ELITE` at difficulty 0, `SKILL_SPVETERAN` at 1 and `SKILL_SPNORMAL` at 2.
  - `luamw_init.lua` 265-270 defines `SKILL_STUN` 0, `SKILL_SPNORMAL` 1, `SKILL_SPVETERAN` 2 and `SKILL_ELITE` 5.
- The script gives SkillLevelOwn to the US fleets (`SetSkillLevel(unit, Mission.SkillLevelOwn)` at 156 and 195)
  and to its own Yorktown launches (2335-2488).
- A launched squadron's bag takes `Skill = carrier+390h` (`006C50FC-006C5113`, SCENE_UNIT_SKILL 6.1). Pass B fans
  it out to every member (`007F225E-007F226E`).
- The approach captures the PilotBot row once, at `009F9D22` (`00F8A30C + skill*248h + 0Ch`).
- So in the image, at difficulty 1, every US carrier dive bomber flies row 2, SPVeteran.

### 2.2 This installation's rows (read)

`scripts/datatables/robots.lua` (2025-06-01 16:03:10; `robots__.lua.bak` 16:03:09 beside it), PilotBot:

| row | AimPrecDist | AimPrecMul | PullPlus | PullMinus | AimPitchRatio | MaxBrake |
| --- | --- | --- | --- | --- | --- | --- |
| SPNormal (1) | 70.0 | 0.3 | 0.018 | 0.025 | 3.0 | 0.5 |
| SPVeteran (2) | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| Elite (5), lines 1128-1135 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| `.bak` SPVeteran | 35.0 | 0.24 | 0.014 | 0.018 | 3.0 | 0.25 |

**So the readiness doc's lever is wrong.** Difficulty 0 gives the US side Elite, which is zeroed like SPVeteran.
Only difficulty 2 gives SPNormal.

### 2.3 The host picks the same row (measured)

`local\st2_u4base.log` (USN04, `s39_u4_p1.txt`, 45200/45000, 0.0222 s, jitter `20,3`):
- `air ops launch skill: ... Lexington-class01 ... skill=2` and `... Yorktown-class01 ... skill=2`;
- `SetSkillLevel` levels: 27 at 2 and 10 at 1;
- game+6ACh = 1.

The census script is `local\st2_dive_sum.py`.

| side | dive bombers | entered aimdive/aimglide | releases |
| --- | --- | --- | --- |
| US (skill 2) | 15 | 9 | **0** |
| Japanese (row 1: `Mission.SkillLevel` at difficulty 1, or the default; the Val SpawnNew read at 2590 authors no `Skill`) | 17 | 16 | 1 (`D3A Val #1.1\|.-4`) |

The "1 of 33" in the readiness doc and GUNNERY 181.1 is this Japanese Val. It is not a US aircraft.

### 2.4 Zero gains do not forbid a release; they remove the steering (read)

The release, `009C608C-009C6154` (`dive_bomb_aimdive_release_009c60f1`), has three gates:
- altitude below the floor `+A8h` = U(ReleaseAlt) = U(250, 300) for rows 2 and 5;
- the re-arm timer has run out;
- `|error| < 25.0` (`00CE3880`).

The pull-out edge is at `0.5 * +A8h` (`00D7A280`), so the window is 125-300 m.

The error `009C5C9B` is `gain * (cos(bearing) * planar - lead)`:
- `lead` runs from 0 at `+A8h + 100` up to AimPrecDist high up;
- `gain` runs from **1.0** at `+A8h + 100` to AimPrecMul high up.

The whole release window lies below `+A8h + 100`. So at any skill, the gate tests the true along-track distance
there. The zero row changes the gate only above the window.

What the zero row removes is the feedback. `009C5C9F-009C5CE5` (`dive_bomb_aimdive_steer_009c5c9f`):
- multiplies the error by PullPlus/PullMinus into the pitch slot `+29Ch`. With both 0, the stick is 0 on every
  tick steeper than 30 degrees;
- below 30 degrees it is a full -1 push.
- AimPitchRatio 0 also zeroes the power cap's x and the brake floor (GUNNERY 181).

So a row-2 dive bomber flies an uncorrected dive. It releases only if that line happens to cross within 25 m
along track inside the 125-300 m band. That arithmetic is the image's applied to this installation's row
(SQUADRON_LAND_TASK 5fi reaches the same conclusion from a pitch trace).

**Verdict:** not a host defect in the dive law as far as it was read. The open part is the flight model's
pull-through after the turndown hand-over (5fi), which decides where the uncorrected line lands.

### 2.5 The lever, measured, and USN13 / USN19 (read)

**Difficulty 2 on USN04** (`local\st2_u4d2.log`, the `BSP_DIFFICULTY=2` build of the diff below, same launch
form):
- game+6ACh = 2, and both US carriers launch at skill 1.
- But the Japanese strike is heavier at 2: 30 Vals, 8 releases, and torpedo task releases of 21 of 30.
- Lexington's hull dies at 233.76 s in phase 1, and the mission fails ("Game Over", `Mission.MissionStatus` at
  234.06 s).
- The 3 US dive bombers launched by then never reach aimdive.
- So difficulty 2 is not a usable lever on USN04 in this host (one run, jitter seed 3), and phase 3 at difficulty
  2 also needs both carriers at or below 20%.
- Changing the robots row would not be faithful: the `.bak` is not known to be retail, and the installed file
  is what this installation runs.

**USN13** (`usn_13_truk.lua` 243-254, the same mapping):
- `SkillLevelOwn` goes to the TF58 groups (296-410, carriers included), the Iowa group (1202) and the scripted
  `USBomba`/`USFighta` (1663/1703).
- At difficulty 1 that is SPVeteran, so the US dive release has the same zero row and the same verdict.

**USN19** (`COTP-USN\PRCPUS\prcp_14_okinawa.lua` 594-622):
- `Mission.SkillLevel` is numeric 1/2/3 by difficulty and goes to Japanese groups and the `DEs` (1504).
- No `SetSkillLevel` touches the US carriers.
- `prcp_14_okinawa.scn` authors no `Skill` on Essex, Bunker Hill, Hancock, Bataan, Enterprise, Yorktown or
  Intrepid. The only uncommented `Skill` lines are on AirField01/02, Shipyard01-03 and CommandStation_Center.
- So the carriers keep the constructor default 1 (SPNormal), and their launches fly row 1, the non-zero row.
  **The SPVeteran zero row is not USN19's dive-release problem.**
- Not run; this is read only. A USN19 log with `air ops launch skill` lines would confirm it.

### 2.6 The diff (harness option `BSP_DIFFICULTY`; nothing changes while it is unset)

This only exposes the briefing pick to the harness. It is offered for experiments, not as a fix.

```diff
diff --git a/src/game_hosts_mission.cpp b/src/game_hosts_mission.cpp
index 794e570d7..f2c92b88a 100644
--- a/src/game_hosts_mission.cpp
+++ b/src/game_hosts_mission.cpp
@@ -36,6 +36,7 @@
 
 #include <algorithm>
 #include <cstdio>
+#include <cstdlib>
 #include <map>
 #include <stdexcept>
 #include <string>
@@ -979,7 +980,24 @@ public:
     // profile would put its own SelectedDifficulty (profile+60h) here instead,
     // and the briefing's play action 0051B7B0 would overwrite it with the
     // player's pick. Switch off: the old 0.
-    std::int32_t chosen_difficulty() override { return kSkillLevelBound ? 1 : 0; }
+    // HARNESS OPTION, labelled: BSP_DIFFICULTY=0|1|2 stands in for that pick
+    // (0051B7B0 stores the widget index 0, 1 or 2). Unset or anything else
+    // keeps the reset value. USN04/USN13 give their own carriers SkillLevelOwn,
+    // which is SPVeteran at 1 and Elite at 0 (both zero DiveBombAimPrec* rows in
+    // this installation's robots.lua) and SPNormal only at 2.
+    // docs/CC13_AUDIT_REATTACK_AND_SKILL.md item B.
+    std::int32_t chosen_difficulty() override {
+        if (!kSkillLevelBound) return 0;
+        char* text = nullptr;
+        std::size_t length = 0;
+        if (_dupenv_s(&text, &length, "BSP_DIFFICULTY") == 0 && text != nullptr) {
+            const bool ok = text[0] >= '0' && text[0] <= '2' && text[1] == '\0';
+            const std::int32_t value = text[0] - '0';
+            std::free(text);
+            if (ok) return value;
+        }
+        return 1;
+    }
     // game+6ACh, stored at 0058BF37 (the record's own value) or 0058BF58 (the
     // player's). GetDifficulty's reader in the script host reads it back.
     void set_effective_difficulty(std::int32_t value) override {
```

## 3. Runs (this tree, `local\`; the logs go with the worktree)

| log | binary | row | result |
| --- | --- | --- | --- |
| `st2_i1p1.log` | base (`210dabf10`) | IJN01 + `s38_i1_p1` 24200/24000 | phase 1, 13/25 StrafeShips |
| `st2_i1p1_Aon.log` | first attempt (1.3) | same | identical in effect to OFF (the plane is rejected by `00645060`) |
| `st2_i1p1_A2on.log` | diff A, switch ON | same | phase 2, 25/25 by 612.22 s |
| `st2_u4base.log` | base | USN04 + `s39_u4_p1` 45200/45000, 0.0222 s, jitter `20,3` | phase 2; US dive 0/15, JP 1/17 |
| `st2_u4_A2on.log` | diff A, switch ON | same | pair_diff exit 1, gameplay identical |
| `st2_u4d2.log` | diff B, `BSP_DIFFICULTY=2` | same | failed at 234.06 s, Lexington dead at 233.76 s |

All runs used `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1` through `tools/run_game.ps1`.

## 4. Open

- Why A7M losses on IJN01 rose from 13 (2026-10-06) to 21 of 25 ordered planes on this main, mostly cat-1 MG fire
  from PTs and LSTs at 260-1050 m. This is for the gunnery/AA lane.
- USN04's US dive release at SPVeteran: the flight model's pull-through after the turndown hand-over (5fi). No
  faithful harness lever exists, because difficulties 0 and 1 are zero rows and 2 loses Lexington in phase 1.
- USN19: confirm the launch skill 1 with a run.
