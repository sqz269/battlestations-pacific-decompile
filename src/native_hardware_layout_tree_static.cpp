#include "bsp/native_hardware_layout_tree_static.hpp"
#include "bsp/native_hardware_layout_tree_lifetime.hpp"

#include <cstdlib>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hardware-layout tree startup requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* bound_tree;
const SingletonLifetimeCallbacks* bound_invalid;

void* volatile& pointer(void* storage, std::uint32_t offset) noexcept {
    return *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(storage) + offset);
}
volatile std::uint32_t& word(void* storage, std::uint32_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(storage) + offset);
}
} // namespace

void bind_static_native_hardware_layout_tree_0108d530(
    void* actual_tree, const SingletonLifetimeCallbacks& invalid_parameters) noexcept {
    bound_tree = actual_tree;
    bound_invalid = &invalid_parameters;
}

int initialize_static_native_hardware_layout_tree_00cd7960() {
    auto* const tree = bound_tree;
    auto* const allocated_head = allocate_native_hardware_layout_tree_sentinel_00b25dc0();
    pointer(tree, 4) = allocated_head;
    *reinterpret_cast<volatile unsigned char*>(static_cast<unsigned char*>(allocated_head) + 0x25) = 1;
    auto* head = pointer(tree, 4);
    pointer(head, 4) = head;
    head = pointer(tree, 4);
    pointer(head, 0) = head;
    head = pointer(tree, 4);
    pointer(head, 8) = head;
    word(tree, 8) = 0;
    return std::atexit(&destroy_static_native_hardware_layout_tree_00ce0c50);
}

void destroy_static_native_hardware_layout_tree_00ce0c50() {
    auto* const tree = bound_tree;
    auto* const head = pointer(tree, 4);
    auto* const minimum = pointer(head, 0);
    NativeHardwareLayoutTreeIterator ignored;
    erase_native_hardware_layout_range_00b2f3a0(
        tree, &ignored, {tree, minimum}, {tree, head}, *bound_invalid);
    singleton_lifetime_free(pointer(tree, 4));
    pointer(tree, 4) = nullptr;
    word(tree, 8) = 0;
}
} // namespace bsp
