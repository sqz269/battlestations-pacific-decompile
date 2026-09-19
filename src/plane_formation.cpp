// 007ED260 and 007F23A0, read from the listing. docs/PLANE_FORMATION.md.

#include "bsp/plane_formation.hpp"

#include <array>

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
    // Only case 0 of the table - shape 1, the value 007F2C60 seeds - is read.
    // The other four bodies (007F25BD, 007F26AD, 007F27DE, 007F2838) are in the
    // listing and are NOT reconstructed here: this is a hole, not a proof.
    if (in.shape != 1) return out;

    // 007F250E-007F2544: the lateral spacing is the larger of the triple's
    // first component and the leader class's +A4h times the double at 00D049A8.
    // `FCOMIP` then `JA` keeps the displacement when it is strictly greater.
    const float width_floor =
        static_cast<float>(static_cast<double>(in.leader_class_width)
                           * kPlaneFormationWidthSpacingMul);
    const float spacing_x =
        (in.displacement[0] > width_floor) ? in.displacement[0] : width_floor;

    // 007F2576-007F25B8, case 0. Each component is the pair number times its
    // own displacement times the squadron's morale.
    const float pair = static_cast<float>(pair_number(in.formation_index));
    float x = pair * spacing_x * in.morale;
    float y = pair * in.displacement[1] * in.morale;
    float z = pair * in.displacement[2] * in.morale;

    // 007F2853-007F28B9, the tail. Everything here is under the odd test; an
    // even index is transformed as it stands.
    if ((in.formation_index & 1) != 0) {
        // 007F285F-007F286D, unconditional inside the odd arm.
        x = kPlaneFormationMirrorBase - x;
        // 007F2873-007F2890. `BL` is the per-shape flag that only the shape-2
        // body sets (007F26A2), so it is 0 here; the mirror then runs unless
        // the singleton's +3E9h byte is non-zero.
        if (!in.symmetrical_altitude) {
            y = kPlaneFormationMirrorBase - y;
        }
        // 007F2896-007F28B9. The same for +3E8h, under the same shape flag,
        // which the shape-1 body leaves at the 0 of 007F2564.
        if (!in.symmetrical_position) {
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
