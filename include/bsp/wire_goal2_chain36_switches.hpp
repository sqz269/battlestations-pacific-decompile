#pragma once

namespace bsp {
// Goal 2, chain 36, three arms in src/game_hosts_mission_frame.cpp:
// - 00976F10 cancel-by-target in the loss report (009813A0), over a +E0h list
//   view of the host's pending warnings (PendingWarningCancelView);
// - 00922A90 land-family class test in the default kill block (00982044);
// - 009681D0 target live test in the hit dispatcher (0098986A), own fire only
//   (local attacker, relation 0); otherwise the host's live lambda stays.
inline constexpr bool kWireGoal2Chain36Bound = false;
}
