# Instance-generator application binding: lifetime addendum

Source-only decision at `e9396b4883232059bdad28753bc95403536403bf`
(Root Source749), captured from worker merge baseline
`daab70335fd9f92d14557b00b151eb1aabc4d94b`. This supplements the earlier
[binding readiness](CC12_INSTANCE_GENERATOR_APPLICATION_BINDING_READINESS.md);
its 501-file/750-pin baseline and evidence remain unchanged.

## Smallest next boundary

Propose a ready-only `const NativeInstanceGeneratorContext&
borrow_instance_generator_context()` metadata borrow. Do not expose an
application-owned `NativeInstanceGeneratorOwners&` yet. A search of all 4,205
tracked C++ source/header/include/test files found generator context/owner uses
only in the generator owner and mesh-subset loading interfaces/implementations.
The subset reader already requires `Owners&`; no application consumer currently
needs migrating. This proposal does not make the full subset reader ready.

The future private binding can retain the real context after all existing Impl
members, reusing the six provider inputs and identity checks from the original
readiness packet. Each borrow must check ready phase and the existing conservative
`!requires_process_retention()` predicate, and revalidate provider identity.
No application Owners metadata object is needed for this smaller interface.
Ordinary C++ construction of a context or the real Owners metadata class is
permitted in principle: the latter only allocates its Impl and empty list.
"No Native constructor" means no Native geometry/generator construction entry,
not a prohibition on those ordinary C++ metadata constructors.

This is an explicit contract, not a C++ capability barrier. A const context still
contains mutable references, including the serial cell; it can also be copied to
a mutable aggregate. Callers must not mutate/copy it to activate services. Giving
out const does not itself prevent the existing constructors or an external
Owners object from entering Native work.

## Concrete metadata lifetime contract

1. Retain the application, original singleton/VFS/Lua owners, mapped read-only
   data and canonical process providers through every borrow. The metadata adds
   no Native reference counts and owns none of those providers.
2. Use the borrow only for identity/metadata inspection in a serialized ready
   interval. End all derived views/copies before frame entry, device reset or
   recreation, shared drain, or provider destruction; reborrow and revalidate
   after a later ready transition. The backing C++ context may remain allocated,
   but an old reference is not permission to operate in another phase.
3. Do not call Native construction, attachment/finalization, companion
   registration, setters, release/deletion, or mutable geometry/layout services
   through this borrow. Do not change the serial, profiles, names or referents.
   Preparing this metadata invokes none of those operations and creates no
   generator operation frame or companion requiring retirement.
4. With that no-activation contract, this binding adds no process-retention
   obligation. Existing lifecycle guards still apply to other work. Their false
   result is not evidence that arbitrary external generator work is retired.

## Current branch audit

All entries below are Source observations, not execution or ABI proof. Detailed
file/line anchors and frozen complete files are in the report and evidence.

| Current path | What it does | Consequence for an escaped active generator domain |
| --- | --- | --- |
| Renderer Impl destruction | Accepts only prepared/drained phase, releases retained COM refs, then destroys members in reverse order. | No generator frame/Entries observation; remaining references can dangle. Destructor termination is not retention or proof of safety. |
| StartupHost destructor | Checks renderer retention, closes frontend consumers, drains singletons, deletes singleton/Lua/device owners, then renderer and VFS. | A false generator-blind predicate permits provider retirement. New metadata must not touch the already-deleted singleton host from its destructor. |
| Application shutdown and singleton-manager destruction | Check the same retention predicate; shutdown releases host device borrow; manager destruction calls renderer drain. | Checks cannot discover externally entered generator work. `GameDeviceHost::release()` only clears callback/borrowed pointer, not the renderer's COM ownership. |
| StartupHost exit_process | Same checks, then `std::exit` and CRT callbacks. | Missed obligations can reach process pool cleanup; this is not the no-cleanup retention exit. |
| Renderer drain_singletons | Checks current retention, sets ready to draining, calls raw singleton shutdown, then after_native_drain; failure marks failed. | Native teardown can start before any generator-specific check. |
| Public after_native_drain | Checks publications/queue/shader/resources/compiler/worker completion and sets drained; prepared/drained have early returns. | Post-drain observations cannot protect work already invalidated; no generator check exists. |
| Raw SingletonHost shutdown/fallback destructor | Can directly drain manager and retire VFS without the renderer predicate. | Must never be used to bypass the metadata/activation lifetime contract. Current direct game_main diagnostics concern separate VFS graphs. |
| Native renderer singleton terminal and B32920 | Deletes control worker, releases bindings/resources, flushes declarations/effects/textures, releases COM, destroys base/publication and optionally raw owner. | Ordinary C++ context survival does not preserve the Native renderer, caches, synchronization or backing owner. |
| Prepared/construction/startup failure | Metadata borrow requires ready; construction/device startup catch paths mark failed. | A valid metadata borrow cannot precede once-only startup; keep failed graphs under existing process-retention behavior. |
| Ready begin_frame / control worker | Native begin enters B2ABD0 reset; application follows a replaced COM device afterward. | begin_frame does not consult requires_process_retention; ready phase alone cannot block escaped work or protect its captured device. |
| B2ABD0 reset | Can release resources and COM Reset, restore them, or fall back to B29670; lost-device fallback has the current source hold-policy branch. | Even successful in-place Reset mutates resources. Native locks are not generator acquisition tracking. |
| B29E60 presentation change | On windowed-mode change calls B29670; otherwise sets pending reset. | Source interface exists but no current application caller was found. No generator observation protects a future caller. |
| Logical vertex/index private-buffer retry | Calls the application's ActualRecreation adapter on the native retry condition. | A mutable graphics provider can reach recreation outside begin_frame. |
| Texture load retry | Calls TextureLoadingGraph retry adapter for 2D/cube/volume failure branches. | Same bypass; 2D retry even retains its documented captured-device schedule. Do not substitute a new schedule. |
| Runtime surface/2D/volume factory retry | Directly calls retained B29670 context on eligible create failure. | These existing resource-provider routes also bypass an application generator predicate. |
| B29670 body / DeviceGraph destruction | Releases/rebuilds layouts, buffers, shaders, textures and device; DeviceGraph destruction unbinds recreation and unloads D3DX. | C++ references can stay valid during recreation while Native resources change. Interrupted frames have no established resume safety; destruction later invalidates the provider objects too. |
| Geometry/layout companion destructors, VFS retirement, mapped-data destruction | Companion destructors terminate on live/pending state; VFS retires native bindings; mapped-data destructor frees reservations. | Incidental detection or process-long scalar storage cannot extend the lifetime of all other providers. |

## Why mutable Owners is held

`requires_process_retention()` currently observes application phase, shader and
shadow state, pending section-layout acquisition and layout companions. It has
no generator operation list or Owners Entries observation. A pending/live layout
may incidentally make it true, but failure before layout or before companion
insertion is not covered. Generator creation can leave name/declaration/raw
state before registration. Registration inserts a private Entry before canonical
bind; bind failure can leave an unbound Entry. The Owners destructor terminates
when that list is nonempty and exposes no public empty/quiescent query.

The actual canonical registry has `size()`/`empty()`, but it spans other families
and cannot see pre-registration acquisitions or an unbound Entry. A hypothetical
Owners-empty query alone would miss earlier failed frames too. Conversely,
attachment `Phase::complete` can mean a section still owns a live binding and
generator; consumed creator pointers do not prove retirement.

If a future API nevertheless returns mutable Owners, the conservative obligation
must latch **before the reference escapes**, because existing free functions can
enter work without notifying the application. Never clear it from registry
emptiness, successful return, or an acquisition enum. Actual frames, companions,
application and providers must then survive through no-cleanup process exit
unless a separately recovered retirement protocol proves every obligation ended.
There is no current public API that sets such a generator obligation. Do not
pretend `requires_process_retention()` already does it.

A future latch would need both the existing host retention exits and pre-entry
control of frames/reset/recreation and borrowed provider routes. Adding it only
to drain is insufficient. The already available emergency action is
`std::_Exit`, as used by StartupHost retention failures: retain all records and
providers and take that path before unwinding them or running CRT cleanup.
That is a terminal fallback, not an activation implementation or permission to
continue rendering. No such latch, gate, recovery, replay or new API is added by
this packet. Full activated loading remains a separate held boundary.

## Evidence and limits

The separate follow-up bundle freezes 50 translation-unit roots, 709 complete
quoted-include Source files plus four context files, 1,464 include edges with
none unresolved, all 753 retained Root Source749 pins, and the exact current
use-site searches. Every Source file has its corresponding exact Source749 Git
blob. The previous readiness's 1,952 immutable payloads and 71,415,018-byte ZIP
were verified unchanged; its old Source/build meaning is not relabeled Source749.

Only this document and its JSON report are committed. No C++/CMake modification,
compiler, build, test, new Native/PE bytes, Ghidra query/mutation, runtime probe,
activation or game validation occurred. The read-only replay checks byte pins,
Git object identities, the complete Source include closure and report anchors;
it does not execute the proposed API or establish thread/ABI/runtime safety.
