#ifndef BSP_CC13_KZ_LATCH_HPP
#define BSP_CC13_KZ_LATCH_HPP

// Packet cc13-kzlatch: the kamikaze compensating pair of docs/CC13_SQRTB.md
// section 4, both switches committed OFF. docs/CC13_KZ_LATCH.md carries the
// evidence and the expected movers.

namespace bsp::cc13_kz_latch {

// 009F81CD 007ED790(squadron, 0) walks the +3D0h members calling
// 007B9140(member, 0); 007B914A asks vtable[5Ch](17h) and answers 1 for any
// MPlaneKamikaze before a rack is read (007B9150 MOV AL,1). So 009F81E9 JNZ
// 009F820E skips the spent latch for a squadron with a live kamikaze. True:
// the units host's 009F8160 ordnance-arm latch loop, and for consistency its
// fighter-end loop (leader IsKindOf(13h), inert for a kamikaze squadron) and
// the census diagnostic (log only), count an IsKindOf(17h) member as holding
// a round, as its other callers of plane_holds_rack_round_007b9140 already
// do. False: those loops ask only the racks, as before, and a kamikaze
// squadron latches spent at its first command. Changes no task: pairable on
// its own.
inline constexpr bool kKamikazeRackRoundLatchCc13Bound = true;  // ON: USN21 staged pairs (docs/SHIP_AI_OPEN_ITEMS.md 306.4)

// The same AL feeds the dead-target retest (009F820E-009F8412,
// kSquadronDeadTargetRetestBound). True: its loop also counts a kamikaze
// member, so a kamikaze squadron whose target dies has its command ended
// (0071E430) and re-issued, as in the image. False: the retest skips a
// kamikaze squadron, as before. Separate because it changes tasks.
inline constexpr bool kKamikazeDeadTargetRetestCc13Bound = true;  // ON: USN21 staged pairs (docs/SHIP_AI_OPEN_ITEMS.md 306.4)
static_assert(!kKamikazeDeadTargetRetestCc13Bound || kKamikazeRackRoundLatchCc13Bound,
              "the kamikaze retest arm is the same 007ED790 answer as the latch arm");

// 007F1709-007F1720: 007F16D0 answers null for a kamikaze head (IsKindOf(17h))
// whose +C24h (PilotFires, 007CD930) is clear. True: record_return_to_base_
// 007f16d0 passes the head's plane_pilot_fires_c24 (kPilotFiresBound).
// False: it passes false, so every kamikaze-led resolution is null, as
// before. Only safe with the latch fix: alone it turns the null answer at a
// kamikaze squadron's spawn-time latch into retreat or land. ON only, the
// 007F16D0 log line also carries the retreat arm's zone result
// (zone-hit / zone-miss, 004C7730).
inline constexpr bool kRtbHeadC24PilotFiresCc13Bound = true;  // ON: USN21 staged pairs (docs/SHIP_AI_OPEN_ITEMS.md 306.4)
static_assert(!kRtbHeadC24PilotFiresCc13Bound || kKamikazeRackRoundLatchCc13Bound,
              "PilotFires into 007F16D0 needs the kamikaze latch arm, or kamikazes go home at spawn");

}  // namespace bsp::cc13_kz_latch

#endif  // BSP_CC13_KZ_LATCH_HPP
