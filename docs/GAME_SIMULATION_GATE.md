# The in-mission simulation gate of GGame::OnMove (packet `game_simulation_gate`)

Addresses: 004e50b0, 004c40a0, 004cce50, 004f7740, 0068a140, 004bfe50, 004db030, 0068c1f0,
004cd0f0, 004bca50, 006881f0, 006882a0, 00689030, 006529e0, 006529f0, 00652a20, 00652a30,
00652a50, 004f7620.

Phase 18 of the map in `docs/GAME_ON_MOVE_MAP.md`, 004e50b0..004e525e, with the fallback at
004e53b4. The game update `BSP_Game_OnMove` (004e4a40) has an eight-byte Ghidra body, so every
listing below comes from `python tools/bsp.py disasm-raw 004e4a40 --length 2808`, not from the
decompiler. `game` is ECX/ESI throughout, and equals `DAT_00e188a8`: 004bfe50 reads the same
fields through the global that the call site passes in ECX and never touches ECX.

## The entry predicate

```
004e50b0  if (game+634h == 0) goto label;
004e50b9  if (game+635h == 0) goto 004e53b4;   // interface-only fallback
label:    one-shot profiler label "GGame::OnMove::game" (00ce82e4), bit 1 of 00e18b58
004e5118  if (game+5D4h != EBX) goto 004e53b4; // EBX = 0Dh, loaded at 004e4ffb
004e5124  if (game+7184h != 0)  goto 004e53b4;
```

`EBX` is set to `0Dh` at 004e4ffb and `EDI` to `1` at 004e4ff0, and nothing between there and the
gate rewrites either, which is what fixes the state constant and the `> 1` comparison below.
`game+7184h` is the suspend byte the state-11h arm at 004e5054 also tests; nothing in this packet
writes it. All three failures reach the same site, 004e53b4, which calls the interface-only
update 004c40f0 and skips the entire simulation for the frame.

### What 0Dh, 10h and 11h are

The state word `game+5D4h` and the request ids of `docs/GAME_FRAME_CONTROL.md` share one
numbering: the drain at 004e4430 stores the popped request **into** `game+5D4h` before
dispatching it. `include/bsp/game_frame_control.hpp` already carries the enum, and this packet
adds three confirmations from the other side:

| Value | Evidence found here |
| --- | --- |
| `0Dh` in mission | The only state in which this block runs, and the console command `pause` maps to request 12h, whose handler sets `game+5D4h = 0Dh` and calls `004cd0f0(1,1,1)`. So 12h is resume-into-mission with the HUD hidden and the simulation still allowed, not a pause. |
| `10h` mission teardown | 004cd0f0 keeps the effect layer groups hidden while `game+5D4h == 10h` **or** the front of the request queue is 10h, so the HUD is not restored on the way out of a mission. Matches the `term` console command and the teardown arm at 004e458a. |
| `11h` mission end wait | Handled immediately before the gate (004e504b..004e509d): it raises `game+5ECh`, lowers it again unless `game+7184h` is set, and on an empty queue enqueues request 04h and sets `00e198b0 = (game+1EE1h == 0)`. |

### game+634h and game+635h, the only two cinematic flags

004cd0f0 is the sole writer of both. `+634h` is "HUD hidden"; `+635h` is
`-(game+634h != 0) & p3` at 004cd26c, a mask AND, so it can only be raised while `+634h` is.
The gate's first test is therefore not "is a cinematic playing" but "is a cinematic playing that
is allowed to keep simulating". The pause path raises `+634h` with `p3 = 0`, which is exactly how
pausing stops the simulation without a separate paused flag.

## 004cd0f0, the cinematic-mode setter

`__thiscall`, ECX = game, three stack arguments `(char hide, byte allow_simulation, char p4)`,
**`RET 0Ch`** at 004cd37e. Ghidra's `(int, char, byte, char)` is right; the OnMove call at
004e4b1a passes `(0,0,1)` and 004db030 passes `(game+634h == 0, 0, 1)`, a toggle.

1. 004cd117: refuses to hide while `game+1FE4h != 0 && game+218Ch == 0 && DAT_00e198c4 != 0 &&
   game+608h == 0` — the HUD cannot be hidden inside a network session.
2. 004cd12f: when `00425d10()+25Ch` is set it only writes `00425d10()+261h = hide` and returns.
3. Otherwise, when `hide != game+634h`, it stores the new flag and drives four GUI layer groups
   through `00a7acf0`/`00a7f890`, in this order: `3DEffect`, `InGameGUI`, `Warnings`, `Sonar`
   (`Sonar` is at 00ce7840, next to the `.ema` literal 004cce50 uses). `InGameGUI` receives
   `game+634h` itself, which is what proves the byte means **hidden**, not visible. The other
   three share one value that is cleared only when un-hiding outside state 10h and with no
   pending 10h request (004cd142..004cd165).
4. 004cd23a: on the hidden path with `p4` set it also calls `00a77760(1)` and zeroes the scaled
   delta `game+21F0h`, which freezes the simulation clock for the frame.
5. 004cd26c: `game+635h = (game+634h != 0) & allow_simulation`.

## 004c40a0, the first call inside the block

`__thiscall`, ECX = game, no stack arguments, `RET`. It tests nothing; it is a fixed four-call
sequence, each with the scaled delta `game+21F0h`. Ghidra dropped two of the four ECX values, so
this comes from the listing:

| Site | Call | ECX |
| --- | --- | --- |
| 004c40ad | 00875bb0 | game (ESI, never reloaded) |
| 004c40b4 | 004c3cb0 | game, and no float argument |
| 004c40ce | `[[game+19CCh]]->vtable[+0Ch]` | `game+19CCh`, the unit/entity manager 0068c1f0 also walks |
| 004c40dd | 00447b80 | **`game+30h`**, loaded at 004c40d7 and lost by the decompiler |

## 004cce50, the engine-movie start, behind the game+1EE0h latch

```
004e5138  if (game+1EE0h != 0) { ECX = [00e188a8]; 004cce50(); game+1EE0h = 0; }
```

The call site reloads the game from the global instead of reusing ESI even though the two are the
same pointer. `1EE0h` is a one-shot request, cleared right after the call.

004cce50 is `__thiscall`, ECX = game, no stack arguments, `RET` at 004cd0e6, with an SEH frame.
It allocates 74h bytes, constructs the object with 00460b80 and stores it in `game+38h`; builds a
pooled string from the mission path at `[game+5FCh]+90Ch`, truncates it at its first `.`
(00ce3a70) through 00467cf0/`BSP_NativeString_Substring`, appends `.ema` (00ce7838) and hands the
result to 004602c0. Then:

- `[game+38h]+60h == 5` (the failure state): 00b72030, a walk of the listener list at
  `DAT_00e188b4` calling each node's `vtable[0]`, then 0045d930 and `free`. The field
  `game+38h` is **not** cleared on this path.
- otherwise: 0045db40 and `004cc460(2Dh, 0)`, the switch to `INTF_ENGINEMOVIE`.

`game+38h` is the same object 004c6b20 advances in the timing block
(`docs/GAME_FRAME_CONTROL.md`), which reads it as cutscene playback and tears it down on action 3.
So the latch means "start this mission's in-engine cutscene".

### The interface id table

006881f0 and 00689030 print `INTF_*` names by indexing the pointer table at **00e08cd8** with the
id at `menu+20h`, which decodes every id this packet meets. Read back from the image:

| Id | Name |
| --- | --- |
| 00h | `INTF_NONE` |
| 17h | `INTF_MULTIINGAME` |
| 29h | `INTF_FREECAMERA` |
| 2Dh | `INTF_ENGINEMOVIE` |
| 31h | `INTF_SHIPYARD` |
| 32h | `INTF_AIRBASE` |

## The pause gate, 004e5153..004e5211

### The eliminated-player arm

```
004e5153  if (game+1FE4h != 0) {
004e515c    i = game+18ECh;
004e5162    if (game[18CCh + i*4] != 0) {
004e5175      if ((int16)(slot+10h) <= 0) goto 004e525e;   // clear of both branches
            }
          }
```

`game+18CCh` is the eight-slot local-player array and `game+18ECh` its selector, established by
004c6e50 and 007713a0 in `docs/GAME_FRAME_CONTROL.md`. The word at `slot+10h` is read as signed
and is **not identified**: no other reader of that offset was found within this packet's budget.
The jump lands past both branches, so on such a frame the pause menu is not offered and the
interface-only update does not run, while the award trackers and the world tick still do.

### The 4Bh chain and why it is a suppression

```
004e5180  if (!pressed(4Bh))            goto shared;
004e518d  if (game+634h != 0)           goto shared;
004e5196  n = 004f7740();
004e519d  if (n > EDI /* 1 */)          goto step4;
004e51a5  if (!0068a140([00e198c4]))    goto shared;
step4:
004e51b4  if ([00e188a8]+19C4h != 0 && !006529e0([[00e198c4]+BCh])) goto shared;
004e51d2  if (!004bfe50())              goto 004e5242;   // interface-only, pause test skipped
          /* fall through */
shared:
004e51dd  if (!pressed(1))              goto 004e5242;
004e51e9  if (00425d10()+25Ch != 0)     goto 004e5242;
004e51f7  if (00425d10()+188h != 0)     goto 004e5242;
004e5205  if (00425d10()+218h != 0)     goto 004e5242;
          pause branch at 004e5213
```

The chain reads like a second route to the pause menu and is not one. Every failure inside it
lands on the same 004e51dd test the not-pressed case reaches, and when the chain holds completely
it *falls through* to that test as well. Its only effect is the branch at 004e51db: with the
chain satisfied and 004bfe50 reporting false, control jumps straight to the interface-only branch
and action 1 is never queried. The reconstruction states this directly rather than transcribing
the fall-through, and the one added test case pins the inversion.

`00425d10` is called three times, once per field, with the object re-fetched between calls.
Action index 1 is the same record OnMove phase 15 resets every frame the timed-action set is
non-empty (`docs/GAME_INPUT_TICK.md` step 2), so a script can suppress pause by keeping that set
populated. 004c43c0 is the reviewed rising-edge test; the ECX both sites set is ignored by it.

### 004f7740, the level probe

No arguments, `RET`. Five checked vectors of 4-byte elements sit at 00e18cfc, 00e18d0c, 00e18d1c,
00e18d2c and 00e18d3c. The walk starts at the last one with a counter of 5, steps back 10h at a
time and returns the counter at the first non-empty vector, or 0 once it passes 00e18cfc.
004f7620 walks the same five with the same 1..5 numbering, resolves each element through the
object table at `&DAT_00e18b60` and latches the highest active level in `DAT_00e08310`, which is
what fixes the base and the direction. **What the levels rank is not established**; 0068c1f0
tests level 3 (00e18d1c/00e18d20) before its own 4Bh handling. The gate needs `level > 1` or a
base screen.

### 0068a140, the base-screen test

`__thiscall`, ECX = `DAT_00e198c4`, `RET`: `return id == 32h || id == 31h`, that is
`INTF_AIRBASE` or `INTF_SHIPYARD`. `DAT_00e198c4` is the in-mission interface manager; its
`+20h` is the active interface id, the same field 00689030 prints for the multiplayer menu
`DAT_00e198b4`.

### 004bfe50 and the game modes

No arguments, `RET` at 004bfec1; reads `DAT_00e188a8` itself. It inlines 004bca50:

```
mode = game+614h;
if (game+61Ch == 0 && game+1FE4h == 0 && mode != 9 && mode != 8) mode = 8;   // 004bca50
return mode in {4,5,6,7}
    && 006529e0([[00e198c4]+BCh])          // byte +4h
    && 006529f0([[00e198c4]+BCh]);         // int +5Ch == 1
```

Modes 4..7 are exactly the set 0068c1f0 opens with, so it is one mode set, not two; 8 is the
default when nothing forces a mode and there is no session. **Which real game type each of 4..7
names is not established.** The object at `[00e198c4]+BCh` carries byte `+4h` (006529e0), byte
`+8h` (00652a30 get, 00652a50 set), byte `+9h` (00652a20), byte `+0Ah` and int `+5Ch`; 0068c1f0
drives `+8h` around its free-camera handling, so this reads as the spectator/free-camera
controller.

## Branch A, the pause menu (004e5213..004e5240)

```
004e5213  if (game+634h != 0 && [[00e198c4]+A8h]+8h != 0) { 0054e440(); 004db030(game); }
004e5239  004db030(game);
```

`game+634h` is read a **second** time here, after the action tests. 0054e440 is owned by
`agent/game-frontend-states`; the map records it as the front-end unit-button list
(`BASICSHIP`, `BASICPLANE`, `BASICSUB`, …). The extra 004db030 makes the pair a double toggle,
which leaves the HUD flag where it started and is how leaving the free camera and opening the
pause menu are done in one frame.

### 004db030, the pause toggle

`__thiscall`, ECX = game, no stack arguments, `RET`. It opens with
`FLDZ; push ecx; fstp [esp]` and calls `BSP_InputManager_GetSingleton()` then
`BSP_InputManager_Update(0.0f)` — the input tick is re-run with a **zero** delta so no hold timer
advances across the transition, the same trick 004db920 uses. Ghidra renders the zero as an
integer `uVar1`.

Single-player, `game+1FE4h == 0` (004db0d6):

1. `004cd0f0(game, game+634h == 0, 0, 1)` — flip the HUD flag, with `allow_simulation = 0`, so
   hiding it also clears `game+635h` and closes the gate on the next frame.
2. Now hidden (`game+634h == 0` after the flip is false) and either `[[00e198c4]+A8h]+8h == 0` or
   `+9h != 0`: clear `[[00e198c4]+DCh]+4h`, call 005fd440 with ECX = `[00e198c4]+D8h`, then
   `004d95f0(game, 0)`, and clear `+9h` if it was set.
3. Otherwise: with `game+635h` set, write `[[00e198c4]+DCh]+4h = 1`; else call 005ff0d0 with
   ECX = `[00e198c4]+D8h`. Then `004d95f0(game, 1)` and set `[[00e198c4]+A8h]+9h = 1`.

Multiplayer, `game+1FE4h != 0` (004db045):

1. Only when `game+218Ch` is set does it call 004cd0f0 at all, with the same toggle arguments.
2. `[00e198b4]+3Ch == 0` (the multiplayer menu is not up): with `game+635h` set, write
   `[[00e198c4]+DCh]+4h = 1`; else `00689030(DAT_00e198b4)`. Then `004d95f0(game, 1)`.
3. `[00e198b4]+3Ch != 0`: clear `[[00e198c4]+DCh]+4h`, `006882a0(DAT_00e198b4)`,
   `004d95f0(game, 0)`.

00689030 opens the screen: `0068ab80(0,0)`, the manager virtual `+0Ch`,
`BSP_GuiManager_GetOrCreate(1)`, `BSP_GuiManager_SetEnabled(1)`, the trace
`GVMultiMenu::PushRequestInterface() req:%s set:%s` with the current id and `INTF_MULTIINGAME`,
00688830, then `004cc460(17h, 0)` — 17h being `INTF_MULTIINGAME`, which is what ties the id
table to the constant. 006882a0 is the mirror: `BSP_GuiManager_GetOrCreate(0)`,
`BSP_GuiManager_SetEnabled(0)`, 006881f0 (`GVMultiMenu::PopRequestInterface() req:%s`, which
restores the id under the top of the stack at `menu+60h`), the menu virtual `+0Ch` and the
manager virtual `+8h`. 004d95f0 is the shared "set paused" call on both sides and is left
unanalysed here.

## Branch B, the interface-only update (004e5242..004e525c)

```
004e5242  ECX = [00e198c4];
          if (ECX != 0 && ECX+3Ch != 0) 0068c1f0(ECX);
004e5257  004c40f0(game);
```

004c40f0 is owned by `agent/game-session-polls`; it is the same callee the failed-gate fallback
at 004e53b4 runs, reached from a second site.

### 0068c1f0, the in-mission interface update

`__thiscall`, ECX = `DAT_00e198c4`, no stack arguments, `RET` at 0068cc68. Naming it after the
audio environment understates it: the environment switch is only its tail. The body, in order:

1. Guarded by `004bca50(game) in {4,5,6,7}`: the spectator/free-camera block. It walks the unit
   list at `[[game+19CCh]+58h]`, matches the active local player's unit id at `slot+28h`, and
   moves the free-camera flag `+8h` of `[00e198c4]+BCh` through 00652a50, 0068c0b0, 00927cc0,
   0068b3f0 and 00647300.
2. `DAT_00e1ae80` or action 5Ah: switches to `INTF_FREECAMERA` through `004cc460(29h,0)`, and
   `004cc460(20h, …)` on the way back.
3. A run of rising-edge tests on actions 89h, 8Ah, C1h, D6h, FBh, FCh, 108h and 4Bh. The 4Bh arm
   at 0068cc49 needs `[menu+C0h]+4h`, `!00611750()` and a non-empty level-3 vector, and runs
   004f8670, 004d8b70 and 0068aa40 — the same action the gate reads, handled here for a
   different purpose.
4. The tail, 0068cb79..0068cc55, chooses one environment name and passes it to 00a7b710, a
   routine in the FMOD/sound segment with this as its only caller:
   - `Cockpit` when `[menu+6Ch]+4h` is set and `00604f30() == 1`, or `[menu+84h]+4h` is set and
     `[menu+84h]+74h == 2`.
   - otherwise `Air` (the pointer at 00ce9b1c) when the camera at `game+19FCh` is strictly above
     the water height `0078cf20([cam+120h], [cam+124h])` **and** the device flag at 0068cc0a is
     clear, else `Underwater`. The two terms are combined with a byte `AND`, not a short circuit.
   Then `004c1e90(0Ch)` and `00427190(0Ch)`.

## Fields the branches write, and who reads them later

| Field | Written by | Read by |
| --- | --- | --- |
| `game+634h` | 004cd0f0 only | the gate at 004e50b0 and 004e5213, phase 6 (004e4b07), phase 15 (004e4e6c), 004db030, 004c6b20 |
| `game+635h` | 004cd0f0 only | the gate at 004e50b9, 004db030 twice, 004c6b20 |
| `game+21F0h` | zeroed by 004cd0f0 on the hidden path | every simulation callee of phases 19-21 |
| `game+1EE0h` | cleared by the gate at 004e514c | nothing else in OnMove |
| `game+38h` | 004cce50 | 004c6b20 in the timing block |
| `[[00e198c4]+DCh]+4h` | 004db030, both paths | request 12h in the drain (004e4430) also writes it |
| `[[00e198c4]+A8h]+9h` | 004db030 | 004db030 on the next toggle, and the gate's `+8h` sibling |
| `00e18d3c` level vectors | outside this packet | 004f7740 here, 004f7620, 0068c1f0 |

## Reconstruction

`include/bsp/simulation_gate.hpp` and `src/simulation_gate.cpp`, registered in
`cmake/startup.cmake`. The entry predicate, the layer-visibility decision, the mode resolution,
the level probe, the audio-environment choice and the branch decision are pure functions;
`run_simulation_gate` is the sequence over `SimulationGateHost`, one virtual per native call site,
in the style of `bsp::run_application_frame`. The host is queried lazily so the call **counts**
match the native short circuits, including the two separate reads of `game+634h` and the three
separate 00425d10 fetches. `GameStateId` is reused from `include/bsp/game_frame_control.hpp`
rather than redeclared.

Not modelled: the profiler bracket and the one-shot label around the block, the SEH frames of
004cd0f0 and 004cce50, the pooled-string arithmetic of 004cce50, and everything inside 0068c1f0
before its tail.

## State reached

| Address | State |
| --- | --- |
| 004e50b0..004e525e (the gate) | reconstructed, build-tested |
| 004cd0f0 | analysed; the flag pair and the layer decision reconstructed, build-tested |
| 004c40a0 | analysed (listing, for the two dropped ECX values) |
| 004cce50 | analysed |
| 004f7740 | reconstructed, build-tested |
| 0068a140 | reconstructed, build-tested |
| 004bfe50, 004bca50 | reconstructed, build-tested |
| 004db030 | analysed |
| 0068c1f0 | analysed; the audio-environment tail reconstructed, build-tested |
| 006881f0, 006882a0, 00689030 | analysed |
| 004f7620, 006529e0, 006529f0, 00652a20, 00652a30, 00652a50 | exported, read as corroboration |

Nothing here is game-validated and none of it is a drop-in binary replacement.

## Uncertainties and what remains

1. `slot+10h`, the signed word that skips both branches, has no second reader in this packet.
   Until one is found, "eliminated player" is a reading of the comparison, not of the field.
2. What the five levels at 00e18cfc rank. 004f7620 resolves their elements through
   `&DAT_00e18b60`, so the answer is in whatever fills those vectors; that producer was not found.
3. Which real game type each of modes 4..7 is, and what mode 9 is. `game+614h` and `game+61Ch`
   have no writer in this packet.
4. `game+7184h`, the suspend byte: read here and at 004e5054, written nowhere this packet saw.
   `docs/GAME_ON_MOVE_MAP.md` pairs it with the mission-result object at `+7188h`.
5. 004d95f0, the shared "set paused" call of both 004db030 paths, is unanalysed, as are 005fd440,
   005ff0d0 and 0068c0b0.
6. The 4Bh action's name. Both this gate and 0068c1f0 read it, with different guards; the input
   settings layer would have to supply the binding.

## Tooling notes

- `python tools/bsp.py strings <addr>` takes a search string, not an address; the usage line
  reads as though it took either. There is no address-to-referenced-strings query, so string
  evidence had to come from `ghidra bytes` on the literal.
- Ghidra dropped the ECX of 004c40dd (`game+30h`) and rendered the `FLDZ`-sourced 0.0f argument of
  004db030 as an integer, so both call sites had to be read from the listing.
- Every function in this packet still has an `undefined FUN_xxxx(void)` prototype in Ghidra, so
  `ghidra proto` supplies the body range but not the convention; the `RET` operands quoted above
  were read with `disasm-raw` at the body ends.
