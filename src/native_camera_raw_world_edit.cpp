#include "bsp/native_camera_raw_world_edit.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_node_raw_transform.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera world editing requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(NativeCameraDeriveLocalFrame) == 4);
static_assert(sizeof(NativeCameraDirectionFrame) == 4);
static_assert(offsetof(NativeCameraDirectionFrame, words) == 0);
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* ptr(Word p) noexcept { return reinterpret_cast<void*>(p); }
void* at(Word p, Word offset) noexcept { return ptr(p + offset); }
Word read(Word p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(p + offset);
}
void write(Word p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(p + offset) = value;
}
void* buffer(volatile Word* p) noexcept { return const_cast<Word*>(p); }

void set_world_captured(void* actual, Word source,
    const NativeCameraDeriveLocalFrame& frame, NativeNodeBaseWorldDispatch& calls) {
    const Word node = bits(actual);
    copy_native_camera_matrix_004134f0(at(node, 0xf0), nullptr, ptr(source)); // B6E87E
    const Word attachment = read(node, 0xa0); // after matrix stores
    if (attachment != 0) {
        const Word profile = read(attachment);
        const Word target = calls.resolve_profile(profile)[0x3c / 4];
        calls.invoke_virtual3c(target, ptr(attachment)); // B6E892
    }
    derive_native_camera_local_00b6e7e0(actual, frame); // B6E896
    invalidate_raw_descendants_00b6da30(actual); // B6E89D
    const Word profile = read(node);
    const Word target = calls.resolve_profile(profile)[0x40 / 4];
    calls.invoke_virtual40(target, actual); // B6E8A9
    write(node, 0x5c, 2); // after current virtual40 returns
}
} // namespace

void derive_native_camera_local_00b6e7e0(void* actual,
    const NativeCameraDeriveLocalFrame& frame) {
    const Word node = bits(actual);
    const Word parent = read(node, 0x30); // one captured parent
    if (parent != 0) {
        const Word flags = read(parent, 0x5c);
        if (!(flags & 8u)) {
            if (!(flags & 2u)) refresh_native_camera_world_00b6db70(ptr(parent)); // B6E7FE
            void* inverse = invert_native_camera_scaled_affine_00b63b30(
                buffer(frame.words), at(parent, 0xf0)); // B6E80D
            copy_native_camera_matrix_004134f0(at(parent, 0x60), nullptr, inverse); // B6E816
            write(parent, 0x5c, read(parent, 0x5c) | 8u);
        }
        void* local = multiply_native_camera_matrices_00413920(at(node, 0xf0),
            nullptr, buffer(frame.words + 16), at(parent, 0x60)); // B6E82E
        copy_native_camera_matrix_004134f0(at(node, 0xb0), nullptr, local); // B6E83A
    } else {
        copy_native_camera_matrix_004134f0(at(node, 0xb0), nullptr, at(node, 0xf0)); // B6E855
    }
}

void set_native_raw_world_matrix_00b6e870(void* actual,
    const volatile Word& source_argument, const NativeCameraDeriveLocalFrame& frame,
    NativeNodeBaseWorldDispatch& calls) {
    const Word source = source_argument; // B6E870
    set_world_captured(actual, source, frame, calls);
}

__declspec(naked) void __fastcall refresh_native_raw_camera_direction_00b70660(
    void*, const NativeCameraDirectionFrame&) noexcept {
    __asm {
        push esi
        push edi
        mov esi, ecx
        mov edi, dword ptr [edx] // pure disjoint frame metadata
        test byte ptr [esi + 0x5c], 2 // B70666
        jnz direction_ready
        call refresh_native_camera_world_00b6db70 // B7066C
direction_ready:
        fld dword ptr [esi + 0x110]
        fstp dword ptr [esi + 0x1ac]
        fld dword ptr [esi + 0x114]
        fstp dword ptr [esi + 0x1b0]
        fld dword ptr [esi + 0x118]
        fstp dword ptr [esi + 0x1b4]
        test byte ptr [esi + 0x5c], 2 // B70695 fresh test
        jnz position_ready
        mov ecx, esi
        call refresh_native_camera_world_00b6db70 // B7069D
position_ready:
        fld dword ptr [esi + 0x120]
        fstp dword ptr [edi]
        fld dword ptr [esi + 0x124]
        fstp dword ptr [edi + 4]
        fld dword ptr [esi + 0x128]
        fstp dword ptr [edi + 8]
        fld dword ptr [edi]
        fadd dword ptr [esi + 0x1ac]
        fstp dword ptr [edi + 0x0c]
        fld dword ptr [esi + 0x1b0]
        fadd dword ptr [edi + 4]
        fstp dword ptr [edi + 0x10]
        fld dword ptr [esi + 0x1b4]
        fadd dword ptr [edi + 8]
        fstp dword ptr [edi + 0x14]
        fld dword ptr [edi + 0x0c]
        fstp dword ptr [esi + 0x1a0]
        fld dword ptr [edi + 0x10]
        fstp dword ptr [esi + 0x1a4]
        fld dword ptr [edi + 0x14]
        fstp dword ptr [esi + 0x1a8]
        pop edi
        pop esi
        ret
    }
}

void set_native_raw_camera_world_00b71460(void* actual,
    const volatile Word& source_argument, const NativeCameraRawWorldEditFrame& frame,
    NativeNodeBaseWorldDispatch& calls) {
    const Word source = source_argument; // B71460 before flag mutation
    const Word camera = bits(actual);
    write(camera, 0x2f0, read(camera, 0x2f0) & 0xfffffe4bu);
    set_world_captured(actual, source, frame.derive, calls); // B71472
    refresh_native_raw_camera_direction_00b70660(actual, frame.direction); // B71479
}
} // namespace bsp
