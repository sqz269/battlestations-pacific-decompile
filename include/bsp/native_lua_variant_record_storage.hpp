#pragma once

#include <cstdint>

namespace bsp {

// Plain Source storage, not a Native class/vtable, owner or resource destructor.
// No constructor, base, default member initializer or value seed belongs here.
struct alignas(std::uint32_t) NativeLuaVariantRecordStorage20 final {
    std::uint32_t profile_word;
    std::uint32_t tag_word;
    std::uint32_t payload_word;
    std::uint32_t name_length_word;
    std::uint32_t name_data_word;
};

// Caller supplies a genuine retained, nonnull, 4-aligned writable 20-byte slot
// whose prior lifetime/resource debts permit fresh placement. No allocation,
// extent check, value read/write, Native construction or cleanup is performed.
// Use the returned new pointer immediately before genuine 8849B0 or an admitted
// copy; its five scalar values remain indeterminate until actual later writes.
// No ()/{} initialization, post-hoc byte-image adoption, or typed array indexing
// across individually born records is permitted. The caller retains actual
// allocation/partial-attempt custody and every inherited nested/provider domain.
[[nodiscard]] NativeLuaVariantRecordStorage20*
begin_native_lua_variant_record_lifetime_20(void* actual_storage) noexcept;

} // namespace bsp
