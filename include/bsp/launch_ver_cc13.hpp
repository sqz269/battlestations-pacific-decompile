#pragma once

// Carrier and airfield launch side, verification models. Packet cc13-ver15;
// docs/CC13_VER_15.md.
//
// Semantic interfaces read from the disk bytes (Capstone disasm-raw), not native
// layouts or ABI-compatible replacements. Build-tested only; not wired, not
// game-run.
//
// ADOPTION NOTE (cc9 terms). src/air_operations.cpp and
// include/bsp/air_operations.hpp were leased to cc13-fix5 when this packet ran,
// so the divergences below are not fixed in the host; these models are what the
// owner adopts.
// - Replaces: bsp::air_ops_queue_slot_006ca640 (src/air_operations.cpp:932),
//   whose 006C7D10 call is skipped ("not read"), and the busy arm of
//   bsp::air_ops_launch_squadron_006cc690 (src/air_operations.cpp:630-633),
//   which sets `queued` and never queues.
// - Inputs: the deck's stock list and queue as the host already keeps them.
// - Effects: the stock goes DOWN by the slot's count when a slot is queued
//   (player/harness launches through 006CCDA0 order 3, and 006CC690 while
//   block+38h is set). With the C7h stock return (game_hosts_units.cpp:14198,
//   ON) a returning sortie nets to zero, as in the image. Predicted: JM08 and
//   USN21 player strikes stop once their class stock is spent (JM08 airfield
//   class 166: 3 sorties of 3 against stock 9, where the host flew 7; USN21:
//   about 13 sorties per carrier class against 40, where the host flew 16-25).
//   PRCP01 stays inside its stock. No RNG draw.
// - Switch: the adopter puts it behind its own OFF `k...Bound`.

#include "bsp/air_operations.hpp"

#include <cstdint>
#include <vector>

namespace bsp {

// 006C7D10 BSP_AirOps_ConsumeStock, __thiscall(block, class, count), RET 8.
// 006C7D40-006C7D4F walks the stock list at block+40h for the node whose +8h is
// the class; 006C7D81-006C7D8E: entry.count -= min(entry.count, count) (JL takes
// the entry when it is the smaller). It then builds the STOCKITEM message
// (006BD9E0, class and the new count) and routes it through owner block+7Ch
// (0077C7B0), and on every path updates the party's statistics with 006BF100's
// category total (00984EB0 on 00F8A0C4). The message and the statistics call
// are host callbacks the model does not make. Returns the amount taken, or -1
// when the class has no stock entry (the image then only runs the statistics
// call).
std::int32_t air_ops_consume_stock_006c7d10(std::vector<AirOpsStockEntry>& stock,
                                            std::uint32_t vehicle_class,
                                            std::int32_t count) noexcept;

struct AirOpsQueueConsumeResult {
    bool queued{false};
    bool requeued{false};        // the slot was already in the queue: unlinked and re-appended
    std::int32_t consumed{-2};   // -2: no consume call; otherwise 006C7D10's answer
};

// 006CA640 BSP_AirOps_QueueSlot, __thiscall(block, slot), with the stock arm.
// 006CA653-006CA65E: only a state 1 or 5 slot. 006CA664-006CA686: state 2,
// timer 0, or 5.0 (00CE3850) with +34h cleared. 006CA6A0-006CA6AB: find the
// slot index in the queue at block+10h; 006CA6C5: not found -> 006C7D10(slot+4,
// slot+8) at 006CA6D5; found -> 006C48F0, which is list::remove(value) (every
// node equal to the index). 006CA6E8-006CA714: append at the tail. 006CA716:
// 006BF150 (the slot notice) is not modelled.
AirOpsQueueConsumeResult air_ops_queue_slot_consume_006ca640(AirOpsDeck& deck,
                                                             int slot_index) noexcept;

// 006CC690's busy arm, 006CC715-006CC72C: when block+38h is non-zero the image
// does not start the launch; it returns the count to the stock first
// (006CA770(class, count) at 006CC71F) and then queues the slot (006CA640 at
// 006CC727), so the stock nets to zero and 006C64B0 launches the slot about one
// second after the brake clears. The class and count are already in the slot
// (006CC6F5, 006CC70F). Call after the slot has been written.
AirOpsQueueConsumeResult air_ops_launch_busy_arm_006cc690(AirOpsDeck& deck,
                                                          int slot_index) noexcept;

}  // namespace bsp
