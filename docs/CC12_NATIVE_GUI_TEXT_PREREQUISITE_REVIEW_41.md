# Native GUI Text host prerequisite review 41

Independent read-only Source review of integration commit
`07c72d19b50a1e576f3c6e4e2f6edf2941660063`. This includes host wiring
`ab0ddbce9`, entered-failure containment `3429ff668`, process module 37
`de08e5308`, mutable-state module 38 `86b767ab4`, and complete type-header/
entry-boundary integration `07c72d19b`.
The dependency headers and implementations were inspected in their worker trees
and their unchanged blobs were verified again after the Root merge.

Verdict: no remaining blocking Source lifetime, order, replay, or duplicate-state
finding in this bounded prerequisite wiring. Combined canonical build and actual
startup remain separate acceptance steps owned by the integrator. The reviewer
ran no build, test, game, Ghidra query, or Ghidra mutation.

## Explicit startup order

`game_main.cpp:1084-1096` constructs the one `GameNativeGuiTextProcess`, retains
its pointer before native pool entry, invokes CD71F0 once, logs the real integer
atexit-registration result, and then enters the existing particle pool subset.
Construction alone binds trim dispatch; it runs neither pool initialization nor
type initialization. CD71F0 sets its Source state to entered before AB77C0,
registers CE0A80 only after initialization returns, and caches the registration
result (`game_native_gui_text_process.cpp:45-63`). Nonzero registration status
does not roll back the initialized pool or invent a cleanup callback.

`game_hosts.cpp:2606-2640` initializes the live Element/Text lineage before the
font host and GUI consumers. The inputs are exactly
`vfs_->native_owners().types()` and `vfs_->native_types().light_types()`.
These are views into the same retained `GameNativeVfsApplication`:

- `game_hosts_vfs.cpp:141-142` forwards both accessors to its same native owner.
- `game_native_vfs_application.cpp:53-59,82-90` owns the counter publication,
  `NativeVfsOwnerServices`, `GameNativeTypeStorage`, and its existing common
  bootstrap. That bootstrap uses the same owner-services counter and
  `type_storage.light_types()` view passed by the new host wiring.
- `native_vfs_owner_services.cpp:17` binds the counter lifetime to that actual
  publication; `game_native_type_storage.cpp:8-12` binds the light-type view to
  its existing root and guard cells. No new root owner or copied counter is used.

Process module 37 constructs a temporary bootstrap over those references for
AA8200, not a second descriptor domain. It stages the current root ID after the
root initializer returns, copies current Element parent IDs into Text lineage,
and consumes the same counter through the existing generic initializer
(`game_native_gui_text_process.cpp:22-41,89-102`; `gui_widgets_cc13.hpp:81-91`).
Accessors reject uncompleted lineage; zero-filled preimages are not advertised
as ready types. A repeated completed initialization only accepts the same
counter/root/guard identities and performs no native initialization again.

This preserves the bounded relative pool-before-particle order supported by
module 37's existing table evidence. Source type initialization is deliberately
later, when the VFS domain exists. It is not adjacent to pool initialization in
this Source schedule, and it does not establish the full Original CRT schedule
or absolute numeric IDs. The host comment and `source_bootstrap=late` log make
that distinction explicit. This review did not independently re-export Original
bytes or certify its CRT/register ABI.

## Failure containment and no replay

| Entered operation | Source state policy | Host containment |
| --- | --- | --- |
| CD71F0 pool initialization | State becomes `threw` before AB77C0; only complete returned startup caches a reusable result | Both standard and unknown exception catches check interrupted metadata first, then take `_Exit(2)` |
| CD7210/AA8200 lineage initialization | Same-domain identities and `threw` state are recorded before guard/name/parent/counter effects; failed/reentrant initialization cannot replay | `catch (...)` retains the full VFS/renderer/type graph and takes `_Exit(1)` |
| CE0A80 pool retirement | State becomes `retiring` before existing AB7420 and `retired` only after it returns | No retry or application-level substitute destructor is supplied |

The completed main catches (`game_main.cpp:1179-1200`) test
`native_operation_interrupted()` before diagnostics. Each interrupted branch
wraps logging/closing in `try/catch (...)`, has a bounded stdio fallback, and
then calls `_Exit(2)` unconditionally. An exception from the diagnostics cannot
escape into normal CRT cleanup. Unknown pool-startup exceptions are covered too;
an unknown failure outside this Text operation terminates at WinMain's existing
unknown-failure boundary without inventing cleanup for another process owner's
partial state. The explicit complete VFS/type-storage headers are present.

The type-startup catch at `game_hosts.cpp:2614-2622` follows the same wrapped
diagnostic and terminal-retention pattern. The predicate reads Source metadata
only, without acquiring the startup mutex or touching native/VFS storage, so it
can be used while deciding whether ownership may unwind. Successful ready and
retired states are not mistaken for interrupted states.

The wrapper adds no rollback or reset to the existing native pool initializer's
represented unwind effects. These are Source C++ control-flow checks, not runtime
failure injection or proof of complete native SEH/unwind behavior.

## CRT lifetime and expired VFS pointers

The Text process constructor first obtains the existing physical-pool process's
canonical allocator-list domain. The physical process therefore exists before
the Text static object; the Text static object exists before CD71F0 registers
CE0A80. Reverse CRT registration order runs the pool callback while the Text
storage/trim companion and common list remain alive. After that callback the
companion destructor only unbinds trim dispatch; it does not replay native pool
destruction (`gui_text_factory.cpp:86-95,253-259`).

The CE0A80 Source callback (`game_native_gui_text_process.cpp:66-72`) reads only
pool metadata and destroys that same pool. It never reads `counter_`, `root_`,
or `root_guard_`. Those pointers are assigned during explicit type initialization
and compared only for same-domain repeated admission. The lineage accessors read
the process-owned arrays. The default process destructor does not dereference
the saved VFS identities. Thus the later CRT callback can run after normal VFS
host destruction without dereferencing expired VFS/type-counter objects.

AB7420 frees pool slabs/table and removes the same allocator-list element; it
does not destroy Text payloads. Before future native Text activation, every
Text, retained raw factory allocation, pending operation, frame, and trim/list
borrower must be settled before this callback. This packet does not create a
native Text factory or allocate native Text payloads through the normal frontend.

## One mutable-state provider

Module 38 has one out-of-line `game_native_gui_text_state()` definition. Its
private constructor and deleted copying prevent caller-created substitute
owners; the function retains one heap allocation for process lifetime. Its
references remain alive through CRT callbacks without borrowing VFS or mapped
read-only-data storage. The module owns no lineage, pool, factory, or renderer.

Root `cmake/startup.cmake` registers process 37 once at line 2040 and state 38
once at 2113, both on `bsp_game`. The merged definitions match the worker blobs;
there is no second local copy of either provider in this wiring.

`game_hosts_text.cpp:45-46,187,281,457` now samples the shared E12FD4 backing for
summary initialization, wrapped layout, and glyph-quad writing. The old
sprite-only constant is absent. The summary value and per-call parameter values
are observations of that backing, not another mutable service owner. Existing
run caches are not rebuilt merely because the scalar changes; this packet makes
no live cache-invalidation claim.

Module 38 keeps distinct aspect/vertical/shadow/color/mask/blend cells with their
documented preimages. Its white accessor initializes the same quartet only when
mask bit zero is absent; merely reading/logging shared state does not eagerly
initialize white. Future native material/clip/Text services must borrow these
same references. Wiring all such services and native Text drawing is still
outside this change; the normal frontend remains the existing sprite bridge.

## Reviewed identities and evidence boundary

| Merged file | Inspected Git blob |
| --- | --- |
| `src/game_main.cpp` | `65a1a0db50d2ae420e9114a898771a060d7072a2` |
| `src/game_hosts.cpp` | `a9dbd6db939a085e27f3797194c25b269a215001` |
| `src/game_hosts_text.cpp` | `100b7df174476daa44e129edbde13dc20a3e79e1` |
| `cmake/startup.cmake` | `dcef1a26ebff8f251832eb70eafca1856f3466e4` |
| `include/bsp/game_native_gui_text_process.hpp` | `f00ee0d19c0ad67736751df12451f1e4b1e7f1ef` |
| `src/game_native_gui_text_process.cpp` | `bbddde0782ae85e048f5cbc5ca4d5afd46ce6996` |
| `include/bsp/game_native_gui_text_state.hpp` | `1d5a5d3ceebf618fa6beeda9d09c9d5b8b0e1806` |
| `src/game_native_gui_text_state.cpp` | `256293ad3740e894ecade510d0a7e514768f0fd2` |

The verdict covers these Source contracts and the inspected existing ownership
prerequisites. It does not report a combined build/test result, startup execution,
pool retirement observation, visual/native Text rendering, gameplay validation,
concurrent-use safety, full Original timing/IDs, or complete Original ABI.
