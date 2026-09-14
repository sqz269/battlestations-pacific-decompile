# Game native VFS application owner (BJ)

`GameNativeVfsApplication` retains the source-side raw VFS graph for one
`GameSingletonHost`. It borrows that host's actual `01090AA0` publication and
deletion table, plus one `GameNativeReadOnlyData`. It does not create a second
raw singleton manager. The mapped numeric data and the original executable's
verified initial-image constants must remain available until the shared drain.
This is an application binding over reconstructed bodies, not an original ABI
or recovered native constructor.

Construction prepares stable application publication cells (`0109CEEC`,
`0109DC28`, `0108FE8C`, `0109DBBC`, `01090AA8`, `01090AA4`, `0109DB7C`),
`NativeVfsOwnerServices`, `GameNativeTypeStorage`, a common
`LightTypeBootstrap`, retained memory counters/profiles, archive constants,
the initial empty enumeration name, and a diagnostic sink backed by
`GameHostLog`. It validates the borrowed numeric table spans, but invokes no
raw getter, native type initializer, or VFS constructor.

`initialize_core()` makes one attempt, in this order:

1. Bind the owner services to the existing singleton deletion table **before**
   any type getter can register an owner.
2. Initialize the memory stream type through `00CD8FC0`, the process physical
   pool through `00CD9010`, then the physical stream type through `00CD9030`.
   The process pool and common `00E188B4` allocator domain come from
   `game_native_physical_pool_process()` and remain process-lived. A nonzero
   `00CD9010` return is a CRT registration status; the pool has still been
   initialized and is not rolled back. The status is exposed separately.
3. Allocate actual A0h manager storage, construct and retain
   `GameNativeVfsRuntime`, bind it to the same singleton host, then call
   `construct_and_register_core()`. The runtime may now register raw owners.

If the runtime object itself cannot be constructed, the A0h allocation is
freed before any core constructor/registration has touched it. If core
construction throws, the retained runtime and A0h storage remain in place:
native publication or registration may be partial, so no invented rollback or
free is attempted. Type initialization may likewise leave registered owners
before an exception. The caller must drain **the same** singleton host while
this bundle, its data, and all borrowed bindings still live, on both normal
and exceptional paths. `GameSingletonHost::shutdown()` retires the bound VFS
runtime immediately after its raw drain. The bundle destructor does not access
the singleton host; the application may destroy the host before the VFS owner.

`runtime()` supplies phase-2 and later raw operations after initialization;
`owners()` and `types()` expose the stable shared services to later consumers.
Calling `runtime()` before its construction or retrying `initialize_core()`
throws. Consumers requiring projected `VfsMountContext` still need a concrete
adapter or migration; the A0h manager is not such a context.

The strict MSVC Win32 `/W4 /WX /fp:strict` translation-unit compile passed.
An ignored adaptation of the prior BH handoff probe linked this source object
with the existing `6B5D0DE1` core archive and ran in the controlled rebuilt
child. It built three loose mounts and six search groups, read the installed
657-byte `scripts/fundamentals.lua` through capacity and complete reads,
checked repeated missing reads, drained ten raw singleton registrations, and
destroyed `GameSingletonHost` before this retained bundle. Exact inputs and
results are in `reports/game_native_vfs_application_bj.json`. This is fixture
evidence, not production `bsp_game` startup, original executable execution,
gameplay, or full ABI compatibility. Production wiring and exceptional
shutdown validation remain integration work.
