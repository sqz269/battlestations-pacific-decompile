# Gameplay gap ranking

Addresses: 0099AF53, 007BCBE0, 007D681E, 009C8790, 009C5C9B, 009E2020, 009F9E40, 009BEE30, 009BFEE0,
009FADA0, 00959C20, 00A2B400, 00A25B90, 00A25A30, 00758090, 00819880, 00824B60, 009E4DC1, 008A6AC0,
008A47B0, 00890A10, 008A7060, 008B8AD0, 0089B840

Packet `cc9_gameplay_gap_ranking`, worker cc9-ships14, 2026-09-29 (stamped 11:29 UTC). Read-only;
no switch, no Ghidra write.

## Sources

- **Reference m.** The sixteen `local\rb13_*.log` in the cc9-gunnery11 tree, on binary `490D03C8A285`
  (`b234f20ac`).
- **Fresh runs on current main.** A no-flip `pair_export` of `f314ba9d2`, run with the reference
  environment. The logs are `local\s14m_<row>.log` in the cc9-ships14 tree: USN04 4700/4500, USN13,
  JM05, USNOS, IJN01 and LOMP10 at 3200/3000.
  - Deaths on the fresh runs: USN04 46 (m: 43), IJN01 28 (23), JM05 0 (1), USN13 31 (32), USNOS 6 (6).
    Those moves are the post-m switches (reference n's business). They do not change the ranking.
- **The census tool.** `local\s14_gap.py` parses every `UNIMPLEMENTED calls=` row of the host-method
  table and every `summary` line with a refusal-type counter.
  - Reference m has 627 UNIMPLEMENTED rows. After the presentation classes are removed, 378 remain.
  - Current main has 609 rows.

**The UNIMPLEMENTED census is a poor guide by itself.** Most of the high-count rows are counters on
paths the host already models:
- the labelled substitutions `NearFieldProbe::probe`, `ShipAiTorpedoStandoff::torpedo_bot_accuracy`,
  `LeakManager::route_ship_sink_92h` and `Unit::can_release_007bb110`;
- diagnostics such as the attackmove building arm `00836B95`;
- structure, such as `ShipAiOrder::slot_to_order_ring` (section 1 of SHIP_AI_OPEN_ITEMS).

So each entry below was checked against the host's comment at the record site, the row's outcome
counters and, where the summary names one, its blocker.

## Top 15

| # | gap (address) | reach | gameplay consequence | lane | suggested packet |
| --- | --- | --- | --- | --- | --- |
| 1 | **Torpedo-bomber releases.** The queued release-order count `unit+C58h`, spent at `0099AF53` (`BSP_PilotBot_Tick`), has no host raiser. The candidates are `007BCBE0` `BSP_Unit_SetQueuedReleaseOrders` and the property-bag `LEA` at `007D681E` (TORPEDO_RELEASE_ORDERS open item 5). | fresh: USN13 **0 of 60** aircraft, JM05 0 of 12, USN04 4 of 16. m: USN01 0 of 5. | damage and deaths: the carrier missions' air strikes do not drop torpedoes | planes/units (lua15) | census every `+C58h` writer (the store scan plus the `LEA`), then bind the raiser OFF and pair it on USN13 and JM05 |
| 2 | **Dive-bomb releases.** The state walk stalls before the drop. The chain is the in-range latch `approach+D0h` (`009C7C31`), then turndown (`009C7EA0`), then aimdive, then the 25 m aim error at `009C5C9B`. | fresh: USN04 1 of 19 (`bomb_drops=1`), JM05 0 of 6. m: USN04 0 of 19. | damage and deaths | planes/units (lua15) | find which of the three gates each aircraft stops at (the per-aircraft lines), then read that state tick whole |
| 3 | **Kamikaze boat attack step `009E2020`.** `BSP_ShipAi_KamikazeAttackStateStep`, vtable `00D216B8` +0Ch. The body runs to `009E23A6` exclusive (`RET 4` at `009E23A3`, then INT3), and the ledger's `009E23AD` is off by 7. It is not projected, so the six Shinyo boats sit at `throttle 0.000 dir=stopped` while their fire target is a transport. | USNOS 3000 and 9000: 2622 and 2880 calls from 6 boats. No other row reaches it. | movement, ramming damage and deaths on the kamikaze rows | ship AI (**this lane**) | `cc9_kamikaze_attack_step`: project the step, bind it behind `kShipAiKamikazeAttackStepBound`, pair on USNOS 3000 and 9000 |
| 4 | **Touchdown and the landing chain.** On the fresh runs 286 per-plane touchdown lines all read `touchdowns=0`. LOMP10's fighters now reach `final entries=1` with `touched=0`. The land-task placements carry `unread=48..141` arm rows. | every row with aircraft | planes never land or rearm, which caps the sorties in long missions (E2, USNOSL, LOMP10L) | planes/units (lua15) | the lane's queued touchdown and land/park re-pair (pause record) |
| 5 | **The wingman steering.** `009F9E40` `BSP_PilotBot_CommandHeadingToPoint` is unread, and the host flies its own `heading_command_009f9e40`. With it `009BEE30` command_step and `009BFEE0` steer_point. | m: 8 rows, 85066 calls each. fresh: 4 rows, 63978. | how every formation flies, and so when the escorts and strike planes arrive | planes/units (lua15) | read `009F9E40` whole; replace the host heading behind a switch; pair on USN04 and USN13 |
| 6 | **The approach target point.** `009FADA0` `BSP_BotApproachTargetRef_Update`: the host aims at the target's own position, not at a lead. | m: 5 rows, 121210 calls. | torpedo and bomb aim, once #1 and #2 open | planes/units (lua15) | read `009FADA0` and the torpedo approach's vtable slot 0 |
| 7 | **The player gun seat, group 1/2 arm** (`00959C91..00959F6D` of `00959C20`), recorded whole. It covers the hand-over to the role-2 holder, the turn `0085ABA0` and the trigger. | m: 9 rows, 47371 calls | the Function 1/5/6 mounts of the controlled ship. Uncertain for an idle player: the AI-held seats are handed back, so the effect may be small. | gunnery (gunnery12) | bind the in-window arm alone; pair on USN01 and USN04 |
| 8 | **The AI planner spawn arm.** The `[capture]` / `[defend]` quick-spawns at `00A2B400` and `00A29B8E`, through `00A25B90` -> `00A25A30` (`contract: unread`). AI_PLANNERS calls it "unreachable in this process", but it is **due** 23 to 98 times per row. | m: USN13, USNOS, USNOSL, JM08, USN01 | whether the AI party's capture and defend planners get groups to order. Uncertain: `00A25A30` may create a group only, not units. | AI (**this lane**, `game_hosts_ai.cpp`) | read `00A25A30` whole; decide whether it creates units, then bind or close it |
| 9 | **Scripted-order natives still unimplemented:** `UnitHoldFire` `008A6AC0`, `PilotLand` `008A47B0`, `SetShipMaxSpeed` `00890A10`, `NavigatorEnable` `008A7060`, `Scoring_SetMissionCompleted` `008B8AD0`, and the non-squadron arms of `EntityTurnToEntity` (`008A0D1C`) and `UnitSetFireStance` (`0071BE80`). | one or two rows each, 1 to 9 calls: IJN01, BSM01, JM05, USN02, LOMP10, USN01 | targeting and movement orders the script gives and the host drops | lua (lua15) | one packet binding the five small natives |
| 10 | **The carrier motion remainder** `00758090` (`+EF8h` gate). It filters the carrier's turn rate into `+88h..+94h`. | m: 8 rows (144000 calls of the base fragment). fresh: 5 rows. | a deck-motion input to the landing (#4) | units/ship motion | read the readers of `carrier+8Ch` before binding |
| 11 | **Submarine periscope sub-state** `009E4DC1` (`ShipAiSubAttack`, `+122Ch`) | JM06 132, USNOS 61 | how the submarine attack surfaces and fires | ship AI (**this lane**) | read the `+122Ch` state arm |
| 12 | **The kill handlers**: the ship vt[84h] `00819880`, the plane `007CC580` and the ship wreck handler `00824B60`. The host's death route does the physics writes. | m: 4 to 7 rows, at most 31 calls | effects and any post-death messages the handlers send. The deaths themselves are modelled. | gunnery (gunnery12) | read `00819880` to list what it sends beyond the physics |
| 13 | **`GetCapturePercentage` `0089B840`**, neutral 0 | JM05, 48 calls | JM05 uses it for the score text only (`jm05.lua:5178..5216`, this installation, mtime 2024-07-13). Capture progress reads 0. | lua (lua15) | fold into #9 |
| 14 | **The Lua `Kill` misses:** `unresolved=1..3` on all six fresh rows (`008AC5C0`) | 6 of 6 fresh rows | a scripted kill of a name the host cannot resolve does not happen | lua (lua15) | log the unresolved names; they may be squadron members |
| 15 | **The approach sub-throttle and sub-heading producers** `009E6A90` / `009E5E90`. The host answers 0 instead of the 9999 sentinel. | m: 8 rows, 48718 calls | none today: SHIP_AI_OPEN_ITEMS section 26 counts no submarine approach. It opens if a submarine takes the approach. | ship AI (**this lane**) | park until a row reaches it |

## Checked and not gaps

- **`ai follow refused=`** (every row) is the image's own answer. AI_CAUTIOUS_ROUTE shows `00779D50`
  refusing members of the same group and ships of the other party.
- **`ai world sets ... Objectives_Add is unimplemented`**: the summary wording is stale. `008CD440` is
  bound, and the sets are empty because the scripts pass no targets (AI_WORLD_SETS, the correction to
  section 2). The log text should be fixed; that is lua15's call.
- **`FindEntity` UNIMPLEMENTED on JM05** is the first-call status only: 139 of 146 calls resolve.
- **`DamageControl::pending_exceeds_health_0090e6c0`**: `0090E6C0` is tagged `stl_probable`.
- **USN02's failure at 29.75 s** is the image's own for an idle player (GAME_EXECUTABLE, reference i).
- **The `MissionFrame::*`, `Hud*`, `Gui*`, `FrontEnd*`, `MissionLoad(*)` and `Title*` rows** are
  presentation or load plumbing.
