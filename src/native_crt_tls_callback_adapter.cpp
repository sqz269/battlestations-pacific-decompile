#include "bsp/native_crt_tls_callback_adapter.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT TLS callback adapter requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(DWORD) == 4);
static_assert(sizeof(PFLS_CALLBACK_FUNCTION) == 4);

__declspec(naked) DWORD __stdcall
native_crt_tls_alloc_ignoring_callback_00c0504c(PFLS_CALLBACK_FUNCTION) {
    __asm {
        call TlsAlloc
        ret 4
    }
}
} // namespace bsp
