# Native render storage integration

This batch registers two actual raw-slot pools, the command/source pointer
arrays, group construction/binding assignment, and named-resource record
destruction in `bsp_core`. It also corrects the common node constructor's
allocation-size check for the original smaller model slot.

| Component | Concrete behavior now compiled |
| --- | --- |
| D3D9 texture2D pool | Actual 38h canonical pool, 54h slots, shared allocator list, Win32 section, slab growth/trim/return and static lifecycle |
| Model pool | Actual 38h canonical pool, 188h slots, shared allocator list, Win32 section, slab growth/trim/return and static lifecycle |
| Render pointer arrays | Actual 0Ch headers, borrowed cells, reserve/resize/append/destruction with source-cell reads after allocation |
| Group storage | Actual 4Ch constructor preserving the 10h unwritten tail; intrusive assignment over the actual owner +04 count |
| Resource record | Actual 2Ch name/alias storage destructor; callback reloads and untouched resource/payload words |
| Node prefix | Same original 174h constructor writes accepted in a valid 188h model slot |

The primary reviewed the worker sources and byte evidence and reran the
focused comparisons. Texture-pool comparison covers 2,089 states and
134,765,644 slab bytes; model-pool comparison covers 2,089 states and
616,714,188 slab bytes, including constructor allocator-list reentry and
static atexit registration. Both exercise growth beyond 32 slabs, moved IDs,
consecutive empty slab removal, reuse and actual critical-section teardown.

The pointer-array worker's original comparison covers 98 observations.
The extended group/context fixture matches 549 observations using actual
terminal context destruction and frees. The resource-record comparison
matches six real ownership events with callback changes to the next node,
sentinel and name. The existing node fixture, resized to the original model
allocation, matches all 392 bytes. No broad new test suite is introduced.

`scripts/build.ps1` passes the strict Win32 build and both existing CTest
checks. Detailed commands, hashes, function names, original ABIs and evidence
boundaries are recorded in `reports/native_render_storage_integration_audit.json`.
Ghidra annotations preserve old names/comments and include complete returning
free continuations and newly recognized storage entries.

These are compiled and fixture-checked reconstruction components. They do
not complete the native renderer, full populated command queue, texture or
mesh owner, world startup, or gameplay. Current model owner/type work uses
the recovered pool and prefix; the installed mesh probe remains a separate
render-composition check.
