#include "bsp/camera_configuration.hpp"

namespace bsp {
namespace {
// Native FLD/FMUL/FSTP32/FLD32 order, using the actual supplied field addresses.
// Private fastcall arguments replace only the original camera+offset addressing.
__declspec(naked) float __fastcall fov_aspect_kernel(const float*, const float*) {
    __asm {
        push ecx
        fld dword ptr [ecx] // 00B6FC31: actual aspect
        fmul dword ptr [edx] // 00B6FC37: actual fov
        fstp dword ptr [esp] // 00B6FC3D: explicit float32 rounding
        fld dword ptr [esp] // 00B6FC40: native ST0 return
        pop ecx
        ret
    }
}
} // namespace

float get_camera_fov_aspect_product_00b6fc30(const CameraProjection& projection) noexcept {
    return fov_aspect_kernel(&projection.aspect, &projection.fov);
}

CameraFrameState& set_camera_render_mode_00b6fdf0(CameraFrameState& frame,
    std::uint32_t& actual_mask_19c, std::uint32_t mode) noexcept {
    const std::uint32_t mask = 1u << (mode & 31u); // x86 SHL masks CL to five bits.
    static_cast<volatile std::uint32_t&>(frame.render_mode) = mode;
    static_cast<volatile std::uint32_t&>(actual_mask_19c) = mask;
    return frame;
}

DWORD set_camera_clear_flags_00b6fe10(CameraFrameState& frame, DWORD flags) noexcept {
    static_cast<volatile DWORD&>(frame.clear_flags) = flags;
    return flags;
}

const CameraPlane* set_camera_context_depth_scale_00b6fec0(CameraFrameState& frame,
    const CameraPlane* value) noexcept {
    static_cast<const CameraPlane* volatile&>(frame.context_depth_scale_43c) = value;
    return value;
}
} // namespace bsp
