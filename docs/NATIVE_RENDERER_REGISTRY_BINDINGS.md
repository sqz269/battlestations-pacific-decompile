# Renderer registry bindings to the actual lifetime manager

The existing complete state-definition and system-constant constructors can now
use the application's actual `01090AA0` manager publication. State definitions
still occupy the same `28h` owner published at `0108FE90`; system constants still
occupy the same `10h` owner published at `0108FE94`. No literals, array bodies,
string pool, manager state, or success callbacks were duplicated.

State-definition base/full constructor, destructor and scalar-delete overloads
accept `SoundLifetimeAccess(actual_manager_cell)` where the existing overloads
accept `SingletonLifetimeDomain&`. The projected overloads forward to those same
full implementations. Keep the typed `0108FE90` publication and existing
`NativeStringStorage` alive through destruction.

For system constants, construct the existing binding with
`NativeSystemConstantRegistryLifetimeBinding(actual_0108fe94,
actual_01090aa0, strings)`. Existing constructor, array, destructor and scalar
functions keep their signatures and persistent operation frames. The projected
binding constructor and `bind(domain, strings)` remain usable. `lifetime()` is
projected-only; internal code uses `lifetime_access()`. The actual binding does
not provide other-owner callback forwarding: canonical application dispatch
passes the popped owner to the existing scalar body with this borrowed binding.
Its legacy callback adapter terminates for unsupported profiles or an unbound
invalid-parameter provider.

Both routes use the already implemented `SoundLifetimeAccess` raw providers:
`00415350` reads the actual mutable manager cell and lazily constructs its raw
`14h` owner; `BD0C30` and `BCFCA0` operate on its actual vector. Capture the first
manager's `+10h` section, enter it and increment its actual `+18h` depth. Resolve
the second manager before reading the current registry publication. Construction
publishes before derived arrays exist. Destruction unregisters the current
publication while destroying the captured owner, then clears the publication.
Section cleanup always releases the originally captured section.

State definitions retain their existing explicit `__try`/`__finally` member and
base cleanup. The section object is explicitly placed in trivial local storage
and destroyed by the existing finally block, so introducing raw access does not
replace that block with an automatic source destructor. The literal tables and
the reverse texture-stage/sampler/render member cleanup are shared unchanged.

System constants retain their existing operation admission and failed-state
guard. A failure after `begin` leaves the same owner, source, temporary strings,
array state and operation reachable. It prevents another operation and terminal
deletion. Destroying a running/failed operation or its guarded binding terminates.
This applies even when failure occurs before arrays exist. No cleanup/recovery
algorithm was added. Consequently, the parent renderer's original `B32410`
allocation cleanup cannot free a `B5BF70` owner, or discard the binding/operation,
after an admitted exception. That parent FH3 path remains uncomposed; normal
construction and destruction are the supported integration domain.

Evidence is recorded in `reports/native_renderer_registry_bindings.json`.
The twelve complete original spans were rechecked against the installed PE.
Live wrapper queries verified the existing BSP project/program and base
instruction ordering. `B58320` has a saved listing gap after the first free;
its five subsequent calls through the base destructor and return at `B583BA`
are pinned by original PE bytes and independently read live memory, rather than
claimed as saved function-body call-graph verification. No Ghidra mutation was
performed. Existing names remain descriptive reconstruction hypotheses.

The focused local fixture links the registered production library, runs the
full `31/13/18` state tables and `52` system records through an actual raw
manager and `ActualNativeStringPoolStorage`, replaces each publication before
full scalar destruction, and checks captured depth and canonical pool shutdown.
Its isolated failure mode checks state cleanup and retained system-operation
terminal refusal; retained acquisitions survive until that process exits.
Build, fixture, original PE and toolchain hashes and immutable artifact paths
are recorded in the report. New source provider interfaces do not reproduce
the original private FH3 maps, register/stack ABI, static CRT throw identities,
SEH/hardware-fault behavior, or gameplay/rendering validation.
