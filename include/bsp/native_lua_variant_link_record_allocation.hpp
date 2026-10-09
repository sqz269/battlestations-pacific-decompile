#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua variant link record allocation requires MSVC Win32.
#endif

namespace bsp {

// Provisional Source name for 006EDEA0..006EDED6. The Native body requests
// 34h bytes from 00BF681B and leaves the allocation pointer in EAX at RET.
// This plain C++ entry calls the actual singleton_lifetime_allocate provider
// with object/native_bytes=52/host_bytes=52. That current-host-CRT boundary
// returns usable, suitably aligned, nonnull raw storage or throws; it is not
// a binding to Native operator_new, its new handler or its exception runtime.
//
// In that successful allocation domain, write exactly these volatile fields
// in order: DWORD+0=0, DWORD+4=0, DWORD+8=0, BYTE+30h=1, BYTE+31h=0. All other
// 38 bytes remain untouched by this helper. Return the same raw allocation;
// there is no full node type, value initialization, copy or ownership wrapper.
//
// Native checks EAX, EAX+4 and EAX+8 separately before the three DWORD stores,
// then unconditionally writes the two bytes. Null and wrapped-interior paths
// are outside this provider's usable-allocation success domain and are not
// reproduced. No noexcept or Native register/flags/stack/fault/ABI, lifetime,
// caller integration, startup or gameplay equivalence is claimed.
void* allocate_native_lua_variant_link_record_006edea0();

} // namespace bsp
