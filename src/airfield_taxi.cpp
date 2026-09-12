#include "bsp/airfield_taxi.hpp"

// Reconstruction of the airfield hangar list -> plane taxi state link.
// docs/AIRFIELD_TAXI.md carries the evidence and the coverage notes; every
// routine below names the native site it projects.

namespace bsp {
namespace {

// 00415550 BSP_Math_MaxFloatByRef: one FCOMIP with JBE returning the second
// operand, so the result is the larger and a NaN takes the second operand.
float max_by_ref_00415550(float a, float b) { return a < b ? b : a; }

}  // namespace

bool entity_is_usable_006d2730(const EntityLivenessBytes& bytes) {
    // 006D2748-006D275E, in the listing's order: +5Ch set, then +5Dh, +60h, +5Eh clear.
    return bytes.present && !bytes.out_of_action && !bytes.flag_60 && !bytes.flag_5e;
}

HangarPathPick pick_hangar_path_006d2780_006d2640(const AirfieldHangarCandidate* hangars,
                                                  std::size_t count,
                                                  HangarPathKind kind) {
    HangarPathPick pick;
    if (hangars == nullptr) {
        return pick;
    }

    const bool want_entry = kind == HangarPathKind::Entry;
    // 006D2783 seeds -9999.0f; 006D2643 seeds +9999.0f.
    float best = want_entry ? kTaxiEntryPathSentinel : kTaxiExitPathSentinel;

    for (std::size_t i = 0; i < count; ++i) {
        const AirfieldHangarCandidate& hangar = hangars[i];
        // 006D27B5 / 006D2675: a null object is skipped by both.
        if (!hangar.object_present || hangar.record.object == nullptr) {
            continue;
        }
        // 006D267B-006D268A: the exit search alone gates on object+370h > 0.
        if (!want_entry && !(hangar.condition > 0.0f)) {
            continue;
        }
        // 006D280B-006D2817 keeps the maximum; 006D26DD-006D26E9 the minimum.
        const bool better = want_entry ? hangar.local_z > best : best > hangar.local_z;
        if (!better) {
            continue;
        }
        best = hangar.local_z;
        pick.found = true;
        pick.index = i;
        // 006D281F reads record[1]; 006D26F1 reads record[2].
        pick.path = want_entry ? hangar.record.entry_path : hangar.record.exit_path;
    }
    return pick;
}

int taxi_target_point_index_006cf420(std::ptrdiff_t path_points_begin,
                                     std::ptrdiff_t path_points_end) {
    // 006CF46x: `if (path+8h == 0) count = 0; else count = (path+0Ch - path+8h) >> 2;`
    // then `index = count - 1`.
    if (path_points_begin == 0) {
        return -1;
    }
    const std::ptrdiff_t count = (path_points_end - path_points_begin) / 4;
    return static_cast<int>(count) - 1;
}

float required_yaw_rate_009cd752(float class_yaw_spd,
                                 float tuning_yaw_spd_mul,
                                 float tuning_rate_floor,
                                 float tuning_yaw_blend) {
    // 009CD763-009CD79C: rate = tuning+2B0h * classDesc+1B0h, floored against
    // tuning+188h, then 1.5 / that, then scaled by tuning+2A8h.
    float rate = tuning_yaw_spd_mul * class_yaw_spd;
    rate = max_by_ref_00415550(rate, tuning_rate_floor);
    if (rate == 0.0f) {
        return 0.0f;
    }
    return tuning_yaw_blend * (kTaxiRequiredRateNumerator / rate);
}

TaxiStateAction taxi_state_action_009cd7e1(int flight_state, bool needs_path) {
    // 009CD7E1 SETE CL on (unit+900h == 5); 009CD7EB CMP CL,BL; equal writes nothing.
    const bool on_path = flight_state == 5;
    if (on_path == needs_path) {
        return TaxiStateAction::None;
    }
    return needs_path ? TaxiStateAction::JoinPath : TaxiStateAction::LeavePath;
}

bool contact_site_changes_007b8e80(const void* current_holder, const void* new_holder) {
    // 007B8E8E CMP EAX,EDI with JZ to the epilogue.
    return current_holder != new_holder;
}

void set_ground_contact_site_007b8e80(AirfieldTaxiHost& host,
                                      const void* current_holder,
                                      const void* new_holder) {
    if (!contact_site_changes_007b8e80(current_holder, new_holder)) {
        return;
    }

    if (current_holder != nullptr) {
        // 007B8E96-007B8E9F: detach from the old owner.
        const void* old_block = host.block_of_holder(current_holder);
        host.unregister_observer_006952a0(host.owner_unit_of_block(old_block));
        // 007B8EA4-007B8EB4: re-read plane+BF4h, then mark the site released.
        // The native code re-reads the field rather than reusing the register;
        // nothing between the two reads writes it, so the effect is the same.
        if (old_block != nullptr) {
            host.mark_site_released_007b8eb4(host.launch_site_of_block(old_block));
        }
    }

    if (new_holder != nullptr) {
        // 007B8EBC-007B8EC5: attach to the new owner.
        const void* new_block = host.block_of_holder(new_holder);
        host.register_observer_00694a60(host.owner_unit_of_block(new_block));
    }

    host.store_contact_holder_007b8eca(new_holder);
}

TaxiStepOutcome taxi_step_009cd540(AirfieldTaxiHost& host, float yaw_request, float speed_request) {
    const void* holder = host.plane_contact_holder();

    // 009CD564-009CD586: the task needs state 5 and a holder; anything else fails it.
    if (host.plane_flight_state() != 5 || holder == nullptr) {
        host.fail_task_009cd583();
        return TaxiStepOutcome::Failed;
    }

    // 009CD58C-009CD5B5: a missing or dead owner drops the plane off the path.
    const void* block = host.block_of_holder(holder);
    const void* owner = block != nullptr ? host.owner_unit_of_block(block) : nullptr;
    if (owner == nullptr || host.owner_out_of_action()) {
        host.run_surface_release_007b9000();
        host.leave_path_007c1680();
        return TaxiStepOutcome::DroppedOff;
    }

    // 009CD5B8-009CD5E2.
    host.request_neutral_controls_009cd5b8();

    // 009CD5E8-009CD5FF: the taxi target, in the airfield's local frame.
    float target[3] = {0.0f, 0.0f, 0.0f};
    host.taxi_target_006cf420(target);

    // 009CD62F-009CD643. The deltas feed the rate test whose arithmetic is the
    // host's; they are computed here because the switch below is expressed in
    // terms of the caller's verdict, not of them.
    const float delta_x = target[0] - host.plane_local_x();
    const float delta_z = target[2] - host.plane_local_z();
    static_cast<void>(delta_x);
    static_cast<void>(delta_z);

    // 009CD7E1-009CD802. `needs_path` is the BL of 009CD7D9, which the caller
    // supplies through the sign of its yaw request: a non-zero request is a turn
    // the free band could not serve. The native predicate is the x87 comparison
    // at 009CD7A4-009CD7D7 and is not reduced here.
    const bool needs_path = yaw_request != 0.0f;
    switch (taxi_state_action_009cd7e1(host.plane_flight_state(), needs_path)) {
        case TaxiStateAction::JoinPath:
            host.join_path_007c16f0();
            break;
        case TaxiStateAction::LeavePath:
            host.leave_path_007c1680();
            break;
        case TaxiStateAction::None:
            break;
    }

    // 009CD95E-009CD997: the separation test runs only outside state 5, and a
    // refusal zeroes the speed request.
    float speed = speed_request;
    if (host.plane_flight_state() != 5) {
        if (!host.spot_is_clear_006cf5b0(target[0], target[2])) {
            speed = 0.0f;
        }
    }

    // 009CDC48-009CDC7F.
    host.write_steer_request_009cdc4b(yaw_request);
    host.write_speed_request_009cdc70(speed);
    return TaxiStepOutcome::Drove;
}

bool place_plane_on_spot_006cf9f0(AirfieldTaxiHost& host) {
    // 006CF9F4-006CFA0E: the site needs its airfield and a live block owner.
    const bool allowed = host.site_owner_unit() != nullptr && host.block_owner_alive();

    if (allowed) {
        // 006CFA17 -> 007C5F60. The pose, the attach and the lock, in that order.
        host.pose_on_queue_006cf730();
        host.attach_contact_holder_007c6242();
        host.run_pre_pass_007c5ac0();
        host.set_flight_state_locked_007c627c();
        host.rebuild_slot_lists_008073c0();
        // 006CFA1C-006CFA41, back in 006CF9F0.
        host.arm_after_place_007c3c90();
        host.invalidate_subtree_pose_0042ed50();
        host.notify_plane_placed_006cfa41();
    }

    // 006CFA43-006CFA54: the watch is cleared on both paths.
    host.clear_ready_plane_watch_006cfa4f();
    return allowed;
}

}  // namespace bsp
