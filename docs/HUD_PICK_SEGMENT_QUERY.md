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
  (654) issues orders only.

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
