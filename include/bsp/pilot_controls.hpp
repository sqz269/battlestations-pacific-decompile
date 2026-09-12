#pragma once
#include <cstdint>

#include "bsp/plane_flight.hpp"  // PlaneControlInput, PlaneControlLatch, plane_control_off
#include "bsp/unit_orders.hpp"   // UnitPlanSlot, seed_unit_plan_slot_0099b450

// Projection of the producers of the plane's control axes: the pilot command block at
// unit+9FCh, the quantiser 007BB6E0 that turns it into the pilot control block unit+9E4h,
// the per-step commit 007BB920, the clamp and override in 007CAF10, the local pilot's
// command producer 00519520, the bot's plan-slot seeders 0099B450 / 0099B590 / 0099B2A0,
// and the unit+900h flight-state setter 007C1430.
//
// docs/PILOT_CONTROLS.md carries the evidence, the coverage table and the corrections;
// docs/PLANE_FLIGHT.md is the contract for the latch 007B9770 and the rate law 007DA710.
// Every name here is a hypothesis, not a recovered symbol, and nothing in this header is a
// binary-compatible layout: the offset constants are the native ones, the structs are not.
//
// Contracts named but not reconstructed: the call site of 00519520 and of 0099BEE0 (both
// unestablished - no xref and no vtable entry); the throttle chain 007C47F0 / 007C4810 that
// feeds the third command; 0077C470, 004C5070, 008A5BD0, 007C11E0, 0090F6C0 and 0099BB40;
// whether 0099BEE0's out record is the 0x84-byte block 007C2810 allocates, which is what
// would close the bot's path into unit+9E4h.

namespace bsp {

// ---------------------------------------------------------------------------
// The pilot command block, unit+9FCh. Six dwords and a pending byte, written whole by
// 007B8C90 and zero-initialised by the plane unit constructor at 007CFEB8-007CFED6.
// ---------------------------------------------------------------------------
namespace pilot_command_off {
inline constexpr int kPilotCommandRoll = 0x9FC;      // 007B8C96; becomes unit+9E4h
inline constexpr int kPilotCommandPitch = 0xA00;     // 007B8C9F; becomes unit+9E8h
inline constexpr int kPilotCommandYaw = 0xA04;       // 007B8CA8; becomes unit+9ECh
inline constexpr int kPilotCommandThrottle = 0xA08;  // 007B8CB1; becomes unit+9F0h
inline constexpr int kPilotCommandFifth = 0xA0C;     // 007B8CBA; becomes unit+9F4h
inline constexpr int kPilotCommandBoost = 0xA10;     // 007B8CC3; becomes the byte unit+9F8h
inline constexpr int kPilotCommandPending = 0xA14;   // 007B8CC9 sets, 007BB990 clears
}  // namespace pilot_command_off

// The unit fields the commit path and the override read.
namespace pilot_unit_off {
inline constexpr int kPilotSuppressByte = 0x61;   // 007BB923 skips the whole commit when set
inline constexpr int kPilotFlightState = 0x900;   // 007BB932, 007C145B writes it
inline constexpr int kPilotGroundFlag = 0x904;    // 007C1430 case 4/5
inline constexpr int kPilotGroundTimer = 0x908;   // compared against 5.0f
inline constexpr int kPilotThrottleGate = 0xC0C;  // 007B8C3C, 007B8C58
inline constexpr int kPilotBoostLocked = 0xC24;   // 007BB969
inline constexpr int kPilotStateStamp = 0xC04;    // 007C1430 stamps -1.0f
inline constexpr int kPilotBoostCharge = 0xE84;   // 007BB7BE, 007BB7F2
inline constexpr int kPilotBoostLockout = 0xE88;  // 007BB7FF, cleared at 007BB834
inline constexpr int kPilotEffectiveYaw = 0xBE8;         // 007CB24F
inline constexpr int kPilotEffectivePitch = 0xBEC;       // 007CB2A9
inline constexpr int kPilotEffectiveThrottle = 0xBF0;    // 007CB2E1
inline constexpr int kPilotAuthorityLossYaw = 0xBCC;     // subtracted at 007CB24F
inline constexpr int kPilotAuthorityLossPitch = 0xBD0;   // subtracted at 007CB2A9
inline constexpr int kPilotAuthorityLossThrottle = 0xBD4; // subtracted at 007CB2E1
}  // namespace pilot_unit_off

// The bot fields 0099B450 seeds. UnitPlanSlot (include/bsp/unit_orders.hpp) is the triple.
namespace pilot_bot_off {
inline constexpr int kPilotPlanSlots = 0x274;       // slot 0; five triples of stride 0Ch
inline constexpr int kPilotPlanSlotStride = 0x0C;   // 0099B456 vs 0099B476
inline constexpr int kPilotPlanUnit = 0x2F0;        // 0099B450 MOV EAX,[ECX+2F0h]
inline constexpr int kPilotPlanDescriptor = 0x2F4;  // 0099B4F1
inline constexpr int kPilotPlanDemand = 0x2B4;      // 0099B511, from descriptor+190h
inline constexpr int kPilotPlanModeYaw = 0x2CC;     // provisional, from 0099B5F4
inline constexpr int kPilotPlanModePitch = 0x2D0;   // provisional, from 0099B5D6
inline constexpr int kPilotPlanModeRoll = 0x2D4;    // provisional, from 0099B5B8
inline constexpr int kPilotPlanModeThrottle = 0x2D8; // provisional, from 0099B622
}  // namespace pilot_bot_off

// ---------------------------------------------------------------------------
// Constants, all read out of the image.
// ---------------------------------------------------------------------------
inline constexpr float kPilotQuantizeScale = 127.0f;  // 00CFD408, a double in the image
inline constexpr float kPilotQuantizeBias = 128.5f;   // 00D05998, a double in the image
inline constexpr int kPilotQuantizeCeiling = 0xFF;    // 007BB703 CMP EAX,0FFh
inline constexpr int kPilotQuantizeFloor = 1;         // 007BB714 CMP EAX,1
inline constexpr int kPilotQuantizeCentre = 0x80;     // 007BB71D ADD EAX,-80h

inline constexpr float kPilotAxisLow = -1.0f;      // 00D7A260, the bipolar lower bound
inline constexpr float kPilotAxisHigh = 1.0f;      // 00D7A24C
inline constexpr float kPilotUnipolarLow = 0.0f;   // an immediate zero at 007CB1C7
inline constexpr float kPilotPitchHalfRange = 0.5236f;  // 00CEC724, 30 degrees
inline constexpr float kPilotPitchInputScale = 2.0f;    // 00CEC730, a double in the image
inline constexpr float kPilotGroundSpeedGate = 1.3888889f;  // 00CF8AAC, 5 km/h in m/s
inline constexpr float kPilotGroundTimerGate = 5.0f;    // 00CE3850
inline constexpr float kPilotStateSevenTimer = 3600.0f; // 00CFDEB0

// Override values the 007CAF10 reset writes into the axes and their latches.
inline constexpr float kPilotOverrideRollHeld = 0.2f;      // 00CE54A0
inline constexpr float kPilotOverrideRollLeft = -0.8f;     // 00D05E14
inline constexpr float kPilotOverrideRollRight = 0.8f;     // 00CE74F8
inline constexpr float kPilotOverrideThrottleHeld = 0.01f; // 00D7A238
inline constexpr float kPilotOverrideThrottleIdle = 0.3f;  // 00CE69C8

// unit+900h. Only 2 and 3 are named by a side effect; the rest keep their number.
inline constexpr int kPilotStateOne = 1;             // 007CC857
inline constexpr int kPilotStateThrottleZeroed = 2;  // 007CC7E2; 007C143C zeroes unit+9F0h
inline constexpr int kPilotStateThrottleFull = 3;    // 007C153A writes 1.0f
inline constexpr int kPilotStateGroundA = 4;         // 007C1697, 007C7488
inline constexpr int kPilotStateGroundB = 5;         // 007C171E
inline constexpr int kPilotStateSix = 6;             // 007CBA12, 007C63F4
inline constexpr int kPilotStateSeven = 7;           // 007C7183, 007C6481, 007D6600

// 007BB938-007BB94A: the four states in which 007BB920 does NOT force the fifth command.
bool pilot_state_holds_fifth_command_007bb920(int state) noexcept;

// ---------------------------------------------------------------------------
// The pilot command block and the quantiser.
// ---------------------------------------------------------------------------
struct PilotCommandBlock {
    float roll{0.0f};      // +9FCh
    float pitch{0.0f};     // +A00h
    float yaw{0.0f};       // +A04h
    float throttle{0.0f};  // +A08h
    float fifth{0.0f};     // +A0Ch
    bool boost_request{false};  // +A10h, tested as a byte at 007BB7B9 and 007BB7ED
    bool pending{false};        // +A14h
};

// unit+E84h and unit+E88h, the boost charge and its lockout byte.
struct PilotBoostState {
    float charge{0.0f};
    bool lockout{false};
};

// 007BB6E6-007BB72A and its two repetitions: q = ftol(command * 127.0 + 128.5), then
// q >= 0FFh -> 1.0f, q <= 1 -> -1.0f, otherwise (q - 128) / 127.0. MSVC's ftol truncates
// toward zero, so the rule is not a rounding; the bias 128.5 supplies the half.
float quantize_pilot_axis_007bb6e0(float command) noexcept;

// The throttle arm of 007BB6E0, 007BB79E-007BB82E. Two paths, chosen by the capability query
// and the lockout byte, and the boost byte unit+9F8h latches on the first of them.
struct PilotThrottleArm {
    bool boost_byte{false};   // what 007BB6E0 writes to unit+9F8h
    bool analogue{false};     // true: quantise the command instead of using `throttle`
    float throttle{0.0f};     // the binary value, when `analogue` is false
    bool clear_lockout{false}; // 007BB834 zeroes unit+E88h on the analogue arm only
};

// capability_17h is unit->vtable[+5Ch](17h) and boost_locked the byte unit+C24h. On the
// first path (capability and not locked) the byte persists while there is charge left and
// the throttle is binary. On the second path a request forces the throttle to 1.0f whether
// or not the byte takes; no request drops through to the analogue quantisation.
PilotThrottleArm resolve_throttle_arm_007bb6e0(bool boost_request, const PilotBoostState& boost,
                                               bool capability_17h, bool boost_locked,
                                               bool previous_boost_byte) noexcept;

// 007BB6E0 whole, __thiscall(unit, const float* cmd), RET 4. The three attitude axes always
// take the round trip. The throttle takes it only on the analogue arm; on the boost arm it is
// binary, 1.0f or 0.0f from the boost byte. The fifth axis always takes the round trip.
// The three byte members of the result are left at the caller's values: 007BB6E0 writes only
// unit+9F8h, out of the boost logic.
PlaneControlInput quantize_pilot_command_007bb6e0(const PilotCommandBlock& command,
                                                  const PlaneControlInput& previous,
                                                  const PilotBoostState& boost,
                                                  bool capability_17h,
                                                  bool boost_locked) noexcept;

// ---------------------------------------------------------------------------
// 007BB920, the per-step commit. One method per native call site.
// ---------------------------------------------------------------------------
class PilotCommandCommitHost {
public:
    virtual ~PilotCommandCommitHost() = default;
    virtual bool suppressed() = 0;                  // 007BB923, byte unit+61h
    virtual bool command_pending() = 0;             // 007BB929, byte unit+A14h
    virtual int flight_state() = 0;                 // 007BB932, unit+900h
    virtual void set_fifth_command(float value) = 0;     // 007BB954, unit+A0Ch
    virtual bool unit_capability_17h() = 0;          // 007BB963, vtable[+5Ch](17h)
    virtual bool boost_locked() = 0;                 // 007BB969, byte unit+C24h
    virtual void set_throttle_command(float value) = 0;  // 007BB97A, unit+A08h
    virtual void quantize_commands_into_axes() = 0;  // 007BB98B, 007BB6E0(unit, unit+9FCh)
    virtual void clear_command_pending() = 0;        // 007BB990, byte unit+A14h
};

// Returns true when the commit ran, false when either gate refused it.
bool commit_pilot_command_007bb920(PilotCommandCommitHost& host);

// ---------------------------------------------------------------------------
// 007CAF10, the clamp and the effective-control trio.
// ---------------------------------------------------------------------------
// 007CB143-007CB1EB: the three attitude axes to [-1, 1], the two unipolar axes to [0, 1].
// The native form is `if (low <= v) { if (high < v) v = high; } else v = low;`, so an
// unordered compare yields the low bound; the projection keeps that shape.
PlaneControlInput clamp_control_axes_007caf10(PlaneControlInput axes) noexcept;

// 007CB1F9-007CB2E1: each latched axis minus its authority loss, clamped to [0, 1]. The two
// attitude terms take the absolute value first (the native form negates against -0.0f,
// 00D7A208); the throttle term does not.
struct PilotEffectiveControl {
    float yaw{0.0f};       // +BE8h
    float pitch{0.0f};     // +BECh
    float throttle{0.0f};  // +BF0h
};
PilotEffectiveControl effective_control_007caf10(const PlaneControlLatch& latched,
                                                 float loss_yaw, float loss_pitch,
                                                 float loss_throttle) noexcept;

// ---------------------------------------------------------------------------
// 00519520, the local pilot's command producer. Both attitude commands are aim-tracking
// errors, not stick deflections, and the roll slot is never assigned.
// ---------------------------------------------------------------------------
// The horizontal rule: err < -tolerance -> 1.0f, err > tolerance -> -1.0f, else
// -err / tolerance. The sign is the native one; tolerance is classDesc+274h.
float player_turn_command_00519520(float error, float tolerance) noexcept;

// The vertical rule: outside [-30deg, 30deg] the command saturates to -1.0f / 1.0f, inside
// it is clamp(err / 2.0f, -1, 1). The image's tangent term is multiplied by 00D7A258 = 0.0,
// so it contributes nothing and is not modelled.
float player_pitch_command_00519520(float error) noexcept;

// ---------------------------------------------------------------------------
// The bot's plan slots. UnitPlanSlot is the triple; these are the array-level seeders.
// ---------------------------------------------------------------------------
struct PilotPlanSlots {
    UnitPlanSlot throttle;  // +274h, from unit+9F0h
    UnitPlanSlot roll;      // +280h, from unit+9E4h
    UnitPlanSlot yaw;       // +28Ch, from unit+9ECh
    UnitPlanSlot pitch;     // +298h, from unit+9E8h
    UnitPlanSlot fifth;     // +2A4h, from unit+9F4h
    float demand{0.0f};     // +2B4h, from descriptor+190h
    float constant_2c8{0.0f};  // +2C8h = 20.0f  (00CE3930)
    float constant_2e8{0.0f};  // +2E8h = 1.0f   (00D7A24C)
    float constant_2ec{0.0f};  // +2ECh = 0.24f  (00E0E2EC)
    int mode_yaw{0};        // +2CCh
    int mode_pitch{0};      // +2D0h
    int mode_roll{0};       // +2D4h
    int mode_throttle{0};   // +2D8h
};

// 0099B450, __thiscall(bot), RET 0: both halves of every slot take the live axis, every
// active byte is cleared, the four mode words take 1 / 2 / 1 / 1 and the three constants and
// the demand are written. Reads the unit, writes nothing back to it.
void seed_plan_slots_0099b450(PilotPlanSlots& slots, const PlaneControlInput& live,
                              float descriptor_190h) noexcept;

// 0099B590 (no Ghidra function, 0099B590-0099B629): 0099B450, then the desired halves again
// from the live axes with every active byte set and every mode word zeroed.
void seed_plan_slots_held_0099b590(PilotPlanSlots& slots, const PlaneControlInput& live,
                                   float descriptor_190h) noexcept;

// ---------------------------------------------------------------------------
// 007C1430, the flight-state setter, __thiscall(unit, int state).
// ---------------------------------------------------------------------------
struct PilotFlightStateChange {
    bool changed{false};          // false when the state already matched: 007C144C returns 0
    bool throttle_written{false}; // the state 2, 3 arms
    float throttle{0.0f};
    bool stamp_written{false};    // unit+C04h = -1.0f
    bool notify{false};           // 007C11E0(0) runs on every change
};

// The pre-guard write at 007C143C is part of the contract: a request for state 2 zeroes the
// throttle even when the state already is 2.
PilotFlightStateChange set_flight_state_007c1430(int current_state, int requested) noexcept;

}  // namespace bsp
