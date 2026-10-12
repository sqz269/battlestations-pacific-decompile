# Why `--menu-select JM01` never enters JM01 (cc13-jm01sel1)

Read-only investigation for cc9. It blocks the measurement of JM01's primary objective 4 (docs/SQUADRON_LAND_TASK.md,
the `JM01, 24000` row). Status of the fix below: **exported/reconstructed path read, fix build-tested, chunk
probe-tested against this installation's tree; not game-validated** (no game run was made, per the run-slot rule).

## Answer

**JM01 is not a row in this installation's mission tree.** The modded `scripts/datatables/missiontree.lua` wraps the whole
JM01 entry in a Lua block comment. The harness therefore looks up an id the tree does not contain. The image's
clamp turns the miss into (group 0, mission 0), which is TRN1. Group 0 builds no mission-detail page, so the run stops.

None of the four suspected causes applies:

1. Campaign order or index arithmetic: no. JM01 is simply absent. TRN1 comes from the miss clamp, not from an off-by-one.
2. Locked or tutorial gate: no gate is reached. The run stops because the clamped selection lies in group 0, which
   0058C092 rejects. That is a property of the training group, not a lock.
3. Profile unlocks: not involved. The host does not model prerequisites, and the commented JM01 row has
   `["prerequisites"] = {}` anyway (line 4603).
4. Id clash: no. `"JM01"` occurs once in the whole `scripts/` tree, inside the comment (line 4592). The active Pearl
   Harbor row is `IJN01` (line 1189), which has a different scene and script.

The earlier docs that call JM01 a "menu row" (docs/CC13_AUDIT_LUA51_FLIPS.md line 128, docs/CONSTRUCT_WORLD.md line
1064, docs/SCENE_TRAFFIC_RUNTIME.md line 106) read the record text without noticing the enclosing `--[[ ... ]]`. In
this installation JM01's scene and script exist on disk but no menu row reaches them.

## Evidence

### The run (cc9's log)

`J:\PROG\battlestations-pacific-decompile-cc9-lua51\local\l51_E0_j1.log` (2026-10-12 07:14; `l51_E1_j1.log` is the same):

```
31:    bsp_game milestone 2l, frames=24200 ... menu_select=JM01 mission_frames=24000 ...
556:   mission id JM01 is not in the tree; 005caaf0's tail clamps the selection to the first mission of the first group
557:   mission tree: 5 groups, 143 missions, 34 multiplayer entries
558:     group 0 Training grounds         missions=22 ids: TRN1 TRN2 TRN5 TRN3 ...
559:     group 1 IJN campaign             missions=30 ids: IJN01 IJN02 IJN03 IJN04 ...
560:     group 2 USN campaign             missions=36 ids: BSM01 BSM02 BSM03 BSM04 ...
561:     group 3 IJN DLC                  missions=27 ids: JM02 JM03 JM04 JM05 ...
562:     group 4 USN DLC                  missions=28 ids: PRCP01 PRCP02 PRCP03 PRCP04 ...
563:   mission tree selection: group=0 mission=0 id=TRN1 name=FE.mtree_trn01_name scene=universe/Scenes/TrainingGrounds/IJN_AirTraining.scn
641:   no mission-detail page was built for published group 0, so the page's footer command does not exist; the run stops here
24835: summary mission select=JM01 tree=1 groups=5 missions=143 selected=TRN1 list_page=8 detail=0 requested=0 step=Stopped
```

`requested=0` is `summary.requested_found`, and line 556 already names the miss. Group 3, the IJN DLC group, starts at JM02.

### This installation's tree

`I:\SteamLibrary\steamapps\common\Battlestations Pacific\scripts\datatables\missiontree.lua`, mtime 2025-06-02 19:08:58
(local time), SHA-256 `6B4780EF9AFB7DEB5368FA93DDBC5CF33398C50230994F2D09BEEA1CCB5FC4CB`. The install is modded; this file
is newer than the 2024-07-13 bulk.

- Lines 4582-4583: `{ -- IJN COTP` / `["groupName"] = "IJN DLC",`, which is group index 3.
- Line 4589: `---- PRCP/Vanilla MISSION START ----`.
- Line 4591: `--[[{ -- JM01 Attack on Pearl Harbor Vanilla`. The block comment opens here.
- Line 4592: `["id"] = "JM01",`. Line 4593: `["name"] = "Vanilla - Attack on Pearl Harbor",`.
- Line 4600: `["sceneFile"] = sceneFilePath.."IJN/JM/ijn_01_attack_on_pearl_harbor.scn",`.
- Line 4603: `["prerequisites"] = {},`.
- Line 4645: `},]]`. The block comment closes here.
- Lines 41-51: `sceneFilePath` is a file `local`. The realistic branch is commented out, so it is always
  `"universe/Scenes/missions/"`.
- Lines 1188-1197: the active `IJN01` "Attack on Pearl Harbor" row uses `IJN/ijn_1_pearl.scn`. That scene runs
  `scripts/missions/ijn/ijn_1_pearl.lua` (mtime 2024-08-26), which contains no `ShootedDownPlanes` or
  `PlaneSquadronLost`. IJN01 is therefore not a substitute for measuring JM01's primary 4.

The JM01 assets are present: `universe/scenes/missions/ijn/JM/ijn_01_attack_on_pearl_harbor.scn` (2024-07-13 08:27:00) and
`scripts/missions/ijn/JM/jm01.lua` (2024-07-13 08:26:24, `if Mission.ShootedDownPlanes == 5 then` at lines 1479 and 2938).
A search of every `.lua` under `scripts/` for `"JM01"` finds only line 4592.

### The image's path (static, disk bytes)

- 005CAAF0 `BSP_MissionTreeScreen_LoadTables` is the only reader of the tree. `Scripts/datatables/MissionTree.lua` is at
  00CF176C and has one imm32 reference, `PUSH` at 005CAB63. The only other `missiontree` text in the image is the
  "pls fill it in Missiontree.lua" help string.
- 005CAE8C `CALL 005C3470` (`BSP_MissionTreeScreen_FindMissionByName`, `__stricmp` over each record's id) stores
  {mission, group} at this+0Ch/+10h. This was checked with tools/verify_report_calls.py; see
  reports/cc13_jm01_menu_select_calls.json.
- 005CAEBB `CMP [ESI+0Ch], EBX` / `JGE` / 005CAEC0-005CAEC3 store EBX into +0Ch and +10h. A miss (mission index
  FFFFFFFFh) becomes (0, 0). The reconstruction is `src/mission_tree_data.cpp:313-319`.
- 0058C092-0058C09A in `BSP_MainMenu_BuildMissionDetailPage` (0058C010): `MOV EAX,[00E194D8]` / `ADD EAX,-1` /
  `CMP EAX,3`. Only published groups 1-4 get the detail page. The host mirrors this in
  `src/game_hosts_mission.cpp` (the `DetailPage` step, "no mission-detail page was built ... the run stops here").

So the host is faithful. Given this tree, the image would do the same thing with a shell request for JM01.

## What a real player can do

A player of this installation cannot pick JM01. It is not in the IJN DLC list. To play it, the player must restore the
row by removing the `--[[` on line 4591 and the `]]` on line 4645. Then they pick "Vanilla - Attack on Pearl Harbor",
the first entry of the IJN DLC list, and press continue. The image's normal path does the rest. The fix
reproduces exactly that table without touching the installed game.

## Proposed fix (harness only)

Add a harness option, `--mission-tree-append <lua file>`. Inside 005CAAF0's script step, after `run_script` has run
`Scripts/datatables/MissionTree.lua` and its VFS override chunks (00B69D40), the host reads the given file from disk and
runs it protected in the same Lua state. That is the position the image's own `<stem>_<suffix>.lua` override chunks take.
In this host `content_suffixes_` is empty, so those chunks cannot carry the row. The chunk restores the JM01 record as
the first entry of the "IJN DLC" group, which is where the commented block sits.

What it changes:
- It adds one parsed option and stores it as an absolute path, resolved before `--game-root` changes the directory, as
  `--screenshot` is.
- It adds one process global and its setter, plus one call in `game_main.cpp` and a log line, `mission-tree append <path>`.
- In `MissionTreeScriptBinding::run_script`, it runs the chunk after the existing `run_file` call. A missing file or a
  Lua error throws, so a bad chunk cannot silently fall back to TRN1.

What it does not change:
- Without the option, behaviour and log output are unchanged.
- The lookup 005C3470, the miss clamp, the group 1-4 gate at 0058C092, the briefing start, the load and every
  gameplay path are untouched. JM01 then passes these by the same route as JM02-JM08: group 3, detail page,
  `globals.continue`, 005922F0.
- It does not edit or overlay the installed `missiontree.lua`, and it does not add a VFS mount.
- It does not model profile prerequisites. The host never did, and JM01's prerequisites table is empty. Whether the
  image's list greys out a row with empty prerequisites was not checked. It cannot matter for an empty list.

Uncertainty: inserting at position 1 shifts JM02 and later rows in group 3 by one index for that run only. The image
indexes rows by position, so a profile's "current mission" pointer saved during such a run could name a different
row in a run without the chunk. The harness does not persist one. The chunk's `sceneFilePath` copies the arcade value.
If a future install re-enables the realistic branch (lines 44-49), the chunk must follow it.

### Diff (build-tested; reverted in this tree)

This copy was extracted from this doc and applied in `agent/cc13-jm01sel1` at origin/main 42b7967d3, where
`git apply --check` passed. `./scripts/build.ps1` passed both at the earlier base 708f64e2c and again at 42b7967d3. Both
builds rebuilt bsp_game.exe and passed the 2/2 ctest suites. The diff was then reverted. Only this doc and the
call-site JSON are committed.

```diff
diff --git a/include/bsp/game_hosts.hpp b/include/bsp/game_hosts.hpp
index ea8893b62..ed0efe548 100644
--- a/include/bsp/game_hosts.hpp
+++ b/include/bsp/game_hosts.hpp
@@ -282,6 +282,11 @@ struct GameExecutableOptions {
     // Off (0) keeps the lockstep frame every reference row is taken with.
     float frame_jitter_percent{0.0f};
     std::uint32_t frame_jitter_seed{1};
+    // --mission-tree-append <file>, harness only (docs/CC13_JM01_MENU_SELECT.md): a
+    // Lua chunk run in the mission-tree state after scripts/datatables/missiontree.lua
+    // and its VFS overrides, so a row this installation comments out can be restored
+    // without editing the installed game. Empty runs nothing.
+    std::string mission_tree_append;
     // --present-interval <vsync|immediate|native>, packet cc9_tooling_present_interval (env
     // BSP_PRESENT_INTERVAL, same words, only when the option is absent). Harness only: it
     // replaces the VSync word of the renderer request the host hands to device creation
@@ -786,5 +791,8 @@ const Win32PlatformState* active_platform_state() noexcept;
 void set_mission_frame_jitter(float percent, std::uint32_t seed) noexcept;
 // "<pct>[,<seed>]"; false on a malformed or negative value. Seed defaults to 1.
 bool parse_frame_jitter(const char* text, float& percent, std::uint32_t& seed) noexcept;
+// --mission-tree-append: the chunk 005CAAF0's script step runs after missiontree.lua.
+// Defined in src/game_hosts_mission.cpp. Empty disarms.
+void set_mission_tree_append(std::string path);
 
 }  // namespace bsp::game
diff --git a/src/game_hosts.cpp b/src/game_hosts.cpp
index 6918ac6bc..210007d48 100644
--- a/src/game_hosts.cpp
+++ b/src/game_hosts.cpp
@@ -444,6 +444,25 @@ bool GameExecutableOptions::parse(int argc, char** argv, std::string& error) {
                 return false;
             }
             menu_select.assign(argv[++index]);
+        } else if (std::strcmp(argument, "--mission-tree-append") == 0) {
+            // Harness only (docs/CC13_JM01_MENU_SELECT.md): a Lua chunk run in the
+            // mission-tree state after missiontree.lua, as 00B69D40's override chunks
+            // are. Resolved before --game-root changes the current directory.
+            if (index + 1 >= argc) {
+                error = "--mission-tree-append needs a path";
+                return false;
+            }
+            const char* input = argv[++index];
+            const DWORD required = GetFullPathNameA(input, 0, nullptr, nullptr);
+            if (!required) { error = "cannot resolve the mission-tree append path"; return false; }
+            mission_tree_append.resize(required);
+            const DWORD length = GetFullPathNameA(input, required, mission_tree_append.data(),
+                nullptr);
+            if (!length || length >= required) {
+                error = "cannot resolve the mission-tree append path";
+                return false;
+            }
+            mission_tree_append.resize(length);
         } else if (std::strcmp(argument, "--mission-frames") == 0) {
             // Milestone 2f: how many in-mission frames of 004e4a40 to run once
             // the load has finished and 004da6c0 has written game state 0Dh.
diff --git a/src/game_hosts_mission.cpp b/src/game_hosts_mission.cpp
index 794e570d7..15ffe2cea 100644
--- a/src/game_hosts_mission.cpp
+++ b/src/game_hosts_mission.cpp
@@ -36,6 +36,8 @@
 
 #include <algorithm>
 #include <cstdio>
+#include <fstream>
+#include <iterator>
 #include <map>
 #include <stdexcept>
 #include <string>
@@ -62,6 +64,10 @@ struct MissionFrameJitter {
 };
 MissionFrameJitter g_mission_frame_jitter;
 
+// Harness only (--mission-tree-append, docs/CC13_JM01_MENU_SELECT.md): an absolute
+// path to a Lua chunk run after missiontree.lua. Empty runs nothing.
+std::string g_mission_tree_append;
+
 // Harness only (packet cc9_tooling_1): the in-mission frame the crash record in
 // src/game_main.cpp reads (-1 before the first mission frame), and the --crash-test
 // trigger (-1 disarmed). Neither is read by the simulation.
@@ -459,6 +465,23 @@ public:
         owner_.scripts.runtime().run_file(owner_ptr_->storage_lua_38(), script_path_);
         owner_.summary.tree_script_ran = true;
         owner_.log.implemented("MissionTree::run_script", "00b69d40");
+        if (!g_mission_tree_append.empty()) run_append();
+    }
+
+    // Harness only: the --mission-tree-append chunk, read from disk (not the VFS)
+    // and run protected in the same state, after the file and its override chunks,
+    // so it sees the MissionTree global the image's own overrides would.
+    void run_append() {
+        std::ifstream file(g_mission_tree_append, std::ios::binary);
+        if (!file) throw std::runtime_error("cannot open the mission-tree append chunk");
+        const std::string text((std::istreambuf_iterator<char>(file)),
+            std::istreambuf_iterator<char>());
+        GuiLua51Host appender(*owner_ptr_->storage_lua_38());
+        std::string error;
+        if (!appender.execute_archive(text, error)) {
+            throw std::runtime_error("mission-tree append chunk failed: " + error);
+        }
+        owner_.log.notef("mission-tree append chunk ran: %s", g_mission_tree_append.c_str());
     }
 
     // 00B67980 then 00B67800, and the reader 004425C0 over the result.
@@ -2002,4 +2025,8 @@ void set_mission_frame_jitter(float percent, std::uint32_t seed) noexcept {
     g_mission_frame_jitter.state = seed;
 }
 
+void set_mission_tree_append(std::string path) {
+    g_mission_tree_append = std::move(path);
+}
+
 }  // namespace bsp::game
diff --git a/src/game_main.cpp b/src/game_main.cpp
index 76eb24488..5bc782ceb 100644
--- a/src/game_main.cpp
+++ b/src/game_main.cpp
@@ -982,6 +982,7 @@ int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR comman
             " | --order <command>=<x>,<z>]"
             " [--order-unit <name>] [--ai-drive <unit>=<throttle>,<rudder>]"
             " [--order-frame N] [--mission-frame-seconds S] [--frame-jitter <pct>[,<seed>]]"
+            " [--mission-tree-append <lua file>]"
             " [--present-interval <vsync|immediate|native>]"
             " [--trajectory-csv <path>]"
             " [--screenshot <path>] [--screenshot-frame N]"
@@ -1204,6 +1205,10 @@ int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR comman
     } else {
         log.notef("frame jitter off");
     }
+    bsp::game::set_mission_tree_append(options.mission_tree_append);
+    if (!options.mission_tree_append.empty()) {
+        log.notef("mission-tree append %s", options.mission_tree_append.c_str());
+    }
     // Packet cc9_tooling_present_interval: printed only when an override is in force, so a
     // run without it logs exactly what it did before.
     if (options.present_interval >= 0) {
```

### The chunk

Generate it from the installation, so that it cannot drift from the commented text. Save this as
`local\make_jm01_row.ps1`:

```powershell
# Writes the --mission-tree-append chunk that restores JM01, the row this installation's
# missiontree.lua comments out (lines 4591-4645). Usage: make_jm01_row.ps1 <game root> <out.lua>
param([string]$GameRoot, [string]$Out)
$tree = Join-Path $GameRoot 'scripts\datatables\missiontree.lua'
$lines = [System.IO.File]::ReadAllLines($tree)
if ($lines[4590] -notmatch '^\s*--\[\[\{ -- JM01 ' -or $lines[4591] -notmatch '"JM01"' -or $lines[4644] -notmatch '^\s*\},\]\]') {
    throw "missiontree.lua lines 4591-4645 are not the commented JM01 row"
}
$head = @(
    '-- JM01 row from scripts/datatables/missiontree.lua lines 4591-4645, comment markers removed.',
    '-- sceneFilePath is a local of missiontree.lua; its arcade value (line 51) is repeated here.',
    'local sceneFilePath = "universe/Scenes/missions/"',
    'local group',
    'for _, g in ipairs(MissionTree["missionGroups"]) do',
    '    if g["groupName"] == "IJN DLC" then group = g end',
    'end',
    'assert(group, "IJN DLC group not found")',
    'for _, m in ipairs(group["missions"]) do assert(m["id"] ~= "JM01", "JM01 already present") end',
    '-- The commented row is the group''s first entry (line 4582 opens the group).',
    'table.insert(group["missions"], 1, {')
$body = $lines[4591..4643]
$tail = @('})')
[System.IO.File]::WriteAllLines($Out, [string[]]($head + $body + $tail))
```

Then run, for example:
`bsp_game.exe --game-root <root> --menu-select JM01 --mission-tree-append <abs>\local\jm01_row.lua --mission-frames 24000 ...`.
The log should show `mission-tree append chunk ran`, `group 3 IJN DLC missions=28 ids: JM01 JM02 ...`, and
`mission tree selection: group=3 mission=0 id=JM01`, with no "not in the tree" line.

Probe evidence for the chunk (not a game run): a throwaway C probe linked against the build's `bsp_lua511.lib` ran this
installation's `missiontree.lua` and then the generated chunk in one Lua 5.1.1 state. `DoFile` was stood in by `dofile`
under the game root, which loads `MultiGlobals.lua`. Results:

```
before group 3 n=27: JM02 (universe/Scenes/missions/COTP-IJN/PRCPIJN/prcpijn_02_force_z.scn) JM03 JM04
after  group 3 n=28: JM01 (universe/Scenes/missions/IJN/JM/ijn_01_attack_on_pearl_harbor.scn) JM02 JM03
second append refused: ...cc13jm_jm01_row.lua:9: JM01 already present
```

The other groups are unchanged at 22/30/36/28, which matches the log's 143 missions plus JM01.

## Open

- No run was made, by the run-slot rule. Whether JM01's scene loads and plays on the host, and whether primary 4
  completes, is for cc9's run with this option.
- The `content_suffixes_` override mechanism (00BDEF90 `<stem>_<suffix>.lua`) stays empty in this host. If it is ever
  populated from the image's mounts, an installed `missiontree_<suffix>.lua` would load the same way without the option.
