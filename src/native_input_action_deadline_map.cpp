#include "bsp/native_input_action_deadline_map.hpp"
#include "bsp/native_int_pointer_tree18_erase.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace bsp {
namespace {
void* bound_tree; // Borrowed identity only; no copied header or second owner.
template<class T> volatile T& field(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(static_cast<std::byte*>(storage) + offset);
}
struct Iterator { void* owner; void* node; };
} // namespace

void bind_static_native_input_action_deadline_map_00e18a7c(void* actual_tree) noexcept {
    bound_tree = actual_tree;
}

int initialize_static_native_input_action_deadline_map_00cc9e30() {
    void* const tree = bound_tree;
    // 4C27A0 and existing86AC00 are identical except the allocation CALL.
    void* const allocated = allocate_native_int_pointer_tree18_node_0086ac00();
    field<void*>(tree, 4) = allocated;
    field<std::uint8_t>(allocated, 0x15) = 1;
    void* head = field<void*>(tree, 4);
    field<void*>(head, 4) = head;
    head = field<void*>(tree, 4);
    field<void*>(head, 0) = head;
    head = field<void*>(tree, 4);
    field<void*>(head, 8) = head;
    field<std::uint32_t>(tree, 8) = 0;
    return std::atexit(&destroy_static_native_input_action_deadline_map_00cd9ec0);
}

void destroy_static_native_input_action_deadline_map_00cd9ec0() {
    void* const tree = bound_tree;
    void* const head = field<void*>(tree, 4);
    void* const minimum = field<void*>(head, 0);
    Iterator ignored;
    // 4D2000's full-range path and recursive4C18D0 match existing86EE50/
    // 86AA60. Values are scalar words; the leaf never destroys payloads.
    erase_native_int_pointer_tree18_range_0086ee50(
        tree, nullptr, &ignored, tree, minimum, tree, head);
    singleton_lifetime_free(field<void*>(tree, 4));
    field<void*>(tree, 4) = nullptr;
    field<std::uint32_t>(tree, 8) = 0;
}
} // namespace bsp
