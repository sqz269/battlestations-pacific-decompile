#include "bsp/gun_heading_snap.hpp"

#include <cmath>
#include <cstddef>
#include <limits>

namespace bsp {
namespace {

constexpr float kPi = 3.14159265358979323846f;

// The wrapped difference a - b, in (-pi, pi].
float wrapped_delta(float a, float b) noexcept {
    float d = a - b;
    while (d <= -kPi) d += 2.0f * kPi;
    while (d > kPi) d -= 2.0f * kPi;
    return d;
}

float clamp_to_pi(float v) noexcept {
    if (v < -kPi) return -kPi;
    if (v > kPi) return kPi;
    return v;
}

bool holds_horizontally(const GunFiringArc& arc, float horz) noexcept {
    // widened by 00D08B88 on both sides, no flag test and no vertical test
    return horz >= arc.min_horz - kGunHeadingSnapBoundEpsilonRadians &&
           horz <= arc.max_horz + kGunHeadingSnapBoundEpsilonRadians;
}

}  // namespace

float gun_heading_snap_failure_value() noexcept {
    return std::numeric_limits<float>::max();
}

bool gun_heading_snap_failed(float value) noexcept {
    return !(value < std::numeric_limits<float>::max());
}

float gun_snap_heading_to_fire_window_007f6190(const GunPlatformArcs& arcs,
                                               float horz_radians,
                                               float limit_radians) noexcept {
    const float failure = gun_heading_snap_failure_value();
    if (arcs.first == nullptr || arcs.count == 0) return failure;

    const float horz = clamp_to_pi(horz_radians);

    std::size_t found = arcs.count;
    for (std::size_t i = 0; i < arcs.count; ++i) {
        if (holds_horizontally(arcs.first[i], horz)) {
            found = i;
            break;
        }
    }
    if (found == arcs.count) return failure;          // no window holds it at all
    if ((arcs.first[found].flags & kGunArcFlagFire) != 0) {
        return horz;                                   // already inside a firing window
    }

    // Walk backwards, then forwards, through records that carry the traverse
    // bit; at the first record on each side that also carries the fire bit,
    // take its nearer bound by wrapped angular distance.
    float best = failure;
    float best_distance = 0.0f;
    bool have_best = false;

    auto consider = [&](const GunFiringArc& arc) {
        const float lo = std::fabs(wrapped_delta(arc.min_horz, horz));
        const float hi = std::fabs(wrapped_delta(arc.max_horz, horz));
        const float edge = lo <= hi ? arc.min_horz : arc.max_horz;
        const float distance = lo <= hi ? lo : hi;
        if (!have_best || distance < best_distance) {
            best = edge;
            best_distance = distance;
            have_best = true;
        }
    };

    for (std::size_t step = 0; step < found; ++step) {
        const GunFiringArc& arc = arcs.first[found - 1 - step];
        if ((arc.flags & kGunArcFlagTraverse) == 0) break;
        if ((arc.flags & kGunArcFlagFire) != 0) {
            consider(arc);
            break;
        }
    }
    for (std::size_t i = found + 1; i < arcs.count; ++i) {
        const GunFiringArc& arc = arcs.first[i];
        if ((arc.flags & kGunArcFlagTraverse) == 0) break;
        if ((arc.flags & kGunArcFlagFire) != 0) {
            consider(arc);
            break;
        }
    }

    if (!have_best) return failure;
    if (best_distance > limit_radians) return failure;
    return best;
}

}  // namespace bsp
