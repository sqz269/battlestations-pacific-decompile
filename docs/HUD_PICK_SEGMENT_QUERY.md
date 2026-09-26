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
