# GlobalConfig raw binding and canonical Sound storage readiness

This is a read-only Source audit at `6ccb74bca52c03d3e9188520c14a5f03f08546d9`. Two bounded Source edits are admissible from established contracts: reuse the existing raw/semantic lifetime access in GlobalConfig, and explicitly carry the exact VFS string storage through Sound construction and its remaining default-CRT allocation sites. Neither edit closes the complete raw GlobalConfig owner. Its second effect array lacks a positively attributed concrete raw producer/profile and stop/final-release implementation. Raw GlobalConfig, complete Sound integration, and full application construction remain **UNREADY**. No Source, CMake, tests, packet state, ledgers or GPR were changed; no build, probe, new Native recovery or game run occurred in this packet.

## Existing normal GlobalConfig behavior

`src/global_config.cpp:124` already implements the complete normal `00432650` getter. It has the captured fast return, first manager/critical-section acquisition, guarded second slot check, actual `2E8h` allocation, allocation-preimage preservation around the C++ shell, construction, publication, second manager lookup, current publication reload for registration, captured guard exit, and final publication reload. This is a binding problem, not a missing getter body.

`004324E0` writes its `CE3D98` identity, five vector triples, seven string headers and the embedded `+2C0` effect container without clearing the rest of the allocation. `004325B0` destroys the embedded effects, seven strings in reverse order, the raw vector buffers, and the captured forward name range; it reloads the name buffer before freeing, zeros the first triple, unconditionally clears `F878E4`, and writes base `CE3818` last. `00432710` calls that destructor and frees the original shell only for `flags & 1`. There is no manager-unregister operation in these paths. Reusing these bodies must preserve those identities and callback-sensitive reads.

The four current `GlobalConfigContext` inputs have these producers and gaps:

| Input | Established Source | Application binding still needed |
| --- | --- | --- |
| `lifetime` | Currently `SingletonLifetimeDomain&`; complete raw/semantic `SoundLifetimeAccess` and `CapturedSoundLifetimeSection` already exist. | Borrow the same actual `GameNativeStringProcess.manager_01090aa0` cell used by `GameSingletonHost`; never reinterpret the semantic manager or create another domain. |
| `singleton_00f878e4` | Getter/destructor already operate on the caller's actual volatile pointer cell. | Retain one application cell and context; there is no current production GlobalConfig context/cell composition. |
| `strings` | Existing `NativeStringStorage&` contract and actual pooled VFS storage. | Borrow the **exact** `app.vfs_->borrow_raw_services().strings` object for both GlobalConfig and Sound. Same backing globals behind a different wrapper do not satisfy the existing identity guard. |
| `effects` | Complete abstract callback contract and exact embedded destruction order. | First-array sample final release has an established provider. Second-array concrete raw producer/profile and both necessary virtual behaviors are not closed. |

`NativeGameConstructionCalls::call_00432650` already calls this getter, and its `call_0087D7B0` runs the complete existing loader on that same owner. These calls do not manufacture missing contexts.

## First bounded Source proposal: lifetime access only

The smallest compatible edit is to store an existing `SoundLifetimeAccess` by value in `GlobalConfigContext` and use `CapturedSoundLifetimeSection` in the getter. Its implicit semantic-domain constructor preserves ordinary four-field aggregate initialization; the raw constructor borrows an actual `void* volatile&` publication cell. It allocates no private manager. Existing raw access calls established `00415350` and `00BD0C30`; the captured guard reads actual manager `+10h`, enters that Win32 section, adjusts actual recursion `+18h`, and exits that same section even if publication changes.

Keep the getter's second manager resolution in its own statement **before** reading `singleton_00f878e4` as the registration argument. Keep the first captured section alive through that registration and destroy it before the final slot reload. Preserve the allocation-preimage sequence and all constructor/destructor bodies. The likely owned files are only `include/bsp/global_config.hpp` and `src/global_config.cpp`; a neutral naming wrapper is optional and not a reason to duplicate the established implementation. This edit can be compiled and checked with existing coverage, but must not admit a production raw GlobalConfig owner yet.

The current `NativeSingletonDeletionBindings` and dispatch switch do not have a GlobalConfig context/`CE3D98` case. Once a retained complete context with concrete effects exists, the Source route is straightforward: dispatch the popped **actual owner** and incoming flags to existing `scalar_delete_global_config_00432710`. It must not substitute the current publication value, introduce unregister, weaken the unsupported-profile guard, or route base `CE3818` as though it were the derived profile. Adding this route now would expose the unresolved callbacks.

## Exact Sound string binding and construction order

`src/game_hosts.cpp:1358` currently passes `crt_string_storage()` into `GameSoundRuntimeServices`. That selects a real independent malloc/free string domain. `GameSoundRuntime::Impl` retains the services first, then constructs configuration, resources, samples, instance, shutdown and file services from `services.strings`. `SoundSampleRuntime` initializes its cache and sample contexts from that same argument. Therefore the application must supply the exact VFS `ActualNativeStringPoolStorage&` at this constructor argument, **before** any owning member is built. Do not copy headers, rebind a service after construction, or weaken the loader guard.

`GameNativeStringProcess::strings()` is another storage wrapper over the same process globals; that alone is insufficient for `&sound.cache.strings == &load.strings`. The stable canonical object for this composition is the one returned by the live VFS raw-services borrow. VFS construction/startup precedes SoundServices construction; native Lua services already bind those VFS strings. SoundServices attaches its dialog provider and installs the runtime in `GameSingletonHost` before `core.startup()`. Runtime/cache acquisition requires successful startup and the actual published cache; an audio-disabled option does not justify a fake/null cache or invented owner.

Changing the application argument fixes the sample-cache identity mismatch, but three established Source sites still choose CRT defaults. The second bounded Source packet should close all of them explicitly:

| Site | Current allocation domain | Compatible proposed change |
| --- | --- | --- |
| `sound_startup.cpp:40`, `create_sound_resource_owner_00a858f0(resources)` | Factory's existing default CRT storage for its temporary resource path. | Pass the runtime's strings explicitly to the existing storage-taking factory. This is currently a consistent temporary allocation/free pair, not evidence of a mismatched free. |
| `sound_configuration.cpp:70`, `NativeName`, used for listener/group names in SoundTypes | Hardcoded CRT construction and destruction. | Give the local RAII helper a storage reference and use it for both operations; pass the explicit runtime storage at both owning temporary sites. |
| `sound_configuration.cpp:321`, `create_sound_class_00a7c350()` | Existing factory defaults to CRT; descriptor retains its chosen storage. | Pass the explicit runtime storage to the existing factory so later name destruction uses the same domain. |

Thread an explicit final `NativeStringStorage&` through `construct_sound_system_00a88770`, `initialize_sound_configuration_00a7ff80`, and `apply_sound_configuration_lua_00a7ff80_fragment`, with existing CRT defaults/compatible overloads for ordinary callers. Pass `impl_->services.strings` from the runtime startup call. This preserves existing standalone/semantic clients rather than globally changing their defaults. Likely files: the Sound startup/configuration headers and implementations, `src/game_sound_runtime.cpp`, and `src/game_hosts.cpp`. Existing resource/sample runtime contexts already receive the runtime argument; do not change every unrelated default in the Sound API.

The current runtime already exposes all three `NativeGlobalConfigSoundContext` inputs: `core.lifetime_bindings().global_00f8bbe8`, `core.samples().sample_cache_context()`, and `core.samples()` as `GameplayEffectComponentLifetime`. No replacement cache or extra accessor is needed. These are stable references into the retained Sound runtime, not a claim that the current full GameApplication has been composed or executed.

## Effect payload boundary requiring Native attribution

The GlobalConfig release helper first executes a real `InterlockedDecrement(actual_object + 4)`, then invokes `zero_references_slot_00` only when it reaches zero. `008DBDB0` visits the three pairs in ascending order: capture second pointer, stop it with flag zero, **reload** that second pointer for release, clear it, release/clear the first pointer; later reverse passes preserve callback repopulation. Both callbacks therefore require a concrete actual object contract, including mutation behavior.

For an established actual `D5B074` sample, `SoundSampleRuntime::zero_references_slot_00` checks that current profile and calls existing `00A82E80` with deleting flags. It does not decrement again. This is a usable first-array payload implementation after canonical strings and actual cache are bound.

The projected channel/event implementation is not a substitute for the unresolved second array. `sound_instance.hpp` explicitly says those C++ projections are not native layouts and forbids raw `object+4` operations. `SoundInstance` has projection base state before its named native-profile/reference fields. `SoundChannelRuntime::release_reference` also performs its own logical decrement, so calling it after the GlobalConfig decrement would double-decrement. Its known profile dispatch and stop functions do not establish that an actual GlobalConfig second slot contains any of those raw profiles.

No current production Source producer is positively attributed to the actual second-array slots (`owner+2CCh`, `+2D0h`, `+2D4h`). Existing `008DBE90` performs unchecked indexed assignment from the `+2C0` base; the normal loader iterates ObjectiveSounds values with an incrementing index. Neither shape nor an assumption of three authored values proves second-array concrete profiles. Empty fixtures or presently empty slots cannot justify no-op callbacks.

Future Astra work should start at the **known consumers** `008DBDB0` (current virtual `+8` stop and release reload) and `004C3810`/`00524180` (actual decrement/current virtual `+0` final release), then trace the actual producer, current profile and exact callees. `00A7A570`/`00A7D5A0` are existing projected Source candidates, not attributed raw targets. The missing producer's address is not yet established; this audit does not invent one or treat `00871BA0` as proven by an older general acquisition follow-up. No new register, x87, ABI, EH or control-flow recovery was attempted here.

## Remaining loader composition and lifetime

`NativeGlobalConfigLoadContext` needs five live fields. `strings` is the exact canonical VFS object; `bootstrap` and `files` are available from retained `GameNativeLuaServices::bootstrap()` and `binding().files()`; the Sound context uses the existing runtime views above; `fov_divisor_00f889b4` must reference the actual settings owner `+34h` storage initialized by the current settings producer. The FOV constant/divisor or PE zero bytes are not a substitute for that cell. A retained view/accessor is a Source composition task; no new floating-point derivation is required to identify this existing storage.

The loader's current `bootstrap.do_file == files.do_file` and exact cache-string-storage guards stay intact. Interpreter entry must retain `NativeLuaServiceBindings::Activation`, and Lua/failed construction state must remain alive through the existing unprotected failure boundary. This audit does not add a protected Lua call or claim original EH compatibility.

Current shutdown drains the raw singleton manager while Sound, Lua and VFS still exist, resets Sound before deleting the manager, and deletes VFS after native Lua services. A future retained GlobalConfig context/effects provider must survive the complete manager drain and failed-construction cleanup while every referenced storage/provider is live. The process AA0/AA8/AA4 cells persist, but that does not extend the lifetime of the exact VFS wrapper or the Sound runtime. Add any route only after checking this retained-owner order.

## Evidence qualification

The ignored artifact root is `local/cc12_game_global_config_raw_binding_readiness_20261008a`. It contains complete current source/build-input copies, manifests, qualified existing export triples, and whole earlier archives, not just selected snippets or SHA lists. The companion report records hashes and machine-readable proposals.

All **4,115** inputs from the previous normal build snapshot still match byte-for-byte. The retained Win32 archive at build head `94e38c65ebfc4bbfffc1e027c2ab96da7742a266` has **6,985** verified members, including complete libraries/executables and **20** separately retained relevant whole COFF objects. That prior build ran `./scripts/build.ps1` on 2026-10-08 at 22:53:21–22:54:03 UTC and passed all three existing checks. This packet did not rerun it; unchanged inputs and archive verification are provenance, not execution of the proposed edits.

Historical R116 and R117 whole archives were copied and verified against their report manifests: respectively **167** and **376** artifact members, plus one internal `manifest.json` metadata member each. Their recorded UTC times are 2026-09-18 00:13:22 and 00:53:07. They establish the recorded field-assignment/vector and full-loader fixtures at those earlier revisions. Controlled sample/lifetime fixture providers do not establish an actual FMOD voice, raw second-array callback, full application, original EH or gameplay.

All **26** retained function export triples already existed and identify saved project `bsp`, program `/battlestationspacific.exe`; this is saved metadata verification, not a new live Ghidra/PE check. The older GlobalConfig and Sound integration reports are retained as historical context. Older phase flags and source hashes must not override the current Source wiring. No new native differential result, ABI compatibility, startup or game validation is claimed.
