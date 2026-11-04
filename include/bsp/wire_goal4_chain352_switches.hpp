#pragma once

// Goal 4 chain 352: the mission Lua AddListener (008C6760 -> 00980C10) builds the
// kill, entityKilled, exitzone, recon and hit subscriptions with the image's own
// constructors and Load slots (0097A450 / 00972520, 00972F00, 0097A620 / 009726D0,
// 0097A220 / 00972450, 009725B0) over the
// GuiLuaReader 004425C0 builds on the block, and the listener dispatchers match the
// loaded filters; the entityKilled reason parameter is the 0096DC00 string box. The
// other eighteen channels 0097E360 accepts are loaded and kept the same way, for
// dispatchers the host does not run yet.
// OFF keeps the host's own block parse and per-call subscription build.

namespace bsp {
inline constexpr bool kWireGoal4Chain352Bound = true;
}
