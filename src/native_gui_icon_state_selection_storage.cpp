#include "bsp/native_gui_icon_state_selection_storage.hpp"
#include "bsp/native_gui_icon_storage.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source supplier requires MSVC Win32 inline assembly.
#endif

namespace bsp {
void select_native_gui_icon_state_storage_00ab1710(void* actual_icon,
    volatile std::uint32_t (&actual_argument_homes)[3],
    const volatile float& actual_one_00d7a24c,
    NativeGuiIconCurrentCalls& current) {
    void* const captured_receiver = actual_icon;
    volatile std::uint32_t* const home_address = &actual_argument_homes[0];
    const volatile float* const one_address = &actual_one_00d7a24c;
    std::uint32_t captured_target;
    std::uint32_t captured_index;
    std::uint32_t dispatch_required;
    __asm {
        // Source ABI address bindings only; no Native operand sampled here.
        mov ecx, captured_receiver
        mov ebx, home_address
        mov edx, one_address

        xorps xmm0, xmm0
        movss xmm1, dword ptr [ebx + 8]
        comiss xmm0, xmm1
        ja select_clamped
        movss xmm0, dword ptr [edx]
        comiss xmm1, xmm0
        ja select_clamped
        movaps xmm0, xmm1
    select_clamped:
        mov esi, dword ptr [ebx]
        cmp si, word ptr [ecx + 0ech]
        mov edi, dword ptr [ebx + 4]
        movss dword ptr [ebx + 8], xmm0
        je select_same_index
        mov edx, dword ptr [ecx + 0f4h]
        test edx, edx
        jne select_has_storage
        xor eax, eax
        jmp select_count_ready
    select_has_storage:
        mov eax, dword ptr [ecx + 0f8h]
        sub eax, edx
        sar eax, 6
    select_count_ready:
        movsx edx, si
        cmp edx, eax
        jb select_rebuild
        cmp byte ptr [ecx + 100h], 0
        je select_rebuild
    select_same_index:
        cmp dword ptr [ecx + 110h], edi
        jne select_rebuild
        fld dword ptr [ebx + 8]
        fld dword ptr [ecx + 114h]
        fucomip st(0), st(1)
        fstp st(0)
        lahf
        test ah, 44h
        jnp select_no_rebuild
    select_rebuild:
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [eax + 80h]
        // Logical Source capture at the Native PUSH ESI epoch, not stack ABI.
        mov captured_index, esi
        mov captured_target, edx
        movss dword ptr [ecx + 114h], xmm0
        mov dword ptr [ecx + 110h], edi
        mov dispatch_required, 1
        jmp select_source_done
    select_no_rebuild:
        mov dispatch_required, 0
    select_source_done:
    }
    if (dispatch_required)
        current.call_rebuild_current80(captured_receiver, captured_target, captured_index);
}
} // namespace bsp
