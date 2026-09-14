# Raw system-registry terminal composition

The actual singleton manager now dispatches `D62A3C` and `D626F4` owners to separately reconstructed raw terminal entries. These entries use the constructor's borrowed actual registry/manager/pool context and do not create a retained `Operation` or lifetime binding. Existing legacy registry interfaces remain available.

This packet starts from published `a9160919ec13ed56bceaf4c8e36c2ea57b818be2` with an authorized private original-ancestry merge of constructor packet `5e35e5705960a127b321cc497d02d4767981a171`. It does not change that packet's registry or compiled-record source files. Native evidence is pinned in `local/output/raw_terminal_inputs/manifest.json`: ten spans / 488 bytes, all equal between verified Ghidra and the installed PE. The target is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; there were no worker Ghidra writes.

## Dispatch contract

```cpp
NativeStringRawPoolContext strings{actual_01090aa8, actual_01090aa4, actual_01090aa0};
NativeSystemConstantRegistryRawContext registry{actual_0108fe94, strings};
NativeSingletonDeletionBindings bindings{};
bindings.system_constant_registry = &registry;
bindings.actual_string_pool_publication_01090aa8 = &actual_01090aa8;
bindings.actual_string_returns_disabled_01090aa4 = &actual_01090aa4;
destroy_native_singleton_manager_00bd0400(actual_manager, bindings);
```

The new `system_constant_registry` pointer is appended at offset 60h in the source binding, increasing its Win32 size from 96 to 100 bytes. Prior field offsets remain unchanged. It borrows the same actual `0108FE94`, `01090AA0`, `01090AA8` and `01090AA4` cells used during construction. Keep the context and referenced cells valid throughout drain and cleanup; it adds no private manager, cached publication, host ownership or opaque destructor callback. Admit owners following the raw lifetime contract; this context must not be used to retire an owner retained by an unfinished legacy `Operation`. Existing legacy bindings retain their own admission and guard requirements.

Dispatch passes the **popped owner**, selected by its current profile, with the native scalar flags. It does not require that owner to equal current `0108FE94`, and it does not clear publication itself. D62A3C selects raw B5DF70; D626F4 selects raw B5BB20. The actual base destructor determines which current publication to unregister and clear. As with the existing finite dispatch map, a missing required binding is outside the admitted source domain and raises the existing source contract error.

The raw manager allocation and actual `01090AA0` publication remain caller-owned after BD0400. This packet does not supply a new manager allocation wrapper or clear that publication. All non-registry profile bindings retain their prior dispatch behavior.

## Full terminal bodies

| Native entry | Verified native extent | Schedule |
|---|---|---|
| B5DF00 | B5DF00..B5DF62, 99 bytes | Stamp D62A3C, arm base cleanup, current-array resize/free, disarm, base destructor |
| B5DF70 | B5DF70..B5DF8D, 30 bytes | Capture owner, call B5DF00, test flags bit 0, optionally free captured owner, return captured bits |
| B5BB20 | B5BB20..B5BB3D, 30 bytes | Same scalar schedule with B5BA80 base destructor |
| BD0400 | BD0400..BD04C4, 197 bytes | Pop current last pointer, current profile/slot-zero call with flags 1, recount; finally release section and vector |

B5DF00's native ABI is ECX=actual 10h owner, plain RET. The scalar entries take ECX=owner and one stack flags argument, return the captured owner bits in EAX, and RET4. BD0400 takes ECX=actual 14h manager and plain RET. Source interfaces add context/binding references; they are not binary replacements.

At B5DF1E the destructor writes D62A3C. B5DF2B arms state 0 before B5DF33 calls B5BE10(array, 0). B5DF38 captures the **current** array data, and B5DF3B frees it through BF6989. B5DF45 writes state -1 **before** B5DF4D calls B5BA80. Thus a failing normal base call does not invoke the outer base cleanup again.

The source composes the existing raw B5BF50 array destructor for that zero-resize/current-free schedule. Its explicit B5BE10(array, 0) specialization requires constructor-produced **nonnegative count and capacity**, with valid current storage/extents. It does not implement general B5BE10 negative-count initialization or negative-capacity reserve-on-zero, and adds no runtime guards for those excluded states. Within the admitted domain, it decrements current count before each current-row string release. Data and capacity remain stale after free. It does not reset the fields, walk uncounted rows or free unpublished allocations. The complete raw B5BA80 then unregisters the current `0108FE94` value through the second actual manager getter, clears that publication and leaves the originally captured first-manager section before installing CE3818.

Scalar frees are conditional on a **returning** destructor. Native B5DF70 calls B5DF00 at B5DF73, tests flags at B5DF78 and frees at B5DF80; B5BB20 has corresponding calls/tests at B5BB23/B5BB28/B5BB30. Only bit 0 matters. If destruction throws, neither scalar frees its captured owner.

## Native FH3 ownership

| Owner | Handler / FuncInfo / map | State 0 action and receiver | Next |
|---|---|---|---:|
| B5DF00 | CC1128 / DF9E30 / DF9E28 | CC1120: ECX=`[FH3 EBP-10h]`, tail-jump B5BA80 | -1 |
| BD0400 | CC5458 / DFF490 / DFF488 | CC5450: ECX=`[FH3 EBP-18h]`, tail-jump BD0220 | -1 |

Both FuncInfo structures have magic 19930522, one unwind entry, zero try/IP maps and final flags 1. Each map contains `FFFFFFFF` followed by its funclet address. The scalar wrappers have no local FH3 frame. The full funclets, handler bytes, info words and instruction IPs are retained in the report.

BD0400 captures the native owner at BD041C in `[steady ESP+8]` and arms state 0 at BD0422. It loads the selected pointer at BD0465 and, for a nonempty current vector, lowers current end at BD0478 before testing the pointer. For a nonnull owner it loads the current profile at BD047F, loads slot zero at BD0481, pushes flags 1 at BD0483 and calls at BD0485. BD0489 recounts the current vector after every callback. The source's pre-existing body preserves this schedule; the new cases extend only its finite profile provider.

On a source C++ failure, BD0400's existing outer cleanup calls BD0220 on its captured manager and rethrows. That cleanup frees current vector data and zeros +4/+8/+0C. It does not release the +10 tracked section or free the manager allocation. Likewise, a registry array failure leaves its array allocation and captured owner live even when base cleanup succeeds. The source does not convert these effects into rollback or retain an operation to prevent caller disposal.

During an exception, B5DF00's armed source base cleanup is `noexcept`; another exception there terminates. After normal disarming, a base-destructor exception propagates through its own child cleanup without a second outer call. Original FH3 runtime frames, arbitrary spill aliases, hardware-fault/SEH handling and CRT throw identities remain unimplemented boundaries.

## Actual pool recreation during drain

Starting with empty actual publication cells, B5BF70 registers the registry in its base constructor before the first literal string lazily constructs/registers the pool. The manager therefore contains registry then pool. A reverse canonical drain destroys the pool first, setting actual `01090AA4=1` and clearing actual `01090AA8`.

The registry's next string release still invokes 419CC0. That getter creates and registers another pool through the same current manager even though small returns are disabled. BD1510 then suppresses the small return using the live shutdown gate; it does not dereference the old pool block. Each of the 52 record string releases follows this getter/return schedule. BD0400's current recount observes the newly registered pool and drains it before releasing its own section and vector. No source shortcut caches the destroyed pool or skips its getter because the gate is set.

## Validation

After all eight native seed spans matched live Ghidra and PE, the strict MSVC Win32 `scripts/build.ps1` build passed with `/W4 /WX /fp:strict`; both `reconstructed_math` and `native_math_differential` CTests passed. Source registration was appended only after the other worker's CMake lease ended, under a short coordination lease. The final `/MD`, `/MANIFEST:EMBED` fixture links only its own probe object and the complete rebuilt libraries, and both workflows passed.

An earlier intermediate probe compiled the new terminal and dispatch modules directly against the frozen constructor packet's library. Its passing results and artifacts are retained separately. Preparing the core target while awaiting the CMake lease was not treated as final validation.

The normal workflow constructs all 52 registry records with initially absent manager/pool cells. It checks registry-before-pool registration, a base scalar with flags 2 that returns without freeing, a registered D626F4 base owner, and canonical destruction of both profiles. It observes 52 getters during drain, two total pool constructions, 52 disabled returns, cleared publication/vector fields and released owner/array/section allocations. Only the caller-owned manager allocation remains at the assertion point.

The failure workflow injects one C++ exception at the first registry string getter after the original pool has been destroyed. It verifies count was predecremented to 51; B5DF00's state-0 base cleanup installed CE3818 and cleared current registry publication; the scalar retained its owner and array; and BD0400 cleared its vector while retaining the actual section with depth zero. The getter failed before pool recreation. Diagnostic fixture disposal occurs only after these assertions and does not retry the failed owner or manager.

Test-only entry hooks record actual allocations/frees and inject the selected exception; production has no such hooks. The fixture records loaded PE machine and physically resolved module paths inside the 32-bit process before hashing, distinguishing linked static libraries from loaded runtime DLLs. This is source behavior validation, not execution of original FH3 exception machinery or game validation. The machine-readable report records the final strict build and artifact status separately.

## Integrated validation at aa836cdd

Three terminal entries compose with existing BD0400 canonical manager dispatch using the popped owner and actual current publication cells. Normal pool recreation and first-getter source failure workflows passed; the existing197-byte BD0400 body is not counted as a new native entry. The combined strict Win32 build, eight seed checks and both CTests passed.
The six final-library fixtures, 503 direct/tail audit rows, 31 saved/read-back
annotations, and 59 live/PE spans are preserved in `local/checkpoints/aa836cdd/native-renderer-parent-dependencies/validation.json`
(SHA256 `e6cbd24bb0166528f733e3075862ab1edb655ea9624b623e7cf08f6df90fdbc3`). Original CRT/FH3/SEH/private-frame identity,
full renderer lifecycle/adoption and gameplay remain unvalidated.
