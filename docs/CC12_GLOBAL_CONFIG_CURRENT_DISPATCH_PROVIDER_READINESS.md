# CC12 GlobalConfig current-dispatch provider readiness

A concrete `GlobalConfigEffects` provider is viable for **actual raw objects
with callable current method tables**. The existing interface already separates
the two required calls, and the existing destructor owns the real atomic
decrement and mutation-sensitive slot ordering. Two small provider methods can
close that dispatch implementation gap without selecting a Sound profile.

This is conditional provider readiness. It does not establish the missing
second-array producer, admit projected Sound objects as raw objects, or make
original-image numeric profile identities callable in the rebuilt process.
The current `noexcept` interface also limits the result to its normal-flow,
nonthrowing source contract. No source implementation or execution was performed.

The [report](../reports/cc12_global_config_current_dispatch_provider_readiness.json)
contains native bytes, source excerpts and hashes, exact signatures, proposed
source scope and remaining admission requirements.

## Native and existing source agreement

`004325CF` forms GlobalConfig `+2C0h`, and `004325DD` calls the embedded destructor
`008DBDB0`. The destructor's relevant order is:

| Native site | Required behavior | Existing source |
| --- | --- | --- |
| `008DBDEF..008DBDF5` | Read the non-null second-array object's current table, load `+8`, call with ECX = actual object and one DWORD zero argument. | `effects.stop_slot_08(captured, 0)` |
| `008DBDF7` | Reload the second-array slot after that call. | `destroy_slot(second, effects)` performs a new slot read. |
| `008DBDFD..008DBE09` | Decrement the reloaded object's `+4` counter; continue to zero dispatch only when the result equals zero. | Private `release()` calls real `InterlockedDecrement(actual+4)` and tests `== 0`. |
| `008DBE0B..008DBE11` | Read that released object's current table and call `+0` with ECX = actual object and no stack arguments. | `effects.zero_references_slot_00(object)` after the decrement. |
| `008DBE13..008DBE37` | Clear after callback, retain the explicit extra clear, then release/clear the paired first-array slot without a stop call. | Existing slot and aggregate helpers retain this order. |
| `008DBE42..008DBE72` | Run reverse second-array and then reverse first-array slot destructor passes. | Both reverse loops remain in `src/global_config.cpp`. |

The `00524180` and `004C3810` slot destructors independently show the same
capture, decrement, current `v+0`, clear-after-callback order. Initially null
standalone slots are untouched. PE import metadata resolves IAT `00CE2220` to
`KERNEL32.dll!InterlockedDecrement`.

“Exactly once” means once per non-null **release operation**. Distinct owning
slots can hold the same object, and callbacks can repopulate slots for later
release. Removing the reverse passes or deduplicating object pointers would
change behavior.

## Exact viable provider contract

The proposed concrete class can implement the existing interface unchanged:

```cpp
class NativeGlobalConfigCurrentDispatch final : public GlobalConfigEffects {
public:
    void stop_slot_08(void* actual_object, std::uint32_t flag) noexcept override;
    void zero_references_slot_00(void* actual_object) noexcept override;
};

using StopMethod = void (__thiscall*)(void* actual_object, std::uint32_t flag);
using ZeroMethod = void (__thiscall*)(void* actual_object);
```

For `stop_slot_08`, read the table from the actual object, read `StopMethod` at
table byte offset 8, then call `method(actual_object, flag)`. The aggregate
supplies zero. For `zero_references_slot_00`, read the object's table at callback
entry, read `ZeroMethod` at byte offset 0, then call `method(actual_object)`.
The latter callback must perform **no decrement**: the caller has already
completed the actual `object+4` decrement and reached zero.

Each invocation needs fresh object/table reads. Do not retain a table across
stop, the slot reload, the atomic decrement, or another callback invocation.
The stopped object may differ from the reloaded object that is released.
Either method can end its receiver's lifetime, so the provider should not use
that receiver after the target returns. Slot clearing belongs to the existing
aggregate/slot helpers.

Require MSVC Win32 and four-byte object/function pointers. The existing
`read<T>` helper can load the actual table and function-pointer words without
casting the object into a C++ Sound projection. The provider object is only
the service implementation; the inner `__thiscall` must place the **payload
object**, not the service object, in ECX.

`00BD30E0` is a useful native example: it takes no stack arguments, performs no
decrement, and calls the object's current `v+4` with flag 1. It is not a universal
zero endpoint. The provider must execute actual current `v+0`; it cannot replace
that call with a fixed `00BD30E0`, direct `v+4`, or a projected release routine.

## Existing concrete call patterns

- `NativeGamePhysicsLifetimeCalls::physics_virtual_scalar` reads the actual
  current table at a byte offset and calls a `__thiscall` target on the actual
  receiver. Its header explicitly requires callable source tables. Its scalar
  signature includes a flags argument, so that method itself is unsuitable for
  the no-argument zero callback.
- `release_native_ref_counted_handle_0041de40` in
  `src/native_damageable_section.cpp` already performs a real raw `+4` decrement
  followed by current `v+0` as `void (__thiscall*)(void*)`. It is a full release
  helper and must not be called from an already-decremented zero callback.
- The particle component fallback uses the same zero-call shape. The online
  notification dispatcher demonstrates current table/method loads using
  `memcpy`, followed by a genuine no-argument `__thiscall`.

These are existing source patterns, not fresh build or runtime proof for the
proposed GlobalConfig provider. Inheritance from the physics service or a new
physics/Sound dependency is unnecessary.

## Sound representation boundary

| Existing family | Verified current source contract | Consequence |
| --- | --- | --- |
| `SoundInstance` / `SoundChannelInstance` | Header explicitly says behavioral projection and nonnative layout; fields include inherited `SoundLevelEntry` and `VoiceSoundStartFields`. | A canonical Sound entry is not an admitted raw `table@0 / counter@4` object. |
| `SoundChannelRuntime` | `release_reference` decrements projected `references_04`, then selects a projected scalar helper. Its incoming pointer must be the canonical `SoundLevelEntry` subobject. | Delegation from GlobalConfig's post-decrement callback would introduce another decrement and incompatible storage assumptions. |
| `SoundSampleStorage` / `SoundSampleRuntime` | Sample storage really is `7Ch` with counter `+4`, but its source constructor writes numeric `00D5B074` at `+0`. Runtime final release checks that identity and calls a concrete sample scalar with its retained context. | Raw layout alone does not establish a callable current-process table. That existing adapter, or a separately established callable-table construction contract, remains necessary for this source family. |

The current GlobalConfig sound assignment source obtains sample-cache results
and stores their pointer values in the raw slots. The raw dispatch provider
cannot silently turn the source sample's numeric identity into callable code.
Likewise, the existing `NativeRefCountedDeleteCalls` abstraction resolves a
captured profile to a source `v+4` body; it does not supply arbitrary current
`v+0` behavior.

No `D5ABF8` admission is proposed. The unresolved second-array producer remains
an explicit payload contract: each non-null object must own the expected
intrusive reference and expose the actual current stop/zero interface.

## Bounded implementation scope and remaining conditions

The minimal next implementation would add the concrete class declaration to
`include/bsp/global_config.hpp` and its two methods to `src/global_config.cpp`.
Those files already build. Keep `release()`, slot reloads, clears and aggregate
loops unchanged. The actual `GlobalConfigContext` owner can retain a stateless
provider instance after its payload domain is admitted.

This would provide real stop/zero dispatch bodies, not a no-op or another
abstract callback. It would still require:

- Accessible, aligned four-byte counters at actual object `+4`, with valid owned
  references and lifetime through the decrement/current-table read.
- Callable current-process `v+0` for every non-null first-array object, and
  callable current-process `v+8(flag)` plus `v+0()` for every second-array object.
- The same guarantees independently for any replacement installed during stop
  or another callback. Original PE address labels and C++ service vtables do not
  satisfy this condition by themselves.
- Nonthrowing callback behavior under the existing `noexcept` source interface.
  Native `008DBDB0` has an EH frame; matching exceptional unwinds would require
  a separate interface/lifetime scope.

Shared manager/string bindings, outer GlobalConfig scalar deletion, and actual
payload producer/callable-table integration remain separate work. No empty-array
argument, invented fallback, skipped cleanup or whole-runtime readiness claim
is needed to implement the bounded provider.

## Verification

Fresh guarded Ghidra queries verified the BSP project/program. Five complete
native bodies (469 bytes, 170 instructions) and the four-byte atomic import cell
match the installed executable SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The report retains 12 instruction contexts and 18 source-file pins with relevant
excerpts. The inspected source/include/tests tree contains `GlobalConfigEffects`
only in its interface and existing aggregate implementation; no concrete
provider was found there.

Only this document and its report change. No source, Ghidra, build, fixture,
probe, runtime or gameplay work was performed. After implementation is authorized,
the existing Win32 build and focused emitted-call/lifetime verification can test
the concrete dispatch contract without broad new test infrastructure.
