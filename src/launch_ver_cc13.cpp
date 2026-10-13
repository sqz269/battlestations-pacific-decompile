// Carrier and airfield launch side, verification models. Packet cc13-ver15;
// include/bsp/launch_ver_cc13.hpp carries the per-function evidence and the
// adoption note, docs/CC13_VER_15.md the verification tables.
// Reconstructed and build-tested only: not wired, not game-run.

#include "bsp/launch_ver_cc13.hpp"

#include <algorithm>

namespace bsp {

std::int32_t air_ops_consume_stock_006c7d10(std::vector<AirOpsStockEntry>& stock,
                                            std::uint32_t vehicle_class,
                                            std::int32_t count) noexcept {
    for (AirOpsStockEntry& entry : stock) {           // 006C7D40-006C7D4F
        if (entry.vehicle_class != vehicle_class) continue;
        const std::int32_t take = entry.count < count ? entry.count : count;  // 006C7D88 JL
        entry.count -= take;                           // 006C7D8E SUB [ESI+0Ch],EAX
        return take;
    }
    return -1;                                         // 006C7D6C JE 006C7DD9
}

namespace {
AirOpsQueueConsumeResult queue_slot(AirOpsDeck& deck, int slot_index) noexcept {
    AirOpsQueueConsumeResult out;
    if (slot_index < 0 || static_cast<std::size_t>(slot_index) >= deck.slots.size()) return out;
    AirOpsSlot& slot = deck.slots[static_cast<std::size_t>(slot_index)];
    if (slot.state != AirOpsSlotState::kCooldown && slot.state != AirOpsSlotState::kReady) {
        return out;                                    // 006CA65E JNE 006CA71D
    }
    slot.state = AirOpsSlotState::kLaunching;          // 006CA66B
    slot.timer = 0.0F;                                 // 006CA672
    if (slot.launch_requested) {                       // 006CA664 / 006CA677
        slot.timer = kAirOpsSlotCooldownSeconds;       // 006CA679 00CE3850 = 5.0
        slot.launch_requested = false;                 // 006CA686
    }
    auto& queue = deck.launch_queue;
    const bool present = std::find(queue.begin(), queue.end(), slot_index) != queue.end();
    if (!present) {
        out.consumed = air_ops_consume_stock_006c7d10(deck.stock, slot.vehicle_class,
                                                      slot.assigned_count);  // 006CA6D5
    } else {
        queue.erase(std::remove(queue.begin(), queue.end(), slot_index), queue.end());  // 006C48F0
        out.requeued = true;
    }
    queue.push_back(slot_index);                       // 006CA6F7-006CA714
    out.queued = true;
    return out;
}
}  // namespace

AirOpsQueueConsumeResult air_ops_queue_slot_consume_006ca640(AirOpsDeck& deck,
                                                             int slot_index) noexcept {
    return queue_slot(deck, slot_index);
}

AirOpsQueueConsumeResult air_ops_launch_busy_arm_006cc690(AirOpsDeck& deck,
                                                          int slot_index) noexcept {
    if (slot_index < 0 || static_cast<std::size_t>(slot_index) >= deck.slots.size()) return {};
    const AirOpsSlot& slot = deck.slots[static_cast<std::size_t>(slot_index)];
    // 006CC71B-006CC71F: 006CA770(class, count) on the block, the same call the
    // host makes for the C7h stock return (game_hosts_units.cpp:14197).
    AirOpsStockEntry created{};
    const AirOpsStockAddResult added = air_base_stock_add_006ca770(
        deck.stock.data(), static_cast<int>(deck.stock.size()),
        static_cast<int>(deck.stock.size()) + 1, slot.vehicle_class, slot.assigned_count,
        &created);
    if (added.created_entry) deck.stock.push_back(created);
    return queue_slot(deck, slot_index);               // 006CC727
}

}  // namespace bsp
