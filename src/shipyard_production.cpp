// Packet cc9_shipyard_production. docs/GUNNERY_OPEN_ITEMS.md sections 134 and 136.
#include "bsp/shipyard_production.hpp"

#include "bsp/vehicle_class.hpp"

namespace bsp {

namespace {

ShipyardCreateUnitFn g_create_unit = nullptr;

// 0094EDC0 over the 00CE4BFC delimiter ','. Unread: whether it trims or drops
// empty tokens; this keeps every token as written.
std::vector<std::string> split_names(const std::string& names) {
    std::vector<std::string> out;
    std::size_t start = 0;
    for (;;) {
        const std::size_t cut = names.find(',', start);
        out.push_back(names.substr(start, cut == std::string::npos ? std::string::npos
                                                                    : cut - start));
        if (cut == std::string::npos) break;
        start = cut + 1;
    }
    return out;
}

ShipyardStockRecord* stock_of_class(ShipyardState& state, std::uint32_t class_id) {
    for (ShipyardStockRecord& r : state.stock) {
        if (r.class_id == class_id) return &r;
    }
    return nullptr;
}

}  // namespace

ShipyardState shipyard_scene_attach_00849a30(const std::string& owner_name,
                                             const ShipyardSceneAuthored& authored) {
    ShipyardState state;
    state.owner_name = owner_name;
    for (const ShipyardSceneStock& s : authored.stocks) {
        // 00849E04 TEST EBX,EBX (Count) and 00849E08 TEST EBP,EBP (the class).
        if (s.count == 0 || s.class_id == 0) continue;
        ShipyardStockRecord r;
        r.class_id = s.class_id;
        r.count = s.count;
        r.names = split_names(s.names);
        state.stock.push_back(std::move(r));
    }
    // 00848EA0(NumSlots, default entry).
    if (authored.num_slots > 0) {
        state.entries.resize(static_cast<std::size_t>(authored.num_slots));
    }
    state.authored = authored;
    return state;
}

ShipyardSlotFill shipyard_complete_slots_00849f70(ShipyardState& state,
    ShipyardUnitClassFn unit_class, ShipyardUnitIndexFn unit_index, void* context) {
    ShipyardSlotFill fill;
    if (state.slots_completed) return fill;
    state.slots_completed = true;
    // 00849FC5..0084A12F: "Hangar 1".."Hangar 12"; both names empty ends the list.
    for (const ShipyardSceneHangar& h : state.authored.hangars) {
        if (h.object.empty() && h.path.empty()) break;
        ShipyardHangar hangar;
        hangar.object = h.object;
        hangar.path = h.path;
        state.hangars.push_back(std::move(hangar));
        ++fill.hangars;
    }
    // 0084A137..0084A3E8: "Slot 1".."Slot NumSlots", entry i - 1.
    const std::size_t n = state.entries.size();
    for (std::size_t i = 0; i < n && i < state.authored.slots.size(); ++i) {
        const ShipyardSceneSlot& slot = state.authored.slots[i];
        if (!slot.present) continue;
        ShipyardEntry& entry = state.entries[i];
        const std::uint32_t live_class = (!slot.unit.empty() && unit_class != nullptr)
            ? unit_class(slot.unit, context) : 0u;
        if (live_class != 0u) {
            // 0084A1F0..0084A230: state 4, +30h the entity, +8 its class+538h,
            // +0Ch and +2Ch cleared; 0084A3C9 takes one from its stock.
            entry.state = ShipyardEntryState::Launched;
            entry.unit = unit_index != nullptr ? unit_index(slot.unit, context) : kShipyardNone;
            entry.class_id = live_class;
            entry.equipment = 0;
            entry.hangar = kShipyardNone;
            ++fill.launched;
        } else if (slot.unit_class_present && slot.unit_class_raw > 0) {
            // 0084A2C4..0084A36A: state 1, +8 = GetOrCreate(UnitClass),
            // +0Ch = UnitEquipment; then the same stock draw.
            entry.state = ShipyardEntryState::Chosen;
            entry.class_id = slot.unit_class;
            entry.equipment = slot.unit_equipment;
            ++fill.chosen;
        } else {
            continue;
        }
        if (ShipyardStockRecord* r = stock_of_class(state, entry.class_id)) {
            r->count -= 1;
        } else {
            ++fill.missing_stock;
        }
    }
    return fill;
}

int shipyard_add_stock_0084acb0(ShipyardState& state, std::uint32_t class_id, int count,
                                const std::string& names) {
    if (ShipyardStockRecord* r = stock_of_class(state, class_id)) {
        r->count += count;
        return r->count;
    }
    ShipyardStockRecord r;
    r.class_id = class_id;
    r.count = count;
    r.names = split_names(names);
    state.stock.push_back(std::move(r));
    return count;
}

bool shipyard_choose_class_00844d60(ShipyardState& state, std::size_t entry, bool forward,
                                    const ShipyardClassFacts& facts) {
    if (entry >= state.entries.size()) return false;
    ShipyardEntry& e = state.entries[entry];
    const int size = static_cast<int>(state.stock.size());
    int at;
    if (e.class_id == 0u) {
        at = forward ? -1 : size;
    } else {
        at = 0;
        while (at < size && state.stock[static_cast<std::size_t>(at)].class_id != e.class_id) ++at;
        if (at >= size) return false;  // the image's +8 += 1 lands past the end
        state.stock[static_cast<std::size_t>(at)].count += 1;
    }
    for (;;) {
        if (!forward) {
            --at;
            if (at < 0) break;
        } else {
            ++at;
            if (at >= size) break;
        }
        if (state.stock[static_cast<std::size_t>(at)].count != 0) break;
    }
    if (at < 0 || at >= size) {
        // 00844F43: state 0, class 0.
        e.state = ShipyardEntryState::Idle;
        e.class_id = 0u;
        return true;
    }
    ShipyardStockRecord& r = state.stock[static_cast<std::size_t>(at)];
    r.count -= 1;
    e.state = ShipyardEntryState::Chosen;
    e.class_id = r.class_id;
    e.equipment = facts.default_equipment_134(r.class_id);
    return true;
}

bool shipyard_cycle_equipment_008436f0(ShipyardState& state, std::size_t entry, bool forward,
                                       const ShipyardClassFacts& facts) {
    if (entry >= state.entries.size()) return false;
    ShipyardEntry& e = state.entries[entry];
    if (e.class_id == 0u) return false;
    const int n = facts.equipment_count_128(e.class_id);
    const int step = forward ? 1 : n;
    e.equipment = (e.equipment + step) % (n + 1);
    return true;
}

bool shipyard_order_entry_00846d90(ShipyardState& state, std::size_t entry, bool forward,
                                   std::size_t order_target) {
    if (entry >= state.entries.size()) return false;
    ShipyardEntry& e = state.entries[entry];
    ShipyardStockRecord* r = stock_of_class(state, e.class_id);
    if (r == nullptr || r->names.empty() || r->next_name >= r->names.size()) return false;
    e.state = ShipyardEntryState::Ordered;
    e.name = r->names[r->next_name];
    e.order_target = forward ? order_target : kShipyardNone;
    r->next_name = (r->next_name + 1u) % static_cast<std::uint32_t>(r->names.size());
    return true;
}

std::size_t shipyard_free_hangar_00844ce0(const ShipyardState& state,
                                          const std::vector<bool>& usable) {
    for (std::size_t i = 0; i < state.hangars.size(); ++i) {
        if (i < usable.size() && usable[i] && state.hangars[i].launched_unit == kShipyardNone) {
            return i;
        }
    }
    return kShipyardNone;
}

int shipyard_available_00844610(const ShipyardState& state, std::uint32_t class_id) {
    int total = 0;
    for (const ShipyardStockRecord& r : state.stock) {
        if (r.class_id == class_id) total += r.count;
    }
    for (const ShipyardEntry& e : state.entries) {
        if (e.class_id != 0u && e.class_id == class_id
            && e.state == ShipyardEntryState::Chosen) {
            ++total;
        }
    }
    return total;
}

ShipyardCreator shipyard_creator_for_kind_00844fc0(int kind) {
    // vtable+18h is a kind-of test, so PlaneBase answers for every plane leaf.
    if (kind >= static_cast<int>(VehicleClassKind::PlaneBase)
        && kind <= static_cast<int>(VehicleClassKind::Kamikaze)) {
        return ShipyardCreator::PlaneSquadron;
    }
    switch (static_cast<VehicleClassKind>(kind)) {
    case VehicleClassKind::BattleShip:
    case VehicleClassKind::Cruiser:
    case VehicleClassKind::Destroyer:
    case VehicleClassKind::Cargo:
        return ShipyardCreator::Destroyer;
    case VehicleClassKind::MotherShip:
        return ShipyardCreator::MotherShip;
    case VehicleClassKind::Submarine:
        return ShipyardCreator::Submarine;
    case VehicleClassKind::TorpedoBoat:
        return ShipyardCreator::TorpedoBoat;
    case VehicleClassKind::LandingShip:
        return ShipyardCreator::LandingShip;
    default:
        return ShipyardCreator::None;
    }
}

void shipyard_set_create_unit(ShipyardCreateUnitFn fn) noexcept { g_create_unit = fn; }
ShipyardCreateUnitFn shipyard_create_unit() noexcept { return g_create_unit; }

void ShipyardRegistry::clear() noexcept { yards_.clear(); }

void ShipyardRegistry::set(const std::string& unit_name, ShipyardState state) {
    state.owner_name = unit_name;
    for (ShipyardState& y : yards_) {
        if (y.owner_name == unit_name) {
            const int id = y.owner_entity_id;
            y = std::move(state);
            if (y.owner_entity_id == 0) y.owner_entity_id = id;
            return;
        }
    }
    yards_.push_back(std::move(state));
}

void ShipyardRegistry::bind_entity_id(int entity_id, const std::string& unit_name) {
    for (ShipyardState& y : yards_) {
        if (y.owner_name == unit_name) y.owner_entity_id = entity_id;
    }
}

ShipyardState* ShipyardRegistry::find_mutable(const std::string& unit_name) noexcept {
    for (ShipyardState& y : yards_) {
        if (y.owner_name == unit_name) return &y;
    }
    return nullptr;
}

ShipyardState* ShipyardRegistry::find_mutable_by_entity_id(int entity_id) noexcept {
    if (entity_id == 0) return nullptr;
    for (ShipyardState& y : yards_) {
        if (y.owner_entity_id == entity_id) return &y;
    }
    return nullptr;
}

std::size_t ShipyardRegistry::size() const noexcept { return yards_.size(); }

ShipyardState* ShipyardRegistry::mutable_at(std::size_t index) noexcept {
    return index < yards_.size() ? &yards_[index] : nullptr;
}

ShipyardRegistry& shipyards() noexcept {
    static ShipyardRegistry registry;
    return registry;
}

}  // namespace bsp
