#include "bsp/camera_clip_planes.hpp"
#include <cstddef>

namespace bsp {
static_assert(sizeof(CameraPlaneRecord) == 0x14);
static_assert(offsetof(CameraPlaneRecord, flags) == 0x10);
static_assert(offsetof(CameraPlaneSet, count) == 0x140);

void set_camera_plane_count_00b65070(CameraPlaneSet& planes,
    std::uint32_t count) noexcept {
    // The integer store accepts the complete native DWORD, including counts
    // outside the subsequent append API's valid-storage precondition.
    planes.count = count;
}

void append_camera_plane_00b65990(CameraPlaneSet& planes,
    const CameraPlane& source, std::uint32_t flags) noexcept {
    auto* storage = &planes;
    const auto* input = source.data();
    __asm {
        mov ecx, storage
        mov eax, dword ptr [ecx+140h]
        lea edx, [eax+1]
        mov dword ptr [ecx+140h], edx
        lea eax, [eax+eax*4]
        lea eax, [ecx+eax*4]
        mov ecx, input
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx+4]
        fstp dword ptr [eax+4]
        fld dword ptr [ecx+8]
        fstp dword ptr [eax+8]
        fld dword ptr [ecx+12]
        mov ecx, flags
        fstp dword ptr [eax+12]
        mov dword ptr [eax+16], ecx
    }
}

void set_camera_extra_clip_plane_00b70410(CameraClipPlaneState& state,
    const CameraPlane& source) noexcept {
    const auto* input = source.data();
    auto* destination = state.extra_plane_2e0.data();
    __asm {
        mov eax, input
        mov ecx, destination
        fld dword ptr [eax]
        fstp dword ptr [ecx]
        fld dword ptr [eax+4]
        fstp dword ptr [ecx+4]
        fld dword ptr [eax+8]
        fstp dword ptr [ecx+8]
        fld dword ptr [eax+12]
        fstp dword ptr [ecx+12]
    }
    set_camera_plane_count_00b65070(state.frame.frustum, 6u);
    append_camera_plane_00b65990(state.frame.frustum, source, 3u);
    state.frame.camera.projection.valid_flags |= 0x40u;
}

void clear_camera_extra_clip_plane_00b70470(CameraClipPlaneState& state) noexcept {
    set_camera_plane_count_00b65070(state.frame.frustum, 6u);
    state.frame.camera.projection.valid_flags &= 0xffffffbfu;
}
}
