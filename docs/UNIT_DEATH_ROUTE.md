# The unit death route through 00959450 (packet `cc9_unit_death_route`)

Addresses: 00959450 `BSP_Unit_OnDestroyed` (body 00959450..00959608), 009813A0 (body
009813A0..00982110), 0091BDA0 `BSP_MissionScoring_RecordUnitKill`, 00565FB0 (body
00565FB0..0056602F).

This packet implements the contract in `docs/CONSTRUCT_WORLD.md` section 15 (cc9-side-ai). That
doc is leased to cc9-world-init while this is written, so this packet's record is here, and a
cross-reference is left for the doc's owner. Ghidra was read only.

## 1. The image's order, and the host's before this packet

`docs/CONSTRUCT_WORLD.md` section 15 gives 00959450's body: the report gate, the kind-20h
children, the loss report 009813A0, the kill credit 0091BDA0 (its only caller is 00959519), and
the controlled unit's limbo page 00565FB0.

**One correction, checked against the listing.** The children's `vt[1E8h](0)` call is at
**009594BB**:
- 009594A0..009594AD is `vt[5Ch](20h)`, the kind test;
- 009594AF..009594BB is `vt[1E8h](0)`.

**The host before this packet.** `GameGunneryHost::Impl::kill_unit` (`src/game_hosts_gunnery.cpp`)
ran the entity kill, the sink record, and then the kill credit directly.

## 2. The binding (`kUnitDeathRouteBound`, committed OFF)

After the sink, and before the credit, `kill_unit` runs `bsp::unit_death_00959450`, the model
`docs/CONSTRUCT_WORLD.md` section 15 corrected. It then acts on the decision:

| decision | the image | host (ON) |
| --- | --- | --- |
| gate passed (`reports_alternate`) | the kind-20h children take `vt[1E8h](0)` (009594BB), then 0091BDA0 runs | the victim's guns get `seat_trigger = false`, counted; then the existing `kill_credit_record_unit_kill_0091bda0` |
| gate failed | no children, no report, no credit | return before the credit |
| `reports_kill` | 009813A0 on the warning manager `[00F8A0C4]`. Its own guard: side (`+54h`) < 2 and IsKindOf 18h or 6 | **named record** `WarningManager::report_loss_009813a0`, with the guard's outcome counted per side |
| `registers_limbo_page` | 00565FB0, and then the limbo interface or retarget | **named record** `LimboScreen::take_unit_00565fb0`, counted |

**Substitutions** (labelled in the code):
- `unit+70h` is 1: every gunnery kill is a harm death (`docs/KILL_CREDIT.md`).
- `world+4ACh` is 1: it is set at the construct_world load step and never cleared in this host
  (`docs/CONSTRUCT_WORLD.md` section 9).
- The controller flags `+C41h` (kind 17h) and `+100Ah` (kind 6) are not held, so the loss
  report is never skipped.

**New summary line, printed on both sides:** `summary mission gunnery death route calls=
reported= child_triggers= loss_report_calls= loss_reports side0= side1= limbo_pages= bound=`.

### Contract for the mission-frame owner (the loss report's host entry)

The warning manager (`kWarningManagerTickBound`, ON) has no entry for 009813A0. The frame file,
`src/game_hosts_mission_frame.cpp`, is leased to cc9-world-init (cc9_sentity_init_all). So the
report stays a named record here, and this is the hunk for its owner:

```cpp
// include/bsp/game_hosts_mission_frame.hpp, beside game_warning_torpedo_effect_00977820:
// 009813A0 on [00F8A0C4](this, entity): the loss report. Guard at the top
// (side +54h < 2 and IsKindOf 18h or 6); the body 009813A0..00982110 builds
// the radio "we lost X" warning through BSP_Unit_LossCountingSlot.
void game_warning_report_loss_009813a0(std::size_t unit);
```

Once it exists, the gunnery route calls it in place of the record, under the same switch. The
body is 2.9 KB and has not been read past its guard; it needs its own packet.

## 3. Predictions, written before the pair

Two builds of one tree, differing only by the switch, with streams and the death table on.
USN02 9200/9000 and E2 = USN04 9200/9000.
- **Calls.** `calls` = `reported` = the deaths on each mission: **22 on USN02 and 51 on E2**.
  Every death lands after 27 s, so the clock gate passes, and the other two gate terms are the
  substitutions.
- **Loss reports.** `loss_report_calls` equals `calls`.
  - USN02: every loss is a ship of party 0 (Allied, 12) or 1 (Japanese, 10), which are the
    scene's `party=Allied(0)` / `Japanese(1)` lines. So the guard passes for **side0=12
    side1=10**.
  - E2: every death is an aircraft, which fails IsKindOf 6 and 18h. So it reads **side0=0
    side1=0**.
- **Limbo pages.** **1 on USN02**: DeRuyter, the controlled unit, sinks at 30.25 s. **0 on E2**:
  the Lexington survives.
- **Child triggers** > 0 on both, one per gun of a dead unit.
- **The kill credit** keeps its count and order: the `KillCredit::record_unit_kill_0091bda0` row
  (22 / 51), every `death row` line, and the kill-credit lines.
- **Gameplay** is identical: deaths 22 / 51, hit records 439 / 843, and USN02 failing at
  39.65 s. The cleared seat triggers belong to units that are already disabled.
- **New rows:** `Death::unit_on_destroyed_00959450` and `Death::child_trigger_off_vtable1e8`
  (concrete), and `WarningManager::report_loss_009813a0` and `LimboScreen::take_unit_00565fb0`
  (records).

## 4. The pairs and the verdict

One tree (main `a3916014b` plus `93103dfcb`), `local\dr_off` against `local\dr_on` (SHA-256
prefixes `C9BA4FB81AE3` / `13D327CC4B1E`). Streams and the death table on. Logs:
`local\dr_{off,on}_{usn02,usn04}.log`. Every log shows the 1600x900 override and its own module
directory, and every run exited 0.

| line | USN02 OFF -> ON | E2 OFF -> ON |
| --- | --- | --- |
| death route calls / reported | 0 -> 22 / 22 | 0 -> 51 / 51 |
| loss report calls; side0 / side1 | 0 -> 22; 12 / 10 | 0 -> 51; 0 / 0 |
| limbo pages | 0 -> 1 (DeRuyter) | 0 -> 0 |
| child triggers | 0 -> 309 | 0 -> 236 |
| new rows | `Death::unit_on_destroyed_00959450` 22, `Death::child_trigger_off_vtable1e8` 22, `WarningManager::report_loss_009813a0` 22 (record), `LimboScreen::take_unit_00565fb0` 1 (record) | the same, at 51, and no limbo row |

- Every other native row, every other summary line, and every `death row` and `plane death
  mode` line are identical both ways.
- The kill credit keeps its count and order: 22 / 51.

**Verdict: held, every prediction.** `kUnitDeathRouteBound` is ON. The loss report and the
limbo page stay named records until the warning manager gains the entry in section 2's
contract, and the limbo screen its own.
