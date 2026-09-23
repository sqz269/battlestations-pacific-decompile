# The dogfight moveto's speed slot, the cruise profile, and PilotFires

Addresses: 009C1BC0, 009BECD0, 007EF2C0, 007BCC20, 00999AE0, 009A9C60, 009BE3E0, 007B4E90,
009AAF30, 007CD930.

Packet `cc9_dogfight_moveto`. Every name is a hypothesis, not a recovered symbol.

## 1. The speed chain, read from the listings

**`009C1BC0`** (vtable `00D20B24 +1Ch`, `__thiscall(state, float sep)`, `RET 4`) writes
`plan+2B4h = 009BECD0(classBlock+188h MaxSpd, 007C47F0(), sep)` (`009C1C07`), then
`plan+2B0h = 0` (byte, `009C1C0D`) and `plan+2D8h = 1` (dword, `009C1C14`).
`007C47F0` is `LevelFlight (tuning+24Ch) * StallSpd (class+184h)`, with no arguments.
`sep` is the moveto target's planar range, the argument `009C18C0` passes (`009C198A`-`009C1999`).

**`009BECD0`** (`__thiscall(state, a, b, sep)`, `RET 0Ch`):
* `k = interp(WaitDist1 (tuning+5CCh, 3000), 0, WaitDist2 (+5D0h, 5000), 0.5, sep)`, where
  0.5 is `00CE3800` (`009BED0F`).
* With a squadron (`approach+0Ch`) and a class (`approach+8h`) (`009BED18`-`009BED28`):
  `m = max(k, 007EF2C0(squadron))` (`009BED35`-`009BED4C`), and the result is
  `b + (a - b) * m` (`009BED4C`-`009BED66`).
* Otherwise the result is `a`.

So the speed is `LF * StallSpd` when `m = 0` and `MaxSpd` when `m = 1`.

**`007EF2C0`** (`__fastcall(squadron)`, plain `RET`):
* 1.0 when `+3CCh <= 1` or `+3E4h` (the formation shape) is 0.
* Otherwise `min(1.0, the smallest non-negative member+9C4h[step*8])` over members 1..count-1.
  The loop is unrolled by four (`007EF330`-`007EF3E1`) with a tail (`007EF410`-`007EF440`).
  Negatives are skipped by `COMISS`/`JC`.

**What a member publishes.** `007CDCE7` publishes `007BCC20(member)`:
* -1.0 when the member is dead, squadronless, formation index 0 or pilotless;
* 0.0 when `unit+72Ch`'s `vtable[38h]` is false;
* otherwise `00999AE0(pilot)`: the first task whose `vtable[34h]` is false answers
  `vtable[4Ch]`.

For the dogfight task, `+34h` is `0099B700` (`XOR AL,AL`) and `+4Ch` is `009A9C60`. That is
`009BE3E0(follow)` while the task is in follow (`+310h == +54Ch`), else `0099B720` (-1.0).

**`009BE3E0`** (the follow state's wait value):
* 1.0 when `state+85h` is set.
* `ang = |wrap(heading(member - station) - leader heading)|`, from `007B4E90`, which is
  `pi/2 - atan2` wrapped into [0, 2pi), then `00438B10`.
* `v = interp(DontWaitForHdgDiff (+3ACh, 50 deg), 0, WaitForHdgDiff (+3B0h, 100 deg), 1, ang)`.
* When `v < 1`: `v = min(1, v + interp(GoodPositionDist (+398h, 100), 1, NearbyDist (+3B4h, 200),
  0, horizontal |member - station|))`.

**`009AAF30`** (the cruise profile). For the flight leader, it sets `squadron+3ADh = 1` and
`squadron+394h = CruisingAlt` (`tuning+640h`, 1400) when `+38Dh` is clear, `+380h < 0` and
`+3A9h` is clear. It then clears `+3A9h`, runs `009AAC70(dt = 0)` and tail-jumps to `0099B740`.
It writes **no speed**. `squadron+394h` is the shared altitude ceiling the moveto reads; the
stand-in already pitches toward CruisingAlt, so it gets no new binding.

## 2. PilotFires

`007CD930` (called once from `BSP_Plane_ReadPropertyBag`) sets `unit+C24h` when any
`IsKindOf(20h)` part's platform entry `[[unit+538h]+94h][gun+38Ch]+0Ch` is set. That table is the
class's authored `Platforms`: each `VehicleClass[id].Platforms[k]` lists its guns in `.Gun` and
carries `PilotFires`.

`kPilotFiresBound` (true):
* A one-time Lua flatten writes `BSPPilotFires = 1` when any platform with at least one gun
  authors `PilotFires = true`. The slot reads it at class load.
* It gates the dogfight gun controller (`00999962`) and feeds `007BB920`'s `IsKindOf(17h)`
  throttle override.
* **Labelled:** the host cannot match each part to its platform, so "any platform with a gun"
  stands in for "any gun part's platform".

## 3. The binding (kDogfightMovetoSpeedBound = true)

The moveto stand-in now writes the chain's speed with `+2B0h = 0` and `+2D8h = 1`.
* Wing members publish their follow value from the station the host places them on and the
  leader's heading.
* `state+85h` is not modelled and is taken as clear.
* The `unit+72Ch` `vtable[38h]` gate is not modelled and is taken as true.

## 5. Predictions, written before runs M0/MA/MB

All runs are USN04 at the E2 parameters. M0 is this tree with both new switches off. MA turns
on the speed chain and PilotFires. MB is MA plus `kDogfightThrottleBound`.

1. **MA.** The moveto leaders now command a speed.
   * The range to their Val targets is mostly beyond 5000 m before engagement, so
     `k = 0.5`. After engagement the leader is at about 2150 m, so `k = 0`.
   * The wingmen value: well-placed wingmen give 1, so the leader flies at MaxSpd. A lagging
     wingman gives 0, so the leader slows to `LF * StallSpd`.
   * Expect the Yorktown leader to fly faster between engagements and not to stall in moveto.
     `.-2` is in follow, which already has speed, so MA alone does not change its stall.
   * PilotFires is true for the fighters, so the gun is unchanged.
2. **MB.** The leader survives through the moveto speed. `.-2` may still drown, because its
   stall happens in follow.
3. **Other rows** change only through the fighters' paths: recon and AA, which are RNG-coupled.

## 6. Runs

All runs are USN04 at the E2 parameters, each from its own freshly copied binary.

| run | switches | log | result |
| --- | --- | --- | --- |
| M0 | speed chain and PilotFires off | `local\M0_9000.log` | control |
| MA | on | `local\MA_9000.log` | every per-entity row identical to M0 |
| MB | on + `kDogfightThrottleBound` | `local\MB_9000.log` | Yorktown leader and `.-2` drown at \|v\| 50.94 / 51.18 |

**MA against M0: identical.**
* The summary rows, all 8 water contacts, the killed_by table, the dive-bomb rows and every
  dogfight row match. The only new line is the native row for `009C1BC0` (8041 calls).
* Both leaders command 83.3 m/s on every moveto tick: 4214 ticks for Lexington and 3827 for
  Yorktown. That is a constant, so `m` never moved off one end of the blend.
* No path changed. Whatever speed the planner was already flying toward in moveto gives the same
  motion as the chain's value.
* PilotFires is 1 for all six fighters, so the gun gate is unchanged.
* Prediction 1 was right about the command and wrong about an effect.

**MB, the stall.**
* The leader drowns at |v| 50.94 **while commanding 83.3 m/s in moveto**, and `.-2` at 51.18 in
  follow. These are the same values as F1, T1, HT and A1T.
* The moveto speed command was not the cause, and neither were the re-seed, the head-on
  classification or the missing moveto speed.
* The speeds are identical across five different treatments. That points at, but does not
  prove, a state that the direct-throttle arm (`007B4ED0`: throttle slot active in mode 0)
  leaves behind in this host's throttle path, and that the speed-mode commands that follow do
  not clear.
* Candidates: the throttle slot's `current` (the slew state) and `plane_throttle_last`. **Not
  traced this packet.**
* Prediction 2 was wrong.

## 7. Decisions

* **Land** `kDogfightMovetoSpeedBound` and `kPilotFiresBound`, both true and neutral on USN04.
* The throttle wiring stays off.
* **Read-only:**
  * `unit->vtable[1FCh](gunFire)` at `007CE98D` is `007CA5F0`. It calls the base `00956EE0(gunFire)`
    and, while firing with `unit+A34h` clear, walks a class array at `+238h` (count `+23Ch`).
    Both are unread, so it is partly read.
  * The weapon-slot class at `unit+974h` was not reached.
