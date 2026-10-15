#include "bsp/native_scene_integer_set_extrema.hpp"

#include <cstdint>

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4);

std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}

void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(value);
}

std::uint32_t child(std::uint32_t node, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(node + offset);
}

bool sentinel(std::uint32_t node) noexcept {
    return *reinterpret_cast<const volatile unsigned char*>(node + 0x11u) != 0;
}

} // namespace

void* __fastcall maximum_native_scene_integer_set_node_00466a40(void* actual_node) {
    auto node = address(actual_node);
    auto next = child(node, 8);
    while (!sentinel(next)) {
        node = next;
        next = child(node, 8);
    }
    return pointer(node);
}

void* __fastcall minimum_native_scene_integer_set_node_00466a60(void* actual_node) {
    auto node = address(actual_node);
    auto next = child(node, 0);
    while (!sentinel(next)) {
        node = next;
        next = child(node, 0);
    }
    return pointer(node);
}

} // namespace bsp
