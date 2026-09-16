# Native sampler loader context R42

## Scope

R42 adopts the historical sampler loader interface needed by later system
constant, descriptor sampler, and sampler cache work. The current repository
already contains the complete raw `004DE4B0` implementation as
`get_native_particle_clock_singleton_004de4b0`. R42 reuses that provider through
a narrow persistent operation-frame adapter. It does not compile a second copy
of the getter body.

The packet also restores the complete `00B1B810` resource-factory forwarder.
That function has no current source equivalent and remains a direct two-provider
implementation.

No application publication, owner construction, raw-manager alias, deletion
binding, cache-loading entry, or renderer fallback is added. Callers must supply
the actual `01090AA0`, `00F8D420`, and `00F8D41C` cells and retain them for the
whole operation-frame lifetime.

## Original routines

### `004DE4B0` raw singleton getter

The exact installed-PE body is 199 bytes at `004DE4B0..004DE576`, SHA-256:

```text
fa1f9c042b0a54c37c68547e0fed4dc02526fdfe6a8911e5b66e46ed9d541ec2
```

Fresh guarded Ghidra bytes match the installed PE. The body contains 56
instructions and these direct calls:

| Site | Target | Meaning |
| --- | --- | --- |
| `004DE4D9` | `00415350` | first actual raw-manager lookup |
| `004DE50A` | `00BF681B` | allocate the 1Ch owner |
| `004DE53F` | `00415350` | current raw-manager lookup after publication |
| `004DE54D` | `00BD0C30` | register the current publication |

The original entry consumes no arguments, returns the actual 1Ch owner in EAX,
and uses a plain `RET`. It fast-returns a captured nonnull `00F8D420`. On a miss,
it captures the first manager's section at +10, enters it, increments physical
depth +18, rechecks the publication, allocates and initializes the owner, then
publishes it. It resolves the current manager again, registers a fresh
publication read, releases the captured section, and returns one final
publication read. Registration failure retains an already-published allocation;
the native guard releases the captured section without rollback.

The established current provider implements that complete raw schedule and has
already been source, native-span, object, and fixture reviewed. R42 delegates to
it instead of restating the algorithm.

### `00B1B810` resource factory forwarder

The exact installed-PE body is 20 bytes at `00B1B810..00B1B823`, SHA-256:

```text
8a65e02675980fa957b1a0e9d36c98b9b89afb43e7d2ee3f84189a5421809ce8
```

Fresh guarded Ghidra bytes match the installed PE. Its six instructions contain
two direct calls:

| Site | Target | Meaning |
| --- | --- | --- |
| `00B1B810` | `00B1B730` | obtain the actual `00F8D41C` registry |
| `00B1B81C` | `00B19E90` | perform the existing qualified lookup/creation |

Incoming ECX and the second stacked options argument are unused. The first
stacked name is forwarded, the creator result is returned unchanged in EAX, and
the original entry uses `RET 8`. The source interface exposes its required
manager, registry cell, and factory context explicitly. It adds no retain,
fallback, null substitution, cache insertion, or factory profile.

## Actual storage view

`NativeSamplerLoaderSingletonStorage` is a typed view of the same 1Ch bytes as
`NativeParticleClockStorage`:

| Offset | Sampler view | Raw provider view |
| --- | --- | --- |
| `00` | primary profile | `profile_00` |
| `04` | cache profile | `profile_04` |
| `08..13` | `NativeResourceRecordVectorStorage` | `word_08..word_10` |
| `14` | owner word | `word_14` |
| `18` | sampler time float | untouched `payload_18` bits |

Win32 static assertions require identical 1Ch size, alignment, and offsets
before the adapter can reinterpret the borrowed publication and returned owner.
This is a raw-layout view. It does not start, replace, or own a second C++
lifetime.

## Persistent operation frame

The historical operation frame stored internal getter snapshots such as first
and current manager, allocation, registration argument, and native guard state.
Current and pending production consumers never read those fields. They embed the
frame, pass it to the call, and check whether its phase remains `running` or
`failed` before diagnostic retirement.

R42 therefore retains only state proved at its public source boundary:

- one-shot phase (`fresh`, `running`, `complete`, `failed`, or
  `diagnostic_retired`);
- selected function and current adapter/direct-call site;
- borrowed manager and owner/registry publication-cell addresses;
- resolved registry, final result, source name, and lookup context when those
  values are directly observed by the `00B1B810` implementation.

The `004DE4B0` adapter records its entry boundary and success/failure. It does
not fabricate native internal sites or snapshots around the opaque delegated
call. Real captured-section guard unwind remains inside the established current
provider.

Destroying a `running` or `failed` frame terminates. A failed call rejects
replay. `acknowledge_diagnostic_cleanup` changes a failed frame to
`diagnostic_retired` only after its caller has resolved retained native storage
and publication obligations. It frees nothing, unregisters nothing, retries
nothing, and does not authorize owner retirement while the frame is active.

## Current scalar deletion closure

Current main contains the genuine direct owner cleanup chain:

- `004DE340` flags-based scalar deletion;
- `004DE360` secondary deleting thunk;
- `00B1B680` owner destruction;
- `004DE290`, `004DDA40`, and `004DDAA0` record cleanup;
- `004B4F10` unconditional `00F8D420` clear and base-profile transition.

R42 installs no application owner and no raw-manager deletion binding. The
focused fixture uses the current real unregister provider followed by
`delete_native_particle_clock_004de340(..., flags=1)` to retire empty cold
owners, then drains the genuine managers. This proves that scoped cleanup is
available without implying production host admission.

## Validation

The strict MSVC Win32 Release build and all three current CTests pass. Native
seed verification passes. The fresh module object is x86 and requests
`MSVCRT` (`/MD`). Its undefined references prove the intended composition:

- the sampler getter adapter calls
  `get_native_particle_clock_singleton_004de4b0`;
- the factory forwarder calls only
  `get_native_resource_registry_00b1b730` and
  `create_native_registered_resource_00b19e90`;
- it does not reference the raw allocator, registration wrapper, or singleton
  guard, so it cannot contain a second getter implementation.

The ignored R42 fixture is an adaptation of the historical loader fixture. It
is not represented as an unchanged historical result. It executes copied
original getter and forwarder bytes while rebinding their six direct calls to
the established current providers and using the real Win32 critical-section
imports. Four normal getter schedules plus factory creator/miss match for 84
trace words. The adapter retains public failure state across allocator and
registration failures, releases the delegated raw guard, rejects replay, and
terminates with exit 77 when a failed frame is abandoned. Empty owners use the
current genuine unregister and scalar cleanup chain.

The registry remains producer-layout fixture storage with the original
read-only profile bytes. Original child bodies and original native FH3 paths do
not execute. The fixture executable is x86, `/MD`, and has an embedded
manifest.

## Evidence boundary

This packet establishes a typed source adapter, the complete source
`00B1B810` forwarder, exact native bytes/calls, build/link correctness, and the
focused component schedules above. It does not establish original binary ABI,
native FH3/SEH or hardware-fault identity, multithreaded race parity, a full
`B1A4F0`/`B1B4D0` cache route, descriptor-sampler closure, application owner
admission, raw-drain deletion binding, renderer behavior, game execution, or
gameplay.
