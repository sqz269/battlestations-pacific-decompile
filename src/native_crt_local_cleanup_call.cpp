#include "bsp/native_crt_local_cleanup_call.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native cleanup register entry requires MSVC Win32.
#endif

namespace bsp {
// C16898[3], exact inherited-register/stack contract in the public header.
__declspec(naked) void __cdecl call_native_crt_cleanup_00c16898() {
    __asm {
        call eax
        ret
    }
}
} // namespace bsp
