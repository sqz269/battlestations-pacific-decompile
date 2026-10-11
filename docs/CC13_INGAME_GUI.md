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

## Rewards update 0062B640 (cc13-frontend2, reconstructed, complete, build-tested)

`rewards_screen_update_0062b640` in `src/ingame_gui.cpp`; call rows in `reports/cc13_ingame_gui.json`
(33 rows, 0 failed). `__thiscall`(screen, float seconds), RET 4; the seconds slot is overwritten
with min(*(00E188A8)+21ECh, 0.03) (FCOMI against the double 00CEB690, fallback float 00CE3DB8).

- +3Ch set, or action 4Ah: 0062B110 (cc9-cited; only called here) and return.
- +38h counts a pending storage flush down; at 0 it calls 007FA1B0(+19h, 00BD53C0) on game+650h.
- +258h set: return before any page work.
- Page +58h == 1 (read as the multiplayer page): action 4Fh shows the gamer card, action 50h on a
  live session (slot +198h) the player review for anyone but the local player (00A3EB00).
- Page 4 (read as the rank page): the rank widget *(+210h) grows toward +21Ch by
  target * step * 0.25 per frame and snaps to it, and its alpha bounces between 0 and 1 by one step
  (004155B0 clamp).
- The scrollers at +68h, +128h, +19Ch update; actions 4Ch/4Dh turn the page (00622E30(-1/+1)),
  flash the arrow (+5Ch/+60h slot +88h(2, 0, 1.0f)) for 0.1 s (+64h), then reset both arrows.

The rewards enter 0062C2C0 was not reconstructed: a recursive descent over the disk image reaches
0063158F, about 5200 instructions with 746 calls, which is beyond this packet.

## Slot functions defined by cc13-frontend2

The Ghidra-less slots listed above are now Ghidra functions and named; see the third-packet section
of docs/CC13_FRONTEND_MENUS.md for the method and the table of kinds.

## Support-manager info panel 0066B6C0 (cc13-frontend3, reconstructed, complete, build-tested)

`support_manager_refresh_info_0066b6c0` in `src/ingame_gui.cpp`, Ghidra
`BSP_HudSupportManager_RefreshInfoPanel`. `__thiscall`(screen), RET. Callers: 0066D020 (itself called
by the enter 00674B80 and by 00675C40) and cc9's support-manager update 00675C40. Not a purchase-menu
builder: it refreshes the panel that describes the entry under the cursor. Evidence: a full pass
over the listing (1601 instructions); the pseudocode drops the hidden ECX arguments and garbles the
bar arithmetic and the 0/1/2/3/8 mode test, so the reconstruction follows the listing. Call rows
in `reports/cc13_ingame_gui.json` (167 rows, 0 failed).

Common steps: the name goes to +170h (00ABBE50, localise 1) followed by 0065BBC0
(`BSP_HudSupportManager_LayoutNameLabel`); the type icon +1CCh takes state `map[key]` from the
icon map +42Ch keyed by the record's +70h string (checked form: 00614060 find + 00611C80 against the
end +430h, else state 0; unchecked form: 006171B0 alone, which inserts a 0 entry); detail widgets
+F0h/+F4h are shown one or the other; the loadout icon +158h gets 00AB2690(3, texture, UV) from
009552E0(loadout) on the class record -> +4 -> +8 -> +4 resolved by 00443490 (DL = 0) -> +74h ->
+34h (texture +7Ch, UV +90h..+9Ch; resolved twice natively), then slot +88h states 0 and 3, or
state 0/1 by record flag 17h when there is no loadout.

- Page 0 (+5Ch = 0): +70h = 0. With +34Ch nonzero, 0066A8A0 rebuilds list +3E0h, the selection
  +2C4h is clamped to size - 1 (unsigned JB) and the node at it is shown: a live unit (byte +14h,
  unit +18h; kind 6 shows its class icon and +F0h; otherwise kind 18h redirects to the owner +3D0h
  for class, icon and loadout +C54h) or a class record (+08h; flag 6 shows +F0h, otherwise +F4h and
  the loadout +0Ch). Then, with +1F4h nonzero, the unit at *(+304h)+8: kinds 45h/9 read the 58h-byte
  hangar record +324h under +72Ch (kind 45h, queried again) or +1188h (record +4, loadout +10h, -1 =
  none shows +F0h and returns); other units use 00846FD0(&out, +324h) (record +08h, loadout +0Ch,
  flag 0Fh gates the detail; destroyed by 0065BB00).
- Page 1: with +34Ch nonzero the type icon is 1 when the node at +350h/+354h has +08h set, else 0
  when the unit at +3Ch of the front object of the node's list +0Ch is kind 9, else 1. The two
  list copies (005AAAF0) and front() checks are debug-iterator noise and are not modelled.
- Page 2 with +34Ch: 0065A2B0 syncs the cursors, 0065AB40 gives the catalog record (+3A4h); +164h
  == 0 sets +2B0h and calls 00665F30; flag 6 picks +F0h. CP = 00946F00, cap = 00942130 (ECX =
  *(00E188A8)+18ECh), cost = record+124h. In modes 0..3 and 8 with +66h clear, +1A8h shows
  `ingame.sm_cp|.: <CVTTSS2SI(CP)> / |ingame.sm_cost|.: <cost>`; FILD/FSTP rounds the cost to float
  and FCOMI/JB sends CP < cost (or NaN) to the unaffordable side: +180h (unless +2C1h) gets width
  +1ACh * CP / cap and the others hide; otherwise +184h gets that width, +188h gets (CP - cost) *
  +1ACh / cap, +180h hides. All heights are +188h's (00AA6740 +4). Other modes or +66h set show
  `ingame.sm_support` with +184h at full width (two copies differing only in the EH state).
- Page 2 without +34Ch, with +1F4h: unit kinds 45h/9 take 0065AE90 (+2A0h), kind 46h 0065AB40
  (+3A4h), anything else returns; then name, checked icon and `ingame.sm_stock|.: <0065AC30>`.

Host wiring: none, by instruction (00675C40 is cc9's). Wiring would mean 00675C40's host (and the
0066D020 path) calling `support_manager_refresh_info_0066b6c0` through an adapter that exposes the
screen offsets above; no edit was made on that path. The x87 bar widths are computed in double,
which can differ from the extended-precision result in the last float bit in rare cases.
Named alongside (provisional): 0065A2B0, 0065AB40, 0065AE90, 0065AC30, 0065BBC0, 0065BB00.