#include "bsp/native_skin_model_bindings.hpp"
#include "bsp/native_camera_general_inverse.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native skin binding storage requires MSVC Win32 x87/SSE.
#endif

namespace bsp {
namespace {
using W = std::uint32_t;
using I = std::int32_t;
const W skin_negative_zero_d7a208 = 0x80000000u;
const W skin_positive_one_d7a24c = 0x3f800000u;
void* at(const void* p, W n = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<W>(p) + n);
}
template<class T = W> T read(const void* p, W n = 0) noexcept {
    return *static_cast<const volatile T*>(at(p, n));
}
template<class T> void write(void* p, W n, T value) noexcept {
    *static_cast<volatile T*>(at(p, n)) = value;
}
void copy_x87(void* destination, const void* source, W count) noexcept {
    for (W i = 0; i != count; ++i) {
        void* const d = at(destination, i * 4u);
        const void* const s = at(source, i * 4u);
        __asm { mov eax, s }
        __asm { fld dword ptr [eax] }
        __asm { mov eax, d }
        __asm { fstp dword ptr [eax] }
    }
}
void copy_matrix_words(void* destination, const void* source) noexcept {
    __asm { mov edi, destination }
    __asm { mov esi, source }
    __asm { mov ecx, 16 }
    __asm { rep movsd }
}
void identity(void* matrix) noexcept {
    for (W i = 0; i != 16; ++i)
        write(matrix, i * 4u, (i % 5u == 0) ? skin_positive_one_d7a24c : W{0});
}
void release_node(void* node, NativeSkinNodeLifetime& lifetime) {
    if (InterlockedDecrement(static_cast<volatile LONG*>(at(node, 4))) == 0) {
        const W target = lifetime.table(read(node))[0];
        lifetime.destroy(target, node);
    }
}
} // namespace

void reserve_native_skin_bindings_00b90ea0(void* header, I capacity,
    NativeSkinNodeLifetime& lifetime) {
    const I requested = capacity < 1 ? 1 : capacity;
    if (read<I>(header, 8) >= requested) return;
    const W bytes = static_cast<W>(requested) * 0x60u;
    void* const data = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes});
    for (W index = 0; static_cast<I>(index) < read<I>(header, 4); ++index) {
        void* const destination = at(data, index * 0x60u);
        if (destination) {
            const void* const source = at(read<void*>(header), index * 0x60u);
            write(destination, 0, read(source));
            copy_x87(at(destination, 4), at(source, 4), 7);
            copy_matrix_words(at(destination, 0x20), at(source, 0x20));
        }
    }
    for (W index = 0; static_cast<I>(index) < read<I>(header, 4); ++index) {
        void* const record = at(read<void*>(header), index * 0x60u);
        if (void* const node = read<void*>(record)) {
            release_node(node, lifetime);
            write(record, 0, W{0});
        }
    }
    singleton_lifetime_free(read<void*>(header));
    write(header, 0, data); write(header, 8, requested);
}
void resize_native_skin_bindings_00b91460(void* header, I count,
    NativeSkinNodeLifetime& lifetime) {
    if (count > read<I>(header, 8)) reserve_native_skin_bindings_00b90ea0(header, count, lifetime);
    const W old_count = read(header, 4);
    if (static_cast<I>(old_count) < count) {
        W offset = old_count * 0x60u;
        W remaining = static_cast<W>(count) - old_count;
        do {
            void* const record = at(read<void*>(header), offset);
            if (record) {
                for (W field = 4; field != 0x1c; field += 4) write(record, field, W{0});
                write(record, 0, W{0}); identity(at(record, 0x20));
            }
            offset += 0x60u;
        } while (--remaining);
    }
    while (count < read<I>(header, 4)) {
        write(header, 4, read(header, 4) - 1u);
        void* const record = at(read<void*>(header), read(header, 4) * 0x60u);
        if (void* const node = read<void*>(record)) {
            release_node(node, lifetime);
            write(record, 0, W{0});
        }
    }
    write(header, 4, count);
}
void resize_native_skin_model_bindings_00b91590(void* model, I count,
    NativeSkinNodeLifetime& lifetime) {
    resize_native_skin_bindings_00b91460(at(model, 0x184), count, lifetime);
}
void bind_native_skin_node_00b90c30(void* model, W index, void* node,
    const void* translation, const void* angles, W scalar, NativeSkinNodeLifetime& lifetime) {
    void* record = at(read<void*>(model, 0x184), index * 0x60u);
    void* const old = read<void*>(record);
    if (old != node) {
        write(record, 0, node);
        if (node) InterlockedIncrement(static_cast<volatile LONG*>(at(node, 4)));
        if (old) release_node(old, lifetime);
    }
    record = at(read<void*>(model, 0x184), index * 0x60u);
    copy_x87(at(record, 4), translation, 3);
    copy_x87(at(record, 0x10), angles, 3);
    write(record, 0x1c, scalar);
}

__declspec(naked) void* __fastcall build_native_skin_inverse_pose_00b64db0(void*, const void*, const void*, const void*) {
    __asm {
        sub esp, 0x11c // 00b64db0
        mov eax, dword ptr [esp + 0x120] // 00b64db6
        movss xmm0, dword ptr [eax] // 00b64dbd
        push esi // 00b64dc1
        push edi // 00b64dc2
        mov esi, edx // 00b64dc3
        mov edi, ecx // 00b64dc5
        movss dword ptr [esp + 8], xmm0 // 00b64dc7
        fld dword ptr [esp + 8] // 00b64dcd
        fsin  // 00b64dd1
        fstp dword ptr [esp + 0xc] // 00b64dd3
        movss xmm0, dword ptr [eax] // 00b64dd7
        movss xmm3, dword ptr [esp + 0xc] // 00b64ddb
        movss dword ptr [esp + 8], xmm0 // 00b64de1
        fld dword ptr [esp + 8] // 00b64de7
        fcos  // 00b64deb
        fstp dword ptr [esp + 0x10] // 00b64ded
        xorps xmm0, xmm0 // 00b64df1
        movss xmm2, dword ptr [skin_negative_zero_d7a208] // 00b64df4
        movss xmm4, dword ptr [esp + 0x10] // 00b64dfc
        movaps xmm1, xmm2 // 00b64e02
        subss xmm1, xmm3 // 00b64e05
        movss dword ptr [esp + 0xac], xmm1 // 00b64e09
        movss xmm1, dword ptr [skin_positive_one_d7a24c] // 00b64e12
        movss dword ptr [esp + 0xc4], xmm3 // 00b64e1a
        movss xmm3, dword ptr [eax + 4] // 00b64e23
        movss dword ptr [esp + 0xa4], xmm4 // 00b64e28
        movss dword ptr [esp + 0xa8], xmm0 // 00b64e31
        movss dword ptr [esp + 0xb0], xmm0 // 00b64e3a
        movss dword ptr [esp + 0xb4], xmm0 // 00b64e43
        movss dword ptr [esp + 0xb8], xmm1 // 00b64e4c
        movss dword ptr [esp + 0xbc], xmm0 // 00b64e55
        movss dword ptr [esp + 0xc0], xmm0 // 00b64e5e
        movss dword ptr [esp + 0xc8], xmm0 // 00b64e67
        movss dword ptr [esp + 0xcc], xmm4 // 00b64e70
        movss dword ptr [esp + 0xd0], xmm0 // 00b64e79
        movss dword ptr [esp + 0xd4], xmm0 // 00b64e82
        movss dword ptr [esp + 0xd8], xmm0 // 00b64e8b
        movss dword ptr [esp + 0xdc], xmm0 // 00b64e94
        movss dword ptr [esp + 0xe0], xmm1 // 00b64e9d
        movss dword ptr [esp + 8], xmm3 // 00b64ea6
        fld dword ptr [esp + 8] // 00b64eac
        fsin  // 00b64eb0
        fstp dword ptr [esp + 0x14] // 00b64eb2
        movss xmm4, dword ptr [eax + 4] // 00b64eb6
        movss xmm3, dword ptr [esp + 0x14] // 00b64ebb
        movss dword ptr [esp + 8], xmm4 // 00b64ec1
        fld dword ptr [esp + 8] // 00b64ec7
        fcos  // 00b64ecb
        fstp dword ptr [esp + 0x18] // 00b64ecd
        movss xmm4, dword ptr [esp + 0x18] // 00b64ed1
        movaps xmm5, xmm2 // 00b64ed7
        subss xmm5, xmm3 // 00b64eda
        movss dword ptr [esp + 0x7c], xmm3 // 00b64ede
        movss xmm3, dword ptr [eax + 8] // 00b64ee4
        movss dword ptr [esp + 0x64], xmm1 // 00b64ee9
        movss dword ptr [esp + 0x68], xmm0 // 00b64eef
        movss dword ptr [esp + 0x6c], xmm0 // 00b64ef5
        movss dword ptr [esp + 0x70], xmm0 // 00b64efb
        movss dword ptr [esp + 0x74], xmm0 // 00b64f01
        movss dword ptr [esp + 0x78], xmm4 // 00b64f07
        movss dword ptr [esp + 0x80], xmm0 // 00b64f0d
        movss dword ptr [esp + 0x84], xmm0 // 00b64f16
        movss dword ptr [esp + 0x88], xmm5 // 00b64f1f
        movss dword ptr [esp + 0x8c], xmm4 // 00b64f28
        movss dword ptr [esp + 0x90], xmm0 // 00b64f31
        movss dword ptr [esp + 0x94], xmm0 // 00b64f3a
        movss dword ptr [esp + 0x98], xmm0 // 00b64f43
        movss dword ptr [esp + 0x9c], xmm0 // 00b64f4c
        movss dword ptr [esp + 0xa0], xmm1 // 00b64f55
        movss dword ptr [esp + 8], xmm3 // 00b64f5e
        fld dword ptr [esp + 8] // 00b64f64
        fsin  // 00b64f68
        fstp dword ptr [esp + 0x1c] // 00b64f6a
        movss xmm4, dword ptr [eax + 8] // 00b64f6e
        movss xmm3, dword ptr [esp + 0x1c] // 00b64f73
        movss dword ptr [esp + 0x20], xmm4 // 00b64f79
        fld dword ptr [esp + 0x20] // 00b64f7f
        fcos  // 00b64f83
        fstp dword ptr [esp + 8] // 00b64f85
        movss xmm4, dword ptr [esp + 8] // 00b64f89
        subss xmm2, xmm3 // 00b64f8f
        movss dword ptr [esp + 0x24], xmm4 // 00b64f93
        movss dword ptr [esp + 0x28], xmm3 // 00b64f99
        movss dword ptr [esp + 0x2c], xmm0 // 00b64f9f
        movss dword ptr [esp + 0x30], xmm0 // 00b64fa5
        movss dword ptr [esp + 0x34], xmm2 // 00b64fab
        movss dword ptr [esp + 0x38], xmm4 // 00b64fb1
        movss dword ptr [esp + 0x3c], xmm0 // 00b64fb7
        movss dword ptr [esp + 0x40], xmm0 // 00b64fbd
        movss dword ptr [esp + 0x44], xmm0 // 00b64fc3
        movss dword ptr [esp + 0x48], xmm0 // 00b64fc9
        movss dword ptr [esp + 0x4c], xmm1 // 00b64fcf
        movss dword ptr [esp + 0x50], xmm0 // 00b64fd5
        lea eax, [esp + 0xa4] // 00b64fdb
        push eax // 00b64fe2
        lea ecx, [esp + 0x28] // 00b64fe3
        push ecx // 00b64fe7
        lea edx, [esp + 0x6c] // 00b64fe8
        push edx // 00b64fec
        lea eax, [esp + 0xf0] // 00b64fed
        push eax // 00b64ff4
        lea ecx, [esp + 0x34] // 00b64ff5
        movss dword ptr [esp + 0x64], xmm0 // 00b64ff9
        movss dword ptr [esp + 0x68], xmm0 // 00b64fff
        movss dword ptr [esp + 0x6c], xmm0 // 00b65005
        movss dword ptr [esp + 0x70], xmm1 // 00b6500b
        call multiply_native_camera_matrices_00413920 // 00b65011
        mov ecx, eax // 00b65016
        call multiply_native_camera_matrices_00413920 // 00b65018
        movss xmm0, dword ptr [esi] // 00b6501d
        movss dword ptr [esp + 0x54], xmm0 // 00b65021
        movss xmm0, dword ptr [esi + 4] // 00b65027
        movss dword ptr [esp + 0x58], xmm0 // 00b6502c
        movss xmm0, dword ptr [esi + 8] // 00b65032
        lea edx, [esp + 0x24] // 00b65037
        mov ecx, edi // 00b6503b
        movss dword ptr [esp + 0x5c], xmm0 // 00b6503d
        call invert_native_camera_matrix_00b632d0 // 00b65043
        mov eax, edi // 00b65048
        pop edi // 00b6504a
        pop esi // 00b6504b
        add esp, 0x11c // 00b6504c
        ret 8 // 00b65052
    }
}

__declspec(naked) void __fastcall set_native_animator_translation_00b75d80(void*, void*, const void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4] // 00b75d80
        fld dword ptr [eax] // 00b75d84
        fstp dword ptr [ecx + 8] // 00b75d86
        fld dword ptr [eax + 4] // 00b75d89
        fstp dword ptr [ecx + 0xc] // 00b75d8c
        fld dword ptr [eax + 8] // 00b75d8f
        fstp dword ptr [ecx + 0x10] // 00b75d92
        ret 4 // 00b75d95
    }
}

__declspec(naked) void __fastcall set_native_animator_angles_00b76570(void*, void*, const void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4] // 00b76570
        fld dword ptr [eax] // 00b76574
        fstp dword ptr [ecx + 0x14] // 00b76576
        fld dword ptr [eax + 4] // 00b76579
        fstp dword ptr [ecx + 0x18] // 00b7657c
        fld dword ptr [eax + 8] // 00b7657f
        fstp dword ptr [ecx + 0x1c] // 00b76582
        ret 4 // 00b76585
    }
}

void __fastcall build_native_skin_descendant_matrices_00b90cc0(void* model,
    void*, void* parent, W parent_index) {
    void* child = read<void*>(parent, 0x34);
    while (child) {
        const I count = read<I>(model, 0x188);
        if (count > 0) {
            void* const backing = read<void*>(model, 0x184);
            W index = 0;
            while (static_cast<I>(index) < count && read<void*>(at(backing, index * 0x60u)) != child) ++index;
            if (static_cast<I>(index) < count) {
                void* const record = at(backing, index * 0x60u);
                const void* const parent_matrix = at(backing, parent_index * 0x60u + 0x20u);
                const W one[3] = {skin_positive_one_d7a24c, skin_positive_one_d7a24c, skin_positive_one_d7a24c};
                W inverse[16], product[16];
                build_native_skin_inverse_pose_00b64db0(inverse, at(record, 4), at(record, 0x10), one);
                multiply_native_camera_matrices_00413920(parent_matrix, nullptr, product, inverse);
                copy_x87(at(record, 0x20), product, 16);
                build_native_skin_descendant_matrices_00b90cc0(model, nullptr, child, index);
            }
        }
        child = read<void*>(child, 0x3c);
    }
}
void __fastcall finalize_native_skin_bindings_00b91000(void* model) {
    for (W index = 0; static_cast<I>(index) < read<I>(model, 0x188); ++index) {
        void* const record = at(read<void*>(model, 0x184), index * 0x60u);
        if (void* const node = read<void*>(record)) {
            set_native_animator_translation_00b75d80(read<void*>(node, 0x130), nullptr, at(record, 4));
            const void* const angles = at(read<void*>(model, 0x184), index * 0x60u + 0x10u);
            set_native_animator_angles_00b76570(read<void*>(node, 0x130), nullptr, angles);
        }
    }
    for (W index = 0; static_cast<I>(index) < read<I>(model, 0x188); ++index) {
        void* const record = at(read<void*>(model, 0x184), index * 0x60u);
        void* const node = read<void*>(record);
        if (node && read<void*>(node, 0x30) == model) {
            W base[16], inverse[16], product[16]; identity(base);
            const W one[3] = {skin_positive_one_d7a24c, skin_positive_one_d7a24c, skin_positive_one_d7a24c};
            build_native_skin_inverse_pose_00b64db0(inverse, at(record, 4), at(record, 0x10), one);
            multiply_native_camera_matrices_00413920(base, nullptr, product, inverse);
            copy_x87(at(record, 0x20), product, 16);
            build_native_skin_descendant_matrices_00b90cc0(model, nullptr, node, index);
        }
    }
}
} // namespace bsp
