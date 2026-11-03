#pragma once

// Goal 2 chain 166 (slot g2-w5). True: EnableMessages(false) with no entity
// (008CFE40, 008D000A) runs 0096BF70 BSP_WarningManager_ClearWaitingQueue over
// the mission frame's pending warning queue (the manager's +E0h list).
// False: the drain stays the old no-op.
namespace bsp {
inline constexpr bool kWireGoal2Chain166Bound = false;
}
