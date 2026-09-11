# Native hardware-layout pool static lifetime

The two wrappers reconstruct complete `CD7CA0..CD7CB6` (22 bytes) and
`CE0D40..CE0D4A` (10 bytes). Native startup has no arguments, calls the pool
constructor with `ECX=108FE9C`, registers `CE0D40` through `_atexit`, and returns
that registration status in `EAX`. Native shutdown sets the same `ECX` and
tailcalls `B60270`. Neither wrapper has an exception frame.

`bind_static_native_hardware_layout_pool_0108fe9c` provides explicit host setup
for one actual aligned 38h pool and its shared `AllocatorListDomain`. It first
calls genuine `bind_native_hardware_layout_pool_trim_00d62af0`, then stores the
two borrowed host pointers. Binding does not write the raw pool, initialize a
critical section, reset or publish list links, or register an exit callback.
It must finish before startup and remain unchanged, with both actual objects
alive, through shutdown. This is host binding metadata, not another pool owner.

`initialize_static_native_hardware_layout_pool_00cd7ca0` calls the actual
`B604D0` source initializer with that pool/list, then calls real `std::atexit`
with `destroy_static_native_hardware_layout_pool_00ce0d40`. It returns the CRT
status directly. If construction throws, registration is not reached. If
registration fails, there is no rollback. The shutdown binding invokes actual
`B60270` on the same pool/list. The source adds no state validation, implicit
reset, alternate registration provider, exception catch, or owner destruction.

The native initializer table word at `CE353C` is `CD7CA0`. This reconstructs
the selected initializer and callback, not the CRT's table iteration, encoded
callback array, lock, realloc behavior, or other allocator globals. A host
composition must explicitly bind and invoke startup; this packet does not
silently add a process-global constructor to the library.

## Evidence and original-caller comparison

Eleven fresh project-guarded Ghidra ranges match the installed PE, including
all 32 owned body bytes, the actual lifetime/trim dependencies, complete native
`_atexit` wrapper, profile word, initializer-table entry, and zero-filled actual
pool/list-head image storage. Every query verifies project
`C:/Users/sqz269/bsp.gpr` and program `/battlestationspacific.exe`. Initial saved
analysis has no function object at `CD7CA0`; `CE0D40` is named
`CG_static_init_00ce0d40` despite being the shutdown thunk. Shared annotation,
creation and export refresh belong to the primary integrator.

The retained private fixture runs each full original wrapper from pinned PE
bytes at its original address in a new child process. Three explicit entry
bridges reach the complete current `B604D0`, complete current `B60270`, and real
host `std::atexit`. The 32 owned bytes are unchanged. Saved runtime postimages
verify every mapped span against its preimage with only these three binding
sites. The source side uses the same actual current dependencies. This tests
the original caller boundary; original inner pool and CRT implementations are
not substituted into that boundary claim.

Each side uses actual pool storage at `108FE9C` and actual shared head at
`E188B4`, with a persistent shared list domain. The fixture first verifies that
binding changes neither raw pool bytes nor head. Real construction publishes
the pool and allocates its table. Thirty-three actual slot allocations create
two slabs; returning the first 32 and calling actual shared-list trim frees
the empty slab, moves the partial slab, and rewrites its slot indices. This
exercises the genuine trim binding installed before pool publication.

The fixture then prepends an actual base list element and enters the real
pool critical section twice with corresponding recursion depth. It returns
from `main`. The real CRT invokes the registered source callback or original
shutdown thunk; neither is called manually by a substitute callback collector.
A reporting callback registered before startup runs afterward and verifies
one initialization, one shutdown, registration success, three real allocations
and frees, recursion drained to zero, base profile installed, and predecessor
links preserved. The pool fields that native teardown leaves stale remain
stale. Observation aliases forward unchanged current allocation/free and pool
lifetime source; they do not provide fake pool implementations.

The strict MSVC Win32 build, its two existing tests, and the paired process-exit
comparison pass. The original/current traces match **1,805 DWORDs across 19
events**, with 95 words per event. All eight native math seed spans verify. The audit records
the complete compared observations and final source/artifact pins. No new
repository test suite was added. Registration failure is preserved by direct
return and original/source assembly inspection; the fixture does not force
host CRT exhaustion. Constructor exception behavior is established by the
actual dependency's separate original-EH comparison, while the static source
and assembly retain the direct call-before-registration order.

Dependencies are exact source from lifetime3 worker `e4bd667`, integrated as
`3c1d701`, and trim1 worker `dd67627`, integrated as `2b5709e`. Lifetime's final
independent comparison covers 52,094 DWORDs with original FH3 cleanup. This
packet uses those completed APIs; it does not count their functions again.

Retained local reproduction: `local/prepare_hardware_pool_static.py`,
`local/build_hardware_pool_static.ps1`,
`local/build_hardware_pool_static_check.ps1`, and
`local/write_hardware_pool_static_audit.py`. The worker used a private build
registration include; primary integration now registers the source in shared
CMake and the ledgers. Its unchanged process-exit fixture was rerun against
the frozen current primary library and exact production observation providers.
It again matches 1,805 DWORDs and real registered teardown. All 66 worker pins,
11 fresh live/PE spans and 22 loaded postimages were independently checked;
the complete 32 original static bytes remain unchanged. The missing initializer
was created, prior shutdown comments preserved, annotations saved and exports
refreshed. The committed audit is
`reports/native_hardware_layout_pool_static_audit.json`.

Status: complete new source interfaces, build tested and focused original
caller/process-exit fixture tested. The game installation and process remain
unchanged. No original binary ABI compatibility, arbitrary rebinding or
repeated startup/shutdown, whole CRT startup, or gameplay validation is claimed.
