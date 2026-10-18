# CC13_PLANNER_VERIFY: blind check of the skirmish / strategic planner

Worker cc13-planner1 did this verify-only pass for the cc13 lead on 2026-10-17, against base `fcb6c6fa5`.
It covers the AI party brain and its eight planners (`00A15970`-`00A2CCF0`): Defend, Attack, Sell and
Capture for modes other than 4..7, and Duel, Escort, Siege and Competitive for effective game modes 4..7.
No source was changed and no ledger record was edited. The host files are cc9's and are read-only here.
"Image" means battlestationspacific.exe, read with `bsp.py show --asm`, `disasm-raw` and `ghidra bytes`.
Line numbers are against the base. Every descriptive name is a hypothesis.

The grading was done blind first. Afterwards the findings were checked against CC13_VERIFY13 (the earlier
planner-host pass), CC13_GPR_69 and SHIP_AI_OPEN_ITEMS. Overlaps are marked "known", and the doc keeps them
because the code is unchanged.

## 0. Reach: the mode planners cannot run in this process

`game_mode()` (`src/game_hosts_ai.cpp:3378-3380`) returns `kSinglePlayerEffectiveGameMode` = 8 (`:1089`)
or 0. Neither value is in 4..7. As a result:
- `ai_party_brain_think_00a181a0` (`src/ai_group_think.cpp:304`) never takes its mode arm.
- Duel, Escort, Siege and Competitive are never ticked.
- `kAiDuelEscortThinkBound` (`include/bsp/game_hosts_ai.hpp:161`) switches code that cannot be reached. Its
  pair being identical is expected for that reason, and it tells us nothing about the models.

The image picks the planner set in 00A15A70, from `BSP_Game_GetEffectiveGameMode`. That makes the
skirmish planners unvalidated at run time, and the host as it stands cannot validate them. The base four
planners do run in mode 8 (for example, the Sell think on JM05, SHIP_AI_OPEN_ITEMS section 59).

## 1. Results

| # | Address | Function | Host | Grade |
|---|---|---|---|---|
| 1 | 00A15A70 | PartyBrain_Construct, the planner-set gate | `ai_planner_slot_constructed` `src/ai_planners.cpp:46` | FAITHFUL |
| 2 | 00A15970 | PartyBrain_WantsImmediateThink | `brain_wants_immediate_think` `src/game_hosts_ai.cpp:3681` | REAL (modes 4..7, known) |
| 3 | 00A181A0 | PartyBrain_Think | `ai_party_brain_think_00a181a0` `src/ai_group_think.cpp:304` | FAITHFUL |
| 4 | 00A1D110 (+1D140/1D170/1D1A0) | mode planners' NeedsReplan | `ai_planner_needs_replan_mode` `src/ai_planners.cpp:123` | FAITHFUL as a model, never called (see 2) |
| 5 | 00A1CB80 | ChooseAttackTarget | `ai_planner_choose_attack_target` `src/ai_planners.cpp:209` plus hooks | REAL (known R1) |
| 6 | 00A25F70 | DuelThink, decision arms | `ai_duel_think_00a25f70` `src/gameplay_seg69_70_cc13.cpp:1078` | FAITHFUL |
| 7 | 00A26210 | EscortThink, decision arms | `ai_escort_think_00a26210` `src/gameplay_seg69_70_cc13.cpp:1110` | MINOR |
| 8 | 00A26510 | SiegeThink, decision arm | `ai_mode_planner_tick` `src/ai_planners.cpp:264` | FAITHFUL |
| 9 | 00A265F0 | CompetitiveThink, decision arm | `ai_mode_planner_tick` `src/ai_planners.cpp:264` | MINOR |
| 10 | 00A25B90 / 00A16450 | the spawn arm shared by 6-9: QuickSpawn, then ScheduleThinkWithin(0.5) | `quick_spawn_named_group` / `publish_spawn_bias` `src/game_hosts_ai.cpp:5848-5859` | PLACEHOLDER (known R3) |
| 11 | 00A1C960 | NearestGroupOfTeam | `ai_nearest_group_00a1c960` `src/band_009f_cc13.cpp:125` | MINOR |
| 12 | 00A1CA60 | TeamWeightedCentroid | `ai_team_weighted_centroid_00a1ca60` `src/band_009f_cc13.cpp:147` | FAITHFUL |
| 13 | 00A2CCF0 | OrderCautiousMove (the planner's RNG draw) | `ai_order_cautious_move_00a2ccf0` plus `order_cautious_move_00a2ccf0` `src/game_hosts_ai.cpp:3916` | REAL (RNG, known) |
| 14 | 00A1D010 | TryMergeNearbyGroups | `capture_merge_00a1d010` `src/game_hosts_ai.cpp:4621` | MINOR |
| 15 | 00A22800 | SellThink | `sell_think_00a22800` `src/game_hosts_ai.cpp:4802` | REAL (new) |
| 16 | 00A29FD0 hand-off sub-path (00A2B1E3-00A2B22F, 00A2B2C3-00A2B30F) | Capture think hand-off claim | `capture_hand_off` `src/game_hosts_ai.cpp:4648` | REAL (new) |

Counts: FAITHFUL 6 (1, 3, 4, 6, 8, 12), MINOR 4 (7, 9, 11, 14), REAL 5 (2, 5, 13, 15, 16), PLACEHOLDER 1 (10).
The QuickSpawn and ScheduleThinkWithin models themselves (`planner_quick_spawn_00a25b90`
`src/cc13_seg70.cpp:395`, `ai_party_schedule_think_within_00a16450` `src/band_009f_cc13.cpp:283`) were
checked and are FAITHFUL. The PLACEHOLDER grade is for the host, which never calls them.

## 2. REAL findings

### R-A (item 15, new). The Sell think's split claim does not set the replan byte

Image 00A22800: for each owned group it calls 00A2E4C0 (`00A22841`). A non-null split-off group is then
claimed through `00A2284D CALL 00A22750` with ECX = the Sell planner. 00A22750 pushes the group, sets
`group+5654h`, registers the observer pair (00694A60), and sets `planner+2Ch = 1` (`00A227A9`).

Host `src/game_hosts_ai.cpp:4807-4813` does not call `planner_claim_group` (`:5439`). It sets
`made->claimed_by = &p` and pushes the group itself, so `replan_002c` is never set. In mode 8, 00A15970
ORs the four base planners' 00A18480 answers. The image therefore makes the party think again at once after
a split; the host waits for the scheduled think. The effect is think timing only.

Reach: mode 8 (campaign) whenever a Sell-owned group splits off air members. On JM05 the carrier groups hold
no squadrons (SHIP_AI_OPEN_ITEMS 59.1), so it is probably inert there. No RNG.

### R-B (item 16, new). The Capture hand-off does not set the replan byte

Image 00A29FD0 inlines the claim at both hand-off sites. The first is `00A2B1E3 CALL 00A1C8B0` (OwnsGroup),
then the push (`00A2B1FE` / `00A2B20B`), `00A2B220 MOV [ESI+5654h],EDI`, `00A2B226 CALL 00694A60` and
`00A2B22F MOV byte [EDI+2Ch],1`. The second is the same at `00A2B2C3`, ending `00A2B30F MOV byte [EDI+2Ch],1`.

Host `capture_hand_off` (`src/game_hosts_ai.cpp:4648-4660`) sets `claimed_by` and pushes the group, but never
sets `to.replan_002c`. The effect is the same as R-A: no immediate think after a hand-off. Reach: mode 8,
whenever the Capture think hands a group to the next planner. No RNG.

### R-C (item 5, known as CC13_VERIFY13 R1, still open). The objective-target multiplier is never applied

`ai_planner_choose_attack_target` asks `host.candidate_has_member_in_own_set(node, own_team)`
(`src/ai_planners.cpp:244`). The host answers `g->team == own_team` (`src/game_hosts_ai.cpp:5845`), and
the candidates are enemy-team groups, so the answer is always false. The image, at 00A1CDA2-00A1CDAB, calls
00A2C450(candidate, `[brain+24h]`) instead. That is the objective world-set test, which the host already
has as `group_has_member_in_world_set` (`:5391`). On true, 00A1CDB4 loads tuning+1CCh,
FreeAttack_ObjectiveTargetMul (2.0). Every enemy group with a member in the party's objective set loses
its 2x weight, which can change the chosen target. Reach: the Attack think in mode 8 (00A1CF90), and Escort,
Siege and Competitive in modes 4..7 (unreachable, section 0). No RNG.

### R-D (item 2, known from the CC13_VERIFY13 structural point). Modes 4..7 never ask for an immediate think

Image 00A15970: in modes 4, 5 and 6 it tail-jumps to vtable+30h of brain+10h, +14h and +18h (`00A15996`,
`00A159B4`, `00A159CD`), and mode 7 does the same for brain+1Ch. Those slots are 00A1D110-00A1D1A0, which
clear `+2Ch` (`00A1D118`) and answer true when the planner owns a group whose first group's command is not
engaged (00A2CB10 false, `00A1D138`), and the old byte otherwise.

Host `:3696-3699` returns false with a `record` in modes 4..7. The model `ai_planner_needs_replan_mode` is
correct but has no caller. Unreachable today (section 0), but it is required before modes 4..7 can run.

### R-E (item 13, known from CC13_GPR_69 7.x). The planner's draws use a private LCG

Image 00A2CCF0: `00A2CD42 MOV ECX,1; CALL 00BD2F40`, which is BSP_RandomThreads_DrawUnsignedFraction on
RandomThreads stream 1. It is not 00BD2F10, but it is the same RandomThreads state family.
`00A2CD52 FCOMIP / JBE` then takes CAUTIOUSMOVE when the draw > ratio, and MOVETO when the draw is lower,
equal or unordered. The model's compare matches.

The host draws `random_00bd2f10(0,1)` (`src/game_hosts_ai.cpp:3916`), and so does `order_attack`'s cautious
roll. That is a private LCG (`:1636-1641`), so both the values and the coupling to other stream-1 consumers
differ. Reach: every OrderCautiousMove that gets past the ship and route-cautiously tests (Duel's no-target
and centroid moves), and the matching order_attack roll in mode 8.

## 3. MINOR and PLACEHOLDER detail

- **Item 5, product order and debug arms.** These are minor sub-points of item 5, apart from R-C:
  - 00A1CDE5-00A1CE05 forms `((range*base)*objective)*sticky`, rounding after each multiply.
    `ai_target_score` (`src/ai_planners.cpp:162`) forms `sticky*own_set*base*range`, so the last bit can
    differ when both factors are not 1 (known, CC13_VERIFY13 line 229).
  - The scoring-distance NaN arm: 00A1CD2F `FCOMI` / `JBE` sends an unordered square to distance 0. The
    host's `<=` (`:167`) sends it to sqrt.
  - The host passes `near_radius_squared()` = 9 000 000 (`src/game_hosts_ai.cpp:5814`, 00D22CB8) where the
    image compares against the double 1e-10 at 00CE3820 (bytes `BB BD D7 D9 DF 7C DB 3D`). There is no
    effect with the shipped tuning: under 3000 units both sides clamp the range factor to 1.0, because
    FreeAttack_NearDist is 5000. CC13_VERIFY13 line 227 grades the distance MATCH; that holds for
    `ai_target_scoring_distance` alone, but not for the constant the host passes in.
  - The debug score list (00A2B950 / 00A2BCC0) and the reset arm's `+5624h=0` / `+5628h=-1` stores are not
    performed (`clear_group_target_cache` `:5497` is a no-op). Both are gated on `[00F8A9E0] == 3`, which
    has no writer in the image.
- **Item 7, Escort.** The objective set is walked in host order instead of the 008DA530 tree order (this is
  labelled at `src/game_hosts_ai.cpp:4007`). That matters only for an exact distance tie, since the compare is a strict `best > d2`
  at 00A263B1 / `JBE`. When no entry is under 1e10, the image reads `[0+54h]` (00A263E1), while the model
  returns None (`src/gameplay_seg69_70_cc13.cpp:1129`). Everything else matches: the 0.5 is the double at 00D7A280, the enemy centroid
  is `[planner+34h]` (00A263F1), the attack goes to entity+16Ch (00A26487), and an empty set calls 00A1CB80
  with reset 0 (00A264EB).
- **Item 9, Competitive.** The image pushes a constant `0` for resetTarget (`00A26699 PUSH 0`). Only Siege
  computes `[00F8A9E0] == 3` (`00A265A5` / `00A265B2 SETZ`). `ai_mode_planner_tick` passes
  `host.reset_target_flag()` for both (`src/ai_planners.cpp:277`). This is latent: the host flag is
  constant false, and the global has no writer.
- **Item 10, the spawn arm.** The image calls 00A25B90 with per-kind arguments: Duel `(&"[duel]", 1, 0,
  [E188A8+2030h])`, and Siege and Competitive `(&tag, 0, 1, -1)`. The think then calls 00A16450(ECX =
  `[00E0E344]`, 0.5), which pulls `next_think[party]` down to now+0.5. The host hooks drop the arguments
  and only `record()`. The host keeps `next_think` (`:1222`, `store_next_think_time` `:3674`), so the
  00A16450 half could be performed, but it is not. Known as CC13_VERIFY13 R3.
- **Item 11, NearestGroupOfTeam.** At 00A1C9F2-00A1C9F8 the image does `FMUL` / `FMULP` / `FADDP` on the
  main thread at 24-bit PC, so each product and the sum round to float. The model squares and adds in
  double and rounds once (`src/band_009f_cc13.cpp:136`), which can differ by 1 ulp. The rule
  `best > d || best < 0` and its NaN behaviour (00A1CA06 `JA`, 00A1CA0F `COMISS` / `JBE`) match. The sentinel
  is -1.0 from 00D7A260.
- **Item 14, TryMergeNearbyGroups.** The image also tests `+5648h` (grouping enabled) on both groups
  (`00A1D039`, `00A1D046`). The host drops that test. It is latent: only AIEnableGrouping (00A37733),
  AIMergeGroups and AICreateGroup clear the byte, and the host notes that no installed script calls them
  (`:1031-1033`). The host also adds an `a == b` guard the image lacks; its caller never passes equal
  groups. The 1e6 is the double at 00CE4C08, and the four-member limit and the ship-kind equality match.

## 4. Checks behind the FAITHFUL grades

- **00A15A70.** Mode 4, 5 and 6 build only brain+10h, +14h and +18h; mode 7 builds only +1Ch. Every other mode
  builds Defend, Attack, Sell and Capture (sizes 38h, 38h, 38h and 44h). This matches
  `ai_planner_slot_constructed`.
- **00A181A0.** The mode arms tick one planner and tail-jump to 00A179E0. Otherwise it walks the party's
  groups: for each populated group it calls 00A2E260 (always), skips the group when `+5654h` is set, and
  claims it into brain+0h unless 00A2C5A0 is true and 00A2C450 is false, in which case it goes to brain+0Ch
  (00A18258-00A18273). The four ticks follow at 00A18288-00A182AD.
- **00A25F70.** The nearest enemy is taken over `[planner+34h]` (00A2606B). The attack fires when
  16 000 000 (the double at 00D22FF8) > d2 is ordered (00A2608C `JA`), or when `[planner+30h]` count <= 1
  (00A26099 `JLE`). Otherwise both centroids (own `+30h` at 00A260C3, enemy `+34h` at 00A260D1) go into
  t = 00419010(500 [00CE397C], 0, 2000 [00CFFD60], 1, |leader - own|). The point is own*t + enemy*(1-t),
  each product and sum rounded (00A26136-00A261A5). With no nearest group, it orders a cautious move to the
  own centroid (00A261D1).
- **00A26510.** With `+28h == 0` it spawns. Otherwise it calls 00A1CB80(first owned group, 1.0,
  `[00F8A9E0] == 3`). ScheduleThinkWithin runs only on the spawn arm.
- **00A1CA60.** The three float products are summed in group order and the total weight is added after.
  There are three divisions with no zero check (00A1CB51-00A1CB6C).

## 5. Planner callees with no reconstruction record

This is a census of `bsp.py callees` over the 46 planner functions and their order helpers, with library
shapes excluded. Each of these has host code but no `config/reconstruction` record:

| Address | Name | Called from | Host code |
|---|---|---|---|
| 00A07E40 | BSP_AiGroup_BuildEntityRecords | 00A250A0, 00A0F970 | `src/cc13_seg69.cpp`, `src/game_hosts_ai.cpp` |
| 00A1CF10 | BSP_AiGroup_PickAnchorEntity (tag game_code) | 00A28A60 | `src/game_hosts_ai.cpp` only |
| 00A2C450 | BSP_AiGroup_HasMemberInWorldSet | 00A181A0, 00A1CB80, 00A28A60 | `group_has_member_in_world_set` `src/game_hosts_ai.cpp:5391` |
| 00A2C4C0 | BSP_AiGroup_TotalLeaderWeight | 00A179E0, 00A1CA60 | `group_total_leader_weight_00a2c4c0` `src/game_hosts_ai.cpp:3848` |

The census reads Ghidra's direct-call graph, so the planners' vtable calls (+20h think, +24h release,
+30h needs-replan) are not in it. The bodies at those slots that this pass found all carry records. The only
other unrecorded callee was the STL shape 00A17960.

## 6. Ledger notes (not edited)

No status was found to be clearly wrong. One record is incomplete: 00A1C8B0 BSP_AiPlanner_OwnsGroup has a
name but no reconstruction record, and ClaimGroup and the Capture hand-offs depend on it. The host tests
ownership inline with `std::find` (`:5448`).
