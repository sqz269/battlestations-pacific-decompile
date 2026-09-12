# The scene's collision pass: the broad phase, the narrow phase and the manifold list

Addresses: 00C57070, 00C549D0, 00C35480, 00403CC0, 00C44090, 00C3F760, 00C3F650, 00C38266;
read as contracts 00C4B9B0, 00C37D30, 00C3F4D0, 00C50390, 00C33140, and the broad-phase
manager's vslots 3 to 6 at `[scene+0ACh]`.

Packet `cc_dyn_constraints`, 2026-09-11. Reconstructed in `include/bsp/dyn_collision_pass.hpp`
and `src/dyn_collision_pass.cpp`; semantic C++ interfaces for MSVC Win32, not drop-in binary
replacements. Descriptive names are hypotheses, not recovered symbols. The saved project is
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. This worker made no Ghidra
mutation; the ledger records the new names.

This is the `dyn_collision_pass` follow-up `docs/DYN_CONTACT_SOLVER.md` opened: the producer of
the manifold list and of `point+24h`, which that doc left provisional. The consumer is
`docs/DYN_LCP_IMPULSE_MATH.md`.

## The names that are the image's own

`Dyn::CollisionSystem::IntersectTask2` (vtable `00D7A110`, TypeDescriptor `00E17244`) is the
narrow-phase task, and `Dyn::Task` (vtable `00D7A080`) its base; the scene constructor
`00C38070` stores both (`00C38266`, `00C38273`). The profiler labels `Collide`, `BroadPhase`,
`BroadPhaseUpdate`, `ManifoldUpdate`, `IntersectLoop` and `GetManifold` are image strings.

## `00C57070`, the pass

`void(scene)` with the scene pushed, body `00C57070-00C5787C`, sole caller `00C5BB30` at
`00C5BB66` with `scene = [world+444h]`. Six steps, each but the first and last inside its own
profiler scope:

| step | site | what |
| --- | --- | --- |
| 1 `BroadPhase` | `00C5712A..00C5747A` | refresh every eligible body's world AABB into its proxy |
| 2 `BroadPhaseUpdate` | `00C574C2` | `[[scene+0ACh]+0Ch]()`, the manager's vslot 3 |
| 3 `ManifoldUpdate` | `00C5756C` | `00C549D0([scene+0B0h])` |
| 4 | `00C5759C` | `[[scene+0ACh]+10h]()`, the pair count; `00C575AF` then clears `scene+0DCh` |
| 5 `IntersectLoop` | `00C575AF..00C577F3` | collect pairs, partition them, fork-join the narrow phase |
| 6 | `00C57827` | `00C35480(scene)`, the contact notifications |

Step 6 is outside the pair-count guard, so a frame with no pairs still runs it against the
already-cleared event array.

### Step 1, the world AABB

Walks `[scene+0A8h]+204h` to the sentinel `+208h`, linked by `B+84h`. A body is refreshed only
when `B+50h` bit 3 is set (`00C5714C`) and bit 4 is clear (`00C57154`), so a sleeping body keeps
last frame's proxy. Per body, with the local box at `B+38h`/`B+44h` and the proxy at `B+60h`:

```
centre = (min + max) * 0.5                 // the double 0.5 at 00D7A280
half   = (max - min) * 0.5
worldCentre = B.transform * centre          // the 3x4 at B+08h/+14h/+20h, origin B+2Ch
worldHalf   = |B.rotation| * half           // nine BSP_Math_AbsFloat calls
proxy+04h = worldCentre - worldHalf         // 00C57404..00C5745E
proxy+10h = worldCentre + worldHalf
```

That is the standard conservative rotation of a box, which is why a rotating hull's proxy grows
and shrinks with its heading.

### Step 5, the pairs and the narrow-phase partition

`[[scene+0ACh]+14h]()` returns the first pair and `[[scene+0ACh]+18h](pair)` the next. A pair's
first two fields point at the two bodies. A pair survives only when
`(flagsA & flagsB & 10h) == 0` (`00C576E0..00C57705`): two sleeping bodies are dropped, any other
combination kept. Survivors are compacted into `scene+0B4h` (grown to the raw pair count first).

The surviving count is then partitioned across `IntersectTask2` records of `14h` bytes at
`scene+0C0h`, capacity `scene+0C4h`, writing `+0Ch` and `+10h` as the inclusive pair range. The
rule is the same integer division the solver split uses, with the last task absorbing the
remainder, so `bsp/dyn_contact_solver.hpp`'s `dyn_solver_task_count` and `dyn_solver_task_range`
apply unchanged. `00C33140` then forks and joins the batch.

## `00C549D0`, `ManifoldUpdate`

Takes the manifold container at `scene+0B0h`. Two passes over the doubly-linked list (head
`+0ECh`, sentinel `+0F0h`, next `manifold+0DCh`, previous `manifold+0D8h`):

1. every manifold whose two bodies are not both asleep goes through `00C4B9B0`, which refreshes
   its surviving points. This packet read the call site and the sleep test, not `00C4B9B0`'s body.
2. every manifold left with zero points (`manifold+0C8h`) is unlinked, its count at `+1D0h`
   decremented and the node pushed onto the container's free list at `+0Ch`.

Because this runs **before** the pair set is read, a manifold whose points all separated this
frame is already retired by the time the narrow phase asks for one.

## `00C44090`, the narrow phase

Reached through `00403CC0`, `IntersectTask2` vslot 0, which passes the scene in `ESI`, the first
pair index in `EAX` and the last on the stack. Per pair, per shape of body A (list at `B+70h`,
next at `+208h`) against every shape of body B:

```
if ((maskB & groupA) || (maskA & groupB))                      // 00C44104..00C44110
    fn = scene->dispatch[ typeA * 6 + typeB ]                  // 00C44118..00C44128
    if (fn && fn->vslot0(&result, shapeA, bodyA+8h, shapeB, bodyB+8h))
        friction    = combine(shapeA+28h, shapeB+28h)          // 00C44154..00C4418B
        restitution = (shapeB+24h + shapeA+24h) * 0.5          // 00C4419D..00C441AF
        manifold = 00C3F4D0(bodyA, bodyB)                      // find or create for the pair
        manifold[+00h] = friction;  manifold[+04h] = restitution
        for each of result[0] candidate points: 00C3F760(manifold, candidate)
        if either body has a callback at B+68h selecting the other's group,
            append {manifold, shapeA, shapeB} to scene+0D8h under the spin lock at scene+0E4h
```

The friction combine is a product with an override: a negative value on either shape wins as its
own magnitude, otherwise the two are multiplied. The dispatch table is six by six, so the engine
has six shape kinds; which six is not settled here.

A candidate point is nine floats: local point on A, local point on B, normal.

## `00C3F760`, the contact point insert, and `point+24h`

This is the producer `docs/DYN_CONTACT_SOLVER.md` did not read, and it settles that doc's
provisional field.

```
if (|1 - |n|^2| > 0.05) return                     // 00C3F767..00C3F790, the double at 00D7A270
k = 00C3F650(manifold, candidate)                   // the first matching existing point
if (k < manifold[+0C8h]) {
    point[k].localA = candidate.localA              // the geometry is replaced
    point[k].localB = candidate.localB
    point[k].normal = candidate.normal
    point[k].depth  = n . (worldA - worldB)         // 00C3F93C
    // +24h and +28h are NOT touched
} else if (manifold[+0C8h] < 4) {                   // 00C3F943
    ... the same three writes ...
    point[+24h] = 0                                 // 00C3F9BD
    point[+28h] = 0                                 // 00C3F9D2
    point[+2Ch] = n . (worldA - worldB)
    ++manifold[+0C8h]
} else {
    the four-point reduction at 00C3FA46..00C3FFD5, not read
}
```

`00C3F650` returns the first existing point whose local point on **either** body is within
`0.0025` squared of the candidate's, that is within `0.05` units, and the point count otherwise.
A match keeps that point's `+24h` and `+28h`. Together with `00C4E0B2` (the pre-step reads them)
and `00C35020` (the solve writes them back), that is the whole of the warm start's frame-to-frame
persistence, and it is what identifies the two fields as accumulated impulses rather than
geometry.

The `30h`-byte contact point is therefore six fields:

| point | field |
| --- | --- |
| `+00h` | the normal, unit within `0.05` in the squared length, from B towards A |
| `+0Ch` | the local point on body A |
| `+18h` | the local point on body B |
| `+24h` | the accumulated normal impulse, zeroed only when the point is new |
| `+28h` | the accumulated bias (penetration-correction) impulse, likewise |
| `+2Ch` | the penetration depth, `n . (worldA - worldB)`, positive while they overlap |

and the manifold carries `+00h` the combined friction coefficient and `+04h` the combined
restitution, both rewritten by the narrow phase every frame it finds the pair intersecting.

## `00C35480`, the contact notifications, and who the contact listener is

`scene+0D8h` is an array of `0Ch`-byte events `{manifold, shapeA, shapeB}` and `scene+0DCh` its
count. Per event, `00C35480` transforms each of the manifold's points into world space, then:

```
if ([manifold+0CCh]+68h) (**[manifold+0CCh]+68h])(&{shapeA, shapeB})     // 00C35551, 00C35574
if ([manifold+0D0h]+68h) (**[manifold+0D0h]+68h])(&{shapeB, shapeA})     // the pair swapped
```

so the shipped game's contact notification is **per body, at `B+68h`**, dispatched here, and not
the `world+24h` listener of `docs/DYN_CONTACT_SOLVER.md`. Those are two separate mechanisms:
`world+24h` is zero from `00C41C2C` with no writer found by this packet or by `cc_dyn_step`,
while `B+68h` is filtered by a group mask at `listener+4h` (`00C4420D`/`00C44214` in
`00C44090`) and is the one the narrow phase populates events for. The `dyn_contact_listener`
follow-up should look for the writer of `B+68h`, not only of `world+24h`.

The world-space point transform at `00C354E6..00C35542` writes into a stack array and nothing
afterwards reads it; the decompiler also shows a per-event read of `manifold + eventIndex*30h`
whose result is dead. Both are recorded as unexplained rather than modelled.

## Corrections

**To `docs/DYN_CONTACT_SOLVER.md`, "The records the phase reads", the manifold table and the
`s` (`point+24h`) paragraph.** `point+24h` is the accumulated normal impulse, not the penetration
depth; the record has two more fields (`+28h` accumulated bias impulse, `+2Ch` penetration
depth); and the manifold has two fields before the point array, `+00h` the combined friction
coefficient and `+04h` the combined restitution. Evidence: `00C3F760` in full, plus `00C4E0B2`,
`00C4E0C6`, `00C4EFA8` and `00C4E679` reading them in the pre-step.

**To `docs/DYN_CONTACT_SOLVER.md`, "The four-records-per-manifold allocation is the only
statement anywhere in this routine about how many points a manifold may hold, and nothing checks
it."** Something does check it: `00C3F943` compares the point count with 4 and a fifth candidate
takes the reduction branch instead of being appended. The allocation is not a guess.

**To `docs/DYN_PHYSICS_SUBSTEP.md`, the `dyn_contact_listener` follow-up row.** "The shipped
game may never build a contact report at all" is right about `world+24h` and wrong as a statement
about contact notification: `00C35480` dispatches per-body callbacks from `B+68h` every frame the
narrow phase produces an event.

## Coverage

| routine | state | coverage |
| --- | --- | --- |
| `00C57070` | reconstructed as a sequence over `DynCollisionPassHost`, build-tested | complete for the six steps and the partition; the profiler scope bookkeeping and the exception frame are not projected |
| the AABB refresh `00C5715C..00C5745E` | reconstructed as a pure rule | complete |
| the pair filter and the partition | reconstructed as pure rules | complete |
| `00C549D0` | read in full, contract in the host | complete for the two passes; `00C4B9B0` and `00C37D30` are contracts only |
| `00C35480` | read in full | partial: the two callbacks and the event record are modelled in the doc, not in code; the dead world-point transform is not projected |
| `00C44090` | read for the dispatch, the combines and the event append | partial: the shape-pair dispatch targets, `00C3F4D0` and the six shape kinds are not read |
| `00C3F760` | reconstructed as `dyn_insert_contact_point_00c3f760`, build-tested | partial: the four-point reduction `00C3FA46..00C3FFD5` is not read and is reported rather than performed |
| `00C3F650` | reconstructed as a pure rule | complete |
| `00403CC0` | read in full, the register contract is in the doc | complete |
| `[scene+0ACh]` vslots 3 to 6 | call sites, arguments and return contracts only | none of the SAP radix manager's bodies |

## Run-time evidence

Not on a path `bsp_game.exe` or the probes reach. `src/ship_motion_probe.cpp` drives one hull in
open water and supplies no collision geometry (its banner names `00C5C940`, the hull AABB
producer, as unread), so the broad phase would produce no pairs and this pass no manifolds. The
probe is unchanged by this packet and no new probe was added.

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `dyn_broad_phase_sap_radix` | `[scene+0ACh]` vslots 3 to 6 | the manager itself, the only remaining contract in step 5 |
| `dyn_shape_dispatch_table` | `00C44090`'s table, `00C3F4D0`, the six shape kinds | which pair of shapes each of the 36 entries tests, and how a manifold is found or created for a body pair |
| `dyn_manifold_point_reduction` | `00C3FA46..00C3FFD5` | which four of five candidate points a full manifold keeps |
| `dyn_manifold_refresh` | `00C4B9B0`, `00C37D30` | how an existing manifold's points are re-projected and dropped between frames |
| `dyn_contact_listener` | the writer of `B+68h`, `listener+4h` | who installs a per-body contact callback, which is the mechanism the shipped game actually uses |

## no_ghidra_function

| start | inclusive end | evidence |
| --- | --- | --- |
| none | | Every address named or reconstructed in this document lies inside an existing Ghidra function body, checked with `python tools/bsp.py ghidra proto <addr> --brief` for `00C57070`, `00C549D0`, `00C35480`, `00403CC0`, `00C44090`, `00C3F760`, `00C3F650` and `00C38070`. |
