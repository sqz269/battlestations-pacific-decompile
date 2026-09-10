# The three timed sub-updates of the vehicle base update

Addresses: 008252c0 00956600 00834e90 0042ac60 0080fc30 0092d730 00923be0 00415690 008255b0
00424c40 00467050 00b64780 00b6e0a0 00413920 00b6da70 004bca50 00834820 00834a70 00834cc0

Step 11 of `008255B0` (`docs/UNIT_INSTANCE_UPDATE.md`) calls three routines back to back at
`00825D4D..00825D83`. All three are `__thiscall(this, float scaledDelta)` with `RET 4` and each
gets the same delta reloaded from `[ESP+8Ch]`:

| order | address | body | ins | calls | role |
|---|---|---|---|---|---|
| 1 | `008252C0` | `008252C0..0082544B` | 128 | 4 | engine-audio parameters |
| 2 | `00956600` | `00956600..00956B3E` | 372 | 24 | age, countdowns, damage scan, fade |
| 3 | `00834E90` | `00834E90..0083558E` | 437 | 18 | steering nodes and propellers |

## Where the position integration is: not here

**None of the three integrates a position, a heading or a speed.** There is no ocean sample, no
terrain sample and no write to the scene node's translation anywhere in the three bodies. What
they do instead is *read* the motion state out of the physics body and drive presentation from it.

The read is `0092D730`, `float __thiscall(controller)`, `RET 0`, body `0092D730..0092D76E`:

```
body = controller->p_2Ch;                  ; the physics body
axis = 00C32000(body);                     ; DYN_physics, returns a transform-like block
vel  = 00C31F40(body, &scratch);           ; DYN_physics, returns the linear velocity vec3
return (vel[1]*axis[1Ch] + vel[0]*axis[18h]) + vel[2]*axis[20h];
```

`axis+18h..+20h` is the third 12-byte row of the block `00C32000` returns, so the result is the
signed speed along one body axis. The whole sum stays on the x87 stack and is rounded once. The
controller at `+1018h` and the body at `[+1018h]+2Ch` are the physics contract; `00C31F40` and
`00C32000` are outside this packet and are treated as external.

So the unit's transform is owned by the physics/controller side reached through `+1018h`, and
`008255B0`'s own steps only read the scene node back (step 1 refreshes its world matrix). Whatever
integrates position and heading lives behind `0092BE80` (step 6, the controller sub-update) or in
the physics library itself; it is **not** in step 11. That is the packet's main negative result.

## `0042AC60`, the rate limiter every one of them uses

`__thiscall(float* state, float target, float maxStep)`, `RET 8`, body `0042AC60..0042ACB7`.

```
cur = *state;
if (maxStep > fabsf(cur - target)) { *state = target; return; }   ; FCOMI 0042AC8A, strict
*state = (target > cur) ? cur + maxStep : cur - maxStep;          ; FCOMIP 0042ACA6
```

The magnitude comes from storing the difference to a float and clearing its sign bit with
`AND 7FFFFFFFh`, not from `fabs` on the x87 stack. The distance test is strict, so a step exactly
equal to the distance takes the stepping arm, not the snap. An unordered compare (a NaN operand)
falls to the stepping arm as well, because `JBE` tests `CF|ZF` and both are set on unordered.
16 callers; `00834E90` inlines the identical shape at `00835232..00835285`.

`00415690`, `__fastcall(float* v, const float* low, const float* high)`, `RET 4`, is the matching
clamp: raise to `low` first, then lower to `high`, both tests strict.

## `008252C0`, the engine-audio parameter update

Gate: the byte at `+9C4h` (`008252CA`). `00424C40` (the settings singleton) is called first,
before the gate.

Category select, `008252DE..0082530B`, through the instance `IsKindOf` at vtable `+5Ch`:

| test | settings block | site |
|---|---|---|
| `IsKindOf(0Eh)` | `settings+5E0h` | `008252E8` |
| else `IsKindOf(8)` | `settings+604h` | `008252FD` |
| else | `settings+5BCh` | `00825305` |

The blocks are 0x24 apart and only `+8` inside one is read here: the smoothing rate.

```
step   = block[8] * delta;                     ; FLD/FMUL 0082530B
target = this->byte_5Dh ? 0.0f : fabsf(this->f_980h);   ; FLDZ 0082532D / AND 0082534F
0042AC60(&this->f_BC4h, target, step);         ; 00825362
```

`+980h` is therefore a signed throttle/RPM source and `+BC4h` its smoothed presentation copy.
The smoothed value is then pushed into up to three emitters through their vtable `+1Ch`:

| slot | parameter | site |
|---|---|---|
| `+BBCh` | `"rpm"` (`00D08688`) | `0082538B` |
| `+BB8h` | `"rpm"` | `008253B0` |
| `+BB4h` | `"param00"` (`00D09998`) | `0082541E` |

The `"param00"` value is a different quantity, `008253C6..0082540C`:

```
if (this->byte_5Dh) v = 0.0f;
else {
    a = 0080FC30(this);                        ; reference speed
    b = 0092D730(this->p_1018h);               ; forward speed
    v = (b - 0.0f) * (1.0f / (a - 0.0f)) + 0.0f;
}
```

That is an inlined `remap(b, 0, a, 0, 1)` with the constants folded, i.e. `b / a` with the
reciprocal taken first and **no clamp and no zero guard**. `0080FC30`, `float __thiscall(this)`,
`RET 0`:

```
if (DAT_00E0C978 != 0 && [DAT_00F88C30 + 0B8h] != 0) s = 008E6430(4, this);
else                                                 s = 1.0f;       ; 00D7A24C
return this->f_9C0h * s;
```

so `+9C0h` is the instance's base reference speed and `008E6430(4, ...)` a gameplay scale. Finally
`0082541E..00825444`: when `+BC0h` is set **and** the `+5Dh` gate is set, its vtable `+0Ch` is
called with `0.0f` — the stop/fade of a fourth audio object.

The `!= 0` tests in this function are the compiler's `NEG/SBB/TEST imm32` idiom
(`TEST EDX,0E19AB0h`), which the decompiler renders as `(-(uint)(p != 0) & 0xe19ab0) != 0`. It is a
plain null test.

## `00956600`, the scalar timers

Head, `00956618..00956690`, the clearest statement of how the delta is consumed:

```
this->f_524h += delta;                                ; unbounded accumulator, never reset here
if (this->f_728h > 0.0f)                              ; COMISS, strict
    this->f_728h = max(this->f_728h - delta, 0.0f);
this->f_6D8h -= delta;                                ; unconditional, allowed to go negative
if (this->f_6D8h < 0.0f && this->p_354h && damageTable non-empty) { ... }
```

### What `+6D8h` gates: the damage-threshold scan

Not a reload and not an AI re-plan. `+6D8h` is a **0.2 s scan tick** (`00CE54A0 = 0.2f`, written
back at `009566BD`) over the descriptor's damage table:

```
count  = ([p_354h+10h] - [p_354h+0Ch]) >> 4;      ; 16-byte records
anchorCount = [p_354h+2Ch];
hp = 00923BE0(this);
if (hp < this->f_364h) {                          ; only on a frame where health fell
    for (i = 0; i < count; ++i) {
        rec = [p_354h+0Ch] + i*10h;
        if (!(hp <= rec[0] && rec[0] < this->f_364h)) continue;
        if (0 <= rec[8] && rec[8] < anchorCount)
             transform ([p_354h+28h] + rec[8]*0Ch) through 0042D7E0 / 00414D10;
        else if (!this->byte_0C8h) 00414DB0(this);
        if (0 <= rec[4]) { this->vtable[34h](); 0049C940(); }
        if (rec[0Ch])    { 00440490 / 008689C0 / 004845D0 / 00440A30 }
    }
    this->f_364h = hp;
}
```

Record layout, 16 bytes, from `0095674C`, `009567A3`, `009567B0` and `00956874`:

| offset | type | meaning |
|---|---|---|
| `+0h` | float | health threshold |
| `+4h` | int | announce id; `>= 0` fires the instance vtable `+34h` then `0049C940` |
| `+8h` | int | index into the descriptor's float3 anchor array at `+28h`, bounded by `+2Ch` |
| `+0Ch` | ptr | ref-counted effect template attached at the anchor |

`00923BE0`, `float __thiscall(this)`, `RET 0`, is the health read: `0.0f` while `+5Dh` is set,
otherwise the virtual at vtable `+110h` floored at zero, with the floor also written back to
`+164h`. `+364h` is the previous health the scan compares against, so each record fires exactly
once as health passes down through its threshold.

### What `+728h` and `+524h` gate

Nothing inside this routine reads them; they are written here and consumed elsewhere. `+524h` is
the instance age, `+728h` a clamped countdown. `docs/UNIT_INSTANCE_UPDATE.md` already records the
same two writes; the reconstruction reuses `unit_advance_age_00956626`,
`unit_advance_clamped_countdown_0095662c` and `unit_advance_countdown_0095666f` from
`include/bsp/unit_instance.hpp` rather than restating them.

### The fade at `+2F4h` and the node visibility

`009569F8..00956A9D` moves `+2F4h` toward `+2F8h` at 0.5 per second (`00D7A280`), asymmetrically:

```
if (tgt > cur) { cur += delta*0.5f; if (cur < 0) cur = 0; else if (cur > tgt) cur = tgt; }
else if (cur > tgt) { cur -= delta*0.5f; 00415690(&cur, &tgt, 1.0f); }
; equal: untouched
```

`+2F4h` is the same field `006FF270` returns as the per-part intensity scalar
(`docs/UNIT_INSTANCE_UPDATE.md`), so this is the unit's fade-in/fade-out. The tail,
`00956AA3..00956B25`, pushes it to the scene node at `+4A4h` through `00B6DA70`
(`BSP_SceneNode_SetVisibilityFactor`):

```
mode = 004BCA50(DAT_00E188A8);                 ; BSP_Game_GetEffectiveGameMode
if (mode == 9)
    v = (DAT_00F87152 || this->byte_2F0h) ? 1.0f : (this->f_2F4h > 0.5f ? 1.0f : 0.0f);
else
    v = [DAT_00E188A8+61Dh]                   ? 1.0f
      : (DAT_00F87152 || this->byte_2F0h)     ? 1.0f
      : this->f_2F4h;
00B6DA70(this->p_4A4h, v, 0);
```

The threshold `0.5f` is `00CE3800`, already in `include/bsp/unit_instance.hpp` as
`kUnitIntensityControlled`. Game mode 9 gets a hard on/off instead of the continuous fade.

### The animation chain at `+70Ch`

`00956946..009569F6`, between the damage scan and the fade:

```
if (this->byte_70Ch) {
    owner = [[this->p_360h + 160h] + 0Ch];
    anim  = [owner + 130h];
    if (anim && [[anim+2Ch]+10h]) {
        extra = this->p_714h ? 00B78670(&this->f_714h) : 0;   ; ECX = [[owner+130h]+2Ch]
        00B6DC90(owner, 0);                                   ; CG_adjustor_thunk, result unused
        r = 00BF857A(this->f_710h subtracted from DAT_00F876A4, DAT_00F876A4);
        owner->vtable[24h](extra, r, 1);
    }
}
```

`DAT_00F876A4` is the mission clock the base update also reads. `00BF857A` is an unmatched CRT x87
helper taking the two values on the stack; `fmod` is the likely identity but is **not** confirmed,
so the loop this drives stays provisional. The scene/animation object is an external contract and
was not followed further.

## `00834E90`, the steering nodes and the propellers

MSVC SEH frame (`00C9250B`). The first thing it does is call `0092D730` on the controller and
**discard the result** (`FSTP ST0` at `00834EBC`) — the call is kept for its side effect, the
clamp-and-cache inside `00923BE0`-style accessors on the controller.

### Two node groups

| field | count | stride | meaning |
|---|---|---|---|
| `+106Ch` | 4 | 4 | propeller scene nodes |
| `+107Ch` | 3 | 4 | steering scene nodes |
| `+1088h` | 1 | — | the shared steering angle |
| `+108Ch` | 4 | 4 | per-propeller angular rate |
| `+B44h` | 4 | 4 | per-propeller cavitation effect handle |

`+61h` swaps both scalar inputs: throttle is `+980h` or `+FC4h` and steering is `+984h` or `+FDCh`
(`00834EBE`, `008350BD`, `008350DE`).

### The steering angle, `00834ED7..00834F12`

```
s = (byte_61h ? f_FDCh : f_984h);
target = (s - (-1.0)) - 0.78539818525314331 * (s - (-1.0));
0042AC60(&this->f_1088h, target, delta);        ; the step IS the frame delta
```

`00D7A250` is the double `-1.0` and `00CEDCD0` is the double `0.78539818525314331`, which is
float `pi/4` promoted. The expression is literally `(s + 1) * (1 - pi/4)`, i.e. `0 .. 0.4292 rad`
for `s` in `[-1, 1]`. **Provisional:** that one-sided range does not read like a rudder deflection,
so either `s` is not a signed `[-1, 1]` command or the node's rest pose carries the other half. The
formula itself is exact; the interpretation is not established.

Each of the three nodes then gets its own local position from `00B6E0A0(node, &out)` written into
the translation row of a stack identity 4x4, `00467050(&m, 0.0f, f_1088h, 0.0f)` for the rotation,
and `node->vtable[38h](&m)`. `00467050` is an Euler builder (`FSIN`/`FCOS` on three arguments at
`00467055..0046708A`), so the shared angle is the **second** of the three Euler angles.

### The propeller rate, `008350BD..00835285`

```
R = byte_61h ? f_FC4h : f_980h;                          ; throttle
T = byte_61h ? f_FDCh : f_984h;                          ; steering
W = [p_538h+6A0h] * f_1030h * 1.2 + [p_538h+69Ch];       ; 00CEC160 = 1.2
V = clamp(R * 4.0, -1.0f, 1.0f);                         ; 00D7A328 = 4.0, 00D7A260 = -1.0f
B = (1.0f - fabsf(R)) * V * W * T;                       ; 00835159..00835175

for i in 0..3:
    if (!node[i]) continue;
    starboard = (00B6E0A0(node[i]).x > 0.0f);             ; 0083519D
    wasActive = (0.01f <= fabsf(rate[i]));                ; 00D7A238 = 0.01f
    side      = starboard ? B : (-0.0f - B);              ; 00D7A208 = -0.0f
    target    = [p_538h+6A0h] * R + side;                 ; 00835220
    rate[i]   = stepTowards(rate[i], target, delta * 5.0);; 00D7A370 = 5.0
    isActive  = (0.01f <= fabsf(rate[i]));
    ...effect edge...
    angle     = -( (2*(i&1) - 1) * delta * rate[i] );     ; 0083541F..0083544C
    m         = 00B64780(&angle);                         ; one-axis rotation
    node[i]->vtable[38h]( 00413920(left = m, right = 00B6DB60(node[i])) );
```

So `+108Ch` holds an angular **rate**, not an angle: the node's local matrix is left-multiplied by
an incremental rotation every frame (`BSP_Matrix_Multiply4x4`: ECX is the left operand, the first
stack argument the destination, the second the right operand — `docs/SHADER_CAMERA_CONSTANT_ANALYSIS.md`).
`2*(i&1) - 1` gives even indices one handedness and odd indices the other, which is counter-rotating
screws, and the whole product is rounded to a float before `FCHS` negates it.

The cavitation effect is a plain edge on the same 0.01 threshold, sampled before and after the slew:

* `!wasActive && isActive && [p_538h+6A4h]`: ref-count `[p_538h+6A4h]` up (`00CE221C`), build a
  stack identity 4x4, `008687C0`, then `004845D0` into `+B44h + i*4` (`008352BD..008353C7`).
* `wasActive && !isActive`: `00867B10(handle)`, `handle->byte_9 = 1`, then the `00CE2220` release
  and the slot is zeroed twice (`008353C9..00835415`).

### The tail

`0083553E..0083556F` calls three more `__thiscall(this, delta)` sub-updates in order: `00834820`,
`00834CC0`, `00834A70`. All three sit in the same segment and share the helper set
`00414DB0 / 004842C0 / 0078CF20 / 00866B70 / 00867B10`, i.e. pose refresh, anchor publish, ocean
sample and effect handling. They were not analyzed and are proposed as a follow-up packet.

## Class-block and descriptor fields this packet establishes

`+538h`, the class block (`docs/VEHICLE_CLASS_DESCRIPTORS.md` covers the `+354h` descriptor):

| offset | type | meaning | site |
|---|---|---|---|
| `+69Ch` | float | propeller idle term | `00835116` |
| `+6A0h` | float | propeller gain, used against both the load and the throttle | `008350F8`, `00835220` |
| `+6A4h` | ptr | cavitation effect template | `008352C3` |

`+354h`, the descriptor:

| offset | type | meaning | site |
|---|---|---|---|
| `+0Ch`,`+10h` | ptr | damage-record vector, begin and end, stride 0x10 | `009566A4` |
| `+28h` | ptr | float3 anchor array, stride 0x0C | `009567B6` |
| `+2Ch` | int | anchor count | `009566E8` |

## Instance fields this packet adds to the layout

| offset | type | meaning | site |
|---|---|---|---|
| `+164h` | float | health cache the floor in `00923BE0` writes | `00923C16` |
| `+2F8h` | float | fade target `+2F4h` chases | `00956A0E` |
| `+360h` | ptr | animation owner chain root | `00956952` |
| `+364h` | float | previous health the damage scan compares against | `009566FE` |
| `+70Ch` | byte | animation-chain gate | `00956946` |
| `+710h` | float | animation clock base subtracted from `DAT_00F876A4` | `009569CB` |
| `+714h` | ptr | optional `00B78670` pre-step | `0095697B` |
| `+980h` | float | throttle / RPM source | `00825331`, `008350CE` |
| `+984h` | float | steering source | `00834ECF`, `008350EA` |
| `+9C0h` | float | base reference speed | `0080FC58` |
| `+9C4h` | byte | engine-audio gate | `008252CA` |
| `+B44h` | ptr[4] | cavitation effect handles | `008353CD` |
| `+BB4h` | ptr | `"param00"` emitter | `008253B2` |
| `+BB8h`,`+BBCh` | ptr | `"rpm"` emitters | `0082538D`, `00825367` |
| `+BC0h` | ptr | audio object stopped while `+5Dh` is set | `00825420` |
| `+BC4h` | float | smoothed RPM | `00825319` |
| `+FC4h` | float | alternate throttle under `+61h` | `008350C4` |
| `+FDCh` | float | alternate steering under `+61h` | `00834EC5` |
| `+1030h` | float | propeller load | `00835104` |
| `+106Ch` | ptr[4] | propeller nodes | `00835179` |
| `+107Ch` | ptr[3] | steering nodes | `00834F22` |
| `+1088h` | float | shared steering angle | `00834EEF` |
| `+108Ch` | float[4] | propeller angular rates | `00835161` |

## Callers and callees

Callers: `008255B0` only, for all three.

Callees analyzed here: `0042AC60`, `0080FC30`, `0092D730`, `00923BE0`, `00415690`.
Treated as external contracts: `00424C40` (settings singleton), `008E6430` (gameplay scale),
`00C31F40`/`00C32000` (physics), `00467050`/`00B64780`/`00B6E0A0`/`00B6DB60`/`00413920`/`00B6DA70`
(scene graph and matrices), `008687C0`/`00867B10`/`008689C0`/`004845D0`/`00440490`/`00440A30`
(effects and ref-counted handles), `004BCA50` (game mode), `0042D7E0`/`00414D10`/`00414DB0` (pose),
`00B78670`/`00B6DC90` (animation), `0049C940`, `00834820`/`00834A70`/`00834CC0`.

`bsp.py ghidra flow` reports no fall-through gap in any of the three.

## Uncertainties

1. The steering-angle formula `(s + 1) * (1 - pi/4)` is exact but its one-sided range is not
   explained. Whether the three `+107Ch` nodes are rudders, hydroplanes or control surfaces is
   **not** established; the name records them as steering nodes.
2. `00467050`'s three arguments are Euler angles in an unverified axis order, so "the second angle"
   is as far as this packet goes.
3. `+1030h` (propeller load) is written somewhere outside these three routines; its producer was
   not found.
4. `008E6430(4, this)` is called with a literal selector `4`; what the table it indexes contains is
   not established, only that the fallback is `1.0f`.
5. The discarded `0092D730` result at `00834EBC` is assumed to be kept for a side effect on the
   controller. That side effect was not confirmed.
6. `+164h` is only shown to receive the zero from the health floor; whether anything else writes it
   is unknown.

## What remains

* `00834820`, `00834CC0`, `00834A70`, the three sub-updates that close `00834E90`.
* `0092BE80`, the controller sub-update of step 6, which is where the position integration most
  likely lives.
* The `+70Ch` animation chain and the object at `[+360h]+160h`.

## Follow-up packets proposed

| id | addresses | files | contract |
|---|---|---|---|
| `unit_prop_tail_subupdates` | `00834820` `00834cc0` `00834a70` | `docs/UNIT_PROP_TAIL_SUBUPDATES.md`, `reports/unit_prop_tail_subupdates.json`, `include/bsp/unit_prop_tail.hpp`, `src/unit_prop_tail.cpp` | The three `__thiscall(this, delta)` routines `00834E90` ends with, and the shared helper set `00414DB0 / 004842C0 / 0078CF20 / 00866B70 / 00867B10` they use. |
| `unit_controller_motion` | `0092be80` `0092d730` `00c31f40` `00c32000` | `docs/UNIT_CONTROLLER_MOTION.md`, `reports/unit_controller_motion.json` | Step 6's controller sub-update and the physics body it drives: where heading and position are actually integrated, and what `+1018h`'s object holds. |
| `unit_damage_table` | `00923be0` `0049c940` `00440490` `00440a30` | `docs/UNIT_DAMAGE_TABLE.md`, `reports/unit_damage_table.json` | The descriptor's 16-byte damage records end to end: who authors them, what the vtable `+34h` announce does, and the `+164h` health cache. |

## State reached

| routine | state |
|---|---|
| `008252C0` | reconstructed, build-tested |
| `00956600` | reconstructed, build-tested |
| `00834E90` | reconstructed, build-tested |
| `0042AC60` | reconstructed, build-tested, fixture-tested |
| `00415690` | reconstructed, build-tested |
| `0080FC30` | reconstructed, build-tested |
| `0092D730` | reconstructed, build-tested |
| `00923BE0` | reconstructed, build-tested |
| `00834820`, `00834CC0`, `00834A70` | exported only |

Nothing here is ABI-compatible or game-validated. The C++ in `include/bsp/unit_motion.hpp` and
`src/unit_motion.cpp` exposes new interfaces; it is not a drop-in binary replacement.
