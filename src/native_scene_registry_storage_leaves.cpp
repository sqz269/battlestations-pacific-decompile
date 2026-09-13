#include "bsp/native_scene_registry_storage_leaves.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene registry storage leaves require MSVC Win32.
#endif

namespace bsp {

__declspec(naked) void* __fastcall allocate_native_scene_registry_sentinel_00b82390(
    NativeSceneRegistryAllocator const volatile&) {
    __asm {
        push 0ch // 00b82390
        call dword ptr [ecx] // 00b82392: current actual BF681B entry
        add esp, 4 // 00b82397
        test eax, eax // 00b8239a
        jz link_four // 00b8239c
        mov dword ptr [eax], eax // 00b8239e
    link_four:
        lea ecx, [eax + 4] // 00b823a0
        test ecx, ecx // 00b823a3
        jz done // 00b823a5
        mov dword ptr [ecx], eax // 00b823a7
    done:
        ret // 00b823a9
    }
}

__declspec(naked) void* __fastcall allocate_native_scene_registry_node_00b823b0(
    NativeSceneRegistryAllocator const volatile&, void*, std::uint32_t,
    std::uint32_t, const void*) {
    __asm {
        push 0ch // 00b823b0
        call dword ptr [ecx] // 00b823b2: current actual BF681B entry
        add esp, 4 // 00b823b7
        test eax, eax // 00b823ba
        jz link_four // 00b823bc
        mov ecx, dword ptr [esp + 4] // 00b823be
        mov dword ptr [eax], ecx // 00b823c2
    link_four:
        lea ecx, [eax + 4] // 00b823c4
        test ecx, ecx // 00b823c7
        jz key_eight // 00b823c9
        mov edx, dword ptr [esp + 8] // 00b823cb
        mov dword ptr [ecx], edx // 00b823cf
    key_eight:
        lea ecx, [eax + 8] // 00b823d1
        test ecx, ecx // 00b823d4
        jz done // 00b823d6
        mov edx, dword ptr [esp + 0ch] // 00b823d8
        mov edx, dword ptr [edx] // 00b823dc
        mov dword ptr [ecx], edx // 00b823de
    done:
        ret 0ch // 00b823e0
    }
}

__declspec(naked) void __fastcall fill_native_scene_registry_iterators_00b82570(
    void*, std::uint32_t, const void*, std::uint32_t, std::uint32_t,
    std::uint32_t) noexcept {
    __asm {
        test edx, edx // 00b82570
        jbe done // 00b82572
        mov eax, dword ptr [esp + 4] // 00b82574
        push esi // 00b82578
        lea esp, [esp] // 00b82579: alignment no-op
    loop_pair:
        test ecx, ecx // 00b82580
        jz advance // 00b82582
        mov esi, dword ptr [eax] // 00b82584
        mov dword ptr [ecx], esi // 00b82586
        mov esi, dword ptr [eax + 4] // 00b82588
        mov dword ptr [ecx + 4], esi // 00b8258b
    advance:
        sub edx, 1 // 00b8258e
        add ecx, 8 // 00b82591
        test edx, edx // 00b82594
        ja loop_pair // 00b82596
        pop esi // 00b82598
    done:
        ret 10h // 00b82599
    }
}
} // namespace bsp
