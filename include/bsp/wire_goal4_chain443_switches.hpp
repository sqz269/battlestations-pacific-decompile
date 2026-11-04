#pragma once

namespace bsp {
// Mission argument pushes and numeric-key scalar table entries use the live
// mission Lua state. OFF retains the direct C API calls in the argument host.
inline constexpr bool kWireGoal4Chain443Bound = true;
} // namespace bsp
