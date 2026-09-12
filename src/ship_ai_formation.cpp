// Packet cc_ai_formation. Evidence per routine: docs/SHIP_AI_FORMATION.md,
// reports/ship_ai_formation.json. Every name is a hypothesis.
//
// These are semantic projections over injected hosts, not binary replacements:
// the image keeps the group as a 508h-byte object reached through entity+284h
// and does its float arithmetic on the x87 stack, which this file reproduces
// as double intermediates rounded once to float where the image stores.

#include "bsp/ship_ai_formation.hpp"

#include <cmath>
#include <cstring>

#include "bsp/ship_ai_throttle_ring.hpp"  // heading_to_direction_006bc0c0
#include "bsp/unit_rudder.hpp"            // clamped_interpolate_00419010

namespace bsp {
namespace {

// 00CE3828, the float32 read of the 2pi double 009DACD0 divides and multiplies
// by. The two cancel algebraically; the image still performs both.
constexpr float kFullTurnFloat = 6.28318548f;

float float_of(std::uint32_t bits) noexcept {
    float value = 0.0f;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

std::uint32_t bits_of(float value) noexcept {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

}  // namespace

// ---------------------------------------------------------------------------
// Pattern tables
// ---------------------------------------------------------------------------

const std::array<ShipAiFormationOffset, kShipAiFormationPatternLength>*
ship_ai_formation_pattern(ShipAiFormationShape shape) noexcept {
    switch (shape) {
        case ShipAiFormationShape::kLine:
            return &kShipAiFormationPatternLine;
        case ShipAiFormationShape::kColumn:
            return &kShipAiFormationPatternColumn;
        case ShipAiFormationShape::kDiamond:
            return &kShipAiFormationPatternDiamond;
        case ShipAiFormationShape::kCustom:
        default:
            return nullptr;
    }
}

// 0070EED0-0070EF1B. Store order in the image is +14h, +24h, +18h, +28h,
// +1Ch, +2Ch; every product is one x87 multiply rounded on the store.
void ship_ai_formation_fill_table_columns(ShipAiFormationColumns& columns,
                                          std::size_t index,
                                          float ship_dist) noexcept {
    if (index >= kShipAiFormationPatternLength) {
        return;
    }
    const double dist = static_cast<double>(ship_dist);
    columns.across[1] =
        static_cast<float>(dist * static_cast<double>(kShipAiFormationPatternLine[index].across));
    columns.along[1] =
        static_cast<float>(dist * static_cast<double>(kShipAiFormationPatternLine[index].along));
    columns.across[2] =
        static_cast<float>(dist * static_cast<double>(kShipAiFormationPatternColumn[index].across));
    columns.along[2] =
        static_cast<float>(dist * static_cast<double>(kShipAiFormationPatternColumn[index].along));
    columns.across[3] =
        static_cast<float>(dist * static_cast<double>(kShipAiFormationPatternDiamond[index].across));
    columns.along[3] =
        static_cast<float>(dist * static_cast<double>(kShipAiFormationPatternDiamond[index].along));
}

// 0070F03A-0070F063. The across column is `table.x * FormationShipDist`; the
// along column carries the extra 1.5 at 00CE3D78, which 0070ED30 does not use.
ShipAiFormationOffset ship_ai_formation_reshape_slot(ShipAiFormationShape shape,
                                                     std::size_t slot,
                                                     float ship_dist) noexcept {
    const auto* table = ship_ai_formation_pattern(shape);
    if (table == nullptr || slot >= kShipAiFormationPatternLength) {
        return ShipAiFormationOffset{};
    }
    const double dist = static_cast<double>(ship_dist);
    const ShipAiFormationOffset& entry = (*table)[slot];
    ShipAiFormationOffset out{};
    out.across = static_cast<float>(static_cast<double>(entry.across) * dist);
    out.along = static_cast<float>(dist * kShipAiFormationReshapeAlongScale *
                                   static_cast<double>(entry.along));
    return out;
}

// ---------------------------------------------------------------------------
// The station point
// ---------------------------------------------------------------------------

// 0070D2F9-0070D356. (-dir_z, dir_x) is the normal the across offset is
// applied along; which side that is in world terms follows from the heading
// basis in bsp/ship_ai_throttle_ring.hpp and is not asserted here.
ShipAiFormationStation ship_ai_formation_station_from_wake(
    const ShipAiWakePoint& wake, float across, float along) noexcept {
    ShipAiFormationStation station{};
    station.x = wake.x - across * wake.dir_z;
    station.z = wake.z + across * wake.dir_x;
    station.dir_x = wake.dir_x;
    station.dir_z = wake.dir_z;
    station.yaw_rate = wake.yaw_rate;
    station.yaw_rate_valid = wake.yaw_rate_written;
    station.across = across;
    station.along = along;
    return station;
}

// 0070D362-0070D3F8. The three offsets are stored as literal zero
// (0070D3E1, 0070D3E9, 0070D3F1), so a leader's station carries no offset and
// its yaw-rate slot is defined, unlike the member path's.
ShipAiFormationStation ship_ai_formation_station_for_leader(float x, float z,
                                                            float heading) noexcept {
    const std::array<float, 2> dir = heading_to_direction_006bc0c0(heading);
    ShipAiFormationStation station{};
    station.x = x;
    station.z = z;
    station.dir_x = dir[0];
    station.dir_z = dir[1];
    station.yaw_rate = 0.0f;
    station.yaw_rate_valid = true;
    station.across = 0.0f;
    station.along = 0.0f;
    return station;
}

ShipAiFormationStation ship_ai_formation_station_point(
    ShipAiFormationStationHost& host, std::uint32_t unit, std::uint32_t leader,
    const ShipAiUnitGroupMember* record, std::int32_t column,
    float across_scale, float along_scale) {
    // 0070D2A6 / 0070D2AF: the member path runs only for a non-leader with a
    // record; both failures fall into the leader branch at 0070D362. The image
    // never bounds the column index at group+500h (0070D2B7 uses it raw), so
    // the bound below is this projection's, not the image's.
    const bool member_path = (record != nullptr) && (unit != leader) &&
                             (column >= 0) &&
                             (static_cast<std::size_t>(column) < kShipAiUnitGroupColumnCount);
    if (!member_path) {
        host.refresh_world_pose_00414db0(unit);
        float x = 0.0f;
        float z = 0.0f;
        host.world_position_xz_0fc(unit, x, z);
        return ship_ai_formation_station_for_leader(
            x, z, host.unit_heading_vtable_50(unit));
    }

    const std::size_t col = static_cast<std::size_t>(column);
    // 0070D2BD and 0070D2CC: the two columns, each scaled by its own argument.
    const float across = record->lateral[col] * across_scale;
    const float along = record->axial[col] * along_scale;
    const ShipAiWakePoint wake = host.wake_point_00811150(leader, along);
    return ship_ai_formation_station_from_wake(wake, across, along);
}

// ---------------------------------------------------------------------------
// The speed match
// ---------------------------------------------------------------------------

// 009DACD0 whole.
float ship_ai_formation_speed_ratio(float yaw_rate, float speed,
                                    float across) noexcept {
    float result = kShipAiFormationSpeedRatioUnity;
    if (kShipAiFormationTurnRateEpsilon < static_cast<double>(std::fabs(yaw_rate))) {
        if (kShipAiFormationSpeedGate < std::fabs(speed)) {
            const float radius =
                std::fabs(((kFullTurnFloat / yaw_rate) * speed) / kFullTurnFloat);
            if (yaw_rate > 0.0f) {
                return (radius - across) / radius;
            }
            result = (radius + across) / radius;
        }
    }
    return result;
}

// 009DF61B-009DF64A: clamped_interpolate(0, ratio_now, 400, ratio_at_wake,
// along). The clamp is on the two ordinates, so the result never leaves the
// span of the two ratios however far astern the station is.
float ship_ai_formation_speed_blend(float ratio_now, float ratio_at_wake,
                                    float along) noexcept {
    return clamped_interpolate_00419010(0.0f, ratio_now,
                                        kShipAiFormationSpeedBlendDistance,
                                        ratio_at_wake, along);
}

// 009DF664-009DF69E.
float ship_ai_formation_published_speed(float reference_speed,
                                        float blend) noexcept {
    const float divisor =
        (blend > kShipAiFormationSpeedBlendFloor) ? blend : kShipAiFormationSpeedBlendFloor;
    return static_cast<float>(static_cast<double>(reference_speed) *
                              kShipAiFormationPublishSpeedScale /
                              static_cast<double>(divisor));
}

float ship_ai_formation_member_speed(const ShipAiUnitGroupMember& member) noexcept {
    return float_of(member.field_30);
}

void ship_ai_formation_set_member_speed(ShipAiUnitGroupMember& member,
                                        float speed) noexcept {
    member.field_30 = bits_of(speed);
}

// 0070D140 whole.
float ship_ai_formation_speed_ceiling(ShipAiUnitGroupMember* members,
                                      std::int32_t count) noexcept {
    float minimum = kShipAiFormationSpeedSeed;
    if (members == nullptr) {
        return minimum;
    }
    for (std::int32_t i = 0; i < count; ++i) {
        ShipAiUnitGroupMember& member = members[i];
        if (member.entity == 0) {
            // 0070D18D: a dead slot is reset, not skipped silently.
            ship_ai_formation_set_member_speed(member, kShipAiFormationMemberSpeedUnset);
            continue;
        }
        const float published = ship_ai_formation_member_speed(member);
        if (published < minimum) {
            minimum = published;
        }
    }
    return minimum;
}

// ---------------------------------------------------------------------------
// 0070ED30, the slot producer
// ---------------------------------------------------------------------------

bool ship_ai_formation_produce_member_slots(ShipAiFormationSlotHost& host,
                                            std::uint32_t leader,
                                            std::uint32_t member_entity,
                                            std::size_t index,
                                            ShipAiFormationMemberSlots& out) {
    if (member_entity == 0) {
        return false;  // 0070ED4A
    }

    // 0070ED53-0070ED81: the leader's world pose and the inverse of its world
    // matrix, both behind the byte latch at leader+10Ch.
    host.refresh_world_pose_00414db0(leader);
    host.build_world_inverse_00b63d50(leader);
    // 0070ED85: the member's own pose, behind member+0C8h.
    host.refresh_world_pose_00414db0(member_entity);

    // 0070EDA5-0070EDCE: the member's world position through leader+110h.
    float world_in[3] = {0.0f, 0.0f, 0.0f};
    host.world_position_0fc(member_entity, world_in);
    host.transform_by_leader_inverse_004142e0(leader, world_in, out.relative);

    // 0070EDD3-0070EE3A: clamp the relative position to FollowerMaxDist.
    const float max_dist = host.game_settings_follower_max_dist_00424c40();
    const double length_sq = static_cast<double>(out.relative[0]) * out.relative[0] +
                             static_cast<double>(out.relative[1]) * out.relative[1] +
                             static_cast<double>(out.relative[2]) * out.relative[2];
    if (static_cast<double>(max_dist) * max_dist < length_sq) {
        host.normalize_in_place_0042b260(out.relative);
        out.relative[0] = max_dist * out.relative[0];
        out.relative[1] = max_dist * out.relative[1];
        out.relative[2] = max_dist * out.relative[2];
    }

    // 0070EE3D-0070EEBC: the clamped point back into world through leader+0CCh,
    // then decomposed against the leader's wake into column 0.
    float world[3] = {0.0f, 0.0f, 0.0f};
    host.transform_by_leader_world_004142e0(leader, out.relative, world);
    float across = 0.0f;
    float along = 0.0f;
    host.wake_decompose_00811180(leader, world, across, along);
    out.columns.across[0] = across;
    out.columns.along[0] = along;

    // 0070EEC1-0070EF1B: columns 1..3 from the three tables at the join index.
    ship_ai_formation_fill_table_columns(
        out.columns, index, host.game_settings_formation_ship_dist_00424c40());
    return true;
}

// ---------------------------------------------------------------------------
// 0070EFD0, the reshape
// ---------------------------------------------------------------------------

std::size_t ship_ai_formation_reshape(ShipAiFormationReshapeHost& host,
                                      ShipAiUnitGroupMember* members,
                                      std::int32_t count, std::uint32_t leader,
                                      ShipAiFormationShape shape) {
    if (members == nullptr) {
        return 0;
    }
    // 0070EFD0-0070EFF9: the leader's own index, or -1 when it is not a member.
    std::int32_t leader_index = -1;
    for (std::int32_t i = 0; i < count; ++i) {
        if (members[i].entity == leader) {
            leader_index = i;
            break;
        }
    }

    const float ship_dist = host.game_settings_formation_ship_dist_00424c40();
    std::size_t slot = 1;  // 0070F00B: the counter starts at 1, not 0
    std::size_t written = 0;
    for (std::int32_t i = 0; i < count; ++i) {
        ShipAiUnitGroupMember& member = members[i];
        if (member.entity == 0) {
            continue;  // 0070F01B tests the record's entity, not the leader
        }
        if (i == leader_index) {
            member.lateral[0] = 0.0f;
            member.axial[0] = 0.0f;
        } else {
            const ShipAiFormationOffset offset =
                ship_ai_formation_reshape_slot(shape, slot, ship_dist);
            ++slot;
            member.lateral[0] = offset.across;
            member.axial[0] = offset.along;
        }
        ++written;
    }
    return written;
}

// ---------------------------------------------------------------------------
// 0070EF30, the join
// ---------------------------------------------------------------------------

std::int32_t ship_ai_formation_add_member(ShipAiFormationJoinHost& host,
                                          ShipAiUnitGroupMember* members,
                                          std::int32_t& count,
                                          std::uint32_t group,
                                          std::uint32_t entity) {
    // 0070EF30 itself, before any test: the entity's group pointer is set even
    // when the entity turns out to be a member already.
    host.set_entity_group_284(entity, group);

    for (std::int32_t i = 0; i < count; ++i) {
        if (members[i].entity == entity) {
            // 0070EF52: an existing member only gets the observer pair, and
            // only when it does not already have one.
            if (!host.observer_pair_registered(group, entity)) {
                host.observer_register_pair(group, entity);
            }
            return -1;
        }
    }

    const std::int32_t index = count;
    members[index].entity = entity;
    ship_ai_formation_set_member_speed(members[index],
                                       kShipAiFormationMemberSpeedUnset);
    // 0070EF85: the slots are produced with the **join index**, and before the
    // count is bumped, so the new record is not yet visible to a walker.
    host.produce_member_slots_0070ed30(static_cast<std::size_t>(index));
    count = index + 1;
    host.observer_register_pair(group, entity);
    host.refresh_speed_ceiling_0070da00();
    return index;
}

// ---------------------------------------------------------------------------
// 009DF2D0 whole
// ---------------------------------------------------------------------------

bool ship_ai_follow_update_formation_point(ShipAiFollowFormationPointHost& host,
                                           std::uint32_t unit,
                                           std::uint32_t group,
                                           ShipAiFollowState& state) {
    // 009DF2E0 / 009DF2EC: no group, no work at all.
    if (group == 0) {
        return false;
    }

    // 009DF307: the slot, with both scales at 1.0f.
    const ShipAiFormationStation slot =
        host.station_point_0070d290(unit, 1.0f, 1.0f);
    state.slot_point_0c.x = slot.x;       // 009DF315
    state.slot_point_0c.z = slot.z;       // 009DF320
    state.radius_30 = static_cast<float>(static_cast<double>(host.unit_hull_radius_9c8()) *
                                         kShipAiFollowSlotRadiusScale);  // 009DF33A
    state.slot_direction_1c.x = slot.dir_x;  // 009DF349
    state.slot_direction_1c.z = slot.dir_z;  // 009DF354
    state.leader_speed_28 = host.leader_body_axis_speed_0092d730();  // 009DF359

    // 009DF361-009DF3D8: the station point is the slot offset along the slot
    // direction by the stored radius, **plus** while the latch is set and
    // minus while it is clear.
    const float offset_x = state.radius_30 * state.slot_direction_1c.x;
    const float offset_z = state.radius_30 * state.slot_direction_1c.z;
    if (state.making_way_2c) {
        state.station_point_14.x = state.slot_point_0c.x + offset_x;
        state.station_point_14.z = state.slot_point_0c.z + offset_z;
    } else {
        state.station_point_14.x = state.slot_point_0c.x - offset_x;
        state.station_point_14.z = state.slot_point_0c.z - offset_z;
    }

    // 009DF3DB-009DF400: the ratio at the wake point the ship is sitting on.
    // 0070D290 leaves the wake yaw rate unwritten when the along offset is not
    // positive (00810645-00810718 never stores its fifth out-parameter), so the
    // image reads a stale stack slot there; this projection carries the flag
    // instead and uses zero, which makes 009DACD0 answer 1.
    const float leader_speed_again = host.leader_body_axis_speed_0092d730();
    const float wake_yaw_rate = slot.yaw_rate_valid ? slot.yaw_rate : 0.0f;
    const float ratio_at_wake =
        ship_ai_formation_speed_ratio(wake_yaw_rate, leader_speed_again, slot.across);

    // 009DF41A-009DF43E: push the station point out of the class's zone set.
    const std::uint32_t zone_set = host.zone_set_vtable_218(unit);
    state.station_point_14 = host.push_out_of_zones_00417b10(
        zone_set, state.station_point_14, kShipAiFormationZoneMargin);

    // 009DF441-009DF4C5: a second probe 1.5 turn radii along the slot
    // direction, on the same side as the station offset, pushed out too.
    const float turn = static_cast<float>(
        static_cast<double>(host.ship_class_turn_radius_0082e850()) *
        kShipAiFollowBackOffScale);
    const float probe_dx = turn * state.slot_direction_1c.x;
    const float probe_dz = turn * state.slot_direction_1c.z;
    ShipAiFollowLandXZ probe{};
    if (state.making_way_2c) {
        probe.x = state.station_point_14.x + probe_dx;
        probe.z = state.station_point_14.z + probe_dz;
    } else {
        probe.x = state.station_point_14.x - probe_dx;
        probe.z = state.station_point_14.z - probe_dz;
    }
    const ShipAiFollowLandXZ probe_in = probe;
    const ShipAiFollowLandXZ probe_out = host.push_out_of_zones_00417b10(
        zone_set, probe, kShipAiFormationZoneMargin);

    // 009DF4D0-009DF59B: only a probe the zone set actually moved re-derives
    // the direction, and the subtraction runs the way that keeps it forward.
    const float moved_x = probe_out.x - probe_in.x;
    const float moved_z = probe_out.z - probe_in.z;
    if (moved_x * moved_x + moved_z * moved_z >
        kShipAiFormationDirectionRederiveDistanceSq) {
        float dx = 0.0f;
        float dz = 0.0f;
        if (state.making_way_2c) {
            dx = probe_out.x - state.station_point_14.x;
            dz = probe_out.z - state.station_point_14.z;
        } else {
            dx = state.station_point_14.x - probe_out.x;
            dz = state.station_point_14.z - probe_out.z;
        }
        const double length = std::sqrt(static_cast<double>(dx) * dx +
                                        static_cast<double>(dz) * dz);
        if (length > 0.0) {
            const float inverse = static_cast<float>(1.0 / length);
            state.slot_direction_1c.x = inverse * dx;
            state.slot_direction_1c.z = inverse * dz;
        }
    }

    // 009DF5A4-009DF5DF: the slot heading of whatever direction survived.
    float heading = static_cast<float>(
        kHeadingBasisQuarterTurn -
        static_cast<double>(std::atan2(state.slot_direction_1c.z,
                                       state.slot_direction_1c.x)));
    if (heading < 0.0f) {
        heading = static_cast<float>(static_cast<double>(heading) + kHeadingBasisFullTurn);
    }
    state.slot_heading_24 = heading;

    // 009DF5E4-009DF65B: the ratio at the leader's **current** yaw rate, the
    // blend between the two ratios over the along offset, and the leader speed
    // this step publishes to the state scaled by it.
    const float leader_speed_now = host.leader_body_axis_speed_0092d730();
    const float yaw_rate_now =
        host.leader_command_yaw_rate_00811940(leader_speed_now, slot.across);
    const float ratio_now =
        ship_ai_formation_speed_ratio(yaw_rate_now, leader_speed_now, slot.across);
    const float blend =
        ship_ai_formation_speed_blend(ratio_now, ratio_at_wake, slot.along);
    state.leader_speed_28 = blend * state.leader_speed_28;

    // 009DF664-009DF6AA: the ship's own reference speed, divided by the blend
    // with a 0.25 floor, published into its own member record at +30h.
    const float published = ship_ai_formation_published_speed(
        host.unit_reference_speed_0080fc30(), blend);
    host.publish_member_speed_0070d100(unit, published);
    return true;
}

}  // namespace bsp
