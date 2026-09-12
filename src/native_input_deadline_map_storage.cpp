#include "bsp/native_input_deadline_map_storage.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>

namespace bsp {
namespace {
using Access = detail::TreeInsertAccess<0x14, 0x15>;
std::uint32_t read_word(const void* p, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<const unsigned char*>(p) + offset);
}
void write_word(void* p, std::size_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(p) + offset) = value;
}
void invalid() { _invalid_parameter_noinfo(); }
struct CompletedMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};
[[noreturn]] void throw_length() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(message, "map/set<T> too long", 19);
    const CompletedMessage completed{message};
    throw NativeHardwareLayoutTreeLengthError{message};
}
void rotate_left(void* tree, void* node) {
    rotate_left_native_int_pointer_tree18_0086a2f0(tree, nullptr, node);
}
void rotate_right(void* tree, void* node) {
    rotate_right_native_int_pointer_tree18_00869810(tree, nullptr, node);
}
} // namespace

void* allocate_input_deadline_map_node(void* left, void* parent, void* right,
    const void* pair, std::uint8_t color) {
    void* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x18, 0x18});
    if (node) {
        Access::left(node) = left;
        Access::right(node) = right;
        Access::parent(node) = parent;
        write_word(node, 0x0c, read_word(pair, 0));
        write_word(node, 0x10, read_word(pair, 4));
        Access::color(node) = color;
        Access::byte(node, 0x15) = 0;
    }
    return node;
}

void decrement_input_deadline_map_iterator(NativeInputDeadlineMapIterator& it) {
    detail::decrement_tree_iterator<Access>(&it, invalid);
}

bool equal_input_deadline_map_iterators(const NativeInputDeadlineMapIterator& a,
    const NativeInputDeadlineMapIterator& b) {
    // Capture only owner for the pre-handler comparison. Nodes remain current.
    const auto owner = read_word(&a, 0);
    if (owner == 0 || owner != read_word(&b, 0)) invalid();
    const auto node = read_word(&a, 4);
    return node == read_word(&b, 4);
}

NativeInputDeadlineMapIterator* link_input_deadline_map_node(void* tree,
    NativeInputDeadlineMapIterator* output, std::uint8_t insert_left,
    void* parent, const void* pair) {
    detail::link_tree_node<Access>(tree, output, insert_left, parent, pair, 0x1ffffffeu,
        allocate_input_deadline_map_node, rotate_left, rotate_right, throw_length);
    return output;
}
} // namespace bsp
