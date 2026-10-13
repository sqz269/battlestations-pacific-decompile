// Packet cc13-fix5 model bodies. Header: include/bsp/script_natives_fix5_cc13.hpp;
// docs/CC13_FIX_5.md. Build-tested only.

#include "bsp/script_natives_fix5_cc13.hpp"

#include <algorithm>

namespace bsp {

AirBaseRemoveStockResult air_base_remove_stock_006c7b10(
    AirOpsDeck& deck, std::uint32_t vehicle_class, std::int32_t count,
    const std::int32_t* launched_plane_counts,
    std::vector<std::size_t>* cleared_slots) noexcept {
    AirBaseRemoveStockResult out;
    // 006C7B3F: the first node whose class matches.
    for (AirOpsStockEntry& entry : deck.stock) {
        if (entry.vehicle_class != vehicle_class) continue;
        // 006C7B8F..006C7BA6: signed min(count, node count), subtracted.
        const std::int32_t taken = count <= entry.count ? count : entry.count;
        entry.count -= taken;
        out.removed = taken;
        out.found_entry = true;
        break;
    }
    // 006C7BD7: trim idle slots while the class's available stock is negative.
    while (air_ops_stock_available_006bf230(deck, vehicle_class, launched_plane_counts)
               .available < 0) {
        // 006C7BF0..006C7C1C: the walk keeps overwriting its pick, so the LAST
        // qualifying slot is the one emptied.
        std::size_t pick = deck.slots.size();
        for (std::size_t i = 0; i < deck.slots.size(); ++i) {
            const AirOpsSlot& slot = deck.slots[i];
            const std::int32_t state = static_cast<std::int32_t>(slot.state);
            if ((state == static_cast<std::int32_t>(AirOpsSlotState::kCooldown)
                 || state == static_cast<std::int32_t>(AirOpsSlotState::kReady))
                && slot.vehicle_class == vehicle_class) {
                pick = i;
            }
        }
        if (pick == deck.slots.size()) break;   // SUBSTITUTION: the image spins
        AirOpsSlot& slot = deck.slots[pick];
        if (slot.vehicle_class != 0u) {
            slot.vehicle_class = 0u;     // +4h
            slot.class_field_134 = 0;    // +10h
        }
        slot.assigned_count = 0;         // +8h
        slot.state = AirOpsSlotState::kCooldown;  // +2Ch = 1
        slot.timer = 0.0F;               // +30h
        if (slot.launch_requested) {     // +34h
            slot.timer = kAirOpsSlotCooldownSeconds;  // 00CE3850
            slot.launch_requested = false;
        }
        // 006C7C66 006C48F0: std::list::remove of the index from block+10h.
        deck.launch_queue.erase(std::remove(deck.launch_queue.begin(), deck.launch_queue.end(),
                                            static_cast<int>(pick)),
                                deck.launch_queue.end());
        ++out.slots_cleared;
        if (cleared_slots != nullptr) cleared_slots->push_back(pick);
    }
    return out;
}

AirBaseRemoveAllStockResult air_base_remove_all_stock_006c8840(
    AirOpsDeck& deck, const std::int32_t* launched_plane_counts,
    std::vector<std::size_t>* cleared_slots) noexcept {
    AirBaseRemoveAllStockResult out;
    out.entries = deck.stock.size();
    // 006C8858..006C887E: node order; each call reads the node's own count.
    for (std::size_t i = 0; i < deck.stock.size(); ++i) {
        const AirOpsStockEntry entry = deck.stock[i];
        const AirBaseRemoveStockResult step = air_base_remove_stock_006c7b10(
            deck, entry.vehicle_class, entry.count, launched_plane_counts, cleared_slots);
        out.planes_removed += step.removed;
        out.slots_cleared += step.slots_cleared;
    }
    // 006C8884..006C88B6: unlink, block+48h = 0, free every node.
    deck.stock.clear();
    return out;
}

} // namespace bsp
