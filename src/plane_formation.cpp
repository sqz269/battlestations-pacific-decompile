// 007ED260 and 007F23A0, read from the listing. docs/PLANE_FORMATION.md.

#include "bsp/plane_formation.hpp"

#include <array>

#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_planetasks_cc13.hpp"

// kPc24PlaneTasksCc13Bound sites here (pc24w1-sweep3-7): shapes 2, 3 and 4 of
// plane_formation_station_007f23a0 (007F25BD-007F2836). The same image routine
// is also copied as place_wing_member_on_station_007f23a0 in game_hosts_units.cpp,
// narrowed by pc24w1-sweep3-5, not here.

namespace bsp {

namespace {

// 007ED2A6 / 007ED361 both compare against squadron+3CCh, and the array behind
// it is five slots; 007F4B55 has no bound test but the authored WingCount enum
// keeps a sixth wing unreachable (docs/PLANE_SQUADRON.md line 192).
int clamp_member_count(int count) noexcept {
    if (count < 0) return 0;
    if (count > kPlaneFormationMaxMembers) return kPlaneFormationMaxMembers;
    return count;
}

// 007F254A `LEA EAX,[EBX+1] / CDQ / SUB EAX,EDX / SAR EAX,1`: the signed
// division of index+1 by two, truncating toward zero. It is the pair number,
// so 1 and 2 share pair 1, 3 and 4 share pair 2.
int pair_number(int formation_index) noexcept {
    const int n = formation_index + 1;
    return n < 0 ? -((-n) >> 1) : (n >> 1);
}

}  // namespace

void plane_formation_assign_indices_007ed260(std::int32_t* formation_index,
                                             std::int32_t* spawn_stamp,
                                             int count) noexcept {
    if (formation_index == nullptr) return;
    const int live = clamp_member_count(count);
    if (live <= 0) return;

    // 007ED267-007ED284: the five-entry table, zeroed before the first walk.
    // The three values it holds are 0 (never seen), 1 (a member is carrying
    // that index right now) and -1 (assigned during this pass).
    std::array<std::int32_t, kPlaneFormationMaxMembers> taken{};
    taken.fill(0);

    // 007ED290-007ED2B4, the first walk.
    for (int i = 0; i < live; ++i) {
        if (spawn_stamp != nullptr) spawn_stamp[i] = i;   // 007ED292
        const std::int32_t current = formation_index[i];
        if (current >= 0 && current < kPlaneFormationMaxMembers) {
            taken[static_cast<std::size_t>(current)] = 1;  // 007ED2AC
        }
    }

    // 007ED2C8-007ED367, the second walk.
    for (int slot = 0; slot < live; ++slot) {
        if (slot == 0) {
            // 007ED2CC-007ED2DC: slot 0 is the flight leader and takes 0.
            formation_index[0] = 0;
            taken[0] = -1;
            continue;
        }
        const std::int32_t current = formation_index[slot];

        // 007ED2E1/007ED2FB then 007ED300-007ED308: the odd candidate. The scan
        // steps past entries that are -1 only, so it walks over what this pass
        // has already handed out and stops on 0 or 1.
        int odd = 1;
        if (taken[1] < 0) {
            do {
                odd += 2;
            } while (odd < kPlaneFormationMaxMembers && taken[static_cast<std::size_t>(odd)] < 0);
        }
        // 007ED30A-007ED319: the even candidate, from 2.
        int even = 2;
        if (taken[2] < 0) {
            do {
                even += 2;
            } while (even < kPlaneFormationMaxMembers
                     && taken[static_cast<std::size_t>(even)] < 0);
        }

        // 007ED31B-007ED33A. When the two candidates are adjacent - `even - 1`
        // equals `odd` - a member that is ALREADY carrying a positive EVEN index
        // is pushed to the next odd one, which is how a plane keeps its side
        // across a re-assignment.
        if (odd == even - 1 && current > 0 && (current & 1) == 0) {
            odd += 2;
        }

        // 007ED33E-007ED35A: the smaller of the two wins.
        const int chosen = (odd < even) ? odd : even;
        if (chosen >= 0 && chosen < kPlaneFormationMaxMembers) {
            formation_index[slot] = chosen;
            taken[static_cast<std::size_t>(chosen)] = -1;
        }
    }
}

PlaneFormationStation plane_formation_station_007f23a0(
    const PlaneFormationStationInputs& in) noexcept {
    PlaneFormationStation out;

    // 007F2556-007F2569: `[ESI+3E4h] - 1` indexes a five-entry jump table and
    // anything above 4 is unsigned-above, which falls through to the tail.
    if (in.shape < 1 || in.shape > 5) return out;
    // The table at 007F2900 (disk bytes): shape 1 -> 007F2576, 2 -> 007F25BD,
    // 3 -> 007F2739, 4 -> 007F26AD, 5 -> 007F2838. All five are read below
    // (packet cc9_l60_formation_shapes, docs/SQUADRON_LAND_TASK.md 5hi).

    // 007F250E-007F2544: the lateral spacing is the larger of the triple's
    // first component and the leader class's +A4h times the double at 00D049A8.
    // `FCOMIP` then `JA` keeps the displacement when it is strictly greater.
    const float width_floor =
        static_cast<float>(static_cast<double>(in.leader_class_width)
                           * kPlaneFormationWidthSpacingMul);
    const float spacing_x =
        (in.displacement[0] > width_floor) ? in.displacement[0] : width_floor;

    const float pair = static_cast<float>(pair_number(in.formation_index));
    const float m = in.morale;                       // squadron+3E8h
    const float dy = in.displacement[1];
    const float dz = in.displacement[2];
    const double pd = static_cast<double>(pair);
    const double md = static_cast<double>(m);
    // 00415510 then 00415550 against 1.0 and 0.0 (007F2647-007F267B).
    const auto clamp01 = [](float v) noexcept {
        v = 1.0f < v ? 1.0f : v;
        return 0.0f > v ? 0.0f : v;
    };
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    // BL and [ESP+1Bh], cleared at 007F255F/007F2564 and set together only by
    // shapes 2 and 4 (007F268E-007F26A4: 0.2 (00CE3D10, double) > morale). Set,
    // the tail mirrors neither y nor z.
    bool keep_yz = false;
    switch (in.shape) {
        case 1:
            // 007F2576-007F25B8. Each component is the pair number times its
            // own displacement times the squadron's morale.
            x = pair * spacing_x * m;
            y = pair * dy * m;
            z = pair * dz * m;
            break;
        case 2: {
            // 007F25BD-007F26A8. z over max(1, morale) (007F25DF 00415550);
            // x times max(2 morale - 1, 0.8) (00CE74F8); y times
            // clamp(2.2 - 1.2 morale, 0, 1) (00D05AC8, 00CEC160, doubles).
            if constexpr (kPc24PlaneTasksCc13Bound) {
                // 007F25E4-007F260C: DE F2 FDIVRP 1/m1, FMUL pair*dz, DE CA FMULP;
                // 007F2616-007F2620: FSUB m - 1, FADD ST0,ST0, DE C1 FADDP + 1;
                // 007F2629-007F2633 and 007F2637-007F268A likewise, each op PC24.
                const float m1 = 1.0f < m ? m : 1.0f;
                z = static_cast<float>(cc13x87::x87_mul24(cc13x87::x87_div24(1.0, m1),
                                                          cc13x87::x87_mul24(pair, dz)));
                const double s = cc13x87::x87_sub24(m, 1.0);
                const float wide = static_cast<float>(
                    cc13x87::x87_add24(cc13x87::x87_add24(s, s), 1.0));
                x = static_cast<float>(cc13x87::x87_mul24(cc13x87::x87_mul24(pair, spacing_x),
                                                          wide > 0.8f ? wide : 0.8f));
                y = static_cast<float>(cc13x87::x87_mul24(
                    cc13x87::x87_mul24(pair, dy),
                    clamp01(static_cast<float>(cc13x87::x87_sub24(
                        2.2000000476837158, cc13x87::x87_mul24(m, 1.2000000476837158))))));
            } else {
                const double m1 = static_cast<double>(1.0f < m ? m : 1.0f);
                z = static_cast<float>((1.0 / m1) * (pd * dz));                       // 007F260C
                const float wide = static_cast<float>((md - 1.0) * 2.0 + 1.0);         // 007F2620
                x = static_cast<float>(pd * spacing_x * (wide > 0.8f ? wide : 0.8f));  // 007F2633
                y = static_cast<float>(pd * dy * clamp01(static_cast<float>(
                        2.2000000476837158 - md * 1.2000000476837158)));                // 007F268A
            }
            keep_yz = 0.20000000298023224 > md;
            break;
        }
        case 3:
            // 007F2739-007F2836. y is shape 1's. The rest depends on the tuning
            // singleton's +3E8h byte (007F2744), the flag the tail's z mirror reads.
            if constexpr (kPc24PlaneTasksCc13Bound) {
                // 007F2828-007F2832: FMUL dy, FMUL morale on the pair left on the
                // stack, each PC24.
                y = static_cast<float>(
                    cc13x87::x87_mul24(cc13x87::x87_mul24(pair, dy), m));
                if (in.symmetrical_position) {
                    // 007F2755-007F275D: DE E1 FSUBRP 1 - m. 007F2772-007F27A8:
                    // a = (idx*sx)*0.15 and b = sx*pair stored to floats, then
                    // a*rest + b*m (DE C1 FADDP). 007F27AC-007F27DA: c = (idx*dz)*0.8
                    // and d = dz*pair stored, then c*rest + d*m (DE C2 FADDP ST2).
                    const float idx = static_cast<float>(in.formation_index);
                    const double rest = cc13x87::x87_sub24(1.0, m);
                    const float a = static_cast<float>(cc13x87::x87_mul24(
                        cc13x87::x87_mul24(idx, spacing_x), 0.15000000596046448));
                    const float b = static_cast<float>(static_cast<double>(spacing_x) * pd);
                    x = static_cast<float>(cc13x87::x87_add24(cc13x87::x87_mul24(a, rest),
                                                              cc13x87::x87_mul24(b, m)));
                    const float c = static_cast<float>(cc13x87::x87_mul24(
                        cc13x87::x87_mul24(idx, dz), 0.800000011920929));
                    const float d = static_cast<float>(static_cast<double>(dz) * pd);
                    z = static_cast<float>(cc13x87::x87_add24(cc13x87::x87_mul24(d, m),
                                                              cc13x87::x87_mul24(c, rest)));
                } else {
                    // 007F27EA-007F280A: (0.7 m (D8 C9) + 0.3) * (pair*sx);
                    // 007F280E-007F2822: (0.25 m + 0.75) * (pair*dz), each op PC24.
                    x = static_cast<float>(cc13x87::x87_mul24(
                        cc13x87::x87_add24(cc13x87::x87_mul24(0.699999988079071, m),
                                           0.30000001192092896),
                        cc13x87::x87_mul24(pair, spacing_x)));
                    z = static_cast<float>(cc13x87::x87_mul24(
                        cc13x87::x87_add24(cc13x87::x87_mul24(m, 0.25), 0.75),
                        cc13x87::x87_mul24(pair, dz)));
                }
            } else {
            y = static_cast<float>(pd * dy * md);                                  // 007F2832
            if (in.symmetrical_position) {
                // 007F2751-007F27DC: blend, by morale, the pair spacing with one
                // scaled by the raw formation index ([ESP+C4h], the argument):
                // 0.15 (00CE6618) laterally, 0.8 (00CE3D40) along z. Each product
                // is stored to a float before the blend.
                const double idx = static_cast<double>(static_cast<float>(in.formation_index));
                const double rest = 1.0 - md;
                const float a = static_cast<float>(spacing_x * idx * 0.15000000596046448);
                const float b = static_cast<float>(static_cast<double>(spacing_x) * pd);
                x = static_cast<float>(b * md + a * rest);                          // 007F27A8
                const float c = static_cast<float>(idx * dz * 0.800000011920929);
                const float d = static_cast<float>(static_cast<double>(dz) * pd);
                z = static_cast<float>(d * md + c * rest);                          // 007F2824
            } else {
                // 007F27DE-007F2822: 0.7 (00CEFFA0) m + 0.3 (00CE3DC8) laterally,
                // 0.25 (00D7A348) m + 0.75 (00CEC9D8) along z.
                x = static_cast<float>(pd * spacing_x *
                                       (0.699999988079071 * md + 0.30000001192092896));
                z = static_cast<float>(pd * dz * (md * 0.25 + 0.75));
            }
            }
            break;
        case 4: {
            // 007F26AD-007F2734, then shape 2's y clamp and flag (007F2647-).
            if constexpr (kPc24PlaneTasksCc13Bound) {
                // 007F26D5-007F26F5: DC C9 FMUL m*0.8, FADD 0.2, times pair*dz;
                // 007F26F9-007F2703: DC EA FSUB m - 1, DE C9 FMULP 0.8*, DE C1 FADDP
                // + 1; then 007F270C-007F2716 and 007F2724-007F272E with shape 2's
                // clamp. Each op PC24.
                z = static_cast<float>(cc13x87::x87_mul24(
                    cc13x87::x87_add24(cc13x87::x87_mul24(m, 0.800000011920929),
                                       0.20000000298023224),
                    cc13x87::x87_mul24(pair, dz)));
                const float wide = static_cast<float>(cc13x87::x87_add24(
                    cc13x87::x87_mul24(0.800000011920929, cc13x87::x87_sub24(m, 1.0)), 1.0));
                x = static_cast<float>(cc13x87::x87_mul24(cc13x87::x87_mul24(pair, spacing_x),
                                                          wide > 0.4f ? wide : 0.4f));
                y = static_cast<float>(cc13x87::x87_mul24(
                    cc13x87::x87_mul24(pair, dy),
                    clamp01(static_cast<float>(cc13x87::x87_sub24(
                        2.4000000953674316, cc13x87::x87_mul24(m, 1.399999976158142))))));
            } else {
            z = static_cast<float>(pd * dz * (md * 0.800000011920929 + 0.20000000298023224));
            const float wide = static_cast<float>((md - 1.0) * 0.800000011920929 + 1.0);
            x = static_cast<float>(pd * spacing_x * (wide > 0.4f ? wide : 0.4f));  // 00CE7804
            y = static_cast<float>(pd * dy * clamp01(static_cast<float>(
                    2.4000000953674316 - md * 1.399999976158142)));                 // 00D08B20, 00D045F0
            }
            keep_yz = 0.20000000298023224 > md;
            break;
        }
        default:
            // 007F2838-007F284F, shape 5: the pair spacing laterally, y = z = 0.
            x = pair * spacing_x;
            break;
    }

    // 007F2853-007F28B9, the tail. Everything here is under the odd test; an
    // even index is transformed as it stands.
    if ((in.formation_index & 1) != 0) {
        // 007F285F-007F286D, unconditional inside the odd arm.
        x = kPlaneFormationMirrorBase - x;
        // 007F2873-007F2890: unless BL, the mirror runs when the singleton's
        // +3E9h byte is zero.
        if (!keep_yz && !in.symmetrical_altitude) {
            y = kPlaneFormationMirrorBase - y;
        }
        // 007F2896-007F28B9: unless [ESP+1Bh], the same for +3E8h.
        if (!keep_yz && !in.symmetrical_position) {
            z = kPlaneFormationMirrorBase - z;
        }
    }

    out.produced = true;
    out.local[0] = x;
    out.local[1] = y;
    out.local[2] = z;

    // 007F28BF-007F28EF: 004142E0 against the frame, then the three results are
    // the caller's out[0..2].
    const std::array<float, 3> source{x, y, z};
    std::array<float, 3> result{};
    transform_point_004142e0(source, in.leader_frame, result);
    out.world[0] = result[0];
    out.world[1] = result[1];
    out.world[2] = result[2];
    return out;
}

}  // namespace bsp
