#pragma once
#include "bsp/native_camera_frustum.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {
// Borrow actual shared runtime storage; every pointer is immutable after
// construction. Frustum's fixed complete CRT provider/runtime binding must
// already be valid. All referenced contexts and storage outlive each call.
struct NativeRendererCameraPrepareContext final {
    const NativeCameraFrustumContext* const frustum_00;
    NativeRendererSynchronizationGlobals* const synchronization_04;
    const volatile double* const ambient_scale_08;
    const volatile std::uint32_t* const conversion_mode_0c;
    NativeRendererCameraPrepareContext(const NativeCameraFrustumContext& frustum,
        NativeRendererSynchronizationGlobals& synchronization,
        const volatile double& actual_00ce4b48,
        const volatile std::uint32_t& actual_0109eea4) noexcept
        : frustum_00(&frustum), synchronization_04(&synchronization),
          ambient_scale_08(&actual_00ce4b48), conversion_mode_0c(&actual_0109eea4) {}
};
static_assert(offsetof(NativeRendererCameraPrepareContext, frustum_00) == 0);
static_assert(offsetof(NativeRendererCameraPrepareContext, synchronization_04) == 4);
static_assert(offsetof(NativeRendererCameraPrepareContext, ambient_scale_08) == 8);
static_assert(offsetof(NativeRendererCameraPrepareContext, conversion_mode_0c) == 12);
static_assert(sizeof(NativeRendererCameraPrepareContext) == 16);

// Full B285A0[544]: original ECX renderer, stack actual camera, RET4; no
// semantic return. EDX adds the concrete context in this new interface.
// Actual camera/parents must have ORIGINAL raw node links and camera fields.
// NativeNodeStorage's canonical prefix is valid; pass its actual raw address,
// never a CameraTransform companion. No camera, plane or callback snapshot.
// State98=0 runs before the camera argument load. All raw getter side effects,
// current plane/count/flags reads, clip/state calls, and ambient conversion
// retain native order. No outer synchronization/EH guard or HRESULT policy.
void __fastcall prepare_native_renderer_camera_00b285a0(void* actual_renderer,
    const NativeRendererCameraPrepareContext*, void* actual_camera);
} // namespace bsp
