# cc13-fe6: large frontend screen bodies

Packet cc13-fe6 (cc13 lineage, Claude). Reconstruction pass over the screen bodies the
cc13-fe1b handoff (docs/CC13_FE_1.md) left open. Code: `src/frontend_screens_cc13.cpp`,
`include/bsp/frontend_screens_cc13.hpp`, namespace `bsp::cc13_fe1`.

Status vocabulary: every routine below is **reconstructed and build-tested**. None is
ABI-compatible (the native `__thiscall`/`__fastcall` register conventions are documented, not
reproduced), fixture-tested or game-validated. Names are hypotheses carried over from the
ledger; no Ghidra renames were made in this pass.

Evidence: live `bsp.py ghidra decompile` per routine, then `bsp.py disasm-raw` over the whole body
for every call's ECX/EDX and stack arguments, RET sizes, and x87 sequences. x87 register forms
were checked from the bytes (`ghidra bytes`), not from listing text.

## Routines

| Address | C++ | Native ABI | Notes |
|---|---|---|---|
| 00603120 | `hud_pause_screen_enter_00603120` | fastcall(ECX screen), RET | seven pause rows, arrow placement, objective lists |
| 00603CA0 | `hud_pause_screen_update_00603ca0` | thiscall(screen, dt), RET 4 | dt unread |
| 00604190 | `hud_pause_on_listbox_activate_00604190` | thiscall(listener = screen+8, row, unused), RET 8 | |
| 005EEA00 | `hud_objectives_screen_enter_005eea00` | fastcall(ECX screen), RET | eleven bindings |
| 005EEF60 | `hud_objectives_screen_update_005eef60` | thiscall(screen, dt), RET 4 | x87 layout; dt unread |
| 005F9DD0 | `hud_order_screen_update_005f9dd0` | thiscall(screen, float dt), RET 4 | x87 fade and pulse |
| 005EA2D0 | `multi_main_screen_update_005ea2d0` | thiscall(screen, float dt), RET 4 | x87 scroll velocity |
| 005E2320 | `lobby_settings_option_label_005e2320` | fastcall(ECX NativeString* result, EDX slot) -> result | std::string projection |
| 005F54F0 | `options_build_main_list_005f54f0` | fastcall(ECX screen), RET | seven rows "Elem_<n>_Text" |
| 005F7C40 | `options_activate_list_row_005f7c40` | thiscall(listener = screen+8, unused, row), RET 8 | whole routine, 881 instructions |

| 005E4490 | `lobby_handle_gui_event_005e4490` | thiscall(this = lobby+0Ch, event widget), RET 4 | 856 instructions |
| 00602280 | `hud_pause_register_00602280` | fastcall(ECX screen), RET | three pages, 26 lookups, scroller wiring |
| 006037D0 | `hud_pause_update_hints_006037d0` | fastcall(ECX screen), RET | marks the selected hint line read |

- 006037D0: 00425E50 is the list box's current-row getter. It takes no stack arguments; the two
  pushes before it belong to the following 00AA7E00. The mark-read call (00690FB0) and the
  unread test on tracker+7Ch use the routine's local title name, which stays "" when no title
  precedes the row. The line list comes from `map[title]` through 005FF5D0, a std::map
  `operator[]` that can insert. Both details are kept.

- 00602280: the bindings were extracted mechanically (literal, ECX parent, EAX store). The
  scroll bar call 00683380 receives (thumb +7Ch, span, up +74h, down +78h) from the pushes at
  00602D81..00602D91. The span is the info slider's "back_Icon" height minus the thumb height,
  rounded once to float. The page/widget table `kWidgets3C` in src/hud_screens.cpp lists the
  same names without the parent and store offsets.

### MultiGameLobby HandleGuiEvent (005E4490)

- `this` is lobby+0Ch, and the B8h arm rebases ESI to the lobby (`add esi, -0Ch` at 005E46F0).
  The B8h-arm offsets were converted back to `this`-relative: `[esi+A8h]` is this+9Ch, the
  "Players_Listbox".
- The scrollers at this+24h, +C0h and +14Ch are embedded objects (`LEA ECX`). Auto-scroll uses
  direction 2/1 at speed 0.004f (00CEC178).
- The decompiler drops the third argument of the chat team buttons' vtable +88h; the bytes push
  (state, 0, 1.0f) for all three buttons.
- The second briefing route (A7h with the panel +80h focused) passes the local slot's +28h through
  005D8650 to 0051DCE0 on `[[00E198AC]+64h]` and requests interface 3. This matches
  docs/BRIEFING_SECOND_FILLER.md.

### Options (005F54F0 / 005F7C40)

- BuildMainList: 00A9CDF0 `BSP_GuiListbox_ClearOtherTwo` takes **EDX** as a register argument.
  Its body starts `MOV ESI, ECX; CMP EDX, ESI`. The call is (ECX keep, EDX list, stack list),
  and each list is skipped when it equals ECX. Here EDX = ECX = +84h, so only +88h is cleared.
- ActivateListRow: the earlier `bsp::options_activate_main_row_005f7c40`
  (options_menu_screens.hpp) models only the page-0 arm. The new routine covers the page 1-4
  and 7 apply/defaults/back rows and the second (non-list) row path. The routine moves ESI
  inside the body: it is the listener at entry, `add esi, -8` (screen) on the apply paths, and
  `add esi, 0D0h` (screen+D8h, the edit settings) on the defaults paths. The offsets were
  re-based per path. The dirty flag is screen+252h, the flag BuildMainList clears. 00585810 is
  called with `CL = 0` (`XOR CL, CL` at 005F7D3B). The transition timer +260h gates entry on
  exactly 0.0f (UCOMISS + `TEST AH, 44h; JP`), and every apply path sets it to 0.1f
  (00D7A2F0).

### HudPause (00603120 / 00603CA0 / 00604190)

- The seven rows are clones of the list box's `MainListbox_Text` template in the order resume,
  tips, points, options, restart, quit, exit, with keys read through the pointer table
  00E08C40..00E08C58 (`ingame.pause_*`, strings checked from the image). They are stored at
  +140h, +15Ch, +14Ch, +148h, +150h, +154h, +158h. The activate handler compares its row against
  `listener+138h..+154h`, which are the same slots, because the listener is screen+8.
- Arrow offset (006035EE..00603629): `off = (float)((float)((float)(w / 960.0) * 0.5) + 0.015f)`,
  where w is the tips row's +114h. 960.0 is the double at 00CEC380, and 0.015f is the
  float-widened double at 00CEE6C8. Every step ends in FSTP dword, so each intermediate is
  rounded to float. The left arrow goes at `pos - (off, 0.001f, 0.0)` and the right arrow at
  `pos + (off, -0.001f, +0.0)`. Stack slots were traced by hand from ESP.
- The objective lists come from `[game + 21A4h + [game+18ECh]*4] + 24h`, a std::list. The image's
  008DA180 is `LEA EAX, [ECX+24h]`. The active test 008DA130 reuses
  `cc13_band0085::objective_is_active_008da130`. Kind 0 fills +30h and kind 1 fills +34h; each box
  stops at 3 rows (+104h).
- Update: the arrows show only when an entry of the award tracker's std::map at +7Ch has a
  non-empty string list (node+14h, count +8h of that list). The image copy-constructs the list
  (004D48A0) only to read the count. The reconstruction reads the count in place and walks the
  tree through the host (004B7DE0).
- Activate: restart, quit and exit raise prompts with slots 4, 4 and 6, kind 1 (YesNo),
  callbacks 005FD5B0, 005FD610 and 005FC640, title "", timeout 0.0, countdown "" and dismissible
  1. The argument order was read from the pushes at 006042C6..0060430E. The tail toggles the
  selected hint's name in the expanded-hint list at +128h: it is removed through 005FE8B0, a
  std::list remove, when present and appended otherwise. The hint list is then rebuilt and the
  top row and selection restored.
- 006037D0 (`BSP_HudPauseScreen_UpdateHints`) and 00601200 (`BuildHintList`) stay behind the
  host. Both are open below.

### HudObjectives (005EEA00 / 005EEF60)

- Bindings were extracted mechanically, as literal, ECX parent and store. The list box gets
  +12Ch = 0.0, +130h = 1000.0f and +134h = 0.0, input filter 00696540, and listener screen+8.
- Layout block 005EF162..005EF2A0, traced from ESP with P/(w, h) the position and size of +1Ch:
  scale icon +20h at (P.x+0.001f, P.y+h, P.z), sized (its width, S); bottom icon +24h at
  (P.x+w, P.y+(float)(2h+S), P.z); text +28h at (P.x+0.02f, P.y+h, P.z-10.0). Here
  S = (float)((float)(lineheight/720.0) * (uint)lines). The line count is FILDed with the
  2^32 fix-up, so it is unsigned. The KisPocset marker sits at the selected row's position plus
  (0, 0, -1.0).
- Row colours by objective state: 1 is (0, 0.45f, 0, 1), 2 is (0.75f, 0, 0, 1), and anything
  else is a function-static white at 00E1961C (guard 00E1962C bit 0).
- **ABI note:** 006DBAC0 `BSP_GuiElement_ApplyHighlightPulse` takes **DL** as a register argument.
  It is the enable flag, and its first instructions are `MOV BL, DL; TEST BL, BL`. 005EF442 loads
  `DL = [screen+70h]` just before the call. `include/bsp/marker_manager_cc13.hpp:76` models only
  the three stack arguments.

### HudOrder update (005F9DD0)

- 005F9E36 is `D8 C9`, so `step = dt * 2.0` without an order code. 005F9E62 is `DE C9`, so the
  alpha decrement is `2.0 * step`. The fade timer is +70h, and alpha is reduced only once it
  exceeds 0.5f. Eight 004BEC00 singleton calls with discarded results sit between the fade and
  the close test; the host keeps them.
- Pulse +7Ch: steps of step*7.0 (00CED5D8), turning at 0.99f (00CED5D0, float widened) and
  0.01f (00D7A238). Without an order code it is pinned at 0.5f, rising.

### MultiMainMenu update (005EA2D0)

- The scroller at +6Ch is embedded (`LEA ECX, [ESI+6Ch]`), not a pointer. The velocity +F0h has a
  dead zone of +-1e-5f (0x3727C5AC / 0xB727C5AC). The step is (float)(2*v*dt), and the decay is
  `v - 2*(dt*v)`, with 005EA4AC `D8 C9`.

### LobbySettings option label (005E2320)

- This was already described in docs/MISSION_LOBBY_SETTINGS.md and left unimplemented in
  `MissionLua::lobby_settings_option_label` (src/game_hosts_lua.cpp, cc9's file). The
  reconstruction returns a std::string. Slot 0's players-ready count is used only when the game
  byte +218Ch is set and the effective mode (004BCA50) is above 3; otherwise it is "2".

### cc13-fe3 list (src/frontend_seg12_17_cc13.cpp, namespace bsp::cc13_fe3)

| Address | C++ | Native ABI | Notes |
|---|---|---|---|
| 00566080 | `hud_limbo_enter_00566080` | fastcall(ECX screen), RET | timer +18h = 6.0f (00CE6630) |
| 00561860 | `leaderboards_move_selection_00561860` | thiscall(screen, scroller, list box, clip, bool), RET 10h | x87 visibility test |

- 00561860: the routine overwrites its own argument slot `[ESP0+8]` with `&last` (00561888) and
  keeps `&selected` in a local, so the later `[esp+48h]` reads are the pointer, not the list
  box. Stack offsets were traced across the `push edi` / `pop edi` pair. Only the y components
  enter the visibility test. The x top-left values are computed and then discarded.
  The `+104h` row count is the initial "first".

## For cc9

- `lobby_settings_option_label_005e2320` is available for `MissionLua::lobby_settings_option_label`
  (src/game_hosts_lua.cpp) if the lane wants it. It calls 005E21B0 StepOption as a side effect
  when the stored option is disabled.
- The 006DBAC0 DL argument above affects `marker_manager_cc13.hpp`'s host signature.
- 005E4490 changes session slot state through host calls only: 00771E80
  `BSP_Session_RequestSlotAiEnable` and 0076CC40 `BSP_Session_SetSlotAiSkillLevel`, both with
  ECX = game+1EF0h. It also writes the briefing record `[[00E198AC]+70h]` +94h/+98h. These are
  pre-mission session settings; any host wiring of them is the gameplay lane's decision.

## Handoff (cc13-fe6, 2026-10-12)

Fourteen routines reconstructed and build-tested (no Ghidra renames this pass). Still open:

- **fe1 list:** 00601200 BuildHintList (about 4 KB, std::map/std::list heavy; host method in
  `PauseScreenHost`), 005AB190 HudMap register (11 KB; take it range by range with
  `local/fe6_binds.py`-style binding extraction), and 005BB130 HudNarrative register.
- **fe3 list:** everything in the docs/CC13_FE_3.md handoff except 00566080 and 00561860. That
  includes 005630F0 InsertN, a plain `vector::_Insert_n` instantiation over 40h-byte rows,
  left as a template.
- **Tools** (in this worktree's `local/`, not committed):
  - `fe6_calls.py`: per-call ECX/EDX and push summary from a `disasm-raw` dump.
  - `fe6_binds.py`: FindChildByName literal / parent / store extraction, with strings read from
    the PE.
  - `fe6_callrows.py`: builds the verify_report_calls rows. Note that `fe6_calls.py` follows
    linear flow, so check the ECX of any call reached by a jump into a shared tail by hand.
