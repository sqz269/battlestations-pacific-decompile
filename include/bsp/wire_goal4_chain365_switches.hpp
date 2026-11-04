#pragma once

// Goal 4 chain 365: the ship repair task at unit+A20h. ON: the 9Eh add arm runs
// 0093A470 / 0093A4F0 on the gunnery host's task fields, and the script orders host
// dispatches FailureRepairEnable (008AD540 -> 00939FE0) and AddFire (008ACD10)
// through their bodies. OFF: the previous inline add and the unhandled bindings.
namespace bsp {
inline constexpr bool kWireGoal4Chain365Bound = true;
}
