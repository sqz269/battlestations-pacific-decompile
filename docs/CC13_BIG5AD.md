# cc13-big5ad: 005ADDA0, the HUD map per-icon update

Packet `cc13-big5ad` (2026-10-17). Code: `src/cc13_big5ad.cpp`, header `include/bsp/cc13_big5ad.hpp`
(`Cc13Big5adHost : Cc13BigB2Host`). Nothing is wired. Status: reconstructed, build-tested; not
ABI-compatible (a new C++ interface over the raw records), not game-validated.

## Function

`005ADDA0 BSP_HudMapScreen_UpdateUnitIcon` (5,027 instructions, 841 x87, 318 direct calls, three jump
tables). `__thiscall(ECX = HUD map screen; icon record)`, `RET 4`. The earlier ledger note called it
`thiscall(icon)`: ECX is the screen (kept in EBP and `[entry-180h]`), the stack argument is the icon
(ESI); the icon's unit is `icon+4h` (EBX). Frame: `SUB ESP,188h` under an EH registration (handler
00C71F18); the EH state slot is `[entry-4]`. C++: `cc13_big5ad_hud_map_screen_update_unit_icon_005adda0`.

### How it was read

- The body is too large for a trustworthy decompile (Ghidra loses the frame: `unaff_EBX/EBP/ESI/EDI`,
  four "unreachable" blocks at 005AE8A1..005AE91E that are reachable through `[entry-18Dh]`). It was
  read whole from the export listing with a local walker that tracks ESP against entry through the
  CFG, including the three jump tables 005B2590 (screen+128h modes 1..4), 005B25A0 (lamp switch) and
  005B25C0 (arrow side), and checks every EH-state store lands on `[entry-4]` (all 19 do, zero join
  conflicts). Two indirect calls take no arguments although an argument is already pushed for the
  next call (`unit->vtable[14h]()` at 005B05C7 and 005B115B, whose `PUSH 1` belongs to 00ABBE50).
- Every x87 instruction was re-decoded from the PE bytes with capstone (the Ghidra listing misprints
  the DC/DE register forms); the reverse forms used here were spot-checked against the raw opcodes
  (DE E1 FSUBRP, DE E9 FSUBP, DE F1 FDIVRP, DE EC FSUBP ST4, DC CB, DC E0, D8 C4, D8 CA).
- Constants were read from the image: the doubles 00D7A358, 00CF0B68, 00CF0B60, 00CF0B58, 00D7A390,
  00CE3DC8, 00CE3D10, 00D7A270, 00D7A3A0, 00CEC8F8, 00CF0B28, 00CF0B20, 00CF0B18, 00CF0B10, 00CEB690,
  00CED0E0, 00CF0AD0, 00CF0AC8, 00CF0AC0, 00CF0AB0 and 00D7A268 are float constants widened to
  double (written as `double(0.01f)` or as their exact decimal); 00CEC380 = 960.0, 00CEF1B8 = 720.0,
  00CF0B30 = 38.0, 00CF0B38 = 110.0, 00CF0B48 = 1300.0, 00CF0AB8 = 9.0, 00CF0B08 = 185.2 are true
  doubles. Strings: 00CF0B74 "InferiorFailure", 00CEF794 "globals.newplayer" (compared by address
  only), 00CE4180 "globals.unknown", 00CF0ADC " |globals.kilometer|. ", 00CF0AF4 " |globals.mile|. ",
  00CF0AD8 ". ", 00CE3A70 ".". 00E08890/00E08894 are .data floats (the map offset, read live).
- Main thread: every x87 step is modelled as a float operation in the image's order (24-bit PC).

### What it does, in order

1. 005ADDC8: returns at once when the game root's `+61Fh`/`+620h` flags are set (unless `+1FE4h` is set
   and `+218Ch` clear).
2. 005ADE1A: in multiplayer (`root+1FE4h`), the owner-name label `icon+18h` (cloned from screen+98h,
   text = player record `+58h`, colour white for no local slot else config +108h/+128h by team) and
   the talking icon `icon+1Ch` (screen+9Ch; 007F74B0 and the voice-mute table 00778740; scale
   screen+5CCh * 0.5); both are deleted when no player owns the unit or the owner is the local player.
3. 005AE15C: the two 3D map models (`+124h`, `+128h`): unit matrix scaled by screen+5E4h (x doubled when
   `+2Eh`), lifted by 1500; the second model is flattened to the unit heading (forward falls back to
   00F87598.. when the xz length squared is below 1e-4, up = 00F8758C..; 0085DC80 orthonormalises),
   shown only while screen+5D8h < 1.5. Visibility factors via 00B6DA70 (blink pair screen+5D0h/+5D4h
   while `+8h != +9h`).
4. 005AE5B0: the icon colour (0059A9B0): selected commander config+158h; non-plain units +198h;
   otherwise by team relation, the "InferiorFailure" query, the selectable test 00645160, and for a
   capture building (kind 1Ch, 006F1F90 non-zero) a blend toward the side colour by screen+84h.
   Then the fuel/ammo fill of a kind-6 unit (`+980h` clamped to 0..1 by 00415690) into material
   `+13Ch` (texture screen+4A4h[screen+4B0h], value `+140h`).
5. 005AEA42: seven trail markers `+48h..+60h` (start `+74h`, step 0.5 s; grow 1.01, fade 0.9, delete
   at alpha <= 0.01).
6. 005AEBC8: `+2Bh = +2Ah`; `+2Ah` = the unit inside the map area (0059D410).
7. 005AEC1B: the one-shot selection ping (static 00E19515): three rings `+64h..+6Ch` (on-map clone of
   screen+A4h, else an edge arrow from screen+E8h placed by 0059F7C0), then 005AED58 their animation
   (start `+70h`, step 0.3 s; grow 1.02, fade 0.85).
8. 005AEEF2: `+29h` = recon relation != 3 (`[entry-111h]` = changed); the formation line `+24h` to the
   leader `[[unit+284h]+14h]` (new 50h-byte object, vtable 00CEFFE8, model by 005A1B40, diffuse =
   config+1A8h; composed by 00B64470 from the inset end points; alpha eases to 0.2, width to 50).
9. 005AF3B3: unless the unit is kind 34h or 2Bh, and only while the info panel is closed (`+118h == 0`),
   the row for screen mode 1..4: health bar pair `+78h`; badge row `+98h` (005A8420) plus carried
   units `+B8h` (005AAC00); power-ups `+88h` (005A7C20); status lamps `+A8h` (0059D150). Rows no mode
   filled are deleted (005AFEF6..005B02D4; `+B8h` through 005A0B20).
10. 005B02D5: the off-map edge arrow `+20h`.
11. 005B03D8: places the info panel (0059D4A0, 0059D6D0, 0059D4A0) when its frame widget exists, then
    the panel state machine `+118h`: 1 -> 2 at `+110h` (title and box size), 2 creates the frame and
    waits for 0059CEF0, 3 creates the other widgets, 4 lays out title, distance text, lamps `+D8h`,
    power-up grid `+C8h`, health/capture bars and the side arrow (`+120h`), 5 closes, 6 releases
    (005A1C60).

No call to the shared RNG 00BD2F10 (none of the 63 direct callees is it).

## Hooks

Reused from the chain: the cc13_sweepB..B9 and cc13_bigB2 hooks named in the code. Reconstructed
functions called directly: `cc13_sweepB_hud_map_create_alpha_blend_model_005a1b40`,
`cc13_sweepB_hud_map_icon_refresh_unit_badge_005a8420`,
`cc13_sweepB6_hud_map_icon_refresh_power_up_icons_005a7c20`. New hooks (header comments give each
ABI): 0059A840, 0059A860, 0059A9B0, 0059ADC0, 004B6BC0, 004FB9E0, 004F9950, 006F1F90, 00415690,
0059CEF0, 0059D150, 0059D410, 0059D4A0, 0059D6D0, 0059E0C0, 0059F7C0, 005AAC00, 0059AC50, 0059CB90,
0077A2F0, 00B64470, 004391A0, 005A0B20, 00BF7420, 00BF9940, 00645160, unit vtable +198h with its AL
result, widget vtable +30h with a float pair. 0059E0C0 is tagged `STL_inst_0059e0c0` but is the HUD
grid size (cols = rows = (int)sqrt(count), each grown while the product is short); bound, not
reconstructed. 004391A0 and 005A0B20 are std::vector instantiations (bound per rule 2b).

## Simplifications and uncertainties

- `[entry-124h..-118h]` (the icon colour passed to 0059A9B0 at 005AE9B8): on the paths that write
  nothing (no team match, `[entry-190h]` set without the "InferiorFailure" match, or no commander) the
  image passes stale stack bytes; the C++ passes zeros.
- 005AFC95..005AFDF0 and the state-4 lamps: the image passes the lamp flag's whole stack dword to
  vtable +34h (upper bytes stale from earlier float use of the slot); only the byte is modelled.
- 005AF713: the third argument of 005A8420 is EAX with only AL set (unit+54h == 2); the callee does
  not read it (cc13_sweepB); 0/1 is passed.
- 005B1D59: on the non-1Ch path the image loads the `[entry-170h]` float slot into EDI; it is unread
  there (the capture value stays 0, so the health branch runs); the bits are copied to keep the shape.
- Products of a float with a widened float constant are modelled as `float(double(a) * k)`; the
  image rounds the exact product to 24 bits once, so a double-rounding difference is possible in
  rare cases.
- No guards were added: the virtual call on a possibly null owner at 005ADE6D, the writes through a
  failed `operator new` result at 005AF036, and the divisions by the grid sizes are as in the image.
- EH unwinding (the line object, EH state 1; the distance strings, EH states 2..10h) is not modelled.
- The doubled index checks the image inlines before each vector access are reproduced through
  `vec_checked_at`/`panel` (one per access site).

## 00BA2FF0 ocean beams: NaN arms (second packet item)

`src/water_render.cpp` is a cc13 file (cc13_water1 / cc13-water2), so it was fixed in place. Every float
compare of 00BA2FF0 was checked against its jump: the birth and death tests (COMISS / JC at 00BA3216
and 00BA3226) are taken on unordered input, so they are now `!(x >= 0.0f)`; the fade-out test (COMISS
/ JC at 00BA37AA) skips on unordered input, so it is now `death >= 0.0f`. The camera (00BA304C),
previous clock (00BA3076), clock-past-death (00BA3236), band (00BA33E1..00BA3401), survival
(00BA349B), distance (00BA356D), normal (00BA3719), fade-in (00BA3786), fade-out window (00BA37B6)
and axis (00BA3892) compares already matched. Ledger record replaced; docs/CC13_WATER_RENDER.md notes it.

## Direct call sites (from the listing)

The 318 direct calls, grouped by callee, for `tools/verify_report_calls.py`:

- 004134F0 (3): `005AE2CA CALL 004134F0`, `005AE351 CALL 004134F0`, `005AE4D1 CALL 004134F0`
- 00413920 (2): `005AE2BD CALL 00413920`, `005AE4C4 CALL 00413920`
- 00414DB0 (3): `005AE1A6 CALL 00414DB0`, `005AE1F9 CALL 00414DB0`, `005AE344 CALL 00414DB0`
- 00415690 (1): `005AEA09 CALL 00415690`
- 00419CC0 (8): `005B1436 CALL 00419CC0`, `005B1463 CALL 00419CC0`, `005B1490 CALL 00419CC0`, `005B14BD CALL 00419CC0`, `005B14E4 CALL 00419CC0`, `005B150B CALL 00419CC0`, `005B1532 CALL 00419CC0`, `005B1804 CALL 00419CC0`
- 0041DD20 (1): `005AE760 CALL 0041DD20`
- 0041E870 (3): `005AE70C CALL 0041E870`, `005B1370 CALL 0041E870`, `005B1389 CALL 0041E870`
- 004260B0 (2): `005B139B CALL 004260B0`, `005B13B2 CALL 004260B0`
- 004261A0 (3): `005B13E8 CALL 004261A0`, `005B13FD CALL 004261A0`, `005B1416 CALL 004261A0`
- 0042B2F0 (1): `005B12EF CALL 0042B2F0`
- 00432650 (5): `005ADFD8 CALL 00432650`, `005ADFE4 CALL 00432650`, `005AE5B0 CALL 00432650`, `005AED36 CALL 00432650`, `005AEF92 CALL 00432650`
- 004391A0 (8): `005AF52D CALL 004391A0`, `005AF5B1 CALL 004391A0`, `005AF626 CALL 004391A0`, `005AF668 CALL 004391A0`, `005AF69E CALL 004391A0`, `005AF6D2 CALL 004391A0`, `005AFE07 CALL 004391A0`, `005AFE31 CALL 004391A0`
- 0043C130 (1): `005B13CC CALL 0043C130`
- 0043DCA0 (4): `005AF4A0 CALL 0043DCA0`, `005AF506 CALL 0043DCA0`, `005AFC47 CALL 0043DCA0`, `005B09B7 CALL 0043DCA0`
- 004B6BC0 (1): `005ADFB2 CALL 004B6BC0`
- 004F9950 (1): `005AE123 CALL 004F9950`
- 004FB9E0 (1): `005AE115 CALL 004FB9E0`
- 0059A840 (2): `005ADF5A CALL 0059A840`, `005AE0A4 CALL 0059A840`
- 0059A860 (2): `005ADF78 CALL 0059A860`, `005AE0C0 CALL 0059A860`
- 0059A9B0 (2): `005AE5DF CALL 0059A9B0`, `005AE9BA CALL 0059A9B0`
- 0059AC50 (2): `005AF641 CALL 0059AC50`, `005B1F14 CALL 0059AC50`
- 0059ADC0 (1): `005ADFC3 CALL 0059ADC0`
- 0059CB90 (2): `005B0915 CALL 0059CB90`, `005B106F CALL 0059CB90`
- 0059CEF0 (3): `005B0829 CALL 0059CEF0`, `005B0F71 CALL 0059CEF0`, `005B2550 CALL 0059CEF0`
- 0059D150 (3): `005AFBD1 CALL 0059D150`, `005B06B9 CALL 0059D150`, `005B186F CALL 0059D150`
- 0059D410 (1): `005AEC13 CALL 0059D410`
- 0059D4A0 (2): `005B040C CALL 0059D4A0`, `005B0420 CALL 0059D4A0`
- 0059D6D0 (1): `005B0414 CALL 0059D6D0`
- 0059E0C0 (2): `005AF777 CALL 0059E0C0`, `005AFA84 CALL 0059E0C0`
- 0059F7C0 (2): `005AECFD CALL 0059F7C0`, `005B0371 CALL 0059F7C0`
- 005A0B20 (1): `005B02D0 CALL 005A0B20`
- 005A1B40 (1): `005AEFC0 CALL 005A1B40`
- 005A1C60 (1): `005B256E CALL 005A1C60`
- 005A7C20 (4): `005AFA24 CALL 005A7C20`, `005B06FC CALL 005A7C20`, `005B0863 CALL 005A7C20`, `005B1B2B CALL 005A7C20`
- 005A8420 (1): `005AF717 CALL 005A8420`
- 005AAC00 (1): `005AF89B CALL 005AAC00`
- 00645160 (1): `005AE7EA CALL 00645160`
- 006F1F90 (3): `005AE65E CALL 006F1F90`, `005B1CF2 CALL 006F1F90`, `005B1D25 CALL 006F1F90`
- 00778740 (1): `005AE068 CALL 00778740`
- 007788B0 (1): `005AEF2C CALL 007788B0`
- 0077A2F0 (2): `005AF6BB CALL 0077A2F0`, `005B1F6C CALL 0077A2F0`
- 007F74B0 (1): `005AE027 CALL 007F74B0`
- 00803CE0 (4): `005ADEA5 CALL 00803CE0`, `005AE632 CALL 00803CE0`, `005AE8A5 CALL 00803CE0`, `005AEF12 CALL 00803CE0`
- 0085DC80 (1): `005AE3FF CALL 0085DC80`
- 00927F10 (1): `005ADECC CALL 00927F10`
- 00AA6740 (18): `005AF534 CALL 00AA6740`, `005AF72C CALL 00AA6740`, `005AF741 CALL 00AA6740`, `005AF8A8 CALL 00AA6740`, `005AF8BD CALL 00AA6740`, `005AFA39 CALL 00AA6740`, `005AFA4E CALL 00AA6740`, `005AFE99 CALL 00AA6740`, `005B0723 CALL 00AA6740`, `005B166D CALL 00AA6740`, `005B173D CALL 00AA6740`, `005B1A72 CALL 00AA6740`, `005B1AFE CALL 00AA6740`, `005B1B58 CALL 00AA6740`, `005B1C0C CALL 00AA6740`, `005B1C2B CALL 00AA6740`, `005B1DB8 CALL 00AA6740`, `005B22B5 CALL 00AA6740`
- 00AA6750 (1): `005B0F22 CALL 00AA6750`
- 00AA7DC0 (2): `005ADFA7 CALL 00AA7DC0`, `005AE0F5 CALL 00AA7DC0`
- 00AA8240 (22): `005AEB08 CALL 00AA8240`, `005AED31 CALL 00AA8240`, `005AEE3C CALL 00AA8240`, `005AF5C0 CALL 00AA8240`, `005AF635 CALL 00AA8240`, `005AF83E CALL 00AA8240`, `005AF9CD CALL 00AA8240`, `005AFB4B CALL 00AA8240`, `005AFEE5 CALL 00AA8240`, `005B038D CALL 00AA8240`, `005B1014 CALL 00AA8240`, `005B11CA CALL 00AA8240`, `005B1609 CALL 00AA8240`, `005B16D9 CALL 00AA8240`, `005B17AD CALL 00AA8240`, `005B1ACB CALL 00AA8240`, `005B1C98 CALL 00AA8240`, `005B1E8A CALL 00AA8240`, `005B1F0A CALL 00AA8240`, `005B20C3 CALL 00AA8240`, `005B21B0 CALL 00AA8240`, `005B24E5 CALL 00AA8240`
- 00AAB4C0 (19): `005ADF13 CALL 00AAB4C0`, `005AE087 CALL 00AAB4C0`, `005AEC7E CALL 00AAB4C0`, `005AEC94 CALL 00AAB4C0`, `005AF447 CALL 00AAB4C0`, `005AF4AD CALL 00AAB4C0`, `005AFC28 CALL 00AAB4C0`, `005B0519 CALL 00AAB4C0`, `005B07F1 CALL 00AAB4C0`, `005B08C5 CALL 00AAB4C0`, `005B0998 CALL 00AAB4C0`, `005B0A1E CALL 00AAB4C0`, `005B0A86 CALL 00AAB4C0`, `005B0B19 CALL 00AAB4C0`, `005B0BA6 CALL 00AAB4C0`, `005B0C37 CALL 00AAB4C0`, `005B0D10 CALL 00AAB4C0`, `005B0DBB CALL 00AAB4C0`, `005B0E66 CALL 00AAB4C0`
- 00ABAED0 (1): `005B156D CALL 00ABAED0`
- 00ABBE50 (3): `005ADF49 CALL 00ABBE50`, `005B05CC CALL 00ABBE50`, `005B1160 CALL 00ABBE50`
- 00B179F0 (1): `005AEFCA CALL 00B179F0`
- 00B189F0 (1): `005AEA2C CALL 00B189F0`
- 00B64470 (1): `005AF331 CALL 00B64470`
- 00B6DA70 (2): `005AE56F CALL 00B6DA70`, `005AE5AB CALL 00B6DA70`
- 00BD1510 (8): `005B143D CALL 00BD1510`, `005B146A CALL 00BD1510`, `005B1497 CALL 00BD1510`, `005B14C4 CALL 00BD1510`, `005B14EB CALL 00BD1510`, `005B1512 CALL 00BD1510`, `005B1539 CALL 00BD1510`, `005B180B CALL 00BD1510`
- 00BF6713 (130): `005AF82B CALL 00BF6713`, `005AF85C CALL 00BF6713`, `005AF9BA CALL 00BF6713`, `005AF9EB CALL 00BF6713`, `005AFB38 CALL 00BF6713`, `005AFB69 CALL 00BF6713`, `005AFC81 CALL 00BF6713`, `005AFCA8 CALL 00BF6713`, `005AFCF1 CALL 00BF6713`, `005AFD1C CALL 00BF6713`, `005AFD66 CALL 00BF6713`, `005AFD91 CALL 00BF6713`, `005AFDDB CALL 00BF6713`, `005AFED5 CALL 00BF6713`, `005AFF4E CALL 00BF6713`, `005AFF85 CALL 00BF6713`, `005AFF98 CALL 00BF6713`, `005B001E CALL 00BF6713`, `005B004F CALL 00BF6713`, `005B005F CALL 00BF6713`, `005B00EC CALL 00BF6713`, `005B0123 CALL 00BF6713`, `005B0136 CALL 00BF6713`, `005B01DC CALL 00BF6713`, `005B0213 CALL 00BF6713`, `005B0226 CALL 00BF6713`, `005B0285 CALL 00BF6713`, `005B02B3 CALL 00BF6713`, `005B02C0 CALL 00BF6713`, `005B03EF CALL 00BF6713`, `005B04DB CALL 00BF6713`, `005B0503 CALL 00BF6713`, `005B053D CALL 00BF6713`, `005B0569 CALL 00BF6713`, `005B05AC CALL 00BF6713`, `005B05EB CALL 00BF6713`, `005B0679 CALL 00BF6713`, `005B0791 CALL 00BF6713`, `005B07C0 CALL 00BF6713`, `005B07E2 CALL 00BF6713`, `005B080F CALL 00BF6713`, `005B0882 CALL 00BF6713`, `005B08AB CALL 00BF6713`, `005B08E6 CALL 00BF6713`, `005B093A CALL 00BF6713`, `005B09DB CALL 00BF6713`, `005B0A04 CALL 00BF6713`, `005B0A3F CALL 00BF6713`, `005B0A70 CALL 00BF6713`, `005B0AA7 CALL 00BF6713`, `005B0ADE CALL 00BF6713`, `005B0B03 CALL 00BF6713`, `005B0B3A CALL 00BF6713`, `005B0B6B CALL 00BF6713`, `005B0B90 CALL 00BF6713`, `005B0BC7 CALL 00BF6713`, `005B0BF8 CALL 00BF6713`, `005B0C21 CALL 00BF6713`, `005B0C58 CALL 00BF6713`, `005B0C89 CALL 00BF6713`, `005B0CD1 CALL 00BF6713`, `005B0CFA CALL 00BF6713`, `005B0D31 CALL 00BF6713`, `005B0D62 CALL 00BF6713`, `005B0DA5 CALL 00BF6713`, `005B0DDC CALL 00BF6713`, `005B0E0D CALL 00BF6713`, `005B0E50 CALL 00BF6713`, `005B0E87 CALL 00BF6713`, `005B0EB8 CALL 00BF6713`, `005B0F10 CALL 00BF6713`, `005B0FA0 CALL 00BF6713`, `005B1001 CALL 00BF6713`, `005B1033 CALL 00BF6713`, `005B1094 CALL 00BF6713`, `005B10CF CALL 00BF6713`, `005B1113 CALL 00BF6713`, `005B1144 CALL 00BF6713`, `005B11B7 CALL 00BF6713`, `005B11E9 CALL 00BF6713`, `005B121D CALL 00BF6713`, `005B1246 CALL 00BF6713`, `005B126F CALL 00BF6713`, `005B1558 CALL 00BF6713`, `005B158C CALL 00BF6713`, `005B15F6 CALL 00BF6713`, `005B1628 CALL 00BF6713`, `005B165C CALL 00BF6713`, `005B16C6 CALL 00BF6713`, `005B16F8 CALL 00BF6713`, `005B172C CALL 00BF6713`, `005B179A CALL 00BF6713`, `005B17CC CALL 00BF6713`, `005B18DB CALL 00BF6713`, `005B1918 CALL 00BF6713`, `005B1966 CALL 00BF6713`, `005B19A1 CALL 00BF6713`, `005B19ED CALL 00BF6713`, `005B1A1F CALL 00BF6713`, `005B1A65 CALL 00BF6713`, `005B1AB8 CALL 00BF6713`, `005B1C85 CALL 00BF6713`, `005B1CB6 CALL 00BF6713`, `005B1D77 CALL 00BF6713`, `005B1E77 CALL 00BF6713`, `005B1EA9 CALL 00BF6713`, `005B1EF4 CALL 00BF6713`, `005B1F45 CALL 00BF6713`, `005B1FB4 CALL 00BF6713`, `005B1FE8 CALL 00BF6713`, `005B201C CALL 00BF6713`, `005B204D CALL 00BF6713`, `005B2074 CALL 00BF6713`, `005B20AD CALL 00BF6713`, `005B20FC CALL 00BF6713`, `005B2130 CALL 00BF6713`, `005B2164 CALL 00BF6713`, `005B219A CALL 00BF6713`, `005B21E9 CALL 00BF6713`, `005B221D CALL 00BF6713`, `005B2251 CALL 00BF6713`, `005B2282 CALL 00BF6713`, `005B22A8 CALL 00BF6713`, `005B2345 CALL 00BF6713`, `005B23AF CALL 00BF6713`, `005B2412 CALL 00BF6713`, `005B246D CALL 00BF6713`, `005B24CF CALL 00BF6713`, `005B2504 CALL 00BF6713`, `005B253C CALL 00BF6713`
- 00BF67A7 (4): `005AFFBE CALL 00BF67A7`, `005B0085 CALL 00BF67A7`, `005B015C CALL 00BF67A7`, `005B024C CALL 00BF67A7`
- 00BF681B (1): `005AEF71 CALL 00BF681B`
- 00BF7030 (3): `005AF123 CALL 00BF7030`, `005AF140 CALL 00BF7030`, `005AF8EC CALL 00BF7030`
- 00BF7420 (3): `005AF8F9 CALL 00BF7420`, `005B0734 CALL 00BF7420`, `005B1B0F CALL 00BF7420`
- 00BF9940 (1): `005AF26A CALL 00BF9940`
