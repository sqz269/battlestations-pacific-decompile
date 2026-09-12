// docs/UNIT_PARTS.md, reports/unit_parts.json. Packet cc2_unit_parts.
//
// 00934150 is reconstructed as a sequence over an injected host; the physics
// library, the game dynamics list, the effect system and the scene graph are
// contracts, one host method per native call site. The arithmetic of steps 8
// and 11 is transcribed from the listing, so it is reconstructed but not
// fixture-tested: both native callers pass a zero impulse, which cancels the
// first half of every velocity.
#include "bsp/unit_parts.hpp"

#include <cmath>

namespace bsp {
namespace {

void vec_add(float out[3], const float a[3]) noexcept {
    out[0] += a[0];
    out[1] += a[1];
    out[2] += a[2];
}

// 0042B260 followed by 00BD2F10 at 009349DD/009349FD: normalise with a floor on
// the length, then scale. The floor value itself is inside 0042B260 and is not
// reconstructed here, so a zero-length vector stays zero.
void normalize_in_place(float v[3]) noexcept {
    const float length_squared = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    if (length_squared <= 0.0f) {
        v[0] = 0.0f;
        v[1] = 0.0f;
        v[2] = 0.0f;
        return;
    }
    const float inverse = 1.0f / std::sqrt(length_squared);
    v[0] *= inverse;
    v[1] *= inverse;
    v[2] *= inverse;
}

} // namespace

bool unit_part_is_live(float part_health) noexcept {
    return part_health > kUnitPartLiveThreshold;
}

int unit_part_group_slot(std::int8_t slot_byte) noexcept {
    return static_cast<int>(slot_byte);
}

bool unit_part_detach_is_visual_only(int group_slot) noexcept {
    return group_slot < 0;
}

std::size_t unit_part_group_record_offset(int group_slot) noexcept {
    if (group_slot < 0) {
        return 0;
    }
    return static_cast<std::size_t>(group_slot) * kUnitPartsGroupRecordStride;
}

int unit_last_damage_state_index(bool has_instance, std::size_t state_bytes) noexcept {
    if (!has_instance) {
        return 0;
    }
    const std::size_t count = state_bytes / kPartInstanceStateStride;
    if (count == 0) {
        return 0;
    }
    return static_cast<int>(count) - 1;
}

bool unit_subobject_kind_selects_timer(const SubObjectKindTests& tests, int class_id) noexcept {
    // 00958B07..00958B20 and 00958B40..00958B68. vtable[204h] is only reached
    // through the composite test, so a non-composite sub-object never answers
    // it; that short circuit is the whole rule.
    if (class_id == kUnitKindSubObjectBreakA) {
        return tests.is_kind_46 || (tests.is_composite && tests.secondary_46);
    }
    if (class_id == kUnitKindSubObjectBreakB) {
        return tests.is_kind_45 || (tests.is_composite && tests.secondary_45);
    }
    return false;
}

float unit_subobject_break_timer(const SubObjectKindTests& tests,
                                 const SubObjectClassFields& fields) noexcept {
    // 00958AE8 seeds +728h from the owner class +190h; either class test
    // replaces it with +18Ch.
    float timer = fields.break_timer_default;
    if (unit_subobject_kind_selects_timer(tests, kUnitKindSubObjectBreakA)) {
        timer = fields.break_timer_kinded;
    }
    if (unit_subobject_kind_selects_timer(tests, kUnitKindSubObjectBreakB)) {
        timer = fields.break_timer_kinded;
    }
    return timer;
}

bool unit_subobject_damages_owner(const SubObjectKindTests& tests,
                                  const SubObjectClassFields& fields) noexcept {
    return tests.is_composite && fields.damages_owner;
}

SubObjectHealthAction unit_subobject_health_action(const SubObjectHealthState& state) noexcept {
    if (!state.has_owner) {
        // 00958DA7: the root branch, the one docs/UNIT_DAMAGE_AND_DEATH.md covers.
        return state.health <= 0.0f ? SubObjectHealthAction::kDestroyRoot
                                    : SubObjectHealthAction::kNone;
    }
    if (state.health <= 0.0f && !state.broken) {
        // 00958A63 and 00958A6D both fall through to 00958A7A.
        return SubObjectHealthAction::kFirstBreak;
    }
    if (state.health <= 0.0f) {
        // 00958BD5: already broken. The repeat only runs while the guard field
        // is clear and the timer has run out.
        if (!state.guard_set && state.break_timer <= 0.0f) {
            return SubObjectHealthAction::kRepeatBreak;
        }
        return SubObjectHealthAction::kNone;
    }
    if (state.broken && state.max_health <= state.health) {
        // 00958D12: FCOMIP on +370h against +36Ch.
        return SubObjectHealthAction::kRepairComplete;
    }
    return SubObjectHealthAction::kNone;
}

bool unit_message_is_detach_part(std::uint8_t kind) noexcept {
    return kind == kUnitMessageKindDetachPart;
}

PartDetachRequest unit_detach_request_from_message(std::uint8_t kind, int message_part_index,
                                                   std::int8_t group_slot_byte) noexcept {
    PartDetachRequest request;
    if (!unit_message_is_detach_part(kind)) {
        return request;
    }
    request.index = message_part_index;
    request.group_slot = unit_part_group_slot(group_slot_byte);
    // 0080E440 zeroes the stack vec3 before the call, so the impulse stays zero.
    return request;
}

PartDetachResult unit_parts_detach_part_00934150(UnitPartDetachHost& host,
                                                 const PartDetachRequest& request) {
    PartDetachResult result;

    // Step 2, 009341C1.
    const int node_count = host.part_node_count(request.index);
    for (int i = 0; i < node_count; ++i) {
        const std::uint32_t node = host.part_node(request.index, i);
        if (node != 0) {
            host.release_part_node(node);
            result.released_nodes = true;
        }
    }

    // Steps 3 and 4, 00934248 and 00934267.
    const int effect_count = host.part_effect_slot_count(request.index);
    for (int i = 0; i < effect_count; ++i) {
        const std::uint32_t effect = host.part_effect(request.index, i);
        if (effect != 0) {
            host.stop_effect_children(effect);
            host.mark_effect_orphaned(effect);
            host.release_effect_ref(effect);
        }
    }

    // Step 5, the two erase-to-begin memmove_s calls.
    host.clear_part_vectors(request.index);

    // Step 6, 00934366. A part with no model group stops here.
    if (unit_part_detach_is_visual_only(request.group_slot)) {
        return result;
    }

    // Step 7, the group vector at +340h.
    const int node_group_count = host.group_entry_count(kUnitPartsOffNodeGroups, request.group_slot);
    for (int i = 0; i < node_group_count; ++i) {
        const PartDetachGroupEntry entry =
            host.group_entry(kUnitPartsOffNodeGroups, request.group_slot, i);
        host.set_node_visibility(entry.node, 0.0f);
        float position[3] = {0.0f, 0.0f, 0.0f};
        host.node_world_position(entry.node, position);
        host.node_set_position(entry.node, position);
    }

    // Step 8, the centroid and the lowest bounds centre of the +320h group.
    const int debris_count = host.group_entry_count(kUnitPartsOffDebrisGroups, request.group_slot);
    if (debris_count > 0) {
        float sum[3] = {0.0f, 0.0f, 0.0f};
        float lowest_y = 3.402823466e+38f; // DAT_00D7A248, the seed of the minimum
        for (int i = 0; i < debris_count; ++i) {
            const PartDetachGroupEntry entry =
                host.group_entry(kUnitPartsOffDebrisGroups, request.group_slot, i);
            float centre[3] = {0.0f, 0.0f, 0.0f};
            host.node_bounds_centre(entry.node, centre);
            vec_add(sum, centre);
            if (centre[1] < lowest_y) {
                lowest_y = centre[1];
            }
        }
        const float inverse_count = 1.0f / static_cast<float>(debris_count);
        result.centroid[0] = sum[0] * inverse_count;
        result.centroid[1] = sum[1] * inverse_count;
        result.centroid[2] = sum[2] * inverse_count;
        result.lowest_y = lowest_y;

        // Step 9. The class list wins when it has an entry; the effect is
        // spawned at the centroid with the lowest bounds y substituted for the
        // average y, lowered by 00D7A308.
        const std::uint32_t handle = host.debris_effect_handle_from_class() != 0
                                         ? host.debris_effect_handle_from_class()
                                         : host.debris_effect_handle_from_settings();
        float spawn_at[3] = {result.centroid[0], lowest_y, result.centroid[2]};
        const std::uint32_t effect = host.spawn_debris_effect(handle, spawn_at);
        if (effect != 0) {
            host.mark_effect_orphaned(effect);
        }
        result.spawned_debris = true;

        // Step 10.
        host.physics_begin_debris();

        // Steps 11 to 14, once per group entry.
        for (int i = 0; i < debris_count; ++i) {
            const PartDetachGroupEntry entry =
                host.group_entry(kUnitPartsOffDebrisGroups, request.group_slot, i);

            float velocity[3] = {request.impulse[0], 0.0f, request.impulse[2]};
            normalize_in_place(velocity);

            float centre[3] = {0.0f, 0.0f, 0.0f};
            host.node_bounds_centre(entry.node, centre);
            float radial[3] = {centre[0] - result.centroid[0], centre[1] - spawn_at[1],
                               centre[2] - result.centroid[2]};
            normalize_in_place(radial);
            velocity[0] += radial[0];
            velocity[1] += radial[1];
            velocity[2] += radial[2];

            // 00934D4x: the spin axis is the cross product of the velocity with
            // the constant axis at 00CECA0C, whose two zero components collapse
            // most of the expression in the listing.
            float spin[3] = {-velocity[2], 0.0f, -velocity[0]};
            normalize_in_place(spin);

            host.unit_detail_for_lod();
            host.register_debris_node(entry.node);
            host.refresh_world_matrix(entry.node);
            host.add_game_dynamics_body(entry, velocity, spin, request.index, i);
            ++result.debris_bodies;
        }
    }

    // Step 15, the group vector at +330h.
    const int second_count = host.group_entry_count(kUnitPartsOffSecondGroups, request.group_slot);
    for (int i = 0; i < second_count; ++i) {
        const PartDetachGroupEntry entry =
            host.group_entry(kUnitPartsOffSecondGroups, request.group_slot, i);
        host.refresh_world_matrix(entry.node);
        host.apply_node_transform(entry.node);
        host.set_node_visibility(entry.node, 0.0f);
        float position[3] = {0.0f, 0.0f, 0.0f};
        host.node_world_position(entry.node, position);
        host.node_set_position(entry.node, position);
    }

    // Step 16.
    host.flush_scene_batch();

    // Step 17, the hull node tail.
    const int hull_count = host.hull_node_count();
    for (int i = 0; i < hull_count; ++i) {
        const std::uint32_t node = host.hull_node(i);
        if (host.hull_node_weight(node) <= 0.0f) {
            continue;
        }
        host.refresh_world_matrix(node);
        host.apply_node_transform(node);
        if (!host.hull_node_is_visible()) {
            continue;
        }
        host.set_node_visibility(node, 0.0f);
        float position[3] = {0.0f, 0.0f, 0.0f};
        host.node_world_position(node, position);
        host.node_set_position(node, position);
    }

    // Step 18.
    host.set_unit_parts_detached_flag();
    return result;
}

} // namespace bsp
