#include "bsp/native_ref_counted_handle_initialize.hpp"

namespace bsp {

// The callback identity is recovered from two actual array-constructor data
// operands. "RefCountedHandle" is a descriptive hypothesis from its paired
// 0041DE40 destructor; only the complete addressed DWORD initializer is claimed.
__declspec(naked) void* __fastcall raw_initialize_native_ref_counted_handle_00415680(
    void*) noexcept {
    __asm {
        mov eax, ecx
        mov dword ptr [eax], 0
        ret
    }
}

} // namespace bsp
