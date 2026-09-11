#pragma once

#include <cstdint>

namespace bsp {

// Complete B20A80..B20B74. Native ECX=usage output, EDX=pool output,
// stack flags then resource kind, RET8; no semantic result. New Win32 API.
// Output cells are raw four-byte extents and may be unaligned or overlap.
// Valid low nibble 0..3 writes pool first; other nibbles never access that
// output. Usage always writes last. Flags/kind are captured input values.
void translate_native_resource_creation_flags_00b20a80(
    void* actual_usage_output, void* actual_pool_output,
    std::uint32_t engine_flags, std::uint32_t resource_kind) noexcept;

} // namespace bsp
