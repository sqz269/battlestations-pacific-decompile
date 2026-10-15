#include "bsp/native_scene_integer_set_rotations.hpp"

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4);

std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}

std::uint32_t word(std::uint32_t base, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(base + offset);
}

void put(std::uint32_t base, std::uint32_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(base + offset) = value;
}

bool sentinel(std::uint32_t node) noexcept {
    return *reinterpret_cast<const volatile unsigned char*>(node + 0x11u) != 0;
}

} // namespace

void __fastcall rotate_left_native_scene_integer_set_00467740(
    void* actual_tree, std::uint32_t, void* actual_pivot) {
    const auto pivot = address(actual_pivot);
    const auto promoted = word(pivot, 8);
    const auto transferred = word(promoted);
    put(pivot, 8, transferred);
    const auto reloaded_child = word(promoted);
    if (!sentinel(reloaded_child))
        put(reloaded_child, 4, pivot);

    const auto previous_parent = word(pivot, 4);
    put(promoted, 4, previous_parent);
    const auto current_head = word(address(actual_tree), 4);
    if (pivot == word(current_head, 4)) {
        put(current_head, 4, promoted);
    } else {
        const auto current_parent = word(pivot, 4);
        if (pivot == word(current_parent))
            put(current_parent, 0, promoted);
        else
            put(current_parent, 8, promoted);
    }
    put(promoted, 0, pivot);
    put(pivot, 4, promoted);
}

void __fastcall rotate_right_native_scene_integer_set_00467790(
    void* actual_tree, std::uint32_t, void* actual_pivot) {
    const auto pivot = address(actual_pivot);
    const auto promoted = word(pivot);
    const auto transferred = word(promoted, 8);
    put(pivot, 0, transferred);
    const auto reloaded_child = word(promoted, 8);
    if (!sentinel(reloaded_child))
        put(reloaded_child, 4, pivot);

    const auto previous_parent = word(pivot, 4);
    put(promoted, 4, previous_parent);
    const auto current_head = word(address(actual_tree), 4);
    if (pivot == word(current_head, 4)) {
        put(current_head, 4, promoted);
    } else {
        const auto current_parent = word(pivot, 4);
        if (pivot == word(current_parent, 8))
            put(current_parent, 8, promoted);
        else
            put(current_parent, 0, promoted);
    }
    put(promoted, 8, pivot);
    put(pivot, 4, promoted);
}

} // namespace bsp
