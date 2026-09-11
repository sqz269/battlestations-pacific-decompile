#pragma once

#include "bsp/camera_projection.hpp"
#include "bsp/ship_class_fields.hpp"

namespace bsp {
// Semantic interfaces, not native object layouts or binary replacements.
// Names are hypotheses. Native addresses, ABI and limits: docs/UNIT_RUDDER_CURVE.md.

// 00419010: five float stack arguments, RET 14h, x87 ST0 result. The interpolated
// value is rounded to float before clamping to the two endpoint ordinates.
float clamped_interpolate_00419010(float x0, float y0, float x1, float y1,
                                  float x) noexcept;

// Settings singleton fields used by 0082E890. No assumed defaults or ordering.
struct UnitRudderCurveSettings {
    float value_0438;
    float speed_043c;
    float value_0440;
    float speed_0444;
    float value_0448;
    float speed_044c;
};

struct UnitRudderCurveHost {
    virtual ~UnitRudderCurveHost() = default;
    // 00424C40. Called four times per curve evaluation, in native order.
    // Returned objects must remain live across subsequent getter calls.
    virtual const UnitRudderCurveSettings& settings_00424c40() = 0;
};

// 0082E890: one float on stack, RET 4, x87 ST0 result. Selects the lower
// segment when magnitude <= settings+44Ch, then clamped interpolation.
float unit_rudder_denominator_0082e890(float magnitude, UnitRudderCurveHost& host);

// 0082ECB0: ECX = class, stack (rudder, forward speed, efficiency), RET Ch,
// ST0 result. Uses ShipClassFields' existing MaxRotAngle/MaxSpeed projections.
// No guard is added for zero max speed or zero curve denominator.
float unit_class_yaw_rate_0082ecb0(const ShipClassFields& ship_class, float rudder,
    float forward_speed, float turn_efficiency, UnitRudderCurveHost& host);

struct UnitRudderHost : UnitRudderCurveHost {
    virtual bool scale_manager_present() = 0; // 00F88C30 != 0
    virtual const ShipClassFields& ship_class() = 0; // unit+538h
    virtual bool gameplay_scale_enabled() = 0; // byte 00E0C978
    virtual bool scale_manager_enabled() = 0; // manager+C4h != 0
    virtual float gameplay_scale_008e6430(int category) = 0; // category 5, unit
    virtual float turn_efficiency() = 0; // unit+9DCh, before forward-speed call
    // 0092D730 on unit+1018h; existing unit_forward_speed_0092d730 implements
    // the body-axis rule. Its physics access remains the integrating host's job.
    virtual float forward_speed_0092d730() = 0;
    virtual float steering_command() = 0; // unit+984h, for 00811940 only
};

// 00811890: ECX = unit, stack rudder, RET 4, ST0 result.
float unit_yaw_rate_00811890(float rudder, UnitRudderHost& host);
// 00811940: ECX = unit, RET, passes unit+984h and preserves returned ST0.
float unit_current_yaw_rate_00811940(UnitRudderHost& host);

// 00438AA0/00438B10: two stack floats, RET 8, ST0 result (the saved Ghidra
// void signatures are wrong). Float-store after each wrap into (-pi, pi].
// pi is the game's float-rounded constant, also stored as double in the image.
// Native loops have no bound: infinities or enormous finite inputs can stall.
float wrapped_angle_add_00438aa0(float left, float right) noexcept;
float wrapped_angle_subtract_00438b10(float left, float right) noexcept;

struct UnitHeadingTargetState {
    float target_heading; // heading-command object+44h
    bool active; // +4Ch
};
struct UnitHeadingTargetHost {
    virtual ~UnitHeadingTargetHost() = default;
    virtual float heading_virtual_0050() = 0; // object+50h unit, virtual +50h
    virtual float forward_speed_0092d730() = 0; // that unit's controller+1018h
};
// 00811960: ECX = heading-command object, desired heading stack float, RET 4.
// The heading interpretation is provisional; angle arithmetic/stores are known.
void unit_set_heading_target_00811960(UnitHeadingTargetState& state,
    float desired_heading, UnitHeadingTargetHost& host);

// 00811AB0's ECX is unit+310h: explicit references preserve mutations by host
// calls before the timestamp is read. These are raw field names, not identities.
struct UnitShiftedUpdateState {
    float& scale_0340;
    CameraMatrix& matrix_0074;
    const CameraMatrix& matrix_0674;
    float& timestamp_0308;
    float& field_02f8;
};
struct UnitShiftedUpdateHost {
    virtual ~UnitShiftedUpdateHost() = default;
    virtual void controller_update_0092f930(float seconds) = 0; // unit+1018h
    virtual void unit_virtual_00d8() = 0;
    virtual float clock_00f876a4() = 0;
};
// ECX = shifted pointer, one float stack argument, RET 4. Only this routine's
// sequence is recovered: the controller update and virtual call remain external.
void unit_shifted_update_00811ab0(UnitShiftedUpdateState& state, float seconds,
    UnitShiftedUpdateHost& host);
}
