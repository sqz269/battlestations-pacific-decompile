#include "bsp/native_enum_registry_scalar_delete.hpp"
#include "bsp/native_enum_registry_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native enum registry scalar delete requires MSVC Win32.
#endif
namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
std::uintptr_t delete_native_scene_enum_registry_004d2620(void* owner,
    std::uint32_t flags, void* table_pool, void* symbol_pool,
    NativeStringRawPoolContext& strings) {
    const auto address_bits = reinterpret_cast<std::uintptr_t>(owner);
    destroy_native_scene_enum_registry_008f4f00(owner, table_pool, symbol_pool, strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return address_bits;
}
} // namespace bsp
