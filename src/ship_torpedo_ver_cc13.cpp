// Packet cc13-ver23: ship and submarine torpedo launch gates, image side.
// Names are hypotheses; addresses, ABI and evidence are in
// include/bsp/ship_torpedo_ver_cc13.hpp and docs/CC13_VER_23.md.

#include "bsp/ship_torpedo_ver_cc13.hpp"

#include "bsp/avoid_zone_clearance.hpp"
#include "bsp/ship_ai_settings_block.hpp"

namespace bsp {
namespace ship_torpedo_ver_cc13 {

float torpedo_attack_range_00901ba0(const WeaponHitAccuracyProfile& torpedo_profile,
    const TorpedoBotParameters& level_row, float target_size,
    bool fire_target_06c, float tube_class_range_00731020) noexcept {
    // 00901BED..00901C0B: 008387B0(7, size, range, accuracy). Kind 7 is the
    // Torpedo arm (0083883A ADD ECX,2F0h): 00838530(profile, size, accuracy)
    // FMUL range, stored to a float.
    const auto hit_accuracy = [&torpedo_profile](int kind, float size, float range,
                                  float accuracy) {
        WeaponHitAccuracyProfile profiles[4]{};
        profiles[static_cast<int>(WeaponHitAccuracyCategory::Torpedo)] = torpedo_profile;
        return weapon_hit_accuracy_scaled_008387b0(profiles, kind, size, range, accuracy);
    };
    TorpedoBotParameters row = level_row;
    return gameplay_seg40_59_cc13::torpedo_bot_attack_range_00901ba0(row, target_size,
        fire_target_06c, tube_class_range_00731020, hit_accuracy);
}

void torpedo_gun_set_bot_fire_target_00727f10(TorpedoBotRangeCache& cache, bool target_changed,
    bool flag, const std::function<float(bool fire_target_06c)>& range_for_flag) {
    // 00727F44 -> 009035A0: 009035C3 recomputes on a target change, with the
    // flag the bot already holds.
    if (target_changed) {
        cache.attack_range_068 = range_for_flag(cache.fire_target_06c);
        ++cache.recomputes;
    }
    // 00727F56 -> 00903580: 00903588 CMP [ESI+6Ch],BL; 0090358D recomputes;
    // 00903592 stores the new flag after it.
    if (cache.fire_target_06c != flag) {
        cache.attack_range_068 = range_for_flag(cache.fire_target_06c);
        ++cache.recomputes;
        cache.fire_target_06c = flag;
    }
}

bool torpedo_bot_drops_submerged_target_008fffb2(const SubmergedTargetInputs& in) noexcept {
    if (!in.has_target || !in.target_is_submarine_8) return false;  // 008FFFB0, 008FFFC6
    if (!in.owner_found_5) return false;                             // 00900005
    if (in.owner_is_submarine_8) return false;                       // 00900014
    if (in.target_above_00852820) return false;                      // 0090001F
    return true;                                                     // 0090002A vt[38h](0)
}

bool avoid_zone_segment_clear_00416cd0(const AvoidZonePolygon& zone,
    const std::array<float, 2>& p0, const std::array<float, 2>& p1) noexcept {
    // 00416CE9: 004F2B00 adjusts ECX to zone+14h and reaches 0085C910.
    if (!avoid_zone_box_meets_segment_0085c910(zone.bounds_min, zone.bounds_max, p0, p1)) {
        return true;
    }
    if (avoid_zone_contains_point_00416b50(zone, p0)) return false;  // 00416CF9
    if (avoid_zone_contains_point_00416b50(zone, p1)) return false;  // 00416D09
    if (zone.corners.empty()) return true;                           // 00416D3E
    // 00416D1B: prev starts at the last corner; 00416D69 tests (prev, c)
    // against (p0, p1); 00416D77..00416D91 prev = c.
    std::array<float, 2> prev = zone.corners.back().position;
    for (const AvoidZoneCorner& corner : zone.corners) {
        const std::array<float, 2> cur = corner.position;
        float out[2] = {0.0f, 0.0f};
        if (native_segment_crossing_004f3730(prev.data(), cur.data(), p0.data(), p1.data(),
                out)) {
            return false;  // 00416D70 JNE 00416DA4
        }
        prev = cur;
    }
    return true;
}

bool avoid_zone_group_segment_clear_00417940(const AvoidZoneLayerGroup& group,
    const std::array<float, 2>& p0, const std::array<float, 2>& p1) noexcept {
    for (const AvoidZonePolygon& zone : group.zones) {
        if (!avoid_zone_segment_clear_00416cd0(zone, p0, p1)) return false;  // 0041796B
    }
    return true;  // 0041797D
}

bool torpedo_launch_zone_clear_0090088e(const AvoidZoneTable* table,
    const std::array<float, 2>& gun_xz, const std::array<float, 2>& lead_xz) noexcept {
    if (table == nullptr) return true;
    // 00900893 004120D0(5). The projection answers -1 for an empty table,
    // where the native reads slot 0; a group with no zones passes either way.
    const std::int32_t group = avoid_zone_group_for_layer_004120d0(*table,
        kTorpedoLaunchZoneLayer);
    if (group < 0) return true;  // 0090089E JE 009008F1
    // 009008DF: 00417940(&gun, &lead); 009008E6 SETE / 009008EB JNE 00900974.
    return avoid_zone_group_segment_clear_00417940(
        table->groups[static_cast<std::size_t>(group)], gun_xz, lead_xz);
}

}  // namespace ship_torpedo_ver_cc13
}  // namespace bsp
