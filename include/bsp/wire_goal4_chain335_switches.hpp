#pragma once

namespace bsp {
// Goal 4 chain 335. The gunnery host's three ship-hit/contact sends, MT_SHIP_ADDTORQUE
// (93h), the kamikaze detonate (70h) and MT_SHIPREPAIR_ADDDAMAGE (9Eh), travel through
// 0076E520's local copy (writer, 00768530's constructor, reader) and are delivered from
// the copy. OFF keeps the posted values delivered as they were.
inline constexpr bool kWireGoal4Chain335Bound = true;
}
