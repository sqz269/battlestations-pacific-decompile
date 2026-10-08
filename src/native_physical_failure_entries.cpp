#include "bsp/native_physical_failure_entries.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical failure entries require MSVC Win32.
#endif

namespace bsp {

// Complete genuine Original 00530620 leaf, not a synthetic fallback callback.
__declspec(naked) void __cdecl raw_ignore_native_vfs_mount_failure_00530620() noexcept {
    __asm {
        // MSVC spells inline-asm RET here as C2 0000; retain Original's C3.
        _emit 0xC3
    }
}

// Complete Original 00BD9E30 body. The borrowed actual-manager/callable-target
// contract is in the header; publication and caller field18 loads are outside
// this entry. Preserve EDX and discard exactly one caller stack DWORD.
__declspec(naked) void __fastcall raw_notify_native_vfs_request_failure_00bd9e30(
    void*, std::uint32_t, std::uint32_t) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 090h]
        call eax
        ret 4
    }
}

} // namespace bsp
