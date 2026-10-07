#pragma once

#include <cstdint>

namespace bsp {
// Complete 0070DAB0..0070DB1B (108 bytes). Native ECX is writable, aligned
// 508h-byte group storage; EAX returns that same address, with plain RET.
// Borrow the actual CF4888 constant bits (4479C000h, float 999.0); capture
// them once before any receiver store. The argument exposes native data,
// not a default-speed provider. All accesses require valid Win32 storage.
// Initializes exactly 989 bytes and preserves the other 299, including
// padding, record position triplets, group+4FCh and group+504h.
// The CFD6F8h profile stamp remains raw UNCALLABLE data in this new C++
// interface. This does not create a C++ polymorphic object or replace the
// original class ABI, allocation, publication, formation or lifetime path.
void* construct_native_unit_group_0070dab0(
    void* receiver, const volatile std::uint32_t& native_cf4888_bits) noexcept;
} // namespace bsp
