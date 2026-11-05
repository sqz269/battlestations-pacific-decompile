#pragma once

namespace bsp {
// Goal 4 chain 618: expose the two known primitive pair entries at the live
// 00C44090 dispatch boundary. OFF retains its original CALL EDX instruction.
inline constexpr bool kWireGoal4Chain618Bound = true;
} // namespace bsp
