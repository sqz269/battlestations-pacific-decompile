#pragma once

#include <cstdint>

namespace bsp {

// Whole native 00BFFB50..00BFFB8A, 59 bytes / 23 instructions.
// Native ABI: cdecl, one captured DWORD argument, EAX result, plain RET.
// This Source interface adds a borrowed binding to the actual mutable 45-pair
// table at 00E15880 (90 readable DWORD cells); it is not an Original ABI adapter.
// Read each reached first word, then its current second word only on a match.
// No table snapshot, errno/TLS access, SDK call, or heap/startup ownership.
std::uint32_t map_native_crt_os_error_00bffb50(
    std::uint32_t error,
    const volatile std::uint32_t* actual_pairs90) noexcept;

} // namespace bsp
