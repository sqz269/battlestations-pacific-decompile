#include "bsp/native_input_string_tree_lower_bounds.hpp"
#include "bsp/native_crt_stricmp.hpp"

#include <cstddef>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);

Word word(const void* base, std::size_t offset) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const std::byte*>(base) + offset);
}
void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
bool controller_nil(const void* node) {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<const std::byte*>(node) + 0x21) != 0;
}
bool preset_nil(const void* node) {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<const std::byte*>(node) + 0x29) != 0;
}
} // namespace

void* __fastcall lower_bound_native_input_controller_tree_0069ce30(
    void* actual_tree, std::uint32_t unused_edx, const void* actual_key_header,
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& callees) {
    static_cast<void>(unused_edx);
    auto* const head = pointer(word(actual_tree, 4));
    auto* node = pointer(word(head, 4));
    auto* candidate = head;
    if (controller_nil(node))
        return head;
    do {
        std::int32_t comparison;
        if (word(node, 0x0c) == 0) {
            comparison = word(actual_key_header, 0) != 0 ? -1 : 0;
        } else if (word(actual_key_header, 0) == 0) {
            candidate = node;
            node = pointer(word(node, 0));
            continue;
        } else {
            const auto* const key_data = static_cast<const char*>(
                pointer(word(actual_key_header, 4)));
            const auto* const node_data = static_cast<const char*>(
                pointer(word(node, 0x10)));
            comparison = compare_native_crt_strings_00bf7fbf(
                node_data, key_data, actual_locale_changed_0109de1c, callees);
        }
        if (comparison < 0) {
            node = pointer(word(node, 8));
        } else {
            candidate = node;
            node = pointer(word(node, 0));
        }
    } while (!controller_nil(node));
    return candidate;
}

void* __fastcall lower_bound_native_input_preset_tree_0069cdb0(
    void* actual_tree, std::uint32_t unused_edx, const void* actual_key_header,
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& callees) {
    static_cast<void>(unused_edx);
    auto* const head = pointer(word(actual_tree, 4));
    auto* node = pointer(word(head, 4));
    auto* candidate = head;
    if (preset_nil(node))
        return head;
    do {
        std::int32_t comparison;
        if (word(node, 0x0c) == 0) {
            comparison = word(actual_key_header, 0) != 0 ? -1 : 0;
        } else if (word(actual_key_header, 0) == 0) {
            candidate = node;
            node = pointer(word(node, 0));
            continue;
        } else {
            const auto* const key_data = static_cast<const char*>(
                pointer(word(actual_key_header, 4)));
            const auto* const node_data = static_cast<const char*>(
                pointer(word(node, 0x10)));
            comparison = compare_native_crt_strings_00bf7fbf(
                node_data, key_data, actual_locale_changed_0109de1c, callees);
        }
        if (comparison < 0) {
            node = pointer(word(node, 8));
        } else {
            candidate = node;
            node = pointer(word(node, 0));
        }
    } while (!preset_nil(node));
    return candidate;
}
} // namespace bsp
