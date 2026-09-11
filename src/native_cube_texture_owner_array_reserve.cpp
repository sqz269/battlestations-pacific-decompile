#include "bsp/native_cube_texture_owner_array_reserve.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native cube-texture owner array reserve requires MSVC Win32.
#endif

namespace bsp {
namespace {

volatile std::uint32_t& word(void* storage, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(storage) + offset);
}
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
void* address(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}

} // namespace

void reserve_native_cube_texture_owner_array_00735ff0(
    void* header, std::int32_t requested_capacity) {
    if (requested_capacity < 1) requested_capacity = 1;
    if (signed_word(word(header, 8)) >= requested_capacity) return;
    const auto bytes = static_cast<std::uint32_t>(requested_capacity) * 4u;
    void* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes});
    auto destination = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(replacement));
    std::uint32_t index = 0;
    while (signed_word(index) < signed_word(word(header, 4))) {
        if (destination != 0) {
            const auto source = word(header) + index * 4u;
            word(address(destination)) = word(address(source));
        }
        ++index;
        destination += 4u;
    }
    singleton_lifetime_free(address(word(header)));
    // Original returning-free tail 00736041..00736049 publishes in this order.
    word(header) = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(replacement));
    word(header, 8) = static_cast<std::uint32_t>(requested_capacity);
}

} // namespace bsp
