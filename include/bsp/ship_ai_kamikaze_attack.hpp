#pragma once

// 009E2020, the kamikaze_attack state's step (packet cc9_kamikaze_attack_step,
// docs/SHIP_AI_OPEN_ITEMS.md section 45).
//
// BSP_ShipAi_KamikazeAttackStateStep (a hypothesis, not a recovered symbol):
// __thiscall(state)(float seconds), RET 4, body 009E2020-009E23A6 (RET 4 at
// 009E23A3, INT3 from 009E23A6). vtable 00D216B8 slot +0Ch; the state object is
// brain+2254h. `seconds` is never read. Read whole from the listing.
//
// With a target at [brain+0B20h] the body is 009E23B0's (the attackmove engage
// sub-state) instruction for instruction: the same range and closing solve, the
// same constants (00CE3820, 00CE3AE8, 00CE65D0, 00CE7804, 00D7A308, 00CEB4B8,
// 00CF8850, 00CE3830, 00CE3828), the same latch at state+8h, the close arm
// through 009DE050 / 009DA610 and the run arm through 009DFF40 plus
// brain+0AF0h = 1.0f (00D7A24C). So that arm reuses
// ship_ai_attackmove_engage_step_009e23b0.
//
// The null-target arm differs (009E2329..009E239C). It is 009DFF40 inlined with
// the unit's own heading (unit->vtable[50h]()), then brain+0AF0h = 1.0f. Unlike
// 009E23B0's 009E00A0 it leaves blk+1C8h and blk+1D0h (the throttle hold and the
// desired throttle) alone.
//
// The state's enter, vtable +4h = 009DB320 (body 009DB320-009DB343), is the
// engage member's enter: blk+234h = blk+29Ch = 1225.0f (00D216E8) and
// state+8h = 0. Its exit, vtable +8h = 007B3DC0, is a bare RET.

#include "bsp/ship_ai_attackmove_substates.hpp"

namespace bsp {

struct ShipAiKamikazeAttackHost : ShipAiAttackMoveEngageHost {
    // 009E2334, unit->vtable[50h](): the unit's current heading, null arm only.
    virtual float unit_heading_vtable_0050() = 0;
};

// Returns the latch (state+8h) as the step leaves it.
bool ship_ai_kamikaze_attack_step_009e2020(ShipAiAttackMoveEngageState& state,
                                           ShipAiKamikazeAttackHost& host);

} // namespace bsp
