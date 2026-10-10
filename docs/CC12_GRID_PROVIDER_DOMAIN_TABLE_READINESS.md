# Grid provider allocation and table readiness

The smallest default-service correction is ready as a separate implementation
packet: change only `NativeGameGridCpuCalls::allocate_00bf55be` to the canonical
singleton allocator and `NativeGameGridCpuCalls::free_00bf65ac` to canonical
singleton free. Preserve `free_00bf6989` as `std::free`. This establishes matching
source allocation/free families for the reviewed default paths. It does not
establish a callable grid table, original allocator equivalence, or permission
to construct or activate a game.

This packet changes only documentation and its report. The preceding frame
owner proof at `6a80adb7ff76a3dbd3440666d19a776fc1f7dc42` remains unchanged.
Source is captured at `317d42c18c2675ec59c63a431a1a0bd6f339ed6a`, after merging
Main `f31b71746c038e3fde22d50474f46d7d10012136`. The complete grid implementation
and header, renderer grid graph, construction call sites and current-dispatch
interface were reviewed. Searches cover all 4,208 tracked `src/` and `include/`
files plus eight tracked test files; those test files were read only.

## Exact allocation and release paths

| Storage | Reached allocation | Reached release | Default-service result |
| --- | --- | --- | --- |
| Outer grid, `84h` | Three `NativeGameConstructionCalls` sites inherit `NativeGameArrayCalls::allocate_00bf681b(0x84)`, which calls `singleton_lifetime_allocate({object,n,n})` | Parent lifetime requests current slot zero with flags `1`. A genuine grid scalar target must reach `delete_native_game_grid_0070b280`; after destruction, bit 0 set in the low flags byte reaches `c.cpu.free_00bf65ac(grid)` | Allocation is canonical `std::malloc`/new-handler policy; current grid CPU free is `::operator delete`. Change the grid CPU free default to `singleton_lifetime_free`. The current slot is not yet supplied by a genuine source grid table. |
| Six CPU buffers | `initialize_grid_body` calls the current `c.cpu.allocate_00bf55be` six times and stores each result at `+08,+0C,+10,+14,+18,+1C` | `release_native_game_grid_cpu_arrays_00709760` reloads each current non-null field and calls `calls.free_00bf6989` | Current allocation is `::operator new`; release is `std::free`. Change only the allocation default to canonical allocation; retain the existing release. |
| Renderer streams | Actual vertex/index creation, mapping and owner registry services | Grid destruction releases current vertex `+78`, then current index `+7C`, through the existing actual-owner registry | Separate existing renderer domain. Neither proposed allocator change redirects these services. |

The three outer allocation sites are `004DDFD3`, `004DE008` and `004DE041` in
the existing source reconstruction. Their reached constructor calls are
`004DDFF0`, `004DE029` and `004DE062`; after each returns, the corresponding
publication is `00E19B0C`, `00E19B08` or `00E19B04`. These are source labels,
not newly queried Native evidence. The default constructor adapter requires a
non-null `NativeGameGridContext` and invokes the concrete grid constructor.

`GameGridGraph` owns an ordinary `NativeGameGridCpuCalls cpu` and binds that same
object into its `NativeGameGridContext`. It does not derive an allocator override.
`GameNativeRendererApplication` retains this graph and exposes its context by
reference. Its checks qualify the renderer/string/geometry domain; they do not
make `operator new`, `operator delete` and `std::free` interchangeable.

The parent/container free-default correction already accepted elsewhere does
not override this separate grid CPU service. The similarly named physics
allocation/free methods also belong to their own memory-callback domain.

## Null, size and exception contracts

The correction must preserve these caller-level behaviors:

1. The outer constructor path checks the result of each virtual allocation.
   A custom allocator returning null skips that grid constructor and publishes
   null for that slot. The canonical default itself loops until `std::malloc`
   returns non-null or `_callnewh` returns zero, at which point it throws
   `std::bad_alloc`. A throwing handler also escapes; no new catch is proposed.
2. The grid constructor initializes the six CPU fields to zero, supplies a
   `16 x 16` descriptor, and then initializes the body. Without callback
   mutation, each buffer request is `256 * 12 = 3072` bytes. The two explicitly
   supplied descriptor preimage words are copied before those observable
   allocations and must remain in the same sequence.
3. The general initializer first stores a 32-bit dimension product at grid
   `+4`. Each of six iterations independently reloads that word, multiplies by
   twelve in 64 bits, and passes either the result or `0xFFFFFFFF` when too
   large. A callback may change a later request by mutating that live word.
   There is no new overflow rejection, cached request size, or skipped call.
4. A zero product still makes all six virtual allocation calls with size zero.
   The proposed default forwards zero unchanged to the canonical service:
   `std::malloc(0)`, then the same new-handler/retry policy if it returns null.
   Do not normalize zero to one, return a fabricated pointer, or add an early
   return. This intentionally selects the established canonical source policy;
   it does not prove identical zero-size or replaceable-global-`operator new`
   behavior in every CRT configuration.
5. The initializer stores every allocation result, including a null returned
   by an override, and continues. It does not validate those pointers before
   later geometry accesses. Valid backing storage and dimensions remain caller
   obligations. Zero-size forwarding is not evidence that a zero-dimension
   grid is safe to initialize. The generic initializer also does not release
   old CPU fields before overwriting them; this correction does not admit
   reinitializing a live owner.
6. CPU cleanup visits fields `+08` through `+1C` in order, reading each field
   immediately before its call. Null fields skip `BF6989`. Only after all six
   visits return does a separate loop clear all six fields. An override may
   mutate later fields; those new values are observed. A throwing override
   retains earlier effects and skips the clearing loop. Duplicate, stale or
   foreign pointers are not repaired, and blindly retrying can repeat a free.
7. Grid destruction stamps numeric `00CFD484`, releases CPU buffers, then
   releases the current vertex and index owners. Each stream field is cleared
   after its release returns. Scalar deletion always runs destruction; only
   bit 0 set in the low flags byte requests outer free afterward. A prior exception skips
   outer free. The returned value is the original grid pointer value even when
   its allocation has been released; it is not permission to read that memory.
8. The direct free service accepts null through canonical `std::free`, but
   `delete_native_game_grid_0070b280(nullptr,...)` is not a null-safe operation:
   destruction accesses the receiver first. Parent lifetime already guards
   null publications. Do not add a new receiver guard or suppress virtual calls.

`NativeGameGridOperation` retains the actual owner, context and partial
declaration/stream/map state. A caught initialization failure marks it failed;
it does not roll back CPU allocations or free the outer object. Its destructor
terminates for running/failed states. Diagnostic acknowledgement changes the
phase only after externally resolved cleanup; it performs no release. All
borrowed services must survive the retained operation. The two default-method
changes neither alter this schedule nor establish original FH3/SEH behavior.

## Current callers and override obligations

The complete identifier searches find one concrete default CPU-service owner,
`GameGridGraph`, and no named derived grid CPU service in the captured project
and test corpus. The concrete grid constructor is reached by
`NativeGameConstructionCalls::call_0070bd70`. The public initializer has no
additional captured caller. CPU release is reached by grid destruction, and
grid destruction is reached by scalar deletion. The scalar-delete identifier
has its declaration and definition, with no captured direct external call.
These lexical results are fully retained; they do not rule out foreign clients,
dynamic dispatch or future overrides.

The three methods remain virtual with unchanged signatures. Do not add
`noexcept` to their declarations, bypass a current override, or globally replace
similarly named methods in other services. A custom internal allocator must
pair with its corresponding `free_00bf6989`; a custom outer allocator must pair
with the corresponding CPU `free_00bf65ac` reached by its actual scalar target.
Those two pairs can use different domains if each pair is independently valid.
Overriding only the allocator while inheriting an incompatible free is not
qualified. A caller explicitly invoking a base default assumes that default's
canonical-storage contract.

The CPU service, its referenced grid context, renderer/string services and any
override state must outlive all six allocation/free calls, stream retirement
and any retained failure. Changing the default implementation does not extend
those owners' lifetimes or validate externally supplied buffers.

## Ready implementation boundary

The reviewable proposed bodies are:

```cpp
void* NativeGameGridCpuCalls::allocate_00bf55be(std::uint32_t bytes) {
    return singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes});
}

void NativeGameGridCpuCalls::free_00bf65ac(void* allocation) {
    singleton_lifetime_free(allocation);
}
```

A subsequent implementation can use the existing local `U` alias and add a
direct `bsp/singleton_lifetime.hpp` include. Limit header changes to documenting
canonical default storage and the matching override obligations. Preserve
`BF6989`, all signatures, field layout, virtual dispatch, six-call schedule,
size arithmetic, geometry instructions, publication order, failure state and
scalar flags behavior. No CMake registration or new adapter is needed for this
source boundary. The later implementation should run the normal MSVC Win32
build and existing checks; this readiness packet runs none.

This is a source-family correction: canonical allocation directly uses
`std::malloc`, and both canonical free and the preserved `BF6989` directly use
`std::free`. No assumption that a C++ allocation can be freed by the CRT, no
Native address equivalence, and no unchanged compiled-body claim is needed.

## Separate callable-table ownership requirements

The CPU service's C++ virtual table is not the raw grid's primary method table.
The grid constructor and destructor still write numeric `00CFD484` into the
raw owner at offset zero. `NativeGameCurrentVirtualCalls::virtual_scalar`
forwards to `physics_virtual_scalar`, which reads the receiver's current table
and invokes the requested slot as `void* (__thiscall*)(void*, uint32_t)` on
MSVC Win32. For the three parent grid releases, the byte slot is zero and flags
are one. The parent clears each grid publication only after that call returns.

A future callable-table packet must establish all of the following before any
activation:

- A real source table whose slot zero has the exact current-dispatch call
  shape, with table and code lifetime covering every call. Copied numeric
  profile words and the CPU-service vptr do not meet this requirement.
- An actual grid owner and a proven mapping to its retained
  `NativeGameGridContext`, including the exact CPU and renderer/string services
  used by construction. The raw scalar helper has an explicit third context
  argument and an eight-bit flags parameter, so its ordinary C++ function
  address is not directly the required table entry.
- An explicit adapter contract that forwards the receiver and flags correctly,
  reaches the real scalar helper, returns its pointer value, and performs no
  receiver access after a flag-one deletion. A context registry, if eventually
  chosen, would need its own ownership and retirement proof; none is invented
  here.
- An evidence-backed table installation and destruction-state schedule. The
  current constructor/destructor stamps must be accounted for, including
  partial failure and flag-zero destruction. No post-construction pointer poke
  or automatic table restoration is admitted by this packet.
- Retirement of consumers before destruction and retention of the context and
  table owner through all callbacks and interrupted cleanup. Exceptions must
  leave publication clearing and partial effects in the established order;
  successful allocator pairing does not make failed receivers replayable.

These table obligations remain unready. They are independent of the two-method
allocator correction and are not an application/game composition proposal.

## Retained evidence

The finite bundle is `local/cc12_grid_provider_domain_table_readiness/`; its
manifest, exact source witnesses, complete queries and replay commands are in
`reports/cc12_grid_provider_domain_table_readiness.json`.

Four prior build epochs remain separate: Source3352-wide, worker Source3352
free defaults, primary Source3354 node and primary Source3354 free defaults.
The accepted primary particle epoch is a fifth, separately retained cohort:
`Source3354_particle_admitted_constructor_frozen.json`, SHA-256
`11b434980be42c6144d8083595ff0aaaeeebe4d324944a0fe6c0d05e9793c346`, from
`295b982af240a0e9e57a69531ce66c90e60a9134`. All its selected inputs match the
captured source after line-ending normalization. Each cohort retains its own
four historical build artifacts; none is new build or artifact-equality
evidence for this packet.

There are zero C++/CMake/ledger edits, Native queries, Ghidra writes, builds,
executed tests, runtime/OS probes or original runs. Portable and actual-Git
verification concern retained source evidence only.
