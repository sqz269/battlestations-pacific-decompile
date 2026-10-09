#include "bsp/native_lua_variant_link_record_allocation.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

void* allocate_native_lua_variant_link_record_006edea0() {
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x34, 0x34}); // 006EDEA0..006EDEA7
    auto* const bytes = static_cast<unsigned char*>(allocation);

    // The real provider returns usable nonnull storage or throws. Native
    // null/wrapped-interior branches are outside this Source success domain.
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x00) = 0; // 006EDEAE
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x04) = 0; // 006EDEBB
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x08) = 0; // 006EDEC8
    *reinterpret_cast<volatile unsigned char*>(bytes + 0x30) = 1; // 006EDECE
    *reinterpret_cast<volatile unsigned char*>(bytes + 0x31) = 0; // 006EDED2
    return allocation; // Native surviving EAX at 006EDED6
}

} // namespace bsp
