# The heading-difference scale `0099D0A0` — how much heading a roll-out buys

`docs/PILOT_PLANNER_PITCH_ROLL.md` §1b lists `0099D0A0` under "what is **not** established": it
produces the `C` that the bank command divides by, and nothing about its magnitude was read. This
document reads it. Names below are hypotheses; addresses and encodings are the evidence.

## What it is

The four tuning keys it reads are `Pilot/General/HdgDiffCalc*`, and this installation's
`scripts/datatables/planeglobals.lua` comments them in Hungarian (lines 367-369):

> `HdgDiffCalcMinPitch = 0.1` — *"mikor megnezzuk hogy mennyit kanyarodna meg amig a rolljat
> vizszintesbe hozza, akkor legalabb ekkora pitch-el szamolunk (huzott fordulo)"*
> — "when we check **how much more it would turn while it brings its roll back to level**, we
> compute with at least this much pitch (pulled turn)".
>
> `HdgDiffCalcMinRoll = DEG(5)` — same sentence, "…with at least this much roll".
>
> `HdgDiffCalcLimit = {0.8, 1.5}` — *"lehetoleg 0.1 es 1.0 kozott legyen. mennyire finoman alljon be
> iranyba fordulaskor"* — "should preferably be between 0.1 and 1.0; how smoothly it settles onto
> the heading when turning".

So the function answers: **starting from bank `w`, how much heading will the aircraft still sweep
while it rolls back to wings-level?** It is `turn_rate(w) * rollout_time(w)`, returned negated. The
planner then divides the heading error by it, which is a lead/normalisation: "the error I can still
absorb for free is exactly this many radians."

## ABI

```
float10 __thiscall FUN_0099D0A0(PilotPlan* plan /*ECX*/, float w /*[ESP+4]*/);
```

`RET 4` (`0099D0B9`, `0099D0FF`, `0099D2DF`, `0099D2F6`), result in `ST0`, body
`0099D0A0`-`0099D2F6`, 161 instructions. Saves and restores `ESI`, `EBX`, `EDI`. Called once, at
`0099E0C1`, with `w = min(|bank| * 1.5, maxBank)` (the `min` is `0099E095`-`0099E0B1`).

Bases: `unit = *(void**)(plan + 0x2F0)` (`0099D109`), `class = *(void**)(plan + 0x2F4)`
(`0099D18A`, `0099D1F0`, `0099D232`), `tuning = BSP_GameTuning_GetSingleton() + 0x538`
(`0099D104`/`0099D119`), so `tuning+4Ch/50h/54h/58h` are singleton `+584h/+588h/+58Ch/+590h`.

## Stack map

`ESP_m` = ESP after `PUSH EBX; PUSH EDI` at `0099D102`/`0099D103`, i.e. `ESP_entry - 0x28`. The
epilogue forces this: `POP EDI` (`0099D255`) requires `ESP == ESP_m`, and `POP ESI; ADD ESP,0x1C;
RET 4` (`0099D2DB`) requires `ESP == ESP_m + 8` there. Both hold only if `00419010` cleans its own
`0x14` — it does, `00419030: RET 0x14`.

| slot | contents (address that writes it) |
| --- | --- |
| `ESP_m+00` | saved `EDI` |
| `ESP_m+04` | saved `EBX` |
| `ESP_m+08` | saved `ESI` |
| `ESP_m+0C` | `\|w\|` (`0099D0CB`/`0099D0DF`) → `W` clamped in place (`0099D181`) → `YawSpd` (`0099D24B`) |
| `ESP_m+10` | `L` (`0099D1B9`) → `cos W` (`0099D224`) |
| `ESP_m+14` | `MinPitch` scratch (`0099D149`) → `cos θ` (`0099D22E`) |
| `ESP_m+18` | `P` (`0099D165`) |
| `ESP_m+1C` | `1.5f` clamp-high scratch (`0099D17B`) → `sin W` (`0099D21A`) → `R` (`0099D2BD`) |
| `ESP_m+20` | `θ = unit+C64h` (`0099D126`) → running sum `S`/`Q` (`0099D261`, `D275`, `D299`, `D2AB`) |
| `ESP_m+24` | `T` (`0099D210`) |
| `ESP_m+28` | return address |
| `ESP_m+2C` | **`w`, the incoming argument** — read at `0099D23E`; also the return staging slot (`0099D2D1`/`0099D2E8`) |

## The three return paths

1. **`plan+2F0h == 0` → `0.5f`.** `0099D0A6` `CMP dword ptr [ESI+0x2F0],0`; `0099D0AF`
   `FLD [00CE3800]`, `00CE3800 = 00 00 00 3f = 0.5f`. No unit, no dynamics: a fixed scale.
2. **`|w| < 0.1f` → `-w`.** `0099D0E5`-`0099D0FF`. `FLD [ESP+4]` (=`|w|`), `FLD qword [00D7A3A0]`,
   `FCOMIP ST0,ST1` compares `0.1` against `|w|`; `JBE` falls through when `0.1 > |w|`, and the arm
   reloads the **unmodified** argument `[ESP+0x24]` and `FCHS`. Below a token bank the answer is the
   bank itself, negated — which fixes the sign convention for path 3.
3. **otherwise → `w >= 0 ? -R : +R`**, `R` below. `0099D244` `COMISS XMM0,[00D7A218]` with
   `XMM0 = w` and `00D7A218 = 0.0f` sets `CF` ⟺ `w < 0` (or unordered); the flag is consumed
   **77 bytes later** by `JC 0x0099D2E2` at `0099D2C1`. Nothing between them writes `EFLAGS` — only
   x87 arithmetic, `MOVSS`, `FXCH` and two `POP`s — which is why the compiler could hoist it.
   `JC` taken (`w < 0`) returns `R`; fallthrough (`w >= 0`) computes `-0.0f - R` (`00D7A208 =
   00 00 00 80 = -0.0f`) at `0099D2C3`/`0099D2CB`.

## The expression

```
W = ClampInPlace(|w|, tuning+58h HdgDiffCalcMinRoll, 1.5f)          ; 0099D181, 00CE380C = 1.5f
pitchCmd = (plan+2A0h != 0) ? plan+29Ch : plan+298h                 ; 0099D11F-0099D140
P = max(pitchCmd, tuning+54h HdgDiffCalcMinPitch)                   ; 0099D14D-0099D165
L = InterpolateClamped(0.0f, tuning+4Ch, class+25Ch TurnRoll,
                       tuning+50h, W)                               ; 0099D1B4
if ((unit->vtable[5Ch](0x10) || unit->vtable[5Ch](0x16)) && L >= 1.0f)
    L = 1.0f                                                        ; 0099D1C2-0099D1EA
θ = unit+C64h                                                       ; 0099D10F  (pitch angle)

T = W / class+1A8h RollSpd / L  +  1.0f / class+1BCh RollAccel      ; 0099D1F6-0099D210
Q = class+1C8h TurnRollSpd * cos θ                                  ; 0099D25B-0099D261
  + class+1ACh PitchSpd  * P                                        ; 0099D269-0099D275
  + class+1B0h YawSpd * class+1B8h SlideRatio * cos θ * cos W       ; 0099D27D-0099D299
  + 0.8f * class+1B0h YawSpd                                        ; 0099D2A3-0099D2AB
R = Q * sin W * T                                                   ; 0099D2AF-0099D2BD
return (w >= 0) ? -R : R                                            ; 0099D2C1-0099D2F6
```

Read as physics: `T` is the roll-out **time** — bank over roll rate, plus a `1/RollAccel` allowance
for getting the roll started — and `Q * sin W` is the **turn rate** at that bank (`sin(bank)` is the
usual horizontal projection; `Q` is the effective pitch/turn rate the airframe can hold, summed over
its turn-roll, elevator, sideslip-yaw and a constant yaw floor). Their product is a heading, in
radians. That is exactly the Hungarian comment's "how much more it would turn while it brings its
roll back to level".

### Sign

`W ∈ [DEG(5), 1.5]` so `sin W > 0` and `cos W ≥ cos 1.5 ≈ 0.07 > 0`; `θ` is
`atan2(m21, sqrt(m20²+m22²)) ∈ [-π/2, π/2]` (`007C1966`, `docs/PILOT_BOT_PLAN_CONTROLS.md`) so
`cos θ ≥ 0`; `P ≥ HdgDiffCalcMinPitch = 0.1 > 0`; the class rates are positive. Hence `Q > 0`,
`T > 0`, **`R > 0`**, and path 3 returns a **negative** number for the non-negative `w` the one call
site supplies. Path 2 agrees (`-w ≤ 0`). Corroboration from the consumer: the small-`C` floor at
`0099E0FC` is `00CE3CB4 = cd cc cc bd = -0.1f` on the negative side but only `00D7A238 = +0.01f` on
the positive side — the asymmetry is built around `C` normally being negative.

### x87 encodings settled from bytes

Every mnemonic Ghidra prints ambiguously was checked with `ghidra bytes`:

| addr | bytes | meaning |
| --- | --- | --- |
| `0099D20C` | `de f1` | `FDIVRP ST(1),ST(0)` → `ST1 = ST0/ST1` = `1.0/RollAccel` (not `RollAccel/1.0`) |
| `0099D20E` | `de c1` | `FADDP ST(1),ST(0)` |
| `0099D25D` | `de ca` | `FMULP ST(2),ST(0)` → `ST2 *= ST0` |
| `0099D273` | `de c1` | `FADDP ST(1),ST(0)` |
| `0099D281` | `de ca` | `FMULP ST(2),ST(0)` |
| `0099D289` | `de ca` | `FMULP ST(2),ST(0)` |
| `0099D291` | `de cb` | `FMULP ST(3),ST(0)` (after `d9 c0` `FLD ST(0)` at `0099D28F`) |
| `0099D295` | `de c2` | `FADDP ST(2),ST(0)` |
| `0099D2A3` | `dc 0d 40 3d ce 00` | `FMUL qword [00CE3D40]` → `ST0 *= 0.8` |
| `0099D2A9` | `de c1` | `FADDP ST(1),ST(0)` |
| `0099D2BB` | `de c9` | `FMULP ST(1),ST(0)` |

### The two doubles are float-authored

`00D7A3A0 = 00 00 00 a0 99 99 b9 3f = 0x3FB99999A0000000` and
`00CE3D40 = 00 00 00 a0 99 99 e9 3f = 0x3FE99999A0000000`. Neither is the nearest double to `0.1`
or `0.8` (`0x3FB999999999999A`, `0x3FE999999999999A`); both are the **exact widening of the float**
`0.1f` / `0.8f`. A reconstruction that writes `0.8` as a C `double` literal will not be
bit-identical. Write `(double)0.8f`.

## The clamp at `0099D181` targets `|w|`, not the argument slot

`0099D161 LEA EAX,[ESP+0x1C]`; `0099D173 PUSH EAX` (so `ESP = ESP_m - 4`); `0099D177 LEA
ECX,[ESP+0x10]` = `ESP_m + 0x0C`; `0099D17B MOVSS [ESP+0x20],XMM0` writes `1.5f` into
`ESP_m + 0x1C`, which is what `EAX` points at; `0099D174 LEA EDX,[EDI+0x58]`. With
`BSP_Math_ClampInPlace` = `__fastcall(float* value /*ECX*/, const float* low /*EDX*/, const float*
high /*stack*/), RET 4` (ledger, `00415690`), the clamped object is `ESP_m + 0x0C` — the **absolute
value** computed at `0099D0BC`-`0099D0E5`, not the argument at `ESP_m + 0x2C`.

This distinction is load-bearing, not cosmetic: the sign test at `0099D23E` reads `[ESP+0x2C]`, the
untouched argument. If the clamp had hit the argument slot, the argument would always be in
`[DEG(5), 1.5]` and the `w < 0` path would be unreachable by construction. It is unreachable only
*in practice*, from the call site's value — a different thing.

## The `L` rate multiplier and the `1.0f` cap

`L = InterpolateClamped(x0=0, y0=HdgDiffCalcLimit1, x1=class+25Ch TurnRoll, y1=HdgDiffCalcLimit2,
x=W)` — argument order per the `00419010` ledger entry (`x0,y0,x1,y1,x`), pushed at
`0099D1AF`/`0099D1A8`/`0099D19E`/`0099D197`/`0099D193`. With the shipped values it ramps from `0.8`
at wings-level to `1.5` at `TurnRoll`, clamped to `[0.8, 1.5]`: a steeper bank rolls out
proportionally faster, which is why `T` divides by it.

`0099D1C2`/`0099D1D1` then call `unit->vtable[5Ch]` with `0x10` and `0x16`; either true caps `L` at
`1.0f` (`00D7A24C = 00 00 80 3f`). `docs/VEHICLE_CLASS_DESCRIPTORS.md` kind column maps `10h` →
`LevelBomber` and `16h` → `LargeReconPlane`, so the cap removes the bonus from the two heavy
airframes. Independent corroboration that this pair is the intended grouping: `007D3773`/`007D3782`
query the **same** `{10h, 16h}` pair to give those two classes a 4-second `GearsPullTime` against
everyone else's 2 (`docs/PLANE_CLASS_FIELDS.md`).

## Shipped tuning values

From this installation's `scripts/datatables/planeglobals.lua:367`-`369`:

| singleton | key | shipped value | native default |
| --- | --- | --- | --- |
| `+584h` (`tuning+4Ch`) | `Pilot/General/HdgDiffCalcLimit/1` | `0.8` | none (`007E720D`) |
| `+588h` (`tuning+50h`) | `Pilot/General/HdgDiffCalcLimit/2` | `1.5` | none (`007E727B`) |
| `+58Ch` (`tuning+54h`) | `Pilot/General/HdgDiffCalcMinPitch` | `0.1` | none (`007E72D0`) |
| `+590h` (`tuning+58h`) | `Pilot/General/HdgDiffCalcMinRoll` | `DEG(5)` ≈ `0.0872665` rad | none (`007E7311`) |

`docs/GAME_TUNING_SINGLETON.md:427`-`430` records the same four in its *shipped value* column and a
dash in *native default*: the binary carries **no** fallback for these keys, so the script is the
only source of their values.

**Caveat — not provably retail.** This game installation carries BSPRM/AlterBSP artefacts.
`planeglobals.lua` has mtime `2024-10-29`, later than the bulk install stamp (`globals.lua` is
`2024-07-13`), so it is outside the untouched bulk and cannot be asserted to be the shipped retail
content. The `GAME_TUNING_SINGLETON.md` row is corroboration of the *reading*, not independent
evidence of retail values — it comes from the same script tree. The **structure** above is from the
binary and is unaffected; only the four numbers carry this caveat.

## What is **not** established

* **`unit->vtable[5Ch]` is not read here.** It is taken as the entity-side `IsKindOf(int)` on the
  strength of `docs/PILOT_TASK_HEADING_ARM.md:366` and `docs/PLANE_UNIT_TICK.md:287`. The kind
  numbering `10h`/`16h` is proven for the **descriptor**-side `IsKindOf` (vtable `+18h`,
  `00749010`); that the unit-side slot shares that enum is inference, corroborated by the
  `007D3773`/`007D3782` pairing but not proved.
* **Units.** `T` is called a time and `Q` a rate because the expression has that shape and the Lua
  comment says so. `RollSpd`, `PitchSpd`, `YawSpd`, `TurnRollSpd` and `RollAccel` were not traced to
  their integrators here, so `1/RollAccel` being seconds is unverified. Do not depend on the
  dimensional story; depend on the expression.
* **`plan+29Ch` vs `plan+298h` scaling.** They are pitch slot 3's *desired* and *current*
  (`docs/PILOT_PLAN_SLOT_PIPELINE.md:51`), normalised commands in `[-1, 1]` at the write-back
  (`:214`). Whether `PitchSpd * P` is therefore a rate in the same units as `TurnRollSpd * cos θ` is
  not checked.
* **`w < 0` reachability.** The only caller passes `min(|bank| * 1.5, maxBank)`; `maxBank =
  min(plan+2E8h * class+25Ch TurnRoll, TurnRollLimit{Small,Large})` per
  `docs/PILOT_PLANNER_PITCH_ROLL.md` §1b. The limits are `DEG(85)`/`DEG(56)` > 0, so `w < 0`
  requires `plan+2E8h * TurnRoll < 0`. The sign of `plan+2E8h` was not established. The `w < 0`
  return path therefore exists in code but has no demonstrated reachable input.
* **The discontinuity at `w == 0`.** At exactly `w == 0` path 2 returns `-0.0f`; the consumer's
  `COMISS` against `0.0f` at `0099E0E7` does not set `CF` for `-0.0`, so `C` becomes `+0.01f`. For
  any small `w > 0` it becomes `-0.1f`. `C` therefore flips sign and changes magnitude tenfold as
  `w` crosses zero upward, and `raw = (h - s)/C` flips with it. This is read from the code, not
  judged; whether the planner ever presents `w == 0` was not checked.
* **`00CE3800 = 0.5f` on the null-unit path.** Why `0.5` and not a degenerate value is not
  explained by anything read here.
* Nothing in this document was compiled or run. It is **read**, not reconstructed: no `src/` or
  `include/bsp/` file implements `0099D0A0` (grepped for the address and for `hdg_diff` /
  `heading difference` / `rollout`).
