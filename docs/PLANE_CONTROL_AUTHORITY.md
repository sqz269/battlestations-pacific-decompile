# `007D9A70` — the control authority a plane has right now

Packet `cc7_authority`, read-only analysis on top of `cc7_rate_clamp`.

## Why it matters

`007DA710 BSP_PlaneFlight_ControlRateLaw` multiplies each axis's rotation rate by an acceleration
term, and each of the three acceleration terms is a class constant times **one scalar**:

| axis | term | address |
|---|---|---|
| pitch | `PitchAccel class+1C0h * m` | `007DAA7F`, `007DAA93` |
| yaw | `YawAccel class+1C4h * -m` | `007DAAA1`, `007DAAAD` |
| roll | `PitchAccel class+1C0h * m * RollAccel class+1BCh` | `007DAAB3` |

`m` is what `007DA380 BSP_PlaneFlight_ControllerModeFactors` hands back, and in free flight - mode
0, the arm at `007DA6E6` - both of its float outputs are the same value, the one `007D9A70`
returned at `007DA384`. So `007D9A70` sets how fast a plane can rotate at all, and nothing about
the rate law can be wired without it.

The roll term deserves a flag rather than a smoothing-over: it is the **pitch** product times
`RollAccel`, not `RollAccel * m`. `007DAAB3 FMUL [EAX+1BCh]` operates on the value still on the x87
stack after `007DAA9D` popped the pitch term, which is `PitchAccel * m`. That is traced, not
inferred, and the x87 stack is balanced end to end across the block - `007DAAB9` leaves it empty,
which the `FDIV` on the call result at `007DAAC5` then confirms. It may be deliberate (roll
authority proportional to pitch authority) or a shipped slip; the listing does not say which, and
neither will this doc.

## The rule

`007D9A70`, `__thiscall(ctl)`, x87 return, body `007D9A70`-`007D9B6E`. `ESI` is the controller,
`ESI+8` the unit, `ESI+0Ch` the class descriptor, `ESI+10h` a third object.

```
A = InterpolateClamped(3.0 -> 0.0, 6.0 -> 0.25, unit+908h)      007D9A79..007D9AA9
s = BSP_PlaneFlight_ForwardSpeed(ctl) / class+184h StallSpd     007D9AB4..007D9ABC
r = InterpolateClamped(ControlRangeMin -> 0.0,
                       ControlRangeMax -> 1.0, s)               007D9AC9..007D9AF0
B = (ESI+10h)->+0C0h * 0.6 + r                                  007D9AF9..007D9B0C
t = (A > B) ? A : (B <= 1.0 ? B : 1.0)                          007D9B10..007D9B6E
return 007C0F40(unit) * t*t                                     007D9B35..007D9B4D
```

The constants, each checked for whether it is a real constant or loader zero-fill:

* `00CE3854 = 3.0f`, `00CE6630 = 6.0f`, `00CE3868 = 0.25f`, `00D7A24C = 1.0f` - all `.rdata`
  (`va=00CE2000 rawsize=00126000`), so compiled constants.
* `00CEFF98 = 0.6` as a double, `.rdata`.
* `00F8731C` and `00F87320` are **not** constants. They are entries 11 and 12 of the plane tuning
  block `g_PlaneDynamics_TuningBlock` at `00F872F0`, filled at runtime by the `REP MOVSD` at
  `007EAAE1` from singleton fields `+23Ch` and `+240h`, which
  `include/bsp/game_tuning_singleton.hpp:180-181` records as
  `Dynamics/SpdMultipliers/ControlRangeMin` and `ControlRangeMax`. Reading them statically gives
  0.0 and means nothing - see `docs/PLANE_CONTROL_RATE_LAW.md`.

## Two independent confirmations

The reading is not resting on the listing alone.

`planeglobals.lua` gives `ControlRangeMin = 1.1` and `ControlRangeMax = 1.7`, and the authored
comments beside them say, in Hungarian, "stallspeed % alatt megszunik a kontroll" - below this
fraction of stall speed the control ceases - and "stallspeed % folott max a kontroll" - above it the
control is at maximum. That is exactly a ramp from 0 to 1 over a speed **ratio**.

And the divisor the listing uses at `007DAAC5`/`007D9ABC` is `class+184h`, which
`include/bsp/plane_class_fields.hpp:119` had already named `kStallSpd` from an unrelated packet. The
Lua comment says the ratio is against stall speed; the field the divide uses is the stall speed. Two
sources that know nothing about each other agree, which is what makes the shape of the curve safe to
state.

So: a plane below `1.1 * StallSpd` has **no** control authority from the speed term, a plane above
`1.7 * StallSpd` has all of it, and the result is **squared** before it scales the rotation
accelerations - so authority falls off much faster than speed does near the stall.

## `007C0F40`, the damage scale

`__thiscall(unit)`, body from `007C0F40`. It starts at `1.0f` (`00D7A24C`, `007C0F41`) and walks the
array at `unit+974h` of `unit+994h` entries (`007C0F4F` guards the empty case), calling each
element's `vtable[+210h](2Ah, 0)` and, on the first that answers true, `vtable[+214h]` and then
arithmetic not read here. `2Ah` is the same class id the ordnance tables call a general bomb
(`docs/ORDNANCE_KIND_IDENTITY.md`), so the argument is a **kind selector** over the unit's parts
rather than a bomb; what kind `2Ah` names in this table is not established.

What matters for wiring is the guard: **an empty part list returns 1.0 unchanged**, and it is the
only path read here. An undamaged plane in this reconstruction has no parts on that list, so
`m = t*t` exactly. Whether a damaged plane's parts drive it below 1.0 is a hypothesis this doc does
not test.

## What is not established

* `(ESI+10h)->+0C0h`, the third input to `B`. `ESI+10h` is not identified.
* `007C0F40`'s arithmetic after the matching part is found (`007C0F9F` onward).
* Whether the roll term's dependence on `PitchAccel` is intended.

## Correction: the roll acceleration term

The table at the top of this doc gives the roll term as
`PitchAccel class+1C0h * m * RollAccel class+1BCh`, and the paragraph under it flags the dependence
on `PitchAccel` as possibly deliberate, possibly a shipped slip. Both are wrong, and the error is
mine rather than the game's.

`007DAA9D FSTP [ESP+4Ch]` stores **and pops**. After it, `ST0` is `m` again, not the
`PitchAccel * m` product, so `007DAAB3 FMUL [EAX+1BCh]` multiplies `m` by `RollAccel`. The roll
term is `m * RollAccel`, and the three are symmetric:

| axis | term | address |
|---|---|---|
| pitch | `PitchAccel class+1C0h * m` | `007DAA7F`, `007DAA93` |
| yaw | `YawAccel class+1C4h * -m` | `007DAAA1`, `007DAAAD` |
| roll | `RollAccel class+1BCh * m` | `007DAAB3` |

Caught by packet `cc7_plane_control_targets`, which read the same span independently and against
the decompiler's `local_10 = local_44 * *(float *)(iVar1 + 0x1bc)`. There is nothing anomalous here
to explain, and the paragraph inviting someone to look for intent is withdrawn.

## Correction: `007C0F40` is the bomb-load factor

This doc named it `BSP_Plane_PartDamageAuthorityScale` and said so provisionally, on the strength of
the `1.0f` default and the segment's keywords. It is the **bomb-load factor**, and this repository
already had it: `include/bsp/plane_ai_control.hpp:243-250` records `007C0F40` as `1.0f` unless a
weapon slot still carries bomb-class ordnance, then interpolated toward a class field, with turbo
scaling the result. `2Ah` is the general-bomb class id the projectile tables use
(`docs/ORDNANCE_KIND_IDENTITY.md`), which is exactly what the `vtable[210h](2Ah, 0)` test at
`007C0F72` asks each slot.

So the term `007D9A70` multiplies into a plane's rotation authority is **how much bomb load it is
still carrying**: a loaded bomber turns less sharply than one that has dropped. That is a real
gameplay behaviour, and reading it as damage would have put the dependency on the wrong event.

The `1.0f` default at `007C0F41` and the empty-list guard at `007C0F4F` stand as read; only the name
and the reading of `2Ah` change. Worth noting how it was caught: not by reading the arithmetic, but
by another packet quoting a header this one had not checked. The provisional flag did its job, and
a search of the repository before naming would have done it sooner.
