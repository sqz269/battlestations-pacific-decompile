# cc13-sweepB8: eighth pass of the segment 15-51 leftover sweep

Continues the order left at the end of `docs/CC13_SWEEPB7.md`. Code: `src/cc13_sweepB8.cpp`,
`include/bsp/cc13_sweepB8.hpp` (`Cc13SweepB8Host` extends sweepB7's `Cc13SweepB7Host`, so every
sweepB..B7 and pkt642 hook is reused). The sweepB3/B4/B5 reconstructions of 005A3830, 005A7550,
005A8F50, 005A91E0 and 005A9770 and ng2's 0059AA40 are called directly. Every name is a hypothesis,
not a recovered symbol. Nothing is wired into the game; the status of each function is reconstructed
and build-tested only.

Conventions as in the earlier passes: native records are raw byte blocks touched only at the offsets
the image reads or writes; every call the image makes is a named host method at the same point with
the same arguments, including each inlined checked-iterator failure call (00BF6713) and each inlined
native-string temporary (0041DD40, memcpy, pool release 00419CC0/00BD1510). EH frames are noted, not
modelled. x87 reg-reg operand directions were read from the bytes.

No function in this pass draws from the shared RNG 00BD2F10.

## 005A9DB0 BSP_HudMapScreen_BuildUnitOrderOverlays (segment 20)

`__thiscall(ECX = HUD map screen; unit, int flag)`, RET 8, frame 170h, no EH frame; caller 005B2D70
(sweepB5 calls it through `hud_map_build_unit_order_overlays_005a9db0`, flag 0 for the first list).

- c1 = 0059AA40(0.02, 0.01, 0.005, 0.0025, screen+1Ch), c2 = 0059AA40(0.02 x4, screen+1Ch).
- Unit kind 1Ch false: flag != 0 returns; neither kind 5 nor 18h goes straight to the director part.
  The owner is unit+538h (kind 5) or [unit+3D0h]+538h (kind 18h). For a kind-1Ch unit of neither kind
  the image reads the unassigned stack slot ESP+30h, which holds c1's float bits, as the owner pointer;
  the code does the same. table = owner+B4h (null returns); record = table + 60h * (unit+1E4h)->vt+4h()
  + 8.
- unit->C4h(pos); s = (float)(20070.0 / screen+30h * 4.98256e-05); 00B63B30 inverts diag(s, s, 0, 1)
  and 00B62D10 transforms (0.00333f, 0, 0, 1) through it (the result is never read); the translation
  frame F = identity with row 3 (pos.x, 0, pos.z, 1). 005A1800 looks the unit up in screen+4F4h with the
  inlined iterator checks; the entry is not used.
- Each of record+58h entries (20h bytes at record+54h): kind-1Ch units place the capture radius:
  screen+554h/558h from the GUI cursor (004C12B0 +5Ch/+60h, the .data halves 00E08890/00E08894), d =
  sqrt(dx^2 + dz^2) to unit->C4h() (two calls), and when (double)unit+7C4h + 300.0 > d (unordered skips)
  005A91E0(screen, f / 500.0, {pos.x, pos.z}, 1070.0f, f = (float)unit+7C4h, (float)unit+7A0h, c1 * 5000).
  Flag 0 only: w = [[00E188A8]+21C4h]+74h * entry+8h; kind 0/1 -> 005A8F50(F, w/5000 + c1, 7 or 3) and
  (F, w/5000, 9 or 5); kind 2 -> v = +78h * w, either 005A8F50 layers 0Bh/0Dh (byte entry+4h zero) or
  005A9770(F, unit->C8h(), v/5000 + c1, c1, c2, entry+0Ch, 0Bh) and (F, unit->C8h(), v/5000, 0, 0,
  entry+0Ch, 0Dh).
- Director part (flag 0 only; flag 1 returns after the entries): 005A16F0 lookup in screen+78h; found = node != the
  head read before the call. Kind 45h/46h: idx 0, b17 set; else idx 1, b17 = kind 9. follower =
  007788B0. Not found and (follower && !b17, or unit+188h+4*idx == 8): continue only when game+1FE4h and
  unit+54h is the local party, else jump to the tail. style = 2 for the selected unit (unit->140h() ==
  [00E188D8]), 1 when found or unit+18Ch is 9 or game+18ECh, else 0. Colours A/B/C (alpha 0.8) are
  passed by pointer in the slot that the sweepB3/B4 reconstructions type as an int.
- Director d = unit->114h(): b17 && d->1Ch() and the leader's bytes (+5Ch set, +5Dh/+60h/+5Eh clear) and
  00803CE0(unit+54h, leader): 005A3830 arrow unit -> leader in A. Follower: tail. Byte d+18Ch: target t =
  00521EA0, arrow unit -> t in A (identified, 00803CE0 for kind 2) or C. For each filled slot i <
  0071BE60(d) with d->20h(i): target slots arrow to t (C, A when t == d->2Ch(), else B), point slots
  arrow to the point (B), path slots 005A7550(holder->8h(), holder->14h(), B) when 0 <= k < 0059CAC0;
  each moves the arrow start.
- Tail: o = unit->30h(), o+8h set and the local party: 0059DD20(arrow, screen, o8+1Ch, o8+28h, o8+34h).

## 00789560 BSP_NetworkPacketStatistics_StartCsvLog (segment 44)

`__fastcall(ECX = statistics block)`, RET, AL = the new +762900h; caller 0088D6E0 (the Lua binding
NetworkLog in `src/cc13_left55_seg56.cpp`, through `packet_statistics_start_csv_00789560`).

- f = counter / frequency of the clock [01090AB0] (DE F9); +76498Ch = f, +762968h = truncated f.
- Logging off: GetSystemTime; side = a copy of "_server" (game+1FE4h == 1) or "_client"; +7649A0h and
  +7649A4h = [0109CEEC]->vt+4h(name, 35h) for "ns_" + name(+7649ACh) + side + "_send_" / "_recv_" + month
  "-" day "-" hour "-" minute "-" second ".csv" (no year), every temporary built and released in the
  image's order; 1001 rows of 50h bytes from +74F02Ch reset (words +0Ch/+1Ch = 9999); +762960h =
  +762958h = +764994h = 0, +764998h = +76499Ch, +76295Ch = 1000; the clock again into +764930h and
  +76492Ch.
- Logging on: 007884D0, then for both files InterlockedDecrement(file+4h) and vt+0h at zero (no null
  test).
- The byte toggles.

## 006003A0 BSP_HudPauseListboxListener_Current08 (segment 24)

`__thiscall(ECX = listener = pause screen + 8h; two stack words never read)`, RET 8; vtable 00CF3EE0
slot +8h, no static caller.

- key = listbox+164h + 00A9C920; found in the map +114h (005FD220): 00683790(+94h, 0), the entry's
  string (00426060) loses a leading "AAAAAA" marker (strstr, 00469840 substr(6), 00425F40), goes to
  +40h (00ABAED0) and +38h is hidden.
- Not found: 00683790(+94h, 0); +11Ch zero returns. best = the largest key below the selection key; the
  award tracker (004E1CA0) list for that key (005FF5D0, 006002B0, 004D48A0) names the (key - best)th
  hint (005FFB20 into the 005FD650 record by 005FDD90). command "", description "Nincs description a
  hintben."; the local party picks the field set (party 0: title +20h, command +28h, description +30h;
  party 1: +38h/+40h/+48h; else +0h/+18h/+10h, each only when its description is non-empty); unless
  [00F88A30] the "_pc" localised variant (00A9FAD0, 00436630, 00449AF0) replaces the description when it
  differs. 005FD360 runs the command; +38h gets the description and is laid out under the title
  (00AB6BD0 heights, 0.01 and 0.001 offsets), 006834A0 sets the scroll range (+0.03), then the strings,
  the hint (004C8F00) and the list (004D05E0, free of its head) are released.
- Rows i < listbox+104h without an entry move to x(+3Ch) + 0.018.
- Thumb +78h: s == 15.0 places it at (selF - 15) * (+7Ch - +90h) / (listbox+154h - 16) + +84h; s == 0
  with +F8h != 15.0 and listbox+154h != 10h at (+7Ch - +90h) * selF / (+154h - 16) + +84h; then it is
  clamped to +7Ch + +84h - height; +F8h = s.

## 005EADC0 BSP_MultiModeSelectorScreen_BindLayout (segment 23)

`__fastcall(ECX = screen)`, RET; vtable 00CF2B44 slot +14h, no static caller.

Pages FE_multimode_listbox (+14h), FE_multimode (+18h), FE_createserver (+1Ch) through 00AA5840(name,
1, 0); 25 children by name through 00AA7E00(name, 1) into +20h..+D8h, listeners 00AA6BC0(screen+0Ch, 0)
on the scroll arrows, frame box and +B0h/+B4h; the description scroller +3Ch gets +38h (00683790) and
00683380(+9Ch, size(textx_scroll_bg_Icon).y - size(+9Ch).y, +30h, +34h); popup_Listbox +A8h gets four
00A9D750 rows (+C0h, +C4h, then min_rank_Group +B8h and max_rank_Group +BCh, not the rank texts), +78h
flags, 00A9CEB0(size(+C0h).x + 0.0041667, 1), its +12Ch..+134h and the rows' +30h/+34h offsets.

## 005E3580 BSP_MultiGameLobbyScreen_RefreshSettingRows (segment 23)

`__fastcall(ECX = lobby screen)`, RET; callers 005E50D0, 005E51D0, 005E5AE0, 005E5B30.

- Clears the list boxes (+94h only for the host, bit 0 of +10h, and +228h), 005D8A70; row height =
  max(h(+21Ch), h(+218h)) * 1.05; 005D5930 / 005D8B80 / 005D8FC0, or the game+600h string emptied and
  +2F0h..+2FCh = -1.
- Slots 0..13 (13 only when the setting row is >= 2): host slot 1 relabels +20Ch; 005D5500 < 0 skips;
  host slots > 1 add a +214h clone (setting_Text, setting_var_Text) to +94h and grow the scroll total;
  every slot adds a +218h name clone (name + "|.:", pushed to +230h) and a +21Ch label clone to +228h
  and records the slot in +1BCh.
- Re-selects +274h, sets the listener screen+8h and calls its vt+8h on the selected row; host: scroll
  range total - size(+A0h).y.
- Map preview: for the selected widget index of +1F8h (host) and +244h, the scene bounds come from
  [00E19564]/[00E19568] (scene record 005D86B0 or the scene table row), u = (x + 15000) / 30000, v = 1 -
  (z + 15000) / 30000, aspect = |dx| / |dz|; vt+88h(kind, 0, 1.0f), +118h..+124h, vt+8Ch(), vt+58h with
  the size from the aspect (150/960 and 150/720 scales).
- Deviation (documented): when [00E19564] <= 3 and [00E19568] > 3 the image reads four stack slots it
  never assigned on that path. docs/MISSION_LOBBY_SETTINGS.md says 00E19568 is only written with values
  below 4, so the arm should be unreachable; the code keeps the previous iteration's values (zero
  first).
- Host: 005D98E0; dword +10h = 0; +94h vt+70h().

## 00597870 BSP_MainMenuScreen_BuildMissionListPage (segment 20)

`__thiscall(ECX = main menu screen, int page)`, RET 4; callers 005987F0, 00598B60, 00599340.

- [00E08874] = page; the page arm sets the title (00518D60(10, 0, 0, title)), the frame flag 00518B90,
  the group 00E194D8, the group widget +110h, the point list index and bytes +564h/+565h: 4
  FE.ijn_campaign (group 1), 5 FE.usn_campaign (2), 8 FE.training_grounds (0, no flag, bytes kept), 6
  FE.main_ijn_dlc_title (3), 7 FE.main_usn_dlc_title (4, title freed by 0041DD20).
- Rebuilds +1B8h from the group's missions: unlocked (007FC820 on record+78h) rows show the record name,
  locked ones the C string [00E08870]; mission_mapflag_<n>_Icon shows completion (005C2F70),
  mission_mappoint_<n>_Icon is shown and, for an empty point list, its position relative to
  +124h..+12Ch is captured (004215D0); a completed mission selects n when [00E08878] is -1.
- Selection fix-up and clamp, 00581CE0 when byte +1B0h, flag alphas 1.0 / 0.4 (vt+4Ch, no null test),
  00594B60, 005C35D0, the checkpoint widgets +2FCh/+300h (007F8D60, 007FC370 picks
  FE.main_checkpoint / FE.main_checkpoint_dlc), list box placement {+558h + 0, 0 + +55Ch, +560h +
  1000}, 005830A0.
- The map flag's visibility argument is the dword holding the completion byte; the code passes 0/1.

## 005E7B10 BSP_MultiMainMenuScreen_BindLayout (segment 23)

`__fastcall(ECX = screen)`, RET; vtable 00CF2708 slot +14h, no static caller.

Pages FE_multimenu_listbox (+18h) and FE_multimenu (+1Ch) through 00AA5840(name, 1, 0); 26 children
through 00AA7E00(name, 1) (listeners 00AA6BC0(screen+0Ch, 0) on the arrows +60h/+64h and the slider
+CCh; multiscore_Text +38h and multiscore_var_Text +3Ch hidden). The achievement scroller +6Ch gets
+68h (00683790) and 00683380(+CCh, size(slider_bg_Icon).y - size(+CCh).y, +60h, +64h). A second
achievement_Group lookup fills +D8h; select_FrameBox +D4h takes achievement_0_Icon's (+DCh) local
position (00AA67F0, 00AA7DC0), local z -100.0f (00AA7910) and is hidden. For i < 006B8B10([00E19900])
(re-read each pass) over 006B8CC0's iterator, the node+14h award record is copied (0050A210, 54h
bytes); when its byte +24h is set a clone of +DCh under +D8h gets vtable +2Ch(node+0Ch), +34h(1),
00AB44B0(0, "" + record+14h, {0, 0, 1, 1}) and the listener, and becomes +E8h; the copy is destroyed
(00501FA0) and the iterator advanced (004FBE20).

## Not reconstructed

- 006D5220 BSP_AirField_ReadHangarAndMarkerProperties (segment 31): cc9 lane. It builds the airfield's
  hangar list +830h and gives LandForts their master through 00744A20; the cc9 air-operations and ship-AI
  analyses depend on it (docs/GAMEPLAY_GAP_RANKING.md, docs/SHIP_AI_OPEN_ITEMS.md) and
  game_hosts_scene_contents.cpp already models its "Hangar %d" reads. No record.
- 00776930 BSP_SessionHostInfo_CheckDlc (segment 43; outside the cc9 unit/formation exclusion
  00778820..0077D270): read but not started. It reads DLC.lua into a list of lists of {name, 64-bit
  mask} with inlined std::list node insertion, then builds the missing-DLC message in wide strings.
  No record.

## Not reached in this pass (no record)

In order: 006179D0, 0056F320, 0067F4D0, 005D73A0, 007884D0, 00536A90, 00674B80, 005B3470,
00673A10, 005DF510, 00538F80, 005B45C0, 0060ABD0, 007A91A0, 00670020, 00822C20, 00627960, 005DB050,
005ADDA0; then 005A3210 and 00617470; and 00776930 above.

## Reconstructed

005A9DB0, 00789560, 006003A0, 005EADC0, 005E3580, 00597870, 005E7B10.
