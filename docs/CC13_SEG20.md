# Segment 20: main menu, HUD map, narrative and minimap leftovers (cc13-seg1b)

Segment 20 is 00581CE0-005C5470. Most of its game functions already had reconstructions (main
menu pages, voice manager, mission tree, minimap update). This packet takes the unrecorded functions
that could be reconstructed faithfully in one pass. Code: `include/bsp/frontend_seg20_cc13.hpp`,
`src/frontend_seg20_cc13.cpp`. Status: **reconstructed and Win32 build-tested only**. No fixture,
nothing run in game, not ABI-compatible. Names are hypotheses (the ledger's Ghidra names). No
agent/argon* branch touches this range (checked against origin/main on 2026-10-13).

Objects are raw image storage, read and written at the image's offsets (`Raw::at`). Every engine
routine the image calls is a named `Seg20Host` method invoked at the same point. Where another file
already reconstructs a callee, the code calls it: 0059D410 and 0059A910
(`cc13_fe1::hud_map_is_point_inside_map_area_0059d410`, `hud_map_classify_offscreen_edge_0059a910`)
and 005BE110 (`cc13_fe1::hud_minimap_icon_map_find_005be110`). The checked-index handler 00BF6713 is
a host call wherever it can fire on valid data (an empty vector). Checked-iterator traps that only
fire on a corrupt container are not modelled; each function's ledger record says which.

## Reconstructed

| address | C++ | notes |
|---|---|---|
| `00581CE0` | `main_menu_layout_mission_row_00581ce0` | map anchor +184h = point[[00E194DC]] * zoom, z = 0, plus +1A4h + +190h |
| `00588280` | `main_menu_listener_current_1c_00588280` | captures +E4h / +518h, empties the five point vectors, 006833F0 twice |
| `00590D60` | `main_menu_screen_scalar_deleting_dtor_00590d60` | 00590600, free when bit 0 |
| `00592640` | `main_menu_on_play_mission_prompt_result_00592640` | result 1: 007FC490 record, +6ACh, class index maps, 005922F0; else 0058C010 |
| `005B5C10` | `narrative_set_portrait_005b5c10` | [+40h] vtable[88h](texture, 0, 1.0f) |
| `005B5D20` | `narrative_query_text_005b5d20` | [+B0h] vtable[54h] into a dropped local |
| `005B7390` | `narrative_show_text_005b7390` | text, position from the text height, three outputs from [[00E198C4]+A4h] |
| `005A5920` | `hud_map_icon_observer_on_released_005a5920` | delete the icon, erase the node, +534h = 1 |
| `005BFDF0` | `minimap_icon_observer_on_released_005bfdf0` | the minimap counterpart (node+10h not cleared, no flag) |
| `0059CEF0` | `hud_map_icon_update_size_and_visibility_0059cef0` | interpolated icon position, size and alpha |
| `0059D150` | `hud_map_icon_query_unit_status_0059d150` | four status bytes from the unit |
| `0059D350` | `hud_map_label_center_on_widget_0059d350` | centres the label text on its frame |
| `0059D6D0` | `hud_map_icon_update_offscreen_edge_0059d6d0` | on-screen test, else the off-screen edge code |
| `0059D940` | `hud_map_is_unit_shown_at_zoom_0059d940` | unit kind against the zoom level +2Ch |
| `005A07B0` | `generated_model_find_material_by_effect_name_005a07b0` | first material whose effect name contains the string |
| `005A1220` | `hud_map_find_icon_carrying_unit_005a1220` | icon whose shipyard/airfield hangar object matches |
| `0059EAD0` | `hud_map_icon_array_resize_0059ead0` | reserve (0059E690) and set the count when it changes |
| `005BCAC0` | `minimap_marker_construct_005bcac0` | clone the template for the unit kind, add it, show it |
| `005C0930` | `minimap_acquire_marker_005c0930` | per-kind pooled marker, new one pushed when the pool runs out |

### Notes on the arithmetic

The x87 sequences were read from the bytes where the listing can mislead. In 0059CEF0, `DE EA` is
FSUBP ST2,ST0 and `DE E1` is FSUBRP ST1,ST0, so t = (g - b) / (a - b). Every intermediate the image
stores to a float is rounded to float in the C++. Operations on two floats are done in double and
rounded once, which matches the x87 sequence. The constants are 960.0 (00CEC380), 720.0 (00CEF1B8),
52.0 (00CEFFC0), 60.0 (00CE3D68), 10.0 (00CE3DC0), 0.5 (00D7A280), 0.13333 (00CF0DF0),
0.011111 (00CEED60), and 0.9f stored as a double (00D7A390). The float at 00F876A4 is BSS and is
read through the host at run time.

0059CEF0 truncates the interpolated position with CVTTSS2SI, then keeps the even part
(CDQ / SUB / SAR 1 / ADD, that is `(n / 2) * 2`).

## Skipped

- **x87-heavy, left for an x87-capable pass**: 0059DD20 BSP_HudMapArrowWidget_SetLine (FST double
  temporaries across a long chain), 0059F7C0 BSP_HudMapScreen_WorldToClampedMapPoint (clamps,
  00419260, 00BF9940 and an FIMUL sign), 00581DF0 BSP_MainMenuScreen_ScaleWidget (a debug outline
  drawn by four 00554600 BSP_GuiRect_DrawOutline calls whose stack arguments the decompile garbles;
  no callers).
- **0059D4A0 BSP_HudMapIcon_PlaceWidget**: when icon+11Ch is neither 0 nor 1 it passes
  uninitialised stack floats to 00AA8240. No faithful C++ exists for that path without inventing
  values, so it is skipped.
- **STL instantiations**: 0058AAC0, 0058B870.
- **Not reached this packet** (remaining segment 20 work, unrecorded, no reconstruction found in
  main): 00583E50, 005845D0, 00585C50, 00588A80, 0058CE30, 0058EE40, 005902E0, 00593CA0, 00597870,
  005987F0, 0059DA90, 0059E120, 0059F460, 005A1B40, 005A1C60, 005A2340, 005A2650, 005A2730,
  005A2C30, 005A3210, 005A3610, 005A3830, 005A3BF0, 005A3EF0, 005A4020, 005A4200, 005A48C0,
  005A6AD0, 005A7550, 005A7C20, 005A8420, 005A8600, 005A8F50, 005A91E0, 005A9770, 005A9DB0,
  005AAC00, 005AAEF0, 005ADDA0, 005B25D0, 005B2D70, 005B3470, 005B45C0, 005B83E0, 005B8810,
  005BE240, 005C0BD0, 005C4080, 005C4D40, 005C51F0. Several of these (00583E50, 005A1C60, 005A6AD0,
  005ADDA0, 005B45C0, 005BE240, 005C0BD0) are called through host methods in other headers; those
  are call sites, not reconstructions.
