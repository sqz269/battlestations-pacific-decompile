// 0070DB60, reconstructed from the listing. docs/SHIP_POST_MOTION.md section 13.
#include "bsp/unit_group_slot_swap.hpp"

#include <cmath>
#include <utility>

namespace bsp {

namespace {

// 0070E1B1..0070E1E9 (and three copies): b*b + a*a stored to a float at
// [ESP+10h]; above the double 1e-10 at [00CE3820] it is square-rooted
// (00BF7030) and stored as a float, else 0.
float distance_or_zero(float a, float b) noexcept {
    const float d2 = static_cast<float>(static_cast<double>(b) * b +
                                        static_cast<double>(a) * a);
    if (!(static_cast<double>(d2) > 1.0e-10)) return 0.0f;
    return static_cast<float>(std::sqrt(static_cast<double>(d2)));
}

}  // namespace

UnitGroupSlotSwapResult unit_group_swap_slots_by_distance_0070db60(
    std::vector<ShipAiUnitGroupMember>& members, std::int32_t column,
    std::int32_t type_04fc, std::uint32_t controlled_entity, UnitGroupSlotSwapHost& host) {
    UnitGroupSlotSwapResult out;
    // 0070DB69: a group of type 18h never swaps.
    if (type_04fc == 0x18) { out.gated = true; return out; }
    const std::size_t count = members.size();                   // group+4F8h
    if (count == 0) return out;                                  // 0070DB81
    // 0070DB87..0070DBA0: nor does a group holding the controlled unit.
    for (const ShipAiUnitGroupMember& m : members) {
        if (m.entity == controlled_entity) { out.gated = true; return out; }
    }
    const std::size_t col = static_cast<std::size_t>(column);
    // 0070DBC4..0070E38A: every ordered pair i < j.
    for (std::size_t i = 0; i < count; ++i) {
        if (members[i].entity == 0) continue;                    // 0070DBC7
        for (std::size_t j = i + 1; j < count; ++j) {
            ShipAiUnitGroupMember& mi = members[i];
            ShipAiUnitGroupMember& mj = members[j];
            if (mj.entity == 0) continue;                        // 0070DBF8
            // 0070DC00..0070DC0C: the same class descriptor. Member i is
            // re-read every inner step (0070DBFE), so a swap is seen at once.
            if (host.member_class_0538(mi.entity) != host.member_class_0538(mj.entity)) continue;
            float across_i = 0.0f, along_i = 0.0f, across_j = 0.0f, along_j = 0.0f;
            const bool ok_i = host.decompose_against_leader_00811180(mi.entity, across_i, along_i);
            const bool ok_j = host.decompose_against_leader_00811180(mj.entity, across_j, along_j);
            if (!ok_i || !ok_j) { ++out.pairs_skipped_invalid; continue; }
            ++out.pairs_compared;
            // 0070E15B..0070E197: the two slots' pattern offsets at the group's
            // column, record+10h (across) and record+20h (along).
            const float lat_i = mi.lateral[col], ax_i = mi.axial[col];
            const float lat_j = mj.lateral[col], ax_j = mj.axial[col];
            // Each difference is stored as a float before squaring.
            const float d_ii = distance_or_zero(                 // [ESP+40h]
                static_cast<float>(static_cast<double>(across_i) - lat_i),
                static_cast<float>(static_cast<double>(along_i) - ax_i));
            const float d_ij = distance_or_zero(                 // [ESP+24h]
                static_cast<float>(static_cast<double>(across_i) - lat_j),
                static_cast<float>(static_cast<double>(along_i) - ax_j));
            const float d_ji = distance_or_zero(                 // [ESP+44h]
                static_cast<float>(static_cast<double>(across_j) - lat_i),
                static_cast<float>(static_cast<double>(along_j) - ax_i));
            const float d_jj = distance_or_zero(                 // [ESP+3Ch]
                static_cast<float>(static_cast<double>(across_j) - lat_j),
                static_cast<float>(static_cast<double>(along_j) - ax_j));
            // 0070E2FD..0070E321: swapped = d_ji + d_ij, kept = d_jj + d_ii,
            // each stored as a float; JBE skips unless kept > swapped.
            const float swapped = static_cast<float>(static_cast<double>(d_ji) + d_ij);
            const float kept = static_cast<float>(static_cast<double>(d_jj) + d_ii);
            if (!(kept > swapped)) continue;
            // 0070E323..0070E33E: the entity words and the +30h words trade
            // places; the pattern columns stay with the slots.
            std::swap(mi.entity, mj.entity);
            std::swap(mi.field_30, mj.field_30);
            ++out.swaps;
        }
    }
    return out;
}

}  // namespace bsp
