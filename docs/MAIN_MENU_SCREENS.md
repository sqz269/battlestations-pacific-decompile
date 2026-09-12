# Main-menu screens (packet `main_menu_screens`)

Addresses: 005902e0, 005ca880, 00626630, 0051e4d0, 0052fce0, 00563370, 005098b0, 005884a0

## Correction from docs/GAME_TITLE_SOUND.md

The later concrete title composition corrects several interpretations below.
E19504 is a live C-string buffer passed to 0041E870, not a native eight-byte
string header. The cited 0058BE63/0058BE6F/0059A16B sites are reads, and its
track-selection producer remains unresolved. 0073DB76 reads F889A8 and writes
alternate-owner+218; it does not produce the music-volume global.

A85C20 requests fade-in: unsigned state+20 <=2 and clear byte+A permit setting
byte+B and positive-zero float+C. The older host's `play_stream` name does not
describe this operation. Full 005884A0 composition also requires all six current
menu reloads, actual eight-byte temporary ownership and A877D0's two consumed
arguments. The BF681B throwing allocator/new-handler path does not provide a
successful null-allocation policy. See the later document and report for the
complete sequence, native ABI, installed-stream fixture and application limits.
The old semantic wrapper remains pending replacement at the shared menu binding.

Read-only context: 004f7180, 004f71d0, 00e18b60 (`docs/GAME_FRONTEND_STATES.md`), 00686170,
00686380, 004cc460, 006840f0 (`docs/FRONTEND_MANAGERS.md`), 004e4000 (`docs/GAME_FRONTEND_ENTRY.md`),
00e08cd8 (`docs/GAME_SIMULATION_GATE.md`).

## The seven screens

`00686380` builds seven objects into `00E198AC+58h..+70h`. Every one of them is a leaf of the
`004F7180` hierarchy: each constructor is `__fastcall(this)` returning `this`, chains
`BSP_FrontEndScreen_Construct` first (which writes base vtable `00CEAE54` and clears the two flag
bytes at `+4h`/`+5h`), then overwrites `+00h` with its own primary vtable and installs one or two
secondary vtables at `+08h` and `+0Ch`. Each ends with the SEH frame restored and `RET`.

Slot `+00h` of every primary vtable is a five-byte `mov eax, imm32; ret` leaf. `004F71D0` calls it
with no arguments and uses EAX as the index into the 95-slot registry at `00E18B60`, so that
constant is the screen id. It is also the interface id: every one of the seven decodes through the
`00E08CD8` name table that `bsp::front_end_interface_name` already reconstructs, and the six that
have a distinctive layout or data-table name agree with the name at that index.

| Field | Constructor | Size | Primary vtable | Id | 00E08CD8 name | Id leaf |
| --- | --- | --- | --- | --- | --- | --- |
| +58h | 005902E0 | 578h | 00CEFC5C | 1 | `INTF_MAINMENU` | 00590570 |
| +5Ch | 005CA880 | 34h | 00CF170C | 2 | `INTF_MISSIONTREE` | 005CA8E0 |
| +60h | 00626630 | 260h | 00CF50CC | 4 | `INTF_REWARDS` | 00626710 |
| +64h | 0051E4D0 | 138h | 00CEC948 | 3 | `INTF_BRIEFING` | 0051E250 |
| +68h | 0052FCE0 | 5Ch | 00CED200 | 8 | `INTF_CREDITS` | 0052FD60 |
| +6Ch | 00563370 | 16Ch | 00CEE9CC | 0Ah | `INTF_LEADERBOARDS` | 005633F0 |
| +70h | 005098B0 | 5D0h | 00CEB9B4 | 0Bh | `INTF_ACHIEVEMENTS` | 00509D60 |

The construction order is not the id order: `00686380` builds ids 1, 2, 4, 3, 8, 0Ah, 0Bh, which is
why the briefing screen sits at `+64h` behind the rewards screen at `+60h`. The sizes are the
`operator new` arguments recorded in `docs/FRONTEND_MANAGERS.md`; each is consistent with the
highest field index its constructor writes (`005902E0` reaches `param_1[0x15D]` = `+574h`,
`005CA880` reaches `+30h`, `0052FCE0` reaches `+58h`, `0051E4D0` reaches `+134h`).

**The one naming mismatch.** Id 0Bh is `INTF_ACHIEVEMENTS` in the table, verified by reading
`00E08CD8+2Ch` = `00CF7604` = `"INTF_ACHIEVEMENTS"` directly, not by counting. But every string in
the `005098B0` class says tactical library: `FE_tactical_listbox`, `FE.taclib_orderofbattle_title`,
`FE.taclib_noseart_help`, `FE.unitlib_nounlock`, `FE.unitlib_nounit_d1..d3`, `UnlockID`,
`PowerupClasses`, `Scripts\datatables\PowerUpLib.lua`, `IsJapan`. The top-level menu item that
reaches it is `FE.main_tacticallibrary`, and both routines that request it (`005885D0`, `005886C0`)
push 0Bh. The table also carries an unused `INTF_UNITLIB` at 5 and `INTF_MEDALS` at 7. The most
economical reading is that the slot was renamed during development and the id table kept the old
label; this is **not resolved** and the id, not the label, is what the code uses.

### The ten virtual slots

The base table `00CEAE54` is: `00BF698E` (`__purecall`), `004F7570`, `004F7580`, `004F75E0`,
`004F71D0`, `004F7590`, `004F75A0`, `004F75B0`, `004F75C0`, `004F75D0`. `004F8B50` follows at
`00CEAE7C`, but every derived table in this packet stops after `+24h` and is followed immediately by
string data (`00CEFC84` = `"FE_pc.profile_cantcreate"`, `00CEC970` = `"FE_briefing_grid"`, and so
on), so the table is **ten slots** and `004F8B50` is the next `.rdata` item. This corrects the
implicit reading of `docs/GAME_FRONTEND_STATES.md`, which lists slots but not the count.

| Field | +0Ch destroy | +10h register | +14h | +18h enter | +1Ch exit | +20h update | +24h |
| --- | --- | --- | --- | --- | --- | --- | --- |
| +58h | 00590D60 | 00582F30 | 005861B0 | 005987F0 | 0058F560 | 00599DB0 | 005905E0 |
| +5Ch | 005CAA20 | 005CAAF0 | base | 005C27F0 | 005C2800 | 005C4040 | base |
| +60h | 00627800 | 00629D10 | 0061FBE0 | 0062C2C0 | 00621720 | 0062B640 | 00626760 |
| +64h | 0051E5F0 | 0051E280 | base | 0051CDE0 | 0051B1B0 | 0051C7D0 | 0051E270 |
| +68h | 0052FEF0 | 0052FFC0 | base | 0052EE30 | 0052E150 | 0052E4B0 | 0052FD70 |
| +6Ch | 005635D0 | 00564220 | base | 00562750 | 00560FE0 | 00565540 | 00563430 |
| +70h | 0050D8E0 | 0050FDB0 | 004FE870 | 00511F40 | 0050B2D0 | 005162B0 | 00509DA0 |

"base" is `004F7590`, the untouched base slot. Only the two largest screens override `+14h`, and in
both of them that override is the layout binder rather than the `+10h` register.

`docs/FRONTEND_MANAGERS.md` reports that each screen's `+10h` runs immediately after construction.
Five of the seven `+10h` overrides call `BSP_FrontEndScreen_Register` and `BSP_GuiManager_GetOrCreate`
in the same body, so registering the screen id and binding its GUI layout is one step for them. The
`+10h` of the two screens that override `+14h` splits the work: `00582F30` registers and then calls
`00B29E60` (presentation mode) and `00AB6BD0`; `0050FDB0` registers and binds `FE_tactical_listbox`
while `004FE870` binds the widget set.

### Layouts and data tables per screen

Names below are string references in the routine named, read through `bsp.py lookup`.

- **+58h, `INTF_MAINMENU`.** `005861B0` (slot `+14h`) binds `FE_main` and `FE_main_listbox`, and
  also names `FE_briefing`, `FE_briefing_grid` and `FE_worldmap_historical`, so this 578h object
  carries the campaign map and briefing surface as well as the top-level list. Widgets include
  `Main_Listbox`, `MainListbox_Text`, `content_main_Text`, `missions_US_Group`, `missions_JP_Group`,
  `missions_US_DLC_Group`, `missions_JP_DLC_Group`, `training_Group`, `historical_Group`,
  `mission_mapflag_template_Icon`, `mission_mappoint_template_Icon`, `checkpoint_Text`,
  `bg_01_anim_Movie`. The constructor also clears four dwords at `00E194C4..00E194D0` and sets
  `DAT_00E08874` to 1.
- **+5Ch, `INTF_MISSIONTREE`.** `005CAAF0` reads `Scripts/datatables/MissionTree.lua` through the
  keys `multiMissionInfos` and `missionGroups`, reporting progress via `BSP_LoadingScreen_ReportProgress`.
  There is no GUI layout in this class: it is the tree data, not a page.
- **+60h, `INTF_REWARDS`.** `00629D10` binds `FE_scoring` and names its six pages:
  `FE.scoring_single_page1`, `FE.scoring_multi_page1`, `FE.scoring_objectives`,
  `FE.scoring_detailed_score`, `FE.scoring_rank`, `FE.scoring_awards`. The enter virtual `0062C2C0`
  is the widest routine in the packet (100 callees) and drives `FE.scoring_unlock_text`,
  `FE.scoring_unlockedunit`, `award_title_Text`, `award_detail_Text`, `debriefing_Clipbox`.
- **+64h, `INTF_BRIEFING`.** `0051E280` binds `FE_briefing_grid` and `FE_briefing_listbox` with
  `Main_Listbox` and `MainListbox_Text`. The enter virtual `0051CDE0` builds the objective groups
  (`_pri_objective_Group`, `_sec_objective_Group`, `PRIMARY OBJECTIVE`, `SECONDARY OBJECTIVE`) and
  the mode groups `Island_Capture_Group`, `Competitive_Group`, `Siege_Group`, reading the mode from
  `BSP_Game_GetEffectiveGameMode`.
- **+68h, `INTF_CREDITS`.** `0052FFC0` binds `FE_credits` and reads
  `Scripts/datatables/Credits.lua` through `Credits`, `Credits_Items`, `RowSpace`, `StartY` and
  `Speed`, with `moving_Group`, `clipbox_Clipbox`, `person_template_Text`, `category_template_Text`.
  Its enter virtual `0052EE30` sets the `FE.main_credits` title, offers `globals.back`, and is one
  of the six callers of `00A867B0`, so this is the screen that opens
  `sound/music/creditsfinal.fsb` from the manager's `+48h`.
- **+6Ch, `INTF_LEADERBOARDS`.** `00564220` binds the list and scroll widgets (`data_Listbox`,
  `slider_Group`, `slider2_Group`, `clipbox_Clipbox`, `rank_Text`); the vtable is followed by the
  keys `Columns`, `TableDesc`, `ColumnName`, `LBoards`, `LB_UniqueID`, `Leaderboards`. Enter
  `00562750` sets `FE.main_leaderboards_title`. `005629A0` in this class writes main-menu page 0Bh.
- **+70h, `INTF_ACHIEVEMENTS` / tactical library.** `0050FDB0` binds `FE_tactical_listbox` and names
  `common/whiteGui.tga`, `fe/lobby/unlock_disabled.tga`, `colourremap.tga` and `RANK_`. `004FE870`
  binds the widget set (`units_Group`, `noseart_Group`, `tutorial_Group`, `INFO_box_Group`,
  `Main_Listbox`, `MainOOB_Listbox`). Enter `00511F40` drives the page (`globals.navigate`,
  `globals.select`, `globals.3dview`, `FE.taclib_orderofbattle_title`, `video_Movie`, `lock_Icon`).

## The main-menu page state, 00E08874

The dword at `00E08874` is the page the `005902E0` screen is showing. It is **not** an interface id:
the manager stays on `INTF_MAINMENU` while this walks its own set. Values read off
`mov dword ptr [0xE08874], imm` at each site:

| Page | Written at | In | Evidence |
| --- | --- | --- | --- |
| 1 | 0059054F, 00584B91, 0058F594 | ctor, 00584AE0, exit virtual | `FE.main_menu`; the exit virtual resets it |
| 2 | 00584FE7, 005809BF | 00584F50, 00580940 | `FE.main_singleplayer_title` |
| 3 | 00585458 | 005853C0 | `FE.main_multiplayer_title` |
| 4..7 | 005978A7 (from a parameter) | 00597870 | `FE.main_checkpoint`, `FE.main_checkpoint_dlc`, `FE.main_usn_dlc_title`, `FE.main_ijn_dlc_title` |
| 8 | 0058099A | 00580940 | unidentified |
| 9 | 0058CAC3 | 0058C010 | `globals.continue`, `mission_mappoint_`, `sound/messages/` |
| 0Bh | 0058877C, 005629B6 | 005886F0, 005629A0 | `FE.main_tacticallibrary` |
| 0Ch | 00594C51 | 00594BF0 | `globals.obj_pri`, `globals.obj_sec`, `globals.obj_hid` |

The back handler `00598B60` reads it and splits: page 1 goes to `00588A80`
(`FE_pc.main_quit_confirm`), and pages 4, 5, 6 and 7 form one group at 00598B7B..00598B9C that
caches `00A9C990`'s result into `00E08878` and `00E194DC` and writes 1 into screen `+55Ch` for **5
and 7 only**, which is what marks those two as the downloadable-content arms. `00580940` writes
pages 8 and 2 and two more from registers that were not followed.

Writers outside this packet's classes: `004DB19A` in `BSP_FrontEnd_TeardownForSession`, `00515DDA`,
`006866BF` in `BSP_MainMenu_Init` (the arm `docs/FRONTEND_MANAGERS.md` records as raising the
global when `*(game+1EE1h)` is set).

## The top-level menu, 00584AE0

`00584AE0` is `__fastcall(this)`. It opens the `FE.main_menu` layout, sets page 1, hides the object
at `this+2F0h` through its virtual `+34h`, enables the one at `this+1B8h` through `+60h`, and then
runs a seven-pass loop at 00584BF0..00584C70. Each pass allocates a list entry through `00AAB4C0`,
labels it with `00E087B8[i]` via `00ABBE50`, stores `i` into `entry+0D8h`, and writes
`enable[i] == 0` into the byte at `entry+77h`. The enable table is the seven bytes at `00CEF77C`,
all `01` in this image, so nothing is disabled on this build. Ghidra types `00CEF77C` as a pointer
(`PTR_DAT_00CEF77C`); it is a byte array, and the decompiler's `(int)&PTR_DAT_00CEF77C + i` confirms
the code indexes bytes.

| Index | Label pointer | Label |
| --- | --- | --- |
| 0 | 00CEF764 | `FE.main_singleplayer` |
| 1 | 00CEF750 | `FE.main_multiplayer` |
| 2 | 00CEF738 | `FE.main_tacticallibrary` |
| 3 | 00CEF728 | `FE.main_options` |
| 4 | 00CEF718 | `globals.live` |
| 5 | 00CEF700 | `FE_pc.main_marketplace` |
| 6 | 00CEF6F0 | `FE_pc.main_quit` |

The eighth pointer, `00CEF6D4` = `FE.main_singleplayer_help`, is past the loop bound and is the help
line for item 0, not an item.

**What selection enqueues.** None of the seven screens pushes onto the `game+5D8h` state-request
deque of `docs/GAME_FRAME_CONTROL.md`: every producer of that deque listed there is in `004D`/`004E`,
and no site in `0050`..`0062` appears. The screens request a **front-end interface** instead, through
`BSP_FrontEndManager_PushInterfaceRequest` (`004CC460`) on `00E198AC`, which writes only the pending
record at `+20h`/`+38h` and leaves `006840F0` to drive the manager's virtual `+10h` on the next
servicing pass. So a menu selection is an interface transition, and the state-request deque is
reached only later, by the shell. The chain for quit is the visible exception: item 6 leads to
`00588A80` (`FE_pc.main_quit_confirm`), which is the confirmation dialog, and request `13h` (the
quit path in the drain) is enqueued outside these classes.

Two request sites in this packet were read in full, both opening the tactical library:

- `005885D0`: `__thiscall`, ECX = the main-menu screen, no stack arguments, `RET`. Resolves two
  values (`005806A0` then `005C27E0`, and `005806A0` again), stores them into
  `*(00E198AC)+70h` at `+9Ch` and `+A0h` with the byte at `+A4h` cleared, writes mode 5 at `+94h`
  and selector `63h` at `+98h`, then calls `004CC460(0Bh, 0)`.
- `005886C0`: no arguments, `RET`. Calls `004CC460(0Bh, 0)` **first**, then writes mode 4 and
  selector `63h`. The reversed order means a servicing pass that runs between the two can see the
  request with the previous mode still in place; whether that is reachable was not checked.

## 005884A0, the routine the shell drives

**The entry doc's reading of this call is wrong and is corrected here.** `004E426B` loads
`ECX = *(00E198AC)+58h` and `004E4274` calls `005884A0`, but the body never reads ECX. The only
object it touches is the manager at `00E198AC`. It is a method on the `+58h` screen class whose body
ignores its own `this`; the effect is entirely on the manager, and nothing in the `+58h` screen is
read or written. `docs/GAME_FRONTEND_ENTRY.md` line 71 should read "`005884A0`, whose `this` is
unused".

Calling convention: no stack arguments, ECX unused, `RET` with no operand, no return value. The
`PUSH ECX` at 005884F1 is stack reservation, not an argument.

It starts the title music, which resolves the `+50h` handle that
`docs/FRONTEND_MANAGERS.md` marks provisional as "the streams the two paths open into". Body:

| Address | Step |
| --- | --- |
| 005884B9 | `00584A30()`; non-zero returns immediately |
| 005884CB | `*(00E198AC)+50h != 0` returns immediately |
| 005884D5 | `operator new(54h)`; on failure EAX is zeroed at 0058852F |
| 0058851C | `00426060(temp, 00E198AC+40h)`, a copy of `sound/music/titlescreen.fsb` |
| 00588528 | the stream constructor `00A877D0(this = the 54h block, temp)` |
| 00588537 | `*(00E198AC)+50h = EAX`, null included |
| 0058854A | `BSP_NativeString_Assign` from the native string global at `00E19504` |
| 0058855C | `00A867B0(stream, name)`, which reaches FMOD `createStream` |
| 00588580 | the temporary released through the sized storage pool |
| 0058859F | `00A864F0(stream, *(float*)00F889A8)` |
| 005885AD | `00A85C20(stream)` |

`00584A30` builds the literal `movies/midwaytheme.bik` and passes it to `004F8BA0`, which compares
it against the active clip through `BSP_NativeString_EqualsInsensitive` and `00AAC8E0`. So the gate
is "the intro movie is playing": the title music does not start over it. `00584A30` has exactly one
caller, this routine.

`00F889A8` is a float written by `BSP_Application_Initialize` at 0073DB76 and read by `00685C80`
(two sites) and `BSP_Settings_ApplyAudio` is a caller of `00A864F0`, so `00A864F0` is the volume
setter and `00F889A8` the configured music volume.

**No null guard.** When `operator new` fails, `+50h` becomes null and the code still calls
`00A867B0`, `00A864F0` and `00A85C20` through it. The reconstruction preserves that sequence rather
than adding a guard the binary does not have.

## Uncertainties

- `DAT_00E19504` is a native string global written at 0058BE63/0058BE6F (`0058BDF0`) and 0059A16B,
  none of them in this packet. What selects the track name passed to `createStream` is unresolved,
  so "title music" rests on the `+40h` path the stream is constructed from, not on that name.
- Page 8 (`00580940`) and page 9 (`0058C010`) are named only by their string sets. `00580940` writes
  two further page values from registers that were not followed.
- `00597870` writes the page from its first stack parameter. The four campaign pages are attributed
  to it by `00598B60`'s four-way test plus the DLC title strings in the same body; the parameter's
  call sites were not enumerated.
- The `+58h` screen's update virtual `00599DB0` and its `+24h` slot `005905E0` have **no Ghidra
  function defined** (`ghidra proto` reports `body ?`). They were identified from the vtable only
  and their bodies were not read. The orchestrator must define functions at both before applying any
  name to them.
- Object sizes are taken from `docs/FRONTEND_MANAGERS.md`'s record of the `00686380` allocation
  sites, not re-read here; each is consistent with the highest field its constructor writes.
- `005098B0` was read only as far as `param_1[0x127]`; its tail was not paged in.
- The secondary vtables at `+08h`/`+0Ch` (and `+40h`, `+64h`, `+68h` in the two large screens) are
  recorded but not decoded; which interface each represents is unknown.
- `005885D0`'s two selection values come from `005806A0` and `005C27E0`, neither of which was read.
- Whether the reversed order in `005886C0` is observable was not checked.

## What remains

The enter and update virtuals of all seven screens are named and their string sets recorded, but
only `005884A0`, `00584AE0`, `005885D0` and `005886C0` were read to the instruction. The interior of
`0062C2C0` (100 callees), `00511F40` (47) and `005987F0` (25) is untouched.

## Follow-up packets proposed

- `main_menu_page_machine`: 00584AE0, 00584F50, 005853C0, 00597870, 0058C010, 00594BF0, 00580940,
  00598B60, 00588A80. Files `docs/MAIN_MENU_PAGES.md`. Contract: the full `00E08874` page machine,
  what page 8 and the two register-written pages are, and the quit path from
  `FE_pc.main_quit_confirm` to state request 13h.
- `rewards_screen_debrief`: 00626630's 0062C2C0, 0062B640, 0061FBE0, 00629D10. Files
  `docs/REWARDS_SCREEN.md`. Contract: the six `FE.scoring_*` pages, the unlock and award text, and
  the `260h` field layout.
- `tactical_library_screen`: 005098B0's 0050FDB0, 004FE870, 00511F40, 005162B0, 005885D0, 005886C0.
  Files `docs/TACTICAL_LIBRARY.md`. Contract: the `+94h` mode set, the `5D0h` layout, and whether id
  0Bh's `INTF_ACHIEVEMENTS` label is vestigial.
- `front_end_music`: 005884A0's neighbours 0058BDF0, 0059A160, 0052EE30, 00A877D0, 00A867B0,
  00A864F0, 00A85C20. Files `docs/FRONT_END_MUSIC.md`. Contract: the `00E19504` track-name global,
  the stream object built by `00A877D0`, and how `creditsfinal.fsb` at manager `+48h` reaches `+54h`.

## State reached

| Address | State |
| --- | --- |
| 005884A0 | analyzed, reconstructed, build-tested |
| 005885D0, 005886C0 | analyzed, reconstructed, build-tested |
| 00584AE0 (item table and page) | analyzed, reconstructed (data only), build-tested |
| 005902E0 | analyzed (class, id, vtable, field zeroing; the 578h layout is not recovered) |
| 005CA880 | analyzed (class, id, vtable, data table) |
| 00626630 | analyzed (class, id, vtable, layout and page names) |
| 0051E4D0 | analyzed (class, id, vtable, layout names) |
| 0052FCE0 | analyzed (class, id, vtable, layout and data table) |
| 00563370 | analyzed (class, id, vtable, layout and data table) |
| 005098B0 | analyzed (class, id, vtable, layout; constructor tail not read) |
| 00584A30 | analyzed |
| 00599DB0, 005905E0 | exported only; no Ghidra function exists at either |

## Corrections offered to other packets

- `docs/GAME_FRONTEND_ENTRY.md`, the 004E426B row: `005884A0` does not act on the screen at `+58h`.
  ECX is loaded and ignored. The row should say it starts the title music on the manager.
- `docs/FRONTEND_MANAGERS.md`, the `+50h`/`+54h` row: `+50h` is no longer provisional. `005884A0`
  writes it with the stream built from the `+40h` path (`sound/music/titlescreen.fsb`). `+54h` is
  still unwritten in anything read here; `0052EE30`, the credits screen's enter virtual, is the
  matching caller of `00A867B0` for the `+48h` path.
- `docs/GAME_FRONTEND_STATES.md`, the vtable table: the hierarchy has ten slots, not eleven.
  `004F8B50` at `00CEAE7C` is the next `.rdata` item. Slot `+14h` (`004F7590`) is missing from that
  table and is overridden by two of the seven screens as the layout binder.
- `docs/GAME_SIMULATION_GATE.md`, the interface id table: ids 1..13h are all decoded and already
  match `src/frontend_managers.cpp`'s `kInterfaceNames`; that array is correct as written.

## Reconstruction

`include/bsp/main_menu_screens.hpp` and `src/main_menu_screens.cpp` hold the seven recovered classes
as data (`kMainMenuScreens`), the `MainMenuPage` enum with its evidence sites, the seven-item
top-level table (`kMainMenuItems`), the two tactical-library request shapes, and
`start_title_music_005884a0` over a `TitleMusicHost` with one method per native call site, in the
style of `bsp::run_application_frame`. Interface ids and their names are reused from
`include/bsp/frontend_managers.hpp`; the screen base and the registry from
`include/bsp/frontend_states.hpp`. Nothing is duplicated and no global or type is invented.

## Corrections from docs/MAIN_MENU_SCREEN_UPDATE.md

- The Options item enqueues state requests 6 and 14h, so a menu item does reach the game state deque.
- Pages 4 to 8 bind the Japanese, American, training, Japanese-DLC and American-DLC mission groups, not the order guessed above; page 0Ch is written by `00594c51`.
- The update virtual `00599db0` is `__thiscall` taking one float (the frame delta) and ends `RET 4` at `0059a6e0`; the sixteen returns Ghidra reports are inlined string-destructor artifacts. Action 4Ah is accept, 4Bh back, 50h a third action, settled against the GUI handler `005993a0`; `004d92b0` is an edge query with a 0.4 s delay and 0.1 s repeat.
