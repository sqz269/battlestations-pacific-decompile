# From the title screen to the main menu (packet `cc_main_menu_path`)

Addresses: 0067cfb0, 0068d8a0, 004d7920, 004e4430, 004e4000, 004e4259, 004e4269, 004e4279,
004e5442, 006840f0, 004cc460, 00684600, 00684700, 00685820, 004f8710, 004d8c00, 004f7620,
004f83b0, 004f8830, 004c40f0, 004c43c0, 00686380, 00687330, 004f8530, 004f85d0, 004f8670,
004f87b0, 004da780, 004c4300, 0068ab80, 0068aca0.

The four managers, the five screen-set levels, the recompute and the commit are already recovered
in `docs/FRONTEND_SCREEN_SETS.md`, `docs/FRONTEND_MANAGERS.md` and `docs/FRONTEND_STATE_MACHINE.md`.
This document adds the two things the rebuilt executable still needs: the ordered path that carries
a keypress on the press-start page through to a visible main menu, and the level ownership that
`docs/FRONTEND_SCREEN_SETS.md` left open as the `front_end_screen_levels` follow-up.

Reconstruction: `include/bsp/main_menu_path.hpp`, `src/main_menu_path.cpp`. Nothing here is a
binary-compatible layout, and descriptive names are hypotheses, not recovered symbols.

## The path, in call order

Each row is one step of `bsp::MainMenuPathStep`. The host method column names the pure virtual on
`bsp::MainMenuPathHost` the executable has to implement; the host has no default implementations.

| # | Address | Native routine | What it does | Host method |
| --- | --- | --- | --- | --- |
| 1 | 0067D2DD | inside 0067CFB0 | Tests input action 4Eh through 004C43C0. Until it goes down the screen only pulses its prompt. | `input_action_pressed` |
| 2 | 0068D8A6 | 0068D8A0 `BSP_TitleScreen_Skip` | 00BD3450 on `*0109CECC`, resetting storage availability. Unconditional. | `reset_storage_availability` |
| 3 | 0068D8B1 | same | Refuses to enqueue while `game+5E8h` is non-zero. On that arm it returns without releasing the hold byte, so the frame simply repeats. | state, not a call |
| 4 | 0068D8BC | 004D7920 | `BSP_Game_RequestState(4)`. | `request_game_state` |
| 5 | 0068D8C6 | same | `game+5ECh = 0`, which is what lets the drain run. | `release_state_request_hold` |
| 6 | 004E4D07 | 004E4430 | The drain pops the ring; case 4 calls the shell entry. | `drain_state_request` |
| 7 | 004E4000 | `BSP_Game_EnterFrontEndShell` | Destroys the title object, opens `GVMainMenu`, constructs the three managers and runs their Init virtuals (00686380 for the main menu). Returns with `game+5D4h` still 3. | `enter_front_end_shell` |
| 8 | 004E424A | — | Reads `*(00E198AC)+4h`, the applied interface id. | `main_menu_manager_mode` |
| 9 | 004E4259 | 004CC460 | `PUSH EDI (null payload); PUSH 1; CALL`, with ECX still holding the manager from 004E424A. Skipped when the applied id is already 4. Writes the **pending** record at `manager+20h`/`+38h` only, so nothing is visible yet. | `push_interface_request` |
| 10 | 004E4269 | 00684700 via vtable +8h | The mutual-exclusion activate: deactivates every other manager, sets `manager+3Ch`. | `activate_main_menu_manager` |
| 11 | 004E4274 | 005884A0 | ECX is `*(00E198AC)+58h`, the 578h main-menu screen object. | `start_main_menu_screen_object` |
| 12 | 004E4279 | — | `game+5D4h = 5` (EBX, loaded at 004E41B6). **The frame path changes here.** | `set_game_state` |
| 13 | 004E5442 | 006840F0 | Sees `manager+4h != manager+20h` with `manager+3Ch` set and calls vtable +10h with the pending id and payload. | `service_pending_interface_requests` |
| 14 | 00685826 | 00684600 | Rewrites the pending record from its own arguments, then syncs applied = pending. That sync is what stops 006840F0 firing again. Returns 0 when the request was rejected. | `apply_interface_request` |
| 15 | 00685848 | the identity arm | Interface 1 maps to screen id 1. | `map_interface_to_screen` |
| 16 | 006858BA | 004F8710 | `PUSH 0; PUSH 1; CALL`, the one-element level-4 screen set. 004F8710 runs 004F7620 itself, so the requested bytes are current when it returns. | `publish_screen_set_level4` |
| 17 | 006858E6 | 004D8C00 | `PUSH 0; PUSH 1; PUSH game; CALL`, the one-element input-context set `{1}`. | `publish_input_context_set_level4` |
| 18 | 004C4165 | 004F8830 | Pass B finds slot 1 wanted but not active. | `pump_front_end_screens` |
| 19 | 004F88E8 | 004F83B0 | The commit runs **before** the enter virtual, so the enter body observes itself as already visible. It publishes the +5h byte (004F8434, `MOVZX EDX,[EBX+5h]`) to each non-null child through vtable +34h rather than showing anything; the exit pass clears +5h at 004F88B3 and calls it again at 004F88B7, so the host method carries the byte (`visible`, true here). Per-child walk: `bsp::commit_front_end_screen_visibility_004f83b0`. | `commit_screen_visibility(id, visible)` |
| 20 | 004F88F4 | 005987F0 | The main-menu screen's enter virtual. | `enter_screen` |
| 21 | 004F8925 | 00599DB0 | Pass C, the screen's update virtual with the raw delta. | `update_screen` |

Registry slot 1 is `INTF_MAINMENU`, the 578h object constructed by 005902E0 with vtable 00CEFC5C.
Its layout is `FE_main` and its initial page is 1, `FE.main_menu` (`docs/MAIN_MENU_SCREENS.md`,
`docs/MAIN_MENU_SCREEN_UPDATE.md`). `BSP_MainMenu_Init` 00686380 is the manager Init at vtable +4h,
not a screen builder: it opens the `GVMainMenu` load block, caches
`sound/music/titlescreen.fsb` and `sound/music/creditsfinal.fsb` with their `.def` siblings,
registers the `globals` locale table and constructs the seven screens listed in
`docs/MAIN_MENU_SCREENS.md`. Those page names are already recorded there and are not restated here.

### The frame-path boundary at step 12

This is the part of the path most likely to be got wrong, and it is the reason steps 13 and 18 sit
where they do. `GGame::OnMove` takes the front-end branch at 004E4B9D only for `game+5D4h` of 1, 2
or 4; that branch ends by running the pump 004F8830 at 004E4CA3 and returning. The main-menu shell
rests at state **5**, which is not in that set. So from the instant 004E4279 writes 5:

- the interface request is serviced by 006840F0 at 004E5442, which the front-end branch never
  reaches, and
- the screens are pumped by 004C40F0 `BSP_Game_UpdateInterfaceOnly` (004F8830 at 004C4165), which
  OnMove calls unconditionally at 004E53B6.

`bsp::front_end_screen_pump_site` encodes that split. The third pump caller, 004CAC37 inside
004CAC30, is the loader's synchronous single-frame draw reached from 008C8560 with a delta of zero;
it is not part of the per-frame path.

The same `{1, 2, 4}` state set gates the teardown 004DA780 in the opposite sense: its head at
004DA7A0 jumps to the epilogue 004DB014 for exactly those three states, so the teardown is a no-op
while a front-end state is live.

## Which subsystem owns each screen-set level

`docs/FRONTEND_SCREEN_SETS.md` records the five setters and asks, as an open follow-up, that their
callers be traced to name the levels. The xref sweep settles it.

| Level | Screen setter | Context setter | Call sites | Owner |
| --- | --- | --- | --- | --- |
| 1 | 004F8530 | 004D8A50 | 18 | In-mission HUD interface. All but 004DA9B8 are inside 0068ACA0 `BSP_InGameInterface_ApplyPendingInterface`, through its jump table at 0068B390. |
| 2 | 004F85D0 | 004D8AE0 | 5 | In-mission overlays: 0068ABFC in `CollapseOverlays`, 0068AA9E, 0068AC79, 0068B4E9. |
| 3 | 004F8670 | 004D8B70 | 17 | In-mission transients: 0068ABB8, 0068B445, 0068C12C, 0068C185, five inside `BSP_InGameInterface_Update`, plus 005ED5FA, 0064EA68, 005FB551, 005FB808, 005F98B8, 0068B4A8. |
| 4 | 004F8710 | 004D8C00 | 5 | The front-end managers: 00683AA0, 00685820, 00687800, 00689820. The only level this path touches. |
| 5 | 004F87B0 | none | **0** | Nothing. |

Three consequences.

**Level 5 is dead code.** Ghidra reports no references of any kind to 004F87B0, there is no
level-5 input-context setter to pair with it, and the teardown clears levels 1..4 and skips it. The
descent in 004F7620 therefore always begins over an empty vector, and the suggestion in
`docs/FRONTEND_SCREEN_SETS.md` that "level 5 covers level 4" is unexercised in this build.

**Level 1 is the only level with multi-element sets.** Every level-2, level-3 and level-4 call
outside the teardown publishes either one id or none. The level-1 lists are pushed right to left
with the terminator first, so the argument order is the reverse of the push order: 0068AFD0
publishes `{29h, 49h, 44h, 27h, 4Dh, 3Eh, 25h, 26h}` and 0068B0EC publishes
`{29h, 49h, 44h, 27h, 4Dh, 45h, 47h, 48h}`. Every list read begins `29h, 49h, 44h`, which looks
like a persistent HUD core. The full 17-entry table is in-mission work and is left to a follow-up.

**During the front end, levels 1, 2 and 3 are empty**, because 004DA780 cleared them on the way out
of the last session and nothing in the front end populates them. The level-4 set the manager
publishes is therefore the entire visible front-end screen set.

### 004DA780, the teardown

Named `BSP_Game_TeardownSessionState` by this packet. `__fastcall(GGame*)`, RET 0, SEH handler
00C6681B; callers 004DB190 `BSP_FrontEnd_TeardownForSession`, 004DC5C0 `BSP_Game_OnDestroy` and
004E4430 `BSP_Game_DrainStateRequestQueue`. EBX is zeroed at 004DA7C2 and is the varargs terminator
every list call pushes. The relevant tail clears both stacks in a fixed order: input-context levels
1..4 at 004DA99D/004DA9A4/004DA9AB/004DA9B2, then screen-set levels 1..4 at
004DA9B8/004DA9BE/004DA9C4/004DA9CA.

### The input-context id range

004DA9D2..004DA9FD walks contexts 1..1Eh inclusive (`MOV EDI,1` / `CMP EDI,1Eh` / `JLE`) and drops
each one whose 00A92290 level is above 1. 004C4300's clear pass stops at 19h (`CMP EDI,1Ah` / `JL`).
Contexts 1Ah..1Eh therefore exist but are never dropped by a level clear: a context in that range
that a level vector raised stays raised until the teardown resets it. This is an asymmetry in the
original, not a reconstruction artefact.

### Clearing a level, and the emptiness test

Every clear is the same call with only the terminator. 0068AB80 `BSP_InGameInterface_CollapseOverlays`
shows the guard the game uses first: load the level vector's first pointer (00E18D1C for level 3),
bail when it is null, then compute `(last - first) >> 2` and bail when that is zero. Both halves are
needed because an emptied vector keeps its buffer. That also fixes the 16-byte vector layout as
`{?, first, last, end}`: level 3's base is 00E18D18 and its first/last pointers are at 00E18D1C and
00E18D20. `bsp::front_end_screen_set_level_populated` is that test.

## Corrections

- **The packet brief's "game state 2" is wrong for this transition.** State 2 is the title state
  that `GGame::OnInitTitle` writes. The press-start page requests state **4**
  (`bsp::kGameStateFrontEndRequest`, 0068D8BA), and the shell entry settles at state **5**
  (`bsp::kGameStateFrontEndShellReady`, 004E4279). State 4 never survives the shell entry; it is a
  drain request value only, as `docs/GAME_FRONTEND_STATES.md` already records.
- **`004C43C0` did not need reconstructing.** The packet asked for its pure edge rule over an
  injected input-state host; that rule already exists as
  `bsp::action_pressed_this_frame_004c43c0` in `include/bsp/input_tick.hpp`. It is referenced here,
  not redefined, and no input binding code was touched.
- No correction is made to `bsp::kGameInputContextCount` in `include/bsp/frontend_screen_sets.hpp`.
  Its value 1Ah correctly describes 004C4300's clear bound; the wider id space the teardown proves
  is recorded separately as `bsp::kGameInputContextTeardownLast`.

## Follow-up packets

- `in_game_interface_screen_sets` — the full level-1 table: the 17 call sites inside 0068ACA0 with
  their jump-table arm at 0068B390, the interface id each serves, and the ordered screen list each
  publishes. Needs 0068ACA0, 0068B390 and the screen registry names for ids 25h..50h.
- `front_end_manager_activate` — 00684700's mutual-exclusion walk over the registry at 00E1989C and
  what 00683AA0 does to the screen set on the way out, as a sequence over the same host.
- `main_menu_screen_object_start` — 005884A0, the `*(00E198AC)+58h` call at 004E4274. It is on the
  path but nothing is known about it beyond its call site.
- `frontend_interface_lock` — 00683E90 and the byte 00E19894 that gates every push, including the
  shell's. The lock decides whether step 9 is a no-op.

## `no_ghidra_function`

Every routine read for this packet had a Ghidra function. No new code ranges were found.

| Start | End (inclusive) | Note |
| --- | --- | --- |
| — | — | none |
