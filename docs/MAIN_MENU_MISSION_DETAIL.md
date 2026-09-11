# The main menu's mission-detail page (packet `cc_menu_detail`)

Addresses: 0058C010 0058CAC3 005861B0 00597870 00598B60 00580940 005C2F70 00580650 0054B530 0054A0C0

`0058C010` is the builder for main-menu page 9, the page that shows one mission's
briefing text, its preview movie and the whole group's map markers after the
player has picked a mission out of a mission list. It is the step the rebuilt
executable reaches right after it navigates the main menu to a campaign.

`__thiscall(MainMenuScreen* this)`, no stack argument, aligned frame
(`PUSH EBP / MOV EBP,ESP / AND ESP,0FFFFFFF8h` at `0058C010`), one SEH frame,
1040 instructions, bare `RET` at `0058CE1F`. `this` is the 578h screen of
`include/bsp/main_menu_screen.hpp`; there is no `this` adjustment.

Seven call sites, all argument-free: `00598AD5` (the Enter virtual `005987F0`),
`00599318` (the campaign arm of `00598B60`), `005997C1` and `005998D0`
(`00599340`), `00592722` (`00592640`), `0058F552`, `0059A444` (the update
virtual `00599DB0`).

Ghidra's decompilation of this function is not usable on its own: it carries
`Unable to track spacebase fully for stack`, a jump table it cannot follow, and
nine `Removing unreachable block` warnings. Everything below is read off the
listing.

## What it does, in order

| Range | Effect |
| --- | --- |
| `0058C037`..`0058C047` | `00518D60(ECX = 0Ah, 0, &record.name)` on the selected record from `005806A0`. Same call shape as every arm of `00597870`. Runs before the group is known, so it happens even on the arm that gives up. |
| `0058C04C`..`0058C064` | `background_Icon` (`+32Ch`) virtual `+88h` with `(0, 0, 1.0f)`. The `PUSH ECX / FSTP dword ptr [ESP]` pair at `0058C05C` is the float, not a pointer. |
| `0058C066`..`0058C08D` | Republishes the tree screen's selection: `00E194D8 = [[00E198AC]+5Ch]+10h`, `00E194DC = 005C3850(005C3870(tree))`. |
| `0058C092`..`0058C0A3` | `ADD EAX,-1 / CMP EAX,3 / JA 0058CE0A`, then `JMP [EAX*4 + 58CE20h]`. Only published groups 1..4 continue. |
| `0058C0AA`..`0058C116` | Per group: the group widget handle into `+110h`, `+564h` (US) and `+565h` (DLC), and the point-list index in `EDI`. |
| `0058C11C`..`0058C190` | `+330h = 00AA7E00([+110h], "mission_mappoint_<sel+1>_Icon", 0)`. |
| `0058C235`..`0058C26F` | Latches whether the point vector at `+134h + index*10h` is empty. Element size `0Ch`, from the `/12` magic at `0058C25B`. |
| `0058C274`..`0058C6DB` | The per-mission loop. |
| `0058C6E0`..`0058C7F2` | The visibility block. |
| `0058C7F4`..`0058C808` | `00583E50(this)`, then `historical_Group` (`+344h`) shown. |
| `0058C80A`..`0058C888` | Briefing text and its scroller. |
| `0058C88D`..`0058C8AB` | `video_Movie` (`+33Ch`) source and start. |
| `0058C8B0`..`0058CAB5` | The streamed-dialogue request, guarded by `screen+570h == 0`. |
| `0058CAC3` | `00E08874 = 9`. |
| `0058CABA`..`0058CC1F` | The two footer commands through `0054B530`. |
| `0058CCE8`..`0058CD73` | Hides `+3DCh`, `+1B8h`, `+41Ch`, `+1B8h` again, clears the list box and the help line. |
| `0058CD9E`..`0058CE05` | The list box's two bytes, its position and `00AA6BC0(this+40h, 0)`. |

## Which page is which

`00597870` is the mission-list page builder and the only site that writes pages
4..8. Each arm assigns a title, copies one of the five group widget handles into
`+110h`, and publishes a group index to `00E194D8`. Those three facts agree, and
they settle the naming that `docs/MAIN_MENU_SCREENS.md` got from neighbouring
string literals.

| Page | Title string | Handle into `+110h` | Group at `00E194D8` | `+564h` | `+565h` | Point list |
| --- | --- | --- | --- | --- | --- | --- |
| 4 | `FE.ijn_campaign` (00CEFE5C, `005978FE`) | `+314h` `missions_JP_Group` (`00597961`) | 1 (`00597957`) | 0 | 0 | index 0 |
| 5 | `FE.usn_campaign` (00CEFE4C, `00597987`) | `+310h` `missions_US_Group` (`005979EA`) | 2 (`005979E0`) | 1 | 0 | index 1 |
| 6 | `FE.main_ijn_dlc_title` (00CEFE20, `00597A75`) | `+31Ch` `missions_JP_DLC_Group` (`00597AD9`) | 3 (`00597AD3`) | 0 | 1 | index 3 |
| 7 | `FE.main_usn_dlc_title` (00CEFE08, `00597AFC`) | `+318h` `missions_US_DLC_Group` (`00597B42`) | 4 (`00597B3C`) | 1 | 1 | index 4 |
| 8 | `FE.training_grounds` (00CEFE38, `00597A05`) | `+320h` `training_Group` (`00597A5B`) | 0 (`00597A55`) | -- | -- | index 2 |

The `+564h`/`+565h` stores are at `0059796F`/`00597976` (page 4), `005979F0`
then the shared tail at `00597B51` (page 5), `00597AE7`/`00597AEE` (page 6) and
`00597B4A`/`00597B51` (page 7). Page 8 writes neither.

`0058C010`'s own switch reproduces the same table from the group index instead of
the page (`0058C0AA`..`0058C116`), which is an independent confirmation: group 1
takes `+314h` with both bytes clear, group 2 takes `+310h` with `+564h` set, and
so on.

`00580940`, already reconstructed as
`publish_main_menu_mission_selection_00580940` in `src/mission_briefing_start.cpp`,
closes the loop from the other side. It computes `base - (side_index != 0)` with
`base = 7` for groups 3 and 4 and `base = 5` otherwise (`005809EF` and
`00580A0B`, both the `NEG/SBB/ADD` idiom), and sends group 0 to page 8 at
`00580994`. `side_index` is `005C27E0`, whose whole body is
`return record[0B8h] == 0`; `record+0B8h` is the `allied` side block of
`docs/MISSION_TREE_LUA_READER.md`, so side 0 is the United States and side 1 is
Japan. Side 0 therefore lands on pages 5 and 7 and side 1 on pages 4 and 6,
which is exactly what the title strings say.

So `{4,5}` is not the pair "campaign and its DLC"; it is "Japan and the United
States", and `{6,7}` is the same pair again for downloadable content. The names
are now `CampaignJapan`, `CampaignUs`, `CampaignJapanDlc`, `CampaignUsDlc` and
`TrainingGrounds` in `include/bsp/main_menu_screens.hpp`.

## The widgets the page binds

`005861B0` loads three pages with `00AA5840` and keeps their roots:
`FE_worldmap_historical` at `+2F0h` (`0058629C`), `FE_briefing_grid` at `+244h`
(`0058641A`) and `FE_briefing` at `+248h` (`005864A0`). Every `00AA7E00` lookup
in `005861B0` runs against `+2F0h`. `00AA7E00` finds an existing child one level
deep; it creates nothing (`docs/APP_INIT_FONTS_GUI.md`).

The widgets `0058C010` drives, with the `005861B0` store that binds each:

| Field | Widget | Bound at | What the detail page does |
| --- | --- | --- | --- |
| `+1B8h` | `Main_Listbox` | `00587D3B` | hidden twice, cleared, two bytes at `+149h`/`+14Ah` set, moved to `+558h`..`+560h`, then `00AA6BC0(this+40h, 0)` |
| `+244h` | `FE_briefing_grid` root | `0058641A` | hidden (`0058C7D6`) |
| `+248h` | `FE_briefing` root | `005864A0` | hidden (`0058C7E5`) |
| `+2F0h` | `FE_worldmap_historical` root | `0058629C` | shown (`0058C6E0`) |
| `+310h` | `missions_US_Group` | `0058775E` | shown when `us && !dlc` |
| `+314h` | `missions_JP_Group` | `005877D4` | shown when `!us && !dlc` |
| `+318h` | `missions_US_DLC_Group` | `0058784A` | shown when `us && dlc` |
| `+31Ch` | `missions_JP_DLC_Group` | `005878C0` | shown when `!us && dlc` |
| `+320h` | `training_Group` | `00587936` | always hidden (`0058C7A5` pushes a literal 0) |
| `+324h` | `mission_pic_Group` | `00586E8B` | hidden |
| `+328h` | `bg_01_Icon` | `005879AC` | shown |
| `+32Ch` | `background_Icon` | `00587A48` | virtual `+88h(0, 0, 1.0f)` first, shown later |
| `+330h` | the selected `mission_mappoint_<n>_Icon` | null at `00587AC8` | filled at `0058C190` |
| `+33Ch` | `video_Movie` | `00586963` | source from `backgroundMovie`, then started |
| `+344h` | `historical_Group` | `005868ED` | shown |
| `+354h` | the detail scroller | `00586AF9` | reset and given a range |
| `+3B0h` | `test_Clipbox` | `00586A4F` | its height feeds the scroll range |
| `+3B8h` | `content_main_Text` | `00586B4F` | the briefing text |
| `+114h` | `mission_mappoint_template_Icon` | `00587000` | not touched here |
| `+118h` | `mission_mapflag_template_Icon` | `00587084` | not touched here |

`+2ECh`, `+3DCh` and `+41Ch` are hidden by the page (`0058C7C3`, `0058CCE8`,
`0058CD04`) but no `005861B0` store into any of them was found, so their widget
names are open. `+2ECh` is the only one the page null-checks first.

## The per-mission loop

`00580650` returns the mission-tree group record for the published group; the
loop walks its `std::vector` from `+28h` to `+2Ch` with stride `434h`
(`0058C6C6`) under the secure-SCL bounds checks at `0058C284`, `0058C2A1`,
`0058C2D4`, `0058C2DE`, `0058C2F3` and `0058C6BC`. The ordinal `EBX` starts at 1
(`0058C248`), and the selected mission is compared as `00E194DC + 1`
(`0058C474`, `0058C617`), so the widget names are one-based.

Per mission:

1. `mission_mapflag_<n>_Icon` (`00CEFB90` + ordinal + `00CED1BC`), found with
   `00AA7E00([+110h], name, 1)`.
2. Its virtual `+34h` takes `005C2F70(record)` (`0058C301`, `0058C472`).
3. Its virtual `+4Ch` takes `1.0f` from `00D7A24C` when the mission is the
   selected one and `0.4f` from `00CE7804` otherwise (`0058C474`..`0058C4A9`).
4. `mission_mappoint_<n>_Icon` (`00CEFBA4` + ordinal + `00CED1BC`), same lookup.
5. Its virtual `+34h` takes a literal 1 (`0058C60F`).
6. Its virtual `+88h` takes `(state, 0, 1.0f)` where
   `state = (selected ? 1 : 0) | (record[360h] ? 2 : 0)`. `record+360h` is
   `sideMission`. The selected arm is `LEA EAX,[EAX+EAX*1+1]` at `0058C641` and
   the other is `NEG CL / SBB ECX,ECX / AND ECX,2` at `0058C656`.
7. If the point list was empty on entry, `00AA6750` on the point icon gives its
   resolved position, the backdrop position at `+124h`..`+12Ch` is subtracted,
   and the difference is appended to the vector at `+134h + index*10h` through
   `004215D0` (`0058C662`..`0058C6B3`).

The emptiness flag is latched once before the loop (`SETZ byte ptr [ESP+23h]` at
`0058C26F`), so a list the loop is filling still counts as empty for every later
iteration. That is what makes the fill happen exactly once per group.

## Briefing text, movie and dialogue

`0058C80C` reads `record+30h` (`background`). A null string object sends the
empty literal at `00CE3A0C` through `00ABBE50`; otherwise `00ABAED0` takes the
address of the field. `00683790(this+354h, 0)` resets the scroller, then

```
range = 00AB6BD0(content_main_Text)            ; an x87 double
      - 00AA6740(test_Clipbox).second          ; float
      + 1/90                                   ; the double at 00CEED60
006834A0(this+354h, range)
```

`00CEED60` holds `17 6C C1 16 6C C1 86 3F`, which is `0.011111...`, exactly
`1/90`. The whole expression stays in x87 registers until the single `FSTP` at
`0058C87D`.

`0058C8A0` gives `video_Movie` the record's `backgroundMovie` at `record+40h`
and `0058C8AB` starts it.

The dialogue block runs only when `screen+570h` is zero. The test is written as
`NEG ECX / SBB ECX,ECX / TEST ECX,0E18B5Ch / JNZ` at `0058C8B6`; `ECX` is 0 or
-1 there, so the mask never matters and the condition is a plain
`screen+570h == 0`. The path is three concatenations:

```
"sound/messages/" + 008D57A0(00F88980) + "/streamed_dialogs/" + record[48h]
```

with `008D57A0` supplying the language folder and `record+48h` being
`backgroundVoice`. `00588460` is pushed as the handler to both
`BSP_FileStoreFactory_GetOrCreate` and `BSP_FileStore_RequestFile`.

## The footer

`0058CC1F` calls `0054B530` on the singleton at `00E1930C`. `0054B530` ends with
`RET 3Ch`, so it takes fifteen stack dwords, and the site pushes, left to right:

```
-5Eh, &"globals.continue", 0, 0, &"", 1, 0, &"", 1,
-5Dh, &"globals.back", 2, 0, &"", 1
```

The two negative bytes are command ids and the two non-empty strings are the
labels, so the page offers Continue and Back. How the other eleven values group
around them is not settled here: `0054B530` has 35 callers and deserves its own
packet. `0058CD73` then calls `0054A0C0` on the same singleton with one more
empty string, which clears the help line.

## Corrections

- **`MainMenuPage` 4..7 had both sides reversed.** The enumerators were
  `CampaignUsn = 4`, `CampaignUsnDlc = 5`, `CampaignIjn = 6`,
  `CampaignIjnDlc = 7`, inferred in `docs/MAIN_MENU_SCREENS.md` from literals
  next to the page writes. Page 4 is Japan, page 5 is the United States, page 6
  is Japan's downloadable content and page 7 is the United States'. Evidence:
  the title each arm of `00597870` assigns, the group widget it copies into
  `+110h`, the group index it publishes, and `00580940`'s
  `base - (side_index != 0)` with `side_index == 0` meaning allied. The old
  spellings are gone; `src/main_menu_screens.cpp` was updated with them.
  `include/bsp/main_menu_screen.hpp` had already recorded the same correction
  for `MissionGroup` from `00599DB0`, so the two headers now agree.
- **`Page08` is the training grounds.** `00597A0A` gives page 8 the title
  `FE.training_grounds` and `00597A5F` copies `+320h` `training_Group`. The
  enumerator `TrainingGrounds = 0x08` was added; `Page08` is kept as a second
  spelling of the same value because `src/mission_briefing_start.cpp` and
  `include/bsp/mission_briefing_start.hpp` use it.
- **`is_downloadable_content_page` tested the wrong byte.** It returned pages 5
  and 7 on the evidence that "`00598B60` stores 1 into `screen+55Ch`". `00598B60`
  runs on `this = screen + 8` (`00598FBF` recovers the screen with
  `LEA ESI,[EDI-8]`, and `00598BD4` reaches the list box as `[EDI+1B0h]`), so
  that byte is `screen+564h`, the United States flag, not a content flag. The
  predicate is now `is_us_campaign_page`, and a new
  `is_downloadable_content_page` tests `screen+565h`, which `00597870` sets on
  pages 6 and 7 (`00597AEE`, `00597B4A`).
- **The ledger evidence for `00598B60`** said "pages 4, 5, 6 and 7 form one
  group tested in that order... the DLC arm, `00598B60` sets the flag". The
  first half is right; the flag it sets is the side, not the content. The record
  was amended with `--append-evidence` rather than replaced, so the original
  text stays visible.

## Fidelity notes

- `0058C466` passes the whole dword at `base+4Ch` to the flag icon's virtual
  `+34h`, but only its low byte was written (`MOV byte ptr [ESP+54h],AL` at
  `0058C310`). The upper three bytes are whatever the frame held. The
  reconstruction passes a `bool`; a byte-exact port would have to reproduce the
  dword.
- `0058CCF6` and `0058CD12` hide `+1B8h` twice with identical calls. The
  reconstruction keeps both.
- `0058CDB8`..`0058CDEE` adds `0.0f` to each component of `+558h`..`+560h`
  before storing them. The `FLDZ`/`FADD` pair is in the original; it is a no-op
  for every finite value and is not reproduced as arithmetic.
- The page word is set at `0058CAC3`, before the footer is built, so the page is
  already `MissionDetail` if `0054B530` were to fail.
- `screen+55Ch` is read as a float by `0058CDD9` and written as a byte by
  `00598FB3`. These are not the same field: the second address is
  `this + 8 + 55Ch = screen+564h`. There is no overlap.

## Follow-up packets

- `main_menu_command_bar`: `0054B530` (15 stack dwords, `RET 3Ch`, 35 callers)
  and `0054A0C0`. Settling the argument grouping names the footer for every
  front-end screen at once, including the briefing's `0051B450`.
- `main_menu_detail_hidden_widgets`: `+2ECh`, `+3DCh` and `+41Ch` are hidden by
  `0058C010` but not bound by `005861B0`. Find their binder.
- `main_menu_map_point_geometry`: `004215D0`, the vec3 append into
  `+134h + index*10h`, and `00588C70`, which reads the same lists back.
- `mission_record_side_flag`: `005C2F70`, the predicate that decides whether a
  mission's map flag is shown at all.
- `main_menu_screen_enter`: `005987F0` and `00599340`, the two other routes into
  `0058C010`, and `00583E50`, which every page calls before it shows itself.

## no_ghidra_function

none. Every address in this document already has a Ghidra function. The
undefined region this packet also covered is in `docs/BRIEFING_SECOND_FILLER.md`.

## Correction from docs/MAIN_MENU_COMMAND_BAR.md

The fifteen stack dwords passed to `0054B530` form five source-order triples:
low-byte command id, native-string reference, and placement dword. That routine
rebuilds the command bar. The independent `0054A0C0` setter changes the two help
texts at `+2Ch` and `+30h`; a zero-command rebuild does not clear those texts.
The new packet records the placement lookup and the two display modes.

## Correction from docs/MISSION_MAP_FLAG_POLICY.md

`005C2F70` forwards the mission record's leading key to `0090C560` using the
progress owner at `[game+6B4h]`. Its result is completion status; no side or DLC
condition occurs in the six-instruction adapter. The flag-visibility follow-up
is now reconstructed through that existing progress contract.

`00599340..0059939C` returns page 9 to the mission list: it selects page 7/6
when screen byte `+565h` is set and page 5/4 otherwise, according to whether
`005C27E0` returns zero/nonzero. It never calls the detail builder `0058C010`.
The earlier caller attribution was too broad: calls `005997C1` and `005998D0`
belong to the separately bounded GUI event handler `005993A0..00599D57`.
Both call instructions and those function boundaries were checked live.

## Correction from docs/MAIN_MENU_MAP_POINT_GEOMETRY.md

`00588C70` now has a bounded map-point/flag positioning and zoom reconstruction.
`004215D0` remains a vec3-vector insertion contract; STL storage internals are
not independently ported. See that packet for register inputs, carried zoom,
and the limits of the earlier clamp-only helper.
