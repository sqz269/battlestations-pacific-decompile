# In-game GUI: segments 26 and 27 (packet `cc13_ingame_gui`)

Addresses: 0065F170, 0065F2C0, 0065F410, 0065F6B0, 0065F800, 0065F950, 0065FAA0, 0063E5C0,
0065EA30, 00629D10, 0061FBE0, 0062C2C0, 00621720, 0062B640, 0063E360, 00643F40, 00646C50,
00648000, 0064BB90, 0064DA00, 00650170, 00651230, 00650750, 0065DAA0, 00674B80, 00673100,
0067B110, 0067B1D0, 0067B0B0, 0067BCB0, 0067BD70, 0067BE80, 0067F080, 0067F4D0, 0067DED0.
Files: `include/bsp/ingame_gui.hpp`, `src/ingame_gui.cpp`, `reports/cc13_ingame_gui.json`
(10 call rows, 0 failed).

Segments 26 (0061D5C0..00654A70) and 27 (00654A90..00680D30) hold the post-mission rewards
screen, the HUD marker, spectator, root, ship-status, submarine and periscope screens, the
support-manager (strategic purchase) screen, the tactical sheet, cursor and unit-library screens,
and air-operations code. Status words as in docs/CC13_FRONTEND_MENUS.md; nothing here was run in
`bsp_game`.

## Skipped

A raw function was skipped when its body contains an address cited by a doc edited since
2026-10-04 by another lineage, or by docs/AIROPS_LAUNCH_TICK.md, docs/AIROPS_LOAD_FROM_SCENE.md,
docs/SHIP_AI_OPEN_ITEMS.md, docs/SCRIPTED_HELM.md, docs/GUNNERY_OPEN_ITEMS.md or
docs/SQUADRON_LAND_TASK.md (`local/f1_excl.py`): 14 in segment 26, 12 in segment 27. That covers
cc12's observer-cleanup and GlobalConfig audits (00653390, 00642C20, ...), cc9's air-ops and
ship-AI addresses (00656280, 00656570, 00656670, 00658A60, 0065F560, 0066DD00..0066EC1B,
00673A10, 006743E0.., 00675C40 onward), the scripted-helm range (00645060, 00645600, 00649860,
0064B870..), the press-start/profile range 0067C840..0067D860, and 00650830
(docs/CC13_CAMERA_CONTROLLERS.md). 0065F560 is listed in the sound table below, as the eighth
player, but not renamed.

## Support-manager GUI sounds (reconstructed)

The support-manager screen (HUD slot 4Eh, vtable 00CF6B44, register 0066EA60) acquires eight
samples (00A83FD0) into +10h..+2Ch at 0066EC7B..0066F27F. Eight 336-byte routines each play one of
them; they are identical but for three addresses, so `play_support_manager_sound` is one body over
`kSupportManagerSounds`:

| Player | Field | Sample | Guard / class cache |
|---|---|---|---|
| 0065F170 | +10h | `environment/environment_unavailable.fsb` | 00E1983C / 00E19838 |
| 0065F2C0 | +14h | `environment/sm_sp_to_unit.fsb` | 00E19844 / 00E19840 |
| 0065F410 | +18h | `environment/sm_change_weap.fsb` | 00E1984C / 00E19848 |
| 0065F6B0 | +1Ch | `environment/sm_unit_to_unit.fsb` | 00E1985C / 00E19858 |
| 0065F560 | +20h | `environment/sm_launch_unit.fsb` | 00E19854 / 00E19850 (not renamed) |
| 0065F800 | +24h | `environment/sm_left_right.fsb` | 00E19864 / 00E19860 |
| 0065F950 | +28h | `environment/sm_open.fsb` | 00E1986C / 00E19868 |
| 0065FAA0 | +2Ch | `environment/sm_close.fsb` | 00E19874 / 00E19870 |

Each: `__fastcall`(ECX = the screen), RET. On first use the class `InGameGUI` is cached (00A7ACF0)
behind bit 0 of its guard; then type `Normal` (00A7B0A0, EDX:EAX), a tracked sound on *00F8BBD8
(00A7E490, flag 1) from the sample, and the returned handle released at once. The screen-side
callers are spread over the support manager (00662020, 00662210, 00673A10, 00674B80, 00675C40)
and five routines below this range (006169F0..0061BEA0). Coverage: complete; build-tested.

## Marker distance text (0063E5C0, reconstructed, partial)

`BSP_HudMarker_FormatDistance`, stdcall(NativeString* out, entity* target), RET 8; callers 00642040
(`BSP_InGameHudMarkers_BuildUnitMarker`) and 00641910. `out` is cleared first. With a controlled
unit (00E188D8) and a target, both poses are refreshed and the distance is 0042B2F0 of the
position delta. The text stays empty when the distance compares equal to 0.0f (UCOMISS, 0063E6A0).
Otherwise:

- tenths = float(distance / 100.0) (00D7A220), or / 185.2 (00CF0B08) when 00F88988 is set: the
  world unit is a metre and the display is kilometres or nautical miles with one decimal;
- whole = CVTTSS2SI(tenths); tens = whole / 10, digit = whole % 10;
- if tenths - whole > 0.5 (x87, 00D7A280) the digit goes up by one, and a digit of 10 carries;
- text = tens + "." + digit + " |" + `globals.kilometer` or `globals.mile`.

`format_marker_distance_0063e5c0` takes the distance; the pose refreshes and the length call stay
with the caller (coverage: partial). A NaN distance would pass the equality test natively and is
not modelled.

## Device-class icon index (0065EA30, reconstructed)

stdcall(const char*), RET 4; no reference in the image. Case-insensitive table: 100 lbs 0, 200 lbs
1, 450 lbs 2, US/JP depth charge 3, 5-inch HVAR 4, US/UK/JP aerial torpedo 5, US/JP paratroopers 6,
kamikaze 9, 100 kg 0Ah, 250 kg 0Bh, 500/800 kg 0Ch, 1000 lbs 0Dh, Tiny Tim 0Eh, Fido 0Fh, R4M 10h;
anything else logs `%s` (004254B0) and answers 7. The reconstruction does not emit the log.

## Screen vtable slots named

Slot layout of the base 00CEAE54. Names rest on the slot and the strings each body references.

| Screen (vtable) | Named here |
|---|---|
| rewards / FE_scoring (00CF50CC) | register 00629D10, layout 0061FBE0, enter 0062C2C0, exit 00621720, update 0062B640 |
| HUD markers (00CF5950) | exit 0063E360 |
| spectator (00CF5998) | enter 00643F40 |
| HUD root (00CF5A9C) | exit 00646C50, collect 00648000 |
| ship status (00CF5E30) | register 0064BB90, collect 0064DA00 |
| submarine (00CF61D4) | register 00650170 |
| periscope (00CF639C) | layout 00651230, enter 00650750 |
| support manager (00CF6B44) | layout 0065DAA0, enter 00674B80, collect 00673100 (update 00675C40 is cc9's) |
| tactical sheet (00CF6CEC) | register 0067B110, enter 0067B1D0, update 0067B0B0 |
| cursor (00CF6D28) | register 0067BCB0, enter 0067BD70 |
| follow screen 49 (00CF6D68) | register 0067BE80 |
| unit library (00CF6E50) | register 0067F080, enter 0067F4D0, exit 0067DED0 |

Slots with no Ghidra function: 00626760 (rewards +24h), 006405E0/00640600 (markers +00h/+24h),
00643DC0/00643DE0/00643DF0/00644060 (spectator), 00647FE0, 0064B860/0064D8D0, 00650160/00650190/
00650450/00651350 (submarine), 00650AC0/00650810/00651360 (periscope), 006730F0/00662470 (support
manager), 0067AF60/0067AFA0/0067B2A0, 0067B320/0067B330/0067BE20, 0067BE50/0067BEA0/0067BEB0/
0067BEC0/0067BE60, 0067F050/00680DA0.

## Handoff

Named 35 (9 reconstructed, one partial). Open: the support-manager purchase menu 0066B6C0
(`ingame.sm_cost` / `sm_cp` / `sm_stock` / `sm_support`), the radial icon builders 0065D220,
0065E870, 0065FBF0..00662210, 00665F30, 00666DC0, the rewards update/enter bodies, and the HUD
marker icons 0063C110, 0063DEC0.
