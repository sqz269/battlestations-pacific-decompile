# Mission state entry (state 0Ch to state 0Dh)

Addresses: 004db920 004da6c0 004dc6a0 004de610 00447060 004d87b0 00a7a440 004d7970 00a7a3f0

Packet `mission_state_entry`, worktree `agent/mission-state-entry`. Ghidra was read-only for this
packet; every name below is a hypothesis, not a recovered symbol.

## What the path is

`docs/MISSION_SCENE_LOAD.md` ends with `BSP_Game_LoadMissionScene` (`004DFB70`) returning with
`game+5D4h = 0Ch` and nothing pending. The transition into the simulation is made on the **next**
frame, by two routines:

| Address | Name | Role |
| --- | --- | --- |
| `004DB920` | `BSP_Game_UpdateDeviceWaitScreen` | the state 0Ch frame handler; decides whether the frame may enter |
| `004DA6C0` | `BSP_Game_EnterMissionState` | the entry itself; writes `game+5D4h = 0Dh` |

`BSP_Game_OnMove` reaches `004DB920` at `004E5046` in the frame-bookkeeping block
(`docs/GAME_ON_MOVE_MAP.md` step 16), guarded only by `game+5D4h == 0Ch`. `004DA6C0` has two other
callers, `00427190` (the front-end command dispatcher that `BSP_MainMenuScreen_Update`,
`BSP_MissionTreeScreen_Update`, `BSP_PressStartScreen_Update` and `BSP_InGameInterface_Update`
share) and `00777850` (the multiplayer message pump), so the entry is not exclusive to state 0Ch.

## `004DB920`, the state 0Ch handler

`__thiscall void (GGame* this)`, ECX only, `RET` with no immediate. The body is
`004DB920-004DBAAC` and is wrapped in an SEH scope (handler `00C66908`) that is not modelled here.

Arm selection at `004DB93C`/`004DB94C`:

```
004db93c  cmp dword ptr [ebp + 0x1fe4], 0     ; the local view mode
004db944  lea edi, [ebp + 0x1ef0]             ; EDI = the embedded session object
004db94a  jz  0x004db955                      ; mode 0 -> input-device arm
004db94c  cmp byte ptr [edi + 0x29c], 0
004db953  jz  0x004db9be                      ; clear -> player-count arm
```

The `LEA` makes the literal reading of the selector byte `session+29Ch`, not a standalone game
field; `docs/FRONTEND_MANAGERS.md` reads the same address as `game+218Ch`. Both are the same byte
and neither writer is identified (`docs/GAME_WORLD_OCEAN.md` already lists `session+29Ch` as open).

### The input-device arm, `004DB955-004DB9B9`

1. `00A91020` (`BSP_InputDeviceTable_AnyDynamicDeviceButtonDown`) with `ECX = [00F8BBF4]`.
2. A rising edge against the table's own previous sample at `table+DDh`. The new sample is stored
   back at `004DB977` on **every** pass, so a suppressed edge is consumed, not deferred.
3. No edge, or the menu command screen (`00425D10`, `+25Ch`) busy, or the GUI root
   (`[00F8ABE8]+3E8h`) busy: return without entering.
4. Otherwise `BSP_InputManager_Update(mgr, 0.0f)`. The `FLDZ` at `004DB9A7` pushes the argument of
   `00A92C40`, **not** of the singleton getter `004BEC00`; Ghidra attributes it to the getter.
5. `JMP 004DBA94`, which is `MOV ECX,EBP / CALL 004DA6C0`.

This arm reads as "press any button to start", including in single player, which is why state 0Ch
is entered at all rather than the load falling straight through to 0Dh.

### The player-count arm, `004DB9BE-004DBA8F`

Only local view mode 1 has this arm; any other non-zero mode returns at `004DB9C5`. It walks the
eight local player slots in two unrolled passes of four, stride 4, reading the slot pointers from
the **global** game object `[00E188A8]+18CCh` rather than from ECX, and counts slots whose `+9h`
and `+0Eh` bytes are both zero. `+9h` is the exclusion byte `LocalPlayerSlot::inactive` already
names; `+0Eh` is the byte `004DFB70` step 23 sets to 1 for every local-play slot, so the count is
"slots not yet bound".

```
004dba2f  cmp edx, 1
004dba32  jl  0x004dba3c          ; nothing unbound -> enter
004dba34  cmp dword ptr [ebp + 0x624], esi   ; ESI is 0 here
004dba3a  jz  0x004dba9b          ; still unbound and no override -> keep waiting
```

`game+624h` is unidentified (`docs/GAME_FRAME_CONTROL.md` and `docs/GAME_WORLD_OCEAN.md` both carry
it raw). When the arm proceeds it builds the tag `0Ch` event through `0075B430`, sets the event's
`+0h` vtable to `00CE74C8` and `+4h` to 1, dispatches it into the session through
`0076A9F0(session, &event, 0)`, then runs `007848F0` on `session+188h`, or on `session+18Ch` when
`+188h` is null, and falls into `004DA6C0`. `004DFB70` step 23 builds the same tag `0Ch` event.

## `004DA6C0`, the entry

`__fastcall void (GGame*)`, ECX only, `RET` with no immediate, body `004DA6C0-004DA776`. In order:

| Site | Action |
| --- | --- |
| `004DA6C9` | `004254B0("GGame::SceneInit()")`, `__cdecl`, one argument, ESP fixed at `004DA6D4`. Trivial body in this build. |
| `004DA6D7` | `MOVSS [ESI+21F0h], XMM0` - the scaled frame delta becomes `0.0f`. A **float** store; Ghidra shows an integer zero. |
| `004DA6DF` | `00447060` with `ECX = [ESI+30h]`, loaded at `004DA6D1` and dropped by Ghidra. |
| `004DA6E6-004DA70F` | the five one-shot bytes (below). |
| `004DA711-004DA71E` | single player only: `MOV word ptr [[game+18CCh + game+18ECh*4] + 10h], 1`. A **16-bit** store. |
| `004DA724-004DA734` | `00A7A440` with `ECX = [00F8BBD8]` and the float at `00F889A0`. Both the ECX and the source of the float are dropped by Ghidra. |
| `004DA73C` | **`game+5D4h = 0Dh`**. |
| `004DA746` | `BSP_Game_ApplyInGameInterface(game, 0)`, the same switch `004DFB70` calls with 1 at `004E1873`. |
| `004DA74B-004DA761` | networked only: `BSP_Game_CheckMultiplayerPlayerCount`, then re-read `game+5D4h` and **return early** if it is no longer 0Dh. |
| `004DA769` | `BSP_Game_SetCinematicMode(game, 0, 0, 1)`, pushes at `004DA763-004DA767`. |
| `004DA76E` | the byte `game+608h = 0`. |

The early return is real: `004D87B0` can reach `004D7970` (`GGame::EndScene()`), which enqueues a
state request and moves `game+5D4h`, and in that case the cinematic reset and the `+608h` store do
not happen.

`004CD0F0` computes `game+635h = -(game+634h != 0) & allow_simulation`, so passing `hide = 0` clears
**both** cinematic flags. That is what opens the gate `docs/GAME_SIMULATION_GATE.md` tests first;
`game+608h = 0` clears the companion byte the same gate reads together with `game+1FE4h`.

### The one-shots `game+1EE1h..+1EE5h`

Five adjacent bytes, all initialised by the game constructor `004DDB90` and all rewritten here.
Four are cleared and `+1EE3h` takes `game+1FE4h != 0`. The native store order is `+1EE2h`, `+1EE4h`,
`+1EE1h`, `+1EE5h`, `+1EE3h`; nothing observes the order.

| Byte | Written here | Other writers | Consumers |
| --- | --- | --- | --- |
| `+1EE1h` | 0 | `004D7970` from its own argument (`004D79A6`) | the mission-teardown arm of the drain (`004E45E8`, `004E47B0`, `004E4918`), the state 11h bookkeeping which publishes `00E198B0 = (byte == 0)` (`004E5090`), and `BSP_MainMenu_Init` (`006866B2`). Clear means the mission ended on its own terms rather than by an abort. |
| `+1EE2h` | 0 | `004D7970` at `004D7A49` | `004D7970` itself at `004D799F`: while set, the end-of-scene body (`00920A20`, `007556A0`, `BSP_MissionPlayerRecords_Reset`) is skipped. A re-entry latch. |
| `+1EE3h` | `game+1FE4h != 0` | `0076D030` and `00772610`, both the same expression | the drain at `004E45D2` and `004E4717`: set selects state 8 plus the multiplayer menu, clear selects `BSP_Game_EnterFrontEndShell`. It is a cached copy that outlives `game+1FE4h`. |
| `+1EE4h` | 0 | the session teardown paths `0076D0E0`, `007727A0`, `007728B0`, `00772990` | the drain at `004E4932`: `BSP_MultiMenu_PushRequestInterfaceLogged(4)`, the disconnect notice. |
| `+1EE5h` | 0 | `004D87B0` at `004D88B9` | the debrief `00920A20` at `00920AA2` and `00920B31`, together with `game+2018h`. Cleared again by the session restart paths `0076FAD0` and `00772610`. |

So `004DA6C0` clears the whole result-and-failure record of the previous mission and re-derives the
"this session is networked" copy, immediately before the state that will write those bytes again.

### `00447060`, the deferred dynamics release

`__thiscall void (sub)` on `game+30h`. `docs/GAME_SIMULATION_GATE.md` already records `game+30h` as
the ECX of `00447B80`, the per-frame update the gate runs with a float delta; `00447B80` advances
floats in the `8h`-stride range at `sub+4h/+8h` and reads the `20h`-stride records at
`sub+14h/+18h`. `00447060` releases each record's handle at `+0Ch` through `00C34F70` with
`ECX = [00E188A8]+18h` (a dynamics owner), erases that vector to its begin pointer, then compacts
the `sub+4h/+8h` range through `00446EF0`. The owning subsystem is not identified.

### `00A7A440`, the audio environment

`__thiscall void (SoundManager*, float)`, `RET 4`. It stores the float at `manager+4Ch`, overwrites
its own stack argument with `0FFFFh` and tail-jumps to `00A7A3F0`, which walks the array at
`manager+8Ch` of length `manager+90h` and sets `entry+14h = 1` for every entry whose class index at
`[[entry+44h]+8h]` has its bit in the mask. The whole call is "set the level, then invalidate every
bus".

The other in-engine callers, `004DFBCC` and `004DA852`, write `manager+6Ch` and then hand the
manager's own current `+4Ch` straight back in - a re-apply idiom. `BSP_Settings_ApplyAudio` passes
`settings+20h`. `004DA734` is different: it passes the global float at `00F889A0`, which the whole
image references from that one instruction only and which ships as `0.0f`. So mission entry drives
`manager+4Ch` to zero. The meaning of `+4Ch` is **not** established; a master volume, a duck amount
and a menu attenuation all fit the evidence, and only the last two are consistent with the zero.

### `004D87B0`, the multiplayer player-count check

`__fastcall void (GGame*)`. Returns immediately unless `game+1FE4h` is set, `game+5FCh` (the loaded
scene record) is non-null and `game+5D4h` is `0Dh`.

With `session+29Ch` clear it counts side 0 and side 1 participants, either through `004BB770` or,
when `[00F8A2FC]+4Dh` is set, by walking the eight `118h`-stride blocks at `game+748h` and testing
each with `004B5530`, taking the side from `+28h`. Effective game mode 7
(`BSP_Game_GetEffectiveGameMode`) needs more than one participant in total; every other mode needs
both sides non-empty. On the first failure it stores `DAT_00F876A4 + [00CE3DB0]` as a deadline in
`00E18B40`, sets `game+1EE5h`, latches `00E18B44` and returns. On a later failure it raises the
`ingame.multi_notenoughplayer` message through `00734E50`, and in mode 1 past the deadline calls
`004D7970(0)` - the call that changes `game+5D4h` and makes the caller's re-check meaningful.

With `session+29Ch` set it instead counts entries per side over `game+18CCh` using
`[[game+5FCh]+988h]` as the length, and when a side is empty runs `00982990` and `00530650`
(a menu command) and latches `00E188BD`.

## The world construction contracts

Both routines run **inside** `004DFB70`, one frame earlier; they are contracts this packet reads,
not code it reconstructs. Note the ledger names differ from the packet brief: `004DC6A0` is
`BSP_Game_ConstructGlobalSubsystems` and `004DE610` is `BSP_Game_ConstructWorld`, the opposite
pairing to the one the brief assumed.

### `004DC6A0` `BSP_Game_ConstructGlobalSubsystems`

`__fastcall void (GGame*)`, sole caller `004DFB70` at `004DFEBB`. The whole body runs inside the VFS
file block `Game_Global` (`BSP_FileBlock_Construct` .. `BSP_FileBlock_Destroy`), in this order:

1. `00886900`, `00800160`, `00901610`, `006F7B50` (trivial), `00803A40`, `006DBEB0`.
2. `collectgarbage("collect")` through `006B8AD0` when `[game+1A08h]+4h` is set, i.e. only when the
   Lua host already holds a state.
3. A drain loop over the `8h`-stride vector at `[00432650]+10h/+14h`, each element built by
   `00871BA0(1)` and consumed by `004D9C00`, each result released by an interlocked refcount
   decrement; then `[00432650]+2D8h = 0`.
4. Five constructions, each `operator new` followed by a constructor and a follow-up call:

| Field | Size | Constructor | Follow-up |
| --- | --- | --- | --- |
| `game+21D0h` | `58h` | `004A43C0` | `0049D690` |
| `game+21E4h` | `38h` | `00452660` | `0044FA30` |
| `00F88C30` | `1C4h` | `008EDC60` | `008ECEC0` |
| `game+21E0h` | `1B0h` | `0098A020` | `009870A0` |
| `game+21C4h` | `7Ch` | `00445B10` | `00444D20` |

Of the four subsystem pointers `docs/GAME_WORLD_ENTITIES.md` needs, this routine fills exactly one,
`game+21D0h`. It does not touch `+19CCh`, `+19E8h`, `+21A0h` or `+21D4h`.

### `004DE610` `BSP_Game_ConstructWorld`

`__fastcall void (GGame*)`, sole caller `004DFB70` at `004E01DE`, immediately after the first
`0046DF00` scene pass and inside the `1_` VFS block. Body `004DE610-004DFAFC`. The fields it fills,
in body order:

| Field | Size / source | Constructor | Note |
| --- | --- | --- | --- |
| `game+19CCh` | `4BCh`, memset 0 | `004CB030` | the entity/unit manager the world tick and the render frame both walk |
| `game+19ECh` | `24h` | `00B724E0`, name `"World"` (`00CE7E10`) | the scene root every later instantiation targets |
| `game+19FCh` | `00B71A80` | named node | also published to `DAT_00E188B0`, then `BSP_Camera_SetNearPlane`; this is the `Operator` node `docs/GAME_RENDER_FRAME.md` sees at `0068A0D0` |
| `game+1A00h` | `00B1F850` | | released again through an interlocked refcount decrement in the same block |
| `game+19E8h` | `00BBDFF0(game+19ECh, name)` | `sky_001`, or `[game+5FCh]+C54h` | the ocean owner of `docs/GAME_WORLD_OCEAN.md`, whose `+3Ch` is the ocean object; this is also where `Ocean initialization failed` and the `ShoreWaves` sources are reached |
| `game+19F0h` | `0078DAA0(game+19ECh, ...)` | | |
| `game+21C8h` | `10h` | `008E2B90` | |
| `game+21CCh` | `10h` | `009221E0` | |
| `game+21D4h` | `50h` | `006DECA0` | the fourth world-tick subsystem |
| `game+21DCh` | `70h` | `00707480` | |
| `game+21E8h` | `58h` | `00735030` | |
| `00F89B3C` | `00945820` | | cleared to 0 on the failure path |

`game+21A0h` is created by `BSP_Game_OnInit` at startup (`004E3E74`), not by either routine. So of
the pointers the brief asks about: `+19CCh`, `+19E8h` and `+21D4h` come from `004DE610`, `+21D0h`
from `004DC6A0`, and `+21A0h` from neither.

## Ghidra artefacts encountered

- Every hidden `this` in `004DA6C0` is dropped: `00447060` (`game+30h`), `00A7A440`
  (`[00F8BBD8]`) and the float source `00F889A0`.
- `004DA6D7` is a `MOVSS`, so `game+21F0h` takes `0.0f`, not an integer zero, and `004DA71E` is a
  16-bit store.
- In `004DB920` the `FLDZ` argument is attributed to the singleton getter instead of to
  `BSP_InputManager_Update`, and the `LEA EDI,[EBP+1EF0h]` base is folded into a flat
  `param_1 + 0x218c`.
- In `004DE610` the decompiler aliases the allocation sizes into stack slots (`piStack_ac`,
  `local_88`), so the sizes above come from the listing.
- The `_free` no-return problem `docs/MISSION_SCENE_LOAD.md` records did not bite in `004DA6C0` or
  `004DB920`; both bodies were taken from the listing anyway.

## Uncertainties

1. `game+624h` and `session+29Ch` have no identified writer, so both branch selectors in `004DB920`
   are carried raw.
2. The meaning of `manager+4Ch` in `00A7A440`, and therefore of the constant `0.0f` at `00F889A0`.
   If `+4Ch` is a master volume, mission entry mutes the game, which cannot be right; a duck or
   attenuation reading fits better but is not proven.
3. Slot `+10h` is written as `0FFFDh` by `004DFB70` for every slot and as `1` here for the one slot
   `game+18ECh` selects. No reader of the field was traced.
4. The owning subsystem of `game+30h`.
5. Whether the third argument of `004CD0F0` has an effect when `hide` is zero.

## What remains, and follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `mission_end_scene` | 004d7970 00920a20 009205e0 00916980 | docs/MISSION_END_SCENE.md, include/bsp/mission_end_scene.hpp | `GGame::EndScene()`: the `+1EE2h` latch, the debrief hand-off and what `+1EE5h`/`game+2018h` select |
| `session_teardown_latches` | 0076d0e0 007727a0 007728b0 00772990 00772610 | docs/SESSION_TEARDOWN_LATCHES.md | the four writers of `game+1EE4h` and the one clearer of `+1EE5h`; which disconnect each represents |
| `game_dynamics_list` | 00447060 00447b80 00445db0 00446ef0 00c34f70 | docs/GAME_DYNAMICS_LIST.md | the `game+30h` subsystem: its two ranges, the `20h` record, and what the per-frame update integrates |
| `sound_manager_levels` | 00a7a440 00a7a3f0 008d5430 00a7b230 | docs/SOUND_MANAGER_LEVELS.md | `manager+4Ch`/`+6Ch`, the bus array at `+8Ch`, and the class-bit mask |
| `mission_participant_table` | 004bb770 004b5530 004bca50 00982990 | docs/MISSION_PARTICIPANT_TABLE.md | the `118h`-stride blocks at `game+748h` and the side field at `+28h` that `004D87B0` counts |

## Reconstruction

`include/bsp/mission_state_entry.hpp` and `src/mission_state_entry.cpp`. The arm selection, the edge
latch, the slot count and the enter/wait decision are pure predicates; the one-shots are a whole
struct projection (`arm_mission_one_shots`); the two entry sequences run over
`MissionStateEntryHost` and `MissionDeviceWaitHost`, one method per native call site in body order,
in the style of `bsp::run_application_frame`. `GameStateId`, `kLocalPlayerSlotCount` and
`kGameStateSceneReady` are reused from `bsp/game_frame_control.hpp` and
`bsp/mission_scene_load.hpp`. Nothing that has no evidence has a default implementation.

## State reached

| Address | State |
| --- | --- |
| `004DB920` | reconstructed, build-tested |
| `004DA6C0` | reconstructed, build-tested |
| `00447060` | analysed |
| `004D87B0` | analysed |
| `00A7A440` | analysed |
| `004DC6A0` | analysed (contract only) |
| `004DE610` | analysed (entry, field writes and order only; the body was not read in full) |

Nothing here is ABI-compatible or game-validated. Every routine named in this packet already has a
Ghidra function; there is no listing-only routine to define.
