#include "bsp/native_resource_fallback_item.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uint32_t fallback_profile = 0x00d631c0;
} // namespace

void* construct_native_resource_fallback_item_00b86930(void* item) noexcept {
    auto* const words = static_cast<volatile std::uint32_t*>(item);
    words[0] = 0x00ceb130;
    words[1] = 1;
    words[0] = fallback_profile;
    return item;
}

void* scalar_delete_native_resource_fallback_item_00b86990(void* item,
    std::uint32_t flags) noexcept {
    *static_cast<volatile std::uint32_t*>(item) = 0x00d5c104;
    destroy_native_ref_counted_base_00bd30f0(item);
    if ((flags & 1u) != 0) singleton_lifetime_free(item);
    return item;
}

void NativeResourceFallbackDeleteCalls::delete_vslot04(void* item,
    std::uint32_t captured_profile, std::uint32_t flags) {
    if (captured_profile != fallback_profile)
        throw std::invalid_argument("unbound native resource fallback item profile");
    scalar_delete_native_resource_fallback_item_00b86990(item, flags);
}
} // namespace bsp
