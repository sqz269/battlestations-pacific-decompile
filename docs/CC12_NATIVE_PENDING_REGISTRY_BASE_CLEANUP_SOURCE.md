# Ordinary Source pending-registry base cleanup leaf

Baseline: `5db1a543115ff82ed6b42be0373ab21452ccfeed`, containing the accepted
`8748F0` audit published at `c1893651fd351ea67c5f2bb3151fd06553112d1f`.
This packet adds one ordinary C++ leaf and its declaration. Primary emitted-body,
concrete-child, Core-link and MSVC Win32 build review remain pending. It does not
admit a production owner binding or Original compatibility.

Implementation:
[header](../include/bsp/native_pending_registry_base_cleanup.hpp),
[Source](../src/native_pending_registry_base_cleanup.cpp).
Evidence: [cc12_native_pending_registry_base_cleanup_source.json](../reports/cc12_native_pending_registry_base_cleanup_source.json).

## Explicit borrowed interface

```cpp
void cleanup_native_pending_registry_base_008748f0(
    void* actual_receiver,
    void* volatile& actual_registry_publication_00f878cc);
```

The receiver is the actual writable owner whose first DWORD is being reset.
The reference must bind the application's genuine mutable `F878CC` publication
cell. The parameter's name does not create or prove that binding. The caller
supplies both lifetimes; this leaf owns neither object and introduces no second
cell, process global, default publication or owner/provider factory.

The implementation contains exactly these ordinary statements:

```cpp
actual_registry_publication_00f878cc = nullptr;
destroy_native_generic_singleton_base_00412430(actual_receiver);
```

The volatile clear is unconditional and occurs first. There is no comparison
between the publication's prior value and the receiver. The actual existing
child writes `CE3818` to the receiver's first DWORD through a volatile
`std::uint32_t` store. The receiver's section at `+04` remains untouched by
these ordinary operations.

The leaf adds no validation, section lookup/release/unlock, allocation/free,
manager lookup, registration/removal, publication restoration, guard or catch.
It has no `noexcept` annotation. The child is the existing concrete profile
store, not an injected callback or a same-layout replacement owner.

## Accepted native relation

The complete accepted `008748F0..00874900` body is **17 bytes, three
instructions**:

1. `MOV DWORD[F878CC],0`.
2. `MOV DWORD[ECX],CE3818`.
3. Plain `RET`.

The new C++ leaf refines those ordinary storage effects through the explicit
borrowed-cell interface. The existing child declaration and entire Source file
are pinned; its definition is the single volatile base-profile store. **The
native body does not call `00412430`.** Source reuse is not a claim about native
call topology or identical emitted instructions.

Native ECX supplies the receiver, EDX and caller stack arguments are unused,
and EAX is unchanged with no semantic return result. This C++ interface adds
the publication reference and does not implement that original entry ABI or
promise its register/flag preservation. The header restricts compilation to
MSVC Win32; compilation itself is left to primary integration.

## Constructor and runtime limits

The prior constructor state-zero map routes through `C963E0`, which loads the
current `[EBP-10h]` receiver slot and tail-jumps to `8748F0`. Mapping that slot to
the constructor receiver requires the shared helper's qualified EBP anchor.
This leaf does not implement that frame protocol, original FH3 dispatch,
hardware-fault handling, mutable spill aliases or nested cleanup failure.

For valid bindings and completed ordinary accesses, the clear precedes the
base stamp. This does not guarantee successful Native cleanup. A bad receiver
can fault after the publication has been cleared; no rollback is added.
The absence of section or owner free here does not determine outer getter
cleanup or complete owner retirement.

A producer of the genuine publication reference, actual owner lifetime,
constructor/getter composition, callable Source profile delivery, getter EH,
retirement and canonical executable integration remain absent. The native
identity DWORD `CE3818` is not a callable C++ vtable supplied by this leaf.
There is no runtime/game validation or Original credit.

## Worker verification and primary admission gate

The accepted native audit and its 17-byte span are pinned. The span was replayed
against the current installed PE, whose full SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Its body SHA-256 is
`d6b00cb7e86ca84a971ae28580fec99ba8b7de3ccaf5750da1df4bf81b0d20cb`.
Current Source child files and all eight accepted audit pins were rechecked.
The report hashes both new code files, this document and the evidence inputs.

Worker checks cover JSON/pin coherence, the exact two-statement Source body and
owned-file diff cleanliness. They are not compilation, emitted-code, linkage,
fixture, Native ABI or gameplay evidence. No CMake, ledger or Ghidra changes,
builds, tests or probes were performed by the worker. Primary review must inspect
the entire emitted leaf and actual child, establish Core linkage and run the
required MSVC Win32 build before any admission.

## Primary registration and whole Source review

The integrator registered the leaf in bsp_core and completed the normal
MSVC Win32 build with all three existing checks passing. The entire emitted
ordinary C++ leaf is 25 bytes/nine instructions: cdecl frame setup, load the
borrowed cell address, push the captured receiver argument, clear the cell,
call the actual profile helper, clean four argument bytes, restore EBP and RET.
The clear occurs before that helper call.

The actual helper's complete current body is 14 bytes/six instructions with
one volatile DWORD CE3818 receiver store and no section+4 access or descendant.
The sole external REL32 operand16 resolves to its physical symbol. Both complete
objects occur exactly once in Core, with two unique positive public definitions.
This Source call is not an Original Native call; the Native body remains 17
bytes/three instructions and no calls. No identical ABI/bytes/flags are claimed.

The previously uncounted Original address receives one complete ordinary
storage-schedule reconstruction covering 17 Original bytes. Its Ghidra name is
provisional, old comments are preserved, and the export is refreshed. This root
is absent from the game map. A genuine cell producer, owner/table/lifetime and
constructor/getter/retirement composition remain open, as do Native FH3/fault/
runtime/startup/gameplay qualifications.

Primary evidence is in
`reports/cc12_native_pending_registry_base_cleanup_primary_review.json` and
`local/cc12_registry_base_cleanup_primary/whole_objects_and_actual_profile_store_binding.json`.
Source/build inputs, complete objects/library, game map/executable and test log
are retained in that local directory. Worker document pins precede this appendix.
