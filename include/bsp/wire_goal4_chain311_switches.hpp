#pragma once

namespace bsp {
// Goal 4 chain 311: closed explicit IA scene properties keep an owning int32
// payload and join the native bag merge through 008F3590 (type 10); the eight
// reference letters with one quoted target join it through 008F3AC0 (type 5).
inline constexpr bool kWireGoal4Chain311Bound = true;
}
