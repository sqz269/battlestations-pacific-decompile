#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/ship_ai_navigation.hpp"   // UnitAiOrderRecord, UnitAiOrderSubRecord
#include "bsp/ship_ai_states.hpp"       // ShipAiControlBlock, ShipAiThrottleDirection
#include "bsp/unit_state_message.hpp"   // UnitOrderRing, UnitOrderRingSlot

// The hop that was missing from the ship AI chain: how the AI controller's own
// desired throttle at blk+1D0h and desired rudder at blk+1D4h reach the order
// ring at unit+838h, which is what the ship motion reads.
//
// The answer is the tail of 009F3F80, the routine the controller reaches
// through 009F4DA0 (docs/SHIP_AI_STATES.md chain slot 16). 009F3F80's `this` is
// `blk`, its unit is blk+3FCh. It reads the ring slot under the WRITE cursor at
// its head (009F3FF8..009F402E), rewrites blk+1D0h and blk+1D4h through the
// whole body, then slews the two desired values toward that slot by at most
// dt * 1.5 and stores them back into the same slot with 0080E170 and 0080E190
// (009F4CE8, 009F4CFB). Nothing on this path reads the published order slot at
// unit+0A98h; that slot is for other units, as docs/UNIT_AI_ORDER_SLOT_READER.md
// established.
//
// Every offset and constant below comes from the listing of the address named
// on its line; see docs/SHIP_AI_THROTTLE_TO_RING.md and
// docs/SHIP_AI_ORDER_CONSUMER.md for the ABIs, the evidence and the
// uncertainties. Descriptive names are hypotheses, not recovered symbols. These
// are semantic interfaces for MSVC Win32, not drop-in binary replacements.

namespace bsp {

// ---------------------------------------------------------------------------
// Two library helpers this packet needed and no header declared yet.
// ---------------------------------------------------------------------------

// 00415620, float __fastcall(float* value, const float* low, const float* high),
// RET 4 at 00415654 and 0041565C, body 00415620-0041565E, complete. It reads
// all three operands through pointers:
//   if (*low > *value) return *low;
//   if (*value <= *high) return *value;
//   return *high;
// The first compare is FCOMI/JA and the second FCOMI/JBE, so a NaN value falls
// through both and the routine returns *high.
float clamp_float_by_ref_00415620(float value, float low, float high) noexcept;

// 006BC0C0, float* __thiscall(float* out)(float heading), RET 4 at 006BC10F,
// body 006BC0C0-006BC111, complete. The heading-to-direction conversion:
//   a = 1.5707963705062866 - heading;  if (a < 0) a += 6.2831854820251465;
//   out[0] = cosf(a); out[1] = sinf(a);
// so heading 0 points along +Z and a rising heading turns toward +X.
inline constexpr double kHeadingBasisQuarterTurn = 1.5707963705062866; // 00CE3830
inline constexpr double kHeadingBasisFullTurn = 6.2831854820251465;    // 00CE3828
std::array<float, 2> heading_to_direction_006bc0c0(float heading) noexcept;

// ---------------------------------------------------------------------------
// 0080E170 and 0080E190: the ring's write-slot setters
// ---------------------------------------------------------------------------
// Both are void __thiscall(unit)(float), RET 4, five instructions each, no
// branch, complete:
//   0080E170: EAX = [unit+97Ch]; [unit + (EAX<<5) + 838h] = arg   (throttle)
//   0080E190: EAX = [unit+97Ch]; [unit + (EAX<<5) + 83Ch] = arg   (rudder)
// unit+838h is the ring base and unit+97Ch is the ring's write cursor
// (ring+144h, bsp/unit_state_message.hpp), so each writes param_a / param_b of
// the slot the ring is currently filling. They touch no bound, no cursor and
// neither live field, which is what separates them from 0080D9B0 / 0080DA00.
void ship_ai_ring_set_write_slot_throttle_0080e170(UnitOrderRing& ring, float value) noexcept;
void ship_ai_ring_set_write_slot_rudder_0080e190(UnitOrderRing& ring, float value) noexcept;

// The same slot read back, which is what 009F3FFE..009F402E does at the head of
// 009F3F80 (two independent loads of [unit+97Ch], both shifted left by 5).
float ship_ai_ring_write_slot_throttle(const UnitOrderRing& ring) noexcept;
float ship_ai_ring_write_slot_rudder(const UnitOrderRing& ring) noexcept;

// ---------------------------------------------------------------------------
// 009DA250: the rudder law
// ---------------------------------------------------------------------------
// float10 __thiscall(blk)(float heading_error), RET 4 at 009DA3A2, body
// 009DA250-009DA3A4, complete. This is the only writer of blk+1D4h inside
// 009F3F80 (009F44FC and 009F46A0 both store its result), and it is the rule
// that turns a heading error into a rudder demand.

struct ShipAiRudderLawHost {
    virtual ~ShipAiRudderLawHost() = default;
    // 009DA2D7, 0092D730 with ECX = [unit+1018h] set at 009DA2D1: the hull's
    // signed forward speed. One call site; the reconstruction calls it once.
    virtual float unit_body_axis_speed_0092d730() = 0;
};

// 009DA276, FMUL by 00CEC160: the ship-class yaw authority at
// [[blk+3FCh]+538h]+524h is scaled by this before it divides the error.
// `class+524h` is not a Lua key. It is derived once per class by 00828F20,
// the descriptor's virtual slot +14h, out of `MaxRotAngle` (`class+4F8h`) and
// `MaxRotAngleChangeRatio` (`class+4FCh`); see
// ship_class_ai_derived_motion_00828f20 below and
// docs/SHIP_AI_CLASS_FIELD_0524.md.
inline constexpr double kShipAiRudderAuthorityScale = 1.2; // 00CEC160
// 009DA2FA and 009DA31F, 00CE65D0: below this speed the rudder is zero, and
// above it the demand ramps in over one unit of speed.
inline constexpr double kShipAiRudderSpeedFloor = 0.4; // 00CE65D0
// 009DA28C and 009DA2A8, the clamp on the raw demand.
inline constexpr float kShipAiRudderLow = -1.0f;  // 00D7A260
inline constexpr float kShipAiRudderHigh = 1.0f;  // 00D7A24C

float ship_ai_rudder_from_heading_error_009da250(ShipAiThrottleDirection latched_direction,
                                                 float heading_error,
                                                 float ship_class_yaw_authority_0524,
                                                 ShipAiRudderLawHost& host);

// ---------------------------------------------------------------------------
// 00828F20: where class+520h and class+524h come from
// ---------------------------------------------------------------------------
// bool __thiscall(descriptor)(void), RET at 00828F79, body 00828F20-00828F79,
// complete. It is virtual slot +14h of the ship-class descriptor vtable at
// 00D1ACC4 (00963380 installs that vtable at 009633C0; 00828F20 is the value at
// 00D1ACD8 = 00D1ACC4 + 14h) and it is reached once per class from 0096515D in
// BSP_VehicleClass_GetOrCreate, after the Lua load pass on slot +10h and before
// the query on slot +18h. 00758140 and 00852200 are one-instruction
// `JMP 00828F20` thunks that other descriptor vtables point at, so this body is
// the only implementation. Its return value is ignored at 0096515D.
//
// Neither field is a Lua key: 00831840 (docs/SHIP_CLASS_FIELDS.md) writes
// +4F8h..+51Ch and then +538h onward, and the byte scan for every store form at
// displacement 520h and 524h finds 00828F5A and 00828F66 as the only writers of
// a descriptor. So both are derived, once, from three keys the ship rows do
// author:
//   class+520h = MaxSpeed (+500h) / MaxRotAngle (+4F8h)          -- metres
//   class+524h = 0.5 * MaxRotAngle (+4F8h) / MaxRotAngleChangeRatio (+4FCh)
// 00828F54 folds the 0.5 in as the double at 00D7A280. +520h is the hull's
// turn radius at full speed; 0082E850 is its only reader and multiplies it by
// the tuning float at singleton+438h when a virtual query says so. +524h has
// exactly one reader in the image, 009DA268, the rudder law above.
// MaxRotAngleChangeRatio has no other reader anywhere: 00828F23 is its only
// load, so the whole effect of that key on the game is this one product.
struct ShipClassAiDerivedMotion {
    float turn_radius_0520{0.0f};   // 00828F66
    float yaw_authority_0524{0.0f}; // 00828F5A
    // AL at 00828F79. False leaves both fields at whatever the constructor
    // left; 00963380 does not clear either of them.
    bool derived{false};
};

// 00828F54, FMUL by the double at 00D7A280.
inline constexpr double kShipClassYawAuthorityHalf = 0.5; // 00D7A280

ShipClassAiDerivedMotion ship_class_ai_derived_motion_00828f20(
    float max_rot_angle_04f8, float max_rot_angle_change_ratio_04fc, float max_speed_0500);

// ---------------------------------------------------------------------------
// 009F4B99..009F4D04: the hop itself
// ---------------------------------------------------------------------------
// The tail of 009F3F80, reconstructed operation for operation. Everything
// before 009F4B99 decides blk+1D0h and blk+1D4h; this is the part that moves
// them into the ring.

struct ShipAiRingHopHost {
    virtual ~ShipAiRingHopHost() = default;
    // 009F4BD4, 0092D730 with ECX = [[blk+3FCh]+1018h]: the hull's signed
    // forward speed, read only when the throttle is already inside the
    // deadband. Called at most once.
    virtual float unit_body_axis_speed_0092d730() = 0;
    // 009F4CE8, 0080E190 with ECX = [blk+3FCh]: store the slewed rudder into
    // the ring's write slot. Called before the throttle setter, always.
    virtual void set_ring_write_slot_rudder_0080e190(float value) = 0;
    // 009F4CFB, 0080E170 with ECX = [blk+3FCh]: store the slewed throttle.
    virtual void set_ring_write_slot_throttle_0080e170(float value) = 0;
};

struct ShipAiRingHop {
    float ring_throttle{0.0f}; // what 0080E170 received
    float ring_rudder{0.0f};   // what 0080E190 received
    bool rudder_zeroed{false}; // the 009F4BC6..009F4BFC deadband fired
};

// 009F4BB9, FLD double 00D7A270: the throttle magnitude under which the ship
// counts as stopped, and 009F4BE1, the speed magnitude that goes with it.
inline constexpr double kShipAiRingHopThrottleDeadband = 0.05; // 00D7A270
inline constexpr float kShipAiRingHopSpeedDeadband = 1.0f;     // 00D7A24C
// 009F4C12, FMUL double 00CE3D78: the per-second limit on how far either ring
// value may move in one frame.
inline constexpr double kShipAiOrderSlewRate = 1.5; // 00CE3D78

// `blk` is modified: the deadband arm stores 0.0f at blk+1D4h.
// `previous_*` are the ring write slot's current values, which 009F3F80 loaded
// at its head; pass what ship_ai_ring_write_slot_* returned before the body ran.
ShipAiRingHop ship_ai_order_ring_hop_009f4b99(ShipAiControlBlock& blk,
                                              float previous_ring_throttle,
                                              float previous_ring_rudder, float dt,
                                              ShipAiRingHopHost& host);

// The one-line rule inside the hop, exposed because it is the whole slew:
//   if (dt * 1.5 <= |previous - desired|)
//       desired = (desired <= previous) ? previous - dt*1.5 : previous + dt*1.5;
// Note the comparison is against the PREVIOUS value, so a desired value that is
// already within one step is passed through untouched, sign included.
float ship_ai_slew_toward_ring_009f4c0e(float previous, float desired, float step) noexcept;

// ---------------------------------------------------------------------------
// 00815F30 and 00811D80: the two-slot lateral-offset memory on the order record
// ---------------------------------------------------------------------------
// These are a writer/reader pair over the two 1Ch-byte sub-records of
// UnitAiOrderRecord, and both address the unit's OWN record, not another
// unit's: 009EE649 passes the record the AI is writing this frame, and
// 009E3DC1 computes `unit + 0A98h + 54h * [unit+0B40h]` from `[this+3Ch]`,
// which is the same unit whose radius it reads at unit+9C8h.

// 00815F43, MOVSS from 00CE3958: the timer 00815F30 parks in record+04h.
inline constexpr float kUnitAiOrderTurnLimitTimer = 2.0f; // 00CE3958
// 00815F63 and 00811DCA, FLD double 00CE3D90: the squared distance at which a
// sub-record stops describing the position being asked about. 20 units.
inline constexpr double kUnitAiOrderTurnLimitRadiusSq = 400.0; // 00CE3D90
// 00811D83, the value 00811D80 returns when neither sub-record matches.
inline constexpr float kUnitAiOrderTurnLimitDefault = 30.0f; // 00E0E304

// 00815F30, void __thiscall(record)(const float* xz, int direction, float low,
// float high), RET 10h, body 00815F30-00816046, complete. Sets record+04h,
// shifts sub_a into sub_b when the stored position has moved more than 20
// units, then restates sub_a for the new position and clamps its held value
// into the bounds. For a direction other than 1 both bounds are negated, which
// swaps them, and the clamp runs on the swapped pair.
void unit_ai_order_push_turn_limit_00815f30(UnitAiOrderRecord& record,
                                            const std::array<float, 2>& position_xz,
                                            int direction, float low, float high) noexcept;

// 00811D80, float10 __thiscall(record)(const float* xz), RET 4, body
// 00811D80-00811E7C, RET 4 at 00811E5F and 00811E7A, complete. Returns 30.0f unless a live sub-record sits
// within 20 units of the query position, in which case it returns
// record.blend_00 clamped into that sub-record's bounds, negated when the
// sub-record's high bound is negative. sub_b wins when both match, and sub_b's
// negative arm returns immediately.
float unit_ai_order_turn_limit_at_00811d80(const UnitAiOrderRecord& record,
                                           const std::array<float, 2>& position_xz) noexcept;

// ---------------------------------------------------------------------------
// 009D8CE0: what a ship computes from ANOTHER ship's published triple
// ---------------------------------------------------------------------------
// void __thiscall(self)(void* candidate, void* other), RET 8 at 009D913F, body
// 009D8CE0-009D9141. The one routine in this packet that reads a second unit's
// published slot. It projects both units' tracks from their published headings
// (slot+44h), intersects them, and decides which of the two reaches the
// crossing first. Partial: the four min/max threshold operands at
// 009D8EF0..009D905F are not resolved, so the gate is expressed as a predicate
// the caller supplies.

// Which unit the rule expects to reach the crossing first. The names describe
// the comparison at 009D906D, not a recovered meaning: nothing this packet read
// says what self+8Ch is used for afterwards.
enum class ShipAiCrossingState : int {
    None = 0,       // 009D8D28, the initial and cleared value
    OtherFirst = 1, // 009D9073, 009D90A1, and the default at 009D90AD
    SelfFirst = 2,  // the else arm of 009D906D; 009D90BF tests self+8Ch against 2
};

struct ShipAiCrossingInput {
    std::array<float, 2> self_position{};   // self+20h, self+24h
    std::array<float, 2> other_position{};  // other+20h, other+24h
    float self_published_heading{0.0f};     // self slot+44h  (009D8D35)
    float self_published_distance{0.0f};    // self slot+40h  (009D8DAA, +0AE0h)
    float other_published_heading{0.0f};    // other slot+44h (009D8D71)
    float other_published_path_left{0.0f};  // other slot+48h (009D8DB9)
    float self_radius_09c8{0.0f};           // [self+14h]+9C8h
    float other_length_09cc{0.0f};          // [other+14h]+9CCh
};

struct ShipAiCrossingGeometry {
    bool tracks_cross{false};       // false means 009D8D9B took the early out
    float lateral_offset{0.0f};     // the other unit's offset across self's track
    float closing_lateral{0.0f};    // the rate that offset closes at
    float self_range_to_cross{0.0f};  // along self's heading
    float other_range_to_cross{0.0f}; // along the other's heading
};

// 009D8DCE..009D8F7C, complete. Pure geometry, no host.
ShipAiCrossingGeometry ship_ai_crossing_geometry_009d8dce(const ShipAiCrossingInput& in) noexcept;

// 009D8ED6, the |closing_lateral| under which the two tracks count as parallel.
inline constexpr float kShipAiCrossingParallelEpsilon = 0.001f; // 00D7A23C
// 009D8FDE, FMUL double 00D7A328: the other unit's length is scaled by this
// before it bounds the lateral offset.
inline constexpr double kShipAiCrossingLengthScale = 4.0; // 00D7A328
// 009D9044, FDIV double 00D049A8: self's radius divided by this is the margin
// added to its range before the range is tested against its published distance.
inline constexpr double kShipAiCrossingRadiusDivisor = 1.7999999523162842; // 00D049A8
// 009D903C, the floor under the reach added to the other unit's range before
// that range is tested against the path the other unit still has to run.
inline constexpr float kShipAiCrossingReachFloor = 100.0f; // 00CE3D08
// 009D90EC, FMUL double 00D7A280: the hysteresis a ship already in SelfFirst
// subtracts from its own range.
inline constexpr double kShipAiCrossingHysteresis = 0.5; // 00D7A280

// 009D8FD3..009D8FF0, the first disjunct of the gate, fully resolved: the other
// unit sits close enough to self's track for the crossing to matter at all.
// The second disjunct, 009D8FF2..009D9036, compares the two ranges against two
// nested minima built at 009D8F80..009D8FCB. Three of those four operands are
// resolved (400.0f twice, and the other unit's slot+40h); the fourth, [ESP+3Ch],
// has no writer in the stored listing, which is why the caller supplies
// `inside_gate` rather than this header asserting a value.
bool ship_ai_crossing_length_gate_009d8fd3(const ShipAiCrossingGeometry& geometry,
                                           const ShipAiCrossingInput& in) noexcept;

// 009D8F80..009D9136, partial: the two min() bounds the ranges are tested
// against are built at 009D8F80..009D8FCB from operands this packet did not
// resolve, so `inside_gate` stands for the whole 009D8FE4..009D9036 test.
// `previous` is self+8Ch on entry; the return value is what self+8Ch becomes.
ShipAiCrossingState ship_ai_crossing_decide_009d8fe4(const ShipAiCrossingGeometry& geometry,
                                                     const ShipAiCrossingInput& in,
                                                     ShipAiCrossingState previous,
                                                     bool inside_gate) noexcept;

} // namespace bsp
