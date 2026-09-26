# Who reads what the scoring object's per-slot passes rebuild (packet `cc9_side_ai_scheduler`)

Addresses: 00911E80, 00912A60, 00914390, 00914270; read only 00914EF0, 0090EDE0, 0091C650,
00593CA0, 00593570, 009150A0, 00920A20, 0060E440, 0062A0B0, 0062C2C0, 007556A0, 008B8FF0.

Worker cc9-side-ai, 2026-09-26, from `docs/SIDE_AI_SCHEDULER_HANDOFF.md` (main a016cf264).
Ghidra was read only. Scans are over the disk bytes of this installation's
`battlestationspacific.exe`. Names are hypotheses unless stated otherwise.

## 1. Answer: score-keeping only, so nothing was bound

**No reader turns the rebuilt trees or the totals into a unit order.** The per-slot records
at `[game+21A0h]+4+slot*284h` are the `MissionScoreRecord`s of
`include/bsp/mission_progress.hpp`, one per player slot. They are not "side AI" records.
The three passes evaluate the player's badges and achievements and recompute the score
totals that the debrief shows:

| routine | was | reading now | output |
| --- | --- | --- | --- |
| 00911E80 | `RebuildUnitDemands` | `BSP_MissionScoreRecord_EvaluateBadgeCounters` | counter tree `+78h` (head `+7Ch`, count `+80h`); grants at 009129EE |
| 00912A60 | `RebuildGoalRequests` | `BSP_MissionScoreRecord_EvaluateAchievementCounters` | counter tree `+84h` (head `+88h`, count `+8Ch`); grants at 009133C9, 009139C4 |
| 00914390 | `RecomputeAssetScores` | `BSP_MissionScoreRecord_RecomputeCategoryTotals` | `totals_1c8[0..5]` and the total `+1E0h` |
| 00914270 | unnamed | `BSP_AwardRegistry_GetScoreForName` | `[0050FC30(name)]+3Ch`, the `Score` column |

The ledger carries these names as provisional (`config/names/00910000.jsonl`). The old names
stay in git history. The Ghidra names are for the lead to apply.

The tokens settle it. `BU_DM`, `BO_CV`, `RUA_CU`, `GA_SH` and the rest are rows of this
installation's `scripts/datatables/achievements.lua` (2024-07-13). Examples:
- `Achievements["BU_DM"]` has `Params[1] = 10` and `Score = 100`.
- `Achievements["BO_CV"]` has `Params[1] = 20`, "issue at least 20 successful orders for dive,
  level and torpedo bombers".
- `ACHIEVEMENT_RUA_CU = 28` is "Broadside Kill".

The threshold `[0050FC30(key)]+48h` is read against these rows. 0050FC30 is the registry's
`operator[]` (`docs/AWARD_GRANT.md`). That `+48h` is the first `Params` element is inference:
the AWARD_GRANT value table stops at `+40h`.

The totals follow the record's own field order. Each sum takes `node+14h` over one score map:

| total | source (decompiler index of the head) |
| --- | --- |
| `+1C8h` mission | map `+18h` (`[7]`) |
| `+1CCh` action | map `+24h` (`[10]`), plus 00911CE0 over the damage trees `+9Ch`/`+A8h` and 00911C00 over the kill trees `+B4h`/`+C0h` |
| `+1D0h` ship | map `+30h` (`[0xd]`) |
| `+1D4h` plane | map `+3Ch` (`[0x10]`), plus `00910570(class) * count` over `+D8h` (`[0x37]`) |
| `+1D8h` command | map `+48h` (`[0x13]`) |
| `+1DCh` badge | `max(00914270(name), 0)` over the badge map `+6Ch` (`[0x1c]`) |

The debrief's scoring page 0062C2C0 calls the same three kernels 00910570, 00911C00 and
00911CE0.

## 2. The reader census

**Method.** Two passes found every place that reaches a record:
- **Index sites.** Every instruction with a `284h` immediate in `.text`: 103 IMUL, ADD and LEA
  sites in 58 functions. Some of them index other `284h`-stride arrays.
- **The accessor.** Every call to the commit-record accessor 004B4750, 11 rel32 sites.

Each function's whole body was then swept for the displacements of the two trees
(`78h..90h`), the flags (`188h`, `18Ch`) and the totals (`1C8h..1E4h`). Hits were checked by
reading each base register.

| field | readers outside the three passes | kind |
| --- | --- | --- |
| tree `+78h`, tree `+84h` | 00593CA0 `BSP_MissionScoreRecord_Assign` (00593D6C, 00593D7D), the destructor 00593570, the constructor 0091CE90 | record copy and lifetime |
| same | 0091C650, the network receive (callers 00777CBD in 00777850, and 0076D224 in an unfunctioned body) | a writer: its string cases index the record's keyed maps; case 14h grants through 0090EDE0 |
| totals `+1C8h..+1E0h` | 008B8FF0 `Scoring_GetTotalMissionScore` (008B912A) | Lua |
| same | 0060E440 ("Player_point_Text"), 0062A0B0, 0062C2C0 | debrief and scoreboard pages |
| same | 007556A0 `METRICS_wrapper` (00755EC4..007560AE) | telemetry |
| same | 0091C650 case 12h | receive side of the 15h message: stores the sender's `+1E0h` |
| flags `+188h`/`+18Ch` | 00593CA0 copy only | the `[EDI+188h]` hits in 0076D1B0 and 00915F20 are on other objects (EDI is `ECX` at 0076D1B7 and `[EBP+8]` at 00915F5F) |

**Nothing else.** In particular:
- The four false positives were checked and dropped:
  - 0062C2C0's `[EDX+88h]` is a vtable slot after `CALL 00AA7E00`.
  - 0060E440's `+80h`/`+84h` reads are player objects at `game+18CCh`.
  - 005E5B30's `+284h` is a local object.
  - 0091C650's `+78h` is the message.
- The party AI, air operations, attack-move and HUD order paths have no edge to any record
  field above.
- The badge map `+6Ch` is read by 0090C740 (one caller, 009163CE) and by the grant itself.

**The one gameplay reader is Lua in competitive multiplayer.** Every call to
`Scoring_GetTotalMissionScore` in this installation is in `scripts/global/commandhelpers.lua`
or `scripts/missions/multi/competitive*.lua`:
- In `luaObj_DoScoring` (from line 6309), they sit under `LobbySettings.GameMode ==
  "globals.gamemode_competitive"`. There the point limit picks the winner and ends the mission.
- In `luaMissionCompletedNew` (from line 10404), they sit under `Mission.MultiplayerType ==
  "Competitive"`, after the mission is over.

The host runs mode 0. The native logs of the gate-ON pairs show no `Scoring_*` native
called after stage init except `Scoring_SetFinalScoringFunctionName` and
`Scoring_RealPlayTimeRunning` (`local\bs_on_usn04.log`, `local\bs_on_e2.log` in the
cc9-platform2 tree).

## 3. What the passes would need, and why they stay host records

Binding would take all of the following. It is not cheap:
- **A per-slot record store.** It needs about 15 trees: the seven score maps, the badge map
  `+6Ch`, the four damage and kill trees, `+D8h`, `+120h` and `+15Ch`.
- **The feed.** The kill credit binding in `src/game_hosts_gunnery.cpp` (leased by
  cc9-gunnery2) runs 0091BDA0 into throwaway locals: `kill_tree_leaf` returns one shared int,
  and `add_named_counter` and `grant_award` only count. The damage trees have no host writer.
- **The bodies.** About 10 KB of checked-iterator tree walks: 00911E80, 00912A60 and
  00914390. They need the kernels 00910570, 00911C00, 00911CE0 and 00914270, and 0090D220's
  recursion into 00907F40, 00907F80, 009085D0 and 0090C430.
- **The grant.** 0090EDE0 ends in `BSP_AwardTracker_RecordAtLeast`, a write to the local
  profile. A host must keep that as a record so a test run cannot touch a save.

The only visible effect in single-player would be badge and achievement rows and the
debrief total. No per-entity row, gunnery line or order can move (section 2). By the lead's
rule, "bind it anyway if it is cheap", the three records stay host records. **No switch was
added and no pair was run.** The predictions for a binding would be: every gameplay row
identical; `think_a`/`think_b` concrete at 360/720; award rows only from BU_*/BO_* counters
the kill feed reaches.

## 4. Corrections

- **"Side AI" and "demand/goal lists".** `docs/SIDE_AI_SCHEDULER_HANDOFF.md` sections 1-3 and
  `docs/BOT_SCHEDULER_OUTPUT.md` call these lists "demand" and "goal request" lists, and call
  the records "per-side". They are achievement and badge counters in per-player-slot score
  records. The `BU_`/`BO_`/`RUA_`/`GA_` prefixes are achievements.lua row keys, not
  build/order demands.
- **"Asset scores" and "asset lists".** The "asset lists" of 00914390 are the record's score
  maps and damage/kill trees (section 1 table). The "six category sums" are
  `totals_1c8`: mission, action, ship, plane, command, badge.
- **List heads.** The handoff's "list at `+78h` (count `+80h`)" and "`+88h` (count `+8Ch`)"
  name two trees. Tree one has its object at `+78h` and head at `+7Ch`. Tree two has its
  object at `+84h` and head at `+88h`. The destructor table in
  `docs/NATIVE_MISSION_SCORE_RECORD_R108.md` has both.
- **Open question 3 answered.** The receive side of message 15h code 12h is 0091C650 case
  12h, `record[slot]+1E0h = message+50h`. It syncs the total between machines.
- **Extra callers.** 00914390 and 00912A60 also run at mission end from the debrief bring-up
  (00920D79, 00920D80). 00914390 also runs from `Scoring_ForceRefreshScoringTable`
  (008B8E60 -> 009150A0, 009150CC).

## 5. no_ghidra_function

One body, for the lead to define:

| start | end (inclusive) | contents |
| --- | --- | --- |
| 0076D220 | 0076D234 | `MOV ECX,[ESP+4]; CALL 0091C650; MOV EAX,[00F8A2FC]; MOV byte [EAX+74h],0; RET 8` |

INT3 padding bounds it at 0076D21C and 0076D235. `ghidra proto --brief` resolves no function
at 0076D220 or 0076D224. No rel32 and no absolute reference to 0076D220 exists in any section,
so its caller is unknown. Every other site cited here resolved to a Ghidra function.
