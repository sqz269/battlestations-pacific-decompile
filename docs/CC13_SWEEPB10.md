# cc13-sweepB10: tenth pass of the segment 15-51 leftover sweep

Continues the order left at the end of `docs/CC13_SWEEPB9.md`. Code: `src/cc13_sweepB10.cpp`,
`include/bsp/cc13_sweepB10.hpp` (`Cc13SweepB10Host` extends sweepB9's `Cc13SweepB9Host`, so every
sweepB..B9 and pkt642 hook is reused). sweepB's 0059E120 and 005A48C0, sweepB4's 00536310 and
sweepB5's 005B2D70 and 005B25D0 reconstructions are called directly. Every name is a hypothesis, not
a recovered symbol. Nothing is wired into the game; the status of each function is reconstructed and
build-tested only.

Conventions as in the earlier passes: native records are raw byte blocks touched only at the offsets
the image reads or writes; every call the image makes is a named host method at the same point with
the same arguments, including each inlined checked-iterator failure call (00BF6713) and each inlined
native-string temporary (0041DD40, memcpy, pool release 00419CC0/00BD1510). EH frames are noted, not
modelled. x87 reg-reg operand directions were read from the bytes (`tools/x87_dc_form_scan.py`), and
string literals from the PE bytes (`local/sweepB10_cond.py` in the worker tree annotates each pushed
literal from the image).

x87 precision (rule 4c): all of these run on the main thread (GUI and HUD), so every x87 arithmetic
operation is modelled as a separate rounding to float in the image's order (`pc24`). SSE arithmetic
is float as written. Double constants keep their exact bits; where the image stores a double constant
to a float slot before using it (the cursor clamp bounds of 005B45C0), the C++ rounds it the same way.

No function in this pass draws from the shared RNG 00BD2F10.

## 00538F80 BSP_HudFormationScreen_Register (segment 15)

`cc13_sweepB10_formation_register_00538f80`. `__fastcall(ECX = screen)`, RET; vtable 00CED6FC slot
+10h of HUD screen 2Ah. `src/hud_screens.cpp` only catalogues its pages and widget names (a table, no
behaviour), so this is the first body. 004F71D0, the scalar resets (+164h = 0.0f, +170h = 128h, +80h =
1.0f, ...), five pages through 00AA5840(name, 1, 0) into +98h..+A8h and 25 widgets through
00AA7E00(page, name, 1) (map_class_Icon hidden at once; the table in the C++ follows the image's
order, including all_ship_leave_form_Icon -> +DCh after form_gem_Icon). The listbox +ACh gets its
listener (+78h), the input filter 005353A0 and vtable +60h(1). The 3-D scene +8h: 0093CCC0, camera
clear flags 6, quad 1 (11h, white, "guialphawrite.mshd", 2990, w, w, 5) with w = 2 * [00424C40()+434h]
(stored at +178h), then every terrain object of [[00E188A8]+19CCh]+34Ch is instantiated (00B80D70 /
factory +8h(6, 1.0f)), its "...mapzone" children faded to 0 (inline suffix compare with _stricmp), its
world matrix refreshed inline (00414DB0 / 00413920), registered with 0093D2D0 (root, 1) and (r, 0),
r pushed on +194h and parented under the root, the factory released, the instance pushed on +1A0h.
Quads 2 and 3 use the .data extent 00E0CF40; then +184h = +90h squared, 00536310(screen, 2 * +90h),
+188h..+190h = 1400 / 2990 / 1500 and [+174h] vtable +30h({0, 1400, 0}).

Uncertain: none in the control flow; the two std::list push_backs are modelled as the image's buy-node
/ incsize calls plus the two link stores.

## 005B45C0 BSP_HudMapScreen_Update (segment 20)

`cc13_sweepB10_hud_map_update_005b45c0`. `__thiscall(ECX = screen; float dt)`, RET 4; vtable 00CF0134
slot +20h, 1465 instructions. The whole frame in the image's order; the C++ comment lists every step
with addresses. Parts:

- zoom animation 0059AB10, the MAPPERMANENT hint (0068E5B0 / 00690FD0), the controlled unit's role
  check (vtable +13Ch, 00927F10, 0077C470) and the byte 00E19515;
- cursor in world units, the zoom transition (snap or per-value lerps), +5C0h / +5CCh / +5E4h;
- the filter bar +100h and its six slots +108h..+11Ch (clone on demand, position from 00AA6750 /
  00AA6740 with the constants 1.1f, 7/480 and 0.013888889, vtable +88h), the filter label fade;
- the pointer pick (calls this pass's 005A3210 directly), the spectate shortcut (00645160 / 00647300),
  the filter cycle (004C43C0(120h), ingame.filters_* labels);
- the wheel zoom start (targets, the +4F4h map loop, the zoom-in / zoom-out pan target, 0059E120);
- panning (stick and keys, `+524h` cleared on movement), two cursor edge clamps (bounds -0.1f / 1.1f
  rounded from the doubles BFB99999B0000000 / 3FF1999998000000 when byte [[0109CF04]+0Dh] is set,
  else 0.05 / 0.95), the followed unit +524h, the +5F0h override, the cursor warp (00AA1FA0);
- icon units 005A48C0 twice, 005B2D70, the camera block [00E198C4]+F0h, 005B25D0(dt, 0);
- the shader values +1Ch / +20h / +24h / +28h, the multiplayer timer (009052E0, 00772A40, +60Ch),
  map pings (008E2EE0 with the mode in ECX) and the pointer state 00AA0E60.

This supersedes the zoom fragments of `docs/CC13_MAP_SCREEN_UPDATE.md` (`src/map_screen_update_cc13.cpp`),
which modelled the x87 chains in double; the C++ here rounds every operation to float.

Uncertain:
- vtable +34h of the filter widgets receives EBX, whose low byte is the SETZ result and whose upper
  three bytes are stale stack; the C++ passes 0/1 (the widget's +34h is taken to read the byte).
- 00645160 receives DL = 1 with stale upper EDX bits; its callee reads only the low byte (0064507E).
- [00E08890] / [00E08894] (0.5f) and [00E0CF40..] are .data values read through `image_global` at each
  use, as the image reads them.

## 005A3210 BSP_HudMapScreen_UpdatePointerPick (segment 20)

`cc13_sweepB10_hud_map_update_pointer_pick_005a3210`. `__fastcall(ECX = screen)`, RET; sole caller
005B45C0. Returns when the GUI gate (g+5h && g+70h) is closed; 005A1310(screen, 0); the stick (input
+35F4h / +35C4h, each read through a fresh 004BEC00, thresholds +-0.25f) selects two of eight octants
or returns. Over the map +4F4h (checked iterator), each active entry whose unit is not +520h and not of
kind 34h or 2Bh is projected like the cursor (both in map units, each value stored as float); the
octant bits are (0 > dx) + 2 (0 > dy) + 4 (|dy| > |dx|); within the selected octants the nearest
(squared distance, ordered compare against 1e10f) that is not +524h wins. The winner becomes +524h,
byte +528h = 1, and the cursor +554h / +558h jumps to its vtable +C4h position (x, z).

Uncertain: none.

## 00617470 BSP_HudPowerupManagerScreen_UpdatePowerupBars (segment 25)

`cc13_sweepB10_powerup_update_bars_00617470`. `__fastcall(ECX = carousel = powerup screen + 100h)`,
RET; callers 00619C50 and 0061BEA0 (`src/hud_widgets.cpp` calls it through the host hook
`carousel_tick_00617470`). The "register-passed map subscript" note of the earlier passes does not
hold: 00617030 is an ordinary `__thiscall(ECX = map; const key*)`, RET 4, the map being [00F88C30] +
[game+18ECh] * 0Ch + 14Ch. Four bars from the support block's +148h vector around the carousel
position (+0h slot, +4h index): the slot itself, the slots below it walking the index down modulo the
size (unsigned DIV, no zero guard as in the image), the slots above it walking up (wrap to 0 only
when the size is at least 4). Each value is 008E4BB0 of the record, kept when > 0. Then each of the
four widgets sp+38h..+44h is hidden when its bar is exactly 0 (UCOMISS / JNP), else shown, sized with
00ABE6E0(bar, bc+2A4h - off, 0, 1) and faded to 0.5f unless byte sp+20Ch; off starts at bc+2A8h scaled
by the carousel scroll (+3Fh, +3Ch, +40h) and grows by bc+2A8h per bar.

Uncertain: the image does not range-check the carousel slot +0h when it writes bars[+0h]; neither
does the C++.

## Not reconstructed

- 00776930 BSP_SessionHostInfo_CheckDlc (segment 43): game code, not a vendor body, so the library
  exception (rule 2b) does not apply. The body is dominated by inlined library work: 1125 instructions,
  of which most are std::list buy-node / incsize / clear sequences (0076E970, 0076E990, 007767F0,
  007740B0, 00774050, 00776540, 007736A0, 00773740) with their checked iterators (00BF6713), and
  native / wide string temporaries (0041DD40, 00469840, 004C53E0, 004C5E60, 005CF570, 00770E50). The game
  logic is small: run SCRIPTS/datatables/DLC.lua, walk DLCTable, compare the host DLC masks
  (+170h / +174h) with the client's, and build the localized "globals." name list (00A9FAD0) of the
  missing packs. Recorded as library-dominated; no C++ in this pass.

## Not reached in this pass (no record)

Each is a large body; sizes are listing lines / x87 instructions / calls from the exports:
0060ABD0 (1578 / 274 / 191; the player plane control frame with two stream-1 RNG draws at 0060B0CF /
0060B101 per `docs/CC13_FE_7.md`, which asked for a cc9 ownership decision first), 007A91A0 (1815 / 845 /
46 callees, the MovCam step), 00670020 (2147 / 125 / 261), 00822C20 (2069 / 136 / 190), 00627960 (2287
instructions, no Ghidra export), 005DB050 (4159 / 45 / 620: binds, then the scene-record map icons with
radar-shot textures) and 005ADDA0 (5027 / 841 / 450).

## Reconstructed

00538F80, 005B45C0, 005A3210, 00617470.
