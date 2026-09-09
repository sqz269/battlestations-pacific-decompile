# RTTI-named middleware inventory (battlestationspacific.exe)

Read-only survey of the code blocks that carry MSVC RTTI class names. Method: offline RTTI walk of the disk PE
(TypeDescriptor -> CompleteObjectLocator -> vtable -> slots) mapped onto `exports/bsp/functions.json`, plus a
Capstone static call graph of `.text` used to bound each block by caller/callee locality. Ghidra was used read-only
(strings, xrefs, six decompiles). Machine-readable version: `rtti_middleware.json`.

## Headline

- Only **130 type descriptors** exist in the whole image (128 with vtables): iostdnet 72, Dyn 27, std 14,
  iometrics 7, CRT undname 6, Mit 4. The game's own classes have **no RTTI** (built `/GR-`), so RTTI cannot be used
  to recover game class names; vtable xrefs and string anchors are the only route there.
- None of the RTTI-bearing blocks is a publicly available library. Two are in-house engine code (Dyn physics, Mit core),
  one is Eidos-internal telemetry (stub it), one is an unidentified small IPC helper (stub or identify), and the rest is CRT.
- Net effect on the reconstruction denominator: **~700 functions can be stubbed** (telemetry + wrapper + template block),
  **~290 functions are isolated physics** that can be deferred behind ~73 entry points, and **131 functions** of IPC helper
  are likely stub-able too.

| Block | Range(s) | Real functions | Vendor | Entry surface | Recommendation |
| --- | --- | --- | --- | --- | --- |
| Dyn physics | 00c30930-00c5df60 (+ 00403610-00406400 header code) | 257 (+35) | in-house Mithis "mitengine" | 73 entry targets, ~78 real game callers (31 in 0092a000-0093a000) | reconstruct, deferred; names free from RTTI + profiler markers |
| iostdnet + iometrics | 00a4d610-00a5de34 | 510 | Eidos-internal telemetry | 34 callers, 33 inside the MetricsSystem wrapper | **stub** |
| iostdnet template block | 0094cab0-0094ec00 | 64 | same | only iometrics + EH funclets | **stub** |
| MetricsSystem wrapper (game) | 00750820-00758000 | 123 | game code around the telemetry lib | 5 game-facing entry points | **stub** at those 5 functions |
| Mit core | 00401000-0040f810 (not bounded here) | n/a | in-house | Dyn uses 32 functions here | reconstruct (already in progress) |
| Named-pipe IPC helper | 00a5de34-00a607e8 | 131 | unknown third-party static lib | 5 callers | identify, then stub |
| CRT RTTI (std::*, undname) | inside 00be0000-00ce2000 | n/a | MSVC 2005 | n/a | import/match (other agent) |

## 1. Dyn physics/collision (in-house)

**Vendor.** In-house. Evidence: source paths `y:\mitengine\sources\physics\source\Collision/Primitives/DynConvexMeshShape.cpp`,
`.../Dynamics/Body/DynBody.cpp`, `.../Containers/DynFixedVector.h` (0x00d79d00-0x00d7a008); engine namespace `Mit`
(Mithis Entertainment, later Eidos Hungary); headers `..\System\mitmaximizedarray.h`, `..\System\MitStlCloneVector.h`;
no vendor copyright or version string; no PhysX/Havok/ODE/Newton/Bullet identifiers anywhere in the image. The design
(sweep-and-prune broadphase incl. a radix-sorted variant, LCP constraint solver, sleep groups, contact manifolds, terrain
vs convex intersectors) is conventional 2005-era rigid-body design, not a drop-in of any known library.

**Range.** Main body `00c30930-00c5df60` (257 real functions, 255 `FUN_` + 2 thunks). It is the last-linked static
library: it sits after Lua's `loadlib.c` (00c2f3e0-00c30200) and a small game glue object (00c30210-00c30570), and it
ends exactly where the `.text$x` unwind-funclet block begins (00c5df60). The tail of that funclet block,
`00cc7eb0-00cc9000` (154 `Unwind@`), belongs to Dyn objects; the funclet block is in link order, which is a useful
object-attribution hint in general. A second cluster `00403610-00406400` (35 functions) holds header-instantiated
Dyn/Mit code emitted in the first game object: 21 of the 64 Dyn vtable slots live there (shape constructors/destructors,
`LCPSolverTask::Run` 00403720, `LCPSolver2Task::Run` 00403850, `IntersectTask2::Run` 00403cc0), next to
`Mit::allocator_list_elem::~` (00403970, 61 callers, mostly EH funclets).

**Boundary evidence.** 00c30930-00c32a90 are leaf helpers called only from the game physics layer or from Dyn's own
unwind funclets (00cc8064, 00cc810d, 00cc8910, 00cc8930); 00c320d0/00c321b0/00c32250/00c32510/00c32860 are called from
inside the body. Nothing in the body calls game code: outbound calls are CRT (`malloc` 00bf681b, `free`, `memcpy`,
`memset`, `_alloca_probe`), 32 Mit-core functions in 00401000-0040f810 (`BSP_Math_AbsFloat`, `BSP_Vec3d_*`,
allocators 00407960/004077f0, math 004011d0) and `rdtsc`.

**Classes (27).** Shapes: Shape, ConvexShape, BoxShape, SphereShape, CylinderShape, ConvexMeshShape, TriangleShape,
TerrainShape. Intersectors: ShapeIntersect, BoxBoxIntersect, BoxSphereIntersect, SphereSphereIntersect,
GeneralConvexIntersect, TerrainConvexIntersect, TerrainConvexMeshIntersect, RayIntersection, BoxRayIntersection,
SphereRayIntersection, ConvexRayIntersection. Broadphase: BroadPhaseManager, SAPBroadPhaseManager,
SAPBroadPhaseManager2, SAPRadixBroadPhaseManager. Tasks: Task, CollisionSystem::IntersectTask2, Scene::LCPSolverTask,
Scene::LCPSolver2Task. Vtable slot addresses are in `rtti_classes.json` (scratchpad) and the JSON next to this file.

**Profiler markers (free function names).** `SAPSort, SAPSweep, SAPObjectNum, SAPPossiblePairNum, SAPPairNum, Collide,
BroadPhase, BroadPhaseUpdate, ManifoldUpdate, IntersectLoop, GetManifold, SolverPreStep, SolveConstraints, Simulate,
CreateGroups, Solve, UpdatePosition, SleepGroups` (0x00d79c80-0x00d79f4c). Confirmed anchors: 00c570f9 -> "BroadPhase",
00c5748f -> "BroadPhaseUpdate", 00c5c7ce (in FUN_00c5c7a0) -> "SolverPreStep"; FUN_00c50390 is the profiler node lookup.
FUN_00c5c7a0 decompiles as the scene solver pre-step (rdtsc-timed, allocates a 0x30-byte-per-body work array).

**Coupling.** 297 external call edges from 179 functions; ~101 of those callers are EH funclets hitting the Mit
allocator-list destructor, leaving ~78 real game callers. 73 distinct entry targets; the busiest are 00c32000 (18),
00c38040 (16), 00c31f40 (12), 00c336c0 (12), 00c31dc0 (11), 00c31f90/00c34f70/00c37450 (9), 00c33650/00c37e50/00c5d580 (8).
The game's physics integration layer is concentrated in 0092a000-0093a000 (31 callers, e.g. 0092aae0, 009329c0, 00935d30,
00937c90) plus 00447510/00447b80, 006fa540/006fa6d0, 008255b0/00825f20.

**Recommendation.** Reconstruct, but as an isolated module and late: nothing else depends on its internals, its API is
~73 functions, and the RTTI class names, 64 vtable slots and 18 profiler markers give most routine names for free.

## 2. iostdnet + iometrics telemetry (Eidos-internal)

**Vendor.** Eidos-internal metrics stack; not public. Evidence: endpoint
`http://bp-g4w-game.metrics.eidos.co.uk!0x534307E8.xlsp:1000` (0x00d012e8, copied into the wrapper in FUN_007524d0,
which logs "MetricsSystem ... Initialised ..."), namespaces `iostdnet`/`iometrics` with Z*/T*/I* naming (the `io` prefix
suggests an IO Interactive origin; that is an inference, not evidence), no copyright string. HTTP transport over XLIVE
sockets (`XSocket*`, `XOnlineStartup/Cleanup`), i.e. the Games for Windows LIVE build.

**Range.** Library body `00a4d610-00a5de34`, 510 functions, all `FUN_`. It begins right after the XLIVE import-thunk
group (00a4d356-00a4d608) and ends where a differently compiled block starts (section 4). Classes: 30 concrete
(ZString, ZStringBuilder, ZUtf8Buffer, ZTextDecoder + 4 feeders, ZLog, ZFileWriter, ZDateTime, ZMutex, ZEvent, ZThread,
ZSocket, ZUrl, ZHttpConnection, ZMemoryPool, ZMemoryPoolAllocator, ZMemoryPooledObject, IAllocator, IHashable, IWriter;
iometrics ZMetric, ZMetric::IStoredData, ZQueue, ZQueue::ZQueueProperties, ITransmitter, ZHttpTransmitter) plus 49
template instantiations (TRedBlackTree/TOrderedMap/TMap/TList/TDoublyLinkedList/TStack/TPair/TSingleton/IIterator).
FUN_00a50210 is `ZHttpTransmitter::ZHttpTransmitter`, FUN_00a53210 is `ZString::~ZString` (the most-called entry, 150 edges,
132 of them from EH funclets).

A second piece, `0094cab0-0094ec00` (64 functions), is the `TOrderedMap<char const*, ZString>` / red-black-tree
instantiation emitted in a game object; its only callers are iometrics (00a4e9a0) and EH funclets, so it is telemetry code
too.

**Game-side wrapper.** `00750820-00758000` (123 functions) is the game's MetricsSystem object; 33 of its functions call the
library (FUN_007524d0 init, 00753810, 00753570, 00750820, 00757ea0, 00752020, ...). Outside that wrapper the library has a
single caller (00cdcd20, a CRT-side static init/atexit helper). `ZString` is **not** used by game code elsewhere: its
destructor's non-library callers are the wrapper (12), the template block (6) and EH funclets (132).

**Stub boundary.** Game code enters the wrapper through five functions only:
00750910 (58 callers, nearly all EH funclets: a metrics-record destructor), 00753570 (from 005f65c0),
00753810 (from 0058bdf0, 005e5b30, 00777850), 007556a0 (from 004d7970), 00757ce0 (from 004e4a40).

**Recommendation.** Stub. A no-op MetricsSystem at those five entry points removes 510 + 64 + ~120 = ~700 functions from
the reconstruction denominator with no gameplay effect (the endpoint is dead and the transport is GFWL sockets).

## 3. Mit core (in-house engine base)

Namespace `Mit`: `cPoolAllocator<20,128>`, `<36,128>`, `<532,128>` (vtable slots 00407750/00407bb0/00407e30) and
`allocator_list_elem` (00403970). Together with the math routines already named `BSP_Math_*` / `BSP_Vec3d_*` this is the
engine base in 00401000-0040f810 that Dyn depends on (32 functions). Not bounded further here; it is in-house and is
already the object of the main reconstruction effort.

## 4. Unidentified named-pipe IPC helper (00a5de34-00a607e8, 131 functions)

A block compiled differently from the rest of the image: unaligned entry points, size-optimized bodies, HRESULT-style
return codes (E_PENDING 0x8000000A, E_UNEXPECTED 0x8000FFFF), `SetLastError(0)` before every Win32 call. FUN_00a5e844
creates or opens `\\.\pipe\%08x` keyed by process id (server side builds a DACL denying S-1-5-2 NETWORK; client side finds
the parent process via `CreateToolhelp32Snapshot`/`Process32FirstW`/`Process32NextW`, whose thunks 00a607d0-00a607e2 belong
to this block). The game drives it from a state machine FUN_00a4be50 (sleeps 14897 ms, then exchanges length-prefixed
messages with 5000 ms timeouts) and from 00a4bde0, 00a4c030, 00cd29e0 (9 calls) and 00ce099a. No identifying strings.
Best guess: a Microsoft GFWL-side helper or a launcher/DRM handshake; not gameplay. Recommendation: identify before
spending effort; it is isolated behind five callers and can be stubbed.

Boundary note for the Lua agent: Lua's `lbaselib.c` code (`setfenv`) starts at 00a607e8 and Ghidra has **no function
defined between 00a607e8 and 00a60950**; the Lua block therefore begins ~0x00a607e8, not 0x00a61000.

## 5. CRT-owned RTTI

`std::exception` family, `std::locale::facet`/`_Locimp`, `std::ios_base::failure`, `type_info`, and the undname
`DName`/`DNameNode`/`DNameStatusNode`/`charNode`/`pcharNode` classes are MSVC 2005 CRT/STL (FID bookmarks say
"Visual Studio 2005 Release"). Owned by the CRT/STL agent.

## Ghidra state notes

- `vftable` labels exist for the 128 classes (e.g. `iometrics::ZHttpTransmitter::vftable`), but the methods are not placed
  in class namespaces and the RTTI data carries no references, so `search_functions_enhanced`/`list_classes` return nothing
  useful for them; use the addresses in `rtti_middleware.json` / `rtti_classes.json` instead.
- The static call graph (Capstone, 56,884 callers) and RTTI walk are in the session scratchpad
  (`callgraph.json`, `datarefs.json`, `rtti_classes.json`, `block_analysis*.json`).
