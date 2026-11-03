#pragma once

namespace bsp {
// Goal 4 chain 259: release the deck launch brake using 007ED740 and
// 007B8BD0 over the live squadron members' flight state and ground contact.
// OFF retains the host's inline readiness check.
inline constexpr bool kWireGoal4Chain259Bound = true;
}
