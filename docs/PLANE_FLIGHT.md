# The plane flight controller (packet `cc2_plane_flight`)

Addresses: 007CFD20, 007D7EA0, 007CE040, 00953CC0, 007CC2F0, 007CBFA0, 007CBA50, 007DC830,
007DB680, 007DA710, 007D99C0, 007B9770, 007BB6E0, 007ED0D0, 009FBA50, 009FB800, 009F9E40,
0099B450, 007EAAE1 (in 007E2A20), and read-only 0095DC40, 007C6C30, 007DCCF0, 007DCDD0,
007DA380, 007D81B0, 007D9C10, 007D7C00, 0099D300, 009998A0, 007C2AF0, 009BC3A0, 007F4580,
007F3970.

Worker `agent/cc2-plane-flight`, 2026-09-12 UTC. Ghidra was **read-only** for this packet: no
rename, comment, prototype, function creation or save. Project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Every descriptive name below is a hypothesis, not a
recovered symbol.

## The two corrections that reshape the packet

The brief inherited "the pilot control block at `unit+9D4h`" from `docs/BOT_TASKS.md`. Both
halves of that are wrong, and the right answers are in different places.

**`unit+9D4h` is the plane's squadron.** Scanning `.text` for every `MOV [reg+9D4h]` gives ten
stores and no others; the two that matter are `007ED0E6` and `007F4B49`, and both write a
squadron pointer:

| site | containing function | what it writes |
| --- | --- | --- |
| `007CFE6C` | `007CFD20`, the plane unit constructor | `0` (`XOR EBX,EBX` at `007CFD56`); `+9D8h` becomes `-1` (`OR EDI,0FFFFFFFFh` at `007CFD69`) |
| `007ED0E6` | `007ED0D0` `__thiscall(squadron, plane, spawnIndex)` | `plane+9D4h = squadron`, `plane+9D8h = spawnIndex`, then the sorted insert into `squadron+3D0h[]` keeping `+9D8h` order, `+3CCh += 1`, and `+3ECh = 1` set on entry at `007ED0D7` |
| `007F4B49` | `BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes` | the same back pointer, step 13 of `docs/PLANE_SQUADRON.md` |
| `007F3A07` | `BSP_Squadron_RemovePlane` | clears it |
| `007CDF6C`, `007D68D5`, `007D694B`, `007ECE68`, `007ECE80`, `007ED21A` | not read | `contract: unread` |

That agrees with every reader already in the repository: the leader test
`[[unit+9D4h]+3D0h] == unit` (`docs/HUD_CENTRAL_UPDATES.md`), the member count
`[unit+9D4h]+3CCh` (`docs/MISSION_RESULT_DECISION.md`), the accessor `007B97E0`
(`MOV EAX,[ECX+9D4h]; RET`, `docs/GUNNERY_TABLES.md`) and the back pointer of
`docs/AIR_OPERATIONS.md`. So the altitude fields `docs/BOT_TASKS.md` found at `+394h`,
`+398h`, `+39Ch` and the dirty byte `+3ADh` are **squadron** fields: a formation-level
cruising altitude and its limits, not per-plane pilot input. `009FBA50` reads `+394h`
exactly that way, as a ceiling the whole squadron shares.

**The pilot control block is `unit+9E4h`.** Five floats and three bytes, all zeroed by the
plane unit constructor except the throttle, which `007CFEB0` seeds with `1.0f`:

| offset | axis | constructor | consumer |
| --- | --- | --- | --- |
| `+9E4h` | roll | `007CFE92` zero | `0099B476`, the latch `007B9770` |
| `+9E8h` | pitch | `007CFE98` zero | `0099B4B2`, the latch |
| `+9ECh` | yaw | `007CFE9E` zero | `0099B494`, the latch |
| `+9F0h` | throttle | `007CFEB0` `1.0f` (`00D7A24C`) | `0099B456`, the latch |
| `+9F4h` | fifth axis | `007CFEA4` zero | `0099B4D0`, the latch |
| `+9F8h` | byte | `007CFEAA` zero | `007DC860` copies it to `ctl+5h` |
| `+9F9h` | byte | - | `007DC84F` copies it to `ctl+4h` |
| `+9FAh` | byte | - | the latch only |

`007B9770` (Ghidra body `007B9770`-`007B97D2`, `RET`, no argument) is the whole latch
and is short enough to give in full: it copies `+9E4h`..`+9F4h` into `+BB0h`..`+BC0h` and the
three bytes into `+BC8h`..`+BCAh` **permuted** - `+9F8h` to `+BC8h`, `+9FAh` to `+BC9h`,
`+9F9h` to `+BCAh`. So `unit+BB0h` is the previous step's input, which is what the rate law
`007DA710` reads. The plane tick calls the latch at `007CE96F`.

## The controller object: `unit+AB0h`

`007D7EA0(this = unit+AB0h, unit)` is the constructor, called once from the plane unit
constructor (`007CFF69 PUSH ESI` / `007CFF6A LEA ECX,[ESI+0AB0h]` / `007CFF87 CALL`). It is the
object every flight routine in segments 47 and 48 receives.

| offset | value | site |
| --- | --- | --- |
| `+0h` | vtable `00D06848` | `007D7EAD` |
| `+4h`, `+5h` | bytes `0`; refreshed each step from `unit+9F9h`/`+9F8h` | `007D7EB3`, `007D7EB6`; `007DC85D`, `007DC86B` |
| `+8h` | **the unit** | `007D7EB9` |
| `+0Ch` | **`unit+538h`, the plane class descriptor** | `007D7EBC`/`007D7EC7` |
| `+10h` | `operator new(0D0h)` with vtable `00D06844`; `+88h`/`+8Ch`/`+90h` from `[00F87574..7C]`, `+A4h`, `+C0h`, `+C4h`, `+C8h`, `+CCh` zeroed | `007D7ECA`-`007D7F4F` |
| `+14h` | byte `0` | `007D7F52` |
| `+18h`, `+3Ch` | two blocks `007DB6B6` hands to `007D7C00` | `007DB6AE`, `007DB6B2` |
| `+44h` | the speed fallback when `unit+3Ch` is null | `007D99CA` |
| `+8Ch`, `+ACh` | `0.0f` | `007D7F55`, `007D7F6D` |
| `+90h` | `99.0f` (`00D059A0`) | `007D7F3F` |
| `+98h`, `+9Ch` | `1.0f` | `007D7F5D`, `007D7F65` |
| `+FCh` | a mode dword, zeroed each step and tested `== 1` | `007DC841`, `007DB6D1` |

The vtable at `00D06848` holds one live slot (`007D7FF0`, the destructor) and a null; the
class is effectively non-polymorphic. `coverage: partial` - the `0D0h` sub-object at `+10h`
was read only as far as its constructor, and `+18h`/`+3Ch` only as `007D7C00`'s arguments.

## The per-step law

The plane's tick element is the node at `unit+310h` with vtable `00D05EDC`, installed by
`007CFD20` at `007CFDA0` (and again by `007D03E9`). **Every derived plane class keeps the same
two motion slots**: `007D7757` installs `00D065F4`, `007DD9E7` `00D068DC`, `00951B97`
`00D19CE4`, `00951C77` `00D19FBC` and `00951D57` `00D1A294`, and all five tables carry
`007C6500` at `+4h` and `007CE040` at `+8h`. So one routine advances every plane in the game.

`007CE040` (Ghidra body `007CE040`-`007CF172`, 1095 listed instructions) is
`__thiscall(node, float step)`.
`ESI` is the node and `EDI` is the unit (`007CE0ED LEA EDI,[ESI+0FFFFFCF0h]`, that is
`-310h`). Most of its body is damage, fire, collision, sound and effects - the string
immediates are `powerlost`, `explosion` and `Collided`. The motion is four steps.

| step | site | rule |
| --- | --- | --- |
| 1 | `007CE08F` | `00953CC0(node, step)`, the level-4 unit tick |
| 2 | inside `00953CC0`, `00953CFD` | `unit->vtable[+1F0h](step)`; the plane's is `0095DC40`, a double-tap detector over the input manager and `GameSettings+30h`/`+34h` that writes `unit+63Ch`/`+640h`/`+644h` - the turbo gesture, not a control law |
| 3 | inside `00953CC0`, `00953D98` | `unit->vtable[+1D8h](step)`, **only when `unit+520h` is clear** (`00953D7D CMP byte [ESI+210h],0` then `00953D84 JNZ`); the plane's is `007C6C30`, the out-of-action countdown at `unit+6F0h` that also drives the squadron's `+36Ch` and `00982120` |
| 4 | `007CEC30`-`007CECB4` | the motion dispatch, below |

The dispatch is three mutually exclusive arms and a shared tail:

| arm | gate | call |
| --- | --- | --- |
| free flight | `(*(unit+72Ch))->vtable[+38h]()` is true (`007CEC3F`); first, when `unit+9E0h` is clear, `unit+908h += step` (`007CEC4E`) | `007CEC6E` `007CC2F0(unit, step)` |
| ground roll | `unit+900h` is `4` or `5` (`007CEC7B`, `007CEC80`) | `007CEC92` `007CBFA0(unit, step)` |
| surface | `unit+5F0h == 6` (`007CEC99`) | `007CECAF` `007CBA50(unit, step)` |
| tail | any arm ran | `007CECBA` `0085DC80(unit+674h)` |

When no arm matches, `007CECA0 JNZ` skips the tail as well, so the committed pose is left
untouched for that step.

### The free-flight chain

```
007CC2F0(unit, step)                                   ; Ghidra body 007CC2F0-007CC579
  unit->vtable[+1ECh](step)                            ; 007CC322, the plane's is 007CAF10
  007DC830(unit+0AB0h, step)                           ; 007CC332
007DC830(ctl, step)                                    ; Ghidra body 007DC830-007DCCE2
  ctl->+FCh = 0                                        ; 007DC841
  ctl->+4h = unit->+9F9h ; ctl->+5h = unit->+9F8h       ; 007DC84F..007DC86B
  007DB680(ctl, step, 0.0f, 0.0f, 0)                    ; 007DC86E
  ratio = 007D99C0(ctl) / classDesc->+184h StallSpd     ; 007DC878..007DC87D
  ... InterpolateClamped chains over the Dynamics mirror ; 007DC8C9, 007DC944, ...
007DB680(ctl, step, a, b, flag)                        ; Ghidra body 007DB680-007DC82A
  007D81B0(ctl, step)                                  ; 007DB690
  007D9C10(ctl)                                        ; 007DB697
  007DA710(ctl, step)                                  ; 007DB6A6, the rate law
  007D7C00(ctl->+10h, ctl+18h, ctl+3Ch)                 ; 007DB6B6
  live = unit+9E4h ; latched = unit+0BB0h               ; 007DB6C7, 007DB6DE
  ... the stall and speed block, tuning+518h WireRope
```

`007D99C0(ctl)` is the forward-speed getter: when `unit+3Ch` (the body proxy) is non-null it
refreshes the pose, builds the inverse affine into `unit+110h` through
`00B63D50 BSP_Matrix_BuildOrthogonalScaledAffineInverse` and transforms the body velocity into
plane space; otherwise it returns `ctl->+44h`.

`007CAF10` (**no Ghidra function**, `007CAF10`-`007CB431`, `RET 4`) is the plane's
`vtable[+1ECh]`, and it is the last writer of the pilot control block before the law reads it.
Two blocks were read. At `007CB161`-`007CB1B3` it clamps each of `+9E4h`, `+9E8h` and `+9ECh`
between an upper bound in `XMM1` and a lower bound in `XMM2`, a per-axis control limiter. At
`007CB3B9`-`007CB3DC` it forces `+9E4h` and the latch `+BB0h` to a constant (`[00CE74F8]` or
`[00D05E14]`, selected by a byte test at `007CB3B7`) and zeroes `+9E8h`, `+BB4h` and `+9ECh`,
a control reset. `coverage: partial`: those two blocks only, `007CAF10`-`007CB161` and
`007CB1B3`-`007CB3A6` and the tail past `007CB3DC` unread.

`007DA710(ctl, step)` (Ghidra body `007DA710`-`007DAFCD`) is the control-rate law. `EDI = ctl->+8h` is the
unit and `EBP = ctl->+0Ch` is the class descriptor, so every gain is a per-class field and
every input is a **latched** axis:

| axis | site | rule |
| --- | --- | --- |
| roll | `007DA7C2`-`007DA7DA` | `classDesc+1A8h RollSpd` times three stacked factors from `007DA380`'s three out-parameters |
| turbo / stall blend | `007DA791`-`007DA7B5`, `007DA818`-`007DA8D5` | `BSP_Math_InterpolateClamped` over `unit+0C3Ch` and `unit+0C64h` with `[00F8730C]`/`[00F87310]`, and a `BSP_Math_MaxFloatByRef` against `[00F87338]` scaled by `[00D7A328]` = 4.0 |
| pitch | `007DA8EB`-`007DA922` | `classDesc+1ACh PitchSpd` times the latched pitch `unit+0BB4h`, and again by `classDesc+1D8h NegativePitchRatio` when that input is negative |
| yaw | `007DA926`-`007DA934` | `classDesc+1B0h YawSpd` times the latched roll `unit+0BB0h` and `unit+0BC4h` |
| controller mode | `007DA8D9` | when `ctl->+FCh != 1` the whole roll term is discarded |

`coverage: partial` for `007DA710`: the four gains, the three latched inputs and the mode gate
are read; `007DA380`'s three out-parameters and the tail past `007DA934` are not.

The other two arms reach the same core. `007CBFA0` calls `007DCCF0(ctl, step)`, which calls
`007DB680` and `007DC830`; `007CBA50` calls `007DCDD0(ctl, step)`, which adds
`0078CF20 BSP_GameWorld_SampleWaterHeight`, `0078D3D0`, `BSP_Math_SubtractWrappedAngle` and
`007DAFD0`. Both are `contract: unread` past the call sites.

### How the result reaches the pose

**The plane never touches the physics rigid body.** `00C37E50` (linear velocity) has seven
callers and `00C37E20` (angular velocity) five; all are the ship controller (`0092D300`,
`0092D770`, `0092E5B0`, `0092E8C0`, `009329C0`, `00936DC0`, `00937630`) plus
`00447510 BSP_GameDynamicsList_Add`. `docs/UNIT_FORCE_COMMANDS.md` already established the
same for `AddForce` and `AddTorque`. So the plane path is kinematic, through the tick
element's pose slots (`+4h` `007C6500`, `+0Ch` `007BEEE0`) and the matrix composition at
`007CED40`-`007CED6B` (`00414DB0`, then `00413920 BSP_Matrix_Multiply4x4`, then
`004134F0 BSP_Matrix_Copy4x4X87`) over `unit+338h`..`unit+3B4h`. That composition is
`contract: unread`; `docs/ENTITY_LOCAL_MATRIX.md` and `docs/DYN_PHYSICS_SUBSTEP.md` own it.

## The three pilot command routines

All three take the same receiver, and it is **not** the approach controller.
`009C18C0 BSP_BotStateMoveTo_Tick` sets `EDI = ECX` at `009C18C9`, `ESI = EDI + 4` at
`009C19A8` (the only ESI write between there and the call) and `ECX = ESI` at `009C1AFF`, one
instruction before `009C1B17 CALL 009FBA50`. So `this` is `state+4h`, the state's
owner-approach slot (`bot_task_state_off::kOwnerApproach`), and each routine dereferences it
once: `MOV ECX,[ESI]` / `MOV EAX,[EDI]`. From the approach they read `+4h` the unit, `+8h` the
class descriptor (`unit+538h`), `+0Ch` the squadron (`unit+9D4h`) and `+18h` the command block
(`task+4h`).

### `009FBA50`, the cruising-altitude command

`float __thiscall(this, float base, float rangeLow, float rangeHigh, float scale)`,
`RET 10h`, body `009FBA50`-`009FBB1C`.

| step | site | rule |
| --- | --- | --- |
| 1 | `009FBA51`-`009FBA77` | `span = max(rangeHigh - rangeLow, 0)` |
| 2 | `009FBA7D`-`009FBA95` | `ceilingLimit = tuning->+210h Dynamics/Ceiling - 50.0` (`[00CE3938]`) |
| 3 | `009FBA9B`-`009FBABF` | with a squadron, `ceilingLimit = min(ceilingLimit, squadron->+394h)` |
| 4 | `009FBAD2`-`009FBAE5` | when `span > 0`, `base += span * scale * classDesc->+518h` |
| 5 | `009FBAE9`-`009FBB03` | `alt = min(base, ceilingLimit)` |
| 6 | `009FBB03`-`009FBB13` | `009FB800(this, alt, base)` - the second argument is the value **before** the ceiling clamp |

The `FLD` at `009FBAC5` is never popped, so `scale` is still in `ST0` at the `RET`: the native
ABI returns a float. Every call site discards it.

### `009FB800`, the pitch command from an altitude error

`void __thiscall(this, float desiredAltitude, float reference)`, `RET 8`, body
`009FB800`-`009FBA4F`. It writes `cmd->+2BCh` and sets `cmd->+2D0h = 2` on every path.

| step | site | rule |
| --- | --- | --- |
| 1 | `009FB809`-`009FB82D` | `alt = min(desiredAltitude, Ceiling - 50)` |
| 2 | `009FB833`-`009FB844` | `unit = approach->+4h`; refresh the pose when `unit+C8h == 0` |
| 3 | `009FB849` | `err = alt - unit->+100h`, the world Y |
| 4 | `009FB858`-`009FB86E` | `x = err * ((reference + 1.0) * 0.5)` (`[00D7A210]`, `[00D7A280]`) |
| 5a | `009FB88D`-`009FB94B` | climb, `x > 0`: `limit = max(classDesc+1ECh * 1.6, DEG(40))`; `t = clamp(x / tuning+544h Pilot/General/ClimbDist, 0, reference)`; `cmd->+2BCh = min(classDesc+1ECh * t, limit)` |
| 5b | `009FB96E`-`009FBA4D` | dive, `x <= 0`: `limit = max(classDesc+1F0h DropAngle * 1.6, DEG(60))`; `t = clamp(-x / tuning+548h Pilot/General/DropDist, 0, reference)`; `cmd->+2BCh = -min(DropAngle * t, limit)`, negated through `[00D7A208]` |

`1.6` is `[00CE3D48]`, `DEG(40)` is `[00CE7D20] = 0.69813174f` and `DEG(60)` is
`[00D05AAC] = 1.0471976f`.

**`classDesc+1ECh` has no producer.** It is inside the `+1E4h`/`+1E8h`/`+1ECh` gap between
`.WheelBrake +1E0h` and `.DropAngle +1F0h` in `docs/PLANE_CLASS_FIELDS.md`'s 70 keys; scanning
`.text` for every `MOV`, `MOVSS`, `FST` and `FSTP` at displacement `1ECh` finds no store into a
plane descriptor, and `ClimbAngle` appears zero times in the installed
`Scripts/datatables/autoload/vehicleclasses.lua`. So the field is zero for every shipped class,
the climb `limit` falls back to the `DEG(40)` floor, and `min(0 * t, DEG(40))` makes the climb
arm command **nothing**: an AI plane pitches down toward a lower cruising altitude but never
pitches up toward a higher one through this routine. `producer: unread`.

### `009F9E40`, the heading command

`void __thiscall(this, const float target[3])`, `RET 4`, body `009F9E40`-`009F9ECE`.

1. `approach = this->+0h`; `unit = approach->+4h`; refresh the pose when `unit+C8h == 0`.
2. `h = PI/2 ([00CE3830]) - atan2(target[2] - unit->+104h, target[0] - unit->+0FCh)`; the x87
   order at `009F9E6A`/`009F9E7A` puts `dz` in `ST1` and `dx` in `ST0`, so the library sees
   `y = dz`, `x = dx`.
3. `if (h < 0) h += 2*PI ([00CE3828])`.
4. `cmd = approach->+18h`; `cmd->+2C0h = h`; `cmd->+2CCh = 2`.

That is the game's clockwise-from-north yaw convention, wrapped into `[0, 2*PI)`.

## The tuning the controller reads

The flight code never calls `0042E740` per axis. It reads absolute globals such as
`[00F87310]`, and those are a **verbatim mirror of the whole `Dynamics/*` tuning group**:

```
007E2D03  LEA ESI,[EBP+210h]        ; EBP = the 6D0h tuning singleton (007E2A42)
...                                 ; the only ESI write after 007E2A61 XOR ESI,ESI
007EAAD7  MOV ECX,4Eh               ; 78 dwords = 138h bytes
007EAADC  MOV EDI,00F872F0h
007EAAE1  REP MOVSD ES:[EDI],[ESI]
```

So `00F872F0`..`00F87427` is `tuning+210h`..`tuning+347h`, and
`tuning_offset = mirror_address - 00F870E0h`. The window starts at
`+210h Dynamics/Ceiling` and ends at `+344h Dynamics/SpdMultipliers/LevelBombSlowMul`, which
is exactly the `Dynamics/*` group of `docs/GAME_TUNING_SINGLETON.md`. The `rep movsd` is the
only producer: these addresses are above `.data`'s raw length (VA `00E08000`, vsize `297EDCh`,
raw `10000h`, so everything past `00E18000` is zero-filled at load) and `.text` contains no
absolute store into the window.

| mirror | tuning | key | default |
| --- | --- | --- | --- |
| `00F872F0` | `+210h` | `Dynamics/Ceiling` | 1500 |
| `00F8730C` | `+22Ch` | `Dynamics/SpdMultipliers/StallRangeMin` | 1.2 |
| `00F87310` | `+230h` | `Dynamics/SpdMultipliers/StallRangeMax` | 1.6 |
| `00F87314` | `+234h` | `Dynamics/SpdMultipliers/StallOffPitch` | DEG(-15) |
| `00F87330`, `00F87334` | `+250h`, `+254h` | `Dynamics/DeadMeat/RotationMin` (both slots) | DEG(10) |
| `00F87338` | `+258h` | `Dynamics/DeadMeat/SpinRollSpd` | 5.0 |
| `00F8733C` | `+25Ch` | `Dynamics/DeadMeat/RollMulTime` | 6 |
| `00F87340` | `+260h` | `Dynamics/DeadMeat/RollMul` | 2.0 |
| `00F8734C` | `+26Ch` | `Dynamics/DeadMeat/SpinStallMul` | 10 |
| `00F87350` | `+270h` | `Dynamics/DeadMeat/SpinStallMulTime` | 4 |
| `00F87354` | `+274h` | `Dynamics/DeadMeat/SpinStallMulOnPitch` | DEG(0) |
| `00F87358` | `+278h` | `Dynamics/DeadMeat/SpinStallMulOffPitch` | DEG(-75) |
| `00F8735C` | `+27Ch` | `Dynamics/DeadMeat/StallMul` | 1 |
| `00F87360` | `+280h` | `Dynamics/DeadMeat/StallMulTime` | 4 |
| `00F87364` | `+284h` | `Dynamics/DeadMeat/StallMulOnPitch` | DEG(10) |
| `00F87368` | `+288h` | `Dynamics/DeadMeat/StallMulOffPitch` | DEG(-25) |
| `00F87388` | `+2A8h` | `Dynamics/RunwayYawTurnSpdLimit/1` | KMH(25) |
| `00F873D0` | `+2F0h` | `Dynamics/Water/MinCtrlAngle` | DEG(20) |
| `00F873FC` | `+31Ch` | `Dynamics/AccelCheatMul` | 1.5 |

`007DB680` reads `tuning+518h Pilot/Landing/WireRope` (7.25) and `009FB800` reads `+544h`
`Pilot/General/ClimbDist` (130) and `+548h` `Pilot/General/DropDist` (200) through
`0042E740` directly; those three are outside the mirror.

### The per-class gains, and one installed row

Every rate in the law is a plane class descriptor field, and the descriptor is `unit+538h`.
The installed values below are the `Fighter`-typed `Shooting Star` (P-80) row of
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/Scripts/datatables/autoload/vehicleclasses.lua`,
the first row in the file whose `["Type"]` is `"Fighter"` (line 2452).

| offset | key | Shooting Star | read at |
| --- | --- | --- | --- |
| `+164h` | `Accel` | 5 | `007C4850` |
| `+17Ch` | `StallRotAccel` | 0.872665 | `007DA710` band |
| `+184h` | `StallSpd` | 17.5 | `007DC87D`, `007DB760` |
| `+188h` | `MaxSpd` | 125 | `007C4850` band |
| `+18Ch` | `TravelSpeed` | 141.666672 | `007D23C3` writer; `+190h` is the scaled copy |
| `+1A8h` | `RollSpd` | 1.22173 | `007DA7C2` |
| `+1ACh` | `PitchSpd` | 0.523599 | `007DA8EB` |
| `+1B0h` | `YawSpd` | 0.279253 | `007DA926` |
| `+1B4h` | `YawRollRatio` | 0.6 | `007DA710` band |
| `+1BCh` | `RollAccel` | 2.443461 | `007DA710` band |
| `+1C0h` | `PitchAccel` | 1.22173 | `007DA710` band |
| `+1C4h` | `YawAccel` | 0.523599 | `007DA710` band |
| `+1C8h` | `TurnRollSpd` | 0.10472 | `009B2E50`, `009B3040` |
| `+1CCh` | `YawLimitAngle` | 0.436332 | `007DB680` band |
| `+1D0h` | `PitchLimitAngle` | 0.436332 | `007DB680` band |
| `+1D4h` | `DragPitchRatio` | 0 | `007DB680` band |
| `+1D8h` | `NegativePitchRatio` | 0.8 | `007DA918` |
| `+1DCh` | `AirBrakeDrag` | 0.3 | `007DB680` band |
| `+1E0h` | `WheelBrake` | 80 | ground arm |
| `+1ECh` | (no key) | absent, so 0 | `009FB88D`, `009FB90C` |
| `+1F0h` | `DropAngle` | 0.698132 | `009FB979`, `009FB9FA` |
| `+268h` | `TurnCircleRadius` | 1100 | `007C4850`, `009B2E50` |
| `+274h` | `RollMaxforceLimit` | 0.174533 | `0099D300` |
| `+278h` | `PitchMaxforceLimit` | 0.436332 | `0099D300` |
| `+518h` | (no key) | absent, so 0 | `009FBADB` |

`+518h` shares `+1ECh`'s problem: no Lua key writes it, so `009FBA50` step 4 contributes
nothing and the cruising altitude is the caller's `base` clamped by the ceiling and the
squadron. `producer: unread` for both.

## Where the bot's plan meets the control block

`009998A0 BSP_PilotBot_Update` runs `0099B450 BSP_PilotBot_SeedPlanSlots` first. That routine
is a pure copy from the pilot control block into five `{current, desired, flag}` triples on
the command block at `task+4h`:

| command block | seeded from | site |
| --- | --- | --- |
| `+274h`, `+278h`, flag `+27Ch` | `unit+9F0h` throttle | `0099B456`-`0099B470` |
| `+280h`, `+284h`, flag `+288h` | `unit+9E4h` roll | `0099B476`-`0099B48E` |
| `+28Ch`, `+290h`, flag `+294h` | `unit+9ECh` yaw | `0099B494`-`0099B4AC` |
| `+298h`, `+29Ch`, flag `+2A0h` | `unit+9E8h` pitch | `0099B4B2`-`0099B4CA` |
| `+2A4h`, `+2A8h`, flag `+2ACh` | `unit+9F4h` fifth axis | `0099B4D0`-`0099B4D8` |

`0099D300 BSP_PilotBot_PlanControls` then rewrites the five `desired` halves and sets their
flags - its early-out at `0099D34F`-`0099D3BD` writes exactly `+278h`/`+27Ch`, `+284h`/`+288h`,
`+290h`/`+294h`, `+29Ch`/`+2A0h` and `+2A8h`/`+2ACh`, with `+278h` set to `1.0f` and the rest
to `0.0f`. So the bot's whole output is a five-axis override of the same pilot control block
the flight controller consumes, and the three helpers above (`009FBA50`, `009FB800`,
`009F9E40`) feed the *demand* fields `+2BCh`, `+2C0h`, `+2B4h` that `0099D300` turns into those
five axes.

**The write-back into `unit+9E4h` was not found.** Scanning every store at displacement `9E8h`
gives eight sites in the plane segments, and four of them are now attributed:

| site | owner | role |
| --- | --- | --- |
| `007CB185`, `007CB3CE` | `007CAF10`, the `+1ECh` slot | the per-axis clamp and the control reset, above |
| `007BB75D`, `007BB8E8` | `007BB6E0` (from `007BB920`, from the tick at `007CE865`) | the signed-byte round trip `q = ftol(v*127 + 128.5)` clamped to `[1, 0FFh]` then `(q - 128)/127`, so a locally flown plane and a replicated one see the same input |
| `007C2B11` | `007C2AF0` | the multiplayer snapshot out of the block |
| `007D1676` | a block Ghidra has no function for; start not established | the multiplayer restore into the block, the inverse of `007C2AF0` (`007D166A` reads `[EDI+38h]`, the field `007C2B0E` wrote) |
| `007CA509` | `007CA3F0` (Ghidra body `007CA3F0`-`007CA5E6`) | `contract: unread` |
| `007D1333` | a block Ghidra has no function for; start not established | `contract: unread` |

So the routine that turns the local player's stick or the bot's five planned axes into
`unit+9E4h` is still `contract: unread`; `007CAF10` only bounds and resets what is already
there.

## Takeoff and landing

`contract: unread`, beyond two facts. `007DB680` reads `tuning+518h Pilot/Landing/WireRope`
(7.25), the arrestor-wire parameter, so the landing hook lives in the core law rather than in a
separate mode; and the ground-roll arm's gate is `unit+900h` in `{4, 5}`, the same field
`007DA710` tests for `== 6`, so the plane's ground/air/water state machine is one enum at
`unit+900h` with a second at `unit+5F0h`. The catapult and airbase launch of
`docs/AIR_OPERATIONS.md` were not re-read.

## Coverage

`complete`: the ten `+9D4h` producers and the proof that the field is the squadron; the pilot
control block `unit+9E4h` and the latch `007B9770` in full; the controller object `unit+AB0h`
and its constructor `007D7EA0`; the five plane tick vtables and the proof that all share
`007CE040`; `00953CC0`'s two virtual arms and the `unit+520h` gate; the three-arm motion
dispatch `007CEC30`-`007CECB4`; `009FBA50`, `009FB800` and `009F9E40` in full with their ABI;
`0099B450` in full; the `Dynamics/*` mirror and its single producer; the class-field table.

`partial`: `007CE040` (the motion path only; the damage, effect, collision and sound body
between `007CE094` and `007CEC30` and past `007CECF6` is not read); `007DC830`
(`007DC830`-`007DC88E` and the two interpolation blocks; `007DCA17`-`007DCB47` unread);
`007DB680` (the five callees and the two control-block pointers; the body
`007DB760`-`007DC82A` unread); `007DA710` (the four gains, three latched inputs and the mode
gate; `007DA380`'s out-parameters and the tail past `007DA934` unread); `007BB6E0` (the first
axis in full, the other four by repetition); the controller's `+10h` sub-object.

`contract: unread`: `007CBFA0`, `007CBA50`, `007DCCF0`, `007DCDD0`, `007C6500`, `007BEEE0`,
`007CAF10`, `007D81B0`, `007D9C10`, `007D7C00`, `007DA380`, `007DAFD0`, `007D7A80`, `0095DC40`
past its gesture, `007C6C30` past its countdown, the matrix composition at `007CED40`, the
writer of `unit+9E4h`, `0099D300`'s body past its early-out, `007CDF6C`/`007D68D5`/`007D694B`/
`007ECE68`/`007ECE80`/`007ED21A`, and the producers of `classDesc+1ECh` and `classDesc+518h`.

No run-time evidence is offered: `bsp_game.exe` has no plane and no fixed-step plane tick, so
rule 6 of `docs/WORKER_VERIFICATION_CHECKLIST.md` does not apply to any claim here.

## Corrections

* `docs/BOT_TASKS.md`: "the pilot control block at `unit+9D4h`" - `unit+9D4h` is the plane's
  squadron (`007ED0E6`, `007F4B49`, cleared at `007F3A07`, zeroed at `007CFE6C`); `+394h`,
  `+398h`, `+39Ch` and `+3ADh` are squadron fields. The pilot control block is `unit+9E4h`.
* `docs/BOT_TASK_STATES.md`: the receiver of `009FBA50`, `009FB800` and `009F9E40` is `state+4h`,
  the owner-approach slot, not the approach (`009C19A8 LEA ESI,[EDI+4]`, `009C1AFF MOV ECX,ESI`,
  `009C1B17 CALL`); each routine dereferences it once. The `+38h` in that doc's `009FBA50`
  argument list is the state's own `+38h` (`009C1B09 FLD [EDI+38h]` with `EDI` still the
  state's `this` from `009C18C9`).
* `docs/BOT_TASK_STATES.md`: "`unit+9D4h` (`approach->+0Ch`) only carries the altitude limits" -
  it carries them because it is the squadron, so those limits are shared by the formation.
* `docs/TICK_ELEMENT_OVERRIDES.md`: "finally, while that byte is set, `unit->vtable[+1D8h](step)`"
  is inverted. `00953D7D CMP byte ptr [ESI+210h],0` with `00953D84 JNZ 00953D9A` calls the slot
  only when `unit+520h` is **clear**.
* `config/tags`: `007CFD20` carries the tag `CG_array_ctor_helper_007cfd20`. It is the level-5
  plane unit instance constructor: `007CFD4B` calls the level-4 constructor `0095CC90`,
  `007CFD78`-`007CFDAA` install the eight vptrs, and `007CFF87` builds the flight controller.
  There is no element loop or stride in the body. Ghidra was read-only for this packet, so the
  tag was not changed; the ledger now carries `BSP_PlaneUnitInstance_Construct`.

## Open questions

* Which routine writes the local player's stick into `unit+9E4h`. Four candidate sites remain.
* Why `classDesc+1ECh` and `classDesc+518h` have no producer. Either an unread constructor
  writes them or the shipped data really does leave the climb gain and the cruise-altitude
  gain at zero, which would mean the AI climb command is dead code.
* What `unit+900h`'s six values mean. `4` and `5` take the ground arm, `6` is tested inside
  `007DA710` and `unit+5F0h == 6` takes the surface arm; the enum was not enumerated.
* Whether `009FBA50`'s leaked `ST0` is a deliberate float return that some caller reads. All
  four call sites this packet saw discard it.

## no_ghidra_function

Three of the routines this doc names by name have no Ghidra function. Every other named
address does, and its Ghidra body range is quoted where it is first introduced. Two further
call sites, `007D1333` and `007D1676`, lie inside blocks Ghidra has no function for; their
block starts were not established, so they are listed as sites rather than routines.

| address | name | end_address |
| --- | --- | --- |
| `007C6500` | `BSP_PlaneTickElement_AdvancePose` | `007C675D` |
| `007C6C30` | `BSP_Plane_OutOfActionCountdown` | `007C6E0A` |
| `007CAF10` | `BSP_Plane_LimitAndResetControls` | `007CB431` |

Each end address is the inclusive last byte of the `RET` that is followed by `INT3` padding,
taken from the disk bytes; the next referenced entry point begins at `007C6760`, `007C6E10` and
`007CB440` respectively, so the three blocks are bounded on both sides.

## Correction from docs/PILOT_CONTROLS.md (packet cc2_pilot_controls)

- **Was:** "The write-back into unit+9E4h was not found"; 007BB6E0 is "the signed-byte round trip q = ftol(v*127 + 128.5) ... stored back into unit+9E4h..+9F4h", source and destination the same fields
  **Is:** 007BB6E0 is the write-back. Its source is a separate pilot command block at unit+9FCh..+A10h and its destination is unit+9E4h..+9F4h; the round trip is a transform between two blocks, not an in-place projection
  **Evidence:** 007BB982 LEA EAX,[ESI+9FCh]; 007BB988 PUSH EAX; 007BB989 MOV ECX,ESI; 007BB98B CALL 007BB6E0, with ESI the unit from 007BB921 MOV ESI,ECX. Inside the callee 007BB6E2 MOV EDI,[ESP+0Ch]; 007BB6E6 FLD [EDI] -> 007BB72A FSTP [ESI+9E4h]; 007BB730 FLD [EDI+4] -> 007BB75D [ESI+9E8h]; 007BB763 FLD [EDI+8] -> 007BB798 [ESI+9ECh]; 007BB83A FLD [EDI+0Ch] -> 007BB877 [ESI+9F0h]; 007BB87D FLD [EDI+10h] -> 007BB8EE [ESI+9F4h]
- **Was:** "At 007CB161-007CB1B3 it clamps each of +9E4h, +9E8h and +9ECh between an upper bound in XMM1 and a lower bound in XMM2, a per-axis control limiter"
  **Is:** the clamp block runs to 007CB1EB and covers all five axes, with two different lower bounds: +9E4h, +9E8h and +9ECh are clamped to [-1.0f, 1.0f] and +9F0h and +9F4h to [0.0f, 1.0f] with an immediate zero as the lower bound
  **Evidence:** stores 007CB161, 007CB183 and 007CB1A5 against 00D7A260 = -1.0f and 00D7A24C = 1.0f; 007CB1C7 and 007CB1EB take the 0.0 immediate path (the decompiled form is if (0.0 <= v) { if (1.0f < v) v = 1.0f; } else v = 0.0;). 007CB1F3 CALL 007B9770 follows
- **Was:** "What unit+900h's six values mean. 4 and 5 take the ground arm, 6 is tested inside 007DA710"
  **Is:** unit+900h has at least eight values, 0 through 7, all seven non-zero ones written as immediates or through the setter
  **Evidence:** a .text scan for MOV dword [reg+900h],imm32 gives 007C1697 = 4, 007C171E = 5, 007C63F4 = 6, 007C6481 = 7, 007C7183 = 7, 007C7488 = 4, 007CBA12 = 6, 007CC7E2 = 2, 007CC857 = 1, 007D6600 = 7; the setter 007C1430 has switch arms for 3, 4, 5, 6 and 7 and zeroes the throttle for 2
- **Was:** 007D1676 and 007D1333 belong to "a block Ghidra has no function for; start not established"
  **Is:** both starts are established. 007D1333 is in the raw block 007D0B80-007D1353, whose SEH prologue 6A FF / 64 A1 00000000 begins at 007D0B80 right after the two int3 at 007D0B7E; 007D1676 is in the raw block 007D1360-007D1B4B, which follows the twelve int3 at 007D1354
  **Evidence:** ghidra bytes 007D0B78 gives 00 00 83 C4 3C C3 CC CC 6A FF 64 A1 00 00 00 00; disasm-raw 007D1354 gives twelve int3 then 007D1360 MOV EAX,[00E188A8]; SUB ESP,8Ch; the next int3 run after each block is at 007D1354 and 007D1B4C, and the byte before each is C2 04 00, RET 4

## Correction from docs/PLANE_GROUND_OPS.md (packet cc2_plane_ground_ops)

- **Was:** the motion dispatch's surface arm is gated on a second enum, "surface | unit+5F0h == 6 | 007CECAF 007CBA50", and the doc's open question asks what unit+900h's six values mean
  **Is:** the gate is unit+900h == 6, the Water value of the eight-value enum; there is no enum at unit+5F0h
  **Evidence:** in 007CEC30-007CECB4 EDI is the unit and ESI is unit+310h: 007CECB4 LEA ECX,[ESI+364h] feeds 0085DC80, which the same doc writes as unit+674h, and 007CEC30 MOV EDX,[ESI+41Ch] is its own unit+72Ch. 007CEC99 CMP dword ptr [ESI+5F0h],6 under the same bias is unit+900h == 6, and 007CEC4E FADD [ESI+5F8h] is unit+908h, the airborne clock the doc's own free-flight row describes
- **Was:** the per-class gain table attributes `+1E0h WheelBrake` to the "ground arm"
  **Is:** WheelBrake is read in 007DB680 BSP_PlaneFlight_CoreLaw, the law all three arms share, not in the ground arm 007CBFA0 or the ground law 007DCCF0
  **Evidence:** a scan of every disp32 access at displacement 1E0h in .text finds exactly two: the class-field writer 007D22BD and the read 007DBEF6 FLD dword ptr [EAX+1E0h], whose containing function is 007DB680 (body 007DB680-007DC82A)

## Correction from docs/PILOT_COMMAND_PATH.md (packet cc2_pilot_command_path)

- **Was:** unit+9E4h is roll, unit+9ECh is yaw, and unit+9F4h is an unnamed fifth axis
  **Is:** unit+9E4h is yawInput, unit+9ECh is rollInput, and unit+9F4h is airBrakeInput
  **Evidence:** 007D5D20 reads each field through BSP_LuaReader_ReadField(reader, 0, key, 2, &field), pushing {2,&field} then {0,key}: 007D69BA/007D69D6 pairs unit+9E4h with 00D05D94 'yawInput', 007D69F1/007D6A0D pairs unit+9ECh with 00D05D88 'rollInput', 007D6A5F/007D6A7B pairs unit+9F4h with 00D05D6C 'airBrakeInput'. The dynamics group names the latched copies the same way (unit+BB0h yawF, unit+BB8h rollF, unit+BC0h airBrakeF), matching the latch 007B9770's permutation. 0099BC00 clamps only the +9F0h and +9F4h commands to [0,1].
- **Was:** the rate law's yaw term is classDesc+1B0h YawSpd times the latched roll unit+0BB0h
  **Is:** unit+BB0h is yawF, the latched yawInput, so the term is YawSpd times the latched yaw; the two readings docs/PILOT_CONTROLS.md called contradictory agree once the labels are fixed, because unit+9ECh carrying the horizontal aim error is a bank-to-turn roll command
  **Evidence:** 007D6BB2 pairs unit+BB0h with 00D05D30 'yawF'; 007B9770 copies unit+9E4h to unit+BB0h

## Correction from docs/GAMEPLAY_LOOSE_ENDS_2.md (packet cc2_gameplay_loose_ends_2)

- **Was:** line 163: when ctl->+FCh != 1 the whole roll term is discarded
  **Is:** the roll accumulator is discarded when ctl+FCh == 1, on the ground; it survives in free flight and on the water
  **Evidence:** 007DA8D9 CMP dword [ESI+FCh],1; 007DA8E0 XORPS XMM2,XMM2; 007DA8E3 JNE 007DA8EB; 007DA8E5 MOVSS [ESP+18h],XMM2 - the zeroing store is reached only on equality
- **Was:** line 79: +FCh is a mode dword, zeroed each step and tested == 1
  **Is:** it is not zeroed each step; each of the three laws writes its own value (0, 1, 2) and it is read six ways, including a four-way dispatch in 007DA380 and a three-way dispatch in the core law
  **Evidence:** writers 007DC841, 007DCD24, 007DCDDC; readers 007DA38D, 007DA8D9, 007DB6D1, 007DBE0E, 007DA211; the disp32 FCh scan over 007D7000-007DE000 returns eleven references and no others
