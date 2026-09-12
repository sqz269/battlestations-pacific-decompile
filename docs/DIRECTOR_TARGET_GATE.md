# The weapon director's auto-target hold at `director+40h`

Addresses: `0071DF70` `0071D980` `0071F290` `00720180` `00720225` `00817031` `00721A40` `0071C830`
`0077C2A0` `0071E7F0` `008358D0` `00D7A218` (data) `00D7A260` (data) `00CE3854` (data)
`00CFDA40` (vtable) `00D09F58` (vtable) `00CFD9C4` (vtable).

Packet `cc2_director_target_gate`, read-only analysis of the shipped
`battlestationspacific.exe`. Every descriptive name here is a hypothesis, not a recovered symbol.
The director is the command controller at `unit+738h` (`docs/WEAPON_DIRECTOR.md`,
`docs/COMMAND_EXECUTION.md`).

## Answer to the packet question

`director+40h` is a **float countdown in seconds** with three producers, and the shipped build
writes it. `docs/GAME_EXECUTABLE.md` milestone 2n is wrong on both halves of its finding: the
field has a writer, and the gate's comparison runs the other way.

| # | Site | Containing function | Value | When |
| --- | --- | --- | --- | --- |
| 1 | `00720225` `MOVSS [ESI+40h],XMM0` | `00720180`, the command controller's base constructor | `-1.0f` from `00D7A260`, loaded at `007201E7` | once, at construction |
| 2 | `0071F314` `FSTP [ESI+40h]` | `0071F290`, controller `vtable[0Ch]`, the per-frame update | `hold - frameDelta` | every frame, only while `hold >= 0.0f` |
| 3 | `00817031` `MOVSS [ECX+40h],XMM0` | `00816E30 BSP_UnitInstance_ApplyEntityCommand`, the `cleartarget` arm | `3.0f` from `00CE3854` | when a `cleartarget` command (`00E08F00`) reaches the unit |

`ECX` at site 3 is `[EDI+738h]`, loaded twelve bytes earlier at `00817023`, inside the arm the
compare `00817017 CMP EBP,0E08F00h` selects. The arm then reads the director's current fire target
through `vtable[2Ch]` and clears it through `00835860`, so **`cleartarget` clears the target and
suppresses re-acquisition for three seconds**. That is the whole meaning of the field.

`00720180` reaches the director because `008363E0 BSP_WeaponDirectorBase_Construct` calls it at
`00836403` with `ECX` still holding `this` (`008363FC MOV ESI,ECX`, and the intervening `PUSH ECX`
only reserves a stack slot). Milestone 2n looked at the derived constructor `008366D0`, which
indeed does not write `+40h`; the base-base constructor does. Ghidra's auto-name for `00720180` is
`CG_array_ctor_helper_00720180`, which is why a name search did not find it.

### The scan behind the negative half

`tools/bsp.py ghidra xrefs` does not answer field questions, so the producer was found by scanning
the shipped PE's executable sections for every instruction that addresses `[reg+40h]`
(`local/scan_off40.py`, ignored): `MOVSS`/`MOVSD`/`MOVUPS` stores, `MOV m32,r32`, `MOV m32,imm32`,
`MOV m8`, `FST`/`FSTP`, `ADD`/`SUB`/`INC`/`DEC`, both `disp8` and `disp32` forms, plus the matching
loads. 7568 sites, 3884 of them writes, 794 writes to a non-stack base. The 794 were then filtered
for a `+738h` displacement within the preceding `100h` bytes (`local/scan_dir40.py`), which is how
`00817031` surfaced; the only other hit, `00812DA5` in `BSP_UnitOrderRing_Construct`, initialises
its own record's `+2Ch..+54h` and is unrelated. Sites ruled out by reading them:

| Site | Why it is not the director |
| --- | --- |
| `0071BEA9`, `0071BEF8`, `0071BF48`, `0071BF98` | `MOV EDX,[EAX+40h]` where `EAX = [ESI]`: a vtable slot load, not a field |
| `007214DB` | copies director `+220h..+223h` into a session message at `+40h..+43h` (message vtable `00CFDB9C`) |
| `0092F2EE` | `BSP_UnitController_ApplyNetworkState` writes a four-float block at its own `+40h..+4Ch` |
| `009F4D55` | `BSP_ShipAi_PublishOrderSlot` writes an element of a `54h`-stride order-slot array |
| `0084D8A5` | `FUN_0084D810`, the other subclass of `00720180`, reaches a different object through `EDX` |

## The rules

### `0071DF70 BSP_WeaponDirector_AcceptsNewTarget`

`__thiscall(director) -> bool` in `AL`, `RET` (no stack arguments). Body `0071DF70-0071DFCF`.
Coverage: complete.

| # | Rule | Evidence |
| --- | --- | --- |
| 1 | `hold = [director+40h]`; when `hold > 0.0f` return **false** | `0071DF70`, `0071DF75` against `00D7A218` = `0.0f`, `0071DF7C JBE` to the scan, fall-through `0071DF7E XOR AL,AL` |
| 2 | `n` = leading occupied slots: walk `+54h` in `1Ch` steps from index 0, stop at the first null command pointer, cap 10 | `0071DF83`..`0071DF9E` |
| 3 | for `i` in `0..n-1`: `cat = slot[i].command->vtable[0Ch]()`; when `cat` is 1 or 2 return **false** | `0071DFA7`..`0071DFC2`, the `CMP EAX,1` / `CMP EAX,2` pair |
| 4 | otherwise return **true** | `0071DFC6 MOV AL,1` |

Rule 1 is the correction. `JBE` is taken on below, equal **and** unordered, so the constructed
`-1.0f` passes the gate and only an armed hold rejects. Rule 2 never looks past a gap: a null slot
hides every slot behind it from rule 3, which matches `0071E6C0 PushCommandSlot` filling the first
null slot and `00720850` shifting the tail down so no gap is left.

`vtable[0Ch]` on a command is `int category()`, the third getter of the five-slot command
descriptor (`docs/COMMAND_CLASSES.md`). `docs/SCENE_COMMAND_TYPES.md` reads the four values as
0 = no category (`cleartarget`, `clearorders`, `Leave`, `disband`), 1 = gunnery (`settarget`,
`artillery`, `strafe`, `dogfight`), 2 = weapon run (`torpedo`, `divebomb`, `levelbomb`,
`dropkamikaze`, `depthcharge`, `rocket`, `kamikaze`, `attackmove`), 3 = movement for the remaining
ten. **1 and 2 are exactly the set `008358D0 BSP_WeaponDirector_SetCommand` forces the fire target
for** (`docs/COMMAND_EXECUTION.md`), so the two routines agree on one idea: a gunnery or weapon-run
command in the queue owns the unit's fire target, and the auto-target think must not replace it.

### `0071F290`, the controller's per-frame update, `vtable[0Ch]`

`__thiscall(controller, float frameDelta)`, `RET 4`, body `0071F290-0071F3A4` (inclusive; `RET 4` at `0071F3A2`). **Ghidra has no
function here**; decoded from disk bytes. Its address sits at `00CFDA4C`, `00D09ECC`, `00D09F64`
and `00D0BDA4`, i.e. slot `+0Ch` of the base controller vtable `00CFDA40`, of both director
vtables, and of a fourth class's table, so the director does not override it.

The hold arm, `0071F2F8..0071F316`: load `[ESI+40h]`, compare against `00D7A218` = `0.0f`, and
`JB` past the subtraction. A value below zero is left alone; anything else becomes `hold - dt`
through x87 (`FLD` the spilled value, `FSUB` the argument, `FSTP` back). So an expired hold settles
just under zero on the frame it crosses and stops there, and the `-1.0f` sentinel never moves.
Coverage of the rest of the routine: partial, and only summarised here because it is the reader of
the field: `0071F290..0071F2F7` refreshes a vector on the object at `+1A4h` behind four session
flag tests on `[+34h]`; `0071F317..0071F3A4` lifts `mode` from 0 to 1 when slot 0 is occupied,
sends `0071D9E0(2)` and `0071D810(2)` behind the `+4Ch` and `+44h` accepted flags, and ends with
`[+38h]->vtable[4](dt)` and `vtable[7Ch]()`.

### `0071D980`, the override-command send

`__thiscall(director, const CommandClass* command, const SceneCommandTarget* target)`, `RET 8` at
`0071D9DD` (arguments read at `0071D9A4`/`0071D9A8` as `[ESP+54h]`/`[ESP+50h]`, which is entry
`[ESP+8]`/`[ESP+4]` once the three SEH pushes, the `SUB ESP,3Ch` and the `PUSH ESI` are counted).
One SEH frame, handler `00C85108`, around the stack-built message. Body `0071D980-0071D9DD`.
Coverage: complete.

1. `0071D99E COMISS XMM0,[ESI+40h]` with `XMM0` zeroed at `0071D995`: the send runs only when
   `0.0f > hold`, so an armed hold **and** a hold sitting at exactly zero both suppress it.
2. `0071D9B4` `0071C830 BSP_GameUnitSetCommandMessage_Construct(&stackMessage, command, target, 0)`
   builds `MT_GAMEUNIT_SETCMD` (type byte `5Ch`).
3. `0071D9C9` `0077C2A0 BSP_Session_RouteMessage([director+34h], message, 7, 0)`.

The fourth argument of `0071C830` is the byte at message `+20h`, and it is what separates this
routine from `0071ECF0 BSP_WeaponDirector_IssueCommand`, which passes `1` at `0071ED64` and first
runs a local apply through `vtable[30h]` and a HUD-side call `00A2BD90`. On the receive side
`00721A40 BSP_WeaponDirector_ApplyGameUnitMessage` branches on that byte at `00721B36`:

| `message+20h` | Receive-side arm | Meaning |
| --- | --- | --- |
| non-zero | `00721B5A` `director->vtable[60h](command, target)` = `008358D0 SetCommand` | queue the command |
| zero | `00721B7C` `0071E7F0 BSP_WeaponDirector_SetOverrideCommand(command, target)` | install it as the override at `+188h`, `mode` 2 |

Both arms decode the message the same way, `00721030` for the target record and `007216D0` for the
command ordinal at `+21h`. So **`0071D980` issues an override command**, which is why the
auto-target think reaches for it at step 14 only when `[director+30h]` (`mode`) is already set:
`docs/BOT_FIRE_TARGET.md`'s `0071D980(attackmove, 00465080(chosen, 0.0f))` installs the bot's
`attackmove` as the override without disturbing the queue.

## What this means for the executable's AI

The float half of the gate is not what stops `bsp_game.exe` from firing. A freshly constructed
director holds `-1.0f`, which passes rule 1, and this packet found no `cleartarget` producer on the
executable's path, though it did not look for one beyond the arm's own call sites. What remains to explain a think that never reaches `00835860` is rule 3 (a queued
category 1 or 2 command) or one of the earlier steps of `009F5DA0` (`docs/BOT_FIRE_TARGET.md`
steps 1-12). **No run-time evidence is offered here**: this packet did not run `bsp_game.exe`, so
the claim above is a static reading of the gate, not a statement about an observed frame.

One consequence is worth recording because it reads as a contradiction and is not one:
`0071DF70` needs `hold <= 0.0f` and `0071D980` needs `hold < 0.0f`. The two disagree only on the
single frame where a hold lands exactly on zero, which the subtraction in `0071F290` makes
unlikely but not impossible. On such a frame the think may accept a new target and still skip the
override send at step 14.

## Host table

| Step | Site | Callee | Host method | Arguments | Condition |
| --- | --- | --- | --- | --- | --- |
| category of a queued command | `0071DFAE` | `command->vtable[0Ch]` | `command_category` | ECX = command | slot index < prefix length |
| build the SETCMD message | `0071D9B4` | `0071C830` | `build_set_command_message` | command, target, flag `0` | `0.0f > hold` |
| route it | `0071D9C9` | `0077C2A0` | `route_message` | `[director+34h]`, message, `7`, `0` | same |

## Coverage

| Routine | Coverage |
| --- | --- |
| `0071DF70` | complete, `0071DF70-0071DFCF` |
| `0071D980` | complete, `0071D980-0071D9DD` |
| `0071F290` | partial: the hold arm `0071F2F8-0071F316` is complete and decoded from raw bytes; `0071F290-0071F2F7` and `0071F317-0071F3A4` are summarised from one read and their callees are unread |
| `00720180` | partial: the field stores `007201AB-0072031E` are read, including the `+40h` store; the three `CALL` tails and everything past `0072031E` are unread |
| `00816E30` | partial: only the `cleartarget` arm `00817017-00817098`; `contract: partial` stands from `docs/SESSION_MESSAGE_DISPATCH.md` |
| `00721A40` | partial: the `message+20h` branch `00721B2E-00721B87` and the `vtable[38h]` call at `00721A93`; the rest is `docs/COMMAND_EXECUTION.md`'s |
| `0071ECF0` | read only as the contrast to `0071D980`; its `vtable[140h]` and `00A2BD90` arms are unread |
| `00720850` | not re-read; `docs/COMMAND_EXECUTION.md` owns it |

## Corrections to earlier documents

| Was | Is | Evidence |
| --- | --- | --- |
| `docs/GAME_EXECUTABLE.md` milestone 2n: `0071DF70` "requires the float at `[director+40h]` to be greater than 0.0f" | It requires the float to be **at most** 0.0f; a value greater than 0.0f is the rejection | `0071DF7C JBE 0071DF81` jumps to the slot scan; the fall-through at `0071DF7E` is `XOR AL,AL; RET` |
| milestone 2n: `director+40h` "has no writer in this process, no recovered producer anywhere" | Three writers: `00720225` (`-1.0f`), `0071F314` (`-= dt`), `00817031` (`3.0f`) | this document's producer table |
| milestone 2n: "not written by the director constructor `008366D0`" | True of `008366D0`, but its base `008363E0` calls `00720180` at `00836403`, which writes the field | `008363FC MOV ESI,ECX` leaves `ECX = this` for the call |
| `docs/WEAPON_DIRECTOR.md` coverage: `0071C1E0` "complete as a body; its caller is unread", and the note that Ghidra records no caller | The caller is `00721A40` at `00721A93`, `director->vtable[38h](message)`, reached from `00780120`'s `IsA(59h)` arm | `00721A8F MOV EDX,[EAX+38h]`; already recorded in `docs/SESSION_MESSAGE_DISPATCH.md` lines 246 and 321 |
| `docs/WEAPON_DIRECTOR.md` coverage: `00720CD0` "partial: ... `00720850` ... unread" | `00720850` has been read since, by packet `cc2_director_commands` | `docs/COMMAND_EXECUTION.md` and the reconstruction comment on `00720850` in Ghidra |

## Open questions

- Which command, if any, the executable's units carry in slot 0 when the think runs. That, not
  `+40h`, is the live candidate for the gate that stops `00835860`, and answering it needs a run.
- Whether any `cleartarget` reaches a unit in normal play, i.e. whether the three-second hold is
  ever observed. The arm is in the entity-command receiver, so a mission script or a player order
  is the only producer found.
- `0071F290`'s other arms: what `[+1A4h]` is, and what `vtable[78h]` answers for `0` and `1`.
- The fourth vtable holding `0071F290` at `00D0BDA4`, and whether `FUN_0084D810`'s subclass also
  uses the hold.

## Correction from docs/DIRECTOR_UPDATE_ARMS.md (packet cc2_director_update_arms)

- **Was:** 0071F290: 'Ghidra has no function here'
  **Is:** Ghidra has BSP_CommandControllerBase_Update with body 0071F290-0071F3A4
  **Evidence:** tools/bsp.py ghidra proto 0071F290 returns that body range
- **Was:** 0071F290 'sends 0071D9E0(2) and 0071D810(2) behind the +4Ch and +44h accepted flags'
  **Is:** the accepted flag only decides whether the arm runs; the raise happens when vtable[78h] refuses to begin the command
  **Evidence:** 0071F342 TEST AL,AL / JNZ 0071F34F skips the raise when the begin succeeded; the same shape at 0071F366
- **Was:** open question: what [+1A4h] is and what vtable[78h] answers for 0 and 1
  **Is:** +1A4h is the array of ten pointers to the 50h-byte path objects 00720180 allocates; vtable[78h] is 0071F600, argument 0 selects the override pair and non-zero the queue-head pair
  **Evidence:** 007203AB LEA EDI,[ESI+1A4h] with EDI += 4 per iteration and a count of 10; the ledger entry on 0071F600
