# Application native particle runtime composition

## Result

`GameNativeParticleRuntime` owns one reusable source-level context graph for the
existing raw particle implementation. It does not create a second string pool,
F8D344 parameter pool, VFS, resource manager, cache, renderer owner, platform
service or allocation policy. The application supplies those existing owners;
the runtime stores context nodes and the recursive links between them.

The graph includes:

- the raw parameter builder, F8D344 runtime conversion and type-parameter math;
- type properties and Sprite/Floating, Axial, Object and Tracer parsers;
- type construction and the complete raw particle-type factory;
- Sphere, Cone and SmartArea parsers, linked recursively through both type and
  emitter factories;
- `AF4BA0` resource parsing and `86BA60` VFS-backed resource loading;
- the concrete loaded-resource lifetime, Object model reference chain and
  particle resource cache lifetime;
- `870DD0` cache acquisition with the same VFS date route, live platform
  publication, bound platform identity and load-event host.

The accessor surface returns the high-level loader, parser, two factories,
loaded-resource lifetime, cache and acquisition contexts. Acquired invocation
frames remain caller-owned. In particular, a failed nested VFS frame still has
its existing process-lifetime retention obligation; destroying the runtime or a
failed frame is not a rollback mechanism.

## Inputs and identity checks

`GameNativeParticleRuntimeInputs` borrows the application's actual raw string
cells, F8D344 pool, VFS services, resource manager/load-cache/container-reference
objects, texture cache, render-owner resolver, D3DX half import, CRT access,
mutable `0109EEA4` feature word, cache/model/atlas/platform publications and the
native empty-string storage used by the recovered bodies.

Construction rejects the SAME-domain mismatches that are identifiable from the
public contexts:

- resource manager strings must be the supplied raw string context;
- resource manager and strings must reference the same `01090AA0` cell;
- resource loading must reference the same live `0109CEEC` cell and name
  resolver as the VFS raw services;
- texture loading must reference the same VFS date route.

Other relationships remain application preconditions where the source contexts
do not expose comparable owner identities. All borrowed inputs must outlive the
runtime, every acquired invocation and destruction of every resource created
through the graph.

The native constructors leave several words unwritten. The runtime accepts each
one separately in `GameNativeParticleRuntimeResidues`: resource-emitter child,
Sphere child, Cone child, SmartArea child and Axial builder kinds, plus the type
record and first/later texture-record `+18` words. It does not derive or zero any
of them. Direct callers still supply the incoming builder kind when constructing
each loader, parser, factory, acquisition or component-reader acquired frame.

## Native data admission

The graph reads numeric constants and profile targets from the verified original
image through `GameNativeReadOnlyData`. Applications must merge
`game_native_particle_runtime_data_spans` with their other bootstrap spans before
creating the mapper or accepting its handoff. The runtime requires the `CE`,
`CF`, `D0`, `D5` and `D7` 64-KB bands. A missing band fails through
`GameNativeReadOnlyData::data_at`; no copied table or fallback constant exists.

The mapped values remain data. Numeric profile targets are compared and decoded
by the already reconstructed bodies; they are never invoked as host C++ function
pointers.

## Platform load-pump binding

The full application `PlatformServices` object implements
`ResourceLoadEventHost` directly and derives its cursor, input and online work
from that platform owner. The current `GameStartupHost` route instead exposes
`GamePlatformServices::load_events()`, a `GameSoundLoadEvents` adapter over the
application's existing XLive/input/online services. That adapter is a genuine
load-event provider, but it is not by itself proof of the object identity stored
in `0109CF04`.

For that reason the runtime preserves the acquisition contract as three borrowed
inputs: the live `0109CF04` publication, the explicitly bound owner identity and
the existing `ResourceLoadEventHost`. `870DD0` captures and verifies the current
publication before name work, then calls the recovered `BECCD0` pump through the
supplied host. A publication change or mismatched binding is rejected; the
runtime does not manufacture a pump or choose a default host.

## Component-reader composition

`NativeParticleComponentReaderRawContext` remains a small outer caller context.
Applications compose it from `resource_cache()`, `acquisition()` and their live
CRT SSE2-conversion boolean. Its acquired frame remains caller-owned for the same
nested-failure reason. The component implementation and its CMake registration
are integrated with this batch; the particle runtime does not duplicate that
reader or project the mutable DWORD feature word into a new boolean owner.

## Validation and limits

The runtime translation unit compiles in the strict MSVC Win32 target together
with the complete acquisition, component-reader and VFS-date implementations.
The final report records seed verification, full build and CTest receipts.

Existing focused fixtures separately prove the genuine raw loader/parser,
recursive emitter/type factories, Object-resource loading, texture records,
loaded lifetime and `870DD0` acquisition paths. No single retained fixture owns
all application services required by this runtime: the loader/acquisition fixture
lacks the texture/render owners, while the factory fixture intentionally uses a
null texture branch. Combining them would require new rejecting callback families
or inactive fake owners, contrary to this composition's purpose. No synthetic
runtime smoke was treated as stronger evidence than the strict linked build and
the existing per-domain probes.

This establishes source composition and application-facing ownership rules. It
does not install the runtime in `GameStartupHost`, reproduce original register or
FH3/SEH stack identity, prove hardware-fault/CRT allocation equivalence, or show
gameplay behavior in the retail executable.
