# Procedural factory application operation

`GameNativeProceduralFactoryApplication` composes the existing complete Source
`BBCB40 -> BBC900 -> BBC5F0/BBC740 -> B1B730/B1B3A0` setup and
`BBC5D0 -> BBC520` retirement. It is an ordinary C++ application operation, not
a new reconstruction of those routines or an Original caller-ABI claim.
Its only mutable publication is a required borrowed, caller-owned
`void* volatile&` serving the `01090900` role. No private publication, fixed
mutable-address mapping, registry seed, cache seed or function substitution is
introduced. Startup activation and CMake registration belong to the integrator.

## Required domain and value inputs

The caller retains the designated publication cell, `GameVfsHost`,
`GameSingletonHost` and `GameNativeReadOnlyData` through setup, all consumers,
explicit table retirement and the complete later parent drain. The caller is
the sole setup authority for this publication and the four factory keys; all
operations and external tree/owner mutations are serialized. The wrapper is
immovable and owns one address-stable implementation allocation.

Before constructing it, install the existing singleton registry deletion
binding with the same VFS owner services:

```cpp
singletons.bind_resource_registry_domain(
    vfs.native_owners().strings(),
    vfs.native_owners().physical().invalid_parameters);
```

This is an existing domain binding, not a registry allocation or insertion.
The wrapper uses `vfs.native_owners()` and `vfs.raw_strings()`, which are
available before VFS core startup. It never invokes VFS `phase2` or
`borrow_raw_services`; a CPU caller must not run hardware discovery merely to
obtain these services. A later full `B1B4D0` consumer still requires its genuine
platform/VFS/loader prerequisites.

The wrapper checks AA0/AA8/AA4 cell identity against the retained process
string domain and the VFS owner services, the lifetime manager binding, the
singleton string deletion cells, and the exact F8D41C registry deletion
binding. The borrowed table publication cannot alias any of those four cells.
It borrows actual read-only profiles at D5E594/D5E59C and
D64470/D644AC/D644E8/D644F0, verifying both words. It verifies complete pinned
NUL-terminated literals at CE47BC, CE7DEC, CE7DC8, CE7DA4 and CE7D80 before
using those original mapped addresses. It does not copy numeric profiles into
callable host vtables.

`GameNativeProceduralFactorySourceInputs` requires a complete publication
frame and a separate retirement frame. Every referenced DWORD, typed iterator,
insert-result field and three padding bytes, plus all raw SBO bytes, must have
an initialized value and a live typed object before construction. No incoming
ECX or untouched input value is defaulted by this API. The implementation
copies the supplied values into disjoint typed storage for:

- Publication ECX, allocation cleanup and EH state; five constructor DWORDs
  and EH state; one reused factory frame; four constructor cleanup flag cells.
- Registration arguments/result; value name/result and contiguous three-DWORD
  raw pair; hint arguments/iterator/result; direct link and fallback pair/link.
- Both link paths' arguments and raw 1Ch SBO messages, with separate node
  incoming/constructor argument words and allocation/placement cleanup cells.
- Independent retirement cleanup self, normal flags and three cleanup flags.

All frame objects and fresh acquisition trees exist before native side
effects. The four constructor invocations have separate acquisition trees
while reusing the one stable native factory frame, as required by the existing
provider contract. Raw string/SBO storage has no owning destructor. Input
frames need survive only the application constructor; their aliases and
native stack-address coincidences are not reproduced. Explicit ordinary
Source zero inputs are a valid caller choice, but do not prove Original
hidden ECX, caller stack preimages, register ABI or untouched bytes.

## Setup, lookup and retirement

`setup_00bbcb40()` enters once from `prepared`, rejects an occupied publication,
and invokes the genuine existing publication provider. Successful admission
requires its actual allocated table, completed constructor, all four final
profiles and published slots, and a freshly allocated/linked registry node
borrowing each real factory at node+14. Existing duplicate keys preserve their
old values in native `B1B3A0`; this wrapper therefore rejects such a setup after
the actual effects instead of inventing replacement, unregister or rollback.

`require_lookup_ready()` admits only the live phase and checks publication,
parent, table slot, factory profile and borrowed node identities. It supplies
no alternate lookup route. External destruction, reuse, races and arbitrary
pointer corruption are outside the admitted caller contract; identity checks
are not memory-safety probes for already freed storage.

After all external factory lookups are quiescent,
`retire_00bbc5d0(true)` verifies ownership and invokes the existing shutdown
exactly once. The boolean is the caller's explicit quiescence assertion; the
wrapper cannot observe all external users. Normal completion destroys real
factories in native +C,+8,+4,+0 order and frees the captured table. The original
publication is deliberately left containing the dead table pointer bits.
Subsequent lookup/setup/retirement is rejected before any dead payload read.
The registry's factory values also become borrowed dead pointer bits; its
later genuine erasure frees names/nodes without dereferencing those factories.

Only after the genuine later loader/shared-manager drain may the caller invoke
`after_parent_drain()`. This checks the unchanged dangling table pointer and
null AA0 manager, F8D41C registry and AA8 string-pool publications, without VFS
re-entry or native payload access. It acknowledges `drained`; it does not
perform the drain or clear the table publication. The caller remains
responsible for a complete parent drain, not merely making its cells null.

An exception after entering setup, lookup validation, retirement or drain
acknowledgment permanently sets `failed`; all native effects and acquisition
diagnostics remain observable and retained. Existing providers perform their
own genuine Source cleanup prefix. The wrapper adds no retry, rollback,
unregister or automatic native cleanup. Its destructor terminates if the
operation has entered and has not reached `drained`. Failure disposition must
retain the application and all parents for the process; allowing an ordinary
stack unwind to destroy this graph is not an admitted recovery path.

## Evidence and limits

The native contracts, original call order, complete provider closure and
earlier live Ghidra/installed-PE comparison are recorded in
`NATIVE_PROCEDURAL_FACTORY_SETUP_CC12.md` and
`reports/native_procedural_factory_setup_cc12.json` (prior packet). The native
startup call to BBCB40 is at 73D8CC before platform construction and shader
preload; native shutdown calls BBC5D0 at 738314 before the later F8D420 action.
This operation does not activate or alter that startup/shutdown wiring.

This packet independently checks the installed executable SHA-256 and all six
profile pairs plus the five complete literals used by this operation. It also
checks the complete 130-byte name span, including the Caustics final NUL; the
prior receipt's 129-byte span is preserved unchanged. No new Ghidra mutation or
export is needed for an application composition over unchanged providers.

The new TU is compiled separately with captured MSVC Win32 Release options
(`/std:c++17 /EHsc /fp:strict /O2 /Ob2 /MD /W4 /WX`). The required normal
`scripts/build.ps1` is also run on this worker baseline; because the integrator
owns CMake registration, that baseline build does not prove the new TU is in
the game executable. The integration build must register and compile it.
See `reports/native_procedural_factory_application_cc12.json` and the ignored
`local/procedural_factory_application_20261015/` receipts.

No operation or fixture is executed in this packet. Source1 sampler execution,
canonical resource retirement, a full B1B4D0 loader/platform route, GPU/device
behavior, Original ABI/SEH and game validation remain unproved here. No window,
focus change, probe activation, GPU run or new test suite is added.
