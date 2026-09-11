# Native cube texture pool startup and shutdown

`CD7B80..CD7B96` initializes the actual canonical pool at `108DB70`, then
registers `CE0CB0` through `BF6FF5` and returns that registration status.
`CE0CB0..CE0CBA` selects the same pool and tailcalls `B3E5B0`. These are
complete 22-byte and 10-byte bodies; the descriptive names are hypotheses.

The initializer pointer at `CE3518` lies within the established C++ initializer
interval `[CE2734, CE36E4)`. The full raw instruction bytes, CRT pointer table,
pool profile, and original allocator-list route were checked against the live
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and the installed
PE. Evidence is in `reports/native_texture_pool_global_startup_next.json` and
the primary's ignored `local/cube_texture_pool_primary_verified.json`.

The implementation is `src/native_cube_texture_pool_static.cpp`, with the
public contract in `include/bsp/native_cube_texture_pool_static.hpp`. Explicit
host setup borrows the actual pool storage and its existing shared list domain.
It establishes the genuine `D61944/B3E690` trim binding before publishing those
borrowed pointers. Setup does not initialize or write the raw pool, alter the
list links, or register an exit callback. The pool, list, and binding must stay
alive through the actual process exit callback.

Initialization calls the complete `initialize_native_cube_texture_pool_00b3f090`
on that pool/list, then calls real `std::atexit` with
`destroy_static_native_cube_texture_pool_00ce0cb0`. It returns the real status
without rollback. A construction exception propagates before registration.
Shutdown calls the complete `destroy_native_cube_texture_pool_00b3e5b0` using
the same binding. There is no private pool, replacement trim callback, or local
exit-callback collector.

The actual allocation, trimming, and lifetime dependencies have independent
original-body comparisons, including real CRT new-handler processing, original
FH3 cleanup, current-table reloads, and retained-lock failure state. They are
documented in `NATIVE_CUBE_TEXTURE_POOL_ALLOCATE.md`,
`NATIVE_CUBE_TEXTURE_POOL_TRIM.md`, and `NATIVE_CUBE_TEXTURE_POOL_LIFETIME.md`.

The integrated strict MSVC Win32 build and both existing CTests passed. The
primary library containing all four cube pool translation units is frozen at
`build/cube-pool-primary-check/bsp_core.linked.lib`, SHA256
`c8ac226e4373d04226066e0a7205893292ff442719a50c4a609e2c117d8d1d12`.
Independent verification now matches the original static pair and actual
primary library through real process exit: 18,931 DWORDs and 33 events.
The complete original 32 static bytes and 33 global-list dispatcher bytes are
unchanged; four explicit dependency bridges compose the actual production
lifetime/trim and real CRT. All runtime postimages, linker providers, original
service imports, current source snapshots, and artifact hashes were rechecked.
See `NATIVE_CUBE_TEXTURE_POOL_STATIC_FIXTURE.md` and the paired audit.

The single deliberately failed 128-byte malloc observation invokes the real CRT
new-handler and both genuine cube pool bindings before a successful retry.
The scenario then performs 33 canonical allocations and 32 returns, global trimming
with 32 moved token rewrites, real CRT callback registration and exit invocation,
and destruction with two held critical-section entries. No production source
is recompiled or replaced by observation aliases. Registration exhaustion and
construction failure before registration remain source-order findings.

Both functions now have complete sharded reconstruction records and saved
Ghidra evidence comments. Prior names/comments were preserved, the missing
initializer was defined, and both affected exports were refreshed.

The interfaces are new C++ host APIs. The original no-argument register ABI,
arbitrary rebinding, repeated initialization/destruction, the game's complete
CRT startup sequence, real GPU use, and gameplay have not been validated.
