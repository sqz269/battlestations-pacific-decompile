#include "bsp/recon_slot_lists.hpp"

#include "bsp/fixed_step_countdown.hpp"

#include <algorithm>

// Evidence: docs/RECON_SLOT_LISTS.md. The routine addresses in the comments are
// the native sites each statement comes from.

namespace bsp {

namespace {

// The number of per-class buckets in every one of the four arrays. Declared in
// bsp/fixed_step_countdown.hpp as kReconSlotArrayLength for the first three;
// the carry-over array uses the same 61h (00805302 PUSH 61h in the destructor).
constexpr int kClassBucketCount = static_cast<int>(kReconSlotArrayLength);

std::size_t triple_offset_for(ReconRelation relation) noexcept
{
    switch (relation) {
    case ReconRelation::own:
        return kReconTripleOwnOffset;
    case ReconRelation::enemy:
        return kReconTripleEnemyOffset;
    case ReconRelation::neutral:
    default:
        return kReconTripleNeutralOffset;
    }
}

// 00807581..00807610, repeated verbatim at 008076F9..00807788 and at
// 00807877..00807914: the same ten class ids in the same order for each of the
// three relation arrays.
void run_grouping_pass(ReconSlotListsHost& host, ReconRelation relation)
{
    for (const int member_class_id : kReconPlaneMemberClassIds) {
        host.group_members_00805490(relation, member_class_id, kReconSquadronClassId);
    }
    host.publish_group_level_from_members_008069a0(relation, kReconSquadronClassId);

    host.group_members_00805680(relation, kReconLandVehicleClassId, kReconConvoyClassId);
    host.publish_group_level_stored_00806a60(relation, kReconConvoyClassId);
}

} // namespace

// ---------------------------------------------------------------------------
// The detection value and its level
// ---------------------------------------------------------------------------

float recon_detection_accumulate_00805af0(float current, float delta, bool accumulate) noexcept
{
    // 00805B02: the second argument selects add or assign. Both reached call
    // sites pass 1, so the add arm is the live one.
    float value = accumulate ? current + delta : delta;

    // 00805B25..00805B4D. Written with the native compare order so that a NaN
    // value survives both branches, exactly as COMISS/JA leaves it.
    if (0.0f > value) {
        value = 0.0f;
    } else if (value > kReconDetectionValueMax) {
        value = kReconDetectionValueMax;
    }
    return value;
}

ReconDetectionLevel recon_detection_level_00805b3d(float clamped_value) noexcept
{
    // 00805B45 compares the threshold against the value, so the level falls to
    // `none` only when the value is strictly below it. A NaN takes neither
    // branch and lands on `identified`, which is what the native JBE/JA pair
    // does on unordered flags.
    if (clamped_value < kReconDetectionBlipThreshold) {
        return ReconDetectionLevel::none;
    }
    if (clamped_value < kReconDetectionIdentifyThreshold) {
        return ReconDetectionLevel::blip;
    }
    return ReconDetectionLevel::identified;
}

// ---------------------------------------------------------------------------
// The sensor arithmetic
// ---------------------------------------------------------------------------

float recon_sensor_range_scale_00804947(float environment_factor,
                                        float target_signature,
                                        float tuning_74) noexcept
{
    // 00804953..0080497B, in the native order: the environment factor is
    // squared first, the signature multiplies that product, and the tuning
    // value is squared separately before the final multiply.
    const float environment_squared = environment_factor * environment_factor;
    const float with_signature = environment_squared * target_signature;
    const float tuning_squared = tuning_74 * tuning_74;
    return with_signature * tuning_squared;
}

float recon_sensor_submerged_scale_008049a3(float range_scale, float tuning_78) noexcept
{
    // 008049A3..008049B5: the tuning value is squared, then multiplies the
    // scale. Only a target that is IsKindOf(8) and not a surface target
    // (00922DC0 false) reaches this.
    const float tuning_squared = tuning_78 * tuning_78;
    return tuning_squared * range_scale;
}

float recon_horizontal_distance_sq_00804a1e(float observer_x, float observer_z,
                                            float target_x, float target_z) noexcept
{
    // 00804A1E..00804A57. The height is never read: the world frames supply
    // only +FCh and +104h, so a plane overhead is at its ground distance.
    const float dx = target_x - observer_x;
    const float dz = target_z - observer_z;
    return (dx * dx) + (dz * dz);
}

ReconSensorEntryResult recon_sensor_entry_gain_00804ab0(const ReconSensorEntry& entry,
                                                        float normalized_distance_sq,
                                                        int mask,
                                                        float bearing_error,
                                                        float current_value,
                                                        float dt) noexcept
{
    ReconSensorEntryResult result{};

    // 00804AB7: out of range when the normalised squared distance is strictly
    // greater than the entry's limit.
    if (normalized_distance_sq > entry.max_normalized_distance_sq) {
        return result;
    }
    // 00804AC4: the caller's mask must carry this entry's bit.
    if ((mask & (1 << entry.mask_bit)) == 0) {
        return result;
    }
    // 00804AD7: only an entry whose +18h byte is set takes the bearing test,
    // and the comparison is on the absolute value (00804B1B ANDs off the sign).
    if (entry.bearing_limited) {
        const float magnitude = bearing_error < 0.0f ? -bearing_error : bearing_error;
        if (magnitude > entry.max_bearing_error) {
            return result;
        }
    }

    // 00804B32: the entry's per-second gain over the slot's elapsed time.
    float gain = entry.gain_per_second * dt;

    // 00804B45..00804B79: clamp so the accumulated value does not pass the cap.
    // The native form is the subtraction chain, not a min(), and it is written
    // that way here because the two differ in the last bit.
    const float projected = current_value + gain;
    if (projected > entry.cap) {
        gain = gain - (projected - entry.cap);
    }

    result.gain = gain;
    result.applies = true; // 00804B40 sets the returned byte here, after the tests
    return result;
}

ReconSensorEvaluation recon_evaluate_sensor_row_008048a0(const ReconSensorEntry* entries,
                                                         std::size_t count,
                                                         const float* bearing_errors,
                                                         float normalized_distance_sq,
                                                         int mask,
                                                         float current_value,
                                                         float dt) noexcept
{
    ReconSensorEvaluation evaluation{};
    if (entries == nullptr) {
        return evaluation;
    }
    for (std::size_t index = 0; index < count; ++index) {
        const float bearing_error = bearing_errors != nullptr ? bearing_errors[index] : 0.0f;
        const ReconSensorEntryResult entry_result =
            recon_sensor_entry_gain_00804ab0(entries[index], normalized_distance_sq, mask,
                                             bearing_error, current_value, dt);
        if (!entry_result.applies) {
            continue;
        }
        evaluation.detected = true;
        // 00804B85: strictly greater, so the first entry of an equal pair wins.
        if (entry_result.gain > evaluation.best_gain) {
            evaluation.best_gain = entry_result.gain;
        }
    }
    return evaluation;
}

// ---------------------------------------------------------------------------
// The publish filter
// ---------------------------------------------------------------------------

bool recon_publish_excludes_class_00805de6(int class_id) noexcept
{
    return std::find(kReconPublishExcludedClassIds.begin(),
                     kReconPublishExcludedClassIds.end(),
                     class_id) != kReconPublishExcludedClassIds.end();
}

// ---------------------------------------------------------------------------
// 008073C0 as a sequence
// ---------------------------------------------------------------------------

std::size_t rebuild_recon_slot_lists_008073c0(ReconSlotListsHost& host,
                                              ReconSlotState& slot) noexcept
{
    // 008073C1..008073E6: the elapsed time the sensor pass will use, then the
    // contents-changed byte, cleared before anything can set it again.
    const float now = host.global_time_00f876a4();
    slot.elapsed_seconds = now - slot.last_refresh_time;
    slot.last_refresh_time = now;
    slot.contents_changed = false;

    // 008073D0..00807416: all five triples, in order.
    for (const std::size_t triple : kReconTripleOffsets) {
        host.clear_triple_008042b0(triple);
    }

    // 0080741B..00807460: every class bucket's three relation lists are spliced
    // into that bucket's carry-over list, own first, then enemy, then neutral.
    // Nothing is freed here: the 1Ch records survive the pass and are matched
    // back by unit pointer.
    for (int class_index = 0; class_index < kClassBucketCount; ++class_index) {
        host.move_relation_list_into_carry_over_00804150(class_index, ReconRelation::own);
        host.move_relation_list_into_carry_over_00804150(class_index, ReconRelation::enemy);
        host.move_relation_list_into_carry_over_00804150(class_index, ReconRelation::neutral);
    }

    // 00807462..0080749A: every unit on the world list at [[game+19CCh]+13Ch]
    // has its detection record for this slot reset before the scan.
    host.reset_detection_for_world_list_00805be0();

    // 0080749C..00807527: the scan. One pass per scanned class id, in the
    // singleton's push order, and each class is retired before the next starts.
    std::size_t admitted = 0;
    const std::vector<int>& class_ids = host.scanned_class_ids_00806480();
    for (const int class_id : class_ids) {
        for (void* unit : host.world_units_of_class_008074ca(class_id)) {
            if (!recon_unit_gate_passes_008074d5(host.unit_gate_bytes_008074d5(unit))) {
                continue;
            }
            if (!host.unit_is_kind_of_008074f6(unit, kReconScanRequiredClassId)) {
                continue;
            }
            const ReconRelation relation =
                recon_relation_for_008065ff(slot.index, host.unit_party_00806605(unit));
            if (host.add_or_refresh_unit_008065b0(class_id, unit, relation)) {
                slot.contents_changed = true; // 00806818
            }
            if (relation == ReconRelation::own) {
                host.force_own_unit_detected_00805af0(unit); // 008067FA / 008066E1
            }
            ++admitted;
        }
        // 00807510: reached for every scanned class, even one with no units.
        host.retire_carry_over_00805430(class_id);
    }

    // 00807529..00807545: the own triple takes a copy of every own bucket, and
    // it takes it before the grouping pass, so it carries the individual planes
    // and land vehicles as well as the aggregates added later.
    for (int class_index = 0; class_index < kClassBucketCount; ++class_index) {
        host.append_class_list_to_triple_00804e10(kReconTripleOwnOffset, ReconRelation::own,
                                                  class_index);
    }

    // 00807547..0080757F: the sensor pass, enemy buckets first, then neutral.
    // The own triple is the observer list in both.
    for (int class_index = 0; class_index < kClassBucketCount; ++class_index) {
        host.run_sensor_pass_00806840(ReconRelation::enemy, class_index);
    }
    for (int class_index = 0; class_index < kClassBucketCount; ++class_index) {
        host.run_sensor_pass_00806840(ReconRelation::neutral, class_index);
    }

    // 00807581..008076F3: enemy grouping, then the enemy triple, then the drain.
    run_grouping_pass(host, ReconRelation::enemy);
    for (int class_index = 0; class_index < kClassBucketCount; ++class_index) {
        host.append_class_list_to_triple_00804e10(kReconTripleEnemyOffset, ReconRelation::enemy,
                                                  class_index);
    }
    host.drain_triple_to_unknown_00807634(kReconTripleEnemyOffset);

    // 008076F9..00807871: the same three steps for neutral.
    run_grouping_pass(host, ReconRelation::neutral);
    for (int class_index = 0; class_index < kClassBucketCount; ++class_index) {
        host.append_class_list_to_triple_00804e10(kReconTripleNeutralOffset,
                                                  ReconRelation::neutral, class_index);
    }
    host.drain_triple_to_unknown_00807634(kReconTripleNeutralOffset);

    // 00807877..0080791C: own grouping happens last and its two aggregate
    // buckets are appended to the own triple one at a time. The own triple is
    // never drained, so an own unit is published whatever its detection level.
    run_grouping_pass(host, ReconRelation::own);
    host.append_class_list_to_triple_00804e10(kReconTripleOwnOffset, ReconRelation::own,
                                              kReconSquadronClassId);
    host.append_class_list_to_triple_00804e10(kReconTripleOwnOffset, ReconRelation::own,
                                              kReconConvoyClassId);

    // 00807921..0080792E: the two aggregate classes are never scanned, so they
    // are retired here with two literal ids.
    for (const int aggregate_class_id : kReconAggregateClassIds) {
        host.retire_carry_over_00805430(aggregate_class_id);
    }

    // 00807933..00807966: the fifth triple is the union of the four published
    // ones, in their published order.
    host.append_triple_to_triple_00804e10(kReconTripleAllOffset, kReconTripleOwnOffset);
    host.append_triple_to_triple_00804e10(kReconTripleAllOffset, kReconTripleEnemyOffset);
    host.append_triple_to_triple_00804e10(kReconTripleAllOffset, kReconTripleNeutralOffset);
    host.append_triple_to_triple_00804e10(kReconTripleAllOffset, kReconTripleUnknownOffset);

    // 0080796B..0080799C: a slot that gained an entry invalidates the local
    // player's cached view, but only when it is that player's own slot.
    if (slot.contents_changed) {
        if (host.slot_is_active_local_player_00807986(slot.index)) {
            host.invalidate_local_view_00807995();
        }
        slot.contents_changed = false;
    }

    return admitted;
}

} // namespace bsp
