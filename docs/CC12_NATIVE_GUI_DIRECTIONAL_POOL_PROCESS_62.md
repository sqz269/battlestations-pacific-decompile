# Canonical directional pool process owner 62

Source commit `f4ea5840accdde8ba042a33845352a7ca7baee1b` appends the
missing directional-light pool owner to `GameNativeResourcePoolProcess`.
The distinct 56-byte `01090154` Source storage and companion borrow the
existing process allocator domain `00E188B4`. Existing resource members stay
in their previous declaration order; no second allocator list is introduced.

The companion constructor binds dispatch metadata only. Native list insertion,
critical-section initialization and table setup remain in the existing
`DirectionalLightPool::initialize_00b7b940`. New explicit
`initialize_directional_light_once_00cd8080()` follows the resource process's
existing mutex/state pattern: mark the attempt before binding or initialization,
reuse the actual static adapter, cache its exact atexit return status, and reject
replay after a thrown attempt. A returned nonzero status does not undo native
initialization or authorize a second registration attempt. The accessor rejects
before returned startup.

Existing CD8080 registers CE0EB0 through the real CRT. Process construction
precedes that registration, and allocator-process construction precedes the
resource process, so the callback runs while both owners exist. C++ companion
destruction performs no native teardown. Every payload and companion must be
finished and its actual slot returned before the pool's CRT callback.

Independent Source review by `native_text_owner_readiness_36` passed for the
exact committed header/cpp blobs. Canonical MSVC Win32 build and all three
existing CTests passed (10.79 seconds), including the latest Main scene-list
merge. The pre-existing duplicate `spawn_request_id_matches` LNK4006 warning
remains; it was already present in the preceding Main build. No new tests or
CMake entries were added. Existing directional pool bodies remain unchanged.

This packet adds Source ownership and explicit admission only. It does not call
the initializer from normal startup, execute a new pool/CRT runtime case,
construct a light, establish type identities, or activate a Scene/store/page
manager/Screen. It adds no newly recovered Original body, Ghidra annotation,
fixed-VA identity or binary ABI claim. Exact Source/object/build hashes and a
checked archive are recorded in the JSON receipt.
