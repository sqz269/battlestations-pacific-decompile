// Reconstruction of the ship family's vtable[ECh] hit-record handler.
// Evidence per claim is in docs/SHIP_HIT_RECORD.md; the call sites the host
// methods stand for are listed in reports/ship_hit_record.json.
//
// Coverage: 00826F10 is complete, every branch of the listing
// 00826F10-0082781B modelled. 0092D1F0 is complete apart from its bounds-check
// calls into 00BF6713, which abort in the native. 004155B0 and the two count
// truncations are complete. The routines the sequence delegates to are not
// re-modelled here: 00470510 and 004705C0 are bsp/unit_hit_path.hpp, 0093BED0
// is bsp/unit_fire_flooding.hpp, 008777D0 is the host's tail method.
#include "bsp/ship_hit_record.hpp"

#include <cmath>

namespace bsp {

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

bool ship_hit_vetoed_00826f38(bool shot_present,
                              bool shot_is_depth_charge,
                              bool hull_is_submarine) noexcept
{
    // Three tests in a row, each falling through to the body when it fails:
    // no shot, or the shot is not a depth charge, both proceed. Only a depth
    // charge against a non-submarine hull takes the JZ at 00826F59.
    return shot_present && shot_is_depth_charge && !hull_is_submarine;
}

float clamp_004155b0(float value, float lo, float hi) noexcept
{
    // FCOMI at 004155C5 returns `lo` when lo > value, then FCOMI at 004155DB
    // returns `value` when value <= hi and `hi` otherwise.
    if (lo > value) {
        return lo;
    }
    return value <= hi ? value : hi;
}

int ship_hit_effect_count(float applied_damage) noexcept
{
    // FDIV against the double at 00CE3DC0, so the divide happens in double
    // precision before the clamp sees a float again.
    const float quotient =
        static_cast<float>(static_cast<double>(applied_damage) / kShipHitEffectDamagePerUnit);
    const float clamped = clamp_004155b0(quotient, 0.0f, kShipHitEffectCountMax);
    // FLDCW with bit 0xC00 set is round-toward-zero, and the FISTP is to a
    // qword whose low dword is what the code reads back.
    return static_cast<int>(clamped);
}

bool ship_difficulty_row(int session_mode,
                         bool unit_is_local_players,
                         unsigned campaign_index,
                         unsigned* row) noexcept
{
    if (session_mode != 0) {
        // 0082707E. Every non-campaign session uses row 2 whoever owns the hull.
        *row = 2;
        return true;
    }
    if (unit_is_local_players) {
        // 00827085. Only the local player's own hull is scaled in a campaign.
        *row = campaign_index;
        return true;
    }
    *row = 0;
    return false;
}

ShipDifficultyScaling ship_difficulty_scaled_hull_damage(float hull_damage,
                                                         int session_mode,
                                                         bool unit_is_local_players,
                                                         unsigned campaign_index,
                                                         const float* difficulty_table,
                                                         std::size_t difficulty_table_size) noexcept
{
    ShipDifficultyScaling out;
    out.damage = hull_damage;

    unsigned index = 0;
    if (!ship_difficulty_row(session_mode, unit_is_local_players, campaign_index, &index)) {
        return out;
    }

    // 00827092..008270A3: a null vector or an index past the end calls
    // 00BF6713, which does not return. Nothing is scaled if we get there.
    if (difficulty_table == nullptr || index >= difficulty_table_size) {
        return out;
    }

    out.scaled = true;
    out.index = index;
    out.damage = difficulty_table[index] * hull_damage;
    return out;
}

int ship_roll_direction_sign(const float roll_axis[3], float impact_dir_x, float impact_dir_z) noexcept
{
    // 00827263..00827277: axis.z * dir.x - axis.x * dir.z, the y component of
    // the cross product of the two horizontal vectors.
    const float cross = roll_axis[2] * impact_dir_x - roll_axis[0] * impact_dir_z;
    // 00827285 tests 0 > cross first, so a cross of exactly zero yields zero
    // and the whole torque vanishes.
    if (cross < 0.0f) {
        return -1;
    }
    return cross > 0.0f ? 1 : 0;
}

ShipRollTorque ship_roll_torque(const float roll_axis[3],
                                const float impact_dir[3],
                                float hull_damage_no_armour,
                                float class_mass,
                                float torque_scale,
                                float mass_root) noexcept
{
    // 0082715A..008271AE: the length uses all three components but only x and
    // z are divided by it, and the y component is never read again.
    const float length = std::sqrt(impact_dir[0] * impact_dir[0] + impact_dir[1] * impact_dir[1] +
                                   impact_dir[2] * impact_dir[2]);
    const float dir_x = impact_dir[0] / length;
    const float dir_z = impact_dir[2] / length;

    // 008271E8..00827245: a zero mass short-circuits to zero, a negative mass
    // is negated with -0.0f - mass (the 00D7A208 literal), and the x87
    // FYL2X / F2XM1 / FSCALE run is pow(|mass|, 1 / mass_root).
    float mass_term = 0.0f;
    if (class_mass != 0.0f) {
        const float magnitude = class_mass < 0.0f ? (-0.0f - class_mass) : class_mass;
        mass_term = std::pow(magnitude, 1.0f / mass_root);
    }

    const float scale = static_cast<float>(ship_roll_direction_sign(roll_axis, dir_x, dir_z)) *
                        torque_scale * hull_damage_no_armour * mass_term;

    ShipRollTorque out;
    out.x = roll_axis[0] * scale;
    out.y = roll_axis[1] * scale;
    out.z = roll_axis[2] * scale;
    return out;
}

ShipPartDamageResult ship_part_damage_0092d1f0(float health,
                                               float damage,
                                               signed char enable_flag) noexcept
{
    ShipPartDamageResult out;
    out.health = health;

    // 0092D210: JL on the signed byte, so a negative flag means the part takes
    // nothing at all.
    if (enable_flag < 0) {
        return out;
    }
    // 0092D248: the sentinel doubles as the "already destroyed" test, so a part
    // sitting at exactly -10000.0f is skipped.
    if (!(health > kShipPartDestroyedHealth)) {
        return out;
    }

    out.applied = true;
    out.health = health - damage;
    // 0092D29B compares against 0.0f (00D7A218) with JA to skip, so reaching
    // exactly zero destroys the part.
    if (!(out.health > 0.0f)) {
        out.destroyed = true;
        out.health = kShipPartDestroyedHealth;
    }
    return out;
}

// ---------------------------------------------------------------------------
// The sequence
// ---------------------------------------------------------------------------

bool apply_ship_hit_record_00826f10(ShipHitRecordHost& host,
                                    const HitRecord& hit,
                                    const ShipHitRecordView& view) noexcept
{
    // 00826F38..00826F59.
    if (ship_hit_vetoed_00826f38(view.shot_present, view.shot_is_depth_charge,
                                 host.hull_is_submarine())) {
        return false;
    }

    // 00826F62. The hull pass is skipped whole; the part loop still runs.
    if (hit.hull_segment != kHitRecordNoHullSegment) {
        // 00826F6B..00826F93. A negative selector takes the descriptor's
        // virtual armour instead of the instance's own.
        const float armour =
            hit.armour_selector >= 0.0f ? host.hull_armour() : host.class_armour_virtual();

        // 00826FA3, then 0082704D recomputes it. The first value is what the
        // part subtraction below sees, the second is what everything after the
        // difficulty scaling sees.
        const float raw_hull_damage = host.hull_damage(armour);
        const float class_mass = host.class_mass();

        // 00826FAC..0082703E.
        if (view.segment_kind == kShipHitSegmentKindBreakable && raw_hull_damage > 0.0f &&
            static_cast<double>(class_mass) >= kShipHitPartDamageMassFloor) {
            float direction[3] = {0.0f, 0.0f, 0.0f};
            if (view.shot_present) {
                host.impact_direction(direction);
            }
            host.apply_part_damage(hit.hull_segment, direction, raw_hull_damage);
        }

        // 00827043..008270B7.
        const int session_mode = host.session_mode();
        float hull_damage = host.hull_damage(armour);
        {
            unsigned row = 0;
            const bool local = session_mode == 0 && host.unit_is_local_players();
            if (ship_difficulty_row(session_mode, local,
                                    local ? host.campaign_difficulty_index() : 0u, &row)) {
                hull_damage = host.difficulty_multiplier(row) * hull_damage;
            }
        }

        // 008270BE.
        host.clear_hit_accumulator();

        // 008270C6: everything to 0082742B needs the shot.
        if (view.shot_present) {
            // 008270E5..00827339. Torpedoes only, heavy hulls only, and only in
            // a session mode of 0 or 1.
            if (view.shot_is_torpedo &&
                static_cast<double>(class_mass) > kShipHitRollTorqueMassFloor &&
                (session_mode == 0 || session_mode == 1)) {
                // 0082712E: the same formula with no armour at all.
                const float unarmoured = host.hull_damage(0.0f);
                float direction[3] = {0.0f, 0.0f, 0.0f};
                host.impact_direction(direction);
                float axis[3] = {0.0f, 0.0f, 0.0f};
                host.roll_axis(axis);
                host.route_add_hull_torque(ship_roll_torque(axis, direction, unarmoured, class_mass,
                                                            host.settings_roll_torque_scale(),
                                                            host.settings_roll_mass_root()));
            }

            // 0082733C..0082742B.
            if (view.weapon_present) {
                // 0082734C and 00827363 call the same virtual twice; the record
                // keeps the first result and the message carries the second.
                host.record_flood_rate(host.weapon_water_damage());
                host.route_set_damage_channel(ShipDamageChannel::kWater, host.weapon_water_damage(),
                                              true);

                // 0082738E..008273DF. A hit that got through the armour rolls
                // for a fire; one that did not has a chance of zero.
                const float chance = hull_damage > 0.0f ? host.weapon_fire_chance() : 0.0f;
                if (chance > host.random_unit_float()) {
                    host.record_fire_rate(host.weapon_fire_damage());
                    host.route_set_damage_channel(ShipDamageChannel::kFire,
                                                  host.weapon_fire_damage(), true);
                }
            }
        }

        // 00827432..00827450.
        if (hull_damage > 0.0f) {
            host.roll_component_failure(hull_damage);
        }

        // 00827455..00827497.
        if (host.is_local_players_unit()) {
            float shooter[3] = {0.0f, 0.0f, 0.0f};
            if (host.shooter_world_position(shooter)) {
                host.push_damage_direction(shooter);
            }
        }

        // 0082749C..0082757B.
        if (hull_damage > 0.0f && !host.hull_is_submarine()) {
            const int count = ship_hit_effect_count(hull_damage);
            if (count != 0) {
                host.route_hull_impact_effect(count, view.impact_point);
            }
        }
    }

    // 00827582..008277F3. The native dereferences hit+3Ch without a null test;
    // a count above zero with no array is a fault there, and nothing here.
    const int part_hit_count = hit.part_hits != nullptr ? hit.part_hit_count : 0;
    for (int i = 0; i < part_hit_count; ++i) {
        // 008275A0: the part pass always uses the instance's own armour, never
        // the descriptor's virtual one.
        const float part_damage = host.part_damage(host.hull_armour(), i);
        const HitPartEntry& entry = hit.part_hits[i];

        if (entry.kind == kShipHitSegmentKindBreakable) {
            // 008275D7: a part hit of this kind that did no damage ends the
            // iteration without reaching the effect block.
            if (!(part_damage > 0.0f)) {
                continue;
            }
            // 008275F3 falls through to the effects when the mass is too low.
            if (static_cast<double>(host.class_mass()) >= kShipHitPartDamageMassFloor) {
                float direction[3] = {0.0f, 0.0f, 0.0f};
                if (view.shot_present) {
                    host.impact_direction(direction);
                }
                host.apply_part_damage(entry.part_index, direction, part_damage);
            }
        }

        // 00827663..008277C1.
        if (part_damage > 0.0f && !host.hull_is_submarine()) {
            const int count = ship_hit_effect_count(part_damage);
            if (count != 0) {
                host.route_part_impact_effect(count, view.impact_point);
            }
        }
    }

    // 008277FC.
    return host.apply_base_hit_record();
}

} // namespace bsp
