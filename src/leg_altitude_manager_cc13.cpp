// The converging-leg altitude manager, packet cc13-legmgr1. Evidence and listings
// in docs/CC13_LEG_ALTITUDE_MANAGER.md; layout notes in the header.

#include "bsp/leg_altitude_manager_cc13.hpp"

#include <cstddef>

#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_planetasks_cc13.hpp"

// kPc24PlaneTasksCc13Bound sites here (pc24w1-sweep3-7, 009FC260): the band
// distance test at 009FC3A6-009FC3C0, the span test at 009FC698-009FC6A2 and the
// return blend at 009FC776-009FC78A.

namespace bsp {
namespace {

// Swap-remove at index i, as 009FC07B / 009FC20D / 009FC356 do: the last record
// is copied over slot i and the count drops; the scan re-reads slot i.
void swap_remove(std::vector<LegAltitudeRecord>& v, std::size_t i) {
    v[i] = v.back();
    v.pop_back();
}

}  // namespace

// 009FC000. The scan (009FC058-009FC0B6) stops at the first record of `unit`
// (009FC073 CMP EAX,[ESP+3Ch] / JZ 009FC190, which skips the append and leaves
// the stored point as it was). A null unit (009FC0BA) appends nothing. The append
// grows the array to 2 * capacity + 2 records (009FC0EA-009FC157); the copy is
// order-preserving, so std::vector stands for it.
void LegAltitudeManager::register_009fc000(const LegAltitudeUnitHost& host, LegUnitId unit,
                                           const std::array<float, 3>& point) {
    std::size_t i = 0;
    while (i < records_.size()) {
        const LegUnitId u = records_[i].unit;
        if (u == 0 || host.out_of_action_5d(u)) {   // 009FC067 TEST / 009FC06D CMP byte [EAX+5Dh],0
            swap_remove(records_, i);
            continue;
        }
        if (u == unit) return;                       // 009FC073-009FC077
        ++i;
    }
    if (unit == 0) return;
    records_.push_back(LegAltitudeRecord{unit, point});   // 009FC163-009FC18C
}

// 009FC1C0. One pass: a record goes when it is `unit`'s, null or out of action
// (009FC1F5 CMP EAX,EBX; 009FC1F9 TEST; 009FC1FD CMP byte [EAX+5Dh],0).
void LegAltitudeManager::remove_009fc1c0(const LegAltitudeUnitHost& host, LegUnitId unit) {
    std::size_t i = 0;
    while (i < records_.size()) {
        const LegUnitId u = records_[i].unit;
        if (u == unit || u == 0 || host.out_of_action_5d(u)) {
            swap_remove(records_, i);
            continue;
        }
        ++i;
    }
}

// 009FC3F4-009FC73C. sep = (float)(CircleAltDiff * 0.5) (009FC3F9 FLD [EAX+378h],
// 009FC3FF FMUL qword [00D7A280] = 0.5, 009FC417 FSTP float). down and up both
// start at y. Each pass walks the bands in order (unrolled by four at
// 009FC44D-009FC60A, the remainder at 009FC614-009FC688); per band, with
// lo = (float)(b - sep) and hi = (float)(b + sep):
//   lo < down && down < hi  ->  down = lo       (009FC46C/009FC474, both JBE)
//   lo < up   && up   < hi  ->  up = hi         (009FC492/009FC49E, both JBE)
// The pass repeats while any band moved either (CL, 009FC68A). The choice
// (009FC692-009FC6DB, bytes D8 E1 / D9 CA / DE E1 read at 009FC69C):
//   moved && (up - down) > (y - down)               -> alt = down
//   moved && !(that) && up < ceiling (FCOMIP, JNC)  -> alt = up
//   moved otherwise                                 -> alt = down
//   nothing moved                                    -> alt = y
// Then the floor (009FC6F0-009FC73C): f = y, -20.0f when y < -20.0f ([00CF180C]),
// 120.0f ([00D05804]) when y > 120.0 ([00D1F3F8] qword); alt = f when f > alt.
float leg_separated_altitude_009fc260(const std::vector<float>& bands, float y,
                                      const LegAltitudeTuning& tuning) noexcept {
    float alt = y;
    if (!bands.empty()) {
        const float sep = static_cast<float>(static_cast<double>(tuning.circle_alt_diff_378) * 0.5);
        float down = y;
        float up = y;
        bool moved_any = false;
        bool moved_pass = false;
        do {
            moved_pass = false;
            for (const float b : bands) {
                const float lo = static_cast<float>(static_cast<double>(b) - sep);
                const float hi = static_cast<float>(static_cast<double>(b) + sep);
                if (lo < down && down < hi) {
                    down = lo;
                    moved_pass = moved_any = true;
                }
                if (lo < up && up < hi) {
                    up = hi;
                    moved_pass = moved_any = true;
                }
            }
        } while (moved_pass);
        if (moved_any) {
            // Both differences are x87 intermediates; this is the image's literal
            // test, which reduces to up > y.
            bool span_wins;
            if constexpr (kPc24PlaneTasksCc13Bound) {
                // 009FC698 FLD [ESP+20h]; D8 E1 FSUB ST0,ST1; DE E1 FSUBRP: both
                // differences are PC24 stack values.
                span_wins = cc13x87::x87_sub24(up, down) > cc13x87::x87_sub24(y, down);
            } else {
                const double span = static_cast<double>(up) - down;
                const double below = static_cast<double>(y) - down;
                span_wins = span > below;
            }
            if (span_wins) {
                alt = down;
            } else if (up < tuning.ceiling_210) {
                alt = up;
            } else {
                alt = down;
            }
        }
    }
    float floor_v = y;
    if (-20.0f > y) {
        floor_v = -20.0f;
    } else if (static_cast<double>(y) > 120.0) {
        floor_v = 120.0f;
    }
    if (floor_v > alt) alt = floor_v;
    return alt;
}

// 009FC260. The collect loop (009FC33E-009FC3E4) skips `unit`'s own record
// (remembering it, 009FC344-009FC349), swap-removes null records only (009FC352),
// and keeps the y of every other record whose x/z lie strictly inside the sum of
// the two turn radii: r = (float)(radius(other) + radius(unit)) (009FC384-009FC390),
// dx = (float)(point.x - rec.x), dz = (float)(point.z - rec.z), kept when
// dx*dx + dz*dz < r*r (009FC3C0 FCOMIP, JBE skips). The write-back (009FC742-009FC764)
// copies the point into the record and then overwrites the record's y with alt.
// The return (009FC769-009FC78A): (float)(y + weight * (alt - y)).
float LegAltitudeManager::altitude_009fc260(const LegAltitudeUnitHost& host,
                                            const LegAltitudeTuning& tuning, LegUnitId unit,
                                            const std::array<float, 3>& point, float weight) {
    if (unit == 0) return 0.0f;   // 009FC2A8-009FC2C3 FLDZ
    const float self_r = host.turn_circle_radius_268(unit);
    const float y = point[1];
    std::vector<float> bands;
    bands.reserve(records_.size());
    std::size_t self_index = records_.size();
    bool self_found = false;
    std::size_t i = 0;
    while (i < records_.size()) {
        const LegAltitudeRecord& rec = records_[i];
        if (rec.unit == unit) {
            self_found = true;
            self_index = i;
            ++i;
            continue;
        }
        if (rec.unit == 0) {
            swap_remove(records_, i);
            continue;
        }
        const float r = static_cast<float>(
            static_cast<double>(host.turn_circle_radius_268(rec.unit)) + self_r);
        const float dx = static_cast<float>(static_cast<double>(point[0]) - rec.point[0]);
        const float dz = static_cast<float>(static_cast<double>(point[2]) - rec.point[2]);
        bool inside;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009FC3A6-009FC3C0: DE CA FMULP dx*dx, DE CB FMULP dz*dz, DE C2 FADDP,
            // DC C8 FMUL ST0 r*r, FCOMIP; every product and the sum round to PC24.
            const double d2 = cc13x87::x87_add24(cc13x87::x87_mul24(dz, dz),
                                                 cc13x87::x87_mul24(dx, dx));
            inside = cc13x87::x87_mul24(r, r) > d2;
        } else {
            const double d2 = static_cast<double>(dx) * dx + static_cast<double>(dz) * dz;
            inside = static_cast<double>(r) * r > d2;
        }
        if (inside) bands.push_back(rec.point[1]);
        ++i;
    }
    const float alt = leg_separated_altitude_009fc260(bands, y, tuning);
    if (self_found) {
        records_[self_index].point = {point[0], alt, point[2]};
    }
    float out;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 009FC776-009FC78A: DE EA FSUBP ST2 alt - y, DE CA FMULP weight *, DE C1
        // FADDP y +, each rounded to PC24.
        out = static_cast<float>(cc13x87::x87_add24(
            y, cc13x87::x87_mul24(weight, cc13x87::x87_sub24(alt, y))));
    } else {
        out = static_cast<float>(static_cast<double>(y) +
                                 static_cast<double>(weight) * (static_cast<double>(alt) - y));
    }
    return out;
}

}  // namespace bsp
