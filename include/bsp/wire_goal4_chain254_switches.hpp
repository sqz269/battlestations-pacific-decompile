#pragma once

namespace bsp {
// Goal 4 chain 254: publish the live plane controller's forward-speed latch
// through 007D80C0 and retain its +78h rate. OFF keeps the host assignment.
inline constexpr bool kWireGoal4Chain254Bound = true;
}
