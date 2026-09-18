# Native Dyn convex pool and process lifetime (R149)

Addresses: `00407D70`, `00407E30`, `00CC89C0`, `00CD9240`;
consumed constructor `00407C70`; storage `0109ECF0`, registry `00E188B4`,
table `00D7A1EC`, CRT entry `00CE36C0`.

## Result

The canonical Dyn process now owns its convex-shape pool and complete four-slot
shape table. The pool joins the same `00E188B4` allocator registry used by the
other application pools. Shape construction and physics teardown borrow that
one pool, the existing Dyn allocator and the actual mutable CRT feature word
at `0109EEA4`. No duplicate registry or conversion flag is introduced.

`GameNativeDynProcess::body_creation()` exposes this context after explicit
pool startup. Repeated initialization preserves its original registration
result; rebinding the registry or feature word is rejected. The application
invokes `00CC89C0` after its Dyn dispatcher initializer. Native cleanup runs
through the real CRT `atexit`, while the process and registry still exist.

## Recovered behavior

- **00407D70, 178 bytes:** stamp the concrete allocator profile, free all current
  pages and the pointer table, leave the critical section for each positive
  signed recursion count, delete it, stamp the base profile and unlink the
  allocator. Page count, table pointer, capacity, first-free index and old list
  links remain stale. Do not add clearing stores or repeat this destructor.
- **00407E30, 160 bytes:** the sole allocator virtual slot removes pages whose
  free count is 128. It copies the last page pointer into the removed position,
  decrements the count, and rewrites the page index in **all 128 moved slots**.
  It revisits the replacement and finally recomputes the first page with a
  nonzero free count. It does not enter a lock or shrink the pointer table.
- **00CC89C0, 17 bytes:** call the existing constructor, register `00CD9240`,
  and return the actual CRT status. Failed registration leaves initialization
  intact. The original CRT array contains this entry at `00CE36C0`.
- **00CD9240, 5 bytes:** tail-jump to the global destructor.

Ghidra had omitted 14 and 3 bytes after the destructor's two returning frees,
and 58 bytes after the trim free. These gaps were repaired under the write lock.
The previously undefined initializer was defined with its exact 17-byte range.
Prior annotations, repair receipts and refreshed exports are retained.

The existing constructor's 241 bytes are reference evidence, not new recovery.
Its canonical-list adapter preserves publication before the 128-byte table
allocation. The trim binding is installed before native publication. The list
helper writes only links, preserving this constructor's concrete-table order.
Total new normal bodies: **360 bytes**. Total live/PE verification: **669 bytes
across nine spans**.

## Verification

The strict MSVC Win32 build and all three existing CTests pass. Two ignored,
focused probes use `/MD /fp:strict /W4 /WX /MANIFEST:EMBED`.

The copied-native comparison covers 12 cases and **19,034,324 normalized bytes**,
with **140 allocator/registration events per side**. It checks empty/full/partial
pages, first/middle/last/all/alternating removals, moved metadata, allocation
after compaction, table growth beyond 32 pages, all registry positions, real
critical-section depths 0/1/2 and the deleted section state. It compares unused
capacity and stale fields. One case proves registration status 7 preserves
initialization; the fixture invokes its captured cleanup callback explicitly.

The reference constructor omits native FH3 registration and uses the original
normal instruction sequence. Actual slot allocation/free helpers are shared
setup dependencies. The allocator records logical frees and retains backing
buffers for byte comparison; this is distinct from the real lifetime run below.

The process lifecycle probe constructs an engine with one real worker, a world
and two convex bodies through the production context. Actual SAP processing
produces one pair; the actual convex dispatcher produces one contact. The
constructor-produced intersection and two solver tasks run as three worker
batches, making nine worker allocations and updating the real profile owner.

The existing native physics teardown then destroys the bodies, shapes, scene,
world, engine and task manager using the same context. Both shape slots return
to the canonical pool. Shared-registry trim removes the empty page. The real
CRT callback frees the remaining table and unlinks the pool; an exit observer
sees **zero tracked allocations** before C++ owner destruction.

## Evidence boundaries and follow-up

The lifecycle probe supplies authored hull data, transfers actual SAP output
into the scene vector, and supplies one contact group and derived inertia. It
does not execute the entire world-step/group-building sequence. The task-manager
destructor leaves 101 handles open; the fixture closes those after the actual
worker join. The native stale profile publication is retained.

Ordinary application execution is recorded separately in the report. Full
original CRT ordering, raw game-owner admission, native exception/failure ABI,
parallel mutation, gameplay and visual equivalence remain open. The unlocked
trim method must not run concurrently with pool mutation.

Evidence: `reports/native_dyn_convex_pool_r149.json`, including immutable local
archives of source, dependencies, copied bytes, probes, logs and exact artifacts.
