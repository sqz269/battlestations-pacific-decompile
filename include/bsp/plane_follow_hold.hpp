#pragma once

// The plane follow HOLD arm: what a wing member that is already in good
// position is commanded to do.  Packet `cc9_follow_package`.
// docs/PLANE_FOLLOW_HOLD_ARM.md carries the address-by-address evidence.
//
// ADDRESSES.  `009BEE30 BSP_BotStateFollow_CommandStep`, __thiscall(state,
// float dt), RET 4, body 009BEE30-009BFD67.  `009BEE49 CMP byte [ESI+85h],0 /
// 009BEE50 JZ 009BF9EA` sends a member in good position into the hold arm
// 009BEE56-009BF9E5, which ends `JMP 009BFD39` into the shared epilogue.  The
// fly-to arm 009BF9EA-009BFD38 is plane_follow_law.hpp's.
//
// WHAT THE ARM IS.  A formation controller over four axes with gains the game
// authors itself: `Pilot/Follow/yf_*` (yaw), `pf_*` (pitch), `rf_*` (roll)
// and `pwr_back_meter` / `pwr_spd_meterPerSec` (power), singleton+3ECh..420h,
// read through state+6Ch = singleton+380h at block+6Ch..A0h.  It writes the
// plan's slot `desired` values DIRECTLY with their mode words cleared, unlike
// the fly-to arm, which writes a heading point, an altitude and a speed:
//
//     plan+284h/288h  slot 1 yaw desired + active;  plan+2D4h = 0
//     plan+29Ch/2A0h  slot 3 pitch desired + active; plan+2D0h = 0
//     plan+290h/294h  slot 2 roll desired + active;  plan+2CCh = 0
//       or, leader bank >= 0.75: plan+2C4h bank target, plan+2CCh = 1
//     plan+278h/27Ch  slot 0 throttle desired + active
//     plan+2A8h/2ACh  slot 4 air brake desired + active; plan+2D8h = 0
//
// SCOPE.  This is the arithmetic only.  The caller supplies every frame
// quantity already expressed in the member's body frame, which is what the
// image's 004142E0 / 0042D0D0 calls produce: the rule does no pose work.  Not
// wired into any host seam: the brief forbids it without a same-binary control
// run.  Descriptive names are hypotheses, not recovered symbols.

namespace bsp {

// block+6Ch..A0h = singleton+3ECh..420h, in address order.  Authored values in
// this installation's scripts/datatables/planeglobals.lua (mtime 2024-10-29).
struct PlaneFollowHoldGains {
    float yf_hdg_rad = 0.0f;           // +6Ch  1/DEG(10)
    float yf_yaw_v_rad_per_sec = 0.0f; // +70h  0/DEG(10)
    float yf_sidepos_meter = 0.0f;     // +74h  1/25
    float yf_sidedir = 0.0f;           // +78h  1/25
    float pf_pitch_rad = 0.0f;         // +7Ch  1/DEG(10)
    float pf_pitch_v_rad_per_sec = 0.0f; // +80h 1/DEG(100)
    float pf_vertpos_meter = 0.0f;     // +84h  1/25
    float pf_vertdir = 0.0f;           // +88h  1/25
    float rf_roll_rad = 0.0f;          // +8Ch  -1/DEG(30)
    float rf_roll_v_rad_per_sec = 0.0f; // +90h -0/DEG(80)
    float rf_hdg_rad = 0.0f;           // +94h  0/DEG(40)
    float rf_hdg_v_rad_per_sec = 0.0f; // +98h  0/DEG(400)
    float pwr_back_meter = 0.0f;       // +9Ch  1/10
    float pwr_spd_meter_per_sec = 0.0f; // +A0h 1/KMH(10)
};

// One aircraft's attitude as the arm reads it.  `pitch`/`bank` are unit+C64h /
// +C68h, `heading` is vtable slot +50h (0074E260, FLD [ECX+0C6Ch]), `turn_c70`
// is unit+C70h (paired with rf_hdgV "turn speed difference"), `speed` is
// vtable slot +38h (007B8E60 BSP_Unit_GetCachedSpeed), and `rate_a0/a4/a8` are
// the flight controller's ctl+A0h/A4h/A8h = unit+B50h/B54h/B58h, which the
// gains pair with roll, pitch and yaw speed respectively.
struct PlaneFollowHoldAttitude {
    float pitch = 0.0f;
    float bank = 0.0f;
    float heading = 0.0f;
    float turn_c70 = 0.0f;
    float speed = 0.0f;
    float rate_a0 = 0.0f;
    float rate_a4 = 0.0f;
    float rate_a8 = 0.0f;
};

struct PlaneFollowHoldInputs {
    PlaneFollowHoldAttitude own;
    PlaneFollowHoldAttitude leader;
    bool leader_present = false;           // state+2Ch != 0 (009BEEF5)
    // leader[9C2h + word[00F876B8]*8], the leader's published unit+520h
    // (009BEF09).  docs/BOMBER_AFTER_TASK.md 6c reads it provisionally as
    // "under human control"; for an AI leader it is clear.
    bool leader_published_520 = false;
    // state+30h (the station) through the member's inverse pose, 009BEE92.
    float station_local[3] = {0.0f, 0.0f, 0.0f};
    // leader+ECh (its forward row) as a direction in the member's frame,
    // 009BEEED 0042D0D0(..., normalize = 0).
    float leader_forward_local[3] = {0.0f, 0.0f, 0.0f};
    // state+84h, and state+78h taken through the leader's pose then the
    // member's inverse (009BF13D, 009BF177).  Written by 009C1FD0's search
    // loop 009C216D-009C22F5, which this packet read only in part.
    bool sight_active_84 = false;
    float sight_local[3] = {0.0f, 0.0f, 0.0f};
    // state+44h (009BFEE0's steer point) through the member's inverse, 009BF57F.
    float steer_local[3] = {0.0f, 0.0f, 0.0f};
    // 007C47F0 BSP_PlaneClass_LevelFlightSpeed on [approach+8], 009BF90D.
    float level_flight_speed = 0.0f;
    PlaneFollowHoldGains gains;
};

struct PlaneFollowHoldCommand {
    // 009BF0B8-009BF0E8: unit+520h = 1, plan+270h = leader, 009BE050 copies
    // the leader's unit+AC8h velocity into the member, and the step RETURNS:
    // none of the fields below is written on this path.
    bool locked = false;
    bool clears_plan_270 = false;   // 009BF0F3, every non-locked tick
    bool clears_sight_84 = false;   // 009BF295
    float sight_pitch = 0.0f;       // clamp(20 y/z, -1, 1), 009BF24F
    float sight_yaw = 0.0f;         // clamp(30 x/z, -1.2, 1.2), 009BF28F
    float bank_gate = 0.0f;         // 009BF35E, 1 at |leader bank| <= 0.2
    float yaw_284 = 0.0f;
    float pitch_29c = 0.0f;
    bool writes_roll_290 = false;   // |leader bank| < 0.75, 009BF736
    float roll_290 = 0.0f;
    bool writes_bank_target_2c4 = false;
    float bank_target_2c4 = 0.0f;   // 009BF783, with plan+2CCh = 1
    float power_spd_90 = 0.0f;      // state+90h, 009BF8C4
    float power_back_94 = 0.0f;     // state+94h, 009BF8DE
    float power_floor = 0.0f;       // 009BF94A
    float throttle_278 = 0.0f;
    float air_brake_2a8 = 0.0f;
};

// The shaping both power errors pass through, 009BF7CA-009BF834 and
// 009BF836-009BF8B9: slope 0.4 inside |x| <= 5, joined linearly to the
// identity at |x| = 10, identity beyond.
float plane_follow_hold_shape_009bf7ca(float x) noexcept;

PlaneFollowHoldCommand plane_follow_hold_command_009bee56(
    const PlaneFollowHoldInputs& in) noexcept;

}  // namespace bsp
