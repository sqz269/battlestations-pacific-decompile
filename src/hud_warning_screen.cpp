#include "bsp/hud_warning_screen.hpp"

// Packet cc9_screen_50h. docs/SHIP_SCREEN_UPDATE.md section 16.

namespace bsp {
namespace {

constexpr int kKindPlane = 0x18;                       // 0068304F PUSH 18h
constexpr int kKindShip = 6;                           // 00683097 PUSH 6
constexpr int kKindSubmarine = 8;                      // 006830D8 PUSH 8
constexpr double kOxygenBelow = 0.20000000298023224;   // 00CE3D10
constexpr float kHoldSeconds = 1.0f;                   // 00D7A24C

// 0068306D..00683087 and its three copies: a first raise zeroes the phase.
void raise(WarningScreenState& screen, int alert) {
    if (!screen.active[alert]) screen.phase[alert] = 0.0f;
    screen.active[alert] = true;
    screen.hold[alert] = kHoldSeconds;
}

}  // namespace

void warning_screen_update_00683020(WarningScreenState& screen, WarningScreenHost& host,
                                    float dt) {
    screen.dt_78 = dt;                                                // 0068302D
    const std::size_t unit = host.controlled_unit();                  // 00683032
    if (unit != 0 && !host.controlled_flag_5d()) {                    // 00683038..00683044
        if (host.controlled_is_kind_of(kKindPlane) && host.plane_stall_007c6e10()) {
            raise(screen, 0);                                         // 0068304A..00683087
        }
        if (host.controlled_is_kind_of(kKindShip) && host.ship_shallow_1011()) {
            raise(screen, 2);                                         // 0068308C..006830C8
        }
        if (host.controlled_is_kind_of(kKindSubmarine) &&
            kOxygenBelow > static_cast<double>(host.submarine_127c()) &&
            host.submarine_below_00852860()) {
            raise(screen, 1);                                         // 006830CD..0068311D
        }
        if (!host.pose_current_c8()) host.refresh_pose_00414db0();    // 00683128..00683135
        if (host.near_world_edge_00681f40()) raise(screen, 3);        // 00683140..00683176
    }
    // 00683181..006831C0: a different (or lost) unit clears every alert and
    // stops its sound.
    if (screen.last_unit_74 != 0 && screen.last_unit_74 != unit) {
        for (int i = 0; i < 4; ++i) {
            screen.hold[i] = 0.0f;
            screen.active[i] = false;
            if (screen.sound[i]) host.sound_stop(i, true);            // 006831B5 +8(1)
        }
    }
    // 006831C2..00683203.
    if (!screen.held_2c) {
        host.set_visible(WarningWidget::FirstGroup, false);
        host.set_alpha(WarningWidget::Text, 1.0f);
        host.set_alpha(WarningWidget::Icon1, 1.0f);
        host.set_alpha(WarningWidget::Icon2, 1.0f);
    }
    // 00683205..006832D3: at most one alert is shown per frame, the first in
    // index order whose hold has not run out; every other active alert stops.
    bool shown = false;
    for (int i = 0; i < 4; ++i) {
        if (screen.sound[i] && host.sound_finished(i)) {              // 00683207..00683221
            host.sound_release(i);                                    // 0068322B..00683240
            screen.sound[i] = false;                                  // 00683242, 0068324A
        }
        if (!screen.active[i]) continue;                              // 00683252
        const float left = static_cast<float>(static_cast<double>(screen.hold[i]) - dt);
        screen.hold[i] = left;                                        // 00683269
        // 00683271 FCOMIP with JB: a negative or unordered hold stops.
        if (left >= 0.0f && !shown) {
            shown = true;
            host.show_alert(screen, i);                               // 00683282 jump table
        } else {
            if (screen.sound[i]) host.sound_stop(i, false);           // 006832C6 +8(0)
            screen.active[i] = false;                                 // 006832C8
        }
    }
    screen.last_unit_74 = unit;                                       // 006832DF
}

void follow_screen_update_0067bf00(FollowScreen49State& screen, FollowScreen49Host& host) {
    constexpr int kKindTargetable = 2;                                // 0067BF38 PUSH 2
    screen.unit_08 = 0;                                               // 0067BF06
    if (!host.screen_29h_applied()) return;                          // 0067BF09..0067BF17
    // 0067BF1D..0067BF42: the controlled unit's target, when it answers
    // IsKindOf(2).
    std::size_t target = 0;
    const std::size_t controlled = host.controlled_unit();
    if (controlled != 0) {
        target = host.controlled_target_00927880();
        if (target != 0 && !host.is_kind_of(target, kKindTargetable)) target = 0;
    }
    // 0067BF44..0067BF75: screen 29h's unit when it is alive and visible and
    // is not the controlled unit, else the target.
    std::size_t pick = host.screen_29h_unit();
    if (pick == 0 || !host.alive_and_visible(pick) || pick == controlled) pick = target;
    screen.unit_08 = pick;                                            // 0067BF75
    if (controlled == pick) screen.unit_08 = 0;                       // 0067BF78..0067BF81
    // 0067BF84..0067BFC2: the pick must be alive and visible; a plane also
    // needs a live [unit+3D0h].
    if (screen.unit_08 == 0) return;
    if (!host.alive_and_visible(screen.unit_08)) {
        screen.unit_08 = 0;
        return;
    }
    if (!host.is_kind_of(screen.unit_08, kKindPlane)) return;         // 0067BFA4 PUSH 18h
    if (!host.leader_3d0_alive(screen.unit_08)) screen.unit_08 = 0;   // 0067BFAF..0067BFC2
}

void integrated_controls_0064b870(IntegratedControlsState& screen,
                                  IntegratedControlsHost& host, float dt) {
    constexpr double kAxisDeadZone = 0.10000000149011612;             // 00D7A3A0
    constexpr float kNegZero = -0.0f;                                 // 00D7A208
    constexpr double kHalf = 0.5;                                     // 00D7A280
    constexpr float kThrustLow = -0.5f;                               // 00CE69D0
    constexpr float kOne = 1.0f;                                      // 00D7A24C
    constexpr float kMinusOne = -1.0f;                                // 00D7A260
    if (host.unit_byte_6c8()) {                                       // 0064B879
        const IntegratedControlsInputs in = host.inputs();            // 0064B887..0064B8D6
        const bool device = in.query_a92090 || in.query_a92050;       // [ESP+0Bh]
        bool thrust_moved = false;                                    // CL
        if (in.query_a92050) {                                        // 0064B8FF
            if (!screen.latched_30) {
                thrust_moved = true;                                  // 0064B946
            } else if (screen.latch_34 != in.thrust_1bb4) {           // 0064B90F FUCOMIP
                screen.latched_30 = false;                            // 0064B919
            }
        } else {
            // 0064B91E..0064B944: |thrust| as (x > 0 ? x : -0 - x).
            const float mag = in.thrust_1bb4 > 0.0f ? in.thrust_1bb4 : kNegZero - in.thrust_1bb4;
            thrust_moved = static_cast<double>(mag) > kAxisDeadZone;
        }
        const float turn_mag = in.turn_1be4 > 0.0f ? in.turn_1be4 : kNegZero - in.turn_1be4;
        // 0064B96C..0064B9BD: the transfer that gives the player role 1.
        if ((static_cast<double>(turn_mag) > kAxisDeadZone || thrust_moved) &&
            host.unit_1130_clear() && host.local_player_role(0) && !host.local_player_role(1)) {
            host.role_transfer_0077c470(2, 1);                        // 0064B9A6
            screen.turn_28 = host.unit_ordered_rudder();              // 0064B9AE
            screen.thrust_24 = host.unit_throttle();                  // 0064B9B7
        }
        if (host.local_player_role(1)) {                              // 0064B9C5
            if (!device) {
                screen.thrust_24 = static_cast<float>(
                    static_cast<double>(screen.thrust_24) -
                    static_cast<double>(in.thrust_1bb4) * dt);        // 0064B9D9..0064B9E4
            } else if (in.byte_1b91) {                                // 0064B9F0
                if (0.0f < in.thrust_1bb4) {                          // 0064BA02 JC
                    screen.thrust_24 = static_cast<float>(
                        -static_cast<double>(in.thrust_1bb4) * kHalf);   // 0064BA1A
                } else {
                    screen.thrust_24 = kNegZero - in.thrust_1bb4;     // 0064BA07
                }
            }
            // 0064BA29..0064BA92: 00415690 clamps +24h to [-0.5, 1] and
            // +28h - turn*dt to [-1, 1].
            if (kThrustLow > screen.thrust_24) screen.thrust_24 = kThrustLow;
            else if (screen.thrust_24 > kOne) screen.thrust_24 = kOne;
            screen.turn_28 = static_cast<float>(
                static_cast<double>(screen.turn_28) - static_cast<double>(in.turn_1be4) * dt);
            if (kMinusOne > screen.turn_28) screen.turn_28 = kMinusOne;
            else if (screen.turn_28 > kOne) screen.turn_28 = kOne;
            host.issue_order(screen.thrust_24, screen.turn_28);       // 0064BA97..0064BB12
        }
    }
    // 0064BB19..0064BB3C: role 1 is given back while game+19C4h is set.
    if (host.local_player_role(1) && host.game_19c4()) host.role_transfer_0077c470(2, 0);
}

void ship_view_update_0064d610(ShipViewScreen46Host& host, float dt) {
    constexpr int kActionOrder = 0x95;                                // 0064D680 PUSH 95h
    constexpr int kKind0C = 0x0C;                                     // 0064D6BB PUSH 0Ch
    if (!host.wanted_04() || !host.has_unit_1c()) return;            // 0064D62B..0064D639
    host.view_input_0064a400(dt);                                     // 0064D647
    host.integrated_controls_0064b870(dt);                            // 0064D656
    if (host.screen_2eh_present()) host.screen_2eh_005484f0();        // 0064D65B..0064D66D
    // 0064D675: 00815850(unit), the torpedo stock; its result is discarded.
    if (!host.input_pressed(kActionOrder)) return;                    // 0064D685
    if (!host.unit_virtual_234() || !host.unit_local_player()) return;   // 0064D697..0064D6B1
    if (host.unit_is_kind_of(kKind0C)) {                              // 0064D6BD
        if (host.unit_00812960()) host.order_route_0077d600();        // 0064D6C6..0064D6EA
    } else {
        host.order_route_0077c2a0();                                  // 0064D701..0064D71A
    }
}

}  // namespace bsp
