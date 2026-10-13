// Segment 53 reference models (packet cc13-gpr53). See
// include/bsp/gameplay_seg53_cc13.hpp for the adoption note and
// docs/CC13_GPR_53.md for the evidence. Every arithmetic statement below keeps
// the listing's operand order; with the gameplay FPU at 24-bit precision each
// x87 intermediate rounds like a float. Build-tested only.

#include "bsp/gameplay_seg53_cc13.hpp"

#include <cmath>

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
// The tail 00856B56 runs for every outcome but AlreadyInWater: a heading still
// exactly 10000 (UCOMISS, LAHF/TEST AH,44h/JP) takes the pose's yaw.
// ---------------------------------------------------------------------------
TorpedoWaterEntryResult torpedo_enter_water_008568e0(const TorpedoClassFields& cls,
                                                     float terminal_fall_speed_ec,
                                                     const TorpedoWaterEntryInputs& in,
                                                     const TorpedoWaterEntryHost& host) {
    TorpedoWaterEntryResult out;
    out.heading_46c = in.heading_46c;
    if (in.in_water_44) {
        out.outcome = TorpedoWaterEntry::AlreadyInWater;  // 008568ED
        return out;
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
        out.outcome = in.network_client ? TorpedoWaterEntry::BreaksUpClient
                                        : TorpedoWaterEntry::BreaksUpServer;
    } else {
        // 00856938..00856A04: 006E6450(arg), then the splash when arg and
        // WaterSplashEfx are both set.
        out.outcome = TorpedoWaterEntry::Enters;
        if (in.spawn_effect_arg && cls.has_water_splash_efx_d8 && host.spawn_splash_effect) {
            host.spawn_splash_effect();
        }
    }
    if (out.heading_46c == kHeadingUnset && host.current_yaw) {
        out.heading_46c = host.current_yaw();  // 00856B93..00856B9E
    }
    return out;
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

}  // namespace gameplay_seg53_cc13
}  // namespace bsp
