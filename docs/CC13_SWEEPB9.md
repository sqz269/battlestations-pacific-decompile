# cc13-sweepB9: ninth pass of the segment 15-51 leftover sweep

Continues the order left at the end of `docs/CC13_SWEEPB8.md`. Code: `src/cc13_sweepB9.cpp`,
`include/bsp/cc13_sweepB9.hpp` (`Cc13SweepB9Host` extends sweepB8's `Cc13SweepB8Host`, so every
sweepB..B8 and pkt642 hook is reused). sweepB's 0078DAA0 and sweepB3's 0056D9A0 and 00578580
reconstructions are called directly. Every name is a hypothesis, not a recovered symbol. Nothing is
wired into the game; the status of each function is reconstructed and build-tested only.

Conventions as in the earlier passes: native records are raw byte blocks touched only at the offsets
the image reads or writes; every call the image makes is a named host method at the same point with
the same arguments, including each inlined checked-iterator failure call (00BF6713) and each inlined
native-string temporary (0041DD40, memcpy, pool release 00419CC0/00BD1510). EH frames are noted, not
modelled. x87 reg-reg operand directions were read from the bytes (`tools/x87_dc_form_scan.py`).

x87 precision (rule 4c): all of these run on the main thread (GUI, HUD and the per-frame network
statistics), so every x87 arithmetic operation is modelled as a separate rounding to float in the
image's order (`pc24`). SSE arithmetic is float as written.

No function in this pass draws from the shared RNG 00BD2F10.

## 006179D0 BSP_HudPowerupCarousel_RebuildSlots (segment 25)

`cc13_sweepB9_powerup_carousel_rebuild_slots_006179d0`. `__fastcall(ECX = carousel)`, RET; the
carousel is the powerup screen + 100h (hud_widgets.hpp), caller 00619C50. The first-column
counterpart of sweepB7's 006191A0, with the support block sp = [[00E198C4]+C0h] re-read at every
access. Sets +3Fh, hides bc+100h and bc+104h, saves sp+28h/+48h (up) or sp+34h/+54h (down) into
sp+21Ch/+220h, shifts the clone / section / icon / highlight columns one place and re-finds the three
kept "circle_full_N_Section" children (name built with 004260B0 / 0043C130 / 004261A0, released
prefixed, number, suffix; each hidden, re-found, shown and given 0.5f alpha). The new slot is cloned
from bc+138h; its icon states come from the map sp+1C0h, its frame from the record selected by +4h
(checked index into sp+144h, 00BF6713, then the map sp+1B4h). Differences from 006191A0 that the
bytes show: circle_HL_Icon is stored and hidden (+34h(0)), the up-arm positions are x87 (bc+2A8h *
5.0 -/+ bc+2A4h, each rounded per operation) and the down arm uses SSE (-0.0f - bc+2A4h / bc+2A4h),
and the fourth section is circle_full_3 (up) or circle_full_0 (down). Ends with 0065E870(bc, clone,
0.0f) and +40h = 1.0f before the last name's release.

Uncertain: none in the control flow; EH unwinding is not modelled.

## 0056F320 BSP_MultiSessionBrowserScreen_Register (segment 19)

`cc13_sweepB9_session_browser_register_0056f320`. `__thiscall(ECX = screen)`, RET; vtable 00CEF134
slot +10h. Chains 004F71D0, clears +50h and calls its own vtable +14h. Walks the 434h-byte scene
records of the vector [[00E198AC]+5Ch]+24h with every inlined iterator check (00BF6713). Per record
the file name is the text after the last '/' (00467CF0 from 7FFFFFFFh, then 00469840 from pos + 1;
the substring count 7FFFFFFFh is the first push, left on the stack by 00467CF0's RET 8), cut before
".scn" (strstr; -1 when absent) through an inline assignment that copies only the length bytes. Eight
textures "gui/radarshots/" + first 14h characters + _C/_D/_E/_ICH/_ICL/_ICM/_ICS/_S.tga go through
+A0h vtable +A4h(name, {0, 0, 1, 1}, 0, 0). After the loop: 00ABBE50(+4Ch, "globals.no_server_found",
1) (the 1 is the PUSH taken before the loop-exit JZ), +4Ch hidden, +104h..+10Ch = the resolved
position of +F4h, +25Ch and +260h the y differences of +F8h/+F4h and +98h/+94h (x87, one rounding
each), 0056D9A0, 005684D0, +94h/+98h hidden, [00E086C0] = -1, the session settings
[00E188A8]+670h..+678h into +244h..+24Ch, the list [00E188A8]+664h copied byte-wise into +10h+i and
+19h+i (checked list iterator, no bound on i), and +250h..+258h, +F0h, +F1h cleared.

Uncertain: the +A0h object's vtable +A4h is treated as a texture preload (`texture_owner_vcall_a4`);
the callee is not reconstructed.

## 0067F4D0 BSP_UnitLibraryScreen_Enter (segment 27)

`cc13_sweepB9_unit_library_enter_0067f4d0`. `__thiscall(ECX = screen)`, RET; vtable 00CF6E50 slot
+18h. On first entry (+E0h null) creates the ocean environment (operator new B8h, sweepB's 0078DAA0
with source +0Ch and flag 0; null when new fails) and applies a default 288h-byte stack block
(004CB420, 0078C9B0 even when the environment is null, 004C6520). Selects the "UNITLIB" frame
(00518D60 with ECX = 0Bh, EDX = 0) and the help line, then binds 32 named children (store before each
name's release): the main list box +70h (select 0, listener +64h, +60h(1), +80h(1)), seven icons with
listener +68h (00AA6BC0, after the release), the text and group widgets, +80h through three chained
finds (AlapIze_Group, RedTitle_Group, Title_Text; built in reverse and released in reverse), +78h
hidden before Description_Listbox, and the flags under +70h. Clears +E8h.

## 005D73A0 BSP_MultiGameLobbyScreen_RefreshPlayerRows (segment 22)

`cc13_sweepB9_lobby_refresh_player_rows_005d73a0`. `__fastcall(ECX = lobby screen)`, RET; caller
005E5B30. g = [00E188A8], re-read at every access. The local record's mode +28h drives +148h /
+14Ch and (mode < 2) 00518B90. q0 = 00786010(g+1EF8h, [g+18C8h]). Then for the eight rows: ten named
children, the two flags, a native text and the tag (default " "). Joined players (+8h and not +9h):
check icon from byte rec+0Ch (or hidden with g+218Ch), text "." + name (".globals.newplayer" is
replaced by "globals.newplayer" after _stricmp), a 1-3 character tag, rank icon (00517CB0, register
ABI ECX/EDX), talk icon (+88h frame 2 when 00778740 reports the local player muted one way, 1 when
007F74B0 says talking, 0 when rec+94h == 1, else hidden), connection ping (q0, or (00786010(rec+1Ch)
+ q0) * 0.5, plus 1.0e-5, each x87 step rounded) through sweepB3's 00578580, and the colours (the
00CEF16C/00CEF168/0.6f/1.0f highlight when the g+2060h mask matches, else the static {1, 0, 0, 1}
00E195F0). Empty rows take [00E088F8] and the static white 00E195DC; rows with +9h take
[00E08940 + rec+2Ch*4] (with +0Ah, highlighted) or [00E088F8] (white) and hide five icons. The
selected row [00E08958] keeps its text only when it still matches [00E195D4/D8] (then
[00E088FC]), else [00E08958] = -1. Texts go through 00ABAED0; the "i > 8" store of row+77h is
dead code kept as written.

Uncertain: the SETZ results are pushed as whole dwords whose upper bytes are stale register bytes;
the hooks take them as booleans.

## 007884D0 BSP_NetworkPacketStatistics_WriteCsvRow (segment 44)

`cc13_sweepB9_packet_stats_write_csv_row_007884d0`. `__fastcall(ECX = stats)`, RET; callers
00789560 (sweepB8) and 0078A430 (sweepB7). Samples the clock (counter / frequency, one rounding to
float, modelled as a double quotient that is exact while the counter stays below 2^53) and writes
"Time;" + previous + ";" + now + "\n" to the send and receive files (vtable +5Ch(text, 0)). Each
pending row (r from [+764994h] % 1001 while below [+764998h] % 1001, re-read after every row) sits at
stats + 4 + r*1DE8h with the send entries at row + k*10h and the receive entries at row + EF4h +
k*10h; entry 0 of both is reset, the 234 entries are summed into entry 234, and each direction with a
non-zero count writes "min-max;bytes;bytes/count;" (004263B0, 00788380), the 234 type columns
(00788250, each entry reset after its column) and "\n". Finally both cursors advance by +76499Ch,
+76492Ch takes the new time and +7649A8h is cleared.

## 00536A90 BSP_HudFormationScreen_UpdateOrderLabels (segment 15)

`cc13_sweepB9_formation_update_order_labels_00536a90`. `__thiscall(ECX = screen; float dt)`, RET 4;
caller 0053A8E0. State +170h: 128h returns at once; ECh (orders) and EFh (formation) run the
selector unless +169h (menu committed). Six 004BEC00 reads give the two sticks; the left stick (sum
of SSE absolute values above 0.8) moves the four-way cursor +144h..+150h with the binary's
thresholds and dt steps, otherwise the right stick accumulates c/3.0 and d/3.0 (DC F9 / FDIVR) with
0.55f snaps, copies a/b, or (equal sums through 0042BE90) applies the 0.59f dead zone. +16Ch picks
cancel (0), line (1), diamond / stance (3) or column / disband (4) through 0.4 / -0.4 and the widget
+38h() checks (+E8h is asked up to three times, as in the image); (2, EFh), (1, ECh) and (2, ECh)
fall back to 0. The label goes through 00ABBE50(+F8h, "ingame.*", 1) and the chosen widget turns
white. On the confirm key ([im+4h] + st*30h, unless its release flag is set) the menu commits:
orders toggle the stance +13Ch on every member's director (0070D060, +114h, +54h) or raise the
"ingame.formation_areyousure" prompt (00531B00 with the callback 00535F30); formations apply shape
3/1/2 (0070EFD0) and broadcast a FORMATION_SET message (0077A080, 0077C880, 005347C0; host calls
only, those addresses are in the cc9 keep-out). While committed the frame +A8h fades (alpha -= step
* 2.0 once +164h passes 0.5f; eight unused 004BEC00 calls; at alpha <= 0 the menu closes to 128h
through 00533A30) and the highlight alpha +15Ch pulses by step * 7.0 between 0.01f and 0.99.

Uncertain: the meaning of the widget +38h() gate (taken as "is visible", sweepB3's hook) and of the
+54h/+50h director slots (stance getter / setter).

## 005B3470 BSP_HudMapScreen_BuildPlayerList (segment 20)

`cc13_sweepB9_hud_map_build_player_list_005b3470`. `__fastcall(ECX = HUD map screen)`, RET; vtable
00CF0118 slot 13. Pushes the speech duck (00A7D120(1)), hides the map panel (005BCA30(gui+58h, 0))
when a live local player is in a running session, binds All_Group (+94h) and 23 more children (the
templates and icons hidden after each name's release; map_class_Icon under +90h), rebuilds the
pointer vector +D4h (checked bounds, 005A0AC0 erase of the whole range, 005A5A80 resize to 11) with
popup_FrameBox as element 0 (hidden), caches the camera position from 00644230's vtable +C4h,
centres the GUI cursor (00AA1FA0 with 0.5f, 0.5f by value), sets [00E188A8]+19C4h, clears the map
state and reloads the background through sweepB5's 005B2D70, hides the counters and the hint,
triggers MAPPERMANENT or MAP1STUSE, and walks the scene root 00AD56D0([00F8C210]): every child of a
"Terrain Visibility Group" node that is not "BushGroup" (inline native-string compare, then
_stricmp) gets visibility 0.0f, the matching nodes are collected in the pointer array +570h (grown to
max(2*capacity, 1)) and re-registered under +0Ch (00B6D890) before the array is emptied. Ends with
the pick state cleared (+12Ch, +58Ch..+594h, +59Ch, +610h) and +598h = 1, +60Ch = 1.0f.

Uncertain: 00644230 and 00AD56D0 are trivial getters taken as the camera object and the scene root.

## Not reconstructed

- 006D5220 BSP_AirField_ReadHangarAndMarkerProperties (segment 31): cc9 lane (as sweepB8). No record.
- 00674B80 BSP_HudSupportManagerScreen_Enter (segment 27): set aside, not started in code. 1216
  instructions with 53 callees and long inlined std::list work (the +3BCh list cleared with _free per
  node, list assignment 00665D20 with checked iterators, node buys 00659AA0 with the 00662A70 length
  check, supply-class lists from 0084D560 copied node by node), the same class sweepB3 set aside for
  0066A8A0; its update 00675C40 is cc9's air-operations code. No record.
- 00673A10 (segment 27): cc9 lane. The support manager's purchase, called only from cc9's
  air-operations update 00675C40; it sends the shipyard / airfield spawn messages A7h/A8h/A9h that
  game_hosts_gunnery.cpp and shipyard_production.hpp model (its idle search 0067442D is cited there).
  No record.
- 005DF510 BSP_MultiGameLobby_ReadVehicleClassTypes (segment 22): deferred Lua data-table reader (the
  class sweepB6 deferred with 007A74F0 / 005E12A0). 1391 instructions: an 8-way jump table on the
  mode over the 434h-byte scene records, LuaObject reads of VehicleClass[...].Type (00B67980,
  00B67800, 00B662B0) into a pooled string list with alias-node copies (004C3020, 004CE6F0,
  004CE780). No record.

## Not reached in this pass (no record)

In order: 00538F80, 005B45C0, 0060ABD0, 007A91A0, 00670020, 00822C20, 00627960, 005DB050, 005ADDA0;
then 005A3210, 00617470 and 00776930.

## Reconstructed

006179D0, 0056F320, 0067F4D0, 005D73A0, 007884D0, 00536A90, 005B3470.
