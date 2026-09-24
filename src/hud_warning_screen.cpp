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

}  // namespace bsp
