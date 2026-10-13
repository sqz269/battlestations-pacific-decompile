# 00B529A0: the old-film post effect (cc13-render2b)

The last open body of segments 84-86 (see docs/CC13_RENDER_1.md). Reconstructed as
`old_film_render_00b529a0` in `src/render_fx_cc13.cpp` (declaration and host interface
`OldFilmHost` in `include/bsp/render_fx_cc13.hpp`, namespace `bsp::cc13_render1`).

Status: *reconstructed, build-tested*. Not fixture-tested, ABI-compatible or
game-validated, and not wired into any host. The name is a descriptive hypothesis.

Evidence: the full capstone listing of the disk bytes, `00B529A0..00B5409C` (about 1580
instructions, `RET 14h`). The x87 stack was followed per basic block, and stack slots were
tracked across the `SUB ESP, 8` argument blocks of every 00BD2F10 call. Values are
evaluated in double and rounded at each `FSTP float`. The uniforms are kept as the double
ST0 value the image multiplies before it rounds. Constants come from the PE: 00D7A308 2.0,
00D7A280 0.5, 00D7A220 100.0, 00CE3DC0 10.0, 00CE47A0 1000.0, 00CE3D10 (double)0.2f,
00D7A250 -1.0, 00D7A210 1.0, 00D7A348 0.25, 00CEC9D8 0.75, and the floats 0.5, 1.5,
0.7, 1.3, 0.2, 0.3, 1.7, -1.0, 10, 20, 0.1, 0.8, 1.2, -0.5 and -2.0 (the empty-record
sentinel, 00CE7D7C).

## RNG (cc9 rule)

The image draws every uniform from 00BD2F10 (`BSP_Random_UniformFloatRange(NULL, lo,
hi)`) and every integer from 00BD2FC0 (`BSP_RandomThreads_NextU32`). Both are on the
SHARED GAMEPLAY STREAM 0. The model takes them from `OldFilmHost::uniform_00bd2f10` and
`next_u32_00bd2fc0`, in the image's draw order. A host must route them to a presentation
stream or keep the effect OFF. The draw count per period depends on the parameters and on
the line states, so an effect that draws from stream 0 shifts every later gameplay draw.

## ABI and corrections to the earlier map

`__thiscall(state, targets, const float* params, char extended, float a, float b)`,
`RET 14h`; the only caller is 00B156C0.

- Every float-to-int conversion is a truncation (`FLDCW` with RC = 11b). The decompile
  shows them as `ROUND`.
- The integer draws are unsigned remainders (`DIV`). They are converted with `FILD` plus
  2^32 when the sign bit is set, which is an unsigned conversion.
- The "short line" test at 00B53635 is `i < [ESP+60h]`, and that slot holds n2 on every
  frame (00B52BE2). The handoff map's "i >= n2" was the decompile's inverted flag.
- The globals are written at the end of the update in the order +08h, +0Ch, +10h, +14h
  (both +08h and +0Ch are U(0, 1)).

## Layout (by use)

| Offset | Use |
| --- | --- |
| +08h, +0Ch | U(0, 1) per period |
| +10h | jitter U(1 - params[22]*m, 1), m = 1.0 extended else 0.2 |
| +14h | shake: 0, or U(-0.5, 0.5)*params[23]*k_a while the +BCh toggle is on |
| +18h, +70h, +98h | textures of the dust, line and flicker children |
| +1Ch, +40h, +74h, +9Ch | atlas column counts (dust, hair, lines, flicker) |
| +20h / +30h | cached params[3] / dust child; +24h dust records; +34h/+38h dust toggle/timer |
| +44h..+64h | the hair record (inline 24h); +64h colour 0 = hidden; +68h hair child; +6Ch hair timer |
| +78h | line count n1 + n2; +7Ch line records (n1+n2 lines + n1 twins); +88h line child |
| +8Ch | line states, 0Ch each: {active byte, flag byte, timer, amplitude} |
| +A0h / +B0h | cached params[18] / flicker child; +A4h flicker records; +B4h/+B8h flicker toggle/timer |
| +BCh/+C0h | shake toggle/timer; +C4h period timer; +C8h previous clock |

Records are 24h: {x0, y0, x1, y1, u0, v0, u1, v1, ARGB}. The sentinel record is four -2.0f,
four 0 and colour 0.

## Behaviour

1. Shared terms: k_a = f(2/a); aspect2 = f(2*(h/w)) from colour surface 0;
   bb = f(0.5 - (0.5 - b)/a).
2. Rebuilds:
   - Dust: when params[3] differs from +20h (including NaN), cache it, release +30h, and
     when it is > 0 create a child with 2*trunc(params[3]*100) quads.
   - Lines: n1 = trunc(params[8]*10) and n2 = trunc(params[13]*1000) every frame. When
     n1 + n2 differs from +78h: release +88h. If the new count is non-zero, push n1 + n2
     records and states (active = NextU32 & 1), then n1 twin records, and create a child
     with 2 * record-count quads.
   - Flicker: the same as dust with params[18]*10.
   - Each rebuild zeroes +C4h.
3. Timers: dt = f(now - +C8h), or 0 on the first frame; +C8h = now. Subtract dt from
   +38h, +6Ch, every line timer, +B8h, +C0h and +C4h. When +C4h <= 0 it gets
   f(1/params[0]) added (clamped to 0 when still <= 0), and the update runs:
   - **Dust**: the toggle timer (re-armed with U(0.5, 1.5) * (on ? params[2] :
     params[1])); density U(0.7, 1.3) or U(0.2, 0.3), times 0.2 when not extended, times
     U(0.2, 1). n = trunc(half * density) specks, each:
     - x = U(-1 - w4, 1), y = U(-1, h4 + 1), column = NextU32 % columns
     - x1 = x + U(0.3, 1.7)*w4, y1 = y - U(0.3, 1.7)*h4 (w4, h4 = params[4] times aspect2 / k_a)
     - colour 70FFFFFFh or A0000000h from NextU32 & 1
     The rest of the half is padded with sentinels.
   - **Hair**: when params[5] > 0 and its timer has run out: timer U(0.3, 1.7)*params[5];
     size U(params[6], params[7]); width and height U(0.5, 1.5)*size*aspect2 and *k_a;
     x0 = U(-1 - w, 1), y0 = U(bb - 1, (1 - bb) + h); a random column; colour FF000000h.
     Otherwise the colour word is cleared (hidden).
   - **Lines**: when a line's timer has run out, its flag is cleared and active toggles.
     Outside extended mode a line that turns on stays on only with probability 1/3
     (NextU32 % 3 == 0). An active line gets:
     - width U(0.7, 1.3)*aspect2*(short ? params[14] : params[9]), x = U(-1 - width, 1),
       spanning y 1 to -1
     - short lines: a vertical segment of length U(0.1, 0.3) at U(0, 1 - length), colour
       alpha NextU32 % (extended ? 9 : 7) + 1, amplitude scale params[15]
     - long lines: either alpha (NextU32 & 3F) + 40h, or with the flag set colour
       4000FF40h (green), else black; amplitude scale params[10]
     - amplitude U(-1, 1)*scale
     Inactive lines get the sentinel. Every visited line gets timer U(params[11],
     params[12]). Each active line then moves by f((sin(f(U(10, 20)*timer)) + 1) *
     amplitude * f(1/params[0])) and picks a column NextU32 % (columns - 1) + 1, with
     u = (col - 0.25)/columns and (col + 0.75)/columns. The long lines with the flag set get
     a twin record shifted left by half their width, with its own column and colour
     FF000000h.
   - **Flicker** (extended only): the toggle timer (params[17] / params[16]); while on,
     n = max(1, trunc(U(0.2, 1)*half)) blotches:
     - size U(params[19], params[20]); width and height U(0.8, 1.2)*size*aspect2 and *k_a
     - x0 = U(-1 - w, 1), y0 = U(bb - 1, h + 1 - bb); a random column
     - colour alpha NextU32 % 60h + 50h, tint 473E1Ah with probability 1/4 (the tint draw
       comes first)
     The rest of the half is padded with sentinels.
   - **Globals** +08h..+14h as in the layout table. The shake toggle uses params[25] /
     params[24].
4. Draws, each as 00B4D4C0 (populate from its records), 00B4E3D0 (frame target =
   `targets`), 00B4DEF0 (submit): dust when +30h is set, hair when +64h is non-zero, lines
   when +88h is set, flicker when extended, +B0h is set and +B4h is on.

## Uninitialised stack data in the image

The line rebuild pushes a 24h record from an uninitialised stack buffer ([ESP+68h]), and
line states whose bytes 1..3 and word +08h are stale stack words. The rebuild zeroes
+C4h, so the update runs in the same call when dt >= 0. The update then writes every one
of those words before anything reads them:
- the line loop writes the flag byte and all nine record words, or the sentinel
- the amplitude is written whenever a line activates
- the twin loop writes every twin record

The model therefore uses zeroed values. With a backwards clock (dt < 0) the image could
draw the stale record once.

## Open questions

- The shader meaning of +08h/+0Ch/+10h/+14h; the names above follow how they are drawn.
- Whether the green flagged long lines (4000FF40h) are intended or a debug colour.
