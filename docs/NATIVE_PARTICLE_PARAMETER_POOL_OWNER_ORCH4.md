# F8D344 particle-parameter pool static owner

## Result

The reconstructed static lifecycle now binds the application's distinct
`00F8D344` particle-parameter pool to the existing generic
`NativeWeakHandlePool` implementation. The binding borrows one companion that
already refers to the caller-owned `38h` storage and the application's same
`00E188B4` allocator-list domain. It does not create another pool, storage block,
slab list, allocator-list head or CRT callback family, and it does not reuse the
separate `0109CE94` weak-handle static binding.

`initialize_static_native_particle_parameter_pool_00cd78b0` first invokes the
generic `00B004B0` constructor on the bound companion. Only after construction
does it register `destroy_static_native_particle_parameter_pool_00ce0be0`. The
registration result is returned unchanged. A registration failure leaves the
constructed pool in place because the native initializer has no rollback path.
The registered callback selects the same binding and invokes the generic
`00B002C0` destructor.

Application installation remains an outer responsibility. The application must
own the actual `00F8D344` bytes, construct the companion with its shared
allocator-list domain, bind it once, and keep both alive through CRT shutdown.

## Native evidence

The configured installed executable has SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Saved Ghidra bytes and independently mapped installed-PE bytes match over both
complete functions.

| Address range | Complete bytes and SHA-256 | Recovered behavior |
| --- | --- | --- |
| `00CD78B0..00CD78C5` | `b944d3f800e8f68be2ff68e00bce00e831f7f1ff59c3`; `88d88a5d4640c8a545a0e5b23df12e1ae1d953de9c9431a5fa613d2720cb2991` | `MOV ECX,F8D344`; call `B004B0`; push `CE0BE0`; call `_atexit` at `BF6FF5`; `POP ECX`; `RET`. EAX is the registration result. |
| `00CE0BE0..00CE0BE9` | `b944d3f800e9d6f6e1ff`; `808d65cfdebef6d43c01f52c5e85fdf57346d96bbbcdb0a28207c8eff8c292ce` | `MOV ECX,F8D344`; tail-jump to generic destroy `B002C0`. |

The current saved-analysis name at `00CD78B0` is
`BSP_ParticleParameterPool_StaticInitialize`. `00CE0BE0` was still the generic
inventory label `CG_static_init_00ce0be0`; the ledger records the evidence-backed
descriptive name `BSP_ParticleParameterPool_StaticDestroy` for the primary
integrator to apply. This packet performs no Ghidra mutation.

## Focused lifecycle probe

The temporary Win32 probe uses the actual reconstructed generic provider:
`AllocatorListDomain`, caller-owned `NativeWeakPoolStorage` and
`NativeWeakHandlePool`. Its registrar observes that the list link, capacity-32
pointer table and zero slab count already exist when registration occurs, then
returns the sentinel status `37`. The initializer returns `37` unchanged and
registers the exact `00CE0BE0` source callback.

The probe allocates a genuine raw slot through `009242F0`, returns it through
`00924420`, and confirms that the next allocation reuses the same slot. Invoking
the captured callback frees the generic pool and unlinks its allocator-list
element. The probe printed:

`PASS registration-after-construction status=37 slot-reused destructor-unlinked`

The probe source, runner and executable remain ignored under `local/`; no
permanent test was added.

## Validation and limits

`python tools/ghidra_export.py verify-seeds` matched all seven configured seed
ranges against the installed executable. `scripts/build.ps1` completed the full
MSVC Win32 Release build, including the new translation unit and linked
`bsp_game.exe`. All three existing CTests passed: `reconstructed_math`,
`native_math_differential` and `tool_tests`.

This establishes complete byte coverage, the new source lifecycle, exact
constructor-before-registration ordering, unchanged registration status and
generic pool allocation/return/destruction behavior. The source API is a new C++
interface, not an original register-ABI replacement. Native exception dispatch,
real process-exit timing, application bootstrap installation, concurrent use and
retail gameplay remain unvalidated.
