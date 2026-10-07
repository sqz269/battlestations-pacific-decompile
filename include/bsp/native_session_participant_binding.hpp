#pragma once

namespace bsp {
class NativeObserverLifetime;
struct NativeObserverOwnerStorage;

// Complete normal 0077CC50..0077CC7F body (48 bytes). Native ECX is the
// actual 118h participant, stack argument is the already-adjusted observed
// endpoint, RET4; no EAX result is promised. Participant+38h is its callback
// owner and +4Ch holds the first endpoint. Equal identities perform no work.
// Otherwise unregister the captured old pair, publish the requested pointer,
// then register the new pair. The borrowed lifetime is the existing shared
// observer service. This function neither constructs nor owns either input.
// New C++ interface; not an original binary-ABI replacement.
void bind_native_session_participant_0077cc50(void* actual_participant,
    NativeObserverOwnerStorage* requested_first, NativeObserverLifetime&);
} // namespace bsp
