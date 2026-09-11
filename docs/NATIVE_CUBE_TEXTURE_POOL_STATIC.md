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
Independent real-process-exit verification is in progress; this document does
not yet assert that the original static pair and rebuilt callbacks have been
compared through exit.

The interfaces are new C++ host APIs. The original no-argument register ABI,
arbitrary rebinding, repeated initialization/destruction, the game's complete
CRT startup sequence, real GPU use, and gameplay have not been validated.
