#pragma once

namespace bsp {
// C5 touchdown notifications use the native constructor and local bitstream
// copy before entering the existing landing-state delivery.
inline constexpr bool kWireGoal4Chain294Bound = true;
} // namespace bsp
