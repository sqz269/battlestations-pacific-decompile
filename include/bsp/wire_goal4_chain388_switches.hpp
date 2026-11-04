#pragma once

namespace bsp {

// Goal 4 chain 388: retain the private plane-tuning interpreter's native Lua
// owner, install DoFile through 00B65E20 and read formation triples through
// 00B67A80. OFF retains the direct Lua API registration and element reader.
inline constexpr bool kWireGoal4Chain388Bound = true;

} // namespace bsp
