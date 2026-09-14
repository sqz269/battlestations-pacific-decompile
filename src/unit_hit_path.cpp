// Reconstruction of the impact-to-damage path and the damage-control repair
// tick. Evidence per claim is in docs/UNIT_HIT_PATH.md; the call sites the host
// methods stand for are listed in reports/unit_hit_path.json.
//
// Coverage is not uniform: 008777D0, 00470510, 004705C0, 00879810, 0093C770,
// 00878340, 0080E410 and 0080E440 are complete; 0093C860 leaves its
// "destroyed" dispatch (0093C9AE..0093C9F5) unmodelled and 0087BCC0 is modelled
// only for its health block and part-count divide (0087BCC0..0087BEA4).
#include "bsp/unit_hit_path.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

float hull_damage_00470510(const HitRecord& hit, float armour_scaled) noexcept
{
    // 00470510: local_c = hit+14h, scaled by the owner modifier when the
    // modifier system is live, then (local_c - armour) * weaponScale. There is
    // no floor here; 008777D0 supplies the `> 0` test.
    const float base = hit.owner_modifier * hit.hull_damage_base;
    return (base - armour_scaled) * hit.weapon_scale;
}

float part_damage_004705c0(const HitRecord& hit, float armour_scaled, int part_hit_index) noexcept
{
    const float base = hit.owner_modifier * hit.part_damage_base;

    float falloff = 0.0f;
    if (hit.part_hits != nullptr && part_hit_index >= 0 && part_hit_index < hit.part_hit_count) {
        // 1 - entry[i].distance / hit+24h. The native divides without guarding
        // a zero range, so a zero range is left to the same float semantics.
        falloff = 1.0f - hit.part_hits[part_hit_index].distance / hit.falloff_range;
    }
    if (hit.ignore_falloff && falloff > 0.0f) {
        falloff = 1.0f;
    }

    float damage = falloff * base - armour_scaled;
    if (!(damage > 0.0f)) {
        damage = 0.0f;
    }
    return damage * hit.weapon_scale;
}

float health_delta_to_damage_00879810(float delta) noexcept
{
    // MOVSS XMM0,[00D7A208] / SUBSS XMM0,[ESP+4]: literally -0.0f - delta, not
    // a unary negation, so a delta of +0.0f still produces -0.0f.
    return kUnitHitNegativeZero - delta;
}

// Partial value projection only; see native_unit_health_parts.cpp for the
// complete caller, actual borrowed fields, signed range rule and providers.
UnitInitialCondition unit_initial_condition_0087bcc0(float class_hit_points,
                                                     float class_armour,
                                                     std::size_t part_vector_begin,
                                                     std::size_t part_vector_end) noexcept
{
    UnitInitialCondition condition;
    condition.max_health = class_hit_points; // 0087BD01, desc+48h
    condition.health = class_hit_points;     // 0087BD11, the same XMM0
    condition.armour = class_armour;         // 0087BCF4, desc+4Ch
    condition.one_marker = 1.0f;             // 0087BD09
    condition.condition_flag = true;         // 0087BD19
    // 0087BD25: a null begin pointer short-circuits the divide to zero.
    condition.part_count = (part_vector_begin == 0 || part_vector_end < part_vector_begin)
                               ? 0
                               : (part_vector_end - part_vector_begin) / kVehicleClassPartDescStride;
    return condition;
}

// Partial value choice; detail is part-set virtual+8 argument two, not a
// parameter to unit virtual+190. Does not reproduce either x87 load/store.
float unit_part_detail_0087bcc0(bool is_kind_1b) noexcept
{
    return is_kind_1b ? kUnitPartDetailKind1B : 1.0f;
}

float hull_repair_rate_0093c770(const RepairTask& task, const RepairSettings& settings,
                                float gameplay_modifier) noexcept
{
    // 0093C776..0093C79E, then the modifier multiply at 0093C7DE.
    float rate = 0.0f;
    if (task.hull_enabled) {
        rate = (task.kind == kRepairTaskKindHullSettingsRate) ? settings.hull_kind_rate : 1.0f;
    }
    return gameplay_modifier * rate;
}

float hull_repair_amount_0093c770(const RepairTask& task, const RepairSettings& settings,
                                  float gameplay_modifier, float dt, float max_health) noexcept
{
    // 0093C802..0093C815, in the order the x87 multiplies: task+28h, dt,
    // settings+3B4h, max health, rate.
    const float rate = hull_repair_rate_0093c770(task, settings, gameplay_modifier);
    return task.hull_rate * dt * settings.hull_scale * max_health * rate;
}

float subobject_repair_amount_0093c860(const RepairTask& task, const RepairSettings& settings,
                                       float gameplay_modifier, float dt,
                                       float child_max_health) noexcept
{
    // 0093C880..0093C957. The per-child rate comes from task+24h == 2 and the
    // same gameplay-modifier product as the hull step.
    float rate = (task.kind == kRepairTaskKindSubobjectSettingsRate) ? settings.subobject_kind_rate
                                                                     : 1.0f;
    rate = gameplay_modifier * rate;
    return settings.subobject_scale * dt * child_max_health * rate;
}

bool repair_overshoot_clamp(float health, float max_health) noexcept
{
    return health > max_health;
}

bool subobject_repair_complete(float health, float max_health) noexcept
{
    return max_health <= health;
}

bool repair_completion_logged_0093ca20(const RepairTask& task) noexcept
{
    // 0093CA6C..0093CAAF: kind 3 or 4, both amounts exactly zero, and the byte
    // at +44h still clear.
    const bool kind_matches =
        task.kind == kRepairTaskKindLoggedA || task.kind == kRepairTaskKindLoggedB;
    return kind_matches && task.fire_amount == 0.0f && task.leak_amount == 0.0f &&
           !task.reported_repaired;
}

// ---------------------------------------------------------------------------
// Sequences
// ---------------------------------------------------------------------------

namespace {

// The armour modifier both passes of 008777D0 build: ProductForUnit(2, x) with
// x chosen by IsKindOf(20h), and 1.0f when the modifier system is off.
float hit_armour_modifier(UnitHitPathHost& host, std::uint32_t unit)
{
    const bool owner_scoped = host.unit_is_kind_of(unit, kUnitKindOwnerScopedModifier);
    if (!host.gameplay_modifiers_enabled(2)) {
        return 1.0f;
    }
    const std::uint32_t subject = owner_scoped ? host.unit_owner_scoped_unit(unit) : unit;
    return host.gameplay_modifier_product(2, subject);
}

} // namespace

HitApplication apply_hit_record_008777d0(UnitHitPathHost& host, std::uint32_t unit,
                                         std::uint32_t hit_record, const HitRecord& hit)
{
    HitApplication applied;

    // Hull pass, 008777DD..008778D4.
    if (hit.hull_segment != kHitRecordNoHullSegment) {
        float armour;
        if (host.unit_is_kind_of(unit, kUnitKindAlternateArmourSource) &&
            hit.armour_selector < 0.0f) {
            armour = host.unit_alternate_armour(unit);
        } else {
            armour = host.unit_armour(unit);
        }

        const float damage = hull_damage_00470510(hit, armour * hit_armour_modifier(host, unit));
        if (damage > 0.0f) {
            host.write_applied_damage(hit_record, damage); // 008778BF
            host.unit_add_damage(unit, damage);            // 008778D2
            applied.hull_applied = true;
            applied.hull_damage = damage;
        }
    }

    // Part pass, 008778E4..00877A37. The loop keeps the largest value it sees,
    // seeded with -FLT_MAX, and its index, seeded with -1.
    float worst = kUnitHitWorstPartSeed;
    int worst_index = -1;
    for (int i = 0; i < hit.part_hit_count && hit.part_hits != nullptr; ++i) {
        const HitPartEntry& entry = hit.part_hits[i];
        if (entry.part_index == kHitPartEntryNoPart) {
            continue; // 00877905
        }
        const float armour = (entry.kind == kUnitHitPartEntryAlternateArmour)
                                 ? host.unit_alternate_armour(unit)
                                 : host.unit_armour(unit);
        const float damage =
            part_damage_004705c0(hit, armour * hit_armour_modifier(host, unit), i);
        if (damage > worst) {
            worst = damage;
            worst_index = i;
        }
    }

    if (worst_index > -1 && worst > kUnitHitZero) {
        host.write_applied_damage(hit_record, worst); // 00877A24
        host.unit_add_damage(unit, worst);            // 00877A37
        applied.part_applied = true;
        applied.part_damage = worst;
    }
    applied.worst_part_hit = worst_index;
    return applied;
}

void apply_health_delta_00879810(UnitHitPathHost& host, std::uint32_t unit, float delta)
{
    host.unit_apply_damage(unit, health_delta_to_damage_00879810(delta)); // 00879824
}

void repair_hull_0093c770(UnitHitPathHost& host, std::uint32_t unit, const RepairTask& task,
                          float gameplay_modifier, float dt)
{
    const RepairSettings settings = host.game_settings();
    const UnitHealth before = host.read_health(unit);
    const float amount =
        hull_repair_amount_0093c770(task, settings, gameplay_modifier, dt, before.max_health);

    apply_health_delta_00879810(host, unit, amount); // 0093C820

    const UnitHealth after = host.read_health(unit);
    if (repair_overshoot_clamp(after.current_health, after.max_health)) {
        host.unit_set_health(unit, after.max_health); // 0093C844
    }
}

void repair_subobjects_0093c860(UnitHitPathHost& host, std::uint32_t unit, const RepairTask& task,
                                float gameplay_modifier, float dt)
{
    const RepairSettings settings = host.game_settings();
    const int count = host.repairable_child_count(unit);

    for (int i = 0; i < count; ++i) {
        const std::uint32_t child = host.repairable_child(unit, i);

        // 0093C8F8..0093C91D: IsKindOf(4) and not IsKindOf(0Fh) and byte +378h.
        if (!host.unit_is_kind_of(child, kUnitKindRepairableChild)) {
            continue;
        }
        if (host.unit_is_kind_of(child, kUnitKindRepairExcluded)) {
            continue;
        }
        if (!host.unit_condition_flag(child)) {
            continue;
        }

        const UnitHealth before = host.read_health(child);
        const float amount = subobject_repair_amount_0093c860(task, settings, gameplay_modifier, dt,
                                                              before.max_health);
        apply_health_delta_00879810(host, child, amount); // 0093C957

        const UnitHealth after = host.read_health(child);
        if (subobject_repair_complete(after.current_health, after.max_health)) {
            host.unit_set_health(child, after.max_health); // 0093C99A
            // 0093C9AE..0093C9F5, the "destroyed" string built for the child's
            // vtable[19Ch] and released through the sized-storage pool, is a
            // contract: unread. Nothing is modelled for it here.
        }
    }
}

void restore_full_health_00878340(UnitHitPathHost& host, std::uint32_t unit)
{
    const UnitHealth health = host.read_health(unit);
    host.unit_set_health(unit, health.max_health); // 0087834A
}

void reset_condition_0080e410(UnitHitPathHost& host, std::uint32_t unit)
{
    restore_full_health_00878340(host, unit); // 0080E413
    host.write_condition_reset(unit);         // 0080E422..0080E438
}

void detach_part_0080e440(UnitHitPathHost& host, std::uint32_t unit, int part_index)
{
    const float impulse[3] = {0.0f, 0.0f, 0.0f}; // 0080E443..0080E461
    host.detach_part(unit, part_index, impulse); // 0080E467
}

} // namespace bsp
