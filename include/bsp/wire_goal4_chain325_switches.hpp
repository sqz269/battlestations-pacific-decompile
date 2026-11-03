#pragma once

namespace bsp {
// Goal 4 chain 325: the unit gunnery pass's two policy objects. The attach
// (00864BD0 -> 00864C18) runs 008636A0 once per unit, and the pass reads the
// installed fire-target provider (00863640 or the null 00861B90) and category
// gate (00861BE0 or 008627A0) instead of re-deriving them from class tests.
inline constexpr bool kWireGoal4Chain325Bound = true;
}
