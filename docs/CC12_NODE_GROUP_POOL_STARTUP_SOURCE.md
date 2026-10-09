# Production plain-node and group pool startup

The Source application now initializes the actual plain-node `0108FF58` and
group `010902F4` pool projections through the retained
`GameNativeResourcePoolProcess`. Complete existing Source providers were present
for both pools; this packet composes those providers without changing them.
The shared allocator domain is the existing physical process's `00E188B4`.

The baseline is `99c0522ae`, including the immutable
[Source581 primary review](CC12_MODEL_POOLS_COMMENT_SOURCE_PRIMARY_REVIEW.md).
Only `include/bsp/game_native_resource_pools.hpp`,
`src/game_native_resource_pools.cpp` and `src/game_main.cpp` change, alongside
this document and its report. No new Native body, data, handler, original image
or Ghidra state was opened or changed. No descriptor cell or owner type was added.

## Existing providers and actual storage

| Owner | Existing complete Source chain | Retained production representation |
| --- | --- | --- |
| Plain-node `0108FF58` | `bind_static_native_node_pool_0108ff58()`; `CD7D10` calls `B6E980`; real `std::atexit` registers `CE0E20`, which calls `B6E3D0`. | Separate aligned 0x38-byte process buffer; its `AllocatorListElement` lifetime starts before binding; pool construction remains in `B6E980`. |
| Group `010902F4` | `bind_static_native_group_pool_010902f4()`; `CD8460` calls `NativeGroupPool::initialize_00b8f190()`; real `std::atexit` registers `CE0F00`, which calls `destroy_00b8ec80()`. | Actual `NativeGroupPoolStorage` plus `NativeGroupPool` companion, borrowing the same retained list domain. |

The new members are appended after all previous members. No second list head or
allocator domain is introduced. The group companion binds its real `D634D4` /
`B8F270` trim operation before pool publication; the plain-node binder supplies
`D62C78` / `B6EA60`. Trivial header/companion construction alone leaves the
allocator list empty. The explicit provider initializer constructs each actual
critical section, reserves its table and publishes its distinct list element.

Plain-node and model-base reuse `B6E980`/`B6E3D0`, but have different buffers,
binding cells and CRT callbacks (`CE0E20` versus `CE0E60`). Both allocate 0x178-byte
slots containing 0x174-byte node payloads plus a trailing pool word. Group uses
0x18C-byte slots containing a 0x188-byte payload plus its own trailing pool word.
The model `01090054` pool remains another owner, with 0x188-byte slots.

The plain-node binder has an existing setup-once contract and no arbitrary
rebinding guard; it must not be invoked with another owner later. The new wrapper
binds its fixed buffer/domain exactly once. The unchanged group binder rejects
a different companion. Its future `NativeGroupEnvironment` must therefore borrow
`group_pool_010902f4()`; an independently constructed companion is not a substitute.
The existing resource graph environment also requires distinct actual plain-node
and model-base pools. This packet does not construct either environment.

## Real caller, states and lifetime

`WinMain` calls the new explicit methods in its current resource-pool startup
block, after accepted canonical data handoff and the VFS qualification early
returns, before constructing the application host. The Source sequence is:

`plain-node CD7D10 -> camera CD7DD0 -> mesh CD7E40 -> model CD7F00 -> model-base CD7F20 -> section CD8250 -> hierarchy CD82D0 -> group CD8460`.

The previous six-pool subsequence and unrelated particle-model `CD7830` startup
are unchanged. The two positions are explicit Source composition, qualified in
the caller comments; no new original CRT-table order is inferred from addresses.
Existing CMake registrations already include both provider implementations.

`initialize_plain_node_once_00cd7d10()` and `initialize_group_once_00cd8460()` use
the existing resource mutex and sticky startup states. Each marks `threw` before
binding or initialization, then stores the real registration status and marks
`returned` only after the call returns. A repeated returned call preserves that
status without rebinding, reconstructing or registering another callback. An
exception leaves the attempt sticky and later attempts throw. There is no retry,
rollback, dummy callback, replacement allocator or lazy pool construction.

`plain_node_pool_storage_0108ff58()` and `group_pool_010902f4()` reject access until
their initializer returns and then expose the same retained storage/companion.
A nonzero `std::atexit` result still leaves the provider initialized: its status
is cached and the accessor is available, although no CRT cleanup was registered.
The process C++ destructor does not invent cleanup for that path. Failure-return,
allocation-exception and asynchronous-fault paths were inspected in Source, not
injected or certified by this packet's successful probe.

The physical/list process is constructed before the resource process. Both C++
exit registrations finish before any explicit resource-pool callback is
registered. Successful callbacks therefore run with their original buffers,
companions and list alive. The relative reverse cleanup sequence is
`CE0F00 group -> CE0ED0 hierarchy -> CE0EC0 section -> CE0E60 model-base -> CE0E50 model -> CE0E40 mesh -> CE0E30 camera -> CE0E20 plain-node`.

The existing plain-node and group callbacks unlink their list elements and leave
their host trim-map entries until domain destruction. Model-base's existing
callback additionally unbinds its trim entry. This difference is preserved; no
shared or invented cleanup callback is used. Other application owners have their
own callbacks. All future payloads/companions must finish and return their raw
slots before pool callbacks free slabs and tables; pool destruction does not call
node/group destructors. Cross-owner list mutations remain serialized by the
application caller; the resource mutex is not a global list synchronization claim.

## Verification and admission boundary

The normal `scripts/build.ps1` MSVC Win32 Release build passed, as did both
configured CTests, `reconstructed_math` and `tool_tests`. This worker has no local
seed-reference header, so Native differential tests were not configured or
claimed; Root's earlier three-check Source581 result remains separate evidence.

One ignored local Source probe, compiled with `/MANIFEST:EMBED`, exercised the
concrete publication/CRT risk. It checked cold access rejection, eight actual
list elements in the Source sequence, distinct plain-node/model-base and group
storage, stable identities and original statuses on repeated startup, and raw
slot allocation/return through the real plain-node and group providers. Existing
model/model-base raw slots were also returned before exit. No node, group or model
payload was constructed. Eight interleaved real CRT checkpoints observed reverse
cleanup, ending with an empty still-live list before process/list destruction;
the executable returned 0.

The report pins the build/test and probe logs, probe source/compile command,
selected current Source inputs, frozen library/executable and seven selected COFF
objects with full symbol/relocation dumps. Expected Source symbols are present;
this selected inspection is not a full application graph or compiler include
closure. The game executable was built but not run for this packet.

Source581's startup failure and matching unchanged Source539 FMOD control are
historical evidence preserved without modification. This packet makes no claim
to resolve or reproduce that runtime failure. Production node/group environments,
factory/graph/load providers, descriptor consumers and terminal payload ownership
remain separate work. No model object, complete Native CRT startup, Native ABI,
original/source differential, successful Present/startup or gameplay admission is
claimed.
