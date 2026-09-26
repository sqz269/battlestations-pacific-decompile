# The pilot moveto task, kind 7 (packet cc9_pilot_moveto_task)

2026-09-25. Ghidra read-only. Names are hypotheses. The escort Zeros in USN04 are ordered by the
mission script (`usn_19_coralus.lua`, `luaBombersSpawnedLex`, difficulty 1-2):
`EntityTurnToEntity(unit2, Lex)`, `UnitSetFireStance(unit2, 2)` and `PilotMoveToRange(unit2, Lex)`.
The host left all three natives unimplemented, so the Zeros held no task
(docs/STACKED_SPAWN_SEPARATION.md). The packet is split into landable parts.

## Part 1a: the order, the route and the task record

**The native.** `008A4590` `PilotMoveToRange(unit, target [, range])`:
- it reads the unit and the target (`008A466D`-`008A46C7`);
- the range goes to the descriptor's `+14h`, only when exactly three arguments are passed
  (`008A46DC`-`008A4708`; the rule is `pilot_move_to_range_008a46dc`);
- it issues command class `00E08F68` (`moveto`) through `0077D600` `BSP_Entity_IssueCommand`
  (`008A471C`-`008A472A`);
- it refreshes the world pose afterwards.

**The route.** `0099A170` `BSP_Bot_InstallCommandTask` tests `00E08F68` at `0099A223` with no target
precondition and calls the factory `009C3BE0` `BSP_BotTask_MakeMoveTo` (`0099A236`).

**The task.** `009C3BE0` allocates 0x550 bytes and calls `009C3000` `BSP_BotTaskMoveTo_Construct`:
- `BSP_BotTask_ConstructBase` (`0099C6F0`) with kind **7**;
- a sub-object at `+3F8h` (`009C2DF0`) built on `009C1C30` and holding the three states:

| offset | name (`00411E70`) | constructor |
| --- | --- | --- |
| `+47Ch` | `moveto (moveto)` (`00D20998`) | vtable `00D20A80` set inline |
| `+494h` | `follow (moveto)` (`00D20988`) | `009C2980` `BSP_BotStateFollow_Construct` |
| `+52Ch` | `circle (moveto)` (`00D20978`) | `009C25D0` |

- vtables: task `00D20B68`, sub-object `00D20B58`, `+468h` = `00D20B54`;
- `+548h` = 1.0 (`009C307C`), and `+54Ch` = -U(0, 1) (`009C3084`/`009C3089`).

**The start state** (`009C3097`-`009C30BF`):
- a flight leader (`007B8AD0`, the unit's `+9D8h`) enters `+52Ch` circle when `task+454h` is set,
  otherwise `+47Ch` moveto;
- a wingman enters `+494h` follow;
- the state is stored at `+310h` and entered through `vtable[4]`.
`task+454h` is the sub-object's `+5Ch`, which `009C1D0A` clears at construction. The only other store
is `009C3636`, in `009C3570`, which is part 2's read. So a leader starts in moveto.

**The binding.** `kPilotMoveToTaskBound` (`include/bsp/pilot_order_bindings.hpp`) gates two pieces:
- **`GameScriptOrdersHost::run_pilot_move_to_range`.**
  - It issues `00E08F68` with the range in the descriptor.
  - It installs the task through `bot_install_command_task_0099a170`.
  - It stores the class and the range on the unit.
  - It fans the order out to every squadron member, as PilotSetTarget's `007ECF80` route does.
- **The units host's `run_moveto_task_install_009c3000`.** It installs the kind-7 record and picks
  the start state. Part 1 has no tick: the record changes no flight.

**Not in part 1a (part 1b).**
- **`EntityTurnToEntity` (`008A0A10`).** It builds an orthonormal basis toward the target
  (`0085DC80`, `008A0CE3`). For a squadron (kind 18h) it re-poses each `+3D0h` member at its own
  position through `vtable[88h]` (`008A0D6D`-`008A0DE2`). It sets the spawn heading and separates
  nobody.
- **`UnitSetFireStance` (`008A6490`),** a gunner setting.
Both are read in part 1b.

### Predictions for part 1a's pair (written before the runs)

The pair is `kPilotMoveToTaskBound` OFF against ON, on current main (`9f5c9e373`), E2 9000.

| row | OFF | ON prediction |
| --- | --- | --- |
| `PilotMoveToRange` native status | UNIMPLEMENTED, 9 calls | routed; 9 calls logged |
| moveto task records installed | 0 | 2 per escort pair ordered; the leader in `moveto`, the wingman in `follow` |
| every gameplay row (deaths, hit records, releases, the Lexington, distance) | as main | identical, or within band if the issued command changes the director's current command |

**The risk.** The issued command replaces the Zero's current director command. If anything in the host
reads that command for a Zero (the command-target lookup), the Zero may begin steering. Any moved row
is examined before the verdict.

### Part 1a's pair, measured

`local\PM_OFF_9000.log` (binary `local\pm_off`) and `local\PM_ON_9000.log` (`local\pm_on`), main
`9f5c9e373` plus this commit, window 1600x900 in both.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| `PilotMoveToRange` native | UNIMPLEMENTED, 9 calls | concrete, 9 calls | routed | held |
| moveto task records | 0 | 17: each ordered leader in `moveto (moveto)`, each wingman in `follow (moveto)` | 2 per ordered pair | held (one install short of 18, not traced) |
| every gameplay row | - | identical, distance moved included | identical | held |

**Part 1a is committed OFF,** as the lead briefed. It changes no flight until the tick (part 2) and
the wingman state (part 3) land.

## Part 2, the read so far

- **The tick.** `00D20B68` slot `+64h` is **`009C3950`**, with no Ghidra function
  (`009C3950`-`009C398B` inclusive). In order it runs:
  - `009C3570(task+3F8h, dt)`, the approach update, which writes `+5Ch` (`task+454h`) at `009C3636`;
  - `009C3310(dt)`, the transition rule;
  - the current state's `vtable[0Ch]`, at `009C3988`.
- **The moveto state's vtable** begins at `00D20A80`: destructor `009C2DB0`, tick `009C2430` (slot
  `+0Ch`). That is not the generic moveto tick `009C18C0` that the torpedo and dive-bomb tasks run.
