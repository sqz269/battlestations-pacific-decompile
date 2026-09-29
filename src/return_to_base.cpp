// Packet cc9_squadron_land_task: 007F16D0 and 006C0840, read from the listing.
// docs/CONTROLLED_UNIT.md, "The squadron's returntobase resolution, recorded".
#include "bsp/return_to_base.hpp"

namespace bsp {
namespace {
// 006C08CE..006C08E4 (and 006C0935..006C094B): the own site's side test. The
// first compare is unsigned (`CMP EBX,1` / `JA`), so any side other than 0 and
// 1 takes the site; otherwise the site's side must match, or outside
// multiplayer be 2.
bool own_site_side_ok(std::int32_t side, std::int32_t site_side, bool multiplayer) {
    if (static_cast<std::uint32_t>(side) > 1u) return true;
    if (site_side == side) return true;
    return !multiplayer && site_side == 2;
}
}  // namespace

NearestLandingSiteResult nearest_landing_site_006c0840(const NearestLandingSiteInputs& in,
    const std::vector<LandingSiteCandidate>& list) {
    NearestLandingSiteResult out;
    if (!in.head_present || !in.head_control_9d4) return out;          // 006C084F, 006C085D
    const HeadOwnSite& own = in.own;
    if (own.present) {                                                  // 006C087D
        const bool parent_alive = !(own.parent_present && own.parent_dead_5e);
        if (own.is_airfield_45 && !own.dead_5e && parent_alive) {      // 006C0899..006C08BD
            out.from_own_site = true;
            if (own_site_side_ok(in.side, own.side_54, in.multiplayer_927c90)) {
                out.node = own.airfield_block_7ac;                      // 006C08E6
            }
            return out;
        }
        if (own.is_carrier_09 && !own.dead_5e && parent_alive) {       // 006C08F5..006C0924
            out.from_own_site = true;
            if (own_site_side_ok(in.side, own.side_54, in.multiplayer_927c90)) {
                out.node = own.carrier_block_1208;                      // 006C094D
            }
            return out;
        }
    }
    // 006C0967: the air-ops list. best key -1.0 (00D7A260), no accepting best.
    float best = -1.0f;
    bool best_accepts = false;
    std::uintptr_t best_node = 0;
    bool any_key_unknown = false;
    for (const LandingSiteCandidate& c : list) {
        if (!c.block_present || !c.owner_present || c.owner_dead_5e) continue;
        if (in.local_only && c.owner_remote_5d) continue;               // 006C09B0
        if (in.need_approach_bit && !c.approach_bit_20) continue;        // 006C09CB
        if (in.side >= 0 && c.owner_side_54 != in.side) {                // 006C09DF
            if (in.multiplayer_927c90 || c.owner_side_54 < 2) continue;  // 006C09EA..006C09F8
        }
        ++out.candidates_passed;
        const bool accepts = c.accepts_6bc530;
        const float key = c.key;
        if (!c.key_known) any_key_unknown = true;
        bool take;
        if (best > key || 0.0f > best) {                                // 006C0AA7, 006C0AB0
            take = !(best_accepts && !accepts);                         // 006C0AC4..006C0ACD
        } else {
            take = accepts && !best_accepts;                            // 006C0AB7..006C0AC0
        }
        if (take) {                                                     // 006C0ACF..006C0ADB
            best = key;
            best_accepts = accepts;
            best_node = c.node;
        }
    }
    // With two or more nodes past the filters the key decides; a missing key
    // makes the answer the host's, not the image's.
    out.key_unknown = out.candidates_passed >= 2 && any_key_unknown;
    out.node = best_node;
    out.best_key = best;
    out.best_accepts = best_accepts;
    return out;
}

ReturnToBaseResult resolve_return_to_base_007f16d0(const ReturnToBaseInputs& in) {
    ReturnToBaseResult out;
    // 007F16E2..007F16FB: no descriptor, or MinWaterSpd == 0.0, is the null arm.
    if (!in.class_descriptor_35c || in.min_water_spd_198 == 0.0f) return out;
    // 007F1709..007F1720: a kamikaze head with +C24h clear.
    if (in.head_present && in.head_is_kamikaze_17 && !in.head_c24) return out;
    if (in.has_deck_369) {                                              // 007F1726
        if (in.home_arm_issues) {                                       // 007F173A..007F176E
            out.arm = ReturnToBaseArm::kLandAtHome;
            return out;
        }
        // 007F177E..007F17D2.
        if (in.site_node != 0 && in.site_block_present && !in.site_block_refuses_6bc120) {
            out.arm = ReturnToBaseArm::kLandAtSite;
            return out;
        }
    }
    // 007F17E2..007F18E6: the nearest border zone of the squadron's side.
    if (!in.zone_found) return out;
    const float* c = in.zone_corners_xz;
    // Each sum is stored through binary32 (007F182C..007F18BD), starting from
    // 0.0 + A; x takes +10h,+1Ch,+28h,+34h and z +18h,+24h,+30h,+3Ch.
    float sx = 0.0f + c[0];
    float sz = 0.0f + c[1];
    sx = c[2] + sx;
    sz = c[3] + sz;
    sx = c[4] + sx;
    sz = c[5] + sz;
    sx = c[6] + sx;
    sz = c[7] + sz;
    out.arm = ReturnToBaseArm::kRetreat;
    out.retreat_x = static_cast<float>(static_cast<double>(sx) * 0.25);   // 00D7A348
    out.retreat_z = static_cast<float>(static_cast<double>(sz) * 0.25);
    return out;
}

}  // namespace bsp
