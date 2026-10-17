# cc13-sweepB3: third pass of the segment 15-51 leftover sweep

Packet `cc13-sweepB3`, Claude worker in the cc13 lineage, 2026-10-16. It continues
docs/CC13_SWEEPB2.md: the pool is that document's "Not reached in this pass" list, taken
smallest first after the lead's short list (005A8F50, 005A3610, 00585C50, 005A3830, 00650450,
005EA030, 005E3290, 00578580, 00583E50). Each function is closed with one outcome:
reconstructed, unreferenced recorded, cc9 lane (skipped, no record), or not reached.

Names are hypotheses, not recovered symbols. Files: `include/bsp/cc13_sweepB3.hpp`,
`src/cc13_sweepB3.cpp` (registered in `cmake/startup.cmake` on bsp_core), this document. The host
interface `Cc13SweepB3Host` extends sweepB2's `Cc13SweepB2Host`, so the sweepB and sweepB2 hooks
are reused as they are, and two sweepB reconstructions are called directly
(`cc13_sweepB_movcam_clear_path_records_0079c830`, `cc13_sweepB_movcam_path_point_pair_0078fc60`).
No host file was edited, nothing is wired and no game was run. Ghidra was read only.

Method, per function: `bsp.py lookup` / `docs-for` and `git grep` of src/ and include/ for the
address and the name (none of these had an implementation; several had host hooks only); bodies
read from the disk bytes with `local/sweepB3_dis.py` (Capstone plus the raw bytes; DC/DE
register-register x87 forms are decoded from their encoding, never from listing text); constants
read from the image with their width (`tools/pe_const_read.py`); stack frames mapped by hand from the
ESP offsets. EH frames are noted but unwinding is not modelled. Inlined STL code inside a game
function (list clear, vector erase, checked-iterator `00BF6713` calls) is reproduced; called STL
instantiations are host hooks.

x87 note: as in sweepB/sweepB2, float expressions keep the image's operation order with a float
store wherever the image stores to a float slot; double constants are applied in double.

No function here draws from the shared RNG 00BD2F10.

## Reconstructed in src/cc13_sweepB3.cpp

| Seg | Address | Name (hypothesis) | ABI | What the code does |
|---|---|---|---|---|
| 18 | 0056D660 | BSP_MultiSessionBrowserScreen_FillSessionSettings | thiscall(screen) RET | Rows 0..8 of list +34h (00A9BE00); child `setting_value_Text`; rows 0..4 and 8 get 00ABBE50 with `globals.yes`/`globals.no` from the bytes +10h..+14h/+18h; rows 5..7 get 00ABAED0 with the int +244h/+248h/+24Ch as a string (004260B0) when +15h..+17h is set, else `globals.no` (0041E870); the jump table 0056D974 sends any other row to `ERROR!` (unreachable). |
| 19 | 0056D9A0 | BSP_MultiSessionBrowserScreen_BuildFilterRows | thiscall(screen) RET | +240h = 0; nine rows cloned with 00AAB4C0(+0F4h, +100h) into +54h+4i and shown; `setting_var_Text` gets the .data label 00E086C4[i]; `setting_2_Text` shown; `nike_Icon` +34h(byte +10h+i); 00AA8240 at {+104h, y + +108h, +10Ch}, y += +25Ch; scroller +12Ch: 00683790(0), 006834A0(+240h - 00AA6740(+44h)[1]). |
| 19 | 00578580 | BSP_SessionBrowserRow_UpdatePingIcons | thiscall(row, float ping) RET 4 | `ping_1_Icon`..`ping_5_Icon` (.data 00E0874C): shown and counted when !(ping < registry(00576B10)+A4h+4i), else hidden; level = count < 2 ? 2 : count < 4 ? 1 : 0; each icon vtable +88h(level, 0, 1.0f). |
| 20 | 00583E50 | BSP_MainMenu_ClearObjectiveRows | thiscall(screen) RET | +2C8h set: hide it, clear it, hide +244h/+248h. Byte +2C4h set: delete +2B8h..+2C0h (vtable +4h(1)), delete the widgets of the lists +280h/+298h/+2A4h, clear the three lists (operator delete 00BF65AC). Always: +2C4h = 0, 00AA8240(+1B8h, {+558h + 0, +55Ch + 0, 0 + +560h}), 00AA6BC0(+1B8h; screen+40h, 0). |
| 20 | 00585C50 | BSP_MainMenuScreen_PlayTestSpeech | thiscall(screen, name) RET 4 | 00A83FD0(*(00F8BBE8); tmp, name) ref-assigned to +570h; once (00E19510 bit 1) 00E1950C = 00A7ACF0(`GUITestSpeech`); 00A7B0A0(`Normal`) type; 00A7E490(manager; argument slot, *(*(00E198AC)+58h)+570h, class, type, 1); result ref-assigned to *(*(00E198AC)+58h)+574h; the slot released. |
| 20 | 005A2340 | BSP_HudMap_CreateBuildingQuadModel | cdecl() RET, EAX = 1Ch block | Model `StoredQuad` (00B74EB0/00B75030), geometry (00B73B60/00B73D70), renderer +38h(`pf43.mvfm`) and +5Ch(4, 1, it) stream, 00B73BB0, the +-550 quad written through stream +10h(0,0,0), draw section (00533FA0) with material 00535320(`map_cmd_building.mshd`), +8h/+10h/+18h = 5/4/2, stream +14h, 00B864C0, 00B865A0, 00B73C60, 00B75170(model; 0, geometry, -1, -1). Ref releases are unchecked as in the image (the stream is used after its release). Returns the operator-new block with +0h = model. |
| 20 | 005A3610 | BSP_HudMapArrowWidget_Construct | thiscall(block) RET, EAX | vtable 00CEFFE4; container +4h = 00AA6640(4, "", 0), z 0, hidden, attached to *(*(*(00E198C4)+54h)+94h); eight curves +8h..+24h = 00AA6640(0Dh, "", 0), z 0, hidden, texture = table 00E08884[0], attached. |
| 20 | 005A3830 | BSP_HudMapArrowWidget_SetAiArrow | thiscall(widget; screen, from, to, mode, style) RET 14h | Hides all parts; curve +8h gets two points {off + (p.x - screen+54Ch)/screen+30h + *(00E08890), (screen+550h - p.z)/screen+34h + *(00E08894) + 0.0} with width screen+5CCh * 0.02 (double), off = 1000 when byte *(00E188A8)+61Fh is set; vtable +50h(mode); texture 00E08884[style]; rebuild; shown; +0Ch..+24h hidden. Five stack arguments (the earlier ledger text said three). |
| 20 | 005A3BF0 | BSP_HudMapObject_00cf00ac_Destruct | thiscall(object) RET | vtable 00CF00AC; refs +0Ch/+10h released and cleared; owned +18h, +1Ch, +48h..+60h, +64h..+6Ch, +20h deleted (vtable +4h(1)), +24h (vtable +0h(1)); 005A1C60(object, 0); storage of the 13 vectors +194h..+7Ch freed and zeroed. |
| 20 | 005A8F50 | BSP_HudMapScreen_AcquireLineModel | thiscall(screen; frame, scale, layer) RET 0Ch | Grows the pool +4C8h (first model 005A2C30(5000.0f), then clones vtable +10h(6, 0)) until it holds index +4D8h; model vtable +38h((diag(scale,1,scale,1) x frame) x lift z = (float)(uint)(layer*10+3000)) via 00413920; 00B17310(material, layer); 00B6D890(model, screen+0Ch); index += 1. Returns nothing. |
| 20 | 005C0BD0 | BSP_HudMinimapScreen_HideUnusedMarkers | thiscall(screen) RET | Five vectors +104h..+144h with cursors +154h..+164h: every slot past the cursor deleted (vtable +0h(1)) and cleared, then 005C0390(vector, cursor + 1, 0). |
| 23 | 005E0DB0 | BSP_MultiGameLobbyScreen_Destruct | thiscall(screen) RET | vtables; chat logs 005CFB80; lists +134h/+0B4h 00AA83F0 and their scrollers reset; vectors +1F8h/+244h elements deleted then the inline erase(begin, end); pages +14h/+18h removed (00AA31F0); +2E4h entries 005DACA0 and freed; storage +244h/+234h/+1F8h freed; vtable 00CEB0FC; 004F71A0. |
| 23 | 005E3290 | BSP_MultiGameLobbyScreen_Construct | thiscall(block) RET, EAX | 004F7180; vtables; three scrollers; zeroed fields; 00E19594 = *(*(00E188A8)+60Ch); per option key 0..13 the selected value is found in the option map (local player record +428h when its byte +424h is set, else registry+28h) through 005C6850/005C49C0: key 8 counts up into 00E1957C (stop at registry+60h), other keys count down into 00E1955C[key] (stop at registry+40h+4*key); finally *(00F8A2FC)+75h = 0. |
| 23 | 005EA030 | BSP_MultiMainMenuScreen_LayoutAchievementLocks | thiscall(screen) RET | Places each entry with widget byte +76h set on a 3-column grid from 00AA67F0(+0DCh) (column 0.084375, row 0.102778, doubles) and its `achievement_lock_Icon` child at the template's position; +10h = rows; scroller +6Ch 00683790(0), 006834A0((rows - 3) * 0.102778); 005E9BE0. |
| 25 | 006113A0 | BSP_HudPowerupCarousel_AnimateScroll | thiscall(carousel, float dt) RET 4 | Decays +40h by dt*4.0 to 0; positions the powerup screen's +21Ch/+220h and four row pairs +28h/+48h by the support screen's +2A4h/+2A8h (two formulas by byte +3Ch); 0065E870 alphas; at 0 deletes +21Ch (pointer not cleared), byte +3Fh = 0, shows support+100h/+104h. |
| 25 | 00611770 | BSP_HudPowerupManagerScreen_TickSlide | thiscall(screen, float dt) RET 4 | Same scheme on the manager screen: +1FCh decays by dt*4.0; +210h/+214h by c or 5.0 - c (byte +1F8h); four row pairs +58h/+68h; 0065E870 alphas; at 0 deletes +210h (not cleared), byte +1F9h = 0, shows +0A0h/+0A4h. |
| 25 | 00614320 | BSP_HudPowerupManagerScreen_BindSupportWidgets | thiscall(screen) RET | Copies six widget families per slot from the support screen; for each slot and k builds `circle_full_<k>_Section` and hides that child in +28h+4j and +58h+4j; for k == slot it also keeps it at +38h+4j and sets its vtable +4Ch(0.5f); +0A0h/+0A4h = support+0F8h/+0FCh. (The earlier ledger text "hiding all but the current one" was wrong.) |
| 26 | 00650450 | BSP_HudSubmarineScreen_Update | thiscall(screen, unused) RET 4 | For the controlled submarine: surface ring +18h by 0.2 > unit+127Ch; needle +0Ch from unit+1268h; depth needle +10h from the clamp of unit+100h into the band table unit+1200h..+120Ch (read around 00414DB0 as the image does); gauge +14h from clamp01(unit+127Ch) and the once-initialised 00E19824. |
| 28 | 0068BC80 | BSP_InGameInterface_BuildTacticalOverlayList | thiscall(iface) RET, AL | Returns whether the player's side has overlay content: support sites with supply classes (005AAAF0 copy + 0084D560 per node), units of lists +358h/+88h whose interface (+72Ch / +1188h) vtable +0h answers, lists +364h with a non-empty +774h vector, lists +3B8h by +344h/+330h/+324h. ECX is not read. |
| 46 | 007A7200 | BSP_MovCam_SetPathWalk | thiscall(camera; path, start, end, a, flag, c, name, d) RET 20h | 0079C830 (sweepB), then one 4Ch path record per point from start to end in camera+478h (slot + 2 when the flag byte is 0) filled from 007ACA30/007AF740/007AD070 by kind 0..5 (kind 5 via sweepB's 0078FC60); then 007A5A00(camera; a, flag, c, name, d). |

## Unreferenced, recorded (ledger only)

- 0058CE30 BSP_MainMenu_ShowQueuedNoticePrompt (seg 20): tools/callsite_census.py 0058CE30 found 0
  rel32 CALL/JMP and 0 absolute dwords; positive control 005A3830 found its 4 calls. 243
  instructions, over the ~80-instruction limit, so not reconstructed.

## cc9 lane (skipped, no record)

- 007A84D0 BSP_Recon_PublishedListCursor (seg 46): cited as an open item in cc9's
  docs/SHIP_AI_OPEN_ITEMS.md (the HUD target cycler that may feed the director's fire target,
  "Contract: unread"); left to cc9 like 0081E350.
- 0064AE20 BSP_HudShipScreen45_ApplyUnit (seg 26): packet cc9_ship_screen_parts34,
  docs/SHIP_SCREEN_UPDATE.md sections 12-13, and its gauge contract lives in the cc9 host file
  src/game_hosts_hud.cpp.

## Skipped again

- 0081E350 (cc9's GUNNERY_OPEN_ITEMS), everything sweepB/sweepB2 listed as cc9 lane, and
  0059D4A0 (uninitialised stack floats), as the packet says.

## Not reached in this pass (no record)

The rest of docs/CC13_SWEEPB2.md's list, minus everything above. Instruction counts as there.
005AAC00 and 0066A8A0 were opened and set aside: both are mostly inlined std::set / std::list
manipulation and need a longer turn.

- seg 15: 00528240 (553), 00529D70 (277), 0052A100 (271), 0052A470 (281), 0052A860 (215), 0052B080 (273), 0052BB10 (347), 0052C1B0 (368), 00534870 (241), 00536310 (382), 00536A90 (989), 00538F80 (1560), 0053BCC0 (501), 0053EE00 (310), 0053F2D0 (864)
- seg 16: 00547480 (265), 00547A20 (258)
- seg 19: 0056DD80 (421), 0056F320 (1061), 00570300 (324)
- seg 20: 00593CA0 (355), 00597870 (1081), 005987F0 (259), 005A1C60 (435), 005A2730 (399), 005A2C30 (429), 005A3210 (257), 005A4200 (494), 005A6AD0 (734), 005A7550 (315), 005A7C20 (651), 005A8600 (678), 005A91E0 (405), 005A9770 (345), 005A9DB0 (868), 005AAC00 (239), 005ADDA0 (5027), 005B25D0 (424), 005B2D70 (532), 005B3470 (1316), 005B45C0 (1465), 005BE240 (804), 005C4080 (259), 005C4D40 (371)
- seg 22: 005D18A0 (509), 005D26D0 (618), 005D43C0 (370), 005D73A0 (1155), 005DB050 (4159), 005DF510 (1391)
- seg 23: 005E12A0 (359), 005E1CF0 (265), 005E2750 (630), 005E3580 (1024), 005E7B10 (1206), 005E9BE0 (297), 005EADC0 (1138), 005EBDD0 (331), 005EE520 (387), 005F0CD0 (676), 005F24D0 (633), 005F3B50 (421), 005F65C0 (572), 005FA0F0 (723), 006003A0 (1017)
- seg 24: 0060ABD0 (1578)
- seg 25: 00616C70 (314), 00617470 (255), 006179D0 (1139), 006191A0 (720), 0061B0F0 (576)
- seg 26: 00621C60 (569), 00623710 (631), 00626C50 (275), 00627960 (291), 00637080 (312), 0063C110 (404), 0063C620 (838), 0063D1E0 (863), 00640D70 (615), 00641910 (340), 00642040 (641), 00642C20 (321), 00652C80 (348)
- seg 27: 0065BF00 (259), 0065D220 (657), 0065FBF0 (431), 00660B10 (545), 00661240 (562), 00668770 (261), 006697E0 (258), 0066A8A0 (253), 00670020 (267), 00671D60 (255), 00673A10 (259), 00674B80 (1212), 0067F4D0 (1222)
- seg 29: 006B9450 (818)
- seg 31: 006D5220 (910)
- seg 32: 006DE5C0 (354)
- seg 43: 00776930 (1125)
- seg 44: 007884D0 (1179), 00789560 (928), 0078A430 (729)
- seg 46: 007A5110 (547), 007A5A00 (498), 007A74F0 (469), 007A87B0 (345), 007A8DA0 (315), 007A91A0 (1815), 007ADD70 (252)
- seg 51: 00822C20 (2069), 00824B60 (470)
