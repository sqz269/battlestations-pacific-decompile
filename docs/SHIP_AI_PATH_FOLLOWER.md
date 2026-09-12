# How a ship walks its path plan, and what advances the cursor

Addresses: 009E3C00 009D5930 009D6550 004F3970 00417EF0 009D9E50 009E421F 00811D80 0082E850 004218E0 00414C60

`009E3780` seeds the plan, `009EC680` searches it, and `009E3C00` walks it. This packet reads
`009E3C00` whole, including the tail past `009E3D81` that three earlier packets left unprojected,
and answers the question `ship_ai_path_search` could not: **`plan+34h` is a step cursor, and the
only thing in the image that advances it is `009E421F`, inside `009E3C00` itself.**

The walk is short. Take `plan+34h` links from the head; that node is where the ship came from. The
next node along is the target. The node after that decides whether the ship is at a corner. If it
is, the follower does not aim at the corner: it builds a circle of look-ahead radius centred one
look-ahead clear of the corner along the corner's bisector, and aims at one of the two tangent
points from the ship to that circle, the target node's signed byte `+14h` choosing which. Then it
publishes a point a computed distance along that aim direction, and if the ship is already inside
the look-ahead of the target (or has been carried past it), it asks the avoid-zone manager whether
it can skip the target; a clear answer moves the cursor on by one.

Names here are hypotheses, not recovered symbols. Reconstruction:
`include/bsp/ship_ai_path_follower.hpp`, `src/ship_ai_path_follower.cpp`. Evidence rows:
`reports/ship_ai_path_follower.json`. The node layout, the plan block and the shared direction rule
are `include/bsp/ship_ai_path_planner.hpp`; the 22h-byte record is `ShipAiPathPointRecord` in
`include/bsp/ship_ai_goal_vector.hpp`; the circle is `ShipAiCircleTangentCircle` in
`include/bsp/ship_ai_approach_update.hpp`. Nothing here redefines them.

## `009E3C00`

`void __thiscall(plan)(float* record)`, `RET 4` at `009E3C65`, `009E3D81` and `009E4328`, body
`009E3C00-009E432A`. Two call sites, both passing a plan pointer and a stack record:
`009EE5F4` inside `009ED6B0 BSP_ShipAi_ControlsStep` on `blk+2F4h` (the plan in use), and
`009E4813` inside `009E46F0` on the same slot. The second reads only the record's `+08h`/`+0Ch`
and takes an `atan2` of it, so both sites agree on the contract.

### The prologue, which runs on every exit

`009E3C03-009E3C37` writes five plan fields and clears two record fields before any test:
`plan+60h`/`+64h` and `plan+50h`/`+54h` take the query pose, `plan+4Ch` takes `1.0f` (`00D7A24C`),
`record+1Ch` and `record+18h` take zero. `009E3C3A` then tests `plan+20h`; an empty plan stores
`plan+34h = 0` at `009E3C3F`, answers the pose as the point, clears both record bytes and returns.

### The cursor walk

`009E3C6C` loads `plan+34h` and `009E3C6F`/`009E3D05` run the loop that many times from
`plan+20h`, deciding each undecided node's `+4h` on the way (`009E3C80-009E3CE1` is the direction
rule inlined, `009E3CE1-009E3D03` the step). The node the loop lands on is `cursor`, the node the
ship came from. `009E3D13` calls `009D5930` on it and `009E3D18` steps to `target`; when `target`
exists, `009E3D3C` calls `009D5930` on it and `009E3D41` steps to `after`.

`009E3C80` re-enters the loop body without testing the pointer, so a cursor longer than the path
dereferences null there, and `009E3D11` would hand a null cursor to `009D5930`. The reconstruction
stops instead; the deviation is in the header.

| walk result | what happens |
| --- | --- |
| `after` non-null | `record+20h = 0` (`009E3D88`), a corner is possible |
| `after` null, `target` non-null | `record+20h = 1` (`009E3D65`), `after` is set to `target` (`009E3D84`) |
| `after` null, `target` null | `009E3D6B-009E3D81`: the point is the pose, `record+1Ch = 0`, return |

### The target point and its lateral offset

`009E3D8C` reads `target+10h`, the lateral record. With none, the point is the node's own
`+18h`/`+1Ch`. With one, the point starts at the record's `+0h`/`+4h` and `009E3DCD` calls
`00811D80` for a scalar, which `009E3DE2`-`009E3E03` multiplies into the record's `+10h`/`+14h`
unit and adds. `00811D80`'s `this` is `owner + 0A98h + 54h * [owner+0B40h]` (`009E3DB5-009E3DC1`),
the unit's own **published** order slot, which is `unit_ai_order_published_offset` in
`include/bsp/ship_ai_navigation.hpp`. Its contract is settled in
`include/bsp/ship_ai_throttle_ring.hpp`: `30.0f` unless a live sub-record sits within 20 units of
the query position.

### The two scales

| value | expression | site |
| --- | --- | --- |
| look-ahead radius | `0082E850([[plan+3Ch]+538h]) * 1.5` (`00CE3D78`, a double) | `009E3EAE`, `009E3EB3` |
| step floor | `[[plan+3Ch]+9C8h] * 0.75` (`00CEC9D8`, a double) | `009E3EC0`, `009E3EC6` |

`0082E850 BSP_ShipClass_GetTurnRadius` takes the owner's ship class; `owner+9C8h` is the unit
radius `009E3ADB` reads in `009E3780`. The look-ahead radius is both the corner circle's radius and
the distance under which the follower tries to skip the target node.

### The corner

`009E3F54` and `009E3F69-009E3F76` gate it: the target must carry at least one link and the
following leg must be at least `1.0f` (`00D7A24C`) long. Otherwise `009E4197` sets `record+20h = 1`
and the steering vector is simply `point - pose`.

Inside the gate, `009E3F95-009E3FF8` builds the bisector as the incoming leg divided by minus its
own length plus the outgoing leg divided by its own length, and `009E3FFC` takes its length through
`00414C60`. Below `1e-6f` (`00D7A288`) the corner is straight through and there is no bisector, so
`009E4013-009E4063` substitutes a perpendicular of the following leg, the sign of `target+14h`
choosing which, with the following leg's length as the divisor. `009E406B`/`009E4075` then
normalises.

`009E4087-009E40ED` is the overshoot test, and it is not a plain sign test. With
`n = (-out_leg.z, out_leg.x)`, the unnormalised normal of the following leg, it forms

```
offset_from_node = n . (pose - target.point)        009E40A7..009E40C3, stored float32
bisector_side    = n . bisector_unit                009E40C7..009E40D9, stored float32
overshot         = offset_from_node * bisector_side < -1.0    009E40DD, the double at 00D7A250
```

Both products are exact in the x87 registers, so the comparison is exact; the bound is a length in
the following leg's units, not a sign. It fires when the ship sits on the opposite side of the
following leg from the side the corner offset points to.

`009E40F2-009E4139` then places the circle centre at `point + bisector_unit * look_ahead`, stores
it in `plan+60h`/`+64h` and the radius in `plan+4Ch`, and `009E4162` calls `009D6550` for the two
tangent points. `009E4167` takes the second when `target+14h` is negative and the first otherwise,
and `009E4185` makes the steering vector `aim - pose`.

`009E413D-009E415E` pre-seed the two out buffers with the circle centre and the offset **vector**.
When `009D6550` returns false it writes neither, so a negative side byte then steers at the offset
vector as if it were a point. The reconstruction reproduces that rather than repairing it.

### The advance, and everything that writes `plan+34h`

`009E41AB-009E4221`:

- when the target has no links at all (the dangling goal node), `009E41CF` clamps the step floor
  down to the distance to the point and nothing else happens;
- otherwise, when the ship is inside the look-ahead of the target (`009E41F3`) or `overshot`
  (`009E41F9`), `009E4200` takes the avoid-zone singleton and `009E4216` asks `00417EF0` whether
  the straight run from the pose to `after`'s point meets a zone. A clear answer (`AL == 0`) runs
  `009E421F ADD dword [ESI+34h],1`.

An image-wide scan settles that this is the only advance. Scanning `009D0000-009F7000` for every
store form to `[reg+34h]` and the whole `.text` for `ADD [reg+34h],1`, and confirming the plan
pointer never leaves that window (the plan vtable `00D2152C` has exactly three referencing
functions, `009D9CC0`, `009DBDB0` and `009DBEF0`, and the navigator's own slots `nav+2F4h`/`+2F8h`
are written only at `009ED5BD`/`009ED5C3`), gives nine writers:

| site | function | value |
| --- | --- | --- |
| `009D9D1C` | `009D9CC0 BSP_ShipAiPathPlan_Init` | 0 |
| `009D9D63` | `009D9D40 BSP_ShipAiPathPlan_Reset` | 0 |
| `009DBDD9` | `009DBDB0` (plan vtable `00D2152C`) | 0 |
| `009DBF1E` | `009DBEF0 CG_scalar_deleting_dtor_009DBEF0` | 0 |
| `009DA50C` | `009DA4E0 BSP_ShipAi_ClearPathPlan`, `nav+258h` = slot A + 34h | 0 |
| `009DA557` | `009DA4E0 BSP_ShipAi_ClearPathPlan`, `nav+2C0h` = slot B + 34h | 0 |
| `009EC721` | `009EC680 BSP_ShipAiPathSearch_Tick`, the 3-to-4 transition | 0 |
| `009E3C3F` | `009E3C00`, plan with no head | 0 |
| `009ED65F` | `009ED3E0 BSP_ShipAi_RefreshPathPlan`, the inlined reset | 0 |
| `009E421F` | `009E3C00`, the shortcut | `+1` |

Eight of nine store zero. The field is a cursor.

### The record, and the point

`009E4223-009E4246` publishes the side code and the lateral handle, but **only when a corner was
found** (`record+20h == 0`):

```
record+1Ch = target+14h > 0 ? 2 : (target+14h != 0 ? 1 : 0)     009E4229..009E4240, signed byte
record+18h = target+10h                                          009E4243..009E4246
```

`009E4249-009E4320` finishes: the steering vector's length, floored at `0.1f` (`00D7A2F0`) when it
is below `0.1` (`00D7A3A0`, a double); the step `max(step_floor, distance_to_point)` (`009E42BF`);
and the published point `pose + steer * (step / steer_len)`, stored into `plan+50h`/`+54h` and the
record's `+08h`/`+0Ch`.

The min at `009E41DD` followed by the max at `009E42BF` is redundant on the goal-node branch: the
two together always give exactly `distance_to_point`, so on the last leg the published point sits
exactly on the goal. The min is in the image and the reconstruction keeps it.

### The record's producer-side layout

`ShipAiPathPointRecord` in `include/bsp/ship_ai_goal_vector.hpp` was read from the consumer. This is
the producer.

| field | written at | meaning |
| --- | --- | --- |
| `+00h`/`+04h` | read only | the query pose |
| `+08h`/`+0Ch` | `009E3C42`, `009E3D6B`, `009E4320`/`009E431D` | the published point |
| `+10h`/`+14h` | `009E3C51`, `009E3F90`/`009E3FA9` | the target node's point; **not written** on the `009E3D6B` exit |
| `+18h` | `009E3C37`, `009E4246` | the target node's **lateral record** handle (`node+10h`), zero when there is no corner |
| `+1Ch` | `009E3C34`, `009E3D77`, `009E4240` | the side code 0/1/2 from the target's signed byte `+14h` |
| `+20h` | `009E3C54`, `009E3D65`, `009E3D88`, `009E3F88`, `009E419B` | 1 when there is no corner past the target |
| `+21h` | `009E3C4E`, `009E3C68` | 1 when the plan has a head node |

## `009D5930`, `009D6550` and `004F3970`

`009D5930 BSP_ShipAiPathNode_DecideDirection`, `void __thiscall(node)`, `RET 0`, body
`009D5930-009D5987`, complete. It is the out-of-line form of the direction rule
`include/bsp/ship_ai_path_planner.hpp` projects from `009D9E72-009D9ED4`, instruction for
instruction, including the `FCOMIP` at `009D5970` whose carry means the plus route is strictly
cheaper. Five callers; `009E3C00` uses it twice and inlines it a third time.

`009D6550 BSP_Circle2f_TangentPointPair`, `bool __thiscall(circle)(const float2* point,
float2* first, float2* second)`, `RET 0Ch`, body `009D6550-009D65D0`, complete. It calls `004F3970`
with the same `this` into two temporaries and returns `base + half_chord` and `base - half_chord`.
False writes neither caller buffer.

`004F3970`, `bool __thiscall(circle)(const float2* point, float2* base, float2* half_chord)`,
`RET 0Ch`, body `004F3970-004F3B98`, complete. The circle is `{x at +0h, z at +4h, radius at +8h}`.
With `u` the unit from the centre to the point and `len` their distance:

| case | test | `base` | `half_chord` |
| --- | --- | --- | --- |
| point at the centre | `len < 1e-6` (`004F39F4`) | false, no writes | |
| point on the circle | `abs(len - r) < 1e-6` (`004F3A3B`) | false, no writes | |
| point outside | `len > r` (`004F3A51`) | `centre + u * (r*r/len)` | `perp(u) * (r * sqrt(len*len - r*r) / len)` |
| point inside | otherwise | the point itself | `perp(u) * sqrt(r*r - len*len)` |

So `009D6550`'s pair is the two tangent points for an external query and the two ends of the chord
through the point perpendicular to the centre direction for an internal one. This settles the
contract `docs/SHIP_AI_APPROACH_UPDATE.md` modelled as an unread host call for `009D68B0`, and its
reading of `009D68B0`'s `side` selecting between the two is consistent with `009E3C00`'s.

## `00417EF0`, the shortcut clearance test

`bool __thiscall(manager)(layer, const float2* from, const float2* to, float2* hit)`, `RET 10h`,
body `00417EF0-00417F59`. It asks `004120D0` for the layer's zone group and forwards five arguments
to `004179D0`; on true it copies a two-float hit point into `hit`, pre-seeded from `to`, and on
false leaves `hit` alone. `00417E90 segment_blocked_00417e90` has the same body shape, the same two
callees and the same polarity, with a one-dword out instead of a float pair. `004179D0`, the body
that actually decides, was not read here: it belongs to `avoid_zone_geometry`. The polarity is from
the two siblings and their call sites, not from the deciding body.

## `009D9E50`'s walk, completed

`BSP_ShipAiPathPlan_RemainingLength` was already projected complete by `ship_ai_path_planner`; what
was open was what its loop bound means. `009D9E67` reads `plan+34h`, `009D9E6C` refuses a negative
value, and `009D9E78-009D9EFA` runs `cursor + 1` steps, so it lands on the node at index
`cursor + 1` — the same node `009E3C00` calls the target, which is why the answer is that node's
`+3Ch` plus the straight line from the pose to it. `009E3C00`'s own loop runs `cursor` steps and
stops one node earlier, at the node the ship came from. The two walks are consistent, not
contradictory, once `plan+34h` is read as a cursor. No new reconstruction; the ledger evidence is
appended.

## A two-node plan, which is what the open sea produces

`009E3780` seeds `head = node(ship pose)` and `goal_node = node(goal)`, and `009D9230` links
`head->+20h = goal_node` with `head->+34h` the segment length, leaving `goal_node` with no links.
With `plan+34h` at 0 the follower does this, every tick:

1. the cursor loop does not run; `cursor = head`;
2. `009D5930` on `head` sees no `+24h` link and stores `-1`; the step takes `+20h`, so
   `target = goal_node`;
3. `009D5930` on `goal_node` sees no `+20h` link and stores `(+24h != 0)` = 0; the step yields
   null, so `after` is null, `record+20h = 1` and `after` is set to `target`;
4. the outgoing leg is zero-length, so the corner gate fails at `009E3F54` and the steering vector
   is `goal - pose`;
5. the target has no links, so `009E41CF` clamps the step floor to the distance to the goal, and
   the max at `009E42BF` brings it back to exactly that. The published point **is the goal**;
6. the advance branch is never reached, so `plan+34h` stays 0 for the life of the plan;
7. `record+20h` is 1, so `009E4223` skips the side code and the lateral handle: `record+18h` stays
   0 and `record+1Ch` stays 0.

Arrival is not the follower's business. `009DA590` decides it against the latched goal, 80 units,
through the `nav+2FEh` latch (`include/bsp/ship_ai_path_planner.hpp`). The follower simply keeps
publishing the goal until something else ends the state.

## Run-time evidence

`bsp_game.exe --frames 700 --press-start-frame 30 --menu-select USN02 --mission-frames 500`,
exit 0, `local/follower_run.log`:

```
  ShipAiPath::plan_009e3780          009e3780   concrete      calls=468
  ShipAiPath::search_step_009ec680   009ec680   UNIMPLEMENTED calls=468
  ShipAiPath::next_point_009e3c00    009e3c00   UNIMPLEMENTED calls=468
summary mission ship ai plan requests=468 seeds=6 accepts=468
summary ... path_plan_refreshes=468 path_picks=468 path_publishes=0
```

The executable still records `009E3C00` (`src/game_hosts_ship_ai.cpp` belongs to packet
`cc_exe_2q`), so nothing in this run reached the reconstructed body and the run cannot confirm the
projection. What it does establish is the input: `009EC680` is also a record, so every one of the
six seeded plans stays in state 1 with the two-node graph above, and the follower would run the
two-node path on all 468 picks.

**That predicts `path_publishes` stays 0 even once the follower is wired**, because a two-node plan
sets `record+20h = 1` and never reaches `009E4243`, so `009EE5F9` keeps taking the skip. The zero
in `docs/GAME_EXECUTABLE.md` milestone 2p is correct behaviour for an open-sea plan, not a gap.
What the wiring does change is the point itself: `blk+A94h`/`+A9Ch` would stop taking the pose
and start taking the goal, and `009D9E50` would answer a real remaining length at `009EE6AC`. This
paragraph is a static prediction; the packet that owns the executable host should confirm it with a
run.

## The host the executable must implement

In call order for one `009E3C00` call that finds a corner, with the native call site.
`reports/ship_ai_path_follower.json` carries the same list with `address` / `native` rows.

| # | site | containing function | native | method |
| --- | --- | --- | --- | --- |
| 1 | `009E3D13`, `009E3D3C` | `009E3C00` | `009D5930` | reconstructed here, not a host call |
| 2 | `009E3D8C`, `009E3DD2` | `009E3C00` | field read | `lateral_anchor_node_10(node+10h)` |
| 3 | `009E3DCD` | `009E3C00` | `00811D80` | `order_turn_limit_at_00811d80(xz)` |
| 4 | `009E3EAE` | `009E3C00` | `0082E850` | `owner_class_turn_radius_0082e850()` |
| 5 | `009E3EC0` | `009E3C00` | field read | `owner_radius_09c8()` |
| 6 | `009E3FFC` | `009E3C00` | `00414C60` | `length_2d_00414c60`, already reconstructed |
| 7 | `009E4162` | `009E3C00` | `009D6550` | reconstructed here, not a host call |
| 8 | `009E4200` | `009E3C00` | `004218E0` | `avoid_zone_manager_004218e0()` |
| 9 | `009E4216` | `009E3C00` | `00417EF0` | `segment_hits_zone_00417ef0(...)` |

`00BF7030`, the CRT square root, is called at `009E3E83`, `009E3EF4`, `009E3F36` and `009E426D`
inside the same `1e-10` guard `00414C60` uses, so the reconstruction routes all four through
`length_2d_00414c60` rather than adding a host method.

## Coverage

| routine | coverage |
| --- | --- |
| `009E3C00` | whole: `009E3C00-009E432A`, read from the listing, every float expression from the x87 |
| `009D5930` | whole: `009D5930-009D5987` |
| `009D6550` | whole: `009D6550-009D65D0` |
| `004F3970` | whole: `004F3970-004F3B98` |
| `00417EF0` | whole: `00417EF0-00417F59`. Its decider `004179D0` was not read; see the section above |
| `009D9E50` | whole, projected by `ship_ai_path_planner`; this packet adds the cursor evidence only |
| `009ED6B0` | partial, read only for the consumer mapping: `009EE580-009EE6E5` and `009EE6D3-009EE7BD`. Everything else in `009ED6B0-009EF228` is unread here |
| `009E46F0` | partial: `009E47D3-009E4878` only, to confirm the second call site's contract |
| `00417E90` | partial: `00417E90-00417EEB` read for the polarity comparison, not projected |
| `00811D80`, `0082E850`, `004218E0`, `004120D0`, `004179D0`, `009D5920`, `00417610` | not read here; modelled or already named |

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `ShipAiPathPlanBlock::node_count`, "`009D9D1C`; `009EC6AC` clears it" (`include/bsp/ship_ai_path_planner.hpp`) | a step cursor: the index of the node the ship has already passed | the nine-writer table above; eight store zero and `009E421F` is a `+1`. `009E3C6C` walks that many links and `009D9E67` walks one more |
| `ShipAiPathPointRecord::node_18`, "zero means no point" (`include/bsp/ship_ai_goal_vector.hpp`) | the target node's **lateral record** handle, `node+10h`, not the node | `009E4243 MOV EAX,[EDI+10h]` / `009E4246 MOV [EBP+18h],EAX`, with `EDI` the target node |
| `ShipAiPathPickHost::path_node_width_20`, "the float at node+20h" | the float at the **lateral record**'s `+20h` | follows from the row above: `009EE63A` reads `[EDI+20h]` where `EDI` came from `record+18h` |
| `ShipAiPathPointRecord::direction_1c`, "+1Ch out, 009EE602" read as a direction | a side code 0/1/2 from the target node's signed byte `+14h`, identical to `ShipAiNavTurnSide` | `009E4229-009E4240`: `>0` gives 2, `<0` gives 1, `0` gives 0 |
| `ShipAiPathPointRecord::more_path_20`, "the path continues past this point" | the reverse: 1 means there is **no** node past the target, so the published point is the target point and `+10h`/`+14h` are not written | `009E3D65` sets it when `after` is null and `009E3D88` clears it when `after` exists; `009E419B` and `009E3F88` agree |
| `ShipAiNavState::last_leg_338` (`include/bsp/ship_ai_navigation.hpp`) | `blk+338h` is "a following leg exists", the opposite of `last_leg` | `009EE776 CMP byte [ESP+98h],0` / `009EE77E SETZ AL` / `009EE783 MOV [ESI+338h],AL`, so it is `record+20h == 0` |
| `docs/SHIP_AI_PATH_SEARCH.md`, "`+10h` lateral record read by `009E3D75` and `009E3D8C`" | the reads are `009E3D8C`, `009E3DAB`, `009E3DD2` and `009E4243`; `009E3D75` is not an instruction start | the listing of `009E3C00` |

No prior ledger name or reconstruction record was replaced by this packet.

## Names for the orchestrator to apply

`009E3C00` is leased to `agent/cc-exe-2q` and `004F3970` and `00417EF0` sit inside other packets'
follow-ups, so this packet documents them and applies nothing.

| address | proposed name | evidence |
| --- | --- | --- |
| `009E3C00` | `BSP_ShipAiPathPlan_NextPathPoint` | `void __thiscall(plan)(float* record)`, `RET 4`, body `009E3C00-009E432A`, complete. Walks `plan+34h` links from `plan+20h`, takes the next node as the target, offsets it by the lateral record at `target+10h` through `00811D80`, and publishes a point `max(owner+9C8h * 0.75, distance to the target)` along either the straight line to the target or a tangent of the corner circle of radius `class turn radius * 1.5`. Writes `plan+4Ch`, `+50h`/`+54h`, `+60h`/`+64h`, and is the only site in the image that advances `plan+34h` (`009E421F`). Callers `009ED6B0` at `009EE5F4` and `009E46F0` at `009E4813`. docs/SHIP_AI_PATH_FOLLOWER.md |
| `004F3970` | `BSP_Circle2f_TangentChord` | `bool __thiscall(circle)(const float2* point, float2* base, float2* half_chord)`, `RET 0Ch`, body `004F3970-004F3B98`, complete. Returns the tangent chord's foot and half-chord offset for a point outside the circle and the perpendicular chord through the point for one inside; false and no writes when the point is at the centre (`004F39F4`) or within `1e-6` of the circle (`004F3A3B`). Sole caller `009D6550` at `009D6562`. Belongs to the open `ship_ai_nav_circle_tangent` packet. docs/SHIP_AI_PATH_FOLLOWER.md |
| `00417EF0` | `BSP_AvoidZoneManager_SegmentHitPoint` | `bool __thiscall(manager)(layer, const float2* from, const float2* to, float2* hit)`, `RET 10h`, body `00417EF0-00417F59`, complete. `004120D0` for the layer's group then `004179D0`; true means the segment meets a zone and `hit` receives the point. The float2-out sibling of `00417E90`. Callers `00422500` and `009E3C00` at `009E4216`. docs/SHIP_AI_PATH_FOLLOWER.md |

## Follow-up packets

| packet | addresses | what is left |
| --- | --- | --- |
| `ship_ai_path_lateral_record` | `009D5920`, `00417610`, the record at `node+10h` | this packet read `+0h`/`+4h` and `+10h`/`+14h` from the consumer side and the nav arm reads `+20h`; the producer and the rest of the layout are unread |
| `ship_ai_nav_circle_tangent` | `009D68B0`, `004F47B0` | `009D6550` and `004F3970` are now read whole, so what is left is `004F47B0` and `009D68B0`'s own arms |
| `avoid_zone_geometry` | `004179D0`, `00422500`, `0041B840` | `004179D0` decides both `00417E90` and `00417EF0`; the polarity here rests on the two siblings agreeing |
| `ship_ai_navigation_arm_tail` | `009EEAAB-009EF228` | unchanged, and still the span that would move a navigating ship |
| `cc_exe_2q` (the executable) | `009E3C00`, `009EC680` | wire the follower and the search into `src/game_hosts_ship_ai.cpp` and confirm the run prediction above; also apply the record-field corrections to `include/bsp/ship_ai_goal_vector.hpp` and `include/bsp/ship_ai_navigation.hpp` and the cursor rename in `include/bsp/ship_ai_path_planner.hpp` |

## no_ghidra_function

none. Every routine read for this packet has a Ghidra function whose stored body range matches the
bytes on disk: `009E3C00-009E432A`, `009D5930-009D5987`, `009D6550-009D65D0`,
`004F3970-004F3B98`, `00417EF0-00417F59`, `00417E90-00417EEB`, `009D9E50-009D9F8C`. The one gap met
was inside `009ED6B0`, whose stored listing has undisassembled spans past `009EE6E5`; the consumer
reads quoted here were taken from the stored listing starting at instruction boundaries it does
carry, never from a `disasm-raw` resync.
