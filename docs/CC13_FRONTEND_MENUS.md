# Frontend menus: list pages, new-profile entry, mission map icons (packet `cc13_frontend_menus`)

Addresses: 005805A0, 00580530, 00584F50, 005853C0, 005886F0, 00588370, 00590E60, 005807D0,
00580780, 00585EB0, 0058D9E0, 005845D0, 00585C50, 00586150, 0058CE30, 00583E50, 00588280,
0059CB90, 005A4200, 005A8600, 005A5920, 005BFDF0, 005C0E10, 005A2340, 005A2730, 005A2C30,
005A91E0, 005C4080.
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

## Map and minimap screen handlers (named only)

The in-game tactical map (`GUI_map`) is a front-end screen with vtable 00CF0134; the minimap's is
00CF1404. Slot layout from the base 00CEAE54: +00h screen id, +0Ch deleting destructor, +10h
register, +14h layout, +18h enter, +1Ch exit, +20h update, +24h child collection.

| Vtable slot | GUI_map (00CF0134) | Minimap (00CF1404) |
|---|---|---|
| +00h | 005A8EE0 (no function) | 005C06B0 (no function) |
| +08h | 005A8EF0 (no function) | 004F7580 base |
| +0Ch | 005AB140 deleting destructor -> 005A8600 `BSP_HudMapScreen_Destruct` | 005C0910 |
| +10h | 005AB190 `BSP_HudMapScreen_Register` (cc12 audit, untouched) | 005BEC50 |
| +14h | 004F7590 base | 005BE240 |
| +18h | 005B3470 enter (cc12 audit, untouched) | 005BD550 |
| +1Ch | 005A4200 `BSP_HudMapScreen_Exit` | 005C0E10 `BSP_HudMinimapScreen_Exit` |
| +20h | 005B45C0 (no function) | 005C0F20 |
| +24h | 005A8F20 (no function) | 005C0800 (no function) |

Both screens own an icon map keyed by the observed object and an observer subobject whose slot 0
(005A5920 for the map, vtable 00CF011C, map at +490h; 005BFDF0 for the minimap, vtable 00CF13F0,
map at +F0h) deletes the released object's icon and erases its node. Slots 2..4 of both observer
vtables are the shared 0042B120 / 0042B130 / 0042B140.

005A4200 and 005A8600 both walk the scene for the node named `Terrain Visibility Group` and
restore visibility 1.0 on its `BushGroup` and `TreeGroup` children, skipping models whose effect
name contains `wavedamp`; the exit does it only when +56Ch is set. The hiding counterpart is in the
enter 005B3470, which this packet did not read (cc12 audit).

The map's generated models: 005A2340 (`StoredQuad`, `map_cmd_building.mshd`, `pf43.mvfm`),
005A2730 (`ACone`, `SimpleColor.mvfm`), 005A2C30 (`SimpleColor.mvfm`), and 005A91E0, which
registers `cCapRadSquared`, `cCapRadOutSquared`, `cLandRadSquared` and `cLandRadOutSquared` on a
005A2340 model. Names are provisional and come from those strings.

005C4080 (`BSP_MissionTree_IncompleteMissionSideMask`, unreferenced) folds a four-bit mask over
every mission that 0090C560 reports incomplete: ids starting `USN` (00CF1468, `_stricmp` on the
first three bytes) clear bit 0 or 1, other ids bit 2 or 3, chosen by whether record +68h is zero.
The 005C6A70 schema puts the third `date` component at +68h, and no shipped mission has a zero
day, so what the bit pair distinguishes is open.

## The HUD unit icon index (0059CB90)

`BSP_Unit_GetHudIconIndex`, `__fastcall`(ECX unit or null, EDX class descriptor or null), returns
the icon index the marker and ship screens use (callers 0063D1E0, 0064AE20, 0063C620, 005AAC00,
005ADDA0). Reconstructed as `hud_unit_icon_index_0059cb90`, complete, build-tested. Class ids are
the IsKindOf ids of docs/RECON_SLOT_LISTS.md; a unit is asked through its virtual +5Ch, a
descriptor through its virtual +18h.

| Test (descriptor unless noted) | Index |
|---|---|
| no unit and no descriptor | 0 |
| ship 06h: battleship 0Dh | 17h if +808h else 2 |
| cruiser 0Ah | 3 if +808h (HeavyCruiser) else 4 |
| mothership 09h | 6 if +82Ch else 5 |
| destroyer 07h | 8 |
| landing ship 0Ch | 1Ah if +818h, else 1Bh if +81Ah, else 0Ah |
| cargo 0Bh | 16h if +809h else 15h |
| torpedo boat 0Eh / submarine 08h / other ship | 0Dh / 10h / 0 |
| plane 0Fh, descriptor from the unit | kamikaze 17h with unit+C24h clear: 0Bh; else by 007BCA50: bomb 9 or 0Dh -> 1, torpedo 0Ah -> 11h, depth charge 0Bh -> 7, 0Fh -> 0Ch, rocket 12h -> 0Eh, otherwise 9 |
| plane 0Fh, descriptor supplied | bomber 10h or dive bomber 12h -> 1, torpedo bomber 11h -> 11h, kamikaze 17h -> 0Bh, else 9 |
| dummy target 35h | 0Ch |
| (unit, no descriptor supplied) shipyard 46h / airfield 45h / convoy 1Ah | 0Fh / 14h / 1Eh |
| land fort 1Bh with a member answering 20h / any other non-building | 18h / 19h |
| command building 1Ch, class bytes +1A5h / +1A4h / +1A8h / +1A7h | 1Ch / 1Dh / 21h / 20h, else 1Fh |

When only the unit is given, the descriptor is unit+538h, or the leader's (unit+3D0h)+538h for a
squadron (18h). The kamikaze test and 007BCA50 also run on the leader. Descriptor bytes past +800h
are per-subclass: other packets name +808h HeavyCruiser on cruisers (avoid_zone_draft_bodies.hpp)
and +808h/+809h BigLandingShip/Rocketer on landing ships, which does not name them on the
battleship or cargo tested here. +818h, +81Ah, +82Ch and the command-building bytes are unnamed.

## The tactical map's nearest-icon pick (005A1310)

`BSP_HudMapScreen_PickNearestUnit`, `__thiscall`(GUI_map screen, bool skip_own_group), RET 4.
Reconstructed as `pick_nearest_map_unit_005a1310`, complete, build-tested. Callers 00526A40
(`BSP_HudUnitPickScreen_PickUnit`) and 005A3210.

1. +520h = 0. The cursor (+554h, +558h) goes to map screen units as
   `(x - +54Ch) / +30h + 0.5` and `(+550h - z) / +34h + 0.5` (00E08890/00E08894 are 0.5).
2. With the flag, the controlled unit 00E188D8 is captured; otherwise zero (NEG/SBB/AND at
   005A1343..005A1366).
3. Every node of the icon map at +4F4h/+4F8h whose icon (node+10h) exists, is active (+8h),
   whose entity (+4h) answers IsKindOf(2), whose virtual +140h owner is not the captured unit,
   and which is neither MTorpedo (2Bh) nor MWaterMine (34h), is projected through two virtual
   +C4h calls (x from the first, z from the second). The squared screen distance is kept when it
   is below the best so far, which starts at 0.0009 (00CF0008); ties keep the earlier node.
4. +520h gets the winner's entity (if IsKindOf(2)), replaced by its +140h owner when that is a
   different IsKindOf(2) entity and the winner is not an MLandVehicle (19h).
5. Returns the winner's entity when it is a vehicle (5), the squadron's +3D0h leader when it is a
   PlaneSquadronGen (18h), else null.

The arithmetic is x87 with a float store after every step; `dx*dx` is stored before it is added
to the stored `dy*dy` (005A14B7..005A14C9). The reconstruction replays it in double, narrowing at
the same points, which equals the native result if the process runs at the MSVC CRT's default
53-bit precision control. That precision setting is an assumption, not read from this binary.

## Minimap icon entries and the nearby capture point

This takes the `hud_minimap_icon_entries` follow-up of docs/HUD_MINIMAP.md for its three
routines in segment 20.

**The templates are five, not six.** 005BE240 binds `item_ship_Icon` (+6Ch), `item_plane_Icon`
(+70h), `item_sub_Icon` (+74h), `item_building_Icon` (+78h) and `item_marker_Icon` (+7Ch), all
under `minimap_units_white_Group`. From each colour group it reads only `item_ship_Icon`'s
16-byte colour: white +D0h (005BE58A), red +80h (005BE6A2), blue +90h (005BE7BA), grey +C0h
(005BE8D2), yellow +A0h (005BE9EA), silver +B0h (005BEB02). docs/HUD_MINIMAP.md's row "+6Ch..+7Ch
the six groups' `item_ship_Icon`" should read as above.

**005BD590** (`BSP_HudMinimapIconEntry_Construct`, reconstructed, complete): the 0Ch entry is
{vtable 00CF1158, unit, icon}. The icon is cloned (00AAB4C0, parent 0) from the sub template for
IsKindOf(8), the ship template for 6, the plane template for 0Fh and the building template
otherwise. Its colour is set through virtual +50h from blue (side 0), red (side 1) or grey
(side 2) by unit+54h; any other side keeps the template colour. It is then attached under
`unit_marker_Group` (+F0h, 00AAA5A0 with (icon, 0)) and shown (virtual +34h(1)). The entry's only
virtual, 005BDB10, deletes the icon through its virtual +4(1). The marker template (+7Ch) is not
used here.

**005C0700** (`STL_HudMinimapIconMap_Subscript_005c0700`): `std::map<unit*, entry*>::operator[]`
(lower_bound on node+0Ch, 005C01D0 inserts a null value, returns node+10h). Named, not
reconstructed.

**005BD420** (`BSP_HudMinimapScreen_SelectNearbyCapturePoint`, coverage partial: the loop and
compare; the +F4h clear and store, the preconditions and the pose refreshes are host-side):
clears +F4h; when the controlled unit 00E188D8 answers IsKindOf(6) and the capture-point list at
*(*(00E188A8)+19CCh)+16Ch is non-empty, walks it (point at node+8h, next at node+4h), refreshing
the point's pose and then the unit's (00414DB0 when +C8h is clear), and stores the first point
whose squared distance is not above `float(+7C4h)^2` and whose side (+54h) is 2. The x87 sum is
`(dx*dx + dy*dy) + dz*dz` stored to float, compared with `float(r)*float(r)` stored to float, by
`FCOMIP`/`JA`, so an unordered result counts as inside.

## Mission-tree helpers

**005C3DE0** (`BSP_MissionTree_AdvanceToNextMission`, reconstructed as
`advance_campaign_selection_005c3de0`, complete, build-tested): the scoring screen's enter
0062C2C0 calls it at 0062C866 after a mission. It resolves the current mission key (game+2198h)
through 005C3470 and stores the pair at +0Ch/+10h; `groups.at(group)`; when the mission index is
below `count - 1` (unsigned, 005C3E5E `JNC`) and that mission is completed (0090C560) it steps
by one; it then clamps the index to `[0, found + 1]` with signed compares (005C3EB0 `JGE`,
005C3EB8 `JLE`), copies that record's id (+00h) to *(00E188A8)+6B8h and returns
`found < index` (005C3F2C `SETG`). +6B8h is profile (+650h) + 68h, the string 00586150 copies
out, so 00586150 reads the profile's selected campaign mission (evidence appended to its
ledger row).

**005C4BE0 / 005C4D40 / 005C51F0**: the 154h side block's element constructor, destructor and
copy constructor. 005C89B0 pushes (154h, 2, 005C4BE0, 005C4D40) at 005C8A52..005C8A5E for the
record's two blocks at +B8h/+20Ch; 005C81F0 references 005C51F0 (005C85F1) and 005C4D40.
Named only.

## Handoff

State on agent/cc13-frontend1: 36 segment-20 functions named in the ledger and Ghidra (raw untagged
`FUN_` in 005805A0..005C5600 went from 557 to 522), 15 reconstructed in
`src/frontend_menus.cpp` (005BD420 partial), build-tested with `scripts/build.ps1`.
`reports/cc13_frontend_menus.json`: 71 call rows, 0 failed. Nothing was run in `bsp_game`.

What the remaining raw functions are, by band:

- 00580E40..00583D80, 0058B5D0..0058F480, 00590A90..00594860: mostly STL instantiations (tree
  iterators, vector reserve/insert, `_Xlen` throwers) and EH helpers for the mission score record
  and objective-row containers. Tagging them as STL is the cheap next step; none carries a string.
- 0059C000..005A3BF0: helpers of the GUI_map update path (005ADDA0, 005B2D70, 005B45C0), which
  cc12's GlobalConfig audits touch. Read 005B45C0 (undefined update slot) before these.
- 005BD050..005C0BD0: minimap icon-map STL and capture-icon helpers called from 005C0F20.
- 005C2C50..005C5470: mission-tree STL (map/tree iterators, 434h allocators) and the side-block
  lifetime named above.

Open questions:

1. The undefined functions listed under "Functions Ghidra lacks" (00590D80, 00590E40, 00590E50,
   00588460, 0058EE40, 005861A0, 005A8EE0, 005A8EF0, 005B45C0, 005C06B0, 005C0800) need defining
   before they can be named.
2. `FE.main_uacp` (0058CE30): which list producer fills 00E194E4 (00581080 has six callers) and
   whether anything reaches the unreferenced prompt.
3. What record +68h means to 005C4080 (the third `date` component per the schema).
4. Whether the odd screen vectors +80h..+100h of the main menu hold the flag icons (0058D9E0
   does not write them).

# Second packet (2026-10-11): segments 17 and 23

Leases: 00554680..00568930 and 005E09A0..00604A50 (`cc13_frontend_menus2`).

Skipped: 005EF4D0..005F7310 (the options screens of docs/OPTIONS_MENU_SCREENS.md, edited
2026-10-10, and cc12's options audio/sound-copy reviews of the same week); the functions that
docs/OPTIONS_MENU_SCREENS.md already names for the PC controls screen (00555770, 00555800,
00555C40, 00555C60, 00555DC0, 00558680, 0055FA80, 00560430, 00551B90, 0055D180); 005EC990,
005EAC60 and 005E6F00, which docs/SHIP_AI_OPEN_ITEMS.md (cc9) analyses; 005F9B20..005F9BA2
(docs/SCRIPTED_HELM.md).

## PC controls screen (screen 0Fh): the options_pc_controls_screen follow-up, partly

The controls screen (constructor 00555770, size 250h) has three vtables: primary 00CEE524, a
listbox listener at +08h (00CEE510: 00555810, 0055FCA0 in the activate slot, 0055D180, the
shared no-op) and a widget listener at +0Ch (00CEE4F0: 0055D330 at +00h, 00560BF0 at +04h, which
has no Ghidra function, 005539E0 at +0Ch, 00551BC0 at +18h).

| Address | Name | Coverage |
|---|---|---|
| 0055CCC0 | `BSP_ControlBindingScreen_AdjustSetting` | reconstructed, complete, build-tested |
| 0055D330 | `BSP_ControlBindingScreen_OnSliderChanged` | reconstructed, complete, build-tested |
| 0055C200 | `BSP_ControlBindingScreen_ShowConflictText` (provisional) | named |
| 0055C5A0 | `BSP_ControlBindingScreen_UpdateBindingRowText` (provisional) | named |
| 0055CEC0 | `BSP_ControlBindingScreen_ScrollToSelection` (provisional) | named |
| 0055D380 | `BSP_ControlBindingScreen_BuildSettingsList` (provisional) | named |
| 0055E3A0 | `BSP_ControlBindingScreen_RefreshPage` (provisional) | named |
| 0055F710 | `BSP_ControlBindingScreen_LoadSelectedPreset` (provisional) | named |
| 0055FCA0 | `BSP_ControlBindingScreen_ListboxListener04` | named |
| 00560210 | `BSP_ControlBindingScreen_ConfirmOrNextDevice` (provisional) | named |

0055CCC0, `__thiscall`(screen, int delta), RET 4: the setting is the one named at screen+184h
in the device entry screen+16Ch indexes (input settings 005547D0, +18h vector, 8-byte stride;
per-device record through 0055C110). Its kind comes from 0055A9A0 and its float value from
00444BE0.

- Kinds 3 and 4 (the inversion toggles): `value = -0.0f - value` (SSE, 0055CD58), written
  through 006A9E70, and the row's `var_Text` gets `FE.opt_normal` when the result is above 0.0f,
  else `FE.opt_inverted`.
- Other kinds: `value = float(delta * 0.25 + value)` (x87 FILD/FMUL double/FADD/FSTP), clamped
  to [0.25 (00CE3868), 2.75 (00CEE668)], written, re-read and turned into the slider position
  `-1 - trunc(value * -4.0)` (00CEE4E0, `_ftol`), so 0..10, passed in EDX to 005F3750.
- Both paths set +19Ch, the unsaved-changes byte 00560210 tests before raising
  `FE.opt_acceptchanges` (cancel / discard / accept).

0055D330 forwards a slider click: outside page 3 (+168h), a widget whose +D8h id is 1..FFFFh and
whose low byte equals the selected list index (00A9C920 on screen+20h) passes 005F2290's
non-zero delta to 0055CCC0. 005F2290 and 005F3750 are in the options range and were not named.

## Screen vtables named in segments 23 and 17

Slot layout as in the base 00CEAE54 (+10h register, +14h layout, +18h enter, +1Ch exit, +20h
update, +24h child collection); widget-listener slots named Current<slot> after the main menu's
00CEFC04 convention; listbox-listener slot +04h is the activate slot (00598B60 on the main menu)
and +08h the selection-changed slot (005966F0).

| Screen (primary vtable) | Named here |
|---|---|
| multiplayer game lobby (00CF23E4) | enter 005E51D0, update 005E5B30, collect 005E10F0; listbox +08h 005E19E0 `ShowSelectionHint` (mode/map hint text, weather and victory-point descriptions) |
| multiplayer main menu (00CF2708) | layout 005E7B10, enter 005E8B20, exit 005E6B60; widget listener 00CF26D4 +0Ch 005E6E20, +18h 005E6B80, +1Ch 005EA700 |
| multiplayer mode selector (00CF2B44) | layout 005EADC0, update 005ECD80, collect 005ECB70 (enter 005EC5B0 and exit 005EA960 have no Ghidra function); widget listener 00CF2B10 +0Ch 005EA990, +18h 005EAAC0 |
| HUD objectives (00CF2D20) | enter 005EEA00, update 005EEF60 (005ED5C0 and 005EE510 have no Ghidra function) |
| HUD orders (00CF3B7C) | layout 005FA0F0, enter 005FAD70, update 005F9DD0 (exit 005F9480 has no Ghidra function) |
| HUD pause (00CF3EF4) | enter 00603120, update 00603CA0, collect 005FFAC0 (exit 005FD880 has none); listbox 00CF3EE0 +04h 00604190, +08h 006003A0 |
| leaderboards (00CEE9CC) | register 00564220, enter 00562750, update 00565540 (exit 00560FE0, collect 00563430 have none); widget listener 00CEE998 +04h 00565190, +0Ch 00561320, +18h 00560FF0, +1Ch 00563400; helpers 005638F0 (Leaderboards.lua reader), 00561EB0 (rank rows) |
| HUD limbo (00CEEBAC) | enter 00566080 (exit/update/collect 00565D70/00565E00/00565FA0 have none) |

These names rest on the vtable slot and the strings each body references; none of the bodies
was reconstructed. The multiplayer enters and updates are the `multi_menu_screen_enter` and
`multi_menu_screen_update` follow-ups of docs/MULTI_MENU_SCREENS.md and remain open.

docs/HUD_MINIMAP.md got a dated correction for its +6Ch..+7Ch row.

005FD890 `BSP_HudPauseScreen_ShowPage` (provisional, named only): `__thiscall`(pause screen, bool
second_page, bool multiplayer), called by the pause update 00603CA0 and by 005D3F20 / 005D43C0.
The second page hides the list and shows +18h; the first clears the rows and the +68h row-widget
list and sets the frame title from `ingame.paused` / `ingame.pause_multi_title` (which string
each branch uses was not read). The listing has a list-clear `_free` that Ghidra turns into an
early return, so the decompiled body is incomplete.

## Handoff (second packet)

Commits after the merge of main: the segment 17/23 batch and this one. Reconstructed in this
packet: 0055CCC0, 0055D330 (17 functions reconstructed overall in `src/frontend_menus.cpp`, two of
them partial). Named in this packet: 46. Open: the multiplayer enter/update bodies (005E51D0,
005E5B30, 005E8B20, 005ECD80), the controls update 00560430 (Xbox keyboard preset naming,
`Presets/%S.preset` save, `ControlPresetsOrder`), the pause screen bodies, and the Ghidra-less
vtable slots listed in the table above.

# Third packet (cc13-frontend2, 2026-10-11): Ghidra-less vtable slots defined and named

Leases `cc13_frontend2_seg20` (00584000..005C5600), `cc13_frontend2_seg1723` (00554680..00568930,
005E09A0..00604A50), `cc13_frontend2_seg2627` (0061F000..00681000).

## Defining the slot functions

Every handoff address in "Functions Ghidra lacks" and in the in-game GUI doc's slot list, plus
every other code pointer in the 00CExxxx..00CFxxxx vtables that lands on a 16-byte aligned
address inside these leases with no Ghidra function (`local/f2_scan.py`: an `.rdata` dword scan
filtered by vtable bases taken from constructor `MOV [reg+N],imm32` stores), was defined with
`tools/ghidra_define_function.py` (locked, bytes compared to the disk image, no flow re-discovery).
Extents come from a recursive descent over the disk image (`local/f2_extent.py`, follows the
five-entry switch of 005B45C0 at 005B5B64); two extents with an alignment hole (0055D180, 00646170)
were checked by hand. 178 functions were defined; the record of every bridge call is
`reports/cc13_frontend2_defined.json`. 166 of them were named in the ledger and in Ghidra.

The bulk follows the base screen vtable 00CEAE54 slot layout:

| Kind | Count | Evidence |
|---|---|---|
| `<Screen>_GetScreenId` (+00h) | 20 | `MOV EAX,id; RET`: map 4Ch, minimap 35h, limbo 32h, objectives 39h, orders 3Bh, pause 3Ch, pause-cheat 3Dh, scene-init 5Ah, markers 4Dh, spectator 43h, HUD root 44h, ship status 45h, submarine 48h, periscope 47h, support manager 4Eh, tactical sheet 54h, cursor 28h, follow 49h, unit library 5, narrative 33h |
| `<Screen>_IsSelfManaged` (+04h) | 6 | `MOV AL,1`: pause, pause-cheat, scene-init, spectator, narrative; tactical sheet returns `*(00E188A8)+1FE4h == 0` |
| `<Screen>_OccludesLowerLevels` (+08h) | 2 | `MOV AL,1`: map, scene-init |
| Enter/Exit/Update/BindLayout | 32 | slot position; empty bodies noted per row |
| `<Screen>_CollectChildren` (+24h) | 16 | `ADD ECX,N; PUSH ECX; MOV ECX,[ESP+8]; CALL 004D6790; RET 4` |
| `CG_scalar_deleting_dtor_<target>_Adjustor<N>` | 25 | `SUB ECX,N; JMP dtor` at +00h of listener sub-objects |
| `BSP_SessionMessage<id>_{ScalarDelete,Write,Read,IsType}` | 27 | five-slot vtables 00CF5C38, 00CF5C4C, 00CF61FC, 00CF64C4, 00CF64D8, 00CF64EC, 00CF6500, 00CF3BB8 with ids 160, 148, 161, 130, 167, 168, 169, 188; type tests walk the chain id -> 49h -> 46h |
| listener slots and callbacks | 38 | listed below |

This answers the FRONTEND_SCREEN_SETS.md gap "which leaf types override +4h/+8h" for the screens in
these leases: the overriders are the six +04h and two +08h rows above.

Named listener slots and callbacks with behaviour (all provisional except pure no-ops):
00590D80 `BSP_MainMenu_OnProfileLoaded`, 00590E40/00590E50 `BSP_MainMenu_OnAsyncTextRequestDone(2)`,
00588460 `BSP_MainMenu_RemoveFileStoreFile`, 0058EE40 `BSP_MainMenu_StringMapFindOrInsert`,
005861A0 `BSP_MainMenu_OnMovieAdvance`, 00560BF0 `BSP_ControlBindingScreen_HandleGuiEvent`,
0055D180 `BSP_ControlBindingScreen_OnListboxSelectionChanged`, 00561E50
`BSP_LeaderboardsScreen_OnListboxSelectionChanged`, 005D9D90 `BSP_MultiGameLobbyScreen_OnListboxActivate`,
005E7160/005E73A0 `BSP_MultiMainMenuScreen_OnListboxActivate`/`_OnListboxSelectionChanged`, 005E97D0
`BSP_MultiMainMenuScreen_HandleGuiEvent`, 005EA2D0 `BSP_MultiMainMenuScreen_Update`, 005EBC90
`BSP_MultiModeSelectorScreen_OnListboxSelectionChanged`, 005ED450 `BSP_MultiModeSelectorScreen_HandleGuiEvent`,
005FDB50 `BSP_HudPauseScreen_HandleGuiEvent`, 00645BB0 `BSP_InGameHudRootListener_Current0C`,
0067B0E0 `BSP_HudTacticalSheetScreen_OnListboxActivate`, 0067BC60/0067C130 overlay observers
`_OnReleased`, and the no-op slots (`...NoOp`). Evidence strings are in the ledger rows.

Defined but deliberately not named: 005EF5F0, 005F0440, 005F1CB0, 005F4C10 (options screens,
docs/OPTIONS_MENU_SCREENS.md and cc12's options reviews), 005662D0 (vtable base unclear),
0064B6B0, 0064B6D0, 00650870 (slots +12Ch/+130h/+128h of unit-sized vtables 00CF5CE8/00CF6250),
006502D0, 0067B3A0, 0067BAA0 (observer and enter slots of classes whose register functions carry
generic names), 00646170 (register-argument copy helper, `EAX = EDX` count).

Not covered: the same scan outside these leases (00568930..00584000, 005C5600..005E09A0,
00604A50..0061F000 and 0068xxxx, e.g. the overlay vtables' 0068A320/0068A400/0068A5D0 slots).
