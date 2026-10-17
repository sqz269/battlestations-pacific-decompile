# cc13-sweepB6: sixth pass of the segment 15-51 leftover sweep

Packet cc13-sweepB6 (2026-10-16), continuing docs/CC13_SWEEPB5.md: its "Not reached in this pass"
list, taken in its order (005E2750, 005A7C20, 00623710, then the rest of the sweepB4 order, smallest
byte span first). 0061B0F0 and 00642040 were opened and deferred (see "Deferred"); 005A3210 and 00617470 were left
for last and not reached. Skipped as the packet says: the deferred Lua data-table readers (007A74F0,
005E12A0), 00824B60 (cc9), 0081E350, 007A84D0, 0064AE20, 0059D4A0, the library sets and every cc9
lane in the CC13_SWEEPB*.md docs. Each function ends as reconstructed, recorded unreferenced,
deferred (with the reason), or not reached.

Names are hypotheses, not recovered symbols. Files: `include/bsp/cc13_sweepB6.hpp`,
`src/cc13_sweepB6.cpp` (registered in `cmake/startup.cmake` on bsp_core), this document. The host
interface `Cc13SweepB6Host` extends sweepB5's `Cc13SweepB5Host`, so the sweepB..B5 hooks are reused
as they are, and two earlier reconstructions are called directly:
`cc13_sweepB5_weapon_group_gun_sight_status_00547480` (from 00547A20) and
`cc13_sweepB2_movcam_seed_angles_from_vehicle_camera_007a4ef0` (from 007A5110). 007A5A00 and
007A6200 call this pass's 007A5110 directly. No host file was edited, nothing is wired and no game
was run. Ghidra was read only.

Method, per function: `bsp.py lookup` / `docs-for` and `git grep` of src/ and include/ for the
address and the name (none had an implementation; some had a host hook only); a rel32 + absolute
dword census (`tools/callsite_census.py`, positive control 00576B10); bodies read from the disk
bytes with `local/sweepB6_dis.py` (Capstone; x87 lines carry their raw bytes and the DC/DE
register-register forms are decoded from the encoding, never from listing text); every float/double
constant checked against the image; stack slots traced by hand against ESP after each push
(including the argument slots the image reuses as float scratch and the interleaved argument
pushes of chained calls). EH frames are noted but unwinding is not modelled. Inlined STL code
inside a game function (checked-iterator `00BF6713` tests on game data, inlined push_back, find,
erase, list and tree teardown) is reproduced; called STL instantiations are host hooks.

x87 note: as in sweepB..B5, float expressions keep the image's operation order with a float store
wherever the image stores to a float slot; intermediate x87 values are kept in double; NaN
behaviour follows the image's branch (each row names the compare and jump).

RNG: no function here draws from the shared RNG 00BD2F10.

## Reconstructed in src/cc13_sweepB6.cpp

| Seg | Address | Name (hypothesis) | ABI | What the code does |
|---|---|---|---|---|
| 16 | 00547A20 | BSP_HudWeaponGroup_LayoutSightsForKind | thiscall(screen, int kind) RET 4 | Returns early without groups, a first group entry or a unit (+40h). Clears byte +0D5h, refreshes the unit pose (00414DB0) around reading its position, takes pitch/yaw from 00521370(unit+0ECh); then 00412E20 runs on half the view FOV and its result is dropped (fstp st0). For each group entry's guns ([e+48h], +44h next) matching the kind (jump table 005482B0: 2..4 vtable +5Ch(24h) and 005459B0, 5 and 005459E0, 7/9 the class [[gun+3F4h]+80h], 9 also sets +0D5h): clones a marker (+0FCh into {+0E4h}, 00545CB0) and a bar (+0D8h into {+0F0h}, 00545D10) when the slot is new; barrel path (!00727D30(gun, 0) && 00729A40 && 00729F10) shows both and sets the bar to 1 - min(r, 1) with r = [+414h][i] / [+418h][i] (00727C30 index; ucomiss/lahf/test 44h/jnp so NaN divides; fcomip/ja so NaN keeps r), state 6; otherwise (byte +1Ch clear) 00547480 gives the state, +0E0h latches state 4, and the reload bar shows while [gun+450h] > now - [gun+474h] (fcomip/jbe) as 1.0 (g < 0.5 or NaN, comiss/jb) or (now - [gun+474h]) / g with state forced to 6; marker vtable +88h(state, 0, 1.0f); the slot index goes into list A or B by byte +0Eh of record [[e+538h]+94h][[gun+38Ch]] (005471B0 grows it; 00442190 push_back). Unused slots hidden. Both lists laid on arcs: a = float((i - (n-1)*0.5) * 0.2094..), x = sin a * 0.063 + [+48h], y = [+4Ch] - cos a * 0.084 (A, fsubp) or cos a * 0.084 + [+4Ch] (B); 00AA7DC0 with z 0 (marker) and -0.1f (bar). Vectors freed (00BF65AC). |
| 20 | 005A7C20 | BSP_HudMapIcon_RefreshPowerUpIcons | thiscall(icon, template, char second) RET 8, EAX count | Rate gate per list (stamp +44h / +40h; fld; fadd 1.0; fcomip vs [00F876A4]; ja, NaN not above): not above with a template returns the list count at once. Releases the list (+88h or +0C8h) through vtable +4h(1) and an inlined erase. With a unit (+4h): 008E9AF0([00F88C30], +190h, +150h, +160h, +170h, +180h, 1, 1); entries of +190h, then those of +150h whose deadline [+184h][i] - now > 0 (fcomip/jbe), that 008E62A0 accepts for the unit go into a local vector once (inlined find, inlined push_back, 005A6580). Without a template the vector is freed and its count returned; with one each entry gets a clone (00AAB4C0(template, 0)), +34h(0), the icon state row k = [[icon+4]+54h] (or [[00E188A8] local player row]+28h when k >= 2) through 00AB64C0, +88h(1, 0, 1.0f) and an inlined push_back into the list (0043DB60); then the list count. |
| 20 | 005A8600 | BSP_HudMapScreen_Destruct | thiscall(screen) RET | The GUI_map destructor body: vtables 00CF0134/00CF0130/00CF0118; with +56Ch the Terrain Visibility Group children that are neither BushGroup nor TreeGroup get 00B6DA70(child, 1.0f, 0), the groups go through {+570h} to 00B6D890 and 0059FCE0(0); game+19C4h = 0; pages +8Ch/+90h removed (00AA31F0 on 004C12B0()); the +498h list values and the ref-pointer slots (+130h/+2B4h x61h, +438h x6, +450h x5, +464h, +468h/+474h/+480h/+48Ch x3, +4A4h x3, +108h x6, +5A0h..+5A8h) released by InterlockedDecrement (IAT 00CE2220) and vtable +0h at zero; +128h = 0; then the member containers in the image's order (trees +600h/+4F4h/+78h through 005A6350/00538360/005A5E40 plus the head, vectors +580h/+514h/+504h/+4E0h/+4CCh/+4B8h/+0F4h/+0D8h, the inlined pointer-array destructor of +570h with its capacity < 0 reallocation, 00BF6989(+544h), the +498h list nodes) and 00695870(+64h), 0093CC80(+8h), 004F71A0(screen). |
| 23 | 005F24D0 | BSP_OptionsScreen_BuildSettingsPage | thiscall(screen, title, labels, label_count, value_rows, row_kinds, char list_pages) RET 18h | Clears both list boxes, 00549620([00E1930C], 1), 005189B0(title), +0ACh = value_rows. Value rows (template by kind 1/2/3/other = +9Ch/+98h/+94h/+90h): listener screen+0Ch, +0D8h = 1, arrow children listeners for kinds 1 and 3, the main_Text label from 005F0220, the 00CF385C table lock (byte +188h clear: row+77h = 1, 00AA6BC0(row, 0, 0)), 00A9D750 into +88h, +34h(1). list_pages: page rows from +8Ch into +84h. 005F1620 arrows for [00E19650] mode 1 (row 0) or 3 (rows 0, 1, 5) while [00E198C4] is set. list_pages: +84h listener/selection [00E19630 + [+0A8h]*4]/+80h(1); +88h listener, +34h(1), byte +14Ah = 0; list_pages: 00A9CDF0(+84h, +84h, +88h) and the 0054B530 bar (-5Eh "globals.select", 0 "", 0 "", -5Dh "globals.back", 0 ""), +254h/+258h hidden; else the sweepB5 005F3B50 tail. Byte +251h cleared when set, else +250h. |
| 23 | 005FA0F0 | BSP_HudOrderScreen_BindLayout | thiscall(screen) RET | No rel32 caller; vtable 00CF3B7C slot +14h (00CF3B90). Twenty bindings in the image's order: each name is built from its literal, the field (+0Ch..+58h, cancel_Icon at +38h after the repair glass/frame +3Ch/+40h) = 00AA7E00([screen+8], &name, 1), the name released. |
| 27 | 0065D220 | BSP_HudSupportManager_RebindSlots | thiscall(screen) RET | For the four slots: two clones of the template +138h (shown around the loop) bound with their named children into +7Ch, +8Ch, +9Ch, +0ACh, +0BCh, +0CCh, +0DCh, +108h, +118h; parts hidden; vtable +44h offsets A - v, v - A (three), A + v and -A - v with A = [+2A4h] and v the running [+2A8h] sum (float stores); +0BCh vtable +88h(3, 0, 1.0f); the circle_full_<j>_Section children hidden for j < 4. |
| 46 | 007A5110 | BSP_MovCam_EnterMode | thiscall(camera, char snap) RET 4 | Prelude resets and the +1BCh FOV / near plane; the +3E4h keep rule. Mode 0 view copy; 1 follow (early return without target) with the snap approach f = 1 - s*0.25/len, 00794130(-1.0f) and the 007904D0/00799750/00794000 sequence; 2 vehicle camera (early return without target): 007A4EF0, the 007A0AC0 map, the class camera element (0047E540 translation and the 005CC5E0 mode text by value) or an identity with m[14] = class+0A0h, angles from 007A4E10/00418170, +74h = RotX*RotY*(+420h)*target, near plane 0.1f; 3/4/6 orbit (3: -pi/2 and the HUD offset; 6: the scaled forward point) with the snap resolve; 5 path (007972F0 or 007B04C0 + 00794070 by value). Tail +170h vtable +0Ch and 00445790. |
| 46 | 007A5A00 | BSP_MovCam_BeginPathWalk | thiscall(camera, float start, char keep_points, float duration, path_name, variants) RET 14h | Takes the path name and variant vector, mode 5; without keep_points seeds two inline-built path point records (vtable 00D043D8) at and ahead of the camera; builds the path object (00BF55BE, 00BF79F0, 0047B660), parents it with an identity frame, adds every point (world-transformed when owned) with the speed carry (comiss 0, v; jb, NaN carries), derives knots, t = 007B05E0(count - 1), and with duration > 0 sets the speed limit or appends a moveSpeed interpolator; then 007A5110. |
| 46 | 007A6200 | BSP_MovCam_SelectDeckCamera | thiscall(camera, int index) RET 4 | Not a Ghidra function (inside 007A5A00's Ghidra body; caller 007A8203). index -1 -> mode 1, else mode 2 with that index; 007A5110(camera, 0) only on a change; then +170h vtable +0Ch. |

## Unreferenced (recorded, not reconstructed)

| Address | Name | Evidence |
|---|---|---|
| 005E2750 | BSP_MultiGameLobby_FormatSettingsSummary | No rel32 and no absolute dword (callsite_census, positive control 00576B10); about 700h bytes of string formatting, over the sweep's size limit. |
| 00623710 | BSP_NativeString_ReplaceDateTokens_00623710 | No rel32 and no absolute dword (same census); cc13-sweep60 had already noted it unreferenced; large. |

## Deferred (no record)

- 0061B0F0 BSP_HudPowerupManagerScreen_RebuildLists (seg 25, 583 instructions): opened. Its game
  part is short (008E9AF0 on [00F88C30], two per-team icon state vectors, 00AB64C0 icon states,
  0061A400 AssignLists); the rest of the body is inlined STL: string-keyed case-insensitive map
  lower_bound walks with inlined `__stricmp` (beside 00613900), the int map find 00614060 and the
  subscripts 006170F0 / 006171B0 / 00617870. Recorded as library-dominated; left for a pass that
  takes the map instantiations as hooks wholesale.
- 00642040 BSP_InGameHudMarkers_BuildUnitMarker (seg 26, 649 instructions): opened and read to
  00642497. It calls two pool members not yet reconstructed, 0063C620 (box marker) and 0063D1E0
  (unit marker widgets), passes a 16-byte colour and a float3 by value, and pushes the dword at
  [esp+10h] whose low byte is the "no flag" byte and whose upper bytes are left from the unit
  pointer (as 00528240 in sweepB5). Left so that one packet takes 00642040 with 0063C620 and
  0063D1E0 together.

## Not reached in this pass (no record)

In the sweepB4 order: 005F65C0, 006191A0, 005F0CD0, 005BE240, 00626C50,
005A6AD0, 0065BF00, 006B9450, 00640D70, 0063C620, 0053F2D0, 0078A430, 0063D1E0, 005A9DB0, 00789560,
006D5220, 006003A0, 005EADC0, 00776930, 005E3580, 00597870, 006179D0, 0056F320, 005E7B10, 0067F4D0,
005D73A0, 007884D0, 00536A90, 00674B80, 005B3470, 00673A10, 005DF510, 00538F80, 005B45C0, 0060ABD0,
007A91A0, 00670020, 00822C20, 00627960, 005DB050, 005ADDA0; then 005A3210 and 00617470.
