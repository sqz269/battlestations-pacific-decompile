#pragma once

// Directional shadow view fitting in segment 74, reconstructed by cc13-shadow1.
// Names are hypotheses, not recovered symbols. Evidence, original ABIs and open
// questions are in docs/CC13_SHADOW_CASCADES.md. The routines work on the raw
// directional shadow owner addressed by offset (the object 00A8EA20 and
// 00A8E860 receive in ECX); none is a drop-in binary replacement. Camera,
// look-at, ortho, normalize and render-queue calls stay host calls; the pure
// matrix/vector helpers that already have reconstructions (00413920, 004142E0,
// 004134F0, 00412E20, 00A8AC90, 00A8B860) are called directly.
//
// x87 model: the image runs these bodies with the x87 in 53-bit precision, so
// every unspilled intermediate is reproduced in double and every FSTP to a
// float slot as a float cast. Exponent-range differences (an intermediate
// that overflows float but not the x87 range) are not modelled.

#include "bsp/camera_projection.hpp"

#include <cstdint>

namespace bsp::cc13_shadow {

// Engine services the fitters call. Each method names its native callee.
class ShadowFitHost {
public:
    virtual ~ShadowFitHost() = default;
    // 00B7AAE0 BSP_Render_GetOverrideContextWord (MOV EAX,[0109019C]).
    virtual std::uint32_t override_context_word() = 0;
    // (owner+0Ch)->vslot 0Ch(context word), AL tested. Native reads the light
    // pointer and its vtable before calling 00B7AAE0.
    virtual bool light_query_0c(void* light, std::uint32_t context_word) = 0;
    // 00419510 BSP_Vector3f_Normalize, ECX out, EDX in.
    virtual void normalize(float* out3, const float* in3) = 0;
    // 00419440 BSP_Vector3f_Length, ECX vector, ST0 result (float-rounded).
    virtual float length(const float* v3) = 0;
    // 00BF7030 _CIsqrt on ST0 (53-bit), result left on ST0.
    virtual double crt_sqrt(double value) = 0;
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix (callers test flags +5Ch bit 1 first).
    virtual void refresh_world_matrix(void* camera) = 0;
    // 00B63F10, ECX out, EDX eye, stack target pointer, three up floats, RET 10h.
    virtual void look_at(CameraMatrix& out, const float* eye3, const float* target3,
        float up_x, float up_y, float up_z) = 0;
    // 00B62A10 BSP_Matrix_OrthoOffCenterLH, ECX out, l, r, b, t, zn, zf, RET 18h.
    virtual void ortho_off_center(CameraMatrix& out, float l, float r, float b,
        float t, float zn, float zf) = 0;
    virtual void set_view_matrix(void* camera, const CameraMatrix& m) = 0;       // 00B71490
    virtual void set_projection_matrix(void* camera, const CameraMatrix& m) = 0; // 00B6FD60
    virtual const CameraMatrix& view_matrix(void* camera) = 0;                   // 00B6FCB0
    virtual const CameraMatrix& view_projection_matrix(void* camera) = 0;        // 00B70490
    // 00B6FEC0 BSP_Camera_SetContextDepthScalePointer (camera+43Ch = pointer).
    virtual void set_depth_scale_pointer(void* camera, float* pointer) = 0;
    // 004C11F0 queue, then 00B1F4D0 BSP_RenderCommandQueue_SubmitNamedScene
    // (scene, camera, target, native string by value).
    virtual void queue_named_command(void* scene, void* camera, void* target, const char* name) = 0;
    // 004C11F0 queue, then 00B1CB50(&metadata3).
    virtual void set_last_command_metadata(const std::uint32_t* metadata3) = 0;
};

// 00A8B930, ECX out float[9] (row-major 3x3), EDX p (float2), stack a, b, c
// (float2 pointers), RET 0Ch; EDX is an input register. Unreferenced in the
// image. Builds a 3x3 homography in the trapezoidal-shadow-map style:
// rotation from u = (b - c)/2, translation to the intersection of lines p-c
// and a-b (00A8B860), a shear centring b and c, scales that send b to x = 1,
// the projective step row2 += (0,0,1) / row3 = old row2, a translation
// centring b and p in y, and a final scale. Every FSTP and reread of the native
// order is kept, including the dead stores to out[2] and out[5].
void build_trapezoid_homography_00a8b930(float* out9, const float* p, const float* a,
    const float* b, const float* c) noexcept;

// 00A8AEB0, __thiscall(owner, unused, const float* box_min, const float* box_max),
// RET 0Ch. Callers 00A8E180, 00A8E860 (first argument is their camera, which
// this body never reads). Sets the split table +390h..+39Ch to 100/200/300/400,
// resets +44h and +144h to identity, takes the light direction (normalized
// light+1E0h with |y| pushed up to tan(0.436332) when the light query passes,
// otherwise (0,1,0)) into +38h..+40h, looks from render camera +30h position +
// 10000*dir back at that position, fits an ortho box around the eight
// light-space corners of [box_min, box_max], mirrors x, and stores the light
// view with depth remapped to the box into +3ACh, its third column into
// +4ACh..+4B8h and over the shader matrix column +14Ch/+15Ch/+16Ch/+17Ch.
void fit_shadow_box_view_00a8aeb0(void* owner, const float* box_min, const float* box_max,
    ShadowFitHost& host);

// 00A8ECB0, __thiscall(owner, float near, float far, int cascade), SEH frame,
// RET 0Ch. Unreferenced in the image (an older cascade path; 00A8EA20 calls
// 00A8BD20 instead). Builds the render camera frustum slice [near, far],
// projects its corners through view*P (00A8AC90; the results are overwritten
// before use), forms H = horizontal look-at from the ground point under the
// camera, a top-down look-at TD and ortho O(-1,1,0,1,-100,100), stores
// H*P*TD*O as cascade matrix +44h+40h*c and its product with +244h+40h*c as
// +144h+40h*c, sets view H and projection P*TD*O on cascade camera +20h+4c,
// clears byte +3A0h+c and queues an SHBU command for target +4F0h+4c with
// metadata {0, 62h, 1Eh}.
void fit_and_submit_shadow_slice_00a8ecb0(void* owner, float near_distance, float far_distance,
    std::int32_t cascade, ShadowFitHost& host);

// 00A8BD20, __thiscall(owner, float near, float far, int cascade), RET 0Ch, no
// SEH frame. Caller 00A8EA20 (four cascades). A light-space perspective fit
// (LiSPSM-style) of the render camera slice [near, far]:
//  1. slice corners C in camera space (as 00A8ECB0), W = C through the camera
//     world +F0h;
//  2. a light look-at from mid-slice + 10000*dir with up (0,0,1) when
//     |dir.y| > |dir.x| else (0,1,0); its view times a [zmin,zmax] depth remap
//     goes to +3ACh+40h*c and its third column to +4ACh+10h*c (the camera
//     depth-scale pointer of cascade camera +20h+4c);
//  3. sinGamma from the light and camera forward, n_opt = (+4ECh)/sinGamma;
//     when n_opt >= far/(far-near) the slice is pushed back by
//     shift = (far-near)*n_opt - far + 1e-4 (T = z translation) and a
//     perspective P' over the shifted near plane is built;
//  4. the light direction through view and P' gives a post-perspective light
//     point; byte +3A0h+c records w < 0. With |w| <= 0.001 the fit is an
//     ortho box (z padded by 30) in a look-at from the point towards
//     (0,0,0.5); otherwise a look-at from the point at the PPS box centre with
//     the slice axis as up and a perspective whose x/y scales bound the
//     corners' x/z and y/z;
//  5. cascade matrix +44h+40h*c = V*T*P'*LA*X*mirror (mirror x unless the w
//     flag), +144h+40h*c = that * +244h+40h*c, the depth column copied over
//     +14Ch.., cascade camera view = render camera view, projection =
//     T*P'*LA*X*mirror, camera +178h = owner +8.
void fit_shadow_cascade_00a8bd20(void* owner, float near_distance, float far_distance,
    std::int32_t cascade, ShadowFitHost& host);

} // namespace bsp::cc13_shadow
