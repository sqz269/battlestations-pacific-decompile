#include "bsp/native_settings_language_name.hpp"

#include <cstdint>
#include <cstring>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw settings language name requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 &&
    sizeof(std::uint32_t) == 4, "requires the Win32 address model");

std::uint32_t word(const void* base, std::uint32_t offset) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(base) + offset;
    std::uint32_t value;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    _ReadWriteBarrier();
    return value;
}
} // namespace

const char* native_settings_language_name_008d4870(const void* settings,
    void* const volatile& catalog_data, const char* actual_empty) noexcept {
    const std::uint32_t index = word(settings, 4);
    const std::uint32_t offset = index << 5;
    _ReadWriteBarrier();
    void* const current_data = catalog_data;
    _ReadWriteBarrier();
    const auto row = reinterpret_cast<std::uintptr_t>(current_data) + offset;
    const std::uint32_t data = word(reinterpret_cast<const void*>(row), 4);
    return data != 0
        ? reinterpret_cast<const char*>(static_cast<std::uintptr_t>(data))
        : actual_empty;
}
} // namespace bsp
