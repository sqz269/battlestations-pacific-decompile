#pragma once
#include "bsp/camera_frame_state.hpp"

namespace bsp {
// New C++ interfaces over the existing live views, not native ABI replacements.
// 00B6FC30: ECX camera; ST0 aspect(+1C8)*fov(+1C4), explicitly rounded through
// float32 then reloaded to x87; RET0. The arithmetic does not validate inputs.
float get_camera_fov_aspect_product_00b6fc30(const CameraProjection&) noexcept;

// 00B6FDF0: ECX camera, stack full DWORD mode, EAX same camera, RET4.
// actual_mask_19c must be this SAME camera's +19C word. Stores full mode at
// +198, then 1u<<(mode&31) at +19C. Returns the same supplied frame view.
CameraFrameState& set_camera_render_mode_00b6fdf0(CameraFrameState&,
    std::uint32_t& actual_mask_19c, std::uint32_t mode) noexcept;

// 00B6FE10: ECX camera, stack flags, EAX flags, RET4. Raw +188 DWORD store.
DWORD set_camera_clear_flags_00b6fe10(CameraFrameState&, DWORD flags) noexcept;

// 00B6FEC0: ECX camera, stack pointer, EAX same pointer, RET4. Raw +43C store;
// no pointee read or retention. The pointee must outlive later camera uses.
const CameraPlane* set_camera_context_depth_scale_00b6fec0(CameraFrameState&,
    const CameraPlane*) noexcept;
} // namespace bsp
