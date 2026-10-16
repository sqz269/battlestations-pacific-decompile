#include "bsp/native_font_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>

namespace bsp {
void* allocate_native_font_tree_head_00ad42c0() {
    static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4,
        "AD42C0 raw storage requires Win32 pointer widths");
    static_assert(sizeof(std::uint32_t) == 4, "AD42C0 DWORD width");
    void* const result = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x18u, 0x18u});
    const auto base = reinterpret_cast<std::uintptr_t>(result);
    if (base != 0)
        *reinterpret_cast<volatile std::uint32_t*>(base) = 0;
    const auto parent = base + std::uintptr_t{4};
    if (parent != 0)
        *reinterpret_cast<volatile std::uint32_t*>(parent) = 0;
    const auto right = base + std::uintptr_t{8};
    if (right != 0)
        *reinterpret_cast<volatile std::uint32_t*>(right) = 0;
    *reinterpret_cast<volatile std::uint8_t*>(base + 0x14u) = 1;
    *reinterpret_cast<volatile std::uint8_t*>(base + 0x15u) = 0;
    return result;
}
} // namespace bsp

#include "bsp/game_native_string_process.hpp"
#include "bsp/native_int_pointer_tree18_erase.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include <exception>

namespace bsp {
void* construct_native_font_storage_00ad55c0(void* actual_font,
    const NativeFontConstructorArguments& arguments,
    game::GameNativeStringProcess& same_strings) {
    static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4,
        "AD55C0 raw storage requires Win32 pointer widths");
    static_assert(sizeof(std::uint32_t) == 4 && sizeof(std::uint16_t) == 2,
        "AD55C0 raw field widths");
    const auto font = reinterpret_cast<std::uintptr_t>(actual_font);
    void* const allocated_head = allocate_native_font_tree_head_00ad42c0();
    *reinterpret_cast<volatile std::uint32_t*>(font + 4u) =
        reinterpret_cast<std::uintptr_t>(allocated_head);
    *reinterpret_cast<volatile std::uint8_t*>(
        reinterpret_cast<std::uintptr_t>(allocated_head) + 0x15u) = 1;
    auto current_head = *reinterpret_cast<volatile std::uint32_t*>(font + 4u);
    *reinterpret_cast<volatile std::uint32_t*>(current_head + 4u) = current_head;
    current_head = *reinterpret_cast<volatile std::uint32_t*>(font + 4u);
    *reinterpret_cast<volatile std::uint32_t*>(current_head) = current_head;
    current_head = *reinterpret_cast<volatile std::uint32_t*>(font + 4u);
    *reinterpret_cast<volatile std::uint32_t*>(current_head + 8u) = current_head;
    *reinterpret_cast<volatile std::uint32_t*>(font + 8u) = 0;

    const void* const name = arguments.name_header;
    void* const destination = reinterpret_cast<void*>(font + 0x0cu);
    // Preserve the Native comparison-before-arm schedule explicitly. The real
    // raw helper repeats this pure comparison, with no intervening raw read or
    // provider call; this private volatile slot is not a Native frame alias.
    const volatile bool identical = destination == name;
    (void)identical;
    volatile std::int32_t cleanup_state = 0;
    try {
        copy_construct_native_string_header_00426060(
            destination, name, same_strings.raw_context());
        const auto scale = arguments.scale_bits;
        const auto uppercase = *reinterpret_cast<const volatile std::uint8_t*>(
            &arguments.uppercase_word);
        *reinterpret_cast<volatile std::uint32_t*>(font + 0x18u) = scale;
        const auto alpha = arguments.alpha_bits;
        *reinterpret_cast<volatile std::uint16_t*>(font + 0x14u) = 0;
        *reinterpret_cast<volatile std::uint32_t*>(font + 0x1cu) = alpha;
        *reinterpret_cast<volatile std::uint32_t*>(font + 0x20u) = 0;
        *reinterpret_cast<volatile std::uint32_t*>(font + 0x24u) = 0;
        *reinterpret_cast<volatile std::uint32_t*>(font + 0x28u) = 0;
        *reinterpret_cast<volatile std::uint32_t*>(font + 0x2cu) = 0;
        *reinterpret_cast<volatile std::uint32_t*>(font + 0x30u) = 0;
        *reinterpret_cast<volatile std::uint32_t*>(font + 0x34u) = 0;
        *reinterpret_cast<volatile std::uint32_t*>(font + 0x38u) = 0;
        *reinterpret_cast<volatile std::uint32_t*>(font + 0x3cu) = 0;
        *reinterpret_cast<volatile std::uint32_t*>(font + 0x40u) = 0;
        *reinterpret_cast<volatile std::uint32_t*>(font + 0x44u) = 0;
        *reinterpret_cast<volatile std::uint8_t*>(font + 0x48u) = uppercase;
        return actual_font;
    } catch (...) {
        if (cleanup_state == 0) {
            cleanup_state = -1; // Consume before the full current-tree action.
            try {
                destroy_native_int_pointer_tree18_0086fde0(actual_font);
            } catch (...) {
                std::terminate();
            }
        }
        throw;
    }
}
} // namespace bsp

namespace bsp {
__declspec(noinline) void* __cdecl find_native_font_glyph_iterator_00ad4540(
    void* actual_font, void* actual_output8,
    const volatile std::uint16_t& actual_key) {
    static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4,
        "AD4540 raw lookup requires Win32 pointer widths");
    const auto tree = reinterpret_cast<std::uintptr_t>(actual_font);
    auto candidate = *reinterpret_cast<const volatile std::uint32_t*>(tree + 4u);
    auto node = *reinterpret_cast<const volatile std::uint32_t*>(candidate + 4u);
    if (*reinterpret_cast<const volatile std::uint8_t*>(node + 0x15u) == 0) {
        const auto captured_key = actual_key;
        do {
            const auto node_key =
                *reinterpret_cast<const volatile std::uint16_t*>(node + 0x0cu);
            if (node_key < captured_key) {
                node = *reinterpret_cast<const volatile std::uint32_t*>(node + 8u);
            } else {
                candidate = node;
                node = *reinterpret_cast<const volatile std::uint32_t*>(node);
            }
        } while (*reinterpret_cast<const volatile std::uint8_t*>(node + 0x15u) == 0);
    }
    const auto current_head =
        *reinterpret_cast<const volatile std::uint32_t*>(tree + 4u);
    auto chosen = current_head;
    if (candidate != current_head) {
        const auto final_key = actual_key;
        const auto candidate_key =
            *reinterpret_cast<const volatile std::uint16_t*>(candidate + 0x0cu);
        if (final_key >= candidate_key) chosen = candidate;
    }
    const auto output = reinterpret_cast<std::uintptr_t>(actual_output8);
    *reinterpret_cast<volatile std::uint32_t*>(output) = tree;
    *reinterpret_cast<volatile std::uint32_t*>(output + 4u) = chosen;
    return actual_output8;
}

void* __cdecl find_native_font_glyph_iterator_00ad4370(void* actual_font,
    void* actual_output8, const volatile std::uint16_t& actual_key) {
    return find_native_font_glyph_iterator_00ad4540(
        actual_font, actual_output8, actual_key);
}
} // namespace bsp
