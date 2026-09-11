#include "bsp/unit_instance.hpp"

// docs/UNIT_INSTANCE_UPDATE.md. Every branch below is taken from the listing of
// 008255B0 (008255B0..00825DD4), 00904BF0, 006FE530, 006FF270, 00956600 and
// 004C0890; the x87 sequences were read from the disassembly, not the pseudocode.

namespace bsp {

bool unit_is_kind_of_006fe530(int class_id, int own_class_id) noexcept {
    // 006FE530: a chain of CMP/JE against seven literals, then a compare against
    // the instance field at +0C4h. Id 3 is not in the chain.
    for (int ancestor : kUnitDestroyerAncestry) {
        if (class_id == ancestor) {
            return true;
        }
    }
    return class_id == own_class_id;
}

float unit_intensity_scale_006ff270(bool global_override, bool local_override,
                                    float local_scale) noexcept {
    // 006FF271: CMP byte [00F87152],0 / JNE, then CMP byte [ECX+2F0h],0 / JE.
    if (global_override || local_override) {
        return kUnitIntensityFull;
    }
    return local_scale;
}

float unit_intensity_product_008259eb(bool global_override, bool local_override,
                                      float local_scale, bool is_controlled_unit) noexcept {
    // 008259EB: the player factor is chosen first, then the local scalar, and the
    // product is stored at +10A4h by the FMUL at 00825A39.
    const float player_factor =
        is_controlled_unit ? kUnitIntensityControlled : kUnitIntensityFull;
    const float local =
        unit_intensity_scale_006ff270(global_override, local_override, local_scale);
    return local * player_factor;
}

bool unit_effect_submerged_00825621(float local_offset_y, float node_world_y) noexcept {
    // 00825621: FLD offset.y / FADD node world Y / FCOMIP against -0.5 (double),
    // JBE skips the release, so the release happens strictly below the depth.
    const double height = static_cast<double>(local_offset_y) + static_cast<double>(node_world_y);
    return height < kUnitSubmergedCullDepth;
}

float unit_advance_age_00956626(float age, float scaled_delta) noexcept {
    return age + scaled_delta; // 00956626 FADD [ESI+524h], 00956638 FSTP back
}

float unit_advance_clamped_countdown_0095662c(float remaining, float scaled_delta) noexcept {
    // 00956634 COMISS remaining, 0 / JBE leaves the field untouched when it is
    // already at or below zero; otherwise the difference is clamped at zero.
    if (!(remaining > 0.0f)) {
        return remaining;
    }
    const float stepped = remaining - scaled_delta;
    return (stepped > 0.0f) ? stepped : 0.0f;
}

float unit_advance_countdown_0095666f(float remaining, float scaled_delta) noexcept {
    return remaining - scaled_delta; // 0095667A FSUB, 00956684 FST back
}

UnitBubbleTimerStep unit_step_bubble_timer_008256d3(float remaining, float scaled_delta) noexcept {
    // 008256D3 FLD [ESI+BC8h] / FSUB delta / FST back, then FLDZ + FCOMIP with
    // JC skipping the reload: the reload runs when 0 is not below the result.
    UnitBubbleTimerStep step{};
    step.remaining = remaining - scaled_delta;
    step.expired = !(step.remaining > 0.0f);
    return step;
}

void unit_rotate_hit_latch_00825824(UnitHitLatch& latch) noexcept {
    latch.previous = latch.current; // 00825838
    latch.current = false;          // 0082583E
}

bool unit_anchors_snap_to_surface_00825946(float bow_y) noexcept {
    // 0082594C COMISS -5.0, bow.y / JBE skips the snap, so the snap runs when
    // the threshold is strictly greater than the anchor height.
    return kUnitWaterSurfaceThreshold > bow_y;
}

float unit_anchor_surface_height_00825977(float sampled_ocean_height) noexcept {
    // 00825977 FADD qword [00D7A280] on the value 0078CF20 returned.
    return static_cast<float>(static_cast<double>(sampled_ocean_height) + kUnitAnchorSurfaceLift);
}

UnitWakeSpan unit_wake_span_00825a6b(PoseRefreshView& unit_pose,
                                   const float& descriptor_width) {
    // CMP precedes the arithmetic at each site; its JNZ follows the FSTP.
    // Preserve those checks, reads and single-precision scratch stores rather
    // than refreshing in a loop or snapshotting the matrix scalars.
    const bool refresh_left_lateral = unit_pose.world_valid_c8 == 0; // 00825A64
    const float half =
        static_cast<float>(static_cast<double>(descriptor_width) * kUnitWakeHalfWidthScale);
    if (refresh_left_lateral) refresh_pose_00414db0(unit_pose); // 00825A85

    const bool refresh_left_base = unit_pose.world_valid_c8 == 0; // 00825A8A
    const float left_offset = static_cast<float>(
        static_cast<double>(unit_pose.world_cc[9]) * static_cast<double>(half));
    if (refresh_left_base) refresh_pose_00414db0(unit_pose); // 00825AA8

    const bool refresh_right_lateral = unit_pose.world_valid_c8 == 0; // 00825AAD
    UnitWakeSpan span;
    span.left = static_cast<float>(
        static_cast<double>(unit_pose.world_cc[13]) + static_cast<double>(left_offset));
    if (refresh_right_lateral) refresh_pose_00414db0(unit_pose); // 00825AC6

    const bool refresh_right_base = unit_pose.world_valid_c8 == 0; // 00825ACB
    const float right_offset = static_cast<float>(
        static_cast<double>(unit_pose.world_cc[9]) * static_cast<double>(half));
    if (refresh_right_base) refresh_pose_00414db0(unit_pose); // 00825AE1

    span.right = static_cast<float>(
        static_cast<double>(unit_pose.world_cc[13]) - static_cast<double>(right_offset));
    return span;
}

bool unit_wake_spawns_00825b19(const UnitWakeSpan& span) noexcept {
    // 00825B21 and 00825B2C COMISS -5.0 against each side, both with JBE to the
    // skip, so both sides must sit strictly below the threshold.
    return kUnitWaterSurfaceThreshold > span.left && kUnitWaterSurfaceThreshold > span.right;
}

float unit_gated_sub_update_scalar_0082583e(float controller_value) noexcept {
    // 00825850 FLDZ / FCOMIP with JBE keeping the preloaded 1.0, so a negative
    // controller value is the only case that zeroes the argument.
    return (controller_value < 0.0f) ? 0.0f : kUnitIntensityFull;
}

void update_unit_instance_008255b0(UnitInstanceState& unit, const UnitClassBlock& class_block,
                                   UnitInstanceHost& host, float scaled_delta) {
    // Step 1, 008255CD: release every effect slot whose anchor has gone under.
    for (std::size_t i = 0; i < unit.effect_slots.size(); ++i) {
        UnitEffectSlot& slot = unit.effect_slots[i];
        if (!slot.live) {
            continue;
        }
        const float world_y = host.refresh_scene_node_world_matrix();
        if (unit_effect_submerged_00825621(slot.local_offset_y, world_y)) {
            host.release_submerged_effect(i);
            slot.live = false;
        }
    }

    // Step 2, 0082568F: the underwater bubble timer.
    if (unit.simulate && host.ocean_exists() && host.ocean_world_y() < 0.0f) {
        const UnitBubbleTimerStep step =
            unit_step_bubble_timer_008256d3(unit.bubble_timer, scaled_delta);
        unit.bubble_timer = step.remaining;
        if (step.expired) {
            unit.bubble_timer = host.draw_bubble_interval();
            if (class_block.has_bubble_template) {
                host.spawn_bubble_effect();
            }
        }
    }

    // Step 3, 00825824.
    unit_rotate_hit_latch_00825824(unit.hit_latch);

    // Step 4, 0082583E.
    host.sub_update_00815aa0(unit_gated_sub_update_scalar_0082583e(host.controller_time_value()));

    // Step 5, 00825870: the two water anchors.
    if (unit.has_bow_anchor_sink && unit.has_stern_anchor_sink && unit.has_scene_node) {
        host.refresh_scene_node_world_matrix();
        UnitAnchorPoint bow = host.transform_bow_anchor();
        host.refresh_scene_node_world_matrix();
        UnitAnchorPoint stern = host.transform_stern_anchor();
        if (unit_anchors_snap_to_surface_00825946(bow.y)) {
            bow.y = unit_anchor_surface_height_00825977(host.sample_ocean_height(bow.x, bow.z));
            stern.y =
                unit_anchor_surface_height_00825977(host.sample_ocean_height(stern.x, stern.z));
        }
        host.publish_bow_anchor(bow);
        host.publish_stern_anchor(stern);
    }

    // Step 6, 008259CE.
    host.sub_update_0092be80(scaled_delta);
    host.sub_update_0081c050();

    // Step 7, 008259EB.
    unit.intensity_product = unit_intensity_product_008259eb(
        host.global_intensity_override(), unit.intensity_override, unit.intensity_scale,
        host.is_controlled_unit());
    host.smooth_intensity_008227e0(scaled_delta);

    // Step 8, 00825A59: the wake spawn, and the four lazy pose refreshes around it.
    if (unit.simulate) {
        const UnitWakeSpan span =
            unit_wake_span_00825a6b(unit.pose, class_block.wake_width);
        if (host.wake_enabled() && !unit.controller_one_shot && unit_wake_spawns_00825b19(span)) {
            unit.controller_one_shot = true;
            host.spawn_wake();
        }

        // Step 9, 00825BD4.
        if (unit.has_prop_wash) {
            if (unit.pose.world_valid_c8 == 0) { // 00825BDD
                refresh_pose_00414db0(unit.pose);
            }
            if (host.prop_wash_threshold_passed(unit.pose.world_cc[13])) { // 00825BF5
                host.stop_prop_wash();
            }
        }

        // Step 10, 00825C11: five attachment anchors.
        if (class_block.has_attachments) {
            for (std::size_t i = 0; i < kUnitAttachSlotCount; ++i) {
                const UnitAttachSlot& slot = unit.attach_slots[i];
                if (!slot.live) {
                    continue;
                }
                host.refresh_scene_node_world_matrix();
                host.update_attachment(i, slot.local_point);
            }
        }
    }

    // Step 11, 00825D4D: three timed sub-updates in listing order.
    host.sub_update_008252c0(scaled_delta);
    host.sub_update_00956600(scaled_delta);
    host.sub_update_00834e90(scaled_delta);

    // Step 12, 00825D83: every part gets the intensity scalar 006FF270 returns.
    for (std::size_t i = 0; i < unit.part_count; ++i) {
        const float intensity = unit_intensity_scale_006ff270(
            host.global_intensity_override(), unit.intensity_override, unit.intensity_scale);
        host.tick_part(i, intensity);
    }
}

std::size_t update_world_entities_00904bf0(const std::vector<bool>& active,
                                           WorldEntityUpdateHost& host, float scaled_delta) {
    // 00904BF4: head [world+4], link entity+38h, gate byte entity+5Ch.
    std::size_t updated = 0;
    for (std::size_t i = 0; i < active.size(); ++i) {
        if (!active[i]) {
            continue;
        }
        host.update_entity(i, scaled_delta);
        ++updated;
    }
    host.update_timed_attachments_00904600(scaled_delta); // 00904C2B
    return updated;
}

ControlledUnitBind resolve_controlled_unit_004c0890(bool unit_present,
                                                    ControlledUnitQuery& query) {
    // 004C0893 stores the pointer before any test, so a null unit still clears
    // the published anchor; that path is the caller's, not this function's.
    ControlledUnitBind bind{};
    if (!unit_present) {
        return bind;
    }
    if (query.unit_is_kind_of(kUnitTraitDirectlyControlled)) {
        bind.has_target = true; // 004C08A8, the unit drives itself
    } else if (query.unit_is_kind_of(kUnitTraitHasDrivenSubUnit)) {
        bind.has_target = query.has_driven_sub_unit(); // 004C08C3, [unit+3D0h]
        bind.target_is_sub_unit = bind.has_target;
    }
    if (!bind.has_target) {
        return bind;
    }
    // 004C08D2: the resolved target answers either trait to publish its anchor.
    if (bind.target_is_sub_unit) {
        bind.publishes_anchor = query.sub_unit_is_kind_of(kUnitTraitPublishesAnchor) ||
                                query.sub_unit_is_kind_of(kUnitTraitHasDrivenSubUnit);
    } else {
        bind.publishes_anchor = query.unit_is_kind_of(kUnitTraitPublishesAnchor) ||
                                query.unit_is_kind_of(kUnitTraitHasDrivenSubUnit);
    }
    return bind;
}

} // namespace bsp
