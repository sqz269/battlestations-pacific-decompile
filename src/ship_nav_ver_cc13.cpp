// Packet cc13-ver7. docs/CC13_VER_7.md. Disk bytes read from the Ghidra listing
// export and checked against capstone; constants read from the PE.
#include "bsp/ship_nav_ver_cc13.hpp"

namespace bsp {

ShipOutOfMapResult ship_out_of_map_countdown_00826d70(bool dead_5d, ShipOutOfMapState& state,
                                                      const ShipOutOfMapSettings& settings,
                                                      float dt, ShipOutOfMapHost& host) {
    ShipOutOfMapResult out;
    // 00826D74 CMP byte [ESI+5Dh],0 / JNZ 00826F09.
    if (dead_5d) return out;
    // 00826D7E..00826D87: the pose refresh 00414DB0 when +C8h is clear; the host
    // pose is always current, so it is not modelled.
    if (!host.point_outside_map_0071c4f0() || !host.command_blocks_auto_action_00811f50()) {
        // 00826EE2..00826F00.
        state.exit_timer_6f0 = settings.exit_time_230;
        state.exit_state_6ec = 1;
        state.warning_timer_100c = 0.0f;
        return out;
    }
    out.counting = true;
    // 00826DBE FLD [+6F0h]; 00826DCB FSUB dt; 00826DDD FSTP: one float store.
    state.exit_timer_6f0 = state.exit_timer_6f0 - dt;
    std::int32_t next_state = 2;  // 00826DE3 MOV EDI,2
    // 00826E00 0059C9B0 with the margin, then 00826E09 COMISS 0.0,[+6F0h] / JBE:
    // the timer is below zero (an unordered compare takes the JBE).
    if (host.point_beyond_map_margin_0059c9b0(settings.exit_dist_22c)
        || 0.0f > state.exit_timer_6f0) {
        next_state = 3;                    // 00826E1D
        state.exit_timer_6f0 = -1.0f;      // 00826E15, 00D7A260
    }
    // 00826E2C..00826E40: followers of a formation warn only when they are the
    // local player's role.
    if (!host.formation_follower_007788b0() || host.local_player_role_00927f30()) {
        if (host.local_player_role_00927f30()) {
            // 00826E4F FLD, FSUB dt, FSTP.
            state.warning_timer_100c = state.warning_timer_100c - dt;
        } else {
            state.warning_timer_100c = 1.0f;   // 00826E61, 00D7A24C
        }
        // 00826E71 reads the state of the previous call: +6ECh is only stored at
        // 00826EAF, after this test.
        if (state.exit_state_6ec == 1 || 0.0f > state.warning_timer_100c) {
            host.dispatch_exit_zone_warning_00982120();
            out.warned = true;
            state.warning_timer_100c = settings.warning_repeat_234;  // 00826E99
        }
    }
    state.exit_state_6ec = next_state;   // 00826EAF
    // 00826EA5 COMISS 0.0,[+6F0h] / JBE 00826F08.
    if (0.0f > state.exit_timer_6f0) {
        host.route_unit_exited_00811e80();
        out.exited = true;
    }
    return out;
}

}  // namespace bsp
