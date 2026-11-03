#pragma once

namespace bsp {
// Goal 4 chain 306: the in-mission camera live paths.
// - 0079A3B0 (movie camera update) runs 0078FAF0 on the +51Ch ground-offset
//   state after it sets the goal (call at 0079B691, reached on every path with
//   +4FCh clear). OFF keeps the old arm, which set the goal and never stepped
//   the state.
// - 00432ED0 (ship-captain camera update) sets its basis rows 2 and 1 through
//   0042D5A0 (call at 004330BF). OFF keeps the inline row copies and 0085DC80.
inline constexpr bool kWireGoal4Chain306Bound = true;
}
