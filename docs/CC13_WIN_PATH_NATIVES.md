# Win-path natives: TrulyDead, IsInBorderZone, SquadronForceGunnerMode, SquadronSetReleaseAlt (cc13-natives1, 2026-10-12)

Addresses: 00899500 008AF020 0089F950 008A24E0 009C8920 009C7A80 00928C80 0071C4F0 00489C40 0064A1E0 00647080
007F2C60 0088EFF0 0088F1D0 008132C0 008135C0 00813660 008198A0 0093BD80 00826754 00826B2E

Packet `cc13_win_path_natives`. `docs/CC13_WIN_READINESS.md` section 1.1 lists TrulyDead, IsInBorderZone and
SquadronForceGunnerMode as unbound on the win paths. The cc13 lead added SquadronSetReleaseAlt
(`docs/CC13_SMALL_DIFFS_BATCH_2.md` item 1). For each native this document gives:
- its image semantics and the host's state;
- the script call sites in this installation, with file mtimes;
- one suggested binding diff behind its own switch, committed OFF.

SetFailure / GetFailure were dropped from this packet because cc9's ships worker is binding them. The findings are kept
as notes in appendix A, with no diff. An earlier version of this document (96d315bbc) carried a SetFailure diff; it is
withdrawn.

All names are hypotheses, not recovered symbols. Ghidra was read-only and nothing was renamed. This packet changes no
host source file. The diffs are **suggestions** for the owners of the Lua and units hosts (cc9).

**Status of each diff:** reconstructed and build-tested. None is game-validated or run in a mission.
- Each diff was applied alone to the worktree, built with `./scripts/build.ps1` (exit 0, 2/2 tests) and reverted.
- Each copy below passes `git apply --check` against origin/main. The four diffs also apply in sequence (1, 2, 3, 4).
- Diff 4 also composes with `docs/CC13_SMALL_DIFFS_BATCH_2.md` diff 5.1 in either order, and the addendum 4a applies
  on top of both. The three together build (exit 0).
- `reports/cc13_win_path_natives_calls.json`: 32 call rows, 0 failed (`tools/verify_report_calls.py`).

How to read a diff: save the block to a file and run `git apply <file>`. While a diff's switch is false, its new native
row stays out of the `handled` list, so the log keeps the row's UNIMPLEMENTED line.

## 0. Common binding shape

All of these are `luaMW_*` bodies in segment 56. Each one opens a `luakod` call frame on the Lua state in ECX and returns
`00B66400` (the result count) with a plain `RET`. Argument 0 is resolved through `00888AA0`, the table's `Ptr`
handle. That call has no null or class test anywhere in these five functions.

The host's dispatcher `binding_trampoline` (`src/game_hosts_lua.cpp`) routes a row when its address matches and its
switch is true, and records anything else as unimplemented. An unimplemented row returns no value, which the script
reads as `nil`. Every diff below follows the GetSubmarineOnSurface pattern. Each one adds:
- a row flag;
- an entry in the `handled` list;
- a dispatch `if`;
- a `run_*` method;
- summary counters;
- one summary line.

## 1. TrulyDead 00899500

**Image** (00899500-00899695 RET):
1. Argument 0 (`00B677E0`, 008995EF), then `GetByName("Ptr")` at 00899609. The key is 00CFAD08 (`50 74 72 00`).
2. `ToUserdata` (`00B662D0`) at 00899618.
3. `PushBoolean(pointer == null)` at 0089964A.

There is no class test. **Only `00928C80` nulls `Ptr`.** It is the on-killed slot `vtable[80h]`, reached from the
kill list's drain (`docs/CONSTRUCT_WORLD.md` 1431, `src/native_unit_killed_base.cpp:87`). `Dead` comes earlier, from
`00929800` through `vtable[7Ch]` (`docs/ENTITY_DEAD_FLAG.md`). So TrulyDead is false for a unit that is `Dead` but
still in the world, such as a sinking hull, and true once the drain has released it.

**Host state.**
- The host's self tables carry `Ptr` = the entity id as light userdata (`game_hosts_lua.cpp:3916`, `7457`).
- The units host only **records** `00928C80` (`game_hosts_units.cpp:31581`, the kill-depth drain), so `Ptr` is
  never nulled.
- A faithful binding therefore answers **false** for every host entity. For an unbound row the script reads `nil`.

**The hook it needs (not in the diff):** `GameMissionLuaHost` should null the self table's `Ptr`, as `00928C80` does
through `00B67530(self, "Ptr", nullptr)`, wherever the units host records `Entity::on_killed_lua_self_00928c80`. Today
that is only the kill-depth drain, and the comment on `kSunkShipKillDepthBound` (`game_hosts_units.cpp:5842`) records
that no host wreck has reached KillDepth, so even that hook would not fire in any measured mission.

**Call sites:**

| script (mtime) | line | expression |
| --- | --- | --- |
| `COTP-IJN/PRCPIJN/JM06.lua` (2024-07-13) | 954 | `if not TrulyDead(Mission.Yamato) then` (the completion names the Yamato) |
| `COTP-IJN/PRCPIJN/JM05.lua` (2024-07-13) | 2687, 2828 | `if unit.Dead and not TrulyDead(unit) then`, to choose the fail entity |

**Which missions move.** None. `not nil` and `not false` take the same branch at all three sites. The only effect is
`MissionLuaNative::TrulyDead` moving from the unimplemented list to the implemented one.

```diff
diff --git a/include/bsp/game_hosts_lua.hpp b/include/bsp/game_hosts_lua.hpp
index 00c3ba0b3..02fc860b4 100644
--- a/include/bsp/game_hosts_lua.hpp
+++ b/include/bsp/game_hosts_lua.hpp
@@ -286,6 +286,12 @@ inline constexpr bool kLuaSetSubmarineDepthLevelBound = true;  // ON: pairs held
 // (+1204h + +1200h) / 3.0, with no class test. True: route the row to
 // run_get_submarine_on_surface_008942c0. False: unimplemented (nil, read as false).
 inline constexpr bool kLuaGetSubmarineOnSurfaceBound = true;  // ON: JM06 24000 pair (docs/LUA_BINDING_MISSION.md)
+// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 2). 00899500
+// TrulyDead(entity) reads argument 0's `Ptr` field (00B67800, key 00CFAD08) as
+// userdata (00B662D0) and pushes `Ptr == null` (00B66450). Only the on-killed slot
+// 00928C80 (vtable[80h], from the kill list's drain) nulls `Ptr`. True: the same
+// read on the host's self tables. False: unimplemented, nil.
+inline constexpr bool kLuaTrulyDeadBound = false;
 
 // Packet cc9_set_air_base_slot_count (docs/LUA_BINDING_MISSION.md, "SetAirBaseSlotCount,
 // 008963E0"). SetAirBaseSlotCount(entity, n) resizes the air-ops block's 58h slot array
@@ -710,6 +716,8 @@ struct GameMissionLuaSummary {
     unsigned long long sub_surface_true{0};
     unsigned long long sub_surface_unseeded{0};
     unsigned long long sub_surface_unresolved{0};
+    unsigned long long truly_dead_calls{0};         // packet cc13_natives1, 00899500
+    unsigned long long truly_dead_true{0};
     unsigned long long slot_count_calls{0};
     unsigned long long slot_count_resized{0};
     unsigned long long slot_count_unresolved{0};
@@ -1280,6 +1288,8 @@ public:
     int run_set_submarine_depth_level_00893f40(lua_State* state, int argument_count);
     // Packet cc9_lua_submarine_on_surface, under kLuaGetSubmarineOnSurfaceBound.
     int run_get_submarine_on_surface_008942c0(lua_State* state, int argument_count);
+    // Packet cc13_natives1, under kLuaTrulyDeadBound.
+    int run_truly_dead_00899500(lua_State* state, int argument_count);
     // Packet cc9_set_air_base_slot_count, under kLuaSetAirBaseSlotCountBound.
     int run_set_air_base_slot_count_008963e0(lua_State* state, int argument_count);
     // Packet cc9_lua_add_air_base_stock, under kLuaAddAirBaseStockBound.
diff --git a/src/game_hosts_lua.cpp b/src/game_hosts_lua.cpp
index 7272a6905..ad060104e 100644
--- a/src/game_hosts_lua.cpp
+++ b/src/game_hosts_lua.cpp
@@ -487,6 +487,8 @@ int binding_trampoline(lua_State* state) {
     // Packet cc9_lua_submarine_on_surface.
     const bool sub_surface_row = kLuaGetSubmarineOnSurfaceBound
         && dispatch_row.address == 0x008942c0u;
+    // Packet cc13_natives1.
+    const bool truly_dead_row = kLuaTrulyDeadBound && dispatch_row.address == 0x00899500u;
     // Packet cc9_set_air_base_slot_count.
     const bool slot_count_row = kLuaSetAirBaseSlotCountBound
         && dispatch_row.address == 0x008963e0u;
@@ -568,6 +570,7 @@ int binding_trampoline(lua_State* state) {
         || unlimited_air_row
         || in_formation_row || leave_formation_row || travel_alt_row || border_zone_row
         || untouchable_row
+        || truly_dead_row
         || attack_alt_row
         || ready_row
         || launch_row || generate_row || spawn_new_row || scoring_play_time_row
@@ -734,6 +737,9 @@ int binding_trampoline(lua_State* state) {
     if (sub_surface_row && !host->error_replay()) {
         return host->run_get_submarine_on_surface_008942c0(state, argc);
     }
+    if (truly_dead_row && !host->error_replay()) {
+        return host->run_truly_dead_00899500(state, argc);
+    }
     if (squadron_speed_row) {
         if (!host->error_replay()) host->run_squadron_set_speed_0089f780(state, argc);
         return 0;
@@ -5587,6 +5593,31 @@ int GameMissionLuaHost::run_get_submarine_on_surface_008942c0(lua_State* state,
     return 1;
 }
 
+// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 2). 00899500
+// TrulyDead(entity), 00899500-00899695 RET, ECX the Lua state: 008995EF
+// 00B677E0 (argument 0), 00899609 00B67800 GetByName("Ptr", 00CFAD08), 00899618
+// 00B662D0 ToUserdata and 0089964A 00B66450 with `pointer == null`; one result.
+// There is no class test. The image nulls `Ptr` only in 00928C80 (the on-killed
+// slot vtable[80h], reached from the kill list's drain), so a unit that is `Dead`
+// but still in the world (a sinking hull) answers false.
+// SUBSTITUTIONS (labelled): the host's self tables carry the entity id as light
+// userdata and nothing in the host nulls it yet (the units host records 00928C80),
+// so every entity table answers false until that hook exists; a non-table
+// argument, where the image's GetByName has no table to index, answers true.
+int GameMissionLuaHost::run_truly_dead_00899500(lua_State* state, int argument_count) {
+    ++summary_.truly_dead_calls;
+    void* ptr = nullptr;
+    if (argument_count >= 1 && ::lua_type(state, 1) == LUA_TTABLE) {
+        ::lua_getfield(state, 1, "Ptr");
+        ptr = ::lua_touserdata(state, -1);
+        ::lua_settop(state, argument_count);
+    }
+    if (ptr == nullptr) ++summary_.truly_dead_true;
+    ::lua_pushboolean(state, ptr == nullptr ? 1 : 0);
+    log_.implemented("MissionLuaNative::TrulyDead", "00899500");
+    return 1;
+}
+
 // Packet cc9_set_submarine_depth_level. 00893F40 SetSubmarineDepthLevel(entity, level):
 // argument 0 through 00888AA0, argument 1 as an integer; a request for 1 becomes 0 when
 // +122Ch (periscopeState) is 2 or +1214h (the periscope node) is null; then 008528B0.
@@ -7972,6 +8003,9 @@ void GameMissionLuaHost::report_mission_script_state() {
         "cc9_lua_submarine_on_surface)", kLuaGetSubmarineOnSurfaceBound ? 1 : 0,
         summary_.sub_surface_calls, summary_.sub_surface_true, summary_.sub_surface_unseeded,
         summary_.sub_surface_unresolved);
+    log_.notef("summary mission script truly dead bound=%d calls=%llu true=%llu (00899500, "
+        "Ptr == null, packet cc13_natives1)", kLuaTrulyDeadBound ? 1 : 0,
+        summary_.truly_dead_calls, summary_.truly_dead_true);
     log_.notef("summary mission script air base slot count bound=%d calls=%llu resized=%llu "
         "unresolved=%llu (008963E0 -> 006C7E20, packet cc9_set_air_base_slot_count)",
         kLuaSetAirBaseSlotCountBound ? 1 : 0, summary_.slot_count_calls,
```

## 2. IsInBorderZone 008AF020

**Image** (008AF020-008AF257 RET):
1. `008889C0` (008AF11E) tests whether argument 0 is an entity handle.
   - If it is, `00888AA0` (008AF153) gives the translation `+FCh/+100h/+104h`, refreshed through `00414DB0` when
     `+C8h` is clear.
   - Otherwise `00888760` (008AF1AC) reads a vector.
2. `0071C4F0` runs with ECX = `[00E188A8]` (008AF1E2..008AF1EF).
3. Only when the point is **outside** the map, `00489C40` (008AF1FF) returns the zone record, and `00B664B0`
   (008AF20C) pushes its `+4h`. That field is the side, 0 or 1 after the twelve `004C7150` calls.
4. Inside the map nothing is pushed, so the script reads `nil`.

A side of 0 is a true value in Lua, so **any** point outside the map counts as "in a border zone".

**Host.** The row is unimplemented, so the script reads `nil`, the inside answer. Everything the native needs is
already in the host:
- `bsp::point_outside_world_map_0071c4f0` (`world_map_bounds.hpp`);
- `bsp::border_zone_at_point_00489c40` (`plane_retreat_task.hpp`);
- `border_bounds_` / `border_zones_`, which `set_world_border_zones` fills for GetClosestBorderZone.

**Call sites:**

| script (mtime) | line | effect of a non-nil answer |
| --- | --- | --- |
| `COTP-IJN/PRCPIJN/JM06.lua` (2024-07-13) | 996 | in `luaJM6CheckCarrier` (`MissionPhase == 2`, 760): fail with `ijn06.obj_missionfailed_hornetexited` and `StepPhase(99)` |
| `COTP-IJN/PRCPIJN/JM05.lua` (2024-07-13) | 4398 | the retreat event is finished (`Mission.Event.Finished`, CountdownCancel) |

**Which missions move.** Only a run in which Lexington (JM06) or a JM05 event unit actually leaves the map bounds.
The bound row can only add a JM06 fail; it cannot help the win. The prediction is no change on the measured JM06
runs, but this is unmeasured: the Lexington position in phase 2 has not been logged against `MultiPlayMapSizes`.

```diff
diff --git a/include/bsp/game_hosts_lua.hpp b/include/bsp/game_hosts_lua.hpp
index 00c3ba0b3..836e9ceb0 100644
--- a/include/bsp/game_hosts_lua.hpp
+++ b/include/bsp/game_hosts_lua.hpp
@@ -343,6 +343,14 @@ inline constexpr bool kLuaHitAttackerPlayerIndexBound = true;  // ON: identity p
 // False: unimplemented, neutral.
 inline constexpr bool kLuaSpawnNewIdQueriesBound = true;  // ON: pairs (docs/LUA_BINDING_MISSION.md)
 inline constexpr bool kLuaClosestBorderZoneBound = true;  // ON: pairs (docs/LUA_BINDING_MISSION.md)
+// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 3). 008AF020
+// IsInBorderZone(entity | position): an entity handle (008889C0) gives the pose
+// translation +FCh..+104h, anything else the 00888760 vector; then 0071C4F0 on the
+// game singleton [00E188A8] and, only when the point is outside the map, the side
+// (+4h) of 00489C40's zone record through 00B664B0. Inside the map nothing is
+// pushed. True: run it on the zones set_world_border_zones keeps. False:
+// unimplemented, nil (the inside answer).
+inline constexpr bool kLuaIsInBorderZoneBound = false;
 
 // Packet cc9_hit_rate_limit (docs/LUA_BINDING_MISSION.md, "The hit-callback rate
 // limit"). 00988510 keys a map at this+168h (009882F0 / 00499030) by (victim,
@@ -680,6 +688,9 @@ struct GameMissionLuaSummary {
     // Packet cc9_get_closest_border_zone.
     unsigned long long border_zone_calls{0};
     unsigned long long border_zone_missing{0};
+    unsigned long long in_border_zone_calls{0};     // packet cc13_natives1, 008AF020
+    unsigned long long in_border_zone_outside{0};
+    unsigned long long in_border_zone_no_zone{0};
     unsigned long long forced_recon_calls{0};
     unsigned long long forced_recon_units{0};
     unsigned long long forced_recon_unresolved{0};
@@ -1304,6 +1315,8 @@ public:
     int run_squadron_set_attack_alt_008a22b0(lua_State* state, int argument_count);
     // Packet cc9_get_closest_border_zone, under kLuaClosestBorderZoneBound.
     int run_get_closest_border_zone_008aecd0(lua_State* state, int argument_count);
+    // Packet cc13_natives1, under kLuaIsInBorderZoneBound.
+    int run_is_in_border_zone_008af020(lua_State* state, int argument_count);
     // Packet cc9_submarine_air, under kSubmarineAirBound.
     int run_set_unlimited_air_00893c00(lua_State* state, int argument_count);
 
diff --git a/src/game_hosts_lua.cpp b/src/game_hosts_lua.cpp
index 7272a6905..3939deff0 100644
--- a/src/game_hosts_lua.cpp
+++ b/src/game_hosts_lua.cpp
@@ -56,6 +56,7 @@
 #include "bsp/recon_values.hpp"
 #include "bsp/recon_slot_lists.hpp"  // packet cc9_recon_publication
 #include "bsp/shipyard_production.hpp"  // packet cc9_lua_add_shipyard_stock
+#include "bsp/plane_retreat_task.hpp"   // packet cc13_natives1, 00489C40
 #include "bsp/vfs_locale_runtime.hpp"
 #include "bsp/vfs_provider_manager.hpp"
 
@@ -508,6 +509,9 @@ int binding_trampoline(lua_State* state) {
     // Packet cc9_get_closest_border_zone.
     const bool border_zone_row = kLuaClosestBorderZoneBound
         && dispatch_row.address == 0x008aecd0u;
+    // Packet cc13_natives1.
+    const bool in_border_zone_row = kLuaIsInBorderZoneBound
+        && dispatch_row.address == 0x008af020u;
     // Packet cc9_lua_formation_query.
     const bool in_formation_row = kLuaFormationQueryBound
         && dispatch_row.address == 0x008996a0u;
@@ -572,6 +576,7 @@ int binding_trampoline(lua_State* state) {
         || ready_row
         || launch_row || generate_row || spawn_new_row || scoring_play_time_row
         || spawn_id_requested_row || spawn_id_remove_row
+        || in_border_zone_row
         || select_unit_row || movie_add_row || force_select_row
         || (orders != nullptr && GameScriptOrdersHost::handles(dispatch_row.name));
     // The replay of a failed named call, which the executable makes only to
@@ -720,6 +725,9 @@ int binding_trampoline(lua_State* state) {
     if (border_zone_row && !host->error_replay()) {
         return host->run_get_closest_border_zone_008aecd0(state, argc);
     }
+    if (in_border_zone_row && !host->error_replay()) {
+        return host->run_is_in_border_zone_008af020(state, argc);
+    }
     if (leave_formation_row) {
         if (!host->error_replay()) host->run_leave_formation_00899eb0(state, argc);
         return 0;
@@ -6395,6 +6403,51 @@ int GameMissionLuaHost::run_get_closest_border_zone_008aecd0(lua_State* state,
     return 1;
 }
 
+// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 3). 008AF020
+// IsInBorderZone(entity | position), 008AF020-008AF257, RET, ECX the Lua state.
+// 008AF11E 008889C0 asks whether argument 0 is an entity handle: yes, 008AF153
+// 00888AA0 and the translation +FCh/+100h/+104h (00414DB0 first when +C8h is
+// clear); no, 008AF1AC 00888760. 008AF1EF 0071C4F0 with ECX = [00E188A8]; when
+// it answers outside, 008AF1FF 00489C40 (same ECX) and 008AF20C 00B664B0 push
+// the record's +4h, its side (0 or 1 after the twelve 004C7150 calls). Inside,
+// nothing is pushed and the script reads nil. A side of 0 is a true value in Lua.
+// SUBSTITUTIONS (labelled): the entity test is the host's `ID` field read (an
+// entity table with an ID resolving to a units-host slot), and the translation is
+// the slot's +FCh row; with no zone records the image would dereference null, the
+// host pushes nothing and counts it.
+int GameMissionLuaHost::run_is_in_border_zone_008af020(lua_State* state,
+    int argument_count) {
+    ++summary_.in_border_zone_calls;
+    float position[3] = {0.0f, 0.0f, 0.0f};
+    GameUnitsHost* units = units_hooks_;
+    const int id = air_ops_entity_id(state);
+    if (units != nullptr && id > 0 && static_cast<std::size_t>(id) <= units->count()) {
+        units->unit_position_00fc(static_cast<std::size_t>(id - 1), position[0], position[1],
+            position[2]);
+    } else if (argument_count >= 1) {
+        read_vector3_00888760(state, 1, position);
+    }
+    const std::array<float, 3> p{position[0], position[1], position[2]};
+    log_.implemented("MissionLuaNative::IsInBorderZone", "008af020");
+    if (!border_zones_loaded_) {
+        ++summary_.in_border_zone_no_zone;   // no Map bounds: read as inside
+        return 0;
+    }
+    if (!bsp::point_outside_world_map_0071c4f0(border_bounds_, p)) return 0;
+    ++summary_.in_border_zone_outside;
+    const bsp::BorderZoneRecord* zone
+        = bsp::border_zone_at_point_00489c40(border_bounds_, border_zones_, p);
+    if (zone == nullptr) {
+        ++summary_.in_border_zone_no_zone;
+        return 0;
+    }
+    ::lua_pushinteger(state, static_cast<lua_Integer>(zone->side));
+    log_.notef("  IsInBorderZone 008af020: (%.2f, %.2f, %.2f) outside the map, side %d "
+        "(packet cc13_natives1)", static_cast<double>(p[0]), static_cast<double>(p[1]),
+        static_cast<double>(p[2]), zone->side);
+    return 1;
+}
+
 // Packet cc9_set_invincible_native. 00897A50 SetInvincible(entity, value):
 // argument 0 through 00888AA0; argument 1 is a boolean (00897B6F) giving 1.0 or
 // 0.0, otherwise its number, a fraction of maximum health (a nil or missing
@@ -8026,6 +8079,10 @@ void GameMissionLuaHost::report_mission_script_state() {
         "(008AECD0 / 004C7730, packet cc9_get_closest_border_zone)",
         kLuaClosestBorderZoneBound ? 1 : 0, border_zones_loaded_ ? 1 : 0,
         summary_.border_zone_calls, summary_.border_zone_missing);
+    log_.notef("summary mission script in border zone bound=%d calls=%llu outside=%llu "
+        "no_zone=%llu (008AF020 -> 0071C4F0 / 00489C40, packet cc13_natives1)",
+        kLuaIsInBorderZoneBound ? 1 : 0, summary_.in_border_zone_calls,
+        summary_.in_border_zone_outside, summary_.in_border_zone_no_zone);
     log_.notef("summary mission script set invincible calls=%llu units=%llu unresolved=%llu "
         "(00897A50 -> 0042ED80, packet cc9_set_invincible_native)", summary_.invincible_calls,
         summary_.invincible_units, summary_.invincible_unresolved);
```

## 3. SquadronForceGunnerMode 0089F950

**Image** (0089F950-0089FAE4 RET):
1. `00888AA0` on argument 0 (0089FA4E, ESI).
2. `00B66250` reads argument 1 as a boolean (0089FA80).
3. `MOV [ESI+379h],BL` at 0089FA9C. There is no class test and no result.
4. The squadron constructor `007F2C60` clears the byte (`007F2D25 MOV [ESI+379h],BL`, EBX = 0 from 007F2C91).

The `79 03 00 00` disp32 scan finds two readers of the byte:
- `0064A1E0`, in the HUD root update `00649860`. It tests the byte when the controlled unit `[00E188D8]` is
  `IsKindOf(18h)`, then calls `00647080` at 0064A200.
- `00647080`. Its `+379h` test keeps the interface at `23h` (gunner) instead of switching back to `22h` (pilot).

The other hits are false positives. One is 0092160F, the tail of a `JE` rel32. The others are in unrelated objects:
the AutoTest, the chat screen and the `.rdata` tables.

The byte is **player-seat presentation only**. No AI or flight path reads it.

**Host.**
- The row is unimplemented.
- The HUD host already has the reader seam `controlled_unit_flag_379()` (`game_hosts_hud.cpp:3064`), which answers
  false.
- `toggle_closed_hud` (00647080) is a record.

The diff stores the byte per units-host slot and adds `GameMissionLuaHost::squadron_force_gunner_mode_379(index)` for
that seam. Wiring the seam is the HUD owner's step and is not in the diff.

**Call site:** `COTP-USN/PRCPUS/prcp_13_iwojima.lua` (2024-10-29 12:54), USN18, line 549:
`SquadronForceGunnerMode(unit, true)` for every `Mission.Bombas` squadron, with the pilot role given to `PLAYER_AI`
(547).

**Which missions move.** None in gameplay. USN18 loses its 12 UNIMPLEMENTED calls (`s30_cap_usn18.log`) and gains a
summary line.

```diff
diff --git a/include/bsp/game_hosts_lua.hpp b/include/bsp/game_hosts_lua.hpp
index 00c3ba0b3..30e176d15 100644
--- a/include/bsp/game_hosts_lua.hpp
+++ b/include/bsp/game_hosts_lua.hpp
@@ -264,6 +264,13 @@ inline constexpr bool kLuaUnitGetAttackTargetBound = true;  // ON: pairs held (d
 // each of the +3CCh members at +3D0h (0089F8CA..0089F8FF); on a plane that is
 // 0074E1E0 -> 007D9E80, the controller's forward-speed set. True: route the row to
 // run_squadron_set_speed_0089f780. False: unimplemented.
+// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 4). 0089F950
+// SquadronForceGunnerMode(entity, flag) stores argument 1 (00B66250) in the byte
+// entity+379h (0089FA9C), with no class test and no result. Its only reader is the
+// HUD root update (0064A1E0 on the controlled kind-18h unit) and 00647080, which
+// keep the player in the gunner interface (23h). True: the byte is kept per unit
+// for squadron_force_gunner_mode_379. False: unimplemented, nothing stored.
+inline constexpr bool kLuaSquadronForceGunnerModeBound = false;
 inline constexpr bool kLuaSquadronSetSpeedBound = true;  // ON: pairs held (docs/LUA_BINDING_MISSION.md)
 
 // Packet cc9_is_class_changed (docs/LUA_BINDING_MISSION.md, "IsClassChanged, 008CC4B0").
@@ -701,6 +708,9 @@ struct GameMissionLuaSummary {
     unsigned long long squadron_speed_calls{0};
     unsigned long long squadron_speed_planes{0};
     unsigned long long squadron_speed_unresolved{0};
+    unsigned long long force_gunner_calls{0};       // packet cc13_natives1, 0089F950
+    unsigned long long force_gunner_set{0};
+    unsigned long long force_gunner_unresolved{0};
     unsigned long long class_changed_calls{0};
     unsigned long long class_changed_true{0};
     unsigned long long sub_depth_calls{0};
@@ -1292,6 +1302,12 @@ public:
     unsigned shipyard_units_serial_{0};
     // Packet cc9_device_reload_enabled, under kLuaDeviceReloadEnabledBound.
     int run_set_device_reload_enabled_008c1350(lua_State* state, int argument_count);
+    // Packet cc13_natives1, under kLuaSquadronForceGunnerModeBound: the byte
+    // entity+379h per units-host slot, and its reader for the HUD root's
+    // controlled_unit_flag_379 (0064A1E0). False for an unset or unknown slot.
+    int run_squadron_force_gunner_mode_0089f950(lua_State* state, int argument_count);
+    bool squadron_force_gunner_mode_379(std::size_t unit_index) const;
+    std::vector<std::uint8_t> force_gunner_379_;
     // Packet cc9_lua_formation_query, under kLuaFormationQueryBound.
     int run_is_in_formation_008996a0(lua_State* state, int argument_count);
     int run_get_formation_leader_00899af0(lua_State* state, int argument_count);
diff --git a/src/game_hosts_lua.cpp b/src/game_hosts_lua.cpp
index 7272a6905..1c02f73e0 100644
--- a/src/game_hosts_lua.cpp
+++ b/src/game_hosts_lua.cpp
@@ -478,6 +478,9 @@ int binding_trampoline(lua_State* state) {
     // Packet cc9_squadron_set_speed.
     const bool squadron_speed_row = kLuaSquadronSetSpeedBound
         && dispatch_row.address == 0x0089f780u;
+    // Packet cc13_natives1.
+    const bool force_gunner_row = kLuaSquadronForceGunnerModeBound
+        && dispatch_row.address == 0x0089f950u;
     // Packet cc9_is_class_changed.
     const bool class_changed_row = kLuaIsClassChangedBound
         && dispatch_row.address == 0x008cc4b0u;
@@ -564,6 +567,7 @@ int binding_trampoline(lua_State* state) {
         || override_hp_row
         || attack_target_row || squadron_speed_row || class_changed_row || sub_depth_row
         || sub_surface_row
+        || force_gunner_row
         || slot_count_row || add_stock_row || shipyard_stock_row || device_reload_row
         || unlimited_air_row
         || in_formation_row || leave_formation_row || travel_alt_row || border_zone_row
@@ -738,6 +742,10 @@ int binding_trampoline(lua_State* state) {
         if (!host->error_replay()) host->run_squadron_set_speed_0089f780(state, argc);
         return 0;
     }
+    if (force_gunner_row) {
+        if (!host->error_replay()) host->run_squadron_force_gunner_mode_0089f950(state, argc);
+        return 0;
+    }
     if (attack_target_row && !host->error_replay()) {
         return host->run_unit_get_attack_target_008a6de0(state, argc);
     }
@@ -5526,6 +5534,40 @@ int GameMissionLuaHost::run_squadron_set_speed_0089f780(lua_State* state, int ar
     return 0;
 }
 
+// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 4). 0089F950
+// SquadronForceGunnerMode(entity, flag), 0089F950-0089FAE4 RET: 0089FA4E 00888AA0
+// on argument 0 (ESI), 0089FA80 00B66250 on argument 1, 0089FA9C MOV [ESI+379h],BL.
+// No class or null test, no result. The squadron constructor 007F2C60 clears the
+// byte (007F2D25). Readers: 0064A1E0 (HUD root update, the controlled unit when
+// IsKindOf(18h)) and 00647080 (which keeps interface 23h while it is set).
+// SUBSTITUTIONS (labelled): the byte is kept per units-host slot here rather than
+// on the entity; an entity with no slot is counted unresolved (the image would
+// write through whatever 00888AA0 returned).
+int GameMissionLuaHost::run_squadron_force_gunner_mode_0089f950(lua_State* state,
+    int argument_count) {
+    ++summary_.force_gunner_calls;
+    const bool flag = argument_count >= 2 && ::lua_toboolean(state, 2) != 0;
+    GameUnitsHost* units = units_hooks_;
+    const int id = air_ops_entity_id(state);
+    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
+        ++summary_.force_gunner_unresolved;
+        return 0;
+    }
+    const std::size_t index = static_cast<std::size_t>(id - 1);
+    if (force_gunner_379_.size() <= index) force_gunner_379_.resize(index + 1, 0);
+    force_gunner_379_[index] = flag ? 1 : 0;
+    if (flag) ++summary_.force_gunner_set;
+    const GameUnitRow* row = units->unit_row(index);
+    log_.notef("  SquadronForceGunnerMode 0089f950: \"%s\" +379h=%d (packet cc13_natives1)",
+        row != nullptr ? row->name.c_str() : "?", flag ? 1 : 0);
+    log_.implemented("MissionLuaNative::SquadronForceGunnerMode", "0089f950");
+    return 0;
+}
+
+bool GameMissionLuaHost::squadron_force_gunner_mode_379(std::size_t unit_index) const {
+    return unit_index < force_gunner_379_.size() && force_gunner_379_[unit_index] != 0;
+}
+
 // Packet cc9_is_class_changed. 008CC4B0 IsClassChanged(id): argument 0 as an integer
 // (00B66290, 008CC5AF), then 00437F50 and CMP [EAX+ESI*4+2010h],ESI / SETNZ
 // (008CC5CB..008CC5D6) into 00B66450 (lua_pushboolean). One result.
@@ -7959,6 +8001,10 @@ void GameMissionLuaHost::report_mission_script_state() {
         "unresolved=%llu (0089F780 -> 0074E1E0 -> 007D9E80, packet cc9_squadron_set_speed)",
         kLuaSquadronSetSpeedBound ? 1 : 0, summary_.squadron_speed_calls,
         summary_.squadron_speed_planes, summary_.squadron_speed_unresolved);
+    log_.notef("summary mission script squadron force gunner mode bound=%d calls=%llu set=%llu "
+        "unresolved=%llu (0089F950 -> entity+379h, packet cc13_natives1)",
+        kLuaSquadronForceGunnerModeBound ? 1 : 0, summary_.force_gunner_calls,
+        summary_.force_gunner_set, summary_.force_gunner_unresolved);
     log_.notef("summary mission script class changed bound=%d calls=%llu true=%llu "
         "(008CC4B0, registry+2010h, packet cc9_is_class_changed)",
         kLuaIsClassChangedBound ? 1 : 0, summary_.class_changed_calls,
```

## 4. SquadronSetReleaseAlt 008A24E0

**Image** (008A24E0-008A2702 RET):
1. `00888AA0` reads argument 0 (008A25E6, ESI). There is no null or class test.
2. `00B66270` reads argument 1, a number (008A2617).
3. Argument 2 is read as a boolean (`00B66250`, 008A265F) only when `00B663F0` (008A2634) counts exactly three
   arguments; otherwise the force flag is 0.
4. Five stores on the squadron block:

| site | store | |
| --- | --- | --- |
| 008A2692 | `+384h = [00CE3800]` | `00 00 00 3F` = 0.5, countdown 2 |
| 008A26A0 | `+38Eh = force` | freeze byte |
| 008A26AA | `+39Ch = altitude` | the release altitude |
| 008A26B2 | `+3ABh = 1` | lock |
| 008A26B9 | `+3ADh = 0` | the shared "script altitude live" byte (smalldiffs item 1) |

Nothing is pushed. This is the third of three parallel blocks:
- travel (`0089F550`): `+380h`, `+38Dh`, `+394h`, `+3A9h`;
- attack (`008A22B0`): `+37Ch`, `+38Ch`, `+398h`, `+3AAh`;
- release: the stores above.

**Readers of `+39Ch`.** The disp32 scan `9C 03 00 00` over the plane code finds only this native and the tuning
loader. Every other access goes through `block+37Ch` as a base:
- **Dive profile `009C8920`**, flight leader only. The leader gate `009C8931 007B8AD0` jumps past it at `009C8A1B`.
  `EAX = [ESI+404h] + 37Ch` (009C89E4..009C89EA), then:
  - `[EAX+12h]` (`+38Eh`) set: skip the whole block (009C89EF / JNE 009C8A1B).
  - Otherwise, if `0 > [EAX+8]` (`+384h` has run out, 009C89FA) and `[EAX+2Fh]` (`+3ABh`) is clear:
    - `[EAX+20h]` (`+39Ch`) = `[00CE4C04]` = 9999.0 (009C8A0E);
    - `[EAX+31h]` (`+3ADh`) = 1 (009C8A13).
  - In every non-skipped case, `+3ABh = 0` (009C8A17).

  The lock therefore delays the overwrite by one profile call after the countdown runs out, and the force flag
  holds the script value indefinitely.
- **`009C7A80`**, the dive-bomb approach update, which 009C8920 calls at 009C8A7C after the block.
  - It loads `+39Ch` at 009C7AB3 into the dive-altitude decay 009C7AB3..009C7AC7.
  - The host reconstructs that decay as `bsp::dive_bomb_decay_dive_altitude_009c7a94(a8, control_alt_39c)`. At
    `src/game_hosts_units.cpp:3978` it always passes the 9999 constant `kCruisingAltitudeThird`.

**Host.**
- `SquadronSetReleaseAlt` is listed in `src/mission_lua_host.cpp:206` but not bound, so it is UNIMPLEMENTED.
- The units host has no third block: `+39Ch` is always 9999 in the decay.

**The diff:**
- Adds the block to `GameUnitSlot`: `sq_release_alt_set`, `sq_alt_39c`, `sq_freeze_38e`, `sq_lock_3ab` and
  `sq_timer_expiry_384`. The countdown is held as an expiry clock, as the other two blocks hold theirs.
- Adds the setter `GameUnitsHost::set_squadron_release_alt_008a24e0`.
- Adds the binding `run_squadron_set_release_alt_008a24e0` behind `kLuaSquadronSetReleaseAltBound` (OFF).
- Replaces the decay's constant with the block's value while the script value is live, after the 009C8920 gate
  above.

The units-host part is inert until the binding stores something, so it needs no switch of its own.

What is LABELLED:
- The per-tick approach update stands for the profile's call.
- The leader test is `007B8AD0` without the departed-wingman case that the attack block handles.
- A unit with no squadron slot stores nothing (the image would write through whatever `00888AA0` returned).

**Composition with `docs/CC13_SMALL_DIFFS_BATCH_2.md` item 1 (diff 5.1, `kMoveToLuaAltZeroOffsetBound`).**
- Diff 4 and diff 5.1 touch disjoint lines. Each applies alone on origin/main, and each applies on top of the other.
- Item 1's field `sq_lua_alt_3ad` does not exist on origin/main, so diff 4 cannot set it.
- **Order dependency:** apply addendum 4a only after **both** diff 4 and diff 5.1. It adds the two writes item 1
  needs from this native:
  - `sq_lua_alt_3ad = true` in the setter (008A26B9 `+3ADh = 0`);
  - `sq_lua_alt_3ad = false` where the release gate overwrites `+39Ch` (009C8A13 `+3ADh = 1`).
- Diff 4 + 5.1 + 4a build together (exit 0).

**Call sites in this installation** (every file 2024-07-13 08:26, untouched bulk):

| script | lines | call |
| --- | --- | --- |
| `COTP-USN/usn_01_battle_of_eastern_solomons.lua` | 621, 1055 | `SquadronSetReleaseAlt(unit, 700, true)` |
| `COTP-USN/usn_04_defend_guadalcanal.lua` | 2580 | `SquadronSetReleaseAlt(pEntity, altitude)` |
| `COTP-USN/usn_07_invasion_of_tarawa.lua` | 1708 | `SquadronSetReleaseAlt(unit,400)` |
| `COTP-USN/usn_12_indochina.lua` | 1114, 1251, 1547 | B-29 1100, `planeSquad` altitude, B-17 800 |
| `COTP-USN/PRCPUS/usn_12_indochina.lua` | 1317, 1507, 1807 | the same three |
| `bsm/bsm_09_battle_of_the_coral_sea.lua` | 321 | `SquadronSetReleaseAlt( squad, 220 )` |

`scripts/global/commandhelpers.lua` does not call it, and neither does any script in `CC13_WIN_READINESS.md`'s
not-yet-won list.

**Which missions would move.** Only USN01, USN04 (guadalcanal), USN07, USN12 and BSM09, and only for a dive-bomb
approach of a squadron that the script gave a release altitude:
- That squadron's dive altitude decays toward the script value (700, 400, 220, ...) instead of toward 9999.
- With `force` (USN01), it stays there.
- Without `force`, it stays there until the leader's second profile call after 0.5 s.
- No win-path mission moves. Good pair candidates are USN01 and USN12.

```diff
diff --git a/include/bsp/game_hosts_lua.hpp b/include/bsp/game_hosts_lua.hpp
index 00c3ba0b3..4f08bf235 100644
--- a/include/bsp/game_hosts_lua.hpp
+++ b/include/bsp/game_hosts_lua.hpp
@@ -400,6 +400,13 @@ bool lua_device_reload_enabled_00e17bf2() noexcept;
 // MOV byte [EAX+1D4h],1 (008AC263). No other write, no null test, no result
 // (008AC269 00B66400). The byte is what the unit-AI untouchable gate 00862440
 // reads. True: the row sets the flag below; false: the row stays unimplemented.
+// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 4). 008A24E0
+// SquadronSetReleaseAlt(squadron, altitude[, force]) writes the squadron's third
+// block: +384h = 0.5, +38Eh = force (only with exactly three arguments), +39Ch =
+// altitude, +3ABh = 1, +3ADh = 0. 009C8920 overwrites +39Ch with 9999 once the
+// countdown and lock allow, and 009C7A80 reads it into the dive altitude. True:
+// route the row to the units host's third block. False: unimplemented.
+inline constexpr bool kLuaSquadronSetReleaseAltBound = false;
 inline constexpr bool kLuaAddUntouchableUnitBound = true;   // ON on identity (LUA_BINDING_MISSION)
 // The +1D4h byte of units-host index `index`'s AI object, as AddUntouchableUnit
 // left it. False for every unit while kLuaAddUntouchableUnitBound is false.
@@ -1298,6 +1305,8 @@ public:
     int run_get_last_catapulted_00892860(lua_State* state, int argument_count);
     int run_add_untouchable_unit_008ac140(lua_State* state, int argument_count);
     int run_leave_formation_00899eb0(lua_State* state, int argument_count);
+    // Packet cc13_natives1, under kLuaSquadronSetReleaseAltBound.
+    int run_squadron_set_release_alt_008a24e0(lua_State* state, int argument_count);
     // Packet cc9_squadron_travel_alt, under kSquadronTravelAltBound.
     int run_squadron_set_travel_alt_0089f550(lua_State* state, int argument_count);
     // Packet cc9_squadron_attack_alt, under kSquadronAttackAltBound.
diff --git a/include/bsp/game_hosts_units.hpp b/include/bsp/game_hosts_units.hpp
index 44f37c5cd..ac6ea6886 100644
--- a/include/bsp/game_hosts_units.hpp
+++ b/include/bsp/game_hosts_units.hpp
@@ -556,6 +556,9 @@ public:
     // unit has no slot.
     // Packet cc9_squadron_attack_alt: 008A22B0's five stores on the squadron's slot.
     bool set_squadron_attack_alt_008a22b0(std::size_t unit_index, float altitude, bool force);
+    // Packet cc13_natives1: 008A24E0's five stores on the squadron's third block
+    // (+384h, +38Eh, +39Ch, +3ABh, +3ADh); 009C8920 / 009C7A80 read +39Ch.
+    bool set_squadron_release_alt_008a24e0(std::size_t unit_index, float altitude, bool force);
     bool set_squadron_travel_alt_0089f550(std::size_t unit_index, float altitude, bool force);
 
     // Packet cc9_difficulty. SetSkillLevel's leaf, unit->vtable[128h]: 009565A0
diff --git a/src/game_hosts_lua.cpp b/src/game_hosts_lua.cpp
index 7272a6905..c925106c0 100644
--- a/src/game_hosts_lua.cpp
+++ b/src/game_hosts_lua.cpp
@@ -502,6 +502,9 @@ int binding_trampoline(lua_State* state) {
     // Packet cc9_squadron_travel_alt.
     const bool travel_alt_row = kSquadronTravelAltBound
         && dispatch_row.address == 0x0089f550u;
+    // Packet cc13_natives1.
+    const bool release_alt_row = kLuaSquadronSetReleaseAltBound
+        && dispatch_row.address == 0x008a24e0u;
     // Packet cc9_squadron_attack_alt.
     const bool attack_alt_row = kSquadronAttackAltBound
         && dispatch_row.address == 0x008a22b0u;
@@ -560,6 +563,7 @@ int binding_trampoline(lua_State* state) {
         && (dispatch_row.address == 0x008acb70u || dispatch_row.address == 0x008aca30u);
     const bool handled = checkpoint_row || avoidance_setting || objective_row || get_property_row || kill_row
         || add_listener_row || remove_listener_row || listener_active_row || set_invincible_row
+        || release_alt_row
         || forced_recon_row || add_damage_row || aa_enable_row || ship_speed_row
         || override_hp_row
         || attack_target_row || squadron_speed_row || class_changed_row || sub_depth_row
@@ -704,6 +708,10 @@ int binding_trampoline(lua_State* state) {
         if (!host->error_replay()) host->run_squadron_set_travel_alt_0089f550(state, argc);
         return 0;
     }
+    if (release_alt_row) {
+        if (!host->error_replay()) host->run_squadron_set_release_alt_008a24e0(state, argc);
+        return 0;
+    }
     if (in_formation_row && !host->error_replay()) {
         return host->run_is_in_formation_008996a0(state, argc);
     }
@@ -5948,6 +5956,31 @@ int GameMissionLuaHost::run_squadron_set_attack_alt_008a22b0(lua_State* state,
     return 0;
 }
 
+// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 4). 008A24E0
+// SquadronSetReleaseAlt(squadron, altitude[, force]), 008A24E0-008A2702 RET:
+// 008A25E6 00888AA0 on argument 0 (ESI), 008A2617 00B66270 on argument 1, and
+// argument 2 through 00B66250 (008A265F) only when 008A2634 00B663F0 counts
+// exactly three arguments; then the five stores 008A2692..008A26B9. No result.
+// SUBSTITUTION (labelled): an entity with no squadron slot stores nothing.
+int GameMissionLuaHost::run_squadron_set_release_alt_008a24e0(lua_State* state,
+    int argument_count) {
+    GameUnitsHost* units = units_hooks_;
+    const int id = air_ops_entity_id(state);
+    const float altitude = argument_count >= 2 ? static_cast<float>(::lua_tonumber(state, 2)) : 0.0f;
+    const bool force = argument_count == 3 && ::lua_toboolean(state, 3) != 0;
+    bool stored = false;
+    if (units != nullptr && id > 0 && static_cast<std::size_t>(id) <= units->count()) {
+        stored = units->set_squadron_release_alt_008a24e0(static_cast<std::size_t>(id - 1),
+            altitude, force);
+    }
+    const GameUnitRow* row = stored ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
+    log_.notef("  SquadronSetReleaseAlt 008a24e0: \"%s\" alt=%.1f force=%d stored=%d (packet "
+        "cc13_natives1)", row != nullptr ? row->name.c_str() : "?",
+        static_cast<double>(altitude), force ? 1 : 0, stored ? 1 : 0);
+    log_.implemented("MissionLuaNative::SquadronSetReleaseAlt", "008a24e0");
+    return 0;
+}
+
 // Packet cc9_lua_formation_query. 008996A0 IsInFormation(unit): argument 0 through
 // BSP_ObjectHandle_FromLuaTable, then lua_pushboolean([unit+284h] != 0). One result.
 // SUBSTITUTION (labelled): an entity with no units-host slot answers false (the image
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index f8634b115..6d346e8fc 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -1750,6 +1750,14 @@ struct GameUnitSlot {
     float sq_profile_398{0.0f};
     unsigned long long sq_profile_draws{0};
     unsigned long long sq_profile_writes{0};
+    // Packet cc13_natives1: the third block 008A24E0 writes (+384h countdown 2
+    // held as its expiry clock, +38Eh freeze, +39Ch release altitude, +3ABh lock).
+    // sq_release_alt_set: +39Ch holds the script value, not 009C8A0E's 9999.
+    bool sq_release_alt_set{false};
+    float sq_alt_39c{0.0f};
+    bool sq_freeze_38e{false};
+    bool sq_lock_3ab{false};
+    double sq_timer_expiry_384{-1.0};
     // desc+1ACh PitchSpd (DEG(30) on this installation's TBD). 007DA8EB uses it
     // as the pitch rate; 009D1E39 divides the nose-down angle by it to shallow
     // the aim tick's dive command as the dive steepens.
@@ -3975,8 +3983,25 @@ struct GameUnitsHost::Impl {
         // 009C7A9E-009C7AB4, reconstructed. ctl+39Ch is the 9999.0f at
         // 00CE4C04 the same profile writes, so the min leaves +A8h alone below
         // about 9523 and this is a ceiling, not a decay, at these altitudes.
+        // Packet cc13_natives1: 009C8920's third block, run for the flight leader
+        // only (009C8931 007B8AD0, JE 009C8A1B) before its 009C7A80 call. +38Eh set:
+        // skip (009C89EF). Countdown +384h expired and +3ABh clear: +39Ch = 9999
+        // (009C8A0E) and +3ADh = 1 (009C8A13). Then +3ABh = 0 (009C8A17). LABELLED:
+        // this per-tick update stands for the profile's call; the leader test is
+        // 007B8AD0 without the departed-wingman case.
+        float control_alt_39c = bsp::dive_bomb_constant::kCruisingAltitudeThird;
+        if (GameUnitSlot* rb = squadron_slot_of(slot.process_index);
+            rb != nullptr && rb->sq_release_alt_set) {
+            if (unit_is_flight_leader_007b8ad0(slot.process_index) && !rb->sq_freeze_38e) {
+                if (summary.simulated_seconds >= rb->sq_timer_expiry_384 && !rb->sq_lock_3ab) {
+                    rb->sq_release_alt_set = false;
+                }
+                rb->sq_lock_3ab = false;
+            }
+            if (rb->sq_release_alt_set) control_alt_39c = rb->sq_alt_39c;
+        }
         slot.db_dive_alt_a8 = bsp::dive_bomb_decay_dive_altitude_009c7a94(
-            slot.db_dive_alt_a8, bsp::dive_bomb_constant::kCruisingAltitudeThird);
+            slot.db_dive_alt_a8, control_alt_39c);
         // 009C8A5E, the cruise profile's clamp on task+4B0h == approach+B8h.
         {
             const float wanted = kPilotDiveBombAttackDist * 1.0f;
@@ -33337,6 +33362,24 @@ bool GameUnitsHost::unit_is_kind_of(std::size_t index, int class_id) const {
     return bsp::unit_is_kind_of(impl_->slots[index]->class_id, class_id);
 }
 
+// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 4). 008A24E0
+// SquadronSetReleaseAlt's five stores on the squadron block, 008A2692..008A26B9:
+// +384h = [00CE3800] (0.5), +38Eh = the force flag, +39Ch = altitude, +3ABh = 1,
+// +3ADh = 0. A unit with no squadron is refused (the image has no null test).
+bool GameUnitsHost::set_squadron_release_alt_008a24e0(std::size_t unit_index, float altitude,
+                                                      bool force) {
+    Impl& host = *impl_;
+    GameUnitSlot* sq = host.squadron_slot_of(unit_index);
+    if (sq == nullptr) return false;
+    sq->sq_timer_expiry_384 = host.summary.simulated_seconds + 0.5;  // +384h = [00CE3800]
+    sq->sq_freeze_38e = force;                                        // +38Eh
+    sq->sq_alt_39c = altitude;                                        // +39Ch
+    sq->sq_lock_3ab = true;                                           // +3ABh = 1
+    sq->sq_release_alt_set = true;                                    // +3ADh = 0
+    host.done("PlaneSquadron::set_release_alt_008a24e0", 0x008a24e0u);
+    return true;
+}
+
 // Packet cc9_unit_class_lands_troops. Slot 2Ch of the class descriptor vtables
 // (disk bytes, kVehicleClassDescriptorTable vtables + 2Ch): 00827FB0 in the
 // Destroyer, Cruiser, Cargo, BattleShip, Submarine, TorpedoBoat and MotherShip
```

Addendum 4a (only after diff 4 **and** smalldiffs diff 5.1):

```diff
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index 6ef67a223..4f9222963 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -4000,6 +4000,7 @@ struct GameUnitsHost::Impl {
             if (unit_is_flight_leader_007b8ad0(slot.process_index) && !rb->sq_freeze_38e) {
                 if (summary.simulated_seconds >= rb->sq_timer_expiry_384 && !rb->sq_lock_3ab) {
                     rb->sq_release_alt_set = false;
+                    rb->sq_lua_alt_3ad = false;   // 009C8A13 +3ADh = 1
                 }
                 rb->sq_lock_3ab = false;
             }
@@ -33405,6 +33406,7 @@ bool GameUnitsHost::set_squadron_release_alt_008a24e0(std::size_t unit_index, fl
     sq->sq_alt_39c = altitude;                                        // +39Ch
     sq->sq_lock_3ab = true;                                           // +3ABh = 1
     sq->sq_release_alt_set = true;                                    // +3ADh = 0
+    sq->sq_lua_alt_3ad = true;                                        // 008A26B9
     host.done("PlaneSquadron::set_release_alt_008a24e0", 0x008a24e0u);
     return true;
 }
```

## 5. Verdicts

| native | image | host today | diff | missions that could move |
| --- | --- | --- | --- | --- |
| TrulyDead | `Ptr == null`, nulled only by `00928C80` | unimplemented; `Ptr` never nulled | diff 1, `kLuaTrulyDeadBound` OFF | none (same branch at every site) |
| IsInBorderZone | side of the zone record, only outside the map | unimplemented (nil = inside) | diff 2, `kLuaIsInBorderZoneBound` OFF | JM06 fail if Lexington leaves the map; JM05 retreat event |
| SquadronForceGunnerMode | `entity+379h`, HUD-only reader | unimplemented | diff 3, `kLuaSquadronForceGunnerModeBound` OFF | none in gameplay |
| SquadronSetReleaseAlt | third block `+384h/+38Eh/+39Ch/+3ABh`, `+3ADh = 0`; `+39Ch` feeds the dive altitude | unimplemented; `+39Ch` fixed at 9999 | diff 4 (+ 4a), `kLuaSquadronSetReleaseAltBound` OFF | USN01, USN04, USN07, USN12, BSM09 dive bombers; no win path |
| SetFailure / GetFailure | appendix A | unimplemented | none (cc9's ships worker) | - |

## 6. Open items for cc9

1. **TrulyDead.** Null `Ptr` at the host's on-killed record sites (section 1).
2. **IsInBorderZone.** Log Lexington against the map bounds on JM06 phase 2 before flipping the switch.
3. **SquadronForceGunnerMode.** Wire the HUD host's `controlled_unit_flag_379()` (`game_hosts_hud.cpp:3064`) to
   `squadron_force_gunner_mode_379` when the HUD's 22h/23h requests become real.
4. **SquadronSetReleaseAlt.** Pair it on USN01 and USN12. Land it after smalldiffs item 1, then apply addendum 4a.

## Appendix A. SetFailure 0088EFF0 / GetFailure 0088F1D0 (notes, no diff)

Dropped from this packet: cc9's ships worker is binding both natives. These are the findings, for that worker.

### A.1 The bindings

**SetFailure** (0088EFF0-0088F1CB RET):
1. Argument 0 goes through `00888AA0` (0088F0EE). Argument 1 is read as a string (`00B662B0`, 0088F121) into a
   NativeString.
2. `entity->vtable[194h](name, frame)` is called at 0088F14C (`MOV EAX,[EAX+194h]`, 0088F132).
3. Nothing is pushed.

**GetFailure** (0088F1D0-0088F3BE RET) follows the same first steps, then:
1. It calls `entity->vtable[198h](name, frame, &out)` at 0088F331 (RET 0Ch).
2. It pushes `[ESP+20h]` through `00B66450` at 0088F373. PushBoolean tests the low byte (`CMP [ESP+8],DL`, 00B66469).
3. The out slot is never written before the call (stack accesses 0088F1F7..0088F312). A name that the virtual does not
   match therefore pushes a stale stack byte.

### A.2 The ship virtuals

Eight vtables hold `008132C0` at +194h and `008135C0` at +198h. The absolute dwords are at 00CF9244, 00CFA90C,
00CFB8CC, 00CFC564, 00CFFBC4, 00D0980C, 00D0C114 and 00D0C7DC (disk scan, `local/n1_dwords.py`). 00CFC3D0 is the ship
vtable named in `docs/COMPONENT_FAILURES.md`.

`008132C0`, `__thiscall(unit, name, frame)`, `RET 8`, returns AL = 1 on every path:

| name (case-blind) | effect |
| --- | --- |
| `Explosion` (`_stricmp`, 00D080C8) | Reads arguments 2 and 3 as integers (`00B66290`), 4 and 5 as numbers (`00B66270`) and 6 as an integer (008133A2..008133D0). They become message `6Ah` fields `+24h`, `+28h`, `+2Ch`, `+30h` and `+34h` (008133D5..00813429), routed at 00813441 (`0077C2A0`, flags 7). `008198A0` builds the `00818110` object, whose step applies `vtable[1ACh](msg+30h)`. A call with no extra arguments explodes for **0** damage. |
| `SteeringJam` | Message `6Bh` only (008134C4). `0081999D` sets `unit+9E4h = 1` (`008199A7 MOV byte [EDI+9E4h],1`). **No record and no duration**: a later `600` argument is ignored. Nothing in the image retires this jam, because only a SteeringJam record's retirement sends `6Dh`. |
| `EngineJam` | Message `6Ch` through `007619B0` (00813530), routed at 00813546. `008199C7` sets `unit+9E5h = 1` (`008199D9`) and stops the point effects. Then `0093BD80` runs on `unit+A20h` (0081356F) and pushes `{5, "EngineJam", s}` through `0093BC30` (0093BE89). `s` is argument 2 when more than two arguments are given and it is a number; otherwise it is `1e10` (00CE4970 = `F9 02 15 50`). |
| `Periscope` | `009373C0` on `[unit+1018h]` (00813595) unless `unit+122Ch == 2`. |
| anything else | Nothing. |

`008135C0`, `__thiscall(unit, name, frame, out)`, `RET 0Ch`:

| name | out |
| --- | --- |
| `SteeringJam` | `unit+9E4h` |
| `EngineJam` | `unit+9E5h` |
| `Periscope` | `unit+122Ch == 2` |
| anything else | `00953E60`: `InferiorFailure` gives `unit+720h`; any other name returns 0 and leaves out unwritten |

The other classes:
- **Base pair.** 00CFB1BC, 00CFCEF4, 00CFF58C, 00CFFF74, 00D0B904 and 00D1A82C hold `00953DA0` at +194h, and the next
  slot holds `00953E60`. `00953DA0` handles only `InferiorFailure`: it sends message `7Dh`, and `0095AD0A` sets
  `unit+720h`.
- **Airfield** (00CF8D9C). +194h is `006D2210`, which has no Ghidra function. It tests RunwayFailure and HangarFailure
  before the base (`game_hosts_units.cpp:12381`). +198h is `006D2350`: `RunwayFailure` (00CF8E24) gives `+748h`,
  `HangarFailure` gives `+749h`, and anything else goes to `00953E60` (disassembled on disk, 006D2350..006D23C0).

### A.3 Retirement

`0093C520` runs every record down by dt (priority 0, `task+46h` set). An expired record goes to `vtable[19Ch]`,
which is `00813660` on the ship. `00813660` sends `6Dh` for SteeringJam and `6Eh` for EngineJam, and `008137B0`
clears `+9E4h` / `+9E5h`.

`RepairEnable(unit, false)` stores `task+45h`, the hull repair byte (`kHullRepairEnableRouteBound`). It does not touch
`+46h`, so a scripted EngineJam still runs out.

### A.4 What the jams do to the ship, and who reads them

`docs/MOTION_DIFFERENTIAL.md` and `include/bsp/unit_forces.hpp` already recover the two motion reads:
- **Engine.** `00826754..0082676E`: the engine gate is `!+9E5h && +9D8h != 0`, a 0/1 float. The target speed
  `((max_speed * scale) * throttle) * gate` (00826A46..00826A5F) is therefore **0** while the engine is jammed. The
  ship coasts down to rest through `0092D300` (00826B29).
- **Steering.** `00826B2E`: `CMP byte [EDI+6D4h],0` with EDI = unit+310h, which is `+9E4h`. A jammed rudder skips
  `0092E8C0` (00826B54), the rudder slew. The smoothed rudder stays where it was, so a ship that was turning keeps
  turning.

The other `+9E4h` / `+9E5h` readers in the ship code (disp32 scans for `E4 09 00 00` and `E5 09 00 00`) are these:
- `0080DA80` and `0080DA90`, the bare getters, and `0080DAA0`, the engine test. None of the three has a reference
  (abs and rel32 scans).
- `00811B50` and `00811B90`, steering-jam wrappers of `00811890`, also without references.
- `00818340`, a property dump.
- `008127E0` at `0081288C`.
- `00915F20`.
- The constructor at `00823727` and `0082372E`, and the reset at `0080E434` and `0080E43A`.
- `docs/GAME_AWARD_TRACKERS.md` (the `ENGINE` award string) reads `+9E5h` too.

**Host state:**
- `src/ship_motion.cpp` already honours `ShipMotionState::engine_jam` / `steering_jam` (ship_motion.cpp:211, 367).
  **Nothing in the units host ever sets them.**
- The gunnery host's hit path (`apply_failure_effect_00827b90`) only records the 6Bh / 6Ch effects.
- `unit_failure_active` reads the task's record list. A Lua SteeringJam makes no record, so it can never see one.

**The consumer hook the ship host needs.**
- Keep the two bytes per ship.
- Set them in both message arms: the Lua arm 008132C0, and the hit path `apply_failure_effect_00827b90`, which today
  only records 6Bh / 6Ch.
- Clear them when 0093C520 retires an EngineJam or SteeringJam record (00813660 -> 6Eh / 6Dh), for example in
  `FailureBinding::unit_named_state_dispatch`.
- Copy them into `slot.motion.steering_jam` / `engine_jam` before `bsp::ship_motion_step_00825f20` in
  `GameUnitsHost::motion_step_00825f20`.

Gate the motion copy separately from the binding. Once the hit path sets the bytes, the copy also moves ships whose
EngineJam started from a hit: the USN02 9000-frame pair in `docs/COMPONENT_FAILURES.md` has one, on John2 at
150.15 s, which died on the same impact.

### A.5 Call sites in this installation

| script (mtime) | line | call | arm |
| --- | --- | --- | --- |
| `COTP-IJN/PRCPIJN/JM06.lua` (2024-07-13 08:26) | 1992 | `SetFailure(Mission.Lex, "EngineJam", 600)` | 6Ch + a 600 s record |
| same | 1993 | `SetFailure(Mission.Lex, "SteeringJam", 600)` | 6Bh only. The 600 is ignored and the jam is permanent |
| `COTP-USN/PRCPUS/prcp_14_okinawa.lua` (2024-08-26 16:10), USN19 | 1957 | `SetFailure(Mission.Yamato, "EngineJam")` | 6Ch + a 1e10 s record (permanent) |
| same | 1970 | `SetFailure(Mission.Yamato, "Explosion")` | 6Ah with every field 0: an explosion of 0 damage |
| `COTP-IJN/PRCPIJN/JM05.lua` (2024-07-13 08:26) | 3465 | `GetFailure(ent, "InferiorFailure")` | base, `+720h` |

`luaJM6SittingDucks` (1986) runs once, when JM06's `MissionPhase == 4` starts (768, next to `luaJM6SpawnJapFleet`).
That phase is after the carrier hit, primary 2. In the same function, Lexington also gets `RepairEnable(false)` and
`SetInvincible(0.2)`.

USN19's 1959 `luaObj_Failed("hidden",1)` runs whatever SetFailure does, so the hidden objective does not depend on
this binding.

The `PRCPJM08.lua`, `usn_19_coralus.lua`, `usn_13_truk.lua`, `usn_15_Battle_of_Surigao_Strait.lua`, `usn_sibuyan.lua`
and `ijn_1_pearl.lua` files have no call to any of the five natives. (The `scripts/missions/ijn/JM` and `COTP-IJN/jm*`
copies do, but `CC13_WIN_READINESS.md` does not load them for these missions.)

### A.6 Which missions would move once both the binding and the motion copy are in

These are predictions, not measured:
- **JM06.** At phase 4, Lexington's target speed drops to 0 for 600 s, and its smoothed rudder stays frozen for the
  rest of the mission. That is the stationary carrier that `CC13_WIN_READINESS.md` blocker 3 asks for.
- **USN19.** Yamato stops for good once it is within 400 of the beach.
- **JM05.** `luaJM5IsDisabled` (3465) answers true for an airfield whose destruction rule took the inferior arm.
  Today it reads `nil`.

With the binding alone, only the GetFailure answers and the record list change.
