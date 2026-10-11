# In-game HUD screens (segment 25) and GUI widgets (segment 79) (cc13-hud1)

Packets `cc13_hud1_seg25` (0060CB60..0061D330, extended down to 0060C7D0 by `cc13_hud1_planespawn`
for the plane-spawn class) and `cc13_hud1_seg79` (00AC97F0..00AD5360). Code:
`include/bsp/hud_widgets.hpp`, `src/hud_widgets.cpp`. Report: `reports/cc13_hud1.json`
(host_steps checked with `tools/verify_report_calls.py`). Functions defined in Ghidra:
`reports/cc13_hud1_defined.json`.

Names are hypotheses, not recovered symbols. Status words: *named* (ledger + Ghidra rename),
*reconstructed* (C++ written from the listing), *build-tested* (`scripts/build.ps1` passes). Nothing
here is fixture-tested, ABI-compatible or game-validated; the C++ exposes new host interfaces.

## Segment 25 is four HUD screens, not the minimap

The partition keywords (`usn_point_text`, `radar_sweep`, `pumpermanent`) belong to four screen
classes; the minimap itself is slot 35h at 005BEC50 (docs/HUD_MINIMAP.md).

| Class | Vtable | Slot id | Range | Evidence |
| --- | --- | --- | --- | --- |
| plane-spawn screen | 00CF4A7C (+ listener 00CF4A64) | 41h | 0060C7D0..0060D290 | `GUI_plane_spawn`, constructor 0060CAE0 |
| `_PleaseWait` screen | 00CF4AB8 | 53h | 0060D290..0060D570 | docs/INTERFACE_RUNTIME_TAIL.md |
| multiplayer scoreboard | 00CF4B98 (+ 00CF4B84 listbox, 00CF4B64 widget listeners) | 5Eh | 0060D570..006113A0 | `GUI_scoring`, `ingame.score_usa/jap` |
| powerup manager | 00CF4DEC (+ 00CF4DE8 listener) | 4Fh | 00614170..0061C7F0 | `GUI_support_full`, `GUI_powerups_info`, `PUMPERMANENT`, `PUM1STUSE` |

006113A0..00614170 is mostly red-black-tree and list node code shared with other screens (callers
005CB0F0, 008E8000, 00663D60, 0068C1F0); 0061C7F0 onward belongs to the next class (callers in
0061DF30..0062BEF0).

Slot layout follows the base front-end vtable 00CEAE54 (+10h register, +14h layout, +18h enter,
+1Ch exit, +20h update, +24h collect). The vtables were found from constructor stores
(`local/h1_vtstores.py`) and `.rdata` references to the already named slot functions.

## The multiplayer scoreboard (slot 5Eh)

Object, from the constructor 0060FD90, the destructor body 0060FE50 and the field users below:

| Offset | Field | Written at |
| --- | --- | --- |
| +04h/+05h | requested / applied | 0060DD10 |
| +08h / +0Ch | listbox listener 00CF4B84 / widget listener 00CF4B64 | 0060FDC4 / 0060FDCB |
| +10h | `Header_Group` (hidden) | 0060F2ED |
| +14h | clone of the listbox's `Player_Group`, the row template | 0060F3F9 |
| +18h | `Main_Listbox` | 0060F368 |
| +1Ch..+28h / +2Ch..+38h | team 0 / team 1 `std::vector` of 24h-byte rows | 00610336 / 0061035A |
| +3Ch / +40h | team 0 / team 1 header clones | 006103D5 / 00610B95 |
| +44h | the `GUI_scoring` page | 0060F27A |

The 24h-byte row (copy 0060DFB0/0060F020, destroy 0060DE10): name NativeString +0h, clan
NativeString +8h, score +10h, valid byte +14h, slot +18h, player record +1Ch, row widget +20h.

### Routines

| Address | Name | ABI | Status |
| --- | --- | --- | --- |
| 0060D740 | `BSP_HudScoringScreen_Register` | thiscall, RET | reconstructed, build-tested |
| 0060F210 | `BSP_HudScoringScreen_LoadLayout` (existing name) | thiscall, RET | reconstructed, build-tested |
| 00610070 | `BSP_HudScoringScreen_Enter` | thiscall, RET (00611351) | reconstructed, build-tested |
| 0060D980 | `BSP_HudScoringScreen_Exit` (existing name) | thiscall, RET | reconstructed, build-tested |
| 0060E440 | `BSP_HudScoringScreen_Update` | thiscall(float), RET 4 (0060EF43) | reconstructed, build-tested |
| 0060DD10 | `BSP_HudScoringScreen_Close` | thiscall, RET | reconstructed, build-tested |
| 0060DE00 | `BSP_HudScoringScreen_OnListboxActivate` | ECX-8, RET 8 | named |
| 0060E0E0 | `BSP_HudScoringScreen_OnListboxSelectionChanged` | thiscall(sender, listbox), RET 8 | reconstructed, build-tested |
| 0060DE90 | `BSP_HudScoringScreen_OnWidgetActivated` | thiscall(widget), RET 4 | reconstructed, build-tested |
| 0060D9F0 / 0060DA40 | `..._OnWidgetFocus0C` / `..._OnWidgetFocus18` | thiscall(widget), RET 4 | reconstructed, build-tested |
| 0060DBC0 | `BSP_HudScoringScreen_ListboxInputFilter` | fastcall(ECX up*, EDX down*, select*), RET 4 | reconstructed, build-tested |
| 0060EF50 | `BSP_HudScoringScreen_HidePingIcons` | thiscall(group), RET 4 | reconstructed, build-tested |
| 0060FD90 / 0060FE50 | `..._Construct` / `..._Destroy` | thiscall | named |
| 0060D760..0060FFC0 | `STL_HudScoreRowVector_*` (22 routines), `BSP_HudScoreRow_*` (3) | STL | named |

Behaviour, all from the listing:

- **Enter** (00610070) clears both header pointers, clears the listbox, shows it, calls its +60h(1)
  and writes the row spacing 0.0225 (00CF4C34) to listbox+12Ch with +130h = +134h = 0. Unless
  game+1FE4h (online) or byte game+218Ch is set it stops there (JE 0061133C). It erases both row
  vectors (0060F960) and walks the eight player pointers game+18CCh..+18ECh. A slot contributes a
  row when byte +8h is set; a human (+9h clear) gives name +58h and clan +78h; an AI needs byte +0Ah
  and gets the label `00E08940[+2Ch]` (`FE.easy` x4, `FE.normal`, `FE.hard`) with clan `" "`
  (00CE3A90). The score is `*(game+21A0h)+1E4h+slot*284h`. Team +28h = 0 goes to +1Ch, 1 to +2Ch,
  anything else nowhere; both totals are summed. Each non-empty team clones `Header_Group` (the header pointers were cleared at the top, so every
  enter clones afresh; the exit destroys the clones), hides the template, positions the clone (team 1 offset by the running y + 0.015), sets `USN_Icon`
  state 0 (USN) or 1 (JAP) through icon slot +88h(state, 0, 1.0), the total into `USN_point_Text`
  and the key `ingame.score_usa`/`ingame.score_jap` into `USN_Text`. Rows: each valid record clones
  the template, is named after its slot (vtable +2Ch with `NativeString_FromInt(slot)`), copies
  listbox+104h into row+D8h, is appended (00A9D750) and selected (00A9C740), shown, positioned at
  y += 0.03 (double 00CEB690 added to a float cursor) and gets `Player_point_Text`,
  `Player_name_Text` (localised when the player is an AI or the name equals `globals.newplayer`
  case-insensitively; an empty name shows `FE.multi_lobby_normalai`) and `Player_clan_Text`.
- **Update** (0060E440): actions 4Ah or 4Bh close; action 4Fh reads the selected row's name with
  `_atol` as the slot and, for a human player with a selected signed-in user (state 2) and the
  platform virtual +19Ch false, opens `XShowGamerCardUI`. Any failed test there returns before the
  row refresh. Then team 1 and team 0 rows: rows whose player +20h is -1 are skipped; the score is
  re-read through player+20h, written to `Player_point_Text` and summed; `mic_Icon` is shown only
  when 007F74B0 (spoke within the window at game+64Ch) and the 8x8 mute byte 00778740 on
  game+2080h is clear. Finally the headers get the new totals (+40h first, state 1; then +3Ch, 0).
- **Close** (0060DD10): online hands back with 006881F0; offline raises the manager's +D8h screen.
- **Command buttons** carry their action byte as the first character of the widget name
  (widget+F0h): A3h back, A5h gamer card, matching the -5Dh/-5Bh commands 0060E0E0 gives 0054B530.

Uncertainty: what game+218Ch means (it admits the scoreboard offline); whether listbox +60h is
"enabled"; which device 00F8BBF4+94h's first entry is (treated as pad 0); player+20h as the score
index is inferred from its use in the update, the enter indexes by slot instead.

## The `_PleaseWait` helpers

docs/INTERFACE_RUNTIME_TAIL.md covers the object and its slots. New here: slot +24h 0060D560 had
no Ghidra function and was defined (`BSP_PleaseWaitScreen_CollectChildren`); 0060D2B0/0060D2C0 are
its id (53h) and self-managed slots. The routines that drive it from outside:

| Address | Name | ABI | Status |
| --- | --- | --- | --- |
| 0060D6E0 | `BSP_PleaseWaitScreen_ShowWithWorker` | thiscall(bool), RET 4 | reconstructed, build-tested |
| 0060D3E0 | `BSP_PleaseWaitScreen_Hide` | thiscall, tail JMP 004F83B0 | reconstructed, build-tested |
| 0060D650 | `BSP_PleaseWaitScreen_WorkerFrame` | callback, ECX = context (page), RET | reconstructed, build-tested |
| 0060D570 | `BSP_PleaseWaitScreen_RenderFrameNow` | no arguments, RET | reconstructed, build-tested |

Show sets both bytes, commits, enters, shows `Background_Icon` (+0Ch) per its argument and starts
the GUI render worker (00AA4040) with descriptor = context = the page, callback 0060D650 and rate
25. The worker frame and the synchronous frame both run 00AA3F70 over a vector of pages with step
0.1 (00D7A2F0); the synchronous one adds `*(00E18D48)+1Ch` and then renders one game frame
without testing 004C6C30's result.

## The plane-spawn screen (slot 41h)

| Address | Name | ABI | Status |
| --- | --- | --- | --- |
| 0060CAE0 / 0060D130 | `BSP_HudPlaneSpawnScreen_Construct` / `_Destroy` | thiscall | named |
| 0060C7D0 | `BSP_HudPlaneSpawnScreen_OnObjectReleased` | thiscall(screen+8, object), RET 4 | reconstructed, build-tested |
| 0060C830 | `BSP_HudPlaneSpawnScreen_Update` | thiscall(float), RET 4 / tail JMP 00647300 | reconstructed, build-tested |
| 0060C880 | `BSP_HudPlaneSpawnScreen_Exit` | thiscall, tail JMP 00681EC0 | reconstructed, build-tested |
| 0060CA20 | `BSP_HudPlaneSpawnScreen_GetTargetId` (slot +28h) | thiscall, EAX | reconstructed, build-tested |
| 0060CF90 | `BSP_HudPlaneSpawnScreen_AttachPlane` | thiscall(plane) | named |
| 0060C930, 0060CB60, 0060CC90, 0060CD50, 0060C620, 0060C960 | `BSP_PlaneSpawnCamera_*` | | named |
| 0060CB40 | `BSP_Unit_AsKind9OrNull` | | named |

Fields: +1Ch an observed entity (the camera: 0060CF90 stores the new 3CCh-byte camera there),
+20h the plane, +24h the AirOps block whose +7Ch is observed, +28h the page. The exit releases
the three observations twice over (two copies of the same block at 0060C883 and 0060C8BF). The
camera (vtable 00CF4930 over base 00432750) is attached by 0060CF90 from
`BSP_InGameInterface_ApplyPendingInterface` and published with `BSP_Game_SetCameraMover`; its
pose code is x87 and was named, not reconstructed.

## The powerup manager (slot 4Fh)

Named, not reconstructed: register 00614170, layout 00614600 (existing name; 16 widgets:
`New_Group` +B4h under `GUI_powerups_info` +B0h with `text_Text` +D4h, `left/right/empty_circle_Icon`
+E0h/+E4h/+E8h, `tile_Icon` +ECh, `icon_Icon` +F0h; `center_Group` -> `power_up_Group` +B8h under
`GUI_support_full` +24h with `color_Icon` +C0h, `radar_Icon` +BCh, `sipp_Icon` +C4h, `szub_Icon`
+C8h, `plane_Icon` +CCh, `command_Icon` +D0h), enter 0061B850, exit 006169F0, update 0061BEA0,
collect 00619170, constructor 0061A480, destructor body 00618EA0 and the helpers listed in the
ledger rows (carousel at +100h, `STL_StringMapCI_*`). The screen drives the support-manager
screen's widgets (`*(00E198C4)+BCh`) and its own (`+C0h`), and the PUM hints through the award
tracker. 00618980 is tagged `cg_vector_deleting_dtor` but is the carousel's selected-label refresh;
the ledger name overrides the tag.

## Unreferenced code with no Ghidra function

No rel32 or absolute reference anywhere in the image (`local/h1_xref.py`), so these were left
undefined: 0060C810, 0060CA70, 0060CA90 (plane-spawn area), 0060D730, 0060DC90, 0060DCD0 (checked
iterator helpers of the row vector), 0060F4E0, 00611360, 00611370, 00611380, 00611390.

## Segment 79: the Grid, Model, Sound, ClipBox, FrameBox and AnimIcon types

The type ids come from the loader table in docs/GUI_LAYOUT_LOADER.md: each `00AA6560` case stub
calls a per-class pool allocator (`MOV ECX,pool; JMP allocate`) and then the copy constructor with
the class prototype as the source. Five of those allocators carried the tag
`cg_static_dtor_stub`; they are pool stubs like FrameBox's `00AD0280` and were renamed.

| Type | Id | Vtable | Pool (vtable, global) | Stub / allocate | Prototype ctor / copy ctor |
| --- | --- | --- | --- | --- | --- |
| Grid | 8 | 00D5CE60 | 00D5CE08, 00F8C050 | 00AC9EA0 / 00AC9C10 | 00ACB0A0 / 00ACB3F0 |
| Model | 9 | 00D5CF38 | 00D5CEF0, 00F8C09C | 00ACC490 / 00ACC350 | 00ACC740 / 00ACCF70 |
| AnimIcon | 12 | 00D5BCE0 (Icon-derived) | 00D5D1DC, 00F8C1D4 | 00AD3960 / 00AD3820 | (00AA1C60, seg 76) |
| Sound | 14 | 00D5CFD0 | 00D5CFBC, 00F8C0EC | 00ACD7F0 / 00ACD6B0 | 00ACDB30 / 00ACDBA0 |
| ClipBox | 16 | 00D5D058 | 00D5D0D4, 00F8C13C | 00ACEA10 / 00ACE8D0 | docs/GUI_CLIP_BOX.md |
| FrameBox | 18 | 00D5D130 | 00D5D0FC, 00F8C188 | 00AD0280 / 00AD0050 | docs/GUI_FRAMEBOX_WIDGET.md |

The pools are `Mit::allocator_list_elem` objects linked on 00E188B4 with a critical section, an
array of chunks and a first-free index; each chunk holds sixteen (eight for Model) blocks with a
16-bit free stack at the chunk tail. Pool constructor, destructor, free and trim (vtable slot 0)
were named for all six by the Win32 calls they make (InitializeCriticalSection,
DeleteCriticalSection, Enter/LeaveCriticalSection) and by their stores of the pool vtable.

A naive vtable walk overruns into the next table: Model's 33 slots end where the Sound pool
vtable 00D5CFBC begins (so "Model +84h" is the pool trim 00ACD610), and ClipBox's +7Ch is its
pool trim 00ACE830.

### Properties

Property visitors pass each key and each (kind, pointer) as 8-byte pairs. Kinds seen: 0 string,
2 float, 3 bool, 5 vec3, 6 vec2. A reader's +0Ch takes a default, +10h does not; a describer
writes through +0Ch and, except where noted, only after `00BD5680` says the value differs from
the default. The C++ carries one table per type (`kGuiGridProperties`, `kGuiModelProperties`,
`kGuiSoundProperties`, `kGuiAnimIconProperties`) and the shared drivers `gui_read_properties` /
`gui_describe_properties`.

| Type | Key | Kind | Field | Default |
| --- | --- | --- | --- | --- |
| Grid | `CellSize` | vec2 | +104h | none (+10h read); describer always writes |
| Model | `ModelName` | string | +10Ch | none |
| Model | `ModelTextureOverride` | string | +114h | `""` |
| Model | `ScaleVector` | vec3 | +F4h | (1,1,1) |
| Model | `RotationEuler` | vec3 | +100h | (0,0,0) |
| Sound | `SoundEfx` / `Category` | string | +F8h / +100h | none; describer always writes (00F8C134 for null) |
| AnimIcon | `FPS` | float | +138h | 30.0 (00CE38C8) |
| AnimIcon | `PlaybyDefault` / `Loop` / `Forward` | bool | +140h / +141h / +142h | 1 |

### Reconstructed

| Address | Name | ABI | Status |
| --- | --- | --- | --- |
| 00ACA0B0 | `BSP_GuiGrid_ConstructedHook` (+74h) | thiscall, RET | reconstructed, build-tested |
| 00ACA170 | `BSP_GuiGrid_LoadedHook` (+78h) | thiscall, RET | reconstructed, build-tested |
| 00ACA290 | `BSP_GuiGrid_LayoutCells` (+84h) | thiscall, RET | reconstructed, build-tested |
| 00ACA540 | `BSP_GuiGrid_SyncScrollbars` (+7Ch) | thiscall, RET | reconstructed, build-tested |
| 00ACB140 | `BSP_GuiGrid_IsKindOf` (+0Ch) | thiscall(id), RET 4 | reconstructed, build-tested |
| 00AC9900 / 00AC9950 | Grid read / describe (+18h/+1Ch) | thiscall(visitor), RET 4 | reconstructed (table), build-tested |
| 00ACDDB0 / 00ACD800 | Sound read / describe | thiscall(visitor), RET 4 | reconstructed (table + binding tail), build-tested |
| 00ACDF10 | `BSP_GuiSound_SetVisible` (+34h) | thiscall(bool), RET 4 | reconstructed, build-tested |
| 00ACD2C0 | `BSP_GuiSound_Update` (+40h) | thiscall(float), RET 4 | reconstructed, build-tested |
| 00AD3430 / 00AD3550 | AnimIcon read / describe | thiscall(visitor), RET 4 | reconstructed (table), build-tested |
| 00AD2E00 / 00AD2E10 / 00AD2E20 | AnimIcon `SetForward` / `SetLoop` / `SetFps` (+A8h/+ACh/+B0h) | thiscall(arg), RET 4 | reconstructed, build-tested |

Grid fields: +20h/+24h size, +ECh column vector (14h-byte records: width +0h, cell widget vector
+4h..+10h), +104h/+108h `CellSize`, +10Ch a byte the prototype sets to 1, +110h/+114h scroll
offsets, +118h `Visual_Group`, +11Ch/+120h content size. The layout restarts y at 0 for each
column, so +120h holds the last column's height, not the tallest; the scrollbar maxima are
content minus widget size, and the scrollbar values are `-0.0 - offset`.

Sound fields: +ECh the sample (refcounted), +F0h the tracked sound, +F4h the class, +F8h/+100h the
two strings. Showing a Sound with no live sound creates one of type `Normal`; hiding stops and
releases it; the update hides the widget once the sound reports finished.

### Defined in Ghidra

26 vtable targets had no function and were defined from disk extents (`local/h1_extent.py`,
recursive descent, every one followed by INT3 padding or the next function):
00AC9900, 00AC9950, 00ACA170, 00ACB140, 00ACBCD0, 00ACBCE0, 00ACC780, 00ACD0D0, 00ACD0E0,
00ACDB70, 00ACDF10, 00ACE080, 00ACE090, 00ACE0C0, 00ACEAE0, 00ACEAF0, 00ACEB00, 00AD26A0,
00AD2720, 00AD2A80, 00AD2DF0, 00AD2E00, 00AD2E10, 00AD2E20, 00AD3040, 00AD3A20.

### Unreferenced in segment 79

Per class two type-info registration helpers (guard byte, class-name pointer at +0Ch, the base
chain copied from 00F8BC88, the id from 006FAC20's counter) have no Ghidra function and no rel32
or absolute reference: 00AC9F70/00ACA030 (`cGuiGrid`), 00ACC650/00ACC6F0 (`cGuiModel`),
00ACD890/00ACDAE0 (`cGuiSound`), 00ACEA20/00ACEA70 (`cGuiClipBox`), 00AD0780/00AD07D0
(`cGuiFrameBox`). They were left undefined.
