# CC13 BIG47A: large segment 47 (flight) bodies

Packet `cc13-big47a` (marker `cc13_big47a`). Code: `include/bsp/cc13_big47a_plane.hpp`,
`src/cc13_big47a_plane.cpp`. Same model as `cc13_seg47` / `cc13_seg47b`: raw Win32 offsets,
every image call through a named host method (`Big47aHost : cc13_seg47b::Seg47bHost`) at the
binary's point. 007BAE90 is called through the existing `cc13_seg47::rate_limiter_step_007bae90`.
Reconstructed and build-tested only; nothing is wired, nothing is ABI-compatible. Names are
hypotheses.

Rule 1: `git grep` by address and name, `bsp.py lookup` / `docs-for`, and a search of the plane
hosts by behaviour (`0xA5C`, `0xA4C`, `+918h`, `+6A4h`, wing-tip / engine effect names) found no
existing implementation of the three bodies. `cc13_seg47b` (`007C6820`) and `engine_seg56_cc13`
(`0089E670`) call `007C4DD0` only as a host hook; `cc13_seg47` (`007C0C60`) calls `007BE9B0` the
same way. No adoption notes: nothing hosted to compare.

No function here touches plane level-bomb, goaway, torpedo or submarine AI. None draws from the
shared RNG `00BD2F10`.

## Outcomes

| Address | C++ | Outcome |
|---|---|---|
| `007C4DD0` | `plane_update_engine_effects_007c4dd0` | reconstructed (whole body, 007C4DD0-007C5ABC) |
| `007BEEE0` | `plane_tick_element_commit_step_pose_007beee0` | reconstructed (whole body, 007BEEE0-007BEFDC) |
| `007BE9B0` | `plane_apply_point_event_local_007be9b0` | reconstructed (whole body, 007BE9B0-007BEED8) |
| `007BEFE0` | none | skipped: unreferenced debug-draw body, no Ghidra function (below) |
| `007C0690` | none | skipped: unreferenced, no Ghidra function, outside the packet |

## 007C4DD0 BSP_Plane_UpdateEngineEffects

ABI: `__thiscall(ECX = plane; float dt)`, `RET 4`; SEH frame (handler `00C8C7F2`), frame
`ESP+0CCh` = EH state, `dt` at `ESP+0D4h`. Callers: `007C6BDE` in `007C6820` (plane tick) and
`0089E7D7` in `0089E670` (Lua SetTipEffect, passing 0.0 or 1.0).

Thread / precision: unit fixed step (from `007C6820`) or main thread (from the Lua binding). The
one multi-operation x87 chain, the wing-tip level at `007C503C`-`007C5058`, is modelled at 24-bit
PC (each operation rounded to float). At 53-bit PC it is
`float(double(1/(sum-base)) * double(y-base))` and can differ by one ulp. Single x87 operations
(`v - 0.5`, `v +/- dt`) round the same at either precision. The `007C5040` `DC E9` is
`ST1 = ST1 - ST0` = 1 - 0 (bytes checked); `DE E9` at `007C558B` / `007C5745` is
`ST1 = ST1 - ST0` = v - dt.

Body, in image order:
1. Wing-tip records: `class(+538h)+5A0h` records of 10h bytes at `plane+A5Ch` (`{handle, float
   point[3]}`). When the count is non-zero: `00414DB0(plane)` if `+C8h` is clear, then
   `plane+100h > [00E0B044]` (598.0f, `.data`, read from memory) selects:
   - maintain (`007C4E49`): a missing effect is created at `point` transformed by `plane+CCh`
     (identity matrix with that translation, group `[[00E188A8]+19ECh]`, definition handle
     `class+5A4h` retained into the by-value argument, `00868420`, assign, release the
     temporary). For a live effect: `effect+9 = 0`, `00866E60(effect; &child, 3)`; with a child,
     `level = clamp01((1/(sum-base)) * (y-base) + 0)` with `sum = [00E0B048] + [00E0B044]`
     (300 + 598) and `child->vtable[14h](level)`; release the child. A NaN level is passed through
     (`JBE`).
   - stop (`007C50F5`): every live effect gets `00867B10`, `effect+9 = 1`, release, clear.
2. Engine run test (`007C515F`): run when `+61h` and `+96Ah` are both set; otherwise fade when
   `55.55555725097656 (00D05B38) > +B1Ch`, or `0.9f (00D7A390) > |+9E8h|` (|x| as `x > 0 ? x :
   -0.0f - x`), or `+5Dh` is set; else run. NaN speed or throttle falls through to the next test
   (`JA`).
3. Run (`007C51D2`), over the checked vector `plane+A4Ch` (`A50h` first, `A54h` last, 10h records):
   a missing effect is created from `class+564h` record `index` (checked `begin != 0` and `index <
   (end-begin)>>4`, read even after a failed check), then `effect+9 = 0` and the child gets level
   0.0; a live effect's child moves its level `[child+1Ch]+19Ch` toward 0.5f by `dt` (snaps to 0.5f
   when `dt > |v - 0.5|`); no child stops and releases the effect.
4. Fade (`007C564F`): each live effect's child level moves toward 0 by `dt` (snaps to 0 when `dt
   > |v|`); when the level equals 0.0f (`UCOMISS`/`JP`, ordered equal only) or there is no child,
   the effect is stopped and released.
5. Attach (`007C5804`): every live vector effect gets `004842C0(effect; point transformed by the
   world)`; the world is refreshed inline when `+C8h` is clear (parent `+3Ch` refreshed with
   `00414DB0`, `00413920(plane+74h; tmp, parent+CCh)` or `+74h`, 16-float copy to `+CCh`, `+C8h =
   1`, `+10Ch = 0`).
6. The same for the four fixed wing-tip records at `+A5Ch`, whatever the class count (`007C5989`).

Every `00BF6713` iterator check is placed at its image address (comments carry them).

Uncertainty and simplifications:
- The EH unwind releases of temporary handles (states 0-5) are not modelled; no exception path is.
- The 16-float world copy is `FLD`/`FSTP float` in the image (quiets a signalling NaN); the C++
  copy is bitwise.
- The container-null iterator test (`LEA EAX,[EBX+A4Ch]; TEST EAX,EAX`) is written as a null test
  on `plane+A4Ch`; a compiler may fold it away.
- Field meanings (`+5A4h` = WingTipEfx per `plane_class_fields.hpp`; `+564h` engine definitions;
  `+B1Ch` speed; `+9E8h` throttle) are hypotheses.

## 007BEEE0 BSP_PlaneTickElement_CommitStepPose

ABI: `__thiscall(ECX = tick element)`, `RET 0`, frame 4Ch. Tick-element vtable slot `+0Ch`; the
nine vtable references are `00D00038`, `00D002D0`, `00D05EE8`, `00D06600`, `00D068E8`,
`00D0BA48`, `00D19CF0`, `00D19FC8`, `00D1A2A0` (absolute scan; no direct calls). Offsets are kept
element-relative; with the element at `plane+310h` (docs/PLANE_UNIT_TICK.md) they are plane
`+3Ch` parent, `+6A4h` point, `+918h` published point, `+674h` / `+924h` / `+74h` / `+4E0h`
matrices and `+4D8h` flag.

Body: with a parent, refresh it if `+C8h` is clear (`00414DB0`), publish the point transformed by
`parent+CCh` (`004142E0`), re-read the parent, refresh again if needed, and take
`00413920(element+364h; tmp, parent+CCh)`; without one, copy the point (`FLD`/`FSTP float`) and
take `element+364h`. Then `004134F0(element+614h; that)`, and `004134F0(element+364h;
element+1C8h ? element+1D0h : element-29Ch)`.

Thread: unit fixed step. No arithmetic outside the callees.

The Ghidra function is only `007BEEE0`-`007BEFDD`. The "18C0h" size the earlier passes gave runs
to `007C07A0` across two bodies Ghidra has not defined, separated by INT3 padding
(`007BEFDD`-`007BEFDF`, `007C0687`-`007C068F`, `007C0797`-`007C079F`):
- `007BEFE0`-`007C0686`: a plane debug-draw routine (11 calls to `0085F990
  BSP_DebugDraw_LineRgbaFloats`, two to `00B6DB70`, reads `[[00E188A8]+19FCh]`). No rel32
  call/jump and no absolute dword in any section references it, so it is never called. Skipped:
  dead code, and debug drawing only.
- `007C0690`-`007C0796`: a small body (reads `+C64h`, |x| via `-0.0f - x`). Also unreferenced.
  Skipped.

## 007BE9B0 BSP_Plane_ApplyPointEventLocal

ABI: `__thiscall(ECX = target; float a[3], b[3], c[3], float time by value)`, `RET 28h`, frame
`0F0h` after `PUSH ESI`. Only caller `007C0CE0` in `007C0C60` (plane vtable `+4Ch`) with ECX =
`plane+84Ch`.

Body:
1. When `+A8h > 0` and `+A8h > time`: `+A8h = -1.0f`, `+40h = 1`. Then `previous = +A8h`,
   `+A8h = time`.
2. `0 > previous` (re-seed): `+44h = 0`, `+40h = 1`; when the owner `+ACh` and its class `+538h`
   exist, `+A0h = 1.5f`, `+A4h = 3.0f`, `+58h/+70h/+88h = class+1C4h/+1C0h/+1BCh` and
   `+5Ch/+74h/+8Ch` = those times 3.0 (double `00D7A2B0`).
3. Otherwise `step = time - previous`; when `step > 0`: speed `+44h = |row3 - a| / step`
   (`0042B2F0`), row 3 (`+30h`) = the BSS vector `00F87574` (read from memory), the inverse
   `00B63D50(tmp; target)`, b and c into that frame (`004142E0`), an identity basis (`004134F0`)
   with row `+20h` = b' and row `+10h` = c', `0085DC80`, angles `0042D2E0(basis, EDX = e; s1,
   s2)`, each divided by the step; levels `speed/class+188h` (floor 0.1f below the double 0.1f
   test), `s2/class+1A8h`, `e/class+1ACh`, `s1/class+1B0h` (each clamped to [-1, 1]; NaN passes
   through). With `+40h` set they are stored to `+48h/+60h/+78h/+90h` (s1, e, s2, speed) and `+40h`
   cleared; otherwise `007BAE90` steps each of those limiters toward its level by `step`.
4. Always: rows `+30h = a`, `+20h = b`, `+10h = c`, then `0085DC80(target)`.

Thread: the `007C0C60` caller's (not established). Precision: every x87 result is stored to a
float after one operation (the `DC C9` at `007BEA89` is `ST1 = ST1 * ST0`, bytes checked; the
`DE FA` at `007BEC9F` is `ST2 = ST2 / ST0`), and the three products are float x 3.0 (exact in
53 bits), so 24- and 53-bit PC agree.

Uncertainty: field meanings are hypotheses; which of the three `0042D2E0` outputs is pitch / yaw /
roll is not established, so the C++ names them by how they are passed.
