# Retained movie factory publication authority

The movie factory now has one typed, writable Source publication authority in the same permanently retained `GameNativeStringProcess` that supplies the actual `01090AA0` manager cell. `GameSingletonHost::movie_factory_publication_00f8aefc()` returns that same cell by reference. This is a real source-storage producer; it is **not** a fixed-address mapping at native `00F8AEFC` or a claim of binary ABI compatibility.

Base main: `605373bf732e4bd3089e8f07ddca226ab4d635a3`. The primary explicitly approved the same admitted source-cell authority as the existing manager, after verifying that no current canonical writable page covers `F8AEFC`. The packet changes only the process header/source, SingletonHost header/source and these evidence files. It does not activate `GameMovieFactoryOwner`, Bink, Startup, deletion dispatch or any feature guard.

## Actual supplier and lifetime

The current manager path is:

```text
GameSingletonHost::manager_publication_01090aa0()
  -> manager_publication_01090aa0_ (reference)
  -> game_native_string_process().manager_01090aa0()
  -> GameNativeStringProcess::manager_
```

The new path borrows the same process owner:

```text
GameSingletonHost::movie_factory_publication_00f8aefc()
  -> game_native_string_process().movie_factory_00f8aefc()
  -> GameNativeStringProcess::movie_factory_00f8aefc_
```

`game_native_string_process()` already uses `static auto* const process = new GameNativeStringProcess`. No process destructor is registered for this heap object. Its cells, raw string context and actual string bindings survive ordinary application-host destruction, manager drain, late CRT/atexit callbacks and terminal aggregate retention; the OS reclaims storage at process exit. The new cell is appended after every established process field and value-initializes to a null pointer once. Borrowing it does not clear/reinitialize an existing publication.

The Host accessor is out of line, adds no Host field and changes no include graph. All prior process field declarations/order, raw contexts, publications, feature-word handling and domain-claim semantics remain unchanged. No serialized native node or object layout is extended; `GameNativeStringProcess` is source ownership metadata, not a projection of the native data-section layout.

## Original zero provenance and canonical-page boundary

The installed original executable was rehashed as `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Native address `00F8AEFC` has RVA `00B8AEFC`, in `.data` at section delta `00182EFC`. The section begins at RVA `00A08000`, has virtual size `00297EDC`, raw size `00010000` and characteristics `C0000040` (read/write data). This DWORD is wholly beyond the file-backed raw extent and inside the virtual extent, so its original loader-initialized value is zero. This establishes initialization/storage provenance, not a live publication or a host address equivalence.

The existing `GameNativeCanonicalDataOwner` / `GameNativeMutableCrtData` capability owns only writable pages `00E15000`, `00E16000` and `0109E000` under the existing V2 handoff. It exposes CRT cookie/NLG/failure/feature roles, not an `F8A000` page or factory cell. The read-only mapper covers original `.rdata`, not this mutable publication. This packet neither casts/writes `data_at()` storage nor extends the V2 reservation/page/role protocol. A future literal native-address mapping would need its own controlled producer/admission; it is not silently supplied here.

## Required consumer contract

Every future factory/bootstrap, unregister/destructor, current-publication shutdown and shared-deletion context must borrow **this exact accessor cell** by `void* volatile&`. The pending `GameMovieFactoryOwner` constructor already requires that reference; the root must pass `singletons.movie_factory_publication_00f8aefc()` alongside the same manager cell and genuine matched-heap/module providers. Do not copy its current pointer into another authority, add a parallel host field, reset it during Host construction/drain, substitute a default publication or use read-only image pointers.

This packet creates no factory and makes no registration, unregister, free, sound, module-load or shutdown call. It does not install the `CFEA60/CFEA78` deletion bindings. Original `007366B0` publication and `00736750` clear ordering, scalar captured-owner deletion and caller state `21h` exceptional raw-free behavior remain governed by the pinned owner/EH contracts. In particular, a construction exception can leave a dangling publication; a permanent cell alone does not make ordinary drain safe.

The root still owns stable factory/module/provider aggregate installation, real heap/domain verification, actual Startup order, explicit current-publication shutdown dispatch and terminal retention before ordinary destruction or drain. Cell lifetime is permanent on both normal and terminal paths; the contents are changed only by the admitted native-style owner operations. No readiness, cleanup completion, callback quiescence or runtime success follows from obtaining the reference.

## Verification boundary

Both changed C++ files passed strict MSVC Win32 object compilation with `/std:c++17 /permissive- /W4 /WX /fp:strict /EHsc /O2 /MD /MP2`. The required standard `scripts/build.ps1` passed in the private `build/win32` configuration with `BSP_CL_MP=2` and default `BSP_PCH=ON`, including its existing `reconstructed_math` and `tool_tests` checks (2/2 passed). Receipts are in the report. Source review confirms insertions only, one appended process authority and no added Host data member or constructor/drain effect.

No new tests, probes, games, Bink DLL/import calls, Ghidra mutations, page mappings, shared metadata changes or guard flips were executed by this packet. The existing checks do not call/witness this new publication API. Original fixed-address storage, full ABI/FH3/SEH, actual factory/sound/playback and gameplay remain unclaimed. Source hashes, original PE metadata, ownership/API scope and build receipts are in [the report](../reports/cc16_movie_factory_publication_cell.json).
