# World registration runtime binding

Addresses: 009288F1, 00925906, 00484540, 004B7EC0, 004CB076.

The runtime previously applied the destroyer registrar 006FE620 to every
created unit. This put planes and buildings into ship list6. The binding now
selects the registrar from the actual descriptor creator, using the21
constructor/vtable/registrar mappings in UNIT_WORLD_REGISTRATION.md. Missing
creators remain unresolved; no destroyer fallback is supplied.

Each runtime unit stores its actual mission registry owner in the +30h field
projection, following the store at00925906 inside009258F0. Every list push
reloads that field through the recovered registrar host. The registry is the
existing mission-owned collection. This does not reconstruct the surrounding
placement locks, hierarchy links, scene-node construction or whole4BCh world.

The registry now retains97 count/head/tail triples and stable12-byte nodes
with previous/next/direct-unit pointers.004B7EC0 clears each triple.00484540
allocates12, clears the node words, stores payload at+8 and old tail at+0,
links the old tail or empty head, then publishes tail/next/count. The runtime
implements that normal nonnull-allocation store sequence and preserves the
actual node identities. It does not emulate a custom native allocator returning
null, native exception unwind or original allocation addresses.

Existing index-based readers now traverse those nodes. Opaque head/payload/next
accessors expose the same storage for the upcoming candidate walker, so it can
reload next after an admission callback. No candidate walk is enabled by this
change: its timer/RNG/lock and complete node/observer owners still need binding.
The registry destructor releases process-owned nodes; it is not claimed as the
native world detach/observer teardown sequence.

Runtime reporting checks each chain's count, tail, previous links and unit
parent identity, and records unresolved registrations plus ship/plane list
counts. Native register and push behavior is covered by the N worker fixture;
the private runtime binding is separately verified through the built mission
process and its list audit. No original-game gameplay or ABI parity is claimed.

## Constructor label correction

The older world header and GAME_WORLD_CONSTRUCT.md reversed the two vector
callbacks. At004CB060 the code pushes004C2D30 (destructor);004CB065 pushes
004B7EC0 (constructor), then count97, stride12 and destination world+18 before
the00BF7CD1 iterator call. The complete004B7EC0..004B7ECC body clears all three
words and returns. The source header is corrected; the older document receives
an appended correction while retaining its original record.

Validation and exact compiled/runtime commits are recorded in
reports/unit_world_registration_live.json after the combined check.
