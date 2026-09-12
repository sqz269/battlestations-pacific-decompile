// The commanded speed on the navigator parameter block at *(unit+73Ch) and the
// weapon director's command-stage ladder. docs/UNIT_COMMANDED_SPEED.md.
//
// Packet cc_commanded_speed. Names are hypotheses, not recovered symbols.
#include "bsp/unit_commanded_speed.hpp"

namespace bsp {
namespace {

// 00D7A218 is the 0.0f every commanded-speed compare uses.
constexpr float kCommandedSpeedEnableThreshold = 0.0f;  // 00D7A218

// 00835C28 CVTTSS2SI: the clock is truncated to a signed int before the age
// subtract, so the whole-second floor is what the age is measured from.
float truncate_to_int_00835c28(float value) noexcept {
    return static_cast<float>(static_cast<int>(value));
}

} // namespace

bool navigator_commanded_speed_active(const CruiseSpeedSetting& setting) noexcept {
    // 009E12AC / 00836AC1 / 00836E59: COMISS enable, 00D7A218 then a
    // below/not-below branch, so the boundary value 0.0f counts as active.
    return setting.enable >= kCommandedSpeedEnableThreshold;
}

CruiseSpeedSetting navigator_commanded_speed_store_00890e6f(float requested,
                                                            float mission_clock) noexcept {
    CruiseSpeedSetting setting{};
    // 00890E6F..00890E86 and 008A38D5..008A3901: FLD requested, FLDZ,
    // FCOMPI ST(1), JBE keeps the value, the fall-through zeroes it. The
    // comparison is 0.0 against the request, so a NaN request takes the
    // zeroing branch the same way a negative one does.
    setting.speed = (kCommandedSpeedEnableThreshold <= requested) ? requested : 0.0f;
    // 00890E8B / 008A3906: the mission clock DAT_00F876A4.
    setting.enable = mission_clock;
    return setting;
}

bool navigator_commanded_speed_stale_00835c28(float mission_clock,
                                              float commanded_at) noexcept {
    // 00835C28..00835C44, computed in x87 double after the int round trip.
    const double age = static_cast<double>(truncate_to_int_00835c28(mission_clock)) -
                       static_cast<double>(commanded_at);
    return age > static_cast<double>(kNavigatorCommandedSpeedMaxAge);
}

bool weapon_director_step_prepass_00836941(const WeaponDirectorCommandState& state,
                                           WeaponDirectorStageHost& host) {
    // 00836941..00836957: the flag the idle tail reads at 00836DE0.
    bool no_command_at_all = false;
    if (state.primary_command == 0u) {
        no_command_at_all = (host.director_filled_command_slots_0071be60() == 0);
    }

    // 00836962..00836985.
    if (state.primary_stage == kDirectorCommandStageRunning) {
        const bool crowded = host.director_filled_command_slots_0071be60() > 1;
        if (crowded || state.unit_player_controlled) {
            host.director_raise_primary_stage_0071d810(kDirectorCommandStageFinished);
        }
    }

    // 0083698A..00836993.
    if (state.secondary_stage == kDirectorCommandStageRunning) {
        host.director_raise_secondary_stage_0071d9e0(kDirectorCommandStageFinished);
    }
    return no_command_at_all;
}

bool weapon_director_abandon_for_far_weapon_target_008369a1(int filled_slots,
                                                            int primary_stage,
                                                            int primary_category,
                                                            int queued_category,
                                                            bool queued_target_resolves,
                                                            double planar_distance_sq,
                                                            WeaponDirectorStageHost& host) {
    if (filled_slots <= 1) {                       // 008369A1
        return false;
    }
    if (primary_stage >= kDirectorCommandStageRunning) {  // 008369A9
        return false;
    }
    // 008369BC/008369C4: a weapon command in the primary slot is left alone.
    if (primary_category == 1 || primary_category == 2) {
        return false;
    }
    // 008369ED/008369F1: only a queued weapon command triggers the test.
    if (queued_category != 1 && queued_category != 2) {
        return false;
    }
    if (!queued_target_resolves) {                 // 00836A0E
        return false;
    }
    if (planar_distance_sq <= kDirectorWeaponTargetAbandonDistanceSq) {  // 00836A76
        return false;
    }
    host.director_raise_primary_stage_0071d810(kDirectorCommandStageFinished);  // 00836A7C
    return true;
}

bool weapon_director_stop_arm_00836a8b(const WeaponDirectorCommandState& state,
                                       const CruiseSpeedSetting& commanded_speed,
                                       WeaponDirectorStageHost& host) {
    if (state.primary_stage == kDirectorCommandStageFinished) {  // 00836A81
        return false;
    }
    if (state.primary_command != kCommandObjectStop) {           // 00836A8E
        return false;
    }
    // 00836A9D..00836AC8: any one of the three ends the stop.
    bool raise = host.director_filled_command_slots_0071be60() > 1;
    if (!raise) {
        raise = state.unit_player_controlled;
    }
    if (!raise) {
        raise = navigator_commanded_speed_active(commanded_speed);
    }
    if (raise) {
        host.director_raise_primary_stage_0071d810(kDirectorCommandStageFinished);  // 00836AD2
        return true;
    }
    return false;
}

DirectorDefaultCommand weapon_director_idle_reissue_00836dc9(
        const WeaponDirectorCommandState& state,
        bool prepass_flag,
        const CruiseSpeedSetting& commanded_speed_after_reset,
        WeaponDirectorStageHost& host) {
    const bool finished = (state.primary_stage == kDirectorCommandStageFinished);

    // 00836DCE..00836DE5.
    bool proceed = false;
    if (finished && host.director_filled_command_slots_0071be60() <= 1) {
        proceed = true;
    } else if (prepass_flag) {
        proceed = true;
    }
    if (!proceed) {
        return DirectorDefaultCommand::None;
    }

    // 00836DEB..00836DFC: one slot may stand when the stage already finished.
    const int allowed = finished ? 1 : 0;
    if (host.director_filled_command_slots_0071be60() > allowed) {
        return DirectorDefaultCommand::None;
    }

    // 00836E02..00836E0B: the director's own vtable[6Ch] runs first. It is what
    // can expire the commanded speed the choice below then reads, which is why
    // the caller passes the pair as it stands after the reset.
    host.director_reset_command_stage_vtable6c(true);

    // 00836E0D..00836E90.
    if (host.unit_controller_belongs_to_another_007788b0()) {
        const std::uint32_t owner = host.unit_controller_owner_007788d0();
        const std::uint32_t target = host.make_command_target_00465080(owner, 0.0f);
        host.director_issue_command_0071ecf0(kCommandObjectFollow, target);
        return DirectorDefaultCommand::Follow;
    }

    const bool cruise = state.unit_player_controlled ||
                        navigator_commanded_speed_active(commanded_speed_after_reset);
    // 00836E6C and 00836E84 both push EAX, which still holds [director+24Ch]:
    // the unit orders itself.
    const std::uint32_t self_target = host.make_command_target_00465080(state.unit, 0.0f);
    if (cruise) {
        host.director_issue_command_0071ecf0(kCommandObjectCruise, self_target);
        return DirectorDefaultCommand::Cruise;
    }
    host.director_issue_command_0071ecf0(kCommandObjectStop, self_target);
    return DirectorDefaultCommand::Stop;
}

CruiseSpeedSetting weapon_director_reset_command_stage_00835bf0(
        bool primary,
        const CruiseSpeedSetting& commanded_speed,
        float mission_clock,
        WeaponDirectorStageHost& host) {
    host.director_clear_command_slot_0071c130(primary);  // 00835BFA

    CruiseSpeedSetting after = commanded_speed;
    // 00835C0B..00835C54. `sete al` at 00835C12 makes `al` the "not primary" bit.
    bool clear = false;
    if (navigator_commanded_speed_active(commanded_speed)) {
        if (!primary) {
            clear = true;                                   // 00835C26 -> 00835C4C
        } else {
            clear = navigator_commanded_speed_stale_00835c28(mission_clock,
                                                             commanded_speed.enable);
        }
    } else {
        // 00835C48: the primary path leaves an already inactive pair alone.
        clear = !primary;
    }
    if (clear) {
        after.enable = kCruiseSpeedSettingInactive;         // 00835C4C / 00835C54
        host.set_navigator_commanded_speed_time(kCruiseSpeedSettingInactive);
    }

    // 00835C59..00835C65. The literal 0 makes 00822B70's whole body a no-op;
    // this projection makes the call anyway because the native does.
    if (primary) {
        host.navigator_params_reset_from_tuning_00822b70(false);
    }
    return after;
}

ShipCruiseStateArm ship_cruise_state_arm_009e1188(bool controller_present,
                                                  bool unit_740h_present,
                                                  bool gate_accepts,
                                                  bool unit_player_controlled) noexcept {
    if (!controller_present) {          // 009E1188
        return ShipCruiseStateArm::Return;
    }
    if (!unit_740h_present) {           // 009E1196
        return ShipCruiseStateArm::Return;
    }
    if (!gate_accepts) {                // 009E11B4
        return ShipCruiseStateArm::Detached;
    }
    if (unit_player_controlled) {       // 009E11C8
        return ShipCruiseStateArm::Player;
    }
    return ShipCruiseStateArm::Cruise;  // 009E1265
}

CruiseSpeedSetting ship_cruise_state_commanded_speed_after_arm_009e13f8(
        ShipCruiseStateArm arm, const CruiseSpeedSetting& before) noexcept {
    CruiseSpeedSetting after = before;
    if (arm == ShipCruiseStateArm::Detached) {
        after.enable = kCruiseSpeedSettingInactive;  // 009E13F8
    }
    return after;
}

} // namespace bsp
