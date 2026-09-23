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

## 4. The node clips, the clearance's neighbour services and 009F0100 (packet cc9_neighbour_clips)

2026-09-23. Names are hypotheses. One switch, `kShipNeighbourClipsBound` in
`src/game_hosts_ship_ai.cpp`, on top of section 3's two. Reconstructions are in
`src/ship_ai_neighbour_clips.cpp` (semantic; new C++ interfaces, not ABI replacements).

### Read

| routine | what it does | inputs and constants |
| --- | --- | --- |
| 009D8A30 (body 009D8A30-009D8B88) | the avoid-box point closest to a point: both local coordinates clamped into [-half, +half]; with +68h set, the centre | node +44h..+60h |
| 009D8860 (009D8860-009D8A2C) | the corner extreme along a direction; with +68h set, the centre | node +44h..+60h |
| 009DD010 (009DD010-009DD530) | clips an arc against the box. False with +68h or +69h set, when the closest point (009D8A30) is outside the circle, or when corners 0, 2, 3 and 0 again all lie strictly inside (corner 1 is never tested; the listing loads corner 0 twice at 009DD2F8 and 009DD358). Otherwise each edge is cut by the circle (004F3BA0) and every crossing's bearing, pi/2 - atan2(dz, dx) wrapped into [0, 2pi) (00CE3830, 00CE3828 as doubles), narrows the end bearing when it lies strictly inside the swept delta, captured once per edge (009DD3DE) | two callers: the sector scan's arc probe (009EBE36) and the clearance sweep (009EFDBD) |
| 009D8210 (009D8210-009D84D2) | one box edge against a ray: false with +68h or +69h set, both ends behind, both ends at or past the range, or both on one side; else the crossing distance replaces the range when 0 < t < range | - |
| 009DD540 (009DD540-009DD9BA) | clips a ray against the box: false with no owner, a gone owner or +68h; four 009D8860 rejections (the box wholly behind, wholly beyond the range, wholly on either side of the ray line; the perpendiculars are (dz, -dx) and (-dz, dx), built with -0.0f at 00D7A208); then the four edges through 009D8210, OR-ed | the sector scan's straight probe (009EB8E2) |
| 009D8010 (009D8010-009D80B4) | the pass-side gate: false with no or a gone owner; +7Ch -= dt; +68h set clears +88h; if this ship leads its group (00778890) and the observed ship is in that group (unit+284h equal), +7Ch = max(+7Ch, 0.5) (00CE3800, 00415550) and +88h is cleared; true when +68h is clear and +7Ch < 0 | node +7Ch starts at 0.0 (009E537B) |
| 009D8C60 (009D8C60-009D8CDA) | side 0 clears +88h and +75h. Sides 1 and 2, with a non-null unit and owner, build a message (009D66B0) for (owner, side) and route it to the unit through 0077C2A0. It writes no node field | - |
| 009F0100 (009F0100 to the exclusive end 009F0ACE) | mode 0 and \|0092D730\| < 1.0: clears +88h/+75h on every gated node. Otherwise, when any node is due, it takes the commanded heading blk+324h's direction; within 5 degrees (00CEDF5C) of the hull heading it measures from the pose, otherwise from the turn centre off the shoulder the turn swings (blk+18Ch or +194h plus blk+3CCh along the perpendicular). Each due node is cleared, or sent a pass side 1 or 2 through 009D8C60, from its along/lateral position (limits blk+3E4h, +200.0 behind (00CE4D70), blk+32Ch + settings+1C8h ahead, the box half beam + settings+1CCh aside, own width unit+9CCh) or, far behind, from the distance to the shoulder against blk+3CCh -/+ settings+1CCh. Every processed node's +7Ch is reset to 0.9 (00CE3860) | runs at 009F4D10's order tail |

**009F0100's body is longer than Ghidra's function.** Ghidra's body ends at 009F0A17 (the RET);
the loop jumps to three tail blocks 009F0A1A..009F0ACE (the inner-circle post, the outer-circle
branch and its post), which lie outside it. The true exclusive end is 009F0ACE (INT3 padding
follows at 009F0ACE).

**The pass side is a message.** 009F0100 is the producer of node+88h, but only through a message:
009D8C60 posts it and the brain's vtable slot 009F3E30 (00D21B10) handles it, finding the node for
the sender (009DA690) and running 009D8CE0, which writes +88h at 009D912F. The message route 0077C2A0
and the handler are unbound, so +88h stays 0 and 009EF350 stays inert. Each post is recorded as
`ShipAiNeighbour::pass_side_message_0077c2a0` and counted (`pass_posts=`).

### The clearance and the danger level

The clearance 009EF910 (`bsp::ship_ai_refresh_turn_clearance_009ef910`, already reconstructed) now
sees the consumers' count and nodes: the owner filters (+14h, +5Eh, party +54h against the three
009EC770 answers), 009DD010 on the hull sweep (the last blocking node wins; the outcome +370h becomes
blocked-moving or blocked-stopped by the node's 0092D730 speed against 2.0), and, when nothing
blocks, 009D8860 twice and 009D8A30 for the clearance +37Ch. The danger level blk+0A84h follows
+37Ch / unit+9CCh through the ramp at 009F41AB, and feeds the throttle ceiling 009EC7C0 and the
turn-assist load (009F4332). The free-bearing query 009DC2E0 is section 7's, not the clearance's; it
stays unread.

### Substitutions left

- The pass-side message (0077C2A0) and its handler 009F3E30 / 009D8CE0: recorded, not delivered.
- 009DCEB0, 009D7AF0 (009EF350's crossing arm): unread, and unreachable while +88h stays 0.
- 009DC2E0, section 7's free-bearing query: unread.
- Precision: the reconstructions round every intermediate to float; the listings keep a few sums
  (the squared distances, the ray crossing) in x87 extended precision before the store.

### Predictions, written before the pairs

Sides: **off** (`kShipNeighbourClipsBound` false, section 3's state) and **on**. Same tree,
`BSP_GUNNERY_RNG_STREAMS=1`.

1. **Sector scan**: the straight probe now also blocks through 009DD540's edge crossings, and the
   arc probe through 009DD010, so sector marks rise on the ships that already had node blocks
   (Yorktown, Northampton-class03, Fletcher-class08 on USN04) and appear on a few more of the
   screen. USN01 had none; a few may appear on Enterprise and SaltLakeCity.
2. **Clearance and danger**: the clearance +37Ch falls below its sentinel on ships whose sweep
   meets a node, so the danger level leaves 0 on some USN04 ships (the column is 0.000 everywhere
   off), and the throttle ceiling drops on them. Blocked outcomes appear (`clearance_hits=`).
3. **009F0100**: posts on most USN04 ships (an 800-frame probe: 13 of 18); no row moves from it,
   because the posts are not delivered and every clear writes a +88h that is already 0.
4. **Moved rows**: live pair changes, total ship path, separation turns (section 6 reads the same
   list but the tracks move), plane rows coupled to the ships' tracks. Station requests and plan
   requests stay flat unless a leader's track moves.
5. **Deaths and damage**: may move with the tracks; no direction predicted. Ship deaths: none on
   either side.
6. **Unimplemented calls** fall toward the list side's totals: the clip records (about 710000 on
   USN04), 009F0100's record (one per step) and the clearance's neighbour records go, and the pass
   posts come in (a few thousand).
7. **Flat**: the mission load, the settings rows, everything before the first node forms.

### Results (2026-09-23)

Logs `local\clip2_{off,on}_usn04.log` and `local\clip2_{off,on}_usn01.log`, each side its own copy
of the same tree's build, `BSP_GUNNERY_RNG_STREAMS=1`. A first pair without the danger counters
(`local\clip_*`) matches the second on every other line.

| row, USN04 | off | on |
| --- | --- | --- |
| sector marks | 1751 | 7332 (node blocks on 8 ships, 5 off; Northampton-class03 3043, Fletcher-class08 2584) |
| clearance node hits (sweeps a node blocked) | 0 | Northampton-class03 283, Fletcher-class08 114, Fletcher-class03 45 |
| danger above 0 (steps; max) | none | 5 ships: Northampton-class03 1959, Fletcher-class08 713, Yorktown 682, Fletcher-class03 534, York-class02 137; all reach 1.000 |
| 009F0100 bodies / pass-side posts | record | posts on 15 of 18 ships; 2478 posts |
| live rudder/throttle pair changes | 29988 | 33582 |
| total ship path | 48245.88 m | 47853.34 m |
| plan seeds / accepts | 482 / 43562 | 525 / 43433 |
| station rows, zone rows | - | flat |
| ship deaths | none | none |
| plane deaths | 12 | 11 (units 35 and 42 no longer die; 36 does) |

USN01: sector marks 0 to 27 (all Enterprise), no clearance hit, no danger, 2 ships post (116);
every other row is flat, deaths included.

Against the predictions: sector marks rose on the five ships that already had node blocks and appeared on
three more (predicted); the danger level left 0 on five USN04 ships and stayed 0 on USN01 (predicted:
"some USN04 ships"); 009F0100 posted on 15 of 18 (predicted: most) and moved no row; station rows stayed
flat (predicted), plan seeds moved with the tracks; the kill set shifted on USN04 and not on USN01.

**Unimplemented calls** (sum of the UNIMPLEMENTED table):

| mission | off | on |
| --- | --- | --- |
| USN04 | 3925072 | 3135486 |
| USN01 | 2742985 | 2525598 |

The off side is above section 3's "on" (3744378) because main gained other records in between; the
fall is the two clip records, 009F0100's record and the clearance's neighbour records. The pass-side
posts come in (2478 and 116). The only neighbour-related record left in the table is the clearance's
`path_fade_00778890` (already there, not this packet's).

**Landed:** `kShipNeighbourClipsBound` ON. Every moved ship row traces to the bound reads: node
blocks through 009DD540 / 009DD010 on ships that already held neighbours, and the clearance's
node-driven danger on the five ships it reports.

## 5. The pass-side message and the traffic pass (packet cc9_pass_side_message)

2026-09-23. Names are hypotheses. Switch `kShipPassSideMessageBound` in
`src/game_hosts_ship_ai.cpp`; the traffic pass runs under `kShipNeighbourAvoidanceBound`.
Reconstructions in `src/ship_ai_neighbour_clips.cpp`.

### The route, read

| step | site | what it does |
| --- | --- | --- |
| post | 009D8C60 (009D8CAA..009D8CC4) | builds a kind-8Fh message through 009D66B0 (body 009D66B0-009D66EE): base constructor 0075B430(8Fh), vtable 00D034DC, +1Ch = the observed ship's object id (owner+174h), +20h = the side. Routes it with 0077C2A0(ECX = this ship's unit, msg, 2, 0) |
| route | 0077C2A0 | ECX is the sending entity, not the session (0077C2AB MOV EDI,ECX; 0077C2C6 calls its vtable+5Ch). In a local session the flag word is forced to 1 and the only destination is 0076E520's loopback queue; the addressee is the sender itself (msg+18h = sender+174h, docs/SESSION_MESSAGE_DISPATCH.md) |
| drain | 00778450 -> 0076C600 -> 00780670 | the session pump at fixed-step row 9 (docs/FIXED_STEP_FANOUT.md). The AI steps inside the world entity tick 00904BF0, which 004C40A0 calls after the fixed-step driver (docs/IN_MISSION_SUBSYSTEM_TICK.md), so a message posted during an AI step is delivered at the next step's pump, before any ship steps |
| dispatch | 00780670 -> 00780120 -> 0077FE80 | the message answers IsA only for 8Fh and 46h (00762430), so none of the 47h/61h/48h/4Bh..59h arms take it; 0077FE80's gate is 004499C0 (always true) and 8Fh - 53h = 3Ch is past its 26h-entry table, so it goes to the entity's vtable[164h], 00821E80 |
| receive | 00821E80's 8Fh arm, 00822294 (table 00822400 byte 0Ah, target 008223BC) | resolves +1Ch through 00521E30 and 00815010 (a kind-6 unit), then calls this ship's ai (unit+740h) vtable[28h] (00D21B10 = 009F3E30) with (other, side) |
| handle | 009F3E30 (body 009F3E30-009F3E77, no caller: a vtable slot) | 009DA690 finds this ship's node for the other (blk+604h/+608h, first +14h match); none, or no ai on the other ship, ends it. Then the other ship's node for this one (may be null) and 009D8CE0(node)(side, other node) |
| decide | 009D8CE0 (body 009D8CE0-009D9141) | node+88h = side and +75h = 1, unless both ships have assigned each other sides and the tracks cross: then +8Ch is 1 or 2 by the ranges to the crossing, and 2 takes the other ship's side |

The consumers of +88h: 009F0100 (its lateral threshold), the sector scan's 009D84E0, and the
traffic pass 009EF350, which now runs whole: 009DCEB0 (the pass bearing, body 009DCEB0-009DD00C)
and 009D7AF0 (the corner to steer at by quadrant and side, body 009D7AF0-009D8000, third argument
unread) fill +6Ch/+70h/+74h/+75h; the pass keeps the nearest side-1 and side-2 corners, narrows the
bearing window [lo, hi] by the next ones, clamps blk+324h into it, stores sqrt(nearest squared
distance) at blk+33Ch and raises blk+354h to 3.0.

**blk+33Ch.** Its reader is the drive's rudder clamp gate at 009F4514, open only when the field is
negative. 009F4D10 stores -1.0f there at 009F4D27 before calling 009EF350; this host drops that
store (the publish result's `blk_33c`), so the field is 0 or the traffic pass's distance and the
gate stays shut on both sides of the switch. Binding the -1.0f store is a separate change.

### Substitutions left

- An inactive sender drops the message (00780670's "entity missing" arm holds it ten ticks first).
- The drain is modelled for the pass-side kind only, at the head of the next controller step
  (this host runs one fixed step per 0.05 s frame).
- 009DC2E0 (section 7) is not read in this packet.

### Predictions, written before the pairs

Sides: **off** (`kShipPassSideMessageBound` false, section 4's state) and **on**.

1. **Delivered**: nearly every post (USN04 posts 2478 off; on, posts change with the tracks), less
   those whose sender lost its node for the other ship before the next step. An 800-frame probe
   delivered 370 of 371.
2. **Pass sides**: ships in the screen take sides for the ships they post about; negotiation
   (+8Ch non-zero) is rare, a few ships.
3. **Traffic pass**: writes blk+324h/+33Ch/+354h on most ships that take a side (probe: 8 of 18),
   with turns up to ~1.7 rad; the tracks of those ships move, so separation turns, sector marks,
   clearance hits and danger move too.
4. **Station rows and plan requests**: followers' heading targets change, so plan requests
   and path rows move; station rows (requests, arm runs) may stay flat.
5. **Deaths and damage**: may move with the tracks; no ship death expected on either side.
6. **Unimplemented calls**: the pass-side record goes (2478 on USN04, 116 on USN01); nothing new
   comes in, so the totals fall by about those amounts (to about 3.13M and 2.53M).
7. **USN01**: only SaltLakeCity and Ralph posted; small or no moves.
8. **Flat**: the mission load, the settings rows, everything before the first node forms.

### Results (2026-09-23)

Logs `local\ps_{off,on}_usn04.log` and `local\ps_{off,on}_usn01.log`, each side its own copy of the
same tree's build (main ec4e7f074 plus this packet), `BSP_GUNNERY_RNG_STREAMS=1`.

| row, USN04 | off | on |
| --- | --- | --- |
| pass-side posts / delivered | 2476 / 0 (recorded) | 2636 / 2636 |
| node+88h changes / negotiated (+8Ch) | - | 182 / 124, on 14 ships |
| traffic-pass writes (blk+324h/+33Ch/+354h) | 0 | 23089, on 13 ships; largest single turn 3.14 rad |
| live rudder/throttle pair changes | 33837 | 30575 |
| total ship path | 48039.62 m | 47058.50 m |
| sector marks | 7377 | 8081 |
| plan requests / seeds | 45327 / 541 | 45327 / 532 |
| station rows | - | requests and arm runs flat; the throttle band moves (Northampton-class01 -0.8425 to -0.7946) |
| torpedo drops (aircraft) | 2 | 0 |
| ship deaths | none | none |
| plane deaths | 16 | 20 (the set shifts; see the logs' `entity dead` lines) |

USN01: 116 posts, all delivered (SaltLakeCity 40, Ralph 76); 5 side changes; 1084 traffic writes
that never move blk+324h (the window never binds); every summary row flat, deaths included.

Against the predictions: every post was delivered (predicted "nearly every"); negotiation was more
common than predicted (124 deliveries on 10 ships, not "a few"); the traffic pass wrote on 13 of 18
ships (predicted most that take a side) and its turns reach pi, larger than the probe's 1.7 rad;
plan requests stayed flat and station requests and arm runs stayed flat (predicted); the kill set
moved on USN04 and not on USN01; no ship died.

**The pi turns.** With only side-1 nodes the window's lower bound is the heading - 3.14 (00CF0AA8)
and its upper bound the nearest side-1 corner's bearing - 5 degrees; 009D7AF0 can pick a corner
abeam or astern while 009DCEB0 only requires part of the box ahead, so the clamp can send blk+324h
round to the far side. That follows the listing; it has not been checked against a trace of the
image, and it is the first thing to look at if the USN04 moves need explaining.

**Unimplemented calls** (sum of the UNIMPLEMENTED table):

| mission | off | on |
| --- | --- | --- |
| USN04 | 3094607 | 3092810 |
| USN01 | 2525608 | 2525492 |

The fall is the pass-side record (2476 and 116); USN04's on side posts more (2636), and those are
now concrete. main moved the off side from section 4's 3135486.

**Landed:** `kShipPassSideMessageBound` ON; the traffic pass runs under
`kShipNeighbourAvoidanceBound` (ON). Section 7 (009DC2E0) was not read in this packet.
