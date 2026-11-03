#pragma once

namespace bsp::game {
// Goal 4 chain 291: the HUD root's per-frame unit rows 00648C20
// (hud_root_update_rows) run from screen 44h's update 00649860 over this
// process's HUD. OFF keeps the old record.
inline constexpr bool kWireGoal4Chain291Bound = true;
}
