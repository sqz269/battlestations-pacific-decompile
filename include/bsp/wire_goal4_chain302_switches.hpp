#pragma once

namespace bsp {
// Goal 4 chain 302: the application's embedded input configuration (game+3Ch,
// 00698680 / 00698A10 / 004DCEB0) loaded at OnInitOnce 004DD745 and at the
// press-start commit (0067CEB9 / 0067D50F), retired before the singleton drain.
inline constexpr bool kWireGoal4Chain302Bound = true;
}
