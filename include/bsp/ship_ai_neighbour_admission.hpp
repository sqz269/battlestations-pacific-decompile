#pragma once
#include "bsp/ship_ai_sector_scan.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Borrowed produced storage, not native unit/controller/descriptor layouts.
// View accessors only resolve identities and references; they perform no work.
struct ShipAiNeighbourAdmissionUnitView {
    const std::int32_t& party_54;
    const void* const& class_538;
    const std::int32_t& field_6b8;
};
struct ShipAiNeighbourAdmissionView {
    const void* controller_identity;
    const void* const& self_unit_3fc;
    std::int32_t& count_604;
    ShipAiObstacleNode** slots_608; // stable native128-slot backing
};

struct ShipAiNeighbourAdmissionClassHost {
    virtual ~ShipAiNeighbourAdmissionClassHost() = default;
    // Actual descriptor virtual+18, not the unit virtual+5C or a guessed leaf.
    virtual bool class_is_kind_vtable18(const void* actual_class, int query) = 0;
    // Read only after that same descriptor answers kind12. Raw byte semantics.
    virtual std::uint8_t big_landing_ship_808(const void* actual_class) = 0;
};

// Complete normal predicate: kind14 OR (kind12 AND BigLandingShip byte==0).
// Native ECX=descriptor, AL=result, RET0; this is a new typed interface.
bool ship_ai_class_small_surface_00827f70(
    const void* actual_class, ShipAiNeighbourAdmissionClassHost&);

struct ShipAiNeighbourAdmissionHost : ShipAiNeighbourAdmissionClassHost {
    virtual ShipAiNeighbourAdmissionUnitView unit_view(const void* actual_unit) = 0;
    virtual bool unit_is_kind_vtable5c(const void* actual_unit, int query) = 0;
    // Actual singleton call and its retained +194 float, not a live Lua read.
    virtual const float& neighbour_memory_194_00424c40() = 0;
    // Native operator_new is called with90h; its library failure/throw policy
    // remains external. If a host returns null, native still appends null.
    virtual void* allocate_node_00bf681b(std::size_t native_size) = 0;
    // COMPLETE constructor service over actual allocated storage, returning its
    // semantic node identity. It owns observer-prefix construction/registration,
    // controller+1C and all unrepresented stores. L's existing
    // ship_ai_obstacle_owner_initialize_projection_009e52e0 supplies only its
    // explicitly partial fields; it cannot replace this complete service.
    virtual ShipAiObstacleNode* construct_node_009e52e0(
        void* allocation, const void* observed_owner,
        const void* controller_identity, float lifetime) = 0;
};

// Complete normal admission/refresh/allocation sequence009F0D20..009F0E83.
// Native ECX=controller, stack candidate, RET4; no meaningful return. The older
// sector_scan helper remains partial. This caller adds no null/count repairs:
// existing slots visited must contain valid nodes; candidate may be null only
// on native paths which bypass its field reads. Owner callbacks must preserve
// the actual stable slot backing and valid storage at each native access.
// Raw90h ownership, complete constructor, CRT EH/unwind and binary ABI remain
// external; no runtime world-list membership or gameplay claim is supplied.
void ship_ai_neighbour_admit_009f0d20(
    const ShipAiNeighbourAdmissionView&, const void* candidate_identity,
    ShipAiNeighbourAdmissionHost&);

} // namespace bsp
