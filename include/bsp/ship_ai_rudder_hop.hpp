#pragma once

// The tail of BSP_UnitInstance_UpdateShipMotion (00825F20), 00826C34..00826D69.
//
// Packet cc8_ship_ai_rudder_hop. docs/SHIP_AI_RUDDER_HOP.md carries the evidence.
//
// docs/SHIP_MOTION.md reconstructed 00826121..00826B84 and left "the tail after
// 00826B84" explicitly out. This header is that tail. It is NOT the AI-heading
// to rudder hop that docs/UNIT_AI_ORDER_SLOT_READER.md and
// docs/SHIP_AI_RING_WINNER.md expected here: the ordered rudder unit+984h has
// already been applied to the body by 0092E8C0 at 00826B54, forty instructions
// earlier, and what this tail does with it is derive a yaw rate for the wake
// trail. The corrections section of docs/SHIP_AI_RUDDER_HOP.md states that in
// full.
//
// Register frame, from docs/SHIP_MOTION.md and re-checked by filtering the whole
// listing: ESI = unit, EDI = unit+310h, EBP = EBX = unit+0CCh (the world 4x4,
// rows at +00h/+10h/+20h/+30h), [ESP+0C4h] = the scaled delta (the incoming
// argument slot, rewritten at 00826126..00826144).

#include "bsp/world_ocean.hpp"

namespace bsp {

// 00826C3B..00826C56.
//
//   00826C3B  FLD dword [EBP+20h]          ; world row 2 x  = unit+0ECh
//   00826C3E  FLD dword [EBX+28h]          ; world row 2 z  = unit+0F4h
//   00826C41  CALL 00BF701A                ; LIBCRT_atan2, ST1 = y, ST0 = x
//   00826C56  FSTP dword [EDI+0D40h]       ; unit+1050h
//
// Row 2 of the world matrix is the forward axis (docs/SHIP_MOTION.md names
// unit+0ECh..+0F4h poseRow2), so this is the hull's yaw about the world up
// axis, measured from +Z toward +X. unit+1050h is the field the unit vtable's
// slot 50h returns: 00CFC3D0+50h is 006DFD60, seven bytes,
// `FLD dword [ECX+1050h]; RET`.
//
// The heading is therefore produced here, out of the pose the physics step at
// 00826B6A left behind. It is an output of the tick, not a steering input.
float ship_hull_heading_00826c3b(float forward_x, float forward_z) noexcept;

// 00826C61..00826C81, the same expression 00811940
// (BSP_UnitInstance_GetCurrentCommandYawRate) packages:
//   00826C61  FLD dword [ESI+984h]         ; the ordered rudder
//   00826C6B  PUSH ECX / FSTP dword [ESP]  ; the outgoing float
//   00826C75  CALL 00811890                ; ECX = unit, RET 4, ST0 result
// bsp/unit_rudder.hpp reconstructs 00811890 as unit_yaw_rate_00811890; the host
// method below is how this tail reaches it.

struct ShipMotionTailInputs {
    // unit+0ECh and unit+0F4h, world row 2. Read at 00826C3B and 00826C3E.
    float forward_x{0.0f};
    float forward_z{1.0f};

    // unit+984h, the ordered rudder the order ring last stepped
    // (ring+14Ch, docs/MOTION_DIFFERENTIAL.md). Read at 00826C61.
    float ordered_rudder{0.0f};

    // unit+0FCh, world row 3, the hull's world position. Passed to 00810190 at
    // 00826CE7 as LEA ECX,[ESI+0FCh].
    OceanVec3 world_position{};

    // unit+1158h and [ESP+0C4h]. 00826CFE..00826D38.
    float occupant_timer{0.0f};
    float dt{0.0f};

    // unit+308h and 00D7A218, the pair compared at 00826D4B. The MSVC float
    // "!=" idiom: UCOMISS, LAHF, TEST AH,44h, JNP past the call.
    float field_308{0.0f};
    float field_308_sentinel{0.0f};
};

struct ShipMotionTailStep {
    // What 00826C56 wrote into unit+1050h.
    float hull_heading{0.0f};
    // What 00826CDB read back out of it through the vtable. Equal to
    // hull_heading unless the host's slot 50h disagrees with the write above,
    // which is how a binder detects a unit whose slot 50h is not 006DFD60.
    float heading_read_back{0.0f};
    // 00826C75's result: the yaw rate the rudder curve maps the ordered rudder
    // to. Reaches nothing but the wake sample.
    float yaw_rate{0.0f};
    // 00826CEE ran, which it always does.
    bool wake_sampled{false};
    // 00826CFC: 00778890 said this unit is the one its occupant record points
    // back at, so the timer block ran.
    bool occupant_timer_ran{false};
    // The new unit+1158h, after the subtraction at 00826D04 and, when it went
    // negative, the refill at 00826D2C.
    float occupant_timer_left{0.0f};
    // 00826D1F fell through: the timer expired this tick and 00826D3E fired.
    bool occupant_timer_expired{false};
    // 00826D56 fell through: the two floats differ and 00826D5A fired.
    bool field_308_check_ran{false};
};

// One method per native call site in 00826C34..00826D69 that this reconstruction
// does not itself compute, in call order. Nothing has a default.
struct ShipMotionTailHost {
    virtual ~ShipMotionTailHost() = default;

    // 00826C5C: 0092E5B0(unit+1018h, 0). ECX is the unit controller, the same
    // object 0092D730 and 0092E8C0 take. Body not read by this packet: it is
    // 0092E5B0..0092E8A0, gated on controller+60h, and it reads the physics
    // body at controller+2Ch through 00C31F40. Named by address on purpose.
    virtual void controller_0092e5b0(int flag) = 0;

    // 00826C75: 00811890(unit, rudder), RET 4, ST0. Reconstructed as
    // unit_yaw_rate_00811890 in bsp/unit_rudder.hpp; a binder that already has a
    // UnitRudderHost forwards to it.
    virtual float yaw_rate_from_rudder_00811890(float ordered_rudder) = 0;

    // 00826C7A..00826CC5, the third inlined copy of the world-matrix accessor in
    // this function: if unit+0C8h is clear, rebuild unit+0CCh from unit+74h,
    // combined with [unit+3Ch]+0CCh through 00413920 when the unit has a parent,
    // by 004134F0; then set unit+0C8h and clear unit+10Ch. The first two copies
    // are at 00826B86 and 00826BE0. docs/ENTITY_LOCAL_MATRIX.md owns the
    // accessor itself.
    virtual void refresh_world_matrix() = 0;

    // 00826CDB: CALL [[ESI]+50h] with nothing pushed, __thiscall float(void),
    // RET 0. The concrete target for a unit is 006DFD60, FLD [ECX+1050h]; RET.
    virtual float unit_heading_vtable50() = 0;

    // 00826CEE: 00810190(unit+0BD0h, &unit+0FCh, heading, yaw_rate), RET 0Ch.
    // 00810190 appends to a forty-entry ring of 18h-byte records at
    // sub+8 with the write cursor at sub+3C8h, a drift offset at sub+3D0h and a
    // flag at sub+3CCh; each record is position, then the heading at +14h and
    // the yaw rate at +1Ch. Its only caller in the image is 00825F20. The
    // packet reads it far enough to establish the argument roles and the record
    // layout and no further.
    virtual void append_wake_sample_00810190(const OceanVec3& world_position,
                                             float heading,
                                             float yaw_rate) = 0;

    // 00826CF5: 00778890(unit). Seventeen bytes:
    //   MOV EAX,[ECX+284h]; TEST EAX,EAX; JNE +3; XOR AL,AL; RET
    //   XOR EDX,EDX; CMP [EAX+14h],ECX; SETE DL; ...
    // true when the unit has an occupant record at unit+284h whose +14h points
    // back at this unit.
    virtual bool occupant_owns_unit_00778890() = 0;

    // 00826D21..00826D26: 00424C40()+430h, the refill the expired timer adds.
    virtual float occupant_timer_refill_00424c40_430() = 0;

    // 00826D3E: 0070DB60([unit+284h]). Body not read: 0070DB60..0070E3xx, gated
    // on record+4FCh. Named by address on purpose.
    virtual void occupant_tick_0070db60() = 0;

    // 00826D5A: 0077A650(unit). Fifty-eight bytes: when unit+308h is at or above
    // [00F876A4] and [[00E188A8]+1FE4h] is not 2, it calls
    // 0090EBF0([[00E188A8]+21A0h], unit) and then 00926D90(unit, 3).
    virtual void field_308_crossed_0077a650() = 0;
};

// 00826C34..00826D69, the tail of 00825F20 after the controller step.
//
// Coverage is complete for the range: every instruction from 00826C34 to the
// RET 4 at 00826D69 is either computed here or is one of the host calls above.
// The three inlined world-matrix copies below 00826C34 are not in the range.
ShipMotionTailStep ship_motion_tail_00826c34(const ShipMotionTailInputs& in,
                                             ShipMotionTailHost& host);

} // namespace bsp
