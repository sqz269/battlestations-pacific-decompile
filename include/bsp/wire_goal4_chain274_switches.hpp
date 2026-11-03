#pragma once

namespace bsp {

// Goal 4 chain 274 (slot g4-w1), reports/wire-g4-c274.json. ON: the live
// mission-checkpoint bodies call the native helpers at their native call
// sites: 00438020 writes each script slot (00438747) and each other mission's
// entry (004388C6) through 00435D30, and 004379C0 fills the copied entry
// through 00436730 (00437B21). OFF keeps the inlined helper calls.
inline constexpr bool kWireGoal4Chain274Bound = true;

}  // namespace bsp
