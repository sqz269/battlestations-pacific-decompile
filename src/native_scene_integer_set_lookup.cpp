#include "bsp/native_scene_integer_set_lookup.hpp"

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4);

std::uint32_t address(const void* storage) noexcept {
    return reinterpret_cast<std::uintptr_t>(storage);
}

void* at_offset(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(address(storage) + offset);
}

std::uint32_t word(const void* storage, std::uint32_t offset) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at_offset(storage, offset));
}

void* pointer(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(word(storage, offset));
}

std::int32_t signed_word(const void* storage, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::int32_t*>(at_offset(storage, offset));
}

bool nil(const void* node) noexcept {
    return *static_cast<const volatile unsigned char*>(at_offset(node, 0x11)) != 0;
}

void put(void* storage, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at_offset(storage, offset)) = value;
}

} // namespace

void* __fastcall find_native_scene_integer_set_00468db0(void* actual_tree,
    std::uint32_t, void* actual_output8, const void* actual_key4) {
    void* candidate = pointer(actual_tree, 4);
    void* node = pointer(candidate, 4);
    if (!nil(node)) {
        const auto search_key = signed_word(actual_key4);
        do {
            if (signed_word(node, 0x0c) < search_key) {
                node = pointer(node, 8);
            } else {
                candidate = node;
                node = pointer(node, 0);
            }
        } while (!nil(node));
    }

    void* const current_head = pointer(actual_tree, 4);
    void* selected_node = candidate;
    if (candidate == current_head) {
        selected_node = current_head;
    } else {
        const auto current_key = signed_word(actual_key4);
        const auto candidate_key = signed_word(candidate, 0x0c);
        if (current_key < candidate_key)
            selected_node = current_head;
    }

    const auto captured_owner = address(actual_tree);
    const auto captured_node = address(selected_node);
    put(actual_output8, 0, captured_owner);
    put(actual_output8, 4, captured_node);
    return actual_output8;
}

} // namespace bsp
