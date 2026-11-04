#pragma once

namespace bsp {

// Goal 4 chain 369 (agent/g4-w2). True: the submarine catapult predicate
// 008522C0 answers from the unit's catapult devices and the catapult launch
// registry at its three host sites (the dive law 00936DC0, GetSubmarineDepthLevel
// 00894100 and 00852410), and a submarine's torpedo at a submarine swims at
// 0072C970's depth, 00852410's prediction for the target. False: 008522C0 reads
// false and every torpedo keeps 00856050's -2.5, as before.
inline constexpr bool kWireGoal4Chain369Bound = true;

}  // namespace bsp
