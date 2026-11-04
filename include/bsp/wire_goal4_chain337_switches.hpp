#pragma once

namespace bsp {

// Use the recovered profiler clock calibration and counter operations on the
// application's live records. False restores the previous host operations.
inline constexpr bool kWireGoal4Chain337Bound = true;

} // namespace bsp
