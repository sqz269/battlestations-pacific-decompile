#pragma once
#include "bsp/plane_advance_pose.hpp"

// Packet cc7_plane_angular_velocity. Two things:
//
//  1. The **sense** of the rotation `0085E4D0` applies, which
//     docs/PLANE_ADVANCE_POSE.md left open. It is settled here, by reading
//     00B63F10, 00B64780, 00B646E0 and 00413920 rather than by assuming a
//     convention. Result: the pose is rotated by |w*dt| about **-w**, the
//     negative of the angular velocity. See the doc for the derivation.
//
//  2. The producer of `ctl+24h..2Ch`. docs/PLANE_ADVANCE_POSE.md named
//     007D8470's tail as the candidate. **That was wrong**: 007D8470 integrates
//     the *linear* body velocity and never touches an angular term. The real
//     producer is one hop away and is 007D9C80, from a body-frame angular
//     velocity at ctl+48h..50h that 007DA710 writes.
//
// docs/PLANE_ANGULAR_VELOCITY.md carries the evidence. Every descriptive name is
// a hypothesis, not a recovered symbol.

namespace bsp {

// ---------------------------------------------------------------------------
// The four native matrix primitives, read off their listings in this packet.
// docs/PLANE_ADVANCE_POSE.md took these as a contract because the existing
// in-repo reconstructions are inline-asm transcriptions that cannot be bound
// into a portable host. These are the recovered formulas instead.
// ---------------------------------------------------------------------------

// 00B63F10 BSP_Matrix_BuildLookAt, __fastcall(m ECX, eye EDX, stack: target*,
// up.x, up.y, up.z), RET 10h, EAX = m. This is exactly D3DXMatrixLookAtLH:
//   zaxis = normalize(target - eye)            00B63F87 FLD [EAX] / FSUB [eye]
//   xaxis = normalize(up x zaxis)              00B64120, a = up, b = zaxis
//   yaxis = normalize(zaxis x xaxis)           00B64198, a = zaxis, b = xaxis
// and the axes land in the **columns**: 00B641D7 m[0] = x.x, 00B641E5 m[2] = z.x,
// 00B6424D m[1] = y.x, and so on down each column. Row 3 is
// (-dot(x,eye), -dot(y,eye), -dot(z,eye)) - 00B64290 FCHS / FSTP [ESI+30h].
void build_look_at_lh_00b63f10(AdvanceMatrix& out, const float eye[3],
                               const float target[3], const float up[3]);

// 00B64780 BSP_Matrix_BuildRotationZ, __fastcall(m ECX, const float* angle EDX).
//   [  cos sin 0 0 ][ -sin cos 0 0 ][ 0 0 1 0 ][ 0 0 0 1 ]
// 00B647C0 cos at m[0], 00B647D5 sin at m[1], 00B647E4 -sin at m[4] (from the
// -0.0f at 00D7A208), 00B647C4 cos at m[5].
void build_rotation_z_00b64780(AdvanceMatrix& out, float angle);

// 00B646E0 BSP_Matrix_BuildRotationY, the same shape about Y.
//   [ cos 0 -sin 0 ][ 0 1 0 0 ][ sin 0 cos 0 ][ 0 0 0 1 ]
// 00B64731 cos at m[0], 00B64724 -sin at m[2], 00B64753 sin at m[8],
// 00B6475D cos at m[10], 00B64744 and 00B64776 the two 1.0f.
void build_rotation_y_00b646e0(AdvanceMatrix& out, float angle);

// 00413920 BSP_Matrix_Multiply4x4, __fastcall(left ECX, stack: dst, right).
// The plain row-major product dst = left * right: 00413927..0041397B assembles
// dst[0] from left[0]*right[0] + left[1]*right[4] + left[2]*right[8] +
// left[3]*right[12], which is sum_k left[0][k] * right[k][0].
void multiply_00413920(AdvanceMatrix& dst, const AdvanceMatrix& left,
                       const AdvanceMatrix& right);

// 0042D0D0 BSP_Vector3f_TransformDirectionOptionalNormalize, the normalise =
// false path only - every caller in this chain passes 0. __fastcall(out ECX,
// v EDX, stack: matrix, normalise), RET 8. The 3x3 row-vector product with no
// translation: 0042D0F2..0042D109 builds out[0] from v.x*m[0] + v.y*m[4] +
// v.z*m[8], which is `v * M` down column 0.
void transform_direction_0042d0d0(float out[3], const float v[3],
                                  const AdvanceMatrix& m);

// A complete binding of docs/PLANE_ADVANCE_POSE.md's contract. That packet left
// AdvanceMatrixOps abstract because none of its six members had a portable
// reconstruction; five are recovered above and the sixth already had one in
// include/bsp/unit_rudder.hpp, so the host no longer has to supply anything.
struct NativeAdvanceMatrixOps final : AdvanceMatrixOps {
    void build_look_at_00b63f10(AdvanceMatrix& out, const float eye[3],
                                const float target[3], const float up[3]) override;
    void build_rotation_z_00b64780(AdvanceMatrix& out, float angle) override;
    void build_rotation_y_00b646e0(AdvanceMatrix& out, float angle) override;
    void multiply_00413920(AdvanceMatrix& dst, const AdvanceMatrix& left,
                           const AdvanceMatrix& right) override;
    void transform_direction_0042d0d0(float out[3], const float v[3],
                                      const AdvanceMatrix& m) override;
    float interpolate_clamped_00419010(float x0, float y0, float x1, float y1,
                                       float x) override;
};

// ---------------------------------------------------------------------------
// The rotation sense
// ---------------------------------------------------------------------------

// The closed form of what `rotate_about_axis_0085e4d0` computes, once the four
// primitives above are substituted in: a right-hand rotation of the pose by
// **-|w|** radians about `normalize(w)`, equivalently +|w| about -w.
//
// Same result as the native four-matrix composition, and the packet's probe
// checks the two against each other. Prefer this in a host: it is one Rodrigues
// evaluation instead of a look-at, a transpose and three 4x4 products, and it
// has no degenerate up-reference to pick.
//
// Row 3 is copied from `in`, matching 0085E4D0's 0085E833 restore. `in`'s basis
// rows are rotated; nothing else is touched.
void rotate_about_axis_equivalent(AdvanceMatrix& out, const AdvanceMatrix& in,
                                  const float w[3]);

// ---------------------------------------------------------------------------
// The controller's four velocity slots and the pair that converts between them
// ---------------------------------------------------------------------------
// 007D9C10 and 007D9C80 are mirror images, and together they settle what the
// four slots are:
//
//   |         | body frame   | world frame  |
//   | linear  | ctl+3Ch..44h | ctl+18h..20h |
//   | angular | ctl+48h..50h | ctl+24h..2Ch |
namespace plane_velocity_off {
inline constexpr int kLinearWorld = 0x18;   // 007D9CA0, 007D9CA9, 007D9CB5
inline constexpr int kAngularWorld = 0x24;  // 007D9CC3, 007D9CCB, 007D9CD1
inline constexpr int kLinearBody = 0x3C;    // 007D9C3B, 007D9C44, 007D9C51
inline constexpr int kAngularBody = 0x48;   // 007D9C5C, 007D9C64, 007D9C6A
inline constexpr int kWorldToBody = 0xB0;   // 007D9C1A, rebuilt by 0085DEA0
}  // namespace plane_velocity_off

struct ControllerVelocities {
    float linear_world[3] = {0.0f, 0.0f, 0.0f};
    float angular_world[3] = {0.0f, 0.0f, 0.0f};
    float linear_body[3] = {0.0f, 0.0f, 0.0f};
    float angular_body[3] = {0.0f, 0.0f, 0.0f};
};

// 007D9C10 BSP_PlaneFlightController_RefreshBodyFrame, __thiscall(ctl), body
// 007D9C10-007D9C75, tail-jumps to 007D8020. World to body, through ctl+0B0h,
// which 007D9C25 has just rebuilt from unit+74h with 0085DEA0.
// The 0085DEA0 rebuild and the 007D8020 tail call are NOT modelled here.
void world_to_body_007d9c10(ControllerVelocities& v, const AdvanceMatrix& world_to_body);

// 007D9C80 BSP_PlaneFlightController_BodyToWorldVelocity, __thiscall(ctl), body
// 007D9C80-007D9CDC, tail-jumps to 007D8020. Body to world, through unit+74h
// directly. This is the producer of the angular velocity 0085E4D0 consumes.
// The 007D8020 tail call is NOT modelled here.
void body_to_world_007d9c80(ControllerVelocities& v, const AdvanceMatrix& live_pose);

}  // namespace bsp
