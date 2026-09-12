// Ship fire, flooding and damage control. docs/UNIT_FIRE_AND_REPAIR.md,
// reports/unit_fire_flooding.json. Packet cc2_fire_flooding.
//
// Every descriptive name is a hypothesis, not a recovered symbol. Nothing here
// is a binary-compatible replacement; the native routines are __thiscall over
// layouts this file does not reproduce.
#include "bsp/unit_fire_flooding.hpp"

namespace bsp {
namespace {

// The literal format string at 00D196F8, read at 0093CACC.
const char* const kRepairedFormat = "Ship Repaired Fire && Leak %s Hp %6f %f";

// One shared body for 0093C120 and 0093C210: they differ only in which timer,
// which rate and which priority value they use.
TimerDamageResult timer_damage_step(float seconds, float rate, float divisor, float dt) {
    TimerDamageResult out;
    const float before = seconds;
    float remaining = before - dt;
    out.expired_this_tick = remaining < 0.0f;
    if (out.expired_this_tick) {
        remaining = 0.0f;
    }
    out.seconds_remaining = remaining;
    const float elapsed = before - remaining;
    if (elapsed > 0.0f) {
        out.damage = (elapsed * rate) / divisor;
    }
    return out;
}

}  // namespace

float repair_modifier_factor(bool modifier_manager_present,
                             bool modifier_manager_enabled,
                             float modifier_product) {
    // The whole factor is skipped when 00F88C30 is null; otherwise it is the
    // product only when 00E0C978 and [00F88C30+ACh] are both set.
    if (!modifier_manager_present) {
        return kRepairUnitScale;
    }
    return modifier_manager_enabled ? modifier_product : kRepairUnitScale;
}

float hull_repair_amount_0093c770(const RepairTaskState& task,
                                  const DamageControlSettings& settings,
                                  float unit_max_health,
                                  float modifier,
                                  float dt) {
    float base = 0.0f;
    if (task.hull_repair_enabled) {
        base = (task.priority == RepairPriority::kHull) ? settings.hull_priority_rate
                                                        : kRepairUnitScale;
    }
    const float rate = base * modifier;
    return task.effectivity * dt * settings.hull_repair_scale * unit_max_health * rate;
}

float subobject_repair_amount_0093c860(const RepairTaskState& task,
                                       const DamageControlSettings& settings,
                                       float child_max_health,
                                       float modifier,
                                       float dt) {
    const float base = (task.priority == RepairPriority::kSubObjects)
                           ? settings.subobject_priority_rate
                           : kRepairUnitScale;
    const float rate = base * modifier;
    return settings.subobject_repair_scale * dt * child_max_health * rate;
}

float failure_repair_step_0093c520(const RepairTaskState& task,
                                   const DamageControlSettings& settings,
                                   float modifier,
                                   float dt) {
    float base = 0.0f;
    if (task.failure_repair_enabled) {
        base = (task.priority == RepairPriority::kFailures) ? settings.failure_priority_rate
                                                            : kRepairUnitScale;
    }
    return base * modifier * dt;
}

TimerDamageResult fire_damage_step_0093c120(const RepairTaskState& task,
                                            const DamageControlSettings& settings,
                                            float modifier,
                                            float dt) {
    const float base = (task.priority == RepairPriority::kFire) ? settings.fire_priority_divisor
                                                                : kRepairUnitScale;
    return timer_damage_step(task.fire_seconds, task.fire_damage_rate, base * modifier, dt);
}

TimerDamageResult water_damage_step_0093c210(const RepairTaskState& task,
                                             const DamageControlSettings& settings,
                                             float modifier,
                                             float dt) {
    const float base = (task.priority == RepairPriority::kFlooding)
                           ? settings.water_priority_divisor
                           : kRepairUnitScale;
    return timer_damage_step(task.water_seconds, task.water_damage_rate, base * modifier, dt);
}

bool repair_completion_ready_0093ca20(const RepairTaskState& task) {
    const bool priority_ok = task.priority == RepairPriority::kFlooding ||
                             task.priority == RepairPriority::kFire;
    return priority_ok && task.fire_seconds == 0.0f && task.water_seconds == 0.0f &&
           !task.reported_repaired;
}

float failure_chance_0093bed0(float component_numerator,
                              float component_denominator,
                              const DamageControlSettings& settings,
                              float damage) {
    if (component_numerator < 0.0f || component_denominator < 0.0f) {
        return (settings.failure_chance_numerator * damage) /
               settings.failure_chance_denominator;
    }
    return (component_numerator * damage) / component_denominator;
}

float repair_time_delay_0093ada70(float current, float delay) {
    float value = current + delay;
    // 008ADBC4 COMISS 0,value then 008ADBD1 COMISS value,ceiling; both clamp to
    // whichever bound XMM1 holds on the taken path.
    if (value < 0.0f) {
        value = 0.0f;
    } else if (value > kRepairTimeDelayCeiling) {
        value = kRepairTimeDelayCeiling;
    }
    return value;
}

void leak_zone_sums_0074e8f0(const float* values, int count, float out_bins[kLeakZoneBins]) {
    for (int i = 0; i < kLeakZoneBins; ++i) {
        out_bins[i] = 0.0f;
    }
    if (count <= 0 || values == nullptr) {
        return;
    }
    float position = 0.0f;
    const float step = static_cast<float>(kLeakZoneCount) / static_cast<float>(count);
    for (int i = 0; i < count; ++i) {
        // The native loop does not guard the index; the increment follows the
        // store, so position only reaches 6.0 after the last element.
        const int bin = static_cast<int>(position);
        if (bin >= 0 && bin < kLeakZoneBins) {
            out_bins[bin] += values[i];
        }
        position += step;
    }
}

float leak_load_fraction_00891680(float sum, float displacement) {
    const float scaled = static_cast<float>(static_cast<double>(sum) * kLeakLoadFactor *
                                            kLeakLoadScale) /
                         displacement;
    return scaled > kRepairUnitScale ? kRepairUnitScale : scaled;
}

void repair_level_raise_00827960(int levels[kRepairCategoryCount], int index, float max_level) {
    levels[index] += 1;
    if (static_cast<float>(levels[index]) > max_level) {
        levels[index] = static_cast<int>(max_level);
        return;
    }
    if (index == 0 || levels[0] < 1) {
        // 00827974: walk down from index 5 and take one from the first category
        // other than this one that still has a level.
        for (int j = kRepairCategoryCount - 1; j > 0; --j) {
            if (j != index && levels[j] > 0) {
                levels[j] -= 1;
                return;
            }
        }
        return;
    }
    levels[0] -= 1;
}

bool repair_level_lower_00812a70(int levels[kRepairCategoryCount], int index) {
    if (index == 0) {
        return false;  // 00812A7E: the pool itself is never lowered.
    }
    levels[index] -= 1;
    if (levels[index] < 0) {
        levels[index] = 0;
        return false;  // nothing is returned to the pool, and no message is sent
    }
    levels[0] += 1;
    return true;  // the caller routes 0080FEC0(unit) on channel 0
}

void repair_level_set_00827a40(int levels[kRepairCategoryCount], int index, int target,
                               float max_level) {
    if (levels[index] < target) {
        while (levels[index] < target) {
            const int before = levels[index];
            repair_level_raise_00827960(levels, index, max_level);
            if (levels[index] == before) {
                break;  // the max-level clamp would otherwise spin
            }
        }
        return;
    }
    while (target < levels[index]) {
        const int before = levels[index];
        repair_level_lower_00812a70(levels, index);
        if (levels[index] == before) {
            break;
        }
    }
}

int repair_category_index_0081ad40(const std::string& name) {
    // 0081AD40 compares case-insensitively: __stricmp for "all", then
    // BSP_NativeString_EqualsCStringInsensitive for the rest.
    std::string lowered;
    lowered.reserve(name.size());
    for (const char c : name) {
        lowered.push_back(static_cast<char>(
            (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c));
    }
    if (lowered == "all") {
        return static_cast<int>(RepairCategory::kPool);
    }
    if (lowered == "hull") {
        return static_cast<int>(RepairCategory::kHull);
    }
    if (lowered == "firefighting") {
        return static_cast<int>(RepairCategory::kFireFighting);
    }
    if (lowered == "engine and steering" || lowered == "engine" || lowered == "steering") {
        return static_cast<int>(RepairCategory::kEngineAndSteering);
    }
    if (lowered == "weapons") {
        return static_cast<int>(RepairCategory::kWeapons);
    }
    if (lowered == "spec" || lowered == "runway" || lowered == "airsupply") {
        return static_cast<int>(RepairCategory::kSpecial);
    }
    return static_cast<int>(RepairCategory::kPool);  // 0081ADF6, the default
}

void repair_failures_0093c520(UnitFireFloodingHost& host, std::uint32_t unit,
                              RepairTaskState& task, float dt) {
    const DamageControlSettings& settings = host.game_settings();
    const float modifier = repair_modifier_factor(
        host.modifier_manager_present(), host.modifier_manager_enabled(),
        host.modifier_manager_present() ? host.gameplay_modifier_product(3, unit) : 1.0f);
    const float decrement = failure_repair_step_0093c520(task, settings, modifier, dt);

    std::size_t i = 0;
    while (i < task.failures.size()) {
        ActiveFailure& failure = task.failures[i];
        failure.seconds_remaining -= decrement;
        if (failure.seconds_remaining > 0.0f) {
            ++i;
            continue;
        }
        const std::string name = failure.name;
        host.unit_named_state_dispatch(unit, name);
        host.session_route_failure_cleared(unit, name);
        // 0093C6xx swap-erases with the last entry and shrinks through
        // 0093BA80; `i` is not advanced, so the moved entry is retried.
        task.failures[i] = task.failures.back();
        task.failures.pop_back();
    }
}

void repair_task_update_0093ca20(UnitFireFloodingHost& host, std::uint32_t unit,
                                 RepairTaskState& task, float dt) {
    const DamageControlSettings& settings = host.game_settings();
    const float modifier = repair_modifier_factor(
        host.modifier_manager_present(), host.modifier_manager_enabled(),
        host.modifier_manager_present() ? host.gameplay_modifier_product(3, unit) : 1.0f);

    // 1. 0093CA2B, hull repair.
    const float max_health = host.unit_max_health(unit);
    const float heal =
        hull_repair_amount_0093c770(task, settings, max_health, modifier, dt);
    host.apply_health_delta(unit, heal);
    if (host.unit_health(unit) > max_health) {
        host.set_health(unit, max_health);
    }

    // 2. 0093CA3A, subobject repair. The child walk is the host's; only the
    // arithmetic is modelled here, so this sequence leaves it to the caller.

    // 3. 0093CA49, the named-failure list.
    repair_failures_0093c520(host, unit, task, dt);

    // 4. 0093CA58, fire damage.
    const TimerDamageResult fire = fire_damage_step_0093c120(task, settings, modifier, dt);
    task.fire_seconds = fire.seconds_remaining;
    if (fire.expired_this_tick) {
        task.fire_expired_slot = 0;
    }
    if (fire.damage > 0.0f) {
        host.unit_add_damage(unit, fire.damage);
    }

    // 5. 0093CA67, water damage.
    const TimerDamageResult water = water_damage_step_0093c210(task, settings, modifier, dt);
    task.water_seconds = water.seconds_remaining;
    if (water.expired_this_tick) {
        task.water_expired_slot = 0;
    }
    if (water.damage > 0.0f) {
        host.unit_add_damage(unit, water.damage);
    }

    // The completion report at 0093CA88..0093CAF1.
    if (!repair_completion_ready_0093ca20(task)) {
        return;
    }
    const double health = static_cast<double>(host.unit_health(unit));
    const double reported = static_cast<double>(host.unit_get_health_0923be0(unit));
    host.log_line(kRepairedFormat, host.unit_display_name(unit), health, reported);
    task.reported_repaired = true;
    host.repair_completed_notify(unit);
}

void roll_component_failure_0093bed0(UnitFireFloodingHost& host, std::uint32_t unit,
                                     RepairTaskState& task, std::uint32_t hit,
                                     std::uint32_t owner, int segment, float damage) {
    if (!host.failure_rolls_enabled() || segment == -1) {
        return;
    }
    float numerator = 0.0f;
    float denominator = 0.0f;
    if (!host.resolve_component(owner, segment, &numerator, &denominator)) {
        return;
    }
    const DamageControlSettings& settings = host.game_settings();
    const float chance = failure_chance_0093bed0(numerator, denominator, settings, damage);
    if (host.random_unit_float() > chance) {
        return;
    }
    std::string name;
    float duration = 0.0f;
    if (!host.failure_descriptor_for(owner, &name, &duration)) {
        return;
    }
    if (host.failure_already_active(name)) {
        return;
    }
    host.session_route_failure_started(owner, segment);
    ActiveFailure record;
    record.id = static_cast<std::int32_t>(owner);
    record.name = name;
    record.seconds_remaining = duration;
    task.failures.push_back(record);
    host.warning_fire_failure(unit, name, duration);
    host.failure_side_effect(unit, hit, owner);
    host.unit_on_failure(unit, name, hit);
    task.reported_repaired = false;  // 0093C0F6 clears task+44h
}

void pick_random_failure_0093c300(UnitFireFloodingHost& host, std::uint32_t unit,
                                  RepairTaskState& task) {
    const int size = host.failure_table_size();
    if (size <= 0) {
        return;
    }
    int steps = static_cast<int>(host.random_next_u32() % static_cast<std::uint32_t>(size));
    int index = 0;
    std::string name;
    float duration = 0.0f;
    bool enabled = false;
    while (steps > 0) {
        index = (index + 1) % size;
        if (host.failure_table_row(index, &name, &duration, &enabled) && enabled) {
            --steps;
        } else if (!enabled) {
            // 0093C4A2's loop only decrements on an enabled row; a table with no
            // enabled rows would spin, which the native code does not guard.
            if (index == 0) {
                return;
            }
        }
    }
    if (!host.failure_table_row(index, &name, &duration, &enabled)) {
        return;
    }
    if (host.failure_already_active(name)) {
        return;
    }
    ActiveFailure record;
    record.id = -2;  // 0093C46E writes 0xFFFFFFFE
    record.name = name;
    record.seconds_remaining = duration;
    task.failures.push_back(record);
    host.warning_fire_failure(unit, name, duration);
    host.unit_on_failure(unit, name, 0);
}

namespace {

void push_leak_readout(UnitFireFloodingHost& host, std::uint32_t unit, const float* values) {
    const std::uint32_t parts = host.unit_parts_object(unit);
    const float displacement = host.parts_displacement(parts);
    float bins[kLeakZoneBins] = {};
    leak_zone_sums_0074e8f0(values, host.leak_manager_count(unit), bins);

    float total = 0.0f;
    for (const float bin : bins) {
        total += bin;
    }
    host.lua_push_number(leak_load_fraction_00891680(total, displacement));
    for (const int bin : kLeakZoneResultOrder) {
        host.lua_push_number(leak_load_fraction_00891680(bins[bin], displacement));
    }
}

}  // namespace

void lua_get_water_load_00891680(UnitFireFloodingHost& host, std::uint32_t unit) {
    push_leak_readout(host, unit, host.leak_manager_water(unit));
}

void lua_get_leaks_008918d0(UnitFireFloodingHost& host, std::uint32_t unit) {
    push_leak_readout(host, unit, host.leak_manager_leaks(unit));
}

void lua_cheat_max_repair_008c6dd0(UnitFireFloodingHost& host, std::uint32_t unit) {
    if (host.unit_is_kind_of(unit, kCheatMaxRepairShipClassId)) {
        host.unit_max_repair(unit);
        return;
    }
    if (host.unit_is_kind_of(unit, kCheatMaxRepairGroupClassId)) {
        const int count = host.group_member_count(unit);
        for (int i = 0; i < count; ++i) {
            // 008C6F68 CMP EDI,0x4 / JA: past index 4 the pointer is forced to
            // null and the virtual call goes through a null this.
            const std::uint32_t member =
                (i <= kCheatMaxRepairGroupMemberLimit) ? host.group_member(unit, i) : 0;
            host.unit_max_repair(member);
        }
        return;
    }
    if (host.unit_is_kind_of(unit, kCheatMaxRepairRosterClassId)) {
        const int count = host.roster_size(unit);
        for (int i = 0; i < count; ++i) {
            host.unit_max_repair(host.roster_member(unit, i));
        }
    }
}

}  // namespace bsp
