# cc13-sweepB5: fifth pass of the segment 15-51 leftover sweep

Packet `cc13-sweepB5`, Claude worker in the cc13 lineage, 2026-10-16. It continues
docs/CC13_SWEEPB4.md: the pool is that document's "Not reached in this pass" list (74 bodies),
taken in its order (smallest byte span first). Skipped as the packet says: sweepB4's four deferred
functions (007A87B0, 006697E0, 00641910, 005E12A0), the sets sweepB3 put aside (005AAC00,
0066A8A0), 0081E350, 007A84D0 and 0064AE20 (cc9), 0059D4A0, and every cc9-lane item listed in the
CC13_SWEEPB*.md documents. Each function is closed with one outcome: reconstructed, unreferenced
recorded, deferred (with the reason), or not reached.

Names are hypotheses, not recovered symbols. Files: `include/bsp/cc13_sweepB5.hpp`,
`src/cc13_sweepB5.cpp` (registered in `cmake/startup.cmake` on bsp_core), this document. The host
interface `Cc13SweepB5Host` extends sweepB4's `Cc13SweepB4Host`, so the sweepB..B4 hooks are reused
as they are, and three earlier reconstructions are called directly:
`cc13_sweepB_entity_is_hostile_to_party_005457c0` (from 00547480),
`cc13_sweepB3_hud_map_create_building_quad_model_005a2340` (from 005A91E0) and
`cc13_sweepB4_hud_map_create_cone_model_005a2730` (from 005A9770). No host file was edited, nothing
is wired and no game was run. Ghidra was read only.

Method, per function: `bsp.py lookup` / `docs-for` and `git grep` of src/ and include/ for the
address and the name (none had an implementation; some had a host hook or a host call only);
bodies read from the disk bytes with `local/sweepB5_dis.py` (Capstone; x87 lines carry their raw
bytes and the DC/DE register-register forms are decoded from the encoding, never from listing
text; absolute operands are annotated with their image values); every float/double constant was
checked against the image; stack frames mapped by hand from the ESP offsets (including the
aligned EBP frames and the matrices built in place on the stack). EH frames are noted but
unwinding is not modelled. Inlined STL code inside a game function (checked-iterator `00BF6713`
tests, inlined push_back, list and tree clears, native-string copies) is reproduced; called STL
instantiations are host hooks.

x87 note: as in sweepB..B4, float expressions keep the image's operation order with a float
store wherever the image stores to a float slot; intermediate x87 values are kept in double; NaN
behaviour follows the image's branch (each row names the compare and jump).

RNG: no function here draws from the shared RNG 00BD2F10.

## Reconstructed in src/cc13_sweepB5.cpp

| Seg | Address | Name (hypothesis) | ABI | What the code does |
|---|---|---|---|---|
| 15 | 00528240 | BSP_ControlLayoutScreen_Enter | thiscall(screen) RET | Game +61Fh picks Main_Listbox or ingame_Listbox for +18h (00AA7E00 on +10h, names built by 0041E870) and hides the other; 00518D60(6, 0, 0, FE.opt_title_control) or 004C9680. Three rows cloned (00AAB4C0) from MainListbox_Text with the C strings at 00E08664 (00ABBE50) and inserted (00A9D750); 00A9AC40, 00A9C7C0([00E08670]), vtable +60h(1)/+80h(1), 00A9CF30(1, 1). Two 3x3 text grids: the child named [00E08674 + 0Ch*i + 4k] of [+1Ch + 4j] gets 00ABAED0(&00E18DC0[v][j][k], 1), v = i or (i == 0) with byte +28h. 0054B530 with five triples; ingame_FrameBox of +14h gets vtable +34h(game+61Fh) (see the divergence note). |
| 16 | 00547480 | BSP_HudWeaponGroup_GunSightStatus | thiscall(screen; unit, gun, mode, origin, aim_a, aim_b) RET 18h, EAX 0..5 | gun+490h = 0; +5Dh -> 0; 00729F10 false -> 1; screen+40h null -> 2. Kind-7 guns: 00729A40 false -> 5; heading from unit vtable +50h, w = -00438B10(aim_b, heading) wrapped to (-pi, pi] (FCOMI + JBE, FCOMIP + JB), 0085AB50(gun, w, pi/4), FLT_MAX equality -> 2, \|00438B10(r, gun+480h)\| > pi/4 -> 3 else 4. Other guns: mode 5 aims through 0042D7E0/00521370/00427EB0/00957BD0/00955830, other modes through 004B4D80/00957740/00427EB0/00955630 (false -> 2). Kind-1Ch screen unit: range test (FCOMIP + JBE) unless 005457C0 hostile, then the 0098ADD0 segment (00470470/00419510/0042E630/unit vtable +20h/004704B0; 00414260 and 004142A0 reproduced) -> gun+490h = 1 and 2 on a blocking kind-5 hit. 005472D0 -> 007F60A0 false -> 2; bearing limits 3 deg (mode 5, x87) or 5 deg (COMISS) on 00438B10 of +484h/pitch and +480h/yaw -> 3 or 4. |
| 19 | 0056DD80 | BSP_MultiSessionBrowserScreen_RefreshFilterRows | thiscall(screen) RET | Rows i = 0..8 at +54h + 4i: nike_Icon and setting_2_Text hidden (vtable +34h(0)); i <= 4 shows nike_Icon on flag +10h+i; i == 8 passes byte +18h to nike_Icon of [+74h]; i = 5..7 show setting_2_Text and, with the flag, write 004260B0([+244h/+248h/+24Ch]) through 00ABAED0 on rows +68h/+6Ch/+70h, else 00ABBE50("globals.no", 1). |
| 20 | 00593CA0 | BSP_MissionScoreRecord_Assign | thiscall(dst; const src*) RET 4, EAX dst | The record's member-wise copy constructor (the EH state rises as each member is built): dword/float copies, 39 member copy constructors (00590080, 00593310, 00593480, 00591D90, 0058D4B0, 0058D540, 0058AAC0, 0058B870, 0058D5D0, 0058D660, 0058D6F0, 0058F370) at their offsets, three inlined native-string copies at +254h/+25Ch/+264h (memcpy of dst.length bytes, no terminator). |
| 20 | 005A4200 | BSP_HudMapScreen_Exit | thiscall(screen) RET | 00A7D120([00F8BBD8], 0); with root +56Ch: restores visibility (00B6DA70(child, 1.0f, 0)) under each "Terrain Visibility Group" (BushGroup skipped, TreeGroup restored, other children unless a 00B74310-kind model whose effect name contains "wavedamp"), collects the groups in the pointer array +570h (0059E5E0), 00B6D890 and 0059FCE0(0). 005BCA30 gate; 00B6DFA0 on the model vectors +4C8h/+4DCh then the inlined erase; +53Ch/+540h reset; 00529490/00529350; map +4F4h walk (+53Ch/+540h from the first set dword +48h..+60h, value vtable +0h(1), 006952A0, 00535110) and inlined tree clear; 005A3AA0, 005A09A0, +4F0h release, game+19C4h = 0, 00772A40, 0054E610, six pointers +108h..+11Ch deleted. |
| 20 | 005A91E0 | BSP_HudMap_BuildCaptureRadiusModel | thiscall(screen; scale, xz*, height, land, capture, band) RET 18h | Grows the pooled vector +4B4h (005A2340 seed via 005A7A10, clones via vtable +10h(6, 0) into operator-new 1Ch blocks, 005A6650 when full), sets block +4..+18h (land^2, (land+band)^2, capture^2, (band+capture)^2, xz), registers cCirclePosition (00B18B00) and four radius parameters (00B18B20) on 00B732C0(00B74640(model, 0), 0)+20h, frame = 00413920(diag(scale,1,scale,1), translation), vtable +38h, 00B6D890, index += 1. |
| 20 | 005A9770 | BSP_HudMapScreen_AcquireConeModel | thiscall(screen; frame*, heading, scale, stretch, range, cone_angle, layer) RET 1Ch | Pool +4DCh as 005A8F50 (005A2730(5000.0f, cone_angle) seed). R = diag(scale,1,scale,1) * 00B646E0(-0.0f - heading). Layer 0Bh: ((T(0,0,-range*250000) * diag(b,1,b,1) * T(0,0,range*250000)) * R * frame) * T(0,3110,0) with b = stretch*6+1; other layers (R * frame) * T(0, (float)(uint32)(layer*10+3000), 0). vtable +38h, 00B17310(material, layer), 00B6D890, index += 1. |
| 20 | 005B25D0 | BSP_HudMapScreen_UpdateMarkerModels | thiscall(screen; dt, paused) RET 8 | Selected objective from [[00E198C4]+60h]; 005A3AA0; +594h from [00F876A4]; for every shown (008DA140) and active (008DA130) objective of 008DA180's list, each revealed (008DACD0) entry gets a 50h-byte marker (vtable 00CEFFE8, sweepB's 005A1B40, 00B179F0 colour from the global config +1A8h..+1B4h, icon from 005A22D0(unit vtable +140h)) pushed by 005A7A80. Spin steps 0.3925*dt and dt*0.785 advance +58Ch/+590h and each icon +3Ch unless paused (wrap at 2pi, FCOMI/FCOMIP + JBE). Per marker: alpha 1.0/0.4, angle and position (unit vtable +0C4h, the +1Ch unit or the stored point), frame = rotY * diag(s,s,s,1) * T(x, i + 1300 + y, z) -> m+8h vtable +34h. |
| 20 | 005B2D70 | BSP_HudMapScreen_LoadBackground | thiscall(screen) RET | The map's full refresh: background layer "terkep_alap_hdtv:Layer2" (00B6E750, 00B6DB70, element +34h = -90.0f); +4B0h = _ftol(zoom*2*3) % 3; unit layers via 005B2CB0 for the game's alive kind-2 units and the local player's visible units (vtable +0B4h / 008DDF90, 0059D940 and the kind gates); stale map entries updated (005ADDA0 while +5ACh > 0) or deleted and erased (006952A0, 00537F20); order overlays 005A9DB0 over three player lists; model visibility from the use counters (00B6DA70); 0059F9B0; the +600h tree cleared unless held; the speaking unit 005A2650 while +540h + 3 > zoom. |
| 22 | 005D18A0 | BSP_MultiChatScreen_Update | thiscall(screen; float) RET 4 | The argument slot is overwritten by game+21ECh at entry. Input toggle (004C43C0 F9h, gates, the 004BA6D0 device chord) -> 005D11F0(+64h ? 3 : 4); otherwise close to mode 3 ([00E198B4]+3Ch or 004D92B0(4Bh); modes 1/2 always) or, without the manager, mode 0 / 005D11F0(1); mode 4 parks widgets 28h/55h/57h/5Dh/15h first (00442190, 004D6480). Tail: 00A96F40, 005D0C60, and in mode 3 the per-row timer fade (alpha = timer while 0 <= timer < 1) and the sweep that erases expired timers (005CF8A0) and detaches/deletes their rows (00AA83A0, vtable +4h(1)). |
| 22 | 005D26D0 | BSP_MultiErrorScreen_Enter | thiscall(screen) RET | Binds Message_Text, MessageLower_Text, dialog_BG_FrameBox, Options_Listbox and the Yes/No/Accept texts (00AA7E00); +2Ch vtable +84h; +50h from byte 00F88A30 (00AA0F70(004C12B0()) when set); a glyph wide string (00531030 x2, 004261A0, 004C5E60) set on the three option texts by 00531130 with 00AA6BC0(0, 0) and their +1B4h..+1BCh layout, 00AB80C0 and an empty 00ABBE50; message vector entries 0..2 (3/4 in the two-button case +54h == 4 with [00E08898] == 4) through 00ABAED0; listbox clear (00A9AC40, vtable +60h, 00A9BF90), show/hide and inserts (00A9D750) and, with +50h == 0, the listbox setup (00A9C7C0, 00A9AC70(006964B0), 00A9AC40, vtable +40h(0.0f)); 005D24F0; the command owner +1Ch/+4/+5 and 004F83B0. |
| 23 | 005F3B50 | BSP_OptionsScreen_BuildContentPage | thiscall(screen) RET | Clears both listboxes, 00E19650 = 7, 00549620, title FE.opt_title_dlc (005189B0); one cloned row per 00F8A304 entry (arrow listeners 00AA6BC0, main_Text 00ABB000(entry, -1.0f, 1), row+77h, 00A9D750, hidden arrows on entry+12h); listbox setup, list floats 0.003125/0.0027777778/0, 005F0CD0, empty help line 0054A0C0, +254h/+258h visibility, checked list-iterator value to screen+8 vtable +8h, +251h/+250h, 005F2D20. |
| 26 | 00621C60 | BSP_RewardsScreen_RefreshPage | thiscall(screen; page) RET 4 | Title 005189B0(&[00E196C0][page]) after the subscript check; page widgets +40h..+54h vtable +34h(i == page); 0054B530 with five empty triples when +258h is set, otherwise (A2h, globals.continue, 0), (c, globals.navigate or "", 1) and, on page 1 with 0061D8B0, (A7h, globals.playerreview, 1) when the active xuid (00A3EB00) differs from the row's (0061D710) and (A5h, globals.showgamercard, 2); the rest empty. |
| 27 | 0065FBF0 | BSP_HudSupportManagerScreen_UpdateSlotHighlights | thiscall(screen; dt) RET 4 | Empty list -> +2B0h = 0. Walks the list +15Ch: a non-begin node whose previous slot's circle_HL_Icon alpha < 0.5 (COMISS + JBE) stops; a node not fully lit (UCOMISS + JNP, NaN not lit) fades circle_HL_Icon, circle_s_Icon and icon_s_Icon to min(dt*4 + alpha, 1) (vtable +4Ch). The previous node fully lit (UCOMISS + JP) -> +2B0h = 0. |
| 27 | 00660B10 | BSP_HudSupportManagerScreen_BeginScrollForward | thiscall(screen) RET | +2C0h = 1, arrows +100h/+104h shown; the six 4-slot widget arrays (+7Ch, +8Ch, +9Ch, +0CCh, +108h, +118h) shift down (+154h clear, saving +13Ch/+140h) or up; the freed slot gets a 00AAB4C0 clone of +138h and its icon_L_Icon (vtable +88h(3, 0, 1.0f)), circle_HL_Icon (hidden), number_Text and the two circle_small groups (hidden), vtable +44h offsets from +2A4h/+2A8h; 0065E870(screen, new, 0.0f); +2B8h = 1.0f. |
| 27 | 00661240 | BSP_HudSupportManagerScreen_BeginScrollBack | thiscall(screen) RET | The mirror over +0ACh/+0BCh/+0DCh: +2C1h = 1, arrows +0F8h/+0FCh hidden, saves +148h..+150h, the opposite shift, the same child build into slot 0 or 3, 0065E870, +2BCh = 1.0f. |
| 27 | 00668770 | BSP_HudSupportManagerScreen_RebuildUnitMarkers | thiscall(screen) RET | Clears the unit list +1ECh and the unit -> marker map +3FCh (value vtable +4h(1), inlined tree clear with 00659810); for each local-party unit of kind 46h/9/45h with its weapon block populated: push_back into +1ECh (00659C30/00663050) when absent from +1F8h, a marker cloned (00AAB4C0) into the map (00667E30) when absent; erases map entries whose unit has +5Dh (00662750; the next entry is skipped, as the image); 0065A2B0; per entry vtable +88h(0 or 1, 0, 1.0f) by the cursor, +34h(1), the unit's cached world matrix refresh, 00658580 and 00AA7D00. |

## Divergences and uncertainties

- 00528240 and 00621C60: an argument (the last vtable +34h flag, the second command-bar
  command) is a dword whose low byte is the value and whose upper three bytes are those of a
  stack slot holding the screen pointer (the image stores the byte over that slot and pushes the
  dword). The C++ rebuilds the same merged dword (exact on Win32); the callees read a byte.
- 005B25D0: when an icon's unit is dead (+5Dh/+5Eh) the position copy is not refreshed and the
  previous marker's position is reused; on the first marker the image would read uninitialised
  stack, the C++ starts at zero.
- 005D18A0: the local std::vector's proxy word is left unset by the image; the C++ zeroes it.
- 00668770: after an erase the iterator takes an already-advanced copy and is advanced again,
  so the entry after an erased one is never tested for +5Dh; reproduced, not corrected.
- 00593CA0: the ledger name says Assign; the body is a copy constructor (members are built into
  raw storage with rising EH states). The name is kept so the record replaces nothing else.
- 00547480: 00957BD0 is cc9 lane (docs/CC13_SWEEPC.md) and stays a hook; its return value is
  not read here.

## Skipped (as the packet says)

007A87B0, 006697E0, 00641910, 005E12A0 (sweepB4 deferred); 005AAC00, 0066A8A0 (sweepB3 set
aside); 0081E350, 007A84D0, 0064AE20 (cc9); 0059D4A0; every sweepB..B4 cc9-lane item.

## cc9 lane (no record)

- 00824B60 BSP_UnitInstance_OnWrecked (seg 51): the ship wreck handler; cc9's packet
  cc9_ship_sink_descent owns its sink block 00824FE5..00825086 in game_hosts_units
  (`ship_wreck_sink_00824fe5`), and the body draws from the shared RNG 00BD2F10. Left to cc9.

## Not reached in this pass (no record)

In the sweepB4 order: 0061B0F0 (opened: the powerup-manager list rebuild, mostly called
std::map / string-map work around 008E9AF0), 007A74F0, 0053BCC0, 005E2750, 005A7C20, 00623710, 005F24D0, 007A5A00, 0065D220,
00547A20, 005A8600, 007A5110, 005FA0F0, 00642040, 005F65C0, 006191A0, 005F0CD0, 005BE240,
00626C50, 005A6AD0, 0065BF00, 006B9450, 00640D70, 0063C620, 0053F2D0, 0078A430, 0063D1E0,
005A9DB0, 00789560, 006D5220, 006003A0, 005EADC0, 00776930, 005E3580, 00597870, 006179D0,
0056F320, 005E7B10, 0067F4D0, 005D73A0, 007884D0, 00536A90, 00674B80, 005B3470, 00673A10,
005DF510, 00538F80, 005B45C0, 0060ABD0, 007A91A0, 00670020, 00822C20, 00627960, 005DB050,
005ADDA0; then 005A3210 and 00617470.
