#pragma once

#include <cstdint>

namespace bsp {

// The pilot bot's plan slots, and the pipeline that turns them into the plane's
// live control block. Recovered in docs/PILOT_PLAN_SLOT_PIPELINE.md and
// docs/PILOT_BOT_TICK_GATES.md, which carry the address-by-address evidence;
// docs/PLANE_BOT_CONTROL_WRITEBACK.md has the sixteen-link chain these sit in.
//
// The order of the five axes is the trap in this area and it is worth stating
// once, in full, because three different orders are in play:
//
//   live block   unit+9E4h yaw, +9E8h pitch, +9ECh roll, +9F0h throttle,
//                +9F4h air brake              (plane_flight.hpp is authoritative)
//   plan slots   0 throttle, 1 yaw, 2 roll, 3 pitch, 4 air brake
//                                              (0099B450's seeding order)
//   command buf  0 yaw, 1 pitch, 2 roll, 3 throttle, 4 air brake
//                                              (0099BC00's stores, 007B8C90's copy)
//
// None of the three agrees with either of the others. plane_advance_pose.hpp
// carried the live block in the wrong order until packet
// cc7_plane_control_targets caught it.

// plan+274h onward, stride 0Ch. `active` is written as a byte by both 0099B450
// (clearing) and the planner (0099D362 sets it). The slew path never reads it -
// it is the planner's own "this axis has a demand this tick" flag.
struct PilotPlanSlot {
    float current = 0.0f;
    float desired = 0.0f;
    std::uint8_t active = 0;
    std::uint8_t pad[3] = {0, 0, 0};
};

// The five-float live control block, in the plan's slot order rather than the
// unit's. Index with PilotPlanAxis.
enum PilotPlanAxis {
    kPilotSlotThrottle = 0,   // unit+9F0h
    kPilotSlotYaw = 1,        // unit+9E4h
    kPilotSlotRoll = 2,       // unit+9ECh
    kPilotSlotPitch = 3,      // unit+9E8h
    kPilotSlotAirBrake = 4,   // unit+9F4h
};

// The command buffer's own order, which is not the slot order.
enum PilotCommandIndex {
    kPilotCmdYaw = 0,
    kPilotCmdPitch = 1,
    kPilotCmdRoll = 2,
    kPilotCmdThrottle = 3,
    kPilotCmdAirBrake = 4,
};

// 0099B450 BSP_PilotBot_SeedPlanSlots, `void __thiscall(plan)`, RET 0, body
// 0099B450-0099B588, straight-line with no calls and no branches.
//
// It writes `desired` AND `current` from the same live value and clears
// `active`, on all five slots. Two things follow that a reimplementation gets
// wrong otherwise:
//
// * It runs BEFORE the planner, not after (009998D2 CALL 0099B450 precedes
//   00999907 CALL 0099D300 on both branches). Run last, it would overwrite every
//   `desired` the planner just produced and every axis would slew by zero - an
//   aircraft that holds whatever stick it already had. docs/PLANE_AI_CONTROL.md
//   published the inverted order.
// * Seeding `desired` as well as `current` is what makes an untouched axis a
//   no-op rather than a snap to zero: the planner writes `desired` only where it
//   has a demand, so elsewhere `desired == current`, the delta is zero, and the
//   slew returns the live value unchanged.
//
// `live` is indexed by PilotPlanAxis, so the caller does the unit-to-slot
// reordering once, here.
void pilot_seed_plan_slots_0099b450(PilotPlanSlot slots[5], const float live[5]) noexcept;

// 0099BB40, `float __thiscall(slot, float a, float b)`, RET 8, body
// 0099BB40-0099BBF6. Reads `current` and `desired` only; writes nothing back and
// never touches `active`. `a` and `b` are used only as the product, so the
// caller's "rate" and "dt" naming comes from the call site, not from the body.
//
//   delta = desired - current
//   step  = a * b
//   if (step > |delta|)  return desired;          // snap, 0099BB89 FCOMI / JBE
//   return current + sign(delta) * step;          // sign FILD-ed as an integer
//
// The comparison is strict: `step == |delta|` takes the step path, which is
// numerically the same answer.
float pilot_slew_axis_0099bb40(const PilotPlanSlot& slot, float a, float b) noexcept;

// 0099BC00, `void __thiscall(slots, cmdBuf, float a, float b)`, RET 0Ch, fully
// unrolled. Slews all five and clamps - and the clamps are NOT symmetric:
// throttle and air brake to [0,1], yaw, roll and pitch to [-1,+1], two different
// code shapes in the same unrolled body (FLDZ at 0099BC2C/0099BD6A against
// FLD [00D7A260] at 0099BC72/0099BCC4/0099BD17).
//
// `cmd` is written in the command buffer's order, not the slot order.
void pilot_evaluate_plan_slots_0099bc00(const PilotPlanSlot slots[5], float cmd[5],
                                        float rate, float dt) noexcept;

// 007BB6E0 BSP_Plane_QuantizeControlAxes, one axis of it. The command float
// becomes an 8-bit code and comes back as a float, so the live control block
// only ever holds one of 254 values.
//
//   n = (int)(c * 127.0 + 128.5)        // 00BF7420 _ftol2_sse, truncate to zero
//   n >= 255 -> +1.0f                   // 00CFD408 = 127.0, 00D05998 = 128.5
//   n <=   1 -> -1.0f
//   else     -> (n - 128) / 127.0
//
// The two clamps are continuous with the formula rather than bolted on:
// (1-128)/127 and (255-128)/127 are exactly -1 and +1. For c in [-1,1] the
// argument is in [1.5, 255.5], always positive, so the truncation is a floor and
// the whole thing is round-half-up(c * 127) / 127.
float pilot_quantize_control_axis_007bb6e0(float command) noexcept;

// 0099BF30 BSP_PilotBot_RepairCommandBands runs between the slew and the command
// block and is the last writer of all five command floats, so it is deliberately
// NOT reconstructed here - a host that skips it is passing the slew result
// through, which is an assumption about the plan's state rather than a
// simplification of the code.
//
// The assumption is a good one and docs/PILOT_COMMAND_BAND_REPAIR.md says why:
// the three control-band tables are constructed EMPTY and the throttle ceiling
// at plan+258h is constructed 1.0f, and the function returns without writing on
// both. On a freshly constructed plan it writes nothing at all. But 0099B450
// does not reset either field - its stores start at plan+26Ch - so they are
// sticky, and no writer of either was found by a scan that cannot see
// register-held addresses, SIB or block copies. Treat the pass-through as
// well-founded, not as proven.

// The rate constant 0099ACD0 passes to 0099BEE0: 00CE3D34, .rdata.
inline constexpr float kPilotSlewRate = 4.0f;
// 00D1F39C, the think interval 0099ACD0 gates on. The planner runs at about
// 11 Hz, and the dt that reaches the slew is this ACCUMULATED interval rather
// than the frame delta - 0099AD16 overwrites the incoming argument slot with the
// accumulator and all three later reads see it. A host that passes per-frame dt
// slews the stick roughly five times too slowly.
inline constexpr float kPilotThinkInterval = 0.09f;

}  // namespace bsp
