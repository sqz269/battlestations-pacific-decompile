# The hull's hydrodynamics: drag, buoyancy and the physics material record

Addresses: 009329C0 whole (009329C0..00933BA9), with 009329EC / 00C37E50, 00932A0E / 00C37E20,
00932A22 and ten more / 00424C40, 00932A42 and 00932E2F / the unit vtable slot 5Ch, 00932BB0 /
00C31F40, 00932BE6 / 00C31F20, 00932C21 / 00C32000, 00932C28 / 00C33650, 00932CBF and 00932D02 /
004142E0, 00932D6B / 0078CF20, 00932D8B, 009338F6 and 009339A7 / 004F9B30, 00932F19 and 0093300B /
00BF7030, 00933063 / 0042B260, 00933A01 / 0074F930, 00933A52 / 0074F2E0, 00933B01 / 00C35360 and
00933B38 / 00C35330; and the record's producer, the loop 0083FEE7..008403B7 inside 0083B5E0.

Packet `cc_hydro_forces`, worker `agent/cc-hydro-forces`, 2026-09-12. Reconstructed in
`include/bsp/ship_hydro_forces.hpp` and `src/ship_hydro_forces.cpp`; the probe extension is in
`src/ship_motion_probe.cpp` under `--hydro`. Report: `reports/ship_hydro_forces.json`. No Ghidra
mutation beyond five ledger names and one appended evidence line. Every name below is a hypothesis,
not a recovered symbol.

`docs/GAME_EXECUTABLE.md` milestone 2r named this routine as the single largest thing standing
between the executable and a ship that goes where it is sent. It is a per-element hydrodynamic
model: the hull carries a list of buoyancy elements, and for each one the routine samples the ocean
under it, works out how deep it is, forms the velocity of that point, splits the velocity into the
three body axes, applies a linear and a quadratic drag coefficient per axis, clamps the result so
one element cannot reverse the hull, adds the buoyancy straight up, and turns the whole thing into
a force and a torque on the rigid body.

## The one thing to read first

`00937440`'s tail call at `00937622` forwards the substep `dt`. The routine's very first test is
`CMP byte ptr [ESI + 0x14], 0` at `009329C9`, and `JZ` takes the **live** path at `00932A1F`. The
other branch, `009329CF..00932A1A`, is the disabled one: it zeroes both velocities through
`00C37E50` and `00C37E20`, ORs bit 2 (suppress gravity) into `body+50h`, and jumps straight to the
accumulator zeroing at `00933B40`. Milestone 2r read those two velocity writes as the live path's;
they are not. On the live path the routine never writes a velocity. It writes force and torque.

## The physics material record, from its producer

`00932A53..00932A8F` selects the same row `00937CF1` picks for the hull body: the unit's vtable
slot `5Ch` called with 8 answers material 2, otherwise `class+B0h` against the double `100.0` at
`00D7A220` answers 0 at or above and 1 below. The `LEA`/`SUB`/`ADD` chain at `00932A82..00932A8F`
scales that to a `38h` stride, so the row is at `settings + 4E0h + material*38h`.

The row is fourteen floats, and the **producer** binds every offset to its Lua key: the
three-iteration loop of `0083B5E0` at `0083FEE7..008403B7` pushes one key string and stores the
answer at a fixed offset, over and over.

| offset | key string | Lua key | loader default | read by `009329C0` |
| --- | --- | --- | --- | --- |
| `+4E0h` | `00D0A764` | `KozegellenallasiEgyutthatoL` | `0.0025f` (`00CF01F4`) | `00932A96` |
| `+4E4h` | `00D0A748` | `KozegellenallasiEgyutthatoN` | `0.01f` (`00D7A238`) | `00932AAD` |
| `+4E8h` | `00D0A728` | `KozegellenallasiEgyutthatoLFel` | `0.0025f` | `00932AC4` |
| `+4ECh` | `00D0A708` | `KozegellenallasiEgyutthatoNFel` | `0.01f` | `00932ADB` |
| `+4F0h` | `00D0A6E4` | `KozegellenallasiEgyutthatoLOldalra` | `0.0025f` | `00932AF2` |
| `+4F4h` | `00D0A6C0` | `KozegellenallasiEgyutthatoNOldalra` | `0.01f` | `00932B09` |
| `+4F8h` | `00D0A69C` | `KozegellenallasiEgyutthatoLElore` | `0.0025f` | `00932B20` |
| `+4FCh` | `00D0A678` | `KozegellenallasiEgyutthatoNElore` | `0.01f` | **never** |
| `+500h`..`+508h` | `00D0A668` | `NyomatekSzorzo[1..3]` | `1.0f` | no, `00939BCB` does |
| `+50Ch` | `00D0A660` | `Kitevo` | `1.5f` (`00CE380C`) | `00932B3F` |
| `+510h` | `00D0A654` | `Gravitacio` | `10.0f` (`00CE38B8`) | no, `00937C90` does |
| `+514h` | `00D0A648` | `Friction` | `1.0f` | no, `00939365` does |

This is the order `docs/UNIT_RUDDER_CURVE.md` inferred from the key block, confirmed position by
position from the stores. The installed values in
`scripts/datatables/shipglobals.lua`, `ShipGlobals["Physics"]`:

| | L | N | LFel | NFel | LOldalra | NOldalra | LElore | NElore | Kitevo | Friction |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `Ship` | 0.1 | 0.03 | 0.1 | 0.03 | 0.1 | 0.01 | 0.001 | 0.03 | 1.0 | 0.5 |
| `TBoat` | 0.5 | 0.03 | 0.5 | 0.03 | 0.5 | 0.1 | 0.075 | 0.001 | 0.5 | 0.5 |
| `Submarine` | 0.05 | 0.01 | 0.1 | 0.03 | 0.1 | 0.03 | 0.001 | 0.03 | 1.0 | 1.0 |

Every shipped destroyer takes the `Ship` row. Its lateral coefficient is a hundred times its
forward one, which is the whole shape of the model: a hull slides forward easily and sideways
hardly at all.

**Seven coefficients are read, not eight.** The eighth address milestone 2r lists, `00932B3F`, is
the `Kitevo` read at `+50Ch`. `KozegellenallasiEgyutthatoNElore` at `+4FCh` is never read, and the
forward drag term at `0093324E..0093325B` is correspondingly linear only: `FLD` the speed, `FMUL`
by `LElore`, `FMUL` by the scale, with no speed-squared factor anywhere. The `CALL 00424C40` at
`00932B32` whose result is discarded is where the eighth read would have been.

`00932B3C` then compares the material with 1 and `00932B53` overwrites the loaded exponent with the
`2.0f` at `00CE3958`. That settles `docs/UNIT_RUDDER_CURVE.md`'s open question: the override is
real and the shipped `TBoat` `Kitevo` of 0.5 is dead for this routine.

## The buoyancy element list at `class+52Ch`

The vehicle class descriptor at `unit+538h` carries a vector whose first and last pointers are
`class+52Ch` and `class+530h`; the count is their difference divided by `24h`, which the compiler
does with the reciprocal `38E38E39h` at `00932C4B` and five further sites. One element is nine
floats.

**Its producer was not read.** No function in the exported set writes `class+528h..+534h`, and the
`fizika_%02d` / `hajobelso` node walkers `00935D30` and `005072C0` do not touch it. The field roles
below are a hypothesis reconciled between the only two readers, this routine and `00937C90`'s
displacement sum, and the follow-up packet that finds the writer should re-check them.

| offset | name used here | what the readers do with it |
| --- | --- | --- |
| `+00h` | `coefficient` | `00932EE3` scales the buoyancy by it; `00937C90` sums `coefficient * displacement * Gravitacio / 10` |
| `+04h` | `level_top` | only ever `level_top - level_base`: the buoyancy shape denominator at `00932E18`, and `00937C90`'s full height |
| `+08h` | `level_draft` | only ever `level_draft - level_base`: the span submersion is measured over, `00932DCB` |
| `+0Ch` | `level_base` | the common subtrahend |
| `+10h`, `+14h` | unread | neither reader touches them |
| `+18h`..`+20h` | `position` | the hull-local point, transformed twice at `00932CBF` and `00932D02` |

## The element loop, term by term

The loop index is `EBP`, zeroed at `00932C1F` and only ever incremented at `009339E5`. Every
`TEST EBP,EBP / JL` branch inside the body is therefore **dead in this build**: `00932DC9`,
`00932E16`, the whole whole-hull fallback at `00932E71..00932EE2` (which would use an element span
of the `3.0f` at `00CE3854` and a buoyancy of `min(settings+5B8h * Mass, settings+5B0h)`), and the
element advance guard at `009339DA`. They are read and understood; they are not projected, and the
header says so.

The transform every step works in is the body's own, fetched at `00932C21` / `00932C28`: `00C32000`
returns `body+8h` and `00C33650` expands that `3x4` into a `4x4`. Row 0 is the lateral axis, row 1
the up axis, row 2 the forward axis and row 3 the position, the same convention
`docs/RIGID_BODY_INTEGRATION.md` records for `DynBody`.

**1. The point and the lever arm.** `00932CBF` transforms `element.position` to the world; that
point's X and Z are the water sample and its Y is the height test. `00932D02` transforms the same
point with its local Y forced to zero (`XORPS` at `00932CEE`, stored at `00932CF9`), and
`00932D07..00932D47` subtracts the transform's translation from it. That difference is the lever
arm `r` every torque uses, so all the lever arms lie in the hull's own waterline plane.

**2. The point velocity.** `00932D8B` calls `004F9B30` with the angular velocity as `a` and `r` as
`b`, so the cross product is `omega x r`, and `00932D90..00932DC5` adds the linear velocity. That
is the rigid body's velocity at the element.

**3. How deep it is.** `00932D70` gives `height_above_water = world_point.y - water`. With
`span = level_draft - level_base`, `00932DD5..00932E12` computes
`depth = span - clamp(height_above_water, 0, span)`. A point whose authored level sits exactly at
the surface reads fully submerged; one a whole span above it reads zero.

**4. Buoyancy.** `00932E2F` asks the unit's slot `5Ch` again; a submarine gets a mix `k` of `0.0f`
and anything else the `0.5f` at `00CE3800`. Then `00932E44..00932EE9`, in the listing's order:

```
buoyancy = coefficient * depth * ((1 - k) + k * depth / (level_top - level_base))
```

It is added at `00933780` to the **world Y** component of the element's force, not to the body's up
axis. With the shipped data the total over the list is the hull's weight against a gravity of 10,
which is what cancels `00C41550`'s `world.gravity * dt`.

**5. The drag, when it runs.** `00932F33` and `00932F6A` are the two gates: `depth > 0` and the
squared speed not exactly zero. Inside, `00932FB5` forms `fraction = depth / span`, and
`00932FC6..00933018` raises it to `Kitevo` by an **equality ladder**, not a power: `1.0f` leaves it,
`2.0f` squares it, `0.5f` takes the square root, and any other value falls through leaving the
previous iteration's result in the slot at `ESP+74h`. With the shipped rows and the material-1
override every case lands on one of the three, so the fall-through is latent; the loader's own
default of `1.5f` would reach it. `00933018..00933044` then forms

```
drag_scale = (class Mass / element count) * depth_factor * 10.0      ; the double at 00CE3DC0
```

The `10.0` is a literal, equal in value to the world's gravity magnitude. It is **not** the record's
`Gravitacio` at `+510h`, which this routine never reads.

`00933063` normalises the point velocity in place, and the three axis projections at `009330A8`,
`00933109` and `00933166` are each `(row . direction) * row`. The coefficients:

```
lateral  = speed * LOldalra + speed^2 * NOldalra      ; 00933199, onto row 0
forward  = speed * LElore                             ; 0093324E, onto row 2, linear only
vertical = speed * LFel + speed^2 * NFel   if dir.y >= 0    ; 00933372, onto row 1
           speed * L    + speed^2 * N      if dir.y <  0    ; 009332CC
force    = -drag_scale * (lateral + forward + vertical)     ; 00933223, 00933295, 00933344
```

The vertical branch is `COMISS 0, dir.y` with `JBE` at `00933161`/`009332C6`, so a dir.y of exactly
zero takes the upward pair. The unsuffixed `L`/`N` pair is the downward one; that is what settles
the Hungarian names against the axes.

**6. The impulse clamp.** `0093341B` forms `s = dt / total_mass`, where the mass is
`max(class+B0h + unit+10FCh, 1.0f)` from `00932B72` and `unit+10FCh` is the leak model's
accumulated water. `dv = force * s`, and for each body axis `009335E7`, `00933603` and `0093361D`
test `-1/count > (axis . dv) / (axis . v)` and, when it holds, replace the component with
`-(1/count) * (axis . v)`. So one element can remove at most `1/N` of a velocity component and the
whole list at most all of it, never reversing it. `0093363D..0093375E` divides the clamped
components back by `s` and recombines them as `forward + lateral` first, then `+ up`.

**7. The planing torque.** `00933639` reads `unit+5Dh`; when it is clear, the element is the last
of the list, the material is 1 and the forward speed `row2 . v_linear` is positive, `0093380A`
subtracts from the torque

```
(Width * Length * depth / span * forward_speed) * row1   crossed with   (Length * 0.5) * row2
```

`Length` is `class+A0h`, `Width` is `class+A4h` and the `0.5` is the double at `00D7A280`. That is
a bow-lift moment for a torpedo boat.

**8. The accumulators.** `0093395B` adds the element force into `controller+378h` and `009339A7`
adds `r x force` into `controller+384h`.

## The tail

`00933A01` ticks the leak model at `unit+10D4h` with the same `dt`. `00933A06..00933A4F` zeroes the
two scratch vectors at `controller+68h` and `+74h` and writes the flooding weight
`0 - unit+10FCh * 10.0` into `controller+6Ch`. `00933A52` asks `0074F2E0` for the leak's heeling
torque into `controller+74h..+7Ch`. Both scratch vectors are added into the accumulators, and
`00933B01` / `00933B38` flush them through `00C35360` `AddForce` and `00C35330` `AddTorque`.
`00933B40..00933B98` then zeroes `controller+378h..+38Ch` on **both** paths, which is why the
accumulators are locals in effect: every call starts them at zero.

There are no angular damping coefficients anywhere in the routine. Every torque it produces is a
lever-arm cross product of a linear force. What damps a turn is the lateral drag acting at each
element's offset from the body origin.

## The probe

`src/ship_motion_probe.cpp` gained `--hydro`, which runs `009329C0` inside every substep from
`ProbeSimulateHost::run_substep_00c5bb30`, before `dyn_body_substep` — the same place the library
calls the controller's slot 0, ahead of the velocity phase that consumes `M+38h`. The element list
is a **stand-in**: `stand_in_buoyancy_elements` spreads `--hydro-elements` (default 8) points along
the class `Length` at the local Y = 0 plane and solves the shared coefficient so the hull displaces
its own weight at its draft. It is not the game's list, because nothing has read the producer.

`--class 20 --moveto 4000,4000 --steps 8000`, body B, the class-built hull body for `DeRuyter`:

| | before | after |
| --- | --- | --- |
| minimum distance to the goal | 258.93 m | 0.34 m |
| final distance | 1483.99 m | 567.05 m |
| first step inside 2000 m | 4580 (229.01 s) | 4532 (226.61 s) |
| final heading error | 0.480076 rad | -1.342620 rad |
| peak drift angle | 75.2864 deg | 1.2196 deg |
| final forward speed | 14.4243 | 16.4530 |

The goal is reached: the minimum distance falls from 259 m to 0.34 m and the drift from 75 degrees
to 1.2. The final distance is larger than the minimum because the ship AI has no stop, so the hull
circles the goal after reaching it, and the final heading error is measured mid-circle. The
staged force on the last substep is `(5050.05, 76880.00, -5475.96)`; the Y component is exactly
`mass * 10`, the buoyancy cancelling gravity.

The torque it stages, about 3.0e6 about Y, does nothing in this run. The probe attaches no
collision shape, so the AABB span is zero and `00C37E70` stores a zero inverse inertia. With a real
span that term would be the hull's turn damping, which is what packet `ship_hull_shapes` is for.

## Coverage

| routine | coverage |
| --- | --- |
| `009329C0`, `009329C0..00933BA9` | complete. Four branch bodies are dead in this build and not projected: `00932DC9..00932DD4`, `00932E16..00932E24`, `00932E71..00932EE2`, `009339DA..009339E0` |
| `0042B260`, `004142E0`, `00C33650`, `00C32000` | complete, read whole |
| `004F9B30` | complete; reused as `cross_004f9b30` from `include/bsp/world_ocean.hpp`, re-read only to settle which operand is the angular velocity |
| `00C35330`, `00C35360`, `00C37E20`, `00C37E50`, `00C31F20`, `00C31F40` | contract only; reconstructed in `include/bsp/rigid_body_integration.hpp` |
| `0074F930`, `0074F2E0` | contract only; reconstructed in `include/bsp/unit_forces.hpp` |
| `0078CF20` | contract only; reconstructed in `include/bsp/ocean_height.hpp` |
| `0083B5E0` | partial: only `0083FEE7..008403B7`, the physics-material loop, for the key-to-offset binding. The rest of the settings loader is untouched |
| `00937C90` | none. Its element walk was read as corroboration only; `00937D3F..009399BF` belongs to `cc_hull_shapes` and was not touched |
| the `class+528h` element list | none. No producer read; the field roles are a hypothesis from two readers |
| the unit vtable slot `5Ch` | none. Called at `00932A42` and `00932E2F`; the callee's body was not read, so the host method is named by the record it selects |

## Corrections

**To `docs/GAME_EXECUTABLE.md` milestone 2r, section 2, on the velocity writes.** It reads
"`009329EC` calls `00C37E50` (set linear velocity) and `00932A0E` calls `00C37E20` (set angular
velocity)" as the live path. Both are on the disabled branch: `009329C9 CMP byte ptr [ESI+14h],0`
and `009329CD JZ 0x00932A1F` send the live path past them, and `00932A1A JMP 0x00933B40` is how
that branch leaves. The live path writes force and torque only.
`docs/UNIT_FORCE_CHANNEL.md` already had this right in its table.

**To the same section and to follow-up 2, on "eight drag coefficients".** Seven are read, at
`+4E0h`..`+4F8h`. The eighth listed address, `00932B3F`, reads `Kitevo` at `+50Ch`. `+4FCh`
`KozegellenallasiEgyutthatoNElore` is never read, which is why the forward drag is linear only.
The record has fourteen fields, not six.

**To `docs/UNIT_RUDDER_CURVE.md`'s Corrections, on the `+4E0h` row.** Not a correction but a
confirmation: the loader's stores bind every offset to the key the doc inferred, in every position.
The note "this mapping is inferred from the key names and the row stride" can be retired.

**To the same, on the variant-1 exponent.** The doc flagged a conflict between "variant 1 forces
`+50Ch` to `2.0f`" and the shipped `TBoat` value of 0.5. Both are true and the override wins:
`00932B3C CMP EBX,1` then `00932B53` loads the `2.0f` at `00CE3958` over the loaded value. The
shipped 0.5 is dead for this routine; `00937C90` reads `+50Ch` without the override and does use it.

**To the same, on `Gravitacio`.** The `10.0` this routine multiplies by at `0093303E` and
`00933A41` is the literal double at `00CE3DC0`, not the record's `Gravitacio` at `+510h`, which
`009329C0` never reads. The record's field is read by `00937C90`, where it appears divided by that
same literal.

**To the packet brief, on "the yaw/roll/pitch damping torques".** There are none. Every torque is a
lever-arm cross product of a linear force. Angular damping is the body's own `M+BCh` rate, which
`00939A2F` sets to `1.0f`.

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `ship_buoyancy_element_producer` | the writer of `class+528h..+534h`, the class descriptor's constructor | which model nodes or Lua keys build the `24h`-byte elements, and therefore whether `level_top` / `level_draft` / `level_base` are the right names |
| `ship_hull_shapes` | `00937D3F..009399BF`, `00C5C940` | the collision AABB. Without it the inverse inertia is zero and this routine's yaw torque is discarded. Carried over unchanged; owned by `cc_hull_shapes` |
| `unit_flag_5d` | the writer of `unit+5Dh` | what gates the planing torque at `00933639` |
| `unit_vtable_5c` | the callee behind `unit+0` slot `5Ch` | what category 8 actually means, which would let the submarine branch and the material selection be named from the callee instead of the record |

## no_ghidra_function

none. Every address cited above lies inside an existing Ghidra function body, checked with
`python tools/bsp.py ghidra proto <addr> --brief`: `009329C0`, `0042B260`, `004142E0`, `004F9B30`,
`0074F2E0`, `0074F930`, `0078CF20`, `00424C40`, `0083B5E0`, `00BF7030`, `00C31F20`, `00C31F40`,
`00C32000`, `00C33650`, `00C35330`, `00C35360`, `00C37E20`, `00C37E50`, `00937440`, `00937C90`.

## Correction from docs/SHIP_BUOYANCY_ELEMENTS.md

Packet `cc_buoyancy_elements` found the producer this doc could not: `0082D040` builds the list,
pushing through `std::vector<Element,24h>::push_back` `0082C960` with the vector object's own
base `descriptor+528h` in ECX (the displacement `52Ch` is never used; `009329C0` reads the list
as `ADD EDI,528h` then `[EDI+4]` and `[EDI+8]`). The elements are generated, not authored:
`0082FE30`, the ship class descriptor's model-binding virtual (slot 8 of all eight ship vtables;
`00759120` overrides it for MMothership), resolves the model nodes named `deckline` and
`bottomline`, sorts both polylines by ascending z, walks `Hull.Segments` stations evenly over
`Length`, samples both lines at each station with `0082A920` and pushes two records per station
at `+Width/2` and `-Width/2`. Field roles from the producer: `+04h` is the waterline (not a top),
`+08h` the deck line (not the draught), `+14h` the draught (`008936A0` `luaMW_GetDraught` returns
its maximum), `+10h`/`+14h` the cached section height and draught both readers recompute.
`coefficient * draught` is `Mass*10*0.5/Hull.Segments` on every record and there are
`2*Hull.Segments` of them, so the list sums to exactly `10*Mass` and `00937C90`'s displacement sum
cancels `Gravitacio*Mass` when the shape exponent leaves the factor at 1.
