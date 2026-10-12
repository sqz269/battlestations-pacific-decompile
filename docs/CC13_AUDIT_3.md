# cc13 adversarial audit 3

Addresses: 00BC0510 00BC0D40 00BC1A20 00BC12F0 00BC40F0 00BC3930 00BC3EF0 00BC03C0 00B06B10
00B08210 00B09510 00B4D500 00B8B3B0 00B8C520 00B55F00 00B0DB00 00B101C0 00430020 00430DA0
0042D610 0093E9A0 00445790 00445BF0 00BE3310 00BE3360 0059CB90 00568B60 0060CA20 00BD2F10
00BD2E60

Reviewer: cc13-audit3 (Claude), who wrote none of the routines below. Scope: cc13 reconstructions
that `docs/CC13_AUDIT_1.md` and `docs/CC13_AUDIT_2.md` did not sample, chosen for x87 arithmetic,
RNG draws, register ABIs and jump tables. Every verdict was checked against the disk bytes
(`bsp.py disasm-raw`, saved uncapped under `local/au3_*.txt`) and, for the x87 bodies, against
`tools/x87_sym.py`, which decodes register arithmetic from the opcode bytes. Constants were read
from the PE with `tools/pe_const_read.py` at the load's own width. Status words: the routines are
*reconstructed, build-tested*. Nothing here was game-validated, and no routine was run in
`bsp_game.exe` for this audit (none of them is wired into the executable's host tables).

## Verdicts

| Routine | Source | Coverage of this review | Verdict |
|---|---|---|---|
| 00BC0510 lightning storm update | src/weather_render.cpp:38 | complete. Timer and age update (the arg slot reused for the age), the COMISS/JB NaN handling, the stream-1 bolt pick `% 6`, the -80..80 bearing x 00D646E0, the cos/sin products in operand order, range 500..2000, thunder `(r-500)*0.8667+200`, the light at base+{0,500,0} (x+0 and 0+z spills), radius 20000, diffuse 4, flash count FISTP qword under chop, the repeat scale `4 - (6-n)*0.5` (unsigned FILD), the end test `age > duration` (FCOMIP/JBE), re-arm 0.07 or +4, return byte only on a new strike. All 20 constants match the PE. | CONFIRMED-CORRECT |
| 00BD2F10 / 00BD2FC0 RNG entry points | include/bsp/weather_render.hpp:37 | the ECX selector passes through 00BD2F10 to 00BD2ED0 (per-thread record `(slot*2 + ECX) * 9C8h`); 00BD2E60 returns `lo + (hi-lo)*u` rounded to float, first stack argument = lo | CONFIRMED (the argument-order assumption every weather call relies on) |
| 00BC0D40 storm init tail | src/weather_render.cpp:30 | 00BC1102..00BC112E: timer Uniform(0, +4h), +48h/+4Ch = EDI, age -1000 | CONFIRMED-CORRECT |
| 00BC1A20 waterdrops spawn | src/weather_render.cpp:140 | complete. Budget (unsigned JBE vs 3000), resize, the three placement branches with their constants (1.1, 0.5..0.8, -1..1), the bottom-only disc (1.4142 products, `asin` 0042CF10 RET 4 so the m11 COMISS reads camera+104h, the 3.14 flip), size/height/slide spills, life, the `- 0.01` direction, the sticky test `requested*pct/100 > index` (both FILDs unsigned), doubled life with stick 1000, `_ftol2 & (cells-1)` then SETLE/AND to 0. Six draws per drop, all stream 0. | CONFIRMED-CORRECT |
| 00BC12F0 waterdrops update | src/weather_render.cpp:227 | complete. The -0.0 SUBSS negation, both 0042D0D0 transforms (normalize argument 0, view_step at frame B4h and view_down at A8h after resolving the push order), the life clamp and the LAHF/0x44/JNP equal-zero skip (NaN processed), the stick COMISS/JB jitter gate (NaN skipped), three jitter draws and the product order of every spill, the 0.2/s decay and its floor, the ±pi/5 gravity draw, `gx = c*vd.x - s*vd.y`, `gy = c*vd.y + s*vd.x`, the weight clamp and the final sums. Delta stays on the x87 stack across the loop on both paths. | CONFIRMED-CORRECT |
| 00BC40F0 cloud puff scatter | src/weather_render.cpp:388 | complete: three Uniform(-100,100), the spilled squares summed `(aa+bb)+dd`, the `len > 0` guard, `inv*a, b*inv, inv*d`, radius Uniform(rmin, rmax), `center.x + dx*r`, `(dy*r) + center.y`, spin Uniform(0, 2pi), the add-puff argument order (position, angle, size), the unsigned count loop against the saved double, RET 14h | CONFIRMED-CORRECT |
| 00BC3EF0 / 00BC03C0 puff factory | src/weather_render.cpp:350 | the factory at [010909B0] is vtable 00CFEAC8 (ctor 00737A00); its +0Ch is 00BC03C0, which allocates and constructs through 00BC6430/00BC7B60 and draws no random number | CONFIRMED (no hidden draw) |
| 00BC3930 cloud lighting, alpha draw | src/weather_render.cpp:618 | 00BC3A19..00BC3ACB: transmittance first, mean accumulate, then one Uniform(0.6, 0.75) per puff into 00BC7B10 (`*255`, FISTP under chop, low byte to +30h), `mean / (int)n` by FIDIV | CONFIRMED on sampled range |
| Weather RNG table | docs/CC13_WEATHER_RENDER.md:253 | counts checked against the listings above | **DOC-CLAIM-WRONG** (fixed): `00BC40F0` draws 5 per attempt, not 6 |
| 00B06B10 axial emit | src/particle_emitters_cc13.cpp:257 | complete except the curve hosts: kind-0 integrals kept unrounded and kind-1/2 spilled, every product spill and the `t` left on the x87 stack between the integrals, wind slots 00E13028.., origin add order, frame-advance pop, alpha gate 0.004, colour slots and the B,G,R,A byte packing, the FollowDirection velocity (evaluate, not integrate), the alignment switch (Center/Bottom/Top, 3/4 fall through), floor/ftol frames, fmod blend, NaN-passing clamp, the corner table, flag half 3C00h only for blend 1, the asymmetric room counters +1CCh / +1B8h | CONFIRMED-CORRECT |
| 00B08210 floating emit | src/particle_emitters_cc13.cpp:433 | angle `(k*pi/180)*s54*scale + s50` (constant 00D5DAF8), speed spill, the origin add, y = 0 at B084DA, the corner angles (pi, angle - pi/2 via `fsub st(1),st(0)`, pi/2 + angle, angle), +90h cursor advance, +1BCh room | CONFIRMED on sampled ranges |
| 00B09510 sprite emit, near-camera fade | src/particle_emitters_cc13.cpp:575 | B09929..B09A38: size*argument kept as a double, `/ view z`, the [0,1] clamp (an unpopped 0.0 stays on the stack and is dropped later), the FUCOMIP/JP zero test, the inline pow (FYL2X/F2XM1/FSCALE, exponent 2.0f) and the `0.007 > fade*alpha` gate skipped for the second buffer | CONFIRMED-CORRECT (note N1) |
| 00B4D500 bloom sample table | src/post_effects_cc13.cpp:97 | complete: unsigned FILD extents, G(0,0,dev)*mult, the double `y^2` and `2*rho*rho` slots, `(2pi*dev)*dev` rounded then sqrt then reciprocal, per-tap exponent/exp/weight spills, offsets `(i*inv_w, i*inv_h, 0, 0)` (both axes, as documented), weights alpha 1.0 | CONFIRMED-CORRECT |
| 00B8B3B0 flare edge fade | src/post_effects_cc13.cpp:479 | complete: every compare's unordered branch, check order (x low fade before the y range test), the x/y high fades `1 - (v-0.94)/0.06` via FSUBRP/FDIVRP/FSUBP | CONFIRMED-CORRECT |
| 00B8C520 colour scale | src/post_effects_cc13.cpp:509 | per-byte `in*scale` spill, clamp, FISTP; NaN gives 80000000h -> byte 0 in the image and 0 in the C++ | CONFIRMED-CORRECT |
| 00B55F00 .mshd Caps reader | src/post_effects_cc13.cpp:285 | key strings 00D621E0/00D621CC/00D621B4/00D6219C to +10h/+14h/+2Ch/+28h, 101h defaults at +28h/+2Ch | CONFIRMED-CORRECT |
| 00B0DB00 camera history | src/scene_renderer_cc13.cpp:770 | jump test (>20.0, FCOMIP/JBE), oldest-drop copy, entry layout (fov +40h, time +44h), `target = time - delay`, the bracketing walk (exit on `target <= t[c]`, `t[d] < target`, or d == 0), fraction `(target-td)/(tc-td)`, the three 005803E0 lerps (from = d, to = c), z/y normalisation, both cross products and their `y*y + x*x + z*z` sum order (x87_sym) | CONFIRMED-CORRECT (perspective/invert/multiply hosts not re-read) |
| 00B101C0 motion-blur scene submit | src/scene_renderer_cc13.cpp:328 | complete: +22Ch gate, `dt = elapsed/frequency`, velocity `impulse + delta/dt`, impulse cleared, speed, the MinSpeed hysteresis `speed > min - min*0.05` only while +22Dh is set, visibility 0/1 with the zero second argument, the y < -5 clip plane {0,-1,0,2}, queue metadata {0,24h,5Ch}, camera retain/release | CONFIRMED-CORRECT |
| 00430020 torpedo view step | src/segment2_camera_cc13.cpp:139 | complete: aim from the entity copy, the `0 > y` height rule, the DC C9 at 0043019A (FMUL ST1,ST0, so both products use 100.0), the sub-step loop (min(dt, 0.03), t = 0.08*step/0.03, obstacle top `part + entity + 4.0`), the eight blends and nine eases in operand order, per-sub-step reload of +94h/+84h, the rounded dt on the x87 stack at the back edge | CONFIRMED-CORRECT (note N2) |
| 00430DA0 bomb view step | src/segment2_camera_cc13.cpp:77 | complete: blend `tuning+E8h * dt`, the torpedo test (kind 2Bh, [+314h]+F4h > 0), the 3.0 / -15.0 height rule, drift products, the -3 clamp, `(dy^2 + dx^2) + dz^2` against 1e-10 with 0 otherwise, the mix spills, `g = blend/3`, the up blend, both orbits (yaw about {0,1,0}, pitch about row 0) | CONFIRMED-CORRECT |
| 0042D610 orbit helper | src/segment2_camera_cc13.cpp:67 | negated length, `axis / -len`, `len * angle` into 0085E8C0 | CONFIRMED-CORRECT |
| 0093E9A0 overlay ellipse | src/strategic_map_cc13.cpp:66 | 32 segments, angle i*pi/16 as float, start from the previous (cos, sin) seeded (1, 0), end `cx + rx*c`, `cy + ry*s`, segment argument order | CONFIRMED-CORRECT |
| 00445790 weather camera shake | src/options_hardware_cc13.cpp:143 | complete: null-mover exit, weather map (+68h, key +2Ch) looked up before the view map (+5Ch), `view*weather` rounded once, the three amplitude products, the string release before the call, the 0042C4C0 stack block (amplitudes low, frequencies high) | CONFIRMED-CORRECT |
| 00445BF0 day-time wrap | src/options_hardware_cc13.cpp:202 | 00445CC2..00445CF9: +24 (double) while below 0, -24 while above 24.0f | CONFIRMED-CORRECT |
| 00BE3310 timer calibration | src/engine_core_cc13.cpp:43 | complete: two QPC samples around Sleep(100), 64-bit SUB/SBB, __allmul by 10, FILD qword, plain RET; 00BE3360 FILD [ECX] / [0109DB48] rounded to float | CONFIRMED-CORRECT |
| 0059CB90 HUD unit icon index | src/frontend_menus.cpp:226 | complete: the register ABI (ECX unit, EDX class, EBX keeps "class supplied"), the leader-class derivation, every NEG/SBB/AND/ADD and SETNE arm (17h/02h, 03h/04h, 06h/05h, 1Bh/0Ah, 16h/15h, 10h/00h, 0Eh/09h, 0Bh/09h), the projectile-kind chain, the building tail and the fort member walk | CONFIRMED-CORRECT |
| 00568B60 session browser hover | src/menu_screens_2.cpp:489 | complete: ESI = screen+0Ch so every listener offset is screen offset - 0Ch, the 0.5/1.0 command-button arm, the +114h toggle arm, the literal compare chain at 00568BF5..00568C4D, the kind-6 highlight | CONFIRMED-CORRECT |
| 0060CA20 plane spawn target id | src/hud_widgets.cpp:412 | complete: id in EAX, the +904h arm, UCOMISS/LAHF/JNP (NaN counts as set) | CONFIRMED-CORRECT |

## Defects

None in C++. One documentation defect, fixed in this branch:

- **DOC-CLAIM-WRONG, docs/CC13_WEATHER_RENDER.md random-draw table.** The row for `00BC40F0` said
  "6 per puff attempt". The listing has five 00BD2F10 calls per iteration (00BC4138, 00BC4159,
  00BC417A, 00BC4225, 00BC428E) and the puff factory reached through `[010909B0]` vtable+0Ch
  (00BC03C0 -> 00BC6430 -> 00BC7B60) makes no draw. The doc's own prose (line 150) already listed
  five. Fixed to 5.

## Notes

- **N1 (precision, not a defect).** 00B09510 squares the fade with the x87 pow sequence
  (FYL2X, F2XM1, FSCALE); the C++ calls `std::pow(fade, 2.0)`, which is exact. The two can differ
  in the last bit of the float only when the x87 log/exp rounding error crosses a float rounding
  boundary. The same holds for every place this file and its siblings evaluate in double where the
  image keeps an 80-bit intermediate (e.g. `v*t*scale` for kind-0 curves): double rounding, not a
  behaviour difference.
- **N2 (NaN only).** 00430020 picks the look-at height with `COMISS 0.0, y` / JBE, so a NaN target
  height takes the `y + 3` arm; the C++ `0.0f <= y ? y + 3 : 3` takes the `3` arm. Reachable only
  with a NaN entity position. Not edited: src/segment2_camera_cc13.cpp is cc13-camera2's file;
  reported to the lead.
- The curve hosts (`ParticleCurves::evaluate_*`) are trusted to return float-valued doubles: the
  image always spills a curve result to float before using it, and the C++ rounds explicitly only
  where the kind is checked (00B06B10 speed/wind). If a host ever returns an unrounded value, the
  colour and size products would diverge in the last bit.
- RNG: every weather draw is stream 0 except the bolt pick (stream 1). weather_render.cpp is not
  wired into `bsp_game.exe`, so the RNG RULE (no presentation draws on gameplay stream 0) does not
  bite yet; whoever wires it must route `RandomStream::primary` to a presentation stream.

## Handoff

Not sampled here and still unaudited: 00B05550 container geometry, the particle bounding-sphere
helpers, the `.mshd` layer/constant readers (00B5A1F0, 00B59E70), 00B151E0 stats overlay,
00B156C0 is covered by audit 1, the sky gradient helpers 00BBF280..00BBF590, the cloud instance
step 00BBE8B0, the transmittance 00BC3730, and the remaining routines of frontend_menus.cpp,
menu_screens_2.cpp and hud_widgets.cpp.
