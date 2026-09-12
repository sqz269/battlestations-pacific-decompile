// Airbase and carrier air operations. docs/AIR_OPERATIONS.md.
// Addresses in comments are the native sites each rule was read from.
#include "bsp/air_operations.hpp"

namespace bsp {

std::size_t air_ops_block_offset_006bcd20(std::uint32_t class_id) noexcept {
    // 006BCD2C tests 5Ch(9) first and returns this+1188h (param_1 + 462h dwords);
    // 006BCD3B tests 5Ch(45h) and returns this+72Ch (param_1 + 1CBh dwords).
    if (class_id == kAirOpsClassIdMothership) {
        return kAirOpsBlockOffsetMothership;
    }
    if (class_id == kAirOpsClassIdAirfield) {
        return kAirOpsBlockOffsetAirfield;
    }
    return 0;
}

bool air_ops_block_is_usable_006bcd20(bool require_active, bool owner_present,
                                      bool owner_retired_flag) noexcept {
    // 006BCD50: the block is dropped when the flag is set and either the owner at
    // block+7Ch is null or its byte at +5Dh is non-zero.
    if (!require_active) {
        return true;
    }
    return owner_present && !owner_retired_flag;
}

AirOpsStockAddResult air_base_stock_add_006ca770(AirOpsStockEntry* entries, int entry_count,
                                                 int capacity, std::uint32_t vehicle_class,
                                                 std::int32_t added, AirOpsStockEntry* created) {
    AirOpsStockAddResult result{};
    // 006CA7A0-006CA7AF: linear walk of the list comparing node+8h to the class.
    for (int i = 0; i < entry_count; ++i) {
        if (entries[i].vehicle_class == vehicle_class) {
            // 006CA847 ADD dword ptr [EDI+0Ch], ECX.
            entries[i].count += added;
            result.entry_count = entries[i].count;
            return result;
        }
    }
    // 006CA7E8-006CA7FB: push_back {class, count, 5}.
    if (created != nullptr && entry_count < capacity) {
        created->vehicle_class = vehicle_class;
        created->count = added;
        created->unread_initial_five = 5;
        result.entry_count = added;
        result.created_entry = true;
    }
    return result;
}

std::int32_t air_base_stock_count_by_category_006bf100(const AirOpsStockEntry* entries,
                                                       const std::uint32_t* entry_category_keys,
                                                       int entry_count,
                                                       std::uint32_t category_key) noexcept {
    // 006BF10F: if (*(node[2] + 70h) == key) total += node[3].
    std::int32_t total = 0;
    for (int i = 0; i < entry_count; ++i) {
        if (entry_category_keys[i] == category_key) {
            total += entries[i].count;
        }
    }
    return total;
}

std::int32_t air_base_committed_planes_006bd3f0(const AirOpsSlot* slots,
                                                const std::int32_t* launched_plane_counts,
                                                int slot_count) noexcept {
    // 006BD42B: a slot with a launched squadron contributes the squadron's live
    // plane count at entity+3CCh; otherwise its assigned count at slot+8h.
    std::int32_t total = 0;
    for (int i = 0; i < slot_count; ++i) {
        total += slots[i].launched_squadron != 0 ? launched_plane_counts[i]
                                                 : slots[i].assigned_count;
    }
    return total;
}

AirOpsLaunchDecision air_ops_slot_launch_006cd350(const AirOpsLaunchInputs& in) noexcept {
    AirOpsLaunchDecision out{};
    // 006CD3D7-006CD40B: limit - committed, then min with the class stock, then
    // min with the slot's requested count.
    std::int32_t count = in.plane_limit - in.committed_planes;
    if (in.class_stock < count) {
        count = in.class_stock;
    }
    if (in.slot_requested < count) {
        count = in.slot_requested;
    }
    // 006CD40F: the slot always leaves the ready state, and a launch that was
    // requested from script starts the cooldown timer.
    out.next_state = AirOpsSlotState::kCooldown;
    out.next_timer = in.launch_requested ? kAirOpsSlotCooldownSeconds : 0.0F;
    if (count < 1) {
        out.launch_count = 0;
        out.clear_slot = true;
        return out;
    }
    out.launch_count = count;
    return out;
}

std::int32_t ship_catapult_stock_clamp_009539a0(std::int32_t current, std::int32_t requested,
                                                std::int32_t class_launch_stock) noexcept {
    // 009539A3: the whole routine is skipped when the class allows no stock, so
    // the field keeps whatever it held.
    if (class_launch_stock <= 0) {
        return current;
    }
    if (requested < 0) {
        return 0;
    }
    return requested <= class_launch_stock ? requested : class_launch_stock;
}

bool catapult_can_fire_006ebd30(const CatapultFireGate& gate) noexcept {
    // 006EBD36: disabled byte clear and ammo positive.
    if (gate.disabled || gate.ammo <= 0) {
        return false;
    }
    // 006EBD4C: fewer planes catapulted than the class allows.
    if (gate.catapulted_now >= gate.max_launched) {
        return false;
    }
    // 006EBD57: the reload timer must have run out.
    return !(gate.reload_timer > 0.0F);
}

CatapultPostLaunch catapult_after_launch_006ec070(std::int32_t current_tick,
                                                  std::int32_t launch_tick,
                                                  float class_launch_interval,
                                                  float seconds_per_tick,
                                                  std::int32_t ammo_before) noexcept {
    CatapultPostLaunch out{};
    // 006EC070-006EC09E: the timer is shortened by the time already elapsed, so a
    // client that applies the message late does not wait the full interval again.
    const std::int32_t elapsed_ticks = current_tick - launch_tick;
    out.launch_tick = launch_tick;
    out.barrel_reload_seconds =
        class_launch_interval - static_cast<float>(elapsed_ticks) * seconds_per_tick;
    out.ammo = ammo_before - 1;
    return out;
}

std::int32_t catapult_launch_input_006ec8ff(std::int32_t instance_override,
                                            std::int32_t class_default) noexcept {
    // 006EC90B TEST EBX,EBX / JGE: a negative instance field falls back.
    return instance_override >= 0 ? instance_override : class_default;
}

CatapultSpawnProperties catapult_spawn_properties_006ec8e0(std::int32_t launched_class,
                                                           std::int32_t equipment,
                                                           std::int32_t skill, std::int32_t race,
                                                           std::int32_t party,
                                                           std::int32_t owner_player,
                                                           std::uint16_t owner_entity_id,
                                                           std::int32_t launched_class_resource) {
    CatapultSpawnProperties props{};
    props.type = static_cast<std::uint32_t>(launched_class); // 006EC95D
    props.wing_count = 1;                                    // 006EC977
    props.skill = skill;                                     // 006EC987
    props.race = race;                                       // 006EC9A6
    props.party = party;                                     // 006EC9BE
    props.owner_player = owner_player;                       // 006EC9D6
    props.state = 3;                                         // 006EC9F1
    props.plane_parent_id = owner_entity_id;                 // 006ECA01
    props.equipment = equipment;                             // 006ECA20
    props.behaviour = 1;                                     // 006ECA2F
    // 006ECA3F CVTSI2SS: the class field is an integer converted to float.
    props.resource_usage = static_cast<float>(launched_class_resource);
    return props;
}

int squadron_land_and_kill_plan_008a20e0(int plane_count, SquadronLandStep* steps,
                                         int max_steps) noexcept {
    // 008A2214: start at plane_count - 1 and walk down to 0.
    int written = 0;
    for (int index = plane_count - 1; index >= 0 && written < max_steps; --index) {
        steps[written].index = index;
        // 008A2232 CMP [ESP+10h],4 / JA: above four the pointer is zeroed first.
        steps[written].plane_readable = index < kSquadronMaxPlanes;
        ++written;
    }
    return written;
}

std::int32_t squadron_remove_plane_007f3970(std::uint32_t* planes, std::int32_t plane_count,
                                            std::uint32_t plane, bool plane_has_back_pointer,
                                            bool killed_by_landing, bool* out_last_plane_flag) {
    // 007F3979: a plane whose back pointer at +9D4h is already null is ignored.
    if (!plane_has_back_pointer) {
        return -1;
    }
    // 007F39AC: find the plane in the first plane_count - 1 entries.
    const std::int32_t last = plane_count - 1;
    std::int32_t index = 0;
    while (index < last && planes[index] != plane) {
        ++index;
    }
    // 007F39CB: compact the tail down one slot and clear the freed entry.
    for (std::int32_t i = index; i < last; ++i) {
        planes[i] = planes[i + 1];
    }
    const std::int32_t remaining = plane_count - 1;
    planes[remaining] = 0;
    if (out_last_plane_flag != nullptr && remaining == 0) {
        // 007F3A2A: the flag is set only when the squadron was not killed by the
        // landing path and the plane's byte at +C41h is set. The landing path
        // always passes a non-zero flag, so it always clears it.
        *out_last_plane_flag = !killed_by_landing;
    }
    return remaining;
}

void plane_reload_bomb_platforms_0089f080(AirOperationsHost& host, const std::uint32_t* planes,
                                          std::int32_t plane_count, bool* out_dirty) {
    // 0089F198 calls 007ED650, whose whole body is this loop.
    for (std::int32_t i = 0; i < plane_count; ++i) {
        host.reload_plane_bomb_platforms(planes[i]); // 007ED667
    }
    if (out_dirty != nullptr) {
        *out_dirty = true; // 007ED67C MOV byte ptr [EBX+3ECh],1
    }
}

void squadron_land_and_kill_008a20e0(AirOperationsHost& host, std::uint32_t squadron,
                                     const std::uint32_t* air_ops_blocks, int block_count,
                                     std::uint32_t* planes, std::int32_t* plane_count) {
    // 008A220F -> 007F1B70: every carrier and airfield in the scene is told the
    // squadron is gone, which frees whichever slot launched it.
    for (int i = 0; i < block_count; ++i) {
        host.release_squadron_slot(air_ops_blocks[i], squadron);
    }
    SquadronLandStep steps[kSquadronMaxPlanes * 2]{};
    const int step_count = squadron_land_and_kill_plan_008a20e0(
        *plane_count, steps, static_cast<int>(sizeof(steps) / sizeof(steps[0])));
    for (int i = 0; i < step_count; ++i) {
        const std::uint32_t plane = steps[i].plane_readable ? planes[steps[i].index] : 0;
        host.kill_plane(plane, kSquadronLandKillReason); // 008A223F
        bool last_flag = false;
        // 008A2249: the flag argument is the literal 1.
        const std::int32_t remaining =
            squadron_remove_plane_007f3970(planes, *plane_count, plane, plane != 0, true,
                                           &last_flag);
        if (remaining >= 0) {
            *plane_count = remaining;
        }
    }
}

} // namespace bsp
