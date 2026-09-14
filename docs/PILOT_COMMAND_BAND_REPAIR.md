# `0099BF30` read: the control-band repair, and the heading arm's last input

Packet `cc7-pilot-band-repair`, 2026-09-14 UTC. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`.

Addresses: `0099BF30`-`0099C202` (the repair), `0099B940` (the band search; its end was not read),
`0099BDB0`, `0099BDF0`, `0099BE30` (the plan's constructors), `007D7A40`, `0074E260`,
`0099DDBE`-`0099DF03` (the heading error). Constants `00D7A218`, `00D7A208`, `00CE3830`,
`00CE6448`, `00D06BB0`, `00D7A238`, `00D7A24C`, `00D7A260`, `00CE3814`. Vtables `00D056E8`,
`00D00070`, `00D00308`, `00D05F20`.

Ghidra was **read-only**: no rename, no comment, no prototype, no tag, no write lock, no save,
no lease, no git. This document is the only file written. Every descriptive name is a hypothesis,
not a recovered symbol. No runtime measurement was taken; everything below is static.

This closes the top open item of `docs/PILOT_PLAN_SLOT_PIPELINE.md` and
`docs/PILOT_BOT_TICK_GATES.md`.

---

## Headline

1. **By default the repair is a pass-through on all five axes.** The three band tables it
   consults are constructed **empty** (`0099BDB0` stores `0` to the count at table`+0C4h`), and
   `0099B940` returns "unchanged" immediately on an empty table; the throttle ceiling
   `plan+258h` is constructed **`1.0f`** (`0099BE12`, `0099BE56`), and `0099C164` returns without
   touching anything when the ceiling is `>= 1.0f`. On a freshly constructed plan the function
   writes **nothing at all**. `0099B450 BSP_PilotBot_SeedPlanSlots` does not reset either — its
   stores begin at `plan+26Ch` — so whatever fills them persists until something clears it.
   **I did not find any writer of either.** See [What is not established](#what-is-not-established);
   the negative is bounded, not a proof.
2. **The repair overrides rather than passes through only when a band actually catches the
   value.** When it does, the value is snapped to the nearer edge of the forbidden band, or —
   if the band swallows the whole legal range — replaced by a full-deflection fallback whose sign
   comes from the plane's attitude, not from the slew result. The slew result is then discarded
   entirely for that axis.
3. **The gate structure is not what `docs/PILOT_COMMAND_PATH.md:175` records.** Pitch is gated by
   `(unit+72Ch)->vtable[38h]()`; **yaw is ungated**; roll is gated by a *second, independent* call
   to the same predicate. That doc has it as yaw+pitch unconditional and roll gated. It also
   misattributes two of the three fallback selectors and attributes a clamp to roll that is not
   there. Details under [Contradictions](#contradictions-with-existing-docs).
4. **The heading getter is resolved**: `unit->vtable[50h]` is `0074E260`, whose entire body is
   `FLD dword ptr [ECX+0C6Ch] / RET` — it returns `unit+0C6Ch`. `docs/PILOT_BOT_PLAN_CONTROLS.md`'s
   wiring contract still lists it as unread; `include/bsp/plane_ai_control.hpp:238-242` already
   had it (packet `cc7_yaw_remaining_inputs`). This packet confirms it independently on three of
   the nine plane vtables. The doc is stale, not wrong.

---

## ABI and frame

`void __thiscall BSP_PilotBot_RepairCommandBands(Plan* plan, float* cmd)`, `RET 4`
(`0099C1AE`, `0099C1BF`, `0099C1EF`, `0099C200`). Ghidra body `0099BF30`-`0099C202`,
189 instructions. `ESI = plan` (`0099BF32`), `EDI = cmd` (`0099BF66`). Two callees,
`0099B940` and `007D7A40`, plus one indirect `vtable[38h]` called three times.

Prologue: `PUSH ECX` (a 4-byte local), `PUSH ESI`, `PUSH EDI` (`0099BF49`), `PUSH EBX`
(`0099BF7E`, popped at `0099C107`). With `E` = ESP at entry, `cmd` at `[E+4]`:

| entry-relative slot | role |
| --- | --- |
| `E-4` | `\|bank\|` for the pitch fallback (`0099BF4C`/`0099BF60`), then reused to hold the **original** yaw (`0099C052`), then `-ceiling` / `ceiling` (`0099C187`, `0099C1CF`) |
| `E+4` | **the `cmd` argument slot, overwritten** at `0099BF7F` with `cmd[1]`, and thereafter the by-reference scratch float handed to `0099B940` |

Both slots are reused three and four times. Every access above was normalised with
`python tools/stack_frame_walk.py 0099bf30`, using `frame = disp - depth`, anchored on the three
pre-indirect-call sites the walker resolves outright (`0099BF4C -> -4`, `0099BF66 -> +4`,
`0099BF7F -> +4`). The walker reports `depth=-12` from `0099C1C2` onward; that is a linear-sweep
artifact of falling through a `RET 4`. The reaching edge is `0099C170 JC 0099C1C2` at `depth=12`,
which is the depth used here.

**A host must not model `cmd` as a single variable.** The function destroys its own argument slot
at `0099BF7F`; only `EDI` survives.

---

## The plan's control-band block, `plan+0h`-`plan+25Bh`

The plan's constructor `0099BE30` opens with

```
_eh_vector_constructor_iterator_(plan, 200, 3, 0099BDB0, 007B3BA0)
```

— **three elements of `0xC8` bytes**, element constructor `0099BDB0`, destructor `007B3BA0`.
That fixes the layout, and it matches the three `ECX` values the repair passes to `0099B940`
exactly:

| table | address | `ECX` at the call | command index | axis |
| --- | --- | --- | --- | --- |
| 0 | `plan+0h` | `0099C050 MOV ECX,ESI` | `cmd[0]` | yaw |
| 1 | `plan+0C8h` | `0099BFA4 LEA ECX,[ESI+0xC8]` | `cmd[1]` | pitch |
| 2 | `plan+190h` | `0099C114 LEA ECX,[ESI+0x190]` | `cmd[2]` | roll |

**The table index is the command index**, not the slot index. (The five plan slots at `plan+274h`
are in a different order — see `docs/PILOT_PLAN_SLOT_PIPELINE.md`'s axis-order table.)

The element constructor `0099BDB0`, read from disk bytes:

```
0099bdb0  [eax+0]    = 00D056E8        ; vtable — the table is a polymorphic object
0099bdbc  edx = eax+4 ; esi = 0x17
0099bdc4  [edx] = 0.0f ; [edx+4] = 0.0f ; edx += 8 ; esi-- ; jns    ; 24 iterations (0x17..0)
0099bdd5  [eax+0xC4] = 0                ; the count
```

so each table is

```c
struct ControlBandTable {           // 0xC8 bytes
    void*  vptr;                    // +0h,   00D056E8
    struct { float lo, hi; } band[24];  // +4h .. +0C3h, all 0.0f at construction
    float  count;                   // +0C4h, 0 at construction; read as float, used as int
};
```

`0099B940` reads the count at `param_1[0x31]` = `+0C4h` and the bands at `param_1[2i+1]` /
`param_1[2i+2]` = `+4h+8i` / `+8h+8i`, which is the same layout arrived at from the other end.
Then:

| plan offset | what | value at construction |
| --- | --- | --- |
| `+0h` | band table 0 (yaw) | empty |
| `+0C8h` | band table 1 (pitch) | empty |
| `+190h` | band table 2 (roll) | empty |
| `+258h` | throttle ceiling, float | `1.0f` (`0099BE12`, `0099BE56`, `00D7A24C`) |
| `+25Ch` | dword | `0` (`0099BE1A`, `0099BE5E`) |

Two constructors write `+258h`/`+25Ch`: `0099BDF0` (which runs the vector ctor itself) and
`0099BE30` (which additionally fills `+260h`..`+2F8h` and tail-calls `0099B450`). `0099BE30` also
sets `plan+2F0h = unit` and `plan+2F4h = [unit+538h]`, the class descriptor — consistent with
`docs/PILOT_PLAN_SLOT_PIPELINE.md`.

---

## `0099B940` — the band search

`bool __thiscall(ControlBandTable* t, float* v)`, `RET 4` (the stack walker reports "callee pops 4"
at `0099BFAA`). One caller, `0099BF30`. Returns in `AL`; every call site tests `TEST AL,AL`.

```c
// 0099B940.  Returns false only when the caught band spans the whole legal range.
bool band_repair(const ControlBandTable* t, float* v) {
    if (t->count == 0.0f) return true;          // the empty-table fast path
    int n = (int)t->count;
    if (n <= 0) return true;
    float x = *v;
    for (int i = 0; i < n; i++) {
        if (x <= t->band[i].lo || t->band[i].hi <= x) continue;   // not inside this band
        if (t->band[i].lo < -1.0f && 1.0f < t->band[i].hi)
            return false;                        // band swallows [-1,+1]: no edge to snap to
        *v = (t->band[i].hi - x < x - t->band[i].lo) ? t->band[i].hi   // nearer the top edge
                                                    : t->band[i].lo;
        return true;
    }
    return true;
}
```

The bands are **forbidden** intervals, tested **strictly** open: a value exactly on an edge is
already legal and is not moved. The search stops at the first band that catches the value; it does
not iterate to a fixed point, so overlapping bands can leave the result inside a later band.

`-1.0f` is `00D7A260` and `1.0f` is `00D7A24C`, the same two constants the slew clamp uses.

---

## The per-axis repair rule

Ghidra's decompilation is faithful here; the shapes below were nevertheless re-read from the
listing. `P` abbreviates `(unit+72Ch)->vtable[38h]()`, a `bool __thiscall` predicate called
**three separate times** (`0099BF85`, `0099C026`/`0099C0E1`, `0099C103`) and never cached. It is
the same predicate `0099D300`'s entry gate B uses (`0099D3CB`), unidentified in both places.

```c
// 0099BF30.  bank = unit+0C68h (the bank angle, docs/PILOT_BOT_PLAN_CONTROLS.md:108).
float  absBank = (bank > 0.0f) ? bank : (-0.0f - bank);      // 0099BF42-0099BF60, 00D7A208 = -0.0f
float  v;

// ---- cmd[1], PITCH -------------------------------------------- 0099BF70-0099C037
if (P) {                                                     // 0099BF85/0099BF99, else skip
    v = cmd[1];
    if (!band_repair(&plan->band[1], &v))                    // 0099BFA4, table plan+0C8h
        v = ((double)absBank <= 1.5707963705062866) ?  1.1f  // 00CE3830 = (double)(float)(pi/2)
                                                    : -1.1f; // 00CE6448 / 00D06BB0
    if (v != cmd[1]) {                                       // 0099BFF0 FUCOMIP / LAHF / TEST AH,44h / JNP
        cmd[1] = clampf(v, -1.0f, 1.0f);                     // 0099BFFA-0099C00C
        if (P) notify(unit+0AB0h);                           // 0099C026-0099C032
    }
}

// ---- cmd[0], YAW ------------------------------------------------ 0099C047-0099C0F2
// NOT gated on P.
v = cmd[0];
if (!band_repair(&plan->band[0], &v))                        // 0099C050 MOV ECX,ESI, table plan+0h
    v = (bank > 0.0f) ? 1.1f : -1.1f;                        // 0099C06D re-reads unit+0C68h RAW
if (v != cmd[0]) {                                           // 0099C0AC, same idiom
    cmd[0] = clampf(v, -1.0f, 1.0f);                         // 0099C0B6-0099C0C8
    if (P) notify(unit+0AB0h);                               // 0099C0E1-0099C0ED
}

// ---- cmd[2], ROLL ----------------------------------------------- 0099C0F2-0099C14C
if (P) {                                                     // 0099C103/0099C108, else skip
    v = cmd[2];
    if (!band_repair(&plan->band[2], &v))                    // 0099C114, table plan+190h
        v = (cmd[2] >= 0.0f) ? 1.0f : -1.0f;                 // 0099C129 COMISS on the COMMAND, not the bank
    cmd[2] = v;                                              // 0099C14C, UNCONDITIONAL, NO clamp
}

// ---- cmd[3] THROTTLE and cmd[4] AIR BRAKE ----------------------- 0099C151-0099C200
float ceil = plan->throttleCeiling;                          // plan+258h
if (ceil < 1.0f) {                                           // 0099C161 COMISS 1.0,ceil / JBE return
    if (ceil > 0.0f) {                                       // 0099C16D COMISS 0.0,ceil / JC
        cmd[3] = (ceil <= cmd[3]) ? ceil : cmd[3];           // 0099C1DB / 0099C1E7 / 0099C1F8 = min
    } else {
        cmd[3] = 0.01f;                                      // 0099C172/0099C17A, 00D7A238
        cmd[4] = (cmd[4] <= -ceil) ? -ceil : cmd[4];         // 0099C185 FCHS, 0099C19A / 0099C1A6 / 0099C1B7 = max
    }
}
```

### Which stores pass through and which override

| axis | store | runs when | pass-through or override |
| --- | --- | --- | --- |
| `cmd[1]` pitch | `0099C00C` | `P` **and** the band search changed the value | **override**; the slew result is discarded |
| `cmd[0]` yaw | `0099C0C8` | the band search changed the value | **override** |
| `cmd[2]` roll | `0099C14C` | `P` | **store always executes**, but writes the slew result back unchanged unless a band caught it |
| `cmd[3]` throttle | `0099C17A` | `ceil <= 0.0f` | **override**, to the constant `0.01f` |
| `cmd[3]` throttle | `0099C1E7` / `0099C1F8` | `0.0f < ceil < 1.0f` | **min**: `0099C1E7` writes the slew result back unchanged, `0099C1F8` overrides with `ceil` |
| `cmd[4]` air brake | `0099C1A6` / `0099C1B7` | `ceil <= 0.0f` | **max**: `0099C1A6` writes the slew result back unchanged, `0099C1B7` overrides with `-ceil` |

The pairs at `0099C1A6`/`0099C1B7` and `0099C1E7`/`0099C1F8` that
`docs/PILOT_PLAN_SLOT_PIPELINE.md` lists as two stores each are the two arms of one `min`/`max`;
one arm of each is a same-value store, not a second write.

For pitch and yaw the `!=` test exists only to skip the notify side effect — the clamp behind it
is a no-op when the value did not change, since the slew already clamped to `[-1,+1]`. A host may
model those two as unconditional `cmd[n] = clampf(v, -1, 1)` **provided** it reproduces the notify
gating separately.

### Three asymmetries that a plausible-looking reimplementation gets wrong

* **The pitch fallback selector is `|bank|` against a `double` `pi/2`; the yaw fallback selector
  is the *raw signed* bank against `0.0f`; the roll fallback selector is the sign of the
  *incoming roll command*, not the bank at all.** Three different selectors in three arms of the
  same function. `0099BF42` computes `|bank|` once for pitch; `0099C06D` **re-reads**
  `unit+0C68h` for yaw rather than reusing it, and compares it raw.
* **Pitch and yaw fall back to `±1.1f`, roll to `±1.0f`.** The `1.1f` then meets the `[-1,+1]`
  clamp and lands on `±1.0f`, so the observable outcome is the same full deflection; the `0.1`
  of headroom is only visible to the `v != cmd[n]` test, which it forces to fire even when the
  command was already at full deflection. Roll has no clamp, so its `±1.0f` is used directly.
* **The `pi/2` comparison is done in double precision** (`0099BFB3 FLD double ptr [00CE3830]`,
  `00CE3830 = 00 00 00 60 FB 21 F9 3F` = `1.5707963705062866`, which is `(double)(float)(pi/2)`,
  not the `double` `pi/2`). A host comparing against `M_PI_2` differs in the last bits.

### The notify side effect, `007D7A40`

`void __fastcall(ctl)`, called as `007D7A40(unit+0AB0h)` at `0099C032` and `0099C0ED`:

```c
if (*(void**)(ctl + 0x10) != NULL)
    *(float*)(*(void**)(ctl + 0x10) + 0xC8) = 1.2f;   // 00CE3814 = 9A 99 99 3F
```

`unit+0AB0h` is the plane's flight-control block: it is the same `ctl` that
`007D9A70 BSP_PlaneFlight_ControlAuthority` takes (`docs/PLANE_CONTROL_AUTHORITY.md`), where
`ctl+8` is the unit, `ctl+0Ch` the class descriptor and `ctl+10h` "a third object" whose `+0C0h`
feeds the authority's `B` term. So the repair writes `1.2f` into `[ctl+10h]+0C8h`, eight bytes past
a field the control-authority scalar reads. **What `[ctl+10h]+0C8h` does was not traced.** The
other caller of `007D7A40` is `0099B1B0`.

---

## What this means for a host

On a plan as constructed, `0099BF30` is a **no-op**: all three band counts are `0`, so
`0099B940` returns `true` without touching the value on every axis, so the `v != cmd[n]` tests all
fail and the two gated stores write the slew result back unchanged; and `plan+258h == 1.0f` makes
`0099C164` return before the throttle arm. A host that models the whole function as a no-op
therefore reproduces the *default* behaviour exactly — which is what
`docs/PILOT_PLAN_SLOT_PIPELINE.md` headline 1 could not settle.

That is only safe while nothing fills the tables or lowers the ceiling. Both are plan state that
`0099B450` does **not** reset per think, so a single writer anywhere would make the behaviour
sticky. A host should carry the fields and implement the rule; it should not assume the no-op.

---

## The heading term of the yaw law

`docs/PILOT_BOT_PLAN_CONTROLS.md:565-700` already recovers this whole path. I **take the following
on trust** from it and did not re-derive them: the yaw arm's body at `0099E81A`-`0099EA46`, the
base gain `[ESP+38h]` at `0099DFFB`-`0099E027`, the turn numerator `[ESP+10h]`, the frame table,
and the reaching-definition result that `[ESP+6Ch]` and `[ESP+38h]` each have a single producer.
What follows is the assembled end-to-end path plus what I verified myself.

```
plan+2CCh == 2 ?                                             0099DDBE load, 0099DE8A CMP ECX,2
   h = unit->vtable[50h]()  == unit+0C6Ch                    0099DE9B/0099DE9E -> 0074E260
   e = SubtractWrappedAngle(plan+2C0h, h) * q                0099DEAF, 0099DEB8, 0099DEBD
       q  = [ESP+28h] = 1 / max(unit+340h * 0.4, 1.0)        (frame, 0099D4EA)
   D = tuning+3Ch                                            0099DEC5
   e = (e >= D) ? e - D : (e <= -D) ? e + D : 0              0099DED4 / 0099DEE6-0099DEEC / 0099DEFC
   L = (class+1C8h + class+1ACh) * tuning+38h                0099DF0F-0099DF24
   e = (L > |e|) ? tuning+34h * e
                 : (e > 0 ? e - (1-tuning+34h)*L
                          : e + (1-tuning+34h)*L)
   [ESP+6Ch] = e                                             (else it keeps the 0 from 0099DDD0)

g = InterpolateClamped(tuning+7Ch, 1.0f, tuning+80h, 0.0f, |bank|)      0099DFFB-0099E027
[ESP+38h] = g

base = 0                                                     0099E823
if (g > 0)                                                   0099E820/0099E829
    base = clamp(e / (YawSpd * cos(bank) * tuning+9Ch), -1, +1) * g
if (turn numerator > 0) {                                    0099E88E/0099E897
    t    = min(1, InterpolateClamped(tuning+7Ch, 0, tuning+80h, 3.0f, |bank|))
    base = (1 - t) * base                                    0099E94A-0099E95C
    yaw  = t * turn + base
} else
    yaw  = base
slot(yaw).desired = clamp(yaw, -1, +1)                       0099EA3E
slot(yaw).active  = 1                                        0099EA46
plan+2D4h = 0                                                0099EA4D
```

### What `plan+2CCh` contributes — verified here

It is a hard on/off gate on the heading term, and only the value `2` enables it. `0099DDBE
MOV ECX,[ESI+0x2CC]` is the **only** load of `plan+2CCh` into `ECX` in `0099D300` (a `.text` scan
for `8B 8E CC 02 00 00` returns two hits program-wide, one here and one in
`BSP_MissionScoring_RecordUnitKill` — the pattern demonstrably matches, so the singleton is
meaningful). A disassembly sweep of the whole `0099DDBE`-`0099DE8A` range shows **no write to
`ECX` and no `CALL`** in between, so the value compared at `0099DE8A` is that load. On any other
value of `plan+2CCh`, `0099DE8D JNZ 0099E26E` skips the block and `[ESP+6Ch]` keeps the `0`
stored at `0099DDD0`, which zeroes `base` and leaves the yaw law running on the turn term alone.

`0099D6C6` clears `plan+2CCh` when a direct roll stick input is present, so a human roll input
also disables the bot's heading hold. That is `PILOT_BOT_PLAN_CONTROLS.md`'s reading and it is
consistent with everything here.

### The heading getter — resolved

`unit->vtable[50h]` at `0099DE9B`/`0099DE9E` is `0074E260`. Ghidra has **no function** there;
from disk bytes the entire body is

```
0074e260  d9 81 6c 0c 00 00     FLD dword ptr [ECX + 0xC6C]
0074e266  c3                    RET
0074e267  cc cc cc cc cc        INT3 padding
```

— `float __thiscall(unit)`, `RET 0`, returning `unit+0C6Ch` in `ST0`. I read slot `+50h` on three
plane vtables and all three hold `0074E260`: `00D00070` (installed by
`BSP_ReconPlaneUnitInstance_Construct` at `0074E0F0`), `00D00308` and `00D05F20`. The vtable base
was fixed by locating `0074E1E0 BSP_Plane_SetForwardSpeed_VtableThunk` at `00D000AC` = base`+3Ch`.
`include/bsp/plane_ai_control.hpp:238-242` records the same result across all nine plane vtables
from packet `cc7_yaw_remaining_inputs`; this is an independent confirmation, and it makes
`docs/PILOT_BOT_PLAN_CONTROLS.md:698`'s "did not read" **stale**.

`unit+0C6Ch` is the heading written by `007C1ACA` and read at `007C18C1`
(`docs/PILOT_BOT_PLAN_CONTROLS.md:127`), adjacent to `unit+0C64h` (pitch) and `unit+0C68h` (bank),
the same bank field this repair function reads at `0099BF3A` and `0099C06D`. So the repair and the
heading term draw on the same attitude triple.

### Independently re-read here

`0099DE8A`-`0099DF03`, the deadband. Every x87 form was settled from bytes, because the two
branches differ only by a sign:

| address | text | bytes | meaning |
| --- | --- | --- | --- |
| `0099DED8` | `FSUBRP` | `DE E1` | `ST1 := ST0 - ST1`, pop -> `e - D` |
| `0099DEE6` | `FLD ST1` | `D9 C1` | push a copy of `D` |
| `0099DEE8` | `FCHS` | `D9 E0` | negate -> `-D` |
| `0099DEEE` | `FADDP` | `DE C1` | `ST1 := ST1 + ST0`, pop -> `e + D` |

`0099DED4 FCOMI ST0,ST1` compares `e` against `D` with `JC` taken when `e < D`; `0099DEEA FCOMIP`
compares `-D` against `e` with `JC` taken when `-D < e`, leaving the `XORPS` zero at `0099DEFC` for
`-D < e < D`. That is the doc's rule exactly.

The argument order at the wrap helper is `SubtractWrappedAngle(plan+2C0h, h)`: `0099DEA8 SUB ESP,8`
then `0099DEAB FSTP [ESP+4]` stores `h` as the **second** argument and `0099DEAF FLD [ESI+0x2C0]` /
`0099DEB5 FSTP [ESP]` stores the target heading as the **first**. `00438B10` must be `RET 8`,
because `0099DEBD FMUL [ESP+0x28]` reads a frame slot at the pre-`SUB` depth with no `ADD ESP,8`
in between, and `PILOT_BOT_PLAN_CONTROLS.md:388` establishes that ESP is stable across the whole
body. I did not read `00438B10` itself.

### A naming clash between the docs, not a disagreement

`docs/PILOT_BOT_PLAN_CONTROLS.md` writes these offsets as `task+2C0h`, `task+2CCh`, `task+2D4h`;
`docs/PILOT_BOT_TICK_GATES.md` and `docs/PILOT_PLAN_SLOT_PIPELINE.md` write the same `ESI`-relative
offsets as `plan+2C0h` etc. `ESI = plan` throughout `0099D300`, and `plan = task+4`, so
`plan+2C0h` is `task+2C4h` — the two spellings are the same field under different names for the
object, not two fields. This document uses `plan+`.

---

## Contradictions with existing docs

**`docs/PILOT_COMMAND_PATH.md:175-182`** contains a partial read of this function
(`coverage: partial`, "past `0099C130` unread"). Three of its statements are wrong:

1. *"For `cmd[1]` and then `cmd[0]`, and for `cmd[2]` only when the unit's `+72Ch` `vtable[+38h]`
   holds"* — the gate is on the **wrong axis**. `0099BF85 CALL EAX` / `0099BF99 JZ 0099C047`
   skips the **pitch** arm when the predicate is false and lands on the yaw arm. **Yaw
   (`0099C047`-`0099C0F2`) is not gated at all.** Roll has its own independent call at
   `0099C0F2`-`0099C108`. So the correct statement is: pitch gated, yaw ungated, roll gated.
2. *"replaced by `±1.1f` ... or `±1.0f` chosen by the sign of `unit+C68h`"* — only the **yaw**
   fallback is chosen by the sign of `unit+0C68h` (`0099C075`). The **pitch** fallback is chosen
   by `|bank| <= (double)(float)(pi/2)` (`0099BF42`-`0099BF60` then `0099BFBD`), a different
   predicate that answers "is the plane inverted", and the **roll** fallback (`±1.0f`) is chosen
   by the sign of **`cmd[2]` itself** at `0099C129`, which never reads the bank.
3. *"and then clamped to `[-1, 1]`"* — pitch (`0099BFFA`-`0099C009`) and yaw
   (`0099C0B6`-`0099C0C5`) are clamped; **roll is not**. There is no comparison between
   `0099C146` and the store at `0099C14C`.

It also gives only the `ceil <= 0` arm of the tail and not the `0 < ceil < 1` arm
(`0099C1C2`-`0099C1F8`, `cmd[3] = min(cmd[3], ceil)`), and it calls the tail unread while quoting
part of it.

**`docs/PILOT_PLAN_SLOT_PIPELINE.md`** is not contradicted. Its store census
(`0099C00C` `[EDI+4]`, `0099C0C8` `[EDI]`, `0099C14C` `[EDI+8]`, `0099C17A` `[EDI+0Ch]`,
`0099C1A6`/`0099C1B7` `[EDI+10h]`, `0099C1E7`/`0099C1F8` `[EDI+0Ch]`) is exactly right, and so are
the reads it lists. Its "`0099BF30` is unread ... Nobody has read the body" is stale by one doc
(`PILOT_COMMAND_PATH.md` had read it partially), and its open question — whether the repair passes
the slew result through — is answered above.

**`docs/PILOT_BOT_PLAN_CONTROLS.md:698`** lists `unit->vtable[50h]()` as one of "the two sources
this packet did not read". It is read: `0074E260`, returning `unit+0C6Ch`. The other,
`007D9A70(unit+AB0h)`, has since been read too — it is
`BSP_PlaneFlight_ControlAuthority`, `docs/PLANE_CONTROL_AUTHORITY.md`. **The yaw axis's wiring
contract in that section is stale in the host's favour**: both sources it says a host must refuse
the axis without are now available.

---

## What is **not** established

* **No writer of the band tables or of `plan+258h` was found, and that negative is weak.**
  A `.text` scan for `MOVSS [reg+disp32],xmm` with `disp32 == 0x258` returns four hits: the two
  plan constructors (`0099BE12`, `0099BE56`) and two in unrelated render code
  (`00B0D02A`, `00B14B65`). The integer-store forms `C7 ?? 58 02 00 00` and `89 ?? 58 02 00 00`
  return hits program-wide (the positive control) but none in the pilot-bot range. **The scan is
  blind to** a store through a register holding `plan+258h`, a SIB-indexed store, a block copy over
  the plan, and any write from a caller that holds the address in a local. The band counts at
  `+0C4h`/`+18Ch`/`+254h` are too generic an offset to census this way, and I did not attempt it
  by another method. **Treat "the repair is a no-op" as the constructed default, not as proof that
  it always is.**
* **`(unit+72Ch)->vtable[38h]` is unidentified.** It gates pitch and roll and gates the notify on
  pitch and yaw. It is the same predicate as `0099D3CB`'s in `0099D300` gate B, where
  `docs/PILOT_BOT_TICK_GATES.md` also leaves it unnamed. The owning class of the `unit+72Ch`
  sub-object was not resolved.
* **`[ctl+10h]+0C8h`, the target of `007D7A40`'s `1.2f`, was not traced**, nor was `ctl+10h`
  identified — `docs/PLANE_CONTROL_AUTHORITY.md:91` leaves it open too. Whether the notify has any
  effect on the same tick is unknown.
* **The band tables' vtable `00D056E8` was not followed.** Its eight slots are `007B3BB0`,
  `007B3D30`, `0042B110`, `0042B120`, `0042B130`, `0042B140`, `006935C0`, `007B3E40`; the two I
  read (`007B3D30`, `007B3E40`) are deleting destructors. If there is an "add band" API it is one
  of the other six or a non-virtual member, and I did not find it.
* **Whether `0099DE8A` has an incoming edge from outside `0099DDBE`-`0099DE8A`.** The
  no-`ECX`-write sweep covers that straight-line range only; `0099DE85 JMP 0099E26E` immediately
  precedes the target, so `0099DE8A` is a branch target, and I confirmed only that no edge
  *originating inside the swept range* can clobber `ECX`.
* **`00438B10 SubtractWrappedAngle` was not read**, only its argument order and its inferred
  `RET 8`.
* **The heading term is taken on trust from `docs/PILOT_BOT_PLAN_CONTROLS.md` except for**
  the `plan+2CCh` gate, the `0099DE8A`-`0099DF03` deadband, the `plan+2C0h` read and argument
  order, and the `vtable[50h]` resolution, all of which I re-read here. The gain `[ESP+38h]`, the
  turn numerator, the `L` step limiter's operands, the clamp inside the yaw arm and the store at
  `0099EA3E` are **not** re-verified by this packet.
* **Nothing here is runtime-measured, built, or fixture-tested.** No C++ was written and no
  existing file was edited.
