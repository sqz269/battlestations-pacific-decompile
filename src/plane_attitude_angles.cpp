#include "bsp/plane_attitude_angles.hpp"

#include <cmath>

#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_flightrender_cc13.hpp"

// pc24w1-sweep1-3, kPc24FlightRenderCc13Bound (OFF): ON = 009AC190's bearing with every
// op rounded at PC24 and stored through float as the bytes do. Movers: plane bearings to a
// target (the kamikaze go-towards and the other callers of 009AC190).

namespace bsp {
namespace {

// 00CE3830 and 00CE3828, both the float constant widened to double: the bearing
// arm subtracts from 1.5707963705062866 and wraps with 6.2831854820251465.
constexpr double kBearingHalfPi = 1.5707963705062866;
constexpr double kBearingTwoPi = 6.2831854820251465;

}  // namespace

float plane_bearing_to_target_009ac190(const float self_position[3],
                                       const float target_position[3]) noexcept {
    // 009AC1E0-009AC1F1: only x and z. The y components are never read, so this
    // is a ground-plane bearing and a target directly above the plane has the
    // same bearing as one directly ahead of it.
    float out;
    if constexpr (kPc24FlightRenderCc13Bound) {
        // pc24w1-sweep1-3: the bytes store through float at every step. dx and dz
        // (009AC1E7 FSUB, 009AC1ED FSTP; 009AC1F5, 009AC1FB), the atan2 result
        // (009AC23A FSTP), pi/2 minus it (009AC242 FSUBR, 009AC248 FSTP) and the
        // 2*pi add (009AC256 FADD, 009AC25C FSTP), each op rounded at PC24.
        const float dx = static_cast<float>(cc13x87::x87_sub24(target_position[0], self_position[0]));
        const float dz = static_cast<float>(cc13x87::x87_sub24(target_position[2], self_position[2]));
        const float angle = static_cast<float>(std::atan2(static_cast<double>(dz), static_cast<double>(dx)));
        float bearing = static_cast<float>(cc13x87::x87_sub24(kBearingHalfPi, angle));
        if (0.0f > bearing) {                                    // 009AC250 FLDZ / FCOMPI / JBE
            bearing = static_cast<float>(cc13x87::x87_add24(bearing, kBearingTwoPi));
        }
        out = bearing;
    } else {
    const double dx = static_cast<double>(target_position[0]) -
                      static_cast<double>(self_position[0]);
    const double dz = static_cast<double>(target_position[2]) -
                      static_cast<double>(self_position[2]);

    // 009AC235-009AC25C. The same pi/2-minus-atan2 form the heading uses, then a
    // single conditional 2*pi add rather than a general wrap - the native adds
    // once if the result is negative and does not loop.
    double bearing = kBearingHalfPi - std::atan2(dz, dx);
    if (bearing < 0.0) {
        bearing += kBearingTwoPi;
    }
    out = static_cast<float>(bearing);
    }
    return out;
}

}  // namespace bsp
