# Ship neighbour avoidance, and the planner's travel layer (packet cc9_ship_neighbour_avoidance)

2026-09-23. Names are hypotheses. Offsets are relative to blk = nav = brain+8h. This packet split:
- the secondary, `kShipPlannerTravelLayerBound`, is measured and landed here;
- the neighbour list and its consumers are scoped here for a fresh thread (section 2), because
  their host surface is larger than this thread's remaining context.

## 1. The planner's travel layer (`kShipPlannerTravelLayerBound`)

009ED3E0 hands nav+30Ch, which 009ECA20 produces (docs/AVOID_ZONE_ESCAPE.md), to 009E3780 at
009ED523, 009ED588, 009ED5FD and 009ED692. The host passed the plan block's own `zone_layer`
(0). With the switch on, it passes `travel_layer_30c`.

The planner uses the layer three ways:
- 00417E40 for the zone containing the goal and the pose (009E3831, 009E38EF);
- 004120D0 for the zone group "for this layer or below" (009E387B);
- the segment tests of the search.

Layer 0 selects no group on USN01, whose groups are keyed 1, 3, 11, 46 and 86, so every plan
ignores the shallows. Layer 11 selects the groups on keys 1, 3 and 11.

### Predictions, written before the pair

USN01 3200/3000. Control `build/win32/ptC` (off) against treatment `build/win32/ptT` (on), same
tree, `BSP_GUNNERY_RNG_STREAMS=1`. Both have `kShipAvoidZoneEscapeBound` on.

1. The planner now sees the shallows. Plans whose straight leg crosses a layer-1/3/11 polygon
   gain corner points, so `path search` points and swaps rise on the ships whose goals lie
   across an atoll. Ships sailing open water keep identical plans.
2. Dunlap and SaltLakeCity start at travel layer 0, so their first plans are identical.
3. The ships still never enter a zone, because the plans now route around them.
4. Ship deaths are unchanged (0 in the control). Positions move, so AA and aircraft rows may
   move.

USN04 has no zone groups and its travel layer is 0 on every ship, so a USN04 pair would compare
identical calls. None is run.

### Results (`local\pt_ctl_usn01.log` against `local\pt_trt_usn01.log`)

| row | control | treatment |
| --- | --- | --- |
| path search corner arms | 0 | 2879 |
| path seeds / swaps / accepts | 104 / 104 / 9285 | 103 / 103 / 9284 |
| path points / plan requests | 9589 / 9593 | 9589 / 9593 |
| total path | 12211.40 m | 12211.41 m |
| deaths / damage | 5 / 2250.0 | 5 / 2250.0 |
| queued hits | 144 | 139 |
| unimplemented calls | 2609295 | 2610129 |

Against the predictions:

1. **The planner sees the shallows: met.** The search's corner arm now engages 2879 times.
   Points and swaps do not rise, though, because no plan's output changes enough to add a point,
   and the ships' tracks are the same to the centimetre.
2. **Dunlap's and SaltLakeCity's first plans: not separately traced.**
3. **No ship enters a zone: met.**
4. **Ship deaths: met.** The only aircraft change is timing: Mav5 dies at 94.85 s instead of
   85.90 s, and Mav4 at 110.95 s instead of 109.90 s. Hits go from 144 to 139, with damage
   identical.

The unimplemented calls rise by 834, from the extra corner-arm records.

**Decision.** `kShipPlannerTravelLayerBound` lands ON. It is the image's input to 009E3780, and
the pair shows the search working with no track moving.

## 2. The neighbour list: scope for the next thread

**What already exists.**

| piece | reconstruction | host today |
| --- | --- | --- |
| walk 009F1856 (list 6 at [[00E188A8]+19CCh], +64h head, +60h count) | `ship_ai_walk_neighbour_candidates_009f1856`, ship_ai_neighbour_candidates.cpp | not called. The host's world summary registers list 6 (18 ships on USN04) |
| its timer B50h/B4Ch | `ship_ai_neighbour_timer_due_009f15cc`. The seeds B50h = -U(0,1) and B4Ch = U(1,2) are already drawn by `seed_brain_draws_009f1160` (they are discarded now) | - |
| admission term | \|dy\| < 15.0 (00CF3F20); dist^2 < (0.5 x (len_self + len_other) + max(CollectDist 450, (maxspeed_self + maxspeed_other) x CollectHitTime 10))^2. The values are shipglobals.lua lines 244-245 | - |
| appender 009F0D20 | `ship_ai_neighbour_admit_009f0d20`, ship_ai_neighbour_admission.cpp. Capacity 80h; submarine and party filters; the lifetime is settings+194h + 1.2 | recorded |
| node constructor 009E52E0 | the L projection `ship_ai_obstacle_owner_initialize_projection_009e52e0` (partial) | - |
| per-tick refresh 009F0EA0 plus boxes 009EAE20 / 009EAFC0 | `ship_ai_neighbour_frame_refresh_009f0ea0` (ship_ai_neighbour_frame.cpp) and ship_ai_neighbour_box.cpp | the host runs the simpler `ship_ai_neighbour_list_refresh_009f0ea0` over an empty list |
| consumers | sector scan 009EB660 (bound, walks `ctl.neighbours`); traffic pass 009EF350 (unread, FUN_); 009DE5B0 section 6 (read, docs/SHIP_AI_ARM_FINAL_STEP.md) | 009EB660 runs over the empty list |

**Host methods the binding needs.**
- The frame host (`ShipAiNeighbourFrameHost`): the self view, class +500h, the settings side
  filter +04h and +1B8h, the pose refresh, the velocity vtable+34h, the owner's gone byte and
  party, the director's +241h, and the node bindings.
- The near-box host: heading, body-axis speed, +1A8h, length, beam and pose, for the observed
  ship.
- The avoid-box host: the model bounds through 0098A8E0, the settings block +180h, the velocity,
  speeds, length, and **00811940, the command yaw rate, which the host records as
  unimplemented**.
- The walk host, with the list-6 head and count, and the admission host.

**Two open services.**
- 00811940 is used by the avoid box and by the follow step.
- 0098A8E0, the observed model's world bounds.

**Order in the controller step 009F50E0**, which the binding must keep:
1. the pre-pass walk, on replan ticks when B50h is due;
2. 009F0EA0 at 009F51E4, ageing and box refresh;
3. the sector refresh at 009F51FA (009EB660);
4. the arms;
5. 009DE5B0 section 6;
6. 009F4D10's traffic pass 009EF350.

**Switches to add:**
- `kShipNeighbourListBound`: the walk, the appender and the frame refresh;
- `kShipNeighbourAvoidanceBound`: the consumers see the list. With it off, they are handed an
  empty list, so the list can land alone.

## 3. The neighbour list and its consumers (packet cc9_ship_neighbour_list)

2026-09-23. Names are hypotheses. Two switches in `src/game_hosts_ship_ai.cpp`:
`kShipNeighbourListBound` (the walk, the appender and the frame refresh) and
`kShipNeighbourAvoidanceBound` (the consumers see the list; off, they see an empty list and a
zero blk+604h).

### What is bound

| piece | binding | source |
| --- | --- | --- |
| timer B50h/B4Ch | `ship_ai_neighbour_timer_due_009f15cc` after the torpedo walk, on the same accumulated delta (009F1598 loads ESP+BCh once for both; 009F1604 skips both walks when neither is due). The seeds are no longer discarded | 009F1360, 009F13F0 |
| walk 009F1856 | `NeighbourWalkBinding` over the units host's world list 6 (head, count, payload, next) | 009F1877..009F1A43 |
| admission | the walk's radius test, then `ship_ai_neighbour_admit_009f0d20` | CollectDist 450, CollectHitTime 10, CollectTimer[2] 2 (+1.2 = 3.2 s lifetime), shipglobals.lua 243-245 |
| node 009E52E0 | a host store (node, box motion, observed unit index) and the partial projection | 009E530A..009E5388 |
| frame 009F0EA0 | `ship_ai_neighbour_frame_refresh_009f0ea0` with both box refreshes, over a 128-slot heap array | 009F0EA0..009F115D |
| 00811940 | `GameUnitsHost::unit_current_yaw_rate_00811940` (already reconstructed, `src/unit_rudder.cpp`) | read |
| 0098A8E0 | `ship_ai_hull_copy_bounds_0098a8e0` (already reconstructed) over the band below | read |
| consumers | sector scan node tests 009D80C0 / 009D8160 (reconstructed); 009DE5B0 section 6 (its walk split out as `ship_ai_arm_final_separation_vector_009de96c`, the turn 009DEBB9); the traffic pass 009EF350 | section 2 |

The settings singleton the walk and the boxes read is filled from a new stored read of the
ShipAvoidance block +190h..+1D8h (`read_ship_ai_settings_offsets_lua`, the same table-driven
getters and loader defaults as 0083B7AD..0083BCB8).

**unit+6B8h** is DummyObjectID: 0095CDBE / 0095CDE9 store -1 in the unit constructor, its writers
00953250 / 00953A80 have no caller in the listing, and no installed script names DummyObjectID. The
host answers -1, so 009F0D5C never falls into the candidate-submarine filter.

### 009EF350, read

`void __fastcall(blk)`, body 009EF350..009EF90C, one caller (009F4D10). With blk+604h = 0 it does
nothing. Otherwise it builds a three-entry party table (the 009F0EA0 filter: side blk+3F0h,
director +241h, settings+04h), clears node+74h/+75h on every node, and only a node whose pass side
+88h is non-zero, whose owner is live and whose party passes reaches 009DCEB0 (unread) and the
bearing window that writes blk+324h, +33Ch and +354h. **+88h is 0 for every node**: 009E5364
stores 0, and its only non-zero writer 009D912F is reached from 009F3E30, a brain vtable slot
(00D21B10) with no identified caller. So the pass is complete for every state this host reaches.
The host counts it (`traffic=`) and records the crossing arm if a non-zero +88h ever appears.

### Substitutions left

- **The observed hull's model** (`kShipNeighbourNullModelBandSubstituted`). No unit of this process
  has a model (006D1E30 = unit+360h), so the node would keep its -1000 bounds and fail the vertical
  overlap at 009EB03C every frame. The band the own hull gets from 009DE2F0's null-model arm (max 50,
  min -10) stands in for the observed hull.
- **The velocity vtable+34h** (00812090): heading direction times 0092D730, the torpedo walk's
  stand-in for the body axis.
- **class+808h BigLandingShip**: read as 0 and recorded (the host does not hold it).
- **The class token**: the unit's own identity; every class query resolves through the unit.
- **Still records under the consumer switch**: the sector scan's node clips 009DD010 and 009DD540
  (unread; they answer "no hit", so a node blocks a straight probe only through 009D8160), 009F0100's
  non-empty body, the clearance's neighbour services (009EFD5B keeps answering 0), and the arm
  tail's setback walk list (009EEB8B).

### Predictions, written before the pairs

Sides: **off** (both switches false), **list** (list on, consumers off), **on** (both true). Same
tree, `BSP_GUNNERY_RNG_STREAMS=1` on every side.

1. **off vs list is flat** in every behaviour row: no consumer sees the list, the list draws
   nothing (the B50h/B4Ch draws were already made and discarded), and the walk has no side
   effect. Only the new `neighbour` rows appear, and the host-method table moves: the old
   `ShipAi::neighbour_list_add_009f0d20` record disappears (one per controller step) and new
   concrete methods appear. The unimplemented-call total falls.
2. **Who holds neighbours**: every ship within 450 m of another (plus the closing term). On USN04
   that is every ship of the carrier group; on USN01 the formation members. An 800-frame USN04 probe
   admitted 2 to 6 per ship on all 18 ships, with lists that persist (a walk every 1 to 2 s re-arms
   a 3.2 s lifetime). The `collapsed` column counts node-steps that end with +68h set, which several
   box arms do, not only the far arm.
3. **on vs list**, consumers:
   - sector node blocks on a minority of ships (the probe: two of 18), from 009D8160 on the
     straight probes;
   - separation turns (section 6) on a few followers whose probe point comes within 1/1.5 of the
     summed hull lengths of a neighbour's near-box centre; small turns (probe max 0.22 rad);
   - traffic passes run every step on every ship holding a list and change nothing;
   - the danger level (blk+0A84h) and the arms that read the sectors move on the ships with node
     blocks; station latches and plan requests of followers move with them;
   - deaths and damage: the ships' tracks change, so hits and the RNG-coupled AA rows can move; no
     prediction of direction. Rows expected flat: the mission load, the settings rows, plane rows
     before the first ship track diverges.
   - unimplemented calls rise (clip records, 009F0100's body, the setback walk) and fall
     (009EF350 becomes concrete).

### Results (2026-09-23)

Logs `local\nb_{off,list,on}_usn04.log` (USN04, 4500 mission frames) and
`local\nb_{off,list,on}_usn01.log` (USN01, 3000), each side its own copy of the same tree's
build, `BSP_GUNNERY_RNG_STREAMS=1`. `local\nb_compare.py` diffs two logs without addresses,
host-method tables, the neighbour rows and the refill counter.

**off vs list: flat on both missions**, as predicted. The only differences are the refill
counter (ignored, non-deterministic) and the retired `ShipAi::neighbour_list_add_009f0d20` record.

**Who holds neighbours.** USN04: all 18 ships, 2 to 8 admissions each, up to 7 at once, lists held
for the whole mission on 15 ships. USN01: 13 of 14 ships (Katori holds none); the convoy's six
ships hold 1 to 4 for the whole mission.

**list vs on**, USN04:

| row | list | on |
| --- | --- | --- |
| sector marks | 0 | 1751 (node blocks on 5 ships: Yorktown 430, Northampton-class03 806, Fletcher-class08 456, -class05 37, -class03 22) |
| separation turns | - | on 12 of 18 ships, max 0.224 rad |
| live rudder/throttle pair changes | 24869 | 29988 |
| total ship path | 48854.11 m | 48245.88 m |
| station requests / arm runs | flat | flat |
| danger (blk+0A84h) | 0.000 every ship | 0.000 every ship |
| plan requests | 44991 | 44991 (seeds 471 to 482) |
| ship deaths | none | none |
| plane deaths | 12 | 13 (units 50 and 39 no longer die; 34, 48 and 42 do) |
| torpedo releases | 4 | 4 |

The ships with no separation turn and no node block (Lexington, Northampton-class01/-02,
Fletcher-class01/-06/-09) keep their live-change counts exactly; every moved ship row belongs
to a ship that turned. The danger level stays 0 because its producer, the clearance 009EF910,
still sees a zero count (its neighbour services are unread). No escape-arm or zone row moves.

**list vs on**, USN01: no node blocks; separation turns on SaltLakeCity (261) and Enterprise (842,
max 0.188 rad); live pair changes 7054 to 8189; plan seeds 103 to 102. The same five Mavs die on
both sides, at different times; the torpedo aircraft drop 2 torpedoes instead of 3 (releases 4 to 3),
following the carrier's changed track. Ship deaths none on either side.

**Unimplemented calls** (sum of the log's UNIMPLEMENTED table):

| mission | off | list | on |
| --- | --- | --- | --- |
| USN04 | 3109477 | 3028477 | 3744378 |
| USN01 | 2610129 | 2568129 | 2740922 |

list retires one record per controller step. on adds the two unread node clips 009DD010 (473165)
and 009DD540 (236443) on USN04 and 009F0100's non-empty body (one per step), and retires the
009EF350 record.

**Landed:** `kShipNeighbourListBound` ON (flat pair). `kShipNeighbourAvoidanceBound` ON: every
moved ship row traces to section 6's turns or to 009D8160 node blocks, both read and
reconstructed. The gaps it leaves are the records listed under "Substitutions left", above all the
two node clips (a node never blocks an arc probe) and 009F0100's body.
