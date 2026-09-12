# The unit's timed sub-updates and the timers they own

Addresses: 00834820 00834A70 00834CC0 00834E90 00956600 008252C0 00439820 00866B70 00867B10
0078CF20 004842C0 00414DB0 0042D7E0 00B62D10 00822C20 008255B0 00CE3D30 00CE3D40

`docs/UNIT_INSTANCE_UPDATE.md` proposes a packet `unit_timers` for "the three timed sub-updates of
`008255B0` in full, including the descriptor fields each reads". **That packet is already done**:
`docs/UNIT_TIMED_SUBUPDATES.md` covers `008252C0`, `00956600` and `00834E90` end to end, and
`include/bsp/unit_motion.hpp` reconstructs all three. What it left open is the last line of its own
"What remains": the three further `__thiscall(this, float delta)` routines `00834E90` closes with.
This packet reads those three in full, and collects every timer the six routines own into one
table, because until now they were spread over three documents.

## The six routines and the one delta

`008255B0` step 11 calls three routines at `00825D4D..00825D83`, and the third of them calls three
more at `0083553E..0083556F`. All six are `__thiscall(this, float scaledDelta)` with `RET 4`, and
every one receives the same scaled frame delta, reloaded from the caller's frame each time
(`[ESP+8Ch]` in `008255B0`, `[ESP+120h]` in `00834E90`).

| order | site | address | body | ins | role |
| --- | --- | --- | --- | --- | --- |
| 1 | `00825D5A` | `008252C0` | `008252C0..0082544B` | 128 | engine-audio parameters |
| 2 | `00825D6C` | `00956600` | `00956600..00956B3E` | 372 | age, countdowns, damage scan, fade |
| 3 | `00825D7E` | `00834E90` | `00834E90..0083558E` | 437 | steering nodes and propellers |
| 3.1 | `0083554B` | `00834820` | `00834820..00834A6D` | 169 | the bow wave |
| 3.2 | `0083555D` | `00834CC0` | `00834CC0..00834E8F` | 149 | the stern wave |
| 3.3 | `0083556F` | `00834A70` | `00834A70..00834CB8` | 148 | the propeller spray |

## Every timer the six own

A "timer" here is a unit field the delta is added to or subtracted from. There are seven, and no
two of them follow the same rule.

| field | routine | rule | what crossing does |
| --- | --- | --- | --- |
| `+524h` | `00956600` at `00956626` | `+= delta`, unbounded, never reset here | nothing in the routine; it is the instance age |
| `+728h` | `00956600` at `0095662C` | `max(x - delta, 0)` while `x > 0` | nothing in the routine |
| `+6D8h` | `00956600` at `0095666F` | `-= delta`, allowed to go negative | below zero runs the damage scan and reloads `0.2f` (`00CE54A0`) |
| `+2F4h` | `00956600` at `009569F8` | chases `+2F8h` at `0.5`/s, asymmetric | drives the node visibility factor |
| `+BC4h` | `008252C0` at `00825362` | `0042AC60` rate limiter, step `settings[+8] * delta` | published as `"rpm"` |
| `+9F8h` | `00834820` at `00834996` | `0` while the water line crosses the probes, else `+= delta` | above `0.8f` stops the bow-wave effect |
| `+9FCh` | `00834CC0` at `00834E25` | the same rule | above `0.8f` stops the stern-wave effect |
| `+A10h` | `00834A70` at `00834AFB` | `0.6f` while under power, else `-= delta`, **once per live slot** | below zero stops that slot's spray effect |

`+1088h` and `+108Ch` in `00834E90` take the delta as a *step size* rather than accumulating it, so
they are rates, not timers; `docs/UNIT_TIMED_SUBUPDATES.md` has them.

## The five point effects a ship unit drives

`00822C20`, the ship instance's setup pass, creates all five and stores them in the unit. Each has
its own class template and its own gate, and each has exactly one consumer:

| unit slot | class template | created at | creator | consumer |
| --- | --- | --- | --- | --- |
| `+9E8h` | `+630h` `BowWave` | `008241A7` | `00868420` | `00834820` |
| `+9ECh` | `+634h` `WaveStern` | `008246D0` | `00868420` | `00834CC0` |
| `+9F0h`, `+9F4h` | `+5ACh` `BowParticle` | `00823FBA`, `0082406F` | `008687C0` | step 5 of `008255B0`, `00815AA0` |
| `+A00h`, four slots | `+65Ch` `RotorParticle` | `008248A3` | `00868420` | `00834A70` |

The gates are `[class+630h] != 0` (`008240D1`), `[class+634h] != 0` (`008245FA`),
`[class+65Ch] != 0` (`008247BB`) and, for the anchors, the template plus a non-zero bow matrix
translation (`docs/UNIT_WATER_ANCHORS.md`). The property names are the ship class reader's
(`docs/SHIP_CLASS_FIELDS.md`).

**The wave and spray effects have no parent.** `00868420` passes a null parent to the constructor
(`docs/POINT_EFFECT_CREATION_VARIANTS.md`) and the constructor stores its second stack argument at
`+8Ch` (`0086823F` reads `[ESP+6Ch]`, which the frame map fixes as `[entry+8h]`: `008680E7` reaches
the option byte at `[entry+18h]` from `[ESP+7Ch]` with the same `ESP`). So for these four,
`004842C0` stores the world point unchanged and `00867D00` skips its recombination; only the two
anchors, created with the unit's scene node as the parent, go through the inverse-world transform.
All four are also created with a stack identity matrix.

## `00834820`, the bow wave

Gate, `00834827..0083485A`: the handle at `+9E8h`, the game singleton at `00E188A8` and its ocean
at `+19F0h` must all be non-null. **Only this routine makes the last two tests**; `00834CC0` and
`00834A70` dereference the same two pointers with no test at all.

### The water line

```
0083486A  if (!this->byte_C8h) 00414DB0(this)              ; refresh the unit pose
00834882  P0 = 00439820(out, EDX = &this->f_1054h, this+CCh, 0)
00834892  if (!this->byte_C8h) 00414DB0(this)              ; again, before the second probe
008348A4  P1 = 00439820(out, EDX = &this->f_1060h, this+CCh, 0)
008348D3  h  = 0078CF20(ocean, P0.x, P0.z)                 ; sampled under P0 only
008348C0  d  = (double)(P1.y - P0.y)                       ; spilled to a double before the divide
008348E9  t  = (float)((h - P0.y) / d)                     ; no zero guard
0083493B  W  = P0 + (float)((float)(P1 - P0) * t)          ; per axis, each product and sum
                                                            ; rounded to a float
```

`00439820` is `float3* __thiscall(out = ECX, local = EDX, const Matrix4x4*, float* outW)`, `RET 8`,
body `00439820..004398D4`: it stages `(x, y, z, 1.0f)`, calls `00B62D10`
`BSP_Vector4f_Transform`, and divides the result by its `w`, writing `1/w` through the fourth
argument when it is non-null. Both probes pass null, so only the divided point is kept.

`+1054h` and `+1060h` are two unit-local float3s, copied out of the class block at `00823B87` and
`00823BB1` from `class+594h` and `class+5A0h`. `00834820` is the only code that takes the address
of either: a byte-pattern search for `8D ?? 54 10 00 00` and `8D ?? 60 10 00 00` returns exactly one
hit each in the whole image, `00834878` and `0083489A`. A component read through some other
instruction form was not excluded.

### The timer

```
0083495D  straddles = (W.y > P1.y && P0.y > W.y) || (P1.y > W.y && W.y > P0.y)   ; all strict
00834985  if (straddles)  this->f_9F8h = 0
00834996  else { this->f_9F8h = (float)(f_9F8h + delta);
008349BA         if (f_9F8h > 0.8f) { 00867B10(effect); return; } }
```

`00CE3D40` is the double `0x3FE99999A0000000`, float `0.8f` promoted. Every compare in the
straddle chain is `FCOMI`/`FCOMIP`, and every unordered compare leaves the chain on the accumulate
arm. That matters: two probes at the same height make `t` non-finite and `W.y` a NaN, so a flat
probe pair does not reset, and the effect is stopped 0.8 s later.

### The publish

```
008349C2  00866B70(effect)                                  ; clear the stop byte
008349CE  effect->f_54h = this->vtable[38h]()                ; a float in ST0; contract unread
008349DD  effect->f_58h = this->f_984h                       ; the steering source
008349F3  effect->f_50h = 1.0f / [this->p_538h + 500h]       ; 1 / MaxSpeed, no zero guard
008349F6  004842C0(effect, &W)
008349FD  m = 0042D7E0(this); effect->f_68h..70h = m[20h..28h]
00834A20  effect->f_2Ch = 0
00834A48  effect->f_74h = class+A4h; f_78h = class+A8h; f_7Ch = class+A0h
```

The last line is a rotation, not a copy: the base descriptor's `Width`, `Height` and `Length`
(`docs/VEHICLE_CLASS_FIELDS.md`, `+A4h`, `+A8h`, `+A0h`) land at `+74h`, `+78h` and `+7Ch` in that
order. `+50h` and `+54h` are the first two floats of `PointEffectInstanceStorage::fields_50`, which
the constructor leaves at `1.0f`.

`00866B70` is `void __thiscall(effect)`: under the effect manager's critical section it clears
`effect->byte_9h` and sets `effect->byte_Ah` when `effect->byte_8h` is set. `00867B10` (live Ghidra
symbol `BSP_PointEffect_StopChildren`) moves the effect's live rows into its own reference array
and releases them under the same lock.

## `00834CC0`, the stern wave

The same routine with three differences and no others:

1. the handle is `+9ECh` and the timer `+9FCh`;
2. the probes are read straight out of the class block, `class+638h` and `class+644h`
   (`00834D07`, `00834D29`), not out of per-instance copies;
3. the publish is only `effect->f_54h = this->vtable[38h]()`, `effect->f_50h = 1/MaxSpeed` and
   `004842C0`. No steering, no matrix row, no hull extent, and no `+2Ch` clear.

## `00834A70`, the propeller spray

```
00834A78  m = (this->f_980h > 0.0f) ? f_980h : (-0.0f - f_980h)     ; taken once, before the loop
00834AA2  for (i = 0; i < [this->p_538h + 654h]; ++i) {
00834AE8      e = this->p_A00h[i]; if (!e) continue;
00834AEE      this->f_A10h = (0.01f <= m) ? 0.6f : (f_A10h - delta)
00834B20      if (!(0.0f <= f_A10h)) { 00867B10(e); continue; }
00834B35      00866B70(e)
00834B45      if (!this->byte_C8h) 00414DB0(this)
00834B4A      P = transform([p_538h + 650h][i], this+CCh) / w          ; 00439820 inlined
00834BFF      h = 0078CF20(ocean, P.x, P.z)
00834C16      if (!((float)(h + 3.0) > P.y)) continue
00834C28      P.y = min(P.y, (float)((float)(h + 3.0) - 3.1f))
00834C7B      e->f_54h = clamp(m + m, 0.1f, 1.0f); e->f_50h = 1.0f
00834C85      004842C0(e, &P)
          }
```

Four things in that loop are worth stating plainly.

**The magnitude is taken by subtraction, not by masking.** `00834A96` computes `-0.0f - x` on the
not-greater arm of a strict `COMISS` against zero, so a NaN throttle also takes it.
`008252C0` clears the sign bit instead (`docs/UNIT_TIMED_SUBUPDATES.md`); the two routines disagree
on NaN and on nothing else.

**The timer is a hold, not a countdown.** While the magnitude reaches `0.01f` the field is
*reloaded* with `0.6f` (`00CE3D30`) every frame; only when the throttle falls below the epsilon
does it start falling. So the spray keeps running for 0.6 s after the engines stop.

**The timer is stepped inside the loop.** A unit with three live slots subtracts the frame delta
three times in one frame, so the spray fades roughly N times faster on a ship with N propellers.
This is what the listing does; it is not modelled as a bug and no run-time evidence was gathered
either way.

**The slot array and the loop bound do not come from the same place.** The producer always fills
exactly four slots (`MOV dword ptr [ESP+18h],4` at `008247D7`, `SUB ... ,1 / JNZ` at `00824900`),
while the loop walks `class+654h` entries of both `unit+A00h` and `class+650h`. A class declaring
more than four spray points would read `unit+A10h` (the timer) and beyond as effect pointers. Every
shipped ship row would have to be checked to say whether that can happen;
`reports/ship_class_fields.json` does not carry `+654h`, because the Lua reader does not write it.

The band test uses the doubles `00D7A2B0` (`3.0`) and `00D09DB0` (float `3.1f` promoted), so the
published height is pulled down to about a tenth of a metre under the surface, and points more
than three metres above the water publish nothing at all.

## Descriptor and class fields these routines read

`docs/UNIT_TIMED_SUBUPDATES.md` records `+354h` (the descriptor) and the class fields `+69Ch`,
`+6A0h`, `+6A4h`. The class block `unit+538h` fields this packet adds:

| offset | type | meaning | site |
| --- | --- | --- | --- |
| `+A0h`, `+A4h`, `+A8h` | float | `Length`, `Width`, `Height` from the base descriptor reader | `00834A3D`, `00834A2D`, `00834A35` |
| `+500h` | float | `MaxSpeed`; both wave routines publish its reciprocal | `008349E6`, `00834E5E` |
| `+594h`, `+5A0h` | float3 | the bow wave's two probe points, copied into the unit | `00823B81`, `00823BAB` |
| `+630h` | ptr | `BowWave` template | `008240D1` |
| `+634h` | ptr | `WaveStern` template | `008245FA` |
| `+638h`, `+644h` | float3 | the stern wave's two probe points, read in place | `00834D07`, `00834D29` |
| `+650h` | ptr | spray point array, stride `0x0C` | `00834B4A` |
| `+654h` | int | spray point count, and the loop bound | `00834AA2` |
| `+65Ch` | ptr | `RotorParticle` template | `008247BB` |

Unit fields this packet adds:

| offset | type | meaning | site |
| --- | --- | --- | --- |
| `+9E8h` | ptr | bow-wave effect | `00834827` |
| `+9ECh` | ptr | stern-wave effect | `00834CC8` |
| `+9F8h` | float | bow-wave timer | `00834988` |
| `+9FCh` | float | stern-wave timer | `00834E17` |
| `+A00h` | ptr[4] | spray effects | `00834AB8` |
| `+A10h` | float | the shared spray timer | `00834AFB` |
| `+1054h`, `+1060h` | float3 | the bow wave's probe points, copies of `class+594h` and `+5A0h` | `00834878`, `0083489A` |

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/UNIT_INSTANCE_UPDATE.md` proposes `unit_timers` as open work | `008252C0`, `00956600` and `00834E90` were finished by `docs/UNIT_TIMED_SUBUPDATES.md`; what was open is the three routines `00834E90` tails into | the three ledger names and `include/bsp/unit_motion.hpp` |
| `docs/UNIT_TIMED_SUBUPDATES.md`: the tail three "share the helper set `00414DB0 / 004842C0 / 0078CF20 / 00866B70 / 00867B10`" | correct, and `00439820` and `00B62D10` belong in that set: the two wave routines call `00439820` and the spray routine inlines it | `00834882`, `00834D11`, `00834B91` |
| `docs/UNIT_TIMED_SUBUPDATES.md` calls the tail three "three more sub-updates in order `00834820`, `00834CC0`, `00834A70`" | the order is right; the three are not peers of the first three, they are a second level, and only one of them tests the game singleton | `0083554B`, `0083555D`, `0083556F`; the gate at `00834846`, and its absence from the heads `00834CC0..00834CED` and `00834A70..00834AA2` |

No claim in either earlier document was found to be wrong.

## Coverage

| routine | body | coverage |
| --- | --- | --- |
| `00834820` | `00834820..00834A6D` | complete |
| `00834CC0` | `00834CC0..00834E8F` | complete |
| `00834A70` | `00834A70..00834CB8` | complete |
| `00439820` | `00439820..004398D4` | complete |
| `00866B70` | `00866B70..00866BAA` | partial: the two byte writes and the lock bracket; the manager fetch `00866440` is an external contract |
| `00867B10` | `00867B10..00867BFE` | partial: read only far enough to establish that it detaches and releases the effect's rows |
| `008252C0`, `00956600`, `00834E90` | see the table above | complete, in `docs/UNIT_TIMED_SUBUPDATES.md` and `include/bsp/unit_motion.hpp`; this document only collects their timers |
| `00822C20` | `00822C20..00824B57` | partial: `00823B7B..00823BCF` and `008240CB..0082490B`, the five effect creations and the probe copies |

## Uncertainties

1. `this->vtable[38h]`, called at `008349CE` and `00834E59`, was **not** read. It returns a float
   that both wave routines store next to `1/MaxSpeed`, which makes a speed the obvious hypothesis,
   but the slot is virtual and no ship-instance vtable was resolved. The host method keeps its
   address as its name.
2. The `+A10h` decrement inside the loop and the four-versus-`+654h` bound mismatch are both read
   straight from the listing. Whether either is reachable in a shipped mission needs the class rows
   and a run, neither of which this packet has.
3. `class+594h`, `+5A0h`, `+638h`, `+644h`, `+650h` and `+654h` have no producer in the ship Lua
   reader `00831840`; where they come from is the follow-up below.
4. `00834820` publishes `m[20h..28h]` of the unit's world matrix into the effect. That row is the
   third of the four; which axis it is depends on the matrix convention, which this packet did not
   re-derive.
5. The effect fields `+2Ch`, `+58h`, `+68h..+70h` and `+74h..+7Ch` are written here and read
   somewhere inside the effect system. No reader was looked for.

## no_ghidra_function

| start | inclusive end | evidence |
| --- | --- | --- |
| none | | All six routines and every helper cited have Ghidra functions: `00834820..00834A6D`, `00834A70..00834CB8`, `00834CC0..00834E8F`, `00439820..004398D4`, `00866B70..00866BAA`, `00867B10..00867BFE`, `00822C20..00824B57`. `bsp.py ghidra flow` reports 0 gaps for `00834820` and `00834CC0`. It reports one 11-byte gap for `00834A70` at `00834AC5..00834ACF`: the bytes are `EB 09` (a jump to the loop head at `00834AD0`) followed by a 7-byte and a 2-byte NOP, and no instruction in the body reaches them. That is alignment fill after the `JMP 00834AD6` at `00834AC3`, not a missing path. |

## Follow-up packets

| id | addresses | files | contract |
| --- | --- | --- | --- |
| `unit_speed_virtual` | `008255B0`'s class vtable `+38h`, `0080FC30`, `0092D730` | `docs/UNIT_SPEED_VIRTUAL.md`, `reports/unit_speed_virtual.json` | Resolve the unit vtable slot `+38h` for the ship class and settle whether the wave effects are fed a speed. Two other float sources for the same quantity already have names. |
| `ship_class_anchor_matrices` | `0082FE30`, `00830195`, `008302B9` | `docs/SHIP_CLASS_ANCHOR_MATRICES.md`, `reports/ship_class_anchor_matrices.json` | The producer of `class+594h`, `+5A0h`, `+5B0h`, `+5F0h`, `+638h`, `+644h`, `+650h`, `+654h`, none of which the Lua reader writes. Shared with `docs/UNIT_WATER_ANCHORS.md`. |
| `ship_effect_setup_pass` | `00822C20` | `docs/SHIP_EFFECT_SETUP_PASS.md`, `reports/ship_effect_setup_pass.json` | The rest of `00822C20`: what else the ship's setup pass builds, and whether `class+654h` can exceed the four slots it allocates. |
| `unit_damage_table` | `00923BE0`, `0049C940`, `00440490`, `00440A30` | as proposed in `docs/UNIT_TIMED_SUBUPDATES.md` | Unchanged; the `+6D8h` scan tick in the table above is its entry point. |

## State reached

| routine | state |
| --- | --- |
| `00834820` | reconstructed, build-tested, one focused fixture case |
| `00834CC0` | reconstructed, build-tested |
| `00834A70` | reconstructed, build-tested |
| `00439820` | analysed; its rule lives in the host method, not in a separate reconstruction |
| `00866B70`, `00867B10` | analysed as host contracts only |

Nothing here is ABI-compatible or game-validated. `include/bsp/unit_timers.hpp` and
`src/unit_timers.cpp` expose new C++ interfaces; they are not drop-in binary replacements.
