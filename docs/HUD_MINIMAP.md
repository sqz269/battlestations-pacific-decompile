# The HUD minimap (packet `cc_hud_minimap`)

Addresses: 005C0F20, 005BEC50, 005BE240, 005BD420, 005BE110, 005BD590, 005C0700, 00694A60,
004B4B00, 00432650, 0087D7B0, 00414DB0, 00427EB0, 00427E30, 00927880, 00B6DB70, 00BF701A,
00BF7030, 00AA7DC0, 006F1F90, 00CEDAE8, 00CE42B0, 00CF1440, 00CE3830, 00CF1438, 00F876A4.

The minimap is HUD screen slot 35h, page `GUI_minimap` (`interface/gui_minimap.lua`). Its
register virtual is 005BEC50, its layout binder 005BE240 and its update virtual 005C0F20
(`BSP_HudMinimapScreen_Update`, already named; docs/HUD_CENTRAL_UPDATES.md covers its animation
block). This packet reads the part that makes units appear: the widget slots, the world-to-minimap
transform and the per-frame icon walk. The marker half of the packet is
docs/HUD_MARKERS_RUNTIME.md.

Names here are hypotheses, not recovered symbols.

## Widgets

005BEC50 loads `GUI_minimap` and stores the page root at `this+1Ch` (00AA5840 at 005BECC7),
then both it and 005BE240 bind children through `BSP_GuiWidget_FindChildByName` (00AA7E00).

| Slot | Widget | Bound at | Page node |
| --- | --- | --- | --- |
| +1Ch | page root | 005BECCC | `GUI_minimap` |
| +48h | `minimap_dir_Icon` | 005BEB6D | `gui/minimap/minimap_direction02.tga`, pivot (0.0667, 0.5) |
| +4Ch | `minimap_compass_Icon` | 005BE2AC | `gui/minimap/minimap_compass.tga` |
| +50h | `minimap_islandmap_Icon` | 005BED46 | shader `minimap_terrain.mshd`, texture `error.tga` |
| +6Ch..+7Ch | the six groups' `item_ship_Icon` | 005BE397, 005BE406, 005BE475, 005BE4E4, 005BE553 and on | `minimap_units_white/red/blue/grey/yellow/silver_Group` |
| +F0h | `unit_marker_Group` | 005BEBDD | the per-unit icon map's parent |
| +F4h | selected capture point | 005BEC76 | cleared at register |

The string constants are `minimap_compass_Icon` at 00CF1280, `item_ship_Icon` at 00CF1254,
`minimap_dir_Icon` at 00CF1170, `minimap_islandmap_Icon` at 00CF13C8 and `unit_marker_Group` at
00CF115C. The colour names of the six groups are the same six values
`BSP_InGameHudMarkers_ResolveMarkerColourIndex` (00639990) returns for the markers screen.

## The terrain layer

`minimap_islandmap_Icon` is the map itself. Its authored texture is the placeholder `error.tga`;
the real content comes from `ShaderName = "minimap_terrain.mshd"`, whose source is
`shaderfx/gui/minimap_terrain.shfx` in the installed game. That shader is where the world-to-map
mapping for the terrain lives, and it is independent of the icon transform below:

- Samplers: `RadarMap` (register 0, clamped) and `FadeBorder` (wrapped).
- Constants: `cWorldCamPos`, `cWorldBorderNWSE`, `cBorderFlash`.
- Vertex: `OneOverWorldSize = 1/30000`; `eyeOffs = float2(cam.x, -cam.z) * OneOverWorldSize`;
  `limits = (float4(border.x, -border.w, border.z, -border.y) + 15000) * OneOverWorldSize`.
- Pixel: `finalUV = IN.UV + eyeOffs`, sampled from `RadarMap`; anything outside `limits` is
  darkened by the border tile; and `alpha = 0` wherever `dot(IN.UV - 0.5, IN.UV - 0.5) > 0.018`.

So the radar texture covers a 30000-unit world square with `uv = (world + 15000) / 30000` and the
z axis inverted, the quad's own UV is centred on 0.5, and the round mask has a radius of
`sqrt(0.018) = 0.13416` in UV, that is `0.13416 * 30000 = 4025` world units. That radius is the
same 4000-unit range the icon transform uses, which is the cross-check that the two layers agree.

`this[50h]` is also rotated with the camera every frame (005C1825), so the terrain turns under a
fixed compass rather than the icons turning over a fixed map.

## Range and the world-to-minimap transform

The two radii are global config fields, read by the config loader 0087D7B0 from
`scripts/datatables/globals.lua`:

| Field | Lua key | Installed value | Use |
| --- | --- | --- | --- |
| `[00432650() + 6Ch]` | `Globals["Minimap"]["MinimapRange"]` | 4000 | clamp radius and scale denominator |
| `[00432650() + 70h]` | `Globals["Minimap"]["VisibilityRange"]` | 4000 | cull radius |

005C15A0..005C15B3 squares both once per frame. The camera unit comes from `004B4B00` at 005C154E
and its world position from `+FCh`, `+100h`, `+104h` after a pose refresh.

The compass angle is built once at 005C17A3..005C17F7. Bit 1 of `[renderer+5Ch]` gates a refresh
through `00B6DB70`; then `heading_raw = atan2([renderer+110h], [renderer+118h])` through the CRT
helper 00BF701A, and

```
scale        = 80.0 / MinimapRange                ; 00CF1440 is 80.0, FDIVR at 005C1799
icon_heading = -heading_raw                       ; FCHS at 005C17F2, stored at [S+1Ch]
map_heading  = +heading_raw                       ; FCHS again at 005C1812, stored at [S+10h]
```

`minimap_compass_Icon` and `minimap_islandmap_Icon` get `SetRotation(map_heading)`;
`minimap_dir_Icon` gets `SetRotation(obj->vtable[C8h]() - icon_heading + pi/2)` (00CF1438).

For each icon the placement is, with `cam` the camera unit's world position and `p` the icon's
world position from `unit->vtable[C4h]`:

```
if ((p - cam) . (p - cam) > MinimapRange^2)                      ; 005C1A3C..005C1A48
    d = sqrt((p - cam) . (p - cam))                              ; 005C1A52
    p = cam + (p - cam) * MinimapRange / d                       ; 005C1A63..005C1B5A

mx = (p.x - cam.x) * scale                                       ; 005C1B62..005C1B83
my = (p.z - cam.z) * scale                                       ; 005C1B87..005C1B8D

if (icon_heading != 0)                                           ; UCOMISS at 005C1B6C, JNP 005C1B91
    mx' = mx*cos(icon_heading) + my*sin(icon_heading)             ; 005C1B93..005C1BCD
    my' = my*cos(icon_heading) - mx*sin(icon_heading)             ; 005C1BD1..005C1BFF

widget.local_position = { mx' * (1/1024), -my' / 768, depth }     ; 005C1C61..005C1C99
widget.rotation       = pi/2 - (unit_heading + icon_heading)      ; 005C1CA1..005C1CD0
```

`1/1024` is the double at 00CEDAE8, `768` the double at 00CE42B0, `pi/2` the double at 00CE3830,
and the placement call is `BSP_GuiWidget_SetLocalPositionAndBounds` (00AA7DC0). The x87 sequence
005C1A2C..005C1B8D was read instruction by instruction on both the clamped and the unclamped path;
both arrive at 005C1B62 with `cam.z` and `cam.x` on the x87 stack, which is what makes the two
`FSUBRP`/`FSUBR` at 005C1B73 and 005C1B87 read as `p - cam`.

Depth literals, chosen at 005C1C3E..005C1C59 and at the two later placement sites:

| Depth | Global | Case |
| --- | --- | --- |
| -1 | 00D7A260 | default icon |
| -4 | 00CF1430 | the controlled unit's icon |
| -2 | 00CE7D7C | a highlighted icon (same group as the target) |
| -8 | 00CE3CC8 | capture icons |
| -10 | 00CE6848 | marker icons |

The minimap is therefore **camera-relative and camera-rotated**, with a 4000-unit radius mapped to
80 units of group-local space, then divided by 1024 horizontally and 768 vertically. A unit at the
rim sits at local x = +-0.078125 or y = -+0.104167.

## The per-frame unit walk

005C154E..005C1783 is the pass that produces the dots. Gates first: `[00E188A8 + 18ECh]`, the
local team index, must be in `0..7`, and `004B4B00()` must return a camera unit.

The list is `[[[00E188A8 + 18ECh*4 + 18CCh] + 30h] + E0Ch]`, singly linked with next at `+4h` and
the unit at `[node + 8h] + 4h`. For each unit:

1. `[unit+5Ch] != 0`, `[unit+5Dh] == 0`, `[unit+60h] == 0`, `[unit+5Eh] == 0` (005C1628..005C164A,
   the same four bytes `0043F080` tests).
2. `unit != camera_unit`.
3. `unit->IsKindOf(5)` (005C165D) and `unit->vtable[B8h]()` (005C1675).
4. Unless the unit is the displayed self unit from `00927880([00E188D8])`, its squared distance to
   the camera, through `BSP_EntityPose_GetWorldPositionRefreshed` (00427EB0) and
   `BSP_Vector3f_LengthSquared` (00427E30), must not exceed `VisibilityRange^2` (005C16D3).

Survivors are looked up in the per-unit icon map at `this+F8h` through 005BE110; a miss allocates
0Ch bytes (005C171A), constructs the entry with `005BD590(entry, unit)`, inserts it with
`005C0700` and attaches it with `00694A60(unit, this+8h)`. A second distance test against
`VisibilityRange^2` at 005C19B1 hides icons that drifted out of range between the two passes.

Because the mission's destroyers answer `IsKindOf(5)` and carry live `+5Ch`, they pass every gate;
what the rebuilt executable is missing is the icon-entry allocation and the placement call, not
the selection.

## Host contract for the rebuilt executable

| # | Host method | address | native | Containing function | Owner area |
| --- | --- | --- | --- | --- | --- |
| 1 | `prepare_frame()` | 005C0F3F | 005BD420 | 005C0F20 | minimap |
| 2 | `capture_ring_fill(point)` | 005C1010 | 006F1F90 | 005C0F20 | capture points |
| 3 | `camera_unit()` | 005C154E | 004B4B00 | 005C0F20 | controlled unit |
| 4 | `global_config()` for `MinimapRange` | 005C157E | 00432650 | 005C0F20 | global config |
| 5 | `global_config()` for `VisibilityRange` | 005C158A | 00432650 | 005C0F20 | global config |
| 6 | `refresh_pose(camera_unit)` | 005C15BB | 00414DB0 | 005C0F20 | entity matrix |
| 7 | `displayed_self_unit()` | 005C15F2 | 00927880 | 005C0F20 | controlled unit |
| 8 | `world_position(unit)` | 005C1687 | 00427EB0 | 005C0F20 | entity matrix |
| 9 | `length_squared(delta)` | 005C16CE | 00427E30 | 005C0F20 | math |
| 10 | `find_icon_entry(map, unit)` | 005C170A | 005BE110 | 005C0F20 | minimap entries |
| 11 | `allocate(0Ch)` | 005C171C | 00BF681B | 005C0F20 | CRT |
| 12 | `construct_icon_entry(entry, unit)` | 005C173A | 005BD590 | 005C0F20 | minimap entries |
| 13 | `insert_icon_entry(map, key)` | 005C175B | 005C0700 | 005C0F20 | minimap entries |
| 14 | `attach_icon(unit, screen+8h)` | 005C1767 | 00694A60 | 005C0F20 | minimap entries |
| 15 | `refresh_renderer_basis()` | 005C17A7 | 00B6DB70 | 005C0F20 | renderer |
| 16 | `atan2(y, x)` | 005C17E0 | 00BF701A | 005C0F20 | CRT |
| 17 | `sqrt(d2)` for the rim clamp | 005C1A52 | 00BF7030 | 005C0F20 | CRT |
| 18 | `set_local_position(icon, xyz)` | 005C1C99 | 00AA7DC0 | 005C0F20 | GUI |

Steps 3 through 9, 17 and 18 are the minimum for moving dots; 10 through 14 are what creates an
icon widget the first time a unit is seen, and they belong to the `hud_minimap_icon_entries`
packet. Rotation of the compass, the direction wedge and the island map goes through the widget
`SetRotation` virtual `+44h` at 005C1825, 005C184F and 005C1872; those sites are indirect and are
recorded as `00AA7D00+vtable44` in the report.

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| docs/HUD_CENTRAL_UPDATES.md: "`00432650()` a world descriptor whose `+6Ch` and `+70h` are the map half-extents" | They are `Minimap.MinimapRange` and `Minimap.VisibilityRange` from the global config script, both 4000 in the shipped data. `+6Ch` is the rim-clamp radius and the scale denominator, `+70h` the cull radius. Neither is a map bound; the map bounds are the shader's `cWorldBorderNWSE`. | 0087D7B0 at `*(float*)(param_1+0x6c) = GetNumber("MinimapRange")` and `*(float*)(param_1+0x70) = GetNumber("VisibilityRange")`; `scripts/datatables/globals.lua` lines 22-26; use sites 005C15A0, 005C16D3, 005C1799, 005C19B1, 005C1A3C |
| docs/HUD_CENTRAL_UPDATES.md: "`pos.x = world_dx * 00CEDAE8; pos.y = -world_dz / 00CE42B0`" | Correct in shape but incomplete: `world_dx` and `world_dz` are first scaled by `80 / MinimapRange` and rotated by the camera heading, and the source point is rim-clamped to `MinimapRange`. | 005C1B62..005C1BFF, 005C1A3C..005C1B5A |
| docs/HUD_CENTRAL_UPDATES.md: "`00427EB0(unit)` returns an icon-space float3" | It returns `unit+FCh`, the world position, after a lazy pose refresh. | 00427EB0..00427EC8 |
| docs/HUD_CENTRAL_UPDATES.md: "the result is divided into 00CF1440 (80.0) for the compass at `this->[4Ch]`" | `80.0 / [config+6Ch]` is computed before the atan2 and is the icon scale, not a compass value. The compass at `+4Ch` receives `SetRotation(+atan2)`. | 005C1788..005C179F is the divide; 005C17E0 is the atan2; 005C1825 is the compass rotation |

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `hud_minimap_icon_entries` | 005BE110, 005BD590, 005C0700, 005BCC00, 00694A60 | docs/HUD_MINIMAP_ICON_ENTRIES.md, include/bsp/hud_minimap_entries.hpp | The 0Ch-byte per-unit icon entry, the map at `screen+F8h` and which of the six groups an entry is parented to |
| `hud_minimap_capture_points` | 005C1F90 onward, 005C239E onward, 005C2523 onward, 006F1F90 | docs/HUD_MINIMAP_CAPTURE.md | The three tail passes that place capture icons and markers at depths -8 and -10 |
| `hud_minimap_terrain_texture` | the `RadarMap` binding, `cWorldBorderNWSE`, `cBorderFlash` | docs/HUD_MINIMAP_TERRAIN.md | Which render target the radar map is, who writes `cWorldBorderNWSE`, and the mission border record it comes from |

## no_ghidra_function

none. 005C0F20, 005BE240, 005BEC50, 005BD420, 00432650, 0087D7B0, 00427EB0, 00427E30 and every
other address in this doc has a Ghidra function body; `bsp.py ghidra proto --brief` reported a
range for each, and every call site this doc cites was checked with
`python tools/verify_report_calls.py reports/hud_minimap.json`. The one hole the packet found is
on the markers side and is recorded in docs/HUD_MARKERS_RUNTIME.md.
