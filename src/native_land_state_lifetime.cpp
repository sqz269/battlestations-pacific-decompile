#include "bsp/native_land_state_lifetime.hpp"
#include "bsp/observer_edges.hpp"
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uint32_t kBaseProfile = 0x00cf5c20u;
constexpr std::uint32_t kElementProfile = 0x00cf5c94u;

void destroy_vector_element(NativeLandStateObserverElement24& element,
    NativeObserverLifetime& lifetime) {
    // Native callers freshly load element.table->slot0. Only this proved
    // scalar profile is admitted; raw image tables remain uncallable in SOURCE.
    if (element.callback_00.native_vtable_00 != kElementProfile)
        throw std::logic_error("unsupported native land state element profile");
    scalar_delete_native_land_state_element_0064b5f0(element, 0, lifetime);
}
} // namespace

NativeLandStateObserverElement24* copy_native_land_state_element_007b3fc0(
    NativeLandStateObserverElement24& destination,
    const NativeLandStateObserverElement24& source, NativeObserverLifetime& lifetime) {
    destination.callback_00.edges_04.data_00 = nullptr;
    destination.callback_00.edges_04.count_04 = 0;
    destination.callback_00.edges_04.capacity_08 = 0;
    destination.enabled_10 = 0;
    destination.callback_00.native_vtable_00 = kBaseProfile;
    auto* const first = source.first_endpoint_14;
    destination.first_endpoint_14 = first;
    destination.enabled_10 = 1;
    if (first) register_observer_pair_00694a60(*first, destination.callback_00, lifetime);
    return &destination;
}

void destroy_native_land_state_element_base_0064a610(
    NativeLandStateObserverElement24& element, NativeObserverLifetime& lifetime) {
    element.callback_00.native_vtable_00 = kBaseProfile;
    auto* const first = element.first_endpoint_14;
    if (first) lifetime.unregister_pair_006952a0(*first, element.callback_00);
    lifetime.destroy_callback_owner_00695870(element.callback_00);
}

NativeLandStateObserverElement24* scalar_delete_native_land_state_element_0064b5f0(
    NativeLandStateObserverElement24& element, const std::uint32_t flags,
    NativeObserverLifetime& lifetime) {
    auto* const identity = &element;
    destroy_native_land_state_element_base_0064a610(element, lifetime);
    const volatile bool release_self = (static_cast<std::uint8_t>(flags) & 1u) != 0;
    if (release_self) singleton_lifetime_free(identity);
    return identity;
}

void reserve_native_land_state_elements_007b4400(
    NativeLandStateObserverVector& vector, const std::int32_t capacity,
    NativeObserverLifetime& lifetime) {
    // The native stack parameter is reloaded after free, before publication.
    volatile std::int32_t requested_capacity = capacity;
    if (requested_capacity < 1) requested_capacity = 1;
    if (vector.capacity_08 >= requested_capacity) return;
    const std::uint32_t bytes = static_cast<std::uint32_t>(requested_capacity) * 0x18u;
    auto* const replacement = static_cast<NativeLandStateObserverElement24*>(
        singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes}));
    std::int32_t index = 0;
    while (index < vector.count_04) {
        auto* const incoming = vector.data_00 + index;
        auto& outgoing = replacement[index];
        copy_native_land_state_element_007b3fc0(outgoing, *incoming, lifetime);
        outgoing.callback_00.native_vtable_00 = kElementProfile;
        ++index;
    }
    index = 0;
    while (index < vector.count_04) {
        auto& current = vector.data_00[index];
        destroy_vector_element(current, lifetime);
        ++index;
    }
    auto* const retired = vector.data_00;
    singleton_lifetime_free(retired);
    const std::int32_t published_capacity = requested_capacity;
    vector.data_00 = replacement;
    vector.capacity_08 = published_capacity;
}

void resize_native_land_state_elements_007b4500(
    NativeLandStateObserverVector& vector, const std::int32_t count,
    NativeObserverLifetime& lifetime) {
    if (count > vector.capacity_08)
        reserve_native_land_state_elements_007b4400(vector, count, lifetime);
    const std::int32_t old_count = vector.count_04;
    for (std::int32_t index = old_count; index < count; ++index) {
        auto& element = vector.data_00[index];
        element.callback_00.edges_04.data_00 = nullptr;
        element.callback_00.edges_04.count_04 = 0;
        element.callback_00.edges_04.capacity_08 = 0;
        element.first_endpoint_14 = nullptr;
        element.enabled_10 = 1;
        element.callback_00.native_vtable_00 = kElementProfile;
    }
    while (count < vector.count_04) {
        --vector.count_04;
        const std::int32_t index = vector.count_04;
        auto& current = vector.data_00[index];
        destroy_vector_element(current, lifetime);
    }
    vector.count_04 = count;
}

void destroy_native_land_state_007b45f0(
    const NativeLandStateCleanupView& state, NativeObserverLifetime& lifetime) {
    resize_native_land_state_elements_007b4500(state.elements_0c, 0, lifetime);
    auto* const retired = state.elements_0c.data_00;
    singleton_lifetime_free(retired);
    state.profile_00 = 0x00d056d0u;
}

void destroy_native_land_moveto_007b65e0(
    const NativeLandMoveToCleanupView& state, NativeObserverLifetime& lifetime) {
    lifetime.destroy_callback_owner_00695870(state.callback_18);
    destroy_native_land_state_007b45f0(state.state, lifetime);
}

} // namespace bsp
