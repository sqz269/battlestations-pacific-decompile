# The 29h pick screen's segment query and the minimap camera heading (packet `cc9_hud_pick_segment_query`)

Addresses: 009043A0, 00526B5F..00526DB2 (the call and the hit branch of 00526A40), 004B4B00.

Ranks 29 and 30 of `docs/UNIMPLEMENTED_RANKING_2.md` (18,160 calls each on E2):
- `UnitPickScreen::segment_query` (009043A0);
- `HudMinimap::camera_heading_virtual` (004B4B00).

**Status.**
- The second is already bound. Its row no longer appears on current main (section 3).
- The first needs one read-only entry on the gunnery host, which owns the only segment-query
  binding in the process. Section 4 is that entry as a contract hunk. The HUD binding and the hit
  branch wait for it.

## 1. 009043A0: the ignore-aware wrapper of the segment query

`__stdcall bool(const float from[3], const float to[3], HitRecord* record, Unit* ignore,
int kind_filter)`, body 009043A0..009043FA, `RET 14h` at 009043DB. Ghidra has no prototype
(`FUN_009043a0`); the argument roles below are from the listing.
- **With `ignore`** (009043A7): 0042E630, the spatial-index singleton, becomes `this`. Then
  `ignore->vtable[B0h](record, kind_filter)` gives the entity to exclude (009043C5), and 0098ADD0
  runs as `QuerySegment(from, to, excluded, record, kind_filter)` (009043D4).
- **Without `ignore`**: the stack is rewritten so the exclusion is 0 (009043E8..009043EC), and it
  jumps to 0098ADD0 (009043F6).

0098ADD0 is read whole in `docs/HIT_NARROWPHASE.md`:
- It is a grid broadphase over `this+84h` (150 x 150 cells of [00CE3D90] world units, y not
  indexed), plus the unbucketed array `this+8h`.
- Per entity it tests the kind filter, the exclusion pointer, the AABB `+13Ch..+150h` against
  the segment's AABB, 0085CAD0's exact segment-box test, and 0098AC20's narrowphase.
- It keeps the nearest hit, shortening `to` to each hit.
- The index is the world's (`[00E188A8]+19CCh`). `construct_world` 004DE610 is a load record in
  this process, so there is no grid. The gunnery host answers the same query over its units
  (`SegmentBinding`, `src/game_hosts_gunnery.cpp`), with every unit in the loose array and the
  hull mesh box, or the hull extents, as the bounds.

## 2. The pick's call and what it does with the result

The call, 00526BC2..00526C4A:
- the camera basis from 00B6DB70 on game+19FCh's node;
- `from` = the node's position +120h..+128h;
- `to` = `from` + forward (+110h..+118h) * [00CE4BD8] (10000);
- the record reset by 00470470 at 00526B5F;
- the arguments are `push 0` (kind filter: none), `push EBP` (the firing unit from 004B4B00),
  the record, `to`, `from`;
- `ECX = [game+19CCh]` at 00526C44 is ignored by the wrapper.

After it returns:
- 00526C4F: no hit, go to the lock-radius walk (00526DB7);
- 00526C57: the plane-bot flag set, go to the lock-radius walk;
- 00526C62: `EBP = record+0` (the hit entity); zero, go to the lock-radius walk.

The hit branch, 00526C74..00526DB2 (never reached so far in this process):
- **The hit section.** 00526C80: if the hit is kind 6 (a ship) and `ESI = record+38h` (read as
  `[ESP+9Ch]`; the record starts at `[ESP+64h]`) has `+4h != 9`, the screen's +54h gets a section
  label by `+4h`:

  | `+4h` | label |
  |---|---|
  | 8 | 00CECE48 |
  | 7 | 00CECE3C |
  | 5 | 00CECE24 |
  | 6 | 00CECE0C |
  | anything else | EDI |

  Then:
  - `record+38h->+24h` float3 goes to screen+64h..+6Ch, and `->+20h` to +58h..+60h;
  - the hit's pose is refreshed through 00414DB0 when +C8h is clear;
  - 004134F0(screen+70h, hit+CCh) copies the hit's matrix.

  `docs/HIT_NARROWPHASE.md` found no writer of `record+38h`. The host's narrowphase leaves it
  null, so this part stays a record: `UnitPickScreen::hit_section`, 00526C8C.
- **The pick.** 00526D1A..00526D78: a hit that is kind 6, 0Fh, 45h, 46h, 1Bh or 35h is the pick
  itself.
- **Kind 1Eh.** 00526D80: the screen's +50h = the hit, and the pick is
  00923810(hit, 1), a record here: `UnitPickScreen::part_owner_00923810`.
- **The rest.** Any other kind gives no pick (00526DAF).
- **By ray.** With a pick, `*by_ray = 1` (00526DA7) and it jumps to 005271CB, the kind-44h
  check and the alive test. The lock-radius walk does not run.

**Consequence.** With the query bound, a ship on the camera's forward line within 10000 is
picked by ray, ahead of any radius candidate. The camera's own firing unit is excluded.

## 3. 004B4B00 and the minimap heading: already bound

004B4B00 (`__cdecl Unit*()`, body 004B4B00..004B4B3E) answers [00E188D8], the controlled unit, if
kind 5, its +3D0h if kind 18h, and null otherwise (`docs/HUD_MINIMAP_CAMERA.md`). The minimap
wedge at 005C1872 turns by that unit's vtable +C8h, 0042BA40:
`-atan2(unit+ECh, unit+F4h)`, minus the icon heading, plus pi/2.

`kHudMinimapDirectionWedgeBound` (packet cc9 minimap camera, `src/game_hosts_hud_world.cpp`)
already runs that through `camera_unit_004b4b00`. The record `HudMinimap::camera_heading_virtual`
is its fallback when there is no camera unit.

| Log (tree 98ef34226 + 1e7f0aeae, switch ON) | `camera_heading_0042ba40` | `camera_heading_virtual` |
|---|---|---|
| `local\rc_on_usn04.log` | 9,160 concrete | absent |
| `local\rc_on_usn02.log` | 18,160 concrete | absent |

The ranking's 18,160 was measured before that switch landed. Nothing is left to bind at 004B4B00
for the minimap.

## 4. The contract for the gunnery host (cc9-gunnery owns the file)

One read-only entry over the existing `SegmentBinding`. It has no counters and no log, so a HUD
caller cannot move a gunnery summary line. Kind filter 0 (the pick's) makes `entity_is_kind`
irrelevant; a caller with a non-zero filter would need `SegmentBinding::entity_is_kind` to answer
from the class chain first.

```cpp
// include/bsp/game_hosts_gunnery.hpp, public:
// Packet cc9_hud_pick_segment_query: 0098ADD0 over this host's units, as
// 009043A0 calls it with kind filter 0. `exclude` is one-based (0 = none),
// standing for ignore->vtable[B0h]. True with the nearest hit's one-based
// unit and point.
bool query_segment_units(const float from[3], const float to[3], std::size_t exclude,
    std::size_t& hit_unit, float hit_point[3]) const;

// src/game_hosts_gunnery.cpp:
bool GameGunneryHost::query_segment_units(const float from[3], const float to[3],
    std::size_t exclude, std::size_t& hit_unit, float hit_point[3]) const {
    hit_unit = 0;
    SegmentBinding query(*impl_, exclude == 0 ? static_cast<std::size_t>(-1) : exclude - 1);
    bsp::SegmentQueryArgs args;
    args.from = bsp::HitQueryPoint{from[0], from[1], from[2]};
    args.to = bsp::HitQueryPoint{to[0], to[1], to[2]};
    args.exclude_entity = reinterpret_cast<const void*>(exclude);
    bsp::HitRecordFill record;
    bsp::hit_record_reset_00470470(record);
    if (!bsp::query_segment_0098add0(query, args, record) || query.hit_unit == 0) return false;
    hit_unit = query.hit_unit;
    hit_point[0] = record.position.x;
    hit_point[1] = record.position.y;
    hit_point[2] = record.position.z;
    return true;
}
```

`SegmentBinding` takes `Impl&`. The entry is const, so it needs either a const-correct binding or
a `const_cast` the owner judges safe; the binding only reads.

**The HUD side, once the entry lands** (`src/game_hosts_hud.cpp`, one switch):
- `ray_pick_009043a0` calls the entry with `exclude` = the firing unit;
- `ray_hit_branch` implements section 2: the kind list, records for `hit_section` and 00923810,
  and 0 otherwise;
- predictions and the USN04 and USN02 pairs follow the standing pattern.

## 5. The binding and its predictions (the switch committed OFF)

The gunnery entry landed as 09298b10e under the integrator's arbitration of 2026-09-26. Section 4
was wrong on one point. The trace bumps `shell_mesh_hits` (per mesh hit) and
`narrowphase_box_0085cdb0` (per box hit), so the entry restores both after each query.

`kHudPickSegmentQueryBound` (`src/game_hosts_hud.cpp`):
- `ray_pick_009043a0` calls `query_segment_units(from, to, firing, ...)`.
- `ray_hit_branch` implements section 2:
  - kinds 6, 0Fh, 45h, 46h, 1Bh and 35h are the pick, counted as `ray_pick_own` or
    `ray_pick_other` by side against the controlled unit;
  - a ship hit also records `UnitPickScreen::hit_section` (00526C8C);
  - kind 1Eh records `UnitPickScreen::part_owner_00923810`;
  - any other kind counts `ray_hit_other_kind`.

**What reads the pick.** 00527260 stores it in +4Ch, and +B0h is the pick or its vtable +140h
owner.
- The weapon-group screen 2Eh reads +4Ch as its target (00548856). With gunner roles held and
  the target not on the first entry's side (+54h), it builds the fire message 00954A10 with the
  target's id. `route_fire_message` hands that to `GameGunneryHost::apply_gun_aim_message_00959c20`.
  **That is a gameplay path.** An enemy picked by ray, while the group's gunner path runs, can
  move the player's guns.
- The follow screen 49h reads +4Ch (0067BF44). That is camera and HUD only.
- The lock branches of 00527260 need input. With an idle player they do not run.

**Predictions, written before the pairs.** One tree (main 661e8bc45 plus 09298b10e plus this),
`local\bin\rp_off` against `local\bin\rp_on`, `BSP_GUNNERY_RNG_STREAMS=1`, 1600x900. USN04
4700/4500 and USN02 9200/9000.
- **Rows.**
  - `UnitPickScreen::segment_query` turns concrete at its OFF count (9,160 in USN04, 18,160 in
    USN02).
  - `UnitPickScreen::ray_hit_branch` (a record that never ran) is replaced by the new rows.
  - `segment_query_hit` counts the calls whose ray hits a unit other than the controlled one.
- **What the ray meets.** The camera follows the controlled unit: the Lexington in USN04, and
  the controlled ship of USN02. Its forward line runs over that ship and on for 10000. The ship
  itself is excluded.
  - Hits come from the ships of the controlled unit's own formation that lie ahead on that line.
    They are own-side ship hits, so they count `ray_pick_own` and `hit_section`.
  - Enemy aircraft or ships crossing the line count `ray_pick_other`.
  - I expect own-ship hits in the thousands, and enemy hits in the tens or none.
- **Resolved picks.** `UnitPickScreen::owner_140` rises by the ray picks. A ship is not kind 19h,
  so +B0h goes through vtable +140h.
- **Gameplay.** Own-side picks are filtered at the weapon group's side test and move nothing.
  - If `ray_pick_other` is zero, no gameplay row, per-entity row or summary line moves.
  - If it is non-zero, a move in the gunnery lines is possible through 00954A10. Any such move
    will be attributed through the `HudWeaponGroupScreen` rows, not called a finding.
  - A gameplay move with `ray_pick_other` at zero would be the finding.

## 6. The pairs and the verdict

One tree (661e8bc45 + 09298b10e + 222d60660), `local\bin\rp_off` against `local\bin\rp_on`,
`BSP_GUNNERY_RNG_STREAMS=1`. All four logs show the 1600x900 fit line and the final COM release.

| Line | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
|---|---|---|---|---|
| `segment_query` | 9,160 record | 9,160 concrete | 18,160 record | 18,160 concrete |
| `segment_query_hit` | - | 0 | - | 26 |
| `ray_pick_other` / `ray_pick_own` | - | 0 / 0 | - | 26 / 0 |
| `hit_section` (record, ship hits) | - | 0 | - | 26 |
| `owner_140` (record, resolved picks) | 0 | 0 | 0 | 26 |
| lock-radius walks (`list_1970`) | 9,160 | 9,160 | 18,160 | 18,134 |
| unimplemented total | 2,248,522 | 2,239,362 | 4,718,386 | 4,700,278 |

**Gameplay: nothing moved.** The per-entity tables and the gunnery damage lines are identical, so
the clock offset is zero, and no summary line differs. The 26 enemy picks in USN02 did not reach
the fire message: no `HudWeaponGroupScreen` row moved, so the weapon group's gunner path did not
run with them.

**The prediction of what the ray meets was wrong.** I expected own-formation hits in the
thousands. A probe build, not committed (`local\rp_probe.log`, USN04, every 500th query), shows
why there are none.
- The camera sits 53.7 above the sea, about 250 behind the Lexington, and its forward pitches
  down by about 10 degrees. `to` is at y = -1682.8 after 10000.
- The segment therefore enters the water about 300 ahead of the camera.
- 0098ADD0 tests unit boxes, not water, so only a unit within a few hundred of the camera on
  that line is hit. In USN04 nothing ever is.
- In USN02, 26 queries hit an enemy ship (kind 6), which became the pick by ray.

Whether the image's camera pitches the same way is the mission camera's contract, not this
packet's.

**Verdict: ON.** The query and the hit branch are the image's (sections 1 and 2), and no gameplay
line moves. `kHudPickSegmentQueryBound` is set true. Two named records remain:
- `UnitPickScreen::hit_section` (00526C8C), because record+38h has no writer;
- `UnitPickScreen::part_owner_00923810`, which did not run.

`owner_140` is an existing record of the pick's owner getter, now reached 26 times.

## 7. The ray's camera on a mission whose player unit is not a ship (packet `cc9_pick_ray_camera_read`, read)

**What the cast reads.**
- 00526B64..00526BC7 load `[[00E188A8]+19FCh]`, the camera **node**, with no null test.
- They refresh the node's world through 00B6DB70 when bit 2 of `+5Ch` is clear.
- They read the world translation `+120h..+128h` as `from` and row 2 `+110h..+118h` as the forward.
- The node is posed by whichever **mover** 004BC410 last installed (`docs/MISSION_CAMERA.md`
  section 2). The mover's tail 004329D0 pushes its pose into the node every frame.

**Which mover the player's interface installs.** The level-1 dispatch
(`docs/IN_GAME_INTERFACE_SCREEN_SETS.md`) maps the controlled unit's kind to an interface:
- **A ship** (kind 06h) takes **25h** `INTF_CAPTAIN`. Its arm calls 0064DA40 on `interface+7Ch`,
  which creates the `ShipCaptain` mover and installs it at 0064DC9C. This is the only mover the host
  models (`GameHudHost::Impl::bind_mission_camera_0064da40`). That is why USN02 (a ship) records 26
  ray picks from a real camera, and why USN04's Lexington camera sits 53.7 above the sea.
- **An airfield** (kind 45h, 0068AEAE) takes **2Eh** `INTF_AIRFIELD`. Its arm jumps straight to
  the shared tail 0068B23A: it leaves the screen set untouched and installs **no mover**.
  USN01's log reads `applied as 2Eh`, and the controlled unit is `Airfield2`.
- The only other publishing path, 2Ah `INTF_IDLECAMERA` at 0068AFAB, needs `game+1FE4h != 0`,
  which is multiplayer.

**So on USN01 the image's node keeps the pose of the last mover installed by something else.**
There are 18 rel32 callers of 004BC410. The candidates on this mission:
- **The in-game movie.** `usn_1_marshall.lua` runs `luaIntroMovie` (line 440), whose
  `luaIngameMovie` keyframes place the camera relative to `SaltLakeCity` and `CB2` (lines
  619..630). The movie screen's `BSP_HudMovieScreen_EnsureCamera` 005CC170 installs a mover at
  005CC2A9, and 005CDC50 (call at 005CDC72) is a second movie-screen setter.
- **Otherwise the node's pose before any mover**, which was not read. The node's constructor and
  its initial world were not located.

**Why the host's ray is (0,0,0) -> (0,0,0).** `camera_basis` answers from the ShipCaptain
publication only (`mission_camera_publication().ready`). With no ship there is no publication,
so both vectors are the record's zeros (`UnitPickScreen::camera_basis` 00526B71 UNIMPLEMENTED,
6,160 calls on USN01).
- **The image's ray is not degenerate.** An identity node alone would cast 10,000 along +Z from
  the origin.
- **So the zero ray is a host artefact.**
- **But the right replacement is not the ShipCaptain camera**, which the image never installs on
  USN01.

**Not bound.** No host model of either candidate camera exists, and a pose chosen without one would
be invented:
- the movie mover's keyframed pose after the intro, and what the movie screen restores when the
  movie ends;
- or the node's initial world.

A binding needs one of three reads first:
1. **The movie screen's mover**, 005CC170 / 005CDC50 and what `luaIngameMovie` leaves installed
   at the movie's end. If it stays installed, USN01's pick casts from the last keyframe near `CB2`.
2. **The node's construction** and its initial world, if the movie restores "no mover".
3. **The camera-control input path**, since a player on the airfield interface can move the view.

### 7.1 The node's own pose, and what the intro leaves installed (ruling (b), 2026-09-27)

**The node before any mover is a constant.** Two instructions write `game+19FCh`:
- **004DE78E, inside construct_world 004DE610.** It stores the result of 00B71A80
  `BSP_Camera_Construct` (the "Operator" camera, 004DE75E pushes 00CE7E04 = `Operator`), called at
  004DE782 when EDI is non-null, or 0.
  - The camera's base 00B6F5A0 `BSP_Node_Construct` writes identity into its matrices at `+B0h`,
    `+F0h` and `+60h` (ledger record).
  - So before any mover the node's world is identity. The pick would cast from (0,0,0) along row 2 =
    (0,0,1), 10,000 units.
- **004DAB7C**, the release through 00B6DFA0.

**But USN01's pick does not see that pose after the intro.** The mission runs `luaIntroMovie`
(`usn_1_marshall.lua` line 440). This installation's `scripts/global/commandhelpers.lua` then
takes the movie through these steps:
- `luaIngameMovie` (line 7747) arms the `luaIngameMovieBOStart` blackout. This host logs it running
  twice on USN01.
- `luaIngameMovieBOStart` (line 7802) starts `luaCamIngameMovieAuto` (line 7648). That feeds every
  keyframe to `MovCamNew_AddPosition`, the new movie camera, which is the interface
  runtime's `kMovieCameraNewInterface` path (`src/interface_runtime_tail.cpp`). Its mover is
  installed by the movie screen through 004BC410 at 005CC2A9 (005CC170
  `BSP_HudMovieScreen_EnsureCamera`).
- At the movie's end `luaCamOnTargetExt` (line 7824) does three things: it removes the input
  listener, kills the camera script and the delay, and calls the mission's callback. **It sets no
  camera and requests no interface.**
- The airfield's 2Eh arm installs no mover of its own. **So after the intro the movie mover stays
  installed.** The node carries the last keyframe's pose, relative to `CB2` in USN01's intro
  (lines 627..630), for the rest of the run, unless the mission's callback or a later movie changes
  it. The callback was not read.
- **Correction (section 8.6, 2026-09-27):** the callback does change it. `luaIntroMovieEnd`
  blacks out into `luaIn`, and `luaIn` calls `SetSelectedUnit(Mission.BmdGroup[1])`, which is
  Northampton (line 669). In the image that reaches 00645600 and a 20h push, so the 25h arm
  installs the ShipCaptain mover on Northampton at about t = 20.1 s. Also, the pick screen is
  not in the 2Ch movie set, so the pick casts nothing during the movie.

**This host has neither mover.**
- `toggle_movie_camera` 0068A160 and `toggle_new_movie_camera` 0068A1F0 are records in
  `src/game_hosts_hud.cpp`.
- `ensure_movie_camera` returns the screen's handle without a pose.
- Only the ShipCaptain mover publishes.

**Verdict: not bound.** On an airfield mission the image's pick ray depends on the movie mover's
keyframes. It is not the node's constant identity pose, which the intro replaces in its first
frames. Binding identity would be a pose the image never casts from after the intro.
- **The pick stays unbound on airfield missions.** The camera record `UnitPickScreen::camera_basis`
  00526B71 remains, with its zero ray, and is labelled as such.
- **Ship missions are unchanged and correct.** USN02 and USN04 cast from the ShipCaptain mover,
  so a pair there is identity by construction. None was run.
- **The movie camera becomes a later HUD packet:** `MovCamNew_AddPosition`'s keyframe mover and
  its pose at the last keyframe.


## 8. The new movie camera, part 1: the read (packet `cc9_movie_camera_mover`, docs only)

Worker cc9-init-passes, 2026-09-27, base main 27ab3a4b4. Ghidra was read only. This covers what
the movie camera is, how keyframes reach it, and how it poses the node. The interpolation bodies
(about 11 KB, mostly x87) are **unread**, so nothing is bound here. Section 8.4 is the plan for the
binding packet.

### 8.1 How USN01's intro starts the movie (V, this installation's scripts)

- **The script path.** `usn_1_marshall.lua` `luaIntroMovie` (line 609) calls `luaIngameMovie`
  with eight keyframes and `bo = true` (lines 618..631). `luaIngameMovie`
  (`commandhelpers.lua` 7747) arms `Blackout(true, "luaIngameMovieBOStart", 1)`. Then
  `luaIngameMovieBOStart` (7802) runs `luaCamIngameMovieAuto` (7648).
- **The keyframes.** `luaCamIngameMovieAuto` turns each entry into `{postype, position, starttime
  = sum of the earlier moveTimes, blendtime = moveTime, nonlinearblend, wanderer, smoothtime,
  transformtype, zoom}` and calls `MovCamNew_AddPosition` once per entry.
  - USN01 uses only `postype` camera/target, `position = {pos, parent}`, `moveTime` 0 or 7 and
    `transformtype = "keepall"`.
  - The parents are `SaltLakeCity` for keyframes 1..4 and `CB2` for 5..8.
  - The start times run 0, 0, 0, 0, 7, 7, 7, 7, and the total is 14 s.
  - The callback `luaCamOnTargetExt` is delayed to `starttime - 1` = 13 s.
- **No other native starts or stops the movie camera.** `EnableInput(false)` and
  `BlackBars(true)` come first. `luaCamOnTargetExt` (7824) removes the listener, kills the script
  and calls `luaIntroMovieEnd`, which is `Blackout(true, "luaIn", 3)` (mission line 648). `luaIn`
  (654) issues orders and, at line 669, calls `SetSelectedUnit(Northampton)`. The earlier text
  said "orders only", which is corrected in section 8.6.

### 8.2 The native side (V)

- **`MovCamNew_AddPosition` 008B79F0** (`LuaBinding_MovCamNew_AddPosition`) calls
  005CD240 `BSP_HudMovieScreen_GetCamera`.
  - On first use that engages the movie interface through 005CD1A0, which pushes
    `kMovieCameraNewInterface`. It then runs 005CC170, and returns `screen+1Ch`.
  - When the camera is non-null, 008B79F0 hands the table to 007A44D0.
  - **So the first keyframe starts the movie.** Nothing else does.
- **005CC170 `BSP_HudMovieScreen_EnsureCamera`**, once, while `screen+1Ch` is null:
  - allocates 570h bytes and constructs them through 0079D020 (vtable 00D04750, base 00432750);
  - places the camera in the world (00923870 with identity);
  - copies the camera node's current world `[[game+19FCh]+F0h]` (16 floats, after 00B6DB70) into
    the camera through 007A0860, so the movie starts from wherever the node was;
  - installs it as the mover through 004BC410 at 005CC2A9;
  - registers an observer pair.
- **Keyframe store.** 007A44D0 checks that `postype` is a string, then calls 007A42C0, which
  matches it case-insensitively:
  - `camera` builds one 104h keyframe (00799B00) with `+28h = 1`;
  - `target` builds one with `+28h = 0`;
  - `cameraandtarget` builds both, paired through `+20h` with the id `+DAh` taken from
    `camera+3C0h`.
  Each keyframe is parsed by 007A0EB0 (5 KB, unread) and inserted by 007A2CD0.
- **The mover update, slot `+DCh` = 0079A3B0** (`HudMovieCamera_Update`,
  0079A3B0..0079BBE1). ShipCaptain's is 00432ED0.
  - **Clock.** `+3B0h += dt` while `+391h` is set, else it is zeroed. With no keyframes in either
    track (`+508h` or `+484h` zero), the clock is zeroed and the update returns.
  - **Editor mode.** `+568h`, the input actions and the `CAMERA EDITOR` overlay (0079A4xx..,
    the `004C7070` prints) are a debug stepper, reached only while paused (`dt == 0`) with a
    selected keyframe `+514h`.
  - **Tracks.** Track `+414h` is the look-at: 00797DA0 sets it to `+3B0h` at 0079B288, and
    00798130 evaluates it at 0079B2A8 into `+418h..+420h`. That result is copied to
    `+384h..+38Ch`. Track `+498h` is the camera: set at 0079B29D and evaluated at 0079B2D7 into
    `+49Ch..+4A4h`, with the up vector at `+4A8h..+4B0h`.
  - **Ground clearance** (0079B2FB..0079B684), with the world present and `+4FCh` set: it samples
    `BSP_World_GroundHeightAt` along the camera's horizontal move and smooths the height offset
    `+520h`/`+524h` through 0078FAF0 (0079B691).
  - **Basis.** The forward is `(+418h..) - (+49Ch.. + (0, +524h, 0))`, normalized, or (0, 0, 1)
    when the length is under `[00D7A350]`. With `+4F4h` clear, the forward and the camera up go
    to `+548h..` and `+538h..`, 0085DC80 builds the basis, and the position is kept at
    `+558h..+560h`. Otherwise a matrix copy is used.
  - **FOV.** `+410h / +4F8h` goes to `[mover+1BCh]` (the node) through 00B6FBB0 at 0079B833.
  - **Pose.** The built matrix is copied into `mover+74h` (0079B83C), `+3B8h` is cleared, and
    00435410(dt) runs, the base mover step (0043541F end). The base tail 004329D0 then pushes the
    mover's pose into the node (`docs/MISSION_CAMERA.md` section 2).
- **Hand-over, slot `+124h` = 0079A340** (no Ghidra function, 0079A340..0079A34C): it stores the
  previous mover at `+3B8h`.
- **005CDC50 clears the mover.** `HudScreen38_ClearCameraMover` is slot `+18h` of screen 38h's
  vtable 00CF17D0 (entry 00CF17E8). Screen 38h is `GUI_movie` page 2. It calls 004BC410 with a
  null mover at 005CDC72, then five 00442190 pushes (36h, 37h, 38h, 53h, 33h). **When it runs on
  USN01 is unread.** If it runs at the movie's end, the node keeps the pose the movie mover last
  pushed, because nothing pushes after the clear. If it never runs, the movie mover keeps
  pushing its last-keyframe pose. Either way the pick ray casts from the movie's final pose, as
  long as the track evaluation holds the last keyframe after the clock passes it. That holding
  is unread in 00798130.

### 8.3 Why no binding in this packet

- **What a faithful pose needs:**
  - 007A0EB0's parse: which keys it reads (`pos`, `parent`, `polar`, `starttime`, `blendtime`,
    `nonlinearblend`, `smoothtime`, `transformtype`, `zoom`), and how `keepall` makes `pos`
    relative to the parent's full transform;
  - 007A2CD0's insertion order;
  - 00797DA0 and 00798130: the segment search, the blend and its curve, and the hold after the
    last keyframe;
  - 0078FAF0's smoothing, and `+4FCh`'s default;
  - 007A0860's seed.
  That is about 11 KB of x87 bodies. A binding from the Lua script's shape alone would be a
  modelled camera, not the image's.
- **The node's pose after the movie** also depends on when 005CDC50 runs.

### 8.4 Plan for the binding packet

1. Read 007A0EB0, 007A2CD0, 00799B00, 00797DA0, 00798130, 0078FAF0 and 007A0860, in that order,
   with the assembly where the pseudocode shows register inputs. The track calls are
   `__thiscall` on `camera+414h`/`+498h`, so their `ECX` is dropped by the decompiler.
2. Find what runs screen 38h's slot `+18h`: the screen manager's deactivate path in
   `docs/IN_MISSION_INTERFACE_MANAGER.md` (line 275: `screen->[5h]` then its `+1Ch`). Decide
   whether USN01's intro reaches it.
3. Bind in the HUD host: `ensure_movie_camera` builds the camera state, `MovCamNew_AddPosition`
   stores keyframes, and a per-frame update evaluates both tracks and publishes the pose as the
   mission camera. That last part is `camera_basis`'s other source besides the ShipCaptain
   publication.
4. Measure:
   - **USN01 3200/3000:** predict the logged pick-ray endpoints following the eight keyframes,
     then the final pose relative to `CB2`: pos (-60, 13, 0) in `CB2`'s frame, looking at
     (-50, 12, 0). Also predict land hits within the island bounds, `owner_140`, and gameplay
     identical unless a resolved pick reaches the weapon-group fire path.
   - **USN04 4700/4500:** identity.

### 8.5 Bodies without a Ghidra function, and names

- **No Ghidra function:** 0079A340..0079A34C inclusive (`RET 4` at 0079A34A, `INT3` from
  0079A34D), the movie camera's `+124h` hand-over.
- **Names added** (hypotheses): 008B79F0 `LuaBinding_MovCamNew_AddPosition`, 007A44D0
  `HudMovieCamera_AddPositionChecked`, 007A42C0 `HudMovieCamera_AddPosition`, 0079A3B0
  `HudMovieCamera_Update`, 00798130 `MovieCameraTrack_Evaluate`, 00797DA0
  `MovieCameraTrack_SetTime` and 005CDC50 `HudScreen38_ClearCameraMover`.

## 8.6 The new movie camera, part 2: the keyframe read, and why the pick never casts from it (packet `cc9_movie_camera_mover_bind`, read)

Worker cc9-movie-camera, 2026-09-27, base main 83ece7303. Ghidra was read only, from the listings
(the pseudocode drops the `__thiscall` receivers and the x87 stack). No binding is in this commit;
the scope question in "The premise" below went to the lead.

### The premise: what the image's pick casts from on USN01 (V)

Section 7.1 said USN01's `luaIn` "issues orders only". **It does not.** Line 669 of this
installation's `usn_1_marshall.lua` calls `SetSelectedUnit(Mission.BmdGroup[1])`, and
`BmdGroup[1]` is `Northampton` (line 275).
- 008AB260 `SetSelectedUnit` calls 00647300 `BSP_InGameHudRoot_SetSpectatedUnit`. When 00645060
  accepts the unit, that calls 00645600 `SetControlledUnit` and then 00647040. 00647040 pushes
  interface 20h with the unit's `+140h` payload through 004CC460.
- A ship classifies to 25h `INTF_CAPTAIN`, whose arm installs the ShipCaptain mover (0064DA40,
  section 7). The host models that mover already.
- This host leaves `SetSelectedUnit` UNIMPLEMENTED (2 calls on USN01, the first at t = 20.1 s,
  log line `blackout callback luaIn ran`).

**The pick screen is not running during the movie.** The first `MovCamNew_AddPosition` engages
the movie interface: 005CD240 calls 005CD1A0, which pushes 2Ch `kMovieCameraNewInterface`. 2Ch's
level-1 set is `{37h}` alone (0068ADC9, `kSetMovieCameraNew`), so screen 29h, the pick, exits.
So on USN01 the image casts:

| mission time | interface | pick ray from |
| --- | --- | --- |
| 0 .. 4.1 s | 2Eh airfield | the Operator node's identity pose (section 7.1) |
| 4.1 .. 20.1 s | 2Ch movie | no cast: 29h is not in the set |
| from 20.1 s | 25h captain | the ShipCaptain mover on Northampton |

The movie camera's pose is never the pick's source on USN01, because `SetSelectedUnit` is the
missing binding. Two more consequences apply to any movie binding:
- **005CD1A0 reseeds RNG streams 1 and 0** with 12345 and 54321 (`kMovieInterfaceRandomSeeds`).
  It does so once per screen instance, at the first movie of the mission.
- **USN04 and USN02 run in-game movies too**, with 6 and 8 `MovCamNew_AddPosition` calls in
  init-passes' logs. In the image the movie mover replaces the ShipCaptain until the script's
  `SetSelectedUnit` and `ForceSelectUnit` bring it back (`usn_19_coralus.lua` 1031..1032). A mover
  binding without that return path would leave the movie's last pose installed for the rest of
  the run.

**005CDC50 never runs on this path.** Slot `+18h` is a screen's enter virtual
(`docs/FRONTEND_STATE_MACHINE.md`, 0068D981). Screen 38h is the only member of 2Dh
`INTF_ENGINEMOVIECAMERA`'s set (0068ADEA). So the null mover is installed only when the engine
movie starts, never by `MovCamNew`.

### Keyframes: parse, insert, pair (V)

**Keyframe defaults.** 00799B00 constructs the 104h keyframe with these values:

| field | default | constant |
| --- | --- | --- |
| `+28h` camera | 1 | |
| `+26h` wanderer | 1 | |
| `+27h` | 1 | |
| `+2Ch` transform | 1 | |
| `+C0h` | -1.0 | 00D7A260 |
| `+C4h` zoom | 1.0 | 00D7A24C |
| `+C8h` | 1.0 | 00D7A24C |
| `+CCh` smoothtime | 1.0 | 00D7A24C |
| `+D0h` | 1.0 | 00D7A24C |
| `+D4h` nonlinear blend | 0.5 | 00CE3800 |
| `+DAh` pair id | -1 | |
| `+F8h` state | 0 | |

**007A0EB0 parse** (`__thiscall(keyframe, camera)`):
- **Inherited values.** If the keyframe's track (`camera+480h` list for target, `+504h` for camera)
  is non-empty, it copies `+CCh`, `+C4h`, `+26h`, `+2Ch` and `+28h` from the track's **last**
  keyframe. Otherwise `+26h` = 1 for camera and 0 for target, `+2Ch` = 1 and `+C4h` = 1.0.
- **`transformtype`.** The values map `keepnone`/`keepy`/`keepz`/`keepall` to `+2Ch` 0/1/2/3.
  The `_thennone` forms also set `+30h`.
- **`position`.**
  - `parent` is an entity table (00888C40) or `parentID`. 00799D70 stores it at `+1Ch` with an
    observer. A dead entity (`+5Eh`) gives null, and a kind-18h squadron gives its `+3D0h`.
  - `+24h` = parent attached.
  - `pos` is `{x,y,z}` by name (0079C040 and 0078FD70) or `{1,2,3}`, into `+34h..+3Ch`. `polar`,
    `upvector` (`+58h..`), `relativetotarget` (`+E8h`), `modifier`, `deckpos` and
    `terrainavoid` (`+D9h`) are not used by USN01.
  - Without an attached parent, the parent's world position `+FCh..+104h` is added to `pos`.
- **Other keys.**
  - `starttime` `+F0h`, `blendtime` `+F4h`.
  - `nonlinearblend` clamped to [0,1] (or `1 - linearblend`) `+D4h`.
  - `zoom` `+C4h`, `smoothtime` `+CCh`, `flyalt` `+DCh`, `maxcamspeed` `+E0h`.
  - `finishscript` and `event` (not used by USN01).

**007A2CD0 insert** (`__thiscall(camera, keyframe)`, RET 4). A camera keyframe goes into track
`+498h` through 007A0770, and 00798080 on `+414h` pairs it. A target keyframe does the reverse.
007A0770 (`__thiscall(track, keyframe)`):
- It sets `+F8h` = 0 and adds the camera clock `+3B0h` to the start time.
- With 0 keyframes in the track, start and blend are forced to 0.
- With 2 or more, a start time equal to the last keyframe's gets `+ 0.001`: the qword 00D7A318
  loaded `FADD double`, rounded to float, then clamped at 0.
- It appends at the end.

00798080 gives a keyframe with pair id < 0 and the first keyframe of the other track that has
pair id < 0 and the same start time a shared id from `camera+3C0h`.

**USN01's first movie**, all at clock 0:

| track | keyframe | start | blend |
| --- | --- | --- | --- |
| camera | 1 | 0 | 0 |
| camera | 4 | 0 | 7 |
| camera | 5 | 7 | 0 |
| camera | 8 | 7.001 | 7 |
| target | 2 | 0 | 0 |
| target | 3 | 0 | 0 |
| target | 6 | 7 | 0 |
| target | 7 | 7.001 | 0 |

The camera keyframes inherit wanderer 1. The target keyframes have 0.

### The track (V)

Track layout (84h bytes at `camera+414h` look-at and `+498h` camera):
- `+4h..+Ch` result position, `+10h..+18h` result up;
- `+5Ch` matrix-override flag, `+60h` zoom, `+64h` flyalt flag;
- `+68h` list (head `+6Ch`, size `+70h`), `+74h` the camera, `+78h` the smoothing half-window;
- `+7Ch`/`+80h` the last active keyframe and its index.

**00797DA0 SetTime(t)** (RET 4) rewinds only: every keyframe with `+F8h != 0` and start > t goes
back to state 0, and its modifier's `+2Ch` runs. It stores no time.

**007911E0 step** (`__thiscall(keyframe, track)` -> bool, RET 4):
- **Inactive.** State 0 with start > clock returns false.
- **Begin.** Otherwise state 0 calls 00791020 begin. Begin sets state 1, `+4h` = track and
  `+40h..` = `+34h..`, and `+64h..` = `+58h..`. It sets `+D0h` = `+CCh`. When `+C0h > 0`, it sets
  `+C4h` = `camera+410h / +C0h * +C4h`. It copies `+C8h` = `+C4h` and, with `+D8h`, re-seeds from
  the camera's current pose.
- **Every step.** `+C8h` = `+C4h`.
- **End of blend.** In state 1 with clock >= start + blend, the step runs `finishscript` (unless
  the camera `+390h` is set) and sets state 2. A finished keyframe stays active.

**00795C10 weight(track, dt)** (RET 8) -> float:
- ts = clock + dt. Start > ts gives 0.
- Otherwise it begins the keyframe if needed and evaluates the position through 00795650.
- With e = ts - start: e >= blend gives 1.0. Otherwise x = e / blend and the weight is
  `(1 - nl) * x + nl * f(x)`, with nl = `+D4h`. Values of nl over 1 apply f again, but the parse
  clamps nl to 1.
- **0078FCF0 `f`** (RET 4) is a quadratic ease-in-out: 0 below 0 and 1 above 1, `2x^2` below
  0.5, else `1 - 2(1-x)^2`. Each product is rounded to float.

**00795650 keyframe position** (`__fastcall(keyframe)`):
- **No parent, or a modifier.** Without a parent (`+24h` clear) or with a modifier (`+25h`),
  `+4Ch..` = `+40h..` and `+70h..` = `+64h..`.
- **Wanderer (`+26h`).** On a parent of kind 8 the flag clears. On kind 6, a ship, it records
  `yoff = pos.y - [[entity+538h]+A8h] * 0.5` (00D7A280 double). On kind 0Fh it takes an offset
  from 0078FCC0.
- **World matrix.** The parent's world matrix `+CCh..+108h` (refreshed through
  `BSP_EntityPose_RefreshWorld` when `+C8h` is clear) is copied. On kind 20h it is replaced by the
  `+3CCh`/`+3C8h` sub-object's `+F0h`.
- **keepall (`+2Ch` = 3).** `+4Ch..` = `+40h..` transformed as a point, and `+70h..` = `+64h..`
  rotated (translation row zeroed).
- **keepnone and keepy.** keepnone (0) and keepy (1) go through 004142A0 and 0085DAD0.
- **Adjustments.** On the kind-6 or kind-0Fh wanderer paths the offset is subtracted:
  `+4Ch..` -= (0, yoff, 0). `terrainavoid` then lifts y to at least ground + 00D7A210.

**00798130 evaluate** (`__thiscall(track)`, 00798130..00798C0F):
1. **Window.** s = min(`+78h`, clock). One pass over the list skips state-3 keyframes. It records
   each active keyframe in `+7Ch`/`+80h` and shrinks s to `(clock - start) * 0.99` for the first
   active keyframe and for every cut (`+27h` with blend <= 0). It shrinks s to
   `max((start - clock) * 0.99, 0)` for an upcoming cut or the last keyframe, and stops at the
   first upcoming cut. 0.99 is the qword 00CED5D0. s is 0 when the active keyframe is the last one.
2. **Samples.** There are n = 5 samples when s > 0.001 (00D7A23C), else 1. The jump table
   00798C10 puts them at dt = -s, -s/2, 0, s/2, s, or dt = 0 when n = 1.
3. **Blend.** For each keyframe not in state 3 and each sample, w = weight(track, dt):
   - w <= 0 skips it.
   - w >= 1, or the sample's first contribution, **replaces** the accumulated pose, up, zoom
     (`+C8h`) and window (`+D0h`). With `+BCh` it also copies the keyframe matrix `+7Ch` to
     `track+1Ch` and sets `+5Ch`.
   - Otherwise it **lerps**, `acc = kf * w + acc * (1 - w)`. With `+DCh` (flyalt) the height
     gets a lift through 00414C60 and 00419010.
4. **Average.** With n = 5, four passes of pairwise averaging give weights 1, 4, 6, 4, 1 over 16.
   The up vector is renormalized each pass (00BF7030 sqrt, 0 below length 0).
5. **Output.** `+4h..` pose, `+10h..` up, `+60h` zoom and `+78h` = the blended `+D0h`, so the
   window follows the active keyframe's smoothtime. With n = 5, `+5Ch` = 0.

**So the camera holds its last keyframe after the clock passes it.** A finished keyframe keeps
weight 1 and is never retired by this path. The hold is exact once the 1-second window no longer
reaches back past the last cut.

### Coverage

| routine | coverage |
| --- | --- |
| 007A0EB0 parse | complete for the keys USN01 uses; `modifier`, `deckpos`, `event` arms read to their allocations only |
| 007A2CD0, 007A0770, 00798080, 00797DA0 | complete |
| 007911E0, 00791020, 00795C10, 0078FCF0 | complete, except the modifier virtuals, which USN01 does not use |
| 00795650 | complete for keepall and the kind-6 wanderer; keepnone/keepy (004142A0, 0085DAD0) and the kind-0Fh offset 0078FCC0 are unread |
| 00798130 | complete, except the flyalt lift 007986C0..007987B4 (00414C60, 00419010), which is unread |
| 0079A3B0 update, 007A0860 seed, 0078FAF0 smoothing, 0079D020 constructor | unread in this commit |

### Named records left by the re-scope (lead ruling, 2026-09-27: option A)

The movie camera is not bound. Its natives and bodies stay records until packet
`cc9_movie_interface_and_reseed`:
- `MissionLuaNative::MovCamNew_AddPosition` 008B79F0 (UNIMPLEMENTED);
- `ensure_movie_camera` 005CC170;
- the keyframe store 007A44D0 and 007A42C0;
- the update 0079A3B0;
- `InGameInterfaceUpdate::toggle_new_movie_camera` 0068A1F0.

USN01's pick after `luaIn` comes from `SetSelectedUnit`, bound in packet `cc9_set_selected_unit`
(`docs/CONTROLLED_UNIT.md`, section "SetSelectedUnit and 00647300").

## 8.7 The movie interface and the reseed (packet `cc9_movie_interface_and_reseed`)

Worker cc9-movie-camera, 2026-09-27, base main 640b48f5d. Ghidra was read only.

### The movie screen 37h (V, listing)

- **Its vtable is 00CF17A8.** `+18h` enter is 005CD9C0 and `+1Ch` exit is 005CDB00. `+20h`
  update is 005CBAF0, the black-bar slide on `+21h`/`+22h`/`+24h`.
- **005CD9C0 enter** (no Ghidra function, 005CD9C0..005CDAF5 inclusive, `RET` at 005CDAF5):
  - when `+20h` is clear, 005CD1A0(`[00E188D8]`);
  - then 005CC170;
  - `game+61Dh` = 1;
  - 004D6480(game, {36h, 37h, 38h, 53h, 33h, 55h, 47h, 5Ah}, 1).
- **005CDB00 exit:**
  - 00B0D0B0(0.0) on `[00F8D39C]`;
  - input contexts 1Eh and 3 to level 0;
  - `game+61Dh` = 0;
  - the same eight ids to 004D6480 with 0.
- **Nothing clears `+20h` after 005CD229.** No write shows in 005CD1A0, 005CD240, the enter, the
  exit, the update or the other slots. **So 2Ch is pushed once per mission, on the first movie.**
  - 005CC170 installs the camera only while `+1Ch` is null. 004BC410 destroys the outgoing
    mover (`docs/MISSION_CAMERA.md` section 2), and the observer 00694A60 that 005CC2B4
    registers then clears `+1Ch`. So a later movie rebuilds and reinstalls the camera under
    whatever interface is current.
  - That last part is switch 3's concern, and the observer's clearing is inferred, not read.
- **All three measured missions leave 2Ch through `SetSelectedUnit`:**
  - USN01 `luaIn` line 669;
  - USN04 `luaWeHere` line 3352;
  - USN02 `luaIntroMovieEnd` line 657.
  None of them needs `ForceSelectUnit`.
- **`ForceSelectUnit` 008AAF30 calls 006485A0** (`__thiscall(hudRoot)`, 006485A0..00648644):
  1. `+C0h` = 0, then 00648290 rebuilds the root's unit lists and 00645710 settles the cursor
     `+C2h`/`+C4h`.
  2. With an empty list (`+90h..+94h`) it calls 00645600(null), then pushes 34h, or 2Ah in
     multiplayer, when the manager is idle.
  3. Otherwise it calls 00645600(00644A60()), the list entry at cursor `+C4h` in list `+8Ch`, or
     `+9Ch` when `+C2h` is set. Then, with a controlled unit, it tail-jumps to 00647040.
  **Not bound:** it needs the HUD root's unit lists and cursor, and no measured mission reaches
  it. USN04 calls it only from `luaZuikakuDeadMovie` and its end, which do not run within 4,500
  frames.

### Switch 1, `kMovieInterfacePushBound`: the 2Ch push (predictions written before the pairs)

The binding:
- The three MovCamNew rows (008B7850, 008B79F0, 008B7BA0) call `hud_movie_screen_camera_005cd240`
  first. That runs `bsp::movie_screen_camera_005cd240` over the HUD's movie-screen state.
- The first call pushes 2Ch with the controlled unit. `apply_pending_interface_0068aca0`
  services it through the 2Ch arm, which gives the level-1 set `{37h}`.
- The reseed, 005CC170 and the 1FFh session message stay records.
- The rows stay UNIMPLEMENTED, because the keyframe store and the FOV that follow are unbound.

**USN01 3200/3000** (the movie starts at t = 4.00 s, frame 80; `luaIn` at t = 20.1 s):
1. One `movie interface 005CD1A0 engaged` line, then `applied as 2Ch: screens 37h`. At t = 20.1 s,
   `applied as 25h` follows as before. The phase-2 movie at t = 91 s pushes nothing (`engages=1`).
   `summary ... movie interface bound=1 calls=12 engages=1 pushes_2ch=1`.
2. **Pick.**
   - `UnitPickScreen::update` falls from 6,160 to about 5,516 (2 per frame for the 322 frames in 2Ch).
   - Pick casts fall from 5,999 to about 5,355 (±30).
   - `camera_basis` UNIMPLEMENTED falls from 805 to about 160 (frames 0..80 only).
   - `HudMinimap::update` falls by the same 644.
3. **Gameplay identical.** Screens 29h, 49h, 44h and 35h drive no gameplay on USN01.

**USN04 4700/4500** (the movie starts at t = 4.00 s, frame 80; `luaWeHere` at t = 25.0 s):
1. `applied as 2Ch` at frame 80, then `applied as 25h` at t = 25.0 s.
2. **Screen updates.** The 25h screens stop for 420 frames:
   - `UnitPickScreen::update` falls from 9,160 to about 8,320;
   - pick casts fall from 8,999 to about 8,160;
   - seat casts fall from 8,997 to about 8,160;
   - `HudMinimap::update` falls from 9,160 to about 8,320.
3. **The ShipCaptain is not stepped from 4 to 25 s**, because its step rides the minimap and
   markers updates. After 25 s its sway and zoom state differ, so pick-ray endpoints may differ.
   Pick land hits stay 0.
4. **Weapon groups and roles.** Screen 2Eh's exit (005470A0) at 4 s and its enter (005494C0) at
   25 s add player role releases and takes.
5. **Gameplay identical**, because the first contact is at t = 93 s. The one route that could
   move it is a role on the Lexington's weapon groups left released at 25 s. If rows move, only
   the Lexington's own gunnery rows should move first.

**Switch 1, pairs and verdict.** The same tree at eaa9a301b, switch only, `BSP_GUNNERY_RNG_STREAMS=1`
and `BSP_DEATH_TABLE=1`, binaries `local\bin\mi_off` and `local\bin\mi_on`.

| row | USN01 OFF | USN01 ON | USN04 OFF | USN04 ON |
| --- | --- | --- | --- | --- |
| `pair_diff` exit, gameplay | | 1, identical | | 1, identical |
| deaths / hits / damage / shots | 7 / 150 / 2690.0 / 561 | same | 40 / 799 / 11621.4 / 6395 | same |
| interface sequence | 20h, 2Eh, 25h | 20h, 2Eh, **2Ch** at frame 81, 25h | 20h, 25h, 25h | 20h, 25h, **2Ch**, 25h |
| `UnitPickScreen::update` | 6,160 | 5,518 | 9,160 | 8,320 |
| pick casts | 5,999 | 5,357 | 8,999 | 8,159 |
| `camera_basis` UNIMPLEMENTED | 805 | 163 | 3 | 3 |
| seat casts | 0 | 0 | 8,997 | 8,157 |
| movie calls / engages / 2Ch pushes | | 12 / 1 / 1 | | 6 / 1 / 1 |

Every numeric prediction held.

Not predicted:
- USN04's `mission gunnery aim: steps` fell from 228,484 to 228,101, and `held_retakes` rose
  from 2 to 4. Both belong to screen 45h's seat, which does not run while 2Ch is up, and to the
  2Eh exit and enter. No gameplay row moved.
- USN02 was not paired. Its logs show both movies inside 9,000 frames (8 AddPosition calls), and
  each is left through `SetSelectedUnit(Mission.Houston)` (lines 657, 767). Only the first pushes
  2Ch.

**Verdict: ON** (`kMovieInterfacePushBound = true`).

### Switch 2, `kMovieReseedBound`: the reseed of streams 1 and 0 (predictions written before the pairs)

**What the image does (`docs/RANDOM_STREAMS.md` section 1, V):**
- 005CD1A0 seeds stream 1 = 12345 and stream 0 = 54321 through 00BD2FD0, before the 2Ch push.
- Three later seeds follow, all on the path switch 3 would take:
  - the movie camera's constructor 0079D020 (run inside the same engage by 005CC170) seeds
    stream 1 = 123;
  - its first step with dt > 0, 00798C80, seeds 12345 and 54321 again;
  - its destructor 0079A260 seeds stream 1 from wall-clock milliseconds when the ShipCaptain
    replaces it at `SetSelectedUnit`.
- So the image's gameplay stream after the first movie is not reproducible. The reseed at
  005CD1A0 is only the first of those writes.

**What the host has:**
- No stream-numbered gameplay generator. The gunnery host's shared LCG
  (`random_range_00bd2f10`) is the labelled stand-in for stream 1, and stream 0 has none.
- Under `BSP_GUNNERY_RNG_STREAMS=1`, every gunnery draw is key-local (splitmix of its key), so
  nothing reads the shared LCG.
- The binding `GameGunneryHost::reseed_shared_stream_00bd2fd0` sets the LCG state to 12345 for
  stream 1 and does nothing for stream 0. **It reaches only the default shared path.**

**Predictions, streams ON** (USN01 3200/3000 and USN04 4700/4500, the protocol pairs):
- Identity on every gameplay, per-entity and native row, apart from these:
  - `MovieInterface::seed_random_stream` 00BD2FD0 UNIMPLEMENTED calls goes from 2 to 1;
  - `MovieInterface::seed_random_stream_1` 005CD1B0 concrete appears with calls=1;
  - the summary gets `reseed_bound=1 stream1_reseeds=1`.
- `pair_diff` exit 1.

**Prediction, default path** (streams unset). This is **not a reference protocol**: it only shows
what the reseed reorders. It is one USN04 4700/4500 pair.
- From t = 4.0 s every shared draw is reordered: fire stagger, hull damage, hit effects, aim
  error, ship-AI torpedo, death mode and delay, bullet throw, torpedo gyro, component failure and
  ranging.
- Gameplay moves broadly (exit 3). Expected bands against its own OFF:
  - deaths within ±25%;
  - hit records and damage within ±20%;
  - shots within ±10%;
  - the first hit within ±10 s of OFF's;
  - death rows differ in membership and times.
- The clock offset stays 0, because the step sequence is unchanged.
- The switch is judged on the streams-ON pairs. The default pair is recorded as the reorder
  it causes.

**Switch 2, pairs and verdict.** The same tree at 9637882a8, switch only, binaries `local\bin\rs_off`
and `local\bin\rs_on`.

| pair | `pair_diff` | result |
| --- | --- | --- |
| USN01 3200/3000, streams ON | exit 1 | gameplay, death rows and unit table identical; the native rows move as predicted (00BD2FD0 record 2 -> 1, 005CD1B0 concrete 1); `stream1_reseeds 0 -> 1` |
| USN04 4700/4500, streams ON | exit 1 | the same: identical, and only the predicted rows |
| USN04 4700/4500, **default path** (streams unset; not a reference protocol) | exit 3 | deaths 40 / 40; hit records 754 / 747; hull hits 259 / 290; damage 11236.0 / 11015.0; shots 6355 / 5607; first hit 93.70 s / 92.70 s; torpedo-task releases 8 / 5 of 16; torpedo drops 2 / 1; plane water contacts 18 / 14; all 40 death rows changed in time or killer; clock offset 0 |

- **Failed band:** default-path shots fell 11.8%, outside the predicted ±10%. Every other band held.
- **Verdict: ON** (`kMovieReseedBound = true`), judged on the streams-ON pairs, which are identical.
- **Warning for reference rows.** Any reference run on the default path (streams unset) now draws
  from 12345 from the first movie on. Default-path rows taken before this commit are not
  comparable with rows taken after it.

### Switch 3, the movie camera mover: handoff (not bound)

The lead's ruling let switch 3 become a handoff if needed. It stops here on one open question,
not on context. What is read so far:

- **0079D020, the constructor** (0079D020..0079D1C3), fields after the base 00432750:
  - vtables 00D04750 / `+10h` 00D04738 / `+24h` 00D04730 / `+170h` 00D04718;
  - input latches `+394h..+39Bh` = 0; `+39Ch..+3A8h` = 0.0 and `+3ACh` = 1.0;
  - the tracks `+414h` and `+498h` (vtable 00D04714), each with an empty list (007967E0) and its
    camera back-pointer (`+488h`, `+50Ch`);
  - ground clearance `+4FCh` = 0 (off by default); `+51Ch`/`+520h`/`+524h` = 0.0;
  - clock `+3B0h` and `+3B4h` = 0.0; `+390h` = 0, **`+391h` = 0**; pair counter `+3C0h` = 0;
    editor `+568h` = 0; `+404h` = 0;
  - FOV `+410h` = [00CE7D20] (0.6981317, 40 degrees);
  - then 00BD2FD0(1, 123) at 0079D1AB, with EDX = EBX + 7Bh and EBX zeroed at 0079D065.
- **007A0860, the seed** (`__thiscall(camera, matrix)`, RET 4). It copies the 16 floats into
  `+528h` through 004134F0. When the camera track is empty, it inserts one camera keyframe and
  one target keyframe. Both have `+24h` = 0 (no parent), `+D8h` = 1 (take the camera's current
  pose at begin, 00791020's `+D8h` arm) and start = blend = 0. So each track starts with a
  seed keyframe, and USN01's track insertion gains one:
  - camera: seed 0, kf1 0, kf4 0.001, kf5 7, kf8 7.001;
  - target: seed 0, kf2 0, kf3 0.001, kf6 7, kf7 7.001.
- **Vtable 00D04750:**
  - `+5Ch` 0079A380 (IsKindOf 54h);
  - `+A0h` 0078E8E0, the stop: 0042A910, then `+391h` = 0 and `+410h` = 40 degrees;
  - `+DCh` 0079A3B0, the update;
  - `+124h` 0079A340, the hand-over.

**The open question: what sets `+391h`?**
- 0079A3B0 advances the clock `+3B0h += dt` only while `+391h` is non-zero (`CMP byte ptr
  [ESI+391h],0; JNZ` at 0079A3C3). Otherwise it stores 0.0.
- `scan-bytes "91 03 00 00" --limit 4000` finds ten sites image-wide. Only three touch this
  class: the constructor's `MOV [ESI+391h],BL` with BL = 0 at 0079D197, the stop slot's
  `MOV byte ptr [ESI+391h],0` at 0078E8F0, and the update's read.
  - The rest are rel32 bytes: 00548E46, 005D7F2C, 0067315B.
  - Or other classes: 007A9318 is in the old movie camera (00797070's class, whose `+390h` is a
    pointer), plus 009AA641, 00B0B1C0 and 00B5309D.
- A second scan finds no `LEA`/`ADD` of a `+390h`/`+391h` sub-object base in this class, and no
  word or dword write at `+390h`.
- By the listing, then, the new movie camera's clock stays 0. Only keyframes with start 0 are
  active: the seed, kf1 and kf2. USN01's intro would hold kf1's shot (SaltLakeCity plus
  (0, 8, 100)) and never cut to CB2.
- That contradicts what a scripted cinematic is for, so **the writer is probably one the scans
  cannot see**:
  - a block copy or memset over the object;
  - a write through a pointer to `+390h` held elsewhere;
  - or a base-class slot writing through a computed offset.
  See the memory notes on block-copy writers and sub-object bases.
- Static reading has not settled it, and the original executable must not be run. **Do not bind
  a clock rule until the writer is found.**

**Still unread:**
- the rest of 0079A3B0 past the clock and editor blocks (0079A584..0079BBE1);
- 0078FAF0, which ground clearance needs only when `+4FCh` is set;
- 00435410 and 004329D0, the base step and publish (`docs/MISSION_CAMERA.md` has the ShipCaptain
  side);
- keepnone and keepy (004142A0, 0085DAD0) and the polar arm's degree constant 00D046D8, which
  USN04's movies and USN01's phase-2 movie use;
- the 00694A60 observer that is thought to clear the screen's `+1Ch` when 004BC410 destroys the
  camera.

**What binding the mover would change on the pairs** (for whoever takes it):
- **USN04.** The ShipCaptain is destroyed at t = 4 s and rebuilt by 0064DA40 at t = 25 s (a new
  construct, then a retarget seed).
- **USN01.** The phase-2 movie at t = 91 s rebuilds the movie camera under 25h and replaces the
  ShipCaptain. The pick then casts from the movie camera for the rest of the run, because the
  later `SetSelectedUnit(ScoutBomba)` is rejected.
- **Both.** The camera's own reseeds (123, then 12345 and 54321 at the first step, then wall-clock
  at the destructor) join switch 2's.

### Switch 3, `kMovieMoverBound`: the movie camera mover (binding and predictions, written before the pairs)

**The +391h writer, found (supersedes the handoff above).** The camera's `+170h` sub-object is its
tick element (vtable 00D04718, registered through 00929E50, `docs/TICK_ELEMENT_OVERRIDES.md`).
- Slot `+8h` is 00798C80, run in wave 3 of each fixed step.
- On the first step with dt > 0 it sets `+221h` of the sub-object, which is **camera+391h**
  (170h + 221h), at 00798CA6. It then reseeds stream 1 = 12345 and stream 0 = 54321.
- On every later step it counts `+408h`, accumulates `+40Ch` and draws one stream-1 uniform in
  [0, 65535] (00D046A8) into a 16-entry ring at `+3C4h`. It also calls 00797ED0 on both tracks,
  which runs keyframe modifiers and events only.
- Slot `+4h` 00798C50 calls 00797E40 on both tracks, for modifiers only.
- The displacement scans missed the writer because its base is the sub-object.

**The rest of the update, read (0079B2A2..0079B866):**
1. It evaluates the look-at track and copies it to `+384h`, then evaluates the camera track.
2. **Ground clearance.** With the world present and dt > 0, and `+4FCh` clear, `+520h` becomes
   `-(2 * +51Ch + +524h)`, times 0.6 (double 00CEFF98) when positive. 0078FAF0 then moves `+51Ch`
   towards `+520h` with a rate limit and integrates `+524h`. The state starts at zero, so the
   camera-height offset `+524h` stays 0.
3. **Forward** is target minus camera, normalized. It is (0, 0, 1) below `len^2` 1e-8 (00D7A350).
4. **Basis.** With `+4F4h` clear, `+548h` = forward and `+538h` = the track's up, 0085DC80 builds
   the basis, and `+558h` = the camera position.
5. **FOV.** 00B6FBB0 receives `+410h / +4F8h`, where `+4F8h` is the camera track's zoom `+60h`.
6. **Pose.** `+74h` = the matrix, `+3B8h` = 0, then 00435410, which is 004329D0(dt).
7. In single player the update ends at 0079B866, because `game+1FE4h` is 0.

**Other reads:**
- The base 004329D0's water probe 0042F0C0 returns at once for this class: vtable `+130h` is
  0042A900, `XOR EAX,EAX`. So the published world is the matrix.
- **Position arms** (00795650, jump table 00795BB8):
  - keepnone: parent translation plus position, up unrotated;
  - keepy: row 1 forced to world up, then 0085DAD0, the position transformed, up unrotated;
  - keepz: row 1 forced to world up, then 0085DC80, both transformed; a ship wanderer takes the
    keepy arm;
  - keepall: both transformed.
- **The ship wanderer** subtracts (0, y - Height * 0.5, 0). The class field `+A8h` is `Height`.
- **The plane wanderer** subtracts plane+810h..818h. That field has no traced writer, so it is
  taken as zero (labelled).
- **The seed keyframes** have `+26h` = 1 from 00799B00 and are first in both tracks. Every mission
  keyframe inherits `+26h`, `+2Ch` (keepy) and `+CCh` (1.0) from them, so **look-at keyframes are
  wanderers too**.

**The binding** (`src/hud_movie_camera.cpp`, `src/game_hosts_hud.cpp`, `src/game_hosts_lua.cpp`,
`src/game_hosts_menu.cpp`):
- 005CC170 builds the camera, seeds it from the published node (identity before any mover) and
  installs it. The ShipCaptain is destroyed (`camera_bound = false`).
- The next 0064DA40 destroys the movie camera and clears the screen's `+1Ch`. The destructor's
  wall-clock reseed is a record.
- MovCamNew_AddPosition becomes concrete: 005CD240, then 007A44D0 with the parsed table.
- **SUBSTITUTIONS, labelled:**
  - The mover steps once per frame, from the first screen update (4Dh or 35h under 25h, 37h under
    2Ch), and 00798C80 runs once per frame (one 0.05 s fixed step per frame here).
  - The per-step stream-1 draw 00798D07 is not taken.
  - The camera's reseeds (constructor 123, first step 12345 and 54321) go through switch 2's
    stand-in.

**Predictions, streams ON and `BSP_DEATH_TABLE=1`:**

USN01 3200/3000:
- **Summary:** `builds=2 destroys=1 keyframes=16 active_at_end=1`.
  - The intro at t = 4.0 s adds 8 keyframes, 4 camera and 4 look-at. The builder replaced no mover.
  - `SetSelectedUnit` at 20.1 s destroys the movie camera when 0064DA40 binds Northampton's
    ShipCaptain.
  - The phase-2 movie at about 91 s adds 8, 4 `cameraandtarget` rows at 2 each. It rebuilds the
    camera under 25h and replaces the ShipCaptain.
- **Poses** (logged every 40 frames):
  - From t = 4.0 s the camera sits at SaltLakeCity's frame plus about (0, 8, 100). The look-at
    point is the same point, so the forward falls back to (0, 0, 1). It then blends towards
    (0, 8, 110) over 7 s.
  - It cuts to CB2's frame plus (-50, 12, 0) at clock about 7 s, then blends towards (-60, 13, 0).
  - In both segments the look-at stays on the first point of the pair, smoothed over 1 s windows.
- **Pick.** Identical to OFF until about 91 s, then `unit pick: camera basis now from the movie
  camera`. The rays point from the movie camera at ScoutDauntless or ConLeader.
- `UnitPickScreen::owner_140` goes from 0 to [1, 1,200], expected nonzero, because the look-at
  sits on a unit. Pick land hits stay in [0, 1,200].
- **Gameplay identical**, unless a resolved pick reaches the weapon-group fire path, which needs
  input.

USN04 4700/4500:
- **Summary:** `builds=1 destroys=1 keyframes=12 active_at_end=0`.
  - The intro at t = 4.0 s replaces the ShipCaptain on the Lexington and adds 8 keyframes.
  - `luaIntroMovieEnd` at about t = 18 s appends 4 more to the same camera, with start times
    offset by its clock.
  - `SetSelectedUnit(Lex)` at t = 25.0 s destroys the camera, and 0064DA40 **rebuilds** the
    ShipCaptain: a fresh construct with the retarget seed. OFF keeps the old ShipCaptain.
- **Pick and seat.** The ShipCaptain's pose after 25 s differs from OFF, so pick and seat ray
  endpoints differ from then. The counts stay the same (8,159 and 8,157).
- **Gameplay identical.** Seat and pick hits stay 0 on both sides.

**Switch 3, pairs and verdict.** The same tree at 93e87be80, switch only, streams ON,
`BSP_DEATH_TABLE=1`, binaries `local\bin\mv_off` and `local\bin\mv_on`.

| row | USN01 OFF | USN01 ON | USN04 OFF | USN04 ON |
| --- | --- | --- | --- | --- |
| `pair_diff` exit | | 1 | | **3** |
| deaths / hits / damage / shots | 7 / 150 / 2690.0 / 561 | same | 40 / 799 / 11621.4 / 6395 | **41 / 808 / 11721.4 / 6374** |
| movie builds / destroys / keyframes / active at end | | 2 / 1 / 16 / 1 | | 1 / 1 / 12 / 0 |
| `MovCamNew_AddPosition` | UNIMPLEMENTED 12 | concrete 12 | UNIMPLEMENTED 6 | concrete 6 |
| pick: `ray_pick_other` / `ray_pick_own` / `owner_140` | 0 / 0 / 0 | 2,240 / 160 / 2,404 | 0 / 0 / 0 | 1 / 0 / 0 |
| pick land hits | 0 | 22 | 0 | 0 |
| seat held ticks / returns | | | 7,071 / 5 | 5,707 / 3 |

**USN01: every predicted row held except one band.**
- The intro camera was built at frame 81 and replaced no mover. It started on SaltLakeCity's
  frame and cut to CB2 at clock 7 s: position (3928.4, 15.1, -3158.3) at 8 s, holding (3920.4,
  16.0, -3154.1) from 14 s.
- It was destroyed at frame 403 by Northampton's ShipCaptain. It was rebuilt at frame 1824 under
  25h, replacing that ShipCaptain, and the pick cast from it for the rest of the run.
- Gameplay, death rows and the unit table are identical.
- **Failed band:** `owner_140` reached 2,404, above the predicted [1, 1,200]. The look-at sits on
  ScoutDauntless or ConLeader, so almost every cast after 91 s resolves a unit.
- `camera_basis` UNIMPLEMENTED went 163 -> 162: one frame at 20.1 s cast from the movie's last pose
  before the ShipCaptain published.

**USN04: the identity prediction failed. Gameplay moved, and the cause is identified.**
- The movie camera replaced the Lexington's ShipCaptain at frame 81 and was destroyed at frame
  502, t = 25.1 s. 0064DA40 then built a **new** ShipCaptain with the retarget seed; OFF resumes
  the old one.
- The player gun seat aims the Lexington's guns along the camera ray (`docs/PLAYER_GUN_SEAT.md`
  sections 1 to 3). So its held guns and aim follow the camera from 25 s: held ticks 7,071 ->
  5,707, returns 5 -> 3.
- The first gunnery difference is one extra shot at step 2200 (t = 110 s). Deaths moved 40 -> 41,
  and the ship-AI rows follow the combat.
- The move is the binding's own consequence through the seat. It is faithful exactly insofar as
  the image destroys the ShipCaptain when the movie camera installs, and rebuilds it at the next
  0064DA40. That destruction is read: 004BC410's 00926D90 on the outgoing mover.
- The rebuild relies on 46h's `+20h` being cleared by that destruction's observer. That is
  **inferred**, like the screen's `+1Ch`, not read.

**Verdict: not flipped.** `kMovieMoverBound` stays OFF pending the lead's ruling, because it moves
USN04's reference rows through the seat and the identity prediction failed. The recommendation is
ON, once someone reads the 46h `+20h` observer clearing (the 00694A60 registration at 0064DCxx and
its callback).

**The screen 46h observer, read (lead ruling 2026-09-27: flip if confirmed).**
- 0064DA40 registers the ShipCaptain with the observer embedded at screen 46h's `+8h`
  (0064DBA5 `LEA EDX,[ESI+8]`, 0064DBD1 CALL 00694A60).
- That observer's vtable is 00CF7960, stored by the Init at 0068D0A4 over the base 00CE3CD4. Its
  slot `+4h` is **0064A4C0**, `__thiscall(observer, entity)`, RET 4:
  - when the entity is `[observer+14h]` (screen `+1Ch`, the unit), it clears that field;
  - otherwise, when it is `[observer+18h]` (screen `+20h`, the ShipCaptain), it clears that.
- **Slot `+4h` is the destruction notice.**
  - 004BC410's 00926D90(2) on the outgoing mover queues it for the on-killed dispatch
    (`docs/ENTITY_EVENT_QUEUES.md`).
  - That dispatch, 009274DE..00927510, calls 00696330(entity) while the entity's `+8h` observer
    list is non-empty.
  - 00696330 selects callback 00693550 `BSP_ObserverEndpoint_InvokeCallbackSlot04`, which calls
    `observer->vtable[4h](entity)`.
- So when the movie camera replaces it, the ShipCaptain's destruction clears 46h's `+20h`. The
  next 0064DA40 finds `+20h` null (0064DAB6) and builds a new ShipCaptain (0064DAC5..0064DB02),
  which gets the retarget seed. **The inferred step is confirmed**, and the host's
  `camera_bound = false` at the movie build models it.
- **Expected with the switch ON:**
  - USN04 moves through the player seat, as in the recorded pair (deaths 40 -> 41, hits
    799 -> 808, shots 6395 -> 6374 on that base). The exact rows may shift with main since then.
  - USN01's gameplay stays identical.

**Switch 3 verdict: ON** (`kMovieMoverBound = true`). The same tree at 04b3b0247, switch only,
streams ON and `BSP_DEATH_TABLE=1`, binaries `local\bin\fl_off` and `fl_on`.

| pair | `pair_diff` | result |
| --- | --- | --- |
| USN01 3200/3000 | exit 1 | gameplay identical (7 death rows, 28 unit rows) |
| USN04 4700/4500 | exit 3 | the expected move: deaths 40 -> 41, hit records 799 -> 808, damage 11621.4 -> 11721.4, shots 6395 -> 6374, torpedo-task releases 7 -> 6 of 16 (14 death rows changed, 1 only ON) |

USN04's move comes through the player seat's camera ray, from the ShipCaptain rebuilt at 25 s.
The observer read above confirms that rebuild.

## 9. Handoff (cc9-movie-camera retires after this commit)

Worker cc9-movie-camera, 2026-09-27, base main 87b51d106, branch `agent/cc9-movie-camera`, worktree
`J:\PROG\battlestations-pacific-decompile-cc9-movie-camera`. It holds no leases.

### 9.1 Switches this line set

| switch | file | state | doc |
| --- | --- | --- | --- |
| `kSetSelectedUnitBound` | `include/bsp/game_hosts_hud.hpp` | ON | `docs/CONTROLLED_UNIT.md`, SetSelectedUnit section |
| `kMovieInterfacePushBound` (2Ch push) | same | ON | section 8.7 |
| `kMovieReseedBound` (005CD1A0 reseed, default path only) | same | ON | section 8.7 |
| `kMovieMoverBound` (the movie camera mover) | same | ON | section 8.7 |
| `kWingConstructionLuaBound` | `include/bsp/game_hosts_lua.hpp` | OFF: the joint flip failed on sizes | `docs/WING_CONSTRUCTION_LUA.md`, `docs/CONSTRUCT_WORLD.md` 30.6 |

### 9.2 Open, in order of value

1. **The joint wing flip** (`docs/CONSTRUCT_WORLD.md` 30.6).
   - Every counter held, and the victims are the same.
   - USN04's air-order deltas on the current base are hits +4, hull hits +0 and shots -8. The
     prediction was +2, +2 and -7.
   - Flip both switches with those rows as the expected result, or re-measure on the next base.
2. **Movie camera, unsupported parse keys and arms.** These are counted as `unsupported` in the
   summary line:
   - modifiers (`goaround`, `gamecamera`, `fpscamera`);
   - `deckpos`, `upvector`, `relativetotarget`, `terrainavoid`, `flyalt` (the evaluator lift
     007986C0..007987B4), `event`, `finishscript` and the `_thennone` transforms.
   None of them occur in USN01, USN02 or USN04.
3. **Movie camera substitutions:**
   - one 00798C80 fixed step per frame;
   - the per-step stream-1 draw 00798D07 is not taken;
   - plane+810h is taken as zero;
   - `+5Eh` is read through `alive_and_visible`;
   - the destructor's wall-clock reseed 0079A2FB is a record.
   The pairing ids (`+20h`/`+DAh`) are not modelled; they are not read by the pose.
4. **`ForceSelectUnit` 008AAF30 -> 006485A0** is read (section 8.7) and unbound. It needs the HUD
   root's unit lists `+8Ch`/`+9Ch` and cursor `+C2h`/`+C4h` (00648290, 00645710, 00644A60). No
   measured mission reaches it.
5. **`SetSelectedUnit`'s records:**
   - 005251C0, the screen reset at `[manager+CCh]`;
   - the 00645600 broadcasts 00817380 and 0080E290, the 006952A0/00694A60 observer pair and the
     00954990 audio;
   - `vtable[124h]` for non-ship classes.
6. **From the cc9-init-passes handoff** (`docs/SENTITY_INIT_ATTACH_ORDER.md` 16.2), untouched:
   - the XLive slots `54h` and `184h` callers;
   - the deck-tick launch-lag demonstration run on a mission with a queued `LaunchSquadron`;
   - the loopback drain's per-poster contract.

### 9.3 Local files in the worktree

- **Pair binaries:** `local\bin\{ss,mi,rs,mv,fl,wl,wj,jf}_{off,on}`, each with its logs beside it
  in `local\` under the same prefix.
- **Scripts and message files:** `local\cc9-movie-camera-*`.
- **Raw listings and decompiles:** `local\mc\`.

## 10. The movie camera's substitutions and unsupported keys (packet `cc9_movie_camera_keys`)

Worker cc9-hud2, 2026-09-27, base main 73983fada. This takes items 2 and 3 of the section 9.2
queue. Each substitution and key was read in the image and given one of three outcomes: bound
behind its own switch, kept with a reason, or left for a later packet.

### 10.1 The substitutions (item 3)

| substitution | read | outcome |
| --- | --- | --- |
| one 00798C80 step per frame | 00798C80 is slot `+8h` of the tick element, wave 3, run once per fixed step. The step is `0.05f` (00D0DE84, `docs/ENTITY_THINK_DISPATCH.md`) | **exact under lockstep 0.05**, which every reference run uses: one frame is one fixed step. Kept, relabelled as exact under that protocol |
| the 00798D07 draw is not taken | the else arm (00798CD9..00798D36) runs on every step after the latch. It counts `+408h`, sums dt into `+40Ch`, draws `00BD2F10(ECX = 1, 0.0, [00D046A8] = 65535.0)` and stores the 00BF7420 conversion in the ring `+3C4h[+404h]`, wrapping after 15. No reader of the ring was found: displacement scans of 0078D880..007A4860 for `+254h`/`+294h`/`+298h`/`+29Ch` hit only 00798C80 | **switch 4**, `kMovieStepDrawBound` |
| plane+810h taken as zero | **the writer is found.** Plane+810h is a sub-object, not a field. 007CFD6D `LEA ECX,[ESI+810h]` in `BSP_PlaneUnitInstance_Construct` passes it to 007C4560, which zeroes its three vectors (`+0`, `+Ch`, `+18h`) and seeds `+24h` from stream 1. 007BE060, called only from the plane fixed step 007CE040 (one rel32 call at 007CE0D2, no absolute reference), rewrites `+0..+8` every step from the `Wanderer/*` tuning (`+1C0h..+1E8h`, `docs/GAME_TUNING_SINGLETON.md`). So `+810h..818h` is the **plane wanderer velocity**. The displacement scans missed it because the writer's base is the sub-object | **kept, and consistent.** The host has no plane wanderer: its plane pose advance takes `+810h` as zero too (`src/game_hosts_units.cpp`, the 007D8230 note). So subtracting zero matches the pose the host publishes. Binding the wanderer is a plane-host packet; it moves every plane path and draws stream 1 per plane per step |
| `+5Eh` read through `alive_and_visible` | 00799D70 tests `CMP byte [entity+5Eh],0` alone: set means no parent and return 0. Clear means a kind-18h entity is replaced by `[entity+3D0h]` | **switch 5**, `kMovieParentKilledByteBound` |
| the destructor's wall-clock reseed 0079A2FB | `00BD2FD0(1, ...)` with a wall-clock value truncated by FISTP at 0079A2EF | **kept as a record.** A wall-clock seed is not reproducible, and the reference protocol is deterministic |

**Switch 4's host path, a labelled substitution.** `src/game_hosts_gunnery.cpp` is leased to
cc9-units3, so the draw goes through the gunnery host's public `death_mode_draw_00bd2f10(1, ...)`.
With stream 1 that is the shared stand-in generator on the default path, the same generator every
gunnery draw and the switch 2 reseed use. Under `BSP_GUNNERY_RNG_STREAMS=1` its key is
`(death_mode, FFFFFFh)`, an index no unit has, so no other draw reads it. A dedicated entry point
can replace it once the gunnery file is free.

### 10.2 The unsupported keys (item 2)

The installation's scripts were searched for each key. None of these keys is reached by an idle
run of USN01, USN02 or USN04: every earlier pair logged `unsupported=0`. Their callers are mission
end and failure cameras (`luaMissionEnd_CamOnEnt`, `luaMissionFailedNew_CamOnFailEnt`,
`luaMissionCompletedNew_CamOnComplEnt`), `luaCamOnTargetNew`, and per-mission unit jumps such as
`jm01.lua` `luaJM1JumpToUnit`.

| key | parse (007A0EB0) | consumer | outcome |
| --- | --- | --- | --- |
| `terrainavoid` | boolean only, keyframe `+D9h` (007A1488). 00799C4B clears it | 00795B45: when set and 00903860 answers, `y = ground + 1.0` (double 00D7A210, rounded to float at 00795B75) unless y is already above it. Every arm joins it except the null parent | **switch 6**, `kMovieTerrainAvoidBound`. The host's `world_ground_height_00903860` is the same query (`docs/SCENE_CONTENTS_HOSTS.md` section 6) |
| `flyalt` | any non-nil value sets `+DCh = 1` (007A1D3C). **The number is not read** | the evaluator lift 007986C3.. in 00798130: y += interp(y) * weight * min(horizontal distance, cap) * scale, and it sets a byte at `+64h` of an object not yet identified | left: pure arithmetic, but x87 with four constants (00CE4BC4, 00CFD714, 00D046A0/00D04698, 00D04690) still to read from the listing |
| `maxcamspeed` | number, `+E0h` (007A1D8D) | **no reader found** in 0078D880..007A4860 | nothing to bind. The host silently skips the key, which matches |
| `relativetotarget` | boolean, camera keyframes only, `+E8h` (007A1913) | 00795758, the position routine's parent-matrix arm | left: one script (`bsm_04`) |
| `upvector` | `+58h..+60h` through 0079C040/0078FD70 | the keyframe up | left: no script uses it |
| `deckpos` | integer, the parent's vehicle camera array (`BSP_VehicleCameraArray_Resize`) | position | left: no script uses it |
| `modifier` `goaround` | 64h-byte object 007A0650, vtable 00D04C8C | `+E4h`: slot `+18h` at begin (00791020), slot `+8h` each evaluation (007911E0), slot `+0` destroy | left: a packet of its own |
| `modifier` `gamecamera` / `fpscamera` | 7Ch-byte object 007A06D0(0 or 1), vtable 00D04CC0, with an observer at `+38h`. It also sets `+CCh = 0` and `+25h = 1` | as above; `+25h` sends 00795650 down the unattached arm | left: needs the game camera it hands back to |
| `event` (`killed`, `noparent`, `noentity`) | 28h-byte event objects (00799FA0), a Lua `function` name | the tick element's 00797ED0 (section 8.7) | left |
| `finishscript` | a string, camera or named keyframes only | not read in this packet | left |
| `_thennone` transforms | `+30h` | 00795866/00795976/00795A8E | left |

### 10.3 Predictions (written before any run)

**Protocol.** Each pair is the same tree with the switch flipped by `tools/pair_export.py`, streams
ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player. Switches 5 and 6 are both predicted identical,
so they share one ON export; if that pair moves, they are split. Switch 4 also gets one
default-path USN04 pair (streams unset), which is a record and not a reference protocol.

**Switch 4, `kMovieStepDrawBound`, streams ON:**
- **USN01 3200/3000:** 1,498 camera steps over two camera instances, and each instance's first step
  latches without drawing. So `step_draws = 1496`, and `MovieCamera::fixed_step_draw` goes from
  UNIMPLEMENTED 1,498 to concrete 1,496.
- **USN04 4700/4500:** one instance, 420 steps, so `step_draws = 419`, and the record goes from
  UNIMPLEMENTED 420 to concrete 419.
- **Gameplay identical, `pair_diff` exit 1.** The draw's key is read by nothing else. Keyframes,
  poses, pick counts and casts are unchanged, because the ring has no reader.

**Switch 4, default path (streams unset), USN04:**
- **Gameplay moves, exit 3.** 419 extra draws on the shared generator between t = 4 s and 25 s
  shift every later stream-1 draw.
- Bands, taken from switch 2's default pair: deaths within ±25%, hit records and damage within
  ±20%, shots within ±10%, death rows differ.
- The clock offset stays 0.

**Switches 5 and 6 together, streams ON:**
- **USN01 and USN04: gameplay identical, exit 1.** Only the packet's summary line differs.
- Every keyframe parent is alive at its add, so the one-byte test and the four-byte gate agree.
  Keyframes stay 16 and 12, and every `movie camera pose` line is identical.
- Neither mission has a `terrainavoid` key, so `unsupported` stays 0 on both sides.
- Pick counts and casts are unchanged.

### 10.4 Pairs and verdicts

The OFF side is this tree's build at f8496adb7. The ON sides are `tools/pair_export.py` exports
of the same commit: `local\sd_on` flips switch 4, and `local\pt_on` flips switches 5 and 6
together. All runs use `BSP_DEATH_TABLE=1`, lockstep 0.05 and an idle player, with streams ON
unless marked otherwise.

| pair | `pair_diff` | result |
| --- | --- | --- |
| switch 4, USN01 3200/3000 | exit 1 | gameplay, 7 death rows and 28 unit rows identical. `step_draws` 0 -> 1,496; `MovieCamera::fixed_step_draw` UNIMPLEMENTED 1,498 -> concrete 1,496 |
| switch 4, USN04 4700/4500 | exit 1 | gameplay, 41 death rows and 81 unit rows identical; every other line identical. `step_draws` 0 -> 419; the record goes from UNIMPLEMENTED 420 to concrete 419 |
| switch 4, USN04, **default path** (streams unset; a record, not a reference protocol) | exit 3 | deaths 39 -> 45 (+15%); hit records 777 -> 789; hull hits 283 -> 347; damage 11168.6 -> 12153.0 (+8.8%); shots 6074 -> 5836 (-3.9%); first hit 92.70 -> 93.20 s; torpedo-task releases 3 -> 8 of 16; 39 death rows changed and 6 only ON; clock offset 0 |
| switches 5 and 6, USN01 | exit 1 | gameplay and every table identical; keyframes 16, `unsupported=0` on both sides; only the packet summary line changed |
| switches 5 and 6, USN04 | exit 1 | the same: keyframes 12, `unsupported=0`, every other line identical |

**Every prediction held,** including all the default-path bands.

**USN01's intro poses vary from run to run.** Every USN01 pair showed two to four differing
`movie camera pose` lines, all between frames 81 and 321, the intro camera's life.
- The differences are in `forward` at the fourth decimal. At frame 81 the camera sits exactly on
  its look-at point, so the direction of a sub-millimetre difference flips completely.
- **This does not come from the switches.** Two runs of the same OFF binary
  (`kk_off_usn01.log`, `kk_off2_usn01.log`) differ in the same way, and the two exports also
  differ from each other.
- Gameplay, the native table and every summary line are identical in all of these pairs.
- USN04's poses and USN01's phase-2 poses (after 91 s, where the pick casts from the camera) are
  identical across all runs.
- The source is not found. It is an open item: a value the intro keyframes read, such as the
  node seed or a parent pose at 4 s, differs by a few ulps between runs.

**Verdicts: all three switches ON.**
- `kMovieStepDrawBound = true`, judged on the streams-ON pairs like switch 2.
  - **Warning for reference rows.** From this commit, a default-path run draws 1,496 (USN01) or
    419 (USN04) extra stream-1 values during the movies. Default-path rows taken before this commit
    are not comparable with rows taken after it.
- `kMovieParentKilledByteBound = true`.
- `kMovieTerrainAvoidBound = true`. No measured mission reaches the key, so it is build-tested
  and identity-tested only.

### 10.5 USN01's intro pose noise: gone at the current base, source not identified (packet `cc9_intro_camera_noise`)

Worker cc9-hud3, 2026-09-27, base 961dd9645. No code changed. The diagnostic below was built
locally and reverted.

**The noise no longer occurs.** These are the intro `movie camera pose` lines of USN01, frames up
to 339.

| logs | first publish | intro pose lines |
| --- | --- | --- |
| cc9-hud2 `kk_off`, `kk_off2`, `fs_off`, `lp_off`, `ic_off`, `ic2_off`, `ld_off` (07:19..10:48) | frame 81, clock 0.00 | 6 distinct sets in 7 logs over several binaries; the same-binary pair `kk_off`/`kk_off2` differs; `kk_off2` and `ic2_off` match |
| cc9-hud2 `ic2_on`, `ld_on`, `ob_off`, `ob_on`, `aq_off`, `aq_on`, and this tree's `cq_off`, `cq_on`, `cq_on2` | frame 82, clock 0.05 | one set in all nine logs (six binaries) |
| this tree, five 500/300 runs of one diagnostic build (`cam_a` to `cam_e`) | frame 82 | identical, including hex dumps of every key's start, blend, state, evaluated point and parent pose |
| this tree, three runs with `kLoadTimeInitAllBound` flipped off locally (`cam_f` to `cam_h`) | frame 82 | identical |

**What the old noise was.**
- At frame 81 the first publish came before the fixed-step latch.
  - The update saw the camera not running and reset its clock to 0.0.
  - Camera key 2 (the 7 s move, start 0.001 after `kDuplicateStartStep`) then had weight 0.
  - So the camera stood exactly on its look-at point, and the forward normalised a vector at or
    below the floor. The fallback printed as (0,0,-1).
- In the other runs the forward was a unit vector along the parent's heading, (-0.5547,0,-0.8321)
  or (-0.3714,-0.0007,-0.9285). That is the direction camera key 2 moves, so key 2 had a tiny
  non-zero weight, or the parent pose differed by a few ulps.
- The later lines (frames 161 and 241) also differed at the fourth decimal. That points to a
  parent pose that varied by ulps, which the degenerate frame only amplified.

**Ruled out at the current base:**
- The camera's `seconds` is 0.05 on every step: the mission frame substitutes
  `--mission-frame-seconds` before the interface pump.
- The parent pose is a plain copy of the unit's world matrix (`GameUnitsHost::unit_pose`).
- The keyframe fields are all value-initialised.
- The track evaluation is straight-line float code.

**Why it stopped is not established.**
- The first run at frame 82 is `ic2_on` at 10:43, while `ic_on` (10:40) and `ld_off` (10:48) are
  still at frame 81.
- Flipping `kLoadTimeInitAllBound` off at this base does not bring frame 81 back. So a landing on
  main between those runs changed the start frame, not that switch.
- The frame-81 and frame-82 logs of one period came from different trees and bases. The varying
  input (a ulp-level parent pose) was not caught in a dump, because no current run shows it.
- **Status:** closed as not reproducible.
- **If it returns:** dump both tracks in hex at the first three publishes, and each
  `step_mission_camera` call's `seconds`, running flag and clock. That is the diagnostic used
  here: a block after `publish_mission_camera` in `step_movie_camera`, and one before it in
  `step_mission_camera`. Diff two runs of one binary.

**For pairs:** USN01's intro pose lines are no longer known noise at this base. A moved intro pose
line in a same-tree pair now belongs to the change. `tools/pair_diff.py` never masked them.
