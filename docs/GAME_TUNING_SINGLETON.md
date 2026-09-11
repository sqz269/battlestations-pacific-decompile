# The game tuning singleton

Addresses: 0042E740 007E2A20 008875F0 00887670 007D1F70

`0042E740` returns the process-wide tuning object cached in `DAT_00F87440`. The object is
`6D0h` bytes from `operator new` and is built once by `007E2A20`, which runs two Lua scripts in a
private state and copies several hundred balance values out of the resulting `PlaneGlobals`
table. The getter has 141 call sites; the object is the game's plane tuning block.

## What the loader actually loads

The packet brief and the existing note in `docs/PLANE_CLASS_FIELDS.md` say the loader reads
`Scripts\global\luaMW_init.lua`. It runs that script, but the keys do not come from it.
`luaMW_init.lua` (533 lines in the installed tree) defines only constants: party and player
enumerations, role codes, colour tables, and the `DEG` and `KMH` helpers. The loader runs it
first at 007E2AE9 so that those helpers exist, then runs
`Scripts\datatables\PlaneGlobals.lua` at 007E2B5D, fetches the globals table at 007E2B98 and
fetches the single global `PlaneGlobals` at 007E2BB4. **Every key in the table below is a path
inside `PlaneGlobals`, from `Scripts\datatables\PlaneGlobals.lua`, not from `luaMW_init.lua`.**

Evidence: the two `00B69D40` sites at 007E2AE9 and 007E2B5D with their literal paths, and the
`00B67980` then `00B67800` pair at 007E2B98..007E2BB4 whose key string at `00d085f4` is
`"PlaneGlobals"`.

## Reading the scopes out of the listing

The decompiled body is unusable for the table structure. `LuaObject::GetByName` (`00B67800`) is
`__thiscall(this = parent table, out = first stack argument, name = second)` and returns `out`
in EAX, so the **parent table is the hidden ECX** and Ghidra prints only `(out, name)`. Reading
the pseudocode alone makes every key look like a sibling at the top level, and it made the
`Pilot`/`Dynamics` pair at 007E2C85..007E2CD1 look like nesting when the listing shows
`Pilot` fetched into one slot and the section scope then *reassigned* to `PlaneGlobals.Dynamics`.

The table below was produced from the listing (`local/scope2.py` in this worktree): each
`LEA r,[ESP+X]` is normalised to a frame slot by subtracting the pushes made since the previous
call, since every callee here is `__thiscall` or `__stdcall` and cleans its own arguments, and
the slot-to-path map is propagated through `GetByName`, `GetByIndex` and `LuaObject::operator=`.
Key strings are read from the pushed immediate in the image rather than zipped against the
decompiler's statement order. Caller-saved EAX/ECX/EDX mappings are dropped at every call; not
doing so silently attributed the whole `PlaneCamera` section to `DeathModeChances/Spinning`.

## The object

| | |
|---|---|
| size | `6D0h` (`operator new(0x6d0)` at 0042E7A2) |
| `+0h` | vtable `00d08628`, stored at 007E2A4C |
| construction | `__thiscall` `007E2A20`, `this` in ECX from the allocation, returns `this` |
| cache | `DAT_00F87440`, double-checked under the singleton manager's critical section |
| lifetime | registered with `BSP_SingletonLifetime_Register` (`00BD0C30`) at 0042E7D9 |
| stores by the loader | 439, covering 436 distinct offsets |
| distinct key paths read | 423 |

**There are no constructor defaults.** The block from `operator new` is not zeroed, the loader
writes the vtable and then goes straight into the script run, and every other field is written
only by its own key read. A key that is absent from the shipped table therefore leaves either
the `00B66330` fallback (the 37 `number-or-default` rows) or, for a bare `00B66270` read, the
value that wrapper returns for a nil object. The `4` bytes the loader never writes are listed
under *Unwritten offsets* below.

## Offsets, keys and consumers

`type` is the native getter: `number` is `00B66270`, `integer` `00B66290`, `boolean` `00B66250`,
`number-or-default` `00B66330` with the float shown, `number[3]` the array helper `00B67A80`,
and `derived` a store with no key behind it. `shipped value` is the value in the installed
`Scripts\datatables\PlaneGlobals.lua`, quoted as written (`DEG(x)` and `KMH(x)` are the
`luaMW_init.lua` helpers). `consumers` lists the callers of `0042E740` that reach that offset.

| offset | luaMW_init key path | type | native default | shipped value | read site | consumers |
|---|---|---|---|---|---|---|
| `+0` | `(derived)` | derived | - | - | `007e2a4c` | - |
| `+4` | `CloseToCameraDist` | number | - | 1500.0 | `007e2bed` | - |
| `+8` | `MoveDetailLODError` | number | - | 0.8 | `007e2c26` | - |
| `+c` | `MoveDetailRadius` | number | - | 25.0 | `007e2c5f` | - |
| `+10` | `DeathModeChances/Explosion` | number | - | 0.2 | `007e428c` | `007ca8a0` |
| `+14` | `DeathModeChances/Explosion_delayed` | number | - | 0.2 | `007e42c7` | `007ca8a0` |
| `+18` | `DeathModeChances/Spinning` | number | - | 0.3 | `007e433d` | `007ca8a0` |
| `+1c` | `DeathModeChances/Powerloss` | number | - | 0.4 | `007e4302` | `007ca8a0` |
| `+20` | `DeathModeChances/Explodetoparts` | number | - | 0.2 | `007e4251` | - |
| `+24` | `Sound/WindVolMinSpdRatio` | number | - | 0.4 | `007e6811` | `007eb380` |
| `+28` | `Sound/WindVolMaxSpdRatio` | number | - | 0.9 | `007e684f` | `007eb380` |
| `+2c` | `Sound/WindPitchMinSpdRatio` | number | - | 0.8 | `007e688d` | `007eb380` |
| `+30` | `Sound/WindPitchMaxSpdRatio` | number | - | 2.5 | `007e68cb` | `007eb380` |
| `+34` | `Sound/WindPitchMin` | number | - | 0.9 | `007e6909` | `007eb380` |
| `+38` | `Sound/WindPitchMax` | number | - | 1.2 | `007e6947` | `007eb380` |
| `+3c` | `Sound/FallVolMinSpdRatio` | number | - | 1.2 | `007e6985` | `007eb380` |
| `+40` | `Sound/FallVolMaxSpdRatio` | number | - | 1.8 | `007e69c3` | `007eb380` |
| `+44` | `Sound/FallPitchMinSpdRatio` | number | - | 1.3 | `007e6a01` | `007eb380` |
| `+48` | `Sound/FallPitchMaxSpdRatio` | number | - | 4.0 | `007e6a3f` | `007eb380` |
| `+4c` | `Sound/FallPitchMin` | number | - | 0.8 | `007e6a7d` | `007eb380` |
| `+50` | `Sound/FallPitchMax` | number | - | 1.2 | `007e6abb` | `007eb380` |
| `+54` | `PlaneCamera/ZRotMul` | number | - | 0.3 | `007e43b6` | `0042f800` |
| `+58` | `PlaneCamera/ZRotSmoothRate` | number | - | 1.75 | `007e43f1` | `0042f800` |
| `+5c` | `PlaneCamera/XDistMul` | number | - | 5.0 | `007e442c` | `0042f800` |
| `+60` | `PlaneCamera/XDistSmoothRate` | number | - | 5.0 | `007e4467` | `0042f800` |
| `+64` | `PlaneCamera/YDistMul` | number | - | 2.0 | `007e44a2` | `0042f800` |
| `+68` | `PlaneCamera/YDistSmoothRate` | number | - | 7.0 | `007e44dd` | `0042f800` |
| `+6c` | `PlaneCamera/YDistRollMul` | number | - | .5 | `007e4518` | `0042f800` |
| `+70` | `PlaneCamera/YDistRollSmoothRate` | number | - | 1.0 | `007e4553` | - |
| `+74` | `PlaneCamera/ZDistMul` | number | - | 0.00 | `007e458e` | `0042f800` |
| `+78` | `PlaneCamera/ZDistSmoothRate` | number | - | 10.0 | `007e45c9` | `0042f800` |
| `+7c` | `PlaneCamera/ShipYardDist` | number | - | 100.0 | `007e4604` | `0060cd50` |
| `+80` | `PlaneCamera/LookAroundSmoothRate` | number-or-default | 10 | 10.0 | `007e4649` | `0042f800` |
| `+84` | `PlaneCamera/CockpitFOVMul` | number-or-default | 0.65 | 0.85 | `007e4691` | `0042f760` `006068a0` `00799de0` `007c7800` |
| `+88` | `PlaneCamera/CockpitSmooth` | number-or-default | 0.5 | 0.5 | `007e46d9` | - |
| `+8c` | `PlaneCamera/CockpitYawTurn` | number-or-default | 0.1 | DEG(5) | `007e4721` | - |
| `+90` | `PlaneCamera/CockpitPitchTurn` | number-or-default | 0.1 | DEG(5) | `007e4769` | - |
| `+94` | `PlaneCamera/CockpitRollTurn` | number-or-default | 0.05 | DEG(14) | `007e47b1` | - |
| `+98` | `PlaneCamera/CockpitRollHTurn` | number-or-default | 0.05 | DEG(9) | `007e47f9` | - |
| `+9c` | `PlaneCamera/CockpitRollVTurn` | number-or-default | 0.05 | DEG(2) | `007e4841` | - |
| `+a0` | `PlaneCamera/CockpitViewHMax` | number-or-default | 1.5 | DEG(80) | `007e4889` | - |
| `+a4` | `PlaneCamera/CockpitViewVMax` | number-or-default | 1 | DEG(70) | `007e4915` | - |
| `+a8` | `PlaneCamera/CockpitViewVMin` | number-or-default | 0.7 | DEG(35) | `007e48d1` | - |
| `+ac` | `PlaneCamera/CockpitMaxHeadMoveDist` | number-or-default | 0.08 | 0.06 | `007e495d` | - |
| `+b0` | `PlaneCamera/CockpitGunfireEffectSize` | number-or-default | 0.008 | 0.005 | `007e49a5` | - |
| `+b4` | `PlaneCamera/CockpitGunfireEffectTime` | number-or-default | 0.1 | 0.02 | `007e49ed` | - |
| `+b8` | `PlaneCamera/FOVMinSpeed` | number | - | 80 | `007e4a2b` | - |
| `+bc` | `PlaneCamera/FOVMaxSpeed` | number | - | 100 | `007e4a69` | - |
| `+c0` | `PlaneCamera/FOVMinSpdMul` | number | - | 1.0 | `007e4aa7` | - |
| `+c4` | `PlaneCamera/FOVMaxSpdMul` | number | - | 1.3 | `007e4ae5` | - |
| `+c8` | `PlaneCamera/FOVMinAccel` | number | - | 0.5 | `007e4b23` | - |
| `+cc` | `PlaneCamera/FOVMaxAccel` | number | - | 8.0 | `007e4b61` | - |
| `+d0` | `PlaneCamera/FOVAccelMul` | number | - | 1.0 | `007e4b9f` | - |
| `+d8` | `PlaneCamera/TurboMotionBlurMinSpeed` | number-or-default | 80 | 80 | `007e4be7` | `00608c30` `0067b7e0` |
| `+dc` | `PlaneCamera/TurboMotionBlurMaxSpeed` | number-or-default | 100 | 100 | `007e4c2f` | - |
| `+e0` | `PlaneCamera/TurboMotionBlurMaxBlur` | number-or-default | 0.1 | 0.1 | `007e4c77` | - |
| `+e4` | `BombCamera/MinCameraAlt` | number | - | 2 | `007e4cf3` | - |
| `+e8` | `BombCamera/CameraPosSmooth` | number | - | 0.75 | `007e4d31` | `00430da0` |
| `+ec` | `BombCamera/CameraPosSmooth` | number | - | 0.75 | `007e4d6c` | - |
| `+f0` | `BombCamera/CamVelBlenderAcceleration` | number | - | 0.005 | `007e4da7` | `00430970` |
| `+f4` | `BombCamera/MaxCamVelBlender` | number | - | 0.5 | `007e4de5` | `00430970` |
| `+f8` | `BombCamera/TorpedoNearAlt/2` | number | - | 30 | `007e4ea4` | - |
| `+fc` | `BombCamera/TorpedoNearAlt/1` | number | - | 80 | `007e4e3c` | - |
| `+100` | `BombCamera/TorpedoFollowDistNear` | number | - | 30 | `007e4ef0` | - |
| `+104` | `BombCamera/TorpedoFollowDistFar` | number | - | 80 | `007e4f2b` | `00430970` |
| `+108` | `BombCamera/RocketNearAlt/2` | number | - | 30 | `007e4fed` | - |
| `+10c` | `BombCamera/RocketNearAlt/1` | number | - | 80 | `007e4f82` | - |
| `+110` | `BombCamera/RocketFollowDistNear` | number | - | 30 | `007e503f` | - |
| `+114` | `BombCamera/RocketFollowDistFar` | number | - | 80 | `007e507d` | `00430970` |
| `+118` | `BombCamera/BombNearAlt/2` | number | - | 120 | `007e513f` | - |
| `+11c` | `BombCamera/BombNearAlt/1` | number | - | 380 | `007e50d4` | - |
| `+120` | `BombCamera/BombFollowDistNear` | number | - | 40 | `007e5191` | - |
| `+124` | `BombCamera/BombFollowDistFar` | number | - | 160 | `007e51cf` | `00430970` |
| `+128` | `BombCamera/BulletNearAlt/2` | number | - | 120 | `007e5291` | - |
| `+12c` | `BombCamera/BulletNearAlt/1` | number | - | 380 | `007e5226` | - |
| `+130` | `BombCamera/BulletFollowDistNear` | number | - | 40 | `007e52e3` | - |
| `+134` | `BombCamera/BulletFollowDistFar` | number | - | 160 | `007e5321` | - |
| `+138` | `BombCamera/FinalDist` | number | - | 210 | `007e535f` | `00430970` |
| `+13c` | `BombCamera/BombSubDistLimit` | number | - | 30 | `007e539d` | `0042f800` |
| `+140` | `BombCamera/BombSubDistMin` | number | - | 5 | `007e53db` | `0042f800` |
| `+144` | `BombCamera/BombSubDistMul` | number | - | 0.25 | `007e5419` | `0042f800` |
| `+148` | `BombCamera/TorpedoSubDist` | number | - | -7.5 | `007e5457` | `0042f800` |
| `+14c` | `BombCamera/TorpedoSubDist2` | number | - | -7.5 | `007e5495` | `0042f800` |
| `+150` | `BombCamera/TorpedoSubAngle` | number | - | -0.1 | `007e54d3` | `0042f800` |
| `+154` | `BombCamera/TorpedoSubAngle2` | number | - | -0.2 | `007e5511` | `0042f800` |
| `+158` | `MultiPlayer/SyncSendMul` | number-or-default | 1 | 1.0 | `007e5593` | - |
| `+15c` | `MultiPlayer/SyncUploadLimitKBitPerSec` | number-or-default | 1024 | 1024 | `007e55db` | - |
| `+160` | `Rotor/SpeedBase` | number | - | DEG(360.0) | `007e5f09` | - |
| `+164` | `Rotor/StillMultiplier` | number | - | 5.0 | `007e5f47` | - |
| `+168` | `Rotor/SpeedRandom` | number | - | 0.1 | `007e5f85` | - |
| `+16c` | `Rotor/StillOnlySpeed` | number | - | 0.1 | `007e5fc3` | `007ec3b0` |
| `+170` | `Rotor/BlurredOnlySpeed` | number | - | 0.3 | `007e6001` | `007ec3b0` |
| `+174` | `Rotor/IdlePowerSpeed` | number | - | 0.5 | `007e603f` | `007eb380` |
| `+178` | `Rotor/MaxPowerSpeed` | number | - | 1.0 | `007e607d` | `007eb380` |
| `+17c` | `Rotor/RotorSpeedChange` | number | - | 0.2 | `007e60bb` | - |
| `+180` | `AirField/TurnMultiplier` | number | - | 2.2 | `007e6137` | - |
| `+184` | `AirField/MoveSpd` | number | - | KMH(35.0) | `007e61b3` | `009ce2c0` |
| `+188` | `AirField/MinTurnSpd` | number | - | DEG(50.0) | `007e6175` | `009ce2c0` |
| `+18c` | `AirField/PlayerControlSpd` | number | - | KMH(80.0) | `007e61f1` | `007cbfa0` |
| `+190` | `AirField/PlaneSendInterval` | number | - | 2.0 | `007e622f` | `007f1f00` |
| `+194` | `CameraShake/RollPitchMult` | number | - | DEG(0.5) | `007e5657` | `00609bd0` |
| `+198` | `CameraShake/PowerMult` | number | - | DEG(0.5) | `007e5695` | `00609bd0` |
| `+19c` | `CameraShake/SpeedMult` | number | - | DEG(2.0) | `007e56d3` | `00609bd0` |
| `+1a0` | `CameraShake/Limit` | number | - | DEG(1.0) | `007e5711` | `00609bd0` |
| `+1a4` | `CameraShake/Ratio` | number | - | 4 | `007e574f` | `00609bd0` |
| `+1a8` | `CameraShake/PauseLenMin` | number | - | 1 | `007e578d` | - |
| `+1ac` | `CameraShake/PauseLenMax` | number | - | 3 | `007e57cb` | `00609bd0` |
| `+1b0` | `CameraShake/ShakeLenMin` | number | - | 1 | `007e5809` | `00609bd0` |
| `+1b4` | `CameraShake/ShakeLenMax` | number | - | 2 | `007e5847` | - |
| `+1b8` | `CameraShake/RandomLenFactor` | number | - | 0.9 | `007e5885` | `00609bd0` |
| `+1bc` | `CameraShake/ForceShakeLimit` | number | - | 0.9 | `007e58c3` | `00609bd0` |
| `+1c0` | `Wanderer/SpeedRange/1` | number | - | KMH(60) | `007e5958` | `007c4560` |
| `+1c4` | `Wanderer/SpeedRange/2` | number | - | KMH(100) | `007e59c3` | `007c4560` |
| `+1c8` | `Wanderer/RollChangeChance` | number | - | 0.28 | `007e5a91` | `007c4560` |
| `+1cc` | `Wanderer/RollChangeMax` | number | - | 0.5 | `007e5a15` | `007c4560` |
| `+1d0` | `Wanderer/RollChangeDecay` | number | - | 0.4 | `007e5a53` | `007c4560` |
| `+1d4` | `Wanderer/RollChangeSpeed` | number | - | 0.7 | `007e5acf` | `007c4560` |
| `+1d8` | `Wanderer/TimeRange/1` | number | - | 1.0 | `007e5b26` | `007c4560` |
| `+1dc` | `Wanderer/TimeRange/2` | number | - | 3.2 | `007e5b91` | `007c4560` |
| `+1e0` | `Wanderer/OffsetMax` | number | - | 3.5 | `007e5c9d` | `007c4560` |
| `+1e4` | `Wanderer/ChangeMul` | number | - | 0.7 | `007e5be3` | `007c4560` |
| `+1e8` | `Wanderer/AccelMax` | number | - | 0.7 | `007e5c21` | `007c4560` |
| `+1ec` | `Wanderer/SpeedMax` | number | - | 0.8 | `007e5c5f` | `007c4560` |
| `+1f0` | `Wanderer/AccelDecayTime` | number | - | 5.0 | `007e5cdb` | `007c4560` |
| `+1f4` | `Wanderer/SpeedDecayTime` | number | - | 7.0 | `007e5d19` | `007c4560` |
| `+1f8` | `Wanderer/OffsetDecayTime` | number | - | 8.0 | `007e5d57` | `007c4560` |
| `+1fc` | `Wanderer/SmallPlaneDecalMul` | number | - | 1.2 | `007e5d95` | `007c4560` |
| `+200` | `Wanderer/SmallPlaneRollDecayMul` | number | - | 1.8 | `007e5dd3` | `007c4560` |
| `+204` | `Wanderer/SmallPlaneAccelMul` | number | - | 1.4 | `007e5e11` | `007c4560` |
| `+208` | `Wanderer/SmallPlaneTimeMul` | number | - | 0.7 | `007e5e4f` | `007c4560` |
| `+20c` | `Wanderer/SmallPlaneOffsetMul` | number | - | 0.8 | `007e5e8d` | `007c4560` |
| `+210` | `Dynamics/Ceiling` | number | - | 1500 | `007e2d09` | `005241d0` `006c5050` `007b4f60` `009a87f0` `009a8b20` `009bfee0` `009cadb0` `009fb800` `009fba50` `009fc260` |
| `+214` | `Dynamics/CeilingForce` | number | - | 0.1 | `007e2d3d` | - |
| `+218` | `Dynamics/RotationLimit` | integer | - | 0 | `007e2d75` | - |
| `+21c` | `Dynamics/RotationFactors/A` | number | - | 0.5 | `007e3cb9` | - |
| `+220` | `Dynamics/RotationFactors/B` | number | - | 1.5 | `007e3cfb` | - |
| `+224` | `Dynamics/RotationFactors/C` | number | - | 0.1 | `007e3d3d` | - |
| `+228` | `Dynamics/DragFuncPower` | number | - | 1.8 | `007e2dad` | - |
| `+22c` | `Dynamics/SpdMultipliers/StallRangeMin` | number | - | 1.2 | `007e3daa` | `007c6e10` |
| `+230` | `Dynamics/SpdMultipliers/StallRangeMax` | number | - | 1.6 | `007e3de9` | `007c4830` `007c6e10` |
| `+234` | `Dynamics/SpdMultipliers/StallOffPitch` | number | - | DEG(-15) | `007e3e28` | - |
| `+238` | `Dynamics/SpdMultipliers/StallOnPitch` | number | - | DEG(20) | `007e3e67` | - |
| `+23c` | `Dynamics/SpdMultipliers/ControlRangeMin` | number | - | 1.1 | `007e3ea6` | - |
| `+240` | `Dynamics/SpdMultipliers/ControlRangeMax` | number | - | 1.7 | `007e3ee5` | - |
| `+244` | `Dynamics/SpdMultipliers/DragRangeMin` | number | - | 1.0 | `007e3f24` | - |
| `+248` | `Dynamics/SpdMultipliers/DragRangeMax` | number | - | 2.0 | `007e3f63` | - |
| `+24c` | `Dynamics/SpdMultipliers/LevelFlight` | number | - | 1.8 | `007e3fa2` | `007c47f0` `007c4850` `007cba50` |
| `+250` | `Dynamics/DeadMeat/RotationMin` | number | - | DEG(10) | `007e30e2` | - |
| `+254` | `Dynamics/DeadMeat/RotationMin` | number | - | DEG(10) | `007e311b` | - |
| `+258` | `Dynamics/DeadMeat/SpinRollSpd` | number | - | 5.0 | `007e3400` | - |
| `+25c` | `Dynamics/DeadMeat/RollMulTime` | number | - | 6 | `007e318d` | - |
| `+260` | `Dynamics/DeadMeat/RollMul` | number | - | 2.0 | `007e3154` | - |
| `+264` | `Dynamics/DeadMeat/LostDragTime` | number | - | 5 | `007e31c6` | - |
| `+268` | `Dynamics/DeadMeat/ExtraGravityMul` | number | - | 1.5 | `007e31ff` | - |
| `+26c` | `Dynamics/DeadMeat/SpinStallMul` | number | - | 10 | `007e3238` | - |
| `+270` | `Dynamics/DeadMeat/SpinStallMulTime` | number | - | 4 | `007e3271` | - |
| `+274` | `Dynamics/DeadMeat/SpinStallMulOnPitch` | number | - | DEG(0) | `007e32aa` | - |
| `+278` | `Dynamics/DeadMeat/SpinStallMulOffPitch` | number | - | DEG(-75) | `007e32e3` | - |
| `+27c` | `Dynamics/DeadMeat/StallMul` | number | - | 1 | `007e331c` | - |
| `+280` | `Dynamics/DeadMeat/StallMulTime` | number | - | 4 | `007e3355` | - |
| `+284` | `Dynamics/DeadMeat/StallMulOnPitch` | number | - | DEG(10) | `007e338e` | - |
| `+288` | `Dynamics/DeadMeat/StallMulOffPitch` | number | - | DEG(-25) | `007e33c7` | - |
| `+28c` | `(derived)` | derived | - | - | `007e41df` | `007c4810` |
| `+290` | `Dynamics/WheelFriction` | number | - | 0.75 | `007e2de5` | - |
| `+294` | `Dynamics/WheelFrictionSpeed/1` | number | - | 1.0 | `007e2e33` | - |
| `+298` | `Dynamics/WheelFrictionSpeed/2` | number | - | 1.4 | `007e2e92` | - |
| `+29c` | `Dynamics/WheelFrictionAccel/1` | number | - | 0.0 | `007e2ef1` | - |
| `+2a0` | `Dynamics/WheelFrictionAccel/2` | number | - | 8.0 | `007e2f50` | - |
| `+2a4` | `Dynamics/RunwaySmoothStrength` | number | - | 4.0 | `007e2f99` | - |
| `+2a8` | `Dynamics/RunwayYawTurnSpdLimit/1` | number | - | KMH(25) | `007e2fe7` | `009ce2c0` |
| `+2ac` | `Dynamics/RunwayYawTurnSpdLimit/1` | number | - | KMH(25) | `007e3046` | `009ce2c0` |
| `+2b0` | `Dynamics/RunwayYawTurnSpdMul` | number | - | 2.2 | `007e308f` | `009ce2c0` |
| `+2b4` | `Dynamics/Water/MaxVSpd` | number | - | 10 | `007e36a5` | - |
| `+2b8` | `Dynamics/Water/MaxDownPitch` | number | - | DEG(15) | `007e36e4` | - |
| `+2bc` | `Dynamics/Water/MaxUpPitch` | number | - | DEG(50) | `007e3723` | - |
| `+2c0` | `Dynamics/Water/MaxRoll` | number | - | DEG(20) | `007e3762` | - |
| `+2c4` | `Dynamics/Water/YawControlFactor` | number | - | 0.4 | `007e37a1` | - |
| `+2c8` | `Dynamics/Water/NormalYawControlSpd` | number | - | KMH(45) | `007e37e0` | - |
| `+2cc` | `Dynamics/Water/MaxYawControlSpd` | number | - | KMH(18) | `007e381f` | - |
| `+2d0` | `Dynamics/Water/MaxYawControl` | number | - | 6.0 | `007e385e` | - |
| `+2d4` | `Dynamics/Water/MaxDepth` | number | - | 6.0 | `007e3968` | - |
| `+2d8` | `Dynamics/Water/LiftDepthRatio` | number | - | 1.2 | `007e38ea` | - |
| `+2dc` | `Dynamics/Water/LiftMax` | number | - | 1.5 | `007e3929` | - |
| `+2e0` | `Dynamics/Water/DecelSpeed` | number | - | KMH(15) | `007e389d` | - |
| `+2e4` | `Dynamics/Water/SideDragRatio` | number | - | 0.5 | `007e39a7` | - |
| `+2e8` | `Dynamics/Water/MaxSideDrag` | number | - | 6.0 | `007e39e6` | - |
| `+2ec` | `Dynamics/Water/MaxCtrlAngle` | number | - | DEG(5) | `007e3bde` | - |
| `+2f0` | `Dynamics/Water/MinCtrlAngle` | number | - | DEG(20) | `007e3c33` | - |
| `+2f4` | `Dynamics/Water/TakeOffMaxLength` | number | - | 250 | `007e3a25` | `0099f1c0` |
| `+2f8` | `Dynamics/Water/TakeOffMinLength` | number | - | 140 | `007e3a64` | - |
| `+2fc` | `Dynamics/Water/MinDragSpd` | number | - | KMH(20) | `007e3aa3` | - |
| `+300` | `Dynamics/Water/LiftBeginDiveMul` | number | - | 0.35 | `007e3ae2` | - |
| `+304` | `Dynamics/Water/LiftRotateMax` | number | - | 8.0 | `007e3b21` | - |
| `+308` | `Dynamics/Water/LiftRotateDepthRatio` | number | - | 1.8 | `007e3b60` | - |
| `+30c` | `Dynamics/Water/LiftRotateAngleRatio` | number | - | 8.0 | `007e3b9f` | - |
| `+310` | `Dynamics/MaxDragSpdMul` | number | - | 0.1 | `007e3449` | - |
| `+314` | `Dynamics/MinDragSpdMul` | number | - | 0.1 | `007e3481` | - |
| `+318` | `Dynamics/MaxDragPitch` | number | - | 1.0 | `007e34bc` | - |
| `+31c` | `Dynamics/AccelCheatMul` | number | - | 1.5 | `007e34fa` | `007d1f70` |
| `+320` | `Dynamics/AccelCheatMulMul` | number | - | 1.15 | `007e3538` | `007d1f70` |
| `+324` | `Dynamics/AccelCheatFallMul` | number | - | 2.6 | `007e3576` | - |
| `+328` | `Dynamics/AccelCheatFallPitchRange/1` | number | - | DEG(10) | `007e35cd` | - |
| `+32c` | `Dynamics/AccelCheatFallPitchRange/2` | number | - | DEG(60) | `007e3638` | - |
| `+330` | `Dynamics/SpdMultipliers/TurboMultiplier` | number | - | 1.95 | `007e3fe1` | - |
| `+334` | `Dynamics/SpdMultipliers/NewTravelSpeedMul` | number | - | 1.6 | `007e4020` | `007d1f70` |
| `+338` | `Dynamics/SpdMultipliers/DiveBombSlowMul` | number | - | 1.3 | `007e405f` | `007ce040` |
| `+33c` | `Dynamics/SpdMultipliers/TorpedoBombSlowMul` | number | - | 1.3 | `007e409e` | `007ce040` |
| `+340` | `Dynamics/SpdMultipliers/DepthChargeSlowMul` | number | - | 1.3 | `007e40dd` | `007ce040` |
| `+344` | `Dynamics/SpdMultipliers/LevelBombSlowMul` | number | - | 1.0 | `007e411c` | `007ce040` |
| `+348` | `Retreat/ExitDist` | number | - | 1000 | `007e6bfd` | - |
| `+34c` | `Retreat/ExitTime` | number | - | 20 | `007e6c3e` | - |
| `+350` | `Retreat/WarningRepeatTime` | number | - | 6 | `007e6c7f` | - |
| `+354` | `ParatrooperDrop/MinAltitude` | number | - | 300 | `007e6d01` | `007c4d90` |
| `+358` | `ParatrooperDrop/MaxAltitude` | number | - | 1500 | `007e6d42` | `007c4d90` |
| `+35c` | `Pilot/MoveTo/SmallPlaneTravelAlt` | number | - | 800 | `007e82f2` | - |
| `+360` | `Pilot/MoveTo/LargePlaneTravelAlt` | number | - | 1400 | `007e8333` | - |
| `+364` | `Pilot/MoveTo/TravelAltRandom` | number | - | 50 | `007e8374` | - |
| `+368` | `Pilot/MoveTo/FollowDist/1` | number | - | 20 | `007e83ce` | - |
| `+36c` | `Pilot/MoveTo/FollowDist/2` | number | - | 200 | `007e843c` | - |
| `+370` | `Pilot/MoveTo/ClosingDist` | number | - | 150 | `007e8491` | `0084e010` `009c3570` |
| `+374` | `Pilot/MoveTo/SwitchNextPointTime` | number | - | 2.0 | `007e8513` | - |
| `+378` | `Pilot/MoveTo/CircleAltDiff` | number | - | 30 | `007e84d2` | `009fc260` |
| `+37c` | `Pilot/MoveTo/ReferenceSpeed` | number | - | KMH(300) | `007e8554` | `009bc780` `009bc7e0` `009c1c30` `009c1dc0` |
| `+380` | `Pilot/Follow/FollowedPointDist` | number | - | 250 | `007e8aa8` | - |
| `+384` | `Pilot/Follow/LeaderFollowAlt` | number | - | 10 | `007e8ae9` | - |
| `+388` | `Pilot/Follow/SafeAlt` | number | - | 60 | `007e8b2a` | - |
| `+38c` | `Pilot/Follow/SmallPlaneTurnMul` | number | - | 1.1 | `007e870c` | - |
| `+390` | `Pilot/Follow/LargePlaneTurnMul` | number | - | 1.1 | `007e874d` | - |
| `+394` | `Pilot/Follow/GoodPositionDir` | number | - | 0.5 | `007e87cf` | - |
| `+398` | `Pilot/Follow/GoodPositionDist` | number | - | 100 | `007e878e` | - |
| `+39c` | `Pilot/Follow/GoodPositionSpdTreshold` | number | - | 1.0 | `007e8810` | - |
| `+3a0` | `Pilot/Follow/GoodPositionSpdDiff` | number | - | KMH(200) | `007e8851` | - |
| `+3a4` | `Pilot/Follow/MaxFollowSpdTargetDir` | number | - | DEG(30) | `007e8892` | - |
| `+3a8` | `Pilot/Follow/MinFollowSpdTargetDir` | number | - | DEG(100) | `007e891b` | - |
| `+3ac` | `Pilot/Follow/DontWaitForHdgDiff` | number | - | DEG(50) | `007e89a7` | - |
| `+3b0` | `Pilot/Follow/WaitForHdgDiff` | number | - | DEG(100) | `007e89e5` | - |
| `+3b4` | `Pilot/Follow/NearbyDist` | number | - | 200 | `007e8a26` | - |
| `+3b8` | `Pilot/Follow/TightTurn` | number | - | 1.25 | `007e8a67` | - |
| `+3bc` | `Pilot/Follow/LeaderHeadingSpdTime/1` | number | - | 0.5 | `007e8b84` | - |
| `+3c0` | `Pilot/Follow/LeaderHeadingSpdTime/2` | number | - | 4.0 | `007e8bef` | - |
| `+3c4` | `Pilot/Follow/LeaderHeadingSpdDist/1` | number | - | 100 | `007e8c5a` | - |
| `+3c8` | `Pilot/Follow/LeaderHeadingSpdDist/2` | number | - | 500 | `007e8cc5` | - |
| `+3cc` | `(derived)` | derived | - | - | `007e908d` | - |
| `+3d0` | `Pilot/Follow/SmallPlaneDisplacement` | number[3] | - | - | `007e85db` | `007f23a0` |
| `+3d4` | `Pilot/Follow/SmallPlaneDisplacement` | number[3] | - | - | `007e85db` | - |
| `+3d8` | `Pilot/Follow/SmallPlaneDisplacement` | number[3] | - | - | `007e85db` | - |
| `+3dc` | `Pilot/Follow/BomberDisplacement` | number[3] | - | - | `007e8635` | `007f23a0` |
| `+3e0` | `Pilot/Follow/BomberDisplacement` | number[3] | - | - | `007e8635` | - |
| `+3e4` | `Pilot/Follow/BomberDisplacement` | number[3] | - | - | `007e8635` | - |
| `+3e8` | `Pilot/Follow/SymmetricalPosition` | boolean | - | true | `007e868a` | `007f23a0` |
| `+3e9` | `Pilot/Follow/SymmetricalAltitude` | boolean | - | false | `007e86cb` | `007f23a0` |
| `+3ec` | `Pilot/Follow/yf_hdg_rad` | number | - | 1 / DEG(10) | `007e8d17` | - |
| `+3f0` | `Pilot/Follow/yf_yawV_radPerSec` | number | - | 0 / DEG(10) | `007e8d58` | - |
| `+3f4` | `Pilot/Follow/yf_sidepos_meter` | number | - | 1 / 25 | `007e8d99` | - |
| `+3f8` | `Pilot/Follow/yf_sidedir` | number | - | 1 / 25 | `007e8dda` | - |
| `+3fc` | `Pilot/Follow/pf_pitch_rad` | number | - | 1 / DEG(10) | `007e8e1b` | - |
| `+400` | `Pilot/Follow/pf_pitchV_radPerSec` | number | - | 1 / DEG(100) | `007e8e5c` | - |
| `+404` | `Pilot/Follow/pf_vertpos_meter` | number | - | 1 / 25 | `007e8e9d` | - |
| `+408` | `Pilot/Follow/pf_vertdir` | number | - | 1 / 25 | `007e8ede` | - |
| `+40c` | `Pilot/Follow/rf_roll_rad` | number | - | -1 / DEG(30) | `007e8f1f` | - |
| `+410` | `Pilot/Follow/rf_rollV_radPerSec` | number | - | -0 / DEG(80) | `007e8f60` | - |
| `+414` | `Pilot/Follow/rf_hdg_rad` | number | - | 0 / DEG(40) | `007e8fa1` | - |
| `+418` | `Pilot/Follow/rf_hdgV_radPerSec` | number | - | 0 / DEG(400) | `007e8fe2` | - |
| `+41c` | `Pilot/Follow/pwr_back_meter` | number | - | 1 / 10 | `007e9023` | - |
| `+420` | `Pilot/Follow/pwr_spd_meterPerSec` | number | - | 1 / KMH(10) | `007e9064` | - |
| `+424` | `Pilot/CloseToShip/CruisingAlt` | number | - | 1200 | `007e9237` | `009a2d60` |
| `+428` | `Pilot/CloseToShip/DropAlt` | number | - | 1000 | `007e9278` | `009a2d60` |
| `+42c` | `Pilot/CloseToShip/ReferenceSpeed` | number | - | KMH(300) | `007e92b9` | `009a1d60` `009a1dc0` |
| `+430` | `Pilot/Torpedo/CruisingAlt` | number | - | 500 | `007e933b` | `009d4a70` |
| `+434` | `Pilot/Torpedo/AttackDist` | number | - | 2200 | `007e937c` | `009d4a70` |
| `+438` | `Pilot/Torpedo/SafeDist` | number | - | 700 | `007e93bd` | - |
| `+43c` | `Pilot/Torpedo/MoveOnCruisingAlt` | boolean | - | true | `007e93fe` | - |
| `+440` | `Pilot/Torpedo/ReferenceSpeed` | number | - | KMH(300) | `007e9448` | `009d0380` |
| `+444` | `Pilot/LevelBomb/CruisingAlt` | number | - | 1300 | `007e94ca` | `009b8c90` |
| `+448` | `Pilot/LevelBomb/DropAlt` | number | - | 1300 | `007e950b` | `009b8c90` |
| `+44c` | `Pilot/LevelBomb/AttackDist` | number | - | 2000 | `007e954c` | `009b8c90` |
| `+450` | `Pilot/LevelBomb/SafeDist` | number | - | 1000 | `007e958d` | `009b8d80` |
| `+454` | `Pilot/LevelBomb/MoveOnCruisingAlt` | boolean | - | false | `007e95ce` | - |
| `+458` | `Pilot/LevelBomb/ReferenceSpeed` | number | - | KMH(300) | `007e9618` | `009abab0` `009abbe0` `009b44f0` `009b4690` |
| `+45c` | `Pilot/Kamikaze/RocketLike/CruisingAlt` | number-or-default | 1200 | 1300 | `007e982c` | - |
| `+460` | `Pilot/Kamikaze/RocketLike/DropDist` | number-or-default | 2000 | 3500 | `007e9875` | - |
| `+464` | `Pilot/Kamikaze/RocketLike/AttackRange` | number-or-default | 1400 | 3500 | `007e96bf` | - |
| `+468` | `Pilot/Kamikaze/RocketLike/AttackAlt` | number-or-default | 500 | 800 | `007e9708` | - |
| `+46c` | `Pilot/Kamikaze/RocketLike/TurboRange` | number-or-default | 1000 | 3000 | `007e9751` | - |
| `+470` | `Pilot/Kamikaze/RocketLike/TurboAngle` | number-or-default | 0.2 | DEG(12) | `007e979a` | - |
| `+474` | `Pilot/Kamikaze/RocketLike/ReferenceSpeed` | number-or-default | 180 | KMH(600) | `007e97e3` | `009ab920` |
| `+478` | `Pilot/Kamikaze/FighterLike/CruisingAlt` | number-or-default | 1200 | 1200 | `007e9a63` | - |
| `+47c` | `Pilot/Kamikaze/FighterLike/DropDist` | number-or-default | 1200 | - | `007e9aac` | - |
| `+480` | `Pilot/Kamikaze/FighterLike/AttackRange` | number-or-default | 1200 | 2600 | `007e98fa` | - |
| `+484` | `Pilot/Kamikaze/FighterLike/AttackAlt` | number-or-default | 600 | 800 | `007e9943` | - |
| `+488` | `Pilot/Kamikaze/FighterLike/TurboRange` | number-or-default | 500 | 2400 | `007e998c` | - |
| `+48c` | `Pilot/Kamikaze/FighterLike/TurboAngle` | number-or-default | 0 | - | `007e99d1` | - |
| `+490` | `Pilot/Kamikaze/FighterLike/ReferenceSpeed` | number-or-default | 100 | KMH(400) | `007e9a1a` | `009ab920` |
| `+494` | `Pilot/DepthCharge/AimAltRange/1` | number | - | 20 | `007e9b58` | `009a3390` |
| `+498` | `Pilot/DepthCharge/AimAltRange/2` | number | - | 60 | `007e9bc3` | `009a6500` |
| `+49c` | `Pilot/DepthCharge/ManeuverAltRange/1` | number | - | 80 | `007e9c2e` | `009a3390` `009a5f50` |
| `+4a0` | `Pilot/DepthCharge/ManeuverAltRange/2` | number | - | 150 | `007e9c99` | - |
| `+4a4` | `Pilot/DepthCharge/CruisingAlt` | number | - | 700 | `007e9ceb` | `009a6500` |
| `+4a8` | `Pilot/DepthCharge/FlyAboveDist` | number | - | 400 | `007e9d2c` | `009a3390` |
| `+4ac` | `Pilot/DepthCharge/AttackDist` | number | - | 1800 | `007e9d6d` | `009a6500` |
| `+4b0` | `Pilot/DepthCharge/SafeDist` | number | - | 250 | `007e9dae` | `009a65f0` |
| `+4b4` | `Pilot/DepthCharge/MoveOnCruisingAlt` | boolean | - | true | `007e9e30` | - |
| `+4b8` | `Pilot/DepthCharge/ReferenceSpeed` | number | - | KMH(270) | `007e9def` | `009a3390` `009a35d0` |
| `+4bc` | `Pilot/DepthCharge/TargetLostTime` | number-or-default | 20 | 20 | `007e9e84` | `009a5f50` |
| `+4c0` | `Pilot/DiveBomb/CruisingAlt` | number | - | 1300 | `007e9f47` | `009c8920` |
| `+4c4` | `Pilot/DiveBomb/AttackDist` | number | - | 1100 | `007e9f06` | `009c8920` |
| `+4c8` | `Pilot/DiveBomb/SafeDist` | number | - | 100 | `007e9f88` | `009c8a90` |
| `+4cc` | `Pilot/DiveBomb/BeginAltRange/1` | number | - | 1000 | `007e9fe2` | `009c3ea0` `009c8920` |
| `+4d0` | `Pilot/DiveBomb/BeginAltRange/2` | number | - | 1200 | `007ea04d` | - |
| `+4d4` | `Pilot/DiveBomb/MoveOnCruisingAlt` | boolean | - | true | `007ea0e0` | - |
| `+4d8` | `Pilot/DiveBomb/ReferenceSpeed` | number | - | KMH(280) | `007ea09f` | `009c3ea0` |
| `+4dc` | `Pilot/TakeOff/PrepareTime` | number | - | 1.5 | `007ea717` | `009cdd50` `009cde50` |
| `+4e0` | `Pilot/Landing/ApproachPitch` | number | - | DEG(6) | `007ea16b` | `009afe70` `009b1420` |
| `+4e4` | `Pilot/Landing/ApproachAngle` | number | - | DEG(12) | `007ea1ac` | `006c3b10` `009afe70` |
| `+4e8` | `Pilot/Landing/ParkVelocity` | number | - | KTS(40) | `007ea26f` | - |
| `+4ec` | `Pilot/Landing/ApproachDist` | number | - | 210 | `007ea1ed` | `007c5ac0` `009b1420` |
| `+4f0` | `Pilot/Landing/PosBehind` | number | - | 780 | `007ea332` | `006c3b10` `006c5380` `006c5e20` `006c6020` `009b3c00` |
| `+4f4` | `Pilot/Landing/PosAlt` | number | - | 170 | `007ea373` | `006c5380` |
| `+4f8` | `Pilot/Landing/CircleMultiplierMin` | number | - | 1.1 | `007ea3b4` | `007c6760` |
| `+4fc` | `Pilot/Landing/CircleMultiplierMax` | number | - | 1.05 | `007ea3f5` | `007c6760` |
| `+500` | `Pilot/Landing/StandbyDist` | number | - | 3200 | `007ea436` | `006c45c0` `006c46b0` `006c5e20` `006cc9f0` |
| `+504` | `Pilot/Landing/FollowDistTime` | number | - | 9 | `007ea22e` | `006c3f80` |
| `+508` | `Pilot/Landing/TakeoffDist` | number | - | 180 | `007ea2f1` | `006c2400` |
| `+50c` | `Pilot/Landing/TouchDownDist` | number | - | 135 | `007ea2b0` | `006c2400` |
| `+510` | `Pilot/Landing/LiftDelay` | number | - | 0.5 | `007ea477` | - |
| `+514` | `Pilot/Landing/CruisingAlt` | number | - | 1400 | `007ea57b` | `009b3c60` |
| `+518` | `Pilot/Landing/WireRope` | number | - | 7.25 | `007ea4b8` | `007db680` |
| `+51c` | `Pilot/Landing/MaxWireRope` | number | - | 100.0 | `007ea4f9` | `007c7a40` `007db630` `007db680` |
| `+520` | `Pilot/Landing/MinFreeRunwayLength` | number | - | 12 | `007ea53a` | `006d0530` |
| `+524` | `Pilot/Landing/RadiusChange/1` | number | - | 10 | `007ea5d5` | `006c3e50` |
| `+528` | `Pilot/Landing/RadiusChange/2` | number | - | 50 | `007ea644` | - |
| `+52c` | `Pilot/Landing/ReferenceSpeed` | number | - | KMH(140) | `007ea695` | `009afe70` `009afff0` |
| `+530` | `UnitAI/StopClearSpeed` | number | - | KMH(50.0) | `007e6b3a` | - |
| `+534` | `UnitAI/StopClearAlt` | number | - | 30.0 | `007e6b7b` | - |
| `+538` | `Pilot/General/WaggleLimit` | number | - | 0.5 | `007e6e46` | - |
| `+53c` | `Pilot/General/CruisingAlt` | number | - | 1000 | `007e6fec` | `007f2bd0` `0084ddc1` `009c91b0` |
| `+540` | `Pilot/General/MaxAltOffset` | number | - | 5 | `007e702d` | `009bc780` `009c1c30` |
| `+544` | `Pilot/General/ClimbDist` | number | - | 130 | `007e6e05` | `009fb700` `009fb800` |
| `+548` | `Pilot/General/DropDist` | number | - | 200 | `007e6dc4` | `009fb700` `009fb800` |
| `+54c` | `Pilot/General/MinTurnCircle` | number | - | 450 | `007e7131` | `007c4850` |
| `+550` | `Pilot/General/LevelBombAngleMax` | number | - | DEG(20) | `007e75b4` | `007c7750` `007cc8e0` |
| `+550` | `Pilot/General/LevelBombAngleMax` | number | - | DEG(20) | `007e75f5` | `007c7750` `007cc8e0` |
| `+550` | `Pilot/General/LevelBombAngleMax` | number | - | DEG(20) | `007e7636` | `007c7750` `007cc8e0` |
| `+550` | `Pilot/General/LevelBombAngleMax` | number | - | DEG(20) | `007e7677` | `007c7750` `007cc8e0` |
| `+554` | `Pilot/General/DiveBombRollAngleMin` | number | - | DEG(60) | `007e7352` | - |
| `+558` | `Pilot/General/DiveBombRollAngleMax` | number | - | DEG(90) | `007e7393` | - |
| `+55c` | `Pilot/General/DiveBombRollAngleMinPitch` | number | - | -DEG(80) | `007e73d4` | - |
| `+560` | `Pilot/General/DiveBombRollAngleMaxPitch` | number | - | DEG(90) | `007e7415` | `007c7600` |
| `+564` | `Pilot/General/DiveBombPitchAngleMin` | number | - | DEG(60) | `007e7456` | `007c7600` |
| `+568` | `Pilot/General/DiveBombPitchAngleMax` | number | - | DEG(90) | `007e7497` | `007c7600` |
| `+56c` | `Pilot/General/SoftHdgMul` | number | - | 0.35 | `007e6f13` | - |
| `+570` | `Pilot/General/SoftHdgLimit` | number | - | 0.5 | `007e6e87` | - |
| `+574` | `Pilot/General/SoftHdgZone` | number-or-default | 0.01 | 0.01 | `007e6ed2` | - |
| `+578` | `Pilot/General/SoftRollCtrl` | number | - | 0.05 | `007e6f54` | - |
| `+57c` | `Pilot/General/SoftRollMul` | number | - | 0.7 | `007e6f95` | - |
| `+580` | `(derived)` | derived | - | - | `007e6fd4` | - |
| `+584` | `Pilot/General/HdgDiffCalcLimit/1` | number | - | 0.8 | `007e720d` | - |
| `+588` | `Pilot/General/HdgDiffCalcLimit/2` | number | - | 1.5 | `007e727b` | - |
| `+58c` | `Pilot/General/HdgDiffCalcMinPitch` | number | - | 0.1 | `007e72d0` | - |
| `+590` | `Pilot/General/HdgDiffCalcMinRoll` | number | - | DEG(5) | `007e7311` | - |
| `+594` | `Pilot/General/GuardDist` | number | - | 3000 | `007e706e` | `0084e010` `009f7fe0` |
| `+598` | `Pilot/General/LeaveAlonePwr` | number | - | 0.8 | `007e70f0` | - |
| `+59c` | `Pilot/General/TurnRollLimitSmall` | number | - | DEG(85) | `007e7172` | - |
| `+5a0` | `Pilot/General/TurnRollLimitLarge` | number | - | DEG(56) | `007e71b3` | - |
| `+5a4` | `Pilot/General/TurnRollPitchLimitPitch/1` | number | - | DEG(0) | `007e77ee` | - |
| `+5a8` | `Pilot/General/TurnRollPitchLimitPitch/2` | number | - | DEG(12) | `007e785c` | - |
| `+5ac` | `Pilot/General/TurnRollPitchLimitRoll/1` | number | - | DEG(15) | `007e78ca` | - |
| `+5b0` | `Pilot/General/TurnRollPitchLimitRoll/2` | number | - | DEG(60) | `007e7938` | - |
| `+5b4` | `Pilot/General/YawTurnRollRange/1` | number | - | DEG(30) | `007e76d1` | `007db4d0` |
| `+5b8` | `Pilot/General/YawTurnRollRange/2` | number | - | DEG(60) | `007e773f` | - |
| `+5bc` | `Pilot/General/YawTurnMaxPitch` | number | - | 0.6 | `007e7794` | - |
| `+5c0` | `Pilot/General/PitchTurnMaxPitch` | number | - | DEG(06) | `007e798d` | - |
| `+5c4` | `Pilot/General/PitchTurnHdgRange/1` | number | - | DEG(25) | `007e79e7` | - |
| `+5c8` | `Pilot/General/PitchTurnHdgRange/2` | number | - | DEG(50) | `007e7a55` | - |
| `+5cc` | `Pilot/General/WingmenWaitDist/1` | number | - | 3000 | `007e74f1` | `009becd0` |
| `+5d0` | `Pilot/General/WingmenWaitDist/2` | number | - | 5000 | `007e755f` | - |
| `+5d4` | `Pilot/General/YawCtrlSetTimeMul` | number | - | 0.8 | `007e7aaa` | - |
| `+5d8` | `Pilot/General/PitchCtrlSetTimeMul` | number | - | 0.7 | `007e7aeb` | - |
| `+5dc` | `Pilot/General/MoveCircleMinAngle` | number | - | DEG(15) | `007e7b2c` | `009fbb20` |
| `+5e0` | `Pilot/General/MoveCircleMaxAngle` | number | - | DEG(45) | `007e7b6d` | - |
| `+5e4` | `Pilot/General/MoveCircleFollowedDist` | number | - | 180 | `007e7bae` | `009fbb20` |
| `+5e8` | `Pilot/General/TrgSpeedCorrMinPitch` | number | - | DEG(70) | `007e7bef` | - |
| `+5ec` | `Pilot/General/TrgSpeedCorrSpeedMul` | number | - | 2.6 | `007e7c30` | - |
| `+5f0` | `Pilot/General/TrgSpeedCorrMulDecay` | number | - | 0.8 | `007e7c71` | - |
| `+5f4` | `Pilot/General/DropAllEquipmentPercent` | number | - | 0.41 | `007e70af` | `007f2bd0` |
| `+5f8` | `Pilot/Avoidance/Gunfire/MaxWeight` | number | - | 0.6 | `007e7d40` | - |
| `+5fc` | `Pilot/Avoidance/Gunfire/WeightInc` | number | - | 1.3 | `007e7d7f` | - |
| `+600` | `Pilot/Avoidance/Gunfire/WeightDec` | number | - | 1.0 | `007e7dbe` | - |
| `+604` | `Pilot/Avoidance/Gunfire/AvoidTime/1` | number | - | 2 | `007e7e16` | `0099ec40` |
| `+608` | `Pilot/Avoidance/Gunfire/AvoidTime/2` | number | - | 3 | `007e7e82` | - |
| `+60c` | `Pilot/Avoidance/Gunfire/WaitTime/1` | number | - | 5 | `007e7eee` | `0099ec40` |
| `+610` | `Pilot/Avoidance/Gunfire/WaitTime/2` | number | - | 8 | `007e7f5a` | - |
| `+614` | `Pilot/Avoidance/Gunfire/BomberVSGunfire` | number | - | 1.2 | `007e7fad` | `0099ec40` |
| `+618` | `Pilot/Avoidance/Vehicle/MaxWeight` | number | - | 1.0 | `007e8028` | - |
| `+61c` | `Pilot/Avoidance/Vehicle/WeightInc` | number | - | 4.0 | `007e8067` | - |
| `+620` | `Pilot/Avoidance/Vehicle/WeightDec` | number | - | 1.4 | `007e80a6` | - |
| `+624` | `Pilot/Avoidance/Vehicle/MinCollTime` | number | - | 3.0 | `007e80e5` | - |
| `+628` | `Pilot/Avoidance/Vehicle/AvoidSpdMul` | number | - | 0.4 | `007e81e1` | - |
| `+62c` | `Pilot/Avoidance/Vehicle/MinPlaneSpd` | number | - | KMH(70) | `007e8124` | - |
| `+630` | `Pilot/Avoidance/Vehicle/UseRollStrength` | number | - | 0.5 | `007e8220` | - |
| `+634` | `Pilot/Avoidance/Vehicle/MaxDistMultiplier` | number | - | 1.2 | `007e8163` | - |
| `+638` | `Pilot/Avoidance/Vehicle/MinDistMultiplier` | number | - | 1.0 | `007e81a2` | - |
| `+63c` | `Pilot/Avoidance/Vehicle/BomberVSSmallPlane` | boolean | - | false | `007e825f` | - |
| `+640` | `Pilot/Dogfight/CruisingAlt` | number | - | 1400 | `007ea799` | `009aaf30` |
| `+644` | `Pilot/Dogfight/AttackDist` | number | - | 2000 | `007ea7da` | `009aac70` |
| `+648` | `Pilot/Dogfight/FighterAimMulVersusAI` | number-or-default | 1.8 | 1.6 | `007ea825` | `009fc7c0` |
| `+64c` | `Pilot/Dogfight/FighterAimMulVersusPlayer` | number-or-default | 1.2 | 1.1 | `007ea870` | `009fc7c0` |
| `+650` | `Pilot/Dogfight/ReferenceSpeed` | number | - | KMH(300) | `007ea8b1` | `009a6c10` `009a6d40` |
| `+654` | `Pilot/Strafe/CruisingAlt` | number | - | 1000 | `007ea933` | `009cd020` |
| `+658` | `Pilot/Strafe/AttackDist` | number | - | 2000 | `007ea974` | `009ca4a0` `009cadb0` `009cced0` |
| `+65c` | `Pilot/Strafe/ReferenceSpeed` | number | - | KMH(280) | `007ea9b5` | `009ca4a0` |
| `+660` | `Pilot/Strike/CruisingAlt` | number | - | 500 | `007eaa37` | `007b7af0` |
| `+664` | `Pilot/Strike/AttackDist` | number | - | 1800 | `007eaa78` | `007b41e0` `007b4f60` `007b78f0` |
| `+668` | `Pilot/Strike/ReferenceSpeed` | number | - | KMH(280) | `007eaab9` | `007b41e0` |
| `+66c` | `Pilot/AutoStrafeAngle/Angle_Prepare` | number | - | DEG(0) | `007e90f2` | `009b0fe0` `009bee30` |
| `+670` | `Pilot/AutoStrafeAngle/Angle_MoveTo` | number | - | DEG(2) | `007e9133` | `007b4f60` `009a3770` `009a71e0` `009bca50` `009bee30` `009c18c0` `009c2430` `009c26d0` `009c4220` `009c9310` `009cadb0` `009d07b0` `009d0f10` |
| `+674` | `Pilot/AutoStrafeAngle/Angle_GoAway` | number | - | DEG(10) | `007e9174` | `007b6240` `009b5760` `009c4a40` `009cbb30` `009d0f10` |
| `+678` | `Pilot/AutoStrafeAngle/Angle_Strafe` | number | - | DEG(15) | `007e91b5` | `009a76e0` `009ca870` `009cb1b0` |
| `+67c` | `PlaneGUI/PitchYawZoomControlLimit` | number | - | 0.8 | `007e62ab` | - |
| `+680` | `PlaneGUI/RollZoomControlLimit` | number | - | 1.0 | `007e62e9` | - |
| `+684` | `PlaneGUI/AltimeterCeiling` | number | - | 1600 | `007e6327` | `006093d0` |
| `+688` | `PlaneGUI/MouseInputMultiplier` | number | - | 1.0 | `007e6365` | `006062b0` |
| `+68c` | `PlaneGUI/MouseInputDeadZoneMin` | number | - | 0.05 | `007e63a3` | `006062b0` |
| `+690` | `PlaneGUI/MouseInputDeadZoneMax` | number | - | 0.95 | `007e63e1` | `006062b0` |
| `+694` | `PlaneGUI/MouseInputSmoothTreshold` | number | - | 1.1 | `007e641f` | `006062b0` |
| `+698` | `PlaneGUI/MouseInputSmoothMultiplier` | number | - | 0.50 | `007e645d` | `006062b0` |
| `+69c` | `PlaneGUI/MouseInputExponencialWeight` | number | - | 0.0 | `007e649b` | `006062b0` |
| `+6a0` | `PlaneGUI/SpeedDisplayMultiplier` | number | - | 2 | `007e64d9` | `00609bd0` |
| `+6a4` | `Sound/EnginePowerMultiplier` | number | - | 0.7 | `007e6555` | - |
| `+6a8` | `Sound/EngineSpeedMultiplier` | number | - | 0.35 | `007e6593` | - |
| `+6ac` | `Sound/IdleOffVolume` | number | - | 0.5 | `007e65d1` | `007eb380` |
| `+6b0` | `Sound/EngineMinFreq` | number | - | 0.4 | `007e660f` | - |
| `+6b4` | `Sound/EngineMaxFreq` | number | - | 1.0 | `007e664d` | `007eb380` |
| `+6b8` | `Sound/PitchChangeRate` | number | - | 10.0 | `007e668b` | `007ec5a0` |
| `+6bc` | `Sound/VolumeChangeRate` | number | - | 10.0 | `007e66cc` | `007ec5a0` |
| `+6c0` | `Sound/IdleSafetyTime` | number | - | 2.0 | `007e670d` | `007ec5a0` |
| `+6c4` | `Sound/EngineFreqLimitSpdMul` | number | - | 0.6 | `007e674e` | - |
| `+6c8` | `Sound/EngineFreqLimitMax` | number | - | 1.4 | `007e678f` | - |
| `+6cc` | `Sound/IdleAndEngineVolMax` | number | - | 1.5 | `007e67d0` | `007eb380` |

## Derived stores

Four stores have no key of their own.

| offset | rule | site |
|---|---|---|
| `+0` | vtable `00d08628` | 007E2A4C |
| `+28c` | clamp of three already-loaded fields, see below | 007E41DF |
| `+3cc` | copy of `+330` (`Dynamics/SpdMultipliers/TurboMultiplier`) | 007E908D |
| `+580` | `(1.0 - [+57c]) * [+578]` | 007E6FD4 |

`+28c` is built at 007E4160..007E41DF from `+23c`, `+240`, `+230` and `+24c`: it interpolates
between `ControlRangeMin` and `ControlRangeMax`, takes the larger of that and
`StallRangeMax * 00cf87c0`, raises the result to `LevelFlight * 00d7a390` when that is larger,
and finally clamps it down to `LevelFlight`. The two scaling constants are image floats, so the
rule is `min(LevelFlight, max(lerp(ControlRangeMin, ControlRangeMax, k0), StallRangeMax*k1,
LevelFlight*k2))`.

Two keys are stored transformed rather than raw: `Dynamics/Water/MaxCtrlAngle` and
`Dynamics/Water/MinCtrlAngle` are read as numbers and stored as their **cosine** into `+2ec`
and `+2f0` (`fcos` at 007E3BF0 and 007E3C45). The table lists them with their key; the stored
value is `cos(key)`.

## Defects in the shipped loader

Four fetch sequences repeat a key instead of reading the sibling the destination field implies.
All four were confirmed against the raw listing, not the decompiler.

| destination | key actually fetched | evidence | effect |
|---|---|---|---|
| `+2a8` and `+2ac` | `Dynamics/RunwayYawTurnSpdLimit` | 007e2fc7 and 007e3026 both push index `1` | `RunwayYawTurnSpdLimit[2] = KMH(40)` is never read; the upper limit field holds the lower limit |
| `+250` and `+254` | `Dynamics/DeadMeat/RotationMin` | 007e30bd and 007e30fe both push the string at `00d084d8` (`RotationMin`) | `DeadMeat/RotationMax = DEG(30)` is never read; the maximum field holds the minimum |
| `+e8` and `+ec` | `BombCamera/CameraPosSmooth` | the two fetches at 007e4d31 and 007e4d6c push the same key string | `BombCamera/FinalCameraPosSmooth = 2.0` is never read; the final-smooth field holds the normal one |
| `+550` (four times) | `Pilot/General/LevelBombAngleMax` | 007e7591, 007e75d2, 007e7613 and 007e7654 push `00d07458` and all four stores target `[EBP+550h]` | no field is lost, but the same value is fetched and stored four times |

The three genuine mix-ups are the reason three of the 27 unread keys below are unread.

## Keys the loader never reads

The installed `PlaneGlobals` table has 448 leaves. The loader reads 423 distinct paths and
leaves 27 untouched:

* `BombCamera/FinalCameraPosSmooth` = `2.0`
* `BombCamera/FinalCameraSmooth` = `10.0`
* `Dynamics/DeadMeat/RotationMax` = `DEG(30)`
* `Dynamics/RunwayYawTurnSpdLimit/2` = `KMH(40)`
* `Dynamics/Water/DecelMax` = `3.5`
* `Pilot/Avoidance/Terrain/MaxWeight` = `1.0`
* `Pilot/Avoidance/Terrain/PitchTurnRollRange/1` = `DEG(45)`
* `Pilot/Avoidance/Terrain/PitchTurnRollRange/2` = `DEG(60)`
* `Pilot/Avoidance/Terrain/SafeAlt/1` = `12`
* `Pilot/Avoidance/Terrain/SafeAlt/2` = `30`
* `Pilot/Avoidance/Terrain/TimeTick` = `1.0`
* `Pilot/Avoidance/Terrain/WeightDec` = `1.0`
* `Pilot/Avoidance/Terrain/WeightInc` = `2.0`
* `Pilot/Avoidance/Terrain/YawTurnRollRange/1` = `DEG(35)`
* `Pilot/Avoidance/Terrain/YawTurnRollRange/2` = `DEG(55)`
* `Pilot/Follow/BomberDisplacement/1` = `100`
* `Pilot/Follow/BomberDisplacement/2` = `0`
* `Pilot/Follow/BomberDisplacement/3` = `-100`
* `Pilot/Follow/SmallPlaneDisplacement/1` = `60`
* `Pilot/Follow/SmallPlaneDisplacement/2` = `25`
* `Pilot/Follow/SmallPlaneDisplacement/3` = `70`
* `Pilot/General/EnableFullInvincible` = `true`
* `PlaneCamera/CockpitGunfireEffectSmooth` = `0.1`
* `PlaneCamera/DistY` = `0.6`
* `PlaneCamera/DistZ` = `1.5`
* `WingTip/MaxAlpha` = `1.0`
* `WingTip/MaxalphaOwnPlane` = `0.5`

Three of those (`RunwayYawTurnSpdLimit/2`, `DeadMeat/RotationMax`, `FinalCameraPosSmooth`) are
unread because of the defects above. The rest are dead data in the shipped table: no site in
`007E2A20` pushes their key strings.

## Keys the loader reads that the shipped table does not define

| offset | key | native default | site |
|---|---|---|---|
| `+47c` | `Pilot/Kamikaze/FighterLike/DropDist` | `1200` (`FLD [00ce54a0]`) | 007E9AAC |
| `+48c` | `Pilot/Kamikaze/FighterLike/TurboAngle` | `0` (`FLDZ` at 007E99C1) | 007E99D1 |

`Pilot/Kamikaze/RocketLike` defines both keys; `FighterLike` defines neither. Both reads go
through `00B66330`, so in the shipped game `+47c` is `1200` and `+48c` is `0`.

## The 141 call sites

`0042E740` has 141 callers. The count overstates the number of consumers: the compiler re-fetches
the singleton for each field, so `007D1F70` alone calls it four times across 007D20F3..007D212F.

| | |
|---|---|
| callers | 141 |
| callers where an offset access could be resolved | 133 |
| callers with no resolved access | 8 (`004e3aa0`, `0076f210`, `007d5ac0`, `0099d0a0`, `0099d300`, `009bc3a0`, `009bed80`, `009c2980`) |
| distinct offsets consumed | 197 of the 436 the loader writes |
| offsets consumed that the loader never writes | 0 |
| write sites into the singleton | 1 |

The per-offset consumer lists are the last column of the offset table. The method is a forward
walk of each caller's listing from every `CALL 0x0042e740`, following EAX through register moves
and stack spills and recording each `[reg+disp]` access; the alias set is reset at each getter
call and caller-saved registers are dropped at every other call. It is sound for the direct
pattern and will miss an offset reached after the pointer is passed to another function, so the
consumer lists are a **lower bound**. The eight callers with no resolved access either discard
the result (`004E3AA0` calls the getter at 004E3F5F and uses EAX for nothing, forcing the
singleton to exist during `BSP_Game_OnInit`) or hand the pointer on.

## The `+31Ch` write-back: a clamp, and dead in the shipped game

`+31c` is `Dynamics/AccelCheatMul` and `+320` is `Dynamics/AccelCheatMulMul`. The listing at
007D20F3..007D2144 in `BSP_PlaneClass_ReadLuaFields` is:

```
007d20f3: CALL 0x0042e740
007d20f8: MOVSS XMM0,dword ptr [EAX + 0x31c]     ; AccelCheatMul
007d2100: COMISS XMM0,dword ptr [0x00d7a24c]     ; 1.0f
007d2107: JBE 0x007d212f
007d2115: FLD  float ptr [EAX + 0x320]           ; AccelCheatMulMul
007d211b: FMUL float ptr [EBX + 0x31c]           ; * AccelCheatMul
007d2121: FMUL float ptr [ESI + 0x164]           ; * the row's Accel
007d2127: FSTP float ptr [ESI + 0x164]           ; -> descriptor +164h, no tuning write
007d212d: JMP  0x007d2144
007d212f: CALL 0x0042e740
007d2134: MOVSS XMM0,dword ptr [0x00d7a24c]      ; 1.0f
007d213c: MOVSS dword ptr [EAX + 0x31c],XMM0     ; AccelCheatMul = 1.0f
```

It is a guard, not a per-class mutation of shared tuning, on three separate grounds.

1. The write is reached only on the `<= 1.0f` branch and the value written is the literal
   `1.0f` at `00d7a24c`, the same constant the branch tested against. The field can only move
   to exactly `1.0f`, only from a value at or below `1.0f`, and never further.
2. It is idempotent. Once `+31c` is `1.0f` every later class takes the same branch and rewrites
   the same constant, so repeated class loads do not accumulate. The scaling branch never
   writes the tuning object at all, so a factor above `1.0f` survives every class load.
3. Its shared-state reach is one function. Across all 141 callers, `+31c`, `+320` and `+334` are
   read only by `007D1F70`, and 007D213C is the **only write into the singleton from any of the
   141 call sites**.

With the installed data the branch is never taken: `AccelCheatMul` is `1.5` in
`Scripts\datatables\PlaneGlobals.lua` line 265, which is above `1.0f`, so every plane class
takes the scaling path and 007D213C never executes. The write is dead in the shipped game.

This corrects `docs/PLANE_CLASS_FIELDS.md` line 287, which reads "It is a per-class read
mutating shared state" and leaves the question open.

## Host table

One row per distinct native callee of `007E2A20`, with a representative site. The three fetch
and getter rows stand for the bulk of the body; their site counts are given in the `this/args`
column.

| site | callee | name | this / args / ret | gate |
|---|---|---|---|---|
| `007e2a72` | `008875f0` | `construct_script_override_list` | ECX = a 14h-byte frame local, one dword argument read through `*(*(00e188a8+1A08h)+4)`; returns the object | unconditional, first statement after the vtable store |
| `007e2a86` | `00b66bd0` | `lua_state_construct` | ECX = a frame-local LuaStateOwner; no stack arguments | unconditional |
| `007e2a9c` | `00b6a020` | `lua_state_open` | ECX = the frame-local owner, one argument 41h (library selection mask) | unconditional |
| `007e2ab1` | `0041dd40` | `string_reserve` | ECX = a frame-local string, (1Dh, 1) for the first path and (23h, 1) for the second; each is followed by `_memcpy` of the literal at 007e2acc and 007e2b40 | twice, once per script path |
| `007e2ae9` | `00b69d40` | `run_script` | ECX = the owner, the frame-local path `Scripts\global\luaMW_init.lua`, second argument 0 | unconditional; runs the constants and the DEG/KMH helpers, no key is read from it |
| `007e2b5d` | `00b69d40` | `run_script` | ECX = the owner, the frame-local path `Scripts\datatables\PlaneGlobals.lua`, second argument 0 | unconditional; this is the script every key comes from |
| `007e2b09` | `00419cc0` | `release_path_block` | (block, size+1, 1), paired with `00bd1510` at 007e2b10 and again at 007e2b7d/007e2b84 | guarded by the path buffer being non-null |
| `007e2b98` | `00b67980` | `globals_table` | ECX = the owner, one out-LuaObject argument; returns the out pointer | unconditional |
| `007e2bb4` | `00b67800` | `table_field` | ECX = parent table, (out LuaObject, const char* key); returns out in EAX. 471 sites | unconditional; this first site fetches `PlaneGlobals` (key at 00d085f4) from the globals table |
| `007e2fd8` | `00b67720` | `table_element` | ECX = parent table, (out LuaObject, int one-based index); returns out. 50 sites | used for the array-valued keys |
| `007e2bed` | `00b66270` | `value_number` | ECX = the fetched LuaObject, no stack arguments; returns the number on the x87 stack. 384 sites | unconditional after each fetch |
| `007e2d75` | `00b66290` | `value_integer` | ECX = the fetched LuaObject; returns in EAX. 1 site | only `Dynamics/RotationLimit` |
| `007e825f` | `00b66250` | `value_boolean` | ECX = the fetched LuaObject; returns in AL. 7 sites | the seven boolean keys |
| `007e4649` | `00b66330` | `value_number_or_default` | ECX = the fetched LuaObject, one float argument pushed through `FSTP [ESP]`. 37 sites | the keys that may be absent |
| `007e85db` | `00b67a80` | `value_number_triple` | ECX = the fetched LuaObject, one out argument; returns a pointer to three floats. 2 sites | `Pilot/Follow/SmallPlaneDisplacement` and `Pilot/Follow/BomberDisplacement` |
| `007e2bc8` | `00b67700` | `release_value` | ECX = the LuaObject to release; no arguments, no return. 524 sites | after every fetch, and again for each scope object at the end of its section |
| `007e2c9d` | `00b65f50` | `value_construct` | ECX = a frame-local LuaObject; default construction of a section scope slot | twice, for the two long-lived scope slots |
| `007e2cd1` | `00b67690` | `value_assign` | ECX = a section scope slot, one LuaObject argument; rebinds the slot to a new table. 33 sites | once per section of the table |
| `007eab2b` | `00b669a0` | `lua_state_close` | ECX = the frame-local owner | unconditional, after the last key |
| `007eab3f` | `00887670` | `destroy_script_override_list` | ECX = the 14h-byte frame local built at 007e2a72 | unconditional, last statement |

`008875F0` and `00887670` bracket the whole body. `008875F0` sets a vtable at `00d0e798`, zeroes
four dwords, releases any existing `BSP_LuaVariantVector` and stores its argument at `+4h`; the
argument is read through `*(*(00e188a8 + 1A08h) + 4)`. It is a script-override list handed to
the two script runs. `00B66BD0`, `00B669A0` and `0041DD40` are named from the existing ledger,
not re-derived here.

## Unwritten offsets

The loader writes 436 of the 436 dword slots it touches; within `0..6D0h` the bytes it never
writes are `+3ea`, `+3eb` (padding after the two boolean bytes at `+3e8`/`+3e9`) and the tail
`+6d0` is the end of the block. No caller of `0042E740` reads an offset the loader does not
write, so no consumer observes uninitialised heap through this object.

## Coverage and uncertainty

| routine | coverage |
|---|---|
| `0042E740` | complete |
| `007E2A20` (007E2A20-007EAB5E) | complete for the offset table: every store into `this` is accounted for. The exception-state bookkeeping (`[ESP+19DCh]` and the `00C8F12B` handler) and the `LIBCRT_unmatched_00BF9940` pair at 007E9B3A and 007E9B98 are not modelled. |
| `008875F0` | complete |
| `007D1F70` | partial: only 007D20F3..007D2144 and 007D23F5, read for the `+31Ch` question. `docs/PLANE_CLASS_FIELDS.md` owns the rest. |

* The names `game_tuning_*` in `include/bsp/game_tuning_singleton.hpp` are hypotheses. No
  recovered symbol names the object or any field; the key paths are recovered, the C++ names are not.
* The two `LIBCRT_unmatched_00BF9940` calls at 007E9B3A and 007E9B98 sit next to two `<= 1.0f`
  compares on freshly read values. They are an unidentified CRT routine and the two stores they
  guard are in the table with their keys, so the table is unaffected, but the guard is not modelled.
* The consumer lists are a lower bound, for the reason given above.
* No run-time evidence was collected. `007E2A20` runs once inside the singleton getter and its
  result is fully determined by the installed script, which was read directly; the `+31Ch`
  verdict rests on the branch condition and the installed value `1.5`, both static.

## Follow-ups

| packet | what |
|---|---|
| `game_tuning_field_names` | The offset table gives every field a key path. Naming the 197
consumed offsets in Ghidra as struct fields is mechanical from `local/offsets.json`. |
| `plane_globals_dead_keys` | Whether the 27 unread keys are read by another loader, or are dead. |
| `lua_number_triple_00b67a80` | The array-to-float3 helper, shared with the ship and mothership
class readers. |
