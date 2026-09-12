# Gun aiming and automatic fire

Addresses: `0085AD80`, `0085ABA0`, `0085AD00`, `0085A8B0`, `0085A830`, `00859550`, `0085A270`,
`0085A3D0`, `0085A130`, `0085B0F0`, `0085BB50`, `00859B10`, `00859950`, `0085BA30`, `00729A80`,
`00727E30`, `0072B2D0`, `0072D830`, `0084C6E0`, `0084C7E0`, `00803480`, `007F5FC0`, `007F60A0`,
`007F5960`, `007F6530`, `007F6840`, `00732A30`, `00732A6C`.
Vtables `00CFE0A8` (gun), `00CFBD20`/`00CFE548`/`00CFBF58`/`00CFC190` (turning family),
tick-element vtable block `00CFE504`.

Reconstruction: `include/bsp/gun_aiming.hpp`, `src/gun_aiming.cpp`. Report:
`reports/gun_aiming.json`. Every descriptive name is a hypothesis, not a recovered symbol; the
field names `horzAngle`, `vertAngle`, `tHorzAngle`, `tVertAngle`, `horzRotDir`, `HorzRotSpeed`,
`VertRotSpeed`, `MinHorzAngle`..`MaxVertAngle` and `RestAngles` are the exception - the image and
the shipped Lua tables carry them as literal strings.

Builds on `docs/UNIT_WEAPON_DEVICES.md` (the gun record and `00730160` `Fire`),
`docs/GUN_CLASS_FAMILY.md` (the six classes behind `00CFE0A8`), `docs/VEHICLE_CLASS_FIELDS.md`
(the platform record and its `Windows` firing arcs) and `docs/TICK_ELEMENT_OVERRIDES.md`
(the fixed-step tick element at `+310h`). `docs/WEAPON_DIRECTOR.md` left "where guns receive
their aim angles" open; this packet answers it.

## Summary

A turning gun keeps two angle pairs in its own instance: the **current** pair `horzAngle`/
`vertAngle` and the **target** pair `tHorzAngle`/`tVertAngle`. Aiming is a two-stage rule.
Something outside the gun sets the target pair through `0085ABA0`, which refuses any pair the
gun's firing arcs cannot reach. Then, once per fixed step, the gun's own tick element `0085AD80`
walks the current pair toward the target pair at `HorzRotSpeed`/`VertRotSpeed` radians per second,
and the per-frame update `0085A270` turns the current pair into two node matrices through
`00859550`. Automatic fire is a separate chain: the gun's `FireIfReady` (`00727E30`) asks
`CanFire` and, on a yes, calls `Fire` with a zero throw pair.

The arcs are per-platform data, not per-gun: the unit's class descriptor holds one platform record
per gun index, each with a list of `{MinHorzAngle, MaxHorzAngle, MinVertAngle, MaxVertAngle}`
windows and two flag bits, one for "may traverse here" and one for "may fire here".

## The gun vtable `00CFE0A8`

The table is `1F0h` bytes (124 slots); `00CFE298` onwards is float and string data, not code.
The slots that matter here, with what each derived class installs. `-` means the class inherits.

| Slot | base gun `00CFE0A8` | `22h`/`23h` turning | `24h`/`27h` MRT/MST | Role |
| --- | --- | --- | --- | --- |
| `00h` | `0072DD00` | `006FDEB0`/`00730F50` | `006FDFA0`/`006FE0B0` | scalar deleting destructor |
| `5Ch` | `006E3D50` | - | - | class-id test, `20h` selects any gun |
| `A0h` | `0072E6D0` | `0085A3D0` | - | size from descriptor; the override seeds the angles |
| `A4h` | `0072F060` | `0085BB50` | `0084C800` | load instance state from a Lua table |
| `A8h` | `0072ADC0` | `0085A130` | `0084C640` | debug field dump (the field-name producer) |
| `DCh` | `0072B2D0` | `0085A270` | - | per-frame update, one float argument |
| `164h` | `0072D830` | `00803480` (`23h` only) | `0084C7E0` | network message handler, switch on `msg+10h` |
| `180h` | `006E3DF0` | `00859950` | - | write a replication snapshot |
| `188h` | `004F17F0` | `00859A20` | - | snapshot sibling, **contract: unread** |
| `18Ch` | `006E3E00` | `0085BA30` | - | apply a replication snapshot |
| `1D0h` | `00729A80` | `0085A830` | - | `CanFire(bool checkReload)` |
| `1D4h` | `00730A20` | - | - | fire helper, **contract: unread** |
| `1D8h` | `00730160` | - | - | `Fire(mode, throwA, throwB)`, `RET 0Ch` |
| `1DCh` | `00727E30` | - | `0084C4E0` | `FireIfReady()` |
| `1E0h` | `006E3DC0` | - | `006FE160` (`27h` only) | **contract: unread** |
| `1E4h` | `006E3DE0` | `008598B0` | - | **contract: unread** |
| `1E8h` | `0072D2C0` | - | `006FDC90`/`006FDCD0` | **contract: unread** |

Slot `A0h` of the base is `BSP_Gun_SetupFromDescriptor`, already established in
`docs/UNIT_WEAPON_DEVICES.md`; `0085A3D0` calls it first (`0085A3D8`) and then seeds the angles,
which is why the turning classes all carry `0085A3D0` there.

Only `21h` `MMultipleBombPlatform` (`00CFE308`) is outside the turning branch; it was not read
for this packet - **contract: unread**.

### The second vtable at `+310h`

`006FDDA0` and every derived constructor store a pointer at instance `+310h`
(`param_1[0xC4]`), and for `MRFSGun` that pointer is `00CFE504`. The block's slot `+4h` is
`0085AD80`. This is the same "tick element" mechanism `docs/TICK_ELEMENT_OVERRIDES.md` describes
for the unit at `unit+310h`: the routine's `ECX` is the sub-object and the body reaches the gun
with a `-310h` bias, which is why Ghidra prints `param_1 + 0xE0` for `gun+3F0h` and
`param_1 - 0x310` for the gun's own vtable. Every offset below is stated on the gun.

Evidence for the bias: `0085AD80` reads `[param_1+0xE4]+88h`/`+8Ch` (the descriptor's rotation
speeds, proven at `00732A30`/`00732A6C`), `[param_1+0xE0]+538h` (the unit's class descriptor, the
same expression `0085A830` builds from `gun+3F0h`), and `param_1+0x7C` as the platform index
(`gun+38Ch` in `0085A830` and `0085ABA0`). All four land on known gun fields at a `310h` bias.

## The gun's turning fields

Names from `0085A130`, the class's own debug dump: it pushes a `{type, value}` pair and then a
literal name for each field. `0085BB50` reads the same five names back out of a Lua table, which
is a second, independent producer for the same offsets.

| Offset | Name in the image | Type | Meaning |
| --- | --- | --- | --- |
| `+38Ch` | - | int | platform index into the unit class descriptor's platform vector |
| `+3B8h` | - | byte | set while the gun may neither fire nor step (`00729A80`, `0085AD80`) |
| `+3BCh` | - | ptr | the traverse node; `00859550` writes its local matrix |
| `+3C8h` | - | ptr | the elevation node; null means one node carries both rotations |
| `+3F0h` | - | ptr | the owning unit (see the correction below) |
| `+3F4h` | - | ptr | the weapon class descriptor |
| `+480h` | `horzAngle` | float | current traverse angle, radians |
| `+484h` | `vertAngle` | float | current elevation angle, radians |
| `+488h` | - | float | the step's start copy of `horzAngle` |
| `+48Ch` | - | float | the step's start copy of `vertAngle` |
| `+490h` | - | byte | fire inhibit; `0085A830` returns false while it is set |
| `+494h` | `tHorzAngle` | float | target traverse angle |
| `+498h` | `tVertAngle` | float | target elevation angle |
| `+49Ch` | `horzRotDir` | int | traverse direction hint, replicated |
| `+4A0h` | - | float | set to `-1.0f` (`00D7A260`) on every accepted aim |
| `+4BCh`..`+4C8h` | - | float[4] | received replication angles, zeroed by `006FDDA0` |
| `+4CCh` | - | int | last applied snapshot sequence, `FFFFFFFFh` from `006FDDA0` |

`+488h`/`+48Ch` are the authoritative pair between steps: `0085AD80` opens by copying
`+488h -> +480h` and `+48Ch -> +484h` (`0085ADA4`, `0085ADB2`) and closes by writing the stepped
values back through `+480h`/`+484h`; `00859550` and `0085A830` read `+480h`/`+484h`.

**Correction to `docs/UNIT_WEAPON_DEVICES.md`.** That doc calls `+3F0h` the ammo provider. The
evidence here says it is the owning unit: `0085A830` and `0085ABA0` read `[gun+3F0h]+538h` as the
unit's class descriptor and index its platform vector with `gun+38Ch`, and `00729A80` reads
`[gun+3F0h]+6F8h`/`+6FCh`, the two unit-wide fire cooldowns that
`BSP_UnitGameObject_TickAdvance` (`00953CC0`, `docs/TICK_ELEMENT_OVERRIDES.md`) counts down at
`unit+6F8h`/`unit+6FCh`. The ammo `vtable[1F4h]`/`[1F8h]` calls that doc cites are a different
read of the same pointer, so "the unit is also the ammo provider" is consistent; the header keeps
the existing constant name rather than adding a second name for one offset.

## The platform record and its firing arcs

`unit -> [+538h]` is the unit's class descriptor. `descriptor+94h` is the base of a platform
pointer array and `descriptor+98h` its count; every read bounds-checks the gun's `+38Ch` against
the count and throws through `005471B0` (`std::vector::at`) on a miss (`0085A830`, `0085ABA0`,
`0085AD00`, `0085A3D0`, `0085B0F0`). `docs/VEHICLE_CLASS_FIELDS.md` is the producer: it names the
record's `RestAngles[1]` at `+94h` (default `FLT_MAX`, `00D7A248`) and `RestAngles[2]` at `+90h`
(default `0.0f`), and its `Windows` list of firing arcs built by `007F6B10` from `Nofire`,
`MinHorzAngle`, `MaxHorzAngle`, `MinVertAngle`, `MaxVertAngle`.

The consumers fix the arc layout: `007F5FC0` walks `platform+3Ch` for `platform+40h` records of
`14h` bytes.

| Arc offset | Field | Read at |
| --- | --- | --- |
| `+0h` | flags: bit 0 may traverse, bit 1 may fire | `007F5FC0` bit 0, `007F60A0` bit 1 |
| `+4h` | `MinHorzAngle` | `007F5FC0`, `007F5960` |
| `+8h` | `MaxHorzAngle` | `007F5FC0`, `007F5960` |
| `+Ch` | `MinVertAngle` | `007F5FC0` |
| `+10h` | `MaxVertAngle` | `007F5FC0` |

The three tests differ only in what they check:

| Routine | ABI | Rule |
| --- | --- | --- |
| `007F5FC0` | `__thiscall(platform, float h, float v) -> bool`, `RET 8` | any arc with bit 0 set whose four bounds contain `(h, v)` |
| `007F60A0` | same | the same window test with **bit 1** instead of bit 0 |
| `007F5960` | `__thiscall(arc, float h) -> bool` | one arc, horizontal bounds only |

All three widen each bound by `eps = 0.008726646` rad (`00D08B88`, half a degree) and first clamp
the horizontal angle into `[-pi, +pi]`: above `+pi` (`00CE3D28`) it becomes `00D7A264` = `+pi`,
below `-pi` (`00CE3D18`) it becomes `00CE684C` = `-pi`.

```
inArc(arc, h, v) = arc.minH <= h + eps && h - eps <= arc.maxH
                && arc.minV <= v + eps && v - eps <= arc.maxV
```

## The aim rule

### 1. Accepting a target - `0085ABA0`

`__thiscall(gun, float horz, float vert) -> bool`, `RET 8`, body `0085ABA0..0085ACF1`.

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `0085ABB5` | `gun+4A0h = -1.0f` (`00D7A260`) unconditionally, before any test |
| 2 | `0085ABBD`, `0085AC0E` | wrap each argument: `a = fmod(a, 2pi)` (`00BF857A`), then `a += 2pi` when `a <= -pi`, `a -= 2pi` when `a > +pi`; constants `00CE3828` = `6.2831855`, `00CE3D18` = `-pi`, `00CE3D28` = `+pi` |
| 3 | `0085AC51`-`0085AC78` | platform = `[[gun+3F0h]+538h]+94h` indexed by `gun+38Ch`, bounds-checked |
| 4 | `0085AC94` | `if (!007F5FC0(platform, horz, vert)) return false` - a traverse window must contain the pair |
| 5 | `0085ACA5` | `if (descriptor[+8Ch] == 0) return false` - `VertRotSpeed` |
| 6 | `0085ACB9` | `if (descriptor[+88h] == 0) return false` - `HorzRotSpeed` |
| 7 | `0085ACD0`, `0085ACDE` | `gun+494h = horz; gun+498h = vert; return true` |

The two rate tests are the reason a gun with a zero `HorzRotSpeed` or `VertRotSpeed` never accepts
a new target: it keeps whatever angles it was built with.

`0085AD00` (`AimToRest`, `__fastcall(gun)`) is the same call with the platform's rest pair:
`if (platform[+94h] != FLT_MAX) 0085ABA0(platform[+94h], platform[+90h])`. `FLT_MAX` here is the
double at `00D7A278`; `docs/VEHICLE_CLASS_FIELDS.md` gives the same default as the float at
`00D7A248`, so a platform without `RestAngles` never re-centres.

`0085A8B0` (`__thiscall(gun, float h, float v) -> bool`, `RET 8`) is the public reachability
query: it wraps both angles the same way and returns `007F5FC0` on the gun's platform, with no
side effect.

`0085B980` is the vector form used by the callers that hold a direction rather than angles: it
transforms a world direction into the gun's frame, clamps the `y` component to `[-1, 1]`
(`00D7A250` = `-1.0`), and calls `0085ABA0(-atan2(x, z), asin(y))` - the horizontal angle is
**negated** relative to `atan2`.

### 2. Stepping toward the target - `0085AD80`

`__thiscall(gun+310h, float dt)`, body `0085AD80..0085B0E2`. Offsets restated on the gun.

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `0085AD8C`, `0085AD96` | gate: `unit[+5Dh] == 0 && gun[+3B8h] == 0`, else the whole step is skipped |
| 2 | `0085ADA0`-`0085ADD0` | `horz = gun+488h`, `vert = gun+48Ch` |
| 3 | `0085ADEB` | `if (descriptor[+88h] <= 0) horz = tHorz` - no `HorzRotSpeed` means snap |
| 4 | `0085ADF9` | `if (descriptor[+8Ch] <= 0) vert = tVert` - same for `VertRotSpeed` |
| 5 | `0085AE19`, `0085AE42` | `dH = abs(wrap(tHorz - horz))`, `dV = abs(wrap(tVert - vert))` through `BSP_Math_SubtractWrappedAngle` |
| 6 | `0085AE4A`-`0085AE8C` | `if (dH < 0.000174533 && dV < 0.000174533) return` - `00CFAA48`, one hundredth of a degree |
| 7 | `0085AE99` | platform lookup, bounds-checked as above |
| 8 | `0085AEDA` | `007F6530(horz, vert, tHorz, tVert, &stepH, &stepV)` on the platform: the arc-aware signed deltas, **contract: unread** for how it routes around a blocked window |
| 9 | `0085AEEB` | `rate = descriptor[+88h]`; `if (dt * rate < abs(stepH)) stepH = sign(stepH) * rate * dt` - the per-step clamp; the sign branch collapses to `+rate*dt` when `stepH >= 0` and `-rate*dt` otherwise |
| 10 | `0085AF76`, `0085AFA6` | `if (!gun->vtable[5Ch](23h)) step *= BSP_Math_InterpolateClamped(0, 0.5, 0.174533, 1.0, dH)` - every class except `MRFSGun` scales the step from 1.0 down to 0.5 as the remaining angle falls below ten degrees (`00CE3990` = `0.17453294`, `00CE3800` = `0.5`) |
| 11 | `0085AFC0`-`0085AFD1` | `gun+480h += stepH` |
| 12 | `0085AFE2`-`0085B0AB` | the same clamp, the same `23h` test and the same scale for the vertical axis with `descriptor[+8Ch]`, then `gun+484h += stepV` |
| 13 | `0085B0C5` | `if (!007F6840(horz, vert)) gun+480h = <pre-step value>` - a final platform test restores the traverse angle when the stepped pair left the arc; **contract: unread** for `007F6840`'s exact predicate |

So the aim rule, per fixed step, per axis:

```
delta      = wrap(target - current)                    // BSP_Math_SubtractWrappedAngle
step       = clamp(delta, -rate * dt, +rate * dt)      // rate = HorzRotSpeed / VertRotSpeed
if class != MRFSGun:
    step  *= lerp(0.5, 1.0, min(|delta|, 10deg) / 10deg)
current   += step
```

with `10deg = 0.17453294` rad (`00CE3990`), the floor `0.5` at `00CE3800`, the dead band
`0.000174533` rad (`00CFAA48`), and the rates read from the weapon class descriptor at `+88h`
(`HorzRotSpeed`, stored at `00732A30`) and `+8Ch` (`VertRotSpeed`, stored at `00732A6C`), whose
Lua defaults are `0.0f` and whose shipped values are in
`scripts/datatables/classtables/{arcade,realistic}/deviceclasses.lua`.

### 3. Applying the angles to the nodes - `00859550`

`__fastcall(gun)`, called at the end of `0085A270` (`0085A3B9`), of `0085A3D0` and of `0085B0F0`.
`00D7A208` is `-0.0f`, so every `00D7A208 - x` in the pseudocode is `-x`.

| Case | Rule |
| --- | --- |
| `gun+3BCh == 0` | nothing happens |
| `gun+3C8h != 0` | two nodes: `gun+3BCh` gets a Y rotation by `-horzAngle`, `gun+3C8h` an X rotation by `-vertAngle`; each matrix keeps row 3 (the translation) from the node's existing matrix at `+30h`..`+3Ch`, and is installed with `node->vtable[38h](matrix)` |
| `gun+3C8h == 0` | one node: two rotations are built by `00B646E0` and `00B64640`, multiplied by `BSP_Matrix_Multiply4x4`, given the node's translation and installed on `gun+3BCh` |

The two-node matrices are written out in full:

```
horizontal (gun+3BCh):  [ cos(-h)  0  -sin(-h)  0 ]
                        [   0      1     0      0 ]
                        [ sin(-h)  0   cos(-h)  0 ]
                        [      node translation    ]

vertical   (gun+3C8h):  [   1      0      0     0 ]
                        [   0   cos(-v) sin(-v) 0 ]
                        [   0  -sin(-v) cos(-v) 0 ]
                        [      node translation    ]
```

### 4. The per-frame update - `0085A270`

`__thiscall(gun, float dt)`, `RET 4`, body `0085A270..0085A3C7`. No Ghidra function starts here;
it was read from the raw listing.

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `0085A27E` | `0072B2D0(gun, dt)` - the base gun update |
| 2 | `0085A286` | `if (dt <= 0) return` - the rest of the body is skipped on a zero step |
| 3 | `0085A2A0`-`0085A3B2` | for each barrel record `i` in `[0, (gun+3D8h - gun+3D4h)/14h)`: skip when `barrel[+10h] == 0`; otherwise take the barrel node's world matrix row 2 (`+20h`, `+24h`, `+28h`, the forward axis), scale it by `barrel[+4h]` (`dist`), add the platform's `i`-th offset from the descriptor vector at `descriptor+B8h` (begin `+BCh`, end `+C0h`, stride `0Ch`) and hand the resulting point to `barrelNode->vtable[2Ch](&point)` |
| 4 | `0085A3B9` | `00859550(gun)` - install the angle matrices |

`0072B2D0`, the base update, is the recoil integrator, not an aim routine. Per barrel it sets
`barrel[+10h] = (speed != 0)`, accumulates `barrel[+Ch]` (`DT`) by `dt` and, while `DT` is at or
above `1/120` (`00CEF0B8`), subtracts `1/120` (`00CFDF80`) and advances the spring:

```
if speed >= 0:  dist += speed;  if dist >= 0: dist = 0, speed = 0, DT = 0
else:           dist += speed;  speed = dist * descriptor[+ACh]
                if dist <= descriptor[+B0h]: speed = descriptor[+B4h]
```

So the barrel record's `dist`/`speed` are the **recoil slide and its velocity**, which step 3
above turns into a node position; they are not a projectile speed and no lead computation reads
them. No caller in the AI modules touches `gun+3D4h` at all (a whole-image scan of the `3D4h`
displacement finds no site between `008F0000` and `00960000`).

### 5. Who calls the per-frame update

`docs/GAME_WORLD_ENTITIES.md` and `docs/IN_MISSION_SUBSYSTEM_TICK.md` establish the chain:
`BSP_Game_UpdateInMissionSubsystems` `004C40A0` -> `world->vtable[0Ch]` = `00904BF0`
`BSP_World_UpdateEntities`, which walks the world node's child chain from `[world+4]` through
`entity+38h`, gates on `entity+5Ch` and calls `entity->vtable[0DCh](scaledDelta)`. A whole-image
scan for a `0DCh` virtual dispatch finds it in only three game routines: `00904C0C` (that walk),
`00904CE1`/`00904D1C`/`00904D59` (the world teardown's three settle passes, each with a zero
delta) and `00922EE9`. The gun's `0DCh` is `0085A270`; this packet stops at those call sites and
does not re-read the world walk.

`0085AD80` is reached through the fixed-step tick element at `gun+310h`, slot `+4h` of the block
at `00CFE504`. The unit's equivalent is the peer packet's; see `docs/TICK_ELEMENT_OVERRIDES.md`
for who drives the element list. **contract: unread** for the element's own dispatcher.

## The automatic fire decision

`00727E30` (`__fastcall(gun) -> bool`, gun vtable slot `1DCh`) is the whole of it:

```
FireIfReady(gun):
    if gun->vtable[1D0h](1):          // CanFire with the reload check on
        gun->vtable[1D8h](0, 0, 0)    // Fire, mode 0, throwA 0, throwB 0
        return true
    return false
```

`vtable[1D8h]` is `00730160` `BSP_Gun_Fire`, `RET 0Ch`, already reconstructed. The turning classes
do not override `1D8h`, so every gun in the family fires through it.

`CanFire` is a two-level chain. `0085A830` (`__thiscall(gun, bool checkReload) -> bool`, `RET 4`,
body `0085A830..0085A8A1`) runs first for every turning class:

| Order | Site | Test | On failure |
| --- | --- | --- | --- |
| 1 | `0085A833` | `gun+490h == 0` | false |
| 2 | `0085A842`-`0085A867` | platform index `gun+38Ch` within `[[gun+3F0h]+538h]+98h` | `std::vector::at` throw `005471B0` |
| 3 | `0085A887` | `007F60A0(platform, gun+480h, gun+484h)` - the **current** angles are inside an arc whose **bit 1** (may fire) is set | false |
| 4 | `0085A899` | tail call to `00729A80` | that routine's answer |

`00729A80` (`__thiscall(gun, bool checkReload) -> bool`) is the base gun's test:

| Order | Test | Meaning |
| --- | --- | --- |
| 1 | `[gun+3F8h]+34h != 0` | the fire-parameter block must be armed |
| 2 | `gun+3B8h == 0` | the same disable byte the step gate reads |
| 3 | `gun+358h <= 0` | a per-gun counter, **contract: unread** |
| 4 | `!checkReload \|\| (gun+450h <= 0 && gun+478h <= 0)` | `barrelDelayTime` and a second timer at `+478h` must both be spent |
| 5 | `!006D1E50() \|\| unit[+6F8h] <= 0` | the unit-wide fire cooldown, counted down by `00953CC0` |
| 6 | `weaponType != 7 \|\| unit[+6FCh] <= 0` | the torpedo cooldown; `7` is the torpedo type id from `[gun+3F4h]+80h` |
| 7 | `muzzleWorldPos.y >= 1.0` (`00D7A24C`) or `!00728A90() && !005459B0()` | the muzzle is above the waterline, or two submerged-fire tests both say no |
| 8 | some barrel `i < gun+448h` has `!checkReload \|\| gun[+414h][i] <= 0` | at least one barrel is out of reload |

Failing any of 1-7 returns false; 8 is the only path that returns true, so a gun with
`barrelNum == 0` never fires.

`FireIfReady` is reached from the network message handler `0072D830`, opcode `0ADh`
(`0072D860`). The MRT/MST override `0084C7E0` adds opcode `0B0h` -> `0084C6E0`, which sets the
angles through `0085B0F0` and then calls `vtable[1D8h](fire, throwA, throwB)` from the message's
`+2Ch`/`+30h`/`+34h`; `MRFSGun`'s `00803480` adds opcode `0AFh` and tail-calls the base for
everything else. `008BE440` `BSP_LuaBinding_GunForceFire` is the scripted entry, already
documented. No other `1D8h` dispatch exists in the image outside those and two unrelated classes
(`004650F0`, `004654D0`, `00B5FE60`, `00B5FE90` are not gun vtables).

**What this does not say.** Which routine decides to *send* opcode `0ADh`, and how the weapon
director's `allowFire` at `director+3Ch` reaches it, was not read: **contract: unread**. The
callers that set a gun's target (`008FFA20`, `008FFF20`, `00902920`, `009030C0`, `00959C20`,
`006DF520`) were identified from the call graph but their bodies were not read, so target
selection and any lead computation are outside this packet.

## Replication

`00859950` (slot `180h`) writes a snapshot and `0085BA30` (slot `18Ch`) applies one. `00859B10`
fills the packet: `+20h = gun+488h`, `+24h = gun+48Ch`, `+28h = gun+494h`, `+2Ch = gun+498h`,
`+30h = gun+49Ch`, `+40h = &gun+4BCh`. `0085BA30` drops the packet when
`gun+4CCh != -1 && gun+4CCh > packet+0Ch` (a sequence test), skips it entirely for a locally
controlled unit (`BSP_UnitInstance_IsLocalPlayerRole(0)` or `(1)`), and otherwise copies each of
the four angles into `gun+4BCh`..`+4C8h` behind its own present flag at `packet+34h`..`+37h`.

`0085B0F0` (`__thiscall(gun, float tHorz, float tVert, float horz, float vert)`, `RET 10h`) is the
immediate set the replication path uses. It writes `gun+480h`/`gun+488h` and
`gun+484h`/`gun+48Ch` only when the wrapped difference from the current angle exceeds `0.8` rad
(`00CE3D40`, about 46 degrees) and the candidate passes an arc search built on `007F5960`; it
always writes `gun+494h`/`gun+498h` and ends with `00859550`. The intermediate angle search and
the four `atan2`-derived values it stores into `gun+4BCh`..`+4C8h` are **contract: unread**.

## Host call sites

One row per native call site the reconstruction models. `this`/args are as the native ABI passes
them; `ret` is what the native caller consumes.

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| `0085AC94` | `007F5FC0` | `platform_allows_traverse` | platform, `h`, `v` / bool | always, inside `0085ABA0` |
| `0085A887` | `007F60A0` | `platform_allows_fire` | platform, `gun+480h`, `gun+484h` / bool | after `gun+490h == 0` |
| `0085AEDA` | `007F6530` | `platform_step_deltas` | platform, `h`, `v`, `tH`, `tV`, out `dH`, out `dV` / void | after the dead-band test |
| `0085B0C5` | `007F6840` | `platform_pair_still_valid` | platform, `h`, `v` / bool | after both axes stepped |
| `0085AF76`, `0085B064` | `gun->vtable[5Ch]` | `is_class` | gun, `23h` / bool | once per axis |
| `0085AE19`, `0085AE42` | `BSP_Math_SubtractWrappedAngle` `00438B10` | `wrapped_difference` | `a`, `b` / float | always |
| `0085AFA6`, `0085B094` | `BSP_Math_InterpolateClamped` `00419010` | `approach_scale` | `0`, `0.5`, `10deg`, `1.0`, `dH` / float | non-`MRFSGun` only |
| `0085AC78`, `0085A867`, `0085AE99` | `005471B0` | `platform_index_out_of_range` | vector, `index+1` / noreturn | index at or past the count |
| `0085ABBD`, `0085AC0E` | `00BF857A` `fmod` | `wrap_two_pi` | `a`, `2pi` / float | always, twice |
| `0085A27E` | `0072B2D0` | `base_gun_update` | gun, `dt` / void | always |
| `0085A348` | `00B6DB60` | `barrel_node_world_matrix` | barrel node / matrix ptr | per active barrel |
| `0085A3A3` | indirect `barrelNode->vtable[2Ch]` | `set_barrel_point` | barrel node, `&point` / void | per active barrel |
| `0085A3B9`, `0085B7BB` | `00859550` | `apply_angles_to_nodes` | gun / void | end of the update and of the snap |
| inside `00859550` | indirect `node->vtable[38h]` | `set_node_local_matrix` | node, matrix / void | per present node |
| `00727E3D` | indirect `gun->vtable[1D0h]` | `can_fire` | gun, `1` / bool | always |
| after `00727E47` | indirect `gun->vtable[1D8h]` | `fire` | gun, `0`, `0`, `0` / void | `can_fire` true |
| inside `00729A80` | `00728A90`, `005459B0` | `muzzle_submerged`, `muzzle_blocked` | gun / bool | muzzle `y < 1.0` only |
| inside `00729A80` | `006D1E50` | `unit_cooldown_applies` | gun / bool | before the `+6F8h` test |
| `0072D860` | indirect `gun->vtable[1DCh]` | `fire_if_ready` | gun / bool | message opcode `0ADh` |

## Coverage

| Routine | Coverage |
| --- | --- |
| `0085AD80` step | complete for both axes; `007F6530` and `007F6840` are contracts |
| `0085ABA0`, `0085AD00`, `0085A8B0` | complete |
| `00859550` | complete for the two-node case; `partial: 0085973x..008597C0`, the single-node case, is described but its two rotation builders `00B646E0`/`00B64640` were not read |
| `0085A270` | complete |
| `0072B2D0` | complete |
| `00729A80`, `0085A830`, `00727E30` | complete |
| `007F5FC0`, `007F60A0`, `007F5960` | complete |
| `0085B0F0` | `partial: 0085B108..0085B720`, the arc search and the four stored angles |
| `0085A3D0` | `partial: 0085A4x..0085A822`, only the angle seeding was read |
| `00859950`, `00859B10`, `0085BA30`, `0085BB50` | the field mapping only; the transport is a contract |
| `00803480`, `0084C800`, `006FDC90`, `006FDCD0`, `006FE160`, `008598B0`, `00859A20` | contract: unread |
| target selection and lead | contract: unread, in `008FFA20`, `008FFF20`, `00902920`, `009030C0`, `00959C20`, `006DF520` |

## The MRFSGun difference

`MRFSGun` (`23h`, vtable `00CFE548`, constructor `00730E80`) adds no field of its own: its
constructor calls `006FDDA0` and writes only the class id and five secondary vtable pointers.
Its vtable differs from the `22h` turning base in exactly two slots, the destructor and the
network handler `164h` (`00803480`, which adds opcode `0AFh`). Everything about how it aims is
therefore shared code, and its one behavioural difference is the class test inside the step:
`0085AD80` asks `gun->vtable[5Ch](23h)` before scaling each axis' step, and an `MRFSGun` skips the
`BSP_Math_InterpolateClamped` soft approach. It turns at the full `HorzRotSpeed`/`VertRotSpeed`
right up to the target instead of easing to half rate inside ten degrees.

The "fixed" and "slave" parts of `Rapid_Fixed_Slave_Gun` are data, not code: a platform whose
`Windows` arcs are a single point, or a weapon class with a zero `HorzRotSpeed`, makes
`0085ABA0` refuse every target and leaves the gun at its seeded angles. The follow relationship
implied by "slave" is the platform record's `DefaultGun`/`DirectorFollowPlatforms`
(`docs/VEHICLE_CLASS_FIELDS.md`) and was not read: **contract: unread**.

## Plane guns

Not read. The turning family is reached from the node factory table shared by ships and planes
(`docs/GUN_CLASS_FAMILY.md`), and `0085AD80` reads nothing ship-specific - the platform vector at
`descriptor+94h` belongs to whatever class the unit has - so the same code is expected to serve
plane mounts. That expectation is not evidence: **contract: unread**.
