#pragma once

// Goal 4 chain 352: the mission Lua AddListener (008C6760 -> 00980C10) builds the
// kill, entityKilled and exitzone subscriptions with the image's own constructors
// and Load slots (0097A450 / 00972520, 00972F00, 0097A620 / 009726D0) over the
// GuiLuaReader 004425C0 builds on the block, and the listener dispatchers match the
// loaded filters; the entityKilled reason parameter is the 0096DC00 string box.
// OFF keeps the host's own block parse and per-call subscription build.

namespace bsp {
inline constexpr bool kWireGoal4Chain352Bound = true;
}
