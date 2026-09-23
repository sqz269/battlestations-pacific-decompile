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
