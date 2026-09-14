#pragma once
#include <cstdint>

// Reconstruction of 007C6500 BSP_PlaneTickElement_AdvancePose and the two pose
// writers it dispatches to, 007D8230 and 007D9F60. docs/PLANE_ADVANCE_POSE.md
// carries the evidence, the ABI and the uncertainty. Every descriptive name here
// is a hypothesis, not a recovered symbol.
//
// READ THIS FIRST, it changes where a host must put the result. 007C6500 does
// *not* write unit+674h. It reads unit+674h - the committed fixed-step pose - and
// writes unit+74h, the pose that is drawn. The commit back to unit+674h is a
// different tick-element slot, +0Ch / 006D1FC0, already recorded in
// docs/TICK_ELEMENT_OVERRIDES.md:107. 007C6500 is the plane's slot +4h and is the
// exact analogue of the generic unit's 00811AB0. The fixed-step wave runs it with
// the whole step and commits; the interpolation wave runs it with the frame's
// leftover and does not commit.

namespace bsp {

// ---------------------------------------------------------------------------
// Unit offsets. ECX at 007C6500 is unit+310h, the scene node: 007C6538
// LEA EBP,[ESI-310h] recovers the unit, and every displacement below is stated
// against the unit, not the node.
// ---------------------------------------------------------------------------
namespace plane_advance_off {
inline constexpr int kLivePose = 0x074;          // 007D9F7F / 007DA360, the drawn pose
inline constexpr int kChildList = 0x048;         // 007C6530, the subtree to invalidate
inline constexpr int kParent = 0x03C;            // 007DA2C0
inline constexpr int kActiveByte = 0x05C;        // 007C6528, clear takes the early exit
inline constexpr int kSelfBlockByte = 0x061;     // 007C66E7 and 007C657F
inline constexpr int kWorldPoseValid = 0x0C8;    // 007C653E / 007C6721, cleared
inline constexpr int kWorldPoseFlag2 = 0x10C;    // 007C6544 / 007C6727, cleared
inline constexpr int kEarlyExitGuard = 0x4A4;    // 007C655E
inline constexpr int kArmSelector = 0x520;       // 007C66F4, picks 007D8230 vs 007D9F60
inline constexpr int kTimeScale = 0x340;         // 007C6509, scales the step when > 0
inline constexpr int kFixedStepPose = 0x674;     // 007D9F6E / 007D8252, the input
inline constexpr int kFixedStepPosition = 0x6A4; // 007DA241, = kFixedStepPose + 30h
inline constexpr int kFlightState = 0x900;       // 007C66AE
inline constexpr int kLiveRoll = 0x9E4;          // 007C65F0, and the three that follow
inline constexpr int kLivePitch = 0x9E8;         // 007C6628
inline constexpr int kLiveYaw = 0x9EC;           // 007C6668 / 007C6695
inline constexpr int kSpecialBlocker = 0x9D4;    // 007C66DD, a pointer
inline constexpr int kController = 0xAB0;        // 007C66FE, the flight controller
inline constexpr int kExternalVelocity = 0x810;  // 007DA26B / 007D82BE, added to both arms
inline constexpr int kAttachTarget = 0xBF4;      // 007DA2B7
inline constexpr int kPitchAngle = 0xC64;        // 007DA00C, taken through FCOS
inline constexpr int kBankAngle = 0xC68;         // 007DA037, taken through FSIN
inline constexpr int kControlSurfacesOn = 0xC48; // 007C65CE
// The four control-surface groups: array of node pointers, then its count.
inline constexpr int kSurfaceYawNegArray = 0xD9C;   // 007C664F, count at +DDCh
inline constexpr int kSurfaceYawPosArray = 0xDAC;   // 007C668F, count at +DE0h
inline constexpr int kSurfacePitchArray = 0xDBC;    // 007C6613, count at +DE4h
inline constexpr int kSurfaceRollArray = 0xDCC;     // 007C65E4, count at +DE8h
}  // namespace plane_advance_off

// Flight controller offsets, the object at unit+AB0h. 007D9F69 MOV EAX,[EBX+8]
// recovers the unit from it, matching plane_flight_controller_off::kControllerUnit.
namespace plane_advance_ctl_off {
inline constexpr int kUnit = 0x08;             // 007D9F69 / 007D824D
inline constexpr int kClassDescriptor = 0x0C;  // 007DA0B6, gains live here
inline constexpr int kLinearVelocity = 0x18;   // 007DA218..007DA238 and 007D827B
inline constexpr int kAngularVelocity = 0x24;  // 007D9FA6 / 007D9FCC / 007D9FD5
inline constexpr int kLevelBlendInput = 0x7C;  // 007DA080, feeds 00419010
inline constexpr int kLevelAxis = 0x80;        // 007DA13E, a body direction
inline constexpr int kLevelRate = 0x8C;        // 007DA11C, > 0 enables the levelling
inline constexpr int kStateFC = 0xFC;          // 007DA211, == 1 takes the attached branch
}  // namespace plane_advance_ctl_off

// Class descriptor gains, the object at ctl+0Ch.
namespace plane_advance_class_off {
inline constexpr int kBankYawGain = 0x1C8;  // 007DA0C4
inline constexpr int kAttachField = 0x1FC;  // 007DA30A, the attached branch only
}  // namespace plane_advance_class_off

// 00CE3820 and 00D7A350, the two doubles 0085E4D0 compares against: the squared
// length floor and the angle floor. Both are the exact double forms of 1e-10 and
// 1e-8 (0085E50A and 0085E53C).
inline constexpr double kRotationLengthSqFloor_00ce3820 = 1e-10;
inline constexpr double kRotationAngleFloor_00d7a350 = 1e-8;

// 00D7A218 is 0.0f, 00D7A24C is 1.0f, 00D7A208 is -0.0f. All three are recorded
// in docs/TICK_ELEMENT_OVERRIDES.md:41 and re-read here.

// Row-major 4x4, the storage 007D9F74 MOV ECX,10h / MOVSD.REP settles at 64
// bytes. Rows 0/1/2 are the body axes in world and row 3 is the translation, the
// convention docs/ENTITY_LOCAL_MATRIX.md establishes from 00414DB0.
struct AdvanceMatrix {
    float m[16];
    float* row(int i) { return m + 4 * i; }
    const float* row(int i) const { return m + 4 * i; }
};

// ---------------------------------------------------------------------------
// The dispatch, 007C6500
// ---------------------------------------------------------------------------

enum class PlaneAdvanceArm {
    // 007C6528 found unit+5Ch clear: the pose cache is invalidated and the
    // routine returns without touching either matrix.
    Inactive,
    // 007C66E7 or 007C66F2 blocked it: 007C6714 FSTP ST0 discards the step and
    // no pose is written, but the invalidate at 007C6716 still runs.
    Blocked,
    // 007C6706, unit+520h set. Position only, no rotation at all.
    TranslateOnly,
    // 007C670D, unit+520h clear. The full rotate-and-translate.
    RotateAndTranslate,
};

struct PlaneAdvanceGate {
    float step = 0.0f;
    float time_scale = 0.0f;       // unit+340h
    bool active = false;           // unit+5Ch != 0
    bool self_blocked = false;     // unit+61h != 0
    bool special_blocker = false;  // unit+9D4h != nullptr
    bool blocker_blocks = false;   // (unit+9D4h)->+61h != 0
    bool translate_only = false;   // unit+520h != 0
};

struct PlaneAdvanceSelection {
    PlaneAdvanceArm arm = PlaneAdvanceArm::Inactive;
    float step = 0.0f;  // after the unit+340h scaling
};

// 007C6500's head and its 007C66DD..007C6704 gate, together.
PlaneAdvanceSelection select_advance_arm_007c6500(const PlaneAdvanceGate& gate);

// 007C65DA..007C66AE, the cosmetic block: four groups of animated nodes, each
// driven by one signed control axis. These are the values handed to 007DE2E0;
// this rule does not model 007DE2E0 itself.
struct ControlSurfaceDrive {
    float roll_group = 0.0f;       // 007C65F0  +unit+9E4h
    float pitch_group = 0.0f;      // 007C6628  -unit+9E8h
    float yaw_negative_group = 0.0f;  // 007C6668  -unit+9ECh
    float yaw_positive_group = 0.0f;  // 007C6695  +unit+9ECh
};
ControlSurfaceDrive control_surface_drive_007c65da(float roll, float pitch, float yaw);

// ---------------------------------------------------------------------------
// 0085DAD0, the up-first orthonormaliser
// ---------------------------------------------------------------------------
// The mirror of 0085DC80: row 1 is the authority and row 2 is projected off it,
// where 0085DC80 holds row 2 and projects row 1. Both end with
// row0 = normalize(row1 x row2), so the two agree on handedness and neither
// contradicts the row-2-is-forward convention - they differ only in which row the
// caller has just written and therefore wants preserved. Body 0085DAD0-0085DC7C,
// __fastcall(float* m /*ECX*/), plain RET. No degenerate branch, unlike 0085DC80.
void orthonormalize_up_first_0085dad0(AdvanceMatrix& m);

// ---------------------------------------------------------------------------
// 0085E4D0, the angular-velocity integration
// ---------------------------------------------------------------------------

struct RotationAxisAngle {
    bool rotates = false;  // false takes the 0085E871 exit, which writes nothing
    float axis[3] = {0.0f, 0.0f, 0.0f};  // the argument, normalised in place
    float angle = 0.0f;                  // |w|, before the sign flip at 0085E750
    // Always (1,0,0) or (0,0,1) with y = 0 - that much is proven. Which of the
    // two the native picks is NOT: 0085E58F..0085E6BB's comparison chain was only
    // partially read. The choice cannot change the rotation, because two look-at
    // frames about one axis differ by a rotation about their own Z and that
    // commutes with the RotZ between them. See the .cpp for the argument.
    float up_reference[3] = {0.0f, 0.0f, 0.0f};
    bool up_reference_predicate_unread = false;
};

// 0085E4D6..0085E6DD. Everything 0085E4D0 decides before it calls out to the
// matrix builders: the length guards, the axis, the angle and the up reference.
RotationAxisAngle rotation_axis_angle_0085e4d0(const float w[3]);

// The four native matrix primitives 0085E4D0 and 007D9F60 build the result from.
// All of them are already reconstructed elsewhere in this repo, so they are taken
// as a contract here rather than re-derived: 00B63F10 in src/camera_look_at.cpp,
// 00B64780 and 00B646E0 in src/native_particle_axial_loading.cpp, 0042D0D0 in
// src/material_effect_plane.cpp, 00419010 in src/unit_rudder.cpp. 00413920's
// convention (ECX = left, stack dst and right) is docs/ENTITY_LOCAL_MATRIX.md:82.
struct AdvanceMatrixOps {
    virtual ~AdvanceMatrixOps() = default;
    virtual void build_look_at_00b63f10(AdvanceMatrix& out, const float eye[3],
                                        const float target[3], const float up[3]) = 0;
    virtual void build_rotation_z_00b64780(AdvanceMatrix& out, float angle) = 0;
    virtual void build_rotation_y_00b646e0(AdvanceMatrix& out, float angle) = 0;
    virtual void multiply_00413920(AdvanceMatrix& dst, const AdvanceMatrix& left,
                                   const AdvanceMatrix& right) = 0;
    virtual void transform_direction_0042d0d0(float out[3], const float v[3],
                                              const AdvanceMatrix& m) = 0;
    virtual float interpolate_clamped_00419010(float x0, float y0, float x1, float y1,
                                               float x) = 0;
};

// 0085E4D0, __fastcall(AdvanceMatrix* out /*ECX*/, const AdvanceMatrix* in /*EDX*/,
// float w[3], float scale /*stack, RET 10h*/).
// out = ((in * L) * RotZ(-|w| * scale)) * transpose(L), then out's row 3 is
// restored from in's, then 0085D3D0 runs on out.
// `eye_00f87574` is the global at 00F87574 that 0085E6EA hands to the look-at as
// the eye. It is the **zero vector**: the twelve bytes read as zero, it lies past
// .data's raw size so it is BSS (docs/COMMAND_EXECUTION.md:61),
// docs/AI_PLANNER_TAILS.md:155 names it "the zero vector", and 007D7C00 writes it
// into six accumulators to clear them (docs/PLANE_FREE_FLIGHT_PHYSICS.md:27). It
// stays a parameter rather than a constant only because nothing proves no one
// writes it. With the eye at the origin the look-at's forward is exactly the
// normalised axis.
// Returns false when the 0085E871 exit was taken, in which case `out` is not
// written at all.
bool rotate_about_axis_0085e4d0(AdvanceMatrix& out, const AdvanceMatrix& in,
                                const float w[3], float scale,
                                const float eye_00f87574[3], AdvanceMatrixOps& ops);

// ---------------------------------------------------------------------------
// The position law, shared by both arms
// ---------------------------------------------------------------------------
// 007DA218..007DA2AD and 007D827B..007D8300, the same expression in both:
//   pos = committed_translation + linear_velocity * step + external_velocity * step
// 007D8230 takes the committed translation out of its own copy of unit+674h's
// row 3; 007D9F60 reads unit+6A4h, which is the same sixteen bytes.
void advance_position_007da218(float out[3], const float committed_translation[3],
                               const float linear_velocity[3],
                               const float external_velocity[3], float step);

// ---------------------------------------------------------------------------
// 007D9F60's two extra rotations
// ---------------------------------------------------------------------------

// 007DA00C..007DA0E9. The bank-driven yaw: the angle handed to 00B646E0.
// -(blend * sin(bank) * cos(pitch) * gain * difficulty * step).
// `difficulty` is 1.0f unless the 007DA03D / 007DA05B pair of globals both pass,
// in which case it is 008E6430(0Dh, unit), which this rule does not model.
float bank_yaw_angle_007da0ae(float blend, float bank_angle, float pitch_angle,
                              float bank_yaw_gain, float difficulty, float step);

// 007DA179..007DA19C, the levelling blend: min(rate * step, 1.0f).
float level_blend_007da179(float level_rate, float step);

// 007DA14D..007DA208. Row 1 is pulled toward world up by `blend`:
//   row1 += ((0,1,0) - transformed_level_axis) * blend
// where transformed_level_axis is ctl+80h put through the working matrix by
// 0042D0D0 with normalise false.
void apply_up_levelling_007da14d(AdvanceMatrix& m, const float transformed_level_axis[3],
                                 float blend);

// ---------------------------------------------------------------------------
// The two arms
// ---------------------------------------------------------------------------

struct PlaneAdvanceInputs {
    AdvanceMatrix committed_pose{};  // unit+674h
    float step = 0.0f;               // already scaled by unit+340h
    float linear_velocity[3] = {0.0f, 0.0f, 0.0f};   // ctl+18h
    float angular_velocity[3] = {0.0f, 0.0f, 0.0f};  // ctl+24h
    float external_velocity[3] = {0.0f, 0.0f, 0.0f};  // unit+810h
    float eye_00f87574[3] = {0.0f, 0.0f, 0.0f};       // the look-at eye global
    // The bank-driven yaw term.
    float bank_angle = 0.0f;       // unit+C68h
    float pitch_angle = 0.0f;      // unit+C64h
    float level_blend_input = 0.0f;  // ctl+7Ch
    float bank_yaw_gain = 0.0f;      // classDesc+1C8h
    float difficulty = 1.0f;         // 1.0f, or 008E6430(0Dh, unit)
    float blend_curve_x0 = 0.0f;     // 00CE3854
    float blend_curve_x1 = 0.0f;     // 00CE3850
    // The levelling term.
    float level_rate = 0.0f;             // ctl+8Ch, > 0 enables it
    float level_axis[3] = {0.0f, 0.0f, 0.0f};  // ctl+80h
    // 007DA211's attached branch.
    bool attached = false;  // ctl+FCh == 1 && unit+BF4h && unit+3Ch
};

struct PlaneAdvanceOutput {
    AdvanceMatrix live_pose{};  // what lands in unit+74h
    bool rotation_applied = false;  // 0085E4D0 did not take its 0085E871 exit
    bool levelling_applied = false;
    // True when 007DA211's attached branch would have run. That branch is NOT
    // modelled: see the doc's coverage table.
    bool attached_branch_unmodelled = false;
};

// 007D8230, __thiscall(ctl, float step), RET 4. Position only.
PlaneAdvanceOutput translate_only_007d8230(const PlaneAdvanceInputs& in);

// 007D9F60, __thiscall(ctl, float step), RET 4. Rotation then position.
PlaneAdvanceOutput rotate_and_translate_007d9f60(const PlaneAdvanceInputs& in,
                                                 AdvanceMatrixOps& ops);

}  // namespace bsp
