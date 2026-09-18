# Native Dyn process owner (R148)

Addresses: `00CC8950`, `00C41AD0`, `00C420E0` (existing routines consumed);
`0109E9F4/F8/FC`, `0109EA48`, `00E17434/38/40/44/48/E8/EC`, `00D7A080` (owners/table).

## Result

`GameNativeDynProcess` provides one stable process owner for the actual Dyn
dispatcher objects, all eight dispatcher tables, the eight-slot SAP table,
intersection task table, both solver tables and the one-slot base task table.
It owns the engine/profile/general-convex publication cells and passes one
allocator and the existing borrowed CRT services to their consumers.

The base slot consumes the platform `_purecall`, matching the original
`00D7A080 -> 00BF698E`; it is replaced by the concrete table during task
construction. There is no substitute task implementation.

The ordinary application now invokes the existing `00CC8950` initializer
after its existing explicit startup owners. The original CRT array points to
this routine at `00CE36B0`. It constructs the `290h` GeneralConvex owner and
registers its actual `00CD91D0` cleanup through `std::atexit`. The other seven
objects receive their real tables; the convex-ray owner keeps its full `A0h`
scratch storage. Function-local process destruction is registered before the
native callback, keeping its storage alive until that callback returns.

Initialization is attempted once; repeated access preserves its registration
result. Access before initialization and rebinding allocator/CRT services are
rejected. A thrown initialization cannot be retried against partially changed
storage. Engine/profile/world allocation remains with the existing native game
construction calls. **The application does not yet call that raw game constructor.**

## Evidence and verification

The collector checked the correct `bsp.gpr` program and matched **3,332 live
bytes to the supported PE across 19 spans**. Five existing code bodies are
references; this packet claims **zero newly reconstructed native body bytes**.
The report records the direct CALL rows and original data bytes.

The constructor fixture pins its prior R113 source and copied-native image.
It recompiles current dependencies, including R143's corrected convex-ray
storage, and uses the new process owner on the source side. Four paired world
constructions compare **9,381,152 normalized bytes across 280 buffer pairs**.
They cover engine reuse, counts `1/2/3/4`, capacities `2/2/6/6`, vector
growth/copy/free, callback owner set/clear, all 13 tables, the full convex-ray
object and the general-convex publication cell. Real Windows handles and CS
state are normalized using the existing fixture rules.

An exit observer confirms the original and source cleanup callbacks actually
delete their critical sections. The native hook sees one registration; the
source uses `std::atexit` directly. The receipt's two registrations denote
these two callbacks. Two caught C++ exceptions in the log are the deliberate
pre-initialization/rebinding rejection checks.

A focused source composition probe constructs an actual engine with **one
worker** and one world. It schedules the constructor-produced intersection,
mode0 and mode1 tasks through the real task manager: four batches, nine worker
allocations, one real manifold/contact for the solvers and cold/warm profile
cache use with actual RDTSC. The worker's thread ID matches the manager's
thread handle. It checks velocity change, finite motion storage, balanced
profile parent restoration and profile counts `2/2`.

The intersection batch has an empty range. Contact production uses the actual
body/pool/manifold helpers; group membership and consumed inertia are fixture
inputs. This is task/context ownership evidence, not an integrated collision
or world-step comparison. R146/R147 retain their separate native solver
comparisons.

The strict MSVC Win32 build and all three existing CTests pass. Probe executables
use `/MD /fp:strict /W4 /WX /MANIFEST:EMBED`. After the other orchestrator's
run released the launcher lock, the ordinary application reached this Dyn
initializer with `atexit=0`, created its window and D3D device, and completed
one loop frame with exit code zero. It presented zero frames and skipped one
present; 45 host methods remain unimplemented. This proves the new startup
path executes, not visual output or raw-game admission.

## Limits and follow-up

- Full original CRT ordering, native game admission and full world stepping
  remain open. The startup hook only initializes process dispatcher ownership.
- These are source interfaces, not original native ABI replacements. Native
  FH3/SEH, OOM unwinding and failure-path runtime behavior remain open.
- The worker probe establishes one worker, not parallel solver/profile safety.
- Native task-manager shutdown joins its worker; the fixture separately closes
  101 handles omitted by the original destructor. Both fixtures explicitly
  dispose remaining quiescent storage. Full engine/world destruction is open.
- Keep the process and borrowed CRT/allocator pointees alive through all users.
  Gameplay, visual and performance equivalence have not been established.

Evidence: `reports/native_dyn_process_owner_r148.json`; immutable local archives
include collector output, pinned fixture provenance, source, dependencies,
build logs and exact artifacts.
