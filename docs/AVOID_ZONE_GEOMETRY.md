# Avoid-zone geometry: the polygons an AI plan bends around

Addresses: 004179d0 00416dd0 00416b50 004178f0 00412120 00422500 0041d1e0 0085c910 00417ca0 0041f600

Packet `cc_ai_avoid_zones`, worker `agent/cc-ai-avoid-zones`. Project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`; Ghidra was READ-ONLY for this packet and every descriptive
name here is a hypothesis, not a recovered symbol. Reconstruction:
`include/bsp/avoid_zone_geometry.hpp`, `src/avoid_zone_geometry.cpp`; report
`reports/avoid_zone_geometry.json`.

`docs/SHIP_AI_PATH_SEARCH.md` left the search's zone side a contract: "`00422500`'s tangent walk
and `004179D0` are unread, so the zone polygon's shape stays a host contract", and
`docs/GAME_EXECUTABLE.md` milestone 2q follow-up 6 adds "until it lands, `00417E90` answers not
blocked and every plan is a straight line". This packet is that shape. Everything below is the
polygon, the tests over it and the walk that picks the two detour corners; the manager's
singleton (`004218E0`), its two wrappers (`00417E90`, `00417EF0`), the layer lookup `004120D0`,
the planner's push-out pair (`00417580`, `0041B840`) and the search itself (`009E3040`) belong to
other packets and are referenced, not annotated.

## The data

The manager is `78h` bytes (`0042193A`). Its geometry is twenty inline group pointers at `+8h`
and their count at `+58h` (`0041F600`'s `14h` loop). It has no vtable of its own.

| Record | Size | Offsets |
| --- | --- | --- |
| layer group | `14h`, `00417CC7` | `+0h` vtable `00CE3858` (`00417CD5`), `+4h` zone pointer array, `+8h` count, `+0Ch` capacity, `+10h` layer key (`00417CE4` starts it at `-1`) |
| zone | `24h`, `0041D212` | `+0h` corner pointer array (`0041CCEF`), `+4h` count, `+8h` capacity, `+0Ch` layer key (`0041CD06`), `+10h` an owner object (`0041D099`, unread), `+14h`/`+18h` bounds min (`0041CD43`), `+1Ch`/`+20h` bounds max (`0041CD73`) |
| corner | `24h`, `0041CD4D` | `+0h`/`+4h` position, `+8h`/`+0Ch` unit direction of the edge leaving it, `+10h`/`+14h` unit corner normal, `+18h` that edge's length, `+1Ch` the signed turn at the corner, `+20h` set to `-1.0` (`0041CDC0`) and never written again |

Both vectors are hand-grown, not `std::vector`: capacity doubles plus two (`0041CDDD`,
`0041D25B`) and the elements are pointers. The plane is `(x, z)`: `0041CCD0` reads each path
point as a `float3` through `007AF800` and keeps `[0]` and `[2]`, so a zone has no height and the
`y` of an authored point is discarded.

Two lookups return a group for a layer key, and both assume `00417CA0`'s ascending order:
`004120D0` (`00417E90`, `00417EF0`, `009E3780`) returns the exact key, else the last group whose
key is below it, else slot 0; `00412120` (`00417E40`) records every key at or below the target and
breaks on the exact one. On the sorted array they agree on every input. Neither checks the count
before reading slot 0, so an empty table reads uninitialised memory; after a rebuild slot 0
always exists because `00424DDA` asks for layer 0 unconditionally.

## The producer: what a mission authors

`004DFB70` calls `004218E0` at `004E07B7` and `00424D00` at `004E07BE`
(`docs/MISSION_LOAD_HOSTS.md` section 4). `00424D00` clears the table (`0041F600` at `00424D22`)
and walks the world entity list, and for every entity whose name begins with `AvoidZone`
(`00424D7A`) it takes a layer key and adds the entity:

| Site | Effect |
| --- | --- |
| `00424D8E` | the layer slot is zeroed, so a name without the group form stays on layer 0 |
| `00424D92` | `_strncmp(name, "AvoidZoneG", 10)` |
| `00424DAE` | `_sscanf(name, "AvoidZoneG %*s %d #%03d", &layer, &index)`; the index is parsed and never used, and the skipped token is a word the format ignores |
| `00424DBE` | `00417CA0(manager)(layer)`, find or create that group, **before** the entity is examined |
| `00424DC5` | `0041D1E0(group)(entity)` |
| `0041D201` | `007AC9D0` for the entity's path interface |
| `0041D206` | `CMP dword ptr [EDI+54h], 2`; anything else is skipped |
| `0041D233` | `0041CCD0(zone)(path, group+10h)` builds the polygon |
| `0041D24B` | a polygon with no corners is logged through `004167C0` and freed, not registered |
| `00424DDA` | `00417CA0(0)` |

`0041CCD0` keeps path point 0 and then every later point whose squared distance from the last
kept one is above `25.0` (`0041CEDB`), growing the AABB over them (`00415010` at `0041CFB9`), and
finishes with `0041A200` (`0041D063`). `0041A200` fills each corner's outgoing direction and
length, its normal and its turn angle, sums the turns, and when the sum is above zero reverses the
corner order and measures again with the reverse disabled (`0041A47E`), so a finished polygon
always has a turn sum at or below zero. In the bearing convention below that is `-2*pi` for a
simple closed polygon, which is counter-clockwise in a top-down view.

**The scene record is `(Path)`, nothing else.** Every one of the 6476 `AvoidZone*` entities in
the shipped `universe/scenes` is an `entity "<name>" (Path)` with `template "Path"` and a
`"PathPoints"` block of `Pos = V3` triples; the name carries the layer
(`AvoidZoneG all <layer> #<nnn>`). Sixteen of them use the bare form (`AvoidZone1` in
`bsm_04_vengance_at_luzon.scn` and its five siblings) and fall to layer 0. The layer keys the
shipped missions use are 0, 1, 3, 5, 10, 11, 26, 46 and 86; `usn_1_marshall.scn` holds 21 zones
over five layers with a median of 123 authored points each.

**`usn_2_java.scn` authors no avoid zones at all** (zero matches for the literal), and neither do
`usn_9_santa`, `usn_14_phil_sea`, `usn_17_engano` or `usn_19_coralus`. A ship AI in those missions
gets an empty layer 0 group and every plan really is a straight line, whatever this packet lands.
`usn_12_augusta` (67) and `usn_13_truk` (87) are the dense ones.

## The tests

`004179D0` `__thiscall(group)(const float2* toward, const float2* from, float2* running,
zone** hit_zone, int* edge) -> bool in AL`, `RET 14h`, body `004179D0-00417A39`. It copies
`*from` into the running point once (`004179D4`..`004179E6`) and then hands the **same** running
point to every zone in the group, so the answer is not "some crossing" but the crossing nearest
`toward` over the whole group, with the zone and the edge that produced it.

`00416DD0` `__thiscall(zone)(const float2* toward, const float2* start, float2* running,
int* edge) -> bool in AL`, `RET 10h`, body `00416DD0-00416F20`. The AABB rejects first through
the adjustor thunk `004F2B00` (`LEA EDX,[ECX+8]`, so `0085C910` receives the zone's `+14h` min
and `+1Ch` max). Then it walks the closed polygon, stopping as soon as the running point is within
`0.01` squared units of `toward` (`00416E92`), and for each edge asks `004F3730` whether the
segment `(toward, running)` crosses it; a crossing moves the running point onto it and records the
edge. Because the running point only ever moves toward `toward`, the last recorded crossing is the
one nearest `toward`. The edge index counts from `-1` and is incremented after the test, so it
names the crossing edge's **first** corner, with `00416F11` wrapping a `-1` to the last corner.

`00416B50` `__thiscall(zone)(const float2*) -> bool in AL`, `RET 4`, body `00416B50-00416CC4`.
Half-open AABB reject (`min <=` and `< max`), then a crossing-number walk: an edge counts when the
point's `y` lies between the edge's ends, one end inclusive, and the edge's interpolated `x` is
left of the point. Odd means inside. `004178F0` `__thiscall(group)(const float2*) -> zone*`,
`RET 4`, body `004178F0-0041793B`, returns the first zone of the group that answers yes; that is
the body behind `00417E40 BSP_AvoidZoneManager_ZoneContainingPoint`.

`0085C910` `__fastcall(ECX min, EDX max, b0, b1) -> bool`, `RET 8`, body `0085C910-0085CAC7`, is
a 2D separating-axis test: the box half extent, the segment half vector and the half difference of
the two centres (`00D7A280` is `0.5`), tested on the two box axes and on the segment's
perpendicular.

## The tangent walk, `00422500`

`__thiscall(zone)(float far_x, float far_z, int edge_index, int near_corner_hint,
int far_side_hint, float margin, float2* out_backward, float2* out_forward,
int* out_backward_index, int* out_forward_index) -> int in EAX`, `RET 28h`, body
`00422500-0042318F`, read whole.

Every angle is the compass bearing `wrap_2pi(pi/2 - atan2(dy, dx))`, rounded to float at each
step (`004225C4`), guarded by a squared-distance gate of `1.0` that leaves the bearing at zero for
a pair closer than one unit, and corrected by one `2*pi` (`004225E0`), never a loop.

Two mirrored passes start at the corner the blocked edge begins at. Each carries one angle: the
bearing from the walking corner to the far point minus the bearing of the edge leaving it, moved
from corner to corner by subtracting (forward, `00422752`) or adding (backward, `00422D58`) that
corner's `+1Ch` turn. The flag at `[ESP+13h]` records that the angle has been on the far side
(`004226C7`: forward starts set when the angle is below `0.1`; `00422CCC`: backward when it is
above `-0.1`). A corner is a candidate the first time the angle comes back across zero with the
flag set, and it is accepted when the angle recomputed exactly at that corner has the right sign
(`< 0` forward at `004228AE`, `> 0` backward at `00422EDE`). That corner is the tangent point from
the far point to the polygon on that side.

The published point is the corner pushed `margin` units along the normalised sum of the unit
vectors to its two neighbours, which points out of the polygon at a convex corner. When that sum
is degenerate (`00422A80` / `00423012`, squared length at or below `1e-6`, the two edges are
opposite) the fallback is the perpendicular of the vector to one neighbour: forward takes the
previous corner and `(dy, -dx)`, backward the next corner and `(-dy, dx)`. When the margin is
above ten units (`00422B85`, `00423136`) the push is re-tested: `00417EF0` is asked for the
crossing on the segment from the pushed point back to the corner plus one unit, over the zone's
own layer (`[ESI+0Ch]`), and the published point is pulled back to it. The search passes 30 for
the margin (`docs/SHIP_AI_PATH_SEARCH.md`), so the pull-back always runs there.

`far_side_hint` gates the passes: `>= 0` runs the forward one (`004226ED`), `<= 0` the backward
one (`00422CF9`), so 0 runs both. `near_corner_hint >= 0` stops a pass after that many corners,
the gap from `edge_index` to the hint measured the way that pass walks (`004226DD`, `00422CE9`).
At the search's call site both come off the node: `009E30B6 MOVSX ECX, byte ptr [EDI+14h]` or zero,
and `009E30C3 MOVSX EAX, word ptr [EBX+0Ch]` or `-1`.

**The two indices are not symmetric.** The forward index is `edge_index + steps` (`00422BC2`) and
names the corner it published. The backward index is `edge_index - steps` (`00423168`), one below
the corner it published. Read as an edge index - which is what `edge_index` itself is, since
`00416DD0` reports an edge by its first corner - both name the edge that leaves the published
corner in that pass's direction of travel. That reading is provisional; what is established is
the arithmetic and that the backward corner is `index + 1`.

The return value is the forward flag (`00422BD2` sets it to 1) minus one on a backward success
(`00423186`), so `+1` means only the forward corner, `-1` only the backward one, and `0` means
both **or neither**. The caller separates those two by the sentinel it writes into both points
before the call (`009E3082`, the constant at `00CE4C04`).

## Host methods the executable must implement, in call order

Only the rebuild needs a host; everything after it is pure once the table exists.

| # | Native site | In | Calls | Method |
| --- | --- | --- | --- | --- |
| 1 | `00424D32` | `00424D00` | the world list walk `[[00E188A8]+19CCh]+370h`, `node+4h`, element `node+8h` at `00424D50` | `world_entity_count` |
| 2 | `00424D53` | `00424D00` | `element+154h` / `+158h` (`00424D5C`), the literal at `00CE3870` on a zero length | `world_entity_name` |
| 3 | `0041D201` | `0041D1E0` | `007AC9D0 BSP_Entity_PathInterfaceForKind`, then `0041D206 [entity+54h] == 2` | `world_entity_is_zone_path` |
| 4 | `0041CD32` | `0041CCD0` | `007AF800(path)(&out, index)` with the count from `0041CD1B` | `world_entity_path_points` |

Steps 3 and 4 sit in routines the scan reaches through `0041D1E0`; step 3 folds the interface
cast and the field test into one answer because which of the two the `+54h` field belongs to was
not read.

## Coverage

| Routine | Coverage |
| --- | --- |
| `004179D0` | complete: `004179D0-00417A39`. `004179FD..004179FF` is three bytes of padding Ghidra did not disassemble, after the `JMP` at `004179FB` |
| `00416DD0` | complete: `00416DD0-00416F20` |
| `00416B50` | complete: `00416B50-00416CC4` |
| `004178F0` | complete: `004178F0-0041793B` |
| `00412120` | complete: `00412120-0041216B` |
| `00422500` | complete: `00422500-0042318F` |
| `00417CA0` | complete: `00417CA0-00417D58` |
| `0085C910` | complete: `0085C910-0085CAC7` |
| `0041D1E0` | partial: the register path `0041D1E0-0041D2D2` is projected. The failure branch `0041D2D5-0041D30F` (`004167C0` then the two frees) and the vector-growth copy that returns early at `0041D2A8` are read but not projected |
| `0041F600` | partial: the slot loop `0041F600-0041F62E` is projected; the tail call `0041F62F 0041E960` is unread |
| `0041CCD0` | evidence only, not owned: `0041CCD0-0041D0B1` read whole, but the address is leased to `orch6_ship_ai_lateral_record`, so no ledger record is claimed. The world-box step `0041A540` (`0041D053`) and the owner query `00923810` (`0041D06D`) are not projected |
| `0041A200` | evidence only, not owned: `0041A200-0041A4D8` read whole, same lease. Projected as a rule without an address-suffixed name |
| `00424D00` | referenced, owned by `cc2_mission_load_hosts`: the scan `00424D00-00424DDF` is projected; the tail `00424DDF-00425487`, which registers the ship tuning block's draft keys as further layers through `004223B0` and `00423C50`, is not |

Divergences in `src/avoid_zone_geometry.cpp`, all noted at the site: the two layer lookups return
`-1` on an empty table where the native reads slot 0 regardless; `0041A200` divides the outgoing
edge by a length it has just stored as zero and the projection leaves the direction at zero
instead; `00419260 BSP_Vector2f_ReciprocalLength` is not called (its reconstruction needs the CRT
access block), so normalisation uses the plain `1/|v|` with the same zero-vector answer; and
`00422500` is given an empty-polygon guard the native does not have.

## Corrections

| Target | Was | Is | Evidence |
| --- | --- | --- | --- |
| `include/bsp/ship_ai_path_search.hpp`, `segment_blocked_00417e90` | "`004179D0`'s body is not read here, so which zone wins when the segment crosses several is open" | the zone that owns the crossing nearest the `from` endpoint wins, and the edge index reported is that crossing's edge | `004179D4`..`004179E6` seeds one running point from `*from` and `00417A0A` passes it to every zone; `00416DD0` only ever moves it toward `toward`, so the last write is the nearest crossing |
| `reports/ship_ai_path_follower.json` uncertainty, "`00417EF0`'s true-means-blocked polarity rests on `00417E90` having the same body shape ... `004179D0` itself was not read" | open | settled: `004179D0` returns `BL`, set only where a zone reported a crossing (`00417A1D`), and both wrappers copy their out values only when `AL` is non-zero (`00417EDC`, `00417F3C`) | `004179D0-00417A39` read whole |
| `docs/SHIP_AI_PATH_SEARCH.md`, "`00422500`'s head ... shows `zone+0h` as the corner-record array and `zone+4h` as its count" | the corner records were opaque | a corner is `24h` bytes with the position at `+0h`/`+4h` and the turn angle the walk consumes at `+1Ch`, written by `0041A200` | `0041CD4D` allocates `24h`; `0041A3F7 FSTP float ptr [EDX+1Ch]` writes the turn |
| `docs/SHIP_AI_PATH_SEARCH.md` line 191, "the hit is reported as an index into that array" | inferred from the caller | confirmed from the producer: the index is the crossing edge's first corner, `-1` wrapped to the last | `00416E30` seeds `-1`, `00416EDC` increments after the test, `00416F11` adds the count |

No existing ledger record was replaced by this packet.

## Follow-up packets

1. **`avoid_zone_layer_registration`**, `00424DDF-00425487` of `00424D00` with `004223B0` and
   `00423C50` (`0A21h` bytes). The tail builds `(key, mask)` pairs out of the ship tuning block
   (`00837DE0`) with masks `10h`, `20h`, `40h` and more, and hands each to `00423C50`. That is
   where the layer keys the scenes author (0, 1, 3, 5, 10, 11, 26, 46, 86) get their meaning, and
   `00423C50` is the likely producer of the per-group segment tree the sector scan clips against
   (`docs/SHIP_AI_SECTOR_SCAN.md`, `blk+0A3Ch`). Nothing here is established: the pairing is read
   from the call shape only.
2. **`avoid_zone_world_clip`**, `0041A540` (`0041D053`, `713h` bytes) and the owner query
   `00923810`/`vtable+5Ch(44h)` that fills `zone+10h`. The only two parts of the zone constructor
   this packet did not project.
3. **`avoid_zone_nearest_point`**, `0041AEA0` and `0041B840`, the distance side of the polygon the
   planner uses to push a goal out of a zone. `0041B840` already has a host contract in
   `include/bsp/ship_ai_path_planner.hpp`; `0041AEA0` is the per-zone body under it and is the
   last unread geometry primitive.
4. **`game_executable_avoid_zones`**, wiring this table into `bsp_game.exe` so
   `segment_blocked_00417e90` and `zone_detour_corners_00422500` stop answering "not blocked".
   It needs a mission that authors zones: `usn_2_java`, the executable's current mission, has none,
   so `usn_1_marshall` or `usn_12_augusta` is the run to make.

## no_ghidra_function

none. Every routine read here has a Ghidra function whose body range matches what was read.

## Correction from docs/AVOID_ZONE_OWNER.md and docs/AVOID_ZONE_QUERY_BINDING.md

The concrete native producer is now available in `avoid_zone_owner.hpp`:
0041CCD0 uses actual scene-point transformation007AF800, strict squared spacing
greater than25, the four-pass world clipping0041A540, and the existing
0041A200 derived-record implementation. The semantic `avoid_zone_from_path_points`
and `avoid_zone_rebuild_corner_data` above remain approximations; the new query
adapter does not call them. Clipping can create short edges, and it retains
the original bounds instead of recomputing bounds from the surviving vertices.
World-bound z inputs cross the adjacent XYZ groups: minimum is world+7130h,
maximum is world+7124h. See the owner report for the complete load mapping.

Corner+20h has a known reader/writer: 00423190 lazily computes the clearance
scale described in `SHIP_AI_LATERAL_RECORD.md`. It is not spare storage.
For compatibility this packet's `AvoidZoneCorner::spare` name remains, but
`avoid_zone_polygon_snapshot` copies native `clearance_scale` into it verbatim.
The native pointer-array storage and the semantic vector are distinct layouts;
do not reinterpret one as the other.

00416F30 and the segment-distance/closest-point dependencies are reconstructed
in `AVOID_ZONE_OFFSET.md` and `AVOID_ZONE_SEGMENT_MATH.md`. Together with this
packet's containment implementation and native reciprocal length they supply
all five boundary-query bindings in `AVOID_ZONE_QUERY_BINDING.md`. Executable
registration and mission/gameplay validation remain separate follow-up work.


## Correction from docs/AVOID_ZONE_DRAFT_LAYERS.md

The recovered 00424D00 tail reduces ship depth settings into ordered float-key/mask pairs; 004223B0 appends those temporary pairs. 00423C50 requires an exact existing group, partitions its polygons, and constructs extruded Dyn collision bodies. It does not register source groups or produce the proposed per-group sector tree. The source inputs are flat ShipGlobals.AvoidZoneDepthsSingle/Multi records. Partitioning, native hull ownership and physics-world execution remain unresolved; the implemented input stages are explicitly partial. See AVOID_ZONE_DRAFT_LAYERS.md and reports/avoid_zone_draft_layers.json.

Integration repaired nine false free-call fallthrough gaps in 00423C50 under the Ghidra write lock. 00424D00 had no remaining call gap at the time of repair; its unreachable alignment gap was retained. Evidence: reports/avoid_zone_draft_flow_repair.json.
