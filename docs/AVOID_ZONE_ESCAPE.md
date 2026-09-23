# The avoid-zone escape: 009ECA20 and 009DE5B0 section 4 (packet cc9_avoid_zone_escape)

2026-09-23. Names are hypotheses. Offsets are relative to nav = blk = brain+8h. The producer's
reconstruction predates this packet: `ship_ai_select_navigation_layer_009eca20`
(`src/ship_ai_layer_selection.cpp`, docs/SHIP_AI_LAYER_SELECTION.md). It is complete through
009ECA20..009ED3D9 and passed an original-byte fixture of 3072 cases with zero mismatches. This
packet binds it.

## 1. What 009ECA20 does

It is chain slot 12 of the controller step 009F50E0 (009F51C6), and runs every tick. Three timers
decide how often each part runs:

| timer | reseed | settings (shipglobals.lua, this installation) | what runs |
| --- | --- | --- | --- |
| nav+148h | U(+1FCh, +200h) = U(2.5, 3), scaled by speed / reference through Interp(0.1, 2, 0.6, 1) | `LandAvoidance.CheckShipPosZoneTime = { 2.5, 3 }`, line 287 | probe ahead or astern of the hull. Interp(0.1, 0, 0.5, 1.5) forward and (0.1, 0, 0.4, 1) reverse. Then walk the position layer +164h down through containing groups or up through clear ones |
| nav+314h | U(+1F4h, +1F8h) = U(2.5, 3) | `CheckMovePosZoneTime = { 2.5, 3 }`, line 286 | for navigate modes 2/3, the goal's layer +310h |
| nav+170h | U(+204h, +208h) = U(3, 4) | `CheckTravelZoneTime = { 3, 4 }`, line 288 | the travel pass (below) |

The comments on those lines say the ship checks, every so many seconds, which avoid zone it is
in and which one its path's target is in, for path finding. When the target lies in shallower
water and the ship is near its current zone's edge, it switches to the shallower zone.

- **Layers are water depths.** An avoid zone belongs to a layer key; USN01's retained zones are
  named `AvoidZoneG all 1`, `all 3`, `all 11`, `all 46` and `all 86`. A ship's own layer is
  class+560h through vtable+214h (006DFD80). It is 11 for every surface class this installation
  loads, except TBoat and the two LandingShips at 5 (docs/SHIP_AI_LAYER_SELECTION.md).
- **The travel pass writes the escape.** It walks up from +164h against the hull pose. A
  containing group sets:
  - **+160h** = 1;
  - **+14Ch** = the length of the 00417B10 offset from the hull to the zone's exit point, 1.0
    when that is at most 1.0, capped at +3C8h;
  - **+150h/+154h** = that direction, normalized.

  It then sets **+30Ch**, the travel layer the planner 009E3780 uses at 009ED523/588/5FD/692.
  A pass that finds nothing clears +160h (009ED060).
- **Stream-1 draws.** The timers' three reseeds are stream-1 draws (00BD2F10).

## 2. Section 4, the escape blend (009DE6DB..009DE8ED)

This is docs/SHIP_AI_ARM_FINAL_STEP.md section 4, reconstructed as
`ship_ai_arm_final_escape_turn_009de6db`. With +160h set:
- the turn toward +150h is at most 30 degrees (00CEC730) times three ramps multiplied together:
  - the heading error, Interp(pi/4, 1, 80 deg, 0) (00CEB5A8, 00CF8858);
  - the remaining path over max(100, length), Interp(3, 0, 5, 1) (00CE3854, 00CE3850);
  - +14Ch, Interp(0, 0.15, +3C8h, 1) (00CE7818);
- blk+354h is raised to 3.0 (00CE3854) and unit+102Ch to 1.5 x the product;
- the reverse flag of section 3 (009DE67D..6D7) negates the turn.

It runs before section 5's torpedo override.

## 3. The host binding (`kShipAvoidZoneEscapeBound`)

- **009ECA20** runs through `LayerSelectionBinding` over `GameAvoidZoneRuntime::table()`. That
  is the scene's AvoidZoneG groups as the host already builds them for the planner and the
  searchers. The class+560h array comes from the existing `read_ship_navigation_input` and the
  six timings from `read_ship_layer_timing_input`.
- **Remaining substitutions:**
  - +148h starts at the constructor's recorded low-end draw (0.0); 009E4669's stream-0 draw is
    not modelled.
  - `call_unit_v10c` answers 0, which is the LandingShip body 0042BB40.
  - The stream-1 draws key on (unit | 800000h) under `BSP_GUNNERY_RNG_STREAMS=1`.
- **Sections 3 and 4** run inside the existing 009DE5B0 hook, before section 5. blk+354h is
  raised in both host projections: 009E04E0's hold and the block's clamp.
- **Not bound here**, behind its own switch `kShipPlannerTravelLayerBound` (OFF): feeding +30Ch
  to the planner. It would re-route every path and needs its own pair.

## 4. Which ships can be affected, from the scene data

- **USN04** (`usn_19_coralus.scn`) has no AvoidZoneG or Landscape entity. The host's geometry
  summary reads `groups=1 zones=0`. 009ECA20 runs and finds nothing, so blk+160h stays 0.
- **USN01** (`usn_1_marshall.scn`) has 21 AvoidZoneG and 51 Landscape entities: the Marshall
  atolls. The host builds `groups=6 zones=21`. Zone positions are not in the log. The ships that
  approach land are the likely ones: the escorts and convoy heading for the objectives, and
  Katori.

## 5. Predictions, written before the pairs

Control `build/win32/azC` (off) against treatment `build/win32/azT` (on), same tree,
`BSP_GUNNERY_RNG_STREAMS=1`.

**USN04 4700/4500:**
1. Every ship reads `inside=0` and `escape_turns=0`, and the travel layer sits at the ship's own
   layer (11).
2. All rows are flat: deaths, damage by entity, plan requests, the Yorktown's torpedo rows. The
   only change is the 009ECA20 call becoming concrete, a drop of about 81000 unimplemented calls.

**USN01 3200/3000:**
3. At least one ship enters a zone on layer 11 or shallower and turns. The turns are a few
   degrees per tick, capped at 30 degrees times the ramps. blk+354h is raised to 3.0 on those
   ticks, which delays any torpedo override (none are expected on USN01).
4. Plan requests move only through the moved headings. The planner's layer is unchanged.
5. Ship deaths are unchanged (there are none in the control). Aircraft rows may move through the
   ships' AA.

## 6. Results (2026-09-23)

`local\az_{ctl,trt}_usn04.log` and `local\az_{ctl,trt}_usn01.log`. All four runs used
`BSP_GUNNERY_RNG_STREAMS=1`.

| mission | ships inside a zone | escape turns | travel layer +30Ch | other rows | unimplemented calls |
| --- | --- | --- | --- | --- | --- |
| USN04 4500 | 0 of 18 | 0 | 0 on every ship (empty table) | identical | 3377720 -> 3296720 |
| USN01 3000 | 0 of 14 | 0 | 11; Dunlap and SaltLakeCity start at 0 | identical | 2651295 -> 2609295 |

Every non-environment line matches across each pair: deaths, damage by entity, plan requests and
the torpedo rows. Against the predictions:

1. **USN04 inert: met.** The layer is 0, not 11, because the table has no group above key 0.
2. **USN04 flat: met.** 009ECA20 is now concrete on 4500 ticks x 18 ships.
3. **USN01 ship enters a zone: wrong.** No USN01 ship comes within an AvoidZoneG polygon on
   its layer in 150 s. The escorts and the convoy sail in open water between the atolls.
4. **USN01 plan requests: met, trivially.** Nothing moved.
5. **USN01 ship deaths: met.**

**Decision.** `kShipAvoidZoneEscapeBound` lands ON. It is inert on both reference missions. It
binds the producer faithfully, turns 009ECA20 concrete, and is ready for a mission that sails
near land. `kShipPlannerTravelLayerBound` stays OFF. With it on, USN01's planner would get layer
11 instead of the plan's 0 and could route around shallows. That needs its own pair.

## 7. Secondary: 009F0D20 and section 6 (read only)

- **The appender and the walk are both reconstructed.** 009F0D20 (body 009F0D20-009F0E83) is
  `BSP_ShipAi_NeighbourListAdd`. The candidate walk 009F1856 is
  `ship_ai_walk_neighbour_candidates_009f1856` (docs/SHIP_AI_NEIGHBOUR_CANDIDATES.md). It walks
  the world's list 6 at `[[00E188A8]+19CCh]` (+60h head, +64h count). The host's world summary
  already registers 18 units in list 6 on USN04, and the 90h node constructor is reconstructed
  (docs/NATIVE_SHIP_AI_OBSTACLE_NODE.md).
- **Binding it changes far more than section 6.** A live neighbour list feeds 009EB660's sector
  scan, the danger level, the escape arm and the traffic pass 009EF350, as well as section 6.
  That is ship-to-ship collision avoidance for every ship, a separate packet with its own
  pairs. This packet leaves blk+604h at 0.
