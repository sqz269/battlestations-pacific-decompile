#pragma once

namespace bsp {
// Goal 4 chain 340: the mission Lua machine wrappers of segment 29 (006B8xxx) at
// the live host sites where the original calls them, borrowing the host's
// interpreter through a non-owning LuaMachine (006B7E30).
inline constexpr bool kWireGoal4Chain340Bound = true;
} // namespace bsp
