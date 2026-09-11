#include "bsp/native_hardware_layout_tree_key.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hardware-layout tree key operations require MSVC Win32.
#endif

namespace bsp {
namespace {

std::uint32_t read_word(const void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<const unsigned char*>(storage) + offset);
}
std::int32_t read_count(const void* storage) noexcept {
    return static_cast<std::int32_t>(read_word(storage, 0x10));
}
void write_word(void* storage, std::size_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(storage) + offset) = value;
}
void* read_pointer(const void* storage, std::size_t offset) noexcept {
    return reinterpret_cast<void*>(read_word(storage, offset));
}
bool is_sentinel(const void* node) noexcept {
    return *reinterpret_cast<const volatile unsigned char*>(
        static_cast<const unsigned char*>(node) + 0x25) != 0;
}
const void* node_key(const void* node) noexcept {
    return static_cast<const unsigned char*>(node) + 0x0c;
}

void copy_key_words(void* destination, const void* source) noexcept {
    std::int32_t index = 0;
    if (read_count(source) > 0) {
        do {
            const auto offset = static_cast<std::uint32_t>(index) * 4u;
            const auto value = read_word(source, offset);
            write_word(destination, offset, value);
            ++index;
        } while (index < read_count(source));
    }
    const auto count = read_word(source, 0x10);
    write_word(destination, 0x10, count);
}

} // namespace

bool less_native_hardware_layout_key_00b20bf0(
    const void* left_key, const void* right_key) noexcept {
    const auto right_count = read_count(right_key);
    const auto left_count = read_count(left_key);
    if (left_count < right_count) {
        return false;
    }
    if (left_count > right_count) {
        return true;
    }
    for (std::int32_t index = 0; index < left_count; ++index) {
        const auto offset = static_cast<std::uint32_t>(index) * 4u;
        const auto left = read_word(left_key, offset);
        const auto right = read_word(right_key, offset);
        if (left < right) {
            return true;
        }
        if (left > right) {
            return false;
        }
    }
    return false;
}

void* lower_bound_native_hardware_layout_key_00b23020(
    void* actual_tree, const void* key) noexcept {
    auto* candidate = read_pointer(actual_tree, 4);
    auto* node = read_pointer(candidate, 4);
    while (!is_sentinel(node)) {
        if (less_native_hardware_layout_key_00b20bf0(node_key(node), key)) {
            node = read_pointer(node, 8);
        } else {
            candidate = node;
            node = read_pointer(node, 0);
        }
    }
    return candidate;
}

NativeHardwareLayoutTreeIterator* find_native_hardware_layout_key_00b28220(
    void* actual_tree, NativeHardwareLayoutTreeIterator* output, const void* key,
    const SingletonLifetimeCallbacks& invalid_parameters) {
    auto* node = lower_bound_native_hardware_layout_key_00b23020(actual_tree, key);
    if (actual_tree == nullptr) {
        invalid_parameters.invalid_parameter(invalid_parameters.context);
    }
    auto* const head = read_pointer(actual_tree, 4);
    if (node == head || less_native_hardware_layout_key_00b20bf0(key, node_key(node))) {
        node = head;
    }
    write_word(output, 0, reinterpret_cast<std::uint32_t>(actual_tree));
    write_word(output, 4, reinterpret_cast<std::uint32_t>(node));
    return output;
}

void* construct_native_hardware_layout_pair_00b282b0(
    void* destination_pair, const void* source_key, const void* value_address) noexcept {
    copy_key_words(destination_pair, source_key);
    const auto value = read_word(value_address, 0);
    write_word(destination_pair, 0x14, value);
    return destination_pair;
}

void* copy_native_hardware_layout_pair_00b25ef0(
    void* destination_pair, const void* source_pair) noexcept {
    copy_key_words(destination_pair, source_pair);
    const auto value = read_word(source_pair, 0x14);
    write_word(destination_pair, 0x14, value);
    return destination_pair;
}

} // namespace bsp
