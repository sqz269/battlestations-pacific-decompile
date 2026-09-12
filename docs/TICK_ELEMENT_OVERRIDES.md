# Tick element overrides (what one element does per fixed step)

Addresses: 00875920, 0042bb70, 0042bb80, 0042bb90, 0042bba0, 0042bbb0, 0087b670,
007f2c60, 006e7b00, 00929e50, 00811ab0, 006d1fc0, 00953cc0, 007f3ba0, 006e6750,
006e6490, 006e7d50, 00929cb0, 0092b350

Every name here is a hypothesis, not a recovered symbol.

`docs/FIXED_STEP_JOB_WAVES.md` established the element, its two-sentinel group lists, the
four waves and the four job bodies, and left the element's virtuals as opaque: "what a
concrete `+4h`, `+8h` or `+0Ch` override does is a different packet". This packet reads the
seven concrete vtables and every per-step slot body behind them except two that belong to
other packets.

## The interface is a fixed-step pose contract

The six slots are not six unrelated hooks. Put the job bodies of
`docs/FIXED_STEP_JOB_WAVES.md` next to the overrides and the three live slots are one
scheme:

| slot | base | wave | argument | what every override does |
| --- | --- | --- | --- | --- |
| `+0h` | `00875920` | — | `int` | scalar deleting destructor |
| `+4h` | `0042BB70` `RET 4` | wave 1 with `0.05f`, interpolation with the leftover | `float t` | restore the committed pose, then advance it by `t` |
| `+8h` | `0042BB80` `RET 4` | wave 3 with `0.05f` | `float dt` | advance the simulation by one step |
| `+0Ch` | `0042BB90` `RET` | wave 1, right after `+4h` | none | commit the current pose as this step's pose |
| `+10h` | `0042BBA0` `RET 4` | none | `float` | no override in any of the seven tables |
| `+14h` | `0042BBB0` `XOR AL,AL` + `RET` | none | none, returns `bool` | no override in any of the seven tables |

The base bodies are read from the raw listing (`0042BA00` is the enclosing Ghidra
candidate; none of the five has a function of its own): `0042BB70` `C2 04 00`, `0042BB80`
`C2 04 00`, `0042BB90` `C3`, `0042BBA0` `C2 04 00`, `0042BBB0` `32 C0 C3`, each followed by
`CC` padding.

The unit is the clearest instance. `00811AB0` (slot `+4h`) copies the matrix at `unit+674h`
into `unit+74h` (`004134F0` with `ECX` = the destination, `00811ADC`/`00811AE2`) and then
advances the live matrix by `t`; `006D1FC0` (slot `+0Ch`) copies `unit+74h` back into
`unit+674h` (`006D1FC9` pushes the source, `006D1FD0 ADD ECX,0x364` is the destination). So
`unit+674h` holds the last **fixed-step** pose and `unit+74h` holds the pose that is drawn.
Wave 1 replays the step from the committed pose and commits the result; the interpolation
wave replays it with the frame's leftover and does **not** commit, so the rendered pose is
interpolated while the simulation pose stays on the step grid. `00929CB0` says the same
thing with an explicit alpha: `00929CD2 FDIV dword ptr [0x00D0DE84]` divides the argument by
`0.05f` before handing it to the physics interpolator.

`00D0DE84` is `0.05f`, `00D7A24C` is `1.0f`, `00D7A218` is `0.0f`, `00D7A260` is **`-1.0f`**
and `00D7A348` is the double `0.25`. That corrects a loose statement in
`docs/FIXED_STEP_JOB_WAVES.md`: `element+30h` is not a timer seeded with "a float constant"
but a per-element **time scale** whose disabled value is `-1.0f`; `00811AB9 COMISS XMM0,
dword ptr [0x00D7A24C]` with `JBE` at `00811AC6` means the scale multiplies the step only
while it is above `1.0f`.

## Where each class registers, and into which group

`00875890 BSP_TickRegistration_Construct(node, payload, group)` appends to the pending list;
`00874C90 BSP_TickRegistry_FlushPendingGroups` splices each pending node into
`group[node+14h]` on the next fixed step, so the group that counts is the value of `node+14h`
at the **first splice**, not the constructor argument.

| class | constructor | node offset | registration site | ctor group | group at splice | vtable |
| --- | --- | --- | --- | --- | --- | --- |
| base | `00875890` | — | — | argument | argument | `00D0DEC8` |
| unit | `0087B670` | `+310h` | `0087B6A9` | 0 | **1** | `00D0DF28` then `00D1A654`, `00D09630`, `00CFC38C` |
| plane squadron | `007F2C60` | `+310h` | `007F2C9A` | **3** | 3 | `00D0877C` |
| projectile | `006E7B00` | `+244h` | `006E7B49` | 0 | 0 | `00CF9D78`, or `00CFD554` from `0070CAE0` |
| tickable game entity | `00929E50` | `+170h` | `00929E85` | 0 | 0 | `00D194C4` |

The unit's group is level 5's doing: `0081F198 MOV dword ptr [ESI + 0x324],0x1` inside
`0081ED40`, on the straight-line tail of the constructor, and `unit+324h` is the node's
`+14h`. `include/bsp/unit_instance_layout.hpp` already records `kUnitTickGroupIndex = 1`;
this packet is where it meets the registry. `docs/FIXED_STEP_JOB_WAVES.md`'s "eight sites
pass group 0" is right about the argument and wrong about where the unit lands.

On the `tick_group_semantics` question: the group index does **not** select which waves run.
Each of the four waves loops all five groups (`00875CAA`, `00875D1A`, `00875D8A`,
`00875F55`, each five iterations) and the admission test is the same two payload bytes in
every group. What the index selects is the **job-pool batch**: the driver queues each
admitted element and then issues one dispatch per group, so an element's group decides which
other elements its per-step calls are dispatched with. Aircraft go to group 3 because a
squadron shares no work with a unit: a squadron overrides only `+8h`, so it contributes
nothing to wave 1 or to the interpolation wave and its wave-3 batch is pure squadron AI.
Groups 2 and 4 still have no constructor site in this reading.

## Unit (`00D0DF28` -> `00D1A654` -> `00D09630` -> `00CFC38C`)

Four tables along the constructor chain of `docs/UNIT_INSTANCE_LAYOUT.md`. `this` in every
slot body is the node at `unit+310h`; the bodies reach the unit with `LEA reg,[ESI-0x310]`.

| slot | L3 `00D0DF28` | L4 `00D1A654` | L5 `00D09630` | L6 `00CFC38C` | name |
| --- | --- | --- | --- | --- | --- |
| `+0h` | `0087A4F0` | `00959C10` | `0081F380` | `006FE510` | scalar deleting dtor |
| `+4h` | stub | stub | `00811AB0` | `00811AB0` | `BSP_UnitInstance_RunShiftedControllerUpdate` |
| `+8h` | stub | `00953CC0` | `00825F20` | `00825F20` | L4 `BSP_UnitGameObject_TickAdvance`, L5/L6 `BSP_UnitInstance_UpdateShipMotion` |
| `+0Ch` | stub | `006D1FC0` | `006D1FC0` | `006D1FC0` | `BSP_UnitTickElement_CommitStepPose` |
| `+10h` | stub | stub | stub | stub | — |
| `+14h` | stub | stub | stub | stub | — |

A level-3 unit therefore does nothing of its own per step: wave 1 calls two `RET 4` stubs and
only `BSP_EntityPose_RefreshWorld 00414DB0(payload)` inside the job body has an effect.

`00811AB0` is already reconstructed (`unit_shifted_update_00811ab0`, `src/unit_rudder.cpp`,
`docs/UNIT_RUDDER_CURVE.md`) and is not repeated here. `00825F20
BSP_UnitInstance_UpdateShipMotion` is another packet's (`cc_exe_2i` holds the lease); it is
a **contract** here: slot `+8h` of a level-5 or level-6 unit is the ship motion tick, called
once per fixed step with `0.05f`, and the reading stops at the call site.

`006D1FC0` (no Ghidra function, `006D1FC0..006D1FEE`, `RET`, no argument): commits
`unit+674h` from `unit+74h`, or from `unit+4E0h` when the byte at `unit+4D8h` is set
(`006D1FC0 CMP byte ptr [ECX + 0x1C8],0`).

`00953CC0` (`00953CC0..00953D9D`, `RET 4`) in listing order: when the dword at `unit+528h`
is not negative, `unit->vtable[+5Ch](9)` (the `9` is the literal at `00953CDD`, not the
field); `unit->vtable[+1F0h](step)`; clear `unit+634h` when the byte at `unit+61h` is clear;
count `unit+6F8h` and `unit+6FCh` down by the step while each is positive; when `unit+1ACh`
is not `8`, `00927F10(unit, unit+1ACh)` and a false answer clears the byte at `unit+520h`;
finally, while that byte is set, `unit->vtable[+1D8h](step)`.

## Plane squadron (`00D0877C`)

| slot | address | name | what it does |
| --- | --- | --- | --- |
| `+0h` | `007EFAE0` | scalar deleting dtor | — |
| `+4h` | `0042BB70` | base stub | a squadron carries no pose |
| `+8h` | `007F3BA0` | `BSP_PlaneSquadron_TickAdvance` | the whole squadron step |
| `+0Ch` | `0042BB90` | base stub | nothing to commit |
| `+10h`, `+14h` | stubs | — | — |

`007F3BA0` has no Ghidra function: `007F3BA0..007F3D30` inclusive, `RET 4` at `007F3D21`
plus the tail block `007F3D24..007F3D30` that ends in `JMP 0x7F3C60`, `INT3` from
`007F3D31`. `this` is the node at `squadron+310h`.

1. `007F3BAA`: only while `step > 0.0f`, four countdowns at `squadron+37Ch`, `+380h`,
   `+384h`, `+388h`, each frozen by its own byte at `+38Ch`..`+38Fh` (listing order `380h`,
   `37Ch`, `384h`, `388h`); then, while the byte at `squadron+3ECh` is set, `007EE7F0(squadron, 0)`.
2. `007F3C0B`: `squadron+3E8h += (1 - squadron+3E8h) * step * 0.25`, an exponential approach
   to 1 seeded at 1.0 by the constructor.
3. `007F3C3F`: the countdown at `squadron+3C4h`. While `step < t` it is `t -= step`;
   otherwise it becomes `t + (period - step)` with the period at `squadron+3C0h`, and
   `007EE790(squadron)` fires. The constructor seeds the period with `1.0f` and the
   countdown with `-1.0f`, so it fires on the first step.
4. `007F3C60`: `squadron+3B0h = squadron+3B8h`, then the member loop over
   `[squadron+3D0h][0 .. squadron+3CCh)`. Per member: raise `member+2ECh` to
   `squadron+2ECh`; skip an inactive member (`member+904h`); `007B8AD0(member)` and OR the
   answer into `squadron+3B0h`; when that is set and `member+BF4h` is not null, take
   `[[member+BF4h]+4h]+7Ch` into `squadron+3B4h`; when `member+900h == 1` and the member is
   still active, `00926D90(member, 5)` then `007F3970(squadron, member, 0)` and step the
   index back (`007F3CEC SUB EBP,1`, `007F3CEF SUB EBX,4`) because the array shrank.
5. `007F3D02`: `0077A650(squadron)` only when the float at `squadron+308h` is not `0.0f`.
   The idiom is `UCOMISS` + `LAHF` + `TEST AH,0x44` + `JNP`, where the jump is taken on
   equality; the same idiom decides the tail of `00811AB0` at `00811B29`.

`0077A650`'s ledger caller list shows one caller (`00825F20`) because `007F3BA0` is not a
Ghidra function and its calls are not in the call graph.

## Projectile (`00CF9D78`, and `00CFD554`)

| slot | `00CF9D78` | `00CFD554` | name | what it does |
| --- | --- | --- | --- | --- |
| `+0h` | `006E7BF0` | `0070CB70` | scalar deleting dtor | — |
| `+4h` | `006E6750` | `006E6750` | `BSP_Projectile_PlaceStepPose` | scale the step, drive the body, hand the matrix to the attached node |
| `+8h` | `006E6490` | `0070C370` | `BSP_Projectile_TickAdvance` | flight time, tracer, expiry |
| `+0Ch` | `006E7D50` | `006E7D50` | `BSP_Projectile_CommitStepPose` | previous <- current <- world translation |
| `+10h`, `+14h` | stubs | stubs | — | — |

`006E7B00` builds the projectile on the level-1 base `00925CE0`, puts the node at `+244h`,
registers group 0 and parks a self-pointer at `projectile+23Ch` (`006E7B91`), which is how
the slot bodies reach the object without the `LEA` the unit uses. Class id `+C4h` is `29h`.
`0070CAE0` (a flak variant) replaces the table with `00CFD554`, keeping `+4h` and `+0Ch` and
replacing `+8h` with `0070C370`, which this packet does **not** read: `contract: unread`.
`006E8430` installs no table of its own.

`006E7D50` (`006E7D50..006E7D96`, `RET`): copies `projectile+1D0h..1D8h` into
`+1DCh..+1E4h`, refreshes the world pose when the byte at `projectile+C8h` is clear, then
reads `projectile+FCh/100h/104h` into `+1D0h..+1D8h`. That is the previous/current pair the
tracer and the interpolation read.

`006E6750` (`006E6750..006E67CC`, `RET 4`): scale the step by `[[projectile+174h]+5Ch]`;
`[projectile+170h]->vtable[+2Ch]()` picks `projectile->vtable[+118h]` on a true answer and
`[+114h]` otherwise, with the scaled step as the only argument; then, when `projectile+280h`
is not null, refresh the world pose if needed and call
`[[projectile+280h]+0Ch]->vtable[+34h](projectile+CCh)`.

`006E6490` (`006E6490..006E65B4`, `RET 4`): the same scale, added to `projectile+1C4h`
**before** the mode query, then `[+120h]`/`[+11Ch]` by the same rule. While the byte at
`projectile+5Ch` is set it refreshes the pose and calls `0084C430` with eleven dwords of
arguments (`0084C4D3 RET 0x2C`): two by-value vectors (the snapshot's previous position and
the refreshed world translation), the adjusted `projectile+170h` pointer, `projectile+238h`,
the class descriptor `projectile+174h`, the global `00F87574` and `projectile+1C8h`, plus
`CL` = `([00E188A8]+1FE4h != 2)` and `DL` = 1. Finally, when the accumulated flight time
exceeds `[[projectile+174h]+54h]`, `00696350(projectile, 0)` then `00926D90(projectile, 2)`.

## Tickable game entity (`00D194C4`)

| slot | address | name | what it does |
| --- | --- | --- | --- |
| `+0h` | `0092A030` | scalar deleting dtor | — |
| `+4h` | `00929CB0` | `BSP_TickableGameEntity_PlaceStepPose` | interpolate the physics body by `step / 0.05` |
| `+8h` | `0092B350` | `BSP_TickableGameEntity_TickAdvance` | the physics-driven step, `partial` |
| `+0Ch` | `0042BB90` | base stub | the physics body is the committed state |
| `+10h`, `+14h` | stubs | — | — |

`00929E50` also builds on `00925CE0`, puts the node at `+170h` and links the object into the
intrusive list headed by `00F89AD8` with `next` at `+340h` and `prev` at `+344h`
(`00929FA9..00929FCE`). Class id `+C4h` is `58h`.

`00929CB0` (`00929CB0..00929D5F`, `RET 4`): returns at once when `entity+354h` (the physics
body) is null; otherwise `00C43EA0` (already reconstructed as
`interpolate_body_transform_00c43ea0`, `src/game_dynamics_list.cpp`) with
`alpha = step / 0.05f` then `00C33650`;
`BSP_Vector3f_TransformDirectionOptionalNormalize 0042D0D0` on the pivot at `entity+364h`
with the normalise flag `0`; the pivot is subtracted from the interpolated matrix's
translation row; `00741E90(entity, matrix)`; `[entity+350h]->vtable[+34h](matrix)`.

`0092B350` is `contract: partial`. Its body is `0092B350..0092B91E` (`RET 4` at `0092B91C`),
guarded by the same `entity+354h` null test, and its call sequence is `00424C40`, `00C31FC0`,
`BSP_Geometry_NormalizeVectorWithFloor 0042B260`, `BSP_EntityPose_RefreshWorld 00414DB0`,
`004142E0`, `00C37F60`, `00C31F40`, `00C31F20`, `00C32000`, `00C37F60`, `00C37F60`. The x87
force and torque math between those calls (`0092B380..0092B8FB`) is not projected, and
`00424C40` is leased to another owner.

## Host table

Every row is a native call site inside a body this packet read. `contract` marks a callee
whose body another packet owns or that this packet did not open.

| site | containing body | callee | this / args | ret | gate |
| --- | --- | --- | --- | --- | --- |
| `006D1FD6` | `006D1FC0` | `004134F0 BSP_Matrix_Copy4x4X87` | `node+364h`, `node-29Ch` | — | byte `node+1C8h` clear |
| `006D1FE9` | `006D1FC0` | `004134F0` | `node+364h`, `node+1D0h` | — | byte `node+1C8h` set |
| `00953CDF` | `00953CC0` | `unit->vtable[+5Ch]` | `unit`, `9` | — | `unit+528h >= 0` |
| `00953CFD` | `00953CC0` | `unit->vtable[+1F0h]` | `unit`, `step` | — | none |
| `00953D6E` | `00953CC0` | `00927F10` | `unit`, `unit+1ACh` | `bool` | `unit+1ACh != 8` |
| `00953D98` | `00953CC0` | `unit->vtable[+1D8h]` | `unit`, `step` | — | byte `unit+520h` set |
| `007F3C02` | `007F3BA0` | `007EE7F0` | `squadron`, `0` | — | `step > 0` and byte `squadron+3ECh` |
| `007F3C5B` | `007F3BA0` | `007EE790` | `squadron` | — | countdown expired |
| `007F3CA0` | `007F3BA0` | `007B8AD0` | `member` | `bool` in `AL` | byte `member+904h` |
| `007F3CD9` | `007F3BA0` | `00926D90` | `member`, `5` | — | `member+900h == 1` and active |
| `007F3CE7` | `007F3BA0` | `007F3970` | `squadron`, `member`, `0` | — | same |
| `007F3D1C` | `007F3BA0` | `0077A650` | `squadron` | — | `squadron+308h != 0.0f` |
| `006E6774` | `006E6750` | `[projectile+170h]->vtable[+2Ch]` | `projectile+170h` | `int` | none |
| `006E679E` | `006E6750` | `projectile->vtable[+118h]` / `[+114h]` | `projectile`, scaled step | — | the query's answer |
| `006E67B5` | `006E6750` | `00414DB0 BSP_EntityPose_RefreshWorld` | `projectile` | — | `projectile+280h` set, byte `+C8h` clear |
| `006E67C8` | `006E6750` | `[[projectile+280h]+0Ch]->vtable[+34h]` | that node, `projectile+CCh` | — | `projectile+280h` set |
| `006E64C2` | `006E6490` | `[projectile+170h]->vtable[+2Ch]` | `projectile+170h` | `int` | none |
| `006E64EA` | `006E6490` | `projectile->vtable[+120h]` / `[+11Ch]` | `projectile`, scaled step | — | the query's answer |
| `006E652D` | `006E6490` | `00414DB0` | `projectile` | — | byte `projectile+5Ch` set, `+C8h` clear |
| `006E657F` | `006E6490` | `0084C430` | eleven dwords, `RET 0x2C` | — | byte `projectile+5Ch` set; **contract** |
| `006E659F` | `006E6490` | `00696350` | `projectile`, `0` | — | flight time over the class limit |
| `006E65A9` | `006E6490` | `00926D90` | `projectile`, `2` | — | same; **contract** |
| `006E7D74` | `006E7D50` | `00414DB0` | `projectile` | — | byte `projectile+C8h` clear |
| `00929CEA` | `00929CB0` | `00C43EA0` | body `entity+354h`, alpha, two buffers | ptr in `EAX` | `entity+354h` non-null; **contract** |
| `00929CF1` | `00929CB0` | `00C33650` | that pointer | — | same; **contract** |
| `00929D07` | `00929CB0` | `0042D0D0` | buffer, `entity+364h`, out, `0` | — | same |
| `00929D41` | `00929CB0` | `00741E90` | `entity`, matrix | — | same |
| `00929D56` | `00929CB0` | `[entity+350h]->vtable[+34h]` | that node, matrix | — | same |
| `0087B694` | `0087B670` | `0077EED0` | `unit`, flag | `unit` | none; **contract** |
| `0087B6A9` | `0087B670` | `00875890` | `unit+310h`, `unit`, `0` | node | none |
| `007F2C84` | `007F2C60` | `0077EED0` | `squadron`, flag | `squadron` | none; **contract** |
| `007F2C9A` | `007F2C60` | `00875890` | `squadron+310h`, `squadron`, `3` | node | none |
| `006E7B1F` | `006E7B00` | `00925CE0` | `projectile` | `projectile` | none; **contract** |
| `006E7B34` | `006E7B00` | `006E22D0` | `projectile+170h` | — | none; **contract** |
| `006E7B49` | `006E7B00` | `00875890` | `projectile+244h`, `projectile`, `0` | node | none |
| `00929E70` | `00929E50` | `00925CE0` | `entity` | `entity` | none; **contract** |
| `00929E85` | `00929E50` | `00875890` | `entity+170h`, `entity`, `0` | node | none |
| `00929F0F` | `00929E50` | `004E6480` | `entity+1C4h` | — | none; **contract** |
| `0081F198` | `0081ED40` | — (store) | `unit+324h = 1` | — | none |

## Reconstruction

`include/bsp/tick_element_overrides.hpp` and `src/tick_element_overrides.cpp`:

- `TickElementSlot`, `tick_element_slot_offset`, `tick_element_wave_calls`: the slot-to-wave
  contract above, over `JobWavePhase` from `bsp/fixed_step_job_waves.hpp` (whose
  `kTickElement*Offset` constants are reused, not redeclared).
- `TickElementVtable` / `TickElementClassRow` and the eight-row table, with
  `tick_element_slot_is_override` against the base stubs.
- `unit_tick_commit_pose_006d1fc0`, a pure rule returning the two offsets.
- `unit_tick_advance_sim_00953cc0`, `squadron_tick_advance_sim_007f3ba0`,
  `projectile_tick_place_pose_006e6750`, `projectile_tick_advance_sim_006e6490`,
  `projectile_tick_commit_pose_006e7d50` and `game_entity_tick_place_pose_00929cb0`, each a
  sequence over a host with one virtual per native call site of that body.

Coverage: **complete** for the base interface, for the projectile's three slots, for the
squadron's one slot, for the unit's `+0Ch` and level-4 `+8h`, and for the entity's `+4h`.
`contract: unread` for `00825F20` (the unit's level-5/6 `+8h`, another owner's lease),
`0070C370` (the flak projectile's `+8h`) and the object constructors' base calls.
`contract: partial` for `0092B350`, read as a call sequence only. `00811AB0` is covered by
`src/unit_rudder.cpp` and is referenced, not duplicated.

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/FIXED_STEP_JOB_WAVES.md`: "Eight sites pass group **0** ... and one passes group **3**", read as where the elements land | the constructor argument is 0 for the unit, but level 5 overwrites the node's `+14h` with 1 before the first splice, so a unit ticks in group **1** | `0081F198 MOV dword ptr [ESI + 0x324],0x1` inside `0081ED40`, with `unit+324h` = node `+14h` per `docs/UNIT_INSTANCE_LAYOUT.md`; `00874C90` reads `node+14h` at the splice |
| same doc: `element+30h` is "a float constant, the same one `00778450` resets its timer from" | the constant is `-1.0f` and the field is a time scale that is only applied above `1.0f` | `00D7A260` = `00 00 80 BF`; `00811AB4`/`00811AB9`/`00811AC6` |
| same doc: the base vtable's five stubs "Derived classes override them" | only three of the five are ever overridden; `+10h` and `+14h` are the base stub in all seven concrete tables | `00D0DF28`, `00D1A654`, `00D09630`, `00CFC38C`, `00D0877C`, `00CF9D78`, `00CFD554`, `00D194C4` all end `... A0 BB 42 00 B0 BB 42 00` |

## Open questions

- Nothing in this packet calls slot `+10h` or slot `+14h`. The predicate's `false` default
  and the absence of any override make them dead in this build; the caller, if there is one,
  is outside the waves.
- `0070C370`, the flak projectile's `+8h`, and the x87 middle of `0092B350`.
- `payload+5Eh` and `payload+BDh`, the admission bytes, are still only "gone" and "active".
- Groups 2 and 4 still have no constructor site.

## no_ghidra_function

| start | end (inclusive) | evidence |
| --- | --- | --- |
| `0042BB70` | `0042BB72` | `00D0DECC` points here; `C2 04 00`, `INT3` from `0042BB73` |
| `0042BB80` | `0042BB82` | `00D0DED0` points here; `C2 04 00`, `INT3` from `0042BB83` |
| `0042BB90` | `0042BB90` | `00D0DED4` points here; `C3`, `INT3` from `0042BB91` |
| `0042BBA0` | `0042BBA2` | `00D0DED8` points here; `C2 04 00`, `INT3` from `0042BBA3` |
| `0042BBB0` | `0042BBB2` | `00D0DEDC` points here; `32 C0 C3`, `INT3` from `0042BBB3` |
| `006D1FC0` | `006D1FEE` | `00D1A660` points here; `RET` at `006D1FEE`, `INT3` from `006D1FEF` |
| `007F3BA0` | `007F3D30` | `00D08784` points here; `RET 4` at `007F3D21` and the `JMP 0x7F3C60` tail ends at `007F3D30`, `INT3` from `007F3D31` |
| `00929CB0` | `00929D61` | `00D194C8` points here; `RET 4` at `00929D5F`, `INT3` from `00929D62` |
| `0092B350` | `0092B91E` | `00D194CC` points here; `RET 4` at `0092B91C` |

## Correction from docs/PLANE_FLIGHT.md (packet cc2_plane_flight)

- **Was:** "finally, while that byte is set, unit->vtable[+1D8h](step)"
  **Is:** the slot runs only when unit+520h is clear
  **Evidence:** 00953D7D CMP byte ptr [ESI+210h],0 then 00953D84 JNZ 0x00953D9A, which jumps past the 00953D98 CALL EDX; ESI is the node at unit+310h, so [ESI+210h] is unit+520h.
