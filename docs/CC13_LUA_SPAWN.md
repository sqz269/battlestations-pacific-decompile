# The Lua `Spawn` native's placement (cc13-spawn1)

Addresses: 00944680 00941FA0 00941AA0 00941C20 009426D0 00942CB0 004BC120 004B6720 00941D30 00E0CF78 00CEA760

Packet cc13-spawn1. Code: `include/bsp/lua_spawn_cc13.hpp`, `src/lua_spawn_cc13.cpp`. Call table:
`reports/cc13_lua_spawn_calls.json` (40 rows, `tools/verify_report_calls.py`: 0 failed). Scope came from
docs/CC13_OMITTED_TASKS_SCOPE.md section 3. Every name is a hypothesis, not a recovered symbol.

## 1. Status

| piece | status |
| --- | --- |
| `00941FA0`, `00941AA0`, `00941C20`, `009426D0`, `004BC120`, `00942CB0` | reconstructed from the listing, build-tested |
| `00941D30` as `00942CB0` calls it | reconstructed: terrain probes here, entity test reused from `lua_spawn_new.cpp` |
| `00944680`'s search chain `00944C44..00944D60` and the heading `00944D98..00944DF2` | reconstructed, build-tested |
| argument parse and creation tail of `00944680` | already reconstructed by packet cc_lua_core (`include/bsp/lua_binding_spawn.hpp`) |
| host glue | suggested diff only (section 6), behind `bsp::kLuaSpawnBound` = **false**; applied, built and reverted in this tree |
| game-validated | **no**: nothing runs it |

These are new C++ interfaces over plain inputs, not ABI replacements.

## 2. Entry points and ABI

| address | name (ledger) | ABI | callers |
| --- | --- | --- | --- |
| `00941FA0` | `BSP_LuaSpawn_RingPoints` | `__fastcall(ECX near_out, EDX far_out, origin*, near, far, height, angle_lo, angle_hi)`, `RET 18h` | `00944C44`, `00942D25` |
| `00941AA0` | `BSP_LuaSpawn_RaySphere` | `__fastcall(ECX start, EDX unit dir, center*, radius, t0*, t1*)`, `AL`, `RET 10h` | `00941C41` |
| `00941C20` | `BSP_LuaSpawn_ClearMaskInSphere` | `__fastcall(ECX start, EDX unit dir, length, center*, radius, mask*)`, `RET 10h` | `00942BC0` |
| `009426D0` | `BSP_LuaSpawn_LineSearch` | `__fastcall(ECX out*, EDX party, start*, end*, ok*)`, returns `out`, `RET 0Ch` | `00944C64`, `00944CEF`, `00944D36` |
| `00942CB0` | `BSP_LuaSpawn_PlaceDirectly` | `__fastcall(ECX out*, EDX party, origin*, near, far, height, angle_lo, angle_hi, aircraft)`, returns `out`, `RET 1Ch` | `00944EF6` |
| `004BC120` | `BSP_World_FirstBoxEdgeCrossing` | `__thiscall(GGame, a*, b*, cross_out*, edge_a_out*, edge_b_out*, crossed_out*)`, `RET 18h` | `00944CC4`, `00943191`, `009432F1`, `008AF700` |
| `004B6720` | `BSP_Math_SegmentCrossing2D_Float3` | `__fastcall(ECX a0*, EDX a1*, b0*, b1*, out*)`, `RET 0Ch` | `004BC34E`, `004BC3B5` |

Old Ghidra names were all `FUN_<address>` with no comment. `ghidra_annotate.py --apply` recorded them in
`local/ghidra-annotations-20261012T110728Z.json`. `_free` (`00BF9DC8`) was checked before the batch and
is still `void __cdecl`. No function was created.

ABI evidence:
- The register arguments come from the prologues: `009426E7 MOV [ESP+74h],ECX`, `009426F5 MOV [ESP+54h],EDX`
  and `00942CC0 MOV EBP,EDX` / `00942CD1 MOV [ESP+48h],ECX`.
- The stack arguments come from the call-site pushes, mapped in `00944680`'s frame (`E` = ESP after
  `00944BF1`):
  - origin `E+70h`;
  - spans `E+94h`, `E+98h`, `E+90h`;
  - angles `E+38h`, `E+20h`;
  - near and far points `E+A8h`, `E+50h`;
  - crossing and edge ends `E+9Ch`, `E+B4h`, `E+7Ch`.

## 3. The routines

### 3.1 Ring points `00941FA0`

- `a = U(angle_lo, angle_hi)`: `00941FB6 MOV ECX,1`, `00941FC0 CALL 00BD2F10`. The result is stored as a float.
- Then `FSIN`, and `FCHS` before the store, so the x factor is `-sin a`. `FCOS` gives the z factor.
- `near = origin + r_near * (-sin a, 0, cos a)` and `far = origin + r_far * (-sin a, 0, cos a)`.
  Each product is stored before the add.
- Both y values are `height` (`0094207F`, `00942084`). The `origin.y + 0.0` (`00942020 FADD [00D7A258]`)
  store at `0094203F` is dead.
- The `Spawn` slots are 3 = near distance, 4 = far distance, 5 = height. This corrects the "span x/y/z"
  wording of `lua_binding_spawn.hpp`'s `SpawnBindingArguments`.

### 3.2 Line search `009426D0`

1. **Mask.** `memset(mask, 1, 100h)` (`009426FC`).
2. **Direction and length.** `d = end - start`. The direction is `d / |d|` (`00419440`; zero when the
   length is not positive). `L` is the square root of `d.d` when that exceeds `1e-10`
   (`00CE3820`), else 0. The step is `L / 256` (`00D19918`).
3. **Samples.** Sample `i` is at `start + i * step * dir`, for `i < 100h`.
   - **The box test** (`00942894..009428D8`):
     - The sample is blocked when `x < [711Ch]`, `x > [7128h]`, `z > [7124h]` or `z < [7130h]` on
       `[00E188A8]`.
   - **The terrain walk** runs only when `10 > start.y` (`009428DE COMISS` on `00CE38B8`):
     - The walk runs `t = 0, 50, 100, ...` while `step > t` (`00CE3938`; `009429A5`).
     - Each point `sample + dir * t` asks `00903860` on `[GGame+19CCh]` (`00942965`).
     - Ground found above -10 (`00CE6848`) blocks the sample (`00942A51`).
4. **Entities.** The routine walks `[[GGame+19CCh]+58h]` (node `+4` next, `+8` entity). It refreshes the
   pose when `+C8h` is clear (`00414DB0`, `00413920`, the 16-dword copy to `+CCh`).
   - Then `dy = start.y - entity.y` (`+100h`).
   - **Own party** (`[entity+54h] == EDX`, `00942B1A`): the entity counts when `|dy| <= 100`
     (`00D7A220`), with radius 100 (`00CE3D08`).
   - **Any other entity:** it counts when `|dy| <= 500` (`00CE3840`), with radius 2000 (`00CFFD60`).
   - A counted entity goes to `00941C20` (`00942BC0`).
5. **Result** (`00942BD0..00942C9E`):
   - The routine takes the first `i` with `mask[i] != 0`, and `out = start + (i/256) * (L * dir)`.
   - `*ok = 1` only when `i < 100h`.
   - With no free sample, `i` is 100h, so `out` is the end point and ok stays 0.

### 3.3 Mask clear `00941C20` and ray/sphere `00941AA0`

1. **Flatten the centre.** `00941C2A` writes `start.y` into the caller's copy of the entity position.
2. **Intersect.** `00941AA0` projects the centre onto the line: `proj = dir . (c - s)`, and
   `miss2 = |s + proj*dir - c|^2`.
   - It fails unless `miss2 < r^2` (`00941BCC FCOMI` / `JC`, strict).
   - Otherwise `t0,1 = proj -/+ sqrt(r^2 - miss2)`.
3. **Clamp.** `t0` and `t1` are clamped to `[0, L]` (`00941C51..00941C8B`).
4. **Clear.**
   - `lo = ftol(floor(t0 / L * 255))`, using `00CE4B48` = 255.0, `00BF85B0` and `00BF7420`.
   - `hi` is the same with a truncating `FISTP`.
   - Then `memset(mask + lo, 0, hi - lo + 1)` (`00941D14`).
   - The scale is 255 while the samples are 1/256 apart. That is the image's arithmetic and is kept.

### 3.4 Box edge crossing `004BC120`

- **The tests.** The routine tests `a -> b` in `(x, z)` against four edges, in this order:

  | edge | from | to | call |
  | --- | --- | --- | --- |
  | max z | `(min x, max z)` | `(max x, max z)` | `004BC1EE` (`004F3730`) |
  | max x | `(max x, max z)` | `(max x, min z)` | `004BC2CA` (`004F3730`) |
  | min z | `(max x, min z)` | `(min x, min z)` | `004BC34E` (`004B6720`) |
  | min x | `(min x, min z)` | `(min x, max z)` | `004BC3B5` (`004B6720`) |

  `004B6720` packs elements `[0]` and `[2]` of four float3 into float2.
- **Output.** The routine stops at the first crossing:
  - `cross = (x, 0, z)`, `crossed = 1`;
  - the edge's two endpoints as float3, with the y values the listing stores.
- **The y values are not uniform.** The max-x edge's second end has max y (`004BC320`), and the min-z
  edge's first end has max y (`004BC36E`). Every other end has min y. This was read from a hand-tracked
  frame (`F` = ESP after the four pushes: locals `F+38h..F+58h`).
- **No crossing.** `004F3730` returns without writing its output (its stores are only at
  `004F37E5`/`004F37EB`). So `cross` is `004B6720`'s unwritten locals. The reconstruction reports
  `cross_written = false` and zeroes `cross`.
- **Reuse.** The crossing test is `segment_crossing_004f3730` (`include/bsp/gun_bot_remainder.hpp`). That
  routine marks its parallel test partial: exact zero, not the `004F3560` tolerance.

### 3.5 Point legality `00941D30` as `00942CB0` calls it

`00942CB0` passes:
- `ECX` = the party;
- `EDX` = the point;
- stack: `aircraft` (`EBX`), 0, 0, and a block it builds on its own stack (`00942D68..00942E09`, rebuilt at
  `0094343C` and `009436FE`):

| block offset | value | constant |
| --- | --- | --- |
| `+0` | 100 | `00CE3D08` |
| `+4` | 2000 | `00CFFD60` |
| `+8` | own-party vertical 100 | `00CE3D08` |
| `+Ch` | other vertical 500 | `00CE397C` |
| `+10h` | 0 | |
| `+14h` | own-party square 10000 | `00CE3D64` |
| `+18h` | other square 4000000 | `00D19914` |

The routine:
1. refuses a point outside the map (`00941D41`, `0071C4F0`);
2. with class 0 and no aircraft flag, refuses ground above -10 under any of five probes (`00903860` at
   `00941E22`);
3. then runs the entity test that `src/lua_spawn_new.cpp` already reconstructs, fed with the exact roots
   100 and 2000.

The probes come from the table at `00E0CF78`: `(0,0,0)`, `(-10,0,-10)`, `(-10,0,10)`, `(10,0,10)`,
`(10,0,-10)`. The loop ends at `00E0CFBC`. The height local starts at -1000 (`00D7A240`) once.

The class arm (`0082ADA0`, `0041B680`) is never taken here.

### 3.6 Direct placement `00942CB0`

`G` = ESP after the four pushes; `W = [GGame+7128h]` alone (`009431A1`).

1. **Bearings.** There are up to eight bearings (`00942F66 CMP ESI,8`), and each draws once (`00942D25`).
   - **Ring sweep.** For `t = 0, .25, .5, .75` (`00D7A348`, `FLD1 FCOMIP` / `JA`), the point is
     `near + t*(far - near)`, at `G+A8h`. A legal point returns (`00943027`).
   - **Lifted sweep.** When `aircraft` is set (`00942E5F TEST BL,BL`), the same points are tried with
     y + 1000 (`00CE47A0`), at `G+50h`. A legal point returns (`00943061`).
2. **Border cut** (`00942F6F..009432F1`).
   - **Target.** The target is the origin when the last near point is outside the box. Otherwise it is
     `near + 40000 * unit(far - near)` (`00D19920`).
   - **The cut.** `004BC120(near, target)` gives the crossing.
   - **Second cut.** With no crossing, the routine checks two things: the origin is outside the box,
     and `0071C4F0(near)` is true. When both hold, both target coordinates are clamped to
     `[1 - W, W - 1]` and the box is cut again (`009432F1`). That result is not checked.
3. **Border point** (`009432FA..0094343A`).
   - The crossing takes y = the ring height (`00943313`).
   - It snaps to the first edge it is within 1 m of, in the order `+x`, `+z`, `-x`, `-z`. The routine
     treats the world as `[-W, W]` on both axes.
   - The two walkers' first steps are `(0,0,-/+100)`, `(-/+100,0,0)`, `(0,0,+/-100)` and `(+/-100,0,0)`
     respectively (`00CE65D8` = -100).
   - A legal border point returns (`009434AB`).
4. **Walkers** (`009434E5..009437A4`).
   - Two walkers start at the border point, each with 2 turns (`009434F7`).
   - Each round, a walker with turns left steps.
   - At a bound it clamps to `+/-W` and turns 90 degrees, with step 100 (`00D7A220`, a double) and the
     signs read at `0094359E..009436F5`. It then decrements its turn count.
   - The walker is then tested for legality (`00943779`).
   - The loop ends when both counts are negative.
5. **Give-up.** The routine returns `(0, 10, 0)` (`009437A9`, `00CE38B8`).

### 3.7 The search chain and heading in `00944680`

The trailing boolean decides the branch: `[E+12h]` non-zero means `00944BF8 JNZ 00944EA5`.

- **Direct branch.** It reads the hidden record's class name (`[record+0Ch]`, `00944EA5`) and compares it
  with `"PlaneSquadronGen"` (`00CEA760`) through `00438E10`. `SETZ` gives the aircraft flag. Then it
  calls `00942CB0`.
- **Search branch** (`00944BFE..00944D60`):
  1. `00941FA0`, then `009426D0(near, far)`.
  2. On failure, `004BC120(near, far)`. No crossing jumps to `00944F20`: nil.
  3. Then `009426D0(cross, edge_a)` and `009426D0(cross, edge_b)`. Failing both jumps to `00944F20`.
- **The party.** It is `[record+10h]` (`00944BF5`), where `record` is the result of
  `BSP_SceneDatabase_FindHiddenEntityRecord`. docs/SCENE_RECORD_MAP.md names `+0Ch` the class and
  `+10h` the party.
- **Heading** (`00944D98..00944DF2`).
  - `atan2(pos.z - origin.z, pos.x - origin.x) + pi/2`, through `00BF701A` with ST0 = dx and ST1 = dz,
    then `00CE3830`.
  - The heading is measured from the **origin**, not from the near point.
  - It is passed to the object's `vtable+11Ch`. The position goes to `+118h`.
- **Mode 1** (`00944A16..00944A3F`). The yaw is `atan2([+F4h], [+ECh]) - pi/2`, from the origin entity's
  world +z row. This resolves the "provisional" note in `lua_binding_spawn.hpp`.

## 4. RNG

The only draw site is `00941FC0` (`00BD2F10`, `ECX = 1`, stream 1, the gameplay stream:
docs/CC13_RNG_STREAM_CENSUS.md).
- The search branch draws once per `Spawn`.
- The direct branch draws once per bearing, up to 8.

The reconstruction routes the draw through `SpawnPlacementHost::uniform_00bd2f10(kSpawnRingDrawKey, ...)`.
The suggested glue keys it as `"LuaSpawn#" + class + "#ring"` on a stream of its own. Nothing here draws
from stream 0, and the gunnery stream (H-G) is not advanced. In the image the draw interleaves with every
other stream-1 consumer.

## 5. Deviations, substitutions and findings

- **DEVIATION** (`00941C20`): when `near == far`, the image divides 0 by 0. `00BF7420` turns the NaN into
  `80000000h`, and the `memset` writes outside the mask, which faults. The reconstruction clears nothing.
- **SUBSTITUTION** (`004BC120`): with no crossing, `cross` is unwritten stack in the image. The
  reconstruction sets `cross_written = false` and zeroes `cross`.
- **SUBSTITUTION** (`00942CB0`): the reconstruction returns the give-up `(0, 10, 0)` with
  `undefined_border = true` in two cases. In both, the image walks from uninitialised stack:
  - when the border cut never wrote a crossing;
  - when the crossing is not within 1 m of `+/-W` (the step slots `G+50h..G+64h` keep stale values,
    `00943438`), which happens on a map whose box is not centred.
- **Finding in existing code (not changed here; packet cc_lua_core).** `src/lua_binding_spawn.cpp:105`
  passes `position[2]` to `object_vcall_11c`. The listing passes the heading of section 3.7: `00944DC7`
  adds pi/2 to the atan2, and `00944DDA` stores it. Anyone binding `LuaBindingSpawnHost` should use
  `spawn_heading_00944db8`. The suggested glue does not use `lua_binding_spawn()`, so it is unaffected.
- **Precision.** Every x87 intermediate is stored to a float before reuse. Qword constants are applied
  in double and rounded at the store. The main thread's 24-bit control word (docs/CC13_FPU_PRECISION.md)
  can move the last bit.

## 6. Suggested host glue (cc9's mission Lua host, `kLuaSpawnBound` OFF)

**Host entity-creation path: one exists.** The glue uses GenerateObject's (`run_generate_object_00944fd0`):
- `scene_spawn_pool().find(name)`, which holds the held-back (hidden) records;
- then `create_unit_from_scene_record_0046db4b`, the pass-A push, `00925F20` / `00874D00`, and
  `push_resolved_entity_by_id`.

The native reaches the same creator: `004C6BA0` forwards to `0046D930`, which `GenerateObject` reaches too
(docs/SCENE_ENTITY_CREATE.md).

The other host inputs:
- the world box: `world_border_bounds()`;
- the map test: `point_outside_world_map_0071c4f0`;
- the ground: `world_ground_height_00903860`;
- the entities: the `SpawnPlacementUnits` walk SpawnNew already uses.

SUBSTITUTIONS in the glue, labelled in its comment:
- the keyed stream;
- an unknown class returns nil where the image dereferences a null record;
- `vtable+118h` / `+11Ch` become the record translation and `apply_scene_yaw_00467050(heading)`. The
  convention of `+11Ch` was not read, so the yaw is provisional;
- each call creates a new unit, and the pool entry is not marked spawned (`Spawn` instantiates a template
  every call);
- an empty instance name keeps the template's name.

Verification:
- applied in this tree with `local/cc13spawn_glue.py`, built with `./scripts/build.ps1` (passed, 2/2
  tests), then reverted with `git checkout`;
- `git apply --check` of the section 9 diff passed in a detached checkout of `origin/main` at
  `59b83ed15`.

**Link collision found while building the glue.** The glue's first version called
`bsp::spawn_flag_slot` (lua_binding_spawn.cpp). That pulled `lua_binding_spawn.obj` into `bsp_game` for
the first time, and the link failed with LNK2005: `bsp::spawn_request_id_matches` is defined in both
`src/lua_binding_spawn.cpp:46` and `src/lua_spawn_new.cpp:75`. Nothing links that object today, so the
duplicate is latent. The glue inlines the slot rule instead. Whoever binds `LuaBindingSpawnHost` must
first remove one of the two definitions.

Turning `bsp::kLuaSpawnBound` on is cc9's decision. It changes the challenge missions (section 7) from
`Spawn` returning nil to units being created.

## 7. Reach in this installation

Paths are under the game root. The mtimes show the files are modded, so these are this installation's
lines. `ST_ABSANGLE = 0` and `ST_RELANGLE = 1` are at `scripts/global/luamw_init.lua:276-277`
(2024-07-13 08:26).

**Wrappers** (`scripts/global/commandhelpers.lua`, 2024-10-29 12:54):
- `luaSpawn` `:8947` returns `Spawn(..., newName, true)` at `:8949`, the direct branch. The search
  variant at `:8963-8997` is inside a `--[[ ]]` block.
- `luaSpawnDelayedTimer` `:8768` calls `luaSpawn` at `:8794` and `:8798`.
- `luaSquadronSupply` `:13751` calls `luaSpawn` at `:13770`.
- `luaTCReconSaver_think` `:5431` calls `Spawn` at `:5500` and `:5502` (search). Nothing in `scripts/`
  names it.

**Missions:**

| mission | file (mtime) | line | call | branch |
| --- | --- | --- | --- | --- |
| CHG01 | `scripts/missions/chg/chg_1_monster.lua` (2024-10-29 12:54) | 614, 635, 667, 811 | `Spawn` | search |
| CHG01 | same | 1038 | `Spawn(..., "", true)` | direct |
| CHG01 | same | 855, 979, 1010, 1142 | `luaSpawn` | direct |
| CHG02 | `chg_2_java.lua` (2024-08-03 13:10) | 289, 502, 522 | `luaSpawn` | direct |
| CHG03, MULTI931 | `chg_3_hunt.lua` (2024-08-03 13:10) | 789 | `Spawn(..., "", true)` | direct |
| CHG05 | `chg_5_tulagi.lua` (2024-08-26 16:10) | 287, 379, 382, 451 | `luaSquadronSupply` | direct |
| CHG07 | `chg_7_cargo.lua` (2024-07-13 08:26) | 771 | `luaSpawnDelayed` | direct |
| CHG07 | `PC_cargo.lua` (2024-07-13 08:26) | 266 | `luaSpawnDelayed` (`ST_RELANGLE`) | direct |
| CHG08 | `chg_8_periscope.lua` (2024-07-13 08:26) | 511, 637 | `luaSpawnDelayed` | direct |

Notes on the table:
- The `Spawn` lines in chg_1 (614-811) pass no trailing boolean, so they take the search branch with its
  single draw.
- chg_1:811 passes height 1000, which is a plane template.
- No stage script calls any of these (docs/CC13_OMITTED_TASKS_SCOPE.md 3.3).

## 8. Open questions

1. **`vtable+11Ch`.** Is its heading convention the one `apply_scene_yaw_00467050` uses? The concrete
   vtable of a spawned ship was not resolved.
2. **The pool and the hidden-record map.** Does `scene_spawn_pool()` hold exactly the records
   `BSP_SceneDatabase_FindHiddenEntityRecord` finds (SceneDatabase+18h)? The glue assumes so, from
   GenerateObject's use of it.
3. **Uncentred boxes.** Is any shipped map's box not centred? `00942CB0`'s border walk assumes `[-W, W]`.
   A one-pass read of the scene `Map` blocks would settle it.
4. **Unnamed `0046D930`.** What name does it give an instance whose name is empty (chg_3:789,
   chg_1:1038)? The glue keeps the template's name.

## 9. Diff

The unified diff applies to `origin/main` with `git apply --check`.

```diff
diff --git a/include/bsp/game_hosts_lua.hpp b/include/bsp/game_hosts_lua.hpp
index 00c3ba0b3..891ee5ca6 100644
--- a/include/bsp/game_hosts_lua.hpp
+++ b/include/bsp/game_hosts_lua.hpp
@@ -1330,6 +1330,11 @@ public:
     // 50000 (kSceneMarkerIdBase) and script entities start at 100000.
     static constexpr int kGeneratedSceneMarkerIdBase = 90000;
     int generated_scene_markers_{0};
+    // Packet cc13-spawn1 (bsp::kLuaSpawnBound, OFF): 00944680 Spawn. The placement is
+    // bsp/lua_spawn_cc13.hpp, the creation GenerateObject's. docs/CC13_LUA_SPAWN.md.
+    int run_spawn_00944680(lua_State* state, int argument_count);
+    // The keyed streams 00941FC0's draw takes instead of stream 1.
+    std::map<std::string, std::uint64_t> lua_spawn_streams_;
 
     // Packet cc8_spawn_new_route. 0094C480 SpawnNew is a three-instruction thunk
     // into 00949750 on the manager at *(00F89B3C): it parses ONE Lua table,
diff --git a/src/game_hosts_lua.cpp b/src/game_hosts_lua.cpp
index 7272a6905..2c1fd144d 100644
--- a/src/game_hosts_lua.cpp
+++ b/src/game_hosts_lua.cpp
@@ -37,6 +37,7 @@
 #include "bsp/global_script_folders.hpp"
 #include "bsp/lua_binding_mission_2.hpp"
 #include "bsp/lua_spawn_new.hpp"
+#include "bsp/lua_spawn_cc13.hpp"
 #include "bsp/mission_lobby_settings.hpp"
 #include "bsp/mission_lua_bindings.hpp"
 #include "bsp/mission_lua_machine.hpp"
@@ -535,6 +536,8 @@ int binding_trampoline(lua_State* state) {
     // queues is drained into the units host through this host's own frame step,
     // and its callback needs this host's `thisTable`.
     const bool spawn_new_row = dispatch_row.address == 0x0094c480u;
+    // Packet cc13-spawn1.
+    const bool spawn_row = bsp::kLuaSpawnBound && dispatch_row.address == 0x00944680u;
     // Packet cc9_spawn_new_id_queries.
     const bool spawn_id_requested_row = kLuaSpawnNewIdQueriesBound
         && dispatch_row.address == 0x00946380u;
@@ -570,7 +573,7 @@ int binding_trampoline(lua_State* state) {
         || untouchable_row
         || attack_alt_row
         || ready_row
-        || launch_row || generate_row || spawn_new_row || scoring_play_time_row
+        || launch_row || generate_row || spawn_new_row || spawn_row || scoring_play_time_row
         || spawn_id_requested_row || spawn_id_remove_row
         || select_unit_row || movie_add_row || force_select_row
         || (orders != nullptr && GameScriptOrdersHost::handles(dispatch_row.name));
@@ -793,6 +796,9 @@ int binding_trampoline(lua_State* state) {
     if (spawn_new_row && !host->error_replay()) {
         return host->run_spawn_new_00949750(state, argc);
     }
+    if (spawn_row && !host->error_replay()) {
+        return host->run_spawn_00944680(state, argc);
+    }
     if (spawn_id_requested_row || spawn_id_remove_row) {
         // 009458CF..00945923 (and 00945AD3..00945AEC): argument 0 of the
         // binding (stack slot 1) through 00B662B0 into a NativeString. A value
@@ -3484,6 +3490,182 @@ private:
 };
 }  // namespace
 
+// Packet cc13-spawn1: 00944680 Spawn (docs/CC13_LUA_SPAWN.md). The placement is
+// bsp/lua_spawn_cc13.hpp over the world box, the units and the landscape; the
+// creation is GenerateObject's (0046D930 is the creator under both 004C6BA0 and
+// 00944FD0). SUBSTITUTIONS, labelled:
+//  - 00941FC0's draw is on a keyed stream ("LuaSpawn#" + class + "#ring"), not stream 1;
+//  - the hidden record is the scene spawn pool's entry for the class name; a name the
+//    pool does not hold returns nil where the image dereferences a null record;
+//  - vtable+118h / +11Ch become the record's translation and apply_scene_yaw_00467050
+//    of the heading (vtable+11Ch's convention was not read);
+//  - every call creates a new unit; the pool entry is not marked spawned.
+namespace {
+class LuaSpawnWorld final : public bsp::SpawnPlacementHost {
+public:
+    LuaSpawnWorld(const bsp::WorldMapBounds& bounds,
+        std::map<std::string, std::uint64_t>& streams, std::string prefix)
+        : bounds_(bounds), streams_(streams), prefix_(std::move(prefix)) {}
+    float uniform_00bd2f10(const char* key, float low, float high) override {
+        const std::string name = prefix_ + key;
+        auto it = streams_.find(name);
+        if (it == streams_.end()) {
+            std::uint64_t seed = 1469598103934665603ULL;   // FNV-1a over the key
+            for (const unsigned char ch : name) {
+                seed ^= ch;
+                seed *= 1099511628211ULL;
+            }
+            it = streams_.emplace(name, seed).first;
+        }
+        it->second = it->second * 6364136223846793005ULL + 1442695040888963407ULL;
+        const double unit = static_cast<double>(it->second >> 11) * (1.0 / 9007199254740992.0);
+        return static_cast<float>(low + (static_cast<double>(high) - low) * unit);
+    }
+    bool ground_height_00903860(const float point[3], float& height) override {
+        return world_ground_height_00903860(point, height);
+    }
+    bool point_outside_map_0071c4f0(const float point[3]) override {
+        return bsp::point_outside_world_map_0071c4f0(bounds_, {point[0], point[1], point[2]});
+    }
+private:
+    const bsp::WorldMapBounds& bounds_;
+    std::map<std::string, std::uint64_t>& streams_;
+    std::string prefix_;
+};
+}  // namespace
+
+int GameMissionLuaHost::run_spawn_00944680(lua_State* state, int argument_count) {
+    if (state == nullptr || script_orders_ == nullptr) return 0;
+    const bsp::WorldMapBounds* bounds = world_border_bounds();
+    if (argument_count < 8 || ::lua_type(state, 1) != LUA_TSTRING || bounds == nullptr) {
+        ::lua_pushnil(state);
+        return 1;
+    }
+    const std::string class_name = ::lua_tostring(state, 1);
+    SceneSpawnPoolEntry* entry = scene_spawn_pool().find(class_name);
+    if (entry == nullptr) {
+        log_.notef("  Spawn 00944680: \"%s\" is in no held-back record; the image reads "
+            "record+10h of a null record at 00944BF5, this host returns nil", class_name.c_str());
+        ::lua_pushnil(state);
+        return 1;
+    }
+    GameSceneEntityRecord record = entry->record;
+    bsp::SpawnPlacementRequest request;
+    request.party = record.party;                                                 // record+10h
+    request.aircraft = bsp::spawn_class_is_aircraft_00944ead(record.class_name);  // 00944EAD
+    // Slot 2: an entity table (008889C0 at 009447D4; this host's handle is `ID`) or a
+    // vector3 (00888760).
+    float frame[16];
+    bool origin_entity = false;
+    if (::lua_type(state, 3) == LUA_TTABLE) {
+        ::lua_getfield(state, 3, "ID");
+        const int type = ::lua_type(state, -1);
+        const int id = (type == LUA_TNUMBER || type == LUA_TSTRING)
+            ? static_cast<int>(::lua_tonumber(state, -1)) : 0;
+        ::lua_pop(state, 1);
+        if (id > 0 && spawn_ref_entity_frame_008f8680(id, frame)) {
+            origin_entity = true;
+            for (int i = 0; i < 3; ++i) request.origin[i] = frame[12 + i];   // +FCh..+104h
+        }
+    }
+    if (!origin_entity && !read_vector3_00888760(state, 3, request.origin)) {
+        ::lua_pushnil(state);
+        return 1;
+    }
+    request.near_distance = static_cast<float>(::lua_tonumber(state, 4));
+    request.far_distance = static_cast<float>(::lua_tonumber(state, 5));
+    request.height = static_cast<float>(::lua_tonumber(state, 6));
+    // 0094497B / 009449BE: degrees * pi (00CE3D28) / 180 (00CE3D20).
+    request.angle_low = static_cast<float>(::lua_tonumber(state, 7) * 3.141592653589793 / 180.0);
+    request.angle_high = static_cast<float>(::lua_tonumber(state, 8) * 3.141592653589793 / 180.0);
+    if (static_cast<int>(::lua_tointeger(state, 2)) == 1 && origin_entity) {
+        // Mode 1 (009449E4): the origin entity's yaw from its +z row (+ECh, +F4h).
+        const float yaw = bsp::spawn_relative_yaw_00944a2c(frame[8], frame[10]);
+        request.angle_low += yaw;
+        request.angle_high += yaw;
+    }
+    std::string instance_name;
+    const int slot8 = argument_count > 8 ? ::lua_type(state, 9) : LUA_TNONE;
+    if (slot8 == LUA_TSTRING) instance_name = ::lua_tostring(state, 9);
+    // spawn_flag_slot's rule (lua_binding_spawn.hpp), inline: that object's
+    // spawn_request_id_matches collides with lua_spawn_new.obj's at link time.
+    int flag_slot = argument_count > 8 && (slot8 == LUA_TSTRING || slot8 == LUA_TNIL) ? 9 : 8;
+    if (argument_count <= flag_slot) flag_slot = -1;
+    if (flag_slot >= 0 && ::lua_type(state, flag_slot + 1) == LUA_TBOOLEAN) {
+        request.direct = ::lua_toboolean(state, flag_slot + 1) != 0;
+    }
+
+    bsp::SpawnWorldBox box;
+    box.min_x = bounds->north_west[0];   // +711Ch
+    box.min_y = bounds->north_west[1];   // +7120h
+    box.max_z = bounds->north_west[2];   // +7124h
+    box.max_x = bounds->south_east[0];   // +7128h
+    box.max_y = bounds->south_east[1];   // +712Ch
+    box.min_z = bounds->south_east[2];   // +7130h
+    std::vector<bsp::SpawnPlacementEntity> entities;
+    SpawnPlacementUnits(script_orders_->units()).placement_entities(entities);
+    LuaSpawnWorld world(*bounds, lua_spawn_streams_, "LuaSpawn#" + class_name + "#");
+    const bsp::SpawnResolvedPosition placed
+        = bsp::lua_spawn_resolve_position(request, box, entities, world);
+    if (!placed.ok) {
+        log_.notef("  Spawn 00944680: \"%s\" found no free point (00944F20), nil",
+            class_name.c_str());
+        ::lua_pushnil(state);
+        return 1;
+    }
+
+    if (!instance_name.empty()) record.name = instance_name;
+    record.world[12] = placed.position[0];
+    record.world[13] = placed.position[1];
+    record.world[14] = placed.position[2];
+    apply_scene_yaw_00467050(record.world, placed.heading);
+    const std::size_t units_before = script_orders_->units().count();
+    const std::uint32_t entity_id = script_orders_->create_unit_from_scene_record_0046db4b(record);
+    if (entity_id == 0u) {
+        log_.notef("  Spawn 00944680: \"%s\" (%s) reached no creator, so nothing was made",
+            class_name.c_str(), record.class_name.c_str());
+        ::lua_pushnil(state);
+        return 1;
+    }
+    bool attached = false;
+    if constexpr (kSEntityInitAllBound) {
+        if (record.class_id == 0x18) {
+            route_push_squadron(static_cast<int>(entity_id), record.name, record.type_id,
+                units_before);
+        } else {
+            route_push_entity(static_cast<int>(entity_id), record.name, record.type_id);
+        }
+        if (PendingEntity* node = find_pending(static_cast<int>(entity_id))) {
+            if constexpr (kGeneratedEntityPartyBound) {
+                node->party = record.party;
+                node->race = record.race;
+                node->generated_party = true;
+            }
+        }
+        run_sentity_init_all_00925f20(false, 0x0046dbe8u);
+        // 00944D8F MOV CL,1 / 00944D93 CALL 00874D00.
+        if (kRunExtraFixedStepBound && extra_fixed_step_ != nullptr) {
+            extra_fixed_step_->run_extra_fixed_step_00874d00(true, 0x00944d93u);
+        } else {
+            run_sentity_init_all_00925f20(false, 0x00874d79u);
+        }
+        attached = init_all_attached(static_cast<int>(entity_id));
+    } else {
+        static_cast<void>(units_before);
+        attached = attach_created_entity_00928a00(static_cast<int>(entity_id), record.name,
+                                                  record.type_id);
+    }
+    log_.notef("  Spawn 00944680: \"%s\" as \"%s\" (%s party=%d) -> id %u at (%.1f %.1f %.1f) "
+        "heading %.3f, %s stage %d%s", class_name.c_str(), record.name.c_str(),
+        record.class_name.c_str(), record.party, entity_id, placed.position[0],
+        placed.position[1], placed.position[2], placed.heading,
+        placed.direct ? "direct" : "search", placed.stage, attached ? "" : ", no thisTable slot");
+    log_.implemented("MissionLuaNative::Spawn", "00944680");
+    if (attached && push_resolved_entity_by_id(state, static_cast<int>(entity_id))) return 1;
+    ::lua_pushnil(state);
+    return 1;
+}
+
 bool GameMissionLuaHost::spawn_ref_entity_frame_008f8680(std::int32_t entity_id,
     float frame[16]) const {
     // 008F8680: [ref+14h] set -> (00414DB0 when [entity+C8h] == 0) and entity+CCh.
```

## 10. Handoff

The packet is complete. The next step is cc9's: decide whether to land the section 9 diff behind
`kLuaSpawnBound` and run the challenge missions (CHG01 first: 9 `Spawn` / `luaSpawn` sites, both
branches). Open question 1 is the first thing a challenge run would show (spawned ships facing the
wrong way).
