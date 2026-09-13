# Controlled-unit nonfinite origin and motion dispatch correction

Addresses: 00874FE0, 00874FF6, 00825F20, 00937440, 0093749B, 0080FC30,
006D3110, 006D1C20, 006D2510, 00749B20, 00855420, 00758270.

The controlled unit in the preserved H Marshall run is **Airfield2**, not Enterprise.
`GameUnitsHost` applied the ship motion reconstruction to every active created unit.
The AirField class has no `MaxSpeed`; its semantic Lua reader supplies zero, and the
ship steering force computes `0 / 0` on the first motion step. This packet corrects
the class dispatch in `src/game_hosts_units.cpp`. It does not change force arithmetic,
substitute a speed, or add a finiteness guard.

## First invalid input and operation

The preserved executable was built at `ba2bf73418153d9248053d393b2ef6a9a23ae54a`:
SHA256 `41da178f70a4ee5ea141bafb98ab587e544c0e955b0ddf1793e3d75134dfcd09`.
The original 120-frame H log first prints a nonfinite sample at frame 10. A bounded
one-frame run of that exact executable, with trajectory output added, resolves the
earlier boundary:

| Airfield2 sample | Position Y | Yaw rate |
| --- | ---: | --- |
| Before fixed step 1 | 2.999100 | 0 |
| After fixed step 1, 0.05 s | 2.974100 | `-nan(ind)` |
| After fixed step 2, 0.10 s | 2.924100 | `-nan(ind)` |

The installed `VehicleClass[207]` is `AirField`, has Length 100, Width 10 and Height
1, and has neither `MaxSpeed` nor `Mass`. `GameMissionLuaHost::read_vehicle_class_row`
returns zero for an absent numeric key. The old host stores that zero in its semantic
ship `max_speed` projection and adapts the missing mass to 1. These are host inputs;
they are **not** values stored at native AirField offsets `+9C0h` and `+1018h`.

`ShipMotionBinding::reference_speed` passes the zero through the existing unscaled
`unit_reference_speed_0080fc30` helper. The original unscaled arm reads `unit+9C0h`
at `0080FC72` and multiplies it by 1. `00937440` forms the zero velocity dot product,
spills it as float at `0093748A`, widens it to the double stack slot at `00937492`,
calls `0080FC30` at `00937496`, then executes **`FDIVR double ptr [ESP+18h]` at
`0093749B`**. With numerator and reference both zero this is the first nonfinite
arithmetic in the inspected controlled-unit motion path. The source counterpart is
`along / double(in.reference_speed)` in `unit_steering_torque_00937440`.

The preceding ring, time scale, keel, wave, throttle gate and target-speed operations
have finite inputs for this unit. The dry keel gate suppresses later drive/steering
commands; it does not suppress the earlier force-model call. Multiplying the invalid
ratio by zero steering and the host's zero rudder-torque setting still yields NaN in
all three torque components. The unchanged Dyn velocity phase then propagates it to
angular velocity even with the host's zero inverse inertia. The exact later step on
which position first becomes nonfinite was not needed or established.

## The actual producer and dispatch

The AirField allocator `006D3110` requests **8E4h** bytes, clears that allocation,
and calls `006D1C20` at `006D315D`. That constructor stamps class 45h and stores
`00CF8BC0` at `unit+310h` (`006D1C90`). Its `+8h` slot is **006D2510**. The complete
`006D2510..006D2553` body calls `00953CC0`, primary virtual `+1A8h`, and conditionally
`006CDC70`; it never calls `00825F20`. Its allocation cannot contain the ship-only
offsets that the old process host nevertheless projected for it.

The native wave-3 job `00874FE0..00875003` loads the node from its stack argument,
loads `[node->vtable+8h]`, sets ECX to that node, pushes the fixed float step, and
calls at `00874FF6`. `00825F20` takes this `unit+310h` receiver: `00825F2A` preserves
ECX in EDI and `00825F32` computes `unit = EDI-310h`. Both its return paths use
`RET 4` (`00826642`, `00826D69`).

The correction uses the already recovered `VehicleClassDescriptorRow::allocate_instance`
identity. It does not use speed, kind ancestry, or the separate navigation-controller
classifier. All 21 concrete constructors' final tick-table stores and `+8h` words
were checked against both the original executable and live Ghidra bytes. Constructor
ESI provenance was checked through each store. The null DummyTargetVehicle allocator
has no concrete instance; unknown identities remain unresolved.

| Leaf class | Allocator | Tick table | Native +8h entry | Dispatch coverage |
| --- | --- | --- | --- | --- |
| Destroyer | 006FE590 | 00CFC38C | 00825F20 | direct ship body |
| Cruiser | 006FB430 | 00CFB6F0 | 00825F20 | direct ship body |
| Cargo | 006EB290 | 00CFA730 | 00825F20 | direct ship body |
| BattleShip | 006DFEF0 | 00CF9068 | 00825F20 | direct ship body |
| TorpedoBoat | 00857E20 | 00D0C604 | 00825F20 | direct ship body |
| LandingShip | 0074BE00 | 00CFF9EC | 00749B20 | ship base-call fragment |
| Submarine | 008531A0 | 00D0BF3C | 00855420 | ship base-call fragment |
| MotherShip | 00758D30 | 00D015E8 | 00758270 | ship base-call fragment |
| ReconPlane | 008091D0 | 00D0002C | 007CE040 | unresolved phase |
| SmallReconPlane | 0084CA50 | 00D0BA3C | 007CE040 | unresolved phase |
| LargeReconPlane | 0074E540 | 00D002C4 | 007CE040 | unresolved phase |
| Fighter | 007DDAE0 | 00D068DC | 007CE040 | unresolved phase |
| DiveBomber | 00956390 | 00D19CE4 | 007CE040 | unresolved phase |
| TorpedoBomber | 009564E0 | 00D19FBC | 007CE040 | unresolved phase |
| Kamikaze | 00956240 | 00D1A294 | 007CE040 | unresolved phase |
| LevelBomber | 007D7850 | 00D065F4 | 007CE040 | unresolved phase |
| AirField | 006D3110 | 00CF8BC0 | 006D2510 | unresolved phase |
| Shipyard | 00848380 | 00D0B728 | 00846320 | unresolved phase |
| LandVehicle | 0074DF10 | 00CFFD9C | 00953CC0 | unresolved phase |
| LandFort | 00747000 | 00CFF3B4 | 00953CC0 | unresolved phase |
| CommandBuilding | 006F5C10 | 00CFAFE0 | 00953CC0 | unresolved phase |

TorpedoBomber and Kamikaze allocator calls establish constructors `00951C40` and
`00951D20`, although the older instance metadata leaves those ctor entries zero.
CommandBuilding's store is indirect: `006F5676 LEA EBP,[ESI+310h]`, then
`006F567C MOV [EBP],00CFAFE0`.

| Override body | Implemented projection | Remaining native range |
| --- | --- | --- |
| 00749B20..00749BE2 | unconditional same-receiver base call at 00749B2C | 00749B31..00749BE2 |
| 00855420..00855937 | unconditional same-receiver base call at 0085542F | 00855434..00855937 |
| 00758270..007582AD | unconditional same-receiver base call at 0075827B | 00758280..007582AD |

Each wrapper forwards the float argument before any conditional branch and returns
with `RET 4`. The base fragment is retained; the remaining override is recorded as
unimplemented at its own entry. The root defined and exported the previously missing
LandingShip and MotherShip functions under its write lock. Workers made no Ghidra
mutations. Names remain descriptive hypotheses.

`direct_ship_body` describes **dispatch identity**, not complete reconstruction of
`00825F20`. The existing partial helper and its existing host contracts remain as
documented in [SHIP_MOTION.md](SHIP_MOTION.md). Unsupported entries are recorded before
ship order promotion, forces, or Dyn integration. Ship StartSpeed seeding, hull-body
creation and synthetic buoyancy setup use the same dispatch boundary. Common semantic
unit metadata, command owners and other frame phases are unchanged. Phase method keys
include entry addresses because `GameHostLog` aggregates by method name.

## Verification and remaining work

Win32 Release build and both existing CTests pass. No tracked tests were added.
`local/audit_motion_dispatch.py` checks all 21 source rows against constructor/store
evidence and original table words. The ignored `motion_zero_ratio_probe.cpp` executes
only the four original bytes `DC 7C 24 18`, with bridge-supplied stack/x87 operands.
It produces `FFC00000` and the invalid exception status; the unchanged source torque
helper produces `FFC00000` in each component. A divisor-one control produces zero.
This proves that arithmetic fragment and causal input, not the entire original force
routine, host binding, or game runtime.

The final USN01 run executes 120 frames / 240 fixed steps: **3,360 ship ticks**, 13
directly dispatched units plus Enterprise's MotherShip base fragment. All **18,557**
trajectory rows have finite position, heading, speed, throttle, rudder and yaw rate.
Airfield2 retains `(4013.951660, 2.999100, -3242.676758)` and zero yaw through all 241
samples. The log records 240 AirField phases, 10,080 `00953CC0` phases, 4,800 aircraft
phases, and 240 MotherShip remainder records as unimplemented. This is runtime proof
of excluding those receivers from ship motion; their native simulation remains open.

The post-fix base is `b40c84e6`, which also includes earlier main integrations; the H
baseline is `ba2bf734`. They are not an otherwise identical whole-program A/B. Exact
arguments, executable/source/input hashes, both captures and the focused probe are
preserved under ignored `local/`, indexed by `local/motion_j_artifact_manifest.json`.
The report carries their hashes and the mechanical native-call check result.

Concrete follow-up packets are the bounded AirField phase `006D2510..006D2553` and
its actual owning base/service contracts, the other unresolved phase bodies, and the
three listed subclass remainders. The existing per-class world-list registration,
ship hull collision/AABB inputs, synthetic buoyancy, force-controller binding and
other frame phases remain partial adapters; this packet does not establish their
fidelity or whole-game physics correctness. No original game process was launched.
