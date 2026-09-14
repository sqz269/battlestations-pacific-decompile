#include "bsp/native_resource_cache_insert.hpp"

#include "bsp/native_alias_count_growth.hpp"
#include "bsp/native_resource_cache_links.hpp"
#include "bsp/native_resource_cache_node.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Actual resource cache storage is Win32.");
void* at(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(storage) + offset);
}
void* volatile& pointer(const void* storage, std::uint32_t offset = 0) noexcept {
    return *static_cast<void* volatile*>(at(storage, offset));
}
std::uint32_t volatile& word(const void* storage, std::uint32_t offset) noexcept {
    return *static_cast<std::uint32_t volatile*>(at(storage, offset));
}
unsigned char volatile& byte(const void* storage, std::uint32_t offset) noexcept {
    return *static_cast<unsigned char volatile*>(at(storage, offset));
}
struct CompletedLengthMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedLengthMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};
void require_node_room(const void* tree) {
    if (word(tree, 8) >= 0x15555554u) {
        NativeLegacySboStringStorage message;
        message.capacity_18 = 15;
        message.length_14 = 0;
        message.buffer_04.inline_bytes[0] = '\0';
        native_legacy_sbo_string_assign_counted_00408720(message, "map/set<T> too long", 19);
        // B7FFD1 state0, after counted assignment; CC2150 destroys only this
        // completed message. Payload ctor/profile/ThrowInfo match4CE780.
        const CompletedLengthMessage completed{message};
        throw NativeAliasListLengthError{message};
    }
}
void* publish_inserted_result(void* output, const void* iterator) noexcept {
    void* const owner = pointer(iterator);
    void* const node = pointer(iterator, 4);
    pointer(output, 4) = node;
    byte(output, 8) = 1;
    pointer(output) = owner;
    return output;
}
} // namespace

void* insert_native_resource_cache_at_00b7ff80(void* tree, void* output,
    std::uint32_t insert_left_word, void* parent, const void* source,
    NativeStringRawPoolContext& strings) {
    require_node_room(tree);
    void* const initial_head = pointer(tree, 4); // B7FFF5, used for BOTH child links.
    void* const inserted = allocate_native_resource_cache_node_00b7f6a0(
        initial_head, parent, initial_head, source, 0, strings);
    void* const head_after_allocation = pointer(tree, 4); // B8000B, BEFORE count write.
    word(tree, 8) = word(tree, 8) + 1u; // B80013; do not recheck the initial bound.
    if (parent == head_after_allocation) {
        pointer(head_after_allocation, 4) = inserted;
        pointer(pointer(tree, 4)) = inserted;
        pointer(pointer(tree, 4), 8) = inserted;
    } else if (static_cast<unsigned char>(insert_left_word) != 0) {
        pointer(parent) = inserted;
        void* const current_head = pointer(tree, 4);
        if (parent == pointer(current_head)) pointer(current_head) = inserted;
    } else {
        pointer(parent, 8) = inserted;
        void* const current_head = pointer(tree, 4);
        if (parent == pointer(current_head, 8)) pointer(current_head, 8) = inserted;
    }

    void* current = inserted;
    while (byte(pointer(current, 4), 0x18) == 0) {
        void* const parent_slot = at(current, 4); // Native EAX, retained across color stores.
        void* const current_parent = pointer(parent_slot);
        void* const grandparent = pointer(current_parent, 4);
        if (current_parent == pointer(grandparent)) {
            void* const uncle = pointer(grandparent, 8);
            if (byte(uncle, 0x18) == 0) {
                byte(current_parent, 0x18) = 1;
                byte(uncle, 0x18) = 1;
                byte(pointer(pointer(parent_slot), 4), 0x18) = 0;
                current = pointer(pointer(parent_slot), 4);
            } else {
                if (current == pointer(current_parent, 8)) {
                    current = current_parent;
                    rotate_native_resource_cache_left_00b7d5f0(tree, current);
                }
                byte(pointer(current, 4), 0x18) = 1;
                byte(pointer(pointer(current, 4), 4), 0x18) = 0;
                rotate_native_resource_cache_right_00b7cbd0(
                    tree, pointer(pointer(current, 4), 4));
            }
        } else {
            void* const uncle = pointer(grandparent);
            if (byte(uncle, 0x18) == 0) {
                byte(current_parent, 0x18) = 1;
                byte(uncle, 0x18) = 1;
                byte(pointer(pointer(parent_slot), 4), 0x18) = 0;
                current = pointer(pointer(parent_slot), 4);
            } else {
                if (current == pointer(current_parent)) {
                    current = current_parent;
                    rotate_native_resource_cache_right_00b7cbd0(tree, current);
                }
                byte(pointer(current, 4), 0x18) = 1;
                byte(pointer(pointer(current, 4), 4), 0x18) = 0;
                // B800F6..B80132 inlines the exact current-field schedule of
                // the existing left-rotation body; this is not another native CALL.
                rotate_native_resource_cache_left_00b7d5f0(
                    tree, pointer(pointer(current, 4), 4));
            }
        }
    }
    byte(pointer(pointer(tree, 4), 4), 0x18) = 1;
    pointer(output, 4) = inserted;
    pointer(output) = tree;
    return output;
}

void* insert_native_resource_cache_unique_00b803b0(void* tree, void* output,
    const void* source, NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& invalid_parameters) {
    void* const initial_head = pointer(tree, 4);
    void* node = pointer(initial_head, 4);
    void* parent = initial_head;
    bool insert_left = true;
    while (byte(node, 0x19) == 0) {
        parent = node;
        // B803D1..B80400 inlines443D00, including right-data-before-left reads.
        insert_left = less_native_string_headers_00443d00(source, at(node, 0x0c));
        node = insert_left ? pointer(node) : pointer(node, 8);
    }
    alignas(4) unsigned char iterator[8];
    pointer(iterator, 4) = parent; // B8041C precedes owner store atB80420.
    pointer(iterator) = tree;
    void* candidate = parent;
    if (insert_left) {
        if (parent == pointer(pointer(tree, 4))) {
            insert_native_resource_cache_at_00b7ff80(tree, iterator, 1, parent, source, strings);
            return publish_inserted_result(output, iterator);
        }
        decrement_native_resource_cache_iterator_00b7cdf0(iterator, invalid_parameters);
        candidate = pointer(iterator, 4);
    }
    if (less_native_string_headers_00443d00(at(candidate, 0x0c), source)) {
        insert_native_resource_cache_at_00b7ff80(tree, iterator,
            static_cast<std::uint32_t>(insert_left), parent, source, strings);
        return publish_inserted_result(output, iterator);
    }
    void* const owner = pointer(iterator); // B804AD, before any output store.
    pointer(output, 4) = candidate;
    byte(output, 8) = 0;
    pointer(output) = owner;
    return output;
}
} // namespace bsp
