#include "bsp/native_render_alias_insertion.hpp"

#include "bsp/native_alias_count_growth.hpp"
#include "bsp/native_render_resource_alias_nodes.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native alias insertion requires MSVC Win32 pointer widths.
#endif

namespace bsp {
namespace {
NativeRenderResourceAliasNode* sentinel(void* actual_owner) {
    return *reinterpret_cast<NativeRenderResourceAliasNode* volatile*>(
        static_cast<unsigned char*>(actual_owner) + 4);
}
}

void append_native_render_alias_004d2660(void* actual_destination_owner,
    const void* actual_source_string_header, SizedStoragePool& actual_string_pool) {
    auto* const captured_sentinel = sentinel(actual_destination_owner);
    volatile auto* const end = captured_sentinel;
    auto* const previous = end->previous_04;
    auto* const inserted = allocate_native_render_alias_node_004ce6f0(
        captured_sentinel, previous, actual_source_string_header, actual_string_pool);
    grow_native_alias_list_count_004ce780(actual_destination_owner, 1);
    end->previous_04 = inserted;
    volatile auto* const current_inserted = inserted;
    volatile auto* const current_previous = current_inserted->previous_04;
    current_previous->next_00 = inserted;
}

void insert_native_render_alias_range_004d26a0(void* actual_destination_owner,
    NativeRenderAliasIterator insertion_position_by_value,
    NativeRenderAliasIterator source_by_value, NativeRenderAliasIterator end_by_value,
    SizedStoragePool& actual_string_pool, const SingletonLifetimeCallbacks& callbacks) {
    volatile auto& position = insertion_position_by_value;
    volatile auto& source = source_by_value;
    volatile auto& source_end = end_by_value;
    auto* const fixed_insertion_node = position.node_04;
    void* captured_source_owner = source.owner_00;
    auto* captured_source_node = source.node_04;
    volatile NativeRenderAliasIterator initial_source{
        captured_source_owner, captured_source_node};
    try {
        for (;;) {
            if (!captured_source_owner || captured_source_owner != source_end.owner_00) {
                callbacks.invalid_parameter(callbacks.context);
            }
            if (captured_source_node == source_end.node_04) {
                break;
            }
            if (!captured_source_owner) {
                callbacks.invalid_parameter(callbacks.context);
            }
            if (captured_source_node == sentinel(captured_source_owner)) {
                callbacks.invalid_parameter(callbacks.context);
            }

            volatile auto* const insertion_node = fixed_insertion_node;
            auto* const previous = insertion_node->previous_04;
            auto* const inserted = allocate_native_render_alias_node_004ce6f0(
                fixed_insertion_node, previous,
                reinterpret_cast<unsigned char*>(captured_source_node) + 8,
                actual_string_pool);
            grow_native_alias_list_count_004ce780(actual_destination_owner, 1);

            // 004D2727 captures the source owner before publishing either link.
            void* const advance_owner = source.owner_00;
            insertion_node->previous_04 = inserted;
            volatile auto* const current_inserted = inserted;
            volatile auto* const current_previous = current_inserted->previous_04;
            current_previous->next_00 = inserted;
            if (captured_source_node == sentinel(advance_owner)) {
                callbacks.invalid_parameter(callbacks.context);
            }
            volatile auto* const current_source_node = captured_source_node;
            captured_source_node = current_source_node->next_00;
            captured_source_owner = source.owner_00;
            source.node_04 = captured_source_node;
        }
    } catch (...) {
        if (native_render_alias_iterator_not_equal_004be820(initial_source, source, callbacks)) {
            // Captured anew in the catch, not taken from the normal-loop register.
            auto* const rollback_position_node = position.node_04;
            void* const rollback_position_owner = position.owner_00;
            do {
                position.owner_00 = rollback_position_owner;
                position.node_04 = rollback_position_node;
                auto* const previous = native_render_alias_iterator_previous_004becc0(
                    position, callbacks);
                auto* const erase_node = previous->node_04;
                void* const erase_owner = previous->owner_00;
                erase_native_render_alias_node_004d0990(actual_destination_owner,
                    source_end, {erase_owner, erase_node}, actual_string_pool, callbacks);
                native_render_alias_iterator_next_004b9ff0(initial_source, callbacks);
            } while (native_render_alias_iterator_not_equal_004be820(
                initial_source, source, callbacks));
        }
        throw;
    }
}

} // namespace bsp
