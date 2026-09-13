#pragma once
#include "bsp/native_renderer_camera_prepare.hpp"
#include "bsp/native_renderer_viewport_clear.hpp"

namespace bsp {
// Borrow the actual shared global and immutable concrete-profile binding.
// The global's value and every referenced profile slot remain live reads.
// All providers operate on the SAME actual camera/renderer/viewport storage.
struct NativeCameraFrameCommandContext final {
    void* const volatile* const renderer_00;
    const volatile std::uint32_t* const renderer_profile_04;
    const NativeRendererCameraPrepareContext* const prepare_08;
    const NativeRendererViewportClearContext* const viewport_clear_0c;
    NativeCameraFrameCommandContext(void* const volatile& actual_00f8d394,
        const volatile std::uint32_t* actual_00d5f0a8,
        const NativeRendererCameraPrepareContext& prepare,
        const NativeRendererViewportClearContext& viewport_clear) noexcept
        : renderer_00(&actual_00f8d394), renderer_profile_04(actual_00d5f0a8),
          prepare_08(&prepare), viewport_clear_0c(&viewport_clear) {}
};
static_assert(offsetof(NativeCameraFrameCommandContext, renderer_00) == 0);
static_assert(offsetof(NativeCameraFrameCommandContext, renderer_profile_04) == 4);
static_assert(offsetof(NativeCameraFrameCommandContext, prepare_08) == 8);
static_assert(offsetof(NativeCameraFrameCommandContext, viewport_clear_0c) == 12);
static_assert(sizeof(NativeCameraFrameCommandContext) == 16);

// Complete B71360..B713C5: original ECX actual camera / RET, no semantic return.
// New EDX context; raw camera node links/fields, borrowed viewport identity,
// current renderer global, and D5F0A8 profile must be valid for every dispatch.
// Supported slots: +A0 B285A0, +A4 B26770, +08 B21430. An out-of-domain profile
// traps; it is never routed to a callback or silently given a default result.
// Enabled17C is read once. The three renderer/profile reads and camera field
// reads retain their native order. FLD18C/FSTP of the outgoing depth argument
// occurs even with flags188=0. No state projection, ownership operation, outer
// guard, exception handler, HRESULT policy, or FP environment reset is added.
void __fastcall execute_native_camera_frame_command_00b71360(void* actual_camera,
    const NativeCameraFrameCommandContext*);
} // namespace bsp
