#include "bsp/native_session_deferred_message_storage.hpp"
#include "bsp/native_alias_count_growth.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Session deferred-message storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Node = NativeSessionDeferredMessageNode;
static_assert(sizeof(void*) == 4 && sizeof(Node) == 12);
static_assert(offsetof(Node, previous_04) == 4 && offsetof(Node, message_08) == 8);
template<class T> volatile T& field(void* base, Word offset) noexcept {
    return *reinterpret_cast<volatile T*>(reinterpret_cast<Word>(base) + offset);
}
Node* allocate_node() {
    return static_cast<Node*>(singleton_lifetime_allocate({
        SingletonAllocationKind::object, 12, 12}));
}
} // namespace

Node* allocate_native_session_deferred_node_0077bac0(
    Node* next, Node* previous,
    NativeSessionMessageStorage* const volatile* message_pointer_cell) {
    Node* const node = allocate_node();
    if (reinterpret_cast<Word>(node) != 0) field<Node*>(node, 0) = next;
    if (reinterpret_cast<Word>(node) + 4u != 0) field<Node*>(node, 4) = previous;
    if (reinterpret_cast<Word>(node) + 8u != 0) {
        field<NativeSessionMessageStorage*>(node, 8) = *message_pointer_cell;
    }
    return node;
}

Node* create_native_session_deferred_head_00780830() {
    Node* const node = allocate_node();
    if (reinterpret_cast<Word>(node) != 0) field<Node*>(node, 0) = node;
    if (reinterpret_cast<Word>(node) + 4u != 0) field<Node*>(node, 4) = node;
    return node;
}

void* construct_native_session_deferred_storage_007808c0(void* actual_owner) {
    Node* const head = create_native_session_deferred_head_00780830();
    field<Node*>(actual_owner, 4) = head;
    field<Word>(actual_owner, 8) = 0;
    return actual_owner;
}

void grow_native_session_deferred_count_0077dda0(void* actual_owner, Word increment) {
    // The complete 147-byte bodies, bound, +8 count, 36-byte FuncInfo, one-state
    // unwind maps, and temporary-string funclets agree after address rebasing.
    grow_native_unit_registry_count_004cee30(actual_owner, increment);
}

void destroy_native_session_deferred_storage_00780860(void* actual_owner) {
    Node* const head = field<Node*>(actual_owner, 4);
    Node* current = field<Node*>(head, 0);
    field<Node*>(head, 0) = head;
    Node* const current_head = field<Node*>(actual_owner, 4);
    field<Node*>(current_head, 4) = current_head;
    const bool has_nodes = current != field<Node*>(actual_owner, 4);
    field<Word>(actual_owner, 8) = 0;
    if (has_nodes) {
        do {
            Node* const next = field<Node*>(current, 0);
            singleton_lifetime_free(current);
            const bool finished = next == field<Node*>(actual_owner, 4);
            current = next;
            if (finished) break;
        } while (true);
    }
    singleton_lifetime_free(field<Node*>(actual_owner, 4));
    field<Node*>(actual_owner, 4) = nullptr;
}
} // namespace bsp
