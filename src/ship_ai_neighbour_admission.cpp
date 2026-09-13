#include "bsp/ship_ai_neighbour_admission.hpp"
#include "bsp/unit_kind_query.hpp"
#include "bsp/vehicle_class.hpp"

namespace bsp {
namespace {
float lifetime_009f0dcd(const float& memory) noexcept {
    float result;
    const float* input = &memory;
    const double* span = &kShipAiNeighbourLifetimeSeconds;
    __asm {
        mov eax, input
        fld dword ptr [eax]
        mov eax, span
        fadd qword ptr [eax]
        fstp result
    }
    return result;
}
bool extends_009f0dff(const float& fresh, const float& old) noexcept {
    unsigned char answer;
    const float* fresh_ptr = &fresh;
    const float* old_ptr = &old;
    __asm {
        mov eax, fresh_ptr
        fld dword ptr [eax]
        mov eax, old_ptr
        fld dword ptr [eax]
        fxch st(1)
        fcomi st, st(1)
        fstp st(1)
        seta answer
        fstp st(0)
    }
    return answer != 0;
}
} // namespace

bool ship_ai_class_small_surface_00827f70(
    const void* actual_class, ShipAiNeighbourAdmissionClassHost& host) {
    if (host.class_is_kind_vtable18(
            actual_class, static_cast<int>(VehicleClassKind::TorpedoBoat))) {
        return true;
    }
    if (host.class_is_kind_vtable18(
            actual_class, static_cast<int>(VehicleClassKind::LandingShip))) {
        return host.big_landing_ship_808(actual_class) == 0;
    }
    return false;
}

void ship_ai_neighbour_admit_009f0d20(
    const ShipAiNeighbourAdmissionView& view, const void* candidate,
    ShipAiNeighbourAdmissionHost& host) {
    if (view.count_604 >= kShipAiNeighbourListCapacity) return; // signed JGE

    if (host.unit_view(view.self_unit_3fc).field_6b8 >= 0 && candidate &&
        host.unit_is_kind_vtable5c(candidate, kUnitKindQuerySubmarine)) return;

    const void* const actual_class = host.unit_view(view.self_unit_3fc).class_538;
    if (!ship_ai_class_small_surface_00827f70(actual_class, host) &&
        !host.unit_is_kind_vtable5c(view.self_unit_3fc, kUnitKindQuerySubmarine)) {
        // Native captures candidate party before reloading the current self.
        const std::int32_t candidate_party = host.unit_view(candidate).party_54;
        const std::int32_t self_party = host.unit_view(view.self_unit_3fc).party_54;
        if (self_party != candidate_party && candidate_party != 2 &&
            host.unit_is_kind_vtable5c(candidate, kUnitKindQuerySubmarine)) return;
    }

    const float lifetime = lifetime_009f0dcd(host.neighbour_memory_194_00424c40());
    bool fresh = true;
    for (std::int32_t i = 0; i < view.count_604; ++i) {
        ShipAiObstacleNode& node = *view.slots_608[i];
        if (node.owner == candidate) {
            if (extends_009f0dff(lifetime, node.lifetime_78)) {
                node.lifetime_78 = lifetime;
            }
            fresh = false; // every duplicate is visited, including NaN lifetimes
        }
    }
    if (!fresh) return;

    void* const allocation = host.allocate_node_00bf681b(0x90u);
    ShipAiObstacleNode* node = nullptr;
    if (allocation) {
        node = host.construct_node_009e52e0(
            allocation, candidate, view.controller_identity, lifetime);
    }
    // Both callbacks can change live count: load it again after construction.
    view.slots_608[view.count_604] = node;
    ++view.count_604;
}
} // namespace bsp
