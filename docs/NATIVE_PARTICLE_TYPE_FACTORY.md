# Particle-definition type factory

Addresses: B00CE0, B00770, B076F0, B058E0, B08830, AF89E0, B0A0B0.

These seven complete source bodies construct the actual eleven-slot particle
definition family. They are separate from the six-slot emitter definitions.
Names are descriptive hypotheses, not recovered symbols. Native register
provenance, complete body bytes, direct calls, profiles and cleanup funclets
are recorded in `reports/native_particle_type_factory.json`.

B00CE0 takes the actual eight-byte kind in ECX and name in EDX, then parent
and text on the stack (RET8). Case-insensitive comparison selects Sprite,
Axial, Floating, Object or Tracer allocations of 90h, A4h, 8Ch, 98h or E8h.
The current parent+10 value is read after allocation and passed with the name
and parent to the common constructor (each constructor uses RET0C).

The common B01150 implementation owns the actual pooled name and 1Ch record
array. The six derived constructors preserve native sparse store order and
leave all other bytes unchanged. Axial writes only byte80, preserving81..83;
its84/88 stores are zero-bit MOVSS copies. Sprite and Axial receive their final
profiles in the factory after their base constructors. Floating receives its
final profile in B00770 after B076F0. Object and Tracer set their profiles in
their constructors. No whole-object initialization is added.

An unknown kind reuses the parent. A null allocation still reaches the native
child dereference. Every normal path captures the child's current vtable+08
and invokes the required real application parser with the same text buffer.
The five text parsers remain follow-up work. Constructor failure releases raw
allocation; parser failure leaves the constructed child owned by its caller.
The five analyzed unwind funclets and FH3 dispatcher establish that distinction.

The new C++ binding borrows the common particle base services, current native
profiles, and current-target parser dispatcher. It creates no alternate owner,
string pool, parameter pool or successful parser result. B01150's native
uninitialized record+18 stack word is an explicit input of its base binding.
These interfaces are not drop-in original FH3 ABI replacements.

The original-byte probe passed 33 factory comparisons and 12 derived-constructor
comparisons, including sparse object bytes, allocator changes to parent+10,
current parser target replacement, and aliasing. It compares the 24 defined
record bytes; the unknown native record+18 stack word is explicitly excluded.
Two separate C++ exception cases check constructor-allocation and parser-failure
ownership. The parser itself is a capture fixture. Combined current-library
build/replay results and exact hashes are recorded in the report. Gameplay,
native FH3 exception dispatch and full type-specific parsing remain unvalidated.

Follow-up packets: implement B08AC0/B064A0/B07D60/AF8BD0/B0AD50 current parser08
targets; recover Sprite/Object/Tracer virtual18 initialization and their real
PointLight/model resource dependencies; compose those bindings with the existing
B0CA40 initializer and validate a game startup/particle path when reachable.
