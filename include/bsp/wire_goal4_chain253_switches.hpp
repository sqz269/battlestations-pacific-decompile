#pragma once

namespace bsp {
// Goal 4 chain 253: construct Lua listener scalar/string arguments through
// 008849D0/00884A00/00884A30/00884FC0 before the interpreter consumes them.
inline constexpr bool kWireGoal4Chain253Bound = true;
}
