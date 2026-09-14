# From a filled plan slot to the plane's live controls (packet `cc7-recon-slot`)

Addresses: `0099B450`, `0099BEE0`, `0099BC00`, `0099BB40`, `0099BF30`, `007B8C90`, `007BB920`,
`007BB6E0`, `007B9770`, and the call site `0099B05A`-`0099B0B9` inside `0099ACD0`.
Constants: `00CE3D34`, `00D7A24C`, `00D7A250`, `00D7A260`, `00CFD408`, `00D05998`, `00CE3930`,
`00E0E2EC`, `00D1F39C`. Helper `00BF7420`.

Worker `cc7-recon-slot`, 2026-09-14 UTC. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Ghidra was **read-only**: no rename, no comment, no prototype, no
tag, no write lock, no save, no lease, no git, and no file in this repository was edited — this
document is the only thing written. Every descriptive name below is a hypothesis, not a
recovered symbol.

This is the deterministic half of the chain in `docs/PLANE_BOT_CONTROL_WRITEBACK.md`: everything
between a filled plan slot and `unit+9E4h`. The planner `0099D300` is not covered, except for one
bounded scan of its listing reported under [The `active` byte](#the-active-byte-and-who-writes-current).

## Headline

Three things that a reimplementation gets wrong if it works from the existing docs alone:

1. **`0099BEE0`'s output is not the command.** `0099BF30 BSP_PilotBot_RepairCommandBands` runs
   between the slew and `007B8C90` and **rewrites all five command floats** — `0099C00C`,
   `0099C0C8`, `0099C14C`, `0099C17A`, `0099C1A6` (plus `0099C1B7`, `0099C1E7`, `0099C1F8`), every
   one a `MOVSS dword ptr [EDI+n],XMM0` with `EDI` the command buffer. `PLANE_BOT_CONTROL_WRITEBACK.md`
   lists it as "band repair on the buffer (body not read)"; what was not established there is that
   it is the **last writer of every axis**, so the slew result reaches the plane only where the
   repair happens to pass it through.
2. **The clamps after the slew are not symmetric.** Throttle and air brake clamp to `[0, 1]`;
   yaw, roll and pitch clamp to `[-1, +1]`. Two different code shapes in the same unrolled loop
   (`FLDZ` at `0099BC2C`/`0099BD6A` versus `FLD [00D7A260]` at `0099BC72`/`0099BCC4`/`0099BD17`).
3. **The `dt` that reaches the slew is the accumulated think interval, not the frame delta.**
   `0099ACD0` writes the accumulator back over its own `dt` argument slot at `0099AD16`, and
   `0099B05A FLD [ESP+0x2C]` reads that same slot. A host that passes per-frame `dt` slews the
   bot's stick roughly five times too slowly.

## `0099B450 BSP_PilotBot_SeedPlanSlots` — confirmed, and larger than documented

`void __thiscall(plan)`, `RET 0` (`0099B588 RET`, no immediate). Body `0099B450`-`0099B588`,
47 instructions, no calls, no branches — straight-line. `EAX = [ECX+2F0h]` is the plane unit
(`0099B450`).

The brief's claim is **confirmed**: it writes `desired` and `current` from the *same* live value
and clears the `active` byte, on all five slots. Read straight from the listing:

| slot | live source | -> `desired` | -> `current` | `active` = 0 | axis |
| --- | --- | --- | --- | --- | --- |
| 0 | `unit+9F0h` (`0099B456`) | `plan+278h` (`0099B45E`) | `plan+274h` (`0099B466`) | `plan+27Ch` (`0099B470`) | throttle |
| 1 | `unit+9E4h` (`0099B476`) | `plan+284h` (`0099B47E`) | `plan+280h` (`0099B486`) | `plan+288h` (`0099B48E`) | **yaw** |
| 2 | `unit+9ECh` (`0099B494`) | `plan+290h` (`0099B49C`) | `plan+28Ch` (`0099B4A4`) | `plan+294h` (`0099B4AC`) | **roll** |
| 3 | `unit+9E8h` (`0099B4B2`) | `plan+29Ch` (`0099B4BA`) | `plan+298h` (`0099B4C2`) | `plan+2A0h` (`0099B4CA`) | pitch |
| 4 | `unit+9F4h` (`0099B4D0`) | `plan+2A8h` (`0099B4D8`) | `plan+2A4h` (`0099B4E0`) | `plan+2ACh` (`0099B4EB`) | air brake |

Stride `0Ch`, base `plan+274h`, field order `{ current: float, desired: float, active: byte }`
with three bytes of tail padding. `active` is written as a **byte** (`MOV byte ptr [...],DL` with
`DL = 0` from `0099B46E XOR EDX,EDX`), and the planner sets it as a byte too
(`0099D362 MOV byte ptr [ESI+288h],1`).

### The rest of the function, which no existing doc lists

`0099B450` is a **full reset of the plan object**, not just of the slot array. Beyond the table:

| address | store | value |
| --- | --- | --- |
| `0099B4F7` | `plan+26Ch` byte | `0` |
| `0099B4FD` | `plan+270h` dword | `0` |
| `0099B509` | `plan+2C4h` float | `0.0f` (`XORPS XMM0,XMM0` at `0099B4E8`) |
| `0099B511` | `plan+2B4h` float | `[[plan+2F4h]+190h]` (vehicle class descriptor, via `0099B4F1`) |
| `0099B517` | `plan+2BCh` float | `0.0f` |
| `0099B52C` | `plan+2E8h` float | `1.0f` (`00D7A24C`) |
| `0099B53C` | `plan+2B0h` byte | `0` |
| `0099B542` | `plan+2D8h` dword | `1` |
| `0099B548` | `plan+2CCh` dword | `1` |
| `0099B54E` | `plan+2D0h` dword | `2` |
| `0099B558` | `plan+2D4h` dword | `1` |
| `0099B55E` | `plan+2C8h` float | `20.0f` (`00CE3930`) |
| `0099B566` | `plan+2DCh` byte | `0` |
| `0099B56C` | `plan+2E4h` byte | `0` |
| `0099B572` | `plan+2E5h` byte | `0` |
| `0099B580` | `plan+2ECh` float | `0.24f` (`00E0E2EC`, `8F C2 75 3E`) |

The last three byte stores matter: **`0099B450` also clears the three request bytes** that
`0099BEE0` later copies into the command buffer. They are therefore zero on any tick where the
planner does not set them, which is what makes the boost gate in `007BB6E0` (below) default off.

`plan+2C4h`/`plan+2BCh` are float-zeroed, `plan+26Ch` is a byte and `plan+270h` a dword — the
writeback doc's "clears `plan+26Ch` and `plan+270h`, zeroes `plan+2C4h`/`plan+2BCh`" is right but
does not give the widths.

## `0099BEE0 BSP_PilotBot_EvaluatePlanSlots` — ABI and the request bytes

`void __thiscall(plan, void* cmdBuf, float rate, float dt)`, `RET 0Ch`, body `0099BEE0`-`0099BF23`.
Argument resolution, entry-relative (`E` = ESP at entry, return address at `[E]`):

| arg | slot | how |
| --- | --- | --- |
| `cmdBuf` | `[E+4]` | `0099BEE6 MOV EDI,[ESP+0xC]` with `ESP = E-8` after two pushes |
| `rate` | `[E+8]` | `0099BEF3 FLD [ESP+0x18]` with `ESP = E-0x14` |
| `dt` | `[E+0Ch]` | `0099BEE0 FLD [ESP+0xC]` before any push |

It forwards `(ECX = plan+274h, cmdBuf, rate, dt)` to `0099BC00` (`0099BEF7 LEA ECX,[ESI+274h]`,
`0099BF00 PUSH EDI`, `0099BF01 CALL`), then copies three bytes:

```
0099bf06  plan+2DCh -> cmdBuf+16h
0099bf0f  plan+2E5h -> cmdBuf+15h
0099bf18  plan+2E4h -> cmdBuf+14h
```

`cmdBuf` is 24 bytes: five floats at `+0h`..`+10h` and one dword at `+14h` carrying the three
request bytes plus a pad byte. `007B8C90` copies exactly those six dwords.

### The call site, `0099B05E`-`0099B0B9` in `0099ACD0`

With `S` = ESP before `0099B066 SUB ESP,8`, the buffer is the stack local at `S+10h`
(`0099B075 LEA ECX,[ESP+0x18]`, `0099B07F PUSH ECX`), and `EDI = EBX+4 = plan` (`0099B07C`):

```
0099b080..0099b096   all six cmdBuf dwords := 0
0099b09a             cmdBuf+0Ch := 1.0f            ; 00D7A24C — dead, the slew overwrites it
0099b06f/0099b079    rate := 4.0f                  ; 00CE3D34 = 40 80 00 00
0099b05a             dt   := [ESP+0x2C] = entry+4  ; the accumulated think interval
0099b0a0             CALL 0099BEE0
0099b0a5..0099b0ac   CALL 0099BF30(plan, cmdBuf)   ; rewrites all five floats
0099b0b1..0099b0b9   CALL 007B8C90(unit, cmdBuf)
```

**`dt` is the accumulated think interval.** `0099ACD0` does
`bot+70h += dt` and stores the sum **back into its own argument slot** at
`0099AD16 FSTP [ESP+0x24]` (`ESP = entry-20h`, so `[ESP+0x24] = entry+4`, the `dt` argument, as
`0099AD65 RET 4` confirms). The gate at `0099AD27 FCOMIP` against `00D1F39C = 0.09f` passes only
when the accumulator reaches `0.09 s`, and `0099AD75 MOVSS [ESI+0x70],XMM0` then resets
`bot+70h` to `0`. At `0099B05A`, `ESP = entry-28h` (prologue `SUB ESP,0x1C` + `PUSH ESI`
`0099ACD3` + `PUSH EDI` `0099AE3A` + `PUSH EBX` `0099AE7B`; those are the only register pushes in
the function), so `[ESP+0x2C] = entry+4` — the accumulated value, `>= 0.09`.

Because every frame's `dt` is accumulated and none is dropped, `sum(rate * accumulated) over
think ticks == rate * elapsed`. The effective slew rate is therefore exactly **4.0 units of stick
per second of game time**, as `PLANE_BOT_CONTROL_WRITEBACK.md` states — but only if the host
passes the accumulated interval. Passing a 60 Hz frame `dt` gives `4.0 * 0.0167 = 0.067` per
think tick at ~11 Hz, i.e. about `0.74` units/second.

## The slew rule

Two functions. `0099BC00` is the unrolled five-slot walk and the clamp; `0099BB40` is the
per-axis step.

### `0099BB40` — per-axis step, `float __thiscall(slot, float a, float b)`, `RET 8`

Body `0099BB40`-`0099BBF6`, 54 instructions. Reads `[ECX]` and `[ECX+4]` only; **writes nothing
to the slot** and **never reads `[ECX+8]` (`active`)**. The result is returned in `ST0`.

Every ambiguous x87 form was settled from bytes, not from Ghidra's text:

| address | Ghidra text | bytes | meaning |
| --- | --- | --- | --- |
| `0099BB52` / `0099BB58` | `FLD ST0` | `D9 C0` | push a copy of ST0 |
| `0099BB5A` | `FSUBP ST2,ST0` | `DE EA` | `ST2 := ST2 - ST0`, pop |
| `0099BB89` | `FCOMI ST0,ST1` | `DB F1` | compare, no pop |
| `0099BB8B` / `0099BB8F` | `FSTP ST1` | `DD D9` | `ST1 := ST0`, pop |
| `0099BB91` | `FSTP ST0` | `DD D8` | pop |
| `0099BBA4` | `FSTP ST2` | `DD DA` | `ST2 := ST0`, pop |
| `0099BBBC` / `0099BBE7` | `FMULP ST2` | `DE CA` | `ST2 := ST2 * ST0`, pop |
| `0099BBBE` / `0099BBE9` | `FADDP` | `DE C1` | `ST1 := ST1 + ST0`, pop |

Register-stack trace (entry `ESP = E`, `a` at `[E+4]`, `b` at `[E+8]`, `SUB ESP,8` makes
`[ESP] = E-8` and `[ESP+4] = E-4`):

```
0099bb43  local0 := slot->desired                      ; [ECX+4]
0099bb49  local1 := slot->current                      ; [ECX+0]
0099bb4f-0099bb5e  local0 := desired - current         ; DE EA, then FSTP [ESP]
0099bb61-0099bb69  [E+8]  := a * b                     ; step, over-written into the arg slot
0099bb6d-0099bb7d  [E+4]  := |desired - current|       ; AND EAX,7FFFFFFFh
0099bb81-0099bb8d  FCOMI step vs |delta| ; JBE when step <= |delta|
```

so, with `delta = desired - current` and `step = a * b`:

```c
// 0099BB40, RET 8.  Returns the new value; does NOT store it back into the slot.
float slew(const PlanSlot* s, float a, float b) {
    float delta = s->desired - s->current;
    float step  = a * b;
    if (step > fabsf(delta))                 // 0099BB89 FCOMI / 0099BB8D JBE, fall-through
        return s->desired;                   // 0099BB8F-0099BB9E: snap
    if (0.0f > delta)                        // 0099BBAB COMISS 0.0, delta / 0099BBAE JBE
        return s->current - step;            // 0099BBB0-0099BBCB, sign = -1
    if (delta > 0.0f)                        // 0099BBCE COMISS delta, 0.0 / 0099BBD9 JA
        return s->current + step;            // sign = +1
    return s->current;                       // 0099BBDB sign = 0 (delta == 0 or unordered)
}
```

The sign is materialised as an integer `-1`/`+1`/`0` in a stack slot and brought in with `FILD`
(`0099BBB8`, `0099BBE3`), then `FMULP`/`FADDP` — `current + sign(delta) * step`. `sign(0) == 0`,
so an axis whose `desired` equals its `current` returns `current` bit-exactly.

The comparison is `step > |delta|`, strictly: `FCOMI` sets `ZF` on equality and `JBE` is taken,
so `step == |delta|` takes the *step* path, not the snap path. Numerically identical either way.

### `0099BC00` — the five-slot walk and the clamp, `RET 0Ch`

`void __thiscall(slotArray, void* cmdBuf, float a, float b)`, body `0099BC00`-`0099BD9E`,
117 instructions, fully unrolled, no loop. `EDI = slotArray` (`0099BC0E`),
`ESI = cmdBuf` (`0099BC4B MOV ESI,[ESP+0x10]`). Each result is clamped and stored to the command
buffer while the *next* call is being set up, which is why the store addresses trail the calls.

| slot | `LEA ECX` | call | clamp | result store | cmd index | live field |
| --- | --- | --- | --- | --- | --- | --- |
| 0 `plan+274h` | `EDI+0` | `0099BC17` | `[0.0, 1.0]` (`0099BC2C FLDZ`) | `0099BC5D [ESI+0Ch]` | `cmd[3]` | `unit+9F0h` throttle |
| 1 `plan+280h` | `0099BC56` | `0099BC65` | `[-1.0, 1.0]` (`0099BC72`) | `0099BCB0 [ESI]` | `cmd[0]` | `unit+9E4h` **yaw** |
| 2 `plan+28Ch` | `0099BCA9` | `0099BCB7` | `[-1.0, 1.0]` (`0099BCC4`) | `0099BD02 [ESI+8]` | `cmd[2]` | `unit+9ECh` **roll** |
| 3 `plan+298h` | `0099BCFB` | `0099BD0A` | `[-1.0, 1.0]` (`0099BD17`) | `0099BD55 [ESI+4]` | `cmd[1]` | `unit+9E8h` pitch |
| 4 `plan+2A4h` | `0099BD4E` | `0099BD5D` | `[0.0, 1.0]` (`0099BD6A FLDZ`) | `0099BD76`/`0099BD97 [ESI+10h]` | `cmd[4]` | `unit+9F4h` air brake |

`00D7A260 = 00 00 80 BF = -1.0f`; `00D7A24C = 00 00 80 3F = 1.0f`.

Both clamp shapes are `low` first then `high`:

```
FLD low ; FCOMIP ST0,ST1 (DF F1) ; JBE keep    -> not taken means low > v, so v := low
COMISS v, 1.0f ; JBE keep                      -> not taken means v > 1.0f, so v := 1.0f
```

Complete, the slew stage is:

```c
// 0099BEE0 -> 0099BC00 -> 0099BB40 x5.  slots[] is plan+274h, stride 0Ch.
// rate = 4.0f (00CE3D34); dt = the accumulated think interval, >= 0.09s.
void evaluate_plan_slots(const PlanSlot slots[5], float cmd[6], float rate, float dt) {
    cmd[3] = clampf(slew(&slots[0], rate, dt),  0.0f, 1.0f);   // throttle
    cmd[0] = clampf(slew(&slots[1], rate, dt), -1.0f, 1.0f);   // yaw
    cmd[2] = clampf(slew(&slots[2], rate, dt), -1.0f, 1.0f);   // roll
    cmd[1] = clampf(slew(&slots[3], rate, dt), -1.0f, 1.0f);   // pitch
    cmd[4] = clampf(slew(&slots[4], rate, dt),  0.0f, 1.0f);   // air brake
    ((uint8_t*)cmd)[0x16] = plan->b2DC;                        // 0099BF06
    ((uint8_t*)cmd)[0x15] = plan->b2E5;                        // 0099BF0F
    ((uint8_t*)cmd)[0x14] = plan->b2E4;                        // 0099BF18
}
```

`rate` and `dt` are only ever used as the product `a * b` at `0099BB65`, so nothing in the slew
distinguishes them; the naming comes from the call site (`4.0f` constant versus the interval
read from the argument slot).

### The `active` byte, and who writes `current`

`active` is **not read anywhere in the slew path** — not in `0099BC00`, not in `0099BB40`. It is
consumed only by the planner, as a "this axis has a demand this tick" flag: `0099D7CB
CMP byte ptr [ESI+27Ch],0` then either `0099D7D6 [ESI+278h]` (`desired`) or `0099D7E0
[ESI+274h]` (`current`). That select is what makes `0099B450`'s seeding of `desired` correct
rather than redundant: an axis the planner leaves alone keeps `desired == current`, `delta == 0`,
and the slew returns the live value unchanged.

Scanning the whole `0099D300` listing (1451 instructions) for register-displacement stores:
**0 writes to any `current` field** (`+274h`, `+280h`, `+28Ch`, `+298h`, `+2A4h`) against **18
writes to the `desired` fields** (`+278h`, `+284h`, `+290h`, `+29Ch`, `+2A8h`) found by the same
pattern — the positive control that makes the negative meaningful. So within that function,
`0099B450` is the only writer of `current`, and the slew is stateless across ticks: the "current"
it starts from is always the plane's live control block as of the previous think.

## `007B8C90 BSP_Plane_SetPilotCommandBlock` — confirmed exactly

`void __thiscall(unit, const void* cmd)`, `RET 4`, body `007B8C90`-`007B8CD0`, 15 instructions,
straight-line, no calls. The count is **six dwords** and the order is a straight copy:

| cmd | store | unit | axis |
| --- | --- | --- | --- |
| `+0h` | `007B8C96` | `+9FCh` | yaw |
| `+4h` | `007B8C9F` | `+A00h` | pitch |
| `+8h` | `007B8CA8` | `+A04h` | roll |
| `+0Ch` | `007B8CB1` | `+A08h` | throttle |
| `+10h` | `007B8CBA` | `+A0Ch` | air brake |
| `+14h` | `007B8CC3` | `+A10h` | the three request bytes + pad |

then `007B8CC9 MOV byte ptr [ECX+0xA14],1`. `unit+A14h` is a **byte**, written `1` here, tested
at `007BB929` and cleared at `007BB990`. Three callers: `0099B0B9` (the bot), `00519818`
(`BSP_PlanePilotView_BuildPlayerCommand`, the human stick) and `0060C22B`.

## `007BB920 BSP_Plane_CommitPilotCommand` — the gate, and two overrides

`void __thiscall(unit)`, `RET` (no immediate, `007BB998`), body `007BB920`-`007BB998`,
35 instructions. One call site, `007CE865`.

```
007bb923  if (unit+61h  != 0) return;                      // JNZ 007bb997
007bb929  if (unit+A14h == 0) return;                      // JZ  007bb997 — nothing pending
007bb932  if (unit+900h not in {7,6,4,5}) unit+A0Ch = 1.0f;  // = cmd[4], AIR BRAKE, full
007bb95c  if (unit->vtable[5Ch](0x17) && unit+C24h == 0)
              unit+A08h = 1.0f;                            // = cmd[3], THROTTLE, full
007bb982  007BB6E0(unit, &unit+9FCh);
007bb990  unit+A14h = 0;
```

The two override targets are named here for the first time: by `007B8C90`'s map above,
`unit+A0Ch` is `cmd[4]` (**air brake**) and `unit+A08h` is `cmd[3]` (**throttle**), not the other
way round. `PLANE_BOT_CONTROL_WRITEBACK.md` gives these two lines as raw offsets and does not
name them, so this is an addition, not a contradiction.

## `007BB6E0 BSP_Plane_QuantizeControlAxes` — the quantisation rule

`void __thiscall(unit, const float* cmd)`, `RET 4`, body `007BB6E0`-`007BB91C`, 166 instructions.
`EDI = cmd` (`007BB6E2 MOV EDI,[ESP+0xC]` after two pushes), `ESI = unit`. Exactly one caller,
`007BB98B`.

Constants, read as bytes: `00CFD408 = 00 00 00 00 00 C0 5F 40` = **127.0** (double),
`00D05998 = 00 00 00 00 00 10 60 40` = **128.5** (double), `00D7A250 = BF F0 00 00 00 00 00 00`
= **-1.0** (double). `00BF7420` is `_ftol2_sse`: `007BB6FC`-`007BB743` shows
`FSTP qword [ESP]` then `CVTTSD2SI EAX,[ESP]` — **truncation toward zero**, returning `int` in
`EAX` and popping `ST0`.

### The rule

```c
// 007BB6E0, one axis.  c is the command float.
int n = (int)(c * 127.0 + 128.5);       // 00BF7420, truncate toward zero
if      (n >= 0xFF) out =  1.0f;        // CMP EAX,0xFF ; JL  (signed)
else if (n <= 1)    out = -1.0f;        // CMP EAX,1    ; JG  (signed)
else                out = (float)(n - 128) / 127.0;   // ADD EAX,-0x80 ; FILD ; FDIV
```

The two clamps are continuous with the formula, not offsets to it: `(1-128)/127 == -1.0` and
`(255-128)/127 == +1.0` exactly. For `c` in `[-1, 1]` the argument to `ftol` lies in
`[1.5, 255.5]`, always positive, so truncation is `floor` and

```
out = round_half_up(c * 127.0) / 127.0
```

— a symmetric 255-level quantisation with `c = 0` landing on exactly `0.0f`
(`floor(128.5) = 128`, `(128-128)/127 = 0`). **The scale is 127, not 128**, and the bias is
`128`, not `127.5`. The quantiser is signed on all five axes, including the two the bot's slew
had already clamped to `[0, 1]`.

`PLANE_BOT_CONTROL_WRITEBACK.md` states this rule correctly; this is an independent confirmation
with the constants read as bytes and the `ftol` helper identified.

### Axis map, and the Ghidra `FMUL ST1` trap

| cmd | source read | multiply | result store | unit | axis |
| --- | --- | --- | --- | --- | --- |
| `cmd[0]` | `007BB6E6 [EDI]` | `007BB6F0` `DC C9` = `ST1 *= ST0` | `007BB72A FSTP` | `+9E4h` | **yaw** |
| `cmd[1]` | `007BB730 [EDI+4]` | `007BB733` `D8 CB` = `ST0 *= ST3` | `007BB75D FSTP` | `+9E8h` | pitch |
| `cmd[2]` | `007BB763 [EDI+8]` | `007BB766` `D8 CB` | `007BB798 FSTP` | `+9ECh` | **roll** |
| `cmd[3]` | `007BB83A [EDI+0Ch]` | `007BB843` `DC C9` = `ST1 *= ST0` | `007BB877 FSTP` | `+9F0h` | throttle |
| `cmd[4]` | `007BB87D [EDI+10h]` | `007BB880` `D8 C9` = `ST0 *= ST1` | `007BB8B7 FSTP` | `+9F4h` | air brake |

Ghidra prints **`FMUL ST1` at both `007BB843` (`DC C9`) and `007BB880` (`D8 C9`)**, and they mean
opposite things — `ST1 *= ST0` versus `ST0 *= ST1`. Reading either from the text alone puts the
product in the wrong register and silently changes which value reaches `ftol`. Every multiply,
divide, exchange and pop in this function was settled from bytes:
`007BB728`/`007BB875` `D8 F3`/`D8 F1` = `FDIV ST0,ST(i)`;
`007BB792`/`007BB8B0` `DA 7C ..` = `FIDIVR m32int` (`ST0 := m32 / ST0`, with `ST0 = 127.0` — the
same quotient by a different schedule); `007BB6F8` `DC C2` = `FADD ST2,ST0`;
`007BB735` `D8 C4` = `FADD ST0,ST4`; `007BB768` `DE C4` = `FADDP ST4,ST0`;
`007BB6FA`/`007BB845`/`007BB76A` `D9 CA`/`D9 C9`/`D9 CB` = `FXCH ST(i)`;
`007BB710`/`007BB743` `D9 C1` = `FLD ST1`; `007BB719`/`007BB74C` `D9 C0` = `FLD ST0`;
`007BB778`/`007BB77A`/`007BB78C` `DD D9` = `FSTP ST1`;
`007BB781`/`007BB894`/`007BB89F` `DD D8` = `FSTP ST0`.

The function keeps `-1.0`, `1.0`, `127.0` and `128.5` live in `ST0`-`ST3` across axes 0-2 and
reloads `127.0` at `007BB81E`/`007BB83D` for axes 3-4; all three paths of every axis leave the
stack balanced.

### Throttle is not always quantised

`cmd+14h`, the first request byte, gates the throttle axis entirely:

```
007bb796  bool p = unit->vtable[5Ch](0x17);
007bb7ad  if (p && unit+C24h == 0) {
007bb7b9      if (cmd+14h != 0 && unit+E84h > 0.0f) unit+9F8h = 1;
007bb7d4      else if (!(0.0f < unit+E84h))          unit+9F8h = 0;
007bb7e3      unit+9F0h = (unit+9F8h != 0) ? 1.0f : 0.0f;   // cmd[3] IGNORED
          } else if (cmd+14h != 0) {
007bb7f2      unit+9F8h = (unit+E84h > 0.0f && unit+E88h == 0) ? 1 : 0;
007bb816      unit+9F0h = 1.0f;                             // cmd[3] IGNORED
          } else {
007bb82e      unit+9F8h = 0; unit+E88h = 0;
007bb83a      unit+9F0h = quantize(cmd[3]);                 // the only quantised path
          }
```

So `unit+9F0h` receives the quantised `cmd[3]` **only when `cmd+14h` is zero and the vtable
predicate path does not fire**. For the bot this is the normal case, because `0099B450` clears
`plan+2E4h` at `0099B56C` every think. `unit+E84h` and `unit+E88h` are unidentified;
`unit+9F8h` looks like a boost/WEP latch, and `007B9770` copies it to `unit+BC8h`.

### The three request bytes all have readers

`include/bsp/plane_ai_control.hpp` records `kRequestByte15` and `kRequestByte16` as having "no
reader". They do:

| plan | cmd | unit (cmd block) | read at | live byte | latched (`007B9770`) |
| --- | --- | --- | --- | --- | --- |
| `+2E4h` | `+14h` | `+A10h` | `007BB7B9`, `007BB7ED` | gates `unit+9F8h` | `+BC8h` |
| `+2E5h` | `+15h` | `+A11h` | `007BB8D0` | `unit+9F9h` (`007BB8D6`) | `+BCAh` |
| `+2DCh` | `+16h` | `+A12h` | `007BB8BF` | `unit+9FAh` (`007BB8C2`) | `+BC9h` |

`unit+9FAh` is additionally gated: `007BB8B4 CMP byte ptr [ESI+5Dh],BL` / `007BB8BD JNZ` — when
`unit+5Dh != 0` the byte is forced to `0` (`007BB8CA`) instead of taken from `cmd+16h`.

### A dead tail

`007BB8DD JZ 007BB91A` on `unit+61h`: when `unit+61h != 0`, all five live axes are **overwritten
from another block** — `unit+9E8h := unit+8ACh`, `unit+9F4h := 0.0f`, `unit+9F0h := unit+8DCh`,
`unit+9ECh := unit+8C4h`, `unit+9E4h := unit+894h` (`007BB8DF`-`007BB914`; note the sources are
`0x18` apart and in axis order yaw/pitch/roll/throttle from `unit+894h`).

This branch is **unreachable from the only call site**: `007BB6E0` has exactly one caller,
`007BB98B` inside `007BB920`, and `007BB920` returns at `007BB927` whenever `unit+61h != 0`. A
host may omit it; a host that implements it will never execute it.

## Axis order, stated with evidence

The command-buffer index to live-field mapping, cross-checked from both ends:

| cmd index | slot that feeds it (`0099BC00`) | seeded from (`0099B450`) | quantised into (`007BB6E0`) | axis |
| --- | --- | --- | --- | --- |
| `cmd[0]` `+0h` | slot 1, `plan+280h` | `unit+9E4h` | `unit+9E4h` | **yaw** |
| `cmd[1]` `+4h` | slot 3, `plan+298h` | `unit+9E8h` | `unit+9E8h` | pitch |
| `cmd[2]` `+8h` | slot 2, `plan+28Ch` | `unit+9ECh` | `unit+9ECh` | **roll** |
| `cmd[3]` `+0Ch` | slot 0, `plan+274h` | `unit+9F0h` | `unit+9F0h` | throttle |
| `cmd[4]` `+10h` | slot 4, `plan+2A4h` | `unit+9F4h` | `unit+9F4h` | air brake |

The seed's *source* field and the quantiser's *destination* field agree on all five rows. That is
an independent consistency check on the whole permutation: the loop closes only if the mapping is
exactly this one. It agrees with `include/bsp/plane_flight.hpp`'s corrected axis map
(`kLiveYaw = 0x9E4`, `kLivePitch = 0x9E8`, `kLiveRoll = 0x9EC`, `kLiveThrottle = 0x9F0`,
`kLiveAirBrake = 0x9F4`) and with `PlanSlotIndex` in `include/bsp/plane_ai_control.hpp`.

### It does **not** agree with `plane_pilot_command_off` in `include/bsp/plane_flight.hpp`

That namespace (around lines 148-160) still names the slots by the **old, swapped** axis map,
and contradicts its own evidence comments on the same lines:

```
inline constexpr int kRollCurrent = 0x280;      // 0099B486 from unit+9E4h   <- +9E4h is YAW
inline constexpr int kYawCurrent  = 0x28C;      // 0099B4A4 from unit+9ECh   <- +9ECh is ROLL
```

`+280h` is the **yaw** slot and `+28Ch` is the **roll** slot. `kRollDesired`/`kRollFlag` and
`kYawDesired`/`kYawFlag` are swapped the same way, and `kAuxCurrent`/`kAuxDesired`/`kAuxFlag` at
`+2A4h`/`+2A8h`/`+2ACh` are the **air brake**, not an unnamed auxiliary. The same file's
`plane_control_off` namespace (lines 32-45) carries the correction for `unit+9E4h`/`+9ECh`/`+9F4h`
but it was never propagated to the slot names below. `include/bsp/plane_ai_control.hpp`'s
`PlanSlotIndex` has it right, so the two headers disagree with each other. I did not edit either
file.

## What is **not** established

* **`0099BF30 BSP_PilotBot_RepairCommandBands` is unread** (189 instructions, `RET 4`,
  `0099BF30`-`0099C200`). All I established is the bound that matters: it writes all five command
  floats (`0099C00C` `[EDI+4]`, `0099C0C8` `[EDI]`, `0099C14C` `[EDI+8]`, `0099C17A` `[EDI+0Ch]`,
  `0099C1A6`/`0099C1B7` `[EDI+10h]`, `0099C1E7`/`0099C1F8` `[EDI+0Ch]`), reads `unit+C68h` (bank)
  at `0099BF3A` and `cmd+4h` at `0099BF70`, and calls `0099B940` and a vtable slot `[+38h]`.
  **Until it is read, no claim of the form "the bot's planned value X reaches the plane as Y" is
  supportable** — the slew result is an input to the repair, not the command.
* **No runtime measurement.** Everything here is static. Whether `0099ACD0`'s four entry gates
  pass in the reconstructed host is unknown from this packet.
* **The `current`-field negative covers one function's listing only.** The scan was over
  `0099D300`'s 1451-instruction listing for register+displacement stores. A SIB-indexed store, a
  block copy over the plan object, or a store from one of the planner's callees would be invisible
  to it. The positive control (18 matches on the `desired` fields) establishes that the pattern
  finds stores of that shape, nothing more.
* **`unit+E84h`, `unit+E88h`, `unit+C24h`, `unit+900h`, `unit+61h`, `unit+5Dh`, `unit+9F8h`,
  `unit+894h`/`+8ACh`/`+8C4h`/`+8DCh` are named by role only.** None was independently recovered.
  `unit->vtable[5Ch](0x17)` was not resolved to a class.
* **The meaning of the three request bytes is not recovered.** The chain is traced
  (`plan` -> `cmd` -> `unit+A10h..A12h` -> `unit+9F8h..9FAh` -> `unit+BC8h..BCAh`), but what each
  one requests is not. `cmd+14h`'s effect on throttle is consistent with a boost/WEP flag; that is
  a hypothesis from behaviour, not a recovered symbol.
* **Whether the planner ever leaves `desired` unwritten while setting `active`** was not checked;
  the `0099D362`-style pairs always appear adjacent in the listing, but I did not verify that
  every `active := 1` is dominated by a `desired` store on every path.
* **`plan+2B4h`, `+2C4h`, `+2C8h`, `+2CCh`, `+2D0h`, `+2D4h`, `+2D8h`, `+2E8h`, `+2ECh`,
  `+26Ch`, `+270h`, `+2B0h`, `+2BCh`** are recorded above with the values `0099B450` writes and
  nothing more; none of their readers was traced.
* **Floating-point control word.** `_ftol2_sse` truncates, but whether the x87 rounding mode is
  ever changed around `007BB6E0` (which would move `FDIV`'s last bit) was not checked.
