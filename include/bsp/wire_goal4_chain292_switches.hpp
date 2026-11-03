#pragma once

namespace bsp {

// 0071DDB0's release walk calls director vtable[70h] through
// 0071EDD0 BSP_WeaponDirector_TargetStillValid on the actual dying entity, its
// squadron (plane_squadron_registry, the plane+9D4h stand-in) and the live
// members' +5Dh bytes and +FCh poses. A plane target with a live squadron mate
// is moved to the last live member among the first five and keeps its command
// (vt70 answers 0: no 0071D810 raise, no 0071D900 clear); anything else takes
// the point branch as before. Squadron command blocks (vtable 00D0BD98, slot
// 70h 0084DA60) keep the old path.
inline constexpr bool kWireGoal4Chain292Bound = true;

}  // namespace bsp
