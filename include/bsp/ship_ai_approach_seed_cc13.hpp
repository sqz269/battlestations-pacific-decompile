#pragma once
// Packet cc13-apseed: the construction seed of the approach timers
// nested+121Ch, +1220h and +1224h (bsp/ship_ai_approach_update.hpp).
//
// 009E5530 (the attackmove ring constructor, reached once per ship brain
// through 009E8450 -> 009E5CA0) stores -1.0f ([00D7A260]) into all three:
// 009E5625 (+121Ch), 009E5659 (+1220h) and 009E5661 (+1224h). The approach
// enter 009F30F0 does not touch them (it re-seeds only +11F4h at 009F31B5),
// then calls 009F1BC0(nested, 0.0f) at 009F31BD. With dt = 0 the countdowns at
// 009F1C07 / 009F1C13 / 009F1E1E leave the seed in place, so on the first
// approach of a brain's life:
// - 009F2EC1 (0 > +1220h) refills the own range curve (009F2F11);
// - 009F2F84 (0 > +1224h) refills the target curve (009F2FB1) when a vehicle
//   target is set and mode +1234h is 0;
// - 009F20CD (0 > +121Ch) takes the retarget reset (+11D6h = 0,
//   +11D8h = +121Ch = 1.0f).
// The host constructs the three at 0.0f, so each of these fires one pass late.
//
// True: ShipAiApproachState constructs the three at -1.0f. The state is a
// member of the host's per-unit controller, value-initialised once when the
// unit registers (GameShipAiHost::register_units / register_generated_units),
// which is where the brain is constructed; the host's enter
// (ship_ai_approach_enter_009f3220) does not reset them, as 009F30F0 does not.
// False: they construct at 0.0f, as before.
namespace bsp {

inline constexpr bool kShipAiApproachTimerSeedBound = true;  // cc9-sweep2b: paired at 292c293bd on 7 rows (j8v3 loses, see SHIP_AI_OPEN_ITEMS 331)

inline constexpr float kShipAiApproachTimerSeed =
    kShipAiApproachTimerSeedBound ? -1.0f : 0.0f;  // [00D7A260] when bound

}  // namespace bsp
