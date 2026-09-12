# Dyn engine-to-world owner connection

The existing reconstructed owners now compose in one Win32 fixture: actual
engine/profile/task-manager initialization and static dispatch startup supply
the inputs to the complete world and scene constructors. The zero-worker
original-byte and reconstructed executions agree on all **29 owned buffers**,
the complete world storage, eight dispatch objects, and allocation/publication
order. A separate source case creates one real worker and constructs its scene
and solver task arrays. This packet adds evidence, not a new runtime owner or
an application startup binding.

The dependency commits are engine `afd9a1bd564fa8d7c0bb8b0edc559e4de7b5a9b4`,
dispatch `d0c048c0e951dd33712cbe72d3c5d9f309f72cb1`, and task manager
`7d0f80fe333af0bab0ea645fececee0f610380f3`. They were merged locally into the
isolated worker checkout before its Release build. Exact compiled source and
header hashes are in the report and ignored input manifest.

## Connected ownership and ordering

1. `initialize_static_dyn_dispatch_00cc8950` constructs the real 290h
   GeneralConvexIntersect owner, its 26 normalized double direction vectors,
   and its real critical section. It registers the exact destructor callback
   with the host CRT `atexit`. Seven separate static dispatch objects receive
   the original complete one-slot method tables.
2. `dyn_engine_ensure_00c55f50` allocates the 14h engine and invokes
   `dyn_engine_construct_00c55ea0`. The 9Ch profile and 48h Root node are
   constructed first. Profile publication precedes its 90h zero tail; the tail
   is checked ready at the later task-manager allocation. The real 358h
   manager constructor initializes its synchronization objects and worker
   count. Engine publication follows completed construction. Repeated ensure
   returns that owner without reading a replacement descriptor.
3. A `DynSceneRuntimeContext` borrows that same engine global slot, an explicit
   general-convex global slot, the owned dispatch objects, and the complete
   native SAP/intersection-task tables. `DynWorldRuntimeContext` borrows this
   scene context and the three native solver/base-task tables.
4. `dyn_world_storage_construct_00c41ad0` constructs actual body/motion pools,
   then calls `dyn_scene_construct_00c38070`. The scene constructs SAP pools,
   the manifold pool/container/lock, and its event/task storage. Its count
   comes from the constructed manager at engine+10, manager+4. The world then
   reads that same engine slot for its two solver task vectors and allocates
   the shared motion last. No count or engine-layout projection is supplied.

The source dispatch aggregate has its own storage; it does not reuse the
native GeneralConvex owner. Only the original borrowed tables and explicit
CRT boundary are shared. The eight native/source dispatch identities are
mapped individually when comparing scene pointers.

Native engine ensure receives the descriptor in ECX. Engine construction and
both full world/scene constructors take two stack arguments and return with
`RET8`; profile construction receives its owner in ESI. These are recorded
original contracts. The exposed C++ APIs remain contextual interfaces, not
drop-in binary replacements. See [engine](DYN_ENGINE_RUNTIME.md),
[tasks](DYN_TASK_MANAGER.md), [dispatch](DYN_DISPATCH_INITIALIZATION.md),
[world](DYN_WORLD_RUNTIME.md), and [scene](DYN_SCENE_RUNTIME.md) for complete
body ranges, native calls and failure boundaries.

## Fixture evidence

The ignored `local/dyn_world_connection_probe.cpp` reuses the retained relocated
PE image at 30000000h. Original engine, profile, Root string, task constructor,
static initializer, world, scene and pool instructions execute. Three explicit
operand relocations cover the static initializer/destructor outside the prior
image's code relocation ranges. The installation and Ghidra are unchanged.

All 13 retained table extents contain original PE method targets in executable
sections. Their slots are relocated without replacement methods. An inherited
F-fixture relocation of five words at D7A0AC included the following RTTI word
D7CA20. That unused hull table and an unused shape switch table are omitted
here. The used SAP table has eight methods; the other twelve tables each have
one. Collision, intersection and solver methods are not invoked.

Host allocation/free, byte-copy/set, Win32 synchronization, and genuine CRT
`atexit` are explicit imported services. The valid-construction BF7CD1 CRT
vector iterator calls each actual original element constructor; its exception
unwind is outside this fixture. Removing that required bridge first exposed
unmapped CRT runtime support; its failure log is retained. No reconstructed
source change was needed.

Both dispatch constructors use the existing recovered BF7030 x87 CRT service,
with the actual mapped 0109DD78 slot, control word 027Fh and positive operands.
The external `__87except` boundary aborts if reached and is never called in
this run. This is shared numerical-service evidence, not independent CRT
handler fidelity. No bare FSQRT replacement is installed. Dispatch x87 status
and world status/TOP match; the world case uses the existing descriptor defaults.

The paired stream contains **938,692 bytes**, including 29 allocation records
with engine/profile global state, and hashes identically on both sides:
`3b79446a5134056b5293b78c40e3fa6509ca95eb67dccea0309ad922d0260478`.
Pointer normalization is limited to established fields and native pointer
vectors. All remaining bytes, including A5 allocation poison and untouched
task-manager+0/+8, are compared. Only `CRITICAL_SECTION.DebugInfo` is masked
at manager+18, manifold+1D8 and GeneralConvex+278. All 102 handles per paired
manager are checked valid and unsignalled before normalization by exact role.

The additional source case has 36 allocation records. A real `_beginthreadex`
worker is active at highest priority, its count reaches all three task-record
vectors and their pointer vectors, and the records point to their actual
scene/world owners. The recovered task-manager destructor joins the idle
worker, which exits with code zero. No work batch is submitted in this packet.

After all owners stop using dispatch, the two exact callbacks registered by
native/source startup execute through real CRT process exit. An earlier
registered observer sees one native destructor call and both deleted lock
states matching a real host-created/deleted control lock. Exit code zero and
`local/dyn_world_connection_exit.json` preserve that observation.

## Lifetime and limits

The paired count-zero managers intentionally retain A5 at +0. Their native
destructor would unconditionally free that field, so fixture disposal closes
all 102 handles and deletes the lock directly. The source count-one destructor
closes its semaphore and shutdown event; fixture disposal separately closes
the 100 completion events and retained worker handle that native routines
leave open. There are 305 explicit fixture closes plus two recovered destructor
closes, covering all 307 created handles. No tracked allocation remains.

World/scene child storage is freed wholesale only after manifold lock disposal
and worker shutdown. Engine/profile/world/scene destruction is not validated;
their owner/global lifetimes must be maintained by the integrating application.
Direct C41AD0 construction does not perform the separate C420E0 factory's
engine-world-list registration. Scene events, body creation, collision stepping,
solver execution, native SEH/OOM behavior, concurrent reconfiguration and
whole-game physics remain outside this packet. Method-slot presence does not
establish transitive runtime readiness of unexecuted collision code.

Win32 Release build and both existing CTests pass after seed verification.
No tracked tests or core changes were added. The report retains inherited
native call rows; its live call gate and the ignored artifact manifest make
the evidence reviewable and reproducible.
