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

// ---------------------------------------------------------------------------
// 006CADD0 mode 1
// ---------------------------------------------------------------------------
AirOpsDeck air_ops_load_from_scene_006cadd0(const AirOpsSceneDeck& authored,
                                            AirOpsTypeResolver resolve_type, void* context) {
    AirOpsDeck deck;
    // 006CAE81 leaves the array empty when NumSlots is absent or not positive,
    // and 006CAE7E has already zeroed the live count by then, so a deck with no
    // authored NumSlots is an empty deck rather than an untouched one.
    const std::int32_t count = authored.num_slots > 0 ? authored.num_slots : 0;
    deck.slots.resize(static_cast<std::size_t>(count));
    // 006CB0C9 stores MaxInAirPlanes straight into block+58h, with no clamp and
    // no default: an absent key reaches the store as the bag's own zero.
    deck.max_in_air_planes = authored.max_in_air_planes;

    // 006CB0D5: the PlaneStock loop. 006CB126 reads SquadLimit but this packet
    // did not read where it lands, so it is carried unused. contract.
    for (const AirOpsSceneStock& row : authored.stock) {
        AirOpsStockEntry entry;
        entry.vehicle_class = resolve_type != nullptr ? resolve_type(row.type, context) : 0u;
        entry.count = row.count;
        deck.stock.push_back(entry);
    }

    // 006CB1B2: the Slot loop, 1-based, one authored sub-block per slot.
    for (std::size_t index = 0; index < authored.slots.size(); ++index) {
        if (index >= deck.slots.size()) break;  // NumSlots bounds the array
        const AirOpsSceneSlot& row = authored.slots[index];
        AirOpsSlot& slot = deck.slots[index];
        // 006CB24E resolves Type through 007B8A80 and 006CB260 assigns the pair
        // through 006C0F00, which clamps the authored Count against the stock
        // that is actually available and against the slot's own capacity. This
        // process has no live stock accounting, so the authored count stands and
        // the clamp is a contract.
        slot.vehicle_class = resolve_type != nullptr ? resolve_type(row.type, context) : 0u;
        slot.assigned_count = row.count;
        // 006CB277 stores Arm at slot+10h, the field the Lua reader publishes as
        // `equipment`.
        slot.class_field_134 = row.arm;
        if (row.fake_allocated) {
            // 006CB28B..006CB2A6: a FakeAllocated slot is left in state 6 with a
            // zero timer, and when the launch-requested byte at slot+34h was set
            // the timer becomes 00CE3850 (5.0) and that byte is cleared. State 6
            // is a value the launch routines never showed; it is recorded here
            // because the scene loader writes it, not because its meaning is
            // known. contract.
            slot.state = static_cast<AirOpsSlotState>(6);
            slot.timer = 0.0F;
            if (slot.launch_requested) {
                slot.timer = kAirOpsSlotCooldownSeconds;
                slot.launch_requested = false;
            }
        }
    }
    return deck;
}

// ---------------------------------------------------------------------------
// The process-wide deck table
// ---------------------------------------------------------------------------
void AirOpsDeckRegistry::clear() noexcept {
    decks_.clear();
    entity_ids_.clear();
}

void AirOpsDeckRegistry::set(const std::string& unit_name, AirOpsDeck deck) {
    for (auto& row : decks_) {
        if (row.first == unit_name) {
            row.second = std::move(deck);
            return;
        }
    }
    decks_.emplace_back(unit_name, std::move(deck));
}

void AirOpsDeckRegistry::bind_entity_id(int entity_id, const std::string& unit_name) {
    for (auto& row : entity_ids_) {
        if (row.first == entity_id) {
            row.second = unit_name;
            return;
        }
    }
    entity_ids_.emplace_back(entity_id, unit_name);
}

const AirOpsDeck* AirOpsDeckRegistry::find(const std::string& unit_name) const noexcept {
    for (const auto& row : decks_) {
        if (row.first == unit_name) return &row.second;
    }
    return nullptr;
}

const AirOpsDeck* AirOpsDeckRegistry::find_by_entity_id(int entity_id) const noexcept {
    for (const auto& row : entity_ids_) {
        if (row.first == entity_id) return find(row.second);
    }
    return nullptr;
}

AirOpsDeck* AirOpsDeckRegistry::find_mutable_by_entity_id(int entity_id) noexcept {
    for (const auto& binding : entity_ids_) {
        if (binding.first != entity_id) continue;
        for (auto& row : decks_) {
            if (row.first == binding.second) return &row.second;
        }
        return nullptr;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// 00895D20 and 006CC690, the launch gates
// ---------------------------------------------------------------------------
bool air_ops_is_ready_to_send_planes_00895d20(const AirOpsDeck& deck) noexcept {
    // 00895E4F and 00895E58: the false arm is taken only when BOTH the class
    // test against 45h passes and the byte at entity+720h is set. Either one
    // failing falls through to 006BF620, so a mother ship never takes it.
    if (deck.is_airfield && deck.airfield_blocked) return false;
    // 006BF620, in its own order: the two failure bytes clear, the owner at
    // block+7Ch non-null, its byte at +5Dh clear, and block+38h zero.
    if (deck.runway_failure || deck.hangar_failure) return false;
    if (!deck.owner_present) return false;
    if (deck.owner_blocked) return false;
    return deck.launch_in_progress == 0;
}

int air_ops_pick_launch_slot_006c7210(const AirOpsDeck& deck) noexcept {
    // 006C7243: walk while the index is below block+50h, reading slot+2Ch; the
    // first slot in state 1 or 5 is the answer. When the walk runs out the
    // native continues into the growth path, and the index it has then is one
    // past the last slot, which is where the new record lands.
    for (std::size_t index = 0; index < deck.slots.size(); ++index) {
        const std::int32_t state = static_cast<std::int32_t>(deck.slots[index].state);
        if (state == static_cast<std::int32_t>(AirOpsSlotState::kCooldown)
            || state == static_cast<std::int32_t>(AirOpsSlotState::kReady)) {
            return static_cast<int>(index);
        }
    }
    return static_cast<int>(deck.slots.size());
}

namespace {
AirOpsSquadronFactory* g_squadron_factory = nullptr;
} // namespace

void set_air_ops_squadron_factory(AirOpsSquadronFactory* factory) noexcept {
    g_squadron_factory = factory;
}

AirOpsSquadronFactory* air_ops_squadron_factory() noexcept { return g_squadron_factory; }

std::size_t air_ops_release_squadron_slot_006c65b0(AirOpsDeck& deck,
                                                   std::uint32_t squadron) noexcept {
    std::size_t released = 0;
    for (AirOpsSlot& slot : deck.slots) {
        if (slot.launched_squadron != squadron) continue;
        // 006C65EE guards only the observer unregister and the zeroing of +28h
        // on the field being non-zero; the four stores below are unconditional,
        // so calling this with a zero squadron would reset every free slot. No
        // caller does, and the native has the same shape.
        slot.launched_squadron = 0u;
        slot.assigned_count = 0;
        slot.state = AirOpsSlotState::kCooldown;
        slot.timer = kAirOpsSlotCooldownSeconds;
        slot.launch_requested = false;
        ++released;
    }
    return released;
}

bool air_ops_slot_is_free_006c56d0(const AirOpsSlot& slot) noexcept {
    // 006C56D0's own test: state 6 or state 1. Nothing else counts as free, which
    // is why a slot this process leaves in state 3 is never reused.
    const std::int32_t state = static_cast<std::int32_t>(slot.state);
    return state == 6 || state == static_cast<std::int32_t>(AirOpsSlotState::kCooldown);
}

void air_ops_launch_start_006c7490(AirOpsDeck& deck, int slot_index) noexcept {
    if (slot_index < 0 || static_cast<std::size_t>(slot_index) >= deck.slots.size()) return;
    AirOpsSlot& slot = deck.slots[static_cast<std::size_t>(slot_index)];
    // 006C74C2: the requested count takes the assigned count, but only when the
    // slot was already in state 2.
    if (slot.state == AirOpsSlotState::kLaunching) slot.requested_count = slot.assigned_count;
    // 006C74CE and 006C74D5: state 3 and a zero timer, then 006C74E1's carry of
    // the launch-requested byte into the 5.0 cooldown, which is the same pair
    // 006CADD0 mode 1 writes for a FakeAllocated slot. State 3 is a fourth value
    // for this field, after the 1, 2 and 5 the launch routines showed and the 6
    // the scene loader writes; what it means is not established.
    slot.state = static_cast<AirOpsSlotState>(3);
    slot.timer = 0.0F;
    if (slot.launch_requested) {
        slot.timer = kAirOpsSlotCooldownSeconds;
        slot.launch_requested = false;
    }
    // 006C74C6 builds the squadron before any of the above, and 006C74FF stores
    // it in slot+28h when it differs from what is there, moving the observer
    // pair with it. The build is 006C5050's property bag, which this process can
    // only fill through a factory the units host supplies, because its
    // counterpart to 004F0AD0 is driven from the scene contents pass. With no
    // factory nothing is created and the field stays zero, which is what this
    // process did before the seam existed.
    AirOpsSquadronFactory* factory = air_ops_squadron_factory();
    if (factory == nullptr) return;
    AirOpsSquadronRequest request;
    request.slot_number = slot_index + 1;  // 006C7490 passes the index plus one
    // SUBSTITUTION: 006C7490 passes the class's +70h, not the class id. That
    // field was not read, and 006BF100 uses the same +70h as a grouping key that
    // docs/AIR_OPERATIONS.md also records as unread. The class id is what this
    // process has. contract.
    request.type = slot.vehicle_class;
    request.wing_count = slot.assigned_count;
    request.equipment = slot.class_field_134;
    request.skill = deck.owner_skill;
    request.party = deck.owner_party;
    request.race = deck.owner_race;
    request.owner_player = deck.owner_player;
    request.home_base = deck.owner_name;
    request.state = 1;                     // the flag argument is zero from 006CC690
    const std::uint32_t squadron = factory->create_squadron(request);
    if (squadron != 0u && slot.launched_squadron != squadron) {
        slot.launched_squadron = squadron;
    }
}

AirOpsLaunchResult air_ops_launch_squadron_006cc690(AirOpsDeck& deck,
                                                    const AirOpsLaunchRequest& request) {
    AirOpsLaunchResult result;
    result.slot_index = air_ops_pick_launch_slot_006c7210(deck);
    if (result.slot_index < 0) return result;
    if (static_cast<std::size_t>(result.slot_index) >= deck.slots.size()) {
        // The growth path. 006CADD0's own array grows as 2n+2; what matters to
        // the caller is that the record exists at the index that comes back.
        deck.slots.resize(static_cast<std::size_t>(result.slot_index) + 1);
    }
    AirOpsSlot& slot = deck.slots[static_cast<std::size_t>(result.slot_index)];
    // 006CC6D0: the class is written only when it differs, and that same arm
    // takes the class's own default at class+134h. 006CC6F5 then overwrites the
    // arm with the argument, so the default only survives when the caller passed
    // the class's own value.
    if (slot.vehicle_class != request.vehicle_class) {
        slot.vehicle_class = request.vehicle_class;
        slot.class_field_134 = request.vehicle_class != 0 ? request.class_default_arm : 0;
    }
    slot.assigned_count = request.count;
    slot.class_field_134 = request.arm;
    if (deck.launch_in_progress == 0) {
        // 006CC733 calls 006C7490 with the block in ECX and the slot index and a
        // zero pushed. CORRECTED: an earlier version of this function also wrote
        // block+38h here. Neither 006CC690 nor 006C7490 writes that field; both
        // only read it (006CC715 and 006C5078). Writing it made the deck report
        // a launch in progress for ever after the first call, which would have
        // made IsReadyToSendPlanes answer false from then on.
        // docs/AIROPS_LAUNCH_START.md.
        result.started = true;
        air_ops_launch_start_006c7490(deck, result.slot_index);
    } else {
        // 006CC72C: the stock goes back and 006CA640 queues instead.
        result.queued = true;
    }
    return result;
}

std::size_t AirOpsDeckRegistry::size() const noexcept { return decks_.size(); }

AirOpsDeck* AirOpsDeckRegistry::mutable_at(std::size_t index) noexcept {
    if (index >= decks_.size()) return nullptr;
    return &decks_[index].second;
}

const std::string& AirOpsDeckRegistry::name_at(std::size_t index) const noexcept {
    static const std::string empty;
    if (index >= decks_.size()) return empty;
    return decks_[index].first;
}

// ---------------------------------------------------------------------------
// The tick
// ---------------------------------------------------------------------------
AirOpsStockAvailable air_ops_stock_available_006bf230(
    const AirOpsDeck& deck, std::uint32_t vehicle_class,
    const std::int32_t* launched_plane_counts) noexcept {
    AirOpsStockAvailable out;
    // 006BF243: the list at block+44h, every node whose +8h is the class, summing
    // its +0Ch. The reconstruction's stock vector is that list.
    for (const AirOpsStockEntry& entry : deck.stock) {
        if (entry.vehicle_class == vehicle_class) out.total += entry.count;
    }
    out.available = out.total;
    // 006BF2A0: the slot walk. Both the class AND a state of 1 or 5 are required
    // before anything is subtracted, so a slot that has launched stops holding
    // its stock the moment 006C74E0 writes state 3.
    for (std::size_t index = 0; index < deck.slots.size(); ++index) {
        const AirOpsSlot& slot = deck.slots[index];
        if (slot.vehicle_class != vehicle_class) continue;
        const std::int32_t state = static_cast<std::int32_t>(slot.state);
        if (state != static_cast<std::int32_t>(AirOpsSlotState::kCooldown)
            && state != static_cast<std::int32_t>(AirOpsSlotState::kReady)) {
            continue;
        }
        if (slot.launched_squadron != 0u) {
            out.available -= launched_plane_counts != nullptr
                ? launched_plane_counts[index] : 0;
        } else {
            out.available -= slot.assigned_count;
        }
    }
    return out;
}

AirOpsSlotTickResult air_ops_slot_tick_006c0510(
    AirOpsDeck& deck, std::size_t slot_index, float step_seconds,
    const std::int32_t* launched_plane_counts) noexcept {
    AirOpsSlotTickResult out;
    if (slot_index >= deck.slots.size()) return out;
    AirOpsSlot& slot = deck.slots[slot_index];

    // 006C051A FADD then 006C0522 FSTP: the timer accumulates.
    slot.timer += step_seconds;
    // 006C0527-006C0541. COMISS XMM0(timer), XMM1(5.0) with JA taken keeps the
    // timer, so the store is max(timer, 5.0): the byte spends the wait rather
    // than arming one, which is how a scripted launch takes effect at once.
    if (slot.launch_requested) {
        if (!(slot.timer > kAirOpsSlotCooldownSeconds)) {
            slot.timer = kAirOpsSlotCooldownSeconds;
        }
        slot.launch_requested = false;
    }

    // 006C0549 JNZ: a slot holding a squadron does nothing but mirror the
    // squadron's live plane count into slot+8h, and reports a change so the
    // deck's own message goes out.
    if (slot.launched_squadron != 0u) {
        const std::int32_t live = launched_plane_counts != nullptr
            ? launched_plane_counts[slot_index] : 0;
        if (live != slot.assigned_count) {
            slot.assigned_count = live;
            out.dirty = true;
        }
        return out;
    }

    // 006C054E/006C0553: only states 3 and 4 refill. Every other state with no
    // squadron falls through 006C059F to the zero return.
    const std::int32_t state = static_cast<std::int32_t>(slot.state);
    if (state != static_cast<std::int32_t>(AirOpsSlotState::kLaunched)
        && state != static_cast<std::int32_t>(AirOpsSlotState::kRecalled)) {
        return out;
    }

    // 006C055C zeroes slot+8h before 006BD3F0 is called at 006C0562, so this
    // slot's own count is not in the committed total.
    slot.assigned_count = 0;
    const std::int32_t committed = launched_plane_counts != nullptr
        ? air_base_committed_planes_006bd3f0(deck.slots.data(), launched_plane_counts,
                                             static_cast<int>(deck.slots.size()))
        : 0;
    std::int32_t count = deck.max_in_air_planes - committed;
    // 006C057C CMP EAX,EDI with JL: min of the two.
    const AirOpsStockAvailable stock = air_ops_stock_available_006bf230(
        deck, slot.vehicle_class, launched_plane_counts);
    if (stock.available < count) count = stock.available;
    // 006C0585 CMP ECX,EAX with JGE: min with slot+0Ch.
    if (slot.requested_count < count) count = slot.requested_count;
    slot.assigned_count = count;
    slot.state = AirOpsSlotState::kReady;  // 006C058F
    out.dirty = true;                      // 006C0596 MOV AL,1
    out.became_ready = true;
    out.refilled_count = count;
    return out;
}

namespace {
AirOpsSquadronPlaneCount g_squadron_plane_count = nullptr;
void* g_squadron_plane_count_context = nullptr;
} // namespace

void set_air_ops_squadron_plane_count(AirOpsSquadronPlaneCount reader,
                                      void* context) noexcept {
    g_squadron_plane_count = reader;
    g_squadron_plane_count_context = context;
}

AirOpsDeckTickResult air_ops_deck_update_006c0da0(AirOpsDeck& deck, float step_seconds) {
    AirOpsDeckTickResult out;
    if (deck.slots.empty()) return out;
    // The native reads entity+3CCh inside each slot's tick; this reads all of
    // them once before the walk, which differs only if a squadron's count could
    // change while the walk runs. Nothing in this process changes it there.
    std::vector<std::int32_t> live(deck.slots.size(), 0);
    for (std::size_t index = 0; index < deck.slots.size(); ++index) {
        const std::uint32_t squadron = deck.slots[index].launched_squadron;
        if (squadron == 0u) continue;
        ++out.tracking;
        if (g_squadron_plane_count != nullptr) {
            live[index] = g_squadron_plane_count(squadron, g_squadron_plane_count_context);
        }
    }
    // 006C0DC6: the walk runs while the cursor is below block+50h, in index
    // order, and each slot the tick returned true for is routed at 006C0DFB.
    for (std::size_t index = 0; index < deck.slots.size(); ++index) {
        const AirOpsSlotTickResult step
            = air_ops_slot_tick_006c0510(deck, index, step_seconds, live.data());
        ++out.slots;
        if (step.dirty) ++out.dirty;
        if (step.became_ready) ++out.became_ready;
    }
    return out;
}

AirOpsDeckTickResult air_ops_update_decks_006cdc70(float step_seconds) {
    AirOpsDeckTickResult total;
    AirOpsDeckRegistry& registry = air_ops_decks();
    for (std::size_t index = 0; index < registry.size(); ++index) {
        AirOpsDeck* deck = registry.mutable_at(index);
        if (deck == nullptr) continue;
        const AirOpsDeckTickResult one = air_ops_deck_update_006c0da0(*deck, step_seconds);
        total.slots += one.slots;
        total.dirty += one.dirty;
        total.became_ready += one.became_ready;
        total.tracking += one.tracking;
    }
    return total;
}

AirOpsDeckRegistry& air_ops_decks() noexcept {
    static AirOpsDeckRegistry registry;
    return registry;
}

} // namespace bsp
