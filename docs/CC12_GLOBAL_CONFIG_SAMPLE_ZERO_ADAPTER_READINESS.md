# GlobalConfig sample final-release adapter readiness

**A concrete post-decrement sample route already exists.**
`GameSoundRuntime::samples().zero_references_slot_00(actual_sample)` checks the
current `D5B074` identity and calls
`scalar_delete_sound_sample_00a82e80(sample, 1, sample_)`. Neither this runtime
method nor the scalar decrements the sample again. A domain-qualified effects
adapter can reuse this route after GlobalConfig's real `actual+4` decrement
reaches zero. This packet implements no adapter and adds zero Original functions.

This conclusion is conditional on actual sample provenance and retained service
lifetimes. A numeric `D5B074` stamp alone proves neither. The existing generic
`NativeGlobalConfigCurrentDispatch` needs genuine callable current-process
tables; it cannot directly interpret the Source sample's numeric identity.

## Existing producer and ownership chain

The current Source chain is complete enough to identify the intended domain:

1. `native_global_config_load.cpp` passes the owner's actual `+2C0` storage,
   an unchecked index and the ObjectiveSounds name to `008DBE90`.
2. `assign_global_config_sound_008dbe90` acquires through its borrowed current
   cache and `SoundSampleCacheContext`. It captures the old slot after acquire,
   publishes/retains the new pointer when different, releases the old value,
   then releases the temporary acquisition reference.
3. Canonical `D5B460` cache hits retain actual sample `+4`. A miss calls
   `create_sound_sample_00a85440`, which allocates a complete `0x7C` block through
   `singleton_lifetime_allocate` and calls the bound constructor.
4. `SoundSampleRuntime` supplies that constructor and its retained
   `SoundSampleContext`. Construction writes `D5B074` and reference count 1,
   creates real pooled strings/arrays, and loads the associated resource.
5. The cache map holds weak sample pointers. Its insertion does not retain;
   a reentrant duplicate can return a fresh sample absent from the map.
   Membership is therefore not a substitute for provenance or ownership proof.

The first array occupies indices 0 through 2. The loader and assignment do not
enforce that range: indices 3 through 5 can write samples into the stop-bearing
second array. A sample zero adapter does not make those stop calls valid.

## Native profile versus Source adapter

| Native position | Verified behavior |
| --- | --- |
| `D5B074 +0` | `BD30E0` reads the receiver's current table, calls current `+4` with flags 1, then returns. It performs no decrement. |
| `D5B074 +4` | `A82E80` destroys the actual sample, frees it for `flags & 1`, and returns its captured address. ECX receiver, stack DWORD flags, `RET 4`; no decrement. |
| Following `+8` word | `6F6F4C0A`, literal bytes `0A 4C 6F 6F`. This is not a sample stop method. |

The existing Source runtime supplies the corresponding final scalar call with
its real context. GlobalConfig remains responsible for its single decrement,
current-slot reloads and after-callback clearing. The sample destructor may
decrement the distinct owned resource at `sample+78`; that is not a second
decrement of the sample's counter.

The scalar's cleanup is substantive: optional FMOD group cleanup, removal of
the sample's weak cache entry, resource release, raw array and pooled-string
destruction, base stamp, and matching `singleton_lifetime_free`. It is not a
direct-free shortcut. Its allocator matches the sample factory's Source
malloc/free domain.

## Exact retained domain

`GameSoundRuntime::Impl` constructs its sample and resource runtimes using the
same current owner/cache publications, FMOD library, string storage, resource
accounting word and writable null-byte storage. Its public methods already
provide a consistent input binding: `lifetime_bindings().global_00f8bbe8`,
`samples().sample_cache_context()` and `samples()` can supply
`NativeGlobalConfigSoundContext`'s three borrowed fields.

At final release, `SoundSampleRuntime` reloads its current sample cache. Resource
final release can also need the current sound owner and resource cache. Retain
these objects and publications, the same strings, sample/resource runtimes,
FMOD library and handles, accounting words, and underlying services through the
last sample release. A different cache/runtime or an independently selected
default CRT string pool does not satisfy this producer's contract. Passing
sample options at `+8`, another subobject, `SoundOwnedResource`, a cache owner,
or projected `SoundInstance`/`SoundLevelEntry` storage is invalid.

`samples()` returns the retained C++ adapter without checking `started()`.
That accessor does not prove the cache or FMOD state is alive. The public sound
runtime contract requires external sample/channel references released before
shutdown; shutdown destroys the manager and cache. GlobalConfig's placement in
the full application drain still has to preserve that ordering.

## Proposed adapter boundary and rejected substitutes

A future effects adapter can borrow the proven canonical `SoundSampleRuntime`
and the existing current-table provider. Its zero callback can route an admitted
complete sample with current `D5B074` to the existing sample zero method; an
independently admitted raw object can use the genuine current `+0` method.
The containing domain must establish those admissions before numeric profile
dispatch. Unknown or foreign stamps are not automatically callable tables.

For every admitted second-domain object, stop must remain a genuine fresh
current-table `+8` call with the original flag. Do not install/copy raw tables,
invent a sample stop, ignore an unsupported call, or assume the second slots
are empty. The non-null second-array producer remains unresolved.

`GameSoundRuntime::delete_registered` accepts singleton identities, not samples;
a foreign sample reaches its error/terminate path. The retained-sample release
helper would decrement again, and `SoundChannelRuntime::release_reference`
both decrements and consumes projected channel storage. Neither is a zero
callback substitute. The dialog runtime already demonstrates the narrower
post-zero delegation to `core->samples().zero_references_slot_00`.

Sample runtime/scalar methods can throw, including for an unknown profile or
missing current cache/resource owner. GlobalConfig's interface is `noexcept`.
A future adapter requires a valid nonthrowing service domain; it must not catch
an error and report successful cleanup. General native EH/SEH equivalence,
application wiring, runtime shutdown safety and gameplay remain unproved.

## Evidence and validation boundary

At main `1694bac894b9335bf60f09f51e377650527df8a8`, 25 current Source files are
pinned with 45 bounded excerpts. Five retained native spans (449 bytes) were
replayed against the installed PE, including the complete v0 and scalar bodies,
sample assignment, aggregate destruction and table/literal data. Four existing
full-body hash records for factory, acquisition, constructor and destructor also
match the installed PE. No fresh native export or live byte query was needed.

The report distinguishes that retained native evidence from current Source
inspection and PE replay. No Source changes, Ghidra writes, builds, tests,
probes or game execution occurred. Evidence:
[cc12_global_config_sample_zero_adapter_readiness.json](../reports/cc12_global_config_sample_zero_adapter_readiness.json).
