#pragma once

// Packet cc13-fix5: model bodies for Lua script natives cc13-ver12 found
// unhosted (or hosted with a divergence). docs/CC13_FIX_5.md.
//
// Semantic interfaces read from the disk bytes (disasm-raw) and the Ghidra
// pseudocode, not native layouts or ABI-compatible replacements. Names are
// hypotheses. Each native's host wiring sits behind its own OFF switch in the
// host file that dispatches it; these bodies are build-tested only.

#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/air_operations.hpp"
#include "bsp/script_orders_ver_cc13.hpp"

namespace bsp {

// 008A75C0 `NavigatorCruise(unit)`. NavigatorStop's twin with the cruise command:
// argument 0 through 00888AA0 (008A76BE; no null test), a descriptor with kind,
// position_valid, id, object and +14h all zero and the vector copied from
// 00F87574..7C (008A76D6-008A7723), and 0077D600(entity, cruise 00E08F70,
// &descriptor, 1) at 008A7729 (PUSH 1 008A76EC, PUSH 0E08F70h 008A770F). Nothing
// is pushed. `zero_vector` is the content of 00F87574 (all zero in the image).
ScriptOrderIssue navigator_cruise_order_008a75c0(const float zero_vector[3]) noexcept;

// ---------------------------------------------------------------------------
// 006C7B10 BSP_AirOps_RemoveStockAndTrimSlots, __thiscall(block, class, count),
// RET 8. Two halves:
//  1. 006C7B3F..006C7BA9: the first stock node (list block+44h) whose +8h is
//     the class loses min(count, node+0Ch) (the signed compare keeps the
//     argument when it is <= the node's count). A missing node changes nothing.
//  2. 006C7BD7..006C7CB8, under block+0Ch's critical section: while
//     006BF230(class).available < 0, the LAST slot (stride 58h, count block+50h)
//     whose state is 1 or 5 and whose +4h is the class is emptied: +4h and +10h
//     zeroed (only when +4h is non-zero), +8h = 0, +2Ch = 1, +30h = 0, and when
//     +34h is set +30h = 5.0 (00CE3850) and +34h = 0; its index is removed from
//     the launch list at block+10h (006C48F0, 006C7C66); then 00696350 on
//     block+7Ch (EDX = 4) and the 006BD520 slot message to every peer (0077C7B0).
//     slot+28h is not touched.
//  Last, 00984EB0(block+7Ch, class+70h, 006BF100(class+70h)), the "stock" notice.
// `launched_plane_counts` is per slot, the squadron's +3CCh for a slot whose
// +28h is set (what 006BF230 subtracts for it); it may be null when no slot has
// launched. `cleared_slots` (optional) receives each emptied slot in order.
// SUBSTITUTION (labelled): when no slot qualifies while available < 0 the image
// loops forever; the model stops. available < 0 needs a qualifying slot, since
// the stock total is never negative after half 1, so the stop is unreachable
// on any state the image can reach.
struct AirBaseRemoveStockResult {
    std::int32_t removed{0};        // subtracted from the node
    bool found_entry{false};
    std::size_t slots_cleared{0};
};
AirBaseRemoveStockResult air_base_remove_stock_006c7b10(
    AirOpsDeck& deck, std::uint32_t vehicle_class, std::int32_t count,
    const std::int32_t* launched_plane_counts,
    std::vector<std::size_t>* cleared_slots) noexcept;

// 006C8840, __fastcall(block). Walks the stock list at block+44h in order and
// calls 006C7B10(node+8h, node+0Ch) for each node (006C8874), then unlinks
// every node, zeroes block+48h (the list size) and frees them (006C88A0..). So
// every class's stock drops to zero, every idle (state 1/5) slot holding a
// stocked class is emptied by 006C7B10's trim, and the list is left empty.
// Returns the number of stock entries that were present.
struct AirBaseRemoveAllStockResult {
    std::size_t entries{0};
    std::int32_t planes_removed{0};
    std::size_t slots_cleared{0};
};
AirBaseRemoveAllStockResult air_base_remove_all_stock_006c8840(
    AirOpsDeck& deck, const std::int32_t* launched_plane_counts,
    std::vector<std::size_t>* cleared_slots) noexcept;

// 006C7D10 BSP_AirOps_ConsumeStock, __thiscall(block, class, count): the first
// stock node of the class loses min(count, node count) (the signed compare keeps
// the argument when it is <= the count); then the 85h stock-item message to the
// peers (006BD9E0, 0077C7B0) and the 00984EB0 notice, which the caller records.
// Returns the node's count afterwards, or -1 when no node holds the class.
std::int32_t air_base_consume_stock_006c7d10(AirOpsDeck& deck, std::uint32_t vehicle_class,
                                             std::int32_t count) noexcept;

// 006CA8E0 BSP_AirOps_LaunchSlotMessage83, __thiscall(block, slot index, flag):
// the receiver of the 83h message LaunchAirBaseSlot 00896750 routes.
//  - flag set: 006C7490(slot, 1), then 006C7D10(slot+4h, slot+8h).
//  - flag clear: slot+2Ch = 2, +30h = 0 (5.0 and +34h cleared when +34h is set),
//    the index removed from the launch list (006C48F0) and appended to its tail
//    (004857F0 at block+14h), 006BF150 publishes the slot, then 006C7D10.
// There is no state test here; the native refused state 2 before routing.
struct AirOpsLaunchSlotMessageResult {
    bool started{false};   // the flag arm ran 006C7490
    bool queued{false};    // the queue arm
    std::int32_t stock_left{-1};
};
AirOpsLaunchSlotMessageResult air_ops_launch_slot_message_006ca8e0(
    AirOpsDeck& deck, int slot_index, bool flag) noexcept;

} // namespace bsp
