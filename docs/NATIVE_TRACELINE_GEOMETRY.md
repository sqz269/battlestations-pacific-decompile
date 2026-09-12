# Traceline geometry attachment and dependency composition

Addresses: 00AF3440, 00AF3430, 00B0B6A0, 00B0A840, 00AFFCB0, 00AFFD20

`AF3440..AF3748` is the complete normal attachment body selected by current
Traceline table `D0C928+5C`. Its original ABI is ECX actual 1BCh node, two stack
arguments (actual 80h payload and root), RET8. The descriptive name is a
hypothesis, not a recovered symbol. All 777 bytes match the installed PE and
live `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr`.

The routine captures optional `F8C284`, enters its actual critical section and
increments depth18. It publishes payload184, conditionally converts signed
payload8 to node1A0 with CVTSI2SS, allocates payload20 times14h with unsigned
overflow saturation to FFFFFFFF, publishes node188, and clears190 then18C.
It allocates/constructs the canonical mesh, associates it through B75170 with
two x87-converted reads from the single D7A260 load, then calls current renderer
5C with `(2*payload20+2, 1000h, payload40)`. Integer count arithmetic wraps.

The new stream is attached to mesh0 before its creator reference is released.
The routine creates a draw section and material. All three payload48 blend
branches pass the same `traceline.mshd` string; they differ in their temporary
stack headers. `AnimFrac` registers a borrowed pointer to node1AC. Texture0
comes from payload4C[0]+8. Material assignment precedes its creator release.
Section primitive8 is5; stores clear0C,14,10,18 in that order. Mesh stream0 is
appended, payload44 becomes the section layout, and the section is appended to
the mesh. Creator releases are section then mesh. B6D890 propagates the supplied
root before the captured section is unlocked. No native payload retain occurs.

The implementation reuses `GuiNativeGeometryOwners`, whose mesh, section and
material methods manage actual canonical pools and reference companions. Its
name does not introduce another geometry domain. Model, geometry and material
references use the same `NativeRenderActualOwners`; material parameters and
destruction use the same actual string and parameter pools. Renderer5C and48
must be real current callable native-ABI services over the application's actual
renderer. A semantic stream/effect is not an implementation of that contract.

The public C++ function receives the existing `NativeModelOwner` and canonical
`RenderNodeRootList` view. Raw root storage cannot be cast to that C++ view.
This interface has precondition checks and ordinary C++ cleanup; original FH3,
hardware faults, allocation failure behavior, stack identity and binary ABI
are outside its supported normal domain. Temporary native creator references
are not broadly rolled back after a later failure. Existing companion factories
have their documented host-registration failure behavior.

Optional `NativeParticleTracerReconstruction` now connects the established
Tracer caller to actual AF32F0, AF3440, B0A110 and payload86ADE0. The application
only supplies registration of the already constructed Traceline companion and
lookup of its existing root view. Known calls require their concrete bindings;
unknown current targets retain the existing application dispatch. Binding checks
require the exact canonical node runtime, raw identity, lifetime access, owner
registry, string domain, F8C288 pool and renderer publication. Applications that
have not enabled this composition retain their existing real service interface.
The existing AFFCB0/AFFD20 implementations gain public naked tail entries so
the update routine can preserve caller-owned x87 stack values without duplicating
the curve integrals.

Second-pass review also requires geometry and model setter sentinel references
to address the same D7A260 cell, including reentrant changes during old geometry
release. State and geometry share the identical BF55BE allocation callback and
the lifetime uses the identical BF6989 free callback. BF681B/BF65AC scalar
allocation/free pairing remains an explicit application precondition. A focused
invalid-sentinel binding case rejects before native mutation. The public optional
allocation/cleanup paths also pass with actual pool, payload resources and
canonical node terminal destruction; a mismatched free callback rejects before
ownership mutation, and unknown targets keep the required application dispatch.
Those composition checks begin from valid constructed cleanup input; they do not
establish successful AF3440 attachment or its following initial update.

Verification uses a disposable original-byte probe at
`C:/Users/sqz269/bsp-as-geometry`. Twenty-one native/source comparisons cover
six allocation/flag/integer-conversion cases and one overflowing-size stop under
three x87 control words. The ordinary cases execute actual mesh-pool allocation,
mesh construction, model association and canonical companion registration, then
stop explicitly before renderer5C. They compare node/mesh mutations, allocation
requests, reference counts, captured-lock depth, x87 CW/SW and a live ST0 sentinel.
The native side retains its actual caller instructions and uses independently
reconstructed existing callees. The suffix after renderer5C is assembly/source
reviewed and compiled, not fixture-executed. The overflow case stops at the real
allocation boundary before a 4GB request is attempted. Fixture-only continue-search
SEH handling and `/SAFESEH:NO` do not establish native FH3 compatibility.

Initial strict MSVC Win32 build and both existing seeded CTests passed. Final
combined-commit validation and current-library replays are recorded with the AS
publication evidence. No game or visual rendering validation is claimed.

Follow-up packets: implement and compose the actual renderer vertex-stream
factory, exercise the full attachment suffix with real material/effect/stream
owners, complete AF26A0 geometry filling and submission, then validate a live
particle path. Renderer declaration loading B317E0 and resource slow-path
coverage remain separate dependencies.
