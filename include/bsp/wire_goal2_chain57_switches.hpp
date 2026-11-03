#pragma once

namespace bsp {
// Goal 2, chain 57 (game_hosts_menu.cpp): the main-menu manager's title music
// stream, [00E198AC]+50h. 005884A0 allocates the 54h stream (005884D5),
// constructs it with 00A877D0 at 00588528 over the manager's +40h path, starts
// it with 00A867B0 at 0058855C and requests its fade-in with 00A85C20 at
// 005885AD. 00686C90 releases it at 00686EDE..00686EF0 when the mission load
// destroys the manager. ON builds that stream on the live sound core and runs
// those bodies instead of only recording the calls.
inline constexpr bool kWireGoal2Chain57Bound = true;
}
