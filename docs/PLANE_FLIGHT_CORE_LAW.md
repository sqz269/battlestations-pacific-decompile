# The plane flight core law (`007DB680`)

Addresses: 007DB680, 007D7C00, 007D81B0, 007D8470, 007D9050, 007D9140, 007D92B0, 007D9C10,
007D9C80, 007D9E80, 007D9EE0, 0074E1E0, 007C6340, 007C7110, 007C1430, and read-only 007DC830,
007DCCF0, 007DCDD0, 007CC2F0, 007CBFA0, 007CBA50, 007CFD20, 007BBC50, 0042D0D0, 00419010,
0085DEA0, 007F2920, 007F4580, 00D05F20+3Ch.

Worker `agent/cc7-plane-flight-law`, packet `cc7_plane_flight_core_law`, 2026-09-14 UTC.
Ghidra was **read-only**: no rename, comment, prototype, function creation or save. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. The installation is modded
(BSPRM/AlterBSP); nothing in it was written. Class values below are quoted from
`docs/PLANE_FLIGHT.md`'s installed `vehicleclasses.lua` row (the first `"Fighter"`,
"Shooting Star"), which this packet did not re-read from disk.

`docs/PLANE_UNIT_TICK.md` step 5 said no plane can hold altitude until this body is read.
It is read here. **The law is complete, closed, and reachable: nothing in it needs an
aerodynamic model the host does not have.** The one thing standing between the reconstruction
and a flying plane is a single integer field, `unit+900h`, and its producer is named below.

---

## 1. ABI

```
007db680  SUB ESP,0x48          ; entry
007db683  FLD float ptr [ESP+0x4c]   ; = [E+4], the first stack argument
...
007dc828  RET 0x10              ; callee pops 16 bytes = four stack arguments
```

`__thiscall(void* ctl, float step, float a, float b, void* p)`.

`ECX` is the controller `unit+0AB0h` (`007db68e MOV ESI,ECX`, and every field access below is
through `ESI`). The argument count is the **`RET 0x10`** at `007DC828`, not the pushes at any
one call site; all three call sites match it (§3). `p` is a pointer, not a flag: the water arm
passes `LEA EDX,[ESP+0x34]` (`007DD43F PUSH EDX`) and `007DC422 FLD float ptr [ECX]` loads
through it after `007DC3DC MOV ECX,dword ptr [ESP+0x68]`. `docs/PLANE_FLIGHT.md` line 130
records the fifth parameter as `flag`; **correction:** it is a `float*`, null on two of the
three arms.

Ghidra body `007DB680`-`007DC82A`, 1214 instructions.

## 2. The objects

`ctl = unit+0AB0h` (constructor `007D7EA0`, layout table in `docs/PLANE_FLIGHT.md`). The fields
this law touches, with the site that establishes each:

| field | meaning | evidence |
| --- | --- | --- |
| `ctl+4h`, `ctl+5h` | two control bytes copied from `unit+9F9h`/`+9F8h` each step | `007DB784`, `007DB77D` read them; `007DC84F`..`007DC86B` writes them |
| `ctl+8h` | **the unit** | `007DB6BB MOV ECX,[ESI+8]` then `[ECX+0x904]`, `[ECX+0x9E4]`, `[ECX+0xBB0]` |
| `ctl+0Ch` | **the plane class descriptor** (`unit+538h`) | `007DB750 MOV EDI,[ESI+0xc]`, `007DB760 FDIV [EDI+0x184]` = StallSpd |
| `ctl+10h` | **the dynamics accumulator**, `operator new(0D0h)`; called `dyn` below | `007DB6AB`, and every `[EAX+0x1c..0x2c]` write |
| `ctl+18h`,`+1Ch`,`+20h` | **world-frame linear velocity** | `007DBA32`-`007DBA63` takes its length; `007D9C80` fills it |
| `ctl+24h`,`+28h`,`+2Ch` | world-frame angular velocity | `007DC830` is its only writer (§8) |
| `ctl+3Ch`,`+40h`,`+44h` | **body-frame linear velocity** `(vx, vy, vz)` | `007D9C10` fills it from `ctl+18h` through `ctl+0B0h`; `007D99C0` returns `ctl+44h` when `unit+3Ch` is null |
| `ctl+48h`..`+50h` | body-frame angular velocity | `007D9C80`'s second transform |
| `ctl+90h` | a ramp timer, `99.0f` at construction | `007D7F3F` (ctor), `007DB825`, `007DB86F` — the only three writers in `007B0000`-`007F8390` |
| `ctl+94h` | a byte gate on two extra terms | `007DB9DE`, `007DBC77` |
| `ctl+98h` | extra-drag multiplier, `1.0f` at construction | `007D7F5D`; `007DBBE2` reads it. **No other writer** in `007B0000`-`007F8390` |
| `ctl+9Ch` | lift-ratio multiplier, `1.0f` at construction | `007D7F65`; `007DB8CF` reads it. **No other writer** in `007B0000`-`007F8390` |
| `ctl+0B0h` | the **world-to-body** matrix, rebuilt every step | `007D9C10` calls `0085DEA0(ctl+0B0h, unit+74h)` then transforms `ctl+18h` into `ctl+3Ch` with it |
| `ctl+0FCh` | the arm mode: **0 free flight, 1 ground, 2 water** | `007DC841` writes 0, `007DCD24` writes 1, `007DCDDC` writes 2; `007DB6D1`/`007DBE0E`/`007DBEAA` branch on it |

`dyn = ctl+10h` is six three-float accumulators plus state. `007D7C00(dyn, &ctl+18h, &ctl+3Ch)`
resets them at the top of every step:

```
007d7c04  dyn[0x4c..0x54] = ctl[0x18..0x20]      ; world velocity in
007d7c19  dyn[0x64..0x6c] = ctl[0x3c..0x44]      ; body velocity in
007d7c2a  dyn[0x04..0x48] = [00F87574..7C] x 6   ; six vectors cleared to one constant
007d7d17  dyn[0x94] = 0
007d7d1f  RET 0x8
```

The six cleared vectors pair up as (body accumulator, world accumulator). `007D8470` folds each
world one into its body one with `0042D0D0(out, src, ctl+0B0h, 0)`:

```
007d8480  LEA EDX,[EDI+0x28]      ; V4 (world)   -> V3 (body) at +1Ch    007d848e FADD [EDI+0x1c]
007d8499  LEA EDX,[EDI+0x10]      ; V2 (world)   -> V1 (body) at +04h    007d84b9 LEA EBP,[EDI+4]
007d84c2  LEA EDX,[EDI+0x34]      ; V5 (world)   -> V6 (body) at +40h    007d84e6 LEA EDX,[EDI+0x40]
```

`0042D0D0` is `out.x = v.x*M[0] + v.y*M[0x10] + v.z*M[0x20]` (`0042D0F2`-`0042D148`), i.e. the
row-vector product `v * M` over a 16-byte-stride matrix. With `M = ctl+0B0h` (world-to-body)
that converts a world vector into body coordinates.

| accumulator | frame | components used by this law |
| --- | --- | --- |
| `dyn+04h`,`+08h`,`+0Ch` | body | `+04h` lateral drag, `+08h` vertical drag, `+0Ch` ceiling / dead-meat |
| `dyn+10h`,`+14h`,`+18h` | world | velocity-aligned drag |
| `dyn+1Ch`,`+20h`,`+24h` | body | **`+20h` lift**, **`+24h` thrust** |
| `dyn+28h`,`+2Ch`,`+30h` | world | **`+2Ch` gravity**, ceiling force |
| `dyn+34h`..`+48h` | world / body | written only by the ground branch (`007DC1A4`, `007DC1FD`) |
| `dyn+4Ch`..`+54h` | world | velocity out; the tail copies it to `ctl+18h` |
| `dyn+64h`..`+6Ch` | body | velocity out; the tail copies it to `ctl+3Ch` |

So the axis convention is `(x, y, z) = (lateral, up, forward)`: lift lands on `+20h` (the body
`y`) and thrust on `+24h` (the body `z`), gravity on `+2Ch` (the world `y`).

## 3. The caller census

Method: a byte-level scan of every mapped section of
`I:/.../battlestationspacific.exe` for `E8`/`E9 rel32` whose target is `007DB680`, plus a scan
of every section for the absolute dword `007DB680` (vtable slots and pointer tables). Script
`local/callcensus.py` in this worktree; sections scanned `.text 00401000+8E0136`,
`.rdata 00CE2000+125B24`, `.data 00E08000` (raw `10000h`; everything past `00E18000` is
zero-filled at load, so no pointer can live there), `.rsrc`.

**Exhaustive for the image on disk: three `E8` sites, no `E9`, no absolute reference.**
Ghidra's `callers` agrees here; it does not always — `callers 007F2920` returns 0 while the byte
scan finds four `E8` sites (§6), which is why the census is the evidence and the call graph is not.

| site | containing function | arguments | `ctl+FCh` |
| --- | --- | --- | --- |
| `007DC86E` | `007DC830 BSP_PlaneFlight_FreeFlightStep` | `(ctl, step, 0.0f, 0.0f, nullptr)` | 0 |
| `007DCD64` | `007DCCF0 BSP_PlaneFlight_GroundRollLaw` | `(ctl, step, argByte==0 ? 1.0f : 0.0f, 0.0f, nullptr)` | 1 |
| `007DD48E` | `007DCDD0 BSP_PlaneFlight_WaterSurfaceLaw` | `(ctl, step, a, b, &local)` | 2 |

Evidence for the free-flight row: `007DC830` is `SUB ESP,0x30` / **`D9 EE FLDZ`** / `PUSH EBP`
/ `PUSH ESI` / `MOV ESI,ECX` / `MOV EAX,[ESI+8]` / **`6A 00 PUSH 0`** / `SUB ESP,0xC`, then
`007DC84B FST [ESP+8]` and `007DC866 FSTP [ESP]`-adjacent stores put that zero in both float
slots (bytes `83 ec 30 d9 ee 55 56 8b f1 8b 46 08 6a 00 83 ec 0c ...` at `007DC830`).
For the ground row the fourth slot comes from `007DCD45 PUSH ECX` with `ECX` still zero from
`007DCCF7 XOR ECX,ECX`, plus `007DCD46 SUB ESP,0xC` — 16 bytes, matching `RET 0x10`.

Each arm is selected by the motion dispatch `007CEC30`-`007CECBA` inside `007CE040`
(`docs/PLANE_UNIT_TICK.md`, read here as a contract):
free flight `007CC2F0` -> `007DC830`; ground roll `007CBFA0` -> `007DCCF0`; surface `007CBA50`
-> `007DCDD0`. `007DCCF0` additionally delegates to `007DC830` when
`unit+0BF8h == 0 && unit+900h != 5` (`007DCCF9`, `007DCD01`, `007DCD14`), so the free-flight
body is also what a plane on the ground without a deck owner runs.

## 4. The law, as arithmetic

Constants: `tuning[+X]` is the `Dynamics/*` mirror at `00F870E0 + X`
(`docs/PLANE_FLIGHT.md` §"the mirror"); `desc[+X]` is the plane class descriptor `unit+538h`.
`9.81` is the double at `00CF9058`.

### 4.1 Prologue (`007DB680`-`007DB6B9`)

```
007d81b0(ctl, step)     ; contact timers: dyn[0xc8] -= step while positive, and
                        ; dyn[0xc8] = -1 when (*(unit+72Ch))->vtable[+38h]() is false
007d9c10(ctl)           ; ctl[0xb0] = inverse(unit+74h) via 0085dea0
                        ; ctl[0x3c..0x44] = ctl[0x18..0x20] * ctl[0xb0]   (world -> body)
007da710(ctl, step)     ; BSP_PlaneFlight_ControlRateLaw, docs/PLANE_FLIGHT.md
007d7c00(dyn, &ctl[0x18], &ctl[0x3c])   ; clear the six accumulators, seed the velocities
```

### 4.2 Ground short-circuit (`007DB6BB`-`007DB743`)

```
007db6d7  if (ctl[0xfc] == 1) {
007db6f7      if (a == 0.0f) goto commit_tail(007DC696)       ; UCOMISS/LAHF/TEST AH,44/JNP
007db702      dyn[0x94] = desc[0x1fc] - unit[0xbfc]           ; gear contact height
007db71c      dyn[0x98..0xa0] = (0, 1, 0)                     ; contact normal, world up
          }
```

`JNP` after `UCOMISS`/`LAHF`/`TEST AH,0x44` is taken on **equal** (ZF=1, PF=0 -> one bit set ->
odd parity), so `a == 0` skips the entire law and runs only the commit tail.

### 4.3 Thrust (`007DB744`-`007DB80C`) — `dyn+24h`, body forward

```
spdRatio = 007d99c0(ctl) / desc[0x184]            ; ForwardSpeed / StallSpd     007db760
if (unit[0xbbc] > 0.01f) {                        ; throttle                    007db76c
    t   = 007d9050(desc, unit[0xc64], unit[0xbbc], (int)ctl[5])   ; EDX = ctl[4] 007db7a6
    k   = (modifier system live) ? 008e6430(6, unit) : 1.0f       ;             007db7db
    dyn[0x24] += k * unit[0xcc8] * t                              ;             007db80a
}
```

`007D9050(desc, pitch, throttle, boost)` (`RET 0xC`, `__fastcall` with `EDX` = a fourth byte):

```
007d9052  a = desc[0x164] Accel * throttle
007d9062  if (EDX)      a *= tuning[+330h] SpdMultipliers/TurboMultiplier (1.95)
007d9077  if (boost)    a *= desc[0x604]
007d9091  if (pitch < 0) {                                   ; nose down
007d90cb      u = InterpolateClamped(tuning[+328h] AccelCheatFallPitchRange/1 DEG(10), 0,
                                     tuning[+32Ch] AccelCheatFallPitchRange/2 DEG(60), PI/2, -pitch)
007d90f3      a *= 1 + (tuning[+324h] AccelCheatFallMul (2.6) - 1) * sin(u)
          }
007d9104  return clamp(a, 0, 100)
```

So `unit+0C64h` is the **pitch angle in radians** (negative nose-down) and `unit+0BBCh` the
throttle. A 60-degree dive multiplies thrust by 2.6.

### 4.4 The lift ramp timer (`007DB80D`-`007DB874`) — `ctl+90h`

```
007db814  if (unit && 007bbc50(unit))  ctl[0x90] = 3.0f      ; 007BBC50 is true only when
                                                              ; unit+900h == 4 and unit+0BF8h
                                                              ; and the owner chain resolves
007db84a  else if (ctl[0x90] < 3.0f)   ctl[0x90] += step
007db861  else if (ctl[0x90] < 6.0f)   ctl[0x90] += 3 * step
```

Construction leaves it at `99.0f` (`007D7F3F`). A scan of `007B0000`-`007F8390` for direct
`[reg+disp32]` float and dword stores with `disp == 0x90` finds exactly three writers of
`ctl+90h`: the constructor, `007DB825` and `007DB86F`. **Therefore `ctl+90h >= 3.0` always**,
and the `min` in §4.5 can never reduce the lift — the timer is inert for level flight. This is
worth stating because it is the only candidate for a "lift ramps in after spawn" effect, and
there is none.

### 4.5 Lift (`007DB875`-`007DB98D`) — `dyn+20h`, body up

```
vy   = ctl[0x40]                                  ; body-frame vertical velocity
vz   = ctl[0x44]                                  ; body-frame forward velocity
007db8b1  aoa = (fabs(vz) >= 0.1f) ? (-vy / vz) : 0.0f
007db8cf  q   = (007d99c0(ctl) / desc[0x184] StallSpd) / tuning[+24Ch] LevelFlight (1.8)
                * ctl[0x9c]                                              ; ctl[0x9c] == 1.0
007db8e3  qq  = (q < 1.0f) ? q * q : 1.0f
007db8f1  L   = (1.0f + aoa) * qq
007db91d  v   = clamp(L, -2.0f, +2.0f)
007db951  m   = min(v * tuning[+31Ch] AccelCheatMul (1.5), ctl[0x90])    ; == v * 1.5, §4.4
007db98a  dyn[0x20] += m * 9.81f
```

The x87 sequence at `007DB8DD`-`007DB8F1` is a two-path square: `FCOMI ST0,ST1` against `1.0`,
then `FLD ST2 / FMULP ST3` on the `q < 1` path and `FSTP ST2 / FLD ST1` (substituting 1.0) on
the other; both leave `qq` in `ST2` and `1 + aoa` in `ST0` at `007DB8F1`.

### 4.6 Gravity (`007DB990`-`007DB9DD`) — `dyn+2Ch`, world down

```
007db9c4  g = InterpolateClamped(0, 1.0f,
                                 tuning[+264h] DeadMeat/LostDragTime (5),
                                 tuning[+268h] DeadMeat/ExtraGravityMul (1.5),
                                 unit[0xc3c])
007db9db  dyn[0x2c] -= tuning[+31Ch] AccelCheatMul (1.5) * 9.81f * g
```

`00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x)` returns `y0` when `x1 == x0`
(`0041901E` `FUCOMIP`, `00419030 RET 0x14`) and otherwise
`clamp(y0 + (y1 - y0)*(x - x0)/(x1 - x0), min(y0,y1), max(y0,y1))` (`00419033`-`004190CE`).
`unit+0C3Ch` is the **dead-meat elapsed time in seconds**: it keys `LostDragTime` and
`ExtraGravityMul` here and the aero fade in §4.7. A healthy plane has `unit+0C3Ch == 0`, so
`g == 1` and gravity is a flat **`-14.715 m/s^2`**.

A second gravity application at `007DB9E7`-`007DBA2F` runs only when `ctl+94h` is set; it is the
same `1.5 * 9.81 * InterpolateClamped(1, 0, 2.5, 3.0, unit[0xc3c])` shape on `dyn+2Ch`.

### 4.7 Drag (`007DBA32`-`007DBC76`) — `dyn+10h`, world, along the velocity

```
007dba63  s2    = |ctl[0x18..0x20]|^2
007dba75  if (s2 <= 1e-10) { speed = 0; skip to 007DBD37 }
007dba7b  speed = sqrt(s2)
007dba99  if (speed <= 0.001f) skip to 007DBD37
007dbab3  n     = ctl[0x18..0x20] / speed                       ; unit velocity direction
007dbb0e  r1    = InterpolateClamped(0, 1.0f, tuning[+264h] LostDragTime (5), 0, unit[0xc3c])
007dbb23  r2    = InterpolateClamped(-0.3f, r1, 0.1f, 1.0f, unit[0xc64])      ; pitch
007dbb5e  d     = 007d9140(desc, speed, unit[0x9e4 + 0xc], unit[0xc64],
                           unit[0xbb0 + 0x10], unit[0xbb0 + 4]) * r2
007dbb72  if (ctl[0x44] < 0)  d *= 5.0f                        ; flying backwards
007dbbc6  dyn[0x10..0x18] += n * d
007dbbf3  if (ctl[0x98] > 1.0f)  dyn[0x10..0x18] += (d*n.x, 0, 0) * (ctl[0x98] - 1)
```

`007D9140` (`RET 0x14`, `__fastcall(desc, ...)`) is the signed drag magnitude. Its tail
(`007D926C`-`007D92A2`) computes `sign = -sgn(x)`, `FILD`s it and multiplies it through, so `d`
is **negative for forward motion** — it decelerates along `n`. It reads
`desc+208h`, `desc+1D4h DragPitchRatio`, `desc+50Ch`, `desc+1DCh AirBrakeDrag`,
`desc+188h MaxSpd`, and `tuning[+310h] MaxDragSpdMul`, `[+314h] MinDragSpdMul`,
`[+318h] MaxDragPitch`. `coverage: partial` — the ordering of its five stack arguments past the
first was taken from the call site, and `007D9287`-`007D929B` is the only part of its tail
transcribed instruction by instruction.

### 4.8 Body-frame damping (`007DBD37`-`007DBE0D`) — `dyn+4h`, `dyn+8h`

```
007dbd61  r = 007d92b0(spdRatio)                                 ; spdRatio from 007DB773
007dbd7d  dyn[0x04] += r * (-ctl[0x3c] * desc[0x174] XDrag)      ; lateral
007dbd87  dyn[0x08] += r * (-ctl[0x40] * desc[0x170] YDrag)      ; vertical
007dbe0a  dyn[0x08]  = clamp(dyn[0x08], -c, +c),
              c = InterpolateClamped(0.5f, 1.0f, 20.0f, 100.0f, unit[0x908])
```

`007D92B0(float ratio)` (`RET 4`) is the dynamic-pressure response curve:

```
007d92db  u = InterpolateClamped(tuning[+244h] SpdMultipliers/DragRangeMin (1.0), 0,
                                 tuning[+248h] SpdMultipliers/DragRangeMax (2.0), 1.0, ratio)
007d92f2  if (u == 0) return 0
007d9325  return pow(|u|, tuning[+228h] DragFuncPower (1.8))     ; FYL2X/F2XM1/FSCALE
```

`desc+170h` and `desc+174h` are the Lua keys **`YDrag`** and **`XDrag`**: `007D2167
FSTP [ESI+0x170]` is preceded by `007D2144 PUSH 0xD06500` and `00D06500` holds `"YDrag"`;
`007D21A0 FSTP [ESI+0x174]` by `007D217D PUSH 0xD064F8` and `00D064F8` holds `"XDrag"`.
`007D1F70 BSP_PlaneClass_ReadLuaFields` is the writer.

### 4.9 The ceiling (`007DBE0E`-`007DBEA8`), free flight only

```
007dbe16  if (ctl[0xfc] != 0) goto the ground / water branches
007dbe29  if (unit[0xc8]) 00414db0(unit)                      ; refresh the world pose
007dbe34  f = -tuning[+214h] CeilingForce (0.1) * (unit[0x100] - tuning[+210h] Ceiling (1500))
007dbe58  if (f < 0) {                                        ; i.e. altitude above 1500 m
007dbe6b      dyn[0x2c] += f                                  ; world down
007dbea2      dyn[0x0c] += InterpolateClamped(1.5f, 1.0f, 0.5f, 0, spdRatio) * f
          }
```

`unit+0FCh`/`+100h`/`+104h` is the **world position**; `unit+100h` is the altitude.

### 4.10 The ground and water branches

`007DBEAA`-`007DC204` (`ctl+FCh == 1`) is the ground arm: wheel brake `desc+1E0h`, a
`00415620 BSP_Math_ClampFloatByRef` over `ctl+68h` against `tuning[+29Ch]`/`[+2A0h]`
`WheelFrictionAccel/1,2`, the arrestor wire (`0042E740`-> `tuning+518h WireRope`, then
`tuning+51Ch` through `00415510`), the ground-contact owner through `006049F0
BSP_Plane_GetGroundContactOwner`, and `007DC1A4 dyn[0x48] +=` / `007DC1FD dyn[0x40] +=`.
`coverage: partial` — every instruction was read, but only the terms that reach `dyn` are
transcribed here.

`007DC205`-`007DC68C` (`ctl+FCh == 2`) is the water arm: buoyancy from `unit+0C3Ch` and the
fifth argument, `tuning[+2D8h] Water/LiftDepthRatio`, `[+2E0h] Water/DecelSpeed`,
`[+2E4h] Water/SideDragRatio`, `[+2E8h] Water/MaxSideDrag`, `[+300h] Water/LiftBeginDiveMul`,
`desc+508h`, and `007DC41C dyn+0x28 ...` through the `float*` argument.
**`coverage: partial` — `007DC426`-`007DC64A` was not read.**

### 4.11 The commit tail (`007DC696`-`007DC828`), reached by every path

```
007dc69d  dyn[0xc4] = unit[0x904]                               ; latched at 007DB6BE
007dc6b9  if (!(*(unit+72Ch))->vtable[+38h]())  dyn[0xc0] = 0    ; the free-flight gate again
007dc6e6  007d8470(dyn, step, unit+74h, ctl+0B0h)               ; the integrator
007dc6f4  ctl[0x3c..0x44] = dyn[0x64..0x6c]                     ; body velocity out
007dc714  ctl[0x18..0x20] = dyn[0x4c..0x54]                     ; world velocity out
007dc72b  ctl[0x54..0x5c] = dyn[0x58..0x60]
007dc74a  ctl[0x60..0x68] = dyn[0x7c..0x84]
007dc743  if (b >= 0.25f) ctl[0x30..0x38] = 0.25*b*V + 4*(1-0.25*b)*ctl[0x30..0x38]   ; a blend
          else            ctl[0x30..0x38] = ctl[0x18..0x20]
007dc80e  007d9c80(ctl)                                         ; body -> world refresh
007dc81d  007d80c0(ctl, step)
```

`007D8470(dyn, step, xform, invXform)` is the integrator: it folds the three world accumulators
into their body partners (§2), clamps each component by `FABS` against `dyn+0xC8h`/`+0CCh`
(`007D8783`-`007D883D`), copies `dyn+64h..6Ch` to `dyn+70h..78h` as the previous body velocity,
and steps with `step` at `007D860B`. **`coverage: partial` — only `007D8470`-`007D8624` and the
clamp block were read; `007D8624`-`007D904E` is unread.** That body is a follow-up packet.

## 5. What holds a plane up

Put §4.5 and §4.6 together. At level attitude the world-to-body matrix leaves the vertical axis
alone, so `007D8470` adds gravity into `dyn+20h` unchanged and the net body-vertical
acceleration is

```
a_up = ( clamp((1 + aoa) * qq, -2, +2) - g ) * 1.5 * 9.81      [m/s^2]
```

with `aoa = -vy/vz`, `qq = min(q,1)^2` in the sense of §4.5, `q = ForwardSpeed / (StallSpd *
1.8)` because `ctl+9Ch` is 1.0 and nothing writes it (§2), and `g = 1` for a healthy plane.

**Level flight is exactly `L == 1`.** With `q >= 1` that is `aoa == 0`, and the law is
self-correcting around it: a sinking plane has `vy < 0`, so `aoa > 0`, so `L > 1` and lift
exceeds gravity; a climbing plane has `aoa < 0` and lift falls short. The `YDrag` term in §4.8
damps `vy` directly. So a level plane at or above `1.8 x StallSpd` converges to `vy = 0` and
**holds altitude**. Below that speed `qq = q^2 < 1`, so holding altitude needs
`aoa = 1/q^2 - 1 > 0` — a steep descent through the air — and in practice the plane falls.

The three things that must be true, and where each comes from:

| requirement | field | producer |
| --- | --- | --- |
| the tick must reach the free-flight arm | `unit+900h == 7` | §6 — **this is the gap** |
| airspeed at or above `1.8 * desc[+184h] StallSpd` | `ctl+44h` (or `unit+3Ch`'s velocity) | `007D9E80` through the unit vtable `+3Ch` thunk `0074E1E0`; `007C6340` seeds it with `desc[+18Ch] TravelSpeed` |
| the plane must not be dead | `unit+0C3Ch == 0` | zero at construction (`007CFD20`) |

For the installed Shooting Star row `StallSpd = 17.5`, so the threshold is
**31.5 m/s (113.4 km/h)**, and the spawn seeds `TravelSpeed = 141.666672 m/s (510 km/h)` —
4.5x the threshold. `desc+188h MaxSpd` is 125, below `TravelSpeed`; `docs/PLANE_FLIGHT.md`
notes `+190h` is the scaled copy, and this packet did not resolve which one the drag law
actually balances against.

**No authored coefficient the host lacks is needed.** Lift and gravity need only `StallSpd`
(`desc+184h`), which `007D1F70 BSP_PlaneClass_ReadLuaFields` already parses, and two tuning
scalars with hard-coded Lua defaults. The full aerodynamic model — `007D9050`, `007D9140`,
`007D92B0`, `007DA710` — affects how fast the plane goes and how it turns, not whether it
stays up.

## 6. The `unit+900h` gate and its producer

`unit+900h` is a dword. The census is a byte scan of `.text` for `C7 ?? 00 09 00 00`
(`MOV [reg+900h], imm32`), `89 ?? 00 09 00 00` (`MOV [reg+900h], r32`) and
`8D ?? 00 09 00 00` (`LEA reg,[reg+900h]`, to catch a store through an already-offset pointer).
The `LEA` scan returns two hits, both in slab allocators outside the plane code
(`00ACBD0A`, `00B5FF67`). The plane writers are:

| site | function | value |
| --- | --- | --- |
| `007D0060` | `007CFD20 BSP_PlaneUnitInstance_Construct` | `EBX`, and `007CFD56 XOR EBX,EBX` is its only write before the store — **0** |
| `007C63F4` | `007C6340 BSP_Plane_ChooseSpawnFlightState` | **6**, the water spawn |
| `007C6481` | `007C6340 BSP_Plane_ChooseSpawnFlightState` | **7**, the free-flight spawn |
| `007C7183` | `007C7110 BSP_Plane_BeginFlying` | **7** |
| `007C145B` | `007C1430 BSP_Plane_SetFlightState` | the argument |
| `007C78F1` | `007C78A0 BSP_Plane_SetFlightStateUnguarded` | the argument |
| `007C1697`, `007C171E` | `007C1680`, `007C16F0` | 4 <-> 5 |
| `007C7488`, `007CBA12`, `007CC7E2`, `007CC857`, `007C627C`, `007C36BC`, `007CB673`, `007CB6E2`, `007D63EA`, `007D65DD`, `007D711A`, `007D6600` | taxi, launch-spot, property-bag and state-entry helpers | not read |

The values, from the dispatch table at `007C1550` (`007C1478 JMP [EAX*4 + 0x7C1550]` with
`EAX = state - 3`, guarded by `007C1467 CMP EAX,4 / JA`): 0 constructed, 3, 4 and 5 ground,
6 water, 7 free flight. `007CEC7B`/`007CEC80` gate the ground arm on 4 or 5, `007CEC99` the
surface arm on 6 (`node+5F0h` is `unit+900h`), and `0074E210`'s four bytes make the
free-flight gate true at 7 (`docs/PLANE_UNIT_TICK.md`, contract).

**`007C6340` is the seed, and the candidate is confirmed, not refuted.** Its free-flight path
(the branch predicate below uses the MSVC float-equality idiom `UCOMISS` / `LAHF` /
`TEST AH,0x44`, where `JP` is **not equal** and `JNP` is **equal**: on equality
`ZF=1, PF=0` makes the tested byte `0x40`, one set bit, odd parity, so `PF` comes out clear):

```
007c6347  if (unit[0x900] == 3)           ... unrelated path, tail-jump 007BC550
007c63c0  if (desc[0xa8] - 0.5 > unit[0xa8] && desc[0x198] MinWaterSpd == 0) {
007c63f4      unit[0x900] = 6; unit[0x904] = 0; unit[0x910] = 0
007c6412      unit[0x908] = 0; unit[0x90c] = 0
007c641a      007c11e0(unit, 1)
007c644f      unit[0xa8] = 0078cf20(x, z) + desc[0x194]
007c6467      unit->vtable[+3Ch](0.0f)
          } else {                                                    ; 007C646B
007c6481      unit[0x900] = 7                                         ; FREE FLIGHT
007c648b      unit[0x904] = 0; unit[0x910] = 0
007c6497      unit[0x908] = 3600.0f  (00CFDEB0)                       ; airborne-time seed
007c646d      unit[0x90c] = 0
007c649f      007c11e0(unit, 1)
007c64bb      unit->vtable[+3Ch](desc[0x18c] TravelSpeed)             ; THE AIRSPEED
007c64c2      unit[0xc8] = 0; unit[0x10c] = 0; walk unit+48h children through 0042ED50
          }
007c64e4  007db2c0(unit+0AB0h)                                        ; reset the controller
007c64f1  tail-jump 007BC550(unit)
```

`unit->vtable[+3Ch]` is `00D05F20 + 3Ch = 0074E1E0` (the plane vtable is written at
`007CFD78 MOV dword ptr [ESI],0xD05F20`). `0074E1E0` is a five-instruction thunk with no Ghidra
function — `no_ghidra_function`, inclusive end `0074E1F5`, final instruction
`0074E1F3 ret 4`:

```
0074e1e4  push ecx
0074e1e5  add ecx, 0xab0
0074e1eb  fstp dword ptr [esp]
0074e1ee  call 0x7d9e80
0074e1f3  ret 4
```

and `007D9E80(ctl, float speed)` is the airspeed setter:

```
007d9e86  ctl[0x3c] = 0; ctl[0x40] = 0; ctl[0x44] = speed     ; body velocity = (0,0,speed)
007d9e9b  ctl[0x48..0x50] = [00F87574..7C]                    ; angular velocity cleared
007d9ec2  007d9c80(ctl)                                       ; -> ctl[0x18..0x20] world
007d9ec7  ctl[0x30..0x38] = ctl[0x18..0x20]
007d9eda  RET 0x4
```

So one call to `007C6340` sets both the state and the speed the law needs.

`007C6340`'s own callers, by the same byte census (`local/callcensus.py 007f2920` plus one
`E8` scan for `007C6340`): its single caller is `007F2920`, and `007F2920`'s four `E8` sites are
`007F2E8F`, `007F2F74`, `007F2FC6` (inside the gap `007F2E18`-`007F2FD0`, which has no Ghidra
function) and `007F4DB0` (also unclaimed, past
`007F4580 BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes`'s body end `007F4B94`). Ghidra's
`callers 007F2920` returns **zero** rows; the byte scan is the evidence.
`producer: partial` — the squadron spawn path above `007F2920` was not read.

The narrower entry points are `007C1430 BSP_Plane_SetFlightState(unit, int state)`
(`RET 4`, returns false when the state is unchanged, writes `unit+0C04h = -1.0f` and dispatches
through the `007C1550` table) and `007C7110 BSP_Plane_BeginFlying(unit)` (state 7, `unit+0C04h
= -1.0f`, `unit+908h = 0` when the previous state was 3 or 4 and `3600.0f` otherwise, then
`007C11E0(unit, 0)`). `007C7110`'s only caller is `007CCFA0 BSP_Plane_HandleMessage`; the ledger record on
`007C7110` (packet `cc2_plane_ground_ops`, read here as a contract) says the message sub-kind
is 7. Neither setter touches the airspeed, so a host that uses one of them instead of
`007C6340` must also call `unit->vtable[+3Ch]` itself or the plane will be in free flight at
zero speed, where the lift term is zero and it falls.

## 7. The minimum host prescription, smallest first

Every step below is in `src/game_hosts_units.cpp` and `src/plane_flight.cpp`, which the
integrator owns; this packet publishes no code (§9).

| # | change | native evidence | what it makes measurable |
| --- | --- | --- | --- |
| 1 | Bind the eight `entry == 0x007CE040` dispatch rows to `bsp::run_plane_fixed_step_007ce040`, with a `PlaneFlightHost` whose arms only record. | `007CE040`; `007CEC30`-`007CECBA` | Which arm each plane takes. With `unit+900h == 0` it is provably `PlaneMotionArm::None` for all 31 aircraft and even the pose commit is skipped — the current `moved 0.00` is the correct output. This step proves the plumbing, it moves nothing. |
| 2 | Seed `unit+900h = 7` at plane creation and set the controller's body velocity to `(0, 0, desc[+18Ch] TravelSpeed)`. Both are one `007C6340` call's worth of effect: `unit[0x900] = 7`, `unit[0x908] = 3600`, `unit[0x90c] = 0`, `unit[0x904] = unit[0x910] = 0`, `ctl[0x44] = TravelSpeed`. | `007C6481`, `007C6497`, `007C64BB`; `0074E1E0`; `007D9E80` | **The arm becomes `FreeFlight`** and the pose commit runs. `007DB680` is now reached every tick with `ctl+FCh == 0`. This is the first step that can move a counter. |
| 3 | Give `free_flight_007cc2f0` a body: `007DC830` -> `007DB680` -> `007D8470`, i.e. §4.5, §4.6 and §4.7 over a `dyn` the host owns. The minimum that produces motion is §4.5 + §4.6 + the `007D8470` fold, with drag and the rate law stubbed to zero. | `007DC86E`; §4 | **A plane moves.** At `TravelSpeed` the lift term is `1.5 * 9.81` and gravity is `-1.5 * 9.81`, so it flies straight and level at 141.67 m/s. The 31 aircraft stop reporting `moved 0.00`. |
| 4 | Same, with `007D9140` drag and `007DA710` behind the controls. | §4.7, `docs/PLANE_FLIGHT.md` | The plane decelerates toward its drag-limited speed and its airspeed column becomes meaningful, so `q` and the stall threshold can be checked against 31.5 m/s. |
| 5 | A plane AI. | nothing in the ledger names one | Only after 3. |

**Step 3 is the one that first makes an aircraft reachable.** `docs/AA_VERTICAL_WINDOW.md`
measured the closest enemy aircraft at 2950 m horizontally and 4235 m in 3-D against a derived
AA range of 960 m. A plane flying at 141.67 m/s closes 2950 m in 21 seconds, so the first run
in which step 3 lands is the first run in which an AA gun can have a target in range — and the
`taken`/`shots`/`hits` columns for the 295 `AAMACHINEGUN` and 60 `FLAK` guns leave zero.
Step 2 alone does not: an arm that runs with no force term still produces no displacement.

Note that step 2 is not merely a convenience. `unit+900h` is `0` from
`007CFD20 BSP_PlaneUnitInstance_Construct` (`007CFD56 XOR EBX,EBX`, `007D0060`) and the only
producers of `7` are `007C6340` and `007C7110` — **so a host that creates plane units without
running the squadron spawn path can never reach the flight law at all**, whatever it does to
the tick. That is the single blocker named in the packet, and it is one integer.

## 8. Proven versus assumed

**Proven here, from the listing or the bytes on disk.**
- The ABI, from `007DC828 RET 0x10` and the three call sites' cleanups.
- The caller census: three `E8` sites, no `E9`, no absolute dword, over every mapped section.
- §4.1-§4.9 as written, instruction by instruction, including the x87 stack through the
  two-path square at `007DB8DD` and the nested `00419010` frames at `007DBAAA`-`007DBB23`.
- `00419010`'s contract and `0042D0D0`'s matrix convention, from their own listings.
- The gravity constant `9.81` (`00CF9058`) and every tuning and float constant quoted, read
  from the PE on disk (`local/readfloats.py`).
- The `Dynamics/*` mirror mapping `tuning_offset = mirror - 00F870E0`, cross-checked against
  `docs/GAME_TUNING_SINGLETON.md`'s key table for every address this law reads.
- `ctl+90h`, `ctl+98h`, `ctl+9Ch`: the complete direct-store writer set over
  `007B0000`-`007F8390`, and that `007DC830` writes only `ctl+4h`, `ctl+5h`, `ctl+FCh` and
  `ctl+24h..2Ch` (a full store census of its 302-instruction listing).
- `unit+900h`'s writer census and its value at construction.
- `desc+170h`/`+174h` = `YDrag`/`XDrag` and `desc+198h` = `MinWaterSpd`, from the key strings
  pushed immediately before each field's `FSTP` in `007D1F70`.
- The `UCOMISS` / `LAHF` / `TEST AH,0x44` branch sense: `JP` is taken on **not equal** (the
  tested byte is `0x00`, even parity), `JNP` on **equal** (`0x40`, odd parity). Both branches
  that turn on it here (`007DB6F7`, `007C63EC`) were decided this way, and the second one
  disproves a merged record (below).

**Assumed or partial.**
- `coverage: partial` for `007DB680`: the water branch `007DC426`-`007DC64A` was not read.
- `coverage: partial` for `007D8470`: `007D8624`-`007D904E` unread. The claim that it
  integrates is from `007D860B FLD [ESP+0x50]` (the `step` argument) and the
  `dyn+64h -> dyn+70h` previous-velocity copy, not from the integration itself.
- `coverage: partial` for `007D9140`: argument order past the first is taken from the call site.
- `007D80C0`, `007DB2C0`, `007C11E0`, `0085DEA0`, `007BC550`, `0042ED50`, `008E6430` were not
  read; they are named by address in the tables above. `007BBC50` was read only to `007BBCBC`.
- `ctl+94h` gates two extra terms (§4.6) and its producer was not found.
- `unit+0CC8h` (a thrust scale) and `unit+0C64h`'s producer were not read; `unit+0C64h` is
  called the pitch because `007D9050` feeds it to `AccelCheatFallPitchRange` in radians and
  negates it for the dive case, not because a producer was read.
- The class values (StallSpd 17.5, TravelSpeed 141.666672, Accel 5) are quoted from
  `docs/PLANE_FLIGHT.md`'s installed row, not re-read from `vehicleclasses.lua`.
- No run-time evidence: `bsp_game.exe` cannot reach `007DB680`'s equivalent today, because the
  host has no plane tick bound at all. Checklist item 6 is satisfied vacuously; the first run
  that can test any claim here is step 3 of §7.

**Corrections to merged documents.**
- **Was:** `docs/PLANE_FLIGHT.md` line 130, `007DB680(ctl, step, a, b, flag)`.
  **Is:** the fifth parameter is a `float*`. **Evidence:** `007DD43F PUSH EDX` with
  `EDX = ESP+0x34` at the water call site, and `007DC3DC MOV ECX,[ESP+0x68]` /
  `007DC422 FLD float ptr [ECX]` in the callee.
- **Was:** the ledger record on `007C6340` (packet `cc2_plane_ground_ops`): "classDesc+A8h - 0.5
  <= unit+A8h or classDesc+198h MinWaterSpd != 0 selects state 7 with unit+908h = 3600.0f
  (`007C63F4`), and anything else selects state 6 with unit+908h = 0 (`007C6481`)".
  **Is:** the predicate is right and the two sites are swapped. `007C63F4` is
  `c7 86 00 09 00 00 **06** 00 00 00` — state **6**, with `unit+908h = 0` at `007C6412`;
  `007C6481` is `c7 86 00 09 00 00 **07** 00 00 00` — state **7**, with
  `unit+908h = 3600.0f` (`00CFDEB0`) at `007C6497`. **Evidence:** the immediates in the
  instruction bytes. This matters: `007C6481` is the only site in the spawn chooser that
  produces the value the free-flight gate needs, and it is the fall-through, not the taken
  branch. `desc+198h` is confirmed as `MinWaterSpd` — `007D2440 PUSH 0xD06458` precedes
  `007D2463 FSTP [ESI+0x198]` and `00D06458` holds `"MinWaterSpd"`.
- **Was:** `docs/PLANE_FLIGHT.md`'s descriptor table attributes `+1CCh YawLimitAngle`,
  `+1D0h PitchLimitAngle`, `+1D4h DragPitchRatio` and `+1DCh AirBrakeDrag` to the
  "`007DB680` band".
  **Is:** none of those four offsets is read anywhere in `007DB680`'s body. A complete census of
  every `[reg + 0x1xx]`, `[reg + 0x2xx]` and `[reg + 0x5xx]` access in its 1214-instruction
  listing yields exactly `desc+170h`, `+174h`, `+184h`, `+198h`, `+1A0h`, `+1E0h`, `+1FCh`,
  `+508h` (and `unit+100h`). `+1D4h` and `+1DCh` are read by `007D9140` (`007D91BD`,
  `007D91E0`), which `007DB680` calls; `+1CCh` is read by `007C466D` and `007C49E9`, which it
  does not. The writer of all four is `007D1F70`.

## 9. No code published

`include/bsp/plane_flight.hpp` already carries `plane_flight_controller_off`,
`plane_class_flight_off`, `plane_dynamics_tuning_off`, `PlaneFlightHost` and
`run_plane_fixed_step_007ce040`. `007DB680` is the body behind that host's
`free_flight_007cc2f0` arm, so the lift and gravity rules of §4.5 and §4.6 belong in
`src/plane_flight.cpp` beside them, not in a parallel `plane_flight_core_law` module. This
packet may not edit that file, so it publishes **no header and no source**, adds no line to
`cmake/startup.cmake`, and adds no test. The integrator has everything needed in §4 and §7.

`python tools/bsp.py find plane --limit 30` and `rg -l` over `include/bsp` were run before this
decision; no `plane_flight_core_law` name exists and none was created.

## 10. Follow-up packets

1. **`plane_dynamics_integrator`** — `007D8470` `007D8624`-`007D904E`. The clamp set, the
   per-component loop and the pose write. Everything in §4 lands in `dyn`; only this turns it
   into a position. Owner must also take `007D80C0`.
2. **`plane_water_ditching_law`** — `007DB680` `007DC426`-`007DC64A` plus `007DCDD0`'s
   `007DCE04`-`007DD40B`. The only unread arm of the core law.
3. **`plane_drag_and_thrust_curves`** — `007D9140` in full, `007D9050`'s `desc+604h` and
   `desc+208h`/`+50Ch` keys from `007D1F70`, and the `MaxSpd`/`+190h` question in §5.
4. **`plane_spawn_path`** — `007F2920`, the unclaimed regions `007F2E18`-`007F2FD0` and
   `007F4B94`-onward, and `007C11E0`. This is what a host would imitate to spawn a squadron
   the way the game does, rather than setting `unit+900h` by hand.
5. **`plane_control_state_machine`** — the twelve unread `unit+900h` writers in §6 and the
   `007C1550` dispatch table's five handlers. Needed before a plane can take off or land.
