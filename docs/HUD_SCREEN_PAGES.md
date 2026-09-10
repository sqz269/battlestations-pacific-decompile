# HUD screen pages (packet `hud_screen_pages`)

Addresses: 00645F10, 006463E0, 006488D0, 00649860, 00646040, 00644230, 00648290, 0067B3F0,
0063E280, 00639480, 006435D0, 0051ED60, 005468B0, 005AB190, 005BEC50, 005BE240, 005BD550,
005C0F20, 00538F80, 005EE010, 005F9FF0, 00606A90, 00606C60, 00607BE0, 00519830, 0060D1C0,
0064BB90, 0064C0F0, 0064A360, 0067C430, 00650E00, 00650170, 00650B90, 0067B390, 00540FE0,
0054ED00, 005CC770, 005CCCB0, 005CCE70, 005BB130, 0054D860, 005176A0, 005211B0, 0067BE80,
00521670, 00636F30, 00602280, 00604890, 0060D740, 0060F210, 005D38A0, 00565EE0, 00643E80,
0066EA60, 0065DAA0, 00614170, 00614600, 00682740, 0052CBF0, 00CEAE54

This packet answers the open question left by `docs/IN_MISSION_INTERFACE_MANAGER.md`: the manager's
Init builds 42 HUD screens into `+40h..+E8h` and the installed `interface/gui_*.lua` pages line up
suggestively with them, but nothing read there bound a page name to a slot. **The binding is now
established for all 42 screens.** Thirty-three of them load a named page; the other nine load none.
Every page name the binary passes to the loader exists as an installed file, and every widget name
those screens look up is a key in one of that screen's own pages.

## Where the binding lives

`docs/GAME_FRONTEND_STATES.md` gives the base at 004F7180 with vtable `00CEAE54`. Reading that
table off the image shows the class has **exactly ten virtual slots**; every one of the 42 HUD
screen vtables in `docs/IN_MISSION_INTERFACE_MANAGER.md` is ten slots wide (`00CF5A9C` runs
`00CF5A9C..00CF5AC3`, and so on; where the naive walk runs longer it has walked into the next
vtable in `.rdata`).

| Slot | Base target | Role |
| --- | --- | --- |
| +00h | `__purecall` (00BF698E) | registry slot id |
| +04h | 004F7570 | no-op |
| +08h | 004F7580 | no-op |
| +0Ch | 004F75E0 | scalar deleting destructor in every leaf |
| +10h | 004F71D0 `BSP_FrontEndScreen_Register` | **register** |
| +14h | 004F7590 | **load layout / bind widgets** |
| +18h | 004F75A0 | enter |
| +1Ch | 004F75B0 | exit |
| +20h | 004F75C0 | update(float) |
| +24h | 004F75D0 | no-op |

Init calls `+10h` on each screen. **All 42 override `+10h`**, and each override calls or tail-jumps
to `BSP_FrontEndScreen_Register` (004F71D0) itself, so registration still happens exactly as the
manager doc describes. The page load sits either in that override or in the `+14h` virtual the
override calls. `+14h` is a no-op in the base and is overridden by fifteen screens, seven of which load a page.

The load is always the same shape, `0063E280` (slot 4Dh) being the shortest instance:

```
BSP_FrontEndScreen_Register();                       // 0063E28C
BSP_NativeString_Resize(0xB, 1);                     // length of "GUI_markers"
_memcpy(buffer, "GUI_markers", 0xC);                 // literal at 00CF58F0
BSP_GuiManager_GetOrCreate();                        // 004C12B0
screen->[8Ch] = BSP_GuiManager_LoadPage(&name, 1, 0);// 00AA5840, RET 0Ch
```

Named widgets are then fetched from the loaded page with the child lookup `00AA7E00`, exactly as
`docs/PRESS_START_SCREEN.md` and `docs/MAIN_MENU_SCREEN_UPDATE.md` describe for the front end.

**How the map was recovered.** The page and widget names are `push imm32` operands consumed by a
`memcpy` into a `NativeString`, so they are not string references Ghidra records. Every `push`
of a pointer to a printable C string in `.text` was decoded with a linear Capstone sweep, attributed
to the enclosing function by its Ghidra body range, and then classified by the first `call` to
`00AA5840` (page) or `00AA7E00` (widget) within the next 45 instructions. The classification is a
heuristic; its only residue is eight literals that are localisation keys or Lua datatable fields
(`ingame.*`, `globals.*`, `TempWaitBase`), which the report carries in a separate `text_keys` list.

## The slot-to-page-to-interface map

`Raised by` is the set of interface ids whose arm in `ApplyPendingInterface` (0068ACA0) puts the
slot in the level-1 screen list, taken from the id map in `docs/IN_MISSION_INTERFACE_MANAGER.md`.
`*` marks a routine Ghidra has no function for; the report lists all of those with end addresses.
`(n)` after a page name is the number of `Name_Class` keys the installed `.lua` defines.

| Offset | Slot | Register +10h | Layout +14h | Pages loaded (keys in the installed file) | Widgets bound | Raised by interface ids |
| --- | --- | --- | --- | --- | --- | --- |
| +40h | 44h | 00645F10 | 006463E0 | `GUI_powerups` (2); `GUI_unit` (8); `GUI_selector` (9) | 14 | 20h, 22h, 23h, 24h, 25h, 27h, 28h, 2Fh, 33h |
| +44h | 27h | 0067B3F0* | -- | -- | 0 | 22h, 23h, 24h, 25h, 26h, 27h, 28h, 2Fh, 33h |
| +48h | 4Dh | 0063E280 | -- | `GUI_markers` (23) | 0 | 22h, 23h, 24h, 25h, 26h, 27h, 28h, 2Fh, 33h |
| +4Ch | 26h | 0051ED60 | -- | `GUI_binoculars` (1) | 1 | 23h, 25h, 27h, 2Fh |
| +50h | 2Eh | 005468B0 | 00546A20 | `GUI_cross_gunstate` (3); `GUI_cross_ship` (32) | 16 | 23h, 25h, 27h, 28h, 2Fh |
| +54h | 4Ch | 005AB190 | -- | `GUI_map` (28); `GUI_classicons` (1) | 24 | -- |
| +58h | 35h | 005BEC50 | 005BE240 | `GUI_minimap` (33) | 27 | 20h, 22h, 23h, 24h, 25h, 27h, 28h, 2Fh, 33h |
| +5Ch | 2Ah | 00538F80 | -- | `GUI_formation` (1); `GUI_classicons` (1); `GUI_unit` (8); `GUI_formation_circle` (4); `GUI_formationOvrly` (15) | 26 | -- |
| +60h | 39h | 005EE010 | -- | `GUI_objectives` (12) | 11 | -- |
| +64h | 3Bh | 005F9FF0 | 005FA0F0 | `GUI_order` (21) | 20 | -- |
| +68h | 3Eh | 00606A90 | 00606C60 | `GUI_cross_all` (34); `GUI_plane_effects` (8); `GUI_plane` (21) | 35 | 22h, 23h |
| +6Ch | 3Fh | 00607BE0* | -- | -- | 0 | 22h |
| +70h | 25h | 00519830 | -- | `GUI_bomber` (1) | 1 | 23h |
| +74h | 41h | 0060D1C0 | -- | `GUI_plane_spawn` (2) | 0 | 24h |
| +78h | 45h | 0064BB90 | 0064C0F0 | `GUI_ship` (13); `GUI_repair` (15); `GUI_ship_effects` (24); `GUI_ship_damage` (17) | 40 | 25h, 27h, 28h |
| +7Ch | 46h | 0064A360 | -- | -- | 0 | 25h |
| +80h | 4Ah | 0067C430 | -- | `GUI_binoculars` (1) | 0 | 27h |
| +84h | 47h | 00650E00 | 00651230 | `GUI_periscope` (2) | 2 | 28h |
| +88h | 48h | 00650170 | 00650B90 | `GUI_sub` (6) | 4 | 28h |
| +8Ch | 24h | 0067B390 | -- | -- | 0 | 26h |
| +90h | 2Bh | 00540FE0 | -- | `GUI_freecam` (1) | 1 | 29h |
| +94h | 2Ch | 0054ED00* | -- | -- | 0 | 2Ah |
| +98h | 36h | 005CC770 | -- | `GUI_moviecamera_debug` (4) | 4 | 2Bh |
| +9Ch | 37h | 005CCCB0 | -- | `GUI_movie` (2) | 2 | 2Ch |
| +A0h | 38h | 005CCE70 | -- | `GUI_movie` (2) | 2 | 2Dh |
| +A4h | 33h | 005BB130 | -- | `GUI_blackout` (13); `GUI_subtitle` (4); `GUI_narrative` (3) | 13 | -- |
| +A8h | 34h | 0054D860 | 0054D980 | `GUI_hints` (11) | 11 | -- |
| +ACh | 23h | 005176A0 | -- | -- | 0 | -- |
| +B0h | 2Fh | 005211B0 | -- | -- | 0 | 2Fh |
| +B4h | 49h | 0067BE80 | 0067BEA0* | -- | 0 | 20h, 22h, 23h, 25h, 27h, 28h, 2Ah, 2Fh, 33h |
| +CCh | 29h | 00521670 | -- | -- | 0 | 20h, 22h, 23h, 25h, 27h, 28h, 2Ah, 2Fh, 33h |
| +D4h | 5Ah | 00636F30 | -- | `FE_sceneinit` (13) | 6 | -- |
| +D8h | 3Ch | 00602280 | -- | `GUI_pause_single` (4); `GUI_pause_tip` (30); `GUI_pause_objectives` (6) | 30 | -- |
| +DCh | 3Dh | 00604890 | -- | `GUI_pause_cheat` (2) | 0 | -- |
| +E0h | 5Eh | 0060D740 | 0060F210 | `GUI_scoring` (20) | 25 | -- |
| +E4h | 19h | 005D38A0 | -- | `GUI_pause_multi` (4) | 3 | -- |
| +E8h | 32h | 00565EE0 | -- | `GUI_limbo` (2) | 2 | 34h |
| +B8h | 43h | 00643E80 | -- | `GUI_spectator` (2) | 2 | -- |
| +BCh | 4Eh | 0066EA60 | 0065DAA0 | `GUI_support_full` (50); `GUI_support_infopanel` (7) | 28 | -- |
| +C0h | 4Fh | 00614170 | 00614600 | `GUI_support_full` (50); `GUI_powerups_info` (7) | 16 | -- |
| +C4h | 50h | 00682740 | 006823C0 | `GUI_Warning` (12) | 7 | 22h, 23h, 24h, 25h, 27h, 28h, 2Fh |
| +C8h | 51h | 0052CBF0 | 00528C60* | `GUI_counters` (12) | 0 | -- |

The full per-widget lists, with the virtual each name is looked up in, are in
`reports/hud_screen_pages.json`.

### Validation against the installed pages

`interface/` on this installation holds 48 `gui_*.lua` files plus the `fe_*` set. The 42 screens
load **47 distinct pages**; every one of them exists as `interface/<name>.lua` (matched
case-insensitively: the binary writes `GUI_markers`, the file is `gui_markers.lua`). Two installed
pages, `gui_pause.lua` and `gui_pause_title.lua`, are not loaded by any of the 42 and must be
reached from elsewhere. Every one of the 373 widget names the screens look up is a key defined in
one of that same screen's pages, with one wording exception noted under Uncertainties.

### The nine screens that load no page

`27h`, `3Fh`, `46h`, `24h`, `2Ch`, `23h`, `2Fh`, `49h`, `29h`. Their register overrides were read
in full and none reaches `00AA5840`; a breadth-first walk of every vtable slot and of one level of
callees finds no page load either. Three of them are register-only stubs: `0067B3F0` (slot 27h) is
a bare `jmp 004F71D0`, `0067B390` (slot 24h) is a five-byte thunk that calls it, and `0064A360`
(slot 46h) registers and clears `this+20h`. Slot 3Fh is the plane effects screen: its register
`00607BE0` releases two refcounted objects at `+94h`/`+98h`, resets `+CCh`, and builds three
particle or sound resources named `Planewindsmoke` (00CF4838), `warning heartbeat only` (00CF4820)
and `Turbo_Effect` (00CF4810) through `00871BA0`, not through the GUI loader. Its own page,
`GUI_plane_effects`, is loaded by the neighbouring slot 3Eh instead. So the nine are logic and
effect screens that borrow other screens' pages or draw nothing.

## The three central screens

Chosen by how many interface ids raise them: slot 44h is the HUD root the manager also notifies for
every id, slot 4Dh ties for the most arms at nine, and slot 35h is the in-mission map at eight
arms plus the null-payload 20h set.

### The controlled unit

`00644230` is the whole accessor: `mov eax, [00E188D8]; ret`, `__cdecl` with no arguments (the
manager calls it as `[this+40h]->00644230()` but the body ignores ECX), body 00644230..00644235.
The only writer of `00E188D8` is `BSP_Game_SetControlledUnit` (004C0880..); the twenty-odd other
references, including five inside the markers update, are reads. So HUD screens do not carry a
unit pointer: they read the global each time they need it.

### Slot 44h, the HUD root

`00646040 BSP_InGameHudRootScreen_SetInterface`, body 00646040..00646082, is the per-id hook the
manager calls in step 3 of `ApplyPendingInterface` for every interface change.

**Enter, `006488D0`, `__thiscall(this)`, RET, body 006488D0..00648923. Ghidra has no function
here.** Six steps, all read off the listing:

1. `this->[3Ch]->vtable[4Ch](0.0f)` -- the float is pushed with `fldz` / `fstp [esp]`, so the
   pseudocode would show it as a register input.
2. `this->[5Ch]->vtable[4Ch](0.0f)`
3. `this->[60h]->vtable[4Ch](0.0f)`
4. `this->[24h] = BSP_Game_GetEffectiveGameMode(00E188A8)` (004BCA50), the same accessor
   `docs/GAME_ON_MOVE_MAP.md` and `docs/MISSION_STATE_ENTRY.md` use.
5. `00648290(this)`.
6. `this->[F4h] = 0`, the update cadence counter.

`00648290` rebuilds the screen's two unit vectors: it clears `+8Ch..+94h` and `+9Ch..+A4h`, walks
the world unit list at `[00E188A8 + 1974h]` (singly linked, next at `+4h`, payload at `+8h`),
appends each unit that passes `00645060` and `007788B0`, then handles `00E188D8` separately using
the current index at `[00E188A8 + 18ECh]`.

**Update, `00649860`, `__thiscall(this, float)`, body 00649860..0064A24C.** Gated three ways at the
top: it runs only when `[00E188A8 + 61Fh]` and `[00E188A8 + 620h]` are both zero, and only on every
second call, because `this->[F4h]` is decremented and, when it drops below 1, reset to 2. On a
running frame it destroys and clears both widget vectors (`+108h..+10Ch` and `+118h..+11Ch`), then
for each entry of the unit list at `+168h..+16Ch` it tests `008E62A0(00E188D8)` and, when that
passes, clones a widget subtree (`BSP_GuiWidget_CloneSubtree`), positions it with
`BSP_GuiWidget_GetSize` plus `BSP_GuiWidget_SetResolvedPosition`, and copies a per-team record from
`[[00E188A8 + 18CCh + [00E188A8 + 18ECh]*4] + 28h]` scaled by 38h through `00AB64C0`. The widgets
it clones are the `GUI_powerups` and `GUI_unit` templates its layout virtual bound:
`puptemplate_Icon`, `circletemplate_Section`, `unit_name_Text`, `FlagJP_Icon`, `FlagUS_Icon`,
`unit_payload_Icon`, `unit_HP_damage_Icon`, `unit_HP_healthy_Icon`, `unit_command_Icon`,
`Units_Group`, `ClosedUnitHUD_Group`, `Medal_Icon`, `Medal_Text`, `WeaponInfo_Text`.

This routine is **analyzed, not reconstructed**. Its pseudocode carries register inputs
(`unaff_EBX` used as both an index and a float, and the loop counter aliased to a float), so the
per-widget field assignments would need the listing read end to end before any of it could be
written as C++. Only the gate, the cadence and the clone-and-place structure are established here.

### Slot 4Dh, the markers screen

**Register, `0063E280`, `__thiscall(this)`, RET, body 0063E280..0063E358.** Loads `GUI_markers`
into `this->[8Ch]` and clears `+20h`, `+24h`, `+28h`, `+2Ch`, `+7Ch`, `+80h`, `+84h`, then stores
`_DAT_00CF58EC` at `+30h`. It binds no named widget: the marker icons are cloned from the page at
run time instead.

**Enter, `00639480`, `__thiscall(this)`, RET, body 00639480..006394A2. Ghidra has no function
here.** Three constant stores and nothing else: `this->[88h] = 0.023f` (00CEB698) and
`this->[44h] = this->[48h] = 0.5f` (00CE3800).

**Update, `006435D0`, `__thiscall(this, float)`, body 006435D0..00643D94 (1989 bytes), `ret 4`. Ghidra
has no function here.** It reads the controlled unit `00E188D8` five times, the game `00E188A8` nine
times and the interface manager `00E198C4` seven times, and drives the marker pool through
`006434E0`, `006430C0` and `00640620`. Analyzed only; the per-marker field reads were not resolved.

### Slot 35h, the minimap

**Register, `005BEC50`, `__thiscall(this)`, body 005BEC50..005BF7E5.** Loads `GUI_minimap` and
binds `minimap_islandmap_Icon`, `capture_icon_Group`, `icon_big_item_Icon`,
`icon_big_item_hit_Icon`, `icon_big_Icon`, `bluecapture_Section`, `redcapture_Section` and
`capturebg_Section`; it also passes `cWorldCamPos` and the four `_ICS`/`_ICM`/`_ICL`/`_ICH` suffixes
to non-GUI calls, and calls `BSP_Game_GetEffectiveGameMode`.

**Layout, `005BE240`, body 005BE240..005BEC18.** Binds the per-team unit icon groups:
`minimap_compass_Icon`, `minimap_dir_Icon`, `unit_marker_Group`, and for each of
`minimap_units_white_Group`, `_red_`, `_blue_`, `_grey_`, `_yellow_` and `_silver_` the child
`item_ship_Icon`, plus `item_plane_Icon`, `item_sub_Icon`, `item_building_Icon` and
`item_marker_Icon` in the white group.

**Enter, `005BD550`, `__thiscall(this)`, RET, body 005BD550..005BD582.** Four calls:
`this->vtable[14h]()`, which re-runs the whole layout bind, then `this->[48h]->vtable[34h](1)`,
`this->[F0h]->vtable[34h](1)` and `this->[1Ch]->vtable[34h](1)`, the widget show calls.

**Update, `005C0F20`, `__thiscall(this, float)`, body 005C0F20..005C27DB (6332 bytes), `ret 4`. Ghidra
has no function here.** It does not touch `00E188D8`; it reads the game at `00E188A8` eleven times and
the interface manager at `00E198C4` eleven times, refreshes three transforms through
`BSP_Transform_RefreshWorldMatrix` and places icons with three calls to
`BSP_GuiWidget_SetLocalPositionAndBounds`. Analyzed only.

## Calling conventions and RET sizes

Every routine established here is `__thiscall` with ECX = screen and no stack arguments, ending in
a plain `ret`, except:

| Routine | Convention | RET |
| --- | --- | --- |
| register `+10h`, layout `+14h`, enter `+18h`, exit `+1Ch` (all screens) | `__thiscall(this)` | `ret` |
| update `+20h` | `__thiscall(this, float)` | `ret 4` at 0064A24C, 00643D94 and 005C27DB, the three read here |
| id `+00h` (all screens) | `__thiscall(this)` | `ret`, value in EAX |
| `00644230` | reads `[00E188D8]`, ignores ECX | `ret` |
| `00AA5840` `BSP_GuiManager_LoadPage` | `__thiscall(manager, name, flag, addref)` | `ret 0Ch` |
| `0067B3F0` | tail `jmp 004F71D0`, so it inherits the base's convention | -- |

## Callers and callees

- Every register `+10h` here is called from `0068CC70` only, through the vtable, once per mission.
- The layout `+14h` virtuals are called from their own register override and, for slot 35h, again
  from the enter virtual.
- `00AA5840` has 67 callers; 27 of the 42 register overrides and 7 of the layout overrides are
  among them. The rest of that caller list is the front end (`docs/GUI_LAYOUT_LOADER.md`).
- `00644230` is called by `0068ACA0` twice, in the bomb-view arm.
- `004BCA50 BSP_Game_GetEffectiveGameMode` is reached from the 44h enter and the 35h register.

## Uncertainties

- The classification of a literal as a page name or a widget name is the 45-instruction window
  heuristic described above, not a dataflow proof. It is corroborated for every name by the
  installed `.lua` files, which is why the residue is only the eight localisation keys.
- Slot 3Ch looks up `info_text_group` (00CF3F44) while `gui_pause_tip.lua` defines
  `info_text_Group`. Either the child lookup `00AA7E00` folds case or that one binding fails at run
  time. Nothing read here settles it; the lookup itself is outside this packet's lease.
- Slot 4Eh and slot 4Fh both load `GUI_support_full`. Whether they share one page instance depends
  on the name cache in `00AA3140`, which `docs/GUI_LAYOUT_LOADER.md` describes but this packet did
  not re-derive.
- `GUI_powerups`, `GUI_unit` and `GUI_selector` are loaded by slot 44h and `GUI_unit` again by slot
  2Ah, so the two screens bind the same widget names on what may be the same tree.
- `FE_sceneinit` is the only non-`GUI_` page in the set; slot 5Ah is the in-mission loading screen
  and reuses the front-end page family.
- The `+04h`, `+08h` and `+24h` virtuals were not analyzed beyond noting which screens override
  them.
- The update virtuals of the three central screens are analyzed, not reconstructed, for the reasons
  given above.

## Ghidra state

152 of the 348 vtable targets across the 42 screens have no Ghidra function; they are listed with
start and end addresses in `no_ghidra_function` in `reports/hud_screen_pages.json`. Most are
five-byte id getters and one-byte `ret` stubs, but four matter here: `006488D0` (slot 44h enter,
84 bytes), `00639480` (slot 4Dh enter, 35 bytes), `006435D0` (slot 4Dh update, 1989 bytes) and
`005C0F20` (slot 35h update, 6332 bytes). Ends were taken by linear sweep to the terminating `ret`
before `int3` padding. No `bsp.py ghidra flow` gap was found in any function this packet read.

## What remains

- The update virtuals of 44h, 4Dh and 35h, end to end from the listing.
- The `GUI_ship` family (slot 45h, 40 widgets over four pages) is the richest unit-status screen and
  was mapped but not read.
- `gui_pause.lua` and `gui_pause_title.lua` are installed and unclaimed by these 42 screens.

### Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `hud_root_update` | 00649860, 00648290, 00645060, 007788B0, 008E62A0, 00647C20 | docs/HUD_ROOT_UPDATE.md, reports/hud_root_update.json, include/bsp/hud_root_update.hpp, src/hud_root_update.cpp | Read 00649860 from the listing and recover the per-unit widget clone and placement, including the `[00E188A8 + 18CCh]` team record |
| `hud_ship_status_screen` | 0064BB90, 0064C0F0, 0064B370, 0064DD30, 0064D8D0 | docs/HUD_SHIP_STATUS_SCREEN.md, reports/hud_ship_status_screen.json, include/bsp/hud_ship_status.hpp, src/hud_ship_status.cpp | The slot 45h speed, recon, torpedo and repair readouts over `GUI_ship`, `GUI_repair`, `GUI_ship_damage` |
| `hud_marker_pool` | 006435D0, 006434E0, 006430C0, 00640620, 0063B5E0, 00642C20 | docs/HUD_MARKER_POOL.md, reports/hud_marker_pool.json, include/bsp/hud_marker_pool.hpp, src/hud_marker_pool.cpp | The slot 4Dh marker pool: which units get a marker and how each icon is placed |
| `hud_minimap_update` | 005C0F20, 005C0930, 005C0450, 005C0BD0, 005BCF70, 005BD420 | docs/HUD_MINIMAP_UPDATE.md, reports/hud_minimap_update.json, include/bsp/hud_minimap.hpp, src/hud_minimap.cpp | The slot 35h per-frame icon placement and the six team groups |

## Names recorded

46 reviewed names went into `config/names/`, one per routine whose behaviour is established here:
the register, layout, enter and update virtuals of the three central screens, the accessor
00644230, and the register or layout virtual of every other screen the page it loads identifies.
Two are provisional because no page, string or widget names the screen: `BSP_HudScreenSlot46_Register`
(0064A360) and `BSP_HudCommonOverlayScreen_Register` (0067B3F0). Ghidra was read-only for this
packet, so none of the names is applied to the program yet.

## State reached

| Routine | State |
| --- | --- |
| The 42 register `+10h` virtuals | analyzed; page and widget names installed-file-checked |
| The 14 layout `+14h` virtuals | analyzed; page and widget names installed-file-checked |
| 0063E280, 0064A360, 0067B390, 0067B3F0 | analyzed in full |
| 00607BE0 | analyzed in full |
| 006488D0, 00639480, 005BD550 (the three enter virtuals) | reconstructed and build-tested |
| 00644230 | reconstructed and build-tested |
| 00649860, 006435D0, 005C0F20 (the three update virtuals) | analyzed only |
| 00648290 | analyzed |
| The slot-to-page-to-interface map | reconstructed, build-tested, installed-file-checked |
| 00646040 | referenced only; named before this packet |
