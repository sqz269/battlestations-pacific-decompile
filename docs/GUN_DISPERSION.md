# Gun dispersion: the two producers of miss

Addresses: `00730160`, `00BD2F10`, `00BD2ED0`, `00BD2E60`, `00BA2C20`, `00412E20`,
`007313E0`, `007327B0`, `0072F830`, `0072BF10`, `006FDD70`, `0085C3F0`, `00521E70`,
`00470440`, `0083B5E0`, `006DEFF0`, `006DF520`, `006DF1F0`, `006DEE00`, `008FB4E0`,
`008FD370`, `00901610`, `00900AF0`, `00438AA0`, `00419010`, `0085ABA0`.

Packet `cc7_gun_dispersion`. Ghidra was read-only: no rename, comment, prototype or save.

## 0. The correction the packet opens with

The packet brief placed the per-pellet perturbation in `0072F830`
(`BSP_Gun_SpawnShotAndEffects`) and said it reads the weapon class field `Throw`.
It does not. `0072F830` contains exactly one call to the RNG, at `0072FB6A`, and
that draw is an effect timer written to `gun[+3F0h]+6F8h`; the loop around
`00730042` is a **deterministic** ring (section 5). The `Throw` cone lives one
frame up, in `BSP_Gun_Fire` `00730160`, which `0072F830`'s ledger note already
names as its caller. Everything below reads `00730160`.

## 1. `BSP_Gun_Fire 00730160` — ABI and frame

`__thiscall(gun, int useExplicitThrow, float throwA, float throwB)`.
The single epilogue is `00730A14 POP EBP` / `00730A15 ADD ESP,0FCh` /
`00730A1B RET 0Ch`: **three** stack arguments, confirmed by the cleanup, not by
the decompiler. Ghidra's body is `00730160-00730A1D`. (`docs/UNIT_WEAPON_DEVICES.md`
records `00730160` as `no_ghidra_function`; that is stale — a function object
exists now and `bsp.py ghidra proto` returns it.)

The prologue is `SUB ESP,0FCh`, `PUSH EBP` (`00730166`), then `PUSH ESI` /
`PUSH EDI` at `007301BC`/`007301C7`. Call that frame **A** (entry-108h). All
`[ESP+...]` slots quoted below are in frame A, and the arguments land at
`[ESP+10Ch]` (`useExplicitThrow`), `[ESP+110h]` (`throwA`) and `[ESP+114h]`
(`throwB`) — exactly the slots `00730532`, `007305E4` and `007305F5` read.
`007302F9 POP EBX` is the shift that turns the pre-`PUSH EBX` slot `[ESP+10h]`
at `007302F3` into the `[ESP+0Ch]` the throw magnitude lives in afterwards.

Frame A also holds a copy of the barrel node's world matrix. `007301D1`..`007301E0`
`REP MOVSD`s `10h` dwords from `gun[+3CCh]+0F0h` into `[ESP+48h]`, so

| frame A slot | matrix offset | use |
| --- | --- | --- |
| `[ESP+48h]`..`[ESP+50h]` | `+00h`..`+08h` | row 0, the axis multiplied by `sin` |
| `[ESP+58h]`..`[ESP+60h]` | `+10h`..`+18h` | row 1, the axis multiplied by `cos` |
| `[ESP+2Ch]`..`[ESP+34h]` | `+20h`..`+28h` | row 2, the direction the shot leaves on |

The row-2 copy is made at `0073022A`, `0073023C` and `00730249`; its three
destination stores at `00730236` (frame A), `00730243` (after `PUSH EBX`) and
`00730253` (after `PUSH 24h`) resolve to the contiguous `A+2Ch`, `A+30h`, `A+34h`
once the two pushes are unwound. That is the register-provenance rule applied to
`ESP`: the same literal displacement means three different slots here.

## 2. `Throw`, and where it is produced

`007302DF MOVSS XMM0,[EAX+4]` with `EAX = gun[+3F8h]`. The producer of that
record is **`007313E0`**, called once per array element from
`BSP_GunClass_ReadLuaFields 007327B0` at `00732CAE` with
`ECX = [gunclass+74h] + 48h*index` (`LEA EDX,[EBP+EBP*8]` then `[EAX+EDX*8]`),
so the gun class owns a **48h-stride array at `+74h` with its count at `+78h`**
and `gun[+3F8h]` is the element in use.

`007313E0` writes only `+0h`..`+34h`. The Lua keys it binds:

| offset | Lua key | kind |
| --- | --- | --- |
| `+4h` | **`Throw`** | float, the cone half-angle in radians |
| `+8h` | (the name object) | ptr |
| `+0Ch` | `FireEfx` | ptr |
| `+10h` | `ContFireEfx` | ptr |
| `+14h` | `FireRumble` | ptr |
| `+18h` | `ContFireRumble` | ptr |
| `+1Ch` | `PlayerCameraShake` | float |
| `+20h` | (integer key at `00CFE7CC`) | int |
| `+24h` | `TracerRatio` | int, then re-read as a number |
| `+28h`/`+2Ch` | `ReloadTime[1]`, `ReloadTime[2]` | float pair |
| `+30h` | `BarrelDelayTime` | float |
| `+34h` | `Bullet` | the bullet class |

`+0Ch`/`+10h` and `+34h` independently corroborate the binding: `0072F830`'s
ledger note already says it builds the muzzle effects from `gun[+3F8h]+0Ch/+10h`,
and both `0072F830` and `00730160` read the bullet sub-type as
`[[gun[+3F8h]+34h]+8h]`. `Throw` is therefore `+4h` of that record, `0.01` on the
133 mm turret, i.e. 0.573°.

## 3. The magnitude chain, `0073031D`..`0073049C`

The value in `[ESP+0Ch]` is walked through three gates before it is drawn from.

| site | rule |
| --- | --- |
| `007302EA`..`00730317` | when the weapon `Function` `gunclass[+80h]` is 1, 5 or 6 **and** the settings byte at `00424C40()+760h` is set, the magnitude is zeroed outright (`XORPS`/`MOVSS`) |
| `0073031D`..`00730491` | a sub-type dispatch picks one of six seat pointers; the magnitude is multiplied by that seat's `vtable[24h]()` |
| `00730480` | the default arm additionally requires role 3 to be AI-held |

The settings byte is authored: `BSP_GameSettings_LoadFromLuaGlobals 0083B5E0`
fetches the Lua global name at `00D0A240`, the literal string
**`TurnOffAAGunThrow`**, with `00B67800` at `00841B4B`, reads it as a boolean
with `00B662F0` at `00841B5C` (default `0`) and stores `AL` to `settings+760h` at
`00841B68`.

The seat dispatch keys on the bullet sub-type `[[gun[+3F8h]+34h]+8h]`:

| sub-type | role tested by `00521E70(unit, role)` | seat pointer |
| --- | --- | --- |
| `0Ah` | 5 (`007303BA`) | `gun+39Ch` |
| `0Bh` | 7 (`007303DF`) | `gun+3A4h`, else `gun+3A0h` |
| `10h` | 3 (`0073041A`) | `gun+394h` |
| `4`,`5`,`6`,`7` | 4 (`0073044B`) | `gun+398h` |
| anything else | 2 (`00730471`) **and** 3 (`00730482`) | `gun+390h` |

`gun+3F0h` is the owning unit (`docs/UNIT_WEAPON_DEVICES.md` already corrected
the older "ammo provider" reading), and `00521E70`'s whole body is
`EAX = unit[+1ACh + role*4]; return EAX == 8 || BSP_PartySlot_IsAiHeld(EAX)` —
the nine-role assignment table of `docs/SHIP_AI_ROLE_OWNERS.md`. Role id and
seat offset covary by exactly four bytes across all five arms, so
`gun+390h` is an array of six seat pointers indexed by `role - 2`. **The writer
of those six slots was not read; the mapping is derived from the dispatch, not
from a producer.**

`vtable[24h]` on a seat returns the seat's authored `BulletThrowMul`. Two of them
were read to the instruction:

| getter | vtable | body | rule |
| --- | --- | --- | --- |
| `008FB4E0` | `00D18140+24h`, whose `+0Ch` is `BSP_GunBot_TurretAimAndFireTick 008FFA20` | `008FB4E0..008FB4F0` | `[[00E199A0] + 24h*bot[+34h] + 1Ch]` |
| `006DEE00` | `00CFE000+24h`, whose `+0Ch` is `BSP_GunBot_MuzzleSolutionAimTick 006DF520` | `006DEE00..006DEE10` | `[[00E19994] + 28h*bot[+34h] + 30h]` |

`docs/ROBOT_CONFIG.md` gives `[00E199A0]` as TailGunnerBot (stride `24h`) and
`[00E19994]` as ArtillerySubDirectorBot (stride `28h`), and
`include/bsp/robot_config.hpp` names `+1Ch` and `+30h` of those two records
`bullet_throw_mul`. Both getters are therefore `BulletThrowMul` reads, and both
are consistent with their own class's stride.

The second one is worth flagging: the bot whose tick is `006DF520` reads its aim
error from the **ArtilleryGunnerBot** table (section 7) but its `BulletThrowMul`
from the **ArtillerySubDirectorBot** table. That is not a mis-read — the
ArtilleryGunnerBot record has no `BulletThrowMul` field at all (its seven floats
are listed in section 7), so the throw multiplier for a main-battery gunner is
authored on the sub-director's `Robots.lua` entry.

Neither getter has a Ghidra function start; both were read with `disasm-raw` and
are listed in the report under `no_ghidra_function`.

## 4. The draw, `0073051F`..`0073064A`

`00730525 COMISS XMM0,[00D7A218]` against `0.0f` with `JBE 00730762`: a magnitude
at or below zero skips the entire block, cone and all.

`00730532 CMP byte ptr [ESP+10Ch],0` splits the two paths.

**`useExplicitThrow != 0`** (`007305E4`): `gun+400h = throwA` and
`gun+404h = throwB` straight from the arguments, and `00730606 JMP 0073060A`
jumps past the unit multiplier of the random path. `docs/UNIT_WEAPON_DEVICES.md`
records two callers that pass `1`: `008BE5AD` and `008BE76F`, the scripted fire
path.

**`useExplicitThrow == 0`** — the draw. All three constants were read out of
`.rdata`:

```
00730540  FLD  [00CE3D9C]      ; 40C90FDB = 6.2831855f = 2*pi
0073054D  MOV  ECX,1           ; RNG stream index 1
00730557  CALL 00BD2F10        ; theta = U(0, 2*pi)
0073055C  FSTP [EBP+400h]      ; gun+400h, throwA

00730562  FLD1                 ; 1.0f
0073056B  MOV  ECX,1
00730575  CALL 00BD2F10        ; u = U(0, 1)
0073057A  FSTP qword [ESP+14h] ; kept as a double

0073057E  FLD  [ESP+0Ch]       ; the magnitude
00730586  CALL 00412E20        ; BSP_Math_TangentX87Float: FSINCOS, FDIVP
0073058B  FMUL qword [ESP+14h] ; tan(magnitude) * u
0073059F  FST  [EBP+404h]      ; gun+404h, throwB
```

So the distribution is

* `throwA` = roll about the barrel axis, **uniform on [0, 2π)**;
* `throwB` = **`tan(Throw') · U(0,1)`**, where `Throw'` is the magnitude of
  section 3.

`throwB` is a *tangent*, not an angle: section 5 adds it to a unit forward
vector, so the realised off-axis angle is `atan(tan(Throw')·u)`, which for a
`Throw` of 0.01 rad is `0.01·u` to eleven digits. Because `u` is uniform in
**radius**, not in area, the pattern is centre-weighted — the density per unit
area falls off as `1/r`, and the mean miss angle is half the maximum, not
`2/3` of it as a uniform-disc model would give.

Two scalars then multiply the result:

| site | condition | effect |
| --- | --- | --- |
| `007305D6` | `gun[+3F8h]+34h` non-null, `gun+3F0h` non-null, sub-type in `{4,5,6,7}` | `gun+404h *= unit[+63Ch]`. The producer of `unit+63Ch` was not read. |
| `00730619` | always | `s = 00470440(7, unit)`, and `0073062C`/`00730646` scale **both** `gun+400h` and `gun+404h` by it |

`00470440(category, unit)` is `return [00E0C978] && [[00F88C30] + 0Ch*category + 88h] ? BSP_GameplayModifiers_ProductForUnit(category, unit) : 1.0f` — body read in full at `00470440..00470465`, `RET 8`, `ECX = [00F88C30]`. `docs/GAME_EXECUTABLE.md`
records that `008E6430` runs over an empty category list in `bsp_game.exe`, so in
the harness `s` is exactly `1.0f`. What gameplay category 7 is named was not read.

Scaling the *angle* by `s` as well as the radius is what the listing does; it is
harmless while `s == 1` but it is not a rotation-invariant scaling, and it is
recorded here as observed rather than explained.

## 5. Application, and the two deterministic spreads that are not dispersion

`00730654`..`0073075E`, all in frame A:

```
dir = row2
    + throwB * cos(throwA) * row1
    + throwB * sin(throwA) * row0
```

built component by component with an `FSTP float ptr` at every step, the `cos`
term accumulated first (`0072FFD9`-style adds at `007306C2`, `007306CE`,
`007306DA`) and the `sin` term second (`0073073E`, `0073074A`, `00730756`). The
result is **not** renormalised, which is exactly why the magnitude is a tangent.

Two other spreads share the shape and must not be confused with it:

**The torpedo fan**, `007304A0`..`0073051A`, taken when the sub-type is `0Ah`. It
is deterministic. `006FDD70(gunclass)` returns `([+0A0h] - [+9Ch]) / 18h`, the
muzzle-point count. With fewer than two the magnitude is zeroed; otherwise

```
offset = Throw' * gun[+44Ch] / (n - 1)  -  Throw' / 2      (00D7A280 = 0.5)
```

where `gun[+44Ch]` is `nextFireBarrel`, the tube being fired. `00730515` then
calls `0085C3F0` with `ECX = EDX = &dir` and the stack pair
`(&row1, offset)`, and `0073051A JMP 00730762` skips the random cone entirely.
`0085C3F0`'s only callee is `BSP_Vector3f_Cross 004F9B30` and it reads two float
triples, which is consistent with an axis-angle rotation of the direction about
row 1 — **provisional; only its entry and argument binding were read.**

**The multi-bullet ring**, `0072FEFB`..`00730035`, inside
`BSP_Gun_SpawnShotAndEffects 0072F830`:

```
theta_i = 2*pi * i / gunclass[+0CCh]          (00CE3828 = 6.283185307179586)
dir_i   = dir + gunclass[+0D0h] * (cos(theta_i)*B + sin(theta_i)*A)
```

with one `0072BF10` call per `i` at `00730042` and a single call at `00730076`
when the count is not positive. Both fields are authored and their producer is
`BSP_GunClass_ReadLuaFields 007327B0`: `00732AE4` stores the integer
**`OneTimeBulletAmount`** to `+0CCh` and `00732B1C` stores the float
**`MultiBulletConeAngle`** to `+0D0h`. No RNG is involved; every pellet of a
burst lands on the same ring at the same phase every time. The two basis vectors
come from `BSP_Matrix_BuildLookAt 00B63F10` at `0072FECA` and `006FDD00` at
`0072FED3`; **their provenance was not read** and they are parameters of the rule
in the header rather than a claim.

## 6. The RNG

`00BD2F10(lo, hi)` is a three-hop forward:

```
00BD2F10  forwards (lo, hi) unchanged and leaves the caller's ECX alone, RET 8
00BD2ED0  ECX = the stream index; GetCurrentThreadId through [00CE223C], linear
          search of the [0109 0AC4] table of [0109 0AEC] entries; on a hit
          EAX = 0109 0AF0 + 9C8h * (stream + 2*threadSlot), on a miss 00E14748
00BD2E60  u = (uint32)00BA2C20() * 2^-32 ; return lo + u*(hi - lo)   (RET 8)
```

`00BD2E60`'s conversion is `FILD` of the signed word plus `[00CE3978]`
(`4F800000` = 2³²) when it reads negative, times `[00D63B80]`
(`3DF0000000000000` = 2⁻³²) — an unsigned-to-float fixup, so `u ∈ [0, 1)`.

`00BA2C20` is **MT19937**, already named `BSP_RandomState_NextU32` and
reconstructed in `src/random.cpp`. `[ESI] >= 270h` refills through `00BF0D20`;
the tempering is `y ^= y>>11`, `y ^= (y<<7) & 9D2C5680h`,
`y ^= (y<<15) & EFC60000h`, `y ^= y>>18`. The masks in the listing are
pre-shifted (`AND EDX,0FF3A58ADh` **then** `SHL EDX,7` is `0FF3A58ADh << 7 =
9D2C5680h`; `0FFFFDF8Ch << 15 = 0EFC60000h`), which is why they do not look
standard at a glance. `include/bsp/random.hpp` already has the `9C8h` state:
`index +0`, `words[624] +4`, the two guard bytes at `+9C4h`/`+9C5h`.

**Every dispersion draw in this document passes `ECX = 1`**, that is stream 1 of
the two per thread. `0072FB6A`, `00730557`, `00730575`, `006DF009`, `006DF0B8`,
`006DF0DC` and `006DF5D4` all set `MOV ECX,1` immediately before the call. What
stream 0 is reserved for was not read.

## 7. Producer 2: the gun bot's aim error pair

### 7.1 `006DEFF0 BSP_GunBot_RerollAimErrorEnvelope`

`__thiscall(bot)`, plain `RET` at `006DF12B` — **no** stack arguments — body
`006DEFF0-006DF12B`. Two call sites, both read:

* `006DF5E9`, inside `BSP_GunBot_MuzzleSolutionAimTick 006DF520`;
* `006DF24A`, inside the attach override `006DF1F0`, which first sets
  `bot+74h = 0.0f` and `bot+78h = [00D7A24C] = 1.0f`, so the first tick after an
  attach always rerolls.

The body, in order:

```
006DEFF3  t   = U(0, 1)                                   ; ECX = 1
006DF02A  k   = [[00E19990] + 1Ch*bot[+34h] + 10h]
006DF030  if (t == 0.0f) m = 0.0f                         ; FUCOMIP/LAHF/TEST 44h
006DF065  else m = 2^(k * log2(t)) = t^k                  ; FYL2X, F2XM1, FSCALE
006DF09D  e   = [[00E19990] + 1Ch*bot[+34h] + 0Ch]
006DF0B8  r   = U(0, e) * m                               ; ECX = 1
006DF0DC  phi = U(0, 2*pi)                                ; ECX = 1, 00CE3D9C
006DF0E5  bot+64h = bot+6Ch ; bot+68h = bot+70h           ; the old target
006DF0F1  bot+5Ch = bot+64h ; bot+60h = bot+68h           ; seed the current pair
006DF0FD  bot+6Ch = sin(phi) * r
006DF112  bot+70h = cos(phi) * r
```

The operand order of `FYL2X` is the one thing here that is easy to get backwards:
`006DF065 FLD [ESP+0Ch]` pushes `k`, `006DF069 FLD [ESP+4]` pushes `t` on top, and
`FYL2X` computes `ST(1)·log2(ST(0))`, i.e. `k·log2(t)`. The result is **`t^k`**,
not `2^(k·t)`. The `006DF053` fixup (`[00D7A208] = -0.0f` minus the base) is the
compiler's negative-base guard inside an inlined `powf` and cannot fire for a
draw in `[0, 1)`.

### 7.2 The descriptor, from its producer

`load_robot_config_00901610` ends with eight
`MOV ECX,<name>` / `CALL find_robot_config_00900AF0` / `MOV [global],EAX` triples
in which the store lands one call *after* the name that produced it. Reading them
with that shift:

| name string | name | global |
| --- | --- | --- |
| `00D17D44` | `PilotBot` | `00F8A30C` |
| `00D17D5C` | `TailGunnerBot` | `00E199A0` |
| `00D17D50` | `AAFlakBot` | `00E1999C` |
| `00D17D6C` | `AAGunnerBot` | `00E19998` |
| `00D17D78` | `ArtillerySubDirectorBot` | `00E19994` |
| `00D17D90` | **`ArtilleryGunnerBot`** | **`00E19990`** |
| `00D17DA4` | `TorpedoBot` | `00E1998C` |
| `00D17DB0` | `DepthChargeBot` | `00E19988` |

`00900AF0` is a lookup, not a constructor: it searches the name-keyed map at
`00F89994` and returns the stored config, or `0` when the name is absent.
`src/robot_config.cpp:846` independently binds `00E19990` to `ArtilleryGunnerBot`.

The record layout is therefore **`read_ArtilleryGunnerBot_parameters_008FD370`**,
not `006DEFF0`'s reads. `docs/ROBOT_CONFIG.md` gives the shape: a `0Ch` header
(`vtable +0`, `NoTargetTimeUntilRest +4`, name pointer `+8`) followed by six
levels; ArtilleryGunnerBot's allocation is `0B4h` and its stride `1Ch`, and
`0Ch + 6·1Ch = 0B4h` closes exactly. `008FD370` writes seven floats at
`+0Ch`..`+24h`, which is why the runtime address `base + 1Ch·level + fieldOffset`
looks as if the field ran past the stride: `fieldOffset` is measured from the
config base, past the header, not from the level.

| offset | Lua key |
| --- | --- |
| `+0Ch` | **`MaxAngleError`** |
| `+10h` | **`Power`** |
| `+14h` | `TargetPointRefreshTime` |
| `+18h` | `SectionTargetChance` |
| `+1Ch` | `EngineRoomWeight` |
| `+20h` | `MagazineWeight` |
| `+24h` | `FueltankWeight` |

So `e` is `MaxAngleError` and `k` is `Power`, and the recovered distribution is

> **magnitude** `r = U(0, MaxAngleError) · U(0,1)^Power`, **roll**
> `phi = U(0, 2π)`, pair `(r·sin φ, r·cos φ)` in radians.

`Power > 1` pulls the magnitude towards zero (an accurate crew), `Power < 1`
pushes it out towards `MaxAngleError`. The product of two independent draws is
already centre-heavy at `Power = 1`.

### 7.3 Cadence

`006DF59F`..`006DF5E9`, the head of `006DF520`:

```
bot+74h -= dt                                  ; dt is frame A's [ESP+54h]
if (0.0f <= bot+74h) keep the pair             ; FLDZ, FCOMIP, JBE
else  p = U([00CE3854], [00CE3918]) = U(3.0f, 8.0f)   ; ECX = 1
      bot+78h = bot+74h = p
      006DEFF0(bot)
```

The period is a pair of hard-coded constants, **not** an authored field. It is
re-drawn on every reroll, so the error wanders on an aperiodic 3–8 second cycle.

### 7.4 Application

`006DF623` and `006DF651` call `BSP_Math_InterpolateClamped 00419010` with, in
argument order, `(x0 = 0.0f, y0 = bot+6Ch, x1 = bot+78h, y1 = bot+64h,
x = bot+74h)`, result to `bot+5Ch`; the same over `bot+70h`/`bot+68h` to
`bot+60h`. With the countdown starting at the period and falling to zero, the
current pair therefore slides **from the previous target to the new one** over
the period. `include/bsp/gun_bot_ticks.hpp` already models this as
`gun_bot_muzzle_error_006df520`; note its field labels `start_horz = +6Ch` /
`end_horz = +64h` follow the interpolation's `y0`/`y1` order, which is the
reverse of the time order.

The pair reaches the gun as **angles**, at `006DFB0B`:

```
006DFB1C  yaw'   = BSP_Math_AddWrappedAngle(yaw,   bot+5Ch)
006DFB36  pitch' = BSP_Math_AddWrappedAngle(pitch, bot+60h)
006DFB54  ok    &= BSP_TurningGun_SetTargetAngles(gun, yaw', pitch')
```

`00438AA0` is an add wrapped into `(-π, π]`. `yaw` and `pitch` are frame slots
written by `00955630` at `006DFAD4` — the gravity arc a sibling packet owns.

**Contract with the arc solver.** This packet does not read `00955630`. What it
needs, and all it needs, is: `00955630` returns a bool in `AL` and leaves a
horizontal angle and a vertical angle in the two frame slots
(`006DF520`'s `[ESP+14h]`/`[ESP+18h]` at `006DFB15`/`006DFB2F`) plus an aim point
in `[ESP+24h]`..`[ESP+2Ch]` that `006DFAE9` copies to `gun+408h`. The error pair
is applied **after** the arc solve and **only** to the two angles; the arc's aim
point is stored unperturbed. Composition is therefore additive in angle space,
and a reconstruction of the arc can ignore dispersion entirely.

Note also that the *per-shot* cone of sections 3–5 composes with this the same
way but one layer down: the bot perturbs the commanded angles, the gun's own
`Fire` then perturbs the muzzle direction built from the barrel's realised
matrix. The two are independent draws from the same stream and multiply neither
each other nor a common scale.

## 8. What is proven and what is assumed

Proven from the listing, byte by byte:

* `00730160`'s `RET 0Ch` and the three-argument ABI, and the frame-A slot map
  including the two `POP`/`PUSH` shifts.
* `gun[+3F8h]+4h` is the Lua `Throw`, from `007313E0`, and the `48h`-stride array
  it belongs to, from the `00732CAE` call site's addressing.
* The zeroing gate, its `TurnOffAAGunThrow` string and the `settings+760h` store.
* The six-seat dispatch and `00521E70`'s complete body.
* `throwA = U(0,2π)` and `throwB = tan(Throw')·U(0,1)`, with all four constants
  read out of `.rdata`.
* `00470440`'s complete body and its `1.0f` default.
* The application formula and the two deterministic spreads, with
  `OneTimeBulletAmount`/`MultiBulletConeAngle` from their producer.
* `00BD2F10`/`00BD2ED0`/`00BD2E60`/`00BA2C20` end to end, including the MT19937
  tempering masks and the stream index.
* `006DEFF0`'s complete body, both of its call sites, and the `t^k` operand order.
* The ArtilleryGunnerBot layout, from `008FD370` via `docs/ROBOT_CONFIG.md`, and
  the `00901610` name-to-global pairing.
* The reroll cadence constants `3.0f`/`8.0f` and the interpolation argument order.
* `006DEE00` and `008FB4E0` as `BulletThrowMul` getters, including which config
  table and stride each uses.

Assumed, derived or unread:

* `gun+390h`..`+3A4h` as a six-entry array indexed by `role - 2`: derived from the
  four-byte covariance of offset and role id across five dispatch arms. **No
  writer of those slots was read.**
* That the seat object's `vtable[24h]` is always a `BulletThrowMul` getter. Two of
  the nine robot classes were checked; the other seven were not.
* `0085C3F0` as an axis-angle rotation: entry and argument binding only.
* `unit+63Ch` and gameplay-modifier category 7: consumed, never traced to a producer.
* The `006FDD00`/`00B63F10` basis vectors of `0072F830`'s ring.
* Which of the two per-thread RNG streams is the replicated one.
* The x87 working precision. Every rule in `src/gun_dispersion.cpp` reproduces the
  native `FSTP float ptr` roundings exactly but computes intermediates in `double`,
  because Win32 MSVC has no 80-bit type; a one-ULP divergence is possible where a
  `double` cannot separate two `long double` results.

## 9. Corrections to existing documents

| document | was | is | evidence |
| --- | --- | --- | --- |
| the packet brief | `0072F830` builds the `Throw` perturbation | `0072F830` has one RNG call (`0072FB6A`, an effect timer) and a deterministic ring; the `Throw` cone is in its caller `00730160` | the only `CALL 00BD2F10` in `0072F830..0073015C` is at `0072FB6A` and stores to `gun[+3F0h]+6F8h`; `0072FEFB`'s loop reads `gunclass+0CCh`/`+0D0h` and no RNG |
| `docs/GUN_BOT_TICKS.md` §6 step 2 | the reroll envelope is `2^(k*rand(0,1))` | it is `rand(0,1)^Power`; `FYL2X` computes `ST(1)·log2(ST(0))` with `k` in `ST(1)` and the draw in `ST(0)` | `006DF065 FLD [ESP+0Ch]` (`k`), `006DF069 FLD [ESP+4]` (the draw), `006DF06D FYL2X` |
| `docs/GUN_BOT_TICKS.md` §6 step 2 | the reroll reads only `descriptor+10h` | it also reads `descriptor+0Ch`, `MaxAngleError`, as the magnitude bound at `006DF09D` | `006DF09D FLD [EDX+ECX*4+0Ch]` with `ECX = 7*bot[+34h]` |
| `docs/GUN_BOT_TICKS.md` §4 | `006DF520`'s descriptor fields `+18h`..`+24h` are "the four lead scalars" and `+14h` "the lead re-roll period" | they are `SectionTargetChance`, `EngineRoomWeight`, `MagazineWeight`, `FueltankWeight` and `TargetPointRefreshTime` — target-section weights, not lead coefficients | `read_ArtilleryGunnerBot_parameters_008FD370`, tabulated in `src/robot_config.cpp:216` |
| `docs/UNIT_WEAPON_DEVICES.md` §Firing | `00730160` is `no_ghidra_function` | Ghidra has `BSP_Gun_Fire` with body `00730160-00730A1D` | `bsp.py ghidra proto 00730160 --brief` |
| `docs/UNIT_WEAPON_DEVICES.md` step 7 | the spread is "scaled by the ammo provider through `00521E70`" | `00521E70` is an AI-role test on the owning unit; the multiplier comes from one of six seat pointers at `gun+390h`..`+3A4h` | `00521E70`'s body reads `unit[+1ACh + role*4]`; `00730491` calls `vtable[24h]` on the seat, not on `gun+3F0h` |

No existing header's record layout is contradicted. `include/bsp/robot_config.hpp`
and `include/bsp/gun_bot_ticks.hpp` are both consistent with the producers read
here; the two `gun_bot_ticks.hpp` notes above are naming, not layout.

## 10. Routine table

| routine | body | coverage |
| --- | --- | --- |
| `00730160` | `00730160..00730A1D` | partial: `0073031D..0073075E` complete; the muzzle-position tail `00730762..00730A1B` and the head `00730160..0073031C` by call shape only |
| `006DEFF0` | `006DEFF0..006DF12B` | complete |
| `006DF520` | `006DF520..006DFC6C` | partial: `006DF59F..006DF659` and `006DFAD4..006DFB5B` only; the rest is `docs/GUN_BOT_TICKS.md` |
| `006DF1F0` | `006DF1F0..006DF250` | complete |
| `00BD2F10`, `00BD2ED0`, `00BD2E60`, `00BA2C20` | `00BD2F10..00BD2F30`, `00BD2ED0..00BD2F0B`, `00BD2E60..00BD2ECB`, `00BA2C20..00BA2C7D` | complete |
| `00412E20`, `00521E70`, `00470440`, `006FDD70` | — | complete |
| `006DEE00`, `008FB4E0` | `006DEE00..006DEE10`, `008FB4E0..008FB4F0` | complete; no Ghidra function start at either |
| `0072F830` | `0072F830..0073015C` | partial: `0072FEFB..00730080` and the `0072FB2C..0072FB7E` RNG arm only |
| `007313E0` | `007313E0..00731A00` | partial: the `param_1+0h..+34h` field binds only |
| `00901610` | `00901610..00901AA3` | partial: the `009019DC..00901A58` name-to-global tail only |
| `0085C3F0` | `0085C3F0..?` | partial: entry and argument binding only |
| `00955630` | — | not read; contract only (section 7.4) |

## 11. Follow-up packets

* **`gun_seat_pointer_table`** — find the writer of `gun+390h`..`+3A4h` and prove
  the `role - 2` index. Until then every `BulletThrowMul` claim in section 3 rests
  on a derivation. Addresses: the gun constructor and whatever calls
  `009281C0(role, slot)` for a gun.
* **`gameplay_modifier_categories`** — name the `0Ch`-stride table at
  `[00F88C30]+88h`; category 7 scales all gun dispersion and category 8 the reload
  rate (`docs/UNIT_WEAPON_DEVICES.md`). One producer read would name both.
* **`unit_accuracy_field_63c`** — the producer of `unit+63Ch`, the per-unit radius
  multiplier for bullet sub-types 4 through 7.
* **`gun_muzzle_basis`** — `006FDD00` and the `00B63F10` look-at that build
  `0072F830`'s ring basis, and `0085C3F0`'s rotation contract, which the torpedo
  fan shares.
* **`random_stream_roles`** — which of the two per-thread MT19937 streams is
  replicated across the network and which is cosmetic. Every routine in this
  document uses stream 1; `00BD2ED0`'s `(stream + 2*threadSlot)` indexing is the
  place to start.
* **arc composition (for the `00955630` owner)** — nothing to do, but worth a line
  in that packet's doc: the bot's error pair is added to the arc's two angles
  *after* the solve and never touches the aim point, so the arc can be
  reconstructed and tested with dispersion switched off.

## Correction from docs/GUN_MOUNT_POSITIONS.md (packet cc7_gun_mount_positions)

- **Was:** section 5 reads `006FDD70`'s muzzle count as dividing the byte span by `18h` (24).
  **Is:** it divides by `0Ch` (12), so the element is a 12-byte triple of floats, not a 24-byte
  record.
  **Evidence:** `006FDD70` loads `[ECX+9Ch]`, subtracts to get the byte span, then
  `006FDD83 MOV EAX,0x2AAAAAAB` / `006FDD88 IMUL ECX` / `006FDD8A SAR EDX,1`. `0x2AAAAAAB` is
  `round(2^33/12)`, so the high half of the product is `span/6` and the single arithmetic shift
  makes it `span/12`. A divisor of 24 would need `SAR EDX,2` with the same constant. Verified at
  integration by reading the listing directly. This also agrees with `docs/GUN_PLATFORM_ARC.md`,
  which already records the count as `(descriptor+A0h - descriptor+9Ch) / 0Ch`.
