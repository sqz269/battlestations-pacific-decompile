#pragma once
#include "bsp/ship_ai_neighbour_box.hpp"
#include "bsp/system_camera_axes.hpp"
#include <cstdint>

namespace bsp {

// Borrowed projections of produced fields, not native unit/controller layouts.
// References remain valid across callbacks. Accessors returning views/bindings
// only expose storage; they do not run additional world updates.
struct ShipAiNeighbourFrameUnitView {
    const void* identity;
    const std::uint8_t& pose_valid_c8;
    const float& world_x_fc;
    const float& world_z_104;
    const void* const& class_538;
    const float& full_length_9c8;
    const float& full_beam_9cc;
};
struct ShipAiNeighbourFrameSettingsView {
    const std::uint8_t& side_filter_04;
    const float& minimum_speed_ratio_1b8;
};
struct ShipAiNeighbourFrameView {
    // Actual stable slot backing, at least as many slots as the live count.
    // Native backing has capacity128; callbacks must not replace/reallocate it.
    ShipAiObstacleNode** slots_608;
    std::int32_t& count_604;
    const std::int32_t& party_3f0;
    const float& bounds_max_y_1bc;
    const float& bounds_min_y_1c0;
};
struct ShipAiNeighbourFrameNodeBindings {
    ShipAiNeighbourNodeMotion& motion;
    ShipAiNeighbourNearBoxHost& near_host;
    ShipAiNeighbourAvoidBoxHost& avoid_host;
};
struct ShipAiNeighbourFrameHost {
    virtual ~ShipAiNeighbourFrameHost() = default;
    virtual ShipAiNeighbourFrameUnitView self_unit_3fc() = 0;
    virtual const float& class_top_speed_500(const void* actual_class) = 0;
    // Actual singleton lookup at each call, not a fabricated settings owner.
    virtual ShipAiNeighbourFrameSettingsView settings_00424c40() = 0;
    virtual void refresh_pose_00414db0(const void* captured_unit) = 0;
    // Native virtual+34 returns a pointer to three floats; X/Z are consumed.
    // The host must bind the actual unit's implementation, not a guessed vector.
    virtual std::array<float,3> world_velocity_vtable34(const void* unit) = 0;
    virtual std::uint8_t owner_gone_5e(const void* observed_owner) = 0;
    virtual std::int32_t owner_party_54(const void* observed_owner) = 0;
    // 0080E160 reads unit+738; then the caller reads that DIRECTOR's +241.
    virtual std::uint8_t director_side_filter_241_0080e160(const void* unit) = 0;
    virtual ShipAiNeighbourFrameNodeBindings node_bindings(ShipAiObstacleNode&) = 0;
    // Destroy without freeing, then free the original allocation. No stand-in
    // observer destructor or allocator is supplied by this semantic routine.
    virtual void destroy_node_0064a610(ShipAiObstacleNode&) = 0;
    virtual void free_node_00bf65ac(ShipAiObstacleNode*) = 0;
};

// Complete normal frame preparation/ordering/ageing/compaction projection of
// 009F0EA0..009F115D. Native ECX=controller, stackdt, RET4; new typed interface.
// Existing near/avoid projections are called directly. Their own documented
// math limits (including the shrink-NaN discrepancy) remain external to this
// packet. Actual observer destruction/free and native raw ABI remain external.
// No node initialization occurs here: fresh nodes use the L constructor-field
// projection and retain their geometry/motion state between frames.
ShipAiNeighbourRefreshResult ship_ai_neighbour_frame_refresh_009f0ea0(
    const ShipAiNeighbourFrameView&, float dt, ShipAiNeighbourFrameHost&,
    const CameraAxesCrtAccess&);

} // namespace bsp
