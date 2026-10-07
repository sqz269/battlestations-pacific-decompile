#include "bsp/native_land_moveto_constructor.hpp"
#include "bsp/observer_edges.hpp"
#include <stdexcept>

namespace bsp {

NativeLandMoveToConstructorView native_land_moveto_constructor_view(
    void* root, std::size_t bytes, const NativeLandStateCleanupView& state,
    const void* volatile& owner, std::uint32_t volatile& word_08,
    NativeObserverOwnerStorage& callback, std::uint8_t volatile& byte_28,
    NativeObserverOwnerStorage* volatile& first,
    std::uint32_t volatile& word_30, std::uint32_t volatile& word_34,
    std::uint32_t volatile& word_38) {
    auto* const base = static_cast<unsigned char*>(root);
    if (!base || bytes < 0x3c ||
        static_cast<const volatile void*>(&state.profile_00) != base ||
        static_cast<const volatile void*>(&owner) != base + 4 ||
        static_cast<const volatile void*>(&word_08) != base + 8 ||
        static_cast<const volatile void*>(&state.elements_0c) != base + 0x0c ||
        static_cast<const volatile void*>(&callback) != base + 0x18 ||
        static_cast<const volatile void*>(&byte_28) != base + 0x28 ||
        static_cast<const volatile void*>(&first) != base + 0x2c ||
        static_cast<const volatile void*>(&word_30) != base + 0x30 ||
        static_cast<const volatile void*>(&word_34) != base + 0x34 ||
        static_cast<const volatile void*>(&word_38) != base + 0x38)
        throw std::logic_error("native MoveTo constructor requires same-root >=3Ch fields");
    return {root, state, owner, word_08, callback, byte_28, first,
        word_30, word_34, word_38};
}

NativeLandMoveToCleanupView native_land_moveto_cleanup_view(
    const NativeLandMoveToConstructorView& state) noexcept {
    return {state.state, state.callback_18};
}

void* construct_native_land_moveto_009c2ac0(
    const NativeLandMoveToConstructorView& state, const void* owner,
    NativeObserverOwnerStorage* const first, const std::uint32_t word_30,
    const std::uint32_t word_34, const std::uint32_t word_38,
    NativeObserverLifetime& lifetime) {
    state.owner_04 = owner;
    state.state.elements_0c.data_00 = nullptr;
    state.state.elements_0c.count_04 = 0;
    state.state.elements_0c.capacity_08 = 0;
    state.word_08 = 0;
    state.callback_18.native_vtable_00 = 0x00ce3cd4u;
    state.callback_18.edges_04.data_00 = nullptr;
    state.callback_18.edges_04.count_04 = 0;
    state.callback_18.edges_04.capacity_08 = 0;
    state.byte_28 = 0;
    state.word_30 = word_30;
    state.word_34 = word_34;
    state.state.profile_00 = 0x00d20aecu;
    state.callback_18.native_vtable_00 = 0x00d20ad4u;
    state.first_endpoint_2c = nullptr;
    state.word_38 = word_38;
    if (first) {
        state.first_endpoint_2c = first;
        register_observer_pair_00694a60(*first, state.callback_18, lifetime);
    }
    return state.actual_root;
}

} // namespace bsp
