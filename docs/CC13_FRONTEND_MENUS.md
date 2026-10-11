# Frontend menus: list pages, new-profile entry, mission map icons (packet `cc13_frontend_menus`)

Addresses: 005805A0, 00580530, 00584F50, 005853C0, 005886F0, 00588370, 00590E60, 005807D0,
00580780, 00585EB0, 0058D9E0, 005845D0, 00585C50, 00586150, 0058CE30, 00583E50, 00588280.
Files: `include/bsp/frontend_menus.hpp`, `src/frontend_menus.cpp`,
`reports/cc13_frontend_menus.json` (call-site rows, `tools/verify_report_calls.py`: 58 rows, 0 failed).

Segment 20 (005805A0..005C5600) holds the main-menu screen (`FE_main`, constructor 005902E0,
vtable 00CEFC5C), the in-game tactical map screen (`GUI_map`, vtable 00CF0134), the voice
manager and narrative screen, the minimap and the mission-tree screen. Most of the main-menu
spine was reconstructed in September (docs/MAIN_MENU_*.md). This packet closes the three page
builders the `main_menu_page_builders` follow-up left open, the text owner the screen keeps at
+0Ch, and the routine docs/GAME_EXECUTABLE.md item 3 left open: what clones the two map-icon
templates into the numbered `mission_mappoint_<n>_Icon` / `mission_mapflag_<n>_Icon` icons.

Status vocabulary: **exported** (pseudocode in `exports/`), **reconstructed** (C++ here),
**build-tested** (compiles in `scripts/build.ps1`, Win32 Release). Nothing in this packet is
fixture-tested, ABI-compatible or game-validated; no `bsp_game` run was made.

## Skipped parts of the segment

- `005ADDA0`, `005AE5B0`, `005B25D0`, `005B3470` (the map screen's enter), `0059A9B0`,
  `005AB190`, `005BC640`: named by cc12's GlobalConfig audits of 2026-10-08
  (docs/CC12_GLOBAL_CONFIG_*). Not touched.
- `005B5C00`..`005BCFFF` (voice manager, blackout fade, narrative screen): cc9 lanes,
  docs/MISSION_BLACKOUT.md (2026-10-06) and docs/SQUADRON_LAND_TASK.md. Not touched.
- `0059BBD0` (docs/CC11_UNIT_ROLE_PERMISSION.md). Not touched.

## Routines

| Address | Name (hypothesis) | ABI | Coverage | Status |
|---|---|---|---|---|
| 005805A0 | `BSP_MissionTree_HasIjnDlcMissions` | `__thiscall`, ECX = *(00E198AC)+5Ch, RET | complete | reconstructed, build-tested |
| 00580530 | `BSP_MissionTree_HasUsnDlcMissions` | same | complete | reconstructed, build-tested |
| 00584F50 | `BSP_MainMenu_BuildSinglePlayerPage` | `__thiscall`, ECX = screen, RET | complete normal path | reconstructed, build-tested |
| 005853C0 | `BSP_MainMenu_BuildMultiplayerPage` | same | complete normal path | reconstructed, build-tested |
| 005886F0 | `BSP_MainMenu_BuildTacticalLibraryPage` | same | complete normal path | reconstructed, build-tested |
| 00588370 | `BSP_NewProfileTextOwner_OnTextChanged` | `__thiscall`, ECX = screen+0Ch, RET | complete | reconstructed, build-tested |
| 00590E60 | `BSP_NewProfileTextOwner_OnCommit` | same | complete normal path | reconstructed, build-tested |
| 005807D0 | `BSP_NewProfileTextOwner_OnCancel` | same | complete | reconstructed, build-tested |
| 00580780 | `BSP_MainMenuScreen_CancelNewProfileUnreferenced` | `__thiscall`, ECX = screen | complete (same body) | named only |
| 00585EB0 | `BSP_NativeString_TrimLeftSet` | `__thiscall`(string, set), RET 4 | std::string projection | reconstructed, build-tested |
| 0058D9E0 | `BSP_MainMenuScreen_BuildMissionMapIcons` | `__thiscall`(screen, bool), RET 4 | complete normal path | reconstructed, build-tested |
| 005845D0 | `BSP_MainMenuScreen_AdvanceBackgroundMovie` | ECX = screen | read | named only |
| 00585C50 | `BSP_MainMenuScreen_PlayTestSpeech` | ECX = screen, two stack args | read | named only |
| 00586150 | `BSP_PlayerProfile_CopyMissionId68` (provisional) | `__thiscall`(profile, out) | complete | named only |
| 0058CE30 | `BSP_MainMenu_ShowQueuedNoticePrompt` (provisional) | ECX unused | read | named only |
| 00583E50 | `BSP_MainMenu_ClearObjectiveRows` | ECX = screen, RET | read | named only |
| 00588280 | `BSP_MainMenuWidgetListener_Current1C` | ECX = screen+40h | read | named only |

"Complete normal path": SEH unwinding, pool allocator failure and NativeString storage are
replaced by `std::string`; nothing else is omitted.

## The list-page builders

00588A80 (`BSP_MainMenu_ActivateTopLevelItem`) calls 00584F50 for item 0, 005853C0 for item 1 and
005886F0 for item 2 (main_menu_screen.hpp `kMainMenuItemActions`). The enter 005987F0 calls all
three, and 00584F50 is also called from 005993A0 (00599898) and 00599DB0 (0059A6A2). The three share one body, so the reconstruction is one function driven by
`MainMenuListPageSpec`; the differences are recorded flags, each read off the listing:

| Step | 00584F50 | 005853C0 | 005886F0 |
|---|---|---|---|
| 00518D60(ECX 0Ch, EDX 0, 0, title) | 00584FB1 `FE.main_singleplayer_title` | 00585420 `FE.main_multiplayer_title` | 00588750 `FE.main_tacticallibrary` |
| 00518B90(ECX 2) | 00584FE2 | 00585453 | absent |
| 00E08874 = page | 2 | 3 | 0Bh |
| hide +2F0h | yes | yes | no |
| hide +3DCh, +1B8h, +41Ch (virtual +34h(0)) | yes | yes | yes |
| 00A9AC40 listener null / screen+8 | yes / yes | yes / yes | no / no |
| 005884A0 title music after clearing | no | no | 005887BF |
| list box byte +149h = 1 | 00585046 | no | no |
| rows | 6 from 00E087F0 | 2 from 00E08820 | 7 from 00E08830 |
| remembered row | 00E194C8, in loop, `count-1` | 00E194CC after loop | 00E194D0 after loop |
| virtual order after rows | +80h, +34h, +60h | +80h, +34h, +60h | +34h, +60h, +80h |
| byte +14Ah = 1 | yes | yes | no |
| 00A9CF30(1, 1) | yes | yes | yes |
| 00AA8240 from +558h..560h, 00AA6BC0(screen+40h, 0) | yes | yes | no |
| 0054B530 commands | select A2h/0, navigate B3h/1, empty, back A3h/2, empty | same | same |
| 00E08878 = -1 | yes | yes | no |
| 00E198B4 tail (005EA7F0 on +44h, (+40h)+F4h = 0) | no | 005857C2..005857F5 | no |

Single-player rows: `FE.main_ijn_campaign`, `FE.main_usn_campaign`, `FE.main_ijn_dlc`,
`FE.main_usn_dlc`, `FE.main_training_grounds`, `FE.main_skirmish`. Row 2 exists only when
005805A0 answers true, row 3 only when 00580530 does (00585050..00585079). Each row's +D8h gets
the item index and +77h gets `00CEF784[index] == 0`; all six bytes at 00CEF784 are 1. Because
DLC rows can be skipped, the remembered index is matched against the item index and the list
selects `row count (+104h) - 1`, the row just inserted. Multiplayer rows: `FE_pc.main_live`,
`FE_pc.main_lan`. Tactical-library rows: `FE.main_navalacademy`, `FE.main_unitsweapons`,
`FE.main_powerups`, `FE.main_movies`, `FE.main_personalachievements`, `FE.main_leaderboards`,
`FE.main_credits`.

The command-bar triples match the selection listener's (docs/MAIN_MENU_SELECTION_LISTENER.md)
without its zoom and scroll entries; the two empty labels are the C string 00CE3A0C.

## The DLC gates

005805A0 and 00580530 differ only in the index: vector::at(3) / at(4) on the mission tree's 34h
group vector (+18h/+1Ch, 00BF6713 when null or too short), then the group's 434h mission vector
(+28h/+2Ch inside the entry) is tested non-empty with a signed divide by 434h. Their caller
loads ECX from *(00E198AC)+5Ch at 00585055 / 00585069. Tree group numbering, from 0058D9E0
below and these gates: 0 training, 1 IJN, 2 USN, 3 IJN DLC, 4 USN DLC.

## The mission map icons (0058D9E0)

Called with 0 from the enter 005987F0 and with 1 from the exit 0058F560 (main_menu_screen.hpp
records "0058D9E0(1)" on the exit path).

- Destroy (0058DA0D..0058DA44): 00AA83F0 on +314h, +310h, +320h, +31Ch, +318h, in that order,
  then +330h = 0, RET 4. 00AA83F0 walks the widget's child list (+68h), deletes the children
  whose +76h byte is set through virtual +4(1) and recurses into the rest.
- Build: five unrolled loops, each re-reading the tree through 00E198AC and checking the group
  vector's length first.

| Loop start | Tree group | Parent widget | Point vector | Flag virtual +88h(1, 0, 1.0f) |
|---|---|---|---|---|
| 0058DA65 | 1 | +314h `missions_JP_Group` | +70h (0043DCA0) | no |
| 0058DE17 | 2 | +310h `missions_US_Group` | +90h (inline, 0043DB60) | 0058E1FD |
| 0058E227 | 0 | +320h `training_Group` | +B0h | no |
| 0058E623 | 3 | +31Ch `missions_JP_DLC_Group` | +D0h | no |
| 0058EA23 | 4 | +318h `missions_US_DLC_Group` | +F0h | 0058EE0D |

Per mission, the ordinal restarting at 1 in each group: clone `mission_mappoint_template_Icon`
(+114h) under the parent (00AAB4C0, ECX template, stack parent), name it through virtual +2Ch
`mission_mappoint_<n>_Icon` (004263B0 int append, then concatenation with `_Icon`), place it
with 00AA7DC0 at the record's +6Ch `Pos`, append it to the point vector, then clone
`mission_mapflag_template_Icon` (+118h) under the same parent, name it
`mission_mapflag_<n>_Icon`, place it at the same `Pos`, and on the two US groups set its frame
through virtual +88h(1, 0, 1.0f) (the same virtual 00518B90 uses on `frame_flag_Icon`).

This answers docs/GAME_EXECUTABLE.md item 3: the numbered icons are created at enter time from
the two templates, so the empty authored groups in this installation are expected. The tree
group numbering here differs from the `MissionGroup` enum in `include/bsp/main_menu_screen.hpp`
(00599DB0's 0 IJN, 1 USN, 2 training, 3 IJN DLC, 4 USN DLC); both are recorded, neither is
renamed. Which screen vector each loop appends to (+70h, +90h, +B0h, +D0h, +F0h) refines that
header's "ten 10h-byte vectors at +70h..+10Ch": the even ones hold the mission points. The odd
ones (+80h..+100h) are not written here.

0058D9E0's Ghidra body runs past its last RET into an undefined function at 0058EE40 (a
std::map lookup-or-insert: 00582400 find, 00443D00 compare); 0058EEDD's call to 0058D130 belongs
to that function, not to 0058D9E0.

## The new-profile text owner

The screen constructor stores vtable 00CEFC24 at screen+0Ch (00590334, EDI = ESI+0Ch); the
vector-deleting destructor 00590600 restores it (00590632). 00A96750, the base key handler,
calls slot +4 after an edit (the backspace path is the one read), +8 on `\r` and +0Ch on Esc. Owner fields: +18h text
NativeString, +20h cursor, +24h the name Text widget (screen+30h).

| Slot | Address | Behaviour |
|---|---|---|
| +0 | 0057D3E0 | no Ghidra function starts there (inside 0057D3B0's body); not read |
| +4 | 00588370 | text longer than 15 -> substr(0, 15) and cursor clamped to 15 (signed compare with 10h); 00ABAED0(text, 0); 00AB95F0 caret mode `00E12F34 == 0`; 00AB9240(cursor + 1) |
| +8 | 00590E60 | commit, below |
| +0Ch | 005807D0 | `FE.main_newprofile` through 00ABBE50(…, 1); caret mode 2 (hidden); 00A966E0(0, 0); top-level list box virtual +60h(1) and +77h = 0 |

00590E60: the cancel sequence first (00590E86..00590ECC), then nothing when the text is empty.
Otherwise the name is the text with leading spaces removed (00585EB0, set " " at 00CE3A90),
trailing spaces removed except the first byte (00584110), and `globals.newplayer` when that is
empty. With the storage object *0109CECC ready (+21h) and its virtual +14h(&name) false, it
raises 00531B00 with `FE.profile_nameconflict` when virtual +18h answers true and
`FE_pc.profile_cantcreate` otherwise (00590F78..00590FE0), callback 0057D3A0. Otherwise it
resets the profile (007FDB20 on *(00E188A8)+650h), sets its name (007F9290), resets the input
settings unless 00F88A30 is set (005547D0, 006AC030), and requests a profile write (007FA710
with completion callback 00590E40) only when the name is not `globals.newplayer`
case-insensitively (00449AF0) and the storage is ready and accepts it; then 00584AE0(0).

00580780 is the cancel body addressed from the screen itself (text widget +30h, owner +0Ch, list
box +1B8h). A rel32/absolute scan of the image finds no reference to it.

00585EB0 is the left-trim partner of 00584110: when the set is non-null and non-empty and the
data non-null, `strspn` from the front; a span equal to the length erases through 005558F0(0,
7FFFFFFFh), a smaller non-zero span memmoves and resizes. The reconstruction works on
`std::string` and does not stop at an embedded NUL as `strspn` would.

## Named without reconstruction

- 005845D0: `+58h = (+58h + 1) % 3`, 00AAC870 movie stop, source
  `movies\FE_bg_0<+44h[+58h]>.bik`. Its only entry is the tail jump at 005861A8 in the raw
  movie-completion callback 005861A0 that 005861B0 binds (main_menu_layout_binding.hpp).
- 00585C50: acquires a named sound sample into +570h, finds sound class `GUITestSpeech` once
  (cached at 00E1950C, guard 00E19510) and type `Normal`, and creates a tracked sound into +574h.
  Called only from 00588460, an undefined stdcall(2) callback whose address 0058C010 stores at
  0058C9BF.
- 00586150: copies the NativeString at +68h of its `this` into the out argument. 00590DCD passes
  the player profile (*(00E188A8)+650h) and hands the copy to 005C35D0
  (`BSP_MissionTree_SelectById`); 005CAE7F and 0062C886 are the other callers. The field's
  meaning beyond "a mission id the tree can select" is not established.
- 0058CE30: no reference in the image. With the list at 00E194E4/00E194E8 (count 00E194EC)
  non-empty it builds `FE.main_uacp` (count 1) or `FE.main_uacps`, appends `|.:` and each entry
  after a newline (00CE4390), clears the list and raises 00531B00 kind 2. Only the structure is
  established; "notice" is a placeholder for whatever `uacp` stands for.
- 00583E50: hides +2C8h (then zeroes it), +244h, +248h; when +2C4h is set deletes the headers
  +2B8h/+2BCh/+2C0h and the widgets of the three lists at +280h/+298h/+2A4h through virtual
  +4(1), empties the lists, clears +2C4h, re-places the list box from +558h and resets the +40h
  listener. These are the lists `build_main_menu_objective_rows_00594bf0` fills.
- 00588280: widget-listener slot +1Ch (vtable 00CEFC04, screen+40h; its siblings 005993A0,
  00581970 and 00581B20 are Current04/0C/18). Copies two widget positions into +E4h and +518h,
  clears five 12-byte vectors from +F8h and recaptures two scroller bases (006833F0).

## Functions Ghidra lacks

Each of these is code with a reference but no Ghidra function; defining them is a structural
edit left to the integrator.

| Address | Evidence | Inside Ghidra body of |
|---|---|---|
| 00590D80 | profile-load callback: storage+8 == 0 -> 008D5B50, 00584AE0(1), 00586150 + 005C35D0; else 00590A00(1) | 00590D60 |
| 00590E40, 00590E50 | `00590A00(1)` on *(00E198AC)+58h; 00590E40 is the 007FA710 callback stored at 005910EC | 00590D60 |
| 00588460 | stdcall(2): 00585C50 when the main menu exists, then 004FC150 -> 00BE80B0 -> 00BE7130 | 00588370 |
| 0058EE40 | std::map lookup-or-insert | 0058D9E0 |
| 005861A0 | movie callback, tail jump 005845D0 | (none) |
| 005A8EE0, 005A8EF0, 005B45C0 | map-screen vtable 00CF0134 slots +00h, +08h, +20h | (none) |
| 005C06B0, 005C0800 | minimap vtable 00CF1404 slots +00h, +24h | (none) |

## Uncertainty

- 00A9CF30 and list-box virtual +80h are called with their recorded arguments; their bodies were
  not read (`contract: unread`).
- *0109CECC virtual +14h is modelled as "may create a profile under this name" and +18h as
  "choose the name-conflict message"; both from the message strings, not the callee bodies.
- The `us_flag_frame` frame index 1 is read as the US flag frame from which groups get it.
