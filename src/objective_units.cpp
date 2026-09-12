// docs/OBJECTIVE_UNIT_LIST.md. Packet cc2_mission_objectives. Ghidra was read-only.
#include "bsp/objective_units.hpp"

namespace bsp {

ObjectiveUnitEntry objective_unit_entry_008dee50(void* unit) noexcept
{
    // 008dee63..008dee67 store the unit at +0h and touch nothing else; the three floats stay
    // as operator new left them. position_valid records that, so a reader cannot mistake the
    // default-constructed point for a recovered position.
    ObjectiveUnitEntry entry{};
    entry.unit = unit;
    entry.position_valid = false;
    return entry;
}

ObjectiveUnitEntry objective_position_entry_008deeb0(const ObjectiveWorldPoint& point) noexcept
{
    // 008deec9 writes 0 to +0h, then 008deecf/008deed5/008deedb store x, y and z.
    ObjectiveUnitEntry entry{};
    entry.unit = nullptr;
    entry.position = point;
    entry.position_valid = true;
    return entry;
}

unsigned int objective_party_slot_mask(const int* player_party, int slot_count,
                                       int party) noexcept
{
    // 008cde8f..008cdef2: for each of the eight players, compare player+28h with the argument
    // and OR in 1 << k on a match.
    unsigned int mask = 0;
    if (player_party == nullptr) {
        return mask;
    }
    for (int slot = 0; slot < slot_count; ++slot) {
        if (player_party[slot] == party) {
            mask |= 1u << slot;
        }
    }
    return mask;
}

unsigned int objective_explicit_slot_mask(int slot) noexcept
{
    // 008cdfdb..008cdff1: MOV EDX,1; SHL EDX,CL; MOVZX EAX,DX. The mask word is 16 bits.
    if (slot < 0 || slot >= 32) {
        return 0;
    }
    return (1u << slot) & 0xffffu;
}

unsigned int objective_target_slot_mask(ObjectiveUnitHost& host, bool have_party, int party,
                                        bool have_slot, int slot)
{
    unsigned int mask = 0;
    if (have_party) {
        for (int k = 0; k < kObjectiveSetSlotCount; ++k) {
            if (host.player_party(k) == party) {
                mask |= 1u << k;
            }
        }
    }
    // 008cdfb3: an explicit, active slot replaces the party mask rather than adding to it.
    if (have_slot && slot >= 0 && slot < kObjectiveSetSlotCount && host.player_slot_active(slot)) {
        mask = objective_explicit_slot_mask(slot);
    }
    return mask;
}

void objective_set_add_unit_008df2b0(ObjectiveUnitHost& host, int slot,
                                     const std::string& objective_name, void* unit)
{
    if (!objective_accepts_unit(host.unit_liveness(unit))) {
        return;
    }
    ObjectiveUnitList* objective = host.objective_in_slot(slot, objective_name);
    if (objective == nullptr) {
        return;
    }
    if (!host.observer_pair_registered(*objective, unit)) {
        host.observer_register_pair(*objective, unit);
    }
    objective->entries.push_back(objective_unit_entry_008dee50(unit));
    if (!objective_marker_call_runs(objective->kind, objective->state)) {
        return;
    }
    host.marker_bind_on_add_stub(kObjectiveMarkerClass, unit);
    host.objective_add_tail(kObjectiveMarkerClass, unit);
}

void objective_set_add_position_008df5d0(ObjectiveUnitHost& host, int slot,
                                          const std::string& objective_name,
                                          const ObjectiveWorldPoint& point)
{
    ObjectiveUnitList* objective = host.objective_in_slot(slot, objective_name);
    if (objective == nullptr) {
        return;
    }
    objective->entries.push_back(objective_position_entry_008deeb0(point));
    if (!objective_marker_call_runs(objective->kind, objective->state)) {
        return;
    }
    // The position twin of the add-time marker call, 006de3e0, is also a bare RET 8.
    host.marker_bind_on_add_stub(kObjectiveMarkerClass, nullptr);
}

void objective_walk_unit_markers_008dfe50(ObjectiveUnitHost& host, ObjectiveUnitList& objective)
{
    if (objective.kind == kObjectiveKindHidden) {
        return; // 008dfe6e
    }
    std::vector<void*> live;
    for (const ObjectiveUnitEntry& entry : objective.entries) {
        if (entry.unit != nullptr) {
            live.push_back(entry.unit);                              // 008dfedd
            host.marker_rebind_unit(kObjectiveMarkerClass, entry.unit); // 008dff3b
        } else {
            host.marker_clear_position(kObjectiveMarkerClass, entry.position); // 008dffd2
        }
    }
    // 008e0010..008e003d: every unit collected above is handed to the set-level removal with
    // the objective's own name, so the walk empties the objective of its live units.
    for (void* unit : live) {
        host.set_remove_unit(objective.name, unit);
    }
}

namespace {

// Arguments 0 and 1 of both bindings. 008cde73 and 008cdf19 test each for being an integer
// before reading it; a non-integer argument simply does not contribute.
struct ObjectiveSelectorArguments {
    bool have_party{false};
    int party{0};
    bool have_slot{false};
    int slot{0};
};

ObjectiveSelectorArguments read_selector(LuaBindingArgumentReader& args)
{
    ObjectiveSelectorArguments out{};
    if (args.count() > 0 && args.is_nil(0) == false) {
        out.have_party = true;
        out.party = args.get_integer(0);
    }
    if (args.count() > 1 && args.is_nil(1) == false) {
        out.have_slot = true;
        out.slot = args.get_integer(1);
    }
    return out;
}

} // namespace

int lua_binding_objectives_add_unit_008cdd60(LuaBindingArgumentReader& args,
                                              ObjectiveUnitHost& host,
                                              const std::vector<ObjectiveTarget>& targets)
{
    const ObjectiveSelectorArguments selector = read_selector(args);
    const unsigned int mask = objective_target_slot_mask(host, selector.have_party,
                                                         selector.party, selector.have_slot,
                                                         selector.slot);
    const std::string name = args.get_string(kObjectiveNameArgument);
    for (int slot = 0; slot < kObjectiveSetSlotCount; ++slot) {
        if ((mask & (1u << slot)) == 0) {
            continue; // 008ce2f6
        }
        for (const ObjectiveTarget& target : targets) {
            if (target.is_unit) {
                objective_set_add_unit_008df2b0(host, slot, name, target.unit);
            } else {
                objective_set_add_position_008df5d0(host, slot, name, target.point);
            }
        }
    }
    return 0;
}

int lua_binding_objectives_remove_unit_008ce510(LuaBindingArgumentReader& args,
                                                 ObjectiveUnitHost& host,
                                                 const std::vector<ObjectiveTarget>& targets)
{
    const ObjectiveSelectorArguments selector = read_selector(args);
    const unsigned int mask = objective_target_slot_mask(host, selector.have_party,
                                                         selector.party, selector.have_slot,
                                                         selector.slot);
    const std::string name = args.get_string(kObjectiveNameArgument);
    for (int slot = 0; slot < kObjectiveSetSlotCount; ++slot) {
        if ((mask & (1u << slot)) == 0) {
            continue;
        }
        // The removal helpers take the same (name, target) pair as the add helpers; only
        // their dispatch was read, so the erase itself stays behind the host.
        static_cast<void>(slot);
        for (const ObjectiveTarget& target : targets) {
            if (target.is_unit) {
                host.set_remove_unit(name, target.unit);
            } else {
                host.set_remove_position(name, target.point);
            }
        }
    }
    return 0;
}

} // namespace bsp
