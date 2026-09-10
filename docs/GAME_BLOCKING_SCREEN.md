# The blocking-screen frame

Addresses: 00689cc0, 004db220, 00aa4f80, 00aa0e00, 00aa0e50, 00689d90, 00689c60, 00689e80,
00689c00, 00a91020, 00a90180, 00aa7ef0, 004e4b2a-004e4b8c, 00aa0f70, 00aa3910, 004d35d0

Packet `game_blocking_screen_frame`. Ghidra was read-only for this packet; the names below were
recorded in the ledger, not applied to the program. `docs/GAME_ON_MOVE_MAP.md` phase 7 is the
entry point, `docs/APP_INIT_FONTS_GUI.md` supplies the GUI manager layout, `docs/APP_INIT_INPUT.md`
the input device table and `docs/APP_INIT_GAME_ENTRY.md` the game states.

## What this path actually is

The map doc calls 00689cc0 a loading or modal blocking screen. It is neither. The object in
`DAT_00e198bc` is the **front-end attract screen**: its constructor loads the GUI layout named
`FE_attract` and its activate virtual starts `movies/PacificTheme.bik` on the movie player. The
gate is an idle countdown. After 45 seconds with no button down anywhere the screen goes up, the
whole simulation is skipped and the frame renders GUI only; the next button press takes it down.
That is why this is the smallest frame the game can run: nothing but the GUI manager, the render
kick and one countdown is involved.

## Frame slice 004e4b2a-004e4b8c

`BSP_Game_OnMove` is `__thiscall(GGame*, float rawDelta)` with `RET 4`; `EBP` is zeroed at
004e4a7e and used as the constant 0 for the rest of the body, and the incoming float lives at
`[ESP+0x48]`. The slice, from `local/onmove.txt`:

| Address | Instruction | Meaning |
| --- | --- | --- |
| 004e4b29 | `mov ecx,[00e198bc]` / `cmp ecx,ebp` / `je 004e4b8f` | no attract object, fall through |
| 004e4b33 | `fld [esp+0x48]` / `push ecx` / `fstp [esp]` / `call 00689cc0` | update, ECX = the object, one float |
| 004e4b40 | `mov ecx,[00e198bc]` / `cmp byte [ecx+0x3c],0` / `je 004e4b8f` | **the global is re-read**, so a screen raised by the update is seen on the same frame |
| 004e4b4c | `call 004c6c30` / `test al,al` / `je 004e4b8f` | `BSP_Game_TryBeginRenderFrame` |
| 004e4b57 | `call 004c12b0` | `BSP_GuiManager_GetOrCreate` |
| 004e4b5c | `push ebp` / `push ecx` / `fstp [esp]` / `call 00aa4f80` | `BSP_GuiManager_Update(mgr, rawDelta, 0)` |
| 004e4b6c | `call 004ca440` | `BSP_Game_Render` |
| 004e4b73 | `call 004ca1f0` | `BSP_Game_FinishRenderFrame` |
| 004e4b8c | `ret 4` | the first of the two RETs in `BSP_Game_OnMove` |

The delta reaching 00689cc0 and 00aa4f80 is the raw one. The frame-control scaling at the tail of
`BSP_Game_OnMove` is never reached on this path, which is why the packet does not depend on the
scaled value the `game_frame_control` packet owns.

## The attract screen object, `DAT_00e198bc`

`operator new(0x4c)` at 004c9b92 in `BSP_Game_OnInitTitle`, constructed by 00689d90, vtable
`00cf78e8`. The vtable is `00689e60` (scalar deleting destructor thunk), `00689bd0` (a bare `RET`),
`00689e80`, `00689c00`, `00684600`.

| Offset | Type | Evidence |
| --- | --- | --- |
| +0x00 | vtable `00cf78e8` | 00689dc1 in the constructor, 00689c7d in the destructor stores the same value |
| +0x04..+0x38 | base class, filled by 00684e10 | not recovered |
| +0x3c | `bool active` | set by base activate 00684700 at 0068477a, cleared by 00689c00 at 00689c16, tested by the frame at 004e4b46 |
| +0x40 | GUI layout handle | 00aa5840 result stored at 00689e2a, released through 00aa31f0 at 00689c99 |
| +0x44 | `float idle_countdown` | 45.0f stored at 00689dc7, decremented at 00689d29, reloaded at 00689cf9 and 00689c4b |
| +0x48 | `bool enabled` | set to 1 at 00689ddc, read at 00689ce4 |

`0x4c` is the allocation size; nothing between +0x04 and +0x38 was opened, and no other field was
observed being written.

`00689d90` **BSP_AttractScreen_Construct** runs 00684e10, stores the vtable, writes 45.0f to +0x44,
publishes `this` in `DAT_00e198bc`, sets +0x48, builds the pooled string `FE_attract` (00cf78fc,
capacity 0xa) and calls `BSP_GuiManager_GetOrCreate()` then `00aa5840(&name, 1, 0)`, storing the
handle at +0x40. `00aa5840` with the manager in ECX is the group factory of
`docs/APP_INIT_FONTS_GUI.md`, so `FE_attract` is a top-level GUI layout, not a child widget.

`00689c60` **BSP_AttractScreen_Destruct** stores the base vtable, clears `DAT_00e198bc`, releases
the layout through `BSP_GuiManager_GetOrCreate()` then `00aa31f0(this+0x40)`, and calls 00684fa0.
`004e3aa0 BSP_Game_OnInit` releases the object through virtual +0 when it moves the game to state 3,
so the attract screen exists only in the front end.

`00689e80` **BSP_AttractScreen_Activate** (virtual +8) calls base activate 00684700, which sets
+0x3c; makes the layout visible through its virtual +0x34(1); sets bytes +4 and +5 of the movie
player `DAT_00e18d48`, calls 004f83b0 and the player's virtual +0x18; then builds the pooled string
`movies/PacificTheme.bik` (00cf7908, capacity 0x17) and calls `004f8a20(&path, 1, 1.0f, 1)`, where
the float comes from `00cefcb8`. `DAT_00e18d48` is written by `BSP_Game_BeginStartupSequence` at
004e5589.

`00689c00` **BSP_AttractScreen_Deactivate** (virtual +0xC) calls base deactivate 00683aa0, hides the
layout with virtual +0x34(0), clears +0x3c, and if the player's byte +5 is set calls its virtual
+0x1c, clears its bytes +4 and +5, then 004f83b0 and 004f8ac0; finally reloads +0x44 with 45.0f.
Ghidra has no function at 00689c00 or 00689bd0; both were read from the raw listing.

## 00689cc0 BSP_Game_UpdateBlockingScreen

`__thiscall`, ECX = the screen, one float stack argument, `RET 4`, no return value, two epilogues
(00689d59 and 00689d8b). Recovered from the listing 00689cc0-00689d8b; the pseudocode collapses the
float field at +0x44 into an int and drops the ECX of the two device queries.

```
hold  = BSP_InputDeviceTable_AnyDynamicDeviceButtonDown(DAT_00f8bbf4);   // 00a91020
hold |= BSP_InputDeviceTable_AnyJoystickButtonDown(DAT_00f8bbf4);        // 00a90180, both always run
if (this->active == 0) {                       // 00689cde
    hold |= (this->enabled == 0);              // 00689ce4, SETZ
    hold |= *(byte*)(FUN_00425d10() + 5);      // 00689ced
}
if (hold) this->countdown = 45.0f;             // 00689cf9, DAT_00ce3d60
else      this->countdown -= min(delta, 1.0f); // 00689d08, DAT_00d7a24c, x87 FLD/FSUB/FSTP
if (this->active) {                            // 00689d33
    if (!hold) return;                         // 00689d3b
    this->vtable[+0xC]();                      // 00689d44, Deactivate
    if (DAT_00e198c8) DAT_00e198c8->vtable[+8]();  // 00689d55
    return;
}
if (0.0f > this->countdown                     // 00689d5f, COMISS, strict
    && *(int*)(DAT_00e188a8+0x5d4) != 0xd
    && != 0x10 && != 0x11)                     // 00689d71..00689d7e
    this->vtable[+8]();                        // 00689d87, Activate
```

Facts worth keeping:

- The two device queries are ORed on a byte, not short-circuited: both run every frame.
- The clamp is `COMISS delta, 1.0f` with `JBE` keeping `delta`, so a NaN delta is not clamped.
- The countdown test is strict, so the frame it reaches exactly 0.0 is not the raising frame.
- `this->enabled == 0` and the scene-context byte only enter `hold` while the screen is **down**.
  They hold the countdown at its reload value, which is how the attract screen is kept out of a
  situation without clearing state.
- `min(delta, 1.0f)` means a long hitch costs at most one second of idle time.

`00a91020` **BSP_InputDeviceTable_AnyDynamicDeviceButtonDown** walks the checked vector whose proxy
sits at `DAT_00f8bbf4+0x6c` (`_Myfirst` +0x70, `_Mylast` +0x74). `00a90180`
**BSP_InputDeviceTable_AnyJoystickButtonDown** walks the eight pointers at +0x44..+0x60, which
`docs/APP_INIT_INPUT.md` identifies as the joystick row of the 3x8 slot array (`this+0x04 +
class*0x20 + slot*4`). Both call device virtual +0x2c, which is `00a93f60`: it polls indices
0..0x3b through virtual +0x20 and returns 1 at the first hit. So the pair means "any button on any
device is down". The keyboard row (+0x04) and the mouse row (+0x24) are **not** consulted; only the
joystick row and the dynamic vector are.

## 004db220 BSP_Game_ResetToTitle

`__thiscall`, ECX = the game, no stack arguments, no return value; nine instructions ending in a
tail `JMP` to 004c9a70.

```
FUN_004db190();          // ECX = the game, kept from the entry MOV ESI,ECX
FUN_004cccc0(this);
*(byte*)(this + 0x2180) = 0;
BSP_Game_OnInitTitle(this);   // tail JMP 004db239
```

`004db190` sets `DAT_00e08874`, conditionally runs 004d7970/004da780, releases `DAT_00e198b4`,
`DAT_00e198b8` and `DAT_00e198ac` through their deleting destructors, then 004c1710, 00a4c2c0 and
`BSP_Game_DrainStateRequests`. `004cccc0` releases the in-mission singletons `DAT_00e18678`,
`DAT_00e1867c`, `game+0x19c8`, `DAT_00e19900`, `DAT_00e18db0`, `DAT_00e19698`, `DAT_00e1930c` and
more. So the routine is a full teardown followed by a rebuild of the title state.

Its sole caller is `BSP_Game_OnMove` at 004e4c0d, guarded by `DAT_00e198c8 == 0` in front-end state
2. `BSP_Game_OnInitTitle` builds both `DAT_00e198bc` (the attract screen, `new 0x4c`) and
`DAT_00e198c8` (`new 0x44`, gated on `DAT_00e198cc == 0`), which is why a missing front-end menu is
repaired by re-entering the title rather than by constructing the menu alone.

## 00aa4f80 BSP_GuiManager_Update

`__thiscall`, ECX = the 0x88-byte `cGuiManager`, stack arguments `float seconds` then a byte flag,
`RET 8`. Both `BSP_Game_OnMove` call sites pass flag 0 (004e4b67 and 004e4cb8).

```
manager->byte_70 = flag;                       // 00aa4fa3
if (flag == 0) FUN_00aa3910(manager);          // 00aa4fa8
BSP_GuiManager_ResetScreens(manager);          // 00aa4faf, 00aa0f70
FUN_004d35d0(&snapshot, manager + 0x14);       // 00aa4fbc, vector copy-construct
for (p = snapshot.first; p != snapshot.last; ++p) {
    screen = *p;
    if (screen->vtable[+0x38]()                // 00aa4ffb
        || BSP_GuiScreen_HasLiveEntries(screen))  // 00aa500c
        screen->vtable[+0x40](seconds);        // 00aa502d, one float stack argument
}
manager->byte_70 = 0;                          // 00aa503f, unconditional
free(snapshot.first);                          // 00aa5047
```

- The screen list is the checked vector at `manager+0x14` (`_Myfirst` +0x18, `_Mylast` +0x1c),
  which `00aa5d70` zeroes. `004d35d0` is a plain `vector<T*>` copy-construct, so the walk runs over
  a **snapshot**: screens added or removed during the update are not seen this frame.
- `00aa0f70 BSP_GuiManager_ResetScreens` moves `hl_FrameBox` (+0x74) and `hlCircle_FrameBox`
  (+0x78) to (-1, -1, 0) and hides them, every update.
- `00aa3910` reads the cursor device vector at `DAT_00f8bbf4+0x94` and is therefore the pointer
  hit-test pass. Its interior was not opened; only its input and its guard are established.
- `00aa7ef0 BSP_GuiScreen_HasLiveEntries` returns true when any of the `screen+0x8c` entries in the
  array at `screen+0x88` is non-null. It is the reason an invisible screen still gets its update.
- The many `CALL 00bf6713` sites in the loop are `_SECURE_SCL` iterator assertions.

## 00aa0e00 and 00aa0e50, the pointer pair

`00aa0e00` **BSP_GuiManager_SetEnabled**: `__thiscall`, one byte stack argument, `RET 4`.

```
manager->byte_48 = arg;                        // 00aa0e06
if (arg == 0)
    ((*(void***)(manager + 0x50))[+0x34])(manager->ptr_50, 0);  // tail JMP 00aa0e1b
```

`00aa0e50` **BSP_GuiManager_SetPointerVisible**: four instructions, ECX = `manager+0x50`, then a
bare `JMP` to that object's virtual +0x34 **without pushing anything**, so it forwards its own
stack frame and the callee cleans the argument. Ghidra's zero-argument `__fastcall` signature is
wrong: all four call sites push one argument first (004e4c38 pushes `EBP` = 0, and 00565081,
004daa9d, 004c6e1c likewise). It never touches +0x48.

`manager+0x50` is `MousePtrFE_Icon`, resource 4 of `00aa5e20` in `docs/APP_INIT_FONTS_GUI.md`
(stored at both +0x54 and +0x50), and virtual +0x34 is the visibility setter used throughout that
resource list. `manager+0x48` is a byte the constructor `00aa5d70` zeroes; **its readers were not
found**, so the only established meaning of `SetEnabled` is the store plus the pointer hide.

The `BSP_Game_OnMove` branch that uses the pair, at 004e4c29, tests `byte [DAT_00f8abe8+0x3e8]`:
non-zero takes `SetPointerVisible(0)` then `SetEnabled(0)`; zero takes `SetEnabled(1)` and the
`00425d10` menu branch. That branch belongs to the `game_frontend_states` packet and was not
analysed further here; only the polarity is recorded, because it is the evidence for the argument
of `00aa0e50`.

## Callers and callees

| Address | Name | Callers | Direct callees analysed here |
| --- | --- | --- | --- |
| 00689cc0 | BSP_Game_UpdateBlockingScreen | 004e4a40 only | 00a91020, 00a90180, 00425d10, virtuals +8/+0xC |
| 004db220 | BSP_Game_ResetToTitle | 004e4a40 only | 004db190, 004cccc0, 004c9a70 |
| 00aa4f80 | BSP_GuiManager_Update | 4, incl. 004e4b67 and 004e4cb8 | 00aa3910, 00aa0f70, 004d35d0, 00aa7ef0, _free |
| 00aa0e00 | BSP_GuiManager_SetEnabled | 9, incl. 004e4c4d, 004e4c5c | screen virtual +0x34 |
| 00aa0e50 | BSP_GuiManager_SetPointerVisible | 4: 004e4c40, 0056508c, 004daa9d, 004c6e1c | screen virtual +0x34 |
| 00689d90 | BSP_AttractScreen_Construct | 004c9bab only | 00684e10, 004c12b0, 00aa5840 |
| 00689c60 | BSP_AttractScreen_Destruct | vtable slot 0 thunk | 004c12b0, 00aa31f0, 00684fa0 |
| 00689e80 | BSP_AttractScreen_Activate | 00689d87 only | 00684700, 004f83b0, 004f8a20 |
| 00689c00 | BSP_AttractScreen_Deactivate | 00689d44 only | 00683aa0, 004f83b0, 004f8ac0 |

## Uncertainties

1. **`manager+0x48` has no identified reader.** `SetEnabled` is named for its call sites, not for an
   observed effect beyond hiding the front-end pointer.
2. **The scene-context byte at `00425d10()+5`** suppresses the attract screen, but the singleton
   itself (57 callers, a mode/flag block at +4, +5, +0x188, +0x218, +0x25c) was not opened.
3. **Game states 0xd, 0x10 and 0x11** are excluded from raising the screen; their meaning is
   unknown. `include/bsp/game_entry.hpp` only names 1, 2, 3 and 10.
4. **`DAT_00e198c8` virtual +8** is called on the dismissal frame. The 0x44-byte object is the
   front-end menu built beside the attract screen; the virtual was not opened.
5. **Screen virtuals +0x34, +0x38 and +0x40** are used by name (set visible, wants update, update)
   from their arguments and call sites. No override was read.
6. **`00aa3910`** is identified only by its input, `DAT_00f8bbf4+0x94`, and its guard.
7. **The base class at attract+0x04..+0x38** (00684e10 / 00684700 / 00683aa0, and the list at
   `DAT_00e19898`) was not reconstructed; only the +0x3c write inside 00684700 was traced.
8. The `FE_attract` layout resolves through the GUI factory `00aa5840`, whose lookup path
   (00aa3140, then a name built from the literal at 00d0e67c) was not followed.

## Reconstruction and state reached

`include/bsp/blocking_screen.hpp` and `src/blocking_screen.cpp`, registered in
`cmake/startup.cmake`, build into `bsp_core` with `/W4 /WX` for Win32. They provide
`bsp::update_attract_screen_00689cc0`, `bsp::gui_manager_update_00aa4f80`,
`bsp::gui_manager_set_enabled_00aa0e00`, `bsp::gui_manager_set_pointer_visible_00aa0e50`,
`bsp::reset_to_title_004db220` and `bsp::run_blocking_screen_frame`, each over an injected host
with one method per native call site, in the style of `bsp::run_application_frame`. The attract
object is modelled as the three fields whose offsets are recovered, not as a binary-compatible
image. No global is invented and no unresolved call is stubbed.

| Address | State |
| --- | --- |
| 00689cc0 | exported, analysed, reconstructed, build-tested |
| 004db220 | exported, analysed, reconstructed, build-tested |
| 00aa4f80 | exported, analysed, reconstructed, build-tested |
| 00aa0e00 | exported, analysed, reconstructed, build-tested |
| 00aa0e50 | exported, analysed, reconstructed, build-tested |
| 004e4b2a-004e4b8c | analysed from the listing, reconstructed, build-tested |
| 00689d90, 00689c60, 00689e80, 00689c00 | analysed |
| 00a91020, 00a90180, 00aa7ef0 | analysed |
| 00aa3910, 00aa5840, 00aa31f0, 00684700, 00683aa0 | not analysed beyond their call sites |

Nothing here is ABI-compatible and nothing is game-validated.

## What remains

- Open `manager+0x48`'s readers and confirm what `SetEnabled` gates.
- Open `00425d10` and name the flag at +5.
- Identify game states 0xd, 0x10 and 0x11.
- Open `DAT_00e198c8` and its virtual +8, which is the natural start of `game_frontend_states`.
- Recover the base screen class at 00684e10/00684700/00683aa0 and the `DAT_00e19898` list, which
  would give the rest of the attract object's layout.

## Correction from docs/GUI_LAYER_MANAGER.md

The reader of `manager+48h` that was recorded as not found is at `00aa3a36`: it gates the entire pointer pass of the GUI manager update, and `00aa3910`'s interior is the pointer motion and clamp pass.
