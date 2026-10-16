#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Original CRT lock cleanup requires MSVC Win32.
#endif

namespace bsp {

// Whole [00C051AE,00C051B7),9B/4ops: push12,call real canonical unlock,
// discard its one argument,plainRET. No input argument/register data or
// semantic EAX result. Uses the existing actual E164D8 descriptor and its
// initialized Win32 critical section currently owned by this thread.
// No replacement lock/global/owner/callback, acquisition or PTD initialization.
// Ordinary Source API only; caller SEH/private frames, incidental registers,
// fault/binary-entry identity, PTD/FLS/fatal transport and game remain unproved.
void unlock_native_crt_ptd_initialization_00c051ae();

} // namespace bsp
