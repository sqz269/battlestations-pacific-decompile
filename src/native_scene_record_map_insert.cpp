#include "bsp/native_scene_record_map_insert.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_scene_record_map_iterator.hpp"
#include "bsp/native_scene_record_map_rotations.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"

namespace bsp {
namespace {
using Access = detail::TreeInsertAccess<0x18, 0x19>;
std::uint32_t bits(void* value) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}
struct CompletedMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};
[[noreturn]] void throw_length_error() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = 0;
    native_legacy_sbo_string_assign_counted_00408720(message, "map/set<T> too long", 19);
    const CompletedMessage completed{message}; // C61CB0: arm only after assign.
    throw NativeHardwareLayoutTreeLengthError{message};
}
void prepare_link(void* arguments, void* predecessor, std::uint32_t left,
                  void* parent, void* pair) noexcept {
    Access::word(arguments, 12) = pair;
    Access::word(arguments, 8) = parent;
    *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(arguments) + 4) = left;
    Access::word(arguments, 0) = predecessor;
}
void* publish_inserted(void* outer_arguments, void* iterator) noexcept {
    void* owner = Access::word(iterator, 0);
    void* output = Access::word(outer_arguments, 0);
    void* node = Access::word(iterator, 4);
    Access::word(output, 4) = node;
    Access::byte(output, 8) = 1;
    Access::word(output, 0) = owner;
    return output;
}
} // namespace

void* link_native_scene_record_map_node_0046ad70(void* tree, void* arguments,
    const NativeSceneRecordMapNodeAllocationFrame& frame,
    NativeStringRawPoolContext& pool, NativeSceneRecordMapNodeAcquired& acquired) {
    if (Access::count(tree) >= 0x15555554u) throw_length_error();
    void* pair = Access::word(arguments, 12);
    void* head = Access::head(tree);
    void* parent = Access::word(arguments, 8);
    frame.incoming.color = 0;
    frame.incoming.pair = bits(pair);
    frame.incoming.right = bits(head);
    frame.incoming.parent = bits(parent);
    frame.incoming.left = bits(head);
    void* node = allocate_native_scene_record_map_node_0046a340(frame, pool, acquired);
    head = Access::head(tree);
    Access::count(tree) = Access::count(tree) + 1u;
    if (parent == head) {
        Access::parent(head) = node;
        Access::left(Access::head(tree)) = node;
        Access::right(Access::head(tree)) = node;
    } else if (Access::byte(arguments, 4) != 0) {
        Access::left(parent) = node;
        head = Access::head(tree);
        if (parent == Access::left(head)) Access::left(head) = node;
    } else {
        Access::right(parent) = node;
        head = Access::head(tree);
        if (parent == Access::right(head)) Access::right(head) = node;
    }
    detail::rebalance_linked_tree_node<Access>(tree, node,
        [](void* map, void* pivot) { rotate_left_native_scene_record_map_00467620(map, 0, pivot); },
        [](void* map, void* pivot) { rotate_right_native_scene_record_map_00467670(map, 0, pivot); });
    void* output = Access::word(arguments, 0);
    Access::word(output, 4) = node;
    Access::word(output, 0) = tree;
    return output;
}

void* insert_native_scene_record_map_pair_0046b340(void* tree, void* arguments,
    const NativeSceneRecordMapInsertFrame& frame,
    NativeStringRawPoolContext& pool, NativeSceneRecordMapNodeAcquired& acquired) {
    void* pair = Access::word(arguments, 4);
    void* parent = Access::head(tree);
    void* node = Access::parent(parent);
    bool is_nil = Access::sentinel(node);
    bool left = true;
    Access::byte(frame.descent_flag4, 0) = 1;
    while (!is_nil) {
        parent = node;
        left = less_native_string_headers_00443d00(pair, static_cast<unsigned char*>(node) + 12);
        Access::byte(frame.descent_flag4, 0) = static_cast<std::uint8_t>(left);
        node = left ? Access::left(node) : Access::right(node);
        is_nil = Access::sentinel(node);
    }
    void* predecessor_node = parent;
    Access::word(frame.predecessor8, 4) = predecessor_node;
    Access::word(frame.predecessor8, 0) = tree;
    if (left) {
        if (parent == Access::left(Access::head(tree))) {
            prepare_link(frame.linker_arguments16, frame.predecessor8, 1, parent, pair);
            void* iterator = link_native_scene_record_map_node_0046ad70(tree,
                frame.linker_arguments16, frame.allocation, pool, acquired);
            return publish_inserted(arguments, iterator);
        }
        decrement_native_scene_record_map_iterator_004678f0(frame.predecessor8);
        predecessor_node = Access::word(frame.predecessor8, 4);
    }
    if (less_native_string_headers_00443d00(
            static_cast<unsigned char*>(predecessor_node) + 12, pair)) {
        const auto left_word = *reinterpret_cast<volatile std::uint32_t*>(frame.descent_flag4);
        prepare_link(frame.linker_arguments16, frame.predecessor8, left_word, parent, pair);
        void* iterator = link_native_scene_record_map_node_0046ad70(tree,
            frame.linker_arguments16, frame.allocation, pool, acquired);
        return publish_inserted(arguments, iterator);
    }
    void* output = Access::word(arguments, 0);
    void* owner = Access::word(frame.predecessor8, 0);
    Access::word(output, 4) = predecessor_node;
    Access::byte(output, 8) = 0;
    Access::word(output, 0) = owner;
    return output;
}
} // namespace bsp
