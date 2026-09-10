# Native model owner

`NativeModelOwner` reconstructs the complete model constructor, direct
destructor, geometry-member cleanup, scalar deleting destructor and retained
geometry setter over an actual `188h` model-pool slot. `NativeModelReference`
borrows its existing `+04` atomic and supplies the shared node logical-release
and render-reference interfaces. Final zero release runs real model/node
destruction and returns the slot before retiring host companions.

The native comparison passed one ownership trajectory with **2,367 normalized
observations**, including complete `188h` constructor/preparation storage,
signed-zero pose words, actual terminal owners, reentrant replacement of the
later geometry field, logical/queue/raw releases and physical pool return.
Two additional checks exercised host direct-destructor unwind. Those two
checks did not execute native exception unwinding. MSVC Win32 strict
compilation and the existing two repository tests passed.

## Functions, addresses and ABI

Ends are exclusive. These are new C++ interfaces, not original-ABI exports.
Descriptive names are hypotheses. The [audit report](../reports/native_model_owner_audit.json)
preserves old Ghidra names/comments, byte hashes, dependency evidence and the
validation boundary. No Ghidra annotations or shared ledgers changed here.

| Native function | Complete span | Original ABI |
|---|---|---|
| Construct | `00B75030..00B750B4` (132 bytes) | ECX actual model, stack `NativeString*`, EAX same model, RET4 |
| Direct destroy | `00B750C0..00B75165` (165 bytes) | ECX actual model, RET |
| Scalar delete | `00B75290..00B752B0` (32 bytes) | ECX actual model, stack flags, EAX original address, RET4 |
| Geometry member destroy | `00B74F20..00B74F4B` (43 bytes) | ECX model+178, RET |
| Geometry setter | `00B75170..00B751EB` (123 bytes) | ECX model; stack unused DWORD, raw geometry, float, float; RET10h |

## Actual storage and dependencies

The object occupies `[0,184h)`; the pool's authoritative slab-index DWORD at
`+184` remains untouched. The existing `NativeNodeStorage` occupies the first
`174h` bytes, and `NativeModelTailStorage` holds raw retained `+174`, float
`+178`, float `+17C` and raw geometry `+180`. There is no copied reference
count, transform, scene pointer, name, hierarchy or point-light array.

`NativeModelEnvironment` supplies the canonical `NativeModelPool` representing
`01090054`, the existing node/scene/attachment/string runtime, live
`ModelTypeBootstrap`, actual retained-owner resolver, native constant words and
verified model/node vtable views. The slot must come from that same pool.
Dependencies and stable bindings outlive every call and terminal callback.
Raw references require live aligned storage with an actual atomic at `+04`.
Native profile tables and constructor constants represent their actual native
values. Concurrent mutations and malformed pool/reference storage are outside
the interface's domain.

Preparation preserves the complete slot preimage while establishing typed
lifetimes and the external scene association. Abandoning an unconstructed
prepared owner only removes that association and ends the prepared lifetimes.
A live owner must undergo explicit native destruction before its companion
is destroyed. Physical allocation/return is explicit throughout.

The shared node constructor prerequisite now accepts its actual `174h`
prefix; the owner supplies the full `188h` slot size. Node and camera callers
retain their own respective allocation extents. The source, main prerequisite
and independent model-pool/type commits are recorded in the audit.

## Construction and retained assignment

`construct_native_model_00b75030` first invokes complete `00B6F5A0`. It then
captures `CE4ADC`, publishes `D62DE8`, clears raw `+174`, writes captured
`D01502F9` to `+178`, loads/writes `CE4970` (`501502F9`) to `+17C`, loads
`D7A208` (`80000000`) and clears raw `+180`. It writes that negative zero to
`+18,+1C,+20`, then positive zero to `+24,+28,+2C,+08,+0C,+10,+14` in native
order. These pose values are not replaced with identity/unit-scale defaults.
The node constructor owns its failure cleanup; a failure does not return the
model's physical slot. The enclosing caller owns that distinct step.

`set_native_model_geometry_00b75170` captures old `+180`. Different identities
publish incoming geometry, retain incoming actual `+04`, then release the
captured old actual `+04`, invoking its current terminal only on zero. Equal
identity skips both count changes. No companion lookup precedes a nonzero
release. After callbacks the setter captures the sentinel word once from
`D7A260`; native `UCOMISS/LAHF/TEST44` semantics write each unequal or unordered
argument. A -1 argument preserves its scalar when the captured sentinel is
the normal `BF800000` value. The unused stack DWORD is explicit in the API.
An old-owner failure leaves the incoming publication/retain in place and the
later scalar writes unvisited, matching the absence of native setter unwind.

The resolver `NativeRenderActualOwners` must return a canonical companion that
borrows the exact raw atomic and dispatches the owner's current real terminal
profile. Opaque `+174` semantics remain unresolved; its reference lifetime is
complete. Geometry construction is unnecessary for this generic raw reference
operation, but a geometry reaching zero still requires its real terminal.
`GeneratedModelGeometryReference` and other host shared-pointer objects are not
native geometry storage and must not be published as raw `+180` identities.

## Direct destruction and exception cleanup

The direct destructor installs the model phase, releases and then clears
captured `+174`, reloads current `+180`, releases and then clears it, and calls
the existing full node destructor. It does not require logical release first
or impose the older typed model's precleared parent/root/child restriction.
Existing node lifetime services handle reentrant hierarchy/scene changes,
actual `+130`, point-light array storage and pooled name cleanup.

Handler `CC1C66`, FuncInfo `DFAC50`, map `DFAC40` establish two unwind actions:

| Active state | Cleanup |
|---:|---|
| 1, during `+174` release | `CC1C58` calls `B74F20(model+178)` to release CURRENT `+180`, then `CC1C50` calls full `B6F440` |
| 0, during normal `+180` release | `CC1C50` calls full `B6F440`; the failing geometry release is not retried |

Normal execution sets state -1 before the node base; a throwing base is not
called twice. The failing field remains uncleared. Geometry-member cleanup
preserves both scalar words and raw `+174`. Cleanup failures during an active
exception terminate instead of silently reporting a completed deletion.
Exceptional completion ends the typed tail and removes the same external
scene association after the node base has performed its cleanup.

The direct API propagates actual-resolver failures. Current reference terminal
interfaces are nonthrowing; arbitrary native throwing-terminal support is not
claimed. The host exception checks validate the recovered state sequence
without claiming to have executed the native MSVC exception handler.

`delete_native_model_00b75290` returns the actual slot through canonical
`00B74750` only after successful destruction and only when `flags & 1`.
Its result is the original address even if that address now denotes free pool
storage. It adds no reference decrement or byte-44 logical-release gate.

## Current model dispatch and reference retirement

The model profile is actual `D62DE8`; base destruction enters `D62C88`.
Scene callbacks validate the current known profile/slot before shared
`B6ED80`, `B6EE10` or `B6DBE0` dispatch. Model type testing reads live
`006EF860` descriptor words through `ModelTypeBootstrap`; it does not use
`SceneAttachmentRuntime`'s copied type-token array. A known node-base phase
uses actual `B6F570`. Unknown profiles or entries fail explicitly.

`NativeModelReference` registers one stable companion in the existing
`GeneratedModelLifetimeRuntime`, adds no native reference and borrows the
actual model atomic. Current virtual `+18` must be `B6F310`; its shared logical
release reads the same hierarchy, byte `+44` and current native point-light
array. Current virtual `+54` must be `B6EE10`, including permitted base-node
destruction phase. Final zero requires current virtual `+00=BD30E0` and
`+04=B75290`; it completes scalar deletion, unbinds the host lifetime identity
and invokes the explicit retirement callback. No companion or native storage
is accessed after that callback. Any external raw-owner lookup association
belongs to the caller and can be removed there.

## Verification and remaining scope

`local/prepare_native_model_owner.py` verified 25 code/boundary spans and
12 data spans (3,321 bytes total) against both the installed PE and guarded
Ghidra reads. The fixture maps only those spans with 85 checked absolute
rebases; unrelated pages stay inaccessible and unused bytes remain `INT3`.
The only external jump hooks are the two existing CRT free boundaries; both
call the same actual shared allocator free. Native model/node/context
destructors, current deleting-destructor dispatch and model-pool return execute
their original bodies. It does not load the game entry point or fake terminals.

The single native ownership trajectory uses actual refcounted render contexts
as the retained owners to exercise the model's generic raw reference protocol.
This is not a native mesh/geometry construction claim. It compares complete
slot preimages and constructor results, signed-zero pose, equal-identity and
NaN setter behavior, post-callback sentinel capture, standalone member cleanup,
current `+180` replacement during `+174` retirement, surviving displaced
references, extra-held model logical release, queue/raw final release, and
flags-2 destruction without physical return. Checks inspect physical pool
state after return without reading returned model storage.

Two host-only resolver failures separately confirm state-1 geometry-plus-base
cleanup and state-0 base-only cleanup, no retry and no exceptional pool return.
They retain real zero-count owners for explicit actual terminal cleanup after
the failure. All terminal callbacks in the native comparison are nonthrowing.

`local/build_native_model_owner_check.ps1` compiled the new source and fixture
with MSVC Win32 `/W4 /WX /EHsc /fp:strict /O2` and passed 2,367 observations.
`scripts/build.ps1` passed `reconstructed_math` and `native_math_differential`
after all eight native seed checks matched. CMake registration of the new
model owner/type source belongs to primary integration; the private strict
fixture explicitly compiles both and links the current core library.

Full geometry/material/stream composition, `B752B0` cloning, `B748E0` rendering,
arbitrary derived profiles, gameplay validation and drop-in binary compatibility
remain outside this packet. Existing broader node/scene algorithms are reused;
the focused native trajectory uses empty names and empty child/light/scene
collections and does not establish new native coverage for their populated paths.
