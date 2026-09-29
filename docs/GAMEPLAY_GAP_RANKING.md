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
| 1 | **USN13 torpedo run-in too high (CORRECTED).** The first version of this row was a misdiagnosis. `unit+C58h` is raised after the first drop, and the first drop needs no budget (lua15, `f260985db`, main `d3aa5a529`). The real blocker: the flights stay at 58-731 m in aim, so the 25-40 m altitude flag never opens. | USN13, 60 aircraft | damage and deaths from air torpedoes | planes/units (lua15) | the attackrun descent `009FBA50` |
| 2 | **AA lethality at 300-800 m (CORRECTED; takes in the dive-bomb row).** On USN04, 12 of 16 Kates are shot down in aim. The dive-bomb walk is not stalled either (lua15, `9b2693f83`, main `f9c99b3f0`). All 16 USN04 Vals pass the latch and the turndown, then:<br>- 7 are shot down in the dive at 542-851 m;<br>- 2 overshoot;<br>- 3 fail the fly-over tolerance (a separate lua15 item);<br>- 3 are still diving at the end;<br>- 1 releases.<br>JM05 at 3000 frames is simply too short: it needs 9200/9000 to show attacks. | USN04; JM05 at 9000 | the air strikes die before release | gunnery (gunnery12); fly-over tolerance: lua15 | measure the AA hit rate against the image at 300-800 m |
| 3 | **Kamikaze boat attack step `009E2020`.** `BSP_ShipAi_KamikazeAttackStateStep`, vtable `00D216B8` +0Ch. The body runs to `009E23A6` exclusive (`RET 4` at `009E23A3`, then INT3), and the ledger's `009E23AD` is off by 7. It is not projected, so the six Shinyo boats sit at `throttle 0.000 dir=stopped` while their fire target is a transport. | USNOS 3000 and 9000: 2622 and 2880 calls from 6 boats. No other row reaches it. | movement, ramming damage and deaths on the kamikaze rows | ship AI (**this lane**) | `cc9_kamikaze_attack_step`: project the step, bind it behind `kShipAiKamikazeAttackStepBound`, pair on USNOS 3000 and 9000 |
| 4 | **Touchdown and the landing chain (CORRECTED).** At 3000 frames every touchdown line reads `touchdowns=0`, but **that is expected** and not evidence of a gap. LOMP10's first touchdown is at about 153 s (frame about 3060), and all ten land by 267 s on the 9000-frame rows (SQUADRON_LAND_TASK 5o/5r). The one open question is the `unread=48..141` land-task arm rows. | the 9000-frame rows only | small: landing works on the long rows | planes/units (lua15) | none from this ranking. Re-check `unread=` on a 9000-frame row before planning a packet |
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
- **`ai world sets ... Objectives_Add is unimplemented`**: the summary wording was stale. `008CD440` is bound, and the sets are empty because the scripts pass no targets (AI_WORLD_SETS, the correction to section 2). The log line in `src/game_hosts_ai.cpp` is corrected in the same commit as this note.
  bound, and the sets are empty because the scripts pass no targets (AI_WORLD_SETS, the correction to
  section 2). The log text should be fixed; that is lua15's call.
- **`FindEntity` UNIMPLEMENTED on JM05** is the first-call status only: 139 of 146 calls resolve.
- **`DamageControl::pending_exceeds_health_0090e6c0`**: `0090E6C0` is tagged `stl_probable`.
- **USN02's failure at 29.75 s** is the image's own for an idle player (GAME_EXECUTABLE, reference i).
- **The `MissionFrame::*`, `Hud*`, `Gui*`, `FrontEnd*`, `MissionLoad(*)` and `Title*` rows** are
  presentation or load plumbing.
