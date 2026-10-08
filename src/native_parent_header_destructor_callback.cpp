#include "bsp/native_parent_header_destructor_callback.hpp"

#include "bsp/native_parent_header_clear.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This complete native tail body requires MSVC Win32.
#endif

namespace bsp {

// Native 5B/1 instruction, SHA256
// fcd4639035646b9e7bc377f6227d569bf4315202717b7b3298d19d59a6ad1f57.
// Only the E9 REL32 operand [1,5) binds to the admitted physical raw clear.
// No prologue, CALL, local RET, class dispatch or lifetime provider is added.
__declspec(naked) void __fastcall destroy_native_parent_header_004c2d30(
    void*, std::uint32_t) noexcept {
    __asm {
        jmp clear_native_parent_header_004bf8e0
    }
}

} // namespace bsp
