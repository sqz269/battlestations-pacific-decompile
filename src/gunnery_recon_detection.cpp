#include <bsp/gunnery_recon_detection.hpp>

#include <bsp/unit_rudder.hpp>

#include <cmath>

// Evidence: docs/GUNNERY_RECON_DETECTION.md. Addresses in the comments are the
// native instructions each line stands for.

namespace bsp {

namespace {

// 00804AE4..00804B0B: the bearing the angle-limited entries test. The native
// code calls the observer's vtable[50h] for the heading, _CIatan2 (00BF701A)
// for the target bearing from the same dx/dz the range test used, and
// 00438B10 to wrap the difference into (-pi, pi]. It is computed once per
// (observer, target) pair even though the native recomputes it per entry: the
// inputs do not depend on the entry.
float bearing_error(const GunneryReconObserver& observer,
                    const GunneryReconTarget& target) noexcept
{
    const float dx = target.world_x - observer.world_x;
    const float dz = target.world_z - observer.world_z;
    // 00804AF2 loads dx first and dz second, and _CIatan2 takes the first as y.
    const float bearing = std::atan2(dx, dz);
    return wrapped_angle_subtract_00438b10(bearing, observer.heading);
}

} // namespace

// ---------------------------------------------------------------------------
// 008048A0
// ---------------------------------------------------------------------------

float gunnery_recon_range_scale_008048a0(const GunneryReconObserver& observer,
                                         const GunneryReconTarget& target,
                                         const GunneryReconEnvironment& env) noexcept
{
    // 008048CC..008048FD: the target's ReconModifier square applies only to a
    // unit-base target; everything else contributes 00D7A24C, 1.0f.
    const float modifier_sq =
        target.is_unit_base ? target.recon_modifier_sq : kReconSensorDefaultFactor;

    float range_scale = recon_sensor_range_scale_00804947(observer.environment_factor,
                                                          modifier_sq,
                                                          env.simplified_recon_multiplier);

    // 0080497F..008049B5: both tests must hold. IsKindOf(08h) alone is not
    // enough; a surfaced submarine keeps the unpenalised scale.
    if (target.is_submarine && !target.is_surface_target) {
        range_scale = recon_sensor_submerged_scale_008049a3(range_scale,
                                                            env.simplified_sonar_multiplier);
    }
    return range_scale;
}

std::size_t gunnery_recon_row_index_00804a7b(SensorCategory observer,
                                             SensorCategory target) noexcept
{
    // 00804A7B/00804A7F/00804A82 build exactly the index the loader builds at
    // 008085AA, so the two sides address the same list.
    return sensor_list_index_008085aa(observer, target);
}

GunneryReconObservation gunnery_recon_observe_008048a0(const GunneryReconObserver& observer,
                                                       const GunneryReconTarget& target,
                                                       const GunneryReconEnvironment& env,
                                                       float current_value,
                                                       float dt) noexcept
{
    GunneryReconObservation observation{};
    observation.row_index = kSensorListCount;

    // 008048A8..008048C9: the observer must carry a sensor table, else the
    // routine returns false before touching anything else.
    if (observer.rows == nullptr) {
        return observation;
    }

    observation.range_scale = gunnery_recon_range_scale_008048a0(observer, target, env);

    // 00804A1E..00804A5F. Horizontal only, and the divide is not guarded: a
    // zero range scale reproduces the native infinity rather than hiding it.
    const float distance_sq = recon_horizontal_distance_sq_00804a1e(observer.world_x,
                                                                    observer.world_z,
                                                                    target.world_x,
                                                                    target.world_z);
    observation.normalized_distance_sq = distance_sq / observation.range_scale;

    const std::size_t row = gunnery_recon_row_index_00804a7b(observer.category, target.category);
    // The native indexes the 2A8h record without a bounds test, because no
    // category value reaches 7 (docs/SENSOR_TABLES.md §1). A caller that hands
    // this rule a shorter row array gets an empty row instead of a read past
    // the end; that is a reconstruction guard, not native behaviour.
    if (row >= observer.row_count) {
        return observation;
    }
    observation.row_index = row;

    const GunneryReconSensorRow& list = observer.rows[row];
    if (list.entries == nullptr || list.count == 0) {
        return observation; // 00804AA5: begin == end, nothing applied
    }

    const float error = bearing_error(observer, target);

    // 00804AB0..00804BAE. Every applying entry sets the returned byte; only a
    // strictly larger gain replaces the reported one (00804B85).
    for (std::size_t index = 0; index < list.count; ++index) {
        const ReconSensorEntryResult entry_result =
            recon_sensor_entry_gain_00804ab0(list.entries[index],
                                             observation.normalized_distance_sq,
                                             env.sensor_mask,
                                             error,
                                             current_value,
                                             dt);
        if (!entry_result.applies) {
            continue;
        }
        observation.applied = true;
        if (entry_result.gain > observation.gain) {
            observation.gain = entry_result.gain;
        }
    }
    return observation;
}

// ---------------------------------------------------------------------------
// 00806840, and what the drain publishes
// ---------------------------------------------------------------------------

GunneryReconContact gunnery_recon_detect_00806840(const GunneryReconObserver* observers,
                                                  std::size_t observer_count,
                                                  const GunneryReconTarget& target,
                                                  const GunneryReconEnvironment& env,
                                                  ReconRelation relation,
                                                  float prior_value,
                                                  float dt) noexcept
{
    GunneryReconContact contact{};
    contact.detection_value = prior_value;
    contact.level = recon_detection_level_00805b3d(prior_value);

    // 00806871 and 00806883: two short circuits that leave the detection record
    // untouched and fall straight through to the 0080695C publish.
    const bool skip = env.network_role == kGunneryReconNonOriginatingRole ||
                      target.detection_forced;
    contact.sensor_pass_skipped = skip;

    if (!skip) {
        float best_gain = 0.0f;             // 008068A3, zeroed before the loop
        std::size_t best_observer = kGunneryReconNoObserver;
        bool any_applied = false;           // 008068F4 OR BL,AL

        for (std::size_t index = 0; index < observer_count; ++index) {
            const GunneryReconObserver& observer = observers[index];

            // 008068C2 PUSH 5: only a unit-base own-triple entry observes.
            if (!observer.is_observer_class) {
                continue;
            }

            // 00804B2F reads the cap's current value from the record indexed by
            // the OBSERVER's party. The rebuild's observers all come from this
            // slot's own triple, so the two indices coincide there.
            const GunneryReconObservation observation =
                gunnery_recon_observe_008048a0(observer, target, env, contact.detection_value, dt);

            if (!observation.applied) {
                continue;
            }
            any_applied = true;
            if (observer.party != env.slot_index) {
                contact.observer_party_mismatch = true;
            }
            // 008068FA JBE: strictly greater wins, so the first observer of an
            // equal pair keeps the slot.
            if (observation.gain > best_gain) {
                best_gain = observation.gain;
                best_observer = index;
            }
        }

        contact.any_observer_applied = any_applied;
        contact.best_observer = best_observer;

        if (any_applied) {
            // 0080693D: 00805AF0(bestGain, 1, bestObserver).
            contact.detection_value =
                recon_detection_accumulate_00805af0(contact.detection_value, best_gain, true);
        } else {
            // 00806957: 00805BE0 snaps the value and the level to zero. There is
            // no decay ramp; a target nothing saw this pass drops out at once.
            contact.detection_value = 0.0f;
        }
        contact.level = recon_detection_level_00805b3d(contact.detection_value);
    }

    // 0080695C..00806981: the record's +0Ch takes the forced level when the
    // force byte is set, the accumulated level otherwise.
    contact.published_level = recon_effective_level_0080695c(target.detection_forced,
                                                             contact.level,
                                                             target.forced_level);

    // 00807644 (enemy, the flat list at slot+0DE4h) and 008077C2 (neutral,
    // slot+0DF0h). No filter loop exists over the own list anywhere in
    // 00807520..00807871, the whole region read, so an own-relation record is
    // always a member.
    if (relation == ReconRelation::own) {
        contact.drain = ReconTripleFilterResult{};
        contact.admitted_as_contact = true;
        return contact;
    }
    contact.drain = recon_triple_filter_00807647(contact.published_level);
    contact.admitted_as_contact = !contact.drain.remove_from_relation;
    return contact;
}

} // namespace bsp
