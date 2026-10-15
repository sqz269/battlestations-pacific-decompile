#include "bsp/native_scene_integer_set_insert.hpp"

#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_scene_integer_set_iterator.hpp"
#include "bsp/native_scene_integer_set_node.hpp"
#include "bsp/native_scene_integer_set_rotations.hpp"

namespace bsp {
namespace {
using Access = detail::TreeInsertAccess<0x10, 0x11>;
static_assert(sizeof(void*) == 4);

std::int32_t signed_key(const void* storage, std::uint32_t offset = 0) noexcept {
    const auto bits = reinterpret_cast<std::uintptr_t>(storage) + offset;
    return *reinterpret_cast<const volatile std::int32_t*>(bits);
}
struct CompletedMessage {
    NativeLegacySboStringStorage& message;
    ~CompletedMessage() noexcept {
        //00C61C10: completed temporary at native frame-50h only.
        native_legacy_sbo_string_destroy_004072d0(message);
    }
};
[[noreturn]] void throw_length_error() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = 0;
    native_legacy_sbo_string_assign_counted_00408720(
        message, "map/set<T> too long", 19);
    const CompletedMessage completed{message}; // Arm only after assignment.
    throw NativeHardwareLayoutTreeLengthError{message};
}
void prepare_link(void* parameters, void* predecessor, std::uint32_t left,
    void* parent, void* key) noexcept {
    Access::word(parameters, 12) = key;
    Access::word(parameters, 8) = parent;
    *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(parameters) + 4) = left;
    Access::word(parameters, 0) = predecessor;
}
void* publish_inserted(void* outer_parameters, void* iterator) noexcept {
    void* owner = Access::word(iterator, 0);
    void* output = Access::word(outer_parameters, 0);
    void* node = Access::word(iterator, 4);
    Access::word(output, 0) = owner;
    Access::word(output, 4) = node;
    Access::byte(output, 8) = 1;
    return output;
}
} // namespace

void* link_native_scene_integer_set_node_0046a150(void* tree,
    void* parameters, void* node_parameters) {
    if (Access::count(tree) >= 0x3ffffffeu) throw_length_error();
    void* key = Access::word(parameters, 12);
    void* head = Access::head(tree);
    void* parent = Access::word(parameters, 8);
    *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(node_parameters) + 16) = 0;
    Access::word(node_parameters, 12) = key;
    Access::word(node_parameters, 8) = head;
    Access::word(node_parameters, 4) = parent;
    Access::word(node_parameters, 0) = head;
    void* node = allocate_native_scene_integer_set_node_004681b0(node_parameters);
    head = Access::head(tree);
    Access::count(tree) = Access::count(tree) + 1u;
    if (parent == head) {
        Access::parent(head) = node;
        Access::left(Access::head(tree)) = node;
        Access::right(Access::head(tree)) = node;
    } else if (Access::byte(parameters, 4) != 0) {
        Access::left(parent) = node;
        head = Access::head(tree);
        if (parent == Access::left(head)) Access::left(head) = node;
    } else {
        Access::right(parent) = node;
        head = Access::head(tree);
        if (parent == Access::right(head)) Access::right(head) = node;
    }
    // Existing actual-storage repair,including the final inlined left mirror.
    // These local static adapters bind concrete providers,not host callbacks.
    detail::rebalance_linked_tree_node<Access>(tree, node,
        [](void* actual_tree, void* pivot) {
            rotate_left_native_scene_integer_set_00467740(actual_tree, 0, pivot);
        },
        [](void* actual_tree, void* pivot) {
            rotate_right_native_scene_integer_set_00467790(actual_tree, 0, pivot);
        });
    void* output = Access::word(parameters, 0);
    Access::word(output, 4) = node;
    Access::word(output, 0) = tree;
    return output;
}

void* insert_native_scene_integer_set_key_0046ac50(void* tree,
    void* parameters, const NativeSceneIntegerSetInsertFrame& frame) {
    void* key = Access::word(parameters, 4);
    void* parent = Access::head(tree);
    void* node = Access::parent(parent);
    bool is_nil = Access::sentinel(node);
    bool left = true;
    Access::byte(frame.descent_flag4, 0) = 1;
    if (!is_nil) {
        const std::int32_t captured_search_key = signed_key(key);
        do {
            const std::int32_t candidate_key = signed_key(node, 12);
            parent = node;
            left = captured_search_key < candidate_key;
            Access::byte(frame.descent_flag4, 0) = static_cast<std::uint8_t>(left);
            node = left ? Access::left(node) : Access::right(node);
            is_nil = Access::sentinel(node);
        } while (!is_nil);
    }
    void* predecessor_node = parent;
    Access::word(frame.predecessor8, 4) = predecessor_node;
    Access::word(frame.predecessor8, 0) = tree;
    if (left) {
        if (parent == Access::left(Access::head(tree))) {
            prepare_link(frame.linker_parameters16, frame.predecessor8, 1, parent, key);
            void* iterator = link_native_scene_integer_set_node_0046a150(tree,
                frame.linker_parameters16, frame.node_parameters20);
            return publish_inserted(parameters, iterator);
        }
        decrement_native_scene_integer_set_iterator_004679f0(frame.predecessor8);
        predecessor_node = Access::word(frame.predecessor8, 4);
    }
    const std::int32_t predecessor_key = signed_key(predecessor_node, 12);
    const std::int32_t current_key = signed_key(key);
    if (predecessor_key < current_key) {
        const auto left_word = *reinterpret_cast<const volatile std::uint32_t*>(frame.descent_flag4);
        prepare_link(frame.linker_parameters16, frame.predecessor8, left_word, parent, key);
        void* iterator = link_native_scene_integer_set_node_0046a150(tree,
            frame.linker_parameters16, frame.node_parameters20);
        return publish_inserted(parameters, iterator);
    }
    void* output = Access::word(parameters, 0);
    void* owner = Access::word(frame.predecessor8, 0);
    Access::word(output, 0) = owner;
    Access::word(output, 4) = predecessor_node;
    Access::byte(output, 8) = 0;
    return output;
}
} // namespace bsp
