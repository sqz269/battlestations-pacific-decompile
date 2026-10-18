// Ship catapult launches, the host's record and gate. Packet p17_catapult_adapter;
// see include/bsp/ship_catapult_host.hpp for the image chain and the labelled
// substitutions.

#include "bsp/ship_catapult_host.hpp"

namespace bsp {

ShipCatapultRecord& ShipCatapultRegistry::record(std::size_t unit, const ShipCatapultClass& cls) {
    auto it = records_.find(unit);
    if (it == records_.end()) {
        ShipCatapultRecord fresh;
        fresh.slots.pending_638 = cls.launch_stock;   // 0095E5B0: +638h = class +C8h
        fresh.slots.last_slot_630 = -1;               // the constructor's -1
        it = records_.emplace(unit, fresh).first;
    }
    return it->second;
}

ShipCatapultRecord* ShipCatapultRegistry::find(std::size_t unit) {
    auto it = records_.find(unit);
    return it == records_.end() ? nullptr : &it->second;
}

void ShipCatapultRegistry::prune(ShipCatapultRecord& r,
                                 const std::function<bool(std::uint32_t)>& alive) {
    for (std::uintptr_t& child : r.slots.child_550) {
        if (child != 0 && !alive(static_cast<std::uint32_t>(child))) child = 0;
    }
}

std::int32_t ShipCatapultRegistry::count_00953920(const ShipCatapultRecord& r) {
    std::int32_t n = 0;
    for (const std::uintptr_t child : r.slots.child_550) {
        if (child != 0) ++n;   // 00953922.. CMP [ECX+550h+18h*i], 0
    }
    return n;
}

std::uint32_t ShipCatapultRegistry::last_00953a60(const ShipCatapultRecord& r) {
    const std::int32_t i = r.slots.last_slot_630;
    if (i < 0 || i >= static_cast<std::int32_t>(r.slots.child_550.size())) return 0u;
    return static_cast<std::uint32_t>(r.slots.child_550[static_cast<std::size_t>(i)]);
}

ShipCatapultVerdict ShipCatapultRegistry::gate_008141e0(const ShipCatapultRecord& r,
                                                        const ShipCatapultClass& cls) {
    if (cls.launched_class < 0) return ShipCatapultVerdict::kNoCatapult;
    if (r.slots.pending_638 <= 0) return ShipCatapultVerdict::kNoStock;   // 008141EB SETG
    // 006EBD4C, the class part of the device's CanFire.
    if (count_00953920(r) >= cls.max_launched) return ShipCatapultVerdict::kTooManyInAir;
    return ShipCatapultVerdict::kLaunch;
}

int ShipCatapultRegistry::add_child_00957450(ShipCatapultRecord& r, std::uint32_t entity_id) {
    return gameplay_seg61_62_cc13::unit_add_launched_child_00957450(r.slots, entity_id, nullptr);
}

ShipCatapultRegistry& ship_catapult_registry() {
    static ShipCatapultRegistry registry;
    return registry;
}

}  // namespace bsp
