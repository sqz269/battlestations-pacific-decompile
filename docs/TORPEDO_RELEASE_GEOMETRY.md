# The release geometry: forward speed and release altitude

Addresses: 0092D730, 00C31F40, 00C32000, 00C37E50, 009D0484, 009D046A, 009D0491, 009D0497,
009D3433, 009D3489, 009D1647, 00D7A220, 008568E0, 0072F830.

Packet `cc8_torpedo_release_geometry`, owner `agent/cc8-torpedo-drop`, on main `7105166a3`.

Supersession, as asked: `cc8_torpedo_gun_assignment` (committed `7806064db`) is superseded as a
route to the release. The drop reaches the gun spawn through the bot task's release chain, not
through a gunnery-pass fire target. Its finding still stands as the reason nobody should try the
other route: category 0Ah `BOMBPLATFORM` has an empty preference row at `00E0A1F0`, so
`00863990`'s rank test refuses every candidate for it in every mission.

## 1. Forward speed: one body-velocity field, one writer

`0092D730` `BSP_UnitController_GetBodyAxisSpeed`, `float __thiscall(controller)`, `RET 0`, body
`0092D730`-`0092D76E`, result left on the x87 stack and rounded once at `0092D765`:

```
body = controller->+2Ch
axis = 00C32000(body)                     ; the body axis matrix
v    = 00C31F40(body, &scratch)           ; the body's LINEAR VELOCITY
return v[1]*axis[1Ch] + v[0]*axis[18h] + v[2]*axis[20h]
```

`axis+18h`..`+20h` is the third 12-byte row, so the result is the signed speed along one body
axis. The routine reads the body's linear velocity and nothing else. **It does not care what moved
the body.**

That is the whole of the defect. In this host the one body-velocity field,
`slot.motion.linear_velocity`, had exactly one writer: the ship hydro path, through
`set_linear_velocity_00c37e50`. A plane is integrated by the free-flight arm, which kept its
velocity in a second field of its own and never published it. So `unit_forward_speed_0092d730`
answered `0.0` for every flying plane, while answering correctly for every ship. **That is why the
ship path works and the plane path did not**, and why USN02's ship torpedoes swim while USN01's
air-dropped one inherited no velocity and fell straight down.

The fix is to make the plane publish into the same field the image's one body carries, at both
writers of the plane's world velocity: the spawn seed and the free-flight integration step. No new
field, no special case in the reader, and every other consumer of `0092D730` now sees a plane's
speed too.

Verified: at the release instant the routine answered `0.0 m/s` before and `-6.8 m/s` after. The
field is live. The value it carries is a separate problem, in section 3.

## 2. Release altitude: the authored row

The aim tick's altitude floor is `approach+74h + approach+78h`, loaded at `009D1647`-`009D1650`.
The two halves have different sources:

| slot | source | address |
| --- | --- | --- |
| `+74h` `alt_floor_74` | the control block's second altitude at `+398h`, stored only when it is below the double `100` at `00D7A220` | `009D3489` |
| `+78h` `alt_margin_78` | zeroed unless `+3ADh`, then scaled by the config row's `TorpReleaseAlt` | `009D3433`, `009D046A` |

The row is authored in the installed `scripts/datatables/robots.lua`, and its own comment settles
the units. `TorpReleaseAlt` carries `-- M -- ilyen magasrol dobja a torpedot`, "the height it drops
the torpedo from", so it is **metres of release altitude**, not a scale on one:

| row | TorpReleaseAlt | TorpReleaseDistNear | TorpReleaseDistFar |
| --- | --- | --- | --- |
| `SPNormal` | 12 | 450 | 650 |
| `SPVeteran` | 5 | 800 | 1200 |
| `MPNormal` | 10 | 800 | 1200 |
| `MPVeteran` | 10 | 800 | 1200 |

So the native runs in at roughly **5 to 12 metres**. The host commands 700.

What is bound here, and what is not. The seed call site is in this packet's own file, so the
placeholder that stood in `Pilot/Torpedo/CruisingAlt` (500) for both release distances is replaced
by the authored pair, and `alt_margin_78` now takes `TorpReleaseAlt`. What is **not** bound is the
choice of row: the difficulty index at `[[unit+DF4h]+34h]` is unmodelled, so `SPNormal` is picked
and named as a labelled substitution. The `PilotBot` registry is still unreachable from the units
host, and wiring it touches `src/game_hosts.cpp`, which this worker may not edit; naming the
authored values sidesteps that without pretending the registry is wired.

This did **not** bring the aircraft down. The floor is one input to the aim tick's commanded
altitude, and `+74h` is capped below 100 by `009D3489`, so the pair can never command a 700 m
aircraft to 12 m on its own. Whatever holds the run-in at 700 m is upstream of the floor and was
not found in this packet.

## 3. Measurement, and the next gate

USN01, 3200 frames, `--mission-frames 3000` at `0.05` s. The before run is this branch at
`7806064db`, before the merge of main `7105166a3` and before these changes.

| | before | after |
| --- | --- | --- |
| release altitude | 700 m | 700 m |
| release speed, `0092D730` | **0.0 m/s** | **-6.8 m/s** |
| entry speed | 117.2 m/s | 117.4 m/s |
| `MaxWaterHitVel` | 100.0 m/s | 100.0 m/s |
| water-entry breakups | 1 | 1 |
| torpedo drops | 1 | 1 |
| `swims_started` | 0 | 0 |
| torpedo hits, damage | 0, 0 | 0, 0 |

**The next gate is that the aircraft's velocity and its nose have come apart.** `0092D730` is a dot
product of the body velocity with the third pose row. It returns `-6.8 m/s` at the release instant,
while the same mission seeds every plane at `141.67 m/s` along its forward axis and reports
`distance_moved = 2132014 m` over 60000 steps with `pose_rotations = 28129` and
`heading_change = 28.565 rad`. A dot product near zero against a velocity of that magnitude means
the two vectors are roughly perpendicular: the pose is being rotated without the velocity following
the nose. The sign says the aircraft is drifting very slightly backwards along its own axis.

Two numbers settle it, and neither was taken here: the magnitude of `plane_world_velocity` at the
release instant, and the angle between it and `pose_row2`. If the magnitude is near 141 m/s the
fault is in the free-flight arm's coupling of rotation to velocity; if it is near zero the fault is
that the aircraft has been decelerated to nothing and the pose spin is incidental.

Note the arithmetic that follows either way. At the authored 12 m release the fall contributes
`sqrt(2 * 9.81 * 12)` = 15.3 m/s. A torpedo dropped at a real run-in speed of about 50 m/s enters
at about 52 m/s, comfortably under the 100 m/s limit. A torpedo dropped at the 141.67 m/s seed
enters at about 142 m/s and breaks up anyway. **So altitude alone is not sufficient: the run-in
speed has to come down too**, which is `Pilot/Torpedo/ReferenceSpeed` and `desc.MaxSpd`, the second
blocker `docs/TORPEDO_RUN_PROFILE.md` records and which is still unloaded.

## Host methods

| host method | file | native | kind |
| --- | --- | --- | --- |
| plane body-velocity publish, spawn seed and integration | `src/game_hosts_units.cpp` | `00C31F40` read by `0092D730` | binding |
| the run-profile seed from the authored row | `src/game_hosts_units.cpp` | `009D0484`, `009D046A` | **substitution**, row named |
| `kTorpReleaseAltSPNormal`, `kTorpReleaseDistNearSPNormal`, `kTorpReleaseDistFarSPNormal` | `include/bsp/torpedo_release_spawn.hpp` | authored content | data |

## Corrections

Appended, not rewritten.

* `docs/TORPEDO_RELEASE_SPAWN.md`, "The next gate is release altitude": its second item, that
  `unit_forward_speed_0092d730` returning 0.0 is "a host gap, not an image fact", was right, and
  the gap is now named exactly: one body-velocity field with only a ship writer. It is fixed. Its
  first item, release altitude, stands and now has the authored numbers, 5 to 12 m against the
  host's 700.
* `docs/TORPEDO_RUN_PROFILE.md`, the contract section: blocker 1, the unreachable `PilotBot`
  registry, is worked around rather than closed. The three values are named from the authored
  `SPNormal` row instead of read from the registry, and the difficulty index is still unmodelled,
  so the row choice is a substitution. Blocker 2, `desc.MaxSpd`, is untouched and now matters more
  than it did: it is the run-in speed, and section 3 shows altitude alone cannot clear the
  water-entry limit without it.

## no_ghidra_function

None. `0092D730` has a Ghidra function and a name.

## Follow-up packets

1. **The velocity-nose divergence.** Log `|plane_world_velocity|` and the angle to `pose_row2` at
   the release instant, then read the free-flight arm's rotation handling. This is the gate.
2. **The run-in speed.** Load `desc.MaxSpd` and bind `Pilot/Torpedo/ReferenceSpeed` so the bomber
   slows for its run. Without it no release altitude clears `MaxWaterHitVel`.
3. **What holds the run-in at 700 m.** The floor pair cannot command it, so the commanded altitude
   comes from somewhere this packet did not read.
4. **The difficulty index** at `[[unit+DF4h]+34h]`, which would retire the row-choice substitution.
