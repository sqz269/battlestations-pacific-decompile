#include "bsp/native_unit_observer_callbacks.hpp"

#include <cstring>

namespace bsp {

void native_plane_observer_callback_007c6e90(
    NativePlaneObserverCallbackView plane, NativeObserverOwnerStorage& first,
    NativePlaneObserverCallbackAccess& access) {
    auto* const target = access.observed_virtual_04(first);
    if (target == nullptr || !access.observed_virtual_5c(target, 0x45)) {
        if (!access.observed_virtual_5c(target, 9)) return;
        if (access.effective_game_mode_004bca50() == 9) return;
    }
    if (plane.squadron_9d4 == nullptr) return;
    if (plane.byte_5c == 0) return;
    if (plane.byte_5d != 0) return;
    if (plane.byte_60 != 0) return;
    if (plane.byte_5e != 0) return;
    if (plane.byte_60 != 0) return;
    if (plane.byte_5d != 0) return;
    access.call_007c5ac0(plane.alias, -1.0f);
}

namespace {
std::uintptr_t address(std::byte* pointer) noexcept {
    return reinterpret_cast<std::uintptr_t>(pointer);
}

// The two concrete callback loops share the same reachable validation order.
// CMP EDI,EDI is always equal: its native invalid-parameter edge is unreachable.
// This scans only these two already-owned record ranges; no STL helper is ported.
void clear_first(NativeShipyardObserverVectorView vector, std::size_t stride,
    std::size_t observed_offset, bool clear_two_words,
    NativeObserverOwnerStorage* first, ObserverLifetimeServices& services) {
    auto iterator = address(vector.begin);
    if (iterator > address(vector.end)) services.invalid_parameter_00bf6713();
    for (;;) {
        const auto captured_end = address(vector.end);
        if (address(vector.begin) > captured_end) services.invalid_parameter_00bf6713();
        if (iterator == captured_end) return;
        if (iterator >= address(vector.end)) services.invalid_parameter_00bf6713();
        auto* const record = reinterpret_cast<std::byte*>(iterator);
        NativeObserverOwnerStorage* candidate;
        std::memcpy(&candidate, record + observed_offset, sizeof(candidate));
        if (candidate == first) {
            const std::uint32_t zero = 0;
            if (clear_two_words) {
                std::memcpy(record + 4, &zero, sizeof(zero));
                std::memcpy(record + 8, &zero, sizeof(zero));
            }
            std::memcpy(record + observed_offset, &zero, sizeof(zero));
            return;
        }
        if (iterator >= address(vector.end)) services.invalid_parameter_00bf6713();
        iterator += stride;
    }
}
} // namespace

void native_shipyard_observer_callback_008455a0(
    NativeShipyardObserverCallbackView shipyard, NativeObserverOwnerStorage* first,
    ObserverLifetimeServices& services) {
    clear_first(shipyard.vector_780, 0x10, 0x0c, false, first, services);
    clear_first(shipyard.vector_790, 0x4c, 0x30, true, first, services);
}

void native_unit_observer_noop_0080dfc0() noexcept {}
void native_unit_observer_noop_00952050() noexcept {}

void native_observer_noop_0042b120() noexcept {}

} // namespace bsp
