#pragma once

namespace bsp {
// Goal 4 chain 286: the mission Lua host's `kill`, `entityKilled` and `hit`
// listener channels test their subscriptions through the image's own matchers
// 0096ACE0 / 00968FF0 (vtable slot 3 of 00D1B68C / 00D1B7D0) and the range
// predicate 00968440 (bsp/lua_listener_native_matchers.hpp). OFF keeps the
// host's inline set and bracket comparisons.
inline constexpr bool kWireGoal4Chain286Bound = true;
}
