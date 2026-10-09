#include "bsp/native_vehicle_class_base_constructor.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Vehicle class base construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

void store_word(void* actual, std::size_t offset, std::uint32_t value) noexcept {
    auto* const bytes = static_cast<std::byte*>(actual);
    *reinterpret_cast<volatile std::uint32_t*>(bytes + offset) = value;
}
} // namespace

void* construct_native_vehicle_class_base_00749050(void* actual,
    const NativeDamageableClassConstructionAccess& damageable) {
    construct_native_damageable_class_0087c640(actual, damageable);

    store_word(actual, 0x6c, 0x00cff378);
    store_word(actual, 0x00, 0x00cff7cc);
    store_word(actual, 0x6c, 0x00cff7c8);
    store_word(actual, 0x74, 0);
    store_word(actual, 0x78, 0);
    store_word(actual, 0x94, 0);
    store_word(actual, 0x98, 0);
    store_word(actual, 0x9c, 0);
    store_word(actual, 0xb4, 0);
    store_word(actual, 0xdc, 0);
    store_word(actual, 0xe0, 0);
    store_word(actual, 0xe4, 0);
    store_word(actual, 0xe8, 0);
    store_word(actual, 0xec, 0);
    store_word(actual, 0xf0, 0);
    store_word(actual, 0xf4, 0);
    store_word(actual, 0xf8, 0);
    store_word(actual, 0xfc, 0);
    store_word(actual, 0x100, 0);
    store_word(actual, 0x104, 0);
    store_word(actual, 0x108, 0);
    store_word(actual, 0x10c, 0);
    store_word(actual, 0x114, 0);
    store_word(actual, 0x118, 0);
    store_word(actual, 0x11c, 0);
    *reinterpret_cast<volatile std::uint8_t*>(
        static_cast<std::byte*>(actual) + 0x120) = 0;
    store_word(actual, 0x128, 0);
    store_word(actual, 0x12c, 0);
    store_word(actual, 0x130, 0);
    store_word(actual, 0x80, 0);
    return actual;
}

} // namespace bsp
