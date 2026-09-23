#include "bsp/ship_ai_station_keeping.hpp"

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
float max_00415550(float a, float b) noexcept { return (b > a) ? b : a; }

float safe_acos_007789d0(float x) noexcept {
    if (x > 1.0f) return 0.0f;                              // 007789D6
    if (-1.0f > x) return kPiF;                             // 007789E4
    return static_cast<float>(std::acos(static_cast<double>(x)));  // 00BF9940
}

} // namespace

ShipAiStationKeepingTrace ship_ai_station_keeping_arm_009eda28(
    const ShipAiStationRequest& request, const ShipAiStationKeepingInputs& in,
    ShipAiStationKeepingState& s) noexcept {
    ShipAiStationKeepingTrace t{};

    // 009EDA47..009EDA84: the braking distance, (ref * (ref / Retardation)) * 0.5.
    const double ref = in.reference_speed_3c4;
    const float braking = static_cast<float>(
        (ref * (ref / static_cast<double>(in.retardation_508))) * kHalf);
    t.braking = braking;

    // 009EDAA6..009EDAF8: the station point relative to the hull, projected on
    // the request's direction.
    const float dx = static_cast<float>(static_cast<double>(in.goal_x_1dc) - in.hull_x_184);
    const float dz = static_cast<float>(static_cast<double>(in.goal_z_1e0) - in.hull_z_188);
    const double dir_x = request.direction_x;
    const double dir_z = request.direction_z;
    float along = static_cast<float>(static_cast<double>(dx) * dir_x +
                                     static_cast<double>(dz) * dir_z);
    const float neg_dir_z = static_cast<float>(-dir_z);
    const float across = static_cast<float>(static_cast<double>(dx) * neg_dir_z +
                                            dir_x * static_cast<double>(dz));
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
    if (static_cast<double>(ratio) > kDeadband) {
        v = static_cast<float>((static_cast<double>(ratio) - kDeadband) / kDeadbandSpan);
    } else if (kNegDeadband > static_cast<double>(ratio)) {
        v = static_cast<float>((static_cast<double>(ratio) + kDeadband) / kDeadbandSpan);
    } else {
        v = 0.0f;
    }

    // 009EDC16..009EDC52: the along target and max(120, length).
    const double vb = static_cast<double>(v) * braking;
    const float target_along = static_cast<float>(vb + vb + static_cast<double>(along));
    const float length_floor = max_00415550(kLengthFloor, in.unit_length_9c8);

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
    const double rad = static_cast<double>(degrees) * kPi / kDegPerHalfTurn;
    s.aligned_388 = inside_cone_true ? (rad > static_cast<double>(abs_herr))
                                     : (static_cast<double>(abs_herr) > rad);

    // 009EDDB3..009EDF25: the steer limit.
    const float abs_across = fabs_bits(across);
    const float width = in.unit_width_9cc;
    const float excess = static_cast<float>(static_cast<double>(abs_across) -
                                            static_cast<double>(width) / kThree);
    float steer = kSteerMax;
    if (excess > 0.0f) {
        const float wide = static_cast<float>(static_cast<double>(width) * kTwoAndHalf);
        const float part = static_cast<float>(kPointEight * static_cast<double>(excess));
        const float r = static_cast<float>(static_cast<double>(excess) - min_00415510(part, wide));
        if (in.turn_radius > r) {
            const float q = static_cast<float>(
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
        close = kFifteenDegrees > abs_herr &&
                static_cast<double>(width) * kThree > static_cast<double>(abs_across);
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
    if (along > band) {
        s.throttle_39c = static_cast<float>(
            static_cast<double>(base) + (static_cast<double>(along) - band) / braking);
    } else if (-static_cast<double>(band) > static_cast<double>(along)) {
        s.throttle_39c = static_cast<float>(
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
                if (kFifteenDegrees > abs_herr) s.reversing_389 = true;
                return t;
            }
            s.heading_target_324 = (0.0f > across)                  // 009EE110
                ? wrapped_angle_add_00438aa0(request.heading, steer)
                : wrapped_angle_subtract_00438b10(request.heading, steer);
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
            s.throttle_39c = clamp_00415620(s.throttle_39c, -0.5f, -0.25f);
            if (kFifteenDegrees > abs_herr) s.reversing_389 = false;
            return t;
        }
        s.heading_target_324 = (0.0f > across)                      // 009EE22D
            ? wrapped_angle_subtract_00438b10(request.heading, steer)
            : wrapped_angle_add_00438aa0(request.heading, steer);
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
            s.throttle_39c = -0.5f;
            if (kFifteenDegrees > abs_herr) s.reversing_389 = true;
            return t;
        }
        s.heading_target_324 = (0.0f > across)                      // 009EE36E
            ? wrapped_angle_subtract_00438b10(reversed, steer)
            : wrapped_angle_add_00438aa0(reversed, steer);
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
        s.throttle_39c = clamp_00415620(s.throttle_39c, 0.5f, 1.0f);
        if (kFifteenDegrees > abs_herr) s.reversing_389 = false;
        return t;
    }
    s.heading_target_324 = (0.0f > across)                          // 009EE492
        ? wrapped_angle_add_00438aa0(reversed, steer)
        : wrapped_angle_subtract_00438b10(reversed, steer);
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

} // namespace bsp
