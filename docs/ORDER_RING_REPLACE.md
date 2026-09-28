# What an AI order does to the unit's queue (packet `cc9_order_ring_replace`)

Worker cc9-ships3. Ghidra was read-only. Descriptive names are hypotheses.

## 1. Answer

**The premise is wrong: the host does not append.** The host's AI orders already replace the
queue, exactly as the image's do. The one departure is the host's **duplicate filter**, and
it goes the other way from the premise: it **suppresses** re-issues that the image makes.

- **In the image, a flagged order replaces the queue.** `0077D600` builds an `MT_COMMAND`
  whose `+21h` byte is the caller's flags (docs/ENTITY_ORDER_MESSAGE.md). The receiver
  `00816E30` tests that byte in its tail:

```
00816EA1  MOV BL,[ESI+21h]             ; the flags byte (the only write outside cleartarget)
00817330  TEST EBP,EBP / JE 00817362   ; unknown ordinal: nothing
00817334  TEST BL,BL
00817336  MOV ECX,[EDI+738h]           ; the director
0081733C  JE 00817345
0081733E  CALL 0071D880                ; flags != 0: SendClearCommands, every slot
00817343  JMP 00817351
00817345  MOV EDX,[ECX] / MOV EAX,[EDX+34h] / PUSH EBP / CALL EAX   ; flags == 0: room to append?
0081734D  TEST AL,AL / JE 00817362
0081735D  CALL 0071ECF0                ; issue into the (cleared) queue
```

- **Every AI path passes flags 1.** That covers the command tick `00A02020` (`00A0214B..00A0216A`,
  docs/AI_COMMAND_TICK.md), the close-attack pass `00A13B60` and the squadron fan-out
  `007F4E9C` (docs/CONSTRUCT_WORLD.md).
- **So each AI order clears the whole queue and issues itself at stage 0**, with no test for a
  duplicate. A `moveto` that the follower pass re-issues to the same point therefore restarts:
  the old command is discarded and a new one starts from stage 0. The host's scene-command path
  shows this on a real replace (docs/SHIP_COMMAND_LIFETIME.md: `slots 3 -> 1`,
  `stage 0 -> 0`).
- **The host is the same chain.**
  - `GameUnitsHost::issue_script_command` -> `GameCommandsHost::issue_command_object` ->
    `unit_apply_entity_command_00816e30` (`src/cruise_command.cpp`) calls
    `clear_all_commands()` at `0081733E` when flags != 0, and that clears all ten director
    slots.
  - Census on USN13: `EntityCommand::clear_all_commands 0071d880 calls=376` OFF and 1536 with
    the squadron membership on.
  - Its "UNIMPLEMENTED" label marks the unprojected message round-trip. The slots are
    cleared either way.
- **The only departure is the duplicate filter.** `GameAiCoordinatorHost::Impl::order_is_repeat`
  drops an order with the same token and target, and a point within 1 m, as the entity's
  previous one. It is a labelled substitution whose comment rests on the append premise. OFF it
  suppresses:

  | mission | suppressed |
  | --- | --- |
  | USN02 | 921 |
  | USN13 | 757 |
  | USN04 | 158 |
  | USN01 | 48 |

- **Consequence for USN13's halved damage.** Replace semantics cannot be what amplified the
  squadron churn: the host already replaces. The filter damps the churn, and removing it adds
  re-issues and restarts.

## 2. The binding

`kAiOrderReissueBound` in `src/game_hosts_ai.cpp`, committed OFF. When it is ON, a repeat still
counts in `orders_suppressed`, which then means "re-issued", and the order goes through. That
is the image's behaviour.

## 3. Predictions, written before the ON runs

**The export.**
- OFF is this tree's build: the head, with `kGeneratedSquadronBrainBound` and
  `kAiOrderReissueBound` both OFF.
- ON is `pair_export` with **two flips**: `kAiOrderReissueBound=true` and
  `kGeneratedSquadronBrainBound=true`, as the lead asked.
- The membership row is therefore in both USN04 and USN13, and the difference from the
  membership-only rows of docs/GENERATED_SHIP_AI.md section 7 is the re-issue's own effect.

**Every mission moves (exit 3), because every one has suppressed re-issues OFF.** The re-issues
the filter dropped now clear and restart a command.
- **USN13 3200/3000.**
  - Commands issued rise, relative to both OFF and the membership-only row.
  - Squadron commands and member orders do **not** fall. They stay at or above the
    membership-only 466 / 1398.
  - Deaths and hits do **not** move back toward OFF. They stay near or below the
    membership-only 16 deaths / 294 hit records.
  - This contradicts the lead's stated expectation. It is my prediction from the reading
    above.
- **USN04 4700/4500.** The same: commands and member orders at or above 4 / 12, releases near
  the membership-only row, and no return toward OFF.
- **USN01 3200/3000 and USN02 9200/9000.** They move (exit 3), where the lead expected
  identity: OFF suppresses 48 and 921 re-issues there.
  - On USN02 the re-issued orders are the ships' group `moveto`s. Restarting them resets the
    navigator's stage.
  - Direction on deaths is not predicted.

## 4. The pairs

- **OFF** is this tree's build at `26f3fcdf4`, with both switches OFF.
- **Two-flip ON** is `pair_export --commit 26f3fcdf4 --flip kAiOrderReissueBound=true --flip
  kGeneratedSquadronBrainBound=true` into `local\or_on`.
- **Ring-only ON** is the same export with only `kAiOrderReissueBound=true`, into
  `local\or_ring`. It was added to separate the ring's own effect.
- **Logs:** `local\or_{off,on,ring}_<mission>.log`.

| mission | two flips vs OFF | ring only vs OFF | two flips vs membership only (`sq_on`) |
| --- | --- | --- | --- |
| USN13 3200/3000 | exit 3: deaths 24 -> 16, hit records 720 -> 294 | exit 3: deaths 24 = 24, hit records 720 -> 711 | **exit 1, identical** |
| USN04 4700/4500 | exit 3: deaths 40 -> 29, hit records 644 -> 501 | exit 3: deaths 40 -> 41, hit records 644 -> 661, dive releases 1 -> 9 of 19, torpedo releases 5 -> 4 of 16 | **exit 1, identical** |
| USN01 3200/3000 | exit 1, identical | - | - |
| USN02 9200/9000 | exit 3: deaths 26 -> 24, hit records 847 -> 920 | - (no squadrons, so the two-flip row is the ring's) | - |

**Orders.**
- USN13 with both flips: squadron commands 474, member orders 1422, against 466 / 1398 with
  the membership only.
- USN04 with both flips: 203 / 609, against 4 / 12.
- The re-issues go out as predicted, but on top of the membership they change no gameplay
  number.

**USN02.** Exeter's loss at 210.81 s and the 212.91 s failure are unchanged. Houston's sinking
moves from 295.95 s to 293.75 s. The moves are after the mission has already failed.

**Predictions:**
- **Held:**
  - the re-issues go out;
  - USN13's and USN04's squadron commands and member orders stay at or above the
    membership-only row;
  - deaths and hits do not move back toward OFF;
  - USN02 moves.
- **Failed on spread:** USN01 is identical, not moved. Its 48 re-issues change nothing.
- **The lead's hypothesis is refuted.** Squadron commands did not fall, and USN13 did not move
  back toward OFF. **The membership's USN13 and USN04 moves are the membership's own.** The
  re-issue adds nothing to them, because the host already replaced the queue.

## 5. Decision

- **`kAiOrderReissueBound` is ON.** It is the image's semantics, and it failed only on spread
  (USN01). The ring-only rows are its own moves: USN04's dive releases rise from 1 to 9 of 19,
  and USN13 moves slightly.
- **`kGeneratedSquadronBrainBound` stays OFF,** by the lead's criterion "only the ring if the
  membership still swamps the mission". It still halves USN13's damage.
- This read shows that the swamping is not produced by the order path. The next question for
  it is the membership's own consequence: the brain re-tasking the player's launched squadrons
  (docs/AI_BRAIN_PLAYER_EXEMPTION.md). That is the lead's call.
