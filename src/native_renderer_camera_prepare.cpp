#include "bsp/native_renderer_camera_prepare.hpp"
#include "bsp/native_renderer_cached_states.hpp"
#include "bsp/native_renderer_clip_planes.hpp"
#include "bsp/native_camera_cache_getters.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_camera_general_inverse.hpp"
#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/native_ambient_color.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer camera preparation requires MSVC Win32 x87/SSE assembly.
#endif

namespace bsp {
namespace {
// Private ABI adapters only. No arithmetic, result, callback or state provider
// is substituted. Retain native stacked arguments and call complete providers.
__declspec(naked) void __fastcall state_abi(void*, const NativeRendererCameraPrepareContext*, std::uint32_t, std::uint32_t) {
    __asm {
        push dword ptr [edx + 4]
        push dword ptr [esp + 0ch]
        push dword ptr [esp + 0ch]
        push ecx
        call set_native_renderer_render_state_00b24460
        add esp, 10h
        ret 8
    }
}
__declspec(naked) void __fastcall clip_abi(void*, const NativeRendererCameraPrepareContext*, std::uint32_t, const void*) {
    __asm {
        push dword ptr [edx + 4]
        push dword ptr [esp + 0ch]
        push dword ptr [esp + 0ch]
        push ecx
        call set_native_renderer_clip_plane_00b23e50
        add esp, 10h
        ret 8
    }
}
__declspec(naked) void* __fastcall color_abi(void*, const NativeRendererCameraPrepareContext*, const void*) {
    __asm {
        push dword ptr [edx + 0ch]
        push dword ptr [edx + 8]
        mov edx, dword ptr [esp + 0ch]
        call convert_native_float_rgba_to_argb_004fb850
        ret 4
    }
}
__declspec(naked) void* __fastcall plane_abi(const void*, void*, void*, const void*) {
    __asm {
        mov edx, dword ptr [esp + 4]
        push dword ptr [esp + 8]
        call transform_native_plane_00b65ba0
        ret 8
    }
}
} // namespace

// Complete B285A0..B287BF. The original schedule is retained; only explicit
// integer context/ABI plumbing and corresponding local branch offsets differ.
// Pass canonical native storage; owning C++ companions have separate addresses.
__declspec(naked) void __fastcall prepare_native_renderer_camera_00b285a0(void*, const NativeRendererCameraPrepareContext*, void*) {
    __asm {
        push edx // immutable context outside the original scratch/save frame
        sub esp, 0114h // 00b285a0
        push ebx // 00b285a6
        push ebp // 00b285a7
        push esi // 00b285a8
        push edi // 00b285a9
        push 0 // 00b285aa
        push 098h // 00b285ac
        mov ebp, ecx // 00b285b1
        mov edx, dword ptr [esp + 12ch] // context under two original stacked args
        call state_abi // 00b285b3
        mov esi, dword ptr [esp + 012ch] // 00b285b8
        mov ecx, esi // 00b285bf
        call get_native_camera_inverse_view_projection_00b70510 // 00b285c1
        mov ecx, esi // 00b285c6
        mov edx, dword ptr [esp + 124h]
        mov edx, dword ptr [edx] // fixed frustum context
        call get_native_camera_frustum_00b70710 // 00b285c8
        mov edi, eax // 00b285cd
        lea ebx, [ebp + 017c0h] // 00b285cf
        push edi // 00b285d5
        mov ecx, ebx // 00b285d6
        call copy_native_plane_set_records_00b250b0 // 00b285d8
        mov eax, dword ptr [edi + 0140h] // 00b285dd
        mov dword ptr [ebx + 0140h], eax // 00b285e3
        xor edi, edi // 00b285e9
        cmp byte ptr [ebp + 01b51h], 0 // 00b285eb
        mov dword ptr [ebp + 019ech], edi // 00b285f2
        mov dword ptr [ebp + 019f0h], edi // 00b285f8
        je L_00b2878c // 00b285fe
        mov ecx, ebx // 00b28604
        mov dword ptr [esp + 010h], edi // 00b28606
        call native_plane_set_count_00b65080 // 00b2860a
        test eax, eax // 00b2860f
        jbe L_00b28763 // 00b28611
    L_00b28617:
        push edi // 00b28617
        mov ecx, ebx // 00b28618
        call native_plane_set_flags_00b65700 // 00b2861a
        test al, 4 // 00b2861f
        jne L_00b2874d // 00b28621
        push edi // 00b28627
        mov ecx, ebx // 00b28628
        call native_plane_set_flags_00b65700 // 00b2862a
        test al, 2 // 00b2862f
        je L_00b2874d // 00b28631
        mov ecx, esi // 00b28637
        call get_native_camera_view_00b6fcb0 // 00b28639
        mov edx, eax // 00b2863e
        lea ecx, [esp + 064h] // 00b28640
        call invert_native_camera_scaled_affine_00b63b30 // 00b28644
        push eax // 00b28649
        lea ecx, [esp + 0a8h] // 00b2864a
        push ecx // 00b28651
        mov ecx, esi // 00b28652
        call get_native_camera_projection_00b6fcf0 // 00b28654
        mov edx, eax // 00b28659
        lea ecx, [esp + 0ech] // 00b2865b
        call invert_native_camera_matrix_00b632d0 // 00b28662
        mov ecx, eax // 00b28667
        call multiply_native_camera_matrices_00413920 // 00b28669
        mov esi, eax // 00b2866e
        mov ecx, 010h // 00b28670
        lea edi, [esp + 014h] // 00b28675
        rep movsd  // 00b28679
        movss xmm0, dword ptr [esp + 018h] // 00b2867b
        movss xmm1, dword ptr [esp + 024h] // 00b28681
        mov ecx, dword ptr [esp + 010h] // 00b28687
        movss dword ptr [esp + 018h], xmm1 // 00b2868b
        movss xmm1, dword ptr [esp + 034h] // 00b28691
        movss dword ptr [esp + 024h], xmm0 // 00b28697
        movss xmm0, dword ptr [esp + 01ch] // 00b2869d
        movss dword ptr [esp + 01ch], xmm1 // 00b286a3
        movss xmm1, dword ptr [esp + 044h] // 00b286a9
        movss dword ptr [esp + 034h], xmm0 // 00b286af
        movss xmm0, dword ptr [esp + 020h] // 00b286b5
        movss dword ptr [esp + 020h], xmm1 // 00b286bb
        movss xmm1, dword ptr [esp + 038h] // 00b286c1
        movss dword ptr [esp + 044h], xmm0 // 00b286c7
        movss xmm0, dword ptr [esp + 02ch] // 00b286cd
        lea edx, [esp + 014h] // 00b286d3
        push edx // 00b286d7
        lea eax, [esp + 058h] // 00b286d8
        movss dword ptr [esp + 030h], xmm1 // 00b286dc
        movss xmm1, dword ptr [esp + 04ch] // 00b286e2
        movss dword ptr [esp + 03ch], xmm0 // 00b286e8
        movss xmm0, dword ptr [esp + 034h] // 00b286ee
        push eax // 00b286f4
        movss dword ptr [esp + 038h], xmm1 // 00b286f5
        movss xmm1, dword ptr [esp + 054h] // 00b286fb
        movss dword ptr [esp + 050h], xmm0 // 00b28701
        movss xmm0, dword ptr [esp + 048h] // 00b28707
        push ecx // 00b2870d
        mov ecx, ebx // 00b2870e
        movss dword ptr [esp + 04ch], xmm1 // 00b28710
        movss dword ptr [esp + 058h], xmm0 // 00b28716
        call native_plane_set_plane_00b656f0 // 00b2871c
        mov ecx, eax // 00b28721
        call plane_abi // 00b28723
        mov eax, dword ptr [ebp + 019f0h] // 00b28728
        lea edx, [esp + 054h] // 00b2872e
        push edx // 00b28732
        push eax // 00b28733
        mov ecx, ebp // 00b28734
        mov edx, dword ptr [esp + 12ch] // context under two original stacked args
        call clip_abi // 00b28736
        add dword ptr [ebp + 019f0h], 1 // 00b2873b
        mov edi, dword ptr [esp + 010h] // 00b28742
        mov esi, dword ptr [esp + 012ch] // 00b28746
    L_00b2874d:
        add edi, 1 // 00b2874d
        mov ecx, ebx // 00b28750
        mov dword ptr [esp + 010h], edi // 00b28752
        call native_plane_set_count_00b65080 // 00b28756
        cmp edi, eax // 00b2875b
        jb L_00b28617 // 00b2875d
    L_00b28763:
        mov ecx, dword ptr [ebp + 019f0h] // 00b28763
        mov edx, 1 // 00b28769
        shl edx, cl // 00b2876e
        mov ecx, ebp // 00b28770
        sub edx, 1 // 00b28772
        push edx // 00b28775
        push 098h // 00b28776
        mov edx, dword ptr [esp + 12ch] // context under two original stacked args
        call state_abi // 00b2877b
        mov eax, dword ptr [ebp + 019f0h] // 00b28780
        mov dword ptr [ebp + 019ech], eax // 00b28786
    L_00b2878c:
        mov esi, dword ptr [esi + 0184h] // 00b2878c
        test esi, esi // 00b28792
        je L_00b287b3 // 00b28794
        mov ecx, esi // 00b28796
        call native_ambient_color_address_00b84c60 // 00b28798
        push eax // 00b2879d
        lea ecx, [esp + 014h] // 00b2879e
        mov edx, dword ptr [esp + 128h] // context under original source arg
        call color_abi // 00b287a2
        mov ecx, dword ptr [eax] // 00b287a7
        push ecx // 00b287a9
        push 022h // 00b287aa
        mov ecx, ebp // 00b287ac
        mov edx, dword ptr [esp + 12ch] // context under two original stacked args
        call state_abi // 00b287ae
    L_00b287b3:
        pop edi // 00b287b3
        pop esi // 00b287b4
        pop ebp // 00b287b5
        pop ebx // 00b287b6
        add esp, 0114h // 00b287b7
        pop edx // discard context after original scratch/save-frame cleanup
        ret 4 // 00b287bd
    }
}
} // namespace bsp
