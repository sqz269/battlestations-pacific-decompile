# Production model and model-base pool startup

The Source application now starts the existing `01090054` model pool and the
separate `0109008C` model-base node pool through its retained production resource
pool process. Both use the actual existing `00E188B4` allocator-list domain.
This closes the pool-owner/startup prerequisite identified in the
[consumer readiness audit](CC12_MODEL_DESCRIPTOR_CONSUMER_BINDING_READINESS.md).
It does not connect a production model-ID consumer or construct a model object.

The packet starts from `38700ffd4` and changes three Source files:
`include/bsp/game_native_resource_pools.hpp`,
`src/game_native_resource_pools.cpp`, and `src/game_main.cpp`.
The providers, descriptor storage, build registration, ledgers and Ghidra are
unchanged. No new Native body, data, handler or original image was opened.

## Actual storage and caller

`GameNativeResourcePoolProcess` appends a `NativeModelPoolStorage` and its real
`NativeModelPool` companion, plus a distinct aligned 0x38-byte model-base buffer.
Appending preserves the old members' layout; this is not a binary ABI claim.
The constructor establishes an `AllocatorListElement` lifetime in the latter
buffer before the existing binder takes its reference. That trivial lifetime
does not construct a pool, publish list links, or initialize a critical section.
The unchanged `00B6E980` Source provider performs those operations at startup.

| Retained owner | Explicit initializer | Existing initializer and CRT cleanup |
| --- | --- | --- |
| Model `01090054` storage/companion | `initialize_model_once_00cd7f00()` | Bind actual companion, call `initialize_static_native_model_pool_00cd7f00()` / `00B74B80`, register real `std::atexit` callback `00CE0E50` / `00B74690`. |
| Model-base `0109008C` raw pool | `initialize_model_base_once_00cd7f20()` | Bind actual buffer and same list, call `initialize_static_model_base_node_pool_00cd7f20()` / `00B6E980`, register real `std::atexit` callback `00CE0E60` / `00B6E3D0`. |

The real executable caller is `WinMain`'s existing pool startup block, after
canonical data handoff and the two VFS qualification early-return branches,
before construction of the application host. Its resource-pool sequence is:

`camera CD7DD0 -> mesh CD7E40 -> model CD7F00 -> model-base CD7F20 -> section CD8250 -> hierarchy CD82D0`.

This preserves the previous camera/mesh/section/hierarchy subsequence and all
four providers. The unrelated particle-model `CD7830` startup remains unchanged.
The two inserted calls are explicit Source composition: neither address order
nor this placement is new evidence for the original executable's CRT table.
The initializers use no descriptor cell, model ID, factory or model environment.

`model_pool_01090054()` and `model_base_pool_storage_0109008c()` expose these same
retained owners only after their corresponding startup call returns. A model
slot is 0x188 bytes, including its pool-owned word after the 0x184-byte payload;
a model-base node slot is 0x178 bytes, including its word after the 0x174-byte
payload. Neither raw slot is a constructed owner. `0108FF58` plain-node storage,
particle-model storage, material parameters and the two new pools remain distinct.

## Publication, failure and exit

Both methods use the existing resource-process mutex and `unattempted`,
`returned`, `threw` state convention. Each commits `threw` before binding or
initialization. Successful return caches the actual registration status;
repeated calls return that status without rebinding, initializing or registering
another callback. An exception leaves the attempted state sticky; later calls
throw and the accessor stays unavailable. No wrapper rollback, retry, status
replacement or fallback allocator was added.

An actual nonzero `std::atexit` result still means initialization returned: the
pool stays initialized, the status is cached and the accessor remains usable.
There is no registered native cleanup in that case. The C++ process destructor
does not compensate for registration failure or repeat native teardown. These
failure contracts are established by Source inspection; registration failure,
allocation failure and asynchronous faults were not injected in this packet.
The lower providers retain their existing unwind and allocation behavior.

The physical process containing the list is constructed before the resource
process; both C++ exit registrations finish before explicit pool startup.
On ordinary exit, successfully registered resource callbacks therefore run
while their buffers, companion and shared list are alive. Their relative cleanup
order is hierarchy `CE0ED0`, section `CE0EC0`, model-base `CE0E60`, model `CE0E50`,
mesh `CE0E40`, camera `CE0E30`. Other application owners have their own callbacks.
The model-base callback also removes its host trim binding; the existing model
callback unlinks the native list element and leaves its companion's map entry
until the shared domain is destroyed. This unchanged bookkeeping never adds a
second pool or list. Startup across separate process owners remains serialized
by the application caller; the resource mutex is not a new global list lock.

All model/node payloads and companions must finish and return their raw slots
before these callbacks free slab/table storage. The pool destructors do not
invoke payload destructors. No current production model borrower was introduced,
so this remains a requirement for the future actual consumer composition.
If a C++ exception reaches the existing `WinMain` startup catch, it logs and
returns 1; callbacks already registered remain active for ordinary process exit.

## Validation and remaining boundary

`scripts/build.ps1` completed using MSVC Win32 Release. Both configured CTests
passed: `reconstructed_math` and `tool_tests`. The local seed-reference header is
absent, so this worktree did not configure `native_math_differential`; no Native
seed was opened or generated for this Source-only packet.

One ignored local `model_pool_probe.exe`, linked with `/MANIFEST:EMBED`, exercised
the concrete retention/CRT risk using the built Source library and real CRT:

- Cold accessors reject use and companion construction leaves the list empty.
- The six actual resource pools initialize in the application subsequence.
- The new pools have distinct storage, stable identity and unchanged statuses
  after repeated calls, without duplicate list insertion or exit registration.
- Each new pool allocates and accepts a returned raw slot; no model object is
  constructed and no descriptor or game resource is supplied.
- Six interleaved CRT checkpoints observe cleanup in reverse order, followed by
  an empty live allocator list before process/list destruction. Exit code is 0.

The JSON report pins the final build/test log, probe source/build command and
execution log, selected provider inputs, current library/executable and six
selected COFF objects plus their complete symbol/relocation dumps. These dumps
confirm the expected Source symbols; they are not a complete application graph
or instruction-equivalence audit. The game executable was built but not run.

The missing production resource graph, loader/cache, model/group environments,
other pools/type providers, actual class/unit receiver and terminal payload
lifetimes remain as described in the consumer readiness audit. No model-ID
consumer, complete Native startup order, Native ABI, original/source differential,
renderer/gameplay, startup-Present or model-construction admission is claimed.
Source509/511/534 evidence remains immutable historical evidence; this packet
does not relabel their objects or results as current.
