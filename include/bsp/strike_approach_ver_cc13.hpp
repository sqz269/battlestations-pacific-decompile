#ifndef BSP_STRIKE_APPROACH_VER_CC13_HPP
#define BSP_STRIKE_APPROACH_VER_CC13_HPP

// Packet cc13-ver9: the run-in altitude cap the host's strike approach lacks,
// read from the bytes for the strike-approach verification (docs/CC13_VER_9.md).
//
// Names are hypotheses, not recovered symbols. These are pure models: nothing in
// the host calls them. The seven callers that need them are in
// src/game_hosts_units.cpp (lua56's lease), so wiring them is an adoption note.
//
// 009FBA50 BSP_PilotBot_CommandCruiseAltitude, __thiscall(ECX = state+4), RET 10h.
//   009FBA7D  CALL 0042E740                   tuning
//   009FBA82  FLD  [EAX+210h]                 Dynamics/Ceiling
//   009FBA8A  FSUB qword [00CE3938]           - 50
//   009FBA88  MOV  ECX,[ESI]                  ECX = [state+4], the task base P
//   009FBA90  MOV  EAX,[ECX+0Ch]              P+0Ch, set by 009F9CE0 from unit+9D4h
//   009FBA93  TEST EAX,EAX / 009FBA99 JE      no block: the ceiling alone
//   009FBA9B  FLD  [EAX+394h]                 ctl+394h
//   009FBAAD  FCOMI ST0,ST1 (ctl, limit) / 009FBAB1 JA   keep the limit when ctl is higher
//   009FBAB3  MOVSS limit <- ctl+394h        so limit = min(Ceiling - 50, ctl+394h)
// Every caller passes ECX = state+4: 009C43D9 (dive attackrun, ESI = EDI+4 at
// 009C423D), 009D0A6F (torpedo attackrun, ESI = EDI+4 at 009D07CD), 009C1AFF
// (moveto, ESI = EDI+4 at 009C19A8). The torpedo goaway's 009D0E7C reads the
// same [state+4]+0Ch block for its +394h, which the host already models as the
// task's CruisingAlt, so this is the same field and not "a different object's
// +394h" (game_hosts_units.cpp:19412-19414 says otherwise; the bytes do not).
//
// The block's +394h, by writer:
//   007F2BE8   block init: tuning+53Ch, Pilot/General/CruisingAlt (1000 here)
//   009C8920   dive profile step 4, the leader only: tuning+4C0h,
//              Pilot/DiveBomb/CruisingAlt (1300 in planeglobals.lua:578)
//   009D4A70   torpedo profile step 4: tuning+430h, Pilot/Torpedo/CruisingAlt
//              (500 in planeglobals.lua:611)
//   0089F550   the Lua squadron cruise block (the host's sq_alt_394/sq_alt_set)
// SCOPE (packet cc13-ver11, docs/CC13_VER_11.md): the block is the SQUADRON.
// 007B97E0 BSP_Plane_GetSquadron is `MOV EAX,[ECX+9D4h] / RET`, and 007F2BD0
// initialises it from the squadron constructor 007F2C60 (ECX = squadron+37Ch).
// Both profile writers are leader-only (009C8931 / 009D4A82 CALL 007B8AD0, then
// JE past the write) and write into that shared block, so every member reads the
// flight leader's value: pass the LEADER's task kind for a wingman. A wingman
// reads the init value only before the leader's first profile tick.
// UNCERTAINTY: which write is live on a given think depends on the profile
// cadence (docs/CC13_TASK_PROFILE_CADENCE.md) and the +380h/+38Dh/+3A9h gates.
// The model takes the script value when one is set, else the task's profile
// value, else the init value.

#include "bsp/plane_flight.hpp"

namespace bsp {

// OFF: nothing in the host reads it yet. Flip only through a cc9 pair.
inline constexpr bool kStrikeCtlCruiseCapBound = false;

enum class StrikeCruiseTask {
    kNone,       // no task profile has written +394h: the 007F2BE8 init value
    kDiveBomb,   // 009C8920, tuning+4C0h
    kTorpedo,    // 009D4A70, tuning+430h
};

struct StrikeCtlCruiseInputs {
    bool has_control_block_0c = true;  // P+0Ch non-null; 009FBA99 JE otherwise
    bool script_alt_set = false;       // 0089F550 wrote +394h (host sq_alt_set)
    float script_alt_394 = 0.0f;       // host sq_alt_394
    StrikeCruiseTask task = StrikeCruiseTask::kNone;
    float general_cruising_alt_53c = 1000.0f;   // Pilot/General/CruisingAlt
    float dive_cruising_alt_4c0 = 1300.0f;      // Pilot/DiveBomb/CruisingAlt
    float torpedo_cruising_alt_430 = 500.0f;    // Pilot/Torpedo/CruisingAlt
};

// The value ctl+394h holds when 009FBA9B reads it.
float strike_ctl_cruising_alt_394(const StrikeCtlCruiseInputs& in) noexcept;

// 009FBA50 with its ctl+394h leg fed: sets has_squadron/squadron_limit on a
// copy of `cin` and runs cruise_altitude_command_009fba50.
PlaneCruiseAltitudeResult strike_cruise_altitude_with_ctl_cap_009fba9b(
    PlaneCruiseAltitudeInputs cin, const StrikeCtlCruiseInputs& ctl);

}  // namespace bsp

#endif  // BSP_STRIKE_APPROACH_VER_CC13_HPP
