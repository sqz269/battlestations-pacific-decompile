# The entity command arms of 00816E30: which command a routed order becomes

Addresses: 00816E30 00816EA6 00816EFC 00816F41 00816F65 00816F71 00816F7D 00816F89 00816FBE
00816FE3 00817017 008171BD 0081721F 00817227 00817330 00817334 0081735D 007AC9D0 00521EA0
00835860 0071D880 0071BF20 0077C8D0 0077C980 0077CA60 0064A8E0 0080DC70 007788B0 00470B80
004E5980 0059BD20 006E8040 00464F70 0071ECF0 00D09EC0 00D09F58 0071C0C0 00835E90 00E08EF8
00E08F00 00E08F08 00E08F10 00E08F60 00E08F68 00E08F70 00E08F78 00E08F80 00E08F88 00E08FA0
00E08FB0 00E08FB8

Packet `cc_ship_ai_arms`, worker `agent/cc-ship-ai-arms`, 2026-09-12 UTC. Ghidra was read-only
for this packet: no renames, comments, prototypes, function creation or saves. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; every live query verified both.
Descriptive names are hypotheses, not recovered symbols. The command names below are the
recovered literals of `docs/COMMAND_CLASSES.md`, which this packet treats as a contract and does
not re-derive.

## Answer to the packet question

**No arm of `00816E30` writes a director slot.** The cascade only decides *which* command
singleton the order becomes, and three of its arms decide that the order is not a queued command
at all. Everything that survives reaches the same three-instruction tail, which hands the
singleton and the descriptor to `0071ECF0 BSP_WeaponDirector_IssueCommand`. That routine writes
no slot either: it forwards to the AI group and routes an `MT_GAMEUNIT_SETCMD` message. The slot
at `controller+54h + 1Ch*i` is written on the **receive** side, in `00721A40`'s `5Ch` arm through
`008358D0` and `0071E6C0`, which `docs/COMMAND_EXECUTION.md` and `docs/CRUISE_COMMAND.md` already
carry.

So for the milestone 2m question, the arms are **not** where a scripted `moveto` or `attackmove`
is lost. A `moveto` whose descriptor names no entity passes the cascade unchanged, and a
scripted `attackmove` without an entity target is given one, manufactured on the spot. Both then
meet the same tail gate as any other command.

## The cascade, 00816EA6 to 0081732E

`00816E30` is `__thiscall(unit)(const EntityOrderMessage*)`, `RET 4`, body `00816E30-00817376`.
`00816E52..00816E96` copies the message's `+24h..+38h` into an `18h`-byte `SceneCommandTarget` on
the stack (the descriptor the tail passes at `00817351 LEA ECX,[ESP+20h]`), `00816EA1` latches the
message's `+21h` flags byte into `BL`, and `00816E9C` turns the ordinal at `+20h` into a command
singleton in `EBP`. The cascade then compares `EBP` against ten singletons in this order.

| test | singleton | name | what the arm does |
| --- | --- | --- | --- |
| `00816EA6` | `00E08F60` | `follow` | resolve the target, drop it unless `vtable[5Ch](2)`; `0077C8D0` `BSP_Entity_RequestJoinFormation`, then `0064A8E0`. **Returns; nothing is queued.** |
| `00816EFC` | `00E08FB0` | `Leave` | the same resolve-and-check, then `0077C980`. **Returns.** |
| `00816F41` | `00E08FB8` | `disband` | `0077CA60`. **Returns.** |
| `00816F65` | `00E08F70` | `cruise` | straight to the tail at `00817330` |
| `00816F71` | `00E08F88` | `stop` | the same |
| `00816F7D` | `00E08F80` | `moveonpath` | the same |
| `00816F89` | `00E08F68` | `moveto` | when the descriptor names an entity and `007AC9D0` gives that entity a path interface, the command **becomes `moveonpath`** (`00816FB4`) and jumps past the null test to `00817334` |
| `00816FBE` | `00E08FA0` | `land` | when the **unit itself** fails `vtable[5Ch](0Ch)`, the command **becomes `attackmove`** (`00816FD9`) and jumps to `00817334` |
| `00816FE3` | `00E08EF8` | `settarget` | `00835860 SetFireTarget(resolved target, force 1)`. **Returns.** |
| `00817017` | `00E08F00` | `cleartarget` | the block `00817023..008171BB`, summarised below. **Every path returns.** |
| `008171BD` | `00E08F08` | `clearorders` | `SetFireTarget(0, 1)`; stop if `007788B0` says the controller belongs to another entity; `0071D880 SendClearCommands`; and if `0080DC70` passes, `0071BF20 FreeFire`. **Returns.** |
| `0081721F` / `00817227` | `00E08F78` / `00E08F10` | `attackmove`, `artillery` | both **become `attackmove`** (`00817238`); with no entity target the routine manufactures one, below |
| otherwise | - | - | falls to `00817330` unchanged |

The three `RET 4` of the routine are `00816F62`, `00817014` and `00817374`; the arms that
"return" reach one of the first two or jump to the shared epilogue at `00817362`.

### 007AC9D0, the path interface

`__fastcall(entity)`, `RET 0`, body `007AC9D0-007ACA2C`, **complete**. It is a fixed-offset cast,
not a query:

```
if (entity == 0)                    return 0            ; 007AC9D5
if (entity->vtable[5Ch](47h))       return entity+1E4h  ; 007AC9DE, 007AC9E4
if (entity->vtable[5Ch](48h))       return entity+170h  ; 007AC9F5, 007AC9FB
if (entity->vtable[5Ch](49h))       return entity+310h  ; 007ACA0C, 007ACA12
if (entity->vtable[5Ch](4Ah))       return entity+1E4h  ; 007ACA21, 007ACA27
                                    return 0            ; 007ACA29
```

`170h`, `1E4h` and `310h` are three of the seven secondary bases of `docs/UNIT_INSTANCE_UPDATE.md`,
so the four kinds `47h`, `48h`, `49h` and `4Ah` are four classes that carry the same interface at
different sub-object offsets. A `moveto` aimed at such an entity is a path order, not a point
order; a `moveto` aimed at a point, or at anything else, stays a `moveto`.

### The manufactured attackmove target, 00817243..0081732E

An `attackmove` or `artillery` whose descriptor names **no** entity gets one:

```
00817243: memory = 00470B80(1E4h)                  ; 484 zeroed bytes
00817264: entity = 004E5980(memory, 0)             ; construct it
0081726B..008172E3: a 4x4 identity on the stack from 00D7A24C (1.0f) and a zeroed XMM
008172E9: sessionField = [00E188A8]+19CCh
008172FF: entity->vtable[98h](0, sessionField, &identity)
0081730A: transform = 0059BD20(scratch, &position)
00817312: 006E8040(entity, transform)
00817322: entity+54h = 2
00817329: 00464F70(descriptor, entity, 0.0f)       ; the descriptor now names the entity
```

`00470B80` is `BSP_Memory_AllocZeroed`; the other five bodies are `contract: unread` and each has
its own host method named by address in `include/bsp/entity_command_arms.hpp`. The address passed
to `0059BD20` at `00817301` is a three-float stack vector; the descriptor's position triple is the
only three-float group the prologue builds, so that is what it is, by inference and not by a
transcribed load.

`entity+54h = 2` is the same field `008162B0` compares between two entities at `00816353`
(`MOV EDX,[ESI+54h]`) and `00816356` (`MOV ECX,[EDI+54h]`) before `00816359 CALL 00803510`, so the
throwaway target is given a side before it is aimed at. What the value 2 means was not established.

### The cleartarget block, 00817023..008171BB

Not projected. Read for its shape only: it zeroes `[director+40h]` with `00CE3854`, reads the
director's `vtable[2Ch]` twice, calls `00835860 SetFireTarget(0, 1)`, may call `0071D9E0(2)`, asks
`0071BE60` for a queue length and walks back from it through `controller+54h + 1Ch*i`
(`008170CF MOV EBX,[EDX + ECX*4]`), then either `0071D900`, or `0071BFC0` plus `0059CAC0` on a
`moveonpath` slot, or `0071D880 SendClearCommands`. It is the only arm that reads the slots at all,
and it reads them to cancel, never to fill one.

## The tail, 00817330..00817374

```
00817330: TEST EBP,EBP ; JZ 00817362        ; an unknown ordinal ends here
00817334: TEST BL,BL                        ; the message's +21h flags byte
0081733E: 0071D880 SendClearCommands        ; flags != 0: replace the queue
0081734B: CALL EAX = director->vtable[34h](command)   ; flags == 0: may it be appended?
0081734F: JZ 00817362                       ; refused: nothing is queued
0081735D: 0071ECF0 IssueCommand(director, command, descriptor)
```

`BL` is written once, at `00816EA1 MOV BL,[ESI+21h]`, and the only other writes to `EBX` in the
routine (`008170CF`, `0081716F`) are inside the `cleartarget` block, every path of which returns.
That is the register-provenance check of `docs/WORKER_VERIFICATION_CHECKLIST.md` rule 8.

`vtable[34h]` resolves through the two director vtables that carry `008358D0` at `+60h`,
`00D09EC0` and `00D09F58` (the only two references to `008358D0` in the image):

| vtable | `+34h` | rule |
| --- | --- | --- |
| `00D09EC0` | `0071C0C0` | false for a null command, otherwise "the first empty slot index is below 10", i.e. there is room (`docs/CRUISE_COMMAND.md`) |
| `00D09F58` | `00835E90` | the ship override: the base test, then "a weapon command on top of the queue accepts only another weapon command" (`docs/CRUISE_COMMAND.md`) |

A `moveto` is category 3 (`docs/COMMAND_CLASSES.md`), so on a queue whose head is not a weapon
command the override returns true and the order is issued. **The arms and the tail do not refuse
a scripted `moveto`.**

## Coverage

| Routine | Coverage |
| --- | --- |
| `00816E30` | partial, and this packet is the front half: `00816EA6-0081732E` is projected as `entity_command_arm_cascade_00816ea6`, except the `cleartarget` block `00817023-008171BB`, which is summarised and stands behind one host method. The tail `00817330-00817374` stays `unit_apply_entity_command_00816e30` in `include/bsp/cruise_command.hpp`, unchanged. `00816E30-00816EA6` is the prologue that packet already covers |
| `007AC9D0` | complete |
| `00521EA0`, `00835860`, `0071D880`, `0071BF20`, `0071ECF0`, `00470B80`, `007788B0` | not re-read; named records from earlier packets, cited as contracts |
| `0077C8D0`, `0077C980`, `0077CA60`, `0064A8E0`, `0080DC70`, `004E5980`, `0059BD20`, `006E8040`, `00464F70`, `0071D9E0`, `0071BE60`, `0071BFC0`, `0059CAC0`, `0071D900` | `contract: unread` |
| `0071C0C0`, `00835E90` | not read here; quoted from `docs/CRUISE_COMMAND.md` |
| `007EEC50`, `007EE8F0` | not read; the attack chooser of `docs/ATTACK_COMMANDS.md`, downstream of the director and treated as a contract |

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| `docs/CRUISE_COMMAND.md` coverage: "`00816E30` partial: `00816EA6..00817330` ... is read in pseudocode but not projected" | Projected here, except the `cleartarget` block. The range is also not one block of nine arms: three of them (`follow`, `Leave`, `disband`) never reach the tail, and two rewrite the command | the arm table above |
| `docs/CRUISE_COMMAND.md` follow-up `entity_command_arms`: "the nine non-movement arms ... including the `00E08F78` `attackmove` block that manufactures a `1E4h`-byte throwaway entity" | Ten arms, and the throwaway block serves `artillery` as well as `attackmove`; `artillery` is rewritten to `attackmove` at `00817238` before it runs | `0081721F`, `00817227`, `00817238` |
| `docs/GAME_EXECUTABLE.md` milestone 2m reason (1): "a scripted attackmove or moveto does not reach a director slot through the entity command arms `00816f7c..00817330`" | The arms do not stop either one. A targetless `moveto` passes unchanged and a targetless `attackmove` is given a manufactured target; both then meet the tail, whose only gate is `vtable[34h]`, which accepts a category-3 command on a queue with room. If they do not reach a slot, the loss is on the routing or receive side, not here | the arm table, and the `vtable[34h]` table above |
| `src/cruise_command.cpp`'s `unit_apply_entity_command_00816e30` reads as the whole routine | It is the tail only. The cascade between the ordinal lookup and `00817330` was not modelled there; a caller that wants the routine's behaviour must run this cascade first and pass its `command` into that tail | the two projections side by side |

## no_ghidra_function

none. `00816E30` (`00816E30-00817376`) and `007AC9D0` (`007AC9D0-007ACA2C`) both have Ghidra
functions, and so does every callee named above.

## Follow-up packets

| packet | addresses / files | why |
| --- | --- | --- |
| `entity_command_cleartarget_arm` | `00817023..008171BB`, `0071D9E0`, `0071BE60`, `0071BFC0`, `0059CAC0`, `0071D900` | The one arm this packet did not project, and the only one that touches the ten slots. It walks the queue backwards and cancels, so it is the counterpart of `0071E6C0`'s push. |
| `entity_command_throwaway_target` | `004E5980`, `0059BD20`, `006E8040`, `00464F70`, `[00E188A8]+19CCh`, `entity+54h` | The five unread bodies behind the manufactured `attackmove` target, and what the side value 2 means. A scripted `attackmove` at a point depends entirely on this block. |
| `entity_command_group_arms` | `0077C8D0`, `0077C980`, `0077CA60`, `0064A8E0` | The three arms that never queue anything: `follow`, `Leave` and `disband` are formation operations, not director commands, and their bodies are unread. |
| `entity_path_interface_kinds` | entity kinds `47h`, `48h`, `49h`, `4Ah`, offsets `170h`, `1E4h`, `310h` | Which four classes carry the path interface `007AC9D0` casts to, and what the interface is. It decides whether a `moveto` is a point order or a path order. |

## Uncertainties

1. The address `0059BD20` receives at `00817301` is inferred to be the descriptor's position from
   the frame's contents, not from a transcribed load; the exact `ESP` arithmetic across the
   intervening pushes was not worked out.
2. `entity+54h = 2` is a side or party id by analogy with `008162B0`'s use of the same field. The
   value itself is not interpreted.
3. The `land` arm's `vtable[5Ch](0Ch)` is tested on the **unit**, not the target; which class
   kind `0Ch` is was not established, so "a unit that cannot land" is an inference from the
   rewrite, not from the kind.
4. The two director vtables `00D09EC0` and `00D09F58` are identified by holding `008358D0` at
   `+60h`. Which class each belongs to was not established, only that the ship path takes the
   `00835E90` override, which `docs/CRUISE_COMMAND.md` already attributes to the ship director.
