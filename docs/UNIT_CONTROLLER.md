# The controller subobject at `unit+1018h` and its two readers

Addresses: 0092D730, 0092BE80, 00815AA0, 00815370, 0080DEC0, 00939CB0, 008255B0, 00825F20,
00813020, 0080DAD0, 00816A40, 00C32000, 00C31F40.

Packet `cc_world_update`, worktree `agent/cc-unit-tick`. Ghidra read-only. Reconstruction is
appended to `include/bsp/unit_controller.hpp` and `src/unit_controller.cpp`; machine-readable facts
are in `reports/world_update.json`.

This document completes the `unit_controller` follow-up of `docs/UNIT_INSTANCE_UPDATE.md` for the
parts that packet left open: the two routines the unit update calls on the controller, and the path
by which a player's input reaches it. **The controller's own layout, constructor and force callback
are already established in `docs/UNIT_CONTROLLER_UPDATE.md` and are not redone here.**

## What is already settled, in one table

From `docs/UNIT_CONTROLLER_UPDATE.md`, cited rather than re-derived:

| Fact | Where |
| --- | --- |
| `unit+1018h` holds a `0x390`-byte hydrodynamic force model | creators `0080DEC0`, `00852210`, `00857C20`, `00749790` |
| Base constructor `00939CB0`, `controller* __thiscall(this, unit)`, `RET 4` | `00939CB0..00939E43` |
| `+1Ch` owning unit, `+20h` contact-listener vptr, `+2Ch` the physics body, `+88h` a one-shot | the layout table there |
| `+8Ch..+30Bh` is 20 records of `0x20` bytes | `eh_vector_constructor_iterator` in the ctor |
| Force callback `009329C0`, contact callback `009377E0`; the library integrates motion | that document |
| An allocation failure stores a null controller and nothing null-checks `+1018h` | `0080DF00`/`0080DF06` |

The one thing worth repeating, because it is the reason this document is short: **the controller
object carries no throttle, no rudder and no target speed.** It converts ocean and hull geometry
into a force. Commands reach the rigid body directly, past the controller.

## The unit update's two calls into `+1018h`

`008255B0` touches the controller three times: steps 4, 6 and 8 of `docs/UNIT_INSTANCE_UPDATE.md`.
Step 8 only latches the byte at `+88h`. The other two are these.

### Step 4: `0092D730`, and what its sign is used for

`float __thiscall(controller)`, `RET 0` at `0092D76E`, body `0092D730..0092D76E`, result on the x87
stack. Already named `BSP_UnitController_GetBodyAxisSpeed`; the body is

```
0092d737  ECX = [controller + 2Ch]      ; the physics body
0092d73a  ESI = 00C32000(body)          ; a basis block
0092d73f  ECX = [controller + 2Ch]
0092d749  EAX = 00C31F40(body, &scratch); the linear velocity
0092d74e  return (v[1]*axis[1Ch] + v[0]*axis[18h]) + v[2]*axis[20h]
```

`axis+18h..+20h` is the third 12-byte row of the basis, so this is the signed speed along one body
axis. The accumulation order is native: the y and x products are added first, then the z product.
It is reconstructed as `bsp::unit_forward_speed_0092d730` in `include/bsp/unit_motion.hpp`.

Twenty-eight callers exist. Inside the frame update there is exactly one, and **only the sign of
the result is used**:

```
00825832  ECX = [unit + 1018h]
0082584b  CALL 0092D730
00825850  FLDZ / FCOMIP / FSTP ST0
00825856  JBE 00825861          ; 0.0f <= speed, or unordered: keep the staged 1.0f
00825858  XORPS XMM0,XMM0       ; otherwise 0.0f
0082586b  CALL 00815AA0         ; ECX = unit, the 0/1 scalar as the only argument
```

So the controller's contribution to the unit's own frame is one bit: **making sternway or not**.
The reconstruction is `bsp::unit_effect_intensity_gate_0082583e`.

### Step 6: `0092BE80` does nothing

`void __thiscall(controller, float delta)`, `RET 4`, body `0092BE80..0092BE82`. One instruction.
Both call sites, `008259DF` in `008255B0` and `00826B6A` in `00825F20`, are direct calls, so no
override can intercept it. This is `docs/UNIT_CONTROLLER_UPDATE.md`'s headline correction and is
repeated here only so that the step-4/step-6 pair reads correctly: step 4 does all the work the
unit update gets out of its controller, and step 6 is dead.

## `00815AA0`, the consumer of step 4

`void __thiscall(unit, float gate)`, `RET 4` at `00815D0A` and `00815D13`, body
`00815AA0..00815D15`. Sole caller `0082586B`. Full listing read.

```
00815aa3  intensity = (DAT_00F87152 != 0 || unit->byte_2F0h != 0) ? 1.0f : unit->f_2F4h
00815ae8  value = intensity * gate
00815aca  if (unit->p_9F0h) 00815370(p_9F0h, value)      ; bow water anchor
00815afc  if (unit->p_9F4h) 00815370(p_9F4h, value)      ; stern water anchor
00815b29  if (unit->f_9D0h == value) return              ; FUCOMIP, TEST AH,44h, JNP
00815b5c  unit->f_9D0h = value
          for p in {+9E8h, +9ECh, +A00h, +A04h, +A08h, +A0Ch}: if (p) 00815370(p, value)
00815c19  for i in [0, unit->int_A18h): if (unit->pp_A14h[i]) 00815370(..., value)
          for p in {+B44h, +B48h, +B4Ch, +B50h}: if (p) 00815370(p, value)
00815cd5  for i in [0, 5): if (unit->pp_B54h[i]) 00815370(..., value)
```

Three things in that shape matter and none of them are visible from the call site:

* The intensity expression is `006FF270`'s, inlined. `docs/UNIT_INSTANCE_UPDATE.md` records the
  same expression appearing inline at step 7; this is a third copy.
* **The two water anchors are updated before the change latch**, so they are written every frame
  even when nothing changed. Everything else is behind the latch at `+9D0h`.
* The latch comparison is an equality, not an epsilon, and the early return at `00815D0D` skips
  ten group pointers plus two arrays.

`unit+9F0h` and `unit+9F4h` are the same two pointers step 5 of `008255B0` feeds bow and stern
water positions to through `004842C0`, so the object at each is an emitter group that takes both a
world position and a scalar.

The `NEG/SBB/TEST reg,0E186ECh` idiom that guards every pointer is the compiler's "is this pointer
non-null"; the constant is a data address used as a non-zero mask and has no meaning of its own.

## `00815370`, one emitter group

`void __thiscall(group, float value)`, `RET 4` at `008153DC`, body `00815370..008153DD`. Two
pointer vectors with stride 4:

| Field | Offset | Evidence |
| --- | --- | --- |
| primary begin | `+0Ch` | `00815375` |
| primary count | `+10h` | `00815378` |
| secondary begin | `+18h` | `008153AA` |
| secondary count | `+1Ch` | `008153AD` |

Each element gets `element->vtable[14h](value)` (`008153A1`, `008153CF`). **The two loops are not
symmetric**: the first tests each pointer (`00815386`), the second dereferences at `008153C0` with
no test at all, so a null in the second vector faults. The reconstruction keeps that asymmetry
rather than normalising it.

The same routine is step 12 of `008255B0` (`00825DB4`), called once per part with
`006FF270(unit)`. So a part is an emitter group too, and step 12 and `00815AA0`'s `+A14h` walk push
the same kind of scalar into the same objects by two different routes: step 12 unconditionally with
the raw intensity, `00815AA0` only on change with the intensity times the sternway gate.

## How input and orders reach the controller

They do not reach the controller object. They reach its **physics body** at `controller+2Ch`, and
they do it without going through `008255B0` at all. The chain, with the packet that owns each link:

| Stage | Routine | Owner |
| --- | --- | --- |
| per-frame input records at `singleton+4h`, stride `30h`, rising-edge test `004C43C0` | `00A92C40` | `docs/GAME_INPUT_TICK.md` |
| HUD order producers read the input singleton and issue an order | `0064B870`, `0067C4F0` | `docs/UNIT_ORDER_RECORD.md` |
| build a `20h`-byte order record and publish it | `00816A40` then `0080DAD0` | `docs/UNIT_COMMAND_PRODUCERS.md` |
| the ten-slot order ring at `unit+838h`, slew-limited | `00813020` | `docs/UNIT_STATE_MESSAGE.md` |
| the ring writes `unit+980h` thrust and `unit+984h` toTurn | `00813020` step 2 | `docs/UNIT_STATE_MESSAGE.md` |
| throttle to target speed, then target speed to **body linear velocity** | `00825F20` tail, `0092D300` | `docs/UNIT_FORCE_COMMANDS.md` |
| rudder to yaw rate, then to **body angular velocity**; and rudder to torque | `00811890`, `0092E8C0`, `00937440` | `docs/UNIT_FORCE_COMMANDS.md` |

Two checks on that chain were made here rather than taken on trust: `0064B870` and `0067C4F0` both
call `004BEC00` `BSP_InputManager_GetSingleton` and `00816A40` `BSP_UnitInstance_IssueOrder`, which
is what makes them the input-to-order link. Their bodies were **not** read; `00651760`, the third
caller `docs/UNIT_COMMAND_PRODUCERS.md` lists, does not call the input singleton, so it reaches the
order path some other way.

The consequence for anyone implementing this: **a host that only implements `008255B0` will never
move a ship.** The motion path runs through `00825F20`, which is reached as a virtual (its only
direct callers are `00749B2C`, `0075827B` and `0085542F`, and its address appears in six unit
vftables), not from the world entity walk. `008255B0` consumes motion; it does not produce it.

## Reconstruction

Appended to `include/bsp/unit_controller.hpp` / `src/unit_controller.cpp`:

| Symbol | Native | Coverage |
| --- | --- | --- |
| `unit_effect_intensity_gate_0082583e` | `0082583E..00825868` | complete for the gate expression |
| `set_effect_group_scalar_00815370` | `00815370..008153DD` | complete |
| `publish_unit_effect_intensity_00815aa0` | `00815AA0..00815D15` | complete |
| `kUnitControllerBodyAxisRowOffset` | `0092D757` | constant only; the dot is `unit_forward_speed_0092d730` in `bsp/unit_motion.hpp` |

Nothing was redeclared: `unit_controller_update_0092be80`, the controller layout constants and
`unit_forward_speed_0092d730` already existed and are referenced.

## Corrections

### To this packet's own brief

The brief asked for "the controller subobject at `unit+1018h`: its layout from its constructor". The
layout and the constructor were already fully recovered by `docs/UNIT_CONTROLLER_UPDATE.md`
(`00939CB0`, the `0x390`-byte table). Redoing them would have duplicated a merged packet, so this
document cites them and spends its evidence on the three routines that were still `FUN_`.

### To `docs/UNIT_INSTANCE_UPDATE.md`, step 4

| Was | Is | Evidence |
| --- | --- | --- |
| "`x = (0092D730([this+1018h]) >= 0.0f) ? 1.0f : 0.0f; 00815AA0(this, x)`. Note the argument is a 0/1 scalar, **not** the delta." | Correct, and now complete: `00815AA0` multiplies that gate by the `006FF270` intensity and publishes the product to twelve emitter-group pointers plus two arrays, behind a change latch at `unit+9D0h` | `00815AA0..00815D15` read in full |

The earlier reading did not identify `00815AA0`; it is not a delta consumer at all.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `unit_emitter_groups` | 00815370 callees, vtable `14h` of an emitter, unit `+9E8h`..`+B50h` | docs/UNIT_EMITTER_GROUPS.md | What an emitter group is, what `vtable[14h]` does with the scalar, and which visual each of the twelve unit fields drives |
| `unit_ship_motion_entry` | 00825F20, 00CFC394 and the five other vftable slots | docs/UNIT_SHIP_MOTION_ENTRY.md | Which virtual `00825F20` is and who calls it in a frame; this is the missing producer half of the motion path |
| `unit_water_anchors` | 004842C0, unit `+9F0h`, `+9F4h` | docs/UNIT_WATER_ANCHORS.md | The object that takes both a position from step 5 and a scalar from `00815AA0` |

## no_ghidra_function

none. `0092D730`, `0092BE80`, `00815AA0` and `00815370` all have Ghidra functions:

| Address | Ghidra body |
| --- | --- |
| 0092D730 | 0092D730 - 0092D76E |
| 0092BE80 | 0092BE80 - 0092BE82 |
| 00815AA0 | 00815AA0 - 00815D15 |
| 00815370 | 00815370 - 008153DD |

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 0092D730 | BSP_UnitController_GetBodyAxisSpeed (evidence appended) | exported, analyzed, reconstructed elsewhere |
| 0092BE80 | BSP_UnitController_UpdateStub (unchanged) | analyzed, reconstructed elsewhere |
| 00815AA0 | BSP_UnitInstance_PublishEffectIntensity | analyzed, reconstructed, build-tested |
| 00815370 | BSP_EffectGroup_SetScalar | analyzed, reconstructed, build-tested |

Nothing in this packet is fixture-tested, ABI-compatible or game-validated.

## Uncertainties

* What the `+2F4h` scalar means is still open; `docs/UNIT_INSTANCE_UPDATE.md` raised it and nothing
  here settles it. `00815AA0` proves it is broadcast to every emitter the unit owns, which fits a
  visual fade better than an audio attenuation, but that is an inference, not evidence.
* The twelve single-pointer group fields are distinguished only by offset. None of their types
  was read.
* The emitter `vtable[14h]` body was not read, so "set a scalar" is the contract's shape from its
  one float argument, not from the callee.
* `00C32000` and `00C31F40` are physics-library leaves with no callees and no strings. The reading
  of `axis+18h..+20h` as a forward axis is from how `0092D730` combines it with a velocity, not
  from the library.
* `00651760` issues orders without reading the input singleton. Where its order comes from is not
  established.
