// The ship AI state family at vtable 00D21598 and the control block the three
// desired-value setters write. Every routine below carries its native address,
// ABI and coverage in bsp/ship_ai_states.hpp and docs/SHIP_AI_STATES.md.
// Names are hypotheses, not recovered symbols.

#include "bsp/ship_ai_states.hpp"

#include <array>
#include <cmath>

#include "bsp/unit_rudder.hpp"
#include "bsp/vector_helpers.hpp"

namespace bsp {
namespace {

// 00D7A260 = -1.0f and 00D7A24C = +1.0f. 009DBF90 and 009DFFB0 both clamp with
// two COMISS/JA pairs; JA is not taken on an unordered compare, so a NaN falls
// through to MOVAPS at 009DBFBD and is stored unchanged.
float clamp_unit_interval_comiss(float value) noexcept {
    if (-1.0f > value) {   // 009DBFA5/009DBFAE: COMISS -1.0, v; JA
        return -1.0f;
    }
    if (value > 1.0f) {    // 009DBFB8/009DBFBB: COMISS v, 1.0; JA
        return 1.0f;
    }
    return value;          // 009DBFBD: MOVAPS
}

// 00D7A264, the image's float pi, also the value 00438AA0 wraps against.
constexpr float kPi = 3.1415927410125732421875f;

}  // namespace

// ---------------------------------------------------------------------------

bool ship_ai_sync_state_to_command_009f3dd0(ShipAiSyncHost& host) {
    host.refresh_director_vtable_0114();                        // 009F3DE2
    std::uint32_t command = host.director_current_command_0071be40(); // 009F3DE6
    if (host.unit_player_controlled_0184()) {                   // 009F3DF3
        command = kShipAiCruiseCommandObject;                   // 009F3DFC
    } else if (command == 0u) {
        return false;                                           // 009F3E05
    }
    if (host.active_state_command_vtable24() == command) {      // 009F3E12
        return false;                                           // 009F3E16
    }
    host.select_state_for_command_009f3d00(command);            // 009F3E1B
    return true;                                                // 009F3E21
}

// ---------------------------------------------------------------------------

void ship_ai_set_desired_throttle_009dbf90(ShipAiControlBlock& blk, float throttle) noexcept {
    blk.throttle_hold_1c8 = 0;                                  // 009DBFA8
    blk.desired_throttle = clamp_unit_interval_comiss(throttle); // 009DBFC0
    blk.requested_direction = ShipAiThrottleDirection::Stopped;  // 009DBFC8, +1CCh
}

void ship_ai_set_desired_steering_009dffb0(ShipAiControlBlock& blk, float rudder,
                                           ShipAiSetterHost& host) {
    if (blk.mode != ShipAiSteeringMode::Rudder) {
        blk.timer_360 = 0.0f;
        blk.timer_368 = 0.0f;
        blk.mode = ShipAiSteeringMode::Rudder;
        host.on_steering_mode_change_009da4e0();
    }
    blk.desired_rudder = clamp_unit_interval_comiss(rudder);
}

void ship_ai_set_desired_heading_009e0040(ShipAiControlBlock& blk, float heading,
                                          ShipAiSetterHost& host) {
    if (blk.mode != ShipAiSteeringMode::Heading) {
        blk.timer_360 = 0.0f;
        blk.timer_368 = 0.0f;
        blk.mode = ShipAiSteeringMode::Heading;
        host.on_steering_mode_change_009da4e0();
    }
    blk.desired_heading = heading; // stored unclamped
    host.after_heading_stored_00605070(blk.desired_heading);
}

// ---------------------------------------------------------------------------

bool ship_ai_direct_control_arm_009ed6b0(ShipAiControlBlock& blk, float seconds,
                                         ShipAiDirectControlHost& host) {
    host.prologue_0080e000(seconds);                            // 009ED6DE
    if (blk.flag_3a5 && !host.controller_belongs_to_another_007788b0()) { // 009ED73F
        blk.flag_3a5 = false;
    }
    // 009ED74D..009ED780: the per-step resets this projection needs.
    blk.distance_330 = 0.0f;                                    // 009ED780
    if (blk.timer_368 >= 0.0f) {
        blk.timer_368 -= seconds;
    }
    if (blk.timer_360 >= 0.0f) {
        blk.timer_360 -= seconds;
    }

    // 009ED7D8 also latches (mode == NavigateAstern) into a local the native
    // routine only reads in the unprojected navigation arm; it is not modelled.
    if (blk.mode != ShipAiSteeringMode::Navigate) {
        // The ahead/astern latch. EBP = 1 (009ED7F7) and EDI = 2 (009ED7EC);
        // ECX = 0 (009ED767) is the comparison and store value for "stopped".
        bool enter_direction = false;
        if (blk.throttle_hold_1c8 == 0) {                       // 009ED802
            const float magnitude = std::fabs(blk.desired_throttle); // 009ED80A
            if (!(static_cast<double>(magnitude) > kShipAiThrottleDeadzone)) {
                if (blk.direction != ShipAiThrottleDirection::Stopped) {
                    blk.direction = ShipAiThrottleDirection::Stopped;
                    blk.direction_value_374 = -1.0f;
                    blk.direction_counter_384 = 0;
                    blk.timer_360 = 0.0f;
                }
                blk.desired_throttle = 0.0f;                    // 009ED87F
            } else if (blk.desired_throttle > 0.0f) {
                blk.requested_direction = ShipAiThrottleDirection::Ahead; // 009ED83B
                if (blk.direction != ShipAiThrottleDirection::Ahead) {    // 009ED841
                    blk.direction = ShipAiThrottleDirection::Ahead;
                    enter_direction = true;
                }
            } else {
                blk.requested_direction = ShipAiThrottleDirection::Astern; // 009ED851
                if (blk.direction != ShipAiThrottleDirection::Astern) {    // 009ED897
                    blk.direction = ShipAiThrottleDirection::Astern;
                    enter_direction = true;
                }
            }
        } else if (blk.requested_direction == ShipAiThrottleDirection::Ahead) { // 009ED889
            if (blk.direction != ShipAiThrottleDirection::Ahead) {
                blk.direction = ShipAiThrottleDirection::Ahead;
                enter_direction = true;
            }
        } else if (blk.requested_direction == ShipAiThrottleDirection::Astern) {
            if (blk.direction != ShipAiThrottleDirection::Astern) {
                blk.direction = ShipAiThrottleDirection::Astern;
                enter_direction = true;
            }
        }
        if (enter_direction) {                                   // 009ED8A5
            blk.timer_360 = 1.0f;
            blk.direction_counter_384 = 0;
            blk.direction_value_374 = -1.0f;
        }

        float look_ahead;
        if (blk.direction == ShipAiThrottleDirection::Stopped) { // 009ED8BD
            const float speed = host.unit_body_axis_speed_0092d730(); // 009ED8D1
            const float divisor = host.ship_class_field_0508();       // 009ED8EC
            const float distance =
                (speed / divisor) * speed * 0.5f + host.unit_field_09c8(); // 009ED902
            blk.distance_32c = distance;                          // 009ED910
            look_ahead = distance;
        } else {
            const float distance = blk.cruise_distance_3e0;       // 009ED918
            blk.distance_32c = distance;                          // 009ED930
            look_ahead = distance + kShipAiLookAheadBonus;
        }
        blk.distance_330 = look_ahead;                            // 009ED93F

        if (blk.mode == ShipAiSteeringMode::Rudder) {             // 009ED938
            const float heading = host.unit_heading_vtable_0050(); // 009ED95D
            blk.heading_target_324 = heading;                      // 009ED976
            if (blk.clamp_354 < 2.0f) {                            // 009ED96F, 00CE3958
                blk.clamp_354 = 2.0f;                              // 009ED97E
            }
            float aim = heading;
            if (blk.direction == ShipAiThrottleDirection::Astern) { // 009ED986
                aim = wrapped_angle_add_00438aa0(heading, kPi);
                blk.heading_target_324 = aim;                      // 009ED9A3
            }
            aim = blk.heading_target_324;                          // 009ED9AD
            const float yaw_rate = host.unit_current_yaw_rate_00811940(); // 009ED9C1
            blk.heading_target_324 = blk.yaw_rate_subtracts_364     // 009ED9D1
                ? wrapped_angle_subtract_00438b10(aim, yaw_rate)    // 009ED9E5
                : wrapped_angle_add_00438aa0(aim, yaw_rate);        // 009ED9EC
        } else {
            blk.heading_target_324 = blk.desired_heading;           // 009ED947
        }
    }

    if (blk.early_out_3f5) {                                        // 009ED9FF
        if (blk.clamp_354 < 1.0f) {                                 // 009EDA08
            blk.clamp_354 = 1.0f;                                   // 009EDA11
        }
        return true;                                                // 009EDA25
    }
    return false;
}

// ---------------------------------------------------------------------------

bool ship_ai_controller_step_009f50e0(ShipAiControllerTimers& timers, float seconds,
                                      ShipAiControllerHost& host) {
    if (!host.unit_present_0b00()) {  // 009F50E4
        return false;
    }
    if (host.unit_flag_005d()) {      // 009F50F2
        return false;
    }
    if (host.unit_flag_0061()) {      // 009F50FC
        return false;
    }
    if (host.sync_state_009f3dd0()) { // 009F5106
        timers.interval_0b14 = 0.0f;  // 009F5112
    }
    timers.elapsed_0b18 += seconds;                              // 009F512E
    const bool replan = !(timers.elapsed_0b18 < timers.interval_0b14); // 009F513C
    host.pre_step_009e0270(replan);                              // 009F5156
    if (replan) {
        const float elapsed = timers.elapsed_0b18;
        host.replan_prepare_009f1420(elapsed);                   // 009F516C
        host.state_step_vtable0c(elapsed);                       // 009F5186
        timers.elapsed_0b18 = 0.0f;                              // 009F5191
        timers.interval_0b14 =
            host.state_interval_vtable28() * kShipAiIntervalScale; // 009F519E
        host.replan_finish_009ddbc0();                           // 009F51AE
    } else {
        host.hold_009da0d0();                                    // 009F51B7
    }
    host.step_009eca20(seconds);                                 // 009F51C6
    host.step_009da6e0(seconds);                                 // 009F51D5
    host.step_009f0ea0(seconds);                                 // 009F51E4
    host.step_009e04e0(seconds);                                 // 009F51F3
    host.step_009ef230();                                        // 009F51FA
    if (host.navigate_009ed6b0(seconds)) {                       // 009F5209
        timers.interval_0b14 = 0.0f;                             // 009F5215
    }
    host.publish_009f4d10(seconds);                              // 009F5227
    host.tail_009da8d0(seconds);                                 // 009F5239
    host.tail_009f4da0(seconds);                                 // 009F5248
    return true;
}

// ---------------------------------------------------------------------------

ShipAiArrivalTest ship_ai_movetopos_arrival_009e58b9(float unit_x, float unit_z,
                                                     float goal_x, float goal_z,
                                                     int other_radius_07a0,
                                                     float unit_field_09c8) noexcept {
    ShipAiArrivalTest out{};
    // 009E5891..009E58B5: goal minus position, x then z, each stored to float.
    const std::array<float, 2> delta{goal_x - unit_x, goal_z - unit_z};
    out.distance = length_2d_00414c60(delta);                    // 009E58B9
    // 009E58CE FILD [other+7A0h], 009E58D4 FSUB [unit+9C8h].
    out.radius = static_cast<float>(other_radius_07a0) - unit_field_09c8;
    // 009E58DA FCOMPI, 009E58DE JBE: the step continues only when the radius
    // is strictly greater than the distance.
    out.reached = out.radius > out.distance;
    return out;
}

// ---------------------------------------------------------------------------

bool ship_ai_command_available_008162b0(ShipAiCommandAvailabilityHost& host,
                                        bool has_target) {
    if (host.call_00779d50()) {                                  // 008162BF
        return true;                                             // 008162CA
    }
    if (!host.entity_offers_command_vtable168()) {                // 008162DB
        return false;
    }
    if (!has_target) {
        return !host.registry_command_flag_00467170();            // 008162EB, 008162F7
    }
    const bool kind_ok = host.target_kind_vtable5c(0x05)          // 0081630D
                      || host.target_kind_vtable5c(0x18)          // 0081631C
                      || host.target_kind_vtable5c(0x1A);         // 0081632B
    if (!kind_ok || host.target_flag_005d()) {                    // 00816337
        return false;
    }
    if (host.name_differs_from_attackmove_00438e10()) {           // 00816346
        return true;                                             // 0081634D
    }
    if (!host.sides_related_00803510()) {                         // 00816359
        return false;
    }
    if (host.self_kind_vtable5c(0x08)) {                          // 0081636F
        if (!host.target_kind_vtable5c(0x06)) {                   // 0081637E
            return false;
        }
        if (host.target_kind_vtable5c(0x08)) {                    // 0081638D
            return true;
        }
        if (host.call_00827f70()) {                               // 0081639D
            return false;
        }
    }
    if (host.target_is_airborne_00922b10()) {                     // 008163A8
        return false;
    }
    if (!host.target_kind_vtable5c(0x08)) {                       // 008163BA
        return true;
    }
    if (host.call_00852820()) {                                   // 008163C6
        return true;
    }
    return host.any_child_accepts_00465020_0080f750();             // 008163E2, 008163ED
}

}  // namespace bsp
