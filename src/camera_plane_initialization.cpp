#include "bsp/camera_plane_initialization.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera plane initialization requires MSVC Win32.
#endif

namespace bsp {

static_assert(sizeof(CameraPlaneRecord) == 20);
static_assert(offsetof(CameraPlaneRecord, flags) == 16);
static_assert(sizeof(CameraPlaneSet) == 0x144);
static_assert(offsetof(CameraPlaneSet, count) == 0x140);

namespace {
// Same sixteen-record MOVSS/MOV schedule as 00B652D0. The second register
// argument supplies the original absolute constant address through a binding.
__declspec(naked) CameraPlaneSet* __fastcall initialize_records_kernel(
    CameraPlaneSet*, const volatile std::uint32_t*) {
    __asm {
        xorps xmm0, xmm0
        movss xmm1, dword ptr [edx]
        mov eax, ecx
        push esi
        mov esi, 15
        lea edx, [eax+8]
    next_record:
        movss dword ptr [edx-8], xmm0
        movss dword ptr [edx-4], xmm1
        movss dword ptr [edx], xmm0
        movss dword ptr [edx+4], xmm0
        mov dword ptr [edx+8], 1
        add edx, 20
        sub esi, 1
        jns next_record
        pop esi
        ret
    }
}
} // namespace

CameraPlaneSet* initialize_camera_plane_records_00b652d0(CameraPlaneSet& destination,
    const volatile std::uint32_t& live_one_00d7a24c) noexcept {
    return initialize_records_kernel(&destination, &live_one_00d7a24c);
}

CameraPlaneSet* construct_camera_plane_set_00b659d0(CameraPlaneSet& destination,
    const volatile std::uint32_t& live_one_00d7a24c) {
    initialize_camera_plane_records_00b652d0(destination, live_one_00d7a24c);
    // A second actual load follows all sixteen record writes and precedes count.
    const std::uint32_t diagonal_bits = live_one_00d7a24c;
    static_cast<volatile std::uint32_t&>(destination.count) = 6;
    CameraMatrix matrix;
    auto* matrix_words = matrix.data();
    __asm {
        xorps xmm0, xmm0
        movss xmm1, dword ptr [diagonal_bits]
        mov eax, matrix_words
        movss dword ptr [eax], xmm1
        movss dword ptr [eax+4], xmm0
        movss dword ptr [eax+8], xmm0
        movss dword ptr [eax+12], xmm0
        movss dword ptr [eax+16], xmm0
        movss dword ptr [eax+20], xmm1
        movss dword ptr [eax+24], xmm0
        movss dword ptr [eax+28], xmm0
        movss dword ptr [eax+32], xmm0
        movss dword ptr [eax+36], xmm0
        movss dword ptr [eax+40], xmm1
        movss dword ptr [eax+44], xmm0
        movss dword ptr [eax+48], xmm0
        movss dword ptr [eax+52], xmm0
        movss dword ptr [eax+56], xmm0
        movss dword ptr [eax+60], xmm1
    }
    std::array<CameraPlane, 6> planes;
    extract_camera_frustum_00b653f0(planes, matrix);
    assign_camera_frustum_planes_00b658e0(destination, planes, 7u);
    return &destination;
}

} // namespace bsp
