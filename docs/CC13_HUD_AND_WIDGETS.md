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

Reconstructed and build-tested: register 00614170 (`hud_powerup_register_00614170`: 004F71D0, the
layout, power_up_Group and `map_Group` hidden, +23Ch..+244h = `right_circle_Icon`'s resolved
position with x minus +234h, +F4h..+FCh = the info page's, then counters and the +1A4h vector
cleared) and layout 00614600 (`hud_powerup_layout_00614600`, the `kHudPowerupBindings` table; 16 widgets:
`New_Group` +B4h under `GUI_powerups_info` +B0h with `text_Text` +D4h, `left/right/empty_circle_Icon`
+E0h/+E4h/+E8h, `tile_Icon` +ECh, `icon_Icon` +F0h; `center_Group` -> `power_up_Group` +B8h under
`GUI_support_full` +24h with `color_Icon` +C0h, `radar_Icon` +BCh, `sipp_Icon` +C4h, `szub_Icon`
+C8h, `plane_Icon` +CCh, `command_Icon` +D0h). Named, not reconstructed: enter 0061B850, exit 006169F0, update 0061BEA0,
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

## Handoff (cc13-hud1, 2026-10-11)

State on `agent/cc13-hud1`: 161 ledger names (76 in segment 25 and the plane-spawn range, 85 in
segment 79), 27 functions defined in Ghidra (`reports/cc13_hud1_defined.json`), 38 reconstruction
records in `src/hud_widgets.cpp` / `include/bsp/hud_widgets.hpp`, all build-tested only;
`reports/cc13_hud1.json` has 261 host_steps rows and `tools/verify_report_calls.py` reports 0
failures. Nothing was run in `bsp_game`; none of this is wired into a game host. No gameplay
RNG is drawn anywhere here.

Tools in this worktree's `local/` (copy them, the tree is temporary): `h1_survey.py` (segment
table with sizes, callers and strings), `h1_vtstores.py` (vtable stores per function),
`h1_refs.py` (rdata/text dword search), `h1_binds.py` (page/widget binding extractor: literal,
lookup parent, destination field), `h1_show.py` (compact decompile view from the shared exports),
`h1_callsum.py` (callee/caller summary), `h1_names25.py`, `h1_names79.py`, `h1_funcs25.py`,
`h1_funcs79.py` (the ledger batches), plus the copied `h1_*` helpers from cc13-frontend2 and
`h1_rw_*` from cc13-rewards1.

Open, in order of value:

1. The powerup-manager enter 0061B850, update 0061BEA0, exit 006169F0 and the two carousel
   updates 00619C50 (ECX = screen+100h) / 0061A6C0. They drive the support-manager screen's
   widgets (`*(00E198C4)+BCh`, cc9's support-manager lane reads the same object, so ask cc9
   before wiring) and the PUM hints through the award tracker. 0061B0F0 rebuilds the lists from
   `BSP_PowerUpManager_CollectActive`.
2. The plane-spawn camera (vtable 00CF4930; 0060CB60, 0060CC90, 0060CD50) is x87 pose code; it
   overlaps the camera lane (check cc13-camera1 before taking it).
3. Segment 79's Grid cell containers (00ACA730..00ACB9B0, tree/vector instantiations around the
   column records), the Model loader trio 00ACCCF0/00ACCB90/00ACC8A0, and the AnimIcon update
   00AD3040 (x87 frame stepping against the clock at *(01090AB0)+14h).
4. 006113A0..00614170: shared tree/list node routines with callers in several screens; name them
   once their owners are known.
5. Unreferenced Ghidra-less code is listed in the two "Unreferenced" sections above; it was left
   undefined on purpose.

## The plane-spawn camera (second packet, cc13-hud1)

Lease `cc13_hud1_camera` (0060C7D0..0060D130). Conventions follow docs/CC13_CAMERA_CONTROLLERS.md:
x87 chains in double with a float cast at every native FSTP. Correction to the first packet:
slot +28h 0060CA20 returns an entity pointer (0060CF90 observes it and passes it to 006BCD20),
so it is now `BSP_HudPlaneSpawnScreen_GetHomeEntity`; the C++ keeps `plane_spawn_target_id_0060ca20`.

| Address | Name | ABI | Status |
| --- | --- | --- | --- |
| 0060CF90 | `BSP_HudPlaneSpawnScreen_AttachPlane` | thiscall(plane), RET 4 | reconstructed, build-tested |
| 0060CD50 | `BSP_PlaneSpawnCamera_SetTargets` | thiscall(home, plane), RET 8 | reconstructed, build-tested |
| 0060CC90 | `BSP_PlaneSpawnCamera_UpdatePose` (vtable 00CF4930 +DCh) | thiscall(float dt), RET 4 | reconstructed, build-tested |
| 0060CB60 | `BSP_PlaneSpawnCamera_GetTargetPosition` | thiscall(Vec3* out), EAX, RET 4 | reconstructed, build-tested |

- **Attach** (from 0068ACA0): +20h = plane, observed through screen+8; the home entity (slot
  +28h) is observed and its AirOps block (006BCD20, DL = 0) goes to +24h. The first attach
  creates the 3CCh-byte camera at +1Ch (operator new, memset, 00432750, vtables 00CF4930 /
  00CF4914 / 00CF490C / 00CF48F4), runs 00923870(camera, 0, game+19CCh, identity) even when the
  allocation failed, and observes it. Then SetTargets(camera, block ? block+7Ch : 0, plane) and
  `BSP_Game_SetCameraMover(camera)`.
- **SetTargets** swaps the observed pair +384h (home) / +388h (plane). With an AirOps block the
  seed matrix +38Ch is the block's vtable +18h matrix; otherwise it is the plane pose (+CCh) and
  the translation row +3BCh..+3C4h gets the eye offset, with t = game-tuning +7Ch:
  `x = f(f(f(r2x*t)*1.5) + f(-r0x*t))`, `y = f(t*1.2 + f(f(f(0*t)*1.5) + f(0*t)))`,
  `z = f(f(1.5*f(r2z*t)) + f(-r0z*t))` (rows r0 = +CCh, r2 = +ECh; constants 00D7A258 = 0.0,
  00CE3D78 = 1.5, 00CEC160 = 1.2000000476837158, all doubles). Then camera vtable +ACh(home)
  and +74h = +38Ch.
- **UpdatePose**: eye = +3BCh..+3C4h, forward = target - eye, up = (0,1,0), 0085DC80, then
  `BSP_CameraMover_ApplyShakeAndPublishPose(dt)`.
- **Target**: a kind-9 home gives *(home+538h)+808h; a parked plane (byte +5Ch clear and +900h
  == 1) asks the block's +3Ch object (vtable +40h, translation at +30h), or (0,0,0) without a
  block; a plane parented to the home (or no home) gives plane+A4h; otherwise plane+FCh in the
  home's frame (00414E10, 00414D10).

Uncertainty: what kind 9 and +900h == 1 mean; camera vtable +ACh and 00923870 were not read.

## Segment 22: in-game chat messages, the error dialog, the multiplayer pause menu (cc13-hud1)

Lease `cc13_hud1_seg22` (005CD070..005DF510). The movie screens at the start of the segment
(005CD9C0..005CE5D0) are cc9's in-game movie wiring and were not touched; the lobby block
(005D4910..005DF510, including 005D73A0 and the 3C50h-byte 005DB050) belongs with the lobby work
in docs/CC13_FRONTEND_MENUS.md and was left for that lane. Five vtable targets had no Ghidra
function and were defined: 005CE740 (chat +18h, a lone RET), 005CFE50 (chat +04h, MOV AL,1),
005D1460 (chat listener +00h), 005D3580 (pause-multi +00h, id 19h) and 005D50E0 (a 7-byte
`MOV AL,[ECX+274h]; RET` slot of a lobby vtable at 00D32DE8, left unnamed).

### The chat screen's message logs

`BSP_MultiChatScreen_AddMessage` 005D0050, `__thiscall(screen, NativeString* sender, int type,
NativeString* message, NativeString* second)`, `RET 10h`, reconstructed and build-tested
(`chat_add_message_005d0050`). Its one caller is 005D0770, which drops the message when 007786E0
on game+2080h mutes the sender's xuid (the last two of its seven stack dwords) for the local
player (game+18ECh; -1:-1 outside 0..7) and otherwise forwards (sender, type, message, second);
005D0770 is called by the session message handlers 00772020, 00772170 and 007722E0.

1. Online (game+1FE4h) with the Lua dispatcher 00F8A0C4 present, it fires the `chat` event
   00985F30(sender, type, message) first (docs/MISSION_EVENTS_UPDATE.md lists 005D0050 as that
   channel's producer).
2. The text: type 6 sets Lua globals `SwapPlayer1` = message and `SwapPlayer2` = second and
   resolves `globals.swapmessage`; 7, 8, 9, 10 set `SwapPlayer1` and resolve
   `globals.joinmessage`, `globals.leftmessage`, `ingame.multi_surrender`,
   `ingame.multi_notsurrender`; type 5 resolves the second string as a key, narrows the text to
   its low bytes into `lostunitplayername`, then resolves the message as a key. Any other type
   resolves the message as a key when the sender is empty (case-insensitive compare with
   00CE3A0C) and shows it verbatim otherwise.
3. Logs (all `std::list`, 20h/1Ch-byte nodes): the overlay +68h is front-inserted with a 6.0 s
   timer (00CE6630) after erasing its back while size > 6; the history +74h is front-inserted
   after the same trim at size > 14; the lobby log +80h is appended after erasing its front at
   size > 42. Byte +8Dh is then set.

`BSP_MultiChatScreen_OnKey` 005D1460 (listener 00CF19C0 +00h, `RET 8`): key 26h going down
outside a mission (00E198C4 == 0) opens chat mode 1; key 9 coming up opens mode 3 in a mission
and 1 otherwise (005D11F0); AL reports handled. Reconstructed (`chat_key_mode_005d1460`).

Named only: chat layout 005D14D0, exit 005CFF40, destructor body 005CFE60, mode switch 005D11F0,
row rebuild 005D0C60 (shared with the lobby update 005E5B30), log clear 005CFB80 and the
`STL_Chat*List_*` node routines; error screen enter 005D26D0, update 005D2240, message text
005D24F0, error-code setter 005D2E70 (code 5 when the online client's vtable +14h answers false;
code 1 clears +58h), dismiss 005D2010; pause-multi update 005D3F20, row builder 005D39A0,
confirmation listener 005D43C0, constructor 005D3540, destructor 005D3710.

Uncertainty: the key codes 26h/9 are probably VK_UP/VK_TAB but the listener's caller was not
read; what modes 1 and 3 select in 005D11F0; 005D07E0 (sends the typed line) was not named.

## Handoff update (cc13-hud1, second packet)

Commits after d425f766b: 8f15e962d (plane-spawn camera) and the segment 22 commit. Totals now:
190 ledger names, 32 defined functions, 45 reconstruction records, 347 verified report rows, build
passing. Segment 22 next steps, by value: the chat update 005D18A0 with the overlay timers and
005D0C60's row layout; the error dialog enter/update and the full code-to-key table in 005D24F0;
the pause-multi update 005D3F20; then the lobby block (coordinate with the lobby lane).

## Powerup manager enter, exit, update and the two list selectors (cc13-hud2)

Reconstructed and build-tested, not wired into any host, not run in `bsp_game`: enter 0061B850
(`hud_powerup_enter_0061b850`, vtable 00CF4DEC +18h, `__thiscall(screen)`, SEH 00C77F88), exit
006169F0 (`hud_powerup_exit_006169f0`, +1Ch, no SEH frame), update 0061BEA0
(`hud_powerup_update_0061bea0`, +20h, `__thiscall(screen, float dt)`, RET 4, SEH 00C7803E; the
exported pseudocode calls it `__fastcall` and drops the float), the first list's carousel 00619C50
(`hud_powerup_carousel_update_00619c50`, `__thiscall(screen+100h, float dt)`, RET 4) and the second
list's selector 0061A6C0 (`hud_powerup_update_selection_0061a6c0`, `__thiscall(screen)`, RET).
Evidence: the five listings read whole (`local/h2_m4dis.py`), constants from the PE on disk. The
cleared item scope was reconstruct-only, no wiring (cc9, 2026-10-11).

Object map. `*(00E198C4)` is the HUD manager; its +BCh is the support-manager screen (slot 4Eh,
vtable 00CF6B44, "SM" below), +C0h this screen (slot 4Fh; the routines read their own lists and
widgets through the global as often as through ECX, and the reconstruction keeps the two paths
apart), +C4h slot 50h, +C8h slot 51h, +A8h slot 49h. A list entry's definition is entry+4; the
definition has the name NativeString at +0, kind at +8 (1 = unit-class powerup), display text at
+14h, category bits at +88h (bit 0 `sipp_Icon` +C4h, 1 `szub_Icon` +C8h, 2 `plane_Icon` +CCh,
3 `command_Icon` +D0h, each shown when its bit is clear) and one 38h-byte row per team at
+0Ch + team*38h (team = player record +28h): +0 text source, +1Ch icon name, +20h unit class
(key +70h, texture +D4h).

- Enter: slot 51h 00529A30; SM +1C4h placed at screen+14h, shown and sized (0.2987, 0.1984)
  through virtual +58h; SM +12Ch/+130h/+134h hidden, +1B0h/+178h shown, +1C0h/+1C8h hidden;
  00652A60, 0065D220, the open sound; 00614320; text_Text hidden twice around +20Ch = 0 and
  +230h = 0.0; the info page at (+F4h - 0.30000001192092896, +F8h, +FCh) (FSUB of the double
  00CE3DC8, float store); +248h = right_circle_Icon's resolved position; +110h = screen+8, the
  carousel reset; 0061B0F0. A non-empty first list (+144h) sets mode +20h = 0 and the label from
  its first entry's team row; an empty one sets mode 1, clears +1ECh/+1F0h/+1F4h, takes the
  label from the second list (+184h, through HUD+C0h) and hides +48h..+54h. Both set
  +254h = width(+114h)/960 and +234h = +254h + 0.013, +238h = tile_Icon extent y. Then +1E4h = 0,
  +1E8h = 0.0, +1A4h emptied, PUMPERMANENT if PUM1STUSE was already shown else PUM1STUSE, and
  slot 51h 00528F70.
- Exit: slot 50h's +64h hidden, 0054E610, both pages' widgets restored (SM +F0h/+F4h/+1CCh and
  +12Ch/+130h/+134h shown, HUD+C0h +B8h hidden), the close sound, right_circle_Icon moved to
  empty_circle_Icon's position, the info page at +F4h minus 0.1 when SM's 0066FFA0 returns true,
  else minus 0.25; deleting destructors on +28h..+34h and +58h..+64h (pointers kept), both trees
  (+1B4h, +1CCh) erased and reset, 0065D220, slot 51h 00529060 and a tail jump to 00529A30.
- Update: returns while the active hint is blocking (00690B00 on a second copy of the name) or
  either page's byte +121h is set; tile_Icon sized (+230h*+234h, +238h); right_circle_Icon at
  (tile extent x + +248h, +24Ch, +250h); text_Text at x = circle x - +254h*0.5 (y and z kept);
  slot 50h's +64h shown with `ingame.pum_annoying_message_pc`/`_xbox` (byte 00F88A30); SM arrow
  pairs +100h/+104h and +F8h/+FCh shown only when a list has more than four entries and is not
  scrolling; PUMPERMANENT re-raised unless it is the active hint (00449AF0, case-insensitive) or
  the active hint blocks. A first-list size change resets the carousel (+204h = size,
  +208h = 1.0, +20Ch = 1); while +20Ch is set only 00611AE0 and 00617470 run. Otherwise
  00611770, 0061AFF0, 00612B70, the 118h/119h actions switch between the lists (left/right
  sound), 00619AE0 always runs, mode 0 runs 00619C50 and mode 1 0061A6C0, and the use input
  (input-owner record bytes +0Bh or +10h) calls 008EB6E0 with the selected first-list entry.
- 00619C50: while +13Fh is set only 006113A0(dt) and 00617470; otherwise 11Ah/11Bh step the
  selection (wrap only when more than four entries; the slot index 0..3 moves first, then
  006179D0 scrolls), unit-to-unit sound, the old slot widget hidden, 00618980. Then SM +178h
  text = the selected definition +14h, and the shared tail: kind 1 shows SM +F4h/+1CCh, hides
  +F0h and HUD+C0h +B8h, sets SM +158h's state 3 texture from the team row's icon name (two
  resolves through 009552E0/00443490, state 0 and 3 at alpha 1.0) and the command icon's state
  from the SM+42Ch cache; other kinds show +F0h/+B8h, `radar_sweep` shows radar_Icon, the rest
  show color_Icon and the category icons.
- 0061A6C0: the same over +184h with +1F0h (slot), +1ECh (scroll offset) and +1F4h (selection),
  blocked by +1F9h or +1E4h; its label update widens +254h only when the new text is wider.

The two carousels differ in the cache test: 00619C50 adds a state when the class key is absent
(00611C50, iterator == end), 0061A6C0 adds one when the key is present and the class texture is
non-null (the compare at 0061AC61 jumps away when the node equals the head), so the second list
re-adds a state over an existing key and never fills an empty one. Recorded as the image does it.

### Every write to the support-manager screen object

No routine of the five writes a support-manager gameplay field (purchase, spawn, stock or cursor
state). The direct stores into the SM object are the icon-state cache; everything else is a widget
call or a callee with ECX = SM. Scan: `local/h2_smscan.py` (register taint from `[00E198C4]+BCh`,
including `ADD reg, imm` field addresses), every hit checked in the listing; the helper callees
(00614320, 0061B0F0, 00617470, 006113A0, 006179D0, 00618980, 006191A0, 00619AE0, 00611AE0,
00611770, 0061AFF0, 00612B70, 00682BD0) were scanned the same way.

| Instruction | Routine | SM field | Value source |
| --- | --- | --- | --- |
| 0061A0FC / 0061A101 | 00619C50 | int map +42Ch (00614060 find, 006171B0 subscript), key `*(class+70h)` | `BSP_GuiIcon_AddState(SM+1CCh, class+D4h, {0,0,1,1})`; only when the key is absent |
| 0061ACD1 / 0061ACD6 | 0061A6C0 | the same map | the same, only when the key is present and class+D4h is non-null |
| 0061B3C2 / 0061B3C7 (callee 0061B0F0 from enter 0061BAD5) | RebuildLists | the same map, key `*(row+20h)+70h` | `BSP_GuiIcon_AddState(SM+1CCh, class+D4h, {0,0,1,1})`; only when the key is absent (0061B355) |
| 0061A172, 0061AD48 | both | the same map, read through 006171B0 | reached only when the key is present, so no node is inserted |
| 0061B8FF -> 00652A60 | enter | +7Ch..+88h, +ACh..+B8h | deleting destructor (vtable +4, arg 1) on each non-null pointer; the pointers are not cleared |
| 0061B910, 00616C3D -> 0065D220 | enter, exit | per slot i = 0..3: +7Ch, +8Ch, +9Ch, +ACh, +BCh, +CCh, +DCh, +108h, +118h (+4i) | widget pointers from 00AAB4C0 clones and 00AA7E00 lookups, then visibility (+34h) and x positions (+44h) using SM+2A4h/+2A8h |
| 00616B5A -> 0066FFA0 | exit | +47Ch, +479h | only in effective game mode 8: +47Ch = `*(00E188A8)+648h` when it changed, then 0066FB00, 00668770 when +34Ch is 0, 0065A2B0; +479h = (+34Ch != 0 or +1F4h != 0). 00668770 rebuilds the SM's +1ECh list (+1F4h = 0 first) and writes a display entity's pose (+CCh..+108h, bytes +C8h/+10Ch); those callees belong to the support-manager lane and were not enumerated further |

Widget-only calls on SM members (no SM field written): visibility (virtual +34h) on +F0h, +F4h,
+12Ch, +130h, +134h, +178h, +1B0h, +1C0h, +1C4h, +1C8h, +1CCh and, through 00652B00/30 and
00652AA0/D0, +100h/+104h and +F8h/+FCh; size (virtual +58h) and position (00AA8240) on +1C4h;
text (00ABAED0) on +178h; 00AB2690 and virtual +88h on +158h; 00AB64C0 and virtual +88h on
+1CCh; the four sounds read +1Ch/+24h/+28h/+2Ch. The pushes `push ecx` that the taint scan flags
before `fstp [esp]` at 00619FF8, 0061A01D, 0061A045, 0061A1C1, 0061ABC5, 0061ABEA, 0061AC12 and
0061AD95 reserve a float slot; the widget pointer is not an argument.

The one gameplay-effective call is not on the SM object: 0061C772 calls 008EB6E0 with
ECX = `*(00F88C30)` (the power-up manager that 008E9AF0 `BSP_PowerUpManager_CollectActive` and
008EB110 `BSP_PowerUpManager_Update` belong to) and the selected first-list entry; 008EB6E0
picks a target (008E45D0) and applies the power-up for player `*(00E188A8)+18ECh` (008EADA0).
The reconstruction exposes it as `powerup_use_008eb6e0`; nothing calls it.

Uncertainty: the double-to-float order of the x87 chains follows the listing, but the C++ uses
double intermediates where the image used 80-bit ones (each chain here is one or two operations
before a float store). The helpers 00611AE0, 00611770, 00612B70, 0061AFF0, 00619AE0, 006191A0,
006179D0, 00617470, 00682BD0 and 00614320 are host calls, not reconstructed.

## Handoff (cc13-hud2, 2026-10-11)

Branch `agent/cc13-hud2` (agent/cc13-hud1 merged first). Reconstructed and build-tested, none
wired or run in `bsp_game`: the powerup-manager enter 0061B850, exit 006169F0, update 0061BEA0,
carousel 00619C50 and second-list selector 0061A6C0 here, and the formation Update 0053A8E0 in
docs/CC13_MENU_SCREENS_2.md. 17 ledger names (11 new: 00619AE0, 0061AFF0, 00652A60, 0065D220,
00652B00/30, 00652AA0/D0, 00AA1FA0, 00533D30, 00535EE0; the rest refreshed) and 6 reconstruction
records, all applied in Ghidra (prior values in the worktree's
`local/ghidra-annotations-*.json`); 135 + 73 report rows added, `tools/verify_report_calls.py`
0 failed on both reports. No gameplay RNG is drawn.

The two conditions cc9 set are met in the doc sections: the support-manager write table (no
gameplay field written; the SM+42Ch icon cache, 00652A60's deletes, 0065D220's widget rebinds and
0066FFA0's +47Ch/+479h are the only SM stores) and the 0077D600 table (two calls, Leave and follow
on the leader, complete by byte scan). Two gameplay effects outside those objects are flagged for
whoever wires these: 008EB6E0 (power-up use, from 0061C772) and the formation record write at
0053AF37/0053AF3C with its 0077A080/0077C880 replication.

Scripts in the worktree's `local/` (`h2_` prefix): `h2_smscan.py` (taint scan of
`[00E198C4]+BCh` uses), `h2_m4dis.py` (listing; a bare address runs to the next function),
`h2_rel32.py` (all E8/E9 and absolute references), `h2_consts.py`, `h2_rows.py <report> <spec>
<callees>`, `h2_ln.py` (ledger names of a range), `h2_ledger.py` (this packet's records).

Open, by value: the helpers left as host calls (00611AE0 first-list intro, 00611770, 00612B70,
006113A0 scroll animation, 006179D0/006191A0 scroll starts, 00617470, 00619AE0, 0061AFF0,
00614320, 0061B0F0); the formation screen's 00536A90 order labels, 00535F50 drag press, 00534C00,
00533920/00533830; then the cc13-hud1 list above (segment 79 Grid containers, segment 22 chat
update).
