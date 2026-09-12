#include "bsp/native_ref_counted.hpp"

namespace bsp {
void invoke_native_ref_counted_delete_00bd30e0(void* owner,
    NativeRefCountedDeleteCalls& calls) {
    if (owner == nullptr) return;
    const std::uint32_t profile = *static_cast<const volatile std::uint32_t*>(owner);
    calls.delete_vslot04(owner, profile, 1);
}

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native refcount base profile requires MSVC Win32.
#endif
__declspec(naked) void __fastcall destroy_native_ref_counted_base_00bd30f0(void*) noexcept {
    __asm {
        mov dword ptr [ecx], 00ceb130h
        ret
    }
}
} // namespace bsp
