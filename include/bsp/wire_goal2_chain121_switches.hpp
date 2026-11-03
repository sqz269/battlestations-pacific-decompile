#pragma once

// Goal 2, chain 121 (slot g2-w4), segment 20 range sweep. Every wire of this
// chain sits behind this one switch; it lands OFF. reports/wire-g2-c121.json
// lists the rows, their host sites and the rows skipped with their reasons.
namespace bsp {
// ON: the kill and hit warning constructors (warning_reports_cc13.cpp) set
// their watched-entity slots through 005B7F60 observer_slot_set over a typed
// view of the warning's slot. OFF: the inline set_slot compare/unlink/store/link.
inline constexpr bool kWireGoal2Chain121Bound = false;
} // namespace bsp
