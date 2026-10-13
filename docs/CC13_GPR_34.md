# CC13 GPR 34: segment 34 reference models (parachute target, dummy submarine)

Packet `cc13-gpr34` (cc13 lineage, Claude). Segment 34 is `006FA540`-`00701AD0`. cc9 approved this
reference-model pass on these terms:
- new `*_cc13` files only: `src/gameplay_seg34_cc13.cpp` and `include/bsp/gameplay_seg34_cc13.hpp`,
  registered by one line in `cmake/startup.cmake`;
- ledger records;
- no host edit, no wiring, no game run.

The segment's keywords (`openaftertime`, `velocity`, `openstate`, `dragvert`, `divedepth`, `curfloor`,
`cargorelpos`) come from three families. The depth charge (`dragvert`, `divedepth`) and the carrier
elevator platform (`curfloor`, `cargorelpos`, `cargoentity`) are already reconstructed in cc9's hosts.
The released parachute target (`OpenAfterTime`, `openState`, `velocity`) and the dummy submarine
release are not reconstructed anywhere, so they are the models here. Names come from cc13-gp6b
(`docs/CC13_GP_006B_0070.md`).

Every comparison and float rounding in the models was re-read from the disk bytes: Capstone
`disasm-raw`, and the x87 traced with `sc2_x87.py`. Everything in section 1 is **reconstructed and
build-tested**. None of it is ABI-compatible, wired or game-validated.

## 1. Models

Offsets are from the entity base `E`. The vehicle's tick element is `E+310h`. The rounds' tick
element is `E+3E4h`, as in `docs/BOMB_FAMILY_TICK.md`, whose "record" is `E`.

### 1.1 The released target round: `MDummyTarget` (bullet sub-type 0Ch)

Class record `00CFA5CC`. The reader `00701060` runs after `BSP_BombClass_ReadLuaFields`:

| field | Lua key | default | evidence |
| --- | --- | --- | --- |
| `+D8h` | `OpenAtFallingSpeed` | `0.0` (`00701088` FLDZ) | `007010AC` `00700290`, `ECX = 00CFD1F4` |
| `+DCh` | `OpenAfterTime` | `2.0` (`00CE3958`) | `007010D9` `006FE780`, `ECX = 00CFC878` |
| `+E0h` | `DummyTargetVehicleClass` | required | `00CFD1DC`, `00964790` |

This installation's row is realistic `bulletclasses.lua` 142, "Releaseable target":
`OpenAfterTime 3`, `OpenAtFallingSpeed 40`, `DummyTargetVehicleClass 790`.

`00701460` (`dummy_target_bullet_fixed_step_00701460`), `__thiscall(E+3E4h, float dt)`, `RET 4`:

1. `00701483` If `E+480h < 0` (the activate `006FF570` sets `-1.0`, `00D7A260`) and `00923810(E, 1)`
   returns 0, the launch time `E+480h` becomes the clock `[00F876A4]`. The owner chain is asked only
   when the launch time is negative.
2. `007014AE` `[[E+464h]+0Ch]->vtable[+24h](0, 0.0f, 1)`. This is an animation reset. Its meaning is
   unread.
3. `007014C8` Row 1 of the pose matrix `E+74h` is set to `00F8758C..94`, and then `0085DAD0` runs. The
   round is kept upright.
4. `00701509` The shared bomb tick `006E1300(dt)`.
5. `0070150E`-`0070157A` The spawn gates, in this order: `[[00E188A8]+1FE4h] != 2`; `00923810(E, 1) == 0`;
   no vehicle at `E+49Ch`; `now - E+480h > OpenAfterTime` (FCOMIP, JBE); `-OpenAtFallingSpeed > vy`.
   Here `vy` is `E+31Ch`. So the round opens only once it is falling faster than the threshold.
6. `00701580`-`00701617` The property bag: `Race` = `E+58h`; `Party` = `(E+54h == 0)`, the round's party
   inverted (`SETZ` at `007015B4`); `DummyObjectID` = `E+484h`; `OwnerPlayer` = 8; `Vel3D` = the velocity
   `E+318h..320h`.
7. `0070161C`-`00701683` `E->vtable[+10h](0, E+74h, bag)` returns a value that becomes the second
   argument of `00701170`. `ECX` there is the vehicle class `[classDesc+E0h]`. The new vehicle's `+C0h`
   takes a reference-counted copy of the bag (`00922E20`), and `00700940` makes `E+488h` observe the
   vehicle, so `E+49Ch` is set.

No RNG draw occurs. `00700520`, the round's activate, runs `006FF570`, then `006E0B40(0)`. It then
climbs the owner chain to the first `vtable[+5Ch](5)` unit and copies that unit's `+54h` (party) into
`E+54h`. So the inverted party makes the target belong to the other side from the unit that dropped it.

### 1.2 The chute: `DummyTargetVehicle` (class id 35h)

Vehicle class fields come from `00700E40`. It reads `ExplosionEfx` into `+140h` (the effect `00701250`
spawns), `OpenTime` into `+138h` (required, `00700330`) and `SlowingFactor` into `+13Ch` (`007003B0`).
The ledger calls `00700E40` `BSP_DoorClass_ReadLuaFields`. However, only the dummy target vehicle row
carries the last two keys (`src/vehicle_class_fields.cpp:503`), and the chute tick reads `+138h` and
`+13Ch` from the vehicle descriptor `[E+538h]`. That is consistent with it being this class's reader,
but the descriptor vtable was not traced.
This installation's `vehicleclasses.lua` row "Parachute target - tutorial" has `OpenTime 1`,
`SlowingFactor 2`, `HP 1` and the mesh `bullets/cel_ejtoernyo.mmod`. The installation is modded:
`vehicleclasses.lua` has an mtime of 2026-05-09 and the realistic `bulletclasses.lua` one of 2025-06-02,
so both rows are this installation's values and not necessarily retail.

`00700660` (`dummy_target_vehicle_tick_advance_00700660`), tick table `00CFCD1C` slot `+8h`,
`__thiscall(E+310h, float dt)`, `RET 4`:

```
00953CC0(dt)                                    ; the unit advance
if (1.0 > open)                                 ; COMISS/JBE, 00D7A24C
    open = dt / OpenTime + open                 ; FDIV, FADD, FSTP float
    if (open >= 1.0) open = 1.0                 ; FCOMIP/JB on the stored value
vy = vy - dt * 9.81                             ; 00CF9058 (double float(9.81)); DE E9 FSUBP
k = dt * open * SlowingFactor                   ; FSTP float
if (k > 1.0) k = 1.0                            ; FCOMIP/JBE
v *= (1 - k)                                    ; all three axes, E+734h..73Ch
if (0.0 > E+A8h) 00926D90(E, 2)                 ; COMISS/JBE
```

Once the chute is fully open, the sink rate settles at the fixed point of `v = (v - 9.81 dt)(1 - SF dt)`:
`9.81 (1 - SF dt) / SF`. With this installation's `SlowingFactor 2`, that is about 4.9 m/s.

`00700F70` (`dummy_target_vehicle_commit_pose_00700f70`), slot `+4h`, `__thiscall(E+310h, float dt)`,
`RET 4`:
1. `004134F0` copies the step matrix `E+674h` into the pose matrix `E+74h`.
2. The up row is forced and the matrix re-orthonormalised (`0085DAD0`).
3. The translation `E+A4h..ACh` is increased by `velocity * dt`. Each product is stored as a float first.
4. The pose-valid bytes `E+C8h` and `E+10Ch` are cleared, `0042ED50` runs on every child, and
   `vtable[+D8h]` is called.

`00700770` (`dummy_target_vehicle_update_00700770`), slot `+DCh` of `00CFCD60`, does presentation only.
While `open < 1`, the chute animation time is `00B6DC90(0, 1) * open`. Below 5 HP (`00CE3850`), the
visibility factor is `1 - max((5 - hp) / 5, 0)` (`00B6DA70`). The scanner's DE-form sites `007007EC`
(`5 - hp`), `007007EE` (`/ 5`) and `00700820` (`1 - x`) match.

`007016C0`, the InitAll, restores `Vel3D` (property kind 7) into `E+734h`. Its saved-state table is
`"_dummyTargetVehicle"` (`velocity`, `openState`), written by `00700870`. `00701250` sets the Lua
boolean `HitOnTheFly` on the entity, spawns the class effect `+140h` at the position and kills the
vehicle. These three are named but not modelled here.

### 1.3 The dummy submarine round: `MDummySubmarine` (bullet sub-type 0Eh)

`00700050` reads `OpenAfterTime` into `+D8h` with a default of `1.0` (`00700078` FLD1). It reads
`SubmarineClass` (`00CFCD08`) into `+DCh`.

`006FFA40` (`dummy_submarine_fixed_step_006ffa40`) has **no Ghidra function**. The bytes run
`006FFA40`-`006FFDE9`, and the only reference is the data word in vtable `00CFCAC8`.
It is `__thiscall(E+3E4h, float dt)`, `RET 4`:

1. `006FFA6B` When not attached and `E+480h < 0`: the launch time is set to now, the arming countdown
   `E+45Ch` to `0.5` (`00CE3800`), and `vy += 4.0` (`00D7A328`, FADD qword, FSTP float). The released
   midget submarine gets a 4 m/s upward kick.
2. `006FFAE5` When attached and `E+4B4h` is empty: climb to the first unit ancestor and observe it at
   `E+4A0h` (the same climb as `006FF2F0`).
3. `006FFB6F` `006E1300(dt)`.
4. `006FFB74`-`006FFBC8` The spawn gates: not a network client; no unit yet at `E+49Ch`; not attached;
   `now - launch > OpenAfterTime`.
5. `006FFBCE`-`006FFD10` The bag, in image order: `Type` = `[+DCh]+70h`; `Skill` = `E+4B8h`; `Race`;
   `Party` (not inverted); `OwnerPlayer` = `E+180h`; `Vel3D`; `StartSpeed` = `0042B2F0(velocity)`; a float
   `Invincible` set to 0; `AutoAttackTarget` = the word `[[E+47Ch]+174h]` when `E+47Ch` is set;
   `DummyObjectID`; `TargetDive` 1; `FireStance` 2.
6. `006FFD29` `004F05F0` (`EDX = [+DCh]+54h`) creates the unit. `006FF7C0` makes `E+488h` observe it.
   When the launcher `E+4B4h` answers `vtable[+5Ch](5)`, `0077C2A0` routes
   `VEHICLE_ADD_LAUNCHED(unit)` (`006EB9C0`) to the launcher with `(7, 0)`.

No RNG draw occurs.

## 2. Adoption table (cc9 terms)

| model | replaces (host file:line) | inputs and where the host gets them | RNG | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| `dummy_target_bullet_fixed_step_00701460` | Nothing. A rack holding a bullet of Type `DummyTarget` is released through `GameGunneryHost::release_bomb_drop` (`src/game_hosts_gunnery.cpp:13387`) as a plain bomb. `src/projectile_kinds.cpp:42` has only the kind table row. | Bullet row (`OpenAfterTime`, `OpenAtFallingSpeed`, `DummyTargetVehicleClass`) via `lua.read_bullet_class_number`. The round's velocity is the gunnery projectile's flight velocity. The clock is the fixed-step time. The network mode is the host's MP flag. The party is the dropping unit's row party, inverted. `bomb_tick` is the host's existing bomb flight. | none | OFF: nothing changes. ON: once a released target has been falling for OpenAfterTime (3 s in this installation) and faster than 40 m/s, it spawns a class-35h vehicle owned by the other party, and the round is no longer the thing that falls. Missions without a "Releaseable target" rack (all but the tutorial that carries one; the rack was not traced) see no change. |
| `dummy_target_vehicle_tick_advance_00700660`, `dummy_target_vehicle_commit_pose_00700f70` | Nothing. The host has no class-35h motion. The unit kind is only tested (`src/game_hosts_gunnery.cpp:6211`, `src/game_hosts_ship_ai.cpp:7756`, `src/game_hosts_script_orders.cpp:1455`, `src/local_player_unit_lists.cpp:168`). | Vehicle row `OpenTime` and `SlowingFactor`. The velocity comes from the spawn's `Vel3D`. HP is the unit slot's hit points. The pose and step matrix are the unit motion pose. | none | ON: the target drifts down on its chute at about 4.9 m/s once open and dies at HP < 0. Gunnery and ship AI that already classify kind 35h as `is_dummy_target` get a moving target. |
| `dummy_target_vehicle_update_00700770` | none | The chute animation object and the scene-node visibility. | none | Presentation only. Leave it unbound, or bind it to the renderer. It has no gameplay effect. |
| `dummy_submarine_fixed_step_006ffa40` | Nothing. `DummySubmarine` rounds have only the kind row (`src/projectile_kinds.cpp:49`). | Bullet row `OpenAfterTime` and `SubmarineClass` (its type id `+54h` and Type `+70h`). From the round: velocity, party, race, owner player and skill (`E+4B8h`) come from the launcher. The AutoAttackTarget word comes from the round's target. Unit creation is the scene creator `004F05F0` (`src/scene_unit_creators.cpp`). | none here; the created submarine's own construction may draw (not read) | ON: a released dummy-submarine round climbs at +4 m/s for the first step, flies the bomb path, and after OpenAfterTime (1 s by default) creates a submarine with `TargetDive 1` and `FireStance 2`. The launcher receives VEHICLE_ADD_LAUNCHED. |

The sister round `MDummyKamikazePlane` (`006FEC00`) is already hosted
(`src/kamikaze_task_host.cpp:12`, `src/game_hosts_units.cpp:23814`). Its registry is the template for
binding both rounds above. The gate order and the launch stamp are the same.

## 3. Host reconstructions in the segment (records only, not edited)

| address | host | check |
| --- | --- | --- |
| `006FC480` elevator platform tick | `src/game_hosts_units.cpp:14399` `carrier_elevator_platform_tick_006fc480` | Checked against the pseudocode. Both motion arms, the clamps and the float step match. The host omits the `[+54h]` slot `+0Ch` notify, the platform node call `[[+8h]]+4h(clamp(+18h, 0, +48h))` and the network-client release. |
| `006FD9B0` depth charge activate | `src/game_hosts_gunnery.cpp:8791` | Checked against a Capstone trace. There are three stream-1 draws (`ECX = 1`) in the host's order. `+46Ch` is a copy of `+468h`. |
| `006FCD20` depth charge in water | `src/game_hosts_gunnery.cpp:8830` | The dive arm matches `docs/BOMB_FAMILY_TICK.md`. The submarine-list arm is the host's labelled substitution. |
| 51 others | the first host mention in the ledger record | `host_reference_unverified`: the location is recorded but this packet did not re-read the listing. These include the remaining elevator calls, the MSTGun/MRTGun bodies, the class-id predicates (`entity_class_ids.cpp`, `unit_kind_query.cpp`), the vehicle-class readers and the kind table rows. A table-row mention (`projectile_kinds.cpp`) is not a reconstruction of the function body. |

## 4. Ledger

`python local/gpr34_records.py --apply` (not committed) wrote 136 function records to
`config/reconstruction/006f0000.jsonl` and `00700000.jsonl`. That is every function in the segment that
had no record and no tag, apart from the two `Unwind@` funclets, plus `006FFA40`:

| status | count |
| --- | --- |
| `semantic_build_tested` (this file) | 5 |
| `host_reconstructed_checked` | 3 |
| `host_reference_unverified` | 51 |
| `named_reviewed_not_reconstructed` | 77 |

The `named_reviewed_not_reconstructed` records carry cc13-gp6b's name evidence. They include the convex
object parser front (`006FA540`-`006FB030`), the cruiser/destroyer class bodies, the debug unit inspector
(`006FB680`-`006FBBC0`, debug only), the dummy-kamikaze and dummy-submarine class bodies, and the dummy
target vehicle's create, activate, serializer and HitOnTheFly. `006FF6E0` is a one-instruction
`JMP 006E1010` (the bomb-family commit).

## 5. Verification

- `./scripts/build.ps1` passed in this tree before and after merging `origin/main` (the build plus ctest,
  2 of 2). No test was added: the models are host-callback reference code, and cc9 binds them OFF.
- `tools/x87_dc_form_scan.py --forms dc,de`:
  - `00700660`: `007006D6 fsubp st(1)` (vy - dt*g) and `00700723 fmulp st(2)`;
  - `00700770`: `007007EC fsubrp st(2)` (5 - hp), `007007EE fdivp st(1)` (/5) and `00700820 fsubrp st(1)`
    (1 - x);
  - `00700F70`: `00700FCB fmulp st(2)`;
  - `00701460` and `006FFA40`: none.

  All of them agree with the models.
- `tools/verify_report_calls.py local/gpr34_calls.json`: 15 call rows checked, 0 failed. `006FFA40` has no
  Ghidra function, so its calls (`00923810`, `006E1300`, `004F05F0`, `006FF7C0`, `006EB9C0`, `0077C2A0`,
  `0042B2F0`) were read from the Capstone bytes only.

## 6. Handoff

Open:
- `006FFA40` needs a Ghidra function (a mutation outside this packet; vtable `00CFCAC8`).
- `[[E+464h]+0Ch]->vtable[+24h](0, 0.0f, 1)` in `00701460` is unread.
- Which rack carries bullet 142 in this installation, and which carries the dummy submarine, was not traced.
- The 51 `host_reference_unverified` records need a listing check if anyone wants them promoted.
- `00700E40`'s ledger name (`BSP_DoorClass_ReadLuaFields`) is probably the dummy target vehicle class
  reader (see 1.2). It was not renamed here because the descriptor vtable is untraced.
