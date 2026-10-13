# CC13 files pass 1: segments 96-97 leaves (cc13-files1)

Worker cc13-files1 on packet cc13-files1, 2026-10-12, branch `agent/cc13-files1`. This was a reconstruction pass
over segments 96-97, `00BD9220`-`00C30570`. The band holds startup, the allocator, perftime, cFileStore, the VFS
search groups and aliases, the profile and debug-feature code, the error reporter, the PC save storage, the Win32
platform, RAM images and physical streams. The targets were the functions with a ledger name but no
reconstruction record and no tag in the index. Nothing was run in the game, and nothing is wired into a host.

Status: **reconstructed, build-tested** only. That means `./scripts/build.ps1` passed and ctest passed 2 of 2.
Nothing here is ABI-compatible, fixture-tested or game-validated. No test was added.

## Counts

| | count |
|---|---|
| named, no reconstruction record, no tag (index at the start) | 339 |
| **reconstructed and recorded** (`src/engine_services_cc13.cpp`) | **292** |
| existing reconstructions found without a record, **backfilled** | 2 |
| left: mid-function fragment `00BF3ACC` | 1 |
| left: CRT library code `00BF5DB1`-`00C2DBB0` (library names kept) | 44 |

The two backfills are:

- `00BEA8B0` → `retire_native_application_singleton_base_00bea8b0` in `src/native_render_service_base.cpp`.
- `00BF0DF0` → `ocean_normal_draw_00bf0df0` in `src/ocean_spectrum.cpp`, cc9's lane. Its record points at
  `docs/OCEAN_WAVE_FIELD.md`.

Both were checked against the listing before the record was written. For `00BF0DF0` the check used the constants
2^-32, 2.0 and -2.0, the `_CIlog`/`_CIsqrt` calls and the float stores.

## Files and model

- `include/bsp/engine_services_cc13.hpp` and `src/engine_services_cc13.cpp` are new. They are registered in
  `bsp_core` by one line appended to `cmake/startup.cmake`. Each declaration carries:
  - the address;
  - the original ABI;
  - the listing evidence.
- The routines work on the actual native storage, as the band's sibling files do (`src/native_vfs_*.cpp`,
  `src/native_filestore_*.cpp`, `src/sound_cc13.cpp`):
  - eight-byte strings;
  - `{data, count, capacity}` arrays;
  - MSVC 2005 list and red-black tree nodes;
  - intrusive reference counts at `+4h`, released through the object's own vtable slot 0;
  - singleton bases.
- Operator new and free map to `singleton_lifetime_allocate`/`singleton_lifetime_free`, as in `src/sound_cc13.cpp`.
- Strings use `NativeStringStorage` with these host bodies:
  - `0041DD40` resize;
  - `0041DD20` release;
  - the `00BE0A30` copy fragment;
  - `004BCC00` lowercase;
  - `0054AA70` append byte;
  - `00BEE690` normalize;
  - `00584110` trim;
  - `00435C40` equality;
  - `005EFBA0` find;
  - `00427110`, `00543E50` and `004CDC20` for string vectors.
- Calls to routines reconstructed elsewhere whose host signatures need contexts this file does not have go through
  `EngineServiceCalls`. Each virtual names its image address. Nothing is stubbed.
- No image global is invented. Every published word is passed in, among them:
  - `0109CF04`, `0109DB50`, `0109DB74`, `0109DBDC` and `00E1AE90`;
  - the VFS object `0109CEEC`;
  - the type words `0109DB58`, `0109DB60`, `0109DBA0` and `0109E9D0`;
  - the profiler colour indices.
- Byte-identical bodies (grouped with `local/f1tools/sc2_group.py`) share one C++ body. Each copy has its own record
  with scope `shared_shape_model`.
- `00BF6713` is the checked-iterator failure. It is an `InvalidParameterSink` call that may return, and the code
  continues after it as the listings do. Container-null tests on objects that a C++ reference makes non-null are
  omitted; each such site says so in a comment.

## What was reconstructed

- **Allocation.**
  - Raw frees: `00BD9440`, `00BD95A0`, `00BD95E0`, `00BD9780`.
  - Element allocators with the `0xFFFFFFFF / n` bad_alloc test: `00BD9B20`, `00BD9BD0`, `00BD9CB0`, `00BD9D30`,
    `00BE4F30`, `00BE4FA0`, `00BF1760`, `00BEC310`.
  - Owned-block frees: `00BE2BB0`, `00BE2BD0`, `00BE2BF0`, `00BF2E70`.
  - Performance counter: `00BE3240`.
- **Constant leaves and getters.** These include `00BE2720`, which always answers 0 because it reads a zeroed
  local, and `00C30230`, whose miss returns EAX `0109E900` (AL 0).
- **Teardown.**
  - Vtable-storing scalar deletes.
  - Singleton-base resets with their published words.
  - Scalar deletes over a destroy routine (`DestroyRoutine`).
- **Reference slots and streams.**
  - Release, release-and-keep (`00BE44A0`) and unchecked release (`00BE4500`).
  - Assign without add-reference (`00BEF1E0`).
  - VFS open and reopen.
  - Stream read and write wrappers through the stream's own vtable.
  - The writer wrappers `00BF0340`/`0360`/`0390`/`03B0`. Their callee writes the byte count through the address of
    its own argument slot, and `*remaining` is debited by it (listing read).
- **Checked iterators.**
  - Eleven tree dereferences, two list dereferences, Equals/NotEquals, list increments and post-increment.
  - The 16 pre-step wrappers (`IteratorStep`).
- **Containers.**
  - Twelve tree-header constructors (`TreeHead`; the isnil offset is `1Dh`, `21h` or `19h` per tree).
  - List-header constructors, tree clears and destructs.
  - Pointer-list clear and remove.
  - Text-queue erase.
  - Pointer-array grow and push-backs.
- **String-bearing records.**
  - String releases at `+0h`/`+4h`/`+0Ch` and pair destroys.
  - Named-value, string+int and string-pair copies. `00BDCEA0` is the same body as `0048E500`.
  - Debug-feature records (88h).
  - Stream-writer chunk push and pop.
  - VFS pair push.
  - String-vector grow-to-index and arrays of string vectors.
- **VFS.**
  - Name-list find `00BDBFD0` (ABI from the listing: ECX result, EDX key, 4 stack words, RET 10h).
  - Search-group find, flagged-mount existence, alias add and clear, existing-name resolve on a copy.
  - Provider unmount.
  - Search-group value and prefix unregister.
  - Search-group payload and list nodes.
  - Extension-prefix tree node, link-and-rebalance and insert-equal.
  - Mount find by name and path.
  - Extension-prefix pair find.
  - File-block scope construct.
  - File-store pending equal range.
- **Singletons and platform.**
  - Platform base register and unregister.
  - Deleting destructor `00BEA9A0`, which repeats the `00BEA8B0` steps inline.
  - Frame-clock base and variant; allocation-stats derived construct.
  - Run-loop dispatch.
  - The window-proc thunk `00BEC3B0`. Slot `+28h` is a **stdcall** that takes the platform explicitly; ECX holds
    the vtable pointer, not the object.
  - Window platform base construct. Its aspect ratio is FILD/FIDIV of the **uninitialized** width and height,
    before the 640x480 stores.
  - Win32 ApplyWindowMode. Aspect is rounded to float; FCOMI against `00D5BD98` sets the wide flag, so NaN gives 0.
  - Win32 platform destroy, with the text queue at `+174h`.
  - Profiler construct (six arrays, saturated 64-bit sizes, 17 fixed slot colours) and destruct.
  - Debug-feature lock singleton and the owner's locked reads.
- **Structured reader.** Node readers, remaining bytes, read-string-into, name-stack destroy, reader construct, and
  the two node constructors that push the node name onto the reader's stack.
- **Text scanner.** Accept, expect and read-bool. A consumed token is copied from `+1h` to `+401h` and `+801h` is
  cleared.
- **Images and physical streams.** RAM image destroy. TGA load `00BEFFA0`: the listing releases the stream
  without a null test. Provider block destroy; physical stream close, destroy and construct-and-open.
- **PC storage.**
  - Root path, slot-file delete, file write time, can-create, destroy.
  - Free-space test and file enumeration. The listing never calls `FindClose` on the find handle.
  - The two `sprintf` path builders are bounded in the host, where the image would overrun its 100h-byte buffer.
- **Error reporter.** The `map<thread id, vector<string>>`:
  - min and max, rotations, advance and retreat with the `00BF6713` validations;
  - head allocation, find, node construct and buy, node scalar delete;
  - value destroy;
  - insert-at and insert-unique;
  - erase. Its result is the advanced `where` argument (the listing advances the stack argument in place), with the
    standard MSVC 2005 `_Erase` rebalance confirmed in the listing;
  - erase subtree, clear, erase range, destroy.

  The reporter itself: construct, destroy, push and pop context (under the tracked section), Fail (`MessageBoxA`
  "Failure") and FailUnknownAssertion.

## Ledger name corrections

| address | old | new | evidence |
|---|---|---|---|
| 00BDBEB0 | `FUN_00bdbeb0` | `BSP_VfsEnumerationVisitor_ContinueFalse` | `XOR AL,AL; RET`; visitor slot +8h tested after each mount |
| 00BDBFD0 | `FUN_00bdbfd0` | `BSP_NameList_FindInsensitive` | list find, sole caller 00BE0FC0 AppendUniqueName |
| 00BF2230 | `BSP_ErrorContextNodeValue_ScalarDelete` | `BSP_ErrorContextNode_ScalarDelete` | ECX+10h vector and free of ECX: a node |

`00BE40B0 BSP_Stream_WriteDwordSlot50` keeps its name. The name records that the function sits in slot `+50h`
of the stream vtables; its body calls `+28h`.

Ledger and Ghidra were updated together (`tools/ghidra_annotate.py --apply`). Prior values are in
`local/ghidra-annotations-20261013T004417Z.json` of the worker tree.

## RNG

`00BF0DF0` (polar Box-Muller over `RandomState`) is the only user of the shared generator in the band. It is
called from `00BD2F90` and from ocean `00B95750`. It was already reconstructed in cc9's lane. This pass only
backfilled its record and changed nothing about the RNG or its callers.

## Precision notes

- `00BE2AC0` and `00BEBFA0` divide two integers on the x87 (FILD/FIDIV) and store a float. The host divides in
  double and converts once. That is identical under 53- or 64-bit precision control. Under 24-bit precision control
  (D3D's default) the quotient is rounded once to 24 bits, and the two can differ in rare double-rounding cases.
- `00BEE140` multiplies singles; the host uses float.
- No DC-form x87 instruction occurs in any body reconstructed here.

## Left

- `00BF3ACC BSP_PhysicalDirectory_GetFileDate_EnabledContinuation` is a continuation inside
  `BSP_PhysicalDirectory_GetFileDate`, which `src/native_physical_file_date.cpp` covers. A fragment record belongs
  with that file's owner.
- The 44 CRT functions in `00BF5DB1`-`00C2DBB0` keep their library names: memcpy, free, the stdio and locale
  internals, `type_info` and the printf/scanf engines. They are out of scope for reconstruction.

## Handoff

The area is exhausted for this packet except the two items above. Open questions for the lead:

1. Should `EngineServiceCalls` be bound to the concrete VFS and file-store hosts? It would bind to
   `native_vfs_*`/`native_filestore_*` once a host needs these bodies; none does today.
2. Should `00BF3ACC` get a fragment record from the owner of `native_physical_file_date.cpp`?
