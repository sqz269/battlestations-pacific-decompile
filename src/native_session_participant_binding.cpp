#include "bsp/native_session_participant_binding.hpp"
#include "bsp/observer_edges.hpp"
#include <cstdint>
#include <cstring>

namespace bsp {
void bind_native_session_participant_0077cc50(void* actual_participant,
    NativeObserverOwnerStorage* requested_first, NativeObserverLifetime& lifetime) {
    auto* const bytes = static_cast<std::uint8_t*>(actual_participant);
    auto& callback = *reinterpret_cast<NativeObserverOwnerStorage*>(bytes + 0x38);
    NativeObserverOwnerStorage* captured_old;
    std::memcpy(&captured_old, bytes + 0x4c, sizeof(captured_old));
    if (captured_old == requested_first) return;
    if (captured_old) lifetime.unregister_pair_006952a0(*captured_old, callback);
    // Native 0077CC6D stores even when the requested endpoint is null, and
    // only after unregister returns. A registration failure retains this store.
    std::memcpy(bytes + 0x4c, &requested_first, sizeof(requested_first));
    if (requested_first) register_observer_pair_00694a60(*requested_first, callback, lifetime);
}
} // namespace bsp
