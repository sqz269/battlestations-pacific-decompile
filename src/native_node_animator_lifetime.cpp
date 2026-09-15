#include "bsp/native_node_animator_lifetime.hpp"
#include "bsp/native_animation_registry.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native node animator lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using W = std::uint32_t;
void* at(const void* p, W offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<W>(p) + offset);
}
template<class T = W> T read(const void* p, W offset = 0) noexcept {
    return *static_cast<const volatile T*>(at(p, offset));
}
template<class T> void write(void* p, W offset, T value) noexcept {
    *static_cast<volatile T*>(at(p, offset)) = value;
}
void zero_base_fields(void* animator) noexcept {
    write(animator, 0x20, W{0}); write(animator, 0x24, W{0});
    write(animator, 0x28, W{0}); write(animator, 0x2c, W{0});
    for (W offset = 8; offset != 0x20; offset += 4) write(animator, offset, W{0});
}
// Preserve the original ECXheader/stackcount/RET4 ABI at the arithmetic call
// sites while reusing the complete AU source over the same actual header.
void __fastcall resize_sample_array(void* header, void*, std::int32_t count) {
    resize_native_float_array_00818030(header, count);
}
} // namespace

void initialize_native_track_animator_fragment_00b7a0ba(void* animator) {
    write(animator, 0, W{0x00ceb130}); write(animator, 4, W{1});
    write(animator, 0, W{0x00d62e60}); zero_base_fields(animator);
}
void initialize_native_compact_animator_fragment_00b79d44(void* animator,
    const void* compatible_skin_node) {
    write(animator, 0, W{0x00ceb130}); write(animator, 4, W{1});
    zero_base_fields(animator); write(animator, 0, W{0x00d62eb0});
    write(animator, 0x30, std::uint8_t(compatible_skin_node == nullptr));
    write(animator, 0x34, W{0});
}
void destroy_native_node_animator_00b780d0(void* animator,
    NativeResourceAnimatorLifetime& lifetime) {
    write(animator, 0, W{0x00d62e60});
    if (void* const registry = read<void*>(animator, 0x2c)) {
        if (InterlockedDecrement(static_cast<volatile LONG*>(at(registry, 4))) == 0) {
            const W target = lifetime.table(read(registry))[0];
            lifetime.destroy(target, registry);
        }
        write(animator, 0x2c, W{0});
    }
    auto& tracks = *static_cast<NativeRenderPointerArrayStorage*>(at(animator, 0x20));
    resize_native_instance_entry_pointers_00b1c770(tracks, 0);
    singleton_lifetime_free(read<void*>(&tracks));
    write(animator, 0, W{0x00d5c104});
    destroy_native_ref_counted_base_00bd30f0(animator);
}
void* delete_native_track_animator_00b786e0(void* animator, W flags,
    NativeResourceAnimatorLifetime& lifetime) {
    destroy_native_node_animator_00b780d0(animator, lifetime);
    if (flags & 1u) singleton_lifetime_free(animator);
    return animator;
}
void* delete_native_compact_animator_00b78720(void* animator, W flags,
    NativeResourceAnimatorLifetime& lifetime) {
    write(animator, 0, W{0x00d62eb0});
    destroy_native_node_animator_00b780d0(animator, lifetime);
    if (flags & 1u) singleton_lifetime_free(animator);
    return animator;
}

__declspec(naked) void __fastcall include_native_animation_sample_00b778c0(void*, void*, std::int32_t, std::uint32_t) {
    __asm {
        push esi // 00b778c0
        push edi // 00b778c1
        mov edi, dword ptr [esp + 0xc] // 00b778c2
        mov esi, ecx // 00b778c6
        cmp dword ptr [esi + 0x18], edi // 00b778c8
        jg short native_00b778d9 // 00b778cb
        lea eax, [edi + 1] // 00b778cd
        push eax // 00b778d0
        lea ecx, [esi + 0x14] // 00b778d1
        call resize_sample_array // 00b778d4
    native_00b778d9:
        mov ecx, dword ptr [esi + 0x14] // 00b778d9
        fld dword ptr [ecx + edi*4] // 00b778dc
        lea ecx, [ecx + edi*4] // 00b778df
        fstp dword ptr [esp + 0xc] // 00b778e2
        pop edi // 00b778e6
        fld dword ptr [esp + 0xc] // 00b778e7
        pop esi // 00b778eb
        fld dword ptr [esp + 4] // 00b778ec
        fcomip st, st(1) // 00b778f0
        fstp st(0) // 00b778f2
        jbe short native_00b77903 // 00b778f4
        movss xmm0, dword ptr [esp + 4] // 00b778f6
        movss dword ptr [ecx], xmm0 // 00b778fc
        ret 8 // 00b77900
    native_00b77903:
        movss xmm0, dword ptr [esp + 8] // 00b77903
        movss dword ptr [ecx], xmm0 // 00b77909
        ret 8 // 00b7790d
    }
}

__declspec(naked) void __fastcall finalize_native_track_animator_00b77990(void*) {
    __asm {
        sub esp, 8 // 00b77990
        push ebx // 00b77993
        push esi // 00b77994
        mov ebx, ecx // 00b77995
        xor esi, esi // 00b77997
        cmp dword ptr [ebx + 0x24], esi // 00b77999
        jle short native_00b77a03 // 00b7799c
        push edi // 00b7799e
        nop  // 00b7799f
    native_00b779a0:
        mov eax, dword ptr [ebx + 0x20] // 00b779a0
        cmp dword ptr [eax + esi*4], 0 // 00b779a3
        lea eax, [eax + esi*4] // 00b779a7
        je short native_00b779fa // 00b779aa
        mov ecx, dword ptr [eax] // 00b779ac
        mov edi, dword ptr [ebx + 0x2c] // 00b779ae
        cmp dword ptr [edi + 0x18], esi // 00b779b1
        movss xmm0, dword ptr [ecx + 0x1c] // 00b779b4
        movss dword ptr [esp + 0x10], xmm0 // 00b779b9
        jg short native_00b779cd // 00b779bf
        lea edx, [esi + 1] // 00b779c1
        push edx // 00b779c4
        lea ecx, [edi + 0x14] // 00b779c5
        call resize_sample_array // 00b779c8
    native_00b779cd:
        mov eax, dword ptr [edi + 0x14] // 00b779cd
        fld dword ptr [eax + esi*4] // 00b779d0
        lea ecx, [eax + esi*4] // 00b779d3
        fstp dword ptr [esp + 0xc] // 00b779d6
        fld dword ptr [esp + 0x10] // 00b779da
        fld dword ptr [esp + 0xc] // 00b779de
        fcomip st, st(1) // 00b779e2
        fstp st(0) // 00b779e4
        jbe short native_00b779f0 // 00b779e6
        movss xmm0, dword ptr [esp + 0xc] // 00b779e8
        jmp short native_00b779f6 // 00b779ee
    native_00b779f0:
        movss xmm0, dword ptr [esp + 0x10] // 00b779f0
    native_00b779f6:
        movss dword ptr [ecx], xmm0 // 00b779f6
    native_00b779fa:
        add esi, 1 // 00b779fa
        cmp esi, dword ptr [ebx + 0x24] // 00b779fd
        jl short native_00b779a0 // 00b77a00
        pop edi // 00b77a02
    native_00b77a03:
        pop esi // 00b77a03
        pop ebx // 00b77a04
        add esp, 8 // 00b77a05
        ret  // 00b77a08
    }
}

__declspec(naked) void __fastcall include_native_compact_track_samples_00b92610(void*, void*, void*) {
    __asm {
        push esi // 00b92610
        push edi // 00b92611
        mov edi, ecx // 00b92612
        xor esi, esi // 00b92614
        cmp dword ptr [edi + 0x10], esi // 00b92616
        jle short native_00b92642 // 00b92619
        push ebx // 00b9261b
        push ebp // 00b9261c
        mov ebp, dword ptr [esp + 0x14] // 00b9261d
        xor ebx, ebx // 00b92621
    native_00b92623:
        mov eax, dword ptr [edi + 0xc] // 00b92623
        fld dword ptr [eax + ebx] // 00b92626
        push ecx // 00b92629
        fstp dword ptr [esp] // 00b9262a
        push esi // 00b9262d
        mov ecx, ebp // 00b9262e
        call include_native_animation_sample_00b778c0 // 00b92630
        add esi, 1 // 00b92635
        add ebx, 0x18 // 00b92638
        cmp esi, dword ptr [edi + 0x10] // 00b9263b
        jl short native_00b92623 // 00b9263e
        pop ebp // 00b92640
        pop ebx // 00b92641
    native_00b92642:
        pop edi // 00b92642
        pop esi // 00b92643
        ret 4 // 00b92644
    }
}

__declspec(naked) void __fastcall finalize_native_compact_animator_00b75ee0(void*) {
    __asm {
        mov eax, ecx // 00b75ee0
        mov ecx, dword ptr [eax + 0x34] // 00b75ee2
        test ecx, ecx // 00b75ee5
        je short native_00b75ef2 // 00b75ee7
        mov eax, dword ptr [eax + 0x2c] // 00b75ee9
        push eax // 00b75eec
        call include_native_compact_track_samples_00b92610 // 00b75eed
    native_00b75ef2:
        ret  // 00b75ef2
    }
}
} // namespace bsp
