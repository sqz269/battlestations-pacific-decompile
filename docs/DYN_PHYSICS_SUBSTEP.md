# The Dynamics world's step in full: the schedule and the substep body

Addresses: 00C5C540, 00C5BB30, 00C321B0, 00C4D980; read as contracts 00C41550, 00C5B1B0,
00C50390, 00C57020, 00C57070, 00C4B610, 00C4B550, 00C33140, 00875E0C.

Packet `cc_dyn_step`, 2026-09-11. Reconstructed in `include/bsp/dyn_physics_substep.hpp`
and `src/dyn_physics_substep.cpp`; semantic C++ interfaces for MSVC Win32, not drop-in
binary replacements. Descriptive names are hypotheses, not recovered symbols. The saved
project is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. This worker
made no Ghidra mutation; the ledger records the new names.

`docs/RIGID_BODY_INTEGRATION.md` left `00C5C540` as "partial: the proxy transform copy and
the profiler bookkeeping are described, not reconstructed" and `00C5BB30` as "read for its
phase order and call sites only". This packet closes both. The contact phase's own rules
are `docs/DYN_CONTACT_SOLVER.md`; the callback list of the same fixed step is
`docs/FIXED_STEP_CALLBACKS.md`.

## `00C5C540`, the schedule, whole

`__thiscall void(world, float)`, `RET 4` at `00C5C704`, body `00C5C540..00C5C706`. Sole
caller `00875E0C` with `ECX = [[00E188A8]+18h]` and the fixed step `0.05f`. Seven things in
listing order:

```
00C5C55E  reset the profiler counter tree: 00C321B0 over every child of [[0109E9FC]+0Ch],
          then the root's own +38h/+3Ch/+40h, then [[0109E9FC]+0Ch]+8h += 1
00C5C5AE  push the "Simulate" scope (00C50390("Simulate", 1) at 00C5C5C5 on first use),
          rdtsc
00C5C5F5  world+2Ch += 1                                  ; ADD dword, an int, not a float
00C5C5FF  00C4D980(world): flush the pending body removals
00C5C604  for (B = [world+204h]; B != world+208h; B = [B+84h])
              REP MOVSD 0Ch dwords from B+08h to [B+04h]+84h
00C5C639  the accumulator loop, bounded by the budget, then world+48h = 0
00C5C6CE  rdtsc, pop the scope
```

Three of those were not previously written down.

**The counter reset is the profiler's, not an allocator's.** `00C321B0` recurses over a
tree (children at node+4h, count at node+8h) zeroing `+38h`, `+3Ch` and `+40h`. Those three
dwords are exactly what a scope's pop writes: the tail of this same function adds the rdtsc
delta into `record+38h/+3Ch` and increments `record+40h` (`00C5C6D6..00C5C6E7`). So the
Dynamics world step is where the profiler's per-frame totals are cleared for the whole
process, and `[[0109E9FC]+0Ch]+8h` is the frame counter.

**`world+2Ch` counts calls to `Simulate`, not substeps.** It is incremented before the
budget is even read, so a step whose budget is exhausted still counts.

**The proxy transform copy is a twelve-dword block move with no flag test.** It publishes
`B+08h..+37h` into `[B+04h]+84h`, which `docs/RIGID_BODY_INTEGRATION.md` records as the
previous 3x4 transform and `00C43EA0` interpolates against. A static or sleeping body's
previous transform is refreshed too, so a renderer lerping against it sees no jump when a
body falls asleep.

`00C4D980` is a removal flush rather than a step pass: its other caller is `004DA780`,
`BSP_Game_TeardownSessionState`. Per queued body it releases the proxy handle at `B+60h`
through slot 1 of the vtable of `[[B+00h]+444h]+0ACh` (so `B+00h` is the owning world),
clears `B+50h` bit 3, destroys every node of the contact list at `B+70h` through that node's
own vtable slot 1, unlinks the body from its list (`B+80h` prev, `B+84h` next) and pushes it
onto the pool at `world+4Ch` when `B+50h` bit 0 is set and `world+170h` otherwise.

The accumulator loop itself is unchanged from `docs/DYN_WORLD_SETTINGS.md`, whose
`dyn_world_substep_plan` this reconstruction calls rather than restating.

## `00C5BB30`, the substep body

`__cdecl void(world, float dt)`, body `00C5BB30..00C5C53E`, call sites `00C5C66D` and
`00C5C6BD`. Nine calls, in listing order:

| # | site | callee | `this` / args | what |
| --- | --- | --- | --- | --- |
| 1 | `00C5BB5A` | `00C41550` | world in ESI, dt pushed | the velocity phase |
| 2 | `00C5BB66` | `00C57070` | `[world+444h]` pushed | the collision pass |
| 3 | `00C5BB83` | `00C50390` | `"CreateGroups"`, 16h | the scope record, first use only |
| 4 | `00C5BBBE` | `00C4B610` | `world+448h` pushed | group formation |
| 5 | `00C5BC1E` / `00C5C045` | `00C57020` | `"Solve"` pushed, `EAX = 18h` | the Solve scope |
| 6 | `00C5BCCA` / `00C5C0E8` | `00C33140` | tasks in EAX, scheduler in ECX, count pushed | the solver fork-join |
| 7 | `00C5C43A` | `[[world+24h]]` slot 0 | records, count; `RET 8` | the contact report |
| 8 | `00C5C491` | `00C5B1B0` | world in EBX, dt pushed | the position phase |
| 9 | `00C5C503` | `00C4B550` | `world+448h` pushed | group sleep |

The velocity phase runs **before** the collision pass, so the narrow phase sees this
substep's velocities. The group sleep runs **last**, so a group that sleeps here is excluded
from the next substep's integration and not from this one's.

`world+448h` is not a separate object: `00C41B99` stores the world pointer into it, so the
contact-group manager is a sub-object embedded at `world+448h` whose `+0h` is the owning
world, `+4h` the group vector's data (`world+44Ch`), `+8h` its size (`world+450h`), `+0Ch`
its capacity (`world+454h`) and `+10h` the group count (`world+458h`).

`world+444h` is the collision scene: `00C41BF2` allocates `0E8h` bytes for it and `00C41C09`
constructs it with the world.

## Corrections

**To `docs/DYN_WORLD_SETTINGS.md`, the descriptor table, row `desc+38h`.** It records
`desc+38h` (`0.02f`, `00CE746C`, stored at `004DE15A`) as copied into `world+24h`. There is
no such copy. `Dyn_World_Construct`'s fifteen copies at `00C41AE9..00C41B4A` read the
descriptor at `+00h`, `+04h`, `+08h`, `+0Ch`, `+10h`, `+14h`, `+18h`, `+1Ch`, `+20h`,
`+24h`, `+28h`, `+2Ch`, `+30h`, `+34h` and `+3Ch`; `desc+38h` is **not among them**, and
`desc+3Ch` (`0.5f`) is the one that lands in `world+28h` (`00C41B25`/`00C41B28`). `world+24h`
is instead written once, with zero, at `00C41C2C` (`MOV [ESI+24h],EBP` with `EBP` zeroed at
`00C41B58`), and its only reader is the contact-report dispatch at `00C5C124`/`00C5C431`,
which treats it as an object pointer and calls vtable slot 0 through it. The `0.02f` the
game constructor authors at `004DE15A` therefore reaches no world field this packet found.

**To `docs/DYN_WORLD_SETTINGS.md`, `kDynWorldSolverWord24 = 10` "the shape of an iteration
count, but that is a guess".** It is an iteration count. `desc+24h` lands in `world+38h`
(`00C41B31`/`00C41B34`), and `00C5C71F` inside `Dyn_LCPSolver_SolveGroupConstraints` reads
`[[context+0h]+38h]` as the loop bound over `00C431D0`/`00C42ED0`, with `context+0h` the
world. `Dyn_Scene_LCPSolverTask_vslot0` reads the same field. Ten iterations per group per
substep.

**To `docs/RIGID_BODY_INTEGRATION.md`, the coverage row for `00C5C540`.** No longer partial:
the proxy transform copy and the profiler bookkeeping are reconstructed here.

**Ghidra's listing of `00C5BB30` is truncated after `_free`.** `00C5C449` calls
`00BF6989` (`_free`), which Ghidra's non-returning discovery marked `CALL_RETURN`, so the
three bytes at `00C5C44E..00C5C450` (`ADD ESP,4`) are undisassembled and the decompiler emits
a `return;` there. The disk bytes are `83 c4 04` followed by the fall-through to `00C5C451`,
so the substep continues into the `UpdatePosition` phase; the decompiled early return is an
artifact. `python tools/bsp.py ghidra flow 00c5bb30` reports the gap. The integrator should
run `python tools/ghidra_flow_repair.py 00c5bb30 --apply` before anyone reads this function
again. `00C4B610` has the same pattern at four `_free` sites (`00C4B787`, `00C4B7AB`,
`00C4B816`, `00C4B992`), which is why its decompiled listing shows a `return;` in the middle
of the vector growth; the reconstruction here follows the disk bytes, not that listing.

## Coverage

| routine | state | coverage |
| --- | --- | --- |
| `00C5C540` | reconstructed as a sequence over `DynSimulateHost`, build-tested, probe-exercised | complete |
| the previous-transform copy, `00C5C614..00C5C62F` | reconstructed as a pure rule | complete |
| `00C5BB30` | reconstructed as a sequence over `DynSubstepHost`, build-tested | partial: the per-constraint impulse math under the solver tasks is not reconstructed; see `docs/DYN_CONTACT_SOLVER.md` for the ranges |
| `00C321B0` | read in full, named, contract in the host | complete for what the step needs; what the counted scopes measure beyond tsc totals is not read |
| `00C4D980` | read in full, named, contract in the host | complete for the flush; `00C43AA0` and `00C43C00` are not read |
| `00C41550`, `00C5B1B0` | reused as contracts from `docs/RIGID_BODY_INTEGRATION.md` | as reconstructed there |
| `00C50390`, `00C57020` | call sites, labels and ids read; bodies not read | contract: profiler scope record lookup |

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `dyn_contact_listener` | the writer of `world+24h`; `00C5C43A`'s callee | who installs the contact listener, if anyone does. The field is zero from `00C41C2C` and this packet found no writer, so the shipped game may never build a contact report at all; a writer outside the addresses read would settle it |
| `dyn_collision_pass` | `00C57070`, `00C35480`, `00C549D0`, `Dyn_SAPRadixBroadPhaseManager_vslot3` | the broad phase, the narrow phase and the producer of the manifold list and its contact points, which is the missing producer for `DynContactPoint`'s four fields |
| `dyn_body_pools` | `world+4Ch`, `world+170h`, `00C43AA0`, `00C43C00`, `00C5D580` | the two body pools `00C4D980` returns bodies to, and how `Dyn_World_CreateBody` takes them out |
| `dyn_profiler` | `0109E9F8`, `0109E9FC`, `00C50390`, `00C57020` | the profiler object, its scope records and where the frame totals are read back |

## no_ghidra_function

| start | inclusive end | evidence |
| --- | --- | --- |
| none | | Every address named or reconstructed in this document lies inside an existing Ghidra function body, checked with `python tools/bsp.py ghidra proto <addr> --brief`. The one undefined range this packet found belongs to `docs/FIXED_STEP_CALLBACKS.md` and is recorded there. |
