# The three screens the options manager owns

Addresses: 005F6030, 005F1CA0, 005F3B30, 005F0250, 005F1CC0, 005F5BD0, 005F7020, 005F7310,
005F6000, 00527D00, 00527D30, 00527E40, 00527E60, 00527F90, 00528240, 00528920, 005289B0,
00528A60, 00555770, 00555800, 00555C40, 00555C60, 00555DC0, 0055FA80, 00551B90, 00560430,
00558680. Read-only context: 006898C0, 00689820, 004F7180, 004F71D0, 004CC460, 004D92B0,
005F42D0, 005F7C40, 005F8960, 005F58F0, 005F59F0, 005F41E0, 005F40F0, 005F3B50, 005F5E60,
005F0A80, 005F65C0, 005F54F0, 005F24D0, 005F0800, 00527C80, 008D4520, 008D41C0, 008D41F0,
008D4820, 008D5430, 008D5B50.

`docs/FRONTEND_MANAGERS.md` establishes that Init 006898C0 opens the `GVOptions` block and
constructs three objects into 00E198B8+40h, +44h and +48h, calling each one's virtual +10h
immediately afterwards. This document names those three classes, their fields, their virtuals and
the settings model the first of them drives.

## The three classes

All three chain `BSP_FrontEndScreen_Construct` (004F7180), which stores base vtable 00CEAE54 and
clears the flag bytes at +4h and +5h. Each then installs two secondary vtables at +08h and +0Ch
(base values 00CEB0FC and 00CEB110, overwritten in place by the leaf) and finally its own primary
vtable at +00h. Slot +00h of every primary vtable is a `mov eax, imm32; ret` leaf; decoding it
gives the screen id, which is also the registry index 004F71D0 uses.

| Manager field | Constructor | Size | Primary vtable | +08h vtable | +0Ch vtable | Screen id | GUI pages |
| --- | --- | --- | --- | --- | --- | --- | --- |
| +40h | 005F6030 | 270h | 00CF3974 | 00CF3960 | 00CF3940 | 0Dh (005F1CA0) | `_Options`, `FE_options` |
| +44h | 00527D00 | 30h | 00CECF48 | 00CECF34 | 00CECF14 | 0Eh (00527D30) | `FE_controls_X360_listbox`, `FE_controls_X360` |
| +48h | 00555770 | 250h | 00CEE524 | 00CEE510 | 00CEE4F0 | 0Fh (00555800) | `FE_controls_PC_listbox`, `FE_controls_PC` |

The screen ids line up with the manager's `ApplyPendingInterface` override 00689820, which maps
interface ids 0Ch..0Eh onto screen ids 0Dh..0Fh. Interface 0Ch is committed by Init itself at
006899DA; 0Dh and 0Eh are pushed through 004CC460 from inside screen 0Dh (see *Leaving the
screen*). 00528240 and 005289E0 push 0Ch back, so the X360 layout screen returns to the options
screen.

The vtable extents are read off the string data that follows them: 00CECF70 holds
`FE_controls_X360` and 00CEE54C holds `FE_controls_PC`, both immediately after the tenth slot, so
all three primary tables are ten slots like the 00CEAE54 base.

### Vtable slots

Base 00CEAE54 supplies the defaults 004F7590 (`ret`), 004F75A0 (`ret`), 004F75B0 (`ret`),
004F75C0 (`ret 4`) and 004F75D0 (`ret 4`) for slots +14h..+24h, which fixes the signatures of the
overrides below.

| Slot | Role | Screen 0Dh | Screen 0Eh | Screen 0Fh |
| --- | --- | --- | --- | --- |
| +00h | id leaf | 005F1CA0 | 00527D30 | 00555800 |
| +04h | base, not overridden | 004F7570 | 004F7570 | 004F7570 |
| +08h | base, not overridden | 004F7580 | 004F7580 | 004F7580 |
| +0Ch | scalar deleting destructor | 005F3B30 | 00527E40 | 00555C40 |
| +10h | register | 005F0250 | 00527E60 | 00555C60 |
| +14h | bind widgets | 005F1CC0 | 00527F90 | 00555DC0 |
| +18h | enter | 005F5BD0 | 00528240 | 0055FA80 |
| +1Ch | exit | 005F7020 | 00528920 | 00551B90 |
| +20h | update(float) | 005F7310 | 005289B0 | 00560430 |
| +24h | dependency list | 005F6000 | 00528A60 | 00558680 |

Each +08h table starts with a `sub ecx, 8; jmp <destructor>` adjustor thunk (005F1CB0, 00527D40,
00555810), which proves the second base sits at +08h; the +0Ch table is the GUI listener whose
slot +04h receives list events.

## Calling conventions and RET sizes

| Address | Convention | RET |
| --- | --- | --- |
| 005F6030, 00527D00, 00555770 | `__thiscall(this)`, returns this in EAX | RET |
| 005F1CA0, 00527D30, 00555800 | `__thiscall(this)` -> int in EAX | RET |
| 005F3B30, 00527E40, 00555C40 | `__thiscall(this, unsigned flags)` | RET 4 |
| 005F0250, 00527E60, 00555C60 | `__thiscall(this)` | RET |
| 005F1CC0, 00527F90, 00555DC0 | `__thiscall(this)` | RET |
| 005F5BD0, 00528240, 0055FA80 | `__thiscall(this)` | RET |
| 005F7020, 00528920, 00551B90 | `__thiscall(this)` | RET |
| 005F7310, 005289B0, 00560430 | `__thiscall(this, float seconds)` | RET 4 |
| 005F6000, 00528A60, 00558680 | `__thiscall(this, void* sink)` | RET 4 |
| 005F42D0 | `__thiscall(this, int step)` | RET 4 |
| 005F7C40 | `__thiscall(this = screen+8, ...)` | see below |
| 005F8960 | `__thiscall(this = screen+0Ch, GuiEvent* event)` | RET 4 |
| 00527C80 | `__thiscall(this, char, char, char, char)` | RET 10h |

005F7020 is 22h bytes and ends in `jmp 005F6EF0`, a tail call, so it has no RET of its own; the
`ret 4` at 005F7300 belongs to the neighbouring FUN_005F7050, not to the exit virtual. 00551B90 is
a single `ret` byte: the PC controls screen has an empty exit.

005F7C40 and 005F8960 are reached through the +08h and +0Ch secondary vtables, so Ghidra's `this`
for them is the object plus 8 and plus 0Ch. Every field offset quoted from those two bodies below
has already been shifted back to the primary view. The shift is confirmed three ways: 005F7C40's
`param_1 + 258h` is the transition timer the update virtual reads at +260h, 005F8960's
`param_1 + 7Ch` is the widget the update virtual reads at +88h, and 005F8960's `param_1 + 244h`
and `+245h` are the two flag bytes 005F42D0 and the update virtual use at +250h and +251h.

## Screen 0Dh, the options screen (005F6030, 270h)

### Field layout

Every offset below is one a constructor or a virtual demonstrably touches. Nothing else in the
270h bytes is recovered, and none of this is binary compatible.

| Offset | Type | Meaning | Evidence |
| --- | --- | --- | --- |
| +00h | void* | primary vtable 00CF3974 | 005F6030 |
| +04h, +05h | bool | base "wanted" and "active" flags | 004F7180 |
| +08h, +0Ch | void* | secondary vtables 00CF3960, 00CF3940 | 005F6030 |
| +10h | XOVERLAPPED | write handle; +10h is the status dword | 005F7310 clears +10h..+28h |
| +2Ch | XOVERLAPPED | read handle; +2Ch is the status dword | 005F7310 clears +2Ch..+44h |
| +70h | size_t | storage buffer size, initialised to 10 | 005F6030, memset length in 005F7310 |
| +7Ch | void* | GUI page `_Options` | 005F0250 |
| +80h | void* | GUI page `FE_options` | 005F0250 |
| +84h | void* | `Main_Listbox` widget | 005F1CC0 |
| +88h | void* | widget whose byte +85h gates the left/right adjust | 005F7310 |
| +8Ch | void* | widget the reset prompt writes into | 005F7310 at 005F7A4E |
| +A0h, +A4h | void* | sound channels; virtual +34h replays effects and speech | 005F42D0 |
| +A8h | int | selected main-list row | 005F7C40 stores it, 005F0A80 clears it |
| +C4h, +CCh, +D0h, +D4h | dword | cleared by the constructor; +D0h later takes the pending language | 005F6030, 005F8960 |
| +DCh | int | language index | 005F42D0 page 1 row 0 |
| +E0h | bool | imperial/metric units | 005F42D0 page 1 row 1 |
| +E1h | bool | subtitles | 005F42D0 page 1 row 2 |
| +E4h | int 0..2 | hints level | 005F42D0 page 1 row 3 |
| +E8h | bool | camera shake | 005F42D0 page 1 row 4 |
| +ECh, +F0h | int | screen width, height | 005F42D0 page 3 row 0 |
| +F6h | bool | fullscreen | 005F42D0 page 3 row 1 |
| +F8h | float | master volume | 005F42D0 page 2 row 0 |
| +100h | float | music volume | 005F42D0 page 2 row 1 |
| +104h | float | effects volume | 005F42D0 page 2 row 3 |
| +108h | float | speech volume | 005F42D0 page 2 row 2 |
| +118h | bool | vibration | 005F42D0 page 4 row 1 |
| +119h | bool | swap sticks | 005F42D0 page 4 row 4 |
| +11Ah | bool | invert camera Y | 005F42D0 page 4 row 2 |
| +11Bh | bool | invert flight Y | 005F42D0 page 4 row 3 |
| +11Ch | bool | swap map sticks | 005F42D0 page 4 row 5 |
| +122h | bool | target indicator | 005F42D0 page 1 row 7 |
| +12Ch | int 0..2 | object detail | 005F42D0 page 3 row 6 |
| +130h | int | antialias sample value, from table 00F88968 | 005F42D0 page 3 row 2 |
| +134h | int | antialias index | 005F42D0 page 3 row 2 |
| +138h | bool | vertical sync | 005F42D0 page 3 row 3 |
| +13Ch | float | gamma, clamped between 00CE38B8 and 00CE6848 | 005F42D0 page 3 row 4 |
| +140h | int 0..2 | texture detail | 005F42D0 page 3 row 5 |
| +144h | bool | ocean reflections | 005F42D0 page 3 row 8 |
| +14Ch | bool | clouds | 005F42D0 page 3 row 9 |
| +150h | int | resolution index into table 00F8895C | 005F42D0 page 3 row 0 |
| +154h | bool | camera water drops | 005F42D0 page 1 row 5 |
| +158h | float | marker alpha | 005F42D0 page 1 row 6 |
| +15Ch | bool | shadows | 005F42D0 page 3 row 7 |
| +15Eh | bool | foliage | 005F42D0 page 3 row 10 |
| +165h | bool | motion blur; mirrored into 00F8D39C+219h | 005F42D0 page 3 row 11 |
| +168h | int 0..2 | old film effect | 005F42D0 page 3 row 12 |
| +170h, +174h | void*, int | downloaded-content entry array and count, 8 bytes per entry | 005F42D0 page 7 |
| +188h | bool | X360 compatibility mode | 005F42D0 page 4 row 0 |
| +18Ah | bool | cockpit mode | 005F42D0 page 1 row 8 |
| +250h | bool | a value on the current page has changed | set by 005F42D0 and 005F8960 |
| +251h | bool | rebuild the current page next update | set by 005F8960, consumed at 005F7B4D |
| +252h | bool | a save target is available | 005F7310, 005F8960 |
| +25Ch | bool | snapshot of the gamepad-UI flag 00E188A8+61Fh | 005F7C40, cleared by 005F0A80 |
| +260h | float | transition timer; nothing accepts input while it is non-zero | 005F7310, 005F7C40, 005F8960 |
| +264h | bool | a write to storage is outstanding | 005F6030 clears it, 005F7310 polls it |
| +265h | bool | a read from storage is outstanding | 005F6030 clears it, 005F7310 polls it |
| +268h | float | seconds the outstanding storage operation has run | 005F7310 |
| +26Ch | void* | 10-byte buffer from `malloc(10)` | 005F6030 |

The constructor also calls 008D7710 twice, under EH states 2 and 3, before it touches the fields
above. Those two sub-objects were not read.

### The page state, 00E19650

The dword at 00E19650 is which options page the screen is showing. It is not the interface id: the
interface stays 0Ch for every page. `BSP_MainMenu_ActivateTopLevelItem` (00588A80) and 005D43C0
reset it to 0 before the screen is raised, so the screen always opens on its own list.

| Page | Set by | Title | Per-frame refresh | Rebuild |
| --- | --- | --- | --- | --- |
| 0 | 00588A80, 005D43C0 | `FE.opt_title_main` (005F54F0) | none | none |
| 1 | 005F58F0 | `FE.opt_title_game` | 005F4C90 | 005F58F0 |
| 2 | 005F59F0 | `FE.opt_title_audio` | 005F4FA0 | 005F59F0 |
| 3 | 005F41E0 | `FE.opt_title_video` | 005F2F20 | 005F41E0 |
| 4 | 005F40F0 | `FE.opt_title_control` | 005F34D0 | 005F40F0 |
| 5, 6 | never | n/a | none | none |
| 7 | 005F3B50 | `FE.opt_title_dlc` | 005F2D20 | none |

The per-frame column is the jump table at 005F7BF8, indexed by page minus one over 0..6; entries
for pages 5 and 6 fall through to the no-op label 005F79E4. The rebuild column is the jump table
at 005F7C28, indexed by page minus one over 0..3, taken only when +251h is set; any other page
clears +251h at 005F7BDA instead.

### The main list, page 0

005F54F0 builds it from the seven pointers at 00E08A48. Each label carries a leading `^`, the
marker the list uses for a row that opens something rather than editing a value. 005F7C40 switches
on the selected row.

| Row | Label | Action |
| --- | --- | --- |
| 0 | `^FE.opt_game` | 005F58F0, page 1 |
| 1 | `^FE.opt_audio` | 005F59F0, page 2 |
| 2 | `^FE.opt_video` | 005F41E0, page 3 |
| 3 | `^FE.opt_controls` | 004CC460(manager, 0Eh, null) -> screen 0Fh |
| 4 | `^FE.opt_controllayout` | 00527C80 with +118h..+11Bh, then 004CC460(manager, 0Dh, null) -> screen 0Eh |
| 5 | `^FE.opt_clantext` | only if 00585B40 and 00585810 both return true, then 005EFAB0 |
| 6 | `^FE.opt_downloaded_content` | 005F3B50, page 7 |

### The settings pages

Each of 005F58F0, 005F59F0, 005F41E0 and 005F40F0 stores the page number, assigns its title and
calls the page builder 005F24D0 with `(title, labels, label_count, value_row_count, help_table, 0)`.
The label array holds the value rows first and the command rows after them.

| Page | Labels | Label count | Value rows | Help table |
| --- | --- | --- | --- | --- |
| 1 game | 00E08BE0 | 0Ch | 9 | 00CF3874 |
| 2 audio | 00E08AC0 | 7 | 4 | 00CF3808 |
| 3 video | 00E08AF8 | 10h | 0Dh | 00CF3820 |
| 4 control | 00E08B78 | 8 | 6 | 00CF385C |

The value column of each row is written by the per-frame refresh through 005F0800(row, text); the
edit itself is 005F42D0, which reads the focused row from 00A9C920, sets +250h and then switches on
page and row. Boolean rows are toggled in place. Enumerated rows are cycled by 005EF4D0(count).
Float rows go through 004155B0 with an explicit range.

#### Page 1, game

| Row | Label | Field | Value shown |
| --- | --- | --- | --- |
| 0 | `FE.opt_game_lang` | +DCh int | 008D48C0 reads the name at 00F88974 + index * 20h + 0Ch, or 00F88A3C |
| 1 | `FE.opt_game_impmetric` | +E0h bool | 008D4260 returns `FE.opt_game_metric` or `FE.opt_game_imperial` |
| 2 | `FE.opt_game_subtitles` | +E1h bool | 00E089F4 table |
| 3 | `FE.opt_game_hints` | +E4h int 0..2 | 00E08A2C table |
| 4 | `FE.opt_game_camerashake` | +E8h bool | 00E089F4 table |
| 5 | `FE.opt_vid_camdrops` | +154h bool | 00E089F4 table |
| 6 | `FE.opt_game_markeralpha` | +158h float | drawn by 005F3750, not by 005F0800 |
| 7 | `FE.opt_game_targetindicator` | +122h bool | 00E089F4 table |
| 8 | `FE.opt_game_cockpitmode` | +18Ah bool | 00E089F4 table |
| 9, 10, 11 | `FE.opt_applyandsave`, `FE.opt_reset`, `FE.opt_cancel` | command rows | |

Row 6 is the reason 005F4C90 passes GUI row 7 for label 7 and GUI row 8 for label 8: the slider is
drawn separately and the plain-text writer skips its slot.

#### Page 2, audio

| Row | Label | Field | Applied by |
| --- | --- | --- | --- |
| 0 | `FE.opt_au_mastervol` | +F8h float | 008D5430 |
| 1 | `FE.opt_au_musicvol` | +100h float | 008D5430 |
| 2 | `FE.opt_au_speechvol` | +108h float | 008D5430, then virtual +34h on +A4h replays a speech sample |
| 3 | `FE.opt_au_effectvol` | +104h float | 008D5430, then virtual +34h on +A0h replays an effect sample |
| 4, 5, 6 | `FE.opt_applyandsave`, `FE.opt_reset`, `FE.opt_cancel` | command rows | |

Audio is the only page that applies on every keystroke, which is what makes the preview audible.

#### Page 3, video

| Row | Label | Field | Notes |
| --- | --- | --- | --- |
| 0 | `FE.opt_vid_res` | +150h index | cycles over 00F88960 entries; copies width to +ECh and height to +F0h from the 8-byte records at 00F8895C |
| 1 | `FE.opt_vid_fullscreen` | +F6h bool | |
| 2 | `FE.opt_vid_antial` | +134h index | cycles over 00F8896C entries; copies the sample count to +130h from 00F88968 |
| 3 | `FE.opt_vid_vsynch` | +138h bool | |
| 4 | `FE.opt_vid_gamma` | +13Ch float | range 00CE38B8 to 00CE6848 |
| 5 | `FE.opt_vid_texd` | +140h int 0..2 | `globals.none`, `globals.basic`, `globals.full` |
| 6 | `FE.opt_vid_objd` | +12Ch int 0..2 | same table |
| 7 | `FE.opt_vid_shadows` | +15Ch bool | |
| 8 | `FE.opt_vid_oceanref` | +144h bool | |
| 9 | `FE.opt_vid_clouds` | +14Ch bool | when 00E188A8+19E8h is set, 00BBDDF0 then 00BBCFE0 apply it live |
| 10 | `FE.opt_vid_foliage` | +15Eh bool | `BSP_FoliageSystem_SetEnabled` (00AD71C0); when turned on, 00AD7A30 also pushes 1.0 into 00E188A8+19FCh |
| 11 | `FE.opt_vid_motionblur` | +165h bool | mirrored into 00F8D39C+219h |
| 12 | `FE.opt_vid_oldfilmeffect` | +168h int 0..2 | `FE.opt_oldfilmeffect_off`, `_cutscenes`, `_always` from 00E08A2C+0Ch. 005F46xx passes the literal 3 to 005EF4D0, so the fourth string `_alwaysfull` is never reached |
| 13, 14, 15 | `FE.opt_applyandsave`, `FE.opt_reset`, `FE.opt_cancel` | command rows | |

Rows 9, 10 and 11 are the three that reach the running renderer as soon as they change; everything
else on this page waits for apply.

#### Page 4, control

| Row | Label | Field |
| --- | --- | --- |
| 0 | `FE.opt_x360_comp` | +188h bool |
| 1 | `FE.opt_ctrl_vib` | +118h bool |
| 2 | `FE.opt_ctrl_icamy` | +11Ah bool |
| 3 | `FE.opt_ctrl_iply` | +11Bh bool |
| 4 | `FE.opt_ctrl_swapsticks` | +119h bool |
| 5 | `FE.opt_ctrl_swapmapsticks` | +11Ch bool |
| 6 | `FE.opt_ctrl_layout` | command row: 00527C80 then interface 0Dh |
| 7 | `FE.opt_applyandsave` | command row |

This page has no reset or cancel row.

#### Page 7, downloaded content

005F42D0's page-7 arm walks the +174h entries at +170h, comparing each against the 18h-byte records
at 00F8A304+4 through `BSP_NativeString_EqualsInsensitive`, and hands the match to 005EFBA0 and
005F1930. The rows are content entries rather than fixed settings, so there is no label table.

### The two enumerated value tables

00E089F4 is the boolean pair: index 0 is `FE.opt_disabled` (00CF3778) and index 1 is
`FE.opt_enabled` (00CF3768). 00E08A2C is the quality run: index 0 `globals.none`, 1 `globals.basic`,
2 `globals.full`, then, at offset 0Ch, the four old-film values.

### Input actions and the edge-plus-repeat rule

Every action below is queried through `BSP_FrontEnd_ActionEdgeOrRepeat` (004D92B0) with ECX set to
the game at 00E188A8, so each obeys the edge-plus-repeat rule of `docs/MAIN_MENU_SCREEN_UPDATE.md`.

| Action | Where | Effect |
| --- | --- | --- |
| 4Bh | 005F7859 | while a storage operation is outstanding, cancel both overlapped handles and clear them |
| 4Bh | 005F79E4 | otherwise: page 0 leaves the screen through 005F0A80, pages 1..4 and 7 go back to the list through 005F5E60, pages 5 and 6 do nothing. The byte table at 005F7C20 (`00 01 01 01 01 02 02 01`) picks the arm |
| 50h | 005F7A2F | pages 1..4: synthesise a reset command row through 00AAB4C0 and 00531030(-59h), push its localised text with 00ABAED0 and dispatch it into the list listener at +0Ch virtual +04h, so the reset button and the reset row share one body |
| 4Fh | 005F7AA7 | only when 00F88A30 is set and the page is 4: copy +118h..+11Bh into the X360 screen and push interface 0Dh |
| 4Ch | 005F7B16 | 005F42D0(this, -1) |
| 4Dh | 005F7B29 | 005F42D0(this, +1) |

4Ch and 4Dh are gated on the byte at +85h of the widget at +88h, so the value arrows only respond
while the focused row is editable. 005F24D0 names `arrow_left_Icon` and `arrow_right_Icon`, the two
glyphs those actions drive.

### List commands

005F8960 is slot +04h of the +0Ch listener vtable. It reads the event kind from the event object's
virtual +5Ch and handles three: kind 2 plays the move sound, kind 6 is a value change that calls
005F42D0, and kind 3 is an activation. For an activation it reads the row's command character from
event+0F0h, defaulting to 00E19658, and compares it against four signed values.

| Command | Meaning | Behaviour |
| --- | --- | --- |
| A7h | `FE.opt_reset` | set +250h and +251h, call the page's defaults writer, then 008D5B50. Page 1 also copies 00F88984 into +D0h. Page 7 empties the binding vector through 00427110. Page 3 also calls XLiveOnResetDevice when 00E198C4 is null |
| A2h | `FE.opt_applyandsave` | page 3 calls 008D5B50 and, when 00E198C4 is null, XLiveOnResetDevice, before the save prompt; every page then raises the save dialog through 00531B00 with callback 005F6FB0, choosing its arm on 00F88A30, +252h and `BSP_XenonSystemManager_HasSelectedUser` |
| A3h | `FE.opt_cancel` | page 0 leaves through 005F0A80, any other page returns to the list through 005F5E60 |
| A5h | `FE.opt_ctrl_layout` | page 4 only: 00527C80 with +118h..+11Bh, then interface 0Dh |

The per-page defaults writers are 008D41C0 (game), 008D41F0 (audio), 008D4520 (video) and 008D4820
(control). 008D4520 is a plain constant store: 280h by 1E0h into +14h and +18h, sample count 0 at
+58h and +5Ch, quality 2 at +54h and +68h, and a run of ones across +60h..+90h. That is what makes
A7h a reset rather than a save.

### Applying and persisting

`BSP_Settings_ApplyAll` (008D5B50) is the single commit point. It runs on reset, on apply-and-save
and on the clean back path in 005F5E60. `BSP_Settings_ApplyAudio` (008D5430) is the live audio
path and runs on every volume keystroke. The settings block itself is the one
`docs/APP_INIT_BOOTSTRAP.md` describes; this packet did not read 008D5B50's body, so which fields
it pushes into the registry was not established here.

The device reset for a resolution change is `XLiveOnResetDevice` (00A4D46A), called from both the
A7h and A2h arms of page 3 and only when 00E198C4 is null, that is, when the multiplayer menu
manager does not exist. 005F65C0 is the reverse direction: it compares 00F889F8 against +150h and
00F88A30 against +188h and reloads the screen from the settings block, and it is what the update
virtual calls when an outstanding storage read completes.

### Leaving the screen

005F0A80 is the only exit to the main menu. It clears the gamepad-prompt byte at 00E19698+4, clears
+A8h and +25Ch, and then branches three ways: with no multiplayer menu manager (00E198C4 null) it
enqueues `BSP_Game_RequestState(16h)` followed by `BSP_Game_RequestState(4)`, the same pair
`docs/MAIN_MENU_SCREENS.md` records for the Options item in reverse; otherwise it calls virtual
+08h on 00E198C4 or on 00E198B4 depending on 00E188A8+1FE4h. The screen set is raised by the
manager's own `Activate`, not from here.

005F5E60 is the softer back: if +250h is clear it calls 008D5B50 and rebuilds the list through
005F54F0; if a value has changed it raises the discard prompt 00531B00 with callback 005F5B90
instead.

## Screen 0Eh, the gamepad layout screen (00527D00, 30h)

The smallest of the three. The constructor installs the vtables and nothing else; register
(00527E60) binds the two GUI pages and clears +2Ch.

| Offset | Type | Meaning | Evidence |
| --- | --- | --- | --- |
| +00h, +08h, +0Ch | void* | vtables 00CECF48, 00CECF34, 00CECF14 | 00527D00 |
| +10h | void* | GUI page `FE_controls_X360_listbox` | 00527E60 |
| +14h | void* | GUI page `FE_controls_X360` | 00527E60 |
| +18h | void* | `Main_Listbox` widget | 00528240 |
| +28h..+2Bh | bool | vibration, swap sticks, invert camera Y, invert flight Y | 00527C80 |
| +2Ch | bool | the four bytes above have been supplied | 00527C80 sets it, 00527E60 clears it |

00527C80 is the setter the options screen calls before pushing interface 0Dh. Its four parameters
arrive in source order vibration, swap sticks, invert camera, invert flight, matching the push
order at 005F7ACA (`[esi+118h]`, `[esi+119h]`, `[esi+11Ah]`, `[esi+11Bh]`).

The enter virtual 00528240 skips everything when the gamepad-UI flag at 00E188A8+61Fh is set;
otherwise it assigns the title `FE.opt_title_control`, resolves `Main_Listbox` into +18h and fills
the list. The update virtual 005289B0 is 20h bytes: action 4Bh pushes interface 0Ch on the options
manager, returning to screen 0Dh, and everything else falls straight to `ret 4`. 005289E0, a
separate function, pushes 0Ch as well and is the list-listener path for the same return.

## Screen 0Fh, the keyboard and mouse screen (00555770, 250h)

| Offset | Type | Meaning | Evidence |
| --- | --- | --- | --- |
| +00h, +08h, +0Ch | void* | vtables 00CEE524, 00CEE510, 00CEE4F0 | 00555770 |
| +14h | void* | GUI page `FE_controls_PC_listbox` | 00555C60 |
| +18h | void* | GUI page `FE_controls_PC` | 00555C60 |
| +3Ch, +40h | dword | cleared by the constructor | 00555770 |
| +E0h | int | initialised to 10 | 00555770 |
| +E4h | bool | cleared by the constructor | 00555770 |
| +16Ch..+17Ch | dword | five words cleared by register | 00555C60 |
| +180h | int | set to 1 by register | 00555C60 |
| +184h, +188h | dword | cleared by the constructor | 00555770 |

The constructor also calls 00683610, the front-end screen animation block owned by another packet;
it was not analysed here. The exit virtual 00551B90 is a single `ret`. The segment this class lives
in carries the strings `opt_normal`, `opt_inverted`, `opt_cancel`, `preset`, `presets` and
`scroll_up_icon`, which is the vocabulary of a rebindable key list with axis inversion and named
presets, but the enter virtual 0055FA80 and the update virtual 00560430 were not read, so the
binding model of this screen is not recovered.

## Callers and callees

Callers of the three constructors: 006898C0 only. Callers of 00527C80: 005F7310, 005F7C40,
005F8960. Callers of 005F42D0: 005F7310 and 005F8960. Callers of 005F65C0: 005F7050, 005F7310,
005F7C40, 005F8960. Callers of 005F0A80: 005F7310 and 005F8960. Callers of 005F40F0: 005F7310
only, which is why page 4 has no entry point on this image; see the uncertainties.

Shared callees worth naming: 004F71D0 register, 004C12B0 GUI manager, 00AA5840 page lookup,
00AA7E00 widget lookup, 00ABAED0 localised text, 004155B0 the clamped float step, 005EF4D0 the
modular integer step, 00531B00 the modal prompt, 00A9C920 focused row, 00A9C990 selected row.

## Uncertainties

- Page 4 is unreachable on this image. The only routine that stores 4 into 00E19650 is 005F40F0,
  and its only caller is the rebuild arm that already requires the page to be 4. The main list
  routes `^FE.opt_controls` to interface 0Eh instead. 005F54F0's platform filtering was not
  decoded, so whether another build reaches page 4 through the same list is open.
- Rows 5 and 6 of 00E19650 are never written by anything found here.
- 005F5BD0, the enter virtual, was measured but not read; its extent (005F5BD0..005F5E54) and RET
  come from a linear decode, not from Ghidra.
- 008D5B50 and 008D5430 were identified by call site and role only. Which registry values under
  `BSP_GameSettings_*` each field lands in is therefore not established by this packet.
- 005F42D0's `param_1 + A0h` is the selected-row store in 005F7C40's shifted view and a sound
  object in 005F42D0's own view. Both readings come from Ghidra bodies with dropped arguments; the
  layout table above keeps them at +A0h/+A4h and +A8h respectively, which is consistent with
  005F0A80 clearing +A8h, but the sound-channel reading was not confirmed in the listing.
- The event kinds 2, 3 and 6 in 005F8960 come from the event object's virtual +5Ch and were not
  traced to their producer.
- 00CF3820, 00CF3808, 00CF385C and 00CF3874 are the four help tables. 00CF3820 is sixteen dwords of
  1, so the field is a flag rather than a string index; the other three were not dumped.

## Ghidra function coverage

These addresses have no Ghidra function and need one defined before a name can be applied.

| Address | End | Role |
| --- | --- | --- |
| 005F5BD0 | 005F5E54 | screen 0Dh enter, slot +18h |
| 005F7020 | 005F7041 | screen 0Dh exit, slot +1Ch; tail-jumps to 005F6EF0 |
| 00527E40 | 00527E5B | screen 0Eh scalar deleting destructor, slot +0Ch |
| 00528920 | 005289A1 | screen 0Eh exit, slot +1Ch |
| 005289B0 | 005289D0 | screen 0Eh update, slot +20h |
| 00528A60 | 00528A80 | screen 0Eh dependency list, slot +24h |
| 0055FA80 | 0055FC99 | screen 0Fh enter, slot +18h |
| 00551B90 | 00551B90 | screen 0Fh exit, slot +1Ch; one `ret` byte |

## State reached

| Address | State |
| --- | --- |
| 005F6030, 00527D00, 00555770 | analysed; constructors read in full |
| 005F1CA0, 00527D30, 00555800 | analysed from raw bytes; id leaves decoded |
| 005F0250, 00527E60, 00555C60 | analysed; register bodies read in full |
| 005F7310, 005F42D0, 005F7C40, 005F8960 | analysed; bodies and jump tables read |
| 005F58F0, 005F59F0, 005F41E0, 005F40F0, 005F3B50, 005F5E60, 005F0A80, 00527C80 | analysed |
| 005F1CC0, 00527F90, 00528240, 005F6000 | analysed; partial bodies read |
| 005F3B30, 00555C40, 00555DC0, 00560430, 00558680, 005F65C0, 005F54F0, 005F24D0, 005F0800 | identified by role only |
| 005F5BD0, 005F7020, 00527E40, 00528920, 005289B0, 00528A60, 0055FA80, 00551B90 | analysed from raw bytes; no Ghidra function |
| screen ids, interface ids, page table, row tables, command characters, action ids | reconstructed, build-tested |

Nothing here is ABI compatible or game validated.

## Follow-up packets

- `options_settings_commit`: 008D5B50, 008D5430, 008D41C0, 008D41F0, 008D4520, 008D4820, 008D4210,
  008D44C0, 008D48C0, 008D4260. Files `docs/OPTIONS_SETTINGS_COMMIT.md`,
  `include/bsp/options_settings_commit.hpp`. Contract: the settings block layout behind the screen
  fields, which registry value each one persists to, and what ApplyAll pushes into the renderer,
  the sound system and the input layer.
- `options_pc_controls_screen`: 0055FA80, 00560430, 00558680, 00555DC0, 0055D180, 0055D330,
  00560BF0. Files `docs/OPTIONS_PC_CONTROLS.md`, `include/bsp/options_pc_controls.hpp`. Contract:
  the key-binding model of screen 0Fh, its presets, its conflict handling and how it writes back
  into the `input_settings` tables.
- `options_page_builder`: 005F24D0, 005F54F0, 005F0800, 005F3750, 005F1620, 005F0CD0, 005EF4D0,
  004155B0. Files `docs/OPTIONS_PAGE_BUILDER.md`, `include/bsp/options_page_builder.hpp`. Contract:
  how a label array plus a value-row count becomes a list, where the command characters A2h, A3h,
  A5h and A7h are attached, and the exact stepping rules for the enumerated and float rows.
- `options_storage_prompt`: 005F6FB0, 005F5B90, 005EFCB0, 005EFCD0, 005EF4F0, 00531B00, 005F7050.
  Files `docs/OPTIONS_STORAGE_PROMPT.md`. Contract: the overlapped save and load path the update
  virtual polls, and the four modal callbacks the options screen installs.

## Correction from docs/OPTIONS_SETTINGS_COMMIT.md

No field of the settings block at `00f88980` reaches the registry, and none of the commit-path routines writes the options file. Persistence runs through the serializer `008d64a0` (24-row key table, omit-when-default rule); the path builder `008d5150` has only two callers, both readers, and the only options-file writer in the binary belongs to the hardware probe. The shipped options file carries exactly the loader's tokens plus `HardwareReported`, matched case-insensitively. The four control bytes described earlier as detail bytes are input stick modifiers.
