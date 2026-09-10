#include "bsp/native_render_alias_checked_ops.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render alias checked operations require MSVC Win32 pointer widths.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderAliasIterator) == 8);
static_assert(offsetof(NativeRenderAliasIterator, owner_00) == 0);
static_assert(offsetof(NativeRenderAliasIterator, node_04) == 4);
static_assert(sizeof(NativeRenderResourceAliasNode) == 0x10);
static_assert(offsetof(NativeRenderResourceAliasNode, next_00) == 0);
static_assert(offsetof(NativeRenderResourceAliasNode, previous_04) == 4);
static_assert(offsetof(NativeRenderResourceAliasNode, string_length_08) == 8);
static_assert(offsetof(NativeRenderResourceAliasNode, string_data_0c) == 0x0c);
static_assert(offsetof(NativeRenderResourceRecord, unknown_08) == 8);
static_assert(offsetof(NativeRenderResourceRecord, sentinel_0c) == 0x0c);
static_assert(offsetof(NativeRenderResourceRecord, alias_count_10) == 0x10);

namespace {
// Access the actual list fields through their native addresses. Volatile
// preserves the instruction-ordered loads/stores, including overlapping
// native storage and changes made by a returning validation/allocation call.
template<class T>
volatile T& list_field(void* actual_owner, std::size_t offset) {
    return *reinterpret_cast<volatile T*>(
        static_cast<unsigned char*>(actual_owner) + offset);
}

NativeRenderResourceAliasNode* sentinel(void* actual_owner) {
    return list_field<NativeRenderResourceAliasNode*>(actual_owner, 4);
}
}

bool native_render_alias_iterator_not_equal_004be820(
    const volatile NativeRenderAliasIterator& left,
    const volatile NativeRenderAliasIterator& right,
    const SingletonLifetimeCallbacks& callbacks) {
    void* const initial_left_owner = left.owner_00;
    if (!initial_left_owner || initial_left_owner != right.owner_00) {
        callbacks.invalid_parameter(callbacks.context);
    }
    // The native routine does not validate the owners again after a return.
    auto* const current_left_node = left.node_04;
    return current_left_node != right.node_04;
}

volatile NativeRenderAliasIterator* native_render_alias_iterator_previous_004becc0(
    volatile NativeRenderAliasIterator& iterator,
    const SingletonLifetimeCallbacks& callbacks) {
    if (!iterator.owner_00) {
        callbacks.invalid_parameter(callbacks.context);
    }
    volatile auto* const current_node = iterator.node_04;
    auto* const previous = current_node->previous_04;
    void* const current_owner = iterator.owner_00;
    iterator.node_04 = previous;
    if (previous == sentinel(current_owner)) {
        callbacks.invalid_parameter(callbacks.context);
    }
    return &iterator;
}

volatile NativeRenderAliasIterator* native_render_alias_iterator_next_004b9ff0(
    volatile NativeRenderAliasIterator& iterator,
    const SingletonLifetimeCallbacks& callbacks) {
    if (!iterator.owner_00) {
        callbacks.invalid_parameter(callbacks.context);
    }
    void* const current_owner = iterator.owner_00;
    auto* const checked_node = iterator.node_04;
    if (checked_node == sentinel(current_owner)) {
        callbacks.invalid_parameter(callbacks.context);
    }
    volatile auto* const current_node = iterator.node_04;
    iterator.node_04 = current_node->next_00;
    return &iterator;
}

volatile NativeRenderAliasIterator* erase_native_render_alias_node_004d0990(
    void* actual_destination_owner, volatile NativeRenderAliasIterator& output,
    NativeRenderAliasIterator input_by_value, SizedStoragePool& actual_string_pool,
    const SingletonLifetimeCallbacks& callbacks) {
    void* const input_owner = input_by_value.owner_00;
    auto* const input_node = input_by_value.node_04;
    volatile auto* const node = input_node;
    if (!input_owner) {
        callbacks.invalid_parameter(callbacks.context);
    }
    if (input_node == sentinel(input_owner)) {
        callbacks.invalid_parameter(callbacks.context);
    }

    const bool is_destination_sentinel =
        input_node == sentinel(actual_destination_owner);
    auto* const captured_next = node->next_00;
    if (!is_destination_sentinel) {
        volatile auto* const previous = node->previous_04;
        previous->next_00 = captured_next;
        // 004D09C6/004D09C8 reload both links after the first write.
        volatile auto* const reloaded_next = node->next_00;
        auto* const reloaded_previous = node->previous_04;
        reloaded_next->previous_04 = reloaded_previous;

        auto* const data = node->string_data_0c;
        if (data) {
            const auto release_size = node->string_length_08 + 1u;
            actual_string_pool.release_00bd1510(data, release_size);
        }
        singleton_lifetime_free(input_node);
        // 004D09F1..004D09F8 is hidden by the false _free no-return annotation.
        auto& count = list_field<std::uint32_t>(actual_destination_owner, 8);
        count = count - 1u;
    }
    output.node_04 = captured_next;
    output.owner_00 = input_owner;
    return &output;
}

}
