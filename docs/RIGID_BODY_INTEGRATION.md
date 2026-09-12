# The rigid body the ship controller drives, and how it is integrated

Addresses: 00C41550, 00C5B1B0, 00C5BB30, 00C5C540, 00C31FC0, 00C37F40, 00C37E70, 00C37DE0,
00C37E00; read as contracts 00937440, 009329C0, 0092E8C0, 00447510, 0092AAE0, 00937C90.

Packet `cc_ship_inputs`, 2026-09-11. Reconstructed in `include/bsp/rigid_body_integration.hpp`
and `src/rigid_body_integration.cpp`; semantic C++ interfaces for MSVC Win32, not drop-in
binary replacements. Descriptive names are hypotheses, not recovered symbols. The saved
project is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. This worker made
no Ghidra mutation; the ledger records the new names.

`docs/SHIP_MOTION.md` left the integrator as a labelled stand-in: "the integration of those
forces into velocity, position and orientation happens in the rigid-body library's own step,
which no packet has located". This packet locates it. It is two routines, not one, and both
are outlined blocks of the substep body `00C5BB30`.

## What the library is

The block `[00C30930,00C5DF60)` is in-house (`docs/UNIT_CONTROLLER_UPDATE.md` establishes
that negatively: no middleware string anywhere, Hungarian diagnostics around it). It is not
anonymous, though. Its own RTTI names classes in a `Dyn` namespace: the vtable at `00D7A090`
slot 0 is `Dyn::Scene::LCPSolver2Task::` (TypeDescriptor `00E1736C`, the inventory comment on
`00403850`), and `Dyn_ConvexRayIntersection_vslot0`, `Dyn_CylinderShape_vslot2` and
`Dyn_BoxBoxIntersect_vslot0` carry the same provenance. `Dyn` is therefore the library's own
namespace and the new names use it. The solver being an LCP solver also says what the
collision and constraint phases this packet does not read are.

## The two objects

`B` is the body, which the unit's force-model controller holds at `controller+2Ch`.
`M = *(B+4h)` is its motion state. `docs/UNIT_CONTROLLER_UPDATE.md` already had most of `B`
from the accessors; the two integration phases are the producers of every field below and
settle the ones the accessors left ambiguous.

| `M` | field | producer |
| --- | --- | --- |
| `+00h` | vec3 linear velocity | `00C41550`, `00C5B1B0`; written by `00C37E50` |
| `+0Ch` | vec3 angular velocity | same; written by `00C37E20` |
| `+18h` | float, maximum linear speed | clamp at `00C5B1B0` |
| `+1Ch` | float, maximum angular speed | clamp at `00C5B1B0` |
| `+20h` | vec3 linear pseudo-velocity, added to the position but not to the velocity | `00C5B1B0`, cleared at the end of every substep |
| `+2Ch` | vec3 angular pseudo-velocity | same |
| `+38h` | vec3 accumulated force | `00C32050` sets, `00C35360` adds, `00C41550` consumes, `00C5B1B0` clears |
| `+44h` | vec3 accumulated torque | `00C32030` sets, `00C35330` adds, same consumer |
| `+50h` | float **inverse** mass | `00C37F40` stores `1/mass` here; `00C41550` multiplies the force by it |
| `+54h`,`+58h`,`+5Ch` | float, body-space inverse inertia diagonal | `00C37E70` stores `1/I` per axis |
| `+60h`..`+80h` | 3x3 world-space inverse inertia, rebuilt every substep | `00C41550` |
| `+84h`..`+B3h` | the previous 3x4 transform | copied from `B+8h` by `00C5C540` before the substep loop; `00C43EA0` lerps against it |
| `+B4h` | byte: confine torque response to the body's row-1 axis | `00C41550`'s tail |
| `+B8h` | float linear damping rate | `00C37E00` |
| `+BCh` | float angular damping rate | `00C37DE0` |

`B+50h` bits, from the sites that test or set them: bit 0 static (`00C31FC1`), bit 1 asleep
(set by `00C5B1B0`'s countdown), bit 2 suppresses gravity (`00C41550`), bit 4 excludes the
body from both integration phases. Every mutator clears bits 1 and 4 (`AND 0FFFFFFEDh`),
which is the wake.

The world fields the phases read: gravity at `world+04h..+0Ch`, sleep speed thresholds at
`world+3Ch` and `world+40h`, the sleep countdown reload at `world+44h`, the body list head at
`world+204h` with the sentinel `world+208h` and the next pointer at `body+84h`.

## `00C41550`, the velocity phase, complete for the integration

Body `00C41550..00C41AC8`. It is an outlined block: the world arrives in `ESI` with no
prologue that sets it (the decompiler shows `unaff_ESI`), because `00C5BB30` keeps the world
there across the whole substep. `dt` is the one stack argument. Called at `00C5BB5A`, before
any profiler scope opens.

```
for (body = [world+204h]; body != world+208h; body = [body+84h]) {
  if (body+50h & 10h) continue;
  M = [body+4h];
  k = M[+50h] * dt;
  M.v += k * M.force;                                  ; the inverse mass makes this F/m*dt
  if ((body+50h & 4) == 0) M.v += world.gravity * dt;
  d = 1 - M[+B8h]*dt; if (d <= 0) d = 0; M.v *= d;
  Iw = sum over k of M[+54h + 4k] * (row_k (x) row_k)   ; rows at body+8h, +14h, +20h
  M.w += (Iw * M.torque) * dt;
  d = 1 - M[+BCh]*dt; if (d <= 0) d = 0; M.w *= d;
  if (M[+B4h]) Iw = M[+58h] * (row1 (x) row1);          ; rows 0 and 2 multiplied by 0.0f
}
```

Two details that are easy to lose. `Iw` is `R^T diag(I) R` with `R` the matrix whose rows are
the body axes, so the axes are rows, not columns. And the row-1 lock is recomputed **after**
the angular velocity update, so it takes effect on the next substep, not on the one that set
it. The reconstruction keeps both.

## `00C5B1B0`, the position phase, complete for the integration

Body `00C5B1B0..00C5BB29`, world in `EBX` for the same reason, `dt` on the stack. Called at
`00C5C491` inside the rdtsc profiler scope whose image label is the string `UpdatePosition`
(fetched through `00C50390` at `00C5C469`).

```
pos += (M.v + M.linearBias) * dt;
a   = (M.w + M.angularBias) * 100.0f;                  ; float of the double at 00D7A220
mag = sqrtf(|a|^2);                                    ; through 004011D0
if (mag > 1e-5f) {                                     ; the float at 00D7A310
  a /= mag; angle = (mag * dt) / 100.0f;
  row2 = rodrigues(row2, a, angle);                    ; FSIN/FCOS, one pair per row
  row1 = rodrigues(row1, a, angle);
  row2 = normalise(row2);
  row0 = normalise(row1 x row2);
  row1 = row2 x row0;
}
d = 1 - dt*M[+BCh]; clamp >= 0; M.w *= d;
d = 1 - M[+B8h]*dt; clamp >= 0; M.v *= d;
if (M[+18h]^2 < |M.v|^2) M.v = normalise(M.v) * M[+18h];
if (M[+1Ch]^2 < |M.w|^2) M.w = normalise(M.w) * M[+1Ch];
if (world[+40h]^2 < |M.w|^2 || world[+3Ch]^2 < |M.v|^2) { body+50h &= ~12h; body+54h = world[+44h]; }
else if (--body+54h < 0) { M.v *= 0.9f; M.w *= 0.9f; body+50h |= 2; }
M.force = M.torque = M.linearBias = M.angularBias = 0;
```

The scale-by-100 before the magnitude test and the divide-by-100 in the angle cancel, so the
effective threshold on the unscaled angular speed is `1e-7` rad/s. The `0.9f` is the float of
the double at `00D7A390`.

The basis rebuild is what makes the handedness question `docs/SHIP_MOTION.md` left open
answerable: `row0 = row1 x row2` and `row1 = row2 x row0` is exactly the right-handed
identity set, so `(row0, row1, row2)` is a right-handed orthonormal triple, re-established
every substep in which the body rotates. With row 2 the forward axis (`0092D730`) and row 1
the up axis (the commanded yaw rate lands there), row 0 is `up x forward`, the remaining
lateral axis, and it is perpendicular to both by construction rather than by assumption.
The steering routine's row-0 term at `0092EA5B` therefore acts about the hull's lateral
axis. That is the pitch axis, and the angle it damps is consistent with it: `0092E9E9` takes
`0042CF10(M21)`, the angle of row 2's vertical component, which is the hull's pitch, and
pulls it back toward the +/-15 degrees at `00D19628`. It is a bow-up/bow-down limiter, not a
roll damper. Which way along the lateral axis is positive still depends on the world's own
axis convention, which this packet does not settle, so the sign stays provisional.

**Damping is applied twice per substep**, once in each phase. That is what the listings do;
no reading removed it.

**The force accumulators are cleared at the end of every substep.** The game side pushes
force onto a unit's body once per 0.05 s game step (`00826A6D` dispatches the controller's
force-model slot from `00825F20`, which the entity think list runs, and `00875E0C` runs the
whole physics world earlier in the same step). So the commanded force is integrated by
exactly one substep, whatever the substep count is, and the remaining substeps of that game
step coast. Any attempt to match the game's numbers has to model that, not `F*dt_game`.

## `00C5C540` and the substep schedule

`__thiscall void(world, float)`, body `00C5C540..00C5C706`, row 1 of the fixed-step fanout
(`docs/FIXED_STEP_FANOUT.md`). `world+00h` is the fixed substep, `world+34h` a budget used as
a countdown, `world+48h` the accumulator:

```
accumulator += dt;
while (world[0] < accumulator) { if (budget == 0) break; 00C5BB30(world, world[0]);
                                 budget = (float)((int)budget - 1); accumulator -= world[0]; }
if (budget != 0 && accumulator > 5.0e-5) 00C5BB30(world, accumulator);   ; the double at 00D7A398
accumulator = 0;
```

The substep calls are at `00C5C66D` and `00C5C6BD`. `world+00h` itself is **not established**:
`00424A10` is not its producer, no immediate store of a plausible step constant exists inside
the library range, and this packet did not find the world's constructor. If `world+00h` is
`0.05f` the loop runs zero times and the remainder branch takes one substep of the full step;
if it is smaller, several substeps run and only the first sees the commanded force. Follow-up
`dyn_world_construction`.

## Coverage

| routine | state | coverage |
| --- | --- | --- |
| `00C41550` | reconstructed, build-tested, probe-exercised | complete |
| `00C5B1B0` | reconstructed, build-tested, probe-exercised | complete |
| `00C5BB30` | read for its phase order and call sites only | partial: the collision and constraint phases `00C5BB5F..00C5C455` are not read |
| `00C5C540` | the substep schedule reconstructed | partial: the proxy transform copy and the profiler bookkeeping are described, not reconstructed |
| `00C31FC0`, `00C37F40`, `00C37E70`, `00C37DE0`, `00C37E00`, `00C37E20`, `00C37E50`, `00C35330`, `00C35360` | reconstructed from their full bodies | complete |
| `00937440`, `009329C0`, `0092E8C0` | reused as contracts from `docs/SHIP_MOTION.md` and `docs/UNIT_CONTROLLER_UPDATE.md` | as reconstructed there |
| the hull body's creation | not found | none; see the follow-ups |

## Corrections

**To `docs/UNIT_CONTROLLER_UPDATE.md`, the motion-state table.** It records `M+50h` as
`float mass` and `00C31FC0` as "inverse mass". Both are the wrong way round. `00C37F40`
stores `1.0f/mass` into `M+50h` (`FLD1`/`FDIVRP`/`FSTP [EAX+50h]` at `00C37F47..00C37F53`),
and `00C41550` forms `M[+50h]*dt` and multiplies the accumulated force by it, which is an
acceleration only if the field is the inverse mass. `00C31FC0` returns the reciprocal of that
field, i.e. the mass, and its `FLDZ` for a static body is the guard against `1/0` rather than
a zero mass. `docs/GAME_DYNAMICS_LIST.md` independently uses it that way: its buoyancy force
is `00C31FC0(body) * 10.0 * submersion / record[+1Ch]`, a mass times a gravity of 10.

**To `docs/UNIT_CONTROLLER_UPDATE.md`, the `B+50h` bit table.** It records bit 2 as "set to
freeze". The bit's only reader is `00C41550`, where it skips the gravity term and nothing
else. The disabled controller path at `00932A16` does read as a freeze, but only because it
zeroes both velocities first; the bit on its own suppresses gravity.

**To `docs/UNIT_CONTROLLER_UPDATE.md`, the physics-library boundary.** It says the block has
"no RTTI". The Dyn classes listed above have RTTI TypeDescriptors, and the namespace is
recoverable from them. The rest of that section (no middleware string, in-house library)
stands.

**To `docs/SHIP_MOTION.md`, the handedness.** It records "which handedness the three make is
not established, so whether the row-0 term rights the hull or capsizes it is provisional".
`00C5B1B0`'s basis rebuild settles the handedness: `row0 = row1 x row2` and
`row1 = row2 x row0` make `(row0, row1, row2)` a right-handed orthonormal triple. Row 0 is
therefore the lateral axis and the term at `0092EA5B` is a pitch limiter rather than a roll
damper. The sign of the correction remains provisional, because which direction along the
lateral axis is positive depends on the world's axis convention.

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `dyn_world_construction` | the writer of `[game+18h]`, `world+00h`, `world+34h`, `world+3Ch`..`+44h` | the substep size, the substep budget and the sleep thresholds, which are the last unknowns between this reconstruction and the game's numbers |
| `ship_hull_body_creation` | the writer of `controller+2Ch`; `00937C90`, `0092AAE0`, `00935D30` | where the hull body is created and what mass, inertia and damping it gets. `00C37F40`/`00C37E70` have four callers between them and none of them is on the hull path read here; `0092AAE0` stores its body at `owner+354h`, not `controller+2Ch` |
| `dyn_contact_solver` | `00C5BB5F..00C5C455`, `00C4B610`, `00C4B550`, `00C5C7A0`, `00D7A090` | the collision, group and LCP phases of the substep, and what the contact callback through `world+24h` receives |
| `unit_force_channel` | `0074F2E0`, `unit+10D4h` | the list through which engine thrust, damage and explosions push force onto a unit, which is the missing input to `009329C0` |

## no_ghidra_function

none. Every address named or reconstructed in this packet lies inside an existing Ghidra
function body, checked with `python tools/bsp.py ghidra proto <addr> --brief`.
