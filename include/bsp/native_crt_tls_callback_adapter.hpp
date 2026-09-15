#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace bsp {
// Complete 00C0504C..00C05054: CALL [TlsAlloc IAT]; RET 4.
// Original Win32 stdcall ABI: one PFLS_CALLBACK_FUNCTION word at entry ESP+4,
// ignored without reading or invoking it; the actual TlsAlloc result is EAX.
// __mtinit selects this adapter if any of its four Fls* lookups fails. This
// function only allocates the actual OS TLS index; it neither publishes the
// provider/index nor supplies FLS callback cleanup or CRT initialization.
// CALL/RET preserve the actual provider's register/flags/LastError effects.
DWORD __stdcall native_crt_tls_alloc_ignoring_callback_00c0504c(
    PFLS_CALLBACK_FUNCTION ignored_callback);
} // namespace bsp
