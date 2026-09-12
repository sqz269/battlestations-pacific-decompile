#include "bsp/native_vfs_mount_insert.hpp"

#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_vfs_mount_tree.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS mount insertion requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(const void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* volatile& link(void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<void* volatile*>(at(p, offset));
}
volatile std::uint32_t& word(const void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(p, offset));
}
volatile unsigned char& byte(const void* p, std::uint32_t offset) noexcept {
    return *static_cast<volatile unsigned char*>(at(p, offset));
}
std::int32_t priority(const void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::int32_t*>(at(p, offset));
}
void* head(void* tree) noexcept { return link(tree, 4); }
void* parent(void* node) noexcept { return link(node, 4); }
volatile unsigned char& color(void* node) noexcept { return byte(node, 0x20); }
bool nil(void* node) noexcept { return byte(node, 0x21) != 0; }
struct CompletedLengthMessage {
    NativeLegacySboStringStorage& storage;
    ~CompletedLengthMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(storage); }
};
[[noreturn]] void throw_length_error() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = 0;
    native_legacy_sbo_string_assign_counted_00408720(message, "map/set<T> too long", 19);
    // BE0E21 arms only the completed message. E00E10 -> E00E08 -> CC6780.
    const CompletedLengthMessage completed{message};
    throw NativeHardwareLayoutTreeLengthError{message};
}
} // namespace

void* copy_native_vfs_mount_record_00bdee60(void* destination,
    const void* source, NativeStringStorage& strings) {
    const auto source_priority = word(source);
    auto* const target_string = at(destination, 4);
    auto* const source_string = at(source, 4);
    const bool same = target_string == source_string;
    word(destination) = source_priority;
    word(target_string) = 0;
    word(target_string, 4) = 0;
    if (!same) {
        resize_native_string_header_0041dd40(target_string, strings, word(source_string), true);
        if (word(source_string) != 0) {
            const auto count = word(target_string);
            auto* const from = reinterpret_cast<const void*>(word(source_string, 4));
            auto* const to = reinterpret_cast<void*>(word(target_string, 4));
            // BF7680 has overlap support. Keep the existing actual-header
            // domain's zero-byte copy policy while preserving all reads.
            if (count != 0) std::memmove(to, from, count);
        }
    }
    word(target_string, 8) = word(source_string, 8);
    byte(target_string, 0x0c) = byte(source_string, 0x0c);
    return destination;
}

void* allocate_native_vfs_mount_node_00be05c0(void* left, void* parent_node,
    void* right, const void* record, std::uint8_t node_color, NativeStringStorage& strings) {
    auto* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x24, 0x24});
    try {
        if (node) {
            link(node) = left;
            link(node, 4) = parent_node;
            link(node, 8) = right;
            copy_native_vfs_mount_record_00bdee60(at(node, 0x0c), record, strings);
            color(node) = node_color;
            byte(node, 0x21) = 0;
        }
    } catch (...) {
        // E00D24 catches native states0..1 via BE063E. State1's placement
        // unwind CC6700 invokes RET-only401130. No payload destroy is present.
        singleton_lifetime_free(node);
        throw;
    }
    return node;
}

void* link_native_vfs_mount_node_00be0dd0(void* tree, void* output,
    std::uint8_t insert_left, void* parent_node, const void* record,
    NativeStringStorage& strings) {
    if (word(tree, 8) >= 0x0ccccccbu) throw_length_error();
    auto* const allocated_head = head(tree);
    auto* const node = allocate_native_vfs_mount_node_00be05c0(
        allocated_head, parent_node, allocated_head, record, 0, strings);
    auto* const current_head = head(tree);
    word(tree, 8) = word(tree, 8) + 1u;
    if (parent_node == current_head) {
        link(current_head, 4) = node;
        link(head(tree)) = node;
        link(head(tree), 8) = node;
    } else if (insert_left != 0) {
        link(parent_node) = node;
        auto* const current = head(tree);
        if (parent_node == link(current)) link(current) = node;
    } else {
        link(parent_node, 8) = node;
        auto* const current = head(tree);
        if (parent_node == link(current, 8)) link(current, 8) = node;
    }
    auto* repair = node;
    while (color(parent(repair)) == 0) {
        auto* const direct_parent = parent(repair);
        auto* const grandparent = parent(direct_parent);
        if (direct_parent == link(grandparent)) {
            auto* const uncle = link(grandparent, 8);
            if (color(uncle) == 0) {
                color(direct_parent) = 1;
                color(uncle) = 1;
                color(parent(parent(repair))) = 0;
                repair = parent(parent(repair));
            } else {
                if (repair == link(direct_parent, 8)) {
                    repair = direct_parent;
                    rotate_native_vfs_mount_left_00bda0e0(tree, repair);
                }
                color(parent(repair)) = 1;
                color(parent(parent(repair))) = 0;
                rotate_native_vfs_mount_right_00bd9530(tree, parent(parent(repair)));
            }
        } else {
            auto* const uncle = link(grandparent);
            if (color(uncle) == 0) {
                color(direct_parent) = 1;
                color(uncle) = 1;
                color(parent(parent(repair))) = 0;
                repair = parent(parent(repair));
            } else {
                if (repair == link(direct_parent)) {
                    repair = direct_parent;
                    rotate_native_vfs_mount_right_00bd9530(tree, repair);
                }
                color(parent(repair)) = 1;
                color(parent(parent(repair))) = 0;
                // BE0F46..BE0F84 inlines the same current-child/current-head
                // stores as the established BDA0E0 actual-storage helper.
                rotate_native_vfs_mount_left_00bda0e0(tree, parent(parent(repair)));
            }
        }
    }
    color(parent(head(tree))) = 1;
    link(output, 4) = node;
    link(output) = tree;
    return output;
}

void* insert_native_vfs_mount_record_00be1330(void* tree, void* output,
    const void* record, NativeStringStorage& strings) {
    auto* selected_parent = head(tree);
    auto* current = parent(selected_parent);
    std::uint8_t insert_left = 1;
    if (!nil(current)) {
        const auto captured_priority = priority(record);
        do {
            selected_parent = current;
            insert_left = captured_priority > priority(current, 0x0c) ? 1 : 0;
            current = insert_left ? link(current) : link(current, 8);
        } while (!nil(current));
    }
    void* iterator[2];
    auto* const inserted = link_native_vfs_mount_node_00be0dd0(
        tree, iterator, insert_left, selected_parent, record, strings);
    auto* const result_owner = link(inserted);
    auto* const result_node = link(inserted, 4);
    link(output) = result_owner;
    link(output, 4) = result_node;
    byte(output, 8) = 1;
    return output;
}
} // namespace bsp
