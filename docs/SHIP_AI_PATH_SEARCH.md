# The search that expands the two seed nodes into a path

Addresses: 009EC680 009EC280 009E3330 009E3040 009D5A20 009D9550 009D96A0 009D92E0 009D58F0 009D5930 00417E90 00422500 0071C4F0 00417610 00423190

`009E3780` leaves the plan block holding two nodes, the ship and the goal, joined by one
segment, in state 1. `009EC680` is the search. `009ED3E0 BSP_ShipAi_RefreshPathPlan` calls it once
per navigation tick, at `009ED59B` on the plan being computed and at `009ED614` on the plan in use,
and it performs exactly one state transition per call.

It is not A\*, and it is not a visibility graph built up front. It is a lazy binary expansion over
the node graph: take the cheapest route through the graph, find the first edge on it that has not
been proved clear, ask the avoid-zone manager whether that edge crosses a zone, and if it does,
replace the edge with up to two detours around the zone's two tangent corners, offset 30 units
clear. Repeat, one edge per tick, until a route made entirely of cleared edges exists. Then prune
every fork to its cheaper branch, and smooth away the corners the path no longer needs.

Names here are hypotheses, not recovered symbols. Reconstruction:
`include/bsp/ship_ai_path_search.hpp`, `src/ship_ai_path_search.cpp`. Evidence rows:
`reports/ship_ai_path_search.json`. The block layout, the node layout and the state enum are
`include/bsp/ship_ai_path_planner.hpp`; nothing here redefines them.

## `009EC680`, the state machine

`__thiscall(plan)(float frame_delta)`, `RET 4`, body `009EC680-009EC74C`. `009EC683-009EC68F`
subtracts the frame delta from `plan+14h` before the state is read, so the revalidation delay runs
down on every tick including the ones that do nothing. `009EC692`, `009EC69B` and `009EC6A4` return
at once for states 0, 2 and 7. `009EC6AA` sets state 2 when `plan+3Ch`, the owning ship, is null.
Otherwise `009EC6BB` puts `30.0f` (`00E0E304`) into the argument slot and `009EC6D1` jumps through
the table at `009EC750`, which holds six entries for states 1 to 6.

| state | entry | what the tick does | next state |
| --- | --- | --- | --- |
| 1 Seeded | `009EC6D8` | `009EC280(head)(plan+2Ch)`, `009E3330(head)(30.0f, 0)`, `009D5A20(head)` | 3 on true, unchanged on false |
| 2 Failed | `009EC749` | unreachable: `009EC69B` already returned | - |
| 3 Searched | `009EC712` | `009D9550(head)`, then `plan+34h = 0` | 4 |
| 4 Extracting | `009EC72C` | `009D96A0(head)` | 5 on true, 6 on false |
| 5 ExtendSearch | `009EC6D8` | the same entry as state 1 | 3 on true, unchanged on false |
| 6 Extracted | `009EC742` | nothing | 7 |

`009EC734 NEG AL` / `SBB EAX,EAX` / `ADD EAX,6` is the 5-or-6 store: a removal means the shortcut
edge has to go back through the expansion.

The five callees all take the **head node** as `this`, not the plan: `MOV ECX,[ESI+20h]` at
`009EC6DC`, `009EC6F0`, `009EC6FB`, `009EC712` and `009EC72C`.

### How many ticks a plan takes on an open sea

Four, and the plan is usable after two.

1. `009ED523` seeds and `009ED528` sets `nav+2FCh`; `009ED533` re-reads that byte in the same pass,
   so `009ED59B` runs the first search tick on the seeding tick. `009E3330` probes the one edge,
   `00417E90` reports no zone, the edge is marked resolved, `009D5A20` walks ship -> goal across it
   and returns true: state 3.
2. `009ED588` revalidates, `009EC680` prunes (nothing to prune) and reaches state 4.
   `009ED5A6 CMP [EAX+1Ch],3` / `JLE` now falls through, so `009ED5BD`/`009ED5C3` swap the plan into
   use and clear `nav+2FCh`.
3. On the live plan, `009D96A0` finds `head->+20h->+20h` null and returns false: state 6.
4. State 7, Ready.

A blocked route costs one extra tick per edge split, plus one tick per node the smoothing removes,
bounded by the 150-node descent limit at `009E33A3`.

## `009E3330`, one expansion

`__thiscall(node)(float margin, int depth)`, `RET 8`, body `009E3330-009E376F`. `009E333D OR EBP,-1`
is the `-1` the direction test at `009E335A` compares against, not an argument.

The descent, `009E3340-009E3377`: stop at a node with neither link; otherwise count one step,
call `009D5930` to decide this node's direction, and follow it, but only while the edge on that
side is already resolved. The first unresolved edge is the one this pass works on. `+4h == -1`
selects the `+20h` edge and its `+31h` flag, anything else the `+24h` edge and `+32h`.

`009E3040` then probes that edge. A clear edge (`AL` zero) or a depth of 150 or more sets the
edge's resolved flag and ends the pass (`009E3573`, `009E3764`). Otherwise the two candidate
corners are filtered once more: a candidate that carries the same `(zone, corner index)` pair as
the far node, or that sits within `sqrt(25)` units of the node being expanded, is destroyed
through its own vtable slot 0 (`009E340D`, `009E346E`, `009E3615`, `009E3676`). Both gone, and the
edge is marked resolved as well.

The splice, `009E347A-009E356F` for the `+20h` edge and `009E3682-009E3761` for `+24h`, mirror
images of each other:

- one survivor: `survivor -> far`, then `node -> survivor`, on the same side as the edge.
- two survivors: `left` goes on the node's `+20h` and `right` on its `+24h`, and both rejoin the
  far node, `left` through `009D92E0` and `right` through `009D9230`. The result is a diamond, and
  the two branches are what the cost pass and the prune choose between.
- the far node already carries a back pointer on the slot the splice needs (`009E34C5` reads
  `far+2Ch`, `009E36CD` reads `far+28h`): a positional copy of the far node is allocated and
  inserted first, so the existing structure is not rewired. `009E34F7`/`009E36FF` store that copy
  with a raw `MOV`, not through `009D9230`, so the node keeps the link length and resolved flag the
  old link left behind.
- the node has no free link slot (`009E3512`, `009E371A`): a positional copy of the node itself
  takes the fork and the node points at the copy.

## `009E3040`, the edge probe and the two corners

`__thiscall(node)(int side, node** out_left, node** out_right, float margin) -> bool in AL`,
`RET 10h`, body `009E3040-009E3325`.

1. `009E3058` `004218E0` the avoid-zone manager.
2. `009E3075` `00417E90(manager)(layer, &node.pos, &far.pos, &out_zone, &out_edge)`. False returns
   0 at once and nothing else runs: this is the whole "is the sea clear between these two points"
   question, and on an empty sea it is the only host call the search ever makes.
3. `009E3105` `00422500(zone)(far.pos by value, edge index, near corner hint, far side hint,
   margin, &left_pt, &right_pt, &left_index, &right_index)`. The two hints are only real when the
   node in question already sits on this same zone (`009E3091`, `009E30BE`); otherwise they are 0
   and -1. The return is a side preference: below zero only the left corner is used, above zero
   only the right, zero both (`009E3256`, `009E32AA`).
4. Each candidate with a non-negative index has to survive two tests. `0071C4F0` on the global at
   `00E188A8` must not report it outside the world box, and the wrapped difference between the
   segment's bearing and the candidate's bearing from this node must have the candidate's sign:
   non-negative for the left one (`009E31CA`), non-positive for the right (`009E324A`).
5. A survivor becomes a `54h`-byte node at the offset point carrying `+8h` the zone, `+0Ch` the
   corner index and `+14h` the side code, `0FFh` for left and `1` for right, and `009D58F0`
   attaches the zone's corner record to it.
6. When the edge was blocked and neither candidate was built, `009E330C` sets `node+30h`. The
   cost pass reads that byte as FLT_MAX, which keeps every route through the node out of the
   comparison.

The probe returns 1 in every blocked case, whether or not it produced anything.

## `009D5A20`, when the search is done

`__thiscall(node) -> bool in AL`, `RET 0`, body `009D5A20-009D5A7A`. A node with neither link is a
leaf and answers yes. Otherwise each link is followed only when its resolved flag is set, and
either side answering yes is enough. So the question is "is there a route from here to a leaf
across resolved edges only", and `009EC703` reads it as "the search is done". It is the only reader
of the two flags besides the expansion.

## `009EC280`, the cost pass

`__thiscall(node)(float side_switch_penalty) -> float in ST0`, `RET 4`, body `009EC280-009EC67E`.
A post-order recursion over the whole graph, run at the start of every expansion tick. On every
node it reaches it writes `+4h` the cheaper direction, `+4Ch` and `+50h` the two link costs, and
`+3Ch` the route length from that node onward, which is what `009D9E50` later reports to the
navigation arm and what the direction rule compares.

- `009EC28A`: a node flagged as a dead end costs `FLT_MAX` and returns.
- `009EC2C3`: only the node the ship stands on (`+45h`) carries a turn cost. Its heading seed
  `+48h`, which `009E3980` took from the owner, goes to the child across any link shorter than ten
  units (`009EC2DC`, `009EC2F7`) so a very short first hop moves the penalty one node along.
- `009EC370`/`009EC379`: `SubtractWrappedAngle(HeadingAngle(child - node), seed)`, absolute value
  by `AND 7FFFFFFFh` at `009EC389`, through the `00419010` ramp `(settings+6F0h, 0)` to
  `(settings+6F4h, settings+6F8h)`.
- each link's cost is then capped at a fifth of the route behind it by `00415510`
  (`009EC497`, `009EC52F`, `009EC5B6`, `009EC5D1`).
- `009EC4CB`, `009EC561`, `009EC5FC` and `009EC622`: when the two side codes multiply to a negative
  number, the route pays `plan+2Ch` once. `009D9CE1` writes that field zero and no other writer was
  found, so the side-switch penalty is inert in the image. `009EC6D8 FLD float ptr [ESI+2Ch]` shows
  it is a float.
- with both links present, `009EC63D`/`009EC641` keep the cheaper total, a tie going to `+24h`.

## `009D9550`, the prune

`__thiscall(node)`, `RET 0`, body `009D9550-009D9690`. At every fork it keeps one branch and
destroys the other: an unresolved edge loses (`009D9565`, `009D956A`), a branch `009D5A20` rejects
loses, and otherwise the cheaper of `child->+3Ch + link length + link cost` wins, a tie going to
`+24h` (`009D959B JBE`). The survivor is moved into the `+20h` slot (`009D95DB-009D9600`), the
costs are zeroed, `+38h` takes the `1000000.0f` no-link sentinel, `+31h` is forced to 1 and `+45h`
cleared, and the pass recurses into the next node. On the way out it splices every following node
whose incoming link is under one unit out of the chain (`009D9633-009D968B`).

What state 4 therefore starts from is a plain singly-linked chain along `+20h`, every `+24h` null.

## `009D96A0`, the smoothing

`__thiscall(node) -> bool in AL`, `RET 0`, body `009D96A0-009D98BB`. Tail-recursive along the
chain, at most one removal per call. For each triple `node -> a -> c` it takes the wrapped
difference between the bearing to `c` and the bearing to `a` and removes `a` when the path does not
bend far enough toward the side `a`'s corner sits on: past `+tolerance` with a positive side code
(`009D985E`), or past `-tolerance` with a negative one (`009D9871`). The tolerance is
`min(10 / max(node+34h, 1), 0.05)` radians (`009D97EB-009D9842`), so a long leg tolerates almost
nothing and a short one up to 0.05 radians. Either leg shorter than `sqrt(9)` units forces the
removal regardless (`009D9714`, `009D9734`), and in that case alone `009D98AF` marks the new link
resolved so the next expansion does not re-test it.

Returning true sends the state back to 5, so a turn-rule shortcut is always re-probed against the
zones before the plan is declared Ready.

## What the search reads from the avoid-zone manager

`004218E0 BSP_AvoidZoneManager_GetSingleton` is already named and reconstructed
(`docs/MISSION_LOAD_HOSTS.md`). The search touches its data through four entry points and never
reaches a polygon directly.

| what | evidence |
| --- | --- |
| a layer key selects a zone group | `00417E90` calls `004120D0(manager)(layer)` at `00417ECE`, the same routine `009E3780` uses |
| a group answers "does this segment hit a zone" | `00417E90` calls `004179D0(group)(&from, &to, &out, &out2)` at `00417ED5` and copies the zone it reported into the caller's out slot at `00417EE6`. `004179D0`'s body is not read here |
| a zone is an array of corner records | `00417610(zone)(i)` is `zone->[0][i % zone->[1]]`, a signed `IDIV` at `0041761B`, so `zone+0h` is the record array and `zone+4h` its count |
| the hit is reported as an index into that array | `00422500`'s head indexes `*param_1 + param_4 * 4` and wraps to the next corner, so `00417E90`'s second out value is an edge index |
| a corner record carries a direction pair and a cached scalar | `00423190` reads `record+10h` and `record+14h` (`004231EF`, `00423209`) and writes `record+20h` at `004234BC`, skipping the whole body when `record+20h` is already above the constant at `00D7A218` |
| a node remembers which corner it is | `009E328D` writes the zone into `node+8h` and `009E3297` the index into `node+0Ch`; `009E33BD`/`009E33C6` compare both to recognise a duplicate |

The zones' shapes themselves are not established here. `00422500`'s tangent walk and `004179D0`
are the two bodies that would settle them, and both are left to `avoid_zone_geometry`.

## Which node fields the arm and `009D9E50` read, and who writes them

`docs/SHIP_AI_PATH_PLANNER.md` established the readers. This packet supplies the writers.

| field | read by | written by |
| --- | --- | --- |
| `+04h` direction | both walks, `009D9E80` and `009E3C80` | `009D5930` during the descent, `009EC280`, `009D9550`, and the walks themselves |
| `+10h` lateral record | `009E3D75` and `009E3D8C` for the offset point | `009D5920`, from `00417610` |
| `+14h` side code | `009E3DFE` and `009E4013` to pick the offset direction | `009E329D` and `009E32F1`, a signed byte |
| `+18h`, `+1Ch` point | both walks | `009D9165`, at the offset corner `00422500` returned |
| `+20h`, `+24h` links | both walks | `009D9230`, `009D92E0`, and the raw stores in `009E34F7`, `009E36FF`, `009D95FC`, `009D9682` |
| `+34h`, `+38h` lengths | the direction rule | `009D92AF`, `009D935F`, and `009D95F4`, `009D965B`, `009D9627` |
| `+3Ch` remaining length | `009D9E50`'s answer to `009EE6AC`, and the direction rule | `009EC280` only |
| `+4Ch`, `+50h` link costs | the direction rule | `009EC280`, cleared by `009D9550` and `009D96A0` |
| `+45h`, `+48h` start seed | `009EC280` | `009E3991`/`009E398E`, propagated by `009EC2E7`, cleared by `009D9550` and `009D96A0` |

`plan+34h` is the open one. `009EC721` clears it at the 3 to 4 transition, `009D9CC0`, `009E3C3F`
and `009ED65F` also write zero, and no writer that advances it was found in `009EC680`, any of its
callees, or the first 901 instructions of `009ED6B0`. Both walks read it as a step budget:
`009D9E6C` only refuses a negative value, so zero still allows one step, while `009E3C6F` skips the
walk entirely at zero and uses the head node as the stop. Whatever advances the cursor along the
path belongs to `ship_ai_path_follower`.

## The host the executable must implement

In call order for one Seeded tick, with the native call site.
`reports/ship_ai_path_search.json` carries the same list with `address` / `native` rows.

| # | site | containing function | native | method |
| --- | --- | --- | --- | --- |
| 1 | `009EC30B`, `009EC31E`, `009EC331` | `009EC280` | `00424C40` | `game_settings_turn_ramp_00424c40` |
| 2 | `009E3058` | `009E3040` | `004218E0` | `avoid_zone_manager_004218e0` |
| 3 | `009E3075` | `009E3040` | `00417E90` | `segment_blocked_00417e90` |
| 4 | `009E3105` | `009E3040` | `00422500` | `zone_detour_corners_00422500`, contract partial |
| 5 | `009E3189`, `009E3207` | `009E3040` | `0071C4F0` | `point_outside_world_bounds_0071c4f0` |
| 6 | `009E3263`, `009E32B7`, `009E34CC`, `009E3533`, `009E36D4`, `009E3725` | `009E3040`, `009E3330` | `00BF681B` | `allocate_path_node_00bf681b` |
| 7 | `009D5917` | `009D58F0` | `00417610` | `zone_corner_record_00417610` |
| 8 | `009D5923` | `009D58F0` | `00423190` | `ensure_zone_corner_metric_00423190`, contract partial |
| 9 | `009E340D`, `009E346E`, `009E3615`, `009E3676`, `009D95A9`, `009D95C9`, `009D9680`, `009D98A9` | `009E3330`, `009D9550`, `009D96A0` | `node->vtable[0]` | `release_path_node_vtable0`, contract unread |

`00414EB0 BSP_Geometry_HeadingAngle`, `00438B10 BSP_Math_SubtractWrappedAngle`,
`00415510 BSP_Math_MinFloatByRef` and `00419010 BSP_Math_InterpolateClamped` are named and
reconstructed already; the reconstruction calls them directly rather than through the host.
`00BF701A` is the CRT `atan2` inside `00414EB0`'s kernel and inside the two inline copies at
`009E3127` and `009D975A`.

## Coverage

| routine | coverage |
| --- | --- |
| `009EC680` | complete |
| `009EC280` | complete |
| `009E3330` | complete |
| `009E3040` | complete |
| `009D5A20` | complete |
| `009D9550` | complete |
| `009D96A0` | complete |
| `009D92E0` | complete |
| `009D58F0` | complete |
| `009D5930` | complete, read but not named: it belongs to `ship_ai_path_follower` |
| `009D9150`, `009D9230` | referenced, already reconstructed by `ship_ai_path_planner` |
| `00417E90` | complete, `00417E90-00417EED`. Its callee `004179D0` is unread, so which zone wins when a segment crosses several is not established |
| `00417610` | complete, `00417610-00417623` |
| `0071C4F0` | complete, `0071C4F0-0071C547` |
| `00422500` | partial: `00422500-00422580` only, for the argument use, the corner-record array and count, the hit-edge indexing and the margin floor. `00422580-0042318F` unread |
| `00423190` | partial: `00423190-004231EF` and `004234BC-004234D5` only, for the `+20h` guard, the manager's critical section, the `+10h`/`+14h` reads and the `+20h` write. `004231F0-004234BB` unread |
| `009ED3E0` | partial: `009ED4E4-009ED683` only, for the two `009EC680` call sites and the swap. The rest is `ship_ai_order_consumer`'s |
| `009E3C00`, `009D9E50` | partial: the `plan+34h` reads at `009E3C6C` and `009D9E67` only, for the node-cursor question |

Projected as C++: `009EC680`, `009EC280`, `009E3330`, `009E3040`, `009D5A20`, `009D9550`,
`009D96A0`, `009D92E0`, `009D58F0` and `009D5930`'s rule. The rest are host contracts or
references.

## Corrections

| was | is | evidence |
| --- | --- | --- |
| the packet brief lists `009D9050` as one of the search routines | there is no routine there. `009D9050` is in the middle of `009D8CE0 BSP_ShipAi_PredictTrackCrossing`'s body, `009D8CE0-009D9141`, and nothing in `009EC680`'s call tree reaches it | `bsp.py lookup 009d9050` reports the enclosing candidate; `009D9050 MOVSS dword ptr [ESP+48h],XMM0` is mid-expression |
| `docs/SHIP_AI_PATH_PLANNER.md` calls `009EC680`'s state-1 case "the search" as one step | it is three: `009EC280` costs the graph, `009E3330` splits one edge, `009D5A20` tests for completion. Only the middle one changes the graph | `009EC6E2`, `009EC6F6`, `009EC6FE` |
| the same doc reads `plan+2Ch` as "the search context" and `include/bsp/ship_ai_path_planner.hpp` types it `std::uint32_t` | it is a float, the penalty a route pays when consecutive corners sit on opposite sides of their zones. It is zero in the image | `009EC6D8 FLD float ptr [ESI+2Ch]`; `009EC4D2`, `009EC568`, `009EC613`, `009EC62D` add it |
| the same header names node `+31h` `short_link`, "set when `link_minus_length` is below one unit" | that is only its seed value. Its meaning is "the `+20h` edge has been proved clear": `009E3573` sets it when the probe found no zone, `009D98AF` sets it on a spliced link, `009D95CE` clears it, and `009D5A20` and `009E3330` are the readers. `+32h` is the same flag for `+24h` | `009E3573`, `009E3764`, `009D98AF`, `009D95CE`, `009D9568`, `009D5A38` |
| the same header types node `+2Ch` `std::uint32_t field_2c` with no meaning | it is the back pointer of `link_plus`, exactly what `+28h` is for `link_minus` | `009D92F8 MOV dword ptr [EAX+2Ch],ESI` inside `009D92E0`, against `009D9248 MOV dword ptr [EAX+28h],ESI` inside `009D9230`; `009E34C5` and `009E36CD` read the pair symmetrically |
| the same header types node `+14h` `std::uint8_t field_14` | the byte is signed everywhere it is read | `009D590B CMP byte ptr [ESI+14h],DL` with `009D5911 SETL`; `009D985B`/`009D986D` with `JG`/`JL`; `009EC4B1 MOVSX` |
| the planner doc's follow-up row asks what `009EC680`'s `+14h` countdown gates | nothing in the search: `009EC683-009EC68F` is the only touch, and the reader is `009E3A44` inside `009E3780` | the body has no other reference to `+14h` |
| the decompiler renders `009E3330`'s splice calls as `BSP_ShipAiPathNode_LinkNext(...)` with no `this` | every one of them is a `__thiscall` whose `ECX` is set on the preceding instruction, and half of them are `009D92E0`, the `+24h` mirror, not `009D9230` | `009E3488 MOV ECX,EDI` before `009E348A`; `009E3500 MOV ECX,EDI` before `009E3502 CALL 009D92E0` |
| the decompiler renders `009E3040` with `param_5` as a `float*` and loses the by-value point | the first argument of `00422500` is the far endpoint as two floats on the stack, pushed by `009E30F0 SUB ESP,8` and the two `FSTP`s that follow | `009E30F3`, `009E30F5`, `009E30FE`; `RET 28h` accounts for 40 bytes |
| the decompiler shows `009E3040` calling `004218E0` with no result | `009E3073 MOV ECX,EAX` makes the singleton the `this` of `00417E90` | `009E3058`, `009E3073` |

## Deviations in the reconstruction

Three places where the image faults and the reconstruction stops instead. All three need
`operator new` to fail, which the image never checks on the paths that matter.

- `009E328D` and `009E32E1` write the zone into a candidate the allocator refused.
- `009E34EC` and `009E36F4` call `009D9230`/`009D92E0` with a null `this` when the far-node copy
  failed, and `009E3550`/`009E3742` do the same for the node copy.
- every working case of `009EC680` hands `plan+20h` to a `__thiscall`, so a null head faults.

The fourth is a guard, not an image behaviour: the descent in `009E3330` steps to
`node->+20h` or `node->+24h` after the direction is decided, and a decided direction always has its
link, so the image never reads through a null there. The reconstruction ends the pass instead.

## no_ghidra_function

none. Every routine read for this packet starts a Ghidra function: `009EC680`
(`009EC680-009EC74C`), `009EC280` (`009EC280-009EC67E`), `009E3330` (`009E3330-009E3771`),
`009E3040` (`009E3040-009E3327`), `009D5A20` (`009D5A20-009D5A7A`), `009D9550`
(`009D9550-009D9690`), `009D96A0` (`009D96A0-009D98BB`), `009D92E0` (`009D92E0-009D9380`),
`009D58F0` (`009D58F0-009D5929`), `009D5930` (`009D5930-009D5987`), `00417E90`
(`00417E90-00417EED`), `00417610` (`00417610-00417623`), `0071C4F0` (`0071C4F0-0071C547`),
`00422500` (`00422500-0042318F`) and `00423190` (`00423190-004234F8`).

## Follow-up packets

| packet | addresses | what is left |
| --- | --- | --- |
| `avoid_zone_geometry` | `004179D0`, `00422500`, `0041AEA0`, `00416B50`, `00416F30`, `00414F50`, `00419AB0`, `004F4B50`, `00419260`, `004120D0`, `00412120` | The polygon itself. `004179D0` is the segment-versus-zone test the whole search rests on, and `00422500-0042318F` is the tangent walk that picks the two corners: which corner of a polygon each side code names, and what `00417E90`'s second out value indexes, are only inferred here from the caller |
| `avoid_zone_corner_record` | `00423190`, `00417610` | What `record+20h` holds after `004234BC`, why the computation is guarded on its own previous value, and what `record+10h`/`+14h` are, which is the pair `009E3D8C` offsets the published path point along |
| `ship_ai_path_follower` | `009E3C00`, `009D5930`, `009D6550`, `009D9E50` | Unchanged, plus one question this packet could not answer: what advances `plan+34h`. Nothing in the search does, and both walks read it as a cursor |
| `ship_ai_owner_seed` | `009E3980+vtable50` | Unchanged, and now with a consumer: `009EC280` compares node `+48h` against the bearing to each child, so the slot returns a heading in the `00414EB0` convention |
| `game_settings_turn_ramp` | `settings+6F0h`, `+6F4h`, `+6F8h` | The three floats `009EC310`, `009EC323` and `009EC336` read. Their producer is the settings loader, not read here, so the ramp's units are inferred from `00419010`'s x being an absolute angle |
