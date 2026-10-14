// Segment 53 reference models (packet cc13-gpr53). See
// include/bsp/gameplay_seg53_cc13.hpp for the adoption note and
// docs/CC13_GPR_53.md for the evidence. Every arithmetic statement below keeps
// the listing's operand order; with the gameplay FPU at 24-bit precision each
// x87 intermediate rounds like a float. Build-tested only.

#include "bsp/gameplay_seg53_cc13.hpp"
#include "bsp/plane_pose_commit.hpp"

#include <cmath>
#include <cstring>
#include <utility>

namespace bsp {
namespace gameplay_seg53_cc13 {

namespace {

// 00CE7630 (double 30.0) and 00CE38C8 (float 30.0): the dt clamp of 00852410.
constexpr float kPredictMaxDt = 30.0f;
// 00D7A308 (double 2.0): the half-width of the band the prediction settles in.
constexpr float kPredictBandTolerance = 2.0f;
// 00D7A280 (double 0.5).
constexpr float kHalf = 0.5f;
// 00D0C314 (float -2.5): the shallowest predicted depth and the default swim
// depth of a torpedo (00856174, 0072CA6B).
constexpr float kShallowestSwimDepth = -2.5f;
// 00D7A2B0 (double 3.0).
constexpr float kThree = 3.0f;
// 00CEFF98 (double 0.60000002384185791 == (double)0.6f).
constexpr float kTorpedoRangeFactor = 0.6f;
// 00CF9058 (double 9.8100004196166992 == (double)9.81f).
constexpr float kGravity = 9.81f;
// 00D0C310 (float 10000.0): heading not set.
constexpr float kHeadingUnset = 10000.0f;
// 00D0C318 (double 26.179939270019531): 1500 degrees per second in radians.
constexpr double kPropellerRate = 26.179939270019531;
// 00857780 constants.
constexpr float kActivateMaxAbsCoordinate = 50000.0f;  // 00D0C5F8
constexpr float kActivateMaxAbsHeight = 1700.0f;       // 00D0C5F0 (double)
constexpr float kDefaultDragFront = 0.5999994277954102f;  // 00D0C5EC (bits 3F199990)
constexpr float kDefaultDragSide = 3.0000007152557373f;   // 00D0C5E8 (bits 40400003)
// 00D0C5E0 (double 3FE33332_00000000): a value a float holds exactly, so the
// double product below is exact and the float cast is the one rounding the
// 24-bit FMUL makes.
constexpr double kAccelPerTravelSpeed = 0.5999994277954102;
constexpr float kDefaultSmoothAndWaveRate = 15.0f;     // 00CE5380
constexpr float kAirWeaponScale = 0.25f;               // 00CE3868

}  // namespace

// ---------------------------------------------------------------------------
// 008522C0. 008522C7: unit+1279h clear -> false. 008522D1..008522EE: for each
// child (unit+48h, next +44h): vtable[5Ch](28h) and child+490h >= 0 (signed
// JGE) -> true. A child that is not a catapult, or is one with +490h < 0, is
// skipped.
// ---------------------------------------------------------------------------
bool submarine_catapult_needs_surface_008522c0(
    bool catapult_surface_enable_1279, const std::vector<SubmarineChild>& children) noexcept {
    if (!catapult_surface_enable_1279) {
        return false;
    }
    for (const SubmarineChild& child : children) {
        if (child.is_catapult && child.launch_state_490 >= 0) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// 00852410. Stack slots after PUSH ESI: [esp+4] A, [esp+8] B, [esp+0Ch] C,
// [esp+10h] D, [esp+14h] E, [esp+18h] F, [esp+2Ch] dt. Every comparison is
// FCOMI/FCOMPI followed by JBE, so an unordered compare takes the JBE arm;
// the `!(a <= b)` forms keep that.
// ---------------------------------------------------------------------------
float submarine_predict_depth_after_00852410(const SubmarineDepthFields& unit,
                                             const SubmarineDiveRates& rates,
                                             float body_velocity_y,
                                             bool catapult_needs_surface,
                                             float dt) noexcept {
    // 00852410..0085242E: dt > 30 -> 30.
    if (dt > kPredictMaxDt) {
        dt = kPredictMaxDt;
    }
    // 00852442..00852463: depthLevel clamped to 0..3.
    std::int32_t level = unit.depth_level_1268;
    if (level < 0) {
        level = 0;
    } else if (level > 3) {
        level = 3;
    }
    const float y = unit.world_y_100;  // C, and A at 008524CD
    // 00852468..00852499: target = clearance when it is above the band.
    const float band = unit.bands_1200[level];
    float target = (unit.clearance_123c > band) ? unit.clearance_123c : band;
    // 00852499..008524B6: needAir, else 008522C0, forces the surface.
    if (unit.need_air_1281 || catapult_needs_surface) {
        target = 0.0f;
    }
    const float vy = body_velocity_y;  // D, 008524DE
    // 008524F9..00852509: the two speed targets, each stored as a float.
    const float descend_speed = unit.sink_1270 - rates.down_speed_834;  // F
    const float ascend_speed = rates.up_speed_830 - unit.sink_1270;     // E
    const float low = target - kPredictBandTolerance;   // 00852519
    float result = y;  // A
    if (low > y) {
        // 00852529..008525F7: below the band, rising.
        const float error = ascend_speed - vy;                 // E at 0085253D
        float ramp = std::fabs(error) / rates.accel_824;        // B at 00852561
        if (ramp > dt) {
            ramp = dt;                                          // 00852573
        }
        float end_velocity;
        if (ascend_speed > vy) {
            end_velocity = rates.accel_824 * ramp + vy;         // 0085258B..00852593
        } else {
            end_velocity = vy - rates.accel_824 * ramp;         // 0085259B..008525A5
        }
        const float y1 = y + (vy + end_velocity) * kHalf * ramp;  // E at 008525C3
        const float rest = dt - ramp;                           // dt slot at 008525D3
        const float y2 = ascend_speed * rest + y1;              // A at 008525E1
        const float high = target + kPredictBandTolerance;      // 008525E5
        result = (y2 > high) ? high : y2;                       // 008525EB..008525F3
    } else {
        const float high = target + kPredictBandTolerance;      // 008525FE
        if (y > high) {
            // 0085260C..008526CD: above the band, sinking.
            const float error = descend_speed - vy;             // F at 00852622
            float ramp = std::fabs(error) / rates.accel_824;     // C at 0085264B
            if (ramp > dt) {
                ramp = dt;                                       // 0085265D
            }
            float end_velocity;
            if (descend_speed > vy) {
                end_velocity = rates.accel_824 * ramp + vy;      // 00852675..0085267B
            } else {
                end_velocity = vy - rates.accel_824 * ramp;      // 0085267F..00852685
            }
            const float y1 = y + (vy + end_velocity) * kHalf * ramp;  // F at 008526A1
            const float rest = dt - ramp;                        // dt slot at 008526AB
            const float y2 = descend_speed * rest + y1;          // A at 008526B9
            result = (low > y2) ? low : y2;                      // 008526C3..008526C9
        }
    }
    // 008526D3..00852739: the clearance wins outright; otherwise -2.5 caps it.
    if (unit.clearance_123c > result) {
        return unit.clearance_123c;
    }
    if (result > kShallowestSwimDepth) {
        return kShallowestSwimDepth;
    }
    return result;
}

// 00852740..00852791.
bool submarine_is_below_band1_for_kind0e_00852740(const SubmarineDepthFields& unit,
                                                  bool has_argument,
                                                  std::int32_t argument_kind_8) noexcept {
    if (!has_argument || argument_kind_8 != 0x0E) {
        return false;
    }
    const float limit = unit.bands_1200[1] + kPredictBandTolerance;  // FSTP at 00852771
    return limit > unit.world_y_100;                                 // 00852779 FCOMPI, JBE
}

// 008527A0..008527DA.
bool submarine_is_above_deck_gun_depth_008527a0(const SubmarineDepthFields& unit) noexcept {
    const float limit = unit.bands_1200[0] - kThree;  // FSTP at 008527C5
    return unit.world_y_100 > limit;                 // 008527CD FCOMPI, JBE
}

// 00852820..0085285A. No intermediate store; at 24-bit precision the sum and
// the quotient round like floats.
bool submarine_is_above_depth_charge_depth_00852820(const SubmarineDepthFields& unit) noexcept {
    const float limit = (unit.bands_1200[1] + unit.bands_1200[0]) / kThree;
    return unit.world_y_100 > limit;
}

// 00853E2D..0085420A. 00CE3800 (0.5), 00D06888 (-0.3), 00CE69C8 (+0.3),
// 00CF8850 (double 250.0). The y component of the direction is not scaled or
// used: the point's y is band 1 (00854168..00854190).
bool submarine_activate_00853e10(const SubmarineActivateInputs& in, const SubmarineActivateHost& host) {
    if (in.creation_kind == 3) {
        host.read_saved_submarine_table();
    } else if (in.creation_kind == 1 && in.auto_attack_target >= 0) {
        host.issue_attackmove(in.auto_attack_target);
    }
    if (!in.network_client && in.dummy_object_id_6b8 >= 0) {
        const float r = host.uniform(1, 0.0f, 1.0f);
        const float delta = 0.5f > r ? -0.30000001192092896f : 0.30000001192092896f;
        const float heading = host.add_wrapped_angle(in.heading, delta);
        const Vec3 dir = host.heading_to_direction_xz(heading);
        const float ox = dir.x * 250.0f;
        const float oz = 250.0f * dir.z;
        host.issue_moveto(Vec3{ox + in.world_position.x, in.band1_1204, in.world_position.z + oz});
    }
    return in.has_catapult_child;
}

std::int32_t submarine_depth_step_shallower_00852b50(std::int32_t depth_level_1268) noexcept {
    return depth_level_1268 > 0 ? depth_level_1268 - 1 : -1;  // 00852B58 JLE
}

std::int32_t submarine_depth_step_deeper_00852b70(std::int32_t depth_level_1268) noexcept {
    return depth_level_1268 < 3 ? depth_level_1268 + 1 : -1;  // 00852B79 JGE
}

// ---------------------------------------------------------------------------
// 00852970. 00CE3DE0 (double 2.5) is the fall rate, 00CF1748 (double
// 0.449999988) and 00D0BEB0 (double 0.509999990) the per-second decays, 00D7A23C
// (float 0.001) the rotation threshold (FCOMI, JBE). Each decay is
// v - (v * c) * dt; for z the product is c * vz.
// ---------------------------------------------------------------------------
void periscope_debris_step_00852970(PeriscopeDebris& d, float dt, const PeriscopeDebrisHost& host) {
    constexpr float kFallRate = 2.5f;
    constexpr float kLinearDecay = 0.44999998807907104f;
    constexpr float kAngularDecay = 0.5099999904632568f;
    constexpr float kMinAngle = 0.0010000000474974513f;
    if (d.state_10 == 0) {
        host.capture_node_matrix();                 // 0085297D..00852997
        d.velocity_14 = host.owner_velocity();      // 0085299C..008529B9
    }
    if (d.state_10 != 1) {
        return;
    }
    const float dx = d.velocity_14.x * dt;
    const float dy = d.velocity_14.y * dt;
    const float dz = dt * d.velocity_14.z;
    d.position_5c.x = d.position_5c.x + dx;
    d.position_5c.y = d.position_5c.y + dy;
    d.position_5c.z = dz + d.position_5c.z;
    const Vec3 axis = d.angular_velocity_20;
    const float angle = host.length(axis) * dt;     // FSTP 00852A37
    if (angle > kMinAngle) {
        host.rotate_basis(axis, angle);             // 00852A50..00852A5E
    }
    host.apply_matrix_to_node();                    // 00852A67..00852A73
    d.velocity_14.y = d.velocity_14.y - kFallRate * dt;  // 00852A75..00852A88
    const float lx = d.velocity_14.x * kLinearDecay;
    const float ly = d.velocity_14.y * kLinearDecay;
    const float lz = kLinearDecay * d.velocity_14.z;
    d.velocity_14.x = d.velocity_14.x - lx * dt;
    d.velocity_14.y = d.velocity_14.y - ly * dt;
    d.velocity_14.z = d.velocity_14.z - lz * dt;
    const float ax = d.angular_velocity_20.x * kAngularDecay;
    const float ay = d.angular_velocity_20.y * kAngularDecay;
    const float az = kAngularDecay * d.angular_velocity_20.z;
    d.angular_velocity_20.x = d.angular_velocity_20.x - ax * dt;
    d.angular_velocity_20.y = d.angular_velocity_20.y - ay * dt;
    d.angular_velocity_20.z = d.angular_velocity_20.z - dt * az;
}

// ---------------------------------------------------------------------------
// 00855A90. 00855A95 base finalise 006E9890 (AL kept in BL); +60h = E4h * 54h
// * 0.6; +ECh = sqrt(float(2 * E0h * 9.81)); the result is BL && +ECh > 0.0
// (COMISS against 00D7A218, JBE).
// ---------------------------------------------------------------------------
TorpedoClassDerived torpedo_class_derive_00855a90(const TorpedoClassFields& cls,
                                                  bool base_finalise_ok) noexcept {
    TorpedoClassDerived out;
    out.cached_range_60 = cls.water_travel_speed_e4 * cls.fly_time_54 * kTorpedoRangeFactor;
    const float twice_fall = cls.max_fall_e0 + cls.max_fall_e0;       // 00855AB4
    const float squared_speed = twice_fall * kGravity;                // FSTP 00855ABC
    out.terminal_fall_speed_ec = std::sqrt(squared_speed);            // 00BF7030, FSTP 00855AC9
    out.ok = base_finalise_ok && out.terminal_fall_speed_ec > 0.0f;
    return out;
}

// ---------------------------------------------------------------------------
// 00856050. After 006E2670 and the seven vtable stores: +4E8h..+4F0h, +4F8h,
// +500h..+508h, +510h = 0; +4F4h = +50Ch = 1; +514h = 1.0; +518h =
// -00BD2F10(0, 1) with ECX = 1 (stream 1); +458h = +49Ch = +468h = 0; +488h =
// +498h = 0; +C4h = 2Bh; +47Ch = -2.5; +3DCh = this; +48Ch..+494h = 1.0.
// ---------------------------------------------------------------------------
TorpedoState torpedo_construct_00856050(
    const std::function<float(int stream, float lo, float hi)>& uniform) {
    TorpedoState s;
    s.off_scan_interval_514 = 1.0f;
    s.off_scan_countdown_518 = -uniform(1, 0.0f, 1.0f);  // 00856132..0085613C
    s.flying_458 = false;
    s.launched_in_water_49c = false;
    s.field_488 = 0.0f;
    s.run_time_498 = 0.0f;
    s.class_id_c4 = 0x2B;
    s.swim_depth_47c = kShallowestSwimDepth;
    s.flag_468 = false;
    s.start_position_48c = Vec3{1.0f, 1.0f, 1.0f};
    s.target_entity_4f8 = 0;
    return s;
}

// ---------------------------------------------------------------------------
// 00856420. After operator new(51Ch), memset and 00856050(a1): +54h = a2,
// +314h = class, vtable[98h](0, world+19CCh, identity); velocity +318h =
// direction * float(class+50h); the +74h matrix's +20h row = direction, then
// 0085DC80; +A4h, shot+60h and shot+6Ch = position; +C8h = +10Ch = 0; 0042ED50
// over the children. a7 null: heading 10000 and the target link cleared
// (swimdepth keeps the constructor's -2.5). a7 set: heading = a7+4h, link =
// a7+20h, then swimdepth = a7+8h (00856673). 00414DB0 follows when +C8h is
// clear, which it is.
// ---------------------------------------------------------------------------
TorpedoCreateResult torpedo_create_00856420(const TorpedoClassFields& cls,
                                            const TorpedoState& constructed,
                                            const Vec3& position,
                                            const Vec3& direction,
                                            const TorpedoLaunchParams* params,
                                            const TorpedoCreateHost& host) {
    TorpedoCreateResult out;
    if (host.attach_identity_pose) {
        host.attach_identity_pose();
    }
    const float v0 = cls.muzzle_speed_50;  // FSTP at 0085653F
    out.velocity_318 = Vec3{direction.x * v0, v0 * direction.y, v0 * direction.z};
    out.forward_94 = direction;
    if (host.orthonormalise_pose) {
        host.orthonormalise_pose();
    }
    out.position_a4 = position;
    if (host.invalidate_child_poses) {
        host.invalidate_child_poses();
    }
    if (params == nullptr) {
        out.heading_46c = kHeadingUnset;                 // 00856614
        out.target_entity_4f8 = 0;                       // 0085662B..00856632
        out.swim_depth_47c = constructed.swim_depth_47c;
    } else {
        out.heading_46c = params->heading_4;             // 00856637
        out.target_entity_4f8 = params->target_entity_20;
        out.swim_depth_47c = params->swim_depth_8;       // 00856673
    }
    return out;
}

// ---------------------------------------------------------------------------
// 00857780. 006E2E10; +48Ch = position; when the position is not inside
// |x|,|y|,|z| < 50000 (00855C20) with |y| < 1700, 006E2E10 runs a second
// time. dragfront/dragside defaults, accel = float(E4h) * 0.59999943,
// depthsmoothrate = waverate = 15, +4A0h = 0. Shot not in water ->
// SetWeaponScale(0.25), else +49Ch = 1. Creation kind 3 reads the saved
// "_torpedo" table, kind 2 takes the message's swimdepth. The "bubor" point
// group's first point goes to +33Ch. +48Ch is refreshed from the pose last.
// ---------------------------------------------------------------------------
void torpedo_activate_00857780(const TorpedoActivateInputs& in, TorpedoState& state,
                               const TorpedoActivateHost& host) {
    if (host.base_activate) {
        host.base_activate();
    }
    state.start_position_48c = in.world_position;
    const Vec3& p = state.start_position_48c;
    const bool inside_box = std::fabs(p.x) < kActivateMaxAbsCoordinate &&
                            std::fabs(p.y) < kActivateMaxAbsCoordinate &&
                            std::fabs(p.z) < kActivateMaxAbsCoordinate;
    const bool below_ceiling = kActivateMaxAbsHeight > std::fabs(p.y);
    if (!(inside_box && below_ceiling) && host.base_activate) {
        host.base_activate();  // 00857822
    }
    const float travel_speed = in.water_travel_speed_e4;  // FSTP at 00857842
    state.drag_front_474 = kDefaultDragFront;
    state.drag_side_478 = kDefaultDragSide;
    state.accel_470 = static_cast<float>(travel_speed * kAccelPerTravelSpeed);
    state.depth_smooth_rate_480 = kDefaultSmoothAndWaveRate;
    state.wave_rate_484 = kDefaultSmoothAndWaveRate;
    state.propeller_node_4a0 = 0;
    if (!in.shot_in_water_354) {
        if (host.set_weapon_scale) {
            host.set_weapon_scale(kAirWeaponScale);  // 008578AC
        }
        state.shot_weapon_scale_3e0 = kAirWeaponScale;
    } else {
        state.launched_in_water_49c = true;          // 008578B0
    }
    if (in.creation_kind == 3) {
        if (host.read_saved_torpedo_table) {
            host.read_saved_torpedo_table(state);    // 008578DA..00857ACA
        }
    } else if (in.creation_kind == 2) {
        state.swim_depth_47c = in.message_swim_depth_124;  // 00857AD3
    }
    if (in.has_bubble_point) {
        state.bubble_point_33c = in.bubble_point;    // 00857B9B..00857BB2
    }
    state.start_position_48c = in.world_position;    // 00857BC8..00857BFD
}

// ---------------------------------------------------------------------------
// 008568E0. The break-up test reads the float speed |shot+8h| (FSTP 00856908)
// against MaxWaterHitVel (JA), then -terminal fall speed (+ECh, FCHS) against
// the vertical velocity shot+0Ch (JA). Both JA arms are false when unordered.
// The tail 00856B56 runs on every path but the in-water early exit: a heading
// still exactly 10000 (UCOMISS, LAHF/TEST AH,44h/JP) takes the pose's yaw,
// stored to shot+15Ch at 00856B9E.
// ---------------------------------------------------------------------------
void torpedo_enter_water_008568e0(const TorpedoClassFields& cls,
                                  float terminal_fall_speed_ec,
                                  const TorpedoWaterEntryInputs& in,
                                  float& heading_46c,
                                  const TorpedoWaterEntryHost& host) {
    if (in.in_water_44) {
        return;  // 008568ED
    }
    if (host.set_weapon_scale) {
        host.set_weapon_scale(1.0f);  // 008568FE
    }
    const float speed = host.length ? host.length(in.velocity_318)
                                    : std::sqrt(in.velocity_318.x * in.velocity_318.x +
                                                in.velocity_318.y * in.velocity_318.y +
                                                in.velocity_318.z * in.velocity_318.z);
    const bool too_fast = speed > cls.max_water_hit_vel_dc;
    const bool too_steep = -terminal_fall_speed_ec > in.velocity_318.y;
    if (too_fast || too_steep) {
        // 00856A09..00856B51.
        if (cls.has_expl_water_efx_24 && host.spawn_breakup_effect_and_snap_pose) {
            host.spawn_breakup_effect_and_snap_pose();
        }
        if (in.network_client) {
            if (host.enter_water) {
                host.enter_water(in.spawn_effect_arg);  // 00856B4A..00856B51
            }
        } else if (host.kill_entity) {
            host.kill_entity(true);  // 00856B3C..00856B43: PUSH EBX (EBX = 1 from 00856A16)
        }
    } else {
        // 00856938..00856A04: 006E6450(arg), then the splash when arg and
        // WaterSplashEfx are both set.
        if (host.enter_water) {
            host.enter_water(in.spawn_effect_arg);  // 0085693F
        }
        if (in.spawn_effect_arg && cls.has_water_splash_efx_d8 && host.spawn_splash_effect) {
            host.spawn_splash_effect();
        }
    }
    if (heading_46c == kHeadingUnset && host.current_yaw) {
        heading_46c = host.current_yaw();  // 00856B93..00856B9E
    }
}

// 00855CC0: 006E0D20(B8h, a1, entity), vtable 00D037BC, +120h = heading,
// +124h = swimdepth, +128h = target+174h or 0.
TorpedoCreateMessage torpedo_create_message_00855cc0(const TorpedoState& state,
                                                     bool has_target,
                                                     std::uint16_t target_net_id_174) noexcept {
    TorpedoCreateMessage msg;
    msg.heading_120 = state.heading_46c;
    msg.swim_depth_124 = state.swim_depth_47c;
    msg.target_net_id_128 = has_target ? target_net_id_174 : 0;
    return msg;
}

// 00857160: heading = +120h; the link +4E4h is re-pointed (006952A0 / 00694A60)
// at the entity the net id resolves to through 00F89A0C/00F89A60, or null for
// id 0.
void torpedo_create_message_apply_00857160(
    const TorpedoCreateMessage& msg, TorpedoState& state,
    const std::function<std::uint32_t(std::uint16_t)>& entity_by_net_id) {
    state.heading_46c = msg.heading_120;
    std::uint32_t target = 0;
    if (msg.target_net_id_128 != 0 && entity_by_net_id) {
        target = entity_by_net_id(msg.target_net_id_128);
    }
    state.target_entity_4f8 = target;
}

// 0085721C..0085744D. The depth test is FLD y; FLD swimdepth; FADD ST0,ST0;
// FCOMPI; JBE: 2 * swimdepth > y takes the retire loop.
TorpedoWakeResult torpedo_wake_step_008571f0(const TorpedoWakeInputs& in, const TorpedoWakeHost& host) {
    TorpedoWakeResult out;
    out.run_time_498 = in.dt + in.run_time_498;  // 00857220..00857235
    if (!in.water_mode_nonzero || !in.has_emitter_334) {
        return out;                              // 0085723F, 00857254
    }
    host.place_emitter();                        // 0085725A..00857290
    host.set_emitter_rate(host.speed());         // 00857295..008572AC
    host.set_emitter_direction();                // 008572BF..008572EB
    out.action = TorpedoWakeAction::Placed;
    if (!(in.homing_vert_turn_speed_f4 > 0.0f)) {
        return out;                              // 00857300 COMISS, JBE
    }
    const float twice = in.swim_depth_47c + in.swim_depth_47c;
    if (twice > in.world_y_100) {
        out.action = TorpedoWakeAction::RetireType4;  // 00857340..008573F2
    } else {
        out.action = in.has_type4_child ? TorpedoWakeAction::KeepType4
                                        : TorpedoWakeAction::StopEmitter;  // 0085740A..00857448
    }
    return out;
}

// 00856C50..00856D6B. 004193E0 is x*x + y*y + z*z; the incumbent's value is
// stored to a float (00856D2D) and the candidate replaces it when the
// incumbent's is strictly greater (FCOMPI, JBE).
int torpedo_homing_pick_target_00856c50(const Vec3& own_position,
                                        const std::vector<TorpedoHomingCandidate>& list) noexcept {
    int best = -1;
    for (int i = 0; i < static_cast<int>(list.size()); ++i) {
        const TorpedoHomingCandidate& c = list[static_cast<std::size_t>(i)];
        if (!c.is_submarine) {
            continue;
        }
        if (best < 0) {
            best = i;  // 00856C74 -> 00856D40
            continue;
        }
        const Vec3& b = list[static_cast<std::size_t>(best)].position;
        const float bx = b.x - own_position.x;
        const float by = b.y - own_position.y;
        const float bz = b.z - own_position.z;
        const float cx = c.position.x - own_position.x;
        const float cy = c.position.y - own_position.y;
        const float cz = c.position.z - own_position.z;
        const float best_d2 = bx * bx + by * by + bz * bz;
        const float cand_d2 = cx * cx + cy * cy + cz * cz;
        if (best_d2 > cand_d2) {
            best = i;
        }
    }
    return best;
}

// 00856BBA..00856BF6 and 00856F5F.
TorpedoHomingResult torpedo_homing_countdown_00856bb0(const TorpedoHomingInputs& in) noexcept {
    TorpedoHomingResult out;
    out.scan_countdown_518 = in.scan_countdown_518;
    if (!(in.homing_horz_turn_speed_f0 > 0.0f)) {
        return out;  // 00856BD0 JBE -> 00856F81
    }
    out.active = true;
    const float countdown = in.scan_countdown_518;
    if (!(in.dt >= countdown)) {  // FCOMI, JB (unordered takes it)
        out.scan_countdown_518 = countdown - in.dt;  // FSUBP
        return out;
    }
    out.scan_countdown_518 = countdown + (in.scan_interval_514 - in.dt);  // FSUBR, FADDP
    out.scan = true;
    return out;
}

namespace {
// 00856E8D..00856ECD: limit = float(speed * dt); clamp to [-limit, limit].
float homing_clamp(float error, float speed, float dt) {
    const float limit = speed * dt;
    if (error > limit) {
        error = limit;
    }
    const float negative = -limit;
    if (negative > error) {
        error = negative;
    }
    return error;
}
}  // namespace

// 00856D73..00856F7F.
TorpedoHomingResult torpedo_homing_turn_00856bb0(const TorpedoHomingInputs& in,
                                                 TorpedoHomingResult out,
                                                 const std::function<float(float, float)>& wrap_subtract) {
    if (!out.active || !in.has_target) {
        return out;  // 00856D7A
    }
    out.steered = true;
    out.commanded_heading_46c = in.target_yaw;  // 00856E25
    if (in.homing_vert_turn_speed_f4 > 0.0f && in.target_is_submarine &&
        !in.target_above_depth_charge_depth) {
        const float pitch_error = wrap_subtract(in.own_pitch, in.target_pitch);   // 00856E7E
        out.right_axis_turn = homing_clamp(pitch_error, in.homing_vert_turn_speed_f4, in.dt);
        const float yaw_error = -wrap_subtract(in.own_yaw, in.target_yaw);        // 00856EFA, FCHS
        out.up_axis_turn = homing_clamp(yaw_error, in.homing_horz_turn_speed_f0, in.dt);
        out.homed = true;
        return out;
    }
    out.right_axis_turn = in.own_pitch;  // 00856F6C
    return out;
}

// 00855B59..00855B7D: FLD dt, FMUL qword 00D0C318, FSTP float.
float torpedo_propeller_angle_00855b30(float dt) noexcept {
    return static_cast<float>(dt * kPropellerRate);
}

// 00857C83..00857CBE.
TorpedoBoatTurboState torpedo_boat_sentity_init_00857c80(float class_turbo_time_808,
                                                         bool world_flag_19d0,
                                                         std::uint32_t director_flags_1cc) noexcept {
    TorpedoBoatTurboState s;
    if (world_flag_19d0) {
        s.send_director_flags = true;
        s.director_flags = director_flags_1cc & 0xFFFFFFFBu;
    }
    s.turbo_time_1188 = class_turbo_time_808;
    s.turbo_flag_118c = false;
    return s;
}

// 00852310..0085233B: both tests must answer AL != 0.
int unit_count_devices25_ready_00852300(const std::vector<UnitChildDevice>& children) noexcept {
    int n = 0;
    for (const UnitChildDevice& c : children) {
        if (c.is_kind_25 && c.answers_210_2d) {
            ++n;
        }
    }
    return n;
}

// 00852360..00852377.
int unit_count_bomb_platform_children_00852350(const std::vector<UnitChildDevice>& children) noexcept {
    int n = 0;
    for (const UnitChildDevice& c : children) {
        if (c.is_kind_25) {
            ++n;
        }
    }
    return n;
}

bool torpedo_boat_is_body_speed_above_3_00857d40(float body_speed) noexcept {
    return body_speed > 3.0f;
}

// ---------------------------------------------------------------------------
// MWaterMine
// ---------------------------------------------------------------------------
namespace {
constexpr float kMineSinkGravity = -9.81f;       // 00CE65E0 (double == (double)-9.81f)
constexpr float kQuarter = 0.25f;                // 00D7A348
constexpr float kMineArmSpeedSquared = 0.0625f;  // 00CE6450
constexpr float kMineShipPrefilterSquared = 22500.0f;  // 00CFE050 (150 m)
}  // namespace

// 0085F528..0085F6A2: +D8h, +DCh, +E0h (handle), +E4h, +E8h.
void water_mine_read_class_fields_0085f500(const WaterMineClassRow& row,
                                           WaterMineClassFields& cls) {
    cls.v0_random_factor_d8 = row.number("V0RandomFactor");
    cls.dive_speed_dc = row.number("DiveSpeed");
    cls.has_water_splash_efx_e0 = row.effect_present("WaterSplashEfx");
    cls.floating_depth_e4 = row.number_or("FloatingDepth", -3.0f);  // 00CE3D50
    cls.trigger_radius_e8 = row.number_or("TriggerRadius", 5.0f);   // 00CE3850
}

// 0085F6D1..0085F79C.
bool water_mine_enter_water_0085f6d0(const WaterMineClassFields& cls, bool in_water_44,
                                     const WaterMineEnterWaterHost& host) {
    host.set_weapon_scale(1.0f);
    if (in_water_44) {
        return false;
    }
    host.set_water_mode();
    if (cls.has_water_splash_efx_e0) {
        host.spawn_splash_effect();
    }
    return true;
}

// 0085EC01..0085EC93: hi = float(r + 1), lo = float(1 - r), draw, then
// (vx * s, vy * s, s * vz).
Vec3 water_mine_activate_0085ebc0(const WaterMineClassFields& cls, const Vec3& velocity,
                                  bool shot_in_water, bool network_client,
                                  const WaterMineActivateHost& host) {
    if (host.base_activate) {
        host.base_activate();
    }
    if (!shot_in_water && host.set_weapon_scale) {
        host.set_weapon_scale(kAirWeaponScale);
    }
    Vec3 v = velocity;
    if (!network_client) {
        const float r = cls.v0_random_factor_d8;
        const float hi = r + 1.0f;
        const float lo = 1.0f - r;
        const float s = host.uniform(1, lo, hi);
        v = Vec3{velocity.x * s, velocity.y * s, s * velocity.z};
    }
    if (host.shot_slot34_with_zero_vectors) {
        host.shot_slot34_with_zero_vectors();
    }
    return v;
}

// 0085ECF9..0085EDF2 then 0085EDF8..0085F11B. The squared lengths are
// (y*y + x*x) + z*z, the order the FMULP/FADDP chain at 0085EE50 and
// 0085EF25 uses.
WaterMineStepResult water_mine_step_0085ecd0(const WaterMineClassFields& cls,
                                             const WaterMineStepInputs& in) noexcept {
    WaterMineStepResult out;
    const Vec3& v = in.velocity_318;
    const float k = kMineSinkGravity / cls.dive_speed_dc;  // FDIVR, FSTP 0085ED05
    const float ax = v.x * k;
    const float ay = v.y * k;
    const float az = k * v.z;
    const float offset = in.sim_y_a8 - cls.floating_depth_e4;            // 0085ED86
    const float spring = (offset - in.water_height * kQuarter) * kQuarter;  // 0085ED9A..0085ED9C
    const float ay2 = ay - spring;                                       // FSTP 0085EDA0
    const float dvx = ax * in.dt;
    const float dvy = ay2 * in.dt;
    const float dvz = in.dt * az;
    out.velocity_318 = Vec3{v.x + dvx, v.y + dvy, v.z + dvz};
    if (in.network_client) {
        return out;  // 0085EE05
    }
    if (!in.armed_46c) {
        const float arm_ceiling = cls.floating_depth_e4 + 1.0f;  // FSTP 0085EE30
        if (arm_ceiling > in.sim_y_a8) {
            const Vec3& nv = out.velocity_318;
            const float s2 = (nv.y * nv.y + nv.x * nv.x) + nv.z * nv.z;
            if (kMineArmSpeedSquared > s2) {
                out.send_arm_message = true;  // 0085EE74..0085EE8C
            }
        }
        return out;  // +46Ch still clear: 0085EEA0
    }
    const float radius = cls.trigger_radius_e8;      // FSTP 0085EEB5
    const float radius_squared = radius * radius;    // FSTP 0085EEC6
    for (const WaterMineShip& ship : in.ships) {
        const float dx = in.world_position.x - ship.world_position.x;
        const float dy = in.world_position.y - ship.world_position.y;
        const float dz = in.world_position.z - ship.world_position.z;
        const float d2 = (dy * dy + dx * dx) + dz * dz;
        if (d2 > kMineShipPrefilterSquared) {
            continue;  // 0085EF47 JA
        }
        const float ex = std::fabs(ship.mine_in_ship.x) - ship.width_a4 * kHalf;
        const float ey = std::fabs(ship.mine_in_ship.y) - ship.height_a8 * kHalf;
        const float ez = std::fabs(ship.mine_in_ship.z) - ship.length_a0 * kHalf;
        if (radius > ex && radius > ey && radius > ez) {
            out.detonate = true;  // 0085F04B
            return out;
        }
    }
    for (const Vec3& charge : in.depth_charge_positions) {
        const float dx = in.world_position.x - charge.x;
        const float dy = in.world_position.y - charge.y;
        const float dz = in.world_position.z - charge.z;
        const float d2 = (dy * dy + dx * dx) + dz * dz;  // FSTP 0085F0E8
        if (radius_squared > d2) {
            out.detonate = true;  // 0085F0F8 JA
            return out;
        }
    }
    return out;
}

bool water_mine_hit_detonates_0085f160(bool armed_46c, bool has_source,
                                       bool source_is_water_mine) noexcept {
    if (!armed_46c) {
        return false;
    }
    return !(has_source && source_is_water_mine);
}

// 0085F7A6..0085F8CC.
void water_mine_on_killed_0085f7a0(const WaterMineClassFields& cls, std::int32_t kill_cause_70,
                                   const WaterMineKilledHost& host) {
    if (kill_cause_70 == 1) {
        // 0085F7BA..0085F7D5: (min, max) on stream 1.
        const float damage = host.uniform(1, cls.blast_damage_min_b4, cls.blast_damage_max_b8);
        if (host.apply_radial_damage) {
            host.apply_radial_damage(damage);
        }
        if (host.spawn_explosion_at_water_surface) {
            host.spawn_explosion_at_water_surface();
        }
    }
    if (host.base_on_killed) {
        host.base_on_killed();
    }
}

// ---------------------------------------------------------------------------
// MSubmarine lifecycle (cc13-gpr53d)
// ---------------------------------------------------------------------------

SubmarineConstructDefaults submarine_construct_defaults_00852f10() noexcept {
    // 00852F6D..00852FA0, in listing order: +1228h = 0, +127Ch = 1.0f,
    // +1280h/+1281h/+1235h = 0, +1238h = 0, +C4h = 8, +1274h = 0.0f.
    return SubmarineConstructDefaults{};
}

SubmarineCreateInstanceResult submarine_create_instance_008531a0(std::int32_t flag,
                                                                 const SubmarineCreateInstanceHost& host) {
    SubmarineCreateInstanceResult result;
    void* const block = host.allocate ? host.allocate(kSubmarineInstanceSize) : nullptr;
    if (host.zero) {
        host.zero(block, kSubmarineInstanceSize);   // 008531CE, on the raw return
    }
    if (block != nullptr && host.construct) {
        result.unit = host.construct(block, flag);  // 008531ED
    }
    if (result.unit == nullptr) {
        // 008531F6 XOR ESI,ESI falls through to 00853203 and 0085320C.
        result.null_unit_dereferenced = true;
        return result;
    }
    if (host.set_vehicle_class) {
        host.set_vehicle_class(result.unit);        // 00853203
    }
    if (host.store_descriptor_354) {
        host.store_descriptor_354(result.unit);     // 0085320C
    }
    return result;
}

std::vector<StateSinkEmit> submarine_describe_state_00852d30(const SubmarineSavedState& s) {
    using Op = StateSinkEmit::Op;
    auto open = [](const char* key) {
        StateSinkEmit e;
        e.op = Op::open_table;
        e.key = key;
        return e;
    };
    auto close = [] {
        StateSinkEmit e;
        e.op = Op::close_table;
        return e;
    };
    auto real = [](const char* key, float v) {
        StateSinkEmit e;
        e.key = key;
        e.type = StateSinkType::real;
        e.real = v;
        return e;
    };
    auto boolean = [](const char* key, bool v) {
        StateSinkEmit e;
        e.key = key;
        e.type = StateSinkType::boolean;
        e.boolean = v;
        return e;
    };
    auto integer = [](const char* key, std::int32_t v) {
        StateSinkEmit e;
        e.key = key;
        e.type = StateSinkType::integer;
        e.integer = v;
        return e;
    };
    // Keys: 00D0BF28, 00D0BF1C, 00D0BF18, 00D0BF10, 00D0BF00, then 00D0BEF4.. .
    return {
        open("_submarine"),
        open("airSupply"),
        real("air", s.air_127c),
        boolean("needAir", s.need_air_1281),
        boolean("unlimitedAir", s.unlimited_air_1280),
        close(),
        integer("depthLevel", s.depth_level_1268),
        integer("periscopeState", s.periscope_state_122c),
        real("periscopeY", s.periscope_y_1230),
        boolean("periscopeOut", s.periscope_out_1234),
        real("sullyesztoEro", s.submerge_force_1270),
        close(),
    };
}

namespace {

void register_in_world_lists(const WorldListRegistration& row, const WorldListRegisterHost& host) {
    if (host.register_in_parent_entity_list) {
        host.register_in_parent_entity_list();
    }
    for (const std::uint32_t offset : row.list_offsets) {
        if (host.push_back) {
            host.push_back(offset);
        }
    }
}

int detach_from_class_world_list(const void* entity, const std::vector<const void*>& payloads,
                                 const WorldListDetachHost& host) {
    if (host.base_detach) {
        host.base_detach();
    }
    // CMP [node+8],ESI / JE erase; next = [node+4] until null.
    for (std::size_t i = 0; i < payloads.size(); ++i) {
        if (payloads[i] != entity) continue;
        if (host.erase_node) {
            host.erase_node(i);
        }
        return static_cast<int>(i);
    }
    return -1;
}

}  // namespace

void torpedo_register_in_world_lists_00856360(const WorldListRegisterHost& host) {
    register_in_world_lists(kTorpedoRegistration_00856360, host);   // 00856363..0085638D
}

void water_mine_register_in_world_lists_0085f270(const WorldListRegisterHost& host) {
    register_in_world_lists(kWaterMineRegistration_0085f270, host); // 0085F273..0085F29D
}

std::vector<StateSinkEmit> torpedo_write_state_00855d30(const TorpedoSavedState& s) {
    auto field = [](const char* key, float v) {
        StateSinkEmit e;
        e.key = key;
        e.type = StateSinkType::real;
        e.real = v;
        return e;
    };
    StateSinkEmit open;
    open.op = StateSinkEmit::Op::open_table;
    open.key = "_torpedo";
    StateSinkEmit close;
    close.op = StateSinkEmit::Op::close_table;
    return {
        open,
        field("heading", s.heading_46c),
        field("swimdepth", s.swim_depth_47c),
        field("accel", s.accel_470),
        field("dragfront", s.drag_front_474),
        field("dragside", s.drag_side_478),
        field("swimdepth", s.swim_depth_47c),   // 00855E58: the repeat
        field("depthsmoothrate", s.depth_smooth_rate_480),
        field("waverate", s.wave_rate_484),
        close,
    };
}

WaterMineCreateResult water_mine_create_0085f310(float class_v0_50, std::uint32_t arg2,
                                                 const Vec3& position, const Vec3& direction,
                                                 const TorpedoCreateHost& host) {
    WaterMineCreateResult r;
    r.arg2_54 = arg2;
    if (host.attach_identity_pose) {
        host.attach_identity_pose();                // 0085F408
    }
    // 0085F410..0085F43A: V0 stored as a float, then dir.x*V0, dir.y*V0, V0*dir.z.
    const float v0 = class_v0_50;
    r.velocity_318.x = direction.x * v0;
    r.velocity_318.y = direction.y * v0;
    r.velocity_318.z = v0 * direction.z;
    r.forward_94 = direction;
    if (host.orthonormalise_pose) {
        host.orthonormalise_pose();                 // 0085F46D -> 0085DC80
    }
    r.position_a4 = position;
    if (host.invalidate_child_poses) {
        host.invalidate_child_poses();              // 0085F4D2 -> 0042ED50
    }
    return r;
}

float update_effect_light_instance_008738b0(EffectLightInstanceState& s, float def_duration_20,
                                            float def_intensity_24, float dt,
                                            const EffectLightUpdateHost& host) {
    if (!s.has_light_1c) return 0.0f;
    if (host.point_node_world_position && host.set_light_position) {
        host.set_light_position(host.point_node_world_position());
    }
    if (host.link_if_unlinked) {
        host.link_if_unlinked();
    }
    // 00873924..0087393C: each step stored as a float.
    const float ratio = s.remaining_20 / def_duration_20;
    const float intensity = ratio * def_intensity_24;
    constexpr float kMinIntensity = 0.01f;              // 00D7A238
    const float written = intensity > kMinIntensity ? intensity : kMinIntensity;
    if (host.set_intensity_1f8) {
        host.set_intensity_1f8(written);
    }
    // 00873968..00873985: FLDZ; FCOMPI; JBE keeps the value.
    const float left = s.remaining_20 - dt;
    s.remaining_20 = 0.0f > left ? 0.0f : left;
    return written;
}

std::int32_t waterdrops_spawn_count_00873100(float distance, float fade_distance_34,
                                             std::int32_t count_44) noexcept {
    float x = distance - fade_distance_34;
    if (!(x > 0.0f)) x = 0.0f;                          // 00873200 JBE
    float f = x / fade_distance_34;
    if (0.0f > f) {
        f = 0.0f;                                       // 0087322A, not taken on NaN
    } else if (f > 1.0f) {
        f = 1.0f;                                       // 00873246 COMISS / JBE
    }
    const float g = 1.0f - f;
    // FILD is exact; the 2^32 FADD rounds at 24-bit precision. One rounding for
    // the FMUL (double product is exact for |count| < 2^29).
    double c = static_cast<double>(count_44);
    if (count_44 < 0) c = static_cast<double>(static_cast<float>(c + 4294967296.0));
    const float p = static_cast<float>(c * static_cast<double>(g));
    // FISTP qword, truncating; an out-of-range or NaN value stores the integer
    // indefinite 8000000000000000h, whose low dword is 0.
    if (!(p > -9.2e18f && p < 9.2e18f)) return 0;
    const auto wide = static_cast<std::int64_t>(p);
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::uint64_t>(wide)));
}

void update_effect_waterdrops_instance_00873100(EffectWaterdropsState& s,
                                                const WaterdropsDefinitionFields& def, float dt,
                                                const WaterdropsUpdateHost& host) {
    if (!(host.enabled_f889fc && host.enabled_f889fc())) {
        s.pending_20 = false;                           // 00873112
    }
    if (s.pending_20) {
        const Vec3 reference = host.reference_node_position ? host.reference_node_position() : Vec3{};
        const Vec3 point = host.point_node_position ? host.point_node_position() : Vec3{};
        const Vec3 d{point.x - reference.x, point.y - reference.y, point.z - reference.z};
        const float len = host.length ? host.length(d) : 0.0f;
        const std::int32_t count = waterdrops_spawn_count_00873100(len, def.fade_distance_34, def.count_44);
        if (count != 0 && host.create_and_spawn) {
            s.drops_1c = host.create_and_spawn(def, count);
        }
        s.pending_20 = false;                           // 0087335F
    }
    if (s.drops_1c == 0) return;
    Vec3 camera{};
    if (host.camera_position && !host.camera_position(camera)) {
        camera = Vec3{};                                // 008733BA, null camera node
    }
    if (host.update) {
        host.update(s.drops_1c, camera, dt);
    }
}

bool update_effect_thunderstorm_instance_00873c80(const EffectThunderStormState& s, float dt,
                                                  const ThunderStormUpdateHost& host) {
    if (s.has_atlas_1c && host.update_atlas_1c) {
        host.update_atlas_1c(dt);                       // 00873CA9
    }
    if (!s.has_storm_20) return false;
    Vec3 strike{};
    if (!(host.update_storm && host.update_storm(dt, strike))) return false;
    // 00873CDC..00873D61: identity, translation row = strike.
    const float matrix[16] = {1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 1.0f, 0.0f,  strike.x, strike.y, strike.z, 1.0f};
    const std::uintptr_t effect = host.spawn_strike ? host.spawn_strike(matrix) : 0;
    if (effect != 0) {
        if (host.mark_finished) {
            host.mark_finished(effect);                 // 00873DAE
        }
        if (host.release_reference) {
            host.release_reference(effect);             // 00873DC0
        }
    }
    return true;
}

namespace {

void write_effect_prefix(NativeRegisteredEffectPrefixStorage& prefix, void* definition,
                         PointEffectInstanceStorage* point) noexcept {
    prefix.table_00 = 0x00ceb130;
    prefix.references_04.store(1, std::memory_order_relaxed);
    prefix.secondary_table_08 = 0x00d0c8c0;
    prefix.active_0c = 1;
    prefix.borrowed_10 = point;
    prefix.borrowed_14 = definition;
}

}  // namespace

void construct_effect_flare_instance_00873010(effects_cc13::EffectNodeInstanceStorage& instance,
                                              void* definition, PointEffectInstanceStorage* point) noexcept {
    write_effect_prefix(instance.prefix, definition, point);
    instance.prefix.table_00 = 0x00d0dc60;
    instance.prefix.secondary_table_08 = 0x00d0dc5c;
    instance.prefix.type_18 = 5;
    instance.payload_1c = nullptr;
}

void construct_effect_waterdrops_instance_00873090(effects_cc13::EffectNodeInstanceStorage& instance,
                                                   void* definition, PointEffectInstanceStorage* point) noexcept {
    write_effect_prefix(instance.prefix, definition, point);
    instance.suppressed_20 = 1;                                       // 008730B3
    instance.prefix.type_18 = 0;
    instance.prefix.table_00 = 0x00d0dbb8;
    instance.prefix.secondary_table_08 = 0x00d0dbb4;
    instance.payload_1c = nullptr;
}

void construct_effect_splash_instance_008739a0(effects_cc13::EffectSplashInstanceStorage& instance,
                                               void* definition, float def_20, float def_24, float def_28,
                                               PointEffectInstanceStorage* point,
                                               const EffectSplashConstructHost& host) {
    write_effect_prefix(instance.prefix, definition, point);
    instance.prefix.type_18 = 0;
    instance.prefix.table_00 = 0x00d0dc28;
    instance.prefix.secondary_table_08 = 0x00d0dc24;
    instance.ring_1c = nullptr;
    const Vec3 pos = host.point_node_world_position ? host.point_node_world_position() : Vec3{};
    if (host.add_water_ring) {
        instance.ring_1c = host.add_water_ring(pos, def_20, def_24, def_28);  // 00873A6E
    }
}

void construct_effect_thunderstorm_instance_00873eb0(EffectThunderStormInstanceStorage& instance,
                                                     void* definition, bool def_storm_54,
                                                     PointEffectInstanceStorage* point,
                                                     const EffectThunderStormConstructHost& host) {
    write_effect_prefix(instance.prefix, definition, point);
    instance.prefix.type_18 = 0;
    instance.prefix.table_00 = 0x00d0ddd0;
    instance.prefix.secondary_table_08 = 0x00d0ddcc;
    if (host.load_atlas) {
        host.load_atlas();                                            // 00873F51
    }
    instance.sky_effect_1c = host.create_sky_effect ? host.create_sky_effect() : 0;  // 00873F8F
    instance.storm_20 = 0;
    if (def_storm_54 && host.create_storm) {
        instance.storm_20 = host.create_storm();                      // 00873F9E / 00873FC0
    }
}

void destruct_effect_thunderstorm_instance_00873ff0(EffectThunderStormInstanceStorage& instance,
                                                    const EffectThunderStormDestructHost& host) {
    if (instance.storm_20 != 0) {
        if (host.destroy_and_free_storm) {
            host.destroy_and_free_storm(instance.storm_20);           // 00874032 / 00874038
        }
        instance.storm_20 = 0;
    }
    if (host.unload_atlas) {
        host.unload_atlas();
    }
}

void destruct_effect_waterdrops_instance_00872c10(effects_cc13::EffectNodeInstanceStorage& instance,
                                                  effects_cc13::EffectInstanceServices& services) {
    effects_cc13::stop_effect_waterdrops_instance_00872bf0(instance, services);
}

void destruct_effect_light_instance_00872cb0(effects_cc13::EffectNodeInstanceStorage& instance,
                                             effects_cc13::EffectInstanceServices& services) {
    effects_cc13::stop_effect_light_instance_00872d30(instance, services);
}

void destruct_effect_splash_instance_00872d60(const effects_cc13::EffectSplashInstanceStorage& instance,
                                              const EffectSplashDestructHost& host) {
    if (!(host.ocean_present && host.ocean_present())) return;        // 00872D90
    if (instance.ring_1c == nullptr) return;                          // 00872DA4
    if (host.remove_water_ring) {
        host.remove_water_ring(instance.ring_1c);                     // 00872DB5
    }
}

void effect_handle_request_stop_00866b70(EffectHandleFlags& h) noexcept {
    h.finish_when_done_09 = false;
    if (h.active_08) h.stop_requested_0a = true;
}

bool point_effect_retire_child_008679d0(std::vector<PointEffectChildSlot>& entries_0c,
                                        std::vector<std::uintptr_t>& pending_18, std::uintptr_t child,
                                        const PointEffectRetireHost& host) {
    bool found = false;
    for (PointEffectChildSlot& slot : entries_0c) {
        if (slot.child != child) continue;
        if (slot.active_0c) {
            slot.active_0c = false;
            if (host.stop_child) {
                host.stop_child(slot.child);
            }
        }
        pending_18.push_back(slot.child);
        if (host.retain && slot.child != 0) {
            host.retain(slot.child);
        }
        if (slot.child != 0 && host.release) {
            host.release(slot.child);
        }
        slot.child = 0;
        found = true;
        break;
    }
    if (child != 0 && host.release) {
        host.release(child);                                          // the by-value argument
    }
    return found;
}

bool effect_sound_any_sample_flag15_00869e10(const std::vector<EffectSoundSampleView>& samples) noexcept {
    for (const EffectSoundSampleView& s : samples) {
        if (s.flag_15) return true;                                   // 00869E2C
    }
    return false;
}

bool effect_sound_admit_at_position_0086a740(bool always_2c, float distance,
                                             const std::vector<EffectSoundSampleView>& samples) noexcept {
    if (always_2c) return true;                                       // 0086A74A
    for (const EffectSoundSampleView& s : samples) {
        if (s.resource_word08) return true;                           // 0086A7CE
        if (s.flag_15) return true;                                   // 0086A7DB
        const float limit = s.use_max_distance_20
            ? s.max_distance_10
            : static_cast<float>(static_cast<double>(s.distance_0c) * 128.0);  // 0086A7EF
        if (limit > distance) return true;                            // 0086A801 JA
    }
    return false;
}

void world_atmosphere_apply_preset_00865900(RainDescriptorFields& d, const WeatherPresetBlock& preset,
                                            const WorldAtmospherePresetHost& host) {
    if (d.emitter_38 != 0) {
        if (host.release_node) {
            host.release_node(d.emitter_38);                          // 00865911
        }
        d.emitter_38 = 0;
    }
    if (preset.is_rain_60) {
        d.speed_20 = preset.speed_68;
        d.drop_count_0c = static_cast<float>(preset.drop_count_64); // 0086593C CVTSI2SS
        d.min_length_18 = preset.min_length_78;
        d.length_span_1c = preset.max_length_7c - preset.min_length_78;
        d.colour_34 = preset.colour_74;
        d.emitter_38 = host.build_rain_emitter ? host.build_rain_emitter() : 0;  // 0086595B
    }
    if (!(host.camera_present && host.camera_present())) return;
    if (host.set_fog_scalar) {
        host.set_fog_scalar(0x68, preset.fog_d0);                    // 00B84D00
        host.set_fog_scalar(0x78, preset.fog_dc);                    // 00B84D40
        host.set_fog_scalar(0x6c, preset.fog_d4);                    // 00B84D10
        host.set_fog_scalar(0x70, preset.fog_d8);                    // 00B84D20
        host.set_fog_scalar(0x74, preset.fog_e0);                    // 00B84D30
    }
    if (host.set_fog_colour) {
        host.set_fog_colour(preset.fog_colour_80);
    }
    if (host.set_directional_colour) {
        for (int i = 0; i < 4; ++i) {
            host.set_directional_colour(i, preset.directional_colours_90[i]);
        }
    }
    if (host.set_camera_clear_colour) {
        host.set_camera_clear_colour(preset.fog_colour_80);
    }
    if (host.set_cloud_wind) {
        host.set_cloud_wind(preset.cloud_wind_25c);
    }
}

bool rain_descriptor_update_00865ab0(const RainDescriptorFields& d, const RainDescriptorUpdateHost& host) {
    if (d.emitter_38 == 0) return false;
    const Vec3 camera = host.camera_position ? host.camera_position() : Vec3{};
    const float dt = host.frame_dt_21f0 ? host.frame_dt_21f0() : 0.0f;
    if (host.emitter_step) {
        host.emitter_step(dt, camera);                                // 00865B3B
    }
    return true;
}

Vec3 rotate_vector_about_axis_0085c3f0(const Vec3& v, const Vec3& axis, float angle) noexcept {
    const float d = (axis.y * v.y + axis.x * v.x) + axis.z * v.z;     // 0085C422..0085C450
    const Vec3 p{axis.x * d, axis.y * d, d * axis.z};                // 0085C466..0085C478
    const Vec3 r{v.x - p.x, v.y - p.y, v.z - p.z};                    // 0085C484..0085C498
    const float rr = (r.y * r.y + r.x * r.x) + r.z * r.z;             // 0085C49C..0085C4B8
    constexpr float kEpsilon = 1e-4f;                                 // 00D7A268 (double)1e-4f
    if (kEpsilon > rr) return v;                                      // 0085C4CA JBE not taken
    // 004F9B30: c = a x r.
    const Vec3 c{axis.y * r.z - axis.z * r.y, axis.z * r.x - axis.x * r.z,
                 axis.x * r.y - axis.y * r.x};
    const float s = static_cast<float>(std::sin(static_cast<double>(angle)));  // 0085C511 FSIN
    const Vec3 cs{c.x * s, c.y * s, s * c.z};
    const float k = static_cast<float>(std::cos(static_cast<double>(angle)));  // 0085C53F FCOS
    const Vec3 rk{r.x * k, r.y * k, k * r.z};
    const Vec3 sum{rk.x + p.x, rk.y + p.y, rk.z + p.z};               // 0085C569..0085C589
    return Vec3{sum.x + cs.x, sum.y + cs.y, sum.z + cs.z};            // 0085C58D..0085C5AD
}

void rotate_basis_about_axis_0085e880(float out[16], const float in[16], const Vec3& axis,
                                      float angle) {
    const Vec3 row2 = rotate_vector_about_axis_0085c3f0(Vec3{in[8], in[9], in[10]}, axis, angle);
    out[8] = row2.x;
    out[9] = row2.y;
    out[10] = row2.z;
    const Vec3 row1 = rotate_vector_about_axis_0085c3f0(Vec3{in[4], in[5], in[6]}, axis, angle);
    out[4] = row1.x;
    out[5] = row1.y;
    out[6] = row1.z;
    orthonormalize_pose_matrix_0085dc80(out);                         // 0085E8B5
}

void rotate_about_axis_through_point_0085e8c0(float out[16], const float in[16], const Vec3& pivot,
                                              const Vec3& axis, float angle) {
    rotate_basis_about_axis_0085e880(out, in, axis, angle);          // 0085E8DE..0085E8F9
    const Vec3 t{in[12] - pivot.x, in[13] - pivot.y, in[14] - pivot.z};  // 0085E8FE..0085E924
    const Vec3 r = rotate_vector_about_axis_0085c3f0(t, axis, angle);    // 0085E930
    out[12] = pivot.x + r.x;
    out[13] = pivot.y + r.y;
    out[14] = pivot.z + r.z;
}

namespace {

// 00419440 (not in this segment): f(sqrt((f(x*x) + f(y*y)) + f(z*z))), the
// CRT sqrt 00BF7030 at 24-bit precision.
float length_00419440(const float* v) noexcept {
    const float s = (v[0] * v[0] + v[1] * v[1]) + v[2] * v[2];
    return std::sqrt(s);
}

void normalise_row(float* row) noexcept {
    const float len = length_00419440(row);
    const float inv = len > 0.0f ? 1.0f / len : 0.0f;
    row[0] = row[0] * inv;
    row[1] = row[1] * inv;
    row[2] = row[2] * inv;
}

// rows by index: a is normalised first, b is made orthogonal to it, and
// c = p x q.
void orthonormalize_rows(float m[16], int a, int b, int c, int p, int q) noexcept {
    float* const ra = m + a * 4;
    float* const rb = m + b * 4;
    normalise_row(ra);
    const float d = (ra[1] * rb[1] + ra[0] * rb[0]) + ra[2] * rb[2];
    const float px = ra[0] * d;
    const float py = ra[1] * d;
    const float pz = d * ra[2];
    rb[0] = rb[0] - px;
    rb[1] = rb[1] - py;
    rb[2] = rb[2] - pz;
    normalise_row(rb);
    const float* const u = m + p * 4;
    const float* const w = m + q * 4;
    const float cx = u[1] * w[2] - u[2] * w[1];
    const float cy = u[2] * w[0] - u[0] * w[2];
    const float cz = u[0] * w[1] - u[1] * w[0];
    float* const rc = m + c * 4;
    rc[0] = cx;
    rc[1] = cy;
    rc[2] = cz;
    normalise_row(rc);
}

}  // namespace

void orthonormalize_from_row_x_0085d3d0(float m[16]) noexcept {
    orthonormalize_rows(m, 0, 1, 2, 0, 1);
}

void orthonormalize_from_row_z_0085d770(float m[16]) noexcept {
    orthonormalize_rows(m, 2, 0, 1, 2, 0);
}

void orthonormalize_from_row_y_then_x_0085d920(float m[16]) noexcept {
    orthonormalize_rows(m, 1, 0, 2, 0, 1);
}

void orthonormalize_from_row_y_then_z_0085dad0(float m[16]) noexcept {
    orthonormalize_rows(m, 1, 2, 0, 1, 2);
}

void water_mine_arm_0085f1e0(WaterMineArmState& s, const WaterMineArmHost& host) {
    s.armed_46c = true;
    s.armed_part_468 = host.construct_part ? host.construct_part(s.model_464) : 0;
    if (host.propagate_root_registration) {
        host.propagate_root_registration();
    }
}

std::uintptr_t water_mine_model_pointer_0085ea50(const WaterMineArmState& s) noexcept {
    return s.armed_46c ? s.armed_part_468 : 0;
}

void water_mine_release_sub_objects_0085f120(WaterMineArmState& s, const WaterMineReleaseHost& host) {
    if (s.armed_part_468 != 0) {
        s.model_464 = 0;                            // 0085F12C, before the delete
        if (host.delete_part) {
            host.delete_part(s.armed_part_468);
        }
        s.armed_part_468 = 0;
    }
    if (host.base_release) {
        host.base_release();                        // 0085F156 JMP 006E1860
    }
}

int submarine_detach_from_world_lists_008530e0(const void* unit,
                                               const std::vector<const void*>& class_list_payloads,
                                               const WorldListDetachHost& host) {
    return detach_from_class_world_list(unit, class_list_payloads, host);  // 008530E3, list +78h
}

int torpedo_detach_from_world_lists_008563a0(const void* torpedo,
                                             const std::vector<const void*>& class_list_payloads,
                                             const WorldListDetachHost& host) {
    return detach_from_class_world_list(torpedo, class_list_payloads, host);  // 008563A3, list +21Ch
}

int torpedo_boat_detach_from_world_lists_00857f00(const void* unit,
                                                  const std::vector<const void*>& class_list_payloads,
                                                  const WorldListDetachHost& host) {
    return detach_from_class_world_list(unit, class_list_payloads, host);  // 00857F03, list +C0h
}

int water_mine_detach_from_world_lists_0085f2b0(const void* mine,
                                                const std::vector<const void*>& class_list_payloads,
                                                const WorldListDetachHost& host) {
    return detach_from_class_world_list(mine, class_list_payloads, host);  // 0085F2B3, list +288h
}

void submarine_destruct_00853110(SubmarineLifecycleSlots& slots, const SubmarineDestructHost& host) {
    // 0085312E..00853156; +1224h is left alone.
    if (slots.periscope_effect_1228 != 0) {
        if (host.release_reference) {
            host.release_reference(slots.periscope_effect_1228);
        }
        slots.periscope_effect_1228 = 0;
    }
    if (host.destruct_vehicle_base) {
        host.destruct_vehicle_base();               // 0085316A
    }
}

void submarine_on_killed_release_periscope_00853590(SubmarineLifecycleSlots& slots,
                                                    const SubmarineOnKilledHost& host) {
    const std::uintptr_t effect = slots.periscope_effect_1228;
    if (effect != 0) {
        if (host.stop_effect_children) {
            host.stop_effect_children(effect);
        }
        if (host.mark_effect_finished) {
            host.mark_effect_finished(effect);
        }
        if (host.release_reference) {
            host.release_reference(effect);
        }
        slots.periscope_effect_1228 = 0;
    }
    const std::uintptr_t debris = slots.periscope_debris_1224;
    if (debris != 0) {
        if (slots.debris_name_has_storage && host.return_name_storage) {
            host.return_name_storage(debris);       // 008535F7..00853608
        }
        slots.debris_name_has_storage = false;
        if (host.free_debris) {
            host.free_debris(debris);               // 0085360E
        }
        slots.periscope_debris_1224 = 0;
    }
    if (host.base_on_killed) {
        host.base_on_killed();                      // 00853624 JMP 00819880
    }
}

std::string periscope_debris_init_008533e0(PeriscopeDebris& d, const PeriscopeDebrisInitHost& host) {
    // The 00F87574 zero vector into +14h..+28h, then state +10h = 0.
    d.velocity_14 = Vec3{};
    d.angular_velocity_20 = Vec3{};
    d.state_10 = 0;
    std::string name = kPeriscopeDebrisNamePrefix;
    if (host.owner_name) {
        name += host.owner_name();
    }
    if (!(host.node_world_matrix_current && host.node_world_matrix_current()) &&
        host.refresh_node_world_matrix) {
        host.refresh_node_world_matrix();
    }
    if (host.copy_node_world_matrix) {
        host.copy_node_world_matrix();
    }
    return name;
}

// ---------------------------------------------------------------------------
// Pass cc13-gpr53e: the trace-line effect.

// 00858660 and the four inline copies in 00858700 (00858B06, 00858BCD,
// 00858CE7). CMP count, capacity / JNE; LEA n = 2c + 2; CMP n, capacity / JBE.
void trace_line_pointer_array_append_00858660(TraceLinePointerArray& a, std::uintptr_t item) {
    const auto count = static_cast<std::uint32_t>(a.items.size());
    if (count == a.capacity_54) {
        const std::uint32_t n = a.capacity_54 + a.capacity_54 + 2u;
        if (n > a.capacity_54) {
            a.capacity_54 = n;
        }
    }
    a.items.push_back(item);
}

void trace_line_read_lua_00858700(TraceLineDefinition& def, const TraceLineLuaRow& row,
                                  const TraceLineTextureHost& host) {
    // 0085875A: FMUL qword 0.5 on the 00B66330 float; exact.
    def.half_width_14 = row.number_or("Width", 0.0f) * 0.5f;
    def.width_speed_18 = row.number_or("WidthSpeed", 0.0f);
    def.width_speed_end_1c = row.number_or("WidthSpeedEnd", 0.0f);
    def.max_segment_num_20 = row.integer_or("MaxSegmentNum", 10);
    def.min_segment_length_24 = row.number_or("MinSegmentLength", 1.0f);  // FLD1
    def.segment_life_time_28 = row.number_or("SegmentLifeTime", 1.0f);
    def.outer_color_30 = row.packed_bgra("OuterColor");
    def.anim_on_off_04 = row.boolean_or("AnimOnOff", false);
    def.anim_loop_05 = row.boolean_or("AnimLoop", false);
    def.anim_start_frame_08 = row.integer_or("AnimStartFrame", 0);
    def.anim_end_frame_0c = row.integer_or("AnimEndFrame", def.anim_start_frame_08);  // 008589CA
    def.anim_play_speed_10 = row.number_or("AnimPlaySpeed", 0.0f);

    if (row.is_string("TextureName")) {
        const std::string name = row.string("TextureName");
        const std::int32_t frames = host.count_frames(name);
        if (frames == 0) {
            const std::uintptr_t item = host.find_atlas_item(name);
            if (item != 0) {
                trace_line_pointer_array_append_00858660(def.textures_4c, item);
            }
        } else {
            for (std::int32_t i = 0; i < frames; ++i) {  // JLE at 00858B86
                const std::uintptr_t item = host.find_atlas_item(host.frame_name(name, i));
                if (item == 0) {
                    break;  // 00858BC7 -> 00858C81: the remaining frames are skipped
                }
                trace_line_pointer_array_append_00858660(def.textures_4c, item);
            }
        }
    } else {
        // 00858CD7: no null test.
        trace_line_pointer_array_append_00858660(def.textures_4c,
                                                 host.find_atlas_item("cloud_snake.dds"));
    }

    def.show_bullet_58 = row.boolean_or("ShowBullet", false);
    def.bullet_tail_texture_5c = host.find_atlas_item(
        row.is_string("BulletTailTexture") ? row.string("BulletTailTexture") : "tr_bullet.dds");
    def.bullet_head_texture_60 = host.find_atlas_item(
        row.is_string("BulletHeadTexture") ? row.string("BulletHeadTexture") : "tr_glow_add.dds");
    def.tail_length_6c = row.number_or("TailLength", 16.0f);           // 00CE6454
    def.head_radius_70 = row.number_or("HeadRadius", 0.8f);            // 00CE74F8
    def.bullet_life_time_74 = row.number_or("BulletLifeTime", 2.0f);   // 00CE3958
    def.bullet_fade_begin_78 = row.number_or("BulletFadeBegin", 1.6f); // 00D06BB4
    // 00858FEF: FLD [+74h]; FSUB m32 [+78h]; FSTP. Correctly rounded at 64 bits.
    def.bullet_fade_span_7c = def.bullet_life_time_74 - def.bullet_fade_begin_78;
    // 00859037 / 008590D6: the colour bytes are stored as +2, +1, +0, +3.
    def.tail_color_64 = row.is_nil("TailColor") ? 0xFFFFFFFFu : row.packed_bgra("TailColor");
    def.head_color_68 = row.is_nil("HeadColor") ? 0x87FF8032u : row.packed_bgra("HeadColor");
    def.uv_tile_length_2c = row.number_or("UVTileLength", 1.0f);       // FLD1

    def.vertex_format_40 = host.vertex_format("pf43uf43ccuf44uf41.mvfm");  // 00D0C9C0
    def.vertex_binding_44 = host.vertex_binding(def.vertex_format_40);
}

TraceLineCreateResult trace_line_create_instance_00859240(std::uintptr_t definition, std::uintptr_t owner,
                                                          const TraceLineCreateHost& host) {
    TraceLineCreateResult r;
    r.instance_allocated = host.allocate_instance();
    if (r.instance_allocated) {
        r.instance = TraceLineInstance{};
        r.instance.owner_10 = owner;
        r.instance.component_14 = definition != 0 ? definition + 0x80u : 0u;
    }
    const std::uintptr_t node = host.construct_node();
    if (!r.instance_allocated || node == 0) {
        r.instance.trace_line_1c = node;
        r.faults = true;
        return r;
    }
    r.instance.trace_line_1c = node;
    host.attach(node, definition);
    return r;
}

TraceLineCreateResult trace_line_create_instance_slot_00859370(std::uintptr_t component,
                                                               std::uintptr_t owner,
                                                               const TraceLineCreateHost& host) {
    return trace_line_create_instance_00859240(component - 0x80u, owner, host);
}

void trace_line_instance_update_008585d0(float dt, const TraceLineUpdateHost& host) {
    if (!host.owner_node_world_current()) {
        host.refresh_owner_node_world();
    }
    host.set_point(host.owner_node_world_position(), dt);
    host.update_submodel(dt);
    if (host.owner_stopping()) {
        host.mark_node_finishing();
    }
}

bool trace_line_instance_is_complete_008580a0(const TraceLineNodeView& node) {
    return node.count_190() == 0;
}

void trace_line_instance_set_19c_00858500(const TraceLineNodeView& node, float value) {
    node.set_19c(value);
}

void trace_line_instance_detach_from_root_008583a0(const TraceLineInstance& inst,
                                                   const TraceLineDestructHost& host) {
    if (inst.trace_line_1c != 0) {
        const std::uintptr_t root = host.world_root();
        if (host.node_parent_a4(inst.trace_line_1c) == root) {
            host.propagate_root_registration(inst.trace_line_1c, root);
        }
    }
}

void trace_line_instance_stop_00858520(const TraceLineInstance& inst, const TraceLineNodeView& node) {
    if (inst.trace_line_1c != 0) {
        node.set_stop_1a8();
    }
}

void trace_line_instance_destruct_00858530(TraceLineInstance& inst, const TraceLineDestructHost& host) {
    trace_line_instance_detach_from_root_008583a0(inst, host);  // 0085855A..00858584
    if (inst.trace_line_1c != 0) {
        host.unlink_and_release(inst.trace_line_1c);
        inst.trace_line_1c = 0;
    }
    host.destroy_base();
}

// ---------------------------------------------------------------------------
// Pass cc13-gpr53e: the water tracer reader.

namespace {

// FISTP qword under a truncating control word (0086DD54..0086DD7E,
// 0086DDEC..0086DE16), the low DWORD kept. Out-of-range values give the
// integer indefinite 8000000000000000h, whose low DWORD is 0.
std::int32_t truncate_low_dword(float x) {
    const double d = static_cast<double>(x);
    if (!(d > -9223372036854775808.0 && d < 9223372036854775808.0)) {
        return 0;
    }
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(
        static_cast<std::uint64_t>(static_cast<std::int64_t>(d))));
}

std::uint32_t float_bits(float f) {
    std::uint32_t u;
    std::memcpy(&u, &f, sizeof u);
    return u;
}

// 0086D7B8..0086D862 and its three copies.
void replace_curve(std::uintptr_t& field, const char* gate, const char* key, float scale,
                   const WaterTracerLuaRow& row, const WaterTracerHost& host) {
    std::uintptr_t local = 0;
    if (row.is_table(gate)) {
        local = row.load_curve(key, scale);
    }
    if (field != 0) {
        host.release_curve(field);
        field = 0;
    }
    field = local;
}

}  // namespace

void water_tracer_read_lua_0086d180(WaterTracerComponent& c, const WaterTracerLuaRow& row,
                                    const WaterTracerHost& host,
                                    const WaterTracerCurveHolders& holder_addresses) {
    c.texture_98 = 0;  // 0086D1BC..0086D1C8, nothing released
    c.texture2_9c = 0;
    c.spray_resource_c0 = 0;
    c.show_forward_20 = row.boolean("ShowForward") ? 1u : 0u;
    c.show_backwards_24 = row.boolean("ShowBackwards") ? 1u : 0u;
    c.fit_to_water_28 = row.boolean("FitToWater") ? 1u : 0u;
    c.lifetime_2c = row.number_or("Lifetime", 1.0f);
    const float time_scale = row.number_or("TimeScale", 1.0f);  // [ESP+2Ch]
    c.divisions_34 = static_cast<std::uint32_t>(row.integer_or("Divisions", 4));
    c.line_distance_38 = row.number_or("LineDistance", 4.0f);    // 00CE3D34
    c.lod_3c = row.boolean_or("LOD", false) ? 1u : 0u;
    if (c.lod_3c != 0) {
        const std::int32_t full_lines = row.integer("FullLines");
        if (full_lines == 0) {
            c.lod_3c = 0;
        } else {
            c.full_lines_minus_one_40 = full_lines - 1;
        }
    }
    c.texture_rate_44 = row.number_or("TextureRate", 0.066f);    // 00D0D918
    c.texture_file_48 = row.string("TextureFile");
    c.texture_98 = host.load_texture(c.texture_file_48);
    if (row.is_string("TextureFile2")) {
        c.texture_file2_50 = row.string("TextureFile2");
        c.texture2_9c = host.load_texture(c.texture_file2_50);
    }
    c.alpha_scale_58 = row.number_or("AlphaScale", 1.0f);
    const float alpha_scaler = row.number_or("AlphaScaler", 1.0f);  // [ESP+38h]
    c.width_scale_5c = row.number_or("WidthScale", 6.0f);        // 00CE6630
    c.width_scaler_ac = row.number_or("WidthScaler", 1.0f);
    c.width_offset_a8 = row.number_or("WidthOffset", 0.0f);
    c.bone_length_b0 = row.number_or("BoneLength", 6.0f);        // 00CE6630
    // 0086D71E _ftol2 on the 00B66330 float (truncation).
    c.bone_count_b4 = truncate_low_dword(row.number_or("BoneCount", 25.0f));  // 00CE89CC
    c.length_b8 = row.number_or("Length", 8.0f);                 // 00CE3918
    c.displacement_scale_60 = row.number_or("HeightScale", 1.0f);

    replace_curve(c.alpha_curve_64, "Alpha", "Alpha", time_scale, row, host);
    replace_curve(c.width_curve_68, "Width", "Width", time_scale, row, host);
    replace_curve(c.height_x_curve_6c, "HeightX", "HeightX", time_scale, row, host);
    replace_curve(c.height_y_curve_70, "HeightX", "HeightY", time_scale, row, host);  // 0086D9C8

    c.fade_in_74 = row.number("FadeIn");
    c.start_time_78 = row.number("StartTime");
    c.start_speed_7c = row.number("StartSpeed");
    c.stop_time_80 = row.number("StopTime");
    c.stop_speed_84 = row.number("StopSpeed");
    c.local_space_a0 = row.boolean_or("LocalSpace", false);
    c.width_wave_88 = row.number_or("WidthWave", 0.0f);
    {
        // 0086DC0E: no table test, scale 1.0.
        const std::uintptr_t local = row.load_curve("WidthSpeed", 1.0f);
        if (c.width_speed_curve_8c != 0) {
            host.release_curve(c.width_speed_curve_8c);
            c.width_speed_curve_8c = 0;
        }
        c.width_speed_curve_8c = local;
    }
    c.heading_lines_90 = row.integer_or("HeadingLines", 0);
    c.heading_length_94 = row.number_or("HeadingLength", 0.0f);

    if (c.divisions_34 < 2u) {  // 0086DCF6 JAE (unsigned)
        c.divisions_34 = 2u;
    }
    if (static_cast<double>(c.lifetime_2c) < 0.1) {          // 00D7A3A0
        c.lifetime_2c = 0.1f;                                 // 00D7A2F0
    }
    if (static_cast<double>(c.line_distance_38) < 0.1) {
        c.line_distance_38 = 0.1f;
    }

    const std::int32_t segment_count = truncate_low_dword(row.number_or("SegmentCount", 50.0f));  // 00CEB4D4
    const float vert_tess_power = row.number_or("VertTessPower", 1.0f);
    const std::int32_t horiz_divisions = truncate_low_dword(row.number_or("HorizDivisions", 5.0f));  // 00CE3850
    const float horiz_tess_power = row.number_or("HorizTessPower", 0.0f);

    if (c.fit_to_water_28 == 1u) {
        c.bone_count_b4 = 40;  // 0086DE66
        // 0086DE60..0086DE82: the float spilled and reloaded around FDIV qword 39.0.
        const float t = static_cast<float>(static_cast<double>(c.length_b8) / 39.0);
        c.bone_length_b0 = 3.0 > static_cast<double>(t) ? 3.0f : t;
    }
    c.displacement_scale_60 = row.number_or("DisplacementScale", 0.1f);  // 00D7A2F0

    // 0086DF02..0086DF73. Word [1] is SETNE CL over ECX = &+68h, so its upper
    // three bytes are address bits; 00BADC70 never reads it.
    std::array<std::uint32_t, 17> block{};
    block[0] = static_cast<std::uint32_t>(c.bone_count_b4);
    block[1] = c.fit_to_water_28 != 0 ? 1u : 0u;
    block[2] = c.local_space_a0 ? 1u : 0u;
    block[3] = holder_addresses.width_68;
    block[4] = holder_addresses.height_x_6c;
    block[5] = holder_addresses.height_y_70;
    block[6] = static_cast<std::uint32_t>(segment_count);
    block[7] = float_bits(vert_tess_power);
    block[8] = static_cast<std::uint32_t>(horiz_divisions);
    block[9] = float_bits(horiz_tess_power);
    block[10] = float_bits(c.width_scaler_ac);
    block[11] = float_bits(c.width_offset_a8);
    block[12] = float_bits(c.width_scale_5c);
    block[13] = float_bits(c.displacement_scale_60);
    block[14] = float_bits(c.length_b8);
    block[15] = float_bits(c.texture_rate_44);
    block[16] = float_bits(alpha_scaler);
    c.tracer_resource_a4 = host.create_tracer_resource(block);

    c.has_water_spray_bc = row.is_table("WaterSpray");
    if (!c.has_water_spray_bc) {
        return;
    }
    WaterSprayDefinition spray;
    for (const WaterSprayRow& r : row.rows("WaterSpray")) {
        spray.point_00.push_back(r.p1);
        spray.point_00.push_back(r.p2);
        spray.offset_0c.push_back(r.q1);
        spray.offset_0c.push_back(r.q2);
        spray.a_24.push_back(r.a3);
        spray.b_30.push_back(r.b4);
        spray.count_3c.push_back(r.count5);
        spray.size_18.push_back(r.size6);
    }
    c.spray_resource_c0 = host.create_spray_resource(spray);
}

EffectLightConstructResult construct_effect_light_instance_008740e0(
    EffectLightInstanceStorage& instance, void* definition, const EffectLightDefinitionFields& def,
    PointEffectInstanceStorage* point, const EffectLightConstructHost& host) {
    write_effect_prefix(instance.prefix, definition, point);
    instance.prefix.type_18 = 0;                    // 0087412E
    instance.prefix.table_00 = 0x00d0dbf0;
    instance.prefix.secondary_table_08 = 0x00d0dbec;
    const std::uintptr_t light = host.allocate_light();
    if (light != 0) {
        host.construct_light(light, std::string("dynamic_light_") + host.point_name());
    }
    instance.light_1c = light;
    if (light == 0) {
        return EffectLightConstructResult{true};
    }
    host.set_base_diffuse(light, def.diffuse_28);
    host.set_specular_1c4(light, def.specular_38);
    // 0087423E: COMISS radius, 0.01f; JA keeps the radius.
    host.set_radius_1f8(light, def.radius_24 > 0.01f ? def.radius_24 : 0.01f);
    instance.remaining_20 = def.duration_20;
    return EffectLightConstructResult{};
}

std::uintptr_t effect_create_instance_00869300_family(const EffectInstanceKind& kind,
                                                      const EffectCreateInstanceHost& host) {
    const std::uintptr_t instance = host.allocate(kind.bytes);
    if (instance == 0) {
        return 0;
    }
    host.construct(instance, kind.constructor);
    return instance;
}

void torpedo_commit_step_pose_00855b00(const TorpedoCommitStepPoseHost& host) {
    host.base_commit();
    if (host.has_node_bc()) {
        host.copy_node_matrix_to_c0();
    }
}

void forward_slot_198_to_194_008559d0(float dt, const std::function<void(float)>& slot_194) {
    slot_194(dt);
}

void torpedo_set_node_visibility_00855bb0(bool has_model_464, bool visible,
                                          const std::function<void(float factor)>& set_visibility) {
    if (has_model_464) {
        set_visibility(visible ? 1.0f : 0.0f);
    }
}

void water_mine_place_step_pose_0085f1a0(float dt, const WaterMinePlaceStepPoseHost& host) {
    host.base_place(dt);
    if (host.has_link_84()) {
        host.notify_owner();
    }
}

void torpedo_send_create_message_00855f50(
    const TorpedoState& state, bool has_target, std::uint16_t target_net_id_174, std::uint32_t peer,
    const std::function<void(std::uint32_t peer, const TorpedoCreateMessage&)>& send) {
    send(peer, torpedo_create_message_00855cc0(state, has_target, target_net_id_174));
}

MotionControllerCreateResult unit_create_motion_controller_c_00857c20(const MotionControllerCreateHost& host) {
    MotionControllerCreateResult r;
    const std::uintptr_t controller = host.allocate(0x394u);
    if (controller != 0) {
        host.construct(controller);
    }
    r.controller_1018 = controller;
    return r;
}

MotionControllerCreateResult unit_create_motion_controller_b_00852210(const MotionControllerCreateHost& host) {
    MotionControllerCreateResult r;
    const std::uintptr_t controller = host.allocate(0x39cu);
    if (controller != 0) {
        host.construct(controller);
    }
    r.controller_1018 = controller;
    r.flagged_body_1210 = 0;
    if (controller == 0) {
        r.faults = true;  // 00852263 MOV ECX, [EAX+2Ch]
        return r;
    }
    for (std::uintptr_t body = host.first_body(controller); body != 0; body = host.next_body(body)) {
        if ((host.body_flags(body) & 2u) != 0) {
            r.flagged_body_1210 = body;
            break;
        }
    }
    return r;
}

// ---------------------------------------------------------------------------
// Pass cc13-gpr53f.

void water_tracer_destruct_0086bf10(WaterTracerComponent& c, const WaterTracerDestructHost& host) {
    for (std::uintptr_t* field : {&c.texture_98, &c.texture2_9c, &c.tracer_resource_a4,
                                  &c.spray_resource_c0, &c.width_speed_curve_8c, &c.height_y_curve_70,
                                  &c.height_x_curve_6c, &c.width_curve_68, &c.alpha_curve_64}) {
        if (*field != 0) {
            host.release(*field);
            *field = 0;
        }
    }
    c.texture_file2_50.clear();
    c.texture_file_48.clear();
    host.destroy_base();
}

void thunderstorm_read_lua_0086f3b0(ThunderStormComponent& c, const ThunderStormParams& params) {
    c.glow_texture_40 = params.string("GlowTexture");
    c.has_3d_lightning_54 = params.boolean_or("Has3Dlightning", true);
    {
        const float min_life = params.number_or("MinLifeTime", 0.5f);  // 0086F4DE
        const float max_life = params.number_or("MaxLifeTime", 4.0f);  // 0086F4F3
        c.min_life_time_24 = min_life;
        c.max_life_time_28 = max_life;
    }
    {
        const float min_scale = params.number_or("MinScale", 0.5f);
        const float max_scale = params.number_or("MaxScale", 4.0f);
        c.min_scale_2c = min_scale;
        c.max_scale_30 = max_scale;
    }
    c.max_spawn_count_38 = params.integer_or("MaxSpawnCount", 15);
    c.spawn_delay_3c = static_cast<float>(params.integer_or("SpawnDelay", 1));  // 0086F617
    c.constant_34 = 2.5f;                                                      // 0086F646
    c.textures_48.clear();                                                     // 0086F680
    for (ThunderStormTexture& t : params.textures("Textures")) {
        c.textures_48.push_back(std::move(t));
    }
}

void thunderstorm_destroy_members_0086fb40(ThunderStormComponent& c) {
    c.textures_48.clear();
    c.textures_48.shrink_to_fit();
    c.glow_texture_40.clear();
}

void thunderstorm_destruct_0086fbc0(ThunderStormComponent& c, const std::function<void()>& destroy_base) {
    thunderstorm_destroy_members_0086fb40(c);
    destroy_base();
}

// ---------------------------------------------------------------------------
// Pass cc13-ver2.

BombReleaseResult bomb_release_velocity_006e0a70_006e1f00(const float rack_matrix[16],
                                                         const BombReleaseOwner& owner,
                                                         const Vec3& cone_d) {
    BombReleaseResult r{};
    std::memcpy(r.matrix, rack_matrix, sizeof r.matrix);
    Vec3 v = owner.world_velocity_34;
    if (owner.is_plane_5c_0f && owner.speed_38 > 1.38888896f) {  // 006E0A9B FCOMI / JBE
        v.y = v.y - 3.0f;                                          // 006E0AB3
        r.matrix[8] = v.x;                                         // 006E0AE7..006E0AEF
        r.matrix[9] = v.y;
        r.matrix[10] = v.z;
        orthonormalize_pose_matrix_0085dc80(r.matrix);             // 006E0AF4
    }
    if (!(cone_d.x == 1.0f && cone_d.y == 1.0f && cone_d.z == 1.0f)) {  // 006E1FD5..006E200A
        const float sq = (v.y * v.y + v.x * v.x) + v.z * v.z;               // 006E2010..006E2029
        const float len = static_cast<double>(sq) > 1e-10 ? static_cast<float>(std::sqrt(sq)) : 0.0f;
        const float* r0 = &r.matrix[0];
        const float* r1 = &r.matrix[4];
        const float* r2 = &r.matrix[8];
        float out[3];
        for (int i = 0; i < 3; ++i) {
            const float w2 = (r2[i] * cone_d.z) * len;
            const float w1 = (cone_d.y * r1[i]) * len;
            const float w0 = (cone_d.x * r0[i]) * len;
            out[i] = (w0 + w1) + w2;                                        // 006E213D..006E2181
        }
        v = Vec3{out[0], out[1], out[2]};
    }
    r.velocity = v;
    return r;
}

}  // namespace gameplay_seg53_cc13
}  // namespace bsp
