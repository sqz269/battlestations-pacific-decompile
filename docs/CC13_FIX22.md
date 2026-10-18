# CC13_FIX22: planner fixes for CC13_PLANNER_VERIFY R-A, R-B, R-C and R-E

Packet cc13-fix22. It covers the REAL findings in `docs/CC13_PLANNER_VERIFY.md` section 2. Each fix is behind
its own new switch in `src/game_hosts_ai.cpp`, next to `kAiPartyReplanFlagBound`. **All four are OFF.** With a
switch OFF the host behaves exactly as it did before; the only change is that the code now goes through two
named helpers. Each item was re-read against the image before coding. Nothing has been run: this packet made
no game runs and flipped no switches. Status: build-tested only.

| Switch | Finding | Missions (mode 8 rows to pair) | RNG |
|---|---|---|---|
| `kAiSellSplitClaimBound` | R-A | campaign rows where a Sell-owned group splits off air members | none |
| `kAiHandOffReplanBound` | R-B | campaign rows where Capture or Defend hands a group on | none |
| `kAiTargetObjectiveSetBound` | R-C | Attack think rows (00A1CF90); also Duel/Escort through 00A1CB80 | none |
| `kAiPlannerSharedStreamDrawBound` | R-E | rows with cautious moves or order_attack cautious rolls | adds shared stream-1 draws |

### Which rows reach these paths

This doc cites no mission script file. Rows are named by the mission ID that runs, not by script file names
(a menu ID does not load the file it resembles; the scene's StageScript decides).
- **Every mode-8 row with a party brain builds all four base planners.** 00A15A70 builds Defend, Attack, Sell
  and Capture outside modes 4..7 (`docs/CC13_PLANNER_VERIFY.md` section 4). The party brain runs on all nine
  reference rows (`docs/SHIP_AI_OPEN_ITEMS.md` rank 5 and rank 8: E2, USN02, USN04 and the others). So Sell and
  Capture planners exist on every one of those rows.
- **Capture hand-offs (R-B) are observed on:**
  - JM05: `handoffs=9`. Capture hands groups to brain+8h because all three CommandBuildings are Allied
    (SHIP_AI_OPEN_ITEMS, the Sell/returntobase table).
  - USNOS: `handoffs=21` with `kCaptureGroupValueBound` ON (SHIP_AI_OPEN_ITEMS, the cgv question).

  Defend hand-offs to brain+0Ch have no published per-row count.
- **Sell splits (R-A)** need a Sell-owned group that holds air and non-air members. CC13_PLANNER_VERIFY R-A
  calls JM05 probably inert, because its carrier groups hold no squadrons. No row has a published
  `sell_splits` count, so R-A's reach is unmapped until a pair runs.
- **R-C** reaches every Attack think, but changes a score only when an objective set holds a member of an enemy
  group.
- **R-E** reaches every row that issues a cautious move or an order_attack past the gates.

## R-A, `kAiSellSplitClaimBound`: Sell think split claim

Image 00A22800: `00A22841 CALL 00A2E4C0`, `00A22846 TEST EAX,EAX / JZ`, then `00A2284D CALL 00A22750` with
ECX = the planner (00A2284B), on every non-null split. 00A22750 (read in full, 00A22750-00A227B0, RET 4):
1. `00A2275A CALL 00A2C600` on the group. The answer is discarded, and the function is a pure member walk.
2. `00A22762 CALL 00A1C8B0`, then `JNZ 00A227AE` returns early when the planner already owns the group.
3. The push: `00A2277F CALL 00A16AB0` and `00A2278A CALL 00A172A0`.
4. `00A2279D MOV [group+5654h],planner`.
5. `00A227A3 CALL 00694A60`.
6. `00A227A9 MOV byte [planner+2Ch],1`.

The host think (`sell_think_00a22800`) linked the group itself and never set `replan_002c`. ON: the split
group goes through `planner_claim_group`, the host's existing 00A22750 model. That model sets
`replan_002c` (gated by `kAiPartyReplanFlagBound`, which is ON) and counts `replan_sets`, `planner_claims`
and the party's `groups_claimed`. The effect is think timing: in mode 8, 00A15970 ORs the four 00A18480
answers, so the party thinks again on the next call.

## R-B, `kAiHandOffReplanBound`: Capture and Defend hand-offs

The claim is inlined at four sites. Each has the same shape: 00A2C600 (discarded), 00A1C8B0 on the receiver
with an early skip, the push (00A16AB0 / 00A172A0), `MOV [group+5654h],receiver`, `CALL 00694A60`, and
`MOV byte [receiver+2Ch],1`.

| Think | Receiver | 00A1C8B0 | +5654h | 00694A60 | +2Ch |
|---|---|---|---|---|---|
| Capture 00A29FD0 | brain+4h (`00A2B1D2`) | 00A2B1E3 | 00A2B220 | 00A2B226 | 00A2B22F |
| Capture 00A29FD0 | brain+8h (`00A2B2B2`) | 00A2B2C3 | (same shape) | (same shape) | 00A2B30F |
| Defend 00A28A60 | brain+0Ch (`00A28F38`) | 00A28F49 | 00A28F82 | 00A28F88 | 00A28F95 |
| Defend 00A28A60 | brain+0Ch (`00A297D1`) | 00A297E2 | 00A2981F | 00A29825 | 00A29832 |

The host models all four through `capture_hand_off`. Its link (`claimed_by` standing for +5654h and the observer
pair, and the push) already matched. Only the byte was missing. ON: after the release, `capture_hand_off` asks
`planner_owns_group_00a1c8b0(to, g)`. When that answer is false, it counts `replan_sets` and sets
`to.replan_002c` (gated by `kAiPartyReplanFlagBound`). This adds the two Defend sites, which the verify doc
did not list.

Remaining minor (unchanged): the image overwrites +5654h even when a third planner holds the group. The host
still refuses that link, as `planner_claim_group` does.

## R-C, `kAiTargetObjectiveSetBound`: objective-target multiplier in 00A1CB80

At 00A1CD9E the image loads the planner, then `00A1CDA2 MOV EAX,[EDX+1Ch]` (its brain) and
`00A1CDA5 MOV EAX,[EAX+24h]` (the brain's set index). It pushes that index and calls `00A1CDAB CALL 00A2C450`
with ECX = the candidate. On true, `00A1CDB4 CALL 00A371A0; 00A1CDB9 MOVSS XMM0,[EAX+1CCh]` stores
FreeAttack_ObjectiveTargetMul into the objective factor.

00A371A0 (read in full, eight instructions) is `BSP_Ai_ActiveModeTuning`: 009FFC80 (mode index), then
004C1C50, and it returns `coordinator + 4 + mode*23Ch`. It has no other effect. The model already reads that
field through `host.tuning_field(kAiPlannerTuningOwnSetFactor)` (0x1CC), so 00A371A0 adds nothing to model.

The host hook answered `g->team == own_team`, which is always false for enemy-team candidates. ON: it answers
`group_has_member_in_world_set(candidate, brain->world_set)`, the host's 00A2C450, using the current party's
brain. The planner thinks run inside that brain's think. With no brain the hook answers false; the image
always has one. Reach: every 00A1CB80 call, which means the Attack think 00A1CF90 in mode 8 and the Duel/Escort
`ChooseAttackTarget` arms. Whether anything changes depends on the objective sets being populated
(`game_objective_sets()`). An empty set answers false exactly as before.

00A2C450 now has a reconstruction record (`group_has_member_in_world_set`). Its uncertainty: the host tests
`proxy(member)`, so a squadron answers as its flight leader, while the image passes the member entity.

## R-E, `kAiPlannerSharedStreamDrawBound`: the planners' stream-1 draws

The image draws at two sites, both `MOV ECX,1; CALL 00BD2F40` (BSP_RandomThreads_DrawUnsignedFraction,
stream 1):
- 00A2CCF0 OrderCautiousMove at `00A2CD42`. Then `00A2CD52 FCOMIP / JBE`: the move is cautious only when
  draw > ratio and the compare is ordered. The model's compare already matched.
- 00A2CBD0 order_attack at `00A2CC51` (verified: it follows `00A2CC48 CALL 00A2C9F0 / 00A2CC4F JZ`). It is the
  same stream, so it is covered by the same switch. Both of the host's order_attack arms (with and without
  `kOrderAttackCautiousGatesBound`) use the helper.

Search result: the host has no RandomThreads model for gameplay. `bsp::RandomThreads` exists
(`include/bsp/random_threads.hpp`), but only the particle path uses it, through
`native_particle_unit_random_00bd2f40`. The gameplay hosts share one stream-1 generator owned by the gunnery
host (`GameGunneryHost::ship_ai_draw`, `auto_target_phase_draw_009f6a8f`; seeded by
`reseed_shared_stream_00bd2fd0`). That generator is itself an LCG stand-in for the image's MT19937. ON: the new
helper `planner_stream1_fraction_00bd2f40` draws `ship_ai_draw(leader, 0, 1)`. The coupling and the draw order
are the image's; the values are the stand-in's.

A faithful MT19937 stream 1 would have to replace the gunnery host's generator. That is outside this grant
(game_hosts_gunnery.cpp), so it is not attempted. Under the measurement option `BSP_GUNNERY_RNG_STREAMS=1` the
draw is keyed by the leader unit and shares the ship-AI torpedo key. With no gunnery host, the private LCG
answers and the miss is `record`ed. The counter `planner_shared_stream_draws` counts the shared draws.

**Shared-stream draws this adds (RNG coupling):**
1. One per OrderCautiousMove that reaches 00A2CD42, which means past the ship-leader and route-cautiously
   tests (Duel's no-target and centroid moves).
2. One per order_attack that reaches 00A2CC51: a non-ship leader with 00A2C9F0 true, or every call when
   `kOrderAttackCautiousGatesBound` is off.

Each draw advances the shared stream-1 generator, so every later stream-1 consumer moves too (gunnery fire
stagger, death modes, ship-AI timers and torpedoes, auto-target phase). It also takes these draws off the AI
host's private LCG. The private LCG's other consumers (the 00BD2F10 draws at `random_00bd2f10`'s remaining
call sites) therefore see a different sequence. Judge an ON pair only on per-entity tables, not on aggregates.

## Minor recorded only: near_radius

The host passes `near_radius_squared()` = 9e6 (00D22CB8) into `ai_target_scoring_distance`. The image
compares against the double 1e-10 at 00CE3820 (bytes `BB BD D7 D9 DF 7C DB 3D`). There is no effect with the
shipped tuning (CC13_PLANNER_VERIFY section 3), and no code was changed.

## Records added

- 00A2C450 `group_has_member_in_world_set`
- 00A1C8B0 `planner_owns_group_00a1c8b0` (new helper)
- 00A2C4C0 `group_total_leader_weight_00a2c4c0` (existing host, verified)
- 00A1CF10 `defend_anchor_00a1cf10` (existing host, verified; the 00414DB0 pose refresh is not called)

Skipped: 00A07E40 BSP_AiGroup_BuildEntityRecords. Verifying it means verifying the 00A04560 record build at
two host sites (`src/cc13_seg69.cpp`, `src/game_hosts_ai.cpp`). That is outside this packet's size.
