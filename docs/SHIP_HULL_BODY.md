# The ship controller's hull body: where it is created and what it is given

Addresses: 00937C90, 00C5D580, 00C43CA0, 00C37E70, 0083B5E0; read as contracts 00939CB0,
00C336C0, 00C31F90, 00960230, 00424C40.

Packet `cc_ship_motion_tail`, 2026-09-11. Reconstructed in `include/bsp/ship_hull_body.hpp`
and `src/ship_hull_body.cpp`; semantic C++ interfaces for MSVC Win32, not drop-in binary
replacements. Descriptive names are hypotheses, not recovered symbols. The saved project is
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. This worker made no Ghidra
mutation; the ledger records the new names.

`docs/RIGID_BODY_INTEGRATION.md` left the writer of `controller+2Ch` unfound and listed the
hull body's mass, inertia and damping as "not found". This packet finds all of it.

## Where the body is created

`00939CB0`, the controller constructor, nulls `controller+2Ch` at `00939DBD` and calls
`00937C90` last, at `00939E2A`. `00937C90` is the controller's sub-object builder; its tail,
`009399C0..00939C05`, is the writer:

| address | what it does |
| --- | --- |
| `009399C7` | refreshes the unit's world matrix (`00414DB0`) when the byte at `unit+C8h` is clear |
| `009399E5` | `00C336C0` copies `unit+CCh` into the descriptor's `+14h` as a 3x4 transform, dropping the fourth column of the 4x4 |
| `009399F7` | the descriptor's mass is `class+B0h`, the vehicle class descriptor at `unit+538h` |
| `009399FD`..`00939A26` | the descriptor's inertia diagonal is zeroed |
| `00939A2F` | the descriptor's angular damping is the `1.0f` at `00D7A24C` |
| `00939A47` / `00939A4E` | the unit and the controller's embedded contact listener `controller+20h` |
| `00939A57` | the row-1 torque lock, set when the mass is below the double `100.0` at `00D7A220` |
| `00939A5F`..`00939A6F` | `world = [[00E188A8]+18h]`, then `00C5D580(world, &desc)` |
| `00939A86` | **`controller+2Ch = the returned body`** |
| `00939A89`..`00939C05` | the AABB read back, the box inertia, `00C37E70` |

`00C5D580` is `Dyn::World::CreateBody`. It takes a body out of the world's free list
(`_malloc(0x21340)` for a thousand `88h`-byte bodies at `00C5D5A0`) and, for a dynamic body,
a motion state out of a second pool (`_malloc(200000)` for a thousand `C8h`-byte states at
`00C5D690`), links them into the world's lists, calls `00C43CA0` with the body in `EAX` and
the descriptor in `EDX` (`00C5D8B5`/`00C5D8B7`), and then attaches every shape in the
descriptor's vector through `00C5C940` (`00C5D8C0..00C5D8E7`). The `88h` and `C8h` element
sizes confirm the `B` and `M` layouts `docs/RIGID_BODY_INTEGRATION.md` recorded.

## The body descriptor

`00C43CA0` is the producer of every body and motion-state field, so it settles the
descriptor's layout. It is `84h` bytes; the game default-constructs it at
`009391C2..009392EB` and then overrides seven slots.

| desc | type | body field | producer | default |
| --- | --- | --- | --- | --- |
| `+00h` | byte | `M+B4h`, the row-1 torque lock | `00C43CC7` | 0 (`009392C4`) |
| `+04h` | float mass | `M+50h` as `1/mass` | `00C43DA1`, no zero guard | `1.0f` (`009391CC`) |
| `+08h`..`+10h` | float3 inertia | `M+54h`..`+5Ch` as `1/x`, `0` when `x <= 0` | `00C43E01`..`00C43E0A` | `1.0f` each |
| `+14h`..`+43h` | 3x4 transform | `B+08h` and `M+84h` | `00C43E4D`, `00C43CDE` | identity |
| `+44h`..`+4Ch` | float3 | `M+00h`, linear velocity | `00C43CE6` | zero |
| `+50h`..`+58h` | float3 | `M+0Ch`, angular velocity | `00C43CFA` | zero |
| `+5Ch` | uint flags | `B+50h`; bit 0 makes the body static | `00C43CB8` | 0 (`009392CB`) |
| `+60h` | float | `M+B8h`, linear damping | `00C43D79` | 0 (`00939295`) |
| `+64h` | float | `M+BCh`, angular damping | `00C43D85` | 0 (`0093929E`) |
| `+68h` | float | `M+18h`, maximum linear speed | `00C43D1C` | `1000.0f` (`00CE3804`) |
| `+6Ch` | float | `M+1Ch`, maximum angular speed | `00C43D13` | `1000.0f`, same load |
| `+70h` | ptr | `B+68h` | `00C43E8D` | null |
| `+74h` | ptr | `B+6Ch` | `00C43E97` | null |
| `+78h`/`+7Ch`/`+80h` | `vector<shape*>` | attached one by one | `00C5D8C0` | empty |

`00C43CA0` also seeds `B+54h` with the immediate `14h` at `00C43E57`, the same value the
world carries at `world+44h` as the sleep countdown reload.

## What the hull body actually gets

| field | value | why |
| --- | --- | --- |
| mass | `class+B0h`, the Lua key `Mass` | `009399F7`; the reader is `00960230` at `0096043A`, default `1.0f` (`docs/VEHICLE_CLASS_FIELDS.md`) |
| inverse mass | `1/Mass` | `00C43DA1`; for the two destroyers below, `Mass` is `1800`, so `5.5556e-4` |
| inertia | the box rule below | `00939C05` after the body exists |
| linear damping | `0.0f` | the descriptor default survives; nothing overrides `+60h` |
| **angular damping** | **`1.0f`** | `00939A2F`, the float at `00D7A24C` |
| speed clamps | `1000.0f` each | the descriptor default at `009392D2`/`009392DB` |
| flags | `0` | never overridden: a hull body is always dynamic, never static |
| row-1 torque lock | `Mass < 100.0` | `00939A20`..`00939A57`, the double at `00D7A220` |

The angular damping is the one that shows. Each phase of a substep multiplies the angular
velocity by `1 - rate*dt` and both phases run, so a hull's commanded yaw rate is scaled by
`(1 - 0.05)^2 = 0.9025` inside the same step it is set in. The probe measures exactly that
(below).

## The inertia

`00939A89` calls `00C31F90` on the **new body**, which returns `B+38h` and `B+44h`: the AABB
the attached collision shapes produced. `00C43CA0` had zeroed both (`00C43E5E`..`00C43E77`),
so the span is entirely the shapes' doing. Then, in x87 order at `00939A8E..00939C05`:

```
extent = B[+44h] - B[+38h]                  ; 00939A8E..00939AE2, per axis
ex2, ey2, ez2 = extent^2                    ; 00939AF0..00939B20
sum_x = ey2 + ez2 ; sum_y = ex2 + ez2 ; sum_z = ex2 + ey2    ; 00939B2C..00939B79
k = (float)(Mass / 12.0)                    ; 00939B8F, the double 12.0 at 00CE42D0
I = (mul.x*k*sum_x, mul.y*k*sum_y, mul.z*k*sum_z)            ; 00939B99..00939BFD
00C37E70(body, &I)                          ; 00939C05
```

That is the solid-box inertia tensor, scaled per axis by `mul`, the physics material's
`NyomatekSzorzo`.

## The physics material

`00937CF1..00937D38` picks one of three records:

```
if (unit->vtable[5Ch](8))  material = 2      ; 00937D09
else                       material = (Mass >= 100.0) ? 0 : 1   ; 00937D30..00937D38
```

`EAX` at the compare is the class descriptor from `unit+538h` (`00937D19`) and the threshold
is the double `100.0` at `00D7A220`, the same constant that sets the row-1 torque lock.

The three records are `38h` bytes at `settings + 4E0h + material*38h`; `00939B3C`..`00939B53`
forms the same `base + index*38h` the loader does at `0083FFC3`..`0083FFCD`. Their producer
is `0083B5E0 BSP_GameSettings_LoadFromLuaGlobals`, whose three-iteration loop at
`0083FEE7..008403B7` reads `ShipGlobals["Physics"][name]` with the name selected by index at
`0083FEF2`:

| index | Lua name | string | `NyomatekSzorzo` in the installed `shipglobals.lua` | `Friction` |
| --- | --- | --- | --- | --- |
| 0 | `Ship` | `00CEB79C` | `{1, 1, 2}` | `0.5` |
| 1 | `TBoat` | `00D0A780` | `{1, 1, 2}` | `0.5` |
| 2 | `Submarine` | `00CEB7B0` | `{1, 1, 1}` | `1.0` |

`NyomatekSzorzo` (key string `00D0A668`, read by index 1..3 at `008401E1`, default `1.0f`
from the `FLD1` at `008401F7`) lands at record `+20h`, i.e. `settings+500h + material*38h`,
which is exactly what `00939BCB`/`00939BDB`/`00939BEC` read. `Friction` (`00D0A648`, default
`1.0f`) lands at record `+34h` and `00937C90` copies it onto each shape record at `+4h`
(`00939365`, `FLD float ptr [EBX + EDX*8 + 514h]`, in the shape-collection loop).

So a ship or torpedo boat gets twice the authored yaw inertia of a plain box, and a
submarine gets the plain box. Calling slot `5Ch` "is a submarine" is a hypothesis from the
record it selects; the callee's body was not read.

## The probe

`src/ship_motion_probe.cpp` now builds the body through `ship_hull_body_create_00937c90` and
runs the same trajectory twice, so the change is measured rather than asserted.
`--hull-extent DX DY DZ` supplies the collision AABB span; it defaults to zero, which gives a
zero inertia and hence a zero inverse inertia, i.e. no angular response to torque. The probe
pushes no torque, so that default changes nothing it measures.

400 steps of 0.05 s, full throttle, hard-over rudder, flat sea:

| | VehicleClass[11] A | VehicleClass[11] B | VehicleClass[23] A | VehicleClass[23] B |
| --- | --- | --- | --- | --- |
| x | -78.9317 | -70.8440 | -66.3204 | -59.8336 |
| y | 0.0000 | -2005.0000 | 0.0000 | -2005.0000 |
| z | 352.0871 | 348.7847 | 363.2104 | 359.9183 |
| heading, deg | -79.5344 | -75.5632 | -70.1653 | -66.6620 |
| final forward speed | 17.4030 | 17.4116 | 18.7036 | 18.7108 |
| peak forward speed | 17.4782 | 17.4796 | 18.7656 | 18.7667 |
| peak yaw rate, rad/s | 0.07854 | 0.07088 | 0.06981 | 0.06301 |

A is the old hand-built stand-in (no mass, no inertia, no damping, no gravity, clamps at
`1e30`); B is the class-built body in the world `004DDB90` constructs. Both classes are
`Mass = 1800`, `Length = 110`, `Height = 4`, so both take the `Ship` material.

The peak yaw rate is the result. A turns at exactly `MaxRotAngle / 2`, the shipped rudder
curve's target. B turns at `0.9025` times that, to five decimals, in both classes: the hull's
angular damping of `1.0f` applied twice per substep. The heading after 20 s is 4 degrees
short of A's for class 11 and 3.5 for class 23, and the along-track distance is 3.3 m shorter.

`y` falls 2005 m in 20 s because `00C41550` adds the world's gravity of `-10` and nothing in
the probe cancels it: `009329C0`'s buoyancy is not reconstructed. `0.5 * 10 * 20^2 = 2000`.

## Coverage

| routine | state | coverage |
| --- | --- | --- |
| `00C43CA0` | reconstructed from its full body | complete |
| `00937C90` tail `009399C0..00939C05` | reconstructed | partial: only the tail. `00937C90` is `00937C90..00939C8F` and the shape-collection loops at `00937D3F..009399BF` are read for the descriptor's vector only, not reconstructed |
| `00937CF1..00937D38` | the material selection reconstructed | complete for that block |
| `00939A8E..00939C05` | the inertia rule reconstructed | complete |
| `00C5D580` | read for the pools, the init call and the shape loop | partial: the free-list bookkeeping and the broad-phase links at `00C5D5A0..00C5D8B2` are described, not reconstructed |
| `0083B5E0` | read for the three physics records only | partial: the rest of the settings loader is untouched |
| `00C5C940` | not read | none; it is the producer of the AABB the inertia needs |

## Corrections

**To `docs/RIGID_BODY_INTEGRATION.md`, the `ship_hull_body_creation` follow-up row.** It
says "`00C37F40`/`00C37E70` have four callers between them and none of them is on the hull
path read here". `00C37E70` has a hull-path caller: `00937C90`, at `00939C05`. The row is
right that `00C37F40` (set mass) is never called for a hull body; the hull's mass reaches
`M+50h` through the descriptor instead, at `00C43DA1`.

**To `docs/RIGID_BODY_INTEGRATION.md`, "the hull body's creation: not found".** Found: the
tail of `00937C90`, store at `00939A86`.

**To `src/ship_motion_probe.cpp`'s own header.** It said the hull body's mass, inertia and
damping were open because "no ship-side caller of `00C37F40`, `00C37E70`, `00C37E00` or
`00C37DE0` was found". The damping never goes through `00C37E00`/`00C37DE0` for a hull body;
it is a descriptor field.

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `ship_hull_shapes` | `00937D3F..009399BF`, `00C5C940`, the `fizika_%02d` and `hajobelso` node walks | which model nodes become collision shapes, and therefore the AABB that decides the hull's inertia. The only remaining input to the inertia |
| `unit_type_query_5c` | the unit vtable slot `5Ch` | what category id 8 means; this packet names it from the physics record it selects |
| `ship_physics_material_record` | `settings+4E0h + i*38h`, `0083FEE7..008403B7` | the six fields of the record whose keys this packet did not read, and the readers of `KozegellenallasiEgyutthato*` |

## no_ghidra_function

none. Every address named or reconstructed in this packet lies inside an existing Ghidra
function body, checked with `python tools/bsp.py ghidra proto <addr> --brief`.
