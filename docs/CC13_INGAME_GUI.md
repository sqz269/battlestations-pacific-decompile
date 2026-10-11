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
## Rewards enter 0062C2C0 (cc13-frontend3, opening segment; the rest is in the cc13-rewards1 section below)

The body does not decompile and is far larger than the earlier estimate: the recursive descent
that stopped at 0063158F missed jumps to 006316CB..00631BA3 and to the shared epilogue 00636D63;
the function runs to the RET at 00636D78, about 10760 instructions (190 child lookups, 284
string releases, 128 indirect calls). EBP = screen throughout; the EH state lives at
[esp+E74h] relative to the post-prologue ESP.

`rewards_screen_enter_award_queue_0062c2c0` in `src/ingame_gui.cpp` covers 0062C2C0..0062CA7B
(build-tested, call rows in `reports/cc13_ingame_gui.json`, 213 rows, 0 failed):

- Session slot +198h true: session slot +58h. +64h = 0.0, +258h = 0, +25Ch hidden, the award page
  vectors +238h and +248h cleared (00625A50).
- +18h = commit record (004B4750) +0Ch == 0.0f (UCOMISS/LAHF/TEST AH,44h/JP: ordered equality);
  00594A70(game+6B4h, &game+2198h) is the mission record and 009052D0 its completed flag;
  005C3870 is called and its answer kept at [esp+84h] for later pages; +28h and +30h copy
  game+2198h; +19h = *(record) != 0; +24h, +25h, +27h, +1Ch, +1Dh cleared.
- +3Ch set: the frame title is cleared (00518D60, ECX = EDX = 0) and the routine returns.
- +18h set: straight to the page builders at 0062CA7B.
- Otherwise: a page of kind 0 when the profile's "RANK" counter differs before and after
  007FCD90(0). With game+1FE4h == 0: +24h = 007F93F0; when set, every key of the profile set
  game+6CCh that is not in game+6C0h is offered to 00622360
  (`BSP_RewardsScreen_CollectVehicleUnlocks`): its count (flag 1) pushes that many kind-1
  pages, then the recording pass (flag 0) runs, both skipped for a count of 0. Then 007FB990,
  +27h = 005C3DE0 (campaign advance), +30h = 00586150's mission id when advanced, and with +19h:
  +25h = advanced and not completed, +1Ch = 005C3BE0(+28h) when not completed.
- Every key of the commit record list +6Ch starting "RANK_SR" (00553C80) is offered to 00622360
  in the same way.

Outline of the rest (string anchors and calls in address order; not reconstructed):

| Range | Content |
|---|---|
| 0062CA7B..0062CDDA | background/back icons, `debriefing_Clipbox`, `congrat1_Text` and `move_Group` placement, scroller range |
| 0062CDDF..0062D43B | `rank_Icon` (00574050), `info_Group`: `time_value_Text` (004396F0, 0043CC20), `score_value_Text`, `medal_Icon`, a streamed debrief dialog (`sound/messages/`, `/streamed_dialogs/`, file store 004FC150/00BE80B0/00BE7CD0), second time text, list reset (00A9BEC0, 00A9AC40, 0062A0B0, 00625D10) |
| 0062D464..0062E9A4 | multiplayer scoreboard: `FE.rewards_missioncompleted`/`missionfailed`, `FE.scoring_USN_wins`/`IJN_wins`, `END_Text`, `USA_Text`/`JAP_Text` rows (`rank_Text`, `name_Text` with `globals.newplayer`, `clan_Text`, `score_Text`, `flag_Icon`, `rank_Icon`) in three copies |
| 0062EA00..00630143 | objectives: `FE.scoring_missionfailed`, side 005C27E0, `number_Text`, `primary_Text`, `^FE.rewards_completed`/`^FE.rewards_failed`, `primary_value_Text`, `primary_point_value_Text` (0061EC70, 0052ED20, 0057F010, 0061F340), hidden ones as `globals.obj_hid` |
| 00630211..00635939 | scoring tables framed by `row_line`/`vline1..4`/`separator_line` boxes: `FE.scoring_playeract`, `FE.scoring_partyact` (00911CE0, 00593480, 0062C170, 0062BB60, 00625900, 00911C00; `FE.scoring_friendlyhit`), `FE.scoring_command` (+ `_type`/`_amount`), `FE.scoring_remaining` (00910500), `FE.scoring_suicide` (00910570), `FE.scoring_mapusage` (009064D0, `%d:%02d:%02d`/`%d:%02d`), `FE.scoring_badge`, `FE.scoring_missionspec`; value texts `done_by`/`done_to`/`damage`/`kills`/`total_value_Text` |
| 006359F1..00635F61 | rank page: FE.playerrank table 00E08E00, "RANK" thresholds (0050FC30), 0090BF50 score, " / " text, two clamps (004155B0) |
| 00635FC3..00636C33 | award pages: `award_Icon`, `award_title_Text`, `FE.scoring_unlockedunit|.: |`, `award_detail_Text`, `FE.scoring_unlock_text`, payload icons (00AA78F0), 0061EB80/0061E610/0061EBC0/006277C0, 0090C000/0050FC30/0050A210/00AB44B0, 00501FA0 |
| 00636C69..00636D60 | 0062A970, frame title 00518D60, page 00622E30, help line 0054A0C0, a final +38h store |

A successor should take these ranges one at a time with the same EH-anchored listing reading.
## Rewards enter page builders 0062CA7B..00636D78 (cc13-rewards1, reconstructed, build-tested)

Read from a capstone listing of the disk image (10757 instructions, 0062C2C0..00636D78) folded by
`local/rw1_sq.py` (string temporaries, child lookups, pool releases). ESP offsets are base-relative
(post-prologue ESP, EH state at [esp+E74h]); pushes are subtracted at every read. The screen is EBP
until 0062EBAF, where EBP is reused; from there the screen is [esp+54h] (stored at 0062C2F7).
`rewards_screen_enter_pages_0062ca7b` in `src/ingame_gui.cpp` continues the opening function at the
join point; call rows in `reports/cc13_ingame_gui.json` carry `function: 0062c2c0` and the C++ name
in `cpp`.

Locals: [esp+24h] is the commit record (004B4750), [esp+38h] the 00594A70 mission-score record,
[esp+84h] the 005C3870 mission-tree record (+38h `debriefingText`, +50h `debriefingVoice`, per
docs/MISSION_TREE_LUA_READER.md), [esp+9Ch] the local player's scoreboard row.

| Range | Status | Content |
|---|---|---|
| 0062CA7B..0062CBE0 | reconstructed | +258h = 1 when award pages are queued; `backgrnd_Icon` (+20h) and `back_Icon` (+25Ch) slot +88h state = !+19h; page mode +58h: game+1EE3h or game+1FE4h set gives 1 with 00E08C88 = 0, 00E08C89 = 1, else +19h ? 0 : 2 with 00E08C88 = +19h, 00E08C89 = 0 |
| 0062CBE6..0062D2F6 | reconstructed | mode 0, page +40h: `debriefing_Clipbox`/`move_Group`/`congrat1_Text` gets `debriefingText` (localised); scroller +68h range = that text's height (00AB6BD0) - [+D0h]; `rank_Icon` = max(00574050 - 1, 0); `info_Group`: `time_value_Text` = 0043CC20(004396F0(record+0Ch)), `score_value_Text` = record+1E0h; `medal_Icon` shown with state max(record+4 - 1, 0) when +19h; when [+10h] == 0 (NEG/SBB/TEST 00E18B5Ch) the voice "sound/messages/" + 008D57A0(00F88980) + "/streamed_dialogs/" + `debriefingVoice` is requested with callback 00622E00 |
| 0062D2FB..0062E9B7 | reconstructed | mode 1, page +44h: `time_value_Text`; +230h/+234h cleared; listbox +D4h cleared, listener 0; 0062A0B0 builds the 60h-byte rows at +220h, 00625D10 sorts them (introsort shape); game+614h == 7: title 1 = missioncompleted/missionfailed by record+0, `END_Text` = record+254h, USA/JAP hidden, every row in slot i; otherwise title 1 = USN_wins when (won ? side == 0 : side == 1) else IJN_wins, `USA_Text` = record+25Ch with the side-0 rows in slots 0.., `JAP_Text` = record+264h placed 6 row heights below `USA_Text`, side-1 rows from slot 6 numbered slot-5. Each row: clone of +D8h shown, inserted with its index, byte +22Fh+count = !row+5Ch, `rank_Text`, `name_Text` (localised when row+5Ch or "globals.newplayer"), `clan_Text`, `score_Text`, `flag_Icon` (side, 0, 1), `rank_Icon` state and visibility, position = template + (0, (height + 1/120) * slot, 0). Then listener screen+8, select the local row, active, listbox +12Ch/+130h/+134h, 00A9CEB0(0.575, 0), +128h |
| 0062E9BF..0062EA44 | reconstructed | lost single-player mission: title 2 = "FE.scoring_missionfailed"; side = 005C27E0(tree record) without a session, else record+1E4h == 0 |
| 0062EA44..00630188 | reconstructed | objectives page: the side's three lists in the commit record (+204h/+210h/+21Ch or +228h/+234h/+240h, std::list size at +8); gap = +E0h.y - +ECh.y; spacing = ((0.65277779 - lines/36) - (1/36 + gap) * sections) / (sections + 1); cursor = (+DCh.x, spacing + 0.17361111, -1); 00E08C8A = any objective. Each non-empty section shows its header (+ECh/+F0h/+F4h, placed at the cursor; the primary one takes the x of +F0h) and box (+E0h/+E4h/+E8h, placed one gap lower at z 0, height (n + 1)/36), then per node a clone of +DCh: `number_Text` "N.", `primary_Text` = key without its first character (primary: 00ABB000 with width -1; secondary: 00ABAED0; hidden: "globals.obj_hid" unless 0061F340 finds the key in the 00594A70 record +21Ch/+240h with +1Ch set), `primary_value_Text` "^FE.rewards_completed"/"^FE.rewards_failed" (00ABBE50) by node+1Ch, `primary_point_value_Text` = the record+18h points map (0061EC70, node+14h) or "0", colour (0.7, 0.1, 0.1, 1) when failed, shown, kept in +F8h (0052ED20), placed at the cursor, cursor += 1/36; after a section cursor += spacing + 1/72. An empty section hides both widgets |
| 00630188..006359A3 | reconstructed | scoring page. 00E08C8B = 0; mode [esp+40h] = game+1FE4h ? 2 : game+6ACh; templates +10Ch (table), +110h (group), +114h (row) under +118h; the table's child local positions (00AA67F0 = &widget+0Ch) are saved and re-applied to each clone with 00AA7DC0; the done-to column width = total.x - done_to.x - 1/48. A table is a clone at the cursor with `header_Text` (00ABB000, width -1), shown, cursor.y += +110h.y - +10Ch.y; a group is a +110h clone with `done_by_value_Text` at (group.x, cursor.y, group.z); a row is a +114h clone at (row.x, cursor.y, row.z), cursor.y += 0.0305556; a group ends with cursor.y += 1/360 and its `row_line_FrameBox` sized and placed; a table ends with vline1..4 sized to (vline1 width, cursor.y - vline1.y) (the command table hides vline2/3) and row/separator lines sized. Tables: `FE.scoring_playeract` when record+A4h (map +9Ch) and `FE.scoring_partyact` when record+B0h (map +A8h): one group per by-category (00E0B5E0 unit-category keys), one row per (friendly key, target category): damage, kills from a copy of record+B4h/+C0h (00593480; 0062C170 -> 0062BB60 -> 00625900; freed by 00593180), total = 00911CE0(by, target, friendly, damage, mode, record+24Dh) + 00911C00(friendly, by, target, (float)kills, mode, record+24Dh); friendly key 0 prefixes "FE.scoring_friendlyhit|.: |" and paints done-to, total, kills (player) or done-to, damage, kills, total (party) (0.7, 0.1, 0.1, 1). `FE.scoring_command` (columns `FE.scoring_command_type`/`_amount`, `damage_Text`/`kills_Text` hidden) when any of record+D4h, +E0h, +14h > 0, +1DCh > 0, +50h: groups `FE.scoring_remaining` (map +CCh, side key 0 only, 00910500(category, record+8) * count, label "<category>|. (x<count>)"), `FE.scoring_suicide` (map +D8h, 00910570(category, mode, record+4) * count, painted), `FE.scoring_mapusage` (009064D0(record+14h), "%d:%02d:%02d" / "%d:%02d" / "%d" of max(t, 1), truncated seconds, not localised), `FE.scoring_badge` (record+1DCh, label record+1DCh / 100), `FE.scoring_missionspec` (map +48h, node +0Ch label, +14h points) |
| 006359A3..00635F9E | reconstructed | scroller +128h range = cursor.y - +108h height - +108h y. Rank page, only when game+1EE3h and *(00F8A2FC)+4Dh: 00E08C8C = 1; rank = clamp(00574050 - 1, 0, 19): +204h = FE.playerrank_NN (00E08E00, 00ABBE50), +208h slot +88h state rank; thresholds from 0050FC30(*(00E19900)+10h, "RANK") vector +48h: next = [rank + 1] unless the rank byte is 20, previous = [rank] when the byte > 1; score = 0090BF50(profile+64h); +200h = score - profile+30h (or the score when +30h is -1); shares (last - previous)/(next - previous) (0 when last < previous) and (score - previous)/(next - previous), both 1.0 without a next threshold; profile+30h = score; +1FCh = "score / next" or the score; +218h = 0, +21Ch = clamp(now) * (+214h width - 0.00625), bars +20Ch/+210h sized (clamp(before) * that, +20Ch height) |
| 00635FA3..00636C33 | reconstructed | award pages, when game+1EE3h or +19h: 00E08C8D = 0061EB80 (profile+BCh pending awards non-empty) or +248h non-empty or record+1DCh > 0; entries are clones of +18Ch under +188h at the running local position (start: +18Ch's), each with `award_Icon`, `award_title_Text`, `award_detail_Text` placed 1/144 below the title text (00AA78F0), shown, sized to max(icon height, text height) + 1/72 (FCOMI against the float-rounded text height), position += height + 1/144. Sources in order: +248h (10h elements: 004FB150 icon by id +4, title "FE.scoring_unlockedunit|.: |" + unit +8, detail "FE.scoring_unlock_text"); the profile's pending awards game+70Ch (1Ch elements: icon by name +10h through 004FC070 when id +18h is 0, title +0, detail +8), cleared by 006277C0; with record+1DCh > 0 every key of record+6Ch that 0090C000 accepts, its award record (0050FC30 on *(00E19900)+10h, copied by 0050A210, freed by 00501FA0): texture +14h through 00AB44B0(0, "" + texture, (0, 0, 1, 1)) and the icon shown, or the icon hidden, title +0, detail +0Ch. Scroller +19Ch range = position - +184h height + 1/144 |
| 00636C38..00636D78 | reconstructed | tail: with +258h and pages queued, listbox +D4h slot +60h(0) and 0062A970; +1Dh = more than one of 00E08C88..00E08C8D set; frame title "" (00518D60), 00622E30(0), help line "" (0054A0C0 on *(00E1930C)); +38h = +18h ? -1 : 3 |

Ghidra has no instructions at 006316A3..006316C6 and 00632A5C..00632A7F (the L3 iterator steps of the two damage tables, reached only after 00593180): their call rows (0057F4A0 at 006316B5 and 00632A6E) are left out of the report; a listing repair is needed before the body can be re-exported whole.

Uncertain: the meaning of +10h (the voice gate), record+1E4h, row+5Ch (an AI slot is a guess);
the non-7 branch reads the local player without the 0..7 bound the row matcher applies; the
byte +22Fh+count has no bound against the eight bytes cleared.
