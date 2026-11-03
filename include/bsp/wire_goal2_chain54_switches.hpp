#pragma once

namespace bsp {
// Goal 2, chain 54 (game_hosts_menu.cpp): the front-end managers' load block.
// 00686380 builds "GVMainMenu" in a 00426500 log builder, constructs its 1Ch
// FileBlockScope with 00BE0AE0 at 006863C4 (gate 1) on the live VFS 0109CEEC,
// destroys the builder at 006863D2 and leaves the block with 00BDCB30 at
// 006868E4. ON runs that pair over the VFS host's native FileBlock context
// instead of only recording the two calls.
inline constexpr bool kWireGoal2Chain54Bound = true;
}
