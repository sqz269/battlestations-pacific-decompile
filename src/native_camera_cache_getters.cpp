#include "bsp/native_camera_cache_getters.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_camera_projection.hpp"
#include "bsp/native_camera_general_inverse.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera cache getters require MSVC Win32.
#endif

namespace bsp {

// Complete original 00b6fcb0; calls bind complete raw providers.
__declspec(naked) void* __fastcall get_native_camera_view_00b6fcb0(void*) {
    __asm {
        sub esp,040h // 00b6fcb0
        push esi // 00b6fcb3
        mov esi,ecx // 00b6fcb4
        mov eax,dword ptr [esi + 05ch] // 00b6fcb6
        test al,08h // 00b6fcb9
        jnz L_00b6fce2 // 00b6fcbb
        test al,02h // 00b6fcbd
        jnz L_00b6fcc6 // 00b6fcbf
        call refresh_native_camera_world_00b6db70 // 00b6fcc1
    L_00b6fcc6:
        lea edx,[esi + 0f0h] // 00b6fcc6
        lea ecx,[esp + 04h] // 00b6fccc
        call invert_native_camera_scaled_affine_00b63b30 // 00b6fcd0
        push eax // 00b6fcd5
        lea ecx,[esi + 060h] // 00b6fcd6
        call copy_native_camera_matrix_004134f0 // 00b6fcd9
        or dword ptr [esi + 05ch],08h // 00b6fcde
    L_00b6fce2:
        lea eax,[esi + 060h] // 00b6fce2
        pop esi // 00b6fce5
        add esp,040h // 00b6fce6
        ret // 00b6fce9
    }
}

// Complete original 00b6fcf0; calls bind complete raw providers.
__declspec(naked) void* __fastcall get_native_camera_projection_00b6fcf0(void*) {
    __asm {
        sub esp,040h // 00b6fcf0
        push esi // 00b6fcf3
        mov esi,ecx // 00b6fcf4
        test byte ptr [esi + 02f0h],08h // 00b6fcf6
        jnz L_00b6fd55 // 00b6fcfd
        fld dword ptr [esi + 01d8h] // 00b6fcff
        push edi // 00b6fd05
        sub esp,010h // 00b6fd06
        fstp dword ptr [esp + 0ch] // 00b6fd09
        lea ecx,[esp + 018h] // 00b6fd0d
        fld dword ptr [esi + 01d4h] // 00b6fd11
        lea edi,[esi + 01e0h] // 00b6fd17
        fstp dword ptr [esp + 08h] // 00b6fd1d
        fld dword ptr [esi + 01c8h] // 00b6fd21
        fstp dword ptr [esp + 04h] // 00b6fd27
        fld dword ptr [esi + 01c4h] // 00b6fd2b
        fstp dword ptr [esp] // 00b6fd31
        mov edx, esp // actual callee argument copies, not camera fields
        call build_native_camera_projection_00b642f0 // 00b6fd34
        add esp, 10h // new raw builder uses RET instead of original RET10h
        push eax // 00b6fd39
        mov ecx,edi // 00b6fd3a
        call copy_native_camera_matrix_004134f0 // 00b6fd3c
        push edi // 00b6fd41
        lea ecx,[esi + 02a0h] // 00b6fd42
        call copy_native_camera_matrix_004134f0 // 00b6fd48
        or dword ptr [esi + 02f0h],08h // 00b6fd4d
        pop edi // 00b6fd54
    L_00b6fd55:
        lea eax,[esi + 02a0h] // 00b6fd55
        pop esi // 00b6fd5b
        add esp,040h // 00b6fd5c
        ret // 00b6fd5f
    }
}

// Complete original 00b70490; calls bind complete raw providers.
__declspec(naked) void* __fastcall get_native_camera_view_projection_00b70490(void*) {
    __asm {
        sub esp,080h // 00b70490
        push esi // 00b70496
        mov esi,ecx // 00b70497
        test byte ptr [esi + 02f0h],010h // 00b70499
        jnz L_00b704f6 // 00b704a0
        mov eax,dword ptr [esi + 05ch] // 00b704a2
        test al,08h // 00b704a5
        jnz L_00b704ce // 00b704a7
        test al,02h // 00b704a9
        jnz L_00b704b2 // 00b704ab
        call refresh_native_camera_world_00b6db70 // 00b704ad
    L_00b704b2:
        lea edx,[esi + 0f0h] // 00b704b2
        lea ecx,[esp + 04h] // 00b704b8
        call invert_native_camera_scaled_affine_00b63b30 // 00b704bc
        push eax // 00b704c1
        lea ecx,[esi + 060h] // 00b704c2
        call copy_native_camera_matrix_004134f0 // 00b704c5
        or dword ptr [esi + 05ch],08h // 00b704ca
    L_00b704ce:
        mov ecx,esi // 00b704ce
        call get_native_camera_projection_00b6fcf0 // 00b704d0
        push eax // 00b704d5
        lea eax,[esp + 048h] // 00b704d6
        push eax // 00b704da
        lea ecx,[esi + 060h] // 00b704db
        call multiply_native_camera_matrices_00413920 // 00b704de
        push eax // 00b704e3
        lea ecx,[esi + 0220h] // 00b704e4
        call copy_native_camera_matrix_004134f0 // 00b704ea
        or dword ptr [esi + 02f0h],010h // 00b704ef
    L_00b704f6:
        lea eax,[esi + 0220h] // 00b704f6
        pop esi // 00b704fc
        add esp,080h // 00b704fd
        ret // 00b70503
    }
}

// Complete original 00b70510; calls bind complete raw providers.
__declspec(naked) void* __fastcall get_native_camera_inverse_view_projection_00b70510(void*) {
    __asm {
        sub esp,040h // 00b70510
        push esi // 00b70513
        mov esi,ecx // 00b70514
        test byte ptr [esi + 02f0h],020h // 00b70516
        jnz L_00b70542 // 00b7051d
        call get_native_camera_view_projection_00b70490 // 00b7051f
        mov edx,eax // 00b70524
        lea ecx,[esp + 04h] // 00b70526
        call invert_native_camera_matrix_00b632d0 // 00b7052a
        push eax // 00b7052f
        lea ecx,[esi + 0260h] // 00b70530
        call copy_native_camera_matrix_004134f0 // 00b70536
        or dword ptr [esi + 02f0h],020h // 00b7053b
    L_00b70542:
        lea eax,[esi + 0260h] // 00b70542
        pop esi // 00b70548
        add esp,040h // 00b70549
        ret // 00b7054c
    }
}

// Complete original 00b70710; calls bind complete raw providers.
__declspec(naked) void* __fastcall get_native_camera_frustum_00b70710(void*, const NativeCameraFrustumContext*) {
    __asm {
        push edx // added context word outside the original local frame
        sub esp,060h // 00b70710
        push esi // 00b70713
        mov esi,ecx // 00b70714
        mov eax,dword ptr [esi + 02f0h] // 00b70716
        test al,04h // 00b7071c
        jnz L_00b7074a // 00b7071e
        or eax,04h // 00b70720
        mov dword ptr [esi + 02f0h],eax // 00b70723
        call get_native_camera_view_projection_00b70490 // 00b70729
        push eax // 00b7072e
        lea ecx,[esp + 08h] // 00b7072f
        mov edx, dword ptr [esp + 68h] // reload fixed context after VP
        call extract_native_camera_frustum_00b653f0 // 00b70733
        push 07h // 00b70738
        lea eax,[esp + 08h] // 00b7073a
        push eax // 00b7073e
        lea ecx,[esi + 02f4h] // 00b7073f
        call assign_native_camera_frustum_planes_00b658e0 // 00b70745
    L_00b7074a:
        lea eax,[esi + 02f4h] // 00b7074a
        pop esi // 00b70750
        add esp,060h // 00b70751
        pop edx // discard added context word; original return remains
        ret // 00b70754
    }
}

} // namespace bsp
