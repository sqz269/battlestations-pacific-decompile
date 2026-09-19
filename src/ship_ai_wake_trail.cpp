// Packet cc8_ship_follow. The two wake-trail routines, transcribed from the
// listing: docs/SHIP_UNIT_GROUP_FOLLOW.md sections 5b and 5c carry the
// instruction addresses for every line below.

#include "bsp/ship_ai_wake_trail.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp"  // wrapped_angle_add_00438aa0 / _subtract_00438b10

namespace bsp {
namespace {

constexpr float kHalfPi = 1.5707963705062866f;   // 00CE3830, a double
constexpr float kTwoPi = 6.2831854820251465f;    // 00CE3828, a double

// 0081065C..00810672 and 0081088A..008108A0, the same two instructions twice:
// pi/2 minus the sample heading, brought back into [0, 2pi).
float wrap_two_pi_from_half_pi(float heading) noexcept {
    float a = kHalfPi - heading;
    if (a < 0.0f) a += kTwoPi;
    return a;
}

int ring_step_back(int index) noexcept {
    // 00810330..00810348 for the append and 0081076F..00810771 for the walk:
    // one slot back, wrapping to 27h.
    return (index <= 0) ? static_cast<int>(kShipAiWakeSampleCount) - 1 : index - 1;
}

int ring_step_forward(int index) noexcept {
    // 008105AC and 008107A6: one slot on, wrapping at 27h.
    return (index >= static_cast<int>(kShipAiWakeSampleCount) - 1) ? 0 : index + 1;
}

// 0042B2F0 BSP_Vector3_LengthFloatThreshold at 00810273 and 00810416, and
// 00414C60 BSP_Vector2f_LengthWithCutoff at 008104E2. Both native routines
// return zero below a small magnitude cutoff; that behaviour is NOT reproduced
// here, and the difference can only show for a delta of a few micrometres. Every
// use below is either guarded by the 4 m append gate or by the residual's own
// zero test, so no branch in this file can turn on the cutoff.
float length3(float x, float y, float z) noexcept {
    return std::sqrt(x * x + y * y + z * z);
}

float length2(float x, float z) noexcept { return std::sqrt(x * x + z * z); }

void write_sample(ShipAiWakeSample& sample, float x, float y, float z,
                  float heading, float yaw_rate) noexcept {
    // 0081034B..008103AB and 008105C7..00810620 write the same five fields in
    // the same order, with the arc length zeroed.
    sample.x = x;
    sample.y = y;
    sample.z = z;
    sample.segment = 0.0f;
    sample.heading = heading;
    sample.yaw_rate = yaw_rate;
}

}  // namespace

void ship_ai_wake_append_00810190(ShipAiWakeTrail& trail, const float world_pos[3],
                                  float heading, float yaw_rate) noexcept {
    // 00810198..008101EB: the point is the argument plus the residual.
    float nx = world_pos[0] + trail.residual[0];
    float ny = world_pos[1] + trail.residual[1];
    float nz = world_pos[2] + trail.residual[2];

    const int h = trail.head;
    const ShipAiWakeSample& head_sample = trail.samples[h];

    // 0081020F..0081021D: the Y term is an explicit FLDZ, so this is a
    // horizontal distance even though the sample carries a Y.
    const float dx = nx - head_sample.x;
    const float dz = nz - head_sample.z;
    const float d2 = dx * dx + 0.0f * 0.0f + dz * dz;
    if (kShipAiWakeAppendMinDistSq > d2) return;  // 00810225..0081022F
    ++trail.appends;
    if (trail.written == 0) trail.written = 1;  // host bookkeeping only

    // 00810235..0081026B: when the residual already equals the three globals at
    // 00F87574/78/7Ch the decay is skipped entirely. Those three are .data past
    // the raw size, so they read 0.0 at load; nothing is known to write them and
    // nothing here may assume they stay zero - see docs/SHIP_UNIT_GROUP_FOLLOW.md
    // section 7. The reset value below is the same triple.
    const bool residual_at_reset = trail.residual[0] == 0.0f
        && trail.residual[1] == 0.0f && trail.residual[2] == 0.0f;
    if (!residual_at_reset) {
        const float residual_length
            = length3(trail.residual[0], trail.residual[1], trail.residual[2]);
        const float travelled = kShipAiWakeResidualDecay * std::sqrt(d2);
        if (travelled < residual_length) {
            // 008102CF..008102F0: shorten the residual by `travelled`.
            const float scale = (residual_length - travelled) / residual_length;
            trail.residual[0] *= scale;
            trail.residual[1] *= scale;
            trail.residual[2] *= scale;
        } else {
            // 008102A3..008102CD: back to the reset triple.
            trail.residual[0] = 0.0f;
            trail.residual[1] = 0.0f;
            trail.residual[2] = 0.0f;
        }
        // 008102F3..00810326: the point is rebuilt on the new residual.
        nx = world_pos[0] + trail.residual[0];
        ny = world_pos[1] + trail.residual[1];
        nz = world_pos[2] + trail.residual[2];
    }

    // 0081034B..008103AB: the head sample is rewritten in place.
    write_sample(trail.samples[h], nx, ny, nz, heading, yaw_rate);

    const int prev = ring_step_back(h);
    const int prev2 = ring_step_back(prev);
    ShipAiWakeSample& prev_sample = trail.samples[prev];
    ShipAiWakeSample& prev2_sample = trail.samples[prev2];

    // 00810416: the leg from the previous sample to the new point.
    const float leg = length3(nx - prev_sample.x, ny - prev_sample.y, nz - prev_sample.z);

    // 0081041F..0081047B: the previous sample's heading is back-filled from the
    // direction to the new point, measured from the sample TWO back.
    prev_sample.heading = wrap_two_pi_from_half_pi(
        std::atan2(nz - prev2_sample.z, nx - prev2_sample.x));

    // 0081046E..00810484. The FCOMIP compares the PREVIOUS sample's stored
    // length (ST0) against the new leg (ST1) and the JBE leaves for
    // "stored <= new", so the flag arm below is the one for a leg that SHRANK.
    // Reading this the other way round is what a first pass did, and the run
    // caught it: 4 head advances over 5600 m of travel instead of about 112,
    // because a steadily growing leg kept the flag set and 00810599 then
    // refuses to advance.
    if (prev_sample.segment > leg) {
        // 00810486..00810493
        trail.leg_shrank = true;
        prev_sample.segment = leg;
    } else if (!trail.leg_shrank) {
        // 0081049D..008104B1
        prev_sample.segment = leg;
    } else {
        // 008104B6..008104E7: the 2D distance from the sample two back.
        const float span = length2(prev2_sample.x - nx, prev2_sample.z - nz);
        if (kShipAiWakeMergeDist > span) {
            // 008104FB..0081056B: the last leg is merged. The previous sample
            // becomes the new point, the head slot takes the sample ahead of it
            // (00810160, a plain six-float copy), and the head moves BACK.
            prev2_sample.segment = span;
            write_sample(prev_sample, nx, ny, nz, heading, yaw_rate);
            trail.samples[h] = trail.samples[ring_step_forward(h)];
            trail.head = prev;
            ++trail.merges;
            return;  // prev_sample.segment is now 0, so the tail cannot advance
        }
        // 00810573..0081057E
        prev_sample.segment = leg;
        trail.leg_shrank = false;
    }

    // 00810585..00810599: the head only moves on for a long enough leg. The
    // FCOMIP at 00810591 compares the segment against 50.0 and the JBE returns
    // for "segment <= 50.0", so equality does NOT advance.
    if (!(prev_sample.segment > kShipAiWakeAdvanceLeg)) return;
    if (trail.leg_shrank) return;

    const int next = ring_step_forward(h);
    trail.head = next;
    write_sample(trail.samples[next], nx, ny, nz, heading, yaw_rate);
    ++trail.advances;
    if (trail.written < kShipAiWakeSampleCount) ++trail.written;  // host bookkeeping
}

ShipAiWakePoint ship_ai_wake_sample_at_distance_00810630(const ShipAiWakeTrail& trail,
                                                         float along) noexcept {
    ShipAiWakePoint out{};
    const ShipAiWakeSample& head_sample = trail.samples[trail.head];

    if (!(along > 0.0f)) {
        // 00810645..00810718, the head-sample branch. COMISS/JC at 0081063C
        // takes the walk only for a strictly positive `along`.
        const float a = wrap_two_pi_from_half_pi(head_sample.heading);
        out.dir_x = std::cos(a);
        out.dir_y = 0.0f;
        out.dir_z = std::sin(a);
        // 008106DD..00810712. The middle term is the double 00D7A258 = 0.0, so
        // the extrapolation stays in the horizontal plane.
        out.x = head_sample.x - along * out.dir_x;
        out.y = head_sample.y - along * 0.0f;
        out.z = head_sample.z - along * out.dir_z;
        out.yaw_rate = 0.0f;
        out.yaw_rate_written = false;  // 00810645..00810718 never store it
        return out;
    }

    // 00810740..00810782: walk back one leg at a time, at most the whole ring.
    int i = trail.head;
    std::size_t steps = 0;
    float remaining = along;
    while (steps < kShipAiWakeSampleCount && remaining > trail.samples[i].segment) {
        remaining -= trail.samples[i].segment;
        i = ring_step_back(i);
        ++steps;
    }

    // 00810786..008107A0
    float t = kShipAiWakeWalkExhausted;
    if (steps < kShipAiWakeSampleCount) {
        const float leg = trail.samples[i].segment;
        t = (leg != 0.0f) ? remaining / leg : 0.0f;
    }
    const float one_minus_t = 1.0f - t;

    const int j = ring_step_forward(i);
    const ShipAiWakeSample& back = trail.samples[i];
    const ShipAiWakeSample& fore = trail.samples[j];

    // 008107D7..00810842
    out.x = t * back.x + one_minus_t * fore.x;
    out.y = t * back.y + one_minus_t * fore.y;
    out.z = t * back.z + one_minus_t * fore.z;

    // 00810854..008108D2: the direction is the wrapped angle lerp of the two
    // sample headings, from the forward sample toward the one behind.
    const float difference = wrapped_angle_subtract_00438b10(back.heading, fore.heading);
    const float blended
        = wrapped_angle_add_00438aa0(fore.heading, difference * one_minus_t);
    const float a = wrap_two_pi_from_half_pi(blended);
    out.dir_x = std::cos(a);
    out.dir_y = 0.0f;
    out.dir_z = std::sin(a);

    // 008108D7..008108ED
    out.yaw_rate = back.yaw_rate * t + fore.yaw_rate * one_minus_t;
    out.yaw_rate_written = true;
    return out;
}

float ship_ai_wake_trail_length(const ShipAiWakeTrail& trail) noexcept {
    float total = 0.0f;
    int i = ring_step_back(trail.head);
    // The head's own segment is zero until the next append fills it, so the sum
    // starts one slot back and runs over the slots this ship has laid down.
    for (std::uint32_t n = 1; n < trail.written; ++n) {
        total += trail.samples[i].segment;
        i = ring_step_back(i);
    }
    return total;
}

}  // namespace bsp
