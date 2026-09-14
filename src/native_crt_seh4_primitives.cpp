#include "bsp/native_crt_seh4_primitives.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT SEH4 primitives require MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uintptr_t) == 4);

// Preserve the complete native unlink/restore schedule, including the first
// POP EDI that discards the saved cookie after unlinking FS:[0]. No prolog.
// C4733 also diagnoses this required restore of an existing FS link. Suppress
// it only here; this entry installs no new handler or SafeSEH ownership.
#pragma warning(push)
#pragma warning(disable : 4733)
__declspec(naked) void __cdecl leave_native_crt_seh4_frame_00c07c45() {
    __asm {
        mov ecx, dword ptr [ebp - 10h]
        mov dword ptr fs:[0], ecx
        pop ecx
        pop edi
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        push ecx
        ret
    }
}
#pragma warning(pop)

// Preserve the actual filter call's register/frame ABI and raw return state.
// The borrowed code entry is a native funclet, not an invented host callback.
__declspec(naked) std::int32_t __fastcall call_native_crt_eh4_filter_00c0dcb6(
    std::uintptr_t, void*) {
    __asm {
        push ebp
        push esi
        push edi
        push ebx
        mov ebp, edx
        xor eax, eax
        xor ebx, ebx
        xor edx, edx
        xor esi, esi
        xor edi, edi
        call ecx
        pop ebx
        pop edi
        pop esi
        pop ebp
        ret
    }
}

} // namespace bsp
