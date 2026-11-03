#pragma once

namespace bsp {
// Route the command-building D3h (SETOWNER) and D4h (SETLEVEL) messages
// through 0077C2A0's single-player local copy (0076E520: the 006F2520 /
// 006F25E0 write, 00768530's 00764220 / 00764280 construct, the 006F2550 /
// 006F2610 read) and deliver the copy by its type.
inline constexpr bool kWireGoal4Chain326Bound = true;
} // namespace bsp
