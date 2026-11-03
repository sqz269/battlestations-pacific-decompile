#pragma once

namespace bsp {
// Goal 4 chain 254: publish the live plane controller's forward-speed latch
// through 007D80C0 and retain its +78h rate; publish deck-clamp contact through
// 007D7D70. OFF keeps the host's speed assignment and contact byte.
inline constexpr bool kWireGoal4Chain254Bound = true;
}
