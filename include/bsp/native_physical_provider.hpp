#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical provider ownership requires MSVC Win32.
#endif

namespace bsp {
class NativeStringStorage;
struct NativePhysicalProviderPoolContext;
struct SingletonLifetimeCallbacks;

// Borrow the application's existing canonical pool/domain, string publication
// bridge and returning CRT validation boundary. No new pool or publication.
// Production strings use ActualNativeStringPoolStorage. The literal binding
// supplies the recovered CFF208 "persistent_data" bytes, not a normalized key.
struct NativePhysicalProviderContext {
    NativePhysicalProviderPoolContext& pool_0109dbf0;
    NativeStringStorage& strings;
    const SingletonLifetimeCallbacks& invalid_parameters;
    const char* persistent_name_00cff208;
};

// BF4D30[152]: ECX owner, stacked system-header/full flag DWORD, EAX owner,
// RET8. Construct actual provider base, zero pending/cache headers, copy only
// the flag's low byte, then construct the actual20h index sentinel. Preserve
// provider+2C, padding29..2B and slot metadata38. The native diagnostic is RET.
void* construct_native_physical_provider_00bf4d30(void* actual_owner,
    const void* actual_system_name, std::uint32_t flags, NativeStringStorage&);

// BF4DF0[187]: incoming factory ECX unused, stacked system/virtual raw headers,
// EAX new3Ch pool slot or null, RET8. Require nonzero system length and its
// recorded final byte to be '\\'; compare virtual header with native425850.
// Constructor failure returns the allocation through the same pool's BF3200.
void* create_native_physical_provider_00bf4df0(void* actual_factory,
    const void* actual_system_name, const void* actual_virtual_name,
    NativePhysicalProviderContext&);

// BF4C70[190]: ECX owner, RET0. Erase the actual index through COMPLETE BE0C30,
// free current head, clear head/count, release current cache, resize pending0,
// free current pending backing, destroy base. Pending I/O is not cancelled and
// handles/overlapped buffers are not closed or freed by this native body.
void destroy_native_physical_provider_00bf4c70(void* actual_owner,
    NativeStringStorage&, const SingletonLifetimeCallbacks&);

// BF4DD0[32]: ECX owner, stacked flags, EAX original possibly returned slot,
// RET4. Always run BF4C70; only bit0 returns the slot through actual BF2FC0.
void* delete_native_physical_provider_00bf4dd0(void* actual_owner,
    std::uint32_t flags, NativePhysicalProviderContext&);

// These are explicit C++ service interfaces, not binary/FH3 replacements.
// Existing NativeStringStorage::release is noexcept: throwing lazy-getter
// identity and native SEH remain outside that boundary. Names are hypotheses.
} // namespace bsp
