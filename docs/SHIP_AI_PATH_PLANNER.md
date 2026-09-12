# The planner the ship AI hands its goal to

Addresses: 009E3780 009D9150 009D9230 009D9D40 009D9E50 009D9CC0 009DA590 009DAB10 00417E40 00417580 0041B840

`009ED3E0 BSP_ShipAi_RefreshPathPlan` hands the latched goal `blk+1DCh`/`+1E0h` and the pose
`blk+184h` to `009E3780` at four sites. This packet reads `009E3780` end to end and finds that it
is not the search: it is the request-and-revalidate gate in front of one plan block. On an empty
block it latches the goal, pushes both endpoints clear of the avoid zones, allocates a two-node
graph (the ship, the goal) and leaves the block in state 1. On a block that already holds a plan
it answers one question, is this still the plan for this goal and this ship position, and the
caller tears the block down when the answer is no. The search that fills in the nodes between the
two seeds is `009EC680`, one state transition per navigation tick.

Names here are hypotheses, not recovered symbols. Reconstruction:
`include/bsp/ship_ai_path_planner.hpp`, `src/ship_ai_path_planner.cpp`. Evidence rows:
`reports/ship_ai_path_planner.json`.

## The two plan blocks the navigator owns

`009DA4E0 BSP_ShipAi_ClearPathPlan` clears two identical field sets, `nav+240h..+25Ch` and
`nav+2A8h..+2C4h`, `68h` apart. They are two embedded plan blocks whose bases are `nav+224h` and
`nav+28Ch`; `nav+2F4h` points at the plan in use and `nav+2F8h` at the plan being computed, and
`009ED5BD`/`009ED5C3` swap the two pointers once the second reaches a usable state. The
constructor is `009D9CC0`.

| offset | value at construction | what it is |
| --- | --- | --- |
| `+00h` | `00D2152C` | the block's vtable (`009D9D0D`) |
| `+04h`, `+08h` | `20.0f` (`00CE3930`) | `009EE61E` reads `+8h` as the width the navigator publishes for the point |
| `+0Ch` | `1600.0f` (`00D05A40`) | the capture radius squared: the corridor is 40 units wide |
| `+10h` | `2500.0f` (`00D20278`) | the goal may drift 50 units before the plan dies |
| `+14h` | `0.5f` (`00CE3800`) | the revalidation delay in seconds; `009EC68D` counts it down |
| `+1Ch` | `0` | the search state, below |
| `+20h` | null | the head node, the one at the ship |
| `+24h` | null | the goal node |
| `+2Ch` | `0` | the search context `009EC697` hands to `009EC280` |
| `+30h` | `1.0e7f` (`00CF58EC`) | the goal's clearance from the nearest avoid zone |
| `+34h` | `0` | the node count, which bounds both walks |
| `+38h` | `0` | the avoid-zone layer key |
| `+3Ch` | null | the owning ship: `+9C8h` its radius, `+538h` its class |
| `+40h`, `+44h` | not written | the latched goal, written by `009E37F2` |
| `+48h` | `0` | unread |
| `+4Ch`, `+50h`, `+54h`, `+60h`, `+64h` | not written | written by the walk in `009E3C00` |
| `+28h`, `+58h`, `+5Ch` | not written | unread |

## The search state at `+1Ch`

`009E3780` gates on it and `009EC680` drives it. The names come from `009EC680`'s switch.

| value | name | set at | meaning |
| --- | --- | --- | --- |
| 0 | Empty | `009D9D69` | no graph; `009E3780` seeds |
| 1 | Seeded | `009E381E` | two nodes exist, the search has not run |
| 2 | Failed | `009EC6B0` | `009E3790` refuses every request; `009EC680` sets it when `+3Ch` is null |
| 3 | Searched | `009EC707` | `009D5A20` reported the search done |
| 4 | Extracting | `009EC71A` | `009EC721` also clears the node count |
| 5 | ExtendSearch | `009EC73B` | `009D96A0` wants another pass |
| 6 | Extracted | `009EC73B` | the same store, `6 - (flag != 0)` |
| 7 | Ready | `009EC742` | a usable path |

`009EC683`/`009EC68F` is where `plan+14h` loses the frame delta, before the state is even read.

Above 3 means "a node list exists": that is the test `009E37A5` uses to pick the corridor branch
and the test `009ED5A6` uses before swapping the two plans.

## What `009E3780` does, in order

`__thiscall(plan)(const float2* pose, const float2* goal, int layer, float radius) -> bool in AL`,
`RET 10h`, body `009E3780-009E3BFB`. The four call sites are `009ED523`, `009ED588`, `009ED5FD`
and `009ED692`; none of them cleans the stack, and all seven returns are `RET 10h`, so there are
four stack dwords. The fourth, the unit radius the caller loads from `[ship+9C8h]`, is never read
by the body: `009E3ADB` reads the same field through `plan+3Ch` instead.

### The gate

| # | address | what happens |
| --- | --- | --- |
| 1 | `009E378D` | state 2 returns false at once |
| 2 | `009E379B` | state 0 goes to the seeding branch |
| 3 | `009E379D` | `plan+38h` not equal to the requested layer returns false |
| 4 | `009E37A5` | state above 3 goes to the corridor branch |
| 5 | `009E37AF` | with a goal node, only the goal drift is tested (`009E399E`) |
| 6 | `009E37B7` | without one, reset and loop back into the seeding branch |

### The seeding branch, `009E37CC`

| # | address | what happens |
| --- | --- | --- |
| 1 | `009E37F2` | `plan+40h`/`+44h` latch the goal, `plan+38h` takes the layer |
| 2 | `009E3819` | `plan+30h = 1.0e7f`, `plan+1Ch = 1` |
| 3 | `009E3831` | `00417E40(manager)(&goal, layer)`: the zone containing the goal |
| 4 | `009E3851` | inside a zone: `00417580(zone)(&out, &goal, 10.0f, 1)` moves the goal to the boundary, and `plan+30h` becomes 0 |
| 5 | `009E387B` | outside: `004120D0(manager)(layer)` for the layer's zone group |
| 6 | `009E38A2` | `0041B840(group)(&out, &goal, 100000.0f, 1.0f)`: the nearest zone boundary point |
| 7 | `009E38C3` | `plan+30h` = the distance from the goal to that point, when it is at least 1.0f |
| 8 | `009E38EF` | the same containment query on the ship's own pose, margin `5.0f` at `009E390B` |
| 9 | `009E3927` | `operator new(54h)` and `009D9150(&pose, layer)` into `plan+20h` |
| 10 | `009E394C` | `operator new(54h)` and `009D9150(&goal, layer)` into `plan+24h` |
| 11 | `009E3973` | `009D9230(plan+20h)(plan+24h)` links them and measures the segment |
| 12 | `009E3980` | `owner->vtable[50h]()` seeds the start node's `+48h`; `+45h` becomes 1 |
| 13 | `009E3995` | returns true |

So `plan+30h` is the goal's clearance: `0` when the goal had to be pushed out of a zone, the
distance to the nearest boundary plus one unit when a zone is in range, and the `1.0e7f` sentinel
when the layer has no group or nothing is within `0041B840`'s reach. `0041AEA0`, which `0041B840`
calls per zone, returns `FLT_MAX` for a point outside the zone's slackened AABB, `0` for a point
inside the polygon, and otherwise the distance to the nearest edge with the outward unit normal in
its out parameter; `0041B840` keeps the smallest and returns `pt - normal * (distance + push)`.

### The goal-drift test, `009E399E` and `009E3A05`

The same rule on both paths: the plan survives only while the goal stays within `sqrt(plan+10h)`,
50 units, of the goal it was latched to. The squared distance is rounded to float before the
comparison (`009E39C5` then `009E39C9`), and the comparison is `JBE`, so an unordered pair kills
the plan.

### The corridor test, `009E3A41`

Only reached with a node list. `009E3A44` skips the whole test while `plan+14h` is still positive,
which is how the ship is allowed to leave a node before the plan is re-examined. Otherwise, with
`n0 = plan+20h` and `n1 = n0->+20h`:

1. `009E3AB9`: the ship is on plan while it is within `sqrt(plan+0Ch)`, 40 units, of `n0`.
2. `009E3ABF`: leaving that circle re-arms the delay to
   `min(1.5f, min(100.0f, owner+9C8h) / [owner+538h]+500h)` seconds, and to `1.5f` alone when
   `plan+3Ch` is null. `00828F20 BSP_ShipClass_DeriveTurnFields` writes `class+500h` as the class
   maximum speed, so the delay is roughly the time the ship needs to cross its own radius.
3. `009E3B5B`: the ship's offset from `n0` is projected onto the unit segment `n0 -> n1`. A
   projection that is not strictly positive means the ship is behind the segment, and the plan
   dies.
4. `009E3BBC`: otherwise the ship must be within the same 40 units of its perpendicular foot on
   that segment. Further than that and the plan dies.

The corridor branch always uses `n0->+20h` as the next point, never the direction the walks below
choose. For a freshly extracted chain the two agree; whether they can diverge is not established.

## The path node, `009D9150`

`54h` bytes, vtable `00D214F4`. The graph is undirected: every node carries two links and each walk
picks one of them per node.

| offset | field | written by |
| --- | --- | --- |
| `+04h` | the chosen direction, `0` undecided, `+1` follow `+24h`, `-1` follow `+20h` | the walks |
| `+0Ch` | word `FFFFh` | `009D9173` |
| `+10h` | a lateral-offset record, or null | the search; read at `009E3D75` |
| `+14h` | a byte whose sign selects the side | the search; read at `009E3DFE` |
| `+18h`, `+1Ch` | the point, world X and Z | `009D9165` |
| `+20h` | one link | `009D9242` |
| `+24h` | the other link | the search |
| `+28h` | the back pointer of `+20h` | `009D9248` |
| `+31h` | set when the `+20h` link is shorter than one unit | `009D92BB`/`009D92C7` |
| `+34h`, `+38h` | the two link lengths | `009D92AF` for `+34h` |
| `+3Ch` | the remaining path length from this node onward | the search |
| `+40h` | the layer key the constructor was handed | `009D91A9` |
| `+45h` | set on the node at the ship | `009E3991` |
| `+48h` | the owner seed | `009E398E` |
| `+4Ch`, `+50h` | the two link costs | the search |

`009D9230` is the link: `this->+20h = next`, `next->+28h = this`, `this->+34h` the segment length
through the same x87 kernel as `00414C60` (the rounded sum of squares against the double `1e-10`
at `00CE3820`, then the CRT square root), and `this->+31h` set when that length is below one unit.
A null argument is a no-op.

## Where the path point, side code and remaining length come from

`009D9E50` (`009D9E64-009D9F02`) and `009E3C00` (`009E3C6C-009E3D0E`) share one walk. From
`plan+20h`, for at most `plan+34h + 1` steps, each undecided node picks its direction: `+1` when
`+50h + +38h + (+24h)->+3Ch` is strictly less than `+4Ch + +34h + (+20h)->+3Ch`, `-1` otherwise,
and the degenerate cases when one link is null. The step follows `+24h` on `+1` and `+20h` on
`-1`, and a node with no matching link ends the walk.

- **The remaining length** is `009D9E50`, `__thiscall(plan)(const float2* pose) -> float`. It runs
  the walk to its end, falls back to `plan+24h` when the walk ran out, and returns that node's
  `+3Ch` plus the straight line from the pose to it. The navigation arm reads it at `009EE6AC` and
  `009EE856` tapers the turn lead against it.
- **The path point** is `009E3C00`. It runs the same walk one step short of the end (`009E3C6F`
  compares against `plan+34h` and the loop stops one earlier than `009D9E50`'s), then takes three
  nodes: the one it stopped on, its successor and the one after. `009E3D8C` reads the successor's
  `+10h` and takes the two floats there when it is non-null and the successor's own `+18h`/`+1Ch`
  otherwise; when it is non-null, `009E3DCD` calls `00811D80 BSP_UnitAiOrder_TurnLimitAt` and
  offsets the point by that answer times the record's `+10h`/`+14h` pair. The point goes to
  `record+8h`/`+0Ch`.
- **The side code** is the byte at `record+20h`. `009E3D65` sets it to 1 when the walk found no
  node after the successor, `009E3D88` clears it when it did, and `009E3C54` clears it when the
  plan has no head at all. `009E3F88` in the unprojected tail overwrites it, and `009E4013` tests
  the sign of the successor's `+14h` to pick which way the lateral offset goes. `009EE765` stores
  the byte on `blk+304h`.

Only the walk itself is projected here; `009E3C00`'s tail past `009E3D81` belongs to
`ship_ai_path_follower`, and the two addresses quoted from it above were read from the decompiler,
not projected.

## What ends a `moveto`

Three things, in this order.

1. `009EF034`, inside the controls step: `nav+2FEh = 1` on the frame the navigation arm runs out
   of path, gated on `nav+2FCh` being clear. `009DE0EF` and `009ED779` are the only writes that
   clear it.
2. `state->vtable[2Ch](&goal)` in the state step (`009E5821` for `movetopos`, the same slot for
   `moveonpath`). Both vtables hold `009DAB10`, three instructions that load `[state+4h]+8h` into
   ECX and tail-jump to `009DA590`. `009DA590` returns false at once when `nav+2FEh` is clear;
   otherwise it returns true when the goal handed in this frame is within `sqrt(6400)`, 80 units,
   of `nav+2F4h`'s latched goal `+40h`/`+44h`, and clears `nav+2FEh` and returns false when it is
   not. The radius is the double at `00D21530`.
3. For `movetopos`, `009E5827` requires `007ADC60 BSP_EntityCommand_IsOnFinalLeg` as well before
   the `finished` block runs (`009E5829`).

So a `moveto` ends when the follower has consumed the path **and** the goal it is being asked for
now is the goal the active plan was built for. A goal that moved more than 80 units since the plan
latched it does not end the command; it clears the latch and the ship keeps going.

## The host the executable must implement

In call order, with the native call site. `reports/ship_ai_path_planner.json` carries the same list
with `address`/`native` rows.

| # | site | native | method |
| --- | --- | --- | --- |
| 1 | `009E3821`, `009E3870`, `009E38E3` | `004218E0` | `avoid_zone_manager_004218e0` |
| 2 | `009E3831`, `009E38EF` | `00417E40` | `zone_containing_point_00417e40` |
| 3 | `009E3851`, `009E390B` | `00417580` | `push_point_out_of_zone_00417580` |
| 4 | `009E387B` | `004120D0` | `zone_group_for_layer_004120d0` |
| 5 | `009E38A2` | `0041B840` | `nearest_zone_boundary_0041b840` |
| 6 | `009E3927`, `009E394C` | `00BF681B` | `allocate_path_node_00bf681b` |
| 7 | `009E3980` | `owner->vtable[50h]` | `owner_seed_vtable50`, contract unread |
| 8 | `009E3ADB` | `owner+9C8h` | `owner_radius_09c8` |
| 9 | `009E3AF0` | `[owner+538h]+500h` | `owner_class_max_speed_0500` |
| 10 | `009D9D53` | `head->vtable[0](1)` | `release_node_list_vtable0`, contract unread |

`00414C60` and `00415510` are reconstructed or named already and are called directly rather than
through the host.

## Coverage

| routine | coverage |
| --- | --- |
| `009E3780` | complete |
| `009D9150` | complete |
| `009D9230` | complete |
| `009D9D40` | complete |
| `009D9E50` | complete |
| `009D9CC0` | complete |
| `009DA590` | complete |
| `009DAB10` | complete |
| `00417E40` | complete, and its callees `00412120` and `004178F0` read whole. `00416B50` under `004178F0` is unread |
| `00417580` | complete. `00414F50`, `00416B50` and `00416F30` are unread |
| `0041B840` | complete. `0041AEA0` read from the decompiler for its return contract and outward direction only; `00419AB0`, `004F4B50`, `00419260` unread |
| `004120D0` | complete, read but not named: it is not in this packet's lease |
| `009EC680` | partial: the state transitions and the `+14h` countdown only. `009EC280`, `009E3330`, `009D5A20`, `009D9550` and `009D96A0`, which are the search itself, are unread |
| `009E3C00` | partial: `009E3C00-009E3D81`, the walk this packet shares with `009D9E50` and the plan fields it writes, read from the listing. `009E3D8C-009E4013` was read from the decompiler for the point source and the side byte only; nothing past `009E3D81` is projected, and it belongs to `ship_ai_path_follower` |
| `009ED6B0` | partial: `009EF022-009EF045` only, for the arrival latch. The rest of `009EEAAB-009EF228` is unread |
| `0041AEA0` | partial: return contract only |

Projected as C++: `009E3780`, `009D9150`, `009D9230`, `009D9D40`, `009D9E50`, `009D9CC0` (as the
block's member initialisers) and `009DA590`. The rest are host contracts or references.

## Corrections

| was | is | evidence |
| --- | --- | --- |
| the brief and `docs/SHIP_AI_STATE_STEPS.md`'s follow-up row name `00D21634` and `00D21694` as the arrival slots | they are the step slots. `00D21634` holds `009E5770` and `00D21694` holds `009E59C0`, which the ledger records as the two state steps at vtable `+0Ch`, so the vtables start at `00D21628` and `00D21688` and the arrival slot `+2Ch` is `00D21654` and `00D216B4` | the 64 bytes at `00D21628` and `00D21688`; `00D21634 - 00D21628 = 0Ch` |
| implied by the same row: each navigation state has its own arrival predicate | both slots hold `009DAB10`. There is one predicate, and it tail-jumps to `009DA590` | `00D21654` and `00D216B4` both read `B0 AB 9D 00`; `009DAB16 JMP 009DA590` |
| the brief describes `009E3780` as the planner that builds a path around avoid zones | it seeds and revalidates. It creates exactly two nodes and moves the block to state 1; `009EC680` is the search | `009E3925` and `009E3947` are the only allocations; `009EC685-009EC6C4` is the state machine |
| `include/bsp/ship_ai_state_steps.hpp` models the ten navigator fields `009DA4E0` clears as independent values | they are two embedded `68h`-byte plan blocks at `nav+224h` and `nav+28Ch`; `+240h` is the state, `+244h` the head node, `+248h` the goal node, `+250h` the search context, `+254h` the clearance, `+258h` the node count, `+25Ch` the layer | the constructor `009D9CC0` writes the same set at `plan+1Ch..+38h`; `009DA4E0`'s two groups are `68h` apart; `009ED5BD`/`009ED5C3` swap `nav+2F4h` and `nav+2F8h` |
| the decompiler renders `009E3973` as `FUN_009d9230(uVar6)` | ECX is `plan+20h`, the head node; the goal node is the argument | `009E396C MOV ECX,[ESI+20h]`; `009E396F PUSH EAX` |
| the decompiler renders `009E3980` as a double indirection on `plan+3Ch` | `plan+3Ch` is the object; the extra load is its vtable | `009E3978`, `009E397B`, `009E397D` |
| the decompiler shows four parameters and drops the fourth stack argument | four stack dwords (`RET 10h`), and the fourth, `[ship+9C8h]`, is never read | the seven `RET 10h` sites; nothing reads the fourth slot |
| the decompiler omits the divide between the two `00415510` calls | the capped radius is divided by `[plan+3Ch's +538h]+500h` before the second minimum | `009E3AF0`, `009E3AF6` |

One deliberate deviation in the reconstruction: when `operator new` returns null, `009E396C` and
`009E3986` dereference the null head and the image faults. `ship_ai_path_plan_request_009e3780`
stops with `NodeAllocationFailed` instead.

## no_ghidra_function

| start | end (inclusive) | evidence |
| --- | --- | --- |
| `009D9CC0` | `009D9D32` | `INT3` at `009D9CB7-009D9CBF` after the `RET` at `009D9CB6` that ends the previous routine; the `RET` at `009D9D32` then `INT3` at `009D9D33-009D9D3F` before the Ghidra function `009D9D40` |
| `009DAB10` | `009DAB1A` | `INT3` at `009DAB07-009DAB0F` after the `RET` at `009DAB06`; `INT3` at `009DAB1B-009DAB1F` after the `JMP` at `009DAB16`. Ghidra reports the enclosing candidate as `TRIV_body_009daab0` |

`009DA590` is a Ghidra function (body `009DA590-009DA604`). `009E5770` and `009E59C0`, the two
state steps that reach the arrival test, are not; `docs/SHIP_AI_STATE_STEPS.md` already records
their boundaries.

## Follow-up packets

| packet | addresses | what is left |
| --- | --- | --- |
| `ship_ai_path_search` | `009EC680`, `009EC280`, `009E3330`, `009D5A20`, `009D9550`, `009D96A0`, `009D9050` | The search itself: what expands the two seed nodes into a graph, what writes `node+10h`, `+14h`, `+24h`, `+3Ch`, `+4Ch`, `+50h` and `plan+34h`, and what `plan+2Ch` holds. The state names in this packet's enum come from `009EC680`'s switch and nothing else |
| `ship_ai_path_follower` | `009E3C00`, `009D5930`, `009D6550`, `009D9E50` | Unchanged from `docs/SHIP_AI_ORDER_CONSUMER.md`, minus `009D9E50`, which is projected here. The lateral-offset tail past `009E3D81` is what is left |
| `avoid_zone_geometry` | `00416B50`, `00416F30`, `00414F50`, `0041AEA0`, `00419AB0`, `004F4B50`, `00419260`, `004120D0`, `00412120` | The polygon primitives under this packet's four host methods, and whether the layer key `blk+30Ch` is an id or an object pointer: `004120D0` and `00412120` order groups by `[group+10h]` with a signed compare, which fits both readings |
| `ship_ai_owner_seed` | `009E3980+vtable50` | The float the owner hands the start node at `+48h`. The vtable of the object at `plan+3Ch` is not identified here, so the slot has no callee body and no contract |
| `ship_ai_controls_tail` | `009EEAAB-009EF228` | The gate at `009EF00D-009EF028` that decides when `nav+2FEh` is latched, which is the first half of the rule that ends a `moveto` |

## Correction from docs/SHIP_AI_PATH_SEARCH.md

Packet `cc_ai_path_search` (main 0220faec) read the search that fills the graph and corrects four
field readings of `include/bsp/ship_ai_path_planner.hpp` (recorded here, the header keeps its
names until a packet that owns it applies them): node `+2Ch` is the back pointer of `link_plus`;
node `+14h` is a signed byte; node `+31h` and `+32h` mean "this edge has been proved clear", not
"short link"; `plan+2Ch` is a float. `009D9050`, listed as a search routine, is not a routine: it
lies inside `009D8CE0`'s body. The search itself is a lazy binary expansion (`009EC280` costs the
graph, `009E3330` splits the first unproved edge on the cheapest route, `009D5A20` asks whether a
resolved route exists, `009D9550` prunes forks, `009D96A0` smooths corners), four ticks on an open
sea and usable after two.

## Correction from docs/AVOID_ZONE_BOUNDARY.md

For an outside point, `0041AEA0` subtracts the closest segment point from
the query (`0041B09B-0041B0AB`) and normalizes that vector through `00419260`.
`0041B840` then subtracts this direction times `(distance + push)` from the
query (`0041B8BA-0041B8F3`). A positive push therefore continues toward and
past that closest boundary point; it does not increase outside clearance.
The inside case returns distance zero, and `0041B840` returns the original
query when a newly best distance is nonpositive (`0041B8B1-0041B8B4`,
`0041B941-0041B957`). `docs/AVOID_ZONE_BOUNDARY.md` distinguishes these
contracts and the remaining injected geometry dependencies.

The concrete game host inspected for this batch still returned the query
unchanged from `nearest_zone_boundary_0041b840` and supplied a zero zone
group in `src/game_hosts_ship_ai.cpp`. Reconstruction of the geometry alone
does not establish that the executable uses it or that in-game avoidance
has been validated.
