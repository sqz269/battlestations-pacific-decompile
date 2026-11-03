#pragma once

namespace bsp {
// Goal 2, chain 48 (game_hosts_menu.cpp): the front-end frame's 004E4BBA..
// 004E4BC8 retention test reads the renderer's actual 00F8D440 queue through
// 00B1BF90 instead of answering a constant false.
inline constexpr bool kWireGoal2Chain48Bound = true;
}
