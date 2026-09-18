# The torpedo run profile: where `approach+14h` and `approach+24h` come from

Addresses: 009F9CE0, 009F9CFF, 009F9D08, 009F9D0E, 009F9D11, 009F9D18, 009F9D1E,
009F9D22, 009F9D30, 009F9D37, 009F9D3D, 009F9D4B, 009F9D61, 009F9D6E, 009F9D75,
009D0380, 009D03A1, 009D03A6, 009D03B7, 009D0443, 009D046A, 009D0484, 009D0491,
009D0494, 009D0497, 009D1500, 00997B7A, 00997B9D, 00997BB0, 00997BD3, 00997C09,
007D23A1, 00F8A30C, 00D20198, 00CE4BC4.

Packet `cc8_torpedo_run_profile`. The steering packet left one thing unread: nothing in
`BSP_BotApproachTorpedo_Reset` writes `approach+14h` or `approach+24h`, so no value for
the two slots it scales had any evidence behind it. Both are now read, and the answer
**changes what those slots are**. They are not speeds.

## The producer

`BSP_BotApproach_ConstructSpeedReference` (`009F9CE0`-`009F9D77`, `__thiscall(approach)`,
`RET 8`, so two stack arguments) sets both. `BSP_BotApproachTorpedo_Reset` is its only
torpedo caller, at `009D03B7`.

### `approach+14h`, the profile record

```
009f9d08  mov  edx, [eax+0DF4h]      ; the bot object on the unit
009f9d0e  mov  edx, [edx+34h]        ; its difficulty level index
009f9d11  imul edx, edx, 248h        ; sizeof(PilotBotParameters)
009f9d18  mov  esi, [00F8A30C]       ; the PilotBotConfig
009f9d1e  lea  edx, [edx+esi+0Ch]    ; offsetof(PilotBotConfig, levels)
009f9d22  mov  [ecx+14h], edx
```

So `approach+14h` is `&PilotBotConfig.levels[[[unit+DF4h]+34h]]`. Both constants are
independently corroborated: `include/bsp/robot_config.hpp` already carries
`static_assert(sizeof(PilotBotParameters) == 0x248)` and
`static_assert(offsetof(PilotBotConfig, levels) == 0x0c)`, written by an earlier packet
that never connected the struct to this consumer. The array is six levels, so the index is
a difficulty or skill level.

### `approach+24h`, the scale

```
009f9d30  mov   eax, [eax+538h]      ; the aircraft descriptor
009f9d37  fld   dword ptr [eax+188h] ; MaxSpd
009f9d3d  fdiv  dword ptr [esp+8]    ; / the second argument
009f9d45  fld1
009f9d4b  fcompi st(1)
009f9d4f  jbe   0x9f9d59             ; quotient <= 1.0 -> take the 1.0 at 00D7A24C
009f9d61  movss dword ptr [ecx+24h], xmm0
```

`approach+24h = max(1.0, desc.MaxSpd / arg2)`.

`desc+188h` is `MaxSpd`. The class loader pushes each Lua key immediately before its store,
and the key pushed before `007D23A1 FSTP [ESI+188h]` is the string `"MaxSpd"`. The
neighbours are a positive control, because they match names this repository already
carries: `+184h StallSpd`, `+1A8h RollSpd`, `+1ACh PitchSpd`, `+1B0h YawSpd`,
`+1C8h TurnRollSpd`.

`arg2` is the tuning singleton's `+440h`. The reset pushes it:

```
009d03a1  call 0042E740              ; the tuning singleton
009d03a6  fld  dword ptr [eax+440h]
009d03b1  fstp dword ptr [esp]       ; the float argument
009d03b4  push eax                   ; the unit, below it
009d03b7  call 009F9CE0
```

`src/game_tuning_singleton.cpp` already names `+440h` as `Pilot/Torpedo/ReferenceSpeed`,
default `KMH(300)`, which is 83.33 m/s. So the scale is "how much faster than the reference
this aircraft is", floored at 1.

## What the two slots actually are

`009D0484`-`009D0497` reads the record and stores the pair, and `src/robot_config.cpp`
already names the row's fields from the Lua keys the loader pushes at `00997B7A`,
`00997BB0` and `00997BE6`:

| Record offset | Row field | Lua key | Goes to | Read at |
|---|---|---|---|---|
| `+0h` | `torp_release_alt_00c` | `TorpReleaseAlt` | scales `approach+78h` | `009D046A` |
| `+4h` | `torp_release_dist_near_010` | `TorpReleaseDistNear` | `approach+7Ch` | `009D0491` |
| `+8h` | `torp_release_dist_far_014` | `TorpReleaseDistFar` | `approach+80h` | `009D0497` |
| `+Ch` | `torp_release_drop_closer_mul_018` | `TorpReleaseDropCloserMul` | the next slot | `009D049D` |

**`approach+7Ch` and `+80h` are release distances in metres, not speeds.** The `+0h` entry
is the corroboration: it scales `approach+78h`, and `+78h` with `+74h` is exactly the pair
the aim tick loads as the altitude floor at `009D1647`-`009D1650`. An altitude key feeding
an altitude slot in the same routine that feeds the two distance keys into `+7Ch` and `+80h`
leaves no room for the group to be anything else.

The 15-second switch then reads naturally: `009D1525` takes `+7Ch` once
`elapsed_134 >= 15`, so the run starts on the **Far** distance and tightens to **Near**.
And the scale makes physical sense in a way it never did as a speed multiplier: a faster
aircraft needs to release further out.

## The consequence for `009D1500`

`torpedo_time_to_target_009d1500` is misnamed. It divides `approach+90h`, a range in
metres, by whichever release distance the switch selects, so it returns a **range ratio**:
how many release distances out the aircraft still is. Its three arms read as a ratio, not a
time:

| Arm | Condition | Result |
|---|---|---|
| `009D154E` | ratio <= 1.0 | the ratio |
| `009D1570` | release distance >= 600.0 (`00CE4BC4`) | the ratio |
| `009D1598` | otherwise | `(range - distance)/600.0 + 1.0` |

This retires the open question the steering packet left. The aim tick's clause 2 compares
this against a steering delta in radians, which looked like a unit error and is not one:
both sides are dimensionless. The `600.0` at `00D20198` is a distance scale, not a speed,
which is why no aircraft speed ever made sense of it.

## ABI

| Routine | Convention | Arguments | Result |
|---|---|---|---|
| `009F9CE0` | `__thiscall`, ECX = approach | `[ESP+4]` unit, `[ESP+8]` float reference speed | EAX = the approach, `RET 8` |
| `009D0380` | `__thiscall`, ECX = approach | one stack argument, the unit | `009D0380`-`009D066F` |
| `009D1500` | `__thiscall`, ECX = approach | none | ST0, three `RET`s at `009D15B0`, `009D15B8`, `009D15C0` |

## Corrections

To append to `docs/TORPEDO_STEERING_DELTA.md`, not to rewrite: its "Correction" section
says the right magnitude for the two slots cannot be argued from dimensions. That is now
settled the other way. They are distances in metres, so the placeholder of 500 is at least
the right **kind** of quantity and a plausible magnitude, which is why the host behaved
sensibly with it. What was wrong was the source, `Pilot/Torpedo/CruisingAlt`, not the
scale of the number. Its "blocking follow-up" section is closed by this packet.

To append to `docs/TORPEDO_AIM_TICK.md`: `F=0Ch` is a range ratio, not a time to target,
and every gate interpolated over it (the cone at `009D21AC`/`009D21F2`, the countdown at
`009D229D`, the sector gain at `009D17D7`) is scheduled on range-in-release-distances
rather than on seconds.

## Host methods

| Host method | Native | State |
|---|---|---|
| `bsp::torpedo_seed_run_speeds_009d0484` | `009D0484`-`009D0497` | reconstructed; comment corrected, name still says speed |
| `bsp::torpedo_time_to_target_009d1500` | `009D1500` | reconstructed and faithful; comment corrected, name still says time |
| `bsp::PilotBotParameters` | the row | already in `include/bsp/robot_config.hpp` with the right names and size |
| `Pilot/Torpedo/ReferenceSpeed` | tuning `+440h` | already in `src/game_tuning_singleton.cpp` |

## Contract: what binding the real values still needs

Two pieces are missing, and neither is reachable from this packet's lease.

1. **The units host cannot reach the `PilotBot` registry.** `src/robot_config.cpp` builds
   the descriptor and `src/global_subsystems.cpp` loads it, but `GameUnitsHost` has no
   reference to `RobotConfigRegistry`, so the seed site cannot read
   `levels[idx].torp_release_dist_near_010`. Wiring it touches `src/game_hosts.cpp`, which
   this worker is not permitted to edit.
2. **`desc.MaxSpd` is not loaded.** `src/game_hosts_lua.cpp:244` reads `StallSpd` into the
   plane row; there is no `MaxSpd`. Adding it is a two-line change to that file and its
   header, both currently unleased.

The difficulty index `[[unit+DF4h]+34h]` is also unmodelled, so a binding would have to
pick a level and say which.

Until both land, the seed keeps its labelled placeholder. It is no longer known to be the
wrong kind of quantity, only to come from the wrong key.

## no_ghidra_function

None. `009F9CE0`, `009D0380`, `009D1500`, `009973B0` and `00901610` all have Ghidra
functions and names.

## Uncertainty

- The difficulty index at `[[unit+DF4h]+34h]` is read but its producer is not. The six
  levels are asserted to be difficulty or skill tiers from the array size and the
  `PilotBot` descriptor name, not from a writer. Partial.
- `009D049D` reads `record+Ch`, `TorpReleaseDropCloserMul`, into a slot this packet did
  not follow.
- No run was taken for this packet, because nothing in it changes behaviour: the code
  edits are comments and the binding is a contract. The steering packet's runs stand.

## Follow-up packets

1. **Bind the pair.** Both contract items above, then USN01 before and after with the aim
   census, watching `F0C`, the cone, and the release.
2. **Rename.** `speed_late_7c`, `speed_early_80`, `torpedo_commanded_speed_009d3c99`,
   `torpedo_seed_run_speeds_009d0484` and `torpedo_time_to_target_009d1500` all say speed
   or time and mean distance or ratio. The rename reaches `src/torpedo_aim_tick.cpp`,
   `src/torpedo_task_arm.cpp` and `src/game_hosts_units.cpp` together, so it wants its own
   packet and its own build.
3. **The difficulty index.** Find the writer of `[unit+DF4h]+34h` and whether the mission
   or the profile picks the level.
