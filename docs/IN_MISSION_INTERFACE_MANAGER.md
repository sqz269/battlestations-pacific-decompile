# The in-mission interface manager (packet `in_mission_interface_manager`)

Addresses: 00E198C4, 0068A990, 0068CC70, 0068ACA0, 0068BC60, 0068B630, 0068AB80, 00689FA0,
00689FC0, 00689FE0, 0068BF60, 00646040, 004E0452, 004DFB70, 004DA780, 004DA650, 004F71D0,
00CF7A60, 00E08CD8, 0068B390.

`00E198C4` is the fifth instance of the front-end manager base `00684E10`
(`docs/FRONTEND_MANAGERS.md`), and the only one that exists solely while a mission is loaded. It
owns 42 HUD screens and it is the object that turns an in-session interface id into a level-1
screen set. Everything below is a hypothesis reconstructed from the listing; no symbol was
recovered from the image.

## Where it is created and destroyed

`BSP_Game_LoadMissionScene` (004DFB70) builds it at 004E0452..004E0480:

```
004e0452  push 108h ; call 00BF681B          ; operator new
004e046c  ECX = EAX ; call 0068A990          ; constructor
004e0480  [00E198C4] = EAX
```

So the object is **108h bytes** and is created at mission-scene load, not at title init and not by
`BSP_Game_EnterFrontEndShell`. Ghidra's cross-reference list for 00E198C4 does not report this
store; it was found by scanning for the encoding `a3 c4 98 e1 00`. The two writes Ghidra does
report, 004DA6B3 (`BSP_FrontEnd_DestroyManagers`) and 004DAB94 (004DA780, the mission teardown),
are both `= 0` after a virtual `+00h` call. 0068CCFE inside Init writes the global a second time
with the same value.

`004DA780`, the mission teardown, also clears the interface lock: at 004DAB53 it runs
`if ([00E198C4] != 0) [00E19894] = 0` while unwinding `game+1ED4h`, and at 004DAB82 it calls the
manager's virtual `+00h` with the delete flag and nulls the global. That is the packet's link
between the manager and `front_end_interface_lock`.

## The constructor, 0068A990, `__thiscall(this)` returning this in EAX, RET at 0068A9BE

Calls `00684E10` for the 40h-byte base, stores vtable 00CF7A60, and clears exactly five fields:
`+54h`, `+ECh`, `+FDh` (byte), `+100h` and `+104h`. Every other pointer field is written
unconditionally by Init, which is why the constructor does not clear them.

## The vtable, 00CF7A60

| Slot | Address | Meaning |
| --- | --- | --- |
| +00h | 0068BC60 | scalar deleting destructor, `__thiscall(this, byte flags)`, RET 4 at 0068BC7B |
| +04h | 0068CC70 | `Init`, `__thiscall(this)`, RET at 0068D759 |
| +08h | 00684700 | `Activate`, the base body |
| +0Ch | 00683AA0 | `Deactivate`, the base body |
| +10h | 0068ACA0 | `ApplyPendingInterface` override, `__thiscall(this, int id, void* payload)` -> bool in AL, RET 8 |

The base `Activate` compares `this` against `[00E198C4]` at 0068477E and skips the request replay
for exactly this manager. Combined with the lock at 00E19894, that is why raising the in-mission
manager never re-pushes the interface it last showed.

## Layout, 108h bytes

| Offset | Field | Evidence |
| --- | --- | --- |
| +00h..+3Fh | the shared base | 00684E10 |
| +40h..+E8h | 42 HUD screen pointers | 0068CC70 assigns each; 0068B630 collects the same 42 |
| +D0h | never written | absent from both Init and the destructor's collection list |
| +ECh | zeroed by the constructor, never read | `param_1[3Bh] = 0` at 0068A9AE |
| +FDh | byte, cleared by the constructor, never read | 0068A9B6 |
| +100h | ambient sound source of the unit driving the HUD | assigned from `unit->[538h]+10Ch` by 004E7BB0 at 0068B2DE; compared at 0068B2A7 |
| +104h | the playing instance of that sound | stopped through virtual `+8h(0)` and released by 0054D510 at 0068AF8F |

Both +100h and +104h are refcounted: the destructor releases each with `InterlockedDecrement`
followed by virtual `+00h`.

## Init, 0068CC70

1. Loads the texture atlas `interface/Textures/game.ats`. The installed tree
   (`interface/Textures/`) has no `game.ats`; it has `game_dxt1.ats`, `game_dxt5_1.ats` and
   `game_dxt5_2.ats`, so the literal is a stem the loader expands by compression format. The
   destructor unloads the same literal through 00AEFA30.
2. `[00E198C4] = this` at 0068CCFE, then the base `Init` 00683A90.
3. Constructs the 42 screens, each with `operator new(size)` followed by its constructor and then
   its virtual `+10h`, which is `BSP_FrontEndScreen_Register` (004F71D0): that calls the screen's
   virtual `+00h` for a registry slot id and stores the screen at `[00E18B60 + id*4]`.
4. `004CC460(20h, 0)` — pushes `INTF_SCENE3D` as the first request — then 00644220(0).

### The 42 screens

`registry slot` is what the screen's virtual `+00h` returns and is the id the level-1 lists below
use. Constructor `inline` means Init builds the object in place from 004F7180 plus two vtable
stores, so there is no separate function to name.

| Offset | Slot | Constructor | Vtable | Size | Init order |
| --- | --- | --- | --- | --- | --- |
| +40h | 44h | 0068BF60 | 00CF5A9C | 174h | 0 |
| +44h | 27h | 0068A8A0 | 00CF7A38 | 34h | 1 |
| +48h | 4Dh | 00640C80 | 00CF5950 | B0h | 2 |
| +4Ch | 26h | 0051E950 | 00CEC9B0 | 50h | 3 |
| +50h | 2Eh | 005472F0 | 00CEDF34 | 114h | 4 |
| +54h | 4Ch | 005AAEF0 | 00CF0134 | 614h | 5 |
| +58h | 35h | 005C0810 | 00CF1404 | 1B0h | 6 |
| +5Ch | 2Ah | 00538E30 | 00CED6FC | 1ACh | 7 |
| +60h | 39h | 005EE440 | 00CF2D20 | 74h | 8 |
| +64h | 3Bh | 005F9440 | 00CF3B7C | 90h | 9 |
| +68h | 3Eh | 00608E60 | 00CF4544 | 194h | 10 |
| +6Ch | 3Fh | 00606470 | 00CF43AC | 110h | 11 |
| +70h | 25h | inline | 00CEC590 | 30h | 12 |
| +74h | 41h | 0060CAE0 | 00CF4A7C | 2Ch | 13 |
| +78h | 45h | 0064B780 | 00CF5E30 | 1B0h | 14 |
| +7Ch | 46h | inline | 00CF7978 | 38h | 15 |
| +80h | 4Ah | inline | 00CF79F8 | 30h | 16 |
| +84h | 47h | 00650B00 | 00CF639C | 7Ch | 17 |
| +88h | 48h | 00650140 | 00CF61D4 | 24h | 18 |
| +8Ch | 24h | inline | 00CF7938 | 58h | 19 |
| +90h | 2Bh | 00540BA0 | 00CEDCA8 | 90h | 20 |
| +94h | 2Ch | 0054F8C0 | 00CEE408 | 28h | 21 |
| +98h | 36h | 005CE610 | 00CF1968 | 84h | 22 |
| +9Ch | 37h | 005CC120 | 00CF17A8 | 38h | 23 |
| +A0h | 38h | 005CB0A0 | 00CF17D0 | 20h | 24 |
| +A4h | 33h | 005BA7B0 | 00CF0ED8 | E8h | 25 |
| +A8h | 34h | 0054D680 | 00CEE290 | 7Ch | 26 |
| +ACh | 23h | 005178B0 | 00CEC194 | 2Ch | 27 |
| +B0h | 2Fh | inline | 00CF79B8 | 24h | 28 |
| +B4h | 49h | 0067BE30 | 00CF6D68 | Ch | 29 |
| +CCh | 29h | 00525A90 | 00CECCF8 | F0h | 30 |
| +D4h | 5Ah | 00636D90 | 00CF569C | 34h | 31 |
| +D8h | 3Ch | 005FFCB0 | 00CF3EF4 | 164h | 32 |
| +DCh | 3Dh | 00604780 | 00CF41A0 | Ch | 33 |
| +E0h | 5Eh | 0060FD90 | 00CF4B98 | 48h | 34 |
| +E4h | 19h | 005D3540 | 00CF1D70 | 28h | 35 |
| +E8h | 32h | 00565D40 | 00CEEBAC | 1Ch | 36 |
| +B8h | 43h | 00643DA0 | 00CF5998 | 18h | 37 |
| +BCh | 4Eh | 006735D0 | 00CF6B44 | 480h | 38 |
| +C0h | 4Fh | 0061A480 | 00CF4DEC | 270h | 39 |
| +C4h | 50h | 006821C0 | 00CF7184 | 7Ch | 40 |
| +C8h | 51h | 0052B030 | 00CED0AC | 2Ch | 41 |

For +54h and +5Ch the object's first vtable is written twice; the final value (00CF0134,
00CED6FC) is a `vtordisp`-style entry four bytes into the neighbouring table, and its slot 0 is
`005A8EE0`/`00538CD0`, both `mov eax, <id>; ret`.

Only two screens carry recognisable strings, and both corroborate their slot: +A4h (slot 33h)
references `sound/messages/warning/warning_{enemy,ship,land,sub,fighter}_radio.fsb` and
`Scripts/datatables/DialogGlobals.lua`, and +98h (slot 36h, the `INTF_MOVIECAMERA` screen)
references `Scripts\datatables\MovieCamera.lua`. The installed `interface/gui_*.lua` set has 47
pages whose names line up suggestively with these 42 screens (`gui_freecam`, `gui_limbo`,
`gui_warning`, `gui_plane_spawn`, `gui_ship`, `gui_sub`, `gui_periscope`, `gui_spectator`), but
no screen constructor or vtable slot reachable from here names a page, so that correlation is
**not established**.

## ApplyPendingInterface, 0068ACA0

`__thiscall(this, int interface_id, void* payload)` returning a bool in AL, RET 8 at 0068ACE1 and
0068AFA8. **Ghidra has no function here**; the body runs 0068ACA0..0068B38F (last instruction
0068B38B) and the jump table sits immediately after it at 0068B390. The prologue saves EBX, ESI
and EDI; EBP is pushed later, at 0068AD3D, after the early-out, and popped at 0068AF98.

The prologue, before the switch:

1. `00684600(this, id, payload)`; a false result (the lock 00E19894 rejected the request) returns
   false immediately.
2. If `payload && payload->[5Dh]`: the limbo screen at +E8h takes `payload->[70h]` through
   00565FB0 and the id is **forced to 34h `INTF_LIMBO`**, whatever was asked for. The base record
   keeps the original id.
3. `[this+40h]->00646040(id, payload)` — the HUD root screen (registry slot 44h) is told the new
   interface for every id.
4. `if (game->[1ED4h]) 0042A930()`.
5. `0068AB80(this, id in {29h,2Bh,2Ch,2Dh}, 1)`.
6. `switch (id - 20h)` through the 16h-entry jump table at 0068B390; out of range takes the same
   arm as 21h.

### The id map

Screen ids are level-1 registry slots passed to `004F8530(ids..., 0)`; input contexts are level-1
contexts passed to `004D8A50(game, ids..., 0)`. Both are listed in argument order, which is the
reverse of the push order. "clears" means the empty list, which empties level 1;
"untouched" means the arm never calls the setter.

| Id | Name | Level-1 screens | Level-1 input contexts | Extra |
| --- | --- | --- | --- | --- |
| 20h | INTF_SCENE3D | see below | see below | unit-type refinement |
| 21h | INTF_MAP | clears | untouched | shares the default arm 0068B230 |
| 22h | INTF_PLANE | 29,49,44,27,4D,3E,3F,35,50 | 0A,4,11,12,0C,0B | +6Ch, +68h take the unit |
| 23h | INTF_PLANEBOMBER | 29,49,44,27,4D,3E,25,26,2E,35,50 | 9,4,11,12,0C,0B | +70h, +68h |
| 24h | INTF_PLANESPAWN | 44,27,4D,50,41,35 | 17,4,11,12,0C,0B | +74h |
| 25h | INTF_CAPTAIN | 29,49,44,27,4D,45,46,26,2E,35,50 | 6,4,11,12,0C,0B via 00689FA0 | +7Ch, +78h |
| 26h | INTF_BOMBVIEW | 27,4D,24 | 5,4,0A | see below |
| 27h | INTF_TBOATHELMSMAN | 29,49,44,27,4D,45,4A,26,2E,35,50 | 6,4,11,12,0C,0B via 00689FE0 | +80h, +78h |
| 28h | INTF_SUBMARINE | 29,49,44,27,4D,45,47,48,2E,35,50 | 7,4,11,12,0C,0B via 00689FC0 | +84h, +78h |
| 29h | INTF_FREECAMERA | 2B | 2 | |
| 2Ah | INTF_IDLECAMERA | 2C,29,49 | 4,11,12,0C,0B | |
| 2Bh | INTF_MOVIECAMERA | 36 | 3 | |
| 2Ch | INTF_MOVIECAMERANEW | 37 | 3 | the id the lock exempts |
| 2Dh | INTF_ENGINEMOVIECAMERA | 38 | 3 | |
| 2Eh | INTF_AIRFIELD | untouched | untouched | jumps straight to the tail |
| 2Fh | INTF_COMMANDBUILDING | 29,49,44,27,4D,2F,26,2E,35,50 | 6,4,11,12,0C,0B | +B0h |
| 30h | INTF_SHIPYARD_STAREDUMB | untouched | untouched | |
| 31h | INTF_SHIPYARD | untouched | untouched | |
| 32h | INTF_AIRBASE | untouched | untouched | |
| 33h | INTF_LAUNCHLANDING | 29,49,44,27,4D,35 | 16,11,12 | |
| 34h | INTF_LIMBO | 32 | 13 | |
| 35h | INTF_SUPPORTMANAGER | untouched | untouched | |

**Every screen id in every list is one of the 42 the manager owns.** That closure is the packet's
strongest single check; it is asserted in `tests/math_tests.cpp`.

`00689FA0`, `00689FC0` and `00689FE0` are `__cdecl(void)`, RET: three canned level-1
input-context lists, called instead of an inline `004D8A50` for 25h, 28h and 27h.

The bomb-view arm is the only one whose per-screen calls straddle the setters:
`[this+8Ch]->0067B4D0(payload, [this+40h]->00644230())`, then the two setters, then
`[this+8Ch]+34h = [this+40h]->00644230()`.

### The 20h refinement, 0068AE0B

With a payload, the arm asks `payload->vtable[5Ch](type)` in this order and re-enters its own
virtual `+10h` with the first match:

| Type code | Redispatched id |
| --- | --- |
| 0Eh | 27h INTF_TBOATHELMSMAN |
| 08h | 28h INTF_SUBMARINE |
| 06h | 25h INTF_CAPTAIN |
| 0Fh | 22h INTF_PLANE when 007BB9A0(unit) is true, otherwise 24h INTF_PLANESPAWN |
| 45h | 2Eh INTF_AIRFIELD |
| 1Ch | 2Fh INTF_COMMANDBUILDING |
| 46h | 30h INTF_SHIPYARD_STAREDUMB |
| 18h | 20h again, with `unit->[3D0h]` as the payload |

No match falls into the shared tail. What 007BB9A0 tests is not established; "already airborne"
fits the 22h/24h split but is an inference.

With a null payload (0068AF2F) the arm splits on `game+1FE4h`: zero installs the bare scene set
{29h,49h,44h,35h} with contexts {4,11h,12h,0Ch,0Bh}; non-zero redispatches 2Ah `INTF_IDLECAMERA`
with the same null payload. `game+1FE4h` is the multiplayer discriminator the mission teardown
also branches on.

### The tail, 0068B23A: the unit ambience

Reached by every arm except the null-payload 20h arm, which goes straight to the stop path.

```
if (!payload || !payload->vtable[5Ch](6) || id in {29h,2Bh,2Ch,2Dh,34h}) -> stop
if ([this+104h] && [this+100h] == payload->[538h]->[10Ch]) -> done
if ([this+104h]) stop it
[this+100h] = payload->[538h]->[10Ch]        ; 004E7BB0, refcounted
if ([this+100h]) build a "3DEffect"/"Normal" sound and store it at [this+104h]
```

Type code 6 is the same code the 20h arm maps to `INTF_CAPTAIN`, so the ambience belongs to
surface ships only, and the four camera modes plus limbo silence it. The two literals are
`3DEffect` (00CE7860) and `Normal` (00CECD20); the sound is built through 00A7B0A0, 00A7ACF0 and
00A7E490 on the sound manager at 00F8BBD8 and stored with 0054D4C0.

The stop path at 0068AF68 releases `+104h` only; `+100h` keeps the stale source, which is safe
because the "already playing" test requires `+104h` as well.

## 0068AB80, the level-2 and level-3 collapse

`__thiscall(this, char clear_level2, char run_extra_hook)`, RET 8 at 0068AC21.

- When the level-3 screen vector (00E18D18, head/tail 00E18D1C/00E18D20) is non-empty: if
  `[00E198C4]+BCh`'s byte `+30h` is set, set `[this+BCh]+0Ah`; then `004F8670(0)`,
  `004D8B70(game, 0)` and `0068AA40(004B4B00())`.
- If `run_extra_hook`: `0051E8E0()`. ApplyPendingInterface always passes 1.
- If `clear_level2` and `[this+54h]+4h` (the requested byte of registry slot 4Ch) is set:
  `004F85D0(0)`, `004D8AE0(game, 0)` and `0068AA40(004B4B00())`.

So entering a camera interface takes down the level-2 overlay only while slot 4Ch is asking to be
on screen, and any interface change takes down level 3 unconditionally.

## Teardown

`0068BC60` is the scalar deleting destructor; the body is `0068B630`, `__thiscall(this)`, RET at
0068BC5F:

1. Restore vtable 00CF7A60.
2. Stop and release `+104h`.
3. Collect all 42 screens into a local `std::vector` through 00686C20 (which de-duplicates), then
   for each: if `screen->[5h]` call its virtual `+1Ch`, clear `+4h` and `+5h`, and run
   `BSP_FrontEndScreen_CommitVisibility`.
4. Second pass over the same vector: `screen->vtable[0Ch](1)`, the screens' own deleting
   destructors, which also unregister them from 00E18B60 through 004F71A0.
5. `[00E198C4] = 0`, unload `interface/Textures/game.ats`, `BSP_MenuCommandScreen_GetSingleton(1)`
   then 00530630(1).
6. Free the vector, release `+104h` and `+100h`, then `BSP_FrontEndManager_DestroyBase` (00684FA0).

`python tools/bsp.py ghidra flow 0068b630` reports one fall-through gap after the `_free` at
0068BBD6 (0068BBDB..0068BBDE, 3 bytes), which is why the export shows step 6 as unreachable code
after an early `return`. The gap was **not** repaired; the reconstruction follows the real
control flow.

## Corrections to neighbouring packets

- The 00E08CD8 name table has **36h entries** (00E08CD8..00E08DAF, the float block at 00E08DB0
  follows), not the 45 that `kInterfaceNameCount` in `include/bsp/frontend_managers.hpp` records.
  Ids 2Dh..35h are the nine that count drops, so `front_end_interface_name` returns nullptr for
  them today. `include/bsp/ingame_interface.hpp` supplies 20h..35h separately rather than editing
  a file this packet does not own.
- Entry 2Dh reads `INTF_ENGINEMOVIECAMERA`; `kInterfaceEngineMovie` in
  `include/bsp/simulation_gate.hpp` is named one word short.
- The packet brief labelled 004CD0F0 `BSP_InGameInterface_Update`. It is
  `BSP_Game_SetCinematicMode` in the ledger and is already reconstructed by
  `game_simulation_gate`; the manager's real per-frame tick is 0068C1F0, already named
  `BSP_InGameInterface_Update` and covered by `docs/GAME_SIMULATION_GATE.md` and
  `docs/GAME_ON_MOVE_MAP.md`. Neither address was re-analysed here, and neither was claimed.
- `docs/FRONTEND_SCREEN_SETS.md` lists "trace the callers of 004F8530" as an open item. This
  packet answers it for level 1: the sole in-session caller is 0068ACA0, and level 1 is the HUD.

## Calling conventions and RET sizes

| Address | Convention | RET |
| --- | --- | --- |
| 0068A990 | `__thiscall(this)` -> this in EAX | RET at 0068A9BE |
| 0068CC70 | `__thiscall(this)` | RET at 0068D759 |
| 0068ACA0 | `__thiscall(this, int id, void* payload)` -> bool in AL | RET 8 at 0068ACE1, 0068AFA8 |
| 0068BC60 | `__thiscall(this, byte flags)` -> this | RET 4 at 0068BC7B |
| 0068B630 | `__thiscall(this)` | RET at 0068BC5F |
| 0068AB80 | `__thiscall(this, char clear_level2, char run_extra_hook)` | RET 8 at 0068AC21 |
| 00689FA0, 00689FC0, 00689FE0 | `__cdecl(void)` | RET at 00689FBC, 00689FDC, 00689FFC |
| 004F71D0 | `__thiscall(this)` | RET at 004F71E1 |

## Callers and callees

- Callers of the manager: 004DFB70 (create), 0068CC70 (Init), 004DA780 and 004DA650 (destroy),
  0068C1F0 (per-frame), 0068A140 (base-screen test), 004E5252 (the simulation gate),
  006910D8/00691736/00691AA3/00691B2A (`BSP_AwardTracker_TriggerHint`), 004CD126, 004D2C57.
- 0068ACA0's callees outside this packet: 00684600, 00565FB0, 00646040, 0042A930, 004F8530,
  004D8A50, 00519B00, 00609390, 00608D50, 0060CF90, 0067C1A0, 0064D590, 00651760, 0064DA40,
  0067B4D0, 00644230, 005213D0, 004E7BB0, 0054D510, 0054D4C0, 00A7B0A0, 00A7ACF0, 00A7E490.

## Uncertainties

- `+ECh` and `+FDh` are cleared by the constructor and read by nothing traced here.
- `+D0h` is inside the screen run but is never written and never collected. It may be a member of
  a different type, or a slot for a screen the shipped build dropped.
- What 007BB9A0 tests, and therefore why 0Fh splits into `INTF_PLANE` and `INTF_PLANESPAWN`.
- The `interface/gui_*.lua` page behind each registry slot. Registration carries only the numeric
  slot; the page name must be bound by the GUI layer manager from a string this packet did not
  reach.
- `[this+BCh]+0Ah` and `+30h` in 0068AB80: two flags on the 480h-byte camera screen whose meaning
  is not established. `docs/GAME_SIMULATION_GATE.md` reads `+8h` of the same object as the
  free-camera flag.
- Whether the registry slots this manager never publishes (2Ah, 23h, 33h, 34h, 39h, 3Bh, 3Ch,
  3Dh, 43h, 49h at level 1 only, 4Ch, 4Eh, 4Fh, 51h, 5Ah, 5Eh, 19h) are self-managed screens
  (virtual `+4h` true) or are published by another level.

## Follow-up packets

- `in_mission_hud_root_screen` — 0068BF60, 00646040, 00644230, 00644220, registry slot 44h.
  Contract: the screen every interface change notifies, its layout and what 00646040 does with
  the id and the unit.
- `in_mission_camera_screen` — 006735D0 and registry slot 4Eh, 480h bytes, plus 0068AA40 and
  004B4B00. Contract: the flags at +8h, +0Ah and +30h and the free-camera path
  `docs/GAME_SIMULATION_GATE.md` leaves open.
- `front_end_screen_self_management` — 004F7570 and 004F7580 overrides across the 95 registry
  slots. Contract: which screens are exempt and which occlude, which is the gap
  `docs/FRONTEND_SCREEN_SETS.md` names as its main one and which decides the 16 slots above.
- `in_mission_unit_type_codes` — 007BB9A0 and the `vtable[5Ch]` type codes 6, 8, 0Eh, 0Fh, 18h,
  1Ch, 45h, 46h. Contract: name the unit class each code selects.

## State reached

| Address | State |
| --- | --- |
| 0068ACA0 | exported (raw listing only), analyzed, reconstructed, build-tested |
| 0068A990, 0068CC70, 0068B630, 0068BC60, 0068AB80 | exported, analyzed; layout and behaviour reconstructed as data and predicates, build-tested |
| 00689FA0, 00689FC0, 00689FE0 | analyzed from raw bytes; reconstructed as data, build-tested |
| 004E0452..004E0480 | analyzed from the listing |
| 004DA780, 004DA650 | read only, not claimed |
| The 42 screen constructors and vtables | analyzed only far enough to read their registry slot |
| `interface/Textures/game.ats` | installed-file-checked: the literal is a stem, the tree has the `_dxt1`/`_dxt5_*` variants |
| `interface/gui_*.lua` | installed-file-checked: 47 pages exist, none tied to a slot |
| 004CD0F0, 0068C1F0 | not analysed here; already covered by `game_simulation_gate` |
