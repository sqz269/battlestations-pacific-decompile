# Dyn collision scene construction

Addresses: 00C38070..00C38461 and 0040A1E0..0040A2DF. Descriptive names are
hypotheses; the dispatcher/task class names below come from existing Dyn RTTI.

`dyn_scene_construct_00c38070` implements the complete successful-allocation
constructor behavior on fresh E8h scene storage. It installs the native pair
and ray dispatch objects, creates the actual SAP manager and manifold container,
allocates per-worker collision tasks and their pointer array, and reserves the
event buffer. It reuses `AvoidZoneDynHullMemory` and the previously reconstructed
`dyn_sap_manager_construct_00c36f10`; no substitute physics implementation or
method table is created.

The native entry takes scene and world on the stack, returns scene in EAX and
uses `RET 8` at 00C3845F (`C2 08 00`). The new C++ API has an explicit runtime
context, so it is not a drop-in ABI replacement. Root's world constructor can
pass its in-progress world storage and the same borrowed `engine_slot` used by
the rest of world startup.

## Owned and borrowed storage

| Scene offsets | Constructor result |
| --- | --- |
| 00..8F | Six-by-six pair dispatcher, indexed `typeA * 6 + typeB` |
| 90..A7 | Six ray-dispatch object pointers, indexed by shape type |
| A8 | Borrowed world pointer |
| AC | Owned 248h `Dyn::SAPBroadPhaseManager2`, constructed by 00C36F10 |
| B0 | Owned 1F0h manifold container, including world pointer and critical section |
| B4/B8/BC | Pair pointer/count/capacity, all zero |
| C0/C4/C8 | IntersectTask2 pointer/count/capacity, sized to the worker count |
| CC/D0/D4 | Task-pointer array/count/capacity, sized to the worker count |
| D8/DC/E0 | Event pointer/count/capacity: 12,000 allocated bytes, count 0, capacity 1,000 |
| E4 | Event spin-lock word 0 |

Each 14h `Dyn::CollisionSystem::IntersectTask2` record receives only its complete
vtable at +0 and scene pointer at +8. Words +4, +C and +10 remain unspecified.
The event allocation is reserved capacity; the constructor does not initialize
its individual 0Ch records. With zero workers, both task arrays remain null.
The older collision-pass header's `kDynSceneNarrowTaskCapacityOffset` labels C4
as capacity; this producer establishes C4 as count and C8 as allocation capacity.

The constructor starts by zeroing all four vector triples. Consequently, the
inlined reserve code's old-element copies and frees at 00C3824C..00C382AD,
00C3831A..00C38353, and 00C383EB..00C38442 cannot execute in a normal fresh
construction. The source preserves the resulting allocations, counts, store
order around allocations and unspecified bytes without copying those unreachable
branches. Allocation failure/SEH ABI and externally reentrant mutation of the
object under construction are outside this interface.

## Actual dispatcher objects

`DynSceneRuntimeContext` borrows objects, not vtables posing as objects. Their
virtual methods must already be callable and their lifetimes must exceed the
scene's. Seven objects are statically initialized one-word vtable holders. The
GeneralConvexIntersect object is initialized by 00C48FD0, which installs its
vtable, initializes a critical section and normalizes 26 double support directions.
This module does not replace that global startup routine or its collision method.

| Native object | Actual class | Installed indices |
| --- | --- | --- |
| 00E17434 | Dyn::BoxBoxIntersect, vtable 00D7A184 | pair 7 |
| 00E17438 | Dyn::TerrainConvexMeshIntersect, vtable 00D7A18C | pair 29, 34 |
| 00E17440 | Dyn::SphereSphereIntersect, vtable 00D7A1CC | pair 0 |
| 00E17444 | Dyn::BoxSphereIntersect, vtable 00D7A1D4 | pair 1, 6 |
| 0109EA48 | Dyn::GeneralConvexIntersect, vtable 00D7A1A8 | pair 2, 4, 8, 10, 12, 13, 14, 16, 24, 25, 26, 28 |
| 00E17448 | Dyn::ConvexRayIntersection, vtable 00D7A1F4 | ray 2, 4 |
| 00E174E8 | Dyn::BoxRayIntersection, vtable 00D7A1FC | ray 1 |
| 00E174EC | Dyn::SphereRayIntersection, vtable 00D7A204 | ray 0 |

All other dispatcher cells remain null. In particular, the source does not
invent handlers for ray types 3/5 or extra terrain pairs. 00C380F6 also writes
the shared GeneralConvexIntersect object into global 0109E9F4; the context makes
that writable global slot explicit.

At 00C381FC, after the manifold allocation/initialization, the constructor reads
global engine 0109E9FC. Engine+10 points to its real task manager/thread pool;
pool+4 supplies the worker count at 00C3820B. 00C55EA0 is the engine producer:
it allocates 358h for the task manager, calls 00C37740 with the engine descriptor's
first word, and stores the result at engine+10. Engine/global dispatcher startup
is a borrowed runtime prerequisite. This API does not substitute a worker count
for a functioning task manager or construct a fake engine.

## Manifold pool

0040A1E0 takes the pool pointer on the stack, returns it in EAX and uses `RET 4`.
It initializes the first 1D4h bytes of the 1F0h allocation:

- Page vector +0/+4/+8 becomes pointer/count 1/capacity 2.
- Free head +C points to a 36B00h allocation containing 1,000 E0h slots.
  Each slot's +DC points to the next; the final link is null.
- Start sentinel +10 has previous +E8 null and next +EC equal to pool+F0.
  End sentinel +F0 has previous +1C8 equal to pool+10 and next +1CC null.
  Active count +1D0 is zero.

The scene constructor then calls `InitializeCriticalSectionAndSpinCount` on
container+1D8 with spin count 10,000 and stores world at container+1D4. This is
the real container used by subsequent native manifold allocation and collision
passes. The source does not instantiate contact manifolds during construction.

## Verification and limits

MSVC Win32 Release and both existing CTests passed. One ignored probe links
the worktree's Release library and compares actual original-byte constructors
against source for worker counts 0, 1, 4 and 64. All four E8h scene records and
all 78 scene-owned allocations match, including SAP/manifold pages, sentinels,
free chains, spare pointer capacity, task unspecified bytes and the entire event
reserve. Zero-worker construction makes 18 allocations; each other case makes 20.

Only proven pointer fields are normalized by allocation identity and offset.
`CRITICAL_SECTION.DebugInfo` addresses are excluded because they point to distinct
OS allocations. The other critical-section record fields compare, and both
constructed locks pass an uncontended enter/leave before `DeleteCriticalSection`.
Both record files have SHA256
`3b77266c04a8716ab7530f254f615ef5f73fc0eeb5ad30d6eab59ec501d82ae6`.

The native fixture executes 00C48FD0 to prepare the actual shared general-convex
object and uses the original static dispatcher objects and complete vtables.
The CRT square-root intrinsic is bound to finite-domain FSQRT for that initializer;
allocation, memory and constructor-iterator runtime helpers use the host CRT.
The world pointer refers to storage with actual native body/motion pool constructors
run, but no full world constructor is claimed. The engine input is explicitly a
fixture projection of its consumed engine+10/pool+4 fields; real engine/task-thread
startup and task execution are not tested. The constructor's storage risk is what
this fixture establishes.

All three code spans used for scene/pool construction and the shared dispatcher
initializer matched live Ghidra and installed PE bytes before relocation. Every
batch verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Parent root
repaired the false-free gap after 0040A2AD at 0040A2B2..0040A2B4 under the write
lock and refreshed exports (`reports/dyn_scene_pool_flow_repair.json`). Two scene
alignment gaps remain untouched; there are no remaining call fallthrough gaps.

Ignored artifacts are `local/dyn_scene_probe.cpp`, `local/build_dyn_scene_probe.ps1`,
`local/dyn_scene_native_image.bin`, `local/dyn_scene_*_records.bin`, byte-verification
records, `local/scene_probe.log` and `local/build.log`. No tracked tests were added.
The probe frees quiescent fixture allocations after deleting its locks; this is
not native scene/world destruction validation. Collision execution, thread ownership,
shutdown and game runtime behavior remain separate from this constructor packet.
