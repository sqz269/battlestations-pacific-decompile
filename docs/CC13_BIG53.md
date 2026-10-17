# CC13 big53: large or skipped bodies in segments 52/53

Packet cc13-big53 (2026-10-17). The bodies the cc13 seg52, seg52b and seg53 passes skipped:
the ship class model-data binder, the unit gunnery AI property reader and the seg 53 x87 debug
geometry. Code: `include/bsp/cc13_big53.hpp`, `src/cc13_big53.cpp` (namespace
`bsp::cc13_big53`, marker cc13_big53). Nothing is wired into bsp_game; nothing was run.
Status for every row: reconstructed and build-tested only, not fixture-tested or game-validated.
Names are hypotheses. No torpedo or submarine AI and no shared-RNG (00BD2F10) draws occur in
any of these bodies.

The renderer slot B4h and 0085F990 are reached through `cc13_seg53` (its `Seg53Host` and its
`debug_draw_line_rgba_floats_0085f990` model); every other callee is a named `Big53Host` method.

## Reconstructed

| Address | Function | Notes |
|---|---|---|
| 0082FE30 | `ship_class_bind_model_data_0082fe30` | __fastcall(ECX class), plain RET, 1,732 instructions read whole (listing 0082FE30..00831801). 0095F500, 0082D700; deckline/bottomline through 00718000 (kind 0) into 0082D040(bottom+44h, deck+44h, [class+71Ch]); debarkation copied into class+584h (004D1510 resize, per-element checked reads) or twelve box corners from the half extents A4h/A8h/A0h (n = -0.0 - v, SUBSS); class+5B0h and +5F0h reset to identity, then the two orrhullam bow frames (translation p0, row +10h p2-p0, row +20h cross(p2-p0, p1-p0) through 004F9B30, 0085DC80); wave p0/p1 into +594h/+5A0h with no null test; shipcenter into +578h or zero; wave_stern into +638h/+644h or zero; kemeny (10h-byte {point, index} entries, 00829180 growth) into +678h, farviz into +650h and explosion into +684h (0074D190 growth); path groups with two or more points: 0082E990 resize of +6CCh to i and every point (004FBA10 count, 00484270, 004215D0) appended to the last element, with no path at all erasing +6DCh through 0082F4E0; idle frames (identity, translation p0, row +20h p2-p0, row +10h cross(p2-p0, p1-p0), 0085DC80) pushed onto +6ECh with 0082CA40; finally the frames (40h bytes) and the records +6FCh (1Ch bytes) are shrunk to the smaller count (0082E1F0 + 0082F8E0, or 0082C560). Every inlined checked access calls 00BF6713 on its failure condition (null first or index >= size) and then re-reads first. The std::string temporaries are modelled (00408720 assign, free 00BF65AC when capacity >= 10h). |
| 00863130 | `unit_gunnery_ai_load_properties_00863130` | __thiscall(ECX ai, archive), RET 4. Section "_bot" with 008FE660; operator new 24h (00BF681B) for the visibility cache at +68h {+0 [ai+50h], +4/+8/+0Ch 0, +10h -1.0f, byte +20h 0}, null stored on a failed allocation; "thinkTime" kind 2 by address into +6Ch; section "fire" twelve kind-3 bytes keyed by integer index (kind 1) into +70h+i; section "enable" twelve kind-1 dwords into +80h+4i. |
| 0085C5D0 | `ellipsoid_intersect_segment_0085c5d0` | __fastcall(ECX m, EDX origin; delta, t_plus*, t_minus*, byte allow_inside), RET 10h -> AL. Both points through 00B62D10 with w = 1, divided by w via 1/w; c = abs(a)^2 - 1 (inside test COMISS/JBE, skipped when the byte is set); A = abs(v)^2, B2 = 2 a.v, disc = B2^2 - (4A)c (FCOMI/JC: negative or NaN returns false); sqrt through 00BF7030; t_plus = (r - B2)/(A+A), t_minus = (-B2 - r)/(A+A). The x87 forms DE F2/F1/EA/E9/FB/E2/F9 were checked against the bytes. |
| 0085FB70 | `debug_draw_arrow_rgba_floats_0085fb70` | __fastcall(ECX from, EDX to; eight floats), RET 20h. The 0085F990 body inlined (renderer slot B4h, colours chopped from float(channel*255)); then n = d/len (len = 0 when abs(d)^2 <= 1e-10, so n is inf/NaN as in the image), camera [[00E188A8]+19FCh] refreshed through 00B6DB70 unless byte +5Ch bit 1, w = cross(camera+110h, n); returns when abs(w)^2 <= 1e-10 or sqrt <= 0.01f; else 0042B260, h = 0.1 len, hw = 0.1 h (DC C9 at 0085FE41 checked), and the two barbs (to - n h +/- w hw) -> to drawn in colour 1. |
| 0085FFF0 | `debug_draw_arrow_rgba_floats_one_colour_0085fff0` | ECX/EDX through to 0085FB70 with the four colour dwords twice; RET 10h. |
| 00860140 | `debug_draw_arc_in_frame_00860140` | __fastcall(ECX frame, EDX rgba*; radius, start, sweep, int segments), RET 10h. n = _ftol2(float(abs(sweep)/2pi) * segments), at least 1; step = sweep/n; points (r sin, r cos, r*0.0) at float(i)*step + start (D8 C9 at 008601E6 checked), each transformed by 004142E0 (end point first) and joined by 0085F990. cc13_seg53's hook for this routine passes rgba as a uint32; it is the pointer EDX carries. |
| 008605C0 | `debug_draw_aabb_wireframe_008605c0` | __fastcall(ECX box {min, max}, EDX rgba*), plain RET. Twelve edges: for each axis a, the other two axes take (max, min) combinations, j outer, m inner, 0 = max. |
| 00860710 | `debug_draw_circle64_00860710` | __fastcall(ECX centre, EDX rgba*; radius), RET 4. 64 segments in XZ, y = radius*0.0, angle float(i*pi)/32 (pi as the float value 3.1415927, so both products are single-rounded); 0085F990(centre + A, centre + B). |
| 008608B0 | `debug_draw_circle_with_heading_008608b0` | __fastcall(ECX centre; radius, px, py, pz), RET 10h; EDX is not an input. The function-static colour (1, 0.5, 0, 1) at 00F875A8 under guard bit 0 of 00F875B8; Circle64 in it; d = (cx - px, 0, cz - pz); returns when eps > abs(dx) and eps > 0 and eps > abs(dz) (eps = 1e-4 double, NaN draws); else 0042B260, d *= radius (D8 C9 at 008609A8 checked), 0085F990(centre, centre - d). py is never read. |
| 00861260 | `debug_draw_text_at_world_point_00861260` | __fastcall(ECX world, EDX text object), plain RET. 0043A870(world, &xy, 2, 1); when non-zero 004C7070([00E188A8], x, y, [obj+4] or the empty string 00F875BC) with the text as the format. The C++ passes a literal "" instead of the image address 00F875BC (same bytes). |

## Precision and simplifications

- 0082FE30: the x87 work is float subtraction and multiplication by 0.5, one rounding each, so
  the precision control does not matter. The 0Ch-byte 004D1510 prototype and word +0 of the 10h
  0082E990 prototype are uninitialised stack in the image (zero here); the matrix handed to
  0082C560 is the last idle frame or stale stack when none was built (zero here), and that call is
  reached only to shrink, so it is never read. The SEH unwind (FuncInfo 00DC377C) only destroys the
  string temporaries.
- Debug geometry: main thread, 24-bit precision control. Every operand is a float or a
  float-representable double constant, so each x87 op is one float rounding (written as float
  expressions or float(double product)). FSIN/FCOS are modelled as float(std::sin(double)); the x87
  instruction can differ from the CRT in the last bit before the float store. The 0085FB70 colour
  bytes are chopped from float(channel*255.0), the 24-bit FMUL result; cc13_seg53's own
  `colour_byte` (used by its 0085F990 model, which the arc, box and circle call) truncates the
  double product instead, which can differ when the product rounds up to an integer in float.

## Skipped

- 0084DDA0 BeginCommand: left to cc9 (packet brief).
- 008624C0: already done (seg53 hosted note).
- 00864380 BSP_UnitGunneryAi_Destruct, 0082DF00 / 0082E280 / 0082E5A0 and the container
  instantiations: STL-dominated, outside this packet.
