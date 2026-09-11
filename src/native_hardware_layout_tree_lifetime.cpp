#include "bsp/native_hardware_layout_tree_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hardware-layout tree lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* volatile& word(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(storage) + offset);
}
volatile std::uint8_t& byte(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint8_t*>(static_cast<unsigned char*>(storage) + offset);
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}
} // namespace

void* allocate_native_hardware_layout_tree_sentinel_00b25dc0() {
    void* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x28, 0x28});
    // Preserve each original computed-address check. The established allocator
    // cannot return null successfully; the later byte writes require storage.
    const auto address = reinterpret_cast<std::uintptr_t>(node);
    for (std::uintptr_t offset = 0; offset != 12; offset += 4) {
        const auto link_address = address + offset;
        if (link_address != 0) {
            *reinterpret_cast<void* volatile*>(link_address) = nullptr;
        }
    }
    byte(node, 0x24) = 1;
    byte(node, 0x25) = 0;
    return node;
}

void destroy_native_hardware_layout_subtree_00b230b0(void* tree, void* node) noexcept {
    while (byte(node, 0x25) == 0) {
        destroy_native_hardware_layout_subtree_00b230b0(tree, word(node, 8));
        // Capture only after recursive destruction. The returning-free tail
        // B230D4..B230DE then tests this captured child and repeats the loop.
        void* const next = word(node, 0);
        singleton_lifetime_free(node);
        node = next;
    }
}

NativeHardwareLayoutTreeIterator* erase_native_hardware_layout_range_00b2f3a0(
    void* tree, NativeHardwareLayoutTreeIterator* output,
    NativeHardwareLayoutTreeIterator first, NativeHardwareLayoutTreeIterator last,
    const SingletonLifetimeCallbacks& callbacks) {
    void* captured_owner = word(&first, 0);
    void* const captured_minimum = word(word(tree, 4), 0);
    if (!captured_owner || captured_owner != tree) {
        invalid(callbacks);
    }
    void* captured_node = word(&first, 4);
    if (captured_node == captured_minimum) {
        void* const last_owner = word(&last, 0);
        void* const captured_head = word(tree, 4);
        if (!last_owner || last_owner != tree) {
            invalid(callbacks);
        }
        if (word(&last, 4) == captured_head) {
            destroy_native_hardware_layout_subtree_00b230b0(tree, word(word(tree, 4), 4));
            void* current_head = word(tree, 4);
            word(current_head, 4) = current_head;
            current_head = word(tree, 4);
            *reinterpret_cast<volatile std::uint32_t*>(
                static_cast<unsigned char*>(tree) + 8) = 0;
            word(current_head, 0) = current_head;
            current_head = word(tree, 4);
            word(current_head, 8) = current_head;
            void* const result_node = word(word(tree, 4), 0);
            word(output, 0) = tree;
            word(output, 4) = result_node;
            return output;
        }
    }
    for (;;) {
        if (!captured_owner || captured_owner != word(&last, 0)) {
            invalid(callbacks);
        }
        if (captured_node == word(&last, 4)) {
            word(output, 0) = captured_owner;
            word(output, 4) = captured_node;
            return output;
        }
        increment_native_hardware_layout_iterator_00b20dc0(first, callbacks);
        NativeHardwareLayoutTreeIterator ignored;
        erase_native_hardware_layout_iterator_00b2ef00(
            tree, &ignored, {captured_owner, captured_node}, callbacks);
        captured_node = word(&first, 4);
        captured_owner = word(&first, 0);
    }
}
} // namespace bsp
