#pragma once

namespace bsp {
// Goal 2 chain 209 (g2-w3): the squadron block's force-release byte +3AEh
// (producer: the Lua native 008A28B0 SquadronSetForceRelease) and the two
// level-bomb aim predicates that read it, 009B7B10 and 009B7B40, called from
// 009B88F0. OFF keeps the inline predicates and the literal false byte.
inline constexpr bool kWireGoal2Chain209Bound = false;
}
