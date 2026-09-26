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
| `PilotMoveToRange` native | concrete, 9 calls, doing nothing | concrete, 9 calls | routed | the OFF census row was wrong: `handles()` ignored the switch. Part 1b makes an OFF switch leave the native an unimplemented record |
| moveto task records | 0 | 17: each ordered leader in `moveto (moveto)`, each wingman in `follow (moveto)` | 2 per ordered pair | held: 8 escort pairs give 16, and the ninth call orders `moviefisher`, a single plane, for 1 |
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

## Part 1b: EntityTurnToEntity and UnitSetFireStance

The script calls both from `luaBombersSpawnedLex` (`usn_19_coralus.lua:3086-3098` and the five
repeats): `EntityTurnToEntity` on the bombers (`unit1`) and the escorts (`unit2`) toward
`bombertrg`, then `UnitSetFireStance(unit2, 2)` on the escorts. `luaLexKillersSpawned` (1173-1176)
and the cutscene plane `moviefisher` (2184) also turn. `UnitSetFireStance(Mission.Lex,
STANCE_HOLD_FIRE)` (867) is at the mission's end.

**`008A0A10` EntityTurnToEntity**, single-player arm (`[00E188A8]+1FE4h` is 0, so the
session-message arm through `007EDEB0` is not taken):
- `d = target+FCh - entity+FCh`; `d.y` is zeroed unless a third argument is present and true
  (`008A0C0A`-`008A0C5F`);
- the matrix at `esp+34h` is rows (1,0,0), (0,1,0), `d`, position 0, `[+70h]` 1.0, then `0085DC80`
  orthonormalises it (`008A0C65`-`008A0CE3`, read from the listing);
- a squadron (`vtable[5Ch](18h)`, `007EFB00` answers 18h, 2, 1, 0) re-poses each of its `+3CCh`
  members, at most five, at the member's own `+FCh` through member `vtable[88h]`
  (`008A0D6D`-`008A0DE2`);
- a plane's `vtable[88h]` is `007C9540`: it copies the matrix to `+74h` and `+674h`, clears the two
  pose-valid bytes, and calls `(plane+310h)->vtable[0Ch]` = `007BEEE0`, which re-derives attached
  matrices. It writes no velocity.

**`008A6490` UnitSetFireStance** on a squadron: `vtable[114h]` is `007ECFD0`, the `+348h`
command block built at `007F4FE3`-`007F5009` by `0084D810` (vtable `00D0BD98`). `0071BE80` asks
that block's own predicates and stores through the shared senders:

| slot | body | rule |
| --- | --- | --- |
| `+24h` | `0084D910` | allowFire for stance 1 or 2 |
| `+28h` | `0084D930` | allowMove for stance 0, 2 or 3 |
| `+40h`/`+44h` | `0071DA50`/`0071DAD0` | send; `+64h` `0071D5D0` stores `+3Ch`, `+68h` `0071D5E0` stores `+3Dh` |

The block's defaults (`0084D862`-`0084D8A2`): Behaviour (`+364h`) 0 frees both; any other value
frees fire only for a fighter class (`[+35Ch]->vtable[18h](13h)`), move only otherwise. An
escort Zero squadron has Behaviour -1, so it starts with **allowFire 1, allowMove 0**, and stance
2 sets allowMove 1. A plane's `vtable[114h]` is `0047F180` (`XOR EAX,EAX; RET`), so on a single
plane the native does nothing.

**Readers of `+3Dh`.** `0080DC70` tests `+3Ch` and `+3Dh` together. `009F83CB` in
`BSP_Bot_RevalidateCurrentCommand` calls it on `bot+4` for a category 1 or 2 command. The block's
own `+50h` slot `0084D960` is the same test. Which object a plane bot holds at `+4` is not read
here, so whether allowMove 0 holds a Zero back in the image is open.

**The binding.** `kMissionTurnAndStanceBound`, squadron arms only:
- **`run_entity_turn_to_entity`** builds the basis from the leader's position, the stand-in for
  the squadron's `+FCh` because `007F4580` spawns every member at one matrix. It re-poses each
  member through `GameUnitsHost::set_unit_world_basis_007c9540`. A plane or ship argument is left a
  record, because those arms are unread.
- **`run_unit_set_fire_stance`** keeps the block's two bytes per squadron in the script host,
  because this host builds no `+348h` block. Nothing in the host reads them yet.
- **`handles()`** now returns false for a switch that is off, for `PilotMoveToRange` too.

### Predictions for part 1b's pair (written before the runs)

The pair is `kMissionTurnAndStanceBound` OFF against ON, with `kPilotMoveToTaskBound` OFF on both,
on main `25b5f4fb8`, E2 9000.

| row | OFF | ON prediction |
| --- | --- | --- |
| `EntityTurnToEntity` | UNIMPLEMENTED, 18 calls | concrete, 18 calls; 16 squadron turns (8 bomber, 8 escort) plus the `moviefisher` and `LexKillers` calls if they are reached, which log a record |
| `UnitSetFireStance` | UNIMPLEMENTED, 8 calls | concrete; 8 escort squadrons, allowFire 1->1, allowMove 0->1 |
| `PilotMoveToRange` | now UNIMPLEMENTED, 9 calls | the same |
| each turned member's heading at the callback | its spawn heading | level and toward the Lexington (the Town for waves 4-6) |
| bomber and escort gameplay rows (releases, hits, deaths, distance) | as main | may move within the RNG bands: a bomber's approach starts from a new heading |
| ship rows and everything not in a turned squadron | as main | within band only through RNG coupling |

### Part 1b's pair, measured

`local\TT_OFF_9000.log` (binary `local\tt_off`) and `local\TT_ON_9000.log` (`local\tt_on`), main
`25b5f4fb8` plus this change, window 1600x900 in both, the spawn callback at mission frame 500 in
both, so there is no clock offset.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| `EntityTurnToEntity` | UNIMPLEMENTED, 18 | concrete, 18; all 18 took the squadron arm; 52 member re-poses | 16 squadron turns plus records | held, and more: `movieval` and the `LexKillers` are squadrons too, so no call fell to the record arm |
| `UnitSetFireStance` | UNIMPLEMENTED, 8 | concrete, 8; every escort squadron Behaviour -1, fighter, allowFire 1->1, allowMove 0->1 | the same | held |
| `PilotMoveToRange` | UNIMPLEMENTED, 9 | UNIMPLEMENTED, 9 | the same | held; the `handles()` correction works |
| turned heading | spawn forward (±0.17, 0, 0.98), heading ±10 deg | forward about (0, 0, -1) | level, toward the Lexington | held. **The turn is about 170 degrees**: every wave spawns flying away from the fleet |
| plane deaths | 35 | 38: A6M Zero #1.2 wingman, #6.2 and its wingman added; none removed | within band | **moved beyond the band** |
| torpedo releases | 6 | 3 | within band | moved |
| dive-bomb releases | 2 | 4 | within band | moved |
| gunnery shots / hit records | 4597 / 586 | 6316 / 586 | within band | shots moved |
| plane distance moved | 1168477 | 1089997 | within band | moved |
| the Lexington's distance moved | 6616 | 3086 | within band | moved |
| ship-AI plan seeds, approach frames | 1674, 1504 | 3740, 4872 | as main | moved; the ships react to planes that now close on them |

**Verdict: kept OFF.** The pair moves gameplay well beyond the heading and the stance. That was
expected: a bomber that spawns facing the fleet reaches it about a turn earlier, and the escorts,
which hold no moveto task while part 1a is off, fly into the fleet's anti-aircraft fire.

**What the pair cannot settle.** `007C9540` writes no velocity, so in both the image and this host
a turned plane keeps its spawn velocity of about (±12, 0, 66) for its first steps. Whether the
image's flight model re-derives velocity from the body axes on the next step, and so turns the
plane at once, is not read. The switch should be judged together with parts 1a, 2 and 3 in the
final pair, not alone.
