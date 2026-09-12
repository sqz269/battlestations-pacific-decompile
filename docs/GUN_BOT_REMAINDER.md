# The gun-bot remainder

Addresses: `008FBB00`, `008FB8D0`, `0042D810`, `00816650`, `008FC080`, `00902920`
(`00902F7A`..`009030B0` only), `008FFF20`, `00959C20`'s builders, `004F3730`,
`008527E0`, `008FE140`, `00951FC0`, `0093A570`.

This document continues `docs/GUN_BOT_TICKS.md`. It reads the bodies that
document left as **contract: unread** and the two ticks it marked partial. The
reconstruction is `include/bsp/gun_bot_remainder.hpp` and
`src/gun_bot_remainder.cpp`; the three complete ticks stay in
`include/bsp/gun_bot_ticks.hpp` and are not touched here.

Ghidra was read-only for this packet. Every name below is a hypothesis except
the per-level parameter names, which are Lua key string literals in the image.

## 1. The class-to-tick map this packet used

`docs/GUN_BOT_TICKS.md` section 2 pairs each class name with its descriptor
global. The packet brief that opened this work called `00902920` *AAFlakBot*;
the listing disagrees. `00902B6E` loads `[00E19998]` and indexes it with
`skill * 10h`, which is the `AAGunnerBot` descriptor and its 10h stride
(`docs/ROBOT_CONFIG.md`). `009030C0` is AAFlakBot, with the 20h stride. The
existing document was already right and is not corrected.

| tick | class | descriptor | per-level stride | reader |
| --- | --- | --- | --- | --- |
| `008FFA20` | TailGunnerBot | `[00E199A0]` | `24h` | `008FCA10` |
| `00902920` | AAGunnerBot | `[00E19998]` | `10h` | `008FCD60` |
| `009030C0` | AAFlakBot | `[00E1999C]` | `20h` | `008FC6D0` |
| `008FFF20` | TorpedoBot | `[00E1998C]` | `14h` | `008FD640` |
| `008FC080` | DepthChargeBot | `[00E19988]` | `14h` | `008FD880` |
| `006DF520` | ArtilleryGunnerBot | `[00E19990]` | `1Ch` | `008FD370` |

Every descriptor has the `0Ch` header `docs/ROBOT_CONFIG.md` records (vtable,
`NoTargetTimeUntilRest`, name pointer), so level `i`'s record starts at
`descriptor + 0Ch + i * stride` and the readers store into `+0Ch`, `+10h`, ...
That is why a tick's `[descriptor + 0Ch + skill * stride]` and
`[descriptor + 14h + skill * stride]` are two fields of the same record and not
a stride overrun.

## 2. The torpedo intercept, `008FBB00` over `008FB8D0`

`008FBB00` is `__fastcall(ECX shooter, EDX target, float speed,
const float3* target_velocity, float3* out) -> bool`, `RET 0Ch`, body
`008FBB00..008FBC08`. It is a thin wrapper: `008FB8D0` answers a root count and
up to two times, and the wrapper turns the chosen time into
`target + t * target_velocity`. One root uses the first time (`008FBB9E`), two
roots use the second (`008FBB3C`), zero roots answer `false`.

`008FB8D0` is `__fastcall(ECX shooter, EDX target, float speed,
const float3* target_velocity, float* t1, float* t2) -> int`, `RET 10h`, body
`008FB8D0..008FBAFB`. Read from the listing, because every coefficient is built
on the x87 stack.

| site | value |
| --- | --- |
| `008FB8D3`..`008FB8EF` | `d = shooter - target`, component by component |
| `008FB933`..`008FB945` | `a = dot(v, v) - speed * speed` |
| `008FB949`..`008FB96B` | `b = dot(d, v)` |
| `008FB96F`..`008FB97D` | `c = dot(d, d)` |
| `008FB984`, `008FB98E` | the quadratic arm needs `a` outside `+/- 1e-4` (`00D7A268`, `00D0D098`, doubles) |
| `008FBA02` | `disc = b * b - 4 * a * c`; the `4` is `00D7A328`, a double |
| `008FBA1F` | a negative discriminant answers `0` |
| `008FBA21` | `sqrt` through `00BF7030` |
| `008FBA2C`, `008FBA40` | the denominator is `a + a`; the branch is on `a` against `0.0` (`00D7A218`) |
| `008FBA44` | `a > 0`: `t1 = (b + sqrt) / 2a`, then `t2 = (b - sqrt) / 2a` |
| `008FBA95` | `a <= 0`: `t1 = (b - sqrt) / 2a`, then `t2 = (b + sqrt) / 2a` |
| `008FBA6D`, `008FBABE` | a negative `t1` answers `0` after it has been written |
| `008FBAE4` | the answer is `2` when `t2 >= 0`, otherwise `1` |
| `008FB99C`..`008FB9F7` | the degenerate arm: with `a` inside the epsilon and `b` outside it, `t = c / b`, and a negative `t` is written back as `0.0` with the answer still `1` |

### The equation it actually solves

The exact constant-bearing intercept, with `d = shooter - target`, is

```
(|v|^2 - speed^2) t^2 - 2 dot(d, v) t + |d|^2 = 0
```

The native builds the same `a` and `c` but a linear coefficient of `-dot(d, v)`
and then applies the textbook formula with a full `b^2 - 4ac` discriminant. Both
arms carry the same missing factor of two: the quadratic arm divides by `2a`
where the half-`b` form would divide by `a`, and the degenerate arm answers
`c / b` where the exact answer is `c / 2b`.

The result is correct when `dot(d, v) == 0`, which covers a stationary target
and a target crossing exactly abeam, and under-leads otherwise. A destroyer
1000 m away running directly away at half the torpedo's speed is the clearest
case: the exact intercept is `1000 / (20 - 10) = 100 s`, the native answers
`76.8 s`, and the aim point lands 232 m short of the ship. `tests/math_tests.cpp`
pins that pair so the deviation is not quietly "fixed" later.

The wrapper's root choice is consistent with the sign convention: when the
torpedo outruns the target, `a <= 0`, the parabola opens downward, only one root
is positive and the count is `1`, so the first time is used. When the target
outruns the torpedo both roots share a sign and the count is `2`, so the nearer
of the two is used.

### The call site, `008FFF20`

| site | value |
| --- | --- |
| `00900120` | `00427EB0(gun)` gives the shooter point, the gun's own `+0FCh` world position, copied to the `ECX` argument |
| `0090014A`, `0090014E` | `bot->vtable[44h]()` then `0042D7E0` give the target's world matrix; its `+30h`..`+38h` translation row is the `EDX` argument |
| `0090020E` | `target->vtable[34h](&scratch)` is the target's world velocity |
| `00900258` | the velocity copy's **y component is overwritten with zero**, so the intercept is solved against the target's horizontal velocity only |
| `0090022B` | the speed is `[[gun+3F8h]+34h] + 0E4h`, the weapon descriptor's torpedo run speed |
| `0090025E` | the call; a `false` answer jumps to the common abort at `0090097B` |

## 3. Entity vtable slot `100h`, the lead point

Two bodies implement the slot. Ghidra has a function for neither.

| vtable | classes | body |
| --- | --- | --- |
| `0042D810` | the entity base (`00D1A698`), every plane (`00D05F20`, `00D06638`, `00D06920`), `MLandVehicle`, `MLandFort` | `0042D810..0042D827` |
| `00816650` | every ship: the kind `06` base (`00D09678`), `MDestroyer`, `MSubmarine`, `MBattleship`, `MTorpedoBoat` | `00816650..00816987` |

Both are `__thiscall float3* (float3* out, const float3* box, const float3* origin,
float section_chance, float w0, float w1, float w2)`, `RET 1Ch`, and both return
`out`. The argument count comes from the `RET 1Ch`: seven dwords.

`0042D810` writes three zeroes and ignores everything else. **A plane, a land
vehicle and a land fort therefore have no lead offset at all**, and every AA
bot aiming at an aircraft aims at its origin.

### `00816650`, the ship implementation

`ESI` is the ship. The routine picks either one of three named sections or a
random point in the hull box.

| site | rule |
| --- | --- |
| `00816659` | `section_chance <= 0.0` goes straight to the hull box |
| `0081667C`, `00816687` | `section_chance` must beat `00BD2F10(0.0, 1.0)` |
| `0081668F`..`008166C6` | `w0` counts only when `w0 > 0`, `[ship+0A94h] != 0` and `0093A570(ship+0A20h, 5)` is **false** |
| `008166D1`..`0081670F` | `w1` adds on `w1 > 0`, `[ship+0A74h] != 0` and `0093A570(ship+0A20h, 8)` false |
| `0081671D`..`0081675B` | `w2` adds on `w2 > 0`, `[ship+0A84h] != 0` and `0093A570(ship+0A20h, 6)` false |
| `00816769` | a running total at or below `1e-4` (`00D7A268`) falls through to the hull box |
| `00816796` | `pick = 00BD2F10(0.0, total - 1e-4)` |
| `008167AB` | `w0`'s running sum over `pick`: the point at `ship+0A88h` |
| `008167D1` | `w0 + w1` over `pick`: the point at `ship+0A68h` |
| `008167EB` | otherwise the point at `ship+0A78h` |
| `00816803` | the chosen point is written to `out` unchanged; `origin` is not added |

`0093A570` is `__thiscall(vector, int id) -> bool`, body `0093A570..0093A5C6`: a
linear scan of the `10h`-byte records between `[vector+18h]` and `[vector+1Ch]`
for one whose first dword equals `id`. Every call site negates it, so a section
whose id is already in `ship+0A20h` is skipped.

The three records are `{float3 point; byte present;}` at `ship+0A68h`,
`ship+0A78h` and `ship+0A88h`, with the `present` byte at `+0Ch` of each.

### What the four scalars are

`008FFA20` reads them from `[00E199A0] + 24h * bot+34h` at `008FFB8C`,
`008FFB99`, `008FFBA3` and `008FFBAA`, which are offsets `+20h`, `+24h`, `+28h`
and `+2Ch` of the TailGunnerBot level record. `008FCA10`, the reader, gives the
Lua keys:

| argument | offset | key | section |
| --- | --- | --- | --- |
| `section_chance` | `+20h` | `SectionTargetChance` | - |
| `w0` | `+24h` | `EngineRoomWeight` | `ship+0A88h`, id `5` |
| `w1` | `+28h` | `MagazineWeight` | `ship+0A68h`, id `8` |
| `w2` | `+2Ch` | `FueltankWeight` | `ship+0A78h`, id `6` |

So `docs/GUN_BOT_TICKS.md`'s open question is settled: the slot picks an engine
room, a magazine or a fuel tank on a ship, weighted by three per-skill Lua
values, and falls back to a random point inside the hull.

### The hull-box fallback and the `(0.6, 0.6, 0.6)` argument

`00816820` onwards. `[ship+538h]` holds the hull extents at `+0A0h`, `+0A4h`
and `+0A8h`.

```
u = 00BD2F10(-box.x, box.x)             ; 00816883
v = 00BD2F10( 0.0,   box.y)             ; 008168A0
w = 00BD2F10(-box.z, box.z)             ; 008168D5
taper = 00419010(0.6, 1.0, 1.0, 0.1, |w|)   ; 00816941, 00CE3D30 and 00D7A2F0
out.x = origin.x + taper * u * 0.5  * hull[+0A4h]
out.y = origin.y +         v * 0.25 * hull[+0A8h]
out.z = origin.z +         w * 0.5  * hull[+0A0h]
```

The `box` argument is therefore a fraction of the hull half-extents, not a
direction and not a spread in metres. `0.5` is `00D7A280` and `0.25` is
`00D7A348`, both doubles. The taper pulls the shot back towards midships when
the beam draw lands near the edge.

### The two call sites

`docs/BOT_FIRE_TARGET.md` and `docs/GUN_BOT_TICKS.md` both name `008FFBC6` as
the slot-`100h` site. That instruction is `CALL EAX` with
`EAX = bot->vtable[44h]`, the call that fetches the fire target. The slot-`100h`
call is the `CALL EDX` at **`008FFC03`**, with `ECX` set to that target at
`008FFC01`, and the arguments pushed between `008FFBD4` and `008FFC00`. The
`origin` argument is the zero global `00F87574`.

`00902920` makes the second call at **`00902BFD`**, not at `00902CD0`. The
listing at `00902C86`..`00902CD0` is `008FDAF0`, two `00438B10` deltas and one
`00438AA0`, with no virtual call among them. The arguments at `00902BFD` are
`section_chance = -1.0` (`00D7A260`, loaded at `00902BBF`), `w0 = w1 = w2 = 1.0`
(`FLD1` at `00902B93`), `origin = (0, 0, 0)` (`XORPS` at `00902B95`) and
`box = (0.8, 0.5, 0.8)` (`00CE74F8` and `00CE3800`). A negative
`section_chance` fails `00816659` outright, so **AAGunnerBot never aims at a
named section**; it always gets a hull-box point. The whole block is gated at
`00902B7A` on `AngleDiffErrorRatio > 0`.

## 4. `008FC080`, DepthChargeBot's tick

`__thiscall(bot)(float dt)`, `RET 4` at `008FC3F0`, body `008FC080..008FC3F2`.
Ghidra has no function at the start; the previous packet defined one. Read from
the listing.

The level record is `[00E19988] + 14h * bot+34h` and `008FD880` names its five
floats: `AttackDist` `+0Ch`, `BulletThrowMul` `+10h` (the tick never reads it),
`ContinuousFireTime` `+14h`, and the `FireDelay` pair at `+18h` and `+1Ch`.

| step | site | rule |
| --- | --- | --- |
| 1 | `008FC08A`..`008FC0D2` | `00521EA0(bot+38h)`; the same five-part validity test the other ticks use (`target+5Dh`, `bot+50h`, `[unit+3Ch]`, that entity's `+5Dh`, `00803510` on the two `+54h` parties); any failure is `bot->vtable[38h](0)` |
| 2 | `008FC0E2` | `008FBCE0(bot, dt, gun)`, the idle return-to-rest timer |
| 3 | `008FC0E7`..`008FC10C` | when `bot+60h > 0` a local flag is set and `bot+60h -= dt`; the firing arm decrements it otherwise, so it moves by `dt` exactly once a frame |
| 4 | `008FC11E` | `[unit+1ACh] != 8` requires `00927F10` on that side index |
| 5 | `008FC12B`, `008FC13C` | the gun cache `bot+58h` and `bot->vtable[44h]()` must both be non-null |
| 6 | `008FC146`..`008FC166` | `bot+5Ch -= dt`; return while positive, then reset to `0.1f` (`00D17D3C`) |
| 7 | `008FC17F` | the sink rate is `[[gun+3F8h]+34h] + 0DCh` |
| 8 | `008FC19A`, `008FC1DF` | `00414DB0` refreshes either pose whose `+0C8h` byte is clear |
| 9 | `008FC204` | **the depth gate**: a target whose world `+100h` height is above `-2.0` (`00CE7D7C`, a float) is not submerged, and the tail at `008FC232` jumps into `gun->vtable[1E8h](0)` |
| 10 | `008FC23E`..`008FC270` | `sinkTime = max(0, (ownerY - targetY - 15.0) / sinkRate)`; `15.0` is `00CF3F20`, a double, and the clamp is `00415550` |
| 11 | `008FC28A`..`008FC2AB` | `target->vtable[34h](&v)` then `00414260(v, &tmp, sinkTime)` then `004142A0(&targetPos, &pred, &tmp)`: the predicted point is `targetPos + velocity * sinkTime` |
| 12 | `008FC2B0`..`008FC31E` | two **horizontal** squared distances from the owner, one to the current target point and one to the predicted point; the `y` component is never used |
| 13 | `008FC322`..`008FC354` | fire when `max(10000, AttackDist^2) > min(distNow^2, distPred^2)`; `10000` is `00CE3D64`, so the drop radius is floored at 100 m |
| 14 | `008FC356` | a positive `bot+60h` clears the byte |
| 15 | `008FC377` | bit `3` of `[[gun+3F0h]+634h]` clears the byte |
| 16 | `008FC39A` | `gun->vtable[1E8h](byte)` |
| 17 | `008FC3A0` | on a firing frame, the decrement step 3 skipped happens here |
| 18 | `008FC3B1`..`008FC3E7` | when `ContinuousFireTime > bot+60h`, `bot+60h = 00BD2F10(FireDelay[0], FireDelay[1])` |

Step 18 is the one rule this document cannot close statically. `bot+60h` is at
or below zero on every firing frame and keeps falling, so a positive
`ContinuousFireTime` ends the burst on its first frame and only a negative one
extends it. Which the shipped Lua uses was not read.

## 5. `00902920`, AAGunnerBot's fire byte

`docs/GUN_BOT_TICKS.md` marked `00903010`..`009030A8` as read from pseudocode
only. From the listing:

| site | rule |
| --- | --- |
| `00902F62`..`00902F76` | a negative vertical angle is halved (`00D7A280`) |
| `00902F85` | the byte starts at `0` |
| `00902F83`, `00902F97` | `[gun+3F8h]` and `[[gun+3F8h]+34h]` must both be non-null; the second test is the `NEG`/`SBB`/`TEST` idiom, not a comparison |
| `00902FB0` | `0085ABA0(gun, h, v)`; a rejected aim leaves the byte at `0` |
| `00902FCD`..`00902FE2` | `distance < [weapon descriptor + 60h] * 0.9` (`00D7A390`, a double) |
| `00902FF3`, `0090302A` | `00438B10(gun+480h, h)` and `00438B10(gun+484h, v)`, each made absolute with `AND ..., 7FFFFFFFh` |
| `00903040`..`00903052` | the **sum** of the two absolute deltas must be under `0.0872665` rad (`00CF0098`, a double, five degrees). It is a sum, not a per-axis test |
| `0090305F` | the byte becomes `1` here |
| `00903066` | bit `0` of `[[gun+3F0h]+634h]` clears it again |
| `0090308C` | `target->vtable[5Ch](2)` with the answer discarded; `EBX` is `2` from `00902A82`, the only write that reaches this site |
| `0090309E`, `009030A8` | the byte is written into the outgoing argument slot and the routine tail-jumps `gun->vtable[1E8h]` |

`00902920`'s per-level fields are also named now, from `008FCD60`:
`AngleDiffErrorRatio` `+0Ch`, `Dist2AngleErrRatio` `+10h`, `ConstAngleError`
`+14h`, `BulletThrowMul` `+18h`. The document's step 3 weight is
`AngleDiffErrorRatio` and its step 4 draw is `00BD2F10(0, ConstAngleError)`
converted from degrees by `00CE3D28 / 00CE3D20` (pi over 180).

## 6. `008FFF20`'s remaining callees

| routine | body | contract |
| --- | --- | --- |
| `008FE140` | `008FE140..008FE15B` | `__fastcall(entity) -> entity*`: returns the argument when it is non-null and `vtable[5Ch](6)` accepts, otherwise null. A checked cast to the ship base class |
| `008527E0` | `008527E0..0085281A` | `__fastcall(entity) -> bool`: refreshes the pose when `+0C8h` is clear, then answers `[entity+1204h] - 3.0 < [entity+100h]`. `3.0` is `00D7A2B0`, a double. A submarine deep enough below its own `+1204h` reference cannot be engaged |
| `00427E30` | - | `BSP_Vector3f_LengthSquared`, already named |
| `004F3730` | `004F3730..004F3801` | `__fastcall(ECX a0, EDX a1, b0, b1, out) -> bool`, `RET 0Ch`. Two dimensional: only `[0]` and `[1]` of every argument are touched, and `008FFF20` packs `(x, z)` into them. It calls `004F3630` and accepts only when both segment parameters land in `[0, 1]` (`00D7A24C`), then writes the crossing point on the first segment |
| `004F3630` | `004F3630..004F372E` | `__fastcall(ECX &a0, EDX &da, b0, &db, &t, &u) -> bool`. The denominator is `db.x * da.y - db.y * da.x`; `t` is the parameter along `a0 -> a1` and `u` the one along `b0 -> b1`. A parallel pair is rejected by `004F3560(denominator, 0)`, a relative near-equality test whose tolerance rule was **not** read |
| `00951FC0` | `00951FC0..00952042` | `__fastcall(unit)`: flips the sign of the launch offset at `unit+6D4h`, subtracting `00CF8608` on the positive branch, and zeroes it past a limit. `008FFF20` reads the same field at `00900280` while building the run heading, so consecutive launches alternate their spread. The limit test is x87 and was read from pseudocode only |
| `008053C0` | - | `BSP_Recon_EnsureSlot`, already named; `[result+0DDCh]` is the list the friendly scan walks |

The friendly scan itself, `0090058A`..`009007C7`: `008053C0` gives a per-slot
list; each node's entity at `+4` must answer `IsKindOf(6)` and must not be the
gun's own owner (`[ESP+5Ch]`, stored at `00900146` from `[gun+3Ch]`); the
squared distance from the gun (`00427E30`) must be under `4000000`
(`00D09FE8`, a double, so 2000 m); the entity's own path is projected by
`1000.0` (`00CE47A0`) times its `+94h`/`+9Ch` forward, and `004F3730` at
`009006EE` tests that path against the torpedo run. Coverage of the scan's
tail, `009006FB`..`009007C7`, stays where `docs/GUN_BOT_TICKS.md` left it.

## 7. `00959C20`'s solution builders

| builder | body | contract |
| --- | --- | --- |
| `004B4D80` | `004B4D80..004B4DF2` | already named `BSP_Lighting_DirectionFromAngles`; `__thiscall(float3* out)(float pitch, float yaw)`, `RET 8`: `out.y = sin(pitch)`, `out.x = sin(yaw) * cos(pitch)`, `out.z = cos(yaw) * cos(pitch)`. The lighting name is narrower than the routine; the kind-3 arm uses it as a plain pitch/yaw to direction |
| `00955830` | `00955830..00955965` | `__thiscall(device)(int* out_flag, float* out_pair)`: refreshes the device's world matrix when bit `1` of `[[device+3CCh]+5Ch]` is clear, normalises with a floor, builds the affine inverse once and caches it behind `device+10Ch`, transforms the stored direction at `device+110h` into the local frame, then splits it: `y > 1.0` writes the sentinel `00CE3C64` into the flag, `y < -1.0` writes `00CE3CCC`, and anything between goes to `BSP_Direction_ToPitchYaw`. The horizontal half is negated against `-0.0` last |
| `00957740` | `00957740..00957BCC` | `__fastcall(ECX filter, EDX origin, dir, out, device)`, four stack args. An origin below `y = 0` returns unchanged. Otherwise the ray is extended to `t = -origin.y / dir.y` clamped to the weapon's own `[[device+3F8h]+34h]+60h` maximum range, `BSP_SpatialIndex_QuerySegment` is asked for the first hit along it, and the answer falls back to the `y = 0` plane crossing when there is no hit. Coverage: **partial**, the `00957850`..`00957BCC` tail was not read |
| `00957BD0` | `00957BD0..009580DC` | `__fastcall(ECX, EDX, float, float, float, u4, u4, int, float*)`, seven stack args and an SEH frame. Coverage: **contract: unread**, only the ABI was taken |

## 8. Routine table

| routine | body | coverage |
| --- | --- | --- |
| `008FB8D0` | `008FB8D0..008FBAFB` | complete |
| `008FBB00` | `008FBB00..008FBC08` | complete |
| `0042D810` | `0042D810..0042D827` | complete; no Ghidra function |
| `00816650` | `00816650..00816987` | complete; no Ghidra function |
| `0093A570` | `0093A570..0093A5C6` | complete |
| `008FC080` | `008FC080..008FC3F2` | complete except the sign of `ContinuousFireTime` in step 18 |
| `00902920` | `00902920..009030B2` | the fire byte `00902F62..009030B0` is now complete; the rest stays as `docs/GUN_BOT_TICKS.md` section 6.4 has it |
| `008FFF20` | `008FFF20..0090099B` | step 7 complete; step 12's callees complete, the scan tail `009006FB..009007C7` unchanged |
| `004F3730`, `004F3630` | above | complete except `004F3560`'s tolerance |
| `008527E0`, `008FE140` | above | complete |
| `00812090`, `00470BA0`, `007BBB70` | slot `34h` | complete: `GetWorldVelocity(out)`, `RET 4`, returning `out` |
| `00414260`, `004142A0` | above | complete: `out = v * s` and `out = a + b`, both `RET 8` |
| `00951FC0` | `00951FC0..00952042` | partial: the sign flip is read, the limit test is not |
| `00957740` | `00957740..00957BCC` | partial: entry, the range clamp and the segment query |
| `004B4D80`, `00955830` | above | complete |
| `00957BD0` | `00957BD0..009580DC` | contract: unread, ABI only |
| `004F3560` | `004F3560..?` | contract: unread |
| `008FF040`, `008FF310` | - | not read by this packet; Ghidra still has no function at either |

## 9. Corrections to earlier documents

| document | was | is | evidence |
| --- | --- | --- | --- |
| `docs/BOT_FIRE_TARGET.md`, `docs/GUN_BOT_TICKS.md` | the lead point comes from `gun->vtable[100h]` at `008FFBC6` | the call is on the **target entity** and the site is `008FFC03`; `008FFBC6` is `bot->vtable[44h]()`, which fetches that target | `008FFBAD` loads `[EDX+44h]` into `EAX`, `008FFC01` moves the result into `ECX`, `008FFBCE` loads `[EDX+100h]` from that object's vtable |
| `docs/GUN_BOT_TICKS.md` section 6.4 step 3 | `00902C86`, `00902CAC`, `00902CC7`, `00902CD0` are a second `vtable[100h]` sample | those four sites are `008FDAF0`, `00438B10`, `00438B10` and `00438AA0`; the only `vtable[100h]` call in the body is at `00902BFD` | the listing of `00902920`; `[EDX+0x100]` appears once, at `00902BF0` |
| `docs/GUN_BOT_TICKS.md` section 11 | the four lead scalars and the `(0.6, 0.6, 0.6)` direction are open | they are `SectionTargetChance`, `EngineRoomWeight`, `MagazineWeight` and `FueltankWeight`, and the third argument is a fraction of the hull half-extents, not a direction | `008FCA10`'s Lua keys at descriptor `+20h`..`+2Ch`; `00816820`..`008168D5` draws the box against `[ship+538h]+0A0h`..`+0A8h` |
| `docs/GUN_BOT_TICKS.md` section 6.5 step 7 | `008FBB00(runSpeed, targetVelocity, out)` solves the torpedo intercept | it also takes the shooter and target points in `ECX` and `EDX`, and what it solves is an intercept whose linear coefficient is half the correct one | `008FBB00`'s `RET 0Ch` with two register arguments; `008FB8D0`'s coefficients at `008FB933`..`008FB97D` against the `2a` divisor at `008FBA40` |
| `docs/GUN_BOT_TICKS.md` section 6.5 step 12 | the friendly entity is "projected forward by 1000 seconds of its own velocity" | the projection multiplies the entity's `+94h`/`+9Ch` forward vector, not a velocity, and `004F3730` is a two-dimensional segment crossing in `x` and `z` | `00900630`, `00900647` read `[EDI+94h]` and `[EDI+9Ch]`; `004F3730` indexes only `[0]` and `[1]` of all five arguments |
| the brief that opened this packet | `00902920` is AAFlakBot | `00902920` is AAGunnerBot; AAFlakBot is `009030C0`. `docs/GUN_BOT_TICKS.md` section 2 already had this right | `00902B6E` loads `[00E19998]` and `00902B68` indexes it with `skill * 10h`, the AAGunnerBot descriptor and stride of `docs/ROBOT_CONFIG.md` |

## 10. Open questions

- The sign of `ContinuousFireTime` in the shipped DepthChargeBot Lua. The static
  rule is `burst ends when bot+60h < ContinuousFireTime` and `bot+60h` is
  already negative when it is tested.
- What `ship+0A20h` holds. `0093A570` reads `10h`-byte records keyed by an int
  and the three section ids are `5`, `6` and `8`; the producer was not found.
- Why `00902920` calls `target->vtable[5Ch](2)` at `0090308C` and throws the
  answer away.
- `004F3560`'s tolerance rule, and the `00951FC0` limit test.
- `00957BD0`, the kind-1/2 arm's solution builder, and `00957740`'s tail.
- `008FF040` and `008FF310`, which Ghidra still has no function for.
