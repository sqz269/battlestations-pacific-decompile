# The substep's contact phase: groups, the LCP task split, and the contact report

Addresses: 00C5BB5F..00C5C455, 00C4B610, 00C4B550, 00C3F410, 00C36AC0, 00C36B60, 00C33140,
00C5C7A0, 00C5C710, 00D7A088, 00D7A090; read as contracts 00C57070, 00403720, 00403850,
00C41AD0.

Packet `cc_dyn_step`, 2026-09-11. Reconstructed in `include/bsp/dyn_contact_solver.hpp` and
`src/dyn_contact_solver.cpp`; semantic C++ interfaces for MSVC Win32, not drop-in binary
replacements. Descriptive names are hypotheses, not recovered symbols. The saved project is
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. This worker made no Ghidra
mutation; the ledger records the new names.

This is the `dyn_contact_solver` follow-up `docs/RIGID_BODY_INTEGRATION.md` opened. The
schedule around it is `docs/DYN_PHYSICS_SUBSTEP.md`.

## What the phase is made of

Four names in this block are the image's own, not hypotheses. The RTTI TypeDescriptors
`00E17394` and `00E1736C` spell `Dyn::Scene::LCPSolverTask` (vtable `00D7A088`, slot 0
`00403720`) and `Dyn::Scene::LCPSolver2Task` (vtable `00D7A090`, slot 0 `00403850`), and the
profiler labels are image strings: `CreateGroups`, `Solve`, `SolverPreStep`,
`SolveConstraints`, `SleepGroups` here, and `BroadPhase`, `BroadPhaseUpdate`,
`IntersectLoop`, `GetManifold`, `ManifoldUpdate`, `Collide` inside the collision pass.

## The records the phase reads

The collision pass writes a list of manifolds into the scene at `world+444h`. The record
layout below comes from the record builder at `00C5C13D..00C5C42D` and the group phases,
which are the only readers this packet read; the **producer** is inside `00C57070`, which it
did not read, so the meaning of one field is labelled provisional below.

| manifold | field |
| --- | --- |
| `+08h + i*30h` | contact point `i`, `+00h` normal, `+0Ch` local point on body A, `+18h` local point on body B, `+24h` a scalar |
| `+0C8h` | int contact point count |
| `+0CCh` | body A |
| `+0D0h` | body B |
| `+0D4h` | int16 group mark, `-1` when unassigned |
| `+0DCh` | next manifold |

| scene container (`[world+444h]+0B0h`) | field |
| --- | --- |
| `+0ECh` | list head |
| `+0F0h` | list sentinel |
| `+1D0h` | manifold count |

| body | field |
| --- | --- |
| `+64h` | int16 visit mark, `-1` when unvisited |
| `+74h` | array of manifold pointers this body is in |
| `+78h` | its count |

## `00C4B610`, group formation

Called with `world+448h`. Clears the group vector through `00C3F410`, resets every
`B+64h` and every `manifold+0D4h` to `-1` through `00C36AC0`, then flood-fills the manifold
graph into islands.

```
for (m = head; m != sentinel; m = m->next)
    if (m->points != 0 && m->mark == -1 &&
        ((A->flags & 3) == 0 || (B->flags & 3) == 0)) {
        open a new group; push m; m->mark = 0;
        while (stack) {
            n = pop; append n to the group (00C36B60);
            for (body in {n->A, n->B})
                if ((body->flags & 1) == 0 && body->mark == -1) {
                    body->mark = 0;
                    for (c in body->contacts[0 .. body->contactCount))
                        if (c && c->points != 0 && c->mark == -1) {
                            push c; c->mark = 0;
                            c->A->flags &= ~12h; c->B->flags &= ~12h;
                        }
                }
        }
    }
```

Three things follow from the predicates. A group is seeded only where at least one body is
neither static nor asleep, so a settled pile forms no group and costs nothing. The fill does
not expand **through** a static body (`flags & 1`), so the world geometry does not merge
every island touching it into one. And every manifold the fill reaches wakes both of its
bodies with the same `AND 0FFFFFFEDh` every body mutator uses, so a sleeping body dragged
into a live island is integrated again from the next substep.

`world+450h` (the vector's size) and `world+458h` (the group count) are kept equal, and
`00C3F410` zeroes both.

## `00C4B550`, group sleep

Called with `world+448h` under the `SleepGroups` scope, last in the substep. A group whose
every contact has **both** bodies with `B+50h & 3` set (static or asleep) gets `B+50h` bit 4
set on both bodies of every contact, which is the bit both integration phases test first.
Then the whole group vector is cleared through the same `00C3F410`, so the groups are
per-substep state.

The predicate never fires under the shipped world's sleep settings on its own: both sleep
speed thresholds are `0.0f` (`docs/DYN_WORLD_SETTINGS.md`), so only an exactly stationary
body ever counts down to bit 1. Static bodies pass it for free, so a group made entirely of
static bodies would sleep; the fill above does not create one.

## The solver split

`world+10h` selects the task array. `00C5BBF2` reads it; `0` (the shipped value, `004DE19F`
stores `EBX = 0` into `desc+20h` and `00C41B06` copies it) selects the `LCPSolverTask` array,
`1` the `LCPSolver2Task` array, and anything else jumps straight to the `UpdatePosition`
phase with no solve and no contact report.

| mode | task array | count | pointer vector | vtable | class |
| --- | --- | --- | --- | --- | --- |
| 0 | `world+45Ch` | `world+460h` | `world+468h` | `00D7A088` | `Dyn::Scene::LCPSolverTask` |
| 1 | `world+474h` | `world+478h` | `world+480h` | `00D7A090` | `Dyn::Scene::LCPSolver2Task` |

Both arrays are built by `Dyn_World_Construct` and sized to `[[[0109E9FC]+10h]+4h]`, the task
scheduler's worker count (`00C41C32`). A task is `18h` bytes: `+0h` vtable, `+4h` batch
index (written by `00C33140`), `+8h` the world, `+0Ch` the first group, `+10h` the last group
inclusive, `+14h` the substep dt.

The partition is one signed integer division done once (`CDQ`/`IDIV` at `00C5BC4D`):

```
task_count = min(group_count, capacity)
per_task   = group_count / task_count
tasks 0 .. task_count-2 : [start, start + per_task - 1], start += per_task
task_count-1            : [start, group_count - 1]
```

so the last task absorbs the remainder. `00C33140` then submits the whole batch under the
scheduler's critical section, releases its semaphore by the count and blocks on the batch's
event, which makes it a fork-join: the substep is single-threaded again on return.

Each task runs, per group in its range, `00C5C7A0` (`SolverPreStep`) then `00C5C710`
(`SolveConstraints`). `00C5C710` runs `[[context+0h]+38h]` iterations of `00C431D0` then
`00C42ED0`, and `context+0h` is the world, so the iteration count is `world+38h` = 10.
**The per-constraint impulse math inside those iterations is not reconstructed.**

## The contact report

`world+24h` is a listener object pointer, zero from the constructor (`00C41C2C`). When it is
non-zero the substep allocates `manifoldCount * 4` records of `58h` bytes (`00C5C149`,
`00C5C16D`), fills one per contact point across every manifold, calls slot 0 of the
listener's vtable as `__thiscall(listener, records, count)` (`RET 8`, no cleanup at
`00C5C43A`), and frees the array at `00C5C449`. The listener must not retain it.

Per point, in the listing's order and float32 throughout (x87 at the game's 24-bit
precision):

| record | value |
| --- | --- |
| `+00h` | body A's 3x4 transform applied to the point's local point on A |
| `+0Ch` | body B's transform applied to its local point on B |
| `+18h` | the normal, copied unchanged |
| `+24h` | `normal * s` |
| `+30h` | `normal * (-s)` |
| `+3Ch`, `+48h` | zeroed |
| `+54h` | never written |

The transform is `p' = px*row0 + py*row1 + pz*row2 + t` with the rows at `body+08h`, `+14h`,
`+20h` and `t` at `body+2Ch`, the same layout `bsp/rigid_body_integration.hpp` declares for
`B`. That the two manifold pointers dereference to that layout is what identifies
`manifold+0CCh` and `manifold+0D0h` as bodies.

`s` (`point+24h`) is **provisional**: its producer is in the narrow phase this packet did not
read, so it is named for what the builder does with it (equal and opposite triples along the
normal). Penetration depth and accumulated normal impulse both fit; this packet does not
choose. The four-records-per-manifold allocation is the only statement anywhere in this
routine about how many points a manifold may hold, and nothing checks it.

## Corrections

**To `docs/RIGID_BODY_INTEGRATION.md`, the `dyn_contact_solver` follow-up row.** It expected
"the contact callback through `world+24h`" to be a field the world descriptor supplies. It is
not: see the descriptor correction in `docs/DYN_PHYSICS_SUBSTEP.md`. `world+24h` is zero at
construction and this packet found no writer.

**To `docs/RIGID_BODY_INTEGRATION.md`, "The solver being an LCP solver also says what the
collision and constraint phases this packet does not read are."** Two solver task classes
exist, not one, and the shipped world selects `Dyn::Scene::LCPSolverTask` (`00D7A088`), not
the `LCPSolver2Task` (`00D7A090`) whose RTTI that doc quoted. `LCPSolver2Task` is reachable
only with `world+10h == 1`, which `004DDB90` never authors.

**From packet `cc_dyn_constraints`, to "The solver split".** "Each task runs, per group in its
range, `00C5C7A0` (`SolverPreStep`) then `00C5C710` (`SolveConstraints`)" is true of the
`LCPSolver2Task` only. Both routines have exactly one caller and it is
`Dyn_Scene_LCPSolver2Task_vslot0` (`00403850`), which `world+10h == 1` selects and the shipped
world never does. The shipped `LCPSolverTask` body `00403784..004037DB` runs `00C4F040`, then
`00C42530` and `00C42230` `world+38h` times, then `00C37B50` and `00C35020`. The row build is
`00C4DE40` and the row block is allocated by `00C31C30`. See `docs/DYN_LCP_IMPULSE_MATH.md`.

**From packet `cc_dyn_constraints`, to "The contact report" and to `DynContactPoint`.** `s`
(`point+24h`) is the accumulated normal impulse, not the penetration depth. `00C4E0B2` seeds the
solver row's impulse accumulator from it (scaled by `world+20h`), `00C35020` writes the solved
impulse back to it, and `00C3F9BD` zeroes it only when the narrow phase appends a brand-new
point. The record is `30h` bytes with six fields: `+28h` is the accumulated bias impulse and
`+2Ch` the penetration depth. The manifold also has `+00h`, the combined friction coefficient,
and `+04h`, the combined restitution, both written by `00C44090`. See
`docs/DYN_COLLISION_PASS.md`. `include/bsp/dyn_contact_solver.hpp`'s four-field `DynContactPoint`
is not edited by that packet; the whole record is `DynSolverContactPoint` in
`include/bsp/dyn_lcp_impulse_math.hpp`.

**From packet `cc_dyn_constraints`, to "The four-records-per-manifold allocation is the only
statement anywhere in this routine about how many points a manifold may hold, and nothing checks
it."** `00C3F943` checks it: a fifth candidate point takes the reduction branch instead of being
appended, so a manifold never holds more than four.

## Coverage

| routine | state | coverage |
| --- | --- | --- |
| `00C4B610` | reconstructed as a sequence over `DynGroupFormationHost`, build-tested | complete for the fill; the vector growth and allocation are host-side |
| `00C4B550` | reconstructed as a sequence over `DynContactGroupHost`, build-tested | complete |
| `00C3F410`, `00C36AC0`, `00C36B60` | read in full, named, contracts in the hosts | complete |
| the task partition `00C5BC2B..00C5BCB9` / `00C5C04E..00C5C0D9` | reconstructed as a pure rule | complete |
| the record builder `00C5C1BE..00C5C40C` | reconstructed as a pure rule | complete |
| `00C33140` | read in full, named, contract in the host | complete |
| `00C5C7A0` | read for its sequence and the row array it sizes | partial: `00C35160`, `00C4F140` (`00C4F140..00C5030F`) and `00C437D0` (`00C437D0..00C43A91`) are not read |
| `00C5C710` | read for its sequence and the iteration count | partial: `00C431D0` (`00C431D0..00C437CA`), `00C42ED0` (`00C42ED0..00C431C2`), `00C37C40` and `00C350C0` are not read |
| `00403720`, `00403850` | read for the per-group loop they run | partial: `00C4F040`, `00C42530`, `00C42230`, `00C37B50`, `00C35020` are not read |
| `00C57070` | call site, argument and strings only | none of the body; contract in the substep host |

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `dyn_lcp_impulse_math` | `00C431D0`, `00C42ED0`, `00C4F140`, `00C437D0`, `00C37C40`, `00C350C0`, `00C35020` | the per-constraint impulse math the ten iterations run, which is the last thing between this reconstruction and a contact response that matches the game |
| `dyn_collision_pass` | `00C57070`, `Dyn_SAPRadixBroadPhaseManager_vslot3`, `00C35480`, `00C549D0` | the producer of the manifolds and of `point+24h`, which this doc leaves provisional |
| `dyn_world_solver_settings` | `world+14h`, `+18h`, `+1Ch`, `+20h`, `+28h` | the five remaining carried descriptor fields. `world+10h` (the mode) and `world+38h` (the iteration count) are settled here, so that follow-up from `docs/DYN_WORLD_SETTINGS.md` is two fields smaller |
| `dyn_contact_listener` | the writer of `world+24h` | whether the shipped game ever builds a contact report |

## no_ghidra_function

| start | inclusive end | evidence |
| --- | --- | --- |
| none | | Every address named or reconstructed in this document lies inside an existing Ghidra function body, checked with `python tools/bsp.py ghidra proto <addr> --brief`. |
