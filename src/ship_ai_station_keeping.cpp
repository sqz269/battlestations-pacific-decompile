#include "bsp/ship_ai_station_keeping.hpp"

#include "bsp/ship_ai_seg67_switches_cc13.hpp"
#include "bsp/ship_ai_switch_reach.hpp"
#include "bsp/unit_rudder.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>

namespace bsp {
namespace {

constexpr double kHalf = 0.5;                          // 00D7A280
constexpr double kDeadband = 0.10000000149011612;      // 00D7A3A0
constexpr double kNegDeadband = -0.10000000149011612;  // 00CE3928
constexpr double kDeadbandSpan = 0.8999999761581421;   // 00D7A390
constexpr float kLengthFloor = 120.0f;                 // 00D05804
constexpr float kDegWide = 100.0f;                     // 00CE3D08
constexpr float kDegNarrow = 80.0f;                    // 00CE5444
constexpr double kPi = 3.1415927410125732;             // 00CE3D28
constexpr double kDegPerHalfTurn = 180.0;              // 00CE3D20
constexpr float kSteerMax = 1.3962634801864624f;       // 00CF8858, 80 degrees
constexpr double kThree = 3.0;                         // 00D7A2B0
constexpr double kTwoAndHalf = 2.5;                    // 00CE3DE0
constexpr double kPointEight = 0.800000011920929;      // 00CE3D40
constexpr double kTenDegrees = 0.1745329350233078;     // 00D05850
constexpr float kFifteenDegrees = 0.2617993950843811f; // 00D05AA8
constexpr float kTenDegreesF = 0.1745329350233078f;    // 00CE3990
constexpr double kFifty = 50.0;                        // 00CE3938
constexpr float kFiftyF = 50.0f;                       // 00CEB4D4
constexpr double kStopBand = 0.05000000074505806;      // 00D7A270
constexpr float kFarDistance = 1500.0f;                // 00CED724
constexpr float kPiF = 3.1415927410125732f;            // 00D7A264
// Float forms of the double constants above for the PC24 arm; each double is
// exactly representable as a float, so the per-op results agree.
constexpr float kHalfF = 0.5f;                         // 00D7A280
constexpr float kDeadbandF = 0.100000001490116119f;    // 00D7A3A0
constexpr float kDeadbandSpanF = 0.899999976158142090f; // 00D7A390
constexpr float kDegPerHalfTurnF = 180.0f;             // 00CE3D20
constexpr float kThreeF = 3.0f;                        // 00D7A2B0
static_assert(static_cast<double>(kDeadbandF) == kDeadband, "00D7A3A0 is a float value");
static_assert(static_cast<double>(kDeadbandSpanF) == kDeadbandSpan, "00D7A390 is a float value");
static_assert(static_cast<double>(kPiF) == kPi, "00CE3D28 is a float value");

float fabs_bits(float v) noexcept {
    // AND 7FFFFFFFh on the stored float.
    std::uint32_t bits;
    std::memcpy(&bits, &v, sizeof bits);
    bits &= 0x7FFFFFFFu;
    std::memcpy(&v, &bits, sizeof v);
    return v;
}

float clamp_00415620(float v, float lo, float hi) noexcept {
    if (lo > v) return lo;          // 00415635 FCOMI, JA
    if (v > hi) return hi;          // 0041564B FCOMI, JBE keeps v
    return v;
}

float min_00415510(float a, float b) noexcept { return (b > a) ? a : b; }
template <bool NanArms>
float max_00415550(float a, float b) noexcept {
    // 00415565 FCOMIP a,b / JBE: a only when a > b (ordered), else b.
    if constexpr (NanArms) {
        return (a > b) ? a : b;
    } else {
        return (b > a) ? b : a;
    }
}

// The four side picks (009EE11D, 009EE23A, 009EE37B, 009EE49F): COMISS
// across, 0 / JC, taken for across < 0 and for an unordered compare.
template <bool NanArms>
bool across_below_zero(float across) noexcept {
    if constexpr (NanArms) {
        return !(across >= 0.0f);
    } else {
        return 0.0f > across;
    }
}

float safe_acos_007789d0(float x) noexcept {
    if (x > 1.0f) return 0.0f;                              // 007789D6
    if (-1.0f > x) return kPiF;                             // 007789E4
    return static_cast<float>(std::acos(static_cast<double>(x)));  // 00BF9940
}

// The arm, with the two cc13 switches as parameters so the reach counters
// can evaluate the other settings on a copy of the state.
template <bool NanArms, bool Pc24>
ShipAiStationKeepingTrace station_keeping_arm(
    const ShipAiStationRequest& request, const ShipAiStationKeepingInputs& in,
    ShipAiStationKeepingState& s) noexcept {
    ShipAiStationKeepingTrace t{};

    // 009EDA47..009EDA84: the braking distance, (ref * (ref / Retardation)) * 0.5.
    // ON (kShipAiStationKeepingPc24Cc13Bound): FDIV, FMULP, FMUL [D7A280] each
    // round to float at PC24; 0.5 is exact, so the double constant is 0.5f.
    const double ref = in.reference_speed_3c4;
    const float ref_f = in.reference_speed_3c4;
    const float braking = Pc24
        ? (ref_f * (ref_f / in.retardation_508)) * kHalfF
        : static_cast<float>((ref * (ref / static_cast<double>(in.retardation_508))) * kHalf);
    t.braking = braking;

    // 009EDAA6..009EDAF8: the station point relative to the hull, projected on
    // the request's direction.
    const float dx = static_cast<float>(static_cast<double>(in.goal_x_1dc) - in.hull_x_184);
    const float dz = static_cast<float>(static_cast<double>(in.goal_z_1e0) - in.hull_z_188);
    const double dir_x = request.direction_x;
    const double dir_z = request.direction_z;
    const float neg_dir_z = static_cast<float>(-dir_z);
    float along;
    float across;
    if constexpr (Pc24) {
        // 009EDAC2..009EDAF8: dz*dir_z and dx*dir_x round, then their sum
        // (009EDAE0 FADDP); -dir_z is stored at 009EDAEA, dx*(-dir_z) and
        // dir_x*dz round, then their sum (009EDAF6 FADDP).
        const float dir_x_f = request.direction_x;
        const float dir_z_f = request.direction_z;
        along = (dx * dir_x_f) + (dz * dir_z_f);
        across = (dx * neg_dir_z) + (dir_x_f * dz);
    } else {
        along = static_cast<float>(static_cast<double>(dx) * dir_x +
                                   static_cast<double>(dz) * dir_z);
        across = static_cast<float>(static_cast<double>(dx) * neg_dir_z +
                                    dir_x * static_cast<double>(dz));
    }
    t.along = along;
    t.across = across;

    // 009EDAFC..009EDB2F: the heading error to the station heading.
    const float herr = wrapped_angle_subtract_00438b10(request.heading, in.unit_heading);
    const float abs_herr = fabs_bits(herr);
    t.heading_error = herr;

    // 009EDB28..009EDB8B: the ahead / astern latch.
    const bool making_way = request.making_way;
    bool set = false;
    if (making_way) {
        if (s.direction_35c != ShipAiThrottleDirection::Ahead && 0.0f > s.timer_360) {
            s.direction_35c = ShipAiThrottleDirection::Ahead;
            set = true;
        }
    } else if (s.direction_35c != ShipAiThrottleDirection::Astern && 0.0f > s.timer_360) {
        s.direction_35c = ShipAiThrottleDirection::Astern;
        set = true;
    }
    if (set) {
        s.direction_counter_384 = 0;   // 009EDB73
        s.timer_360 = 1.0f;            // 009EDB83
        s.direction_value_374 = -1.0f; // 009EDB8B
    }

    // 009EDB93..009EDC16: the leader speed ratio with a +/-0.1 deadband.
    const float ratio = clamp_00415620(
        static_cast<float>(static_cast<double>(request.leader_speed) / ref), -1.0f, 1.0f);
    float v;
    // ON: 009EDBE3 FSUBRP / 009EDBFB FADDP round before the FDIV [D7A390]. The
    // doubles 0.10000000149011612 and 0.8999999761581421 are exactly 0.1f and 0.9f.
    if (static_cast<double>(ratio) > kDeadband) {
        v = Pc24
            ? (ratio - kDeadbandF) / kDeadbandSpanF
            : static_cast<float>((static_cast<double>(ratio) - kDeadband) / kDeadbandSpan);
    } else if (kNegDeadband > static_cast<double>(ratio)) {
        v = Pc24
            ? (ratio + kDeadbandF) / kDeadbandSpanF
            : static_cast<float>((static_cast<double>(ratio) + kDeadband) / kDeadbandSpan);
    } else {
        v = 0.0f;
    }

    // 009EDC16..009EDC52: the along target and max(120, length).
    // ON: 009EDC20 FMUL, 009EDC36 FADD ST0,ST0 and 009EDC3E FADD round in turn.
    const double vb = static_cast<double>(v) * braking;
    const float vb_f = v * braking;
    const float target_along = Pc24
        ? (vb_f + vb_f) + along
        : static_cast<float>(vb + vb + static_cast<double>(along));
    const float length_floor = max_00415550<NanArms>(kLengthFloor, in.unit_length_9c8);

    // 009EDC4B..009EDDB3: blk+388h, whether the heading error is inside the
    // 80 / 100 degree cone that matches the side of the station.
    const bool reversing = s.reversing_389;
    const bool astern = s.direction_35c == ShipAiThrottleDirection::Astern;
    const float offset = astern ? (reversing ? length_floor : -0.0f - length_floor)
                                : (reversing ? -0.0f - length_floor : length_floor);
    const float sum = static_cast<float>(static_cast<double>(offset) + target_along);
    float degrees;
    bool inside_cone_true;  // true: flag = rad > |herr|; false: flag = |herr| > rad
    if (astern) {
        if (sum > 0.0f) { degrees = reversing ? kDegWide : kDegNarrow; inside_cone_true = true; }
        else { degrees = reversing ? kDegNarrow : kDegWide; inside_cone_true = false; }
    } else {
        if (sum > 0.0f) { degrees = reversing ? kDegNarrow : kDegWide; inside_cone_true = false; }
        else { degrees = reversing ? kDegWide : kDegNarrow; inside_cone_true = true; }
    }
    // ON: 009EDD9B FMUL [CE3D28] and 009EDDA1 FDIV [CE3D20] each round at
    // PC24 before the FCOMIP; the doubles are exactly kPiF and 180.0f.
    const double rad = Pc24
        ? static_cast<double>((degrees * kPiF) / kDegPerHalfTurnF)
        : static_cast<double>(degrees) * kPi / kDegPerHalfTurn;
    s.aligned_388 = inside_cone_true ? (rad > static_cast<double>(abs_herr))
                                     : (static_cast<double>(abs_herr) > rad);

    // 009EDDB3..009EDF25: the steer limit.
    const float abs_across = fabs_bits(across);
    const float width = in.unit_width_9cc;
    // ON: 009EDE0E FDIVR (width / 3.0) rounds before 009EDE13 FSUBP.
    const float excess = Pc24
        ? abs_across - (width / kThreeF)
        : static_cast<float>(static_cast<double>(abs_across) -
                             static_cast<double>(width) / kThree);
    float steer = kSteerMax;
    if (excess > 0.0f) {
        const float wide = static_cast<float>(static_cast<double>(width) * kTwoAndHalf);
        const float part = static_cast<float>(kPointEight * static_cast<double>(excess));
        const float r = static_cast<float>(static_cast<double>(excess) - min_00415510(part, wide));
        if (in.turn_radius > r) {
            // ON: 009EDE78 FSUBRP (turn - r) rounds before 009EDE7A FDIVP.
            const float q = Pc24
                ? (in.turn_radius - r) / in.turn_radius
                : static_cast<float>(
                      (static_cast<double>(in.turn_radius) - r) / in.turn_radius);
            steer = min_00415510(steer, fabs_bits(safe_acos_007789d0(q)));
        }
        const float abs_along = fabs_bits(along);
        if (abs_along > 1.0f) {
            const float slope = static_cast<float>(static_cast<double>(abs_across) / abs_along);
            const float angle = static_cast<float>(std::atan(static_cast<double>(slope)));
            const float capped = static_cast<float>(static_cast<double>(angle) + kTenDegrees);
            steer = min_00415510(steer, capped);
        }
    } else {
        steer = 0.0f;                                        // 009EDF60
    }
    t.steer_limit = steer;

    // 009EDF29..009EDFA7: blk+38Ah, close to the station line.
    bool close;
    if (s.close_38a) {
        // ON: 009EDF4E FMUL [D7A2B0] rounds at PC24 before the FCOMIP.
        close = kFifteenDegrees > abs_herr &&
                (Pc24
                     ? static_cast<double>(width * kThreeF)
                     : static_cast<double>(width) * kThree) > static_cast<double>(abs_across);
    } else {
        close = kTenDegreesF > abs_herr &&
                static_cast<double>(width) + width > static_cast<double>(abs_across);
    }
    s.close_38a = close;

    // 009EDF8D..009EE039: the throttle command in blk+39Ch.
    const float base = static_cast<float>(static_cast<double>(request.leader_speed) / ref);
    s.throttle_39c = base;
    along = making_way ? static_cast<float>(static_cast<double>(along) - request.radius)
                       : static_cast<float>(static_cast<double>(request.radius) + along);
    const float third = static_cast<float>(static_cast<double>(in.unit_length_9c8) / kThree);
    const float band = (kFifty > static_cast<double>(third)) ? kFiftyF : third;
    // ON: 009EE015 FSUBRP / 009EE029 FADDP, then FDIV braking, then FADDP base,
    // each rounding in turn.
    if (along > band) {
        s.throttle_39c = Pc24
            ? base + ((along - band) / braking)
            : static_cast<float>(
                  static_cast<double>(base) + (static_cast<double>(along) - band) / braking);
    } else if (-static_cast<double>(band) > static_cast<double>(along)) {
        s.throttle_39c = Pc24
            ? base + ((along + band) / braking)
            : static_cast<float>(
                  static_cast<double>(base) + (static_cast<double>(along) + band) / braking);
    }

    // 009EE03B..009EE089.
    if (kStopBand > static_cast<double>(fabs_bits(s.throttle_39c))) {
        s.throttle_39c = 0.0f;
    } else if (s.close_38a) {
        s.aligned_388 = making_way ? (0.0f > s.throttle_39c) : (s.throttle_39c > 0.0f);
    }

    // 009EE08F..009EE0AD.
    const bool changed = s.aligned_388 != s.reversing_389;
    s.distance_32c = kFarDistance;

    const float prod = static_cast<float>(static_cast<double>(herr) * across);
    auto clamp_cos = [&](float lo, float hi) {
        const float scaled = static_cast<float>(static_cast<double>(s.throttle_39c) /
            static_cast<float>(std::cos(static_cast<double>(steer))));
        s.throttle_39c = clamp_00415620(scaled, lo, hi);
    };

    if (!astern) {
        if (!s.reversing_389) {
            if (changed) {                                          // 009EE0C7
                s.throttle_39c = 0.5f;
                s.heading_target_324 = request.heading;
                s.last_writer_324 = 0x009EE0D9u;
                if (kFifteenDegrees > abs_herr) s.reversing_389 = true;
                return t;
            }
            s.heading_target_324 = across_below_zero<NanArms>(across)                  // 009EE110
                ? wrapped_angle_add_00438aa0(request.heading, steer)
                : wrapped_angle_subtract_00438b10(request.heading, steer);
            s.last_writer_324 = 0x009EE142u;
            if (s.close_38a) {                                      // 009EE4CE
                if (0.0f > s.throttle_39c) s.throttle_39c = 0.0f;
                return t;
            }
            if (0.25f > s.throttle_39c) s.throttle_39c = 0.25f;     // 009EE14E
            if (0.0f > prod) {
                s.throttle_39c = clamp_00415620(s.throttle_39c, 0.5f, 1.0f);
            } else {
                clamp_cos(0.5f, 1.25f);
            }
            return t;
        }
        if (changed) {                                              // 009EE1D6
            s.heading_target_324 = request.heading;
            s.last_writer_324 = 0x009EE1E8u;
            s.throttle_39c = clamp_00415620(s.throttle_39c, -0.5f, -0.25f);
            if (kFifteenDegrees > abs_herr) s.reversing_389 = false;
            return t;
        }
        s.heading_target_324 = across_below_zero<NanArms>(across)                      // 009EE22D
            ? wrapped_angle_subtract_00438b10(request.heading, steer)
            : wrapped_angle_add_00438aa0(request.heading, steer);
        s.last_writer_324 = 0x009EE25Fu;
        if (s.close_38a) {                                          // 009EE26B
            if (s.throttle_39c > 0.0f) s.throttle_39c = 0.0f;
            return t;
        }
        if (s.throttle_39c > -0.25f) s.throttle_39c = -0.25f;       // 009EE280
        if (prod > 0.0f) {
            s.throttle_39c = clamp_00415620(s.throttle_39c, -0.5f, -0.25f);
        } else {
            clamp_cos(-0.625f, -0.25f);
        }
        return t;
    }

    // 009EE2EB: astern, about the reversed station heading.
    const float reversed = wrapped_angle_add_00438aa0(request.heading, kPiF);
    if (!s.reversing_389) {
        if (changed) {                                              // 009EE31B
            s.heading_target_324 = reversed;
            s.last_writer_324 = 0x009EE321u;
            s.throttle_39c = -0.5f;
            if (kFifteenDegrees > abs_herr) s.reversing_389 = true;
            return t;
        }
        s.heading_target_324 = across_below_zero<NanArms>(across)                      // 009EE36E
            ? wrapped_angle_subtract_00438b10(reversed, steer)
            : wrapped_angle_add_00438aa0(reversed, steer);
        s.last_writer_324 = 0x009EE3A2u;
        if (s.close_38a) {
            if (s.throttle_39c > 0.0f) s.throttle_39c = 0.0f;       // 009EE26B
            return t;
        }
        if (s.throttle_39c > -0.25f) s.throttle_39c = -0.25f;       // 009EE3B2
        if (prod > 0.0f) {
            s.throttle_39c = clamp_00415620(s.throttle_39c, -0.625f, -0.25f);
        } else {
            clamp_cos(-0.625f, -0.25f);
        }
        return t;
    }
    if (changed) {                                                  // 009EE41E
        s.heading_target_324 = reversed;
        s.last_writer_324 = 0x009EE424u;
        s.throttle_39c = clamp_00415620(s.throttle_39c, 0.5f, 1.0f);
        if (kFifteenDegrees > abs_herr) s.reversing_389 = false;
        return t;
    }
    s.heading_target_324 = across_below_zero<NanArms>(across)                          // 009EE492
        ? wrapped_angle_add_00438aa0(reversed, steer)
        : wrapped_angle_subtract_00438b10(reversed, steer);
    s.last_writer_324 = 0x009EE4C6u;
    if (s.close_38a) {                                              // 009EE4CE
        if (0.0f > s.throttle_39c) s.throttle_39c = 0.0f;
        return t;
    }
    if (0.25f > s.throttle_39c) s.throttle_39c = 0.25f;             // 009EE4E3
    if (0.0f > prod) {
        s.throttle_39c = clamp_00415620(s.throttle_39c, 0.5f, 1.0f);
    } else {
        clamp_cos(0.5f, 1.25f);
    }
    return t;
}

bool same_state(const ShipAiStationKeepingState& a, const ShipAiStationKeepingState& b) noexcept {
    return a.direction_35c == b.direction_35c &&
           ship_ai_reach_same_bits(a.timer_360, b.timer_360) &&
           ship_ai_reach_same_bits(a.direction_value_374, b.direction_value_374) &&
           a.direction_counter_384 == b.direction_counter_384 &&
           a.aligned_388 == b.aligned_388 && a.reversing_389 == b.reversing_389 &&
           a.close_38a == b.close_38a &&
           ship_ai_reach_same_bits(a.throttle_39c, b.throttle_39c) &&
           ship_ai_reach_same_bits(a.heading_target_324, b.heading_target_324) &&
           ship_ai_reach_same_bits(a.distance_32c, b.distance_32c) &&
           a.last_writer_324 == b.last_writer_324;
}

} // namespace

ShipAiStationKeepingTrace ship_ai_station_keeping_arm_009eda28(
    const ShipAiStationRequest& request, const ShipAiStationKeepingInputs& in,
    ShipAiStationKeepingState& s) noexcept {
    constexpr bool kNan = kShipAiStationKeepingNanArmsCc13Bound;
    constexpr bool kPc24 = kShipAiStationKeepingPc24Cc13Bound;
    // Reach (packet cc9-sweep2): run each switch's other setting on a copy of
    // the incoming state and count the calls whose state comes out different.
    ShipAiStationKeepingState flip_nan = s;
    ShipAiStationKeepingState flip_pc24 = s;
    station_keeping_arm<!kNan, kPc24>(request, in, flip_nan);
    station_keeping_arm<kNan, !kPc24>(request, in, flip_pc24);
    const ShipAiStationKeepingTrace t = station_keeping_arm<kNan, kPc24>(request, in, s);
    ShipAiSwitchReach& reach = ship_ai_switch_reach();
    ++reach.station_calls;
    if (std::isnan(in.unit_length_9c8) || std::isnan(t.across)) ++reach.station_nan_operands;
    if (!same_state(s, flip_nan)) ++reach.station_nan_differs;
    if (!same_state(s, flip_pc24)) ++reach.station_pc24_differs;
    return t;
}

} // namespace bsp
