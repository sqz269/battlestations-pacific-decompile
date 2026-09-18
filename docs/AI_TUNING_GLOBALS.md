# The AI coordinator's tuning block

Addresses: `00A335D0` (the loader), `00A371A0` (the reader side, which settles the base and the
stride), `009FFC80` BSP_Ai_EffectiveGameModeIndex (which of the seven records a rule reads),
`004BCA50` BSP_Game_GetEffectiveGameMode, `00A15950` (the difficulty the first arm clamps),
`00B66330` BSP_LuaReference_GetFloatOrDefault, `00A37130` and `00A371C0` BSP_AiGlobals_Reload
(the loader's two callers).

Packet `cc8_ai_tuning_globals`. `GameAiCoordinatorHost` answered 0 for every tuning field, so
both merge phases and the planner's range rule were idle.

## 1. What the loader does, and what was already known

`docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md` read `00A335D0` in full and carries the whole 143-slot
key table with every store site and default. This packet does not restate it. What it needed
from it, verified against that document and the listing:

* The loader opens its own Lua state and runs `Scripts\global\luaMW_init.lua` then
  `Scripts\datatables\HighLvlAIGlobals.lua` (`00A33600`, `00A3365C`), takes the global
  `HighLvlAIGlobals`, and loops seven times (`XOR EDI,EDI` at `00A3374B`, `CMP EDI,7` / `JL` at
  `00A370D4`), selecting one sub-table per iteration.
* The record is `23Ch` bytes, based at `coordinator + 4` (`IMUL EDX,EDX,23Ch` at `00A338DD`
  against `LEA EAX,[EAX + ESI*1 + 4]` at `00A371B3`), so a loader store at `+004h` is record
  `+000h`.
* A missing key takes the float `BSP_LuaReference_GetFloatOrDefault` was handed, which is a
  literal in the image.

**Its two callers.** `00A3717C` inside `CG_array_ctor_helper_00a37130`, the static-init helper,
and a tail jump at `00A371C7` inside `BSP_AiGlobals_Reload`. Neither runs in this process: there
is no AI globals reload and the static-init helper is not reached, which is why the block was
zero.

## 2. The mode selector, `009FFC80`

Read for this packet, because it decides which of the seven records a rule reads.

```
009ffc80  MOV ECX,[00E188A8]
009ffc89  TEST ECX,ECX / JE 009ffcf6      ; no world -> 0
009ffc8d  CALL 004BCA50                    ; the effective game mode
009ffc92  CMP EAX,7 / JA 009ffcf4          ; above 7 -> 0
009ffc97  JMP [EAX*4 + 009FFCFC]
```

The jump table at `009FFCFC` sends game modes 0, 1, 2 and 3 all to `009FFC9E`, the difficulty
arm, which calls `00A15950` and clamps the result into 0..2. Modes 4, 5, 6 and 7 take the
literals 3, 4, 5 and 6 at `009FFCD0`, `009FFCD9`, `009FFCE2` and `009FFCEB`.

So the seven records are three difficulty variants of Island Capture followed by the four
non-campaign modes, and a campaign mission reads one of the first three.

## 3. The four names the planner packet had to guess

`docs/AI_PLANNERS.md`'s open question, "the Lua key names of tuning fields `+1CCh`, `+1D0h`,
`+1D4h`, `+1D8h`, `+208h`'s neighbours", is answered by the key table in
`docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md` plus the shipped script. Those four offsets are exactly
the four `ai_planner_choose_attack_target` reads, and the names confirm the guesses:

| record | `bsp/ai_planners.hpp` name | shipped key | image default |
| --- | --- | --- | --- |
| `+1CCh` | `kAiPlannerTuningOwnSetFactor` | `FreeAttack_ObjectiveTargetMul` | 2 (`00CE3958`) |
| `+1D0h` | `kAiPlannerTuningRangeNear` | `FreeAttack_NearDist` | 5000 (`00D1AF84`) |
| `+1D4h` | `kAiPlannerTuningRangeFar` | `FreeAttack_FarDist` | 12000 (`00CE3968`) |
| `+1D8h` | `kAiPlannerTuningStickyFactor` | `FreeAttack_ExistingTargetMul` | 1.5 (`00CE380C`) |
| `+208h` | (the merge distance) | `AutoMerge_MergeDist` | 650 (`00D20180`) |
| `+20Ch` | — | `AutoMerge_LeaveDist` | 1200 (`00CFD714`) |

The near and far distances and the sticky multiplier are what the guessed names said they were.
"OwnSetFactor" is the one that was off: `+1CCh` is the multiplier applied to a target that is an
objective, not a set membership factor.

## 4. The shipped content

`scripts/datatables/highlvlaiglobals.lua` of this installation, 1194 lines, modified
**2024-07-13**, which is that installation's untouched bulk date, the same as
`reconclasses.lua`. This installation is modded, and this file is not one of the modified ones;
the numbers below are this installation's and are not called retail.

The seven sub-tables appear in the loader's own mode order at lines 3, 191, 377, 563, 718, 887
and 1042.

| mode | table | ObjectiveTargetMul | NearDist | FarDist | ExistingTargetMul | MergeDist | LeaveDist |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | `IslandCaptureParams_Rookie` | 2 | 5000 | 12000 | 2 | 650 | 1200 |
| 1 | `IslandCaptureParams_Regular` | 2 | 5000 | 12000 | 2 | 650 | 1200 |
| 2 | `IslandCaptureParams_Veteran` | 2 | 5000 | 12000 | 2 | 650 | 1200 |
| 3 | `DuelParams` | 2 | 5000 | 12000 | 2 | 650 | 1200 |
| 4 | `EscortParams` | 2 | **9000** | **15000** | 2 | 650 | 1200 |
| 5 | `SiegeParams` | 2 | 5000 | 12000 | 2 | 650 | 1200 |
| 6 | `CompetitiveParams` | 2 | 5000 | 12000 | 2 | 650 | 1200 |

**Two of the six are not the image default**, which is why reading the script matters and why a
block filled from the image defaults alone would be wrong: `FreeAttack_ExistingTargetMul` is 2
in every mode against a 1.5 default, and `EscortParams` widens the two distances.

## 5. The rules

`include/bsp/ai_tuning_globals.hpp` and `src/ai_tuning_globals.cpp`, pure:

* `ai_tuning_mode_009ffc80(effective_game_mode, difficulty)`, the jump table and the clamp.
* `AiTuningBlock`, the `23Ch` record addressed by record offset, with every unreconstructed slot
  left at the zero an unloaded block holds, so a field this packet did not read answers exactly
  what it answered before rather than an invented number.
* `AiTuningLuaReader`, the injected reader, and `ai_tuning_load_00a335d0` over it, which takes
  the image default on the absent-key arm, as `00B66330` does.
* `AiTuningAuthoredReader` over the authored rows above.

No name collides with `bsp/ai_group_think.hpp`, `bsp/ai_planners.hpp` or
`bsp/ai_command_lifetime.hpp`.

## 6. The host

`GameAiCoordinatorHost::create_00a32350` loads the block once. `auto_merge_dist` and
`tuning_field` read it instead of answering 0, and both are now `done` rather than `record`. The
census gains a `summary mission ai tuning` line naming the mode, its table, and the four values.

### Scope, stated plainly

The block is filled from the **authored rows transcribed into
`src/ai_tuning_globals.cpp`**, not by running the script on a Lua state. `00A335D0` runs it on a
private Lua state, and reaching the mission state from this host would mean a constructor
argument threaded through `src/game_hosts_units.cpp`, which another packet holds. The rules take
an injected reader precisely so that swapping in a live one is a one-line change. This follows
the same pattern `src/sensor_table_data.cpp` already uses for `reconclasses.lua`.

Six of 143 slots are reconstructed. The other 137 stay zero and are unchanged by this packet.

## no_ghidra_function

none. `00A335D0`, `00A371A0`, `009FFC80`, `004BCA50`, `00A15950` and `00B66330` all have Ghidra
functions.

## Corrections

Appended to `docs/AI_PLANNERS.md` (the open question is answered) and to
`docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md` (the loader now has a consumer), never rewriting them.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `ai_tuning_live_reader` | `00A335D0` | run `highlvlaiglobals.lua` on the mission Lua state and feed `AiTuningLuaReader` from it, so the other 137 slots stop being zero. Needs the AI host to reach `GameMissionLuaHost`, which is a constructor argument in `src/game_hosts_units.cpp` |
| `ai_difficulty_source` | `00A15950` | the difficulty the `009FFC9E` arm clamps. Nothing in this process produces one, so mode 0 is taken; the three Island Capture rows agree on all six reconstructed fields, so it changes no number yet |

## Validation

`./scripts/build.ps1` Win32 `/W4 /WX`: clean. `ctest --test-dir build/win32 -C Release`: 2/2
pass. No test added.

All six runs on this branch over main at `315adad83`, the before side with the wiring stashed,
the three after runs taken serially with the game process stopped between them.

The block loads on every mission:

```
summary mission ai tuning mode=0 (IslandCaptureParams_Rookie) merge_dist=650.0
  near=5000.0 far=12000.0 sticky=2.00
```

`auto_merge_dist` and `tuning_field` are now `done` rather than `record`.

**Every other number is identical before and after, on all three missions.**

| | IJN01 | USN01 | USN02 |
| --- | --- | --- | --- |
| groups created / splits taken | 4 / 2 | 4 / 2 | 2 / 0 |
| auto merges / proximity merges | 0 / 0 | 0 / 0 | 0 / 0 |
| planner claims / attack orders | 2 / 1 | 2 / 1 | 1 / 1 |
| commands issued | 34 | 17 | 14 |
| gunnery shots | 34 | 3838 | 562 |
| gunnery hull / deaths / damage | 4 / 2 / 500.0 | 24 / 1 / 229.5 | 168 / 3 / 19723.2 |
| plane yaw_plans | 5904 | 19563 | 0 |

That is the honest result, and each consumer's reason is separate.

**`AutoMerge_MergeDist` = 650 is never compared.** Phase 5 walks
`first_group_of_proximity_list`, which is `g_aiGroupsByTeam[2]`, the list `00A2EB11` and its
four siblings read. Every group in this process takes its team from its first member's side, and
these missions carry sides 0 and 1 only, so list 2 is empty and the loop body never runs. The
distance is correct and unreachable. `docs/AI_GROUP_THINK.md` already carries this as an open
question: "what value `+54h` takes for a group to land in `g_aiGroupsByTeam[2]`". Until that is
answered, no proximity merge can happen whatever the tuning says.

**`can_auto_merge` is still false**, so phase 4 takes nothing regardless of tuning. It delegates
to the absorbed group's `+564Ch` `vtable[+14h]` and this process builds no AI command object;
`docs/AI_COMMAND_LIFETIME.md` names `00A13340` as the packet that unblocks it.

**The four planner fields changed the scores and not the argmax.** They feed
`ai_planner_choose_attack_target`, which scores the enemy team's groups and orders the best. One
attack order lands per mission, at the same target as before, so the commands and the outcome are
unchanged to the digit. With a larger target set the sticky multiplier of 2, which is the value
that differs most from the 1.5 default, would start to matter.

So the packet's own deliverable is landed and measured, and the two idle merge phases are idle
for reasons this packet did not cause and names by address.

## On the run wrapper

`tools/run_game.ps1` from main still leaves the game process alive after it returns. Every run
above was followed by an explicit stop before the next began, and on an earlier pass a survivor
held both the log file and `bsp_game.exe`, which failed a link with `LNK1104` and made one log
read return a previous run's contents. Timings from that pass: the wrapper printed its summary
and exited, and two `bsp_game.exe` processes created at 03:24:10 and again at 03:56:28 were
still running minutes later, parented to the wrapper's own `pwsh.exe`.
