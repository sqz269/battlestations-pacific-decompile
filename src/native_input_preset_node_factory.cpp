#include "bsp/native_input_preset_node_factory.hpp"

#include "bsp/native_input_preset_pair_copy_construct.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

void* at(void* storage, std::uint32_t byte_offset) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(storage)) + byte_offset);
}

void word(void* storage, std::uint32_t byte_offset,
    std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(storage, byte_offset)) = value;
}

void byte(void* storage, std::uint32_t byte_offset,
    std::uint8_t value) noexcept {
    *static_cast<volatile std::uint8_t*>(at(storage, byte_offset)) = value;
}
} // namespace

void* __fastcall buy_native_input_preset_node_0069f5f0(
    void* /*unused_ecx*/, std::uint32_t /*unused_edx*/,
    std::uint32_t left_bits, std::uint32_t parent_bits,
    std::uint32_t right_bits, const void* actual_pair,
    std::uint32_t color_word, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    void* const saved_allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x2c, 0x2c});
    try {
        if (saved_allocation != nullptr) {
            word(saved_allocation, 0, left_bits);
            word(saved_allocation, 4, parent_bits);
            word(saved_allocation, 8, right_bits);
            copy_construct_native_input_preset_pair_0069e7a0(
                at(saved_allocation, 0x0c), 0, actual_pair, strings,
                actual_feature_word_0109eea4);
            const auto current_color =
                *reinterpret_cast<const volatile std::uint8_t*>(&color_word);
            byte(saved_allocation, 0x28, current_color);
            byte(saved_allocation, 0x29, 0);
        }
    } catch (...) { // Logical Current-C++ scope for 0069F66E, not its Native ABI.
        singleton_lifetime_free(saved_allocation);
        throw;
    }
    return saved_allocation;
}
} // namespace bsp
