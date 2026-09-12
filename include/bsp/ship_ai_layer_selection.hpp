#pragma once

#include "bsp/avoid_zone_clearance.hpp"
#include "bsp/avoid_zone_geometry.hpp"
#include "bsp/ship_ai_hull_geometry.hpp"
#include "bsp/ship_ai_states.hpp"
#include "bsp/vehicle_class_lua_load.hpp"

#include <array>
#include <cstdint>

namespace bsp {
// Complete native virtual214 getters. ECX=unit, no stack arguments, EAX bits,
// RET. Actual class560..56C producer is ShipLeafTuning.array, not its scalar570.
// Submarine depth1268 must be the produced index0..3; an invalid C++ view throws
// rather than reading beyond the actual four-slot descriptor array.
std::uint32_t ship_ai_unit_navigation_layer_006dfd80(const ShipLeafTuning&) noexcept;
std::uint32_t ship_ai_submarine_navigation_layer_00852fd0(
    const ShipLeafTuning&, std::int32_t depth_level_1268);

// Complete manager leaf bodies. ECX=manager, first/last RET; next/previous RET4
// with signed key. Empty tables return0. Order is actual manager slot order;
// next returns first strictly greater or last; previous returns last strictly
// lower or first. These functions do not sort or invent groups.
std::int32_t avoid_zone_first_layer_00412170(const AvoidZoneTable&) noexcept;
std::int32_t avoid_zone_last_layer_00412180(const AvoidZoneTable&) noexcept;
std::int32_t avoid_zone_next_layer_004121b0(const AvoidZoneTable&, std::int32_t) noexcept;
std::int32_t avoid_zone_previous_layer_00417d60(const AvoidZoneTable&, std::int32_t) noexcept;
// Native body is exactly RET4: it does not mutate the manager or select a key.
void avoid_zone_layer_noop_004121a0(std::int32_t) noexcept;

// Complete00417B10..00417BF9. ECX=group; out,point,push,containment byte; EAX=out,
// RET10h. Copy point, then walk every zone in order against the UPDATED output.
// Native and semantic groups must describe the same ordered polygons. Reuses
// existing00416B50/00416F30; no closest-candidate native-invalid case is repaired.
void avoid_zone_group_offset_00417b10(const AvoidZoneLayerGroup&,
    const AvoidZoneClearanceGroupView&, std::array<float, 2>& output,
    const std::array<float, 2>& point, float push, std::uint8_t test_containment);

// Borrow exactly the six already established GameplayTuningSettings fields.
// Root may bind an actual Lua-produced narrow owner without fabricating the
// remainder of the76Ch singleton. A returned view remains valid across the
// following settings lookup, as the native keeps the first pointer in EDI.
struct ShipAiLayerTimingView {
    const float& move_min_1f4;
    const float& move_max_1f8;
    const float& ship_min_1fc;
    const float& ship_max_200;
    const float& travel_min_204;
    const float& travel_max_208;
};

// References bind the canonical existing navigation fields; no owned block or
// initial values are introduced. In particular314 aliases the goal setter's
// crossing_314,164 the ctor's value_164, and160 its flag_160. Every reference
// remains valid throughout the call, including across host callbacks.
struct ShipAiLayerSelectionView {
    float& timer_148;
    float& escape_distance_14c;
    float& escape_x_150;
    float& escape_z_154;
    float& last_probe_x_158;
    float& last_probe_z_15c;
    bool& escaping_160;
    std::uint32_t& position_layer_164;
    std::uint32_t& class_floor_16c;
    float& timer_170;
    std::uint32_t& requested_layer_308;
    std::uint32_t& travel_layer_30c;
    std::uint32_t& goal_layer_310;
    float& timer_314;
    float& last_goal_x_31c;
    float& last_goal_z_320;
    const ShipAiHullGeometry& hull;
    const ShipAiSteeringMode& mode_1c4;
    const float& goal_x_1dc;
    const float& goal_z_1e0;
    const float& look_ahead_318;
    const float& reference_speed_3c4;
    const float& escape_limit_3c8;
};

class ShipAiLayerSelectionHost {
public:
    virtual ~ShipAiLayerSelectionHost() = default;
    virtual bool has_owner_3fc() = 0;
    virtual bool unit_group_leads_00778890() = 0;
    // External formation packet owns this concrete aggregate; do not supply a
    // fabricated zero for an existing group. Only called on the leader arm.
    virtual std::int32_t group_navigation_layer_0070e450() = 0;
    virtual std::uint32_t unit_navigation_layer_v214() = 0;
    virtual bool unit_is_kind_v5c(std::uint32_t kind) = 0;
    virtual std::uint8_t call_unit_v10c() = 0;
    virtual std::uint32_t class_navigation_floor_0560() = 0;
    // Called at every actual004218E0 site; subsequent queries use that returned
    // table. Geometry and native storage remain alive and coherent throughout.
    virtual const AvoidZoneTable& manager_004218e0() = 0;
    // C++ storage adapter only: resolve this exact selected semantic group to
    // its actual native pointer array. No singleton or layer lookup is hidden.
    virtual AvoidZoneClearanceGroupView native_group(const AvoidZoneLayerGroup&) = 0;
    virtual ShipAiLayerTimingView settings_00424c40() = 0;
    // Body00BD2E60 float-spills the result before returning ST0. Uses actual
    // registered stream state; stream is1 at all three native sites here.
    virtual float uniform_float_00bd2f10(std::uint32_t stream, float low, float high) = 0;
    virtual float unit_forward_speed_0092d730() = 0;
    virtual float unit_reference_speed_0080fc30() = 0;
    // Complete00414C60 contract: squared sum spill,1e-10 cutoff, actual CRT
    // sqrt and final float spill. Required to preserve the chosen CRT binding.
    virtual float vector_length_00414c60(const std::array<float, 2>&) = 0;
};

// Complete009ECA20..009ED3D9 through explicit actual dependencies. ECX=nav,
// stack seconds, RET4. Preserves stateful layer walks, native signed key tests,
// x87 float spill boundaries, unordered branches, timers and callback order.
// Empty actual groups are allowed; a missing selected group at a native
// dereference or mismatched storage is rejected at this C++ boundary. No loop
// cap or layer repair is invented. Caller must supply native-valid ordered
// groups and progress for the final increasing-layer walk.
// New C++ interface, not binary/exception ABI-compatible or game-validated.
void ship_ai_select_navigation_layer_009eca20(
    ShipAiLayerSelectionView, float seconds, ShipAiLayerSelectionHost&);
} // namespace bsp
