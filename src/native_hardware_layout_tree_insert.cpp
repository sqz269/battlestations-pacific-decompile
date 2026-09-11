#include "bsp/native_hardware_layout_tree_insert.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hardware-layout tree insertion requires MSVC Win32.
#endif

namespace bsp {
namespace {

void* volatile& word(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(storage) + offset);
}
volatile std::uint32_t& count(void* tree) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(tree) + 8);
}
volatile std::uint8_t& byte(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint8_t*>(static_cast<unsigned char*>(storage) + offset);
}
void* volatile& left(void* node) noexcept { return word(node, 0); }
void* volatile& parent(void* node) noexcept { return word(node, 4); }
void* volatile& right(void* node) noexcept { return word(node, 8); }
volatile std::uint8_t& color(void* node) noexcept { return byte(node, 0x24); }
bool sentinel(void* node) noexcept { return byte(node, 0x25) != 0; }
void* head(void* tree) noexcept { return word(tree, 4); }
void* key(void* node) noexcept { return static_cast<unsigned char*>(node) + 0x0c; }
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}

struct CompletedLengthMessage {
    NativeLegacySboStringStorage& storage;
    ~CompletedLengthMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(storage); }
};
[[noreturn]] void throw_length_error() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(message, "map/set<T> too long", 19);
    // B2F201 / CBD7C0: only the completed assignment arms string destruction.
    const CompletedLengthMessage completed{message};
    throw NativeHardwareLayoutTreeLengthError{message};
}

// The inlined rotation at B2F326 rereads the replacement's left after the
// first store, unlike the out-of-line helper's captured-child sequence.
void rotate_left_inlined(void* tree, void* node) noexcept {
    auto* const replacement = right(node);
    right(node) = left(replacement);
    auto* const current_child = left(replacement);
    if (!sentinel(current_child)) {
        parent(current_child) = node;
    }
    parent(replacement) = parent(node);
    auto* const current_head = head(tree);
    if (node == parent(current_head)) {
        parent(current_head) = replacement;
    } else {
        auto* const current_parent = parent(node);
        if (node == left(current_parent)) {
            left(current_parent) = replacement;
        } else {
            right(current_parent) = replacement;
        }
    }
    left(replacement) = node;
    parent(node) = replacement;
}

void publish_insert_result(NativeHardwareLayoutTreeInsertResult* output,
    void* owner, void* node, std::uint8_t inserted) noexcept {
    word(output, 4) = node;
    byte(output, 8) = inserted;
    word(output, 0) = owner;
}

} // namespace

NativeHardwareLayoutTreeLengthError::NativeHardwareLayoutTreeLengthError(
    const NativeLegacySboStringStorage& message) {
    construct_native_legacy_logic_error_00411700(storage_, message);
    storage_.native_vtable_00 = 0x00d69260;
}
NativeHardwareLayoutTreeLengthError::NativeHardwareLayoutTreeLengthError(
    const NativeHardwareLayoutTreeLengthError& source) {
    copy_native_legacy_length_error_00411940(storage_, source.storage_);
}
NativeHardwareLayoutTreeLengthError::~NativeHardwareLayoutTreeLengthError() noexcept {
    destroy_native_legacy_logic_error_00411780(storage_);
}

void decrement_native_hardware_layout_iterator_00b20d30(
    NativeHardwareLayoutTreeIterator& iterator, const SingletonLifetimeCallbacks& callbacks) {
    if (word(&iterator, 0) == nullptr) {
        invalid(callbacks);
    }
    auto* node = word(&iterator, 4);
    if (sentinel(node)) {
        node = right(node);
        word(&iterator, 4) = node;
        if (sentinel(node)) {
            invalid(callbacks);
        }
        return;
    }
    auto* child = left(node);
    if (!sentinel(child)) {
        auto* next = right(child);
        while (!sentinel(next)) {
            child = next;
            next = right(child);
        }
        word(&iterator, 4) = child;
        return;
    }
    auto* ancestor = parent(node);
    while (!sentinel(ancestor)) {
        auto* const current = word(&iterator, 4);
        if (current != left(ancestor)) {
            break;
        }
        word(&iterator, 4) = ancestor;
        ancestor = parent(ancestor);
    }
    node = word(&iterator, 4);
    if (sentinel(node)) {
        invalid(callbacks);
        return;
    }
    word(&iterator, 4) = ancestor;
}

void* initialize_native_hardware_layout_node_00b28370(
    void* node, void* left_node, void* parent_node, void* right_node,
    const void* pair, std::uint8_t node_color) noexcept {
    left(node) = left_node;
    right(node) = right_node;
    parent(node) = parent_node;
    copy_native_hardware_layout_pair_00b25ef0(key(node), pair);
    color(node) = node_color;
    byte(node, 0x25) = 0;
    return node;
}

void* allocate_native_hardware_layout_node_00b29cd0(
    void* left_node, void* parent_node, void* right_node,
    const void* pair, std::uint8_t node_color) {
    auto* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x28, 0x28});
    if (node != nullptr) {
        initialize_native_hardware_layout_node_00b28370(
            node, left_node, parent_node, right_node, pair, node_color);
    }
    return node;
}

NativeHardwareLayoutTreeIterator* link_native_hardware_layout_node_00b2f1b0(
    void* tree, NativeHardwareLayoutTreeIterator* output,
    std::uint8_t insert_left, void* parent_node, const void* pair) {
    if (count(tree) >= 0x0aaaaaa9u) {
        throw_length_error();
    }
    auto* const allocated_head = head(tree);
    auto* const node = allocate_native_hardware_layout_node_00b29cd0(
        allocated_head, parent_node, allocated_head, pair, 0);
    auto* const current_head = head(tree);
    count(tree) = count(tree) + 1u;
    if (parent_node == current_head) {
        parent(current_head) = node;
        left(head(tree)) = node;
        right(head(tree)) = node;
    } else if (insert_left != 0) {
        left(parent_node) = node;
        auto* const current = head(tree);
        if (parent_node == left(current)) {
            left(current) = node;
        }
    } else {
        right(parent_node) = node;
        auto* const current = head(tree);
        if (parent_node == right(current)) {
            right(current) = node;
        }
    }
    auto* repair = node;
    while (color(parent(repair)) == 0) {
        auto* const direct_parent = parent(repair);
        auto* const grandparent = parent(direct_parent);
        if (direct_parent == left(grandparent)) {
            auto* const uncle = right(grandparent);
            if (color(uncle) == 0) {
                color(direct_parent) = 1;
                color(uncle) = 1;
                color(parent(parent(repair))) = 0;
                repair = parent(parent(repair));
            } else {
                if (repair == right(direct_parent)) {
                    repair = direct_parent;
                    rotate_native_hardware_layout_left_00b22bd0(tree, repair);
                }
                color(parent(repair)) = 1;
                color(parent(parent(repair))) = 0;
                rotate_native_hardware_layout_right_00b20910(tree, parent(parent(repair)));
            }
        } else {
            auto* const uncle = left(grandparent);
            if (color(uncle) == 0) {
                color(direct_parent) = 1;
                color(uncle) = 1;
                color(parent(parent(repair))) = 0;
                repair = parent(parent(repair));
            } else {
                if (repair == left(direct_parent)) {
                    repair = direct_parent;
                    rotate_native_hardware_layout_right_00b20910(tree, repair);
                }
                color(parent(repair)) = 1;
                color(parent(parent(repair))) = 0;
                rotate_left_inlined(tree, parent(parent(repair)));
            }
        }
    }
    color(parent(head(tree))) = 1;
    word(output, 4) = node;
    word(output, 0) = tree;
    return output;
}

NativeHardwareLayoutTreeInsertResult* insert_native_hardware_layout_pair_00b2f540(
    void* tree, NativeHardwareLayoutTreeInsertResult* output,
    const void* pair, const SingletonLifetimeCallbacks& callbacks) {
    auto* selected_parent = head(tree);
    auto* node = parent(selected_parent);
    bool insert_left = true;
    while (!sentinel(node)) {
        selected_parent = node;
        insert_left = less_native_hardware_layout_key_00b20bf0(pair, key(node));
        node = insert_left ? left(node) : right(node);
    }
    NativeHardwareLayoutTreeIterator predecessor{tree, selected_parent};
    if (insert_left) {
        if (selected_parent == left(head(tree))) {
            link_native_hardware_layout_node_00b2f1b0(tree, &predecessor, 1, selected_parent, pair);
            publish_insert_result(output, predecessor.owner, predecessor.node, 1);
            return output;
        }
        decrement_native_hardware_layout_iterator_00b20d30(predecessor, callbacks);
    }
    node = predecessor.node;
    if (less_native_hardware_layout_key_00b20bf0(key(node), pair)) {
        link_native_hardware_layout_node_00b2f1b0(
            tree, &predecessor, static_cast<std::uint8_t>(insert_left), selected_parent, pair);
        publish_insert_result(output, predecessor.owner, predecessor.node, 1);
    } else {
        publish_insert_result(output, predecessor.owner, node, 0);
    }
    return output;
}

} // namespace bsp
