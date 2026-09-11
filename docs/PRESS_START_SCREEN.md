# The press-start screen (`FE_initial`, vtable `00CF6D90`)

Addresses: 0067c840, 0067c860, 0067c870, 0067c880, 0067c9e0, 0067ca40, 0067ca60, 0067ca80,
0067cb40, 0067cc60, 0067cf50, 0067cfb0, 0067d860, vtable 00CF6D90

Packet `press_start_screen`, worktree `agent/press-start`. Ghidra was read-only for this packet.
This document covers the class the title screen constructs at `0068d92f`, all ten slots of its
vtable, the sign-in and storage flow behind `press_start_Text`, and the handover into state 4.
`docs/GAME_TITLE_INIT.md` proposed the packet and had already established `0067c840`, `0067ca80`
and the `.rdata` literal block; everything below it is new.

Every name here is a hypothesis, not a recovered symbol.

## The class

`BSP_TitleScreen_Activate` (0068d8d0) allocates 0x18 bytes, constructs with `0067c840`, stores the
result in `title+40h`, runs the register slot `+10h`, sets the base bytes `+4h` and `+5h` to 1,
calls `004f83b0` and finally the enter slot `+18h`. It is the only caller of the constructor.

`0067c840` chains `BSP_FrontEndScreen_Construct` (004f7180), clears `+10h` and the byte `+14h`, and
stores vtable `00CF6D90`.

| Offset | Type | Meaning | Evidence |
| --- | --- | --- | --- |
| +00h | vtable | `00CF6D90` | 0067c855 |
| +04h | byte | wanted (base) | 004f7180 |
| +05h | byte | active (base) | 004f7180 |
| +08h | handle | the `FE_initial` GUI layout | written 0067cad9, read 0067cbfe and 0067d860, released 0067ca09 |
| +0Ch | handle | the `press_start_Text` element | written 0067cc13, virtuals `+50h` and `+34h` called every frame |
| +10h | int | the sign-in handler has run once | cleared 0067c851 and 0067cc4a, set 0067d45b, tested 0067d442 |
| +14h | byte | a message screen must be closed before leaving | cleared 0067c84d and 0067cc51, set 0067d584, consumed 0067d541 |

The `.rdata` block that follows the vtable holds `FE_initial` (00CF6DB4), `press_start_Text`
(00CF6DC4), `globals.saving_xbox` (00CF6DD8), `L"Session"` (00CF6DEC) and `FE.unitlib_toggledesc`
(00CF6E00). Only the first three belong to this class: `L"Session"` is referenced from `0067d6e0`
at 0067d801, a neighbouring routine that this packet did not analyse, and `FE.unitlib_toggledesc`
has no reference from any of the ten slots.

### The vtable

| Slot | Target | Convention | Role |
| --- | --- | --- | --- |
| +00h | 0067c860 | `__thiscall`, RET | `mov eax, 5Ch` — the screen id |
| +04h | 004f7570 | base | not overridden |
| +08h | 004f7580 | base | not overridden |
| +0Ch | 0067ca60 | `__thiscall`, RET 4 | scalar deleting destructor over `0067c9e0` |
| +10h | 0067ca80 | `__thiscall`, RET, SEH 00C7CF78 | register, then load `FE_initial` |
| +14h | 004f7590 | base | not overridden |
| +18h | 0067cb40 | `__thiscall`, RET, SEH 00C7CFA0 | enter |
| +1Ch | 0067c870 | `__thiscall`, RET | exit — a bare `ret` |
| +20h | 0067cfb0 | `__thiscall`, one float argument, RET 4, SEH 00C7D018 | update |
| +24h | 0067d860 | `__thiscall`, one argument, RET 4 | publish the layout |

`004f71d0` calls slot `+0h` with no arguments and uses EAX as the index into the 95-slot registry at
`00E18B60` (`docs/GAME_FRONTEND_STATES.md`), so this screen owns slot 92 at `00E18CD0`. **0x5C is
not an interface id**: `docs/FRONTEND_SCREEN_SETS.md` maps interface ids only over 01h..1Fh, and no
screen-set request reaches this screen. `BSP_TitleScreen_Activate` constructs and enters it
directly, and it is never raised through the layered screen-set stack.

## Register and enter

`0067ca80` (slot `+10h`) chains `BSP_FrontEndScreen_Register` (004f71d0), builds the native string
`"FE_initial"`, calls `BSP_GuiManager_GetOrCreate` (004c12b0) and then `00aa5840(&name, 1, 0)`,
storing the handle in `screen+8h`.

`0067cb40` (slot `+18h`) does three things:

1. `00518d60(ecx = 0, edx = 0, -1, &"")` at 0067cb9f. That routine stores its three inputs into
   `00E18D84`, `00E18D88` and `00E18D8C`, then runs `004c1ac0` and `00518c80`; with an empty string
   it skips `005189b0`. 27 screens call it, including `BSP_MainMenu_BuildTopLevelPage`. Read as
   "clear the front-end page context"; **provisional**, the three globals were not chased.
2. `00aa7e00(&"press_start_Text", 1)` on the layout at `screen+8h`, result into `screen+0Ch`.
3. Clears `screen+10h` and `screen+14h`, repeating the constructor. So re-entering the screen
   restarts the sign-in flow.

`0067d860` (slot `+24h`) is `add ecx, 8; push ecx; mov ecx, [esp+8]; call 004d6790; ret 4`: it
appends `screen+8h` to the caller's sink. The base target `004f75d0` "fills the list `004f83b0`
walks", so this publishes the `FE_initial` layout for the frame's draw list.

`0067c9e0` restores the vtable, calls `BSP_GuiManager_GetOrCreate` then `00aa31f0(screen+8h)` to
release the layout, and chains `BSP_FrontEndScreen_Destruct`. `0067ca60` wraps it with the usual
`test byte [esp+8], 1` operator-delete tail.

## The update, `0067cfb0`

`__thiscall`, ECX = the screen, one float stack argument (the frame delta, read at `[esp+30h]`),
RET 4, no return value, SEH handler `00C7D018`.

### The pulsing prompt

The first fourteen instructions are the only floating-point work in the class:

```
phase = fmod((float)(dt * 2.5 + phase), 6.283185307179586)   ; 00CE3DE0, 00CE3828, 00E19888
alpha = |sinf(phase)|                                        ; fsin, then and eax, 7FFFFFFFh
element->vtable[50h]({1.0f, 1.0f, 1.0f, alpha})              ; 00D7A24C is 1.0f
```

`00E19888` is a global, not a field, so the pulse survives a destroy/construct cycle. The absolute
value is taken by masking the sign bit of the stored float at 0067d026, not by a compare.

### The invite fast path, `0067cf50`

Called immediately after the colour is set; a true result ends the frame.

- `00E19880 == 0` returns false, which is the normal boot. `docs/GAME_TITLE_INIT.md` establishes
  `00E19880` as the invite/join flag through `0067c8f0`.
- Otherwise the prompt element is hidden and `00a3e500` (manager+28h) is read:
  - 0 → `BSP_XenonSystemManager_FindSlotForInvitee` (00a3e470) then `00a3f3d0` with that slot;
  - 1 → nothing;
  - 2 → `0067cc60` then `BSP_TitleScreen_Skip`.
- Every one of those returns true, so an invite never shows the press-start prompt.

### The sign-in phase at manager+28h

`00a3e500` is `mov eax, [ecx+28h]; ret`. `00a3f3d0` is `mov dword [ecx+28h], 1; jmp 00a3f100`, and
`00a3f100` is `XenonSystemManager::SignInUser(pad)`: it logs `SignInUser(%d)`, stores the pad at
`manager+3B4h`, and moves the manager's own state at `manager+3B0h` to 4 (already signed in, plus
the callback at `manager+24h`), 3 (show the sign-in UI once, guarded by `manager+3E9h`) or 1.
The only write of 2 to `+28h` in `.text` is at 00a4096f inside the manager pump `00a40510`, which
also moves `manager+3B0h` to state 7 and is guarded by `manager+2Ch == 0`.

| Value | Meaning |
| --- | --- |
| 0 | idle; the screen waits for a press |
| 1 | a sign-in was requested and the blade may be up |
| 2 | a user is selected; the screen may build the profile |

### Frame body

After the colour, the guard and `phase = 00a3e500`, the update always calls
`element->vtable[34h](false)` at 0067d081, then tests `BSP_XenonSystemManager_IsProfileChangePending`
(00a3e3b0).

**Profile changed.** Raise prompt slot 2 with key `FE_xbox.xsm_profilechanged` (00CE7C38) and kind
2 through `00531b00`, then `BSP_XenonSystemManager_ResetSignInState` (00a40020) and `0067c970`. This
arm returns without the idle tail.

**Phase 0, the idle arm** (0067d16d..0067d415). ESI is reloaded with the device pointer at
0067d1ae, so nothing past that point touches the screen object.

1. `element->vtable[34h](menu_command_screen->+5h == 0)`; when a message screen is up, also
   `00532a20(menu, 0)` to clear prompt slot 0.
2. `device = 004ba6d0([00F8BBF4], 2, 0)` — element 0 of the checked vector at `+B8h`, the same
   vector `0067c970` re-binds. `004ba6d0` computes `base + (2*9 + 1Bh)*4 = base + B4h` and reads
   the vector at `+B8h`, which confirms the offset `docs/GAME_TITLE_INIT.md` reported.
3. `manager+3E8h != 0` ends the frame at the tail.
4. `00E1987C` set: with a device, `0067c970` and the device is dropped for this frame; without one,
   the flag is cleared.
5. `00E19885` set: `0067c970`, clear it, end the frame. This is the frame after the blade closes.
6. With a device:
   - a message screen up → `00532a20(menu, 2)`, `BSP_FrontEndScreen_Close` (004b6e50),
     `00E19884 = 1`;
   - else `00E19884` clear → `00a3f3d0(manager, device->vtable[34h]())`, then a discarded
     `00a3e500`;
   - else `0067c880(device)`: while it is true the frame does nothing; once false, `0067c970` and
     `00E19884 = 0`. `0067c880` asks the device's `vtable[20h]` for button 0 and then button 0Eh.
7. With no device, `BSP_InputAction_WasPressedThisFrame(4Eh)` decides:
   - pressed with a message screen up → `00E19884 = 1`, and if `menu+188h` and `menu+218h` are both
     zero, `00532a20(menu, 2)` and `004b6e50`;
   - pressed with `00E19884` set → `004b43b0([00F8BBF4])` (a rising-edge latch cached at `+DDh`);
     when it is false, clear `00E19884`;
   - pressed otherwise → `00a3f3d0(manager, 0)` and a discarded `00a3e500`;
   - not pressed, `00E08CC0` set and no message screen → `00a3f3d0(manager, 0)`, discarded
     `00a3e500`, clear `00E08CC0`.
8. Every return in this arm runs `004c1e90` (a 0x50-byte singleton at `00E17664`, built by
   `00426250`) and `00427190(that, 0)`, a dispatcher on its argument. Its role is **uncertain**;
   only that it runs on all seven idle-arm exits is established.

**Phase 1** (0067d418). `00E19885 = 1` and return. No tail.

**Phase 2** (0067d439). With `screen+10h == 0` and `manager+3E8h == 0`: set `screen+10h = 1`, clear
`00E19885`, then

- `BSP_XenonSystemManager_HasSelectedUser` (00a3e510, `manager+119h`) true → `0067cc60` then
  `00E08CC0 = 1`;
- false → the offline bring-up: `00425c20`/`004374f0`, then `008d4820`, `008d41c0`, `008d41f0` and
  `008d4520(0, 1)` on `00F88980`, `BSP_Settings_ApplyAll` (008d5b50), `BSP_InputSettings_GetSingleton`
  and `006ac030`, `[00F8BBF4]` virtuals `+0Ch` and `+8h` with the second result into `008d44c0`,
  `00698a10(input_manager + 3Ch)` and finally `BSP_TitleScreen_Skip`.

With `screen+10h != 0`, the arm waits for the storage flow: when `screen+14h` is set and no message
screen is up, clear it, `00530650(menu)` and `BSP_TitleScreen_Skip`; otherwise, when `screen+14h` is
clear, `00a3e540` (`manager+120h`) is false and a message screen is up, set `screen+14h`.

The globals this arm owns are all title-scope bytes, not fields:

| Global | Meaning |
| --- | --- |
| `00E19880` | dword; an invite or join is pending (`docs/GAME_TITLE_INIT.md`) |
| `00E1987C` | a primary input binding exists; `0067c970` sets it |
| `00E19884` | a press was consumed by a message screen; wait for the release |
| `00E19885` | the sign-in blade was up; rebind on the next idle frame |
| `00E08CC0` | the sign-in handler ran; re-request a sign-in once nothing is up |
| `00E19888` | the prompt pulse phase |

## The sign-in handler, `0067cc60`

`__thiscall`, ECX = the screen (unused past the prologue), no stack arguments, RET. Its ESI is
`[00E188A8] + 650h`, the player-profile block, and its EDI is `[00F8ABE8]`, the XenonSystemManager.
Two callers: `0067cf50` and `0067cfb0`.

Body order:

1. `BSP_PlayerProfile_ResetToDefaults` (007fdb20) on the profile block.
2. `00425c20` then `004374f0` on the result.
3. `00a3eae0(manager)` returns `manager + 90h + index*80h`, the gamertag buffer of the selected
   slot (`manager+11Ch` is the index, the same field `bsp::OnlineSignInState::user_index` models).
   The string is built once and handed to `007f9290` and `007f9340`.
4. `00a3eb00(manager)` returns the 64-bit value at `manager + 110h + index*8`; it is stored at
   `profile+48h`, which is `game+698h`, matching `docs/GAME_TITLE_INIT.md`.
5. `00bd3450([0109CECC])` on the save manager.
6. `00a3e5d0(manager)` returns a **different** 64-bit id, derived from `manager+3B8h`, and
   `sprintf(buffer, "%llx", id)` (format at 00CF3AE4, 128-byte stack buffer) makes the save name.
7. `[0109CECC]+21h == 0` (no storage device needed): `007fae70(profile)`, and when
   `[0109CECC]+8h == 0` also `00530650(menu)` and `BSP_TitleScreen_Skip`.
8. Otherwise the save manager's own virtuals decide:
   - `vtable[1Ch](save_name, 1)` true → raise prompt slot 0 with key `globals.saving_xbox` and
     kind 3, then `007ff100(profile, save_name, 0067ca40)`;
   - else `vtable[14h](save_name)` true → `BSP_InputSettings_GetSingleton`/`006ac030`, the two
     `[00F8BBF4]` virtuals into `008d44c0`, then `007fa710(profile, save_name, 0067ca40, 0)`;
   - either way `007fae70(profile)` and `00698a10(input_manager + 3Ch)`.

`0067ca40` is the completion callback both queue calls carry: when `[0109CECC]+8h == 0` it runs
`00530650(menu)` and tail-jumps to `BSP_TitleScreen_Skip`. So the storage path reaches the main
menu from the callback, and the phase-2 arm's `screen+14h` handling is the fallback that closes a
message screen the callback left up.

### The prompt call, `00531b00`

Both prompts use the same ten-argument block, recovered from the two call sites:

```
00531b00(menu, slot, &key, kind, 0, 0, &"", 0.0f, 0, ""(by value, 8 bytes), 1)
```

| Site | Slot | Key | Kind |
| --- | --- | --- | --- |
| 0067d0f5 | 2 | `FE_xbox.xsm_profilechanged` | 2 |
| 0067cded | 0 | `globals.saving_xbox` | 3 |

`00532a20(menu, i)` clears one of seven 0x48-byte prompt records based at `menu+60h`
(`lea esi, [edi + eax*8 + 60h]` after `lea eax, [eax + eax*8]`), and `00530650(menu)` clears all
seven. A native string is 8 bytes, an int length followed by a `char*`.

## The handover to the main menu

Every exit is `BSP_TitleScreen_Skip` (0068d8a0), which enqueues state request 4 through
`BSP_Game_RequestState` (004d7920) when `game+5E8h` is clear and then clears `game+5ECh`. There are
four reachable exits: the offline bring-up, the storage callback `0067ca40`, the `screen+14h`
fallback, and the invite fast path. No interface id and no screen set is applied here; raising the
main-menu screen set belongs to the state-4 entry `004e4000`, not to this screen.

## Calls in and out

Callers: `BSP_TitleScreen_Activate` (0068d8d0) constructs and enters; the front-end pump `004f8830`
drives the update through the registry; `004f83b0` reaches slot `+24h`.

Callees not named above: `004c12b0`, `0041dd40`, `0041e870`, `00419cc0`, `00bd1510`, `00bf7680`,
`00bf7a6a`, `00aa5840`, `00aa7e00`, `00aa31f0`, `004d6790`, `00518d60`, `004c1e90`, `00427190`,
`00425c20`, `004374f0`, `007f9290`, `007f9340`, `007fae70`, `007ff100`, `007fa710`, `008d4820`,
`008d41c0`, `008d41f0`, `008d4520`, `008d44c0`, `006ac030`, `00698a10`, `005547d0`, `00530650`,
`00531b00`, `00532a20`, `004b6e50`, `004b43b0`, `004ba6d0`, `00a3e470`, `00a3e500`, `00a3e510`,
`00a3e540`, `00a3e5d0`, `00a3eae0`, `00a3eb00`, `00a3f3d0`, `00a40020`.

## Routines with no Ghidra function

`0067c860`, `0067c870`, `0067ca60`, `0067c880`, `0067ca40` and `0067d860` have no function in the
snapshot; they were read from disk bytes. The integrator has to define a function at each before a
ledger name can be applied. `0067d860` is the awkward one: Ghidra's `FUN_0067d6e0` claims the body
`0067d6e0-0067d85f`, which stops exactly at `0067d860`, so the leaf is unclaimed rather than
swallowed.

| Address | End | Role |
| --- | --- | --- |
| 0067c860 | 0067c865 | screen id, `mov eax, 5Ch; ret` |
| 0067c870 | 0067c870 | exit, a bare `ret` |
| 0067c880 | 0067c8a9 | device button probe over `vtable[20h]` |
| 0067ca40 | 0067ca5c | storage completion callback |
| 0067ca60 | 0067ca7b | scalar deleting destructor |
| 0067d860 | 0067d86f | slot `+24h`, publish the layout |

`0067cfb0` does have a function (`0067cfb0-0067d59c`) but `python tools/bsp.py ghidra export` fails
on it while `ghidra decompile` succeeds; see the tool problems below.

## Uncertainties

- `00427190`'s role. It is a dispatcher on an int code, called only with 0 from this screen, on a
  0x50-byte singleton at `00E17664`. Neither the singleton nor the code table was chased.
- `00518d60` is read as "clear the front-end page context" from its call sites and its three
  globals `00E18D84`/`88`/`8Ch`. The globals themselves were not traced.
- The element virtuals `+50h` and `+34h` are named "set colour" and "set a flag" from their
  arguments (four floats, one bool) and the `SETZ` that feeds `+34h` in the idle arm. The GUI
  element vtable was not enumerated.
- `004b43b0` caches `00a91020()` into `[00F8BBF4]+DDh` and returns 1 only on a rising edge. What
  `00a91020` reports is unknown, so "the press is still latched" is provisional.
- The save-manager virtuals `+1Ch` and `+14h` remain raw predicates. The 2026-09-10 profile
  packet established `0067ce4c -> 007ff100` as read/restore and `0067cea4 -> 007fa710` as
  conditional write; the earlier save/load host labels were inverted. The host now uses
  the two request addresses and predicate offsets explicitly. The `globals.saving_xbox`
  prompt key and original branch ordering remain as observed. See `docs/GAME_PROFILE_RESET.md`.
- `00E08CC0` re-requesting a sign-in on an idle frame is what the code does; why the flow needs it
  was not established.
- `0067cfb0`'s x87 `fsin` is not bit-identical to a C `sin` on the rounded float.

## What remains

- `0067d6e0` and the `L"Session"` save container beside it.
- The manager pump `00a40510` state machine (`manager+3B0h` states 1, 3, 4, 6, 7) that publishes
  phase 2 to this screen.
- The GUI element vtable, which would settle `+34h` and `+50h`.
- `00531b00`'s remaining seven arguments and the meaning of the "kind" values 2 and 3.

## Follow-up packets proposed

1. `frontend_prompt_screen` — `00531b00`, `00532a20`, `00530650`, `004b6e50`, `00533120`, the
   seven 0x48-byte records at `menu+60h`. Files `docs/FRONTEND_PROMPT_SCREEN.md`,
   `reports/frontend_prompt_screen.json`, `include/bsp/frontend_prompts.hpp`,
   `src/frontend_prompts.cpp`. Contract: recover the prompt record, the ten-argument raise, the
   kind values, and how `menu+188h`/`+218h` gate a dismissal.
2. `xenon_sign_in_pump` — `00a40510`, `00a3f100`, `00a3f3d0`, `00a3e470`, `00a3e5d0`, `00a3eae0`,
   `00a3eb00`, `00a40020`. Files `docs/XENON_SIGN_IN_PUMP.md`, the matching report,
   `include/bsp/xenon_sign_in.hpp`, `src/xenon_sign_in.cpp`. Contract: recover `manager+3B0h`'s
   state machine, the `+28h` handshake with this screen, and the callback at `manager+24h`.
3. `player_profile_save` — `007fdb20`, `007f9290`, `007f9340`, `007fae70`, `007ff100`, `007fa710`,
   `0109CECC` and its vtable slots `+8h`, `+14h`, `+1Ch`, `0067d6e0` with `L"Session"`. Files
   `docs/PLAYER_PROFILE_SAVE.md`, the matching report, `include/bsp/player_profile_save.hpp`,
   `src/player_profile_save.cpp`. Contract: recover the profile block at `game+650h`, the save
   container name, and the queue-with-callback contract that `0067ca40` rides on.

## State reached

| Address | State |
| --- | --- |
| `0067c840` | reconstructed, build-tested (`bsp::construct_press_start_screen_0067c840`) |
| `0067ca80` | reconstructed, build-tested (`bsp::register_press_start_screen_0067ca80`) |
| `0067cb40` | reconstructed, build-tested (`bsp::enter_press_start_screen_0067cb40`) |
| `0067c870` | reconstructed, build-tested (`bsp::exit_press_start_screen_0067c870`) |
| `0067d860` | reconstructed, build-tested (`bsp::collect_press_start_layouts_0067d860`) |
| `0067c9e0` | reconstructed, build-tested (`bsp::destruct_press_start_screen_0067c9e0`) |
| `0067cf50` | reconstructed, build-tested (`bsp::run_invite_fast_path_0067cf50`) |
| `0067cfb0` | reconstructed, build-tested (`bsp::update_press_start_screen_0067cfb0`), pulse fixture-tested |
| `0067cc60` | reconstructed, build-tested (`bsp::apply_sign_in_0067cc60`) |
| `0067c860`, `0067ca60`, `0067c880`, `0067ca40` | analysed from raw bytes; no Ghidra function |
| `00a3f100`, `00a3e500`, `00a3f3d0`, `00a3eae0`, `00a3eb00`, `00a3e5d0`, `004ba6d0`, `004b43b0`, `00532a20`, `00530650`, `00518d60`, `0068d8a0` | analysed as evidence |

Nothing here is ABI-compatible or game-validated; the reconstruction exposes new C++ interfaces
over an injected host, in the style of `bsp::run_application_frame`.
