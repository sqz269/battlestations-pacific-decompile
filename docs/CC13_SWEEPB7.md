# cc13-sweepB7: seventh pass of the segment 15-51 leftover sweep

Continues the byte-span order left at the end of `docs/CC13_PKT642.md`. Code:
`src/cc13_sweepB7.cpp`, `include/bsp/cc13_sweepB7.hpp` (`Cc13SweepB7Host` extends pkt642's
`Cc13Pkt642Host`, so every sweepB..B6 and pkt642 hook is reused). The sweepB4 reconstructions of
00652C80 and 0053EE00 are called directly. Every name is a hypothesis, not a recovered symbol.
Nothing is wired into the game; the status of each function is reconstructed and build-tested only.

Conventions as in the earlier passes: native records are raw byte blocks touched only at the offsets
the image reads or writes; every call the image makes is a named host method at the same point with
the same arguments, including each inlined checked-iterator failure call (00BF6713) and each inlined
native-string temporary (0041DD40, memcpy, pool release). EH frames are noted, not modelled. x87
reg-reg operand directions were read from the bytes.

No function in this pass draws from the shared RNG 00BD2F10.

## 006191A0 BSP_HudPowerupManagerScreen_RebuildSecondSlots (segment 25)

`__fastcall(ECX = power-up screen)`, RET; caller 0061A6C0 (reconstructed in `src/hud_widgets.cpp`,
which calls it through `second_start_scroll_006191a0`).

- +1F9h = 1; +A0h and +A4h hidden. The direction byte +1F8h picks the arm: 0 saves +74h/+94h/+64h
  into +214h/+218h/+210h, hides +88h, shifts the three columns (clones +58h..+64h, icons +68h..+74h,
  highlights +88h..+94h) up and fills +58h/+68h/+88h; otherwise saves +88h/+68h/+58h, hides +94h,
  shifts down and fills +64h/+74h/+94h.
- The new slot: 00AAB4C0 of the template [[00E198C4]+BCh]+138h, shown; `icon_L_Icon` under it is the
  icon, its `number_Text` hidden; icon +88h(3, 0, 1.0f); for i from the icon's (short) +98h() while
  i < [[00E198C4]+C0h]+1E0h + start (unsigned, bound re-read), two 006170F0 lookups of key i in the
  map +1D8h feed 00AB64C0(icon, value+D0h, value+D4h); the checked index +1F4h into the vector
  +188h/+18Ch gives the record whose +4h key looks up the state word in the map +1CCh (00617870),
  icon +88h(word, 0, 1.0f); `circle_small_01_Group` and `circle_small_02_Group` hidden;
  `circle_HL_Icon` stored and shown.
- Positions through vcall +44h: up arm icon -0.0f - bc+2A4h (SUBSS with 00D7A208 = -0.0f) and clone
  bc+2A4h; down arm bc+2A8h * 5.0 (double 00D7A370) - bc+2A4h for the icon and + for the clone.
  The HUD runs on the main thread at 24-bit PC, so the `FMUL` product and the `FSUB`/`FADD` result
  each round to float (cc13-fix14; it was one double expression rounded once).
  0065E870(bc, clone, 0.0f); +1FCh = 1.0f.

## 0078A430 BSP_NetworkPacketStatistics_FrameUpdate (segment 44)

`__fastcall(ECX = statistics block)`, RET; caller 004CA440 (`src/game_render_frame.cpp` calls it
through `draw_debug_overlay_0078a430`).

- Nothing unless byte +762900h. Every fifth call (DIV on the counter +76296Ch, incremented first)
  rebuilds 1001 rows of 50h bytes from +74F02Ch out of the per-type cells (types 1..E9h, both
  directions, the global maximum +7648BCh/+7648E4h for the first direction only), then 100 rates
  from rows 10, 20, .., 1000 into +76299Ch + 50h*k. The image adds the same row's unsigned fields
  ten times (its source loop never advances; float stores after each add), and the rate is the
  count sum * 0.125 only when the byte sum is above zero (FCOMIP/COMISS with JBE to zero).
- Byte +7649A8h: 007884D0. Unless byte +764934h: ten grey (0.6f) scale lines y = 0.4 - i*1024/S
  with 00711370 labels printed through 004C7070([00E188A8], 10 + 256*i, 729.6f, text) (the text is
  the format string; null text uses 00F871B8), the temporary released; then for i = 1..1000 the
  bar pair of row i (A at x - 0.5, green 0.5f/0.4f; B at x - 0.498, red 0.4f/0.5f, x = 0.001*i),
  the larger value drawn first, the scale +762970h growing by 1024.0 while count/S (S as read at the start of the sample) exceeds the current scale * 0.3, first for A, then for B,
  and every fifth i the rate polylines (green/red 0.9f) from the previous positive rate.
- Operand directions: DE FA (ST2/ST0), D8 F1 (ST0/ST1), DE E3 and DE E1 (ST0 - STi), DE F2
  (ST0/ST2), DC E9 (ST1 - ST0); every division and the y bases are evaluated in double.

## 005A6AD0 BSP_HudMap_CreateUnitMarkerModel (segment 20)

Constructor, `__thiscall(ECX = 1A0h-byte marker, unit, scene root, const float* shadow-fade
source)`, RET 0Ch, returns the marker; callers 00538D60 and 005B2CB0.

- Writes, in image order: vtable 00CF00ACh, the three pointers of each 10h-byte member from +78h to
  +E8h and from +150h to +190h, byte +2Eh, +4h = unit, the model/material slots +0Ch..+138h,
  +18h/+1Ch, +48h..+6Ch, byte +9h, +24h.
- Texture table +14h and model template by unit kind (+C4h; kind 18h uses the parent's kind): class
  6 -> G+468h, kind 18h or class 0Fh -> G+474h, kinds 1Ch/45h/46h/1Bh -> G+480h, 2Bh/34h ->
  G+48Ch, with G = [00E198C4]+54h. Templates: G[130h + 4*kind], kind 1Bh by [[unit+538h]+17Ch],
  kind 10h by a child (class 25h) whose 006E40F0 device is class 31h, kind 1Ch by unit+770h;
  otherwise for class 8 the "unarmed" byte +2Eh (no bomb platforms, no torpedo barrels, no
  positive loadout +C8h) selects G'[1DCh] and G'+48Ch.
- Up to two models (G[...] and G[2B4h + 4*kind]): template +8h(6, 1.0f), node +0Ch, 00B6D890 with
  the scene, generated-model test through 00B74310, "mapmodel" material (005A07B0) with
  "cShadowFade" bound to the source (00B18B20). With more than one mesh, "alphablend" + "cAlpha"
  bound to +140h = 0.0f. Textures through 00B189F0 by the unit's team index, 2 when 00803CE0 reports
  relation 3. The world matrix is refreshed when byte +C8h is clear; the scale 0.001f times the
  world, translated to the unit position raised by 1500.0, goes to both nodes' +34h.
- Always: the +E8h vector is erased whole (inlined, memmove_s of the empty tail), resized to 11 and
  nulled; +118h, +20h, +3Ch, bytes +28h..+2Dh, +38h, +40h, +44h cleared; +30h = +34h = the
  formation leader [[unit+284h]+14h] when 007788B0, else 0.

## 005BE240 BSP_HudMinimapScreen_LoadLayout (segment 20)

`__fastcall(ECX = minimap screen)`, RET; vtable +14h of HUD screen slot 35h. Under the page root
+1Ch: +4Ch = `minimap_compass_Icon`; `minimap_units_white_Group` hidden and its five item icons into
+6Ch..+7Ch, the ship icon's +54h colour into +D0h..+DCh; red, blue, grey, yellow and silver groups
hidden with their ship-icon colours into +80h, +90h, +C0h, +A0h, +B0h; +48h = `minimap_dir_Icon`,
+F0h = `unit_marker_Group`. `src/game_hosts_hud_world.cpp` binds the same widgets in its own way; it
is a host substitution and is not changed.

## 0065BF00 BSP_HudSupportManagerScreen_UpdateMapIcons (segment 27, new name)

`__fastcall(ECX = support-manager screen)`, RET; caller 00675C40.

- Selection marker +128h shown when +5Ch is 0; returns while byte +2DCh is set and +2DDh clear.
- +34Ch set (the list mode): the +438h..+444h ids move to +448h..+454h; with no entries one or all
  slots are hidden by the effective game mode (004BCA50, modes 4..7); modes 4..7 show slot 0 at
  alpha 0.6f with the first entry's frame; otherwise the cyclic list +3E0h is walked from +2C4h
  (clamped to n - 1): the selected entry (+324h) and the entries before and after it fill up to four
  frames (00652C80) and ids (unit word +174h, or -1), the selected unit places +128h (00658580,
  00AA7D00); each filled slot gets +88h(frame), alpha 1.0f or 0.6f by its id, and +1E0h when its
  byte +6Ch is set.
- +34Ch clear: with +1F4h, the selected support unit's 4Ch slot entries (unit+794h, copied through
  00846FD0 and destroyed with 0065BB00) fill the slots with 00652BA0 frames (alpha 0.5f for states
  1/2), or, for class 45h or a class-9 value, the 58h-byte air-ops records of the group (+72Ch or
  +1188h) read under the group lock (EnterCriticalSection with the depth +18h raised) get
  00658620 frames and alpha 0.35f / +1E0h / 1.0f by state and 00658530.
- `frames` is the image's four-int local; a +324h above 3 overruns it in the image too.

## 006B9450 BSP_AchievementTable_Load (segment 29)

Constructor of the 20h-byte award registry (00E19900), `__fastcall(ECX = registry)`, RET; caller
004E3AA0. Clears the vector +0h and the map +10h (006B8C30 head, 006B9120, the vector erase through
004954F0), +1Ch = 0. A private Lua state (00B66BD0, 00B6A020(41h), 00B69D40) runs
`Scripts\datatables\Achievements.lua`; for each string key of `Achievements`: the record from
0050FC30, the key appended to the name vector, Description/GUITexture/Name/Sound into the record's
native strings at +0Ch/+14h/+0h/+1Ch (the inlined assign: reallocate through 00BD1120 when the
length changes, release when it becomes zero, copy length bytes), ID -> +8h, Multi -> byte +24h,
Score -> +3Ch (default 0), XLastAchievementID -> +40h (default -1), Params[1..21] pushed to +44h
(a nil first element pushes 0), registry +1Ch counted, Unlock.Type -> +28h, Unlock.Index values
appended to the list +30h. Every temporary LuaObject is destroyed where the image destroys it; the
state is closed (00B669A0). The state buffer is the frame from +E8h to the EH record (4CCh bytes).

## 0053F2D0 BSP_HudFreeCameraScreen_UpdateEffects (segment 15)

`__thiscall(ECX = burst block, screen+64h; float dt, camera block)`, RET 8; caller 00540390
(`src/menu_screens_2.cpp` calls it through `burst_update_0053f2d0`). For each burst of the list
+14h: the hit record reset, timer -= dt; an expired burst stops its trail, spawns the type's impact
definition (+28h) at the hit record point and runs the blast 0084BAD0 at its own position (only when
the definition exists), then is unlinked and freed; a live burst moves (velocity * dt, gravity * dt
on vy), steers its trail (velocity as row 2, 0085DC80, 0053D9C0), and tests the segment from its
previous position: a world hit (009043A0) hands the hit unit's root to the debug inspector when it is
class 1 and spawns/blasts at the hit point; otherwise a 0078D6C0 hit spawns the splash definition
(+2Ch) and blasts at its point; either hit stops the trail and erases the node (0053EB10). The list
end calls 0053EE00 (sweepB4). The blast applies real damage through 0084BAD0; this is the
free-camera test-fire path, not the gunnery lane.

## Recorded without reconstruction

- 00626C50 (segment 26): library-dominated. For each of the three Lua option tables (E196D0,
  E196E0, E196F0) and each entry whose owner (E1973C/E1974C/E1975C) is the scoring commit record's
  +250h id, the entry's record (00625F50) gets the default string (E1976C/E1977C/E1978C) and bit i of
  a vector<bool> (00E19710/00E19724, or 0048F930/0048F230 for the first). Both arms of the +1E4h test
  repeat the same loops; nearly all of the body is inlined checked-iterator and
  vector<bool> machinery. Not reconstructed in this pass.

## Not reached in this pass (no record)

In order: 005A9DB0 (x87-heavy matrix overlay builder; listing dumped, not started), 00789560,
006D5220, 006003A0, 005EADC0, 00776930, 005E3580, 00597870, 006179D0, 0056F320, 005E7B10, 0067F4D0,
005D73A0, 007884D0, 00536A90, 00674B80, 005B3470, 00673A10, 005DF510, 00538F80, 005B45C0, 0060ABD0,
007A91A0, 00670020, 00822C20, 00627960, 005DB050, 005ADDA0; then 005A3210 and 00617470.

## Reconstructed

006191A0, 0078A430, 005A6AD0, 005BE240, 0065BF00, 006B9450, 0053F2D0.
