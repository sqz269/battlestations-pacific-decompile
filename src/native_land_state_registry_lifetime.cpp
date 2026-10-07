#include "bsp/native_land_state_registry_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

void release_native_bot_state_vector_00411610(NativeBotStateVectorStorage& vector) {
    // 411613 captures begin once; the returning free continuation precedes ALL
    // clears. Proxy+0 and borrowed pair pointees are not destroyed.
    auto* const captured_begin = vector.begin_04;
    if (captured_begin != nullptr) {
        singleton_lifetime_free(captured_begin);
    }
    vector.begin_04 = nullptr;
    vector.end_08 = nullptr;
    vector.capacity_end_0c = nullptr;
}

void destroy_native_bot_state_registry_004116d0(
    NativeBotStateRegistryStorage& registry) {
    // 4116D3 restamps BEFORE 4116D9 captures array; ordinary path never frees
    // the registry receiver. The raw numeric profile remains uncallable.
    registry.profile_word_00 = 0x00ce37dcu;
    auto* const captured_begin = registry.vector_04.begin_04;
    if (captured_begin != nullptr) {
        singleton_lifetime_free(captured_begin);
    }
    registry.vector_04.begin_04 = nullptr;
    registry.vector_04.end_08 = nullptr;
    registry.vector_04.capacity_end_0c = nullptr;
}

NativeBotStateRegistryStorage* scalar_delete_native_bot_state_registry_00411810(
    NativeBotStateRegistryStorage& registry, std::uint32_t flags) {
    auto* const captured_identity = &registry;
    registry.profile_word_00 = 0x00ce37dcu;
    auto* const captured_begin = registry.vector_04.begin_04;
    if (captured_begin != nullptr) {
        singleton_lifetime_free(captured_begin);
    }
    // 411829 TEST byteflags,1 occurs after the buffer-free return but before
    // 41182E/835/83C clears. Volatile capture retains this SOURCE evaluation
    // order; this is not an instruction-identical binary/stack ABI bridge.
    const volatile bool release_self = (static_cast<std::uint8_t>(flags) & 1u) != 0;
    registry.vector_04.begin_04 = nullptr;
    registry.vector_04.end_08 = nullptr;
    registry.vector_04.capacity_end_0c = nullptr;
    if (release_self) {
        singleton_lifetime_free(captured_identity);
    }
    return captured_identity;
}

} // namespace bsp
