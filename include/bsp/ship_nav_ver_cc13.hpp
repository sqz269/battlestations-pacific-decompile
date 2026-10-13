#pragma once

// Packet cc13-ver7 (docs/CC13_VER_7.md): models for the parts of the running ship
// navigator that the host does not run. Names are hypotheses, not recovered symbols.
// Reconstructed and build-tested only; not ABI-compatible and not game-validated.

#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// 00826D70 BSP_ShipUnit_UpdateOutOfMapCountdown
// ---------------------------------------------------------------------------
// void __thiscall(unit, float dt), RET 4, body 00826D70-00826F0B, complete.
// It is slot +1D8h of every ship class vtable (Battleship 00CF90B0, Cargo 00CFA778,
// Cruiser 00CFB738, UnitInstance 00CFC3D0, LandingShip 00CFFA30 and four more: the
// slot's nine data references 00CF9288 ... 00D0C820), the slot 00953D98 calls for
// every unit; the plane's is 007C6C30. It has no direct caller.
//
// Unit fields: +5Dh dead byte (00826D74), +6ECh exit state, +6F0h exit timer,
// +100Ch warning timer. Settings (00424C40): +22Ch Retreat.ExitDist,
// +230h Retreat.ExitTime, +234h Retreat.WarningRepeatTime.
struct ShipOutOfMapState {
    std::int32_t exit_state_6ec{0};
    float exit_timer_6f0{0.0f};
    float warning_timer_100c{0.0f};
};

struct ShipOutOfMapSettings {
    float exit_dist_22c{0.0f};
    float exit_time_230{0.0f};
    float warning_repeat_234{0.0f};
};

class ShipOutOfMapHost {
public:
    virtual ~ShipOutOfMapHost() = default;
    // 00826D9A, 0071C4F0([00E188A8], unit+FCh).
    virtual bool point_outside_map_0071c4f0() = 0;
    // 00826DAB, 00811F50(1): the local player's or a player-led unit, or a
    // retreat / cruise / qualifying attackmove, moveto or moveonpath command.
    virtual bool command_blocks_auto_action_00811f50() = 0;
    // 00826E00, 0059C9B0([00E188A8], unit+FCh, margin).
    virtual bool point_beyond_map_margin_0059c9b0(float margin) = 0;
    // 00826E2C, 007788B0.
    virtual bool formation_follower_007788b0() = 0;
    // 00826E39 and 00826E46, 00927F30(0); called twice on that path.
    virtual bool local_player_role_00927f30() = 0;
    // 00826E8F, 00982120([00F8A0C4], unit, 0): the "exitzone" warning channel.
    virtual void dispatch_exit_zone_warning_00982120() = 0;
    // 00826ECF..00826ED7: 00489C40([00E188A8], unit+FCh) then 00811E80(record),
    // which routes MT_UNITEXITED (50h); a ship's handler 0081BEB0 kills the
    // mission entity with reason 4 ("exitzone") and sets Lua ExitZoneParty.
    virtual void route_unit_exited_00811e80() = 0;
};

struct ShipOutOfMapResult {
    bool counting{false};     // the countdown arm ran (outside and blocked)
    bool warned{false};       // 00982120 ran
    bool exited{false};       // 00811E80 ran
};

ShipOutOfMapResult ship_out_of_map_countdown_00826d70(bool dead_5d, ShipOutOfMapState& state,
                                                      const ShipOutOfMapSettings& settings,
                                                      float dt, ShipOutOfMapHost& host);

}  // namespace bsp
