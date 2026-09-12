# What pushes force onto a unit's hull body

Addresses: 0092BF30, 00821E80, 0080FFD0, 00762220; read as contracts 00C35360, 00C35330,
00C37E50, 00C37E20, 00C37E70, 0074F2E0, 0074F930, 00768530.

Packet `cc_ship_motion_tail`, 2026-09-11. Reconstructed in
`include/bsp/unit_force_channel.hpp` and `src/unit_force_channel.cpp`; semantic C++
interfaces for MSVC Win32, not drop-in binary replacements. Descriptive names are
hypotheses, not recovered symbols. The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. This worker made no Ghidra mutation; the ledger records the
new names.

## The premise this packet was given, and what is actually there

`docs/RIGID_BODY_INTEGRATION.md` proposed `unit_force_channel` as "`0074F2E0`,
`unit+10D4h`: the list through which engine thrust, damage and explosions push force onto a
unit, which is the missing input to `009329C0`". Both halves of that are wrong, and one half
was already corrected in the tree before this packet started.

* **`unit+10D4h` is the leak (flooding) model, not a force list.** That correction is
  already in `docs/UNIT_CONTROLLER_UPDATE.md` (the closing paragraph) and
  `docs/UNIT_FORCE_COMMANDS.md` (the section "The leak model at `unit+10D4h`"), and the
  record layout and both halves of the tick are already reconstructed in
  `include/bsp/unit_forces.hpp` and `src/unit_forces.cpp`. `0074F930` accumulates water per
  leak point and `0074F2E0` turns the water weights into a heeling torque that `009329C0`
  stages into `controller+74h..+7Ch`. This packet adds nothing there and repeats nothing.
* **There is no damage or explosion force path onto a hull body at all.** The complete xref
  sets settle it: `00C35360` (`AddForce`) has exactly one caller in the whole image, and
  `00C35330` (`AddTorque`) exactly three.

## Every write into a hull body's motion state

From `ghidra xrefs` on the six mutators, with the containing function from
`ghidra proto --brief` on each site. This is the whole channel.

| call site | containing | native | what it writes |
| --- | --- | --- | --- |
| `00933B01` | `009329C0` | `00C35360` `AddForce` | the hydrodynamic model's summed force. The only `AddForce` site in the image |
| `00933B38` | `009329C0` | `00C35330` `AddTorque` | the hydrodynamic model's summed torque, which carries the leak heeling term |
| `00937613` | `00937440` | `00C35330` | the ship override's rudder torque |
| `0092BF33` | `0092BF30` | `00C35330` | the controller's `AddTorque` helper, reached only from unit message `93h` |
| `009329EC` | `009329C0` | `00C37E50` set linear velocity | the disabled-controller path, which zeroes both velocities |
| `00932A0E` | `009329C0` | `00C37E20` set angular velocity | the same path |
| `0092D588` | `0092D300` | `00C37E50` | the throttle half of the motion tick |
| `0092D84D` | `0092D770` | `00C37E50` | the unconditional axial-speed setter |
| `0092EB8F` | `0092E8C0` | `00C37E20` | the rudder half of the motion tick |
| `0092E812` | `0092E5B0` | `00C37E50` | a second controller routine, not read here |
| `0092E895` | `0092E5B0` | `00C37E20` | the same routine |
| `0093739A` | `00936DC0` | `00C37E50` | the depth-holding override |
| `009373A7` | `00936DC0` | `00C37E20` | the depth-holding override |
| `00937675` | `00937630` | `00C37E50` | the frozen override, which zeroes both velocities |
| `00937697` | `00937630` | `00C37E20` | the frozen override |
| `00939C05` | `00937C90` | `00C37E70` set inertia | the hull body's creation, `docs/SHIP_HULL_BODY.md` |

Two further sites exist and reach other bodies, not a hull: `00447A34` in
`BSP_GameDynamicsList_Add` and the `00C32050` set-force site in
`BSP_GameDynamicsList_ApplyBuoyancyStep`, both on the debris bodies of
`docs/GAME_DYNAMICS_LIST.md`. They are listed here so a reader does not have to re-derive
that they are out of scope.

Every entry above lies inside the controller family except one. `0092BF30` is the only
channel from outside.

## The external channel: unit message `93h`

`0092BF30` is two instructions:

```
0092bf30: MOV ECX,dword ptr [ECX + 0x2c]
0092bf33: JMP 0x00c35330
```

Its `ECX` is a controller, `controller+2Ch` the hull body, so it is the controller's own
`AddTorque`. It has exactly one caller, `00821E80` at `00822255`.

`00821E80` is a unit's message handler; `undefined1 __thiscall(unit, msg)` with `RET 4`
(`0082226C`), switching on the byte at `msg+10h`. Case `93h`:

```
00822235: EAX = [msg+1Ch]     00822238: ECX = [msg+20h]     0082223B: EDX = [msg+24h]
0082223E/00822246/00822251: the three land in a stack vec3
0082224A: ECX = [unit+1018h]      ; the controller
00822250: PUSH &vec3
00822255: CALL 0092BF30
0082225B: AL = 1
```

The payload's layout is settled by its **producer**, `0080FFD0`, which writes exactly
`msg+1Ch`, `+20h` and `+24h` from its second, third and fourth arguments after
`BSP_SessionMessage_ConstructBase(93h)`; the handler's use of them agrees.

Two neighbouring cases are the reason the leak model and the force channel were once read
as one thing: case `91h` calls `0074E830` and case `92h` calls `0074E860` with
`ECX = unit+10D4h` (`008221F3` and `0082221B`), i.e. on the leak model.

## Who sends message `93h`

`0080FFD0` is the local constructor for the message class whose vtable is `00D034C8`. **It
has no callers in the image.** The only other reference to that vtable is `0076226E`, inside
`00762220`, which `BSP_SessionMessage_CreateFromStream` (`00768530`) calls: the stream
decoder's constructor for the same class, which fills the sender slot from
`game+18CCh + game+18ECh*4` and leaves the payload to the stream.

So the external torque channel exists and is wired end to end, but nothing in this image
produces such a message locally. In the shipped executable a hull body's only force and
torque come from `009329C0`'s hydrodynamic model and `00937440`'s rudder term; damage and
explosions reach a unit through its damage model, not through Dyn.

That is what `009329C0` was missing an input for, and the answer is that it is not missing
one: it is the producer, not the consumer.

## Coverage

| routine | state | coverage |
| --- | --- | --- |
| `0092BF30` | reconstructed from its full body | complete |
| `00821E80` case `93h` | reconstructed | partial: one case of a switch whose body is `00821E80..00822393`; the other cases are read only far enough to place `91h` and `92h` |
| `0080FFD0` | read in full as the payload's producer | complete for the layout question; not reconstructed |
| `00762220` | read in full | complete for the sender question; not reconstructed |
| `009329C0`, `00937440`, `0074F2E0`, `0074F930` | reused as contracts | as reconstructed in `docs/UNIT_CONTROLLER_UPDATE.md`, `docs/UNIT_FORCE_COMMANDS.md` and `include/bsp/unit_forces.hpp` |

## Corrections

**To `docs/RIGID_BODY_INTEGRATION.md`, the `unit_force_channel` follow-up row.** "The list
through which engine thrust, damage and explosions push force onto a unit" describes
nothing in the image. `unit+10D4h` is the leak model (already corrected in
`docs/UNIT_CONTROLLER_UPDATE.md`), and no damage or explosion routine calls any Dyn
mutator. The row should read: the external channel is unit message `93h`, handled at
`00822235`, and it is the only site outside the controller family that touches a hull body.

**To the same row, "the missing input to `009329C0`".** `009329C0` has no missing force
input. It is the only `AddForce` caller in the image.

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `unit_message_kinds` | `00821E80`, `00768530`, the `msg+10h` kind bytes `4Bh`..`95h` | the whole unit message table, of which this packet reads three cases |
| `unit_controller_0092e5b0` | `0092E5B0..0092E8BF` | the fourth pair of velocity writes, the only controller routine in the table this packet did not read |
| `unit_damage_to_motion` | the damage model's outputs | how damage reaches motion, given that it does not reach the hull body |

## no_ghidra_function

none. Every address named or reconstructed in this packet lies inside an existing Ghidra
function body, checked with `python tools/bsp.py ghidra proto <addr> --brief`.
