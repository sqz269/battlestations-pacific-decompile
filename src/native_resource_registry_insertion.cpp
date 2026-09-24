#include "bsp/native_resource_registry_insertion.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_resource_cache_leaves.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw resource registry insertion requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Access = detail::TreeInsertAccess<0x18, 0x19>;
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* ptr(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const volatile void* p, Word offset) noexcept { return ptr(bits(p) + offset); }
Word read(const void* p, Word byte_offset = 0) noexcept {
    Word result;
    __asm {
        mov eax, p
        add eax, byte_offset
        mov ecx, [eax]
        mov result, ecx
    }
    return result;
}
void write(void* p, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, p
        add eax, byte_offset
        mov ecx, value
        mov [eax], ecx
    }
}
unsigned char byte(const volatile void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile unsigned char*>(at(p, offset));
}
void put_byte(volatile void* p, Word offset, unsigned char value) noexcept {
    *static_cast<volatile unsigned char*>(at(p, offset)) = value;
}
bool nil(Word node) noexcept { return byte(ptr(node), 0x19) != 0; }
bool less(Word left, Word right) {
    return less_native_string_headers_00443d00(ptr(left), ptr(right));
}
void invalid(NativeResourceRegistryInsertionContext& context) {
    context.validation.invalid_parameter(context.validation.context);
}
void begin(bool& started) {
    if (started) throw std::logic_error("resource registry insertion acquisition is not fresh");
    started = true;
}
struct CompletedLengthMessage {
    NativeLegacySboStringStorage& message;
    int& state;
    ~CompletedLengthMessage() noexcept {
        state = -1;
        native_legacy_sbo_string_destroy_004072d0(message); // CBC6F3
    }
};
void publish_result(void* output, Word owner, Word node, unsigned char inserted) noexcept {
    write(output, 4, node);
    put_byte(output, 8, inserted);
    write(output, 0, owner);
}
// The pair has already been captured by the hint path. Native pushes pair,
// then reads current outer output, then pushes parent/side/output. Returning
// the captured outer output is intentional even if the nested result changes.
void* hint_link(void* tree, Word pair, Word parent, Word side,
    const NativeResourceRegistryHintInsertFrame& frame,
    NativeResourceRegistryInsertionContext& context,
    NativeResourceRegistryHintInsertAcquired& acquired) {
    frame.link.pair_argument = pair;
    const Word output = frame.output_argument;
    frame.link.parent_argument = parent;
    frame.link.left_argument = side;
    frame.link.output_argument = output;
    link_native_resource_registry_node_00b1ada0(tree, frame.link, context, acquired.direct);
    return ptr(output);
}
}

void* link_native_resource_registry_node_00b1ada0(void* tree,
    const NativeResourceRegistryLinkFrame& frame, NativeResourceRegistryInsertionContext& context,
    NativeResourceRegistryLinkAcquired& acquired) {
    begin(acquired.started);
    if (read(tree, 8) >= 0x15555554u) {
        auto& message = frame.length_message;
        message.capacity_18 = 15;
        message.length_14 = 0;
        message.buffer_04.inline_bytes[0] = '\0';
        native_legacy_sbo_string_assign_counted_00408720(message, context.text_00ce47bc, 19); // B1ADE3
        acquired.length_eh_state = 0;
        const CompletedLengthMessage completed{message, acquired.length_eh_state};
        throw NativeHardwareLayoutTreeLengthError{message}; // B1ADF5, B1AE0C; source transport
    }
    const Word pair = frame.pair_argument;
    const Word head = read(tree, 4);
    const Word parent = frame.parent_argument;
    frame.allocation.incoming.color = 0;
    frame.allocation.incoming.pair = pair;
    frame.allocation.incoming.right = head;
    frame.allocation.incoming.parent = parent;
    frame.allocation.incoming.left = head;
    void* const node = allocate_native_resource_registry_node_00b1ad10(
        frame.allocation, context.strings, acquired.node); // B1AE24
    const Word current_head = read(tree, 4);
    write(tree, 8, read(tree, 8) + 1u);
    if (parent == current_head) {
        write(ptr(current_head), 4, bits(node));
        write(ptr(read(tree, 4)), 0, bits(node));
        write(ptr(read(tree, 4)), 8, bits(node));
    } else if (byte(&frame.left_argument) != 0) { // B1AE4A, AFTER allocation
        write(ptr(parent), 0, bits(node));
        const Word current = read(tree, 4);
        if (parent == read(ptr(current))) write(ptr(current), 0, bits(node));
    } else {
        write(ptr(parent), 8, bits(node));
        const Word current = read(tree, 4);
        if (parent == read(ptr(current), 8)) write(ptr(current), 8, bits(node));
    }
    acquired.linked = true;
    detail::rebalance_linked_tree_node<Access>(tree, node,
        rotate_native_resource_registry_left_00b19830,
        rotate_native_resource_registry_right_00b19640); // native three calls + mirrored inline left
    void* const output = ptr(frame.output_argument); // B1AF72, AFTER root-black store
    write(output, 4, bits(node));
    write(output, 0, bits(tree));
    acquired.result_published = true;
    return output;
}

void* insert_native_resource_registry_pair_00b1af90(void* tree,
    const NativeResourceRegistryPairInsertFrame& frame, NativeResourceRegistryInsertionContext& context,
    NativeResourceRegistryPairInsertAcquired& acquired) {
    begin(acquired.started);
    const Word pair = frame.pair_argument;
    const Word head = read(tree, 4);
    Word node = read(ptr(head), 4);
    Word parent = head;
    bool insert_left = true;
    put_byte(&frame.direction, 0, 1);
    while (!nil(node)) {
        parent = node;
        // Native inline comparison/BF7FBF has the same current-header load order
        // as the existing443D00 engine: lengths, right data, then left data.
        insert_left = less(pair, node + 0x0cu); // B1AFD6 on nonempty strings
        put_byte(&frame.direction, 0, static_cast<unsigned char>(insert_left));
        node = read(ptr(node), insert_left ? 0u : 8u);
    }
    write(&frame.cursor, 4, parent);
    write(&frame.cursor, 0, bits(tree));
    if (insert_left) {
        if (parent == read(ptr(read(tree, 4)))) {
            frame.link.pair_argument = pair;
            frame.link.parent_argument = parent;
            frame.link.left_argument = 1;
            frame.link.output_argument = bits(&frame.cursor);
            void* const result = link_native_resource_registry_node_00b1ada0(
                tree, frame.link, context, acquired.link); // B1B018
            const Word owner = read(result);
            void* const output = ptr(frame.output_argument);
            const Word selected = read(result, 4);
            publish_result(output, owner, selected, 1);
            return output;
        }
        decrement_native_resource_registry_iterator_00b1ab40(&frame.cursor, context.validation); // B1B03F
    }
    const Word candidate = read(&frame.cursor, 4);
    if (less(candidate + 0x0cu, pair)) { // B1B04F
        const Word direction = frame.direction; // entire current DWORD, preserved upper3 bytes
        frame.link.pair_argument = pair;
        frame.link.parent_argument = parent;
        frame.link.left_argument = direction;
        frame.link.output_argument = bits(&frame.cursor);
        void* const result = link_native_resource_registry_node_00b1ada0(
            tree, frame.link, context, acquired.link); // B1B066
        const Word owner = read(result);
        void* const output = ptr(frame.output_argument);
        const Word selected = read(result, 4);
        publish_result(output, owner, selected, 1);
        return output;
    }
    void* const output = ptr(frame.output_argument);
    const Word owner = read(&frame.cursor);
    publish_result(output, owner, candidate, 0);
    return output;
}

void* insert_native_resource_registry_hint_00b1b0b0(void* tree,
    const NativeResourceRegistryHintInsertFrame& frame, NativeResourceRegistryInsertionContext& context,
    NativeResourceRegistryHintInsertAcquired& acquired) {
    begin(acquired.started);
    if (read(tree, 8) == 0) {
        const Word pair = frame.pair_argument;
        const Word head = read(tree, 4);
        const Word output = frame.output_argument;
        frame.link.pair_argument = pair;
        frame.link.parent_argument = head;
        frame.link.left_argument = 1;
        frame.link.output_argument = output;
        link_native_resource_registry_node_00b1ada0(tree, frame.link, context, acquired.direct); // B1B0CF
        return ptr(output);
    }
    const Word minimum = read(ptr(read(tree, 4)));
    const Word owner = read(&frame.incoming_hint);
    if (owner == 0 || owner != bits(tree)) invalid(context); // B1B0F0
    const Word hint = read(&frame.incoming_hint, 4); // AFTER first validation
    Word pair;
    if (hint == minimum) {
        pair = frame.pair_argument;
        if (less(pair, hint + 0x0cu)) // B1B109
            return hint_link(tree, pair, hint, 1, frame, context, acquired); // B1B121
    } else {
        const Word head = read(tree, 4); // BEFORE second validation of captured owner
        if (owner == 0 || owner != bits(tree)) invalid(context); // B1B13D
        const bool is_end = hint == head;
        pair = frame.pair_argument;
        if (is_end) {
            const Word maximum = read(ptr(read(tree, 4)), 8);
            if (less(maximum + 0x0cu, pair)) { // B1B157
                const Word current_maximum = read(ptr(read(tree, 4)), 8);
                return hint_link(tree, pair, current_maximum, 0, frame, context, acquired); // B1B175
            }
        } else {
            if (less(pair, hint + 0x0cu)) { // B1B18D
                write(&frame.incoming_hint, 0, owner);
                write(&frame.incoming_hint, 4, hint);
                decrement_native_resource_registry_iterator_00b1ab40(
                    &frame.incoming_hint, context.validation); // B1B1A2; SAME incoming cells
                const Word predecessor_for_key = read(&frame.incoming_hint, 4);
                if (less(predecessor_for_key + 0x0cu, pair)) { // B1B1B2
                    const Word predecessor = read(&frame.incoming_hint, 4);
                    if (nil(read(ptr(predecessor), 8)))
                        return hint_link(tree, pair, predecessor, 0, frame, context, acquired); // B1B1D3
                    return hint_link(tree, pair, hint, 1, frame, context, acquired); // B1B1E8
                }
            }
            if (less(hint + 0x0cu, pair)) { // B1B200
                const Word current_head = read(tree, 4);
                write(&frame.incoming_hint, 0, owner);
                write(&frame.incoming_hint, 4, hint);
                write(&frame.local.iterator, 4, current_head);
                write(&frame.local.iterator, 0, bits(tree));
                increment_native_resource_registry_iterator_00b19890(
                    frame.incoming_hint, context.validation); // B1B220
                const Word equal = compare_native_resource_registry_iterators_00b19530(
                    &frame.incoming_hint, &frame.local.iterator, context.validation); // B1B22E
                const Word successor = read(&frame.incoming_hint, 4); // native EBP now successor
                if ((equal & 0xffu) != 0 || less(pair, successor + 0x0cu)) { // B1B242
                    if (nil(read(ptr(hint), 8)))
                        return hint_link(tree, pair, hint, 0, frame, context, acquired); // B1B25F
                    return hint_link(tree, pair, successor, 1, frame, context, acquired); // B1B274
                }
            }
        }
    }
    frame.fallback.pair_argument = pair;
    frame.fallback.output_argument = bits(&frame.local);
    void* const result = insert_native_resource_registry_pair_00b1af90(
        tree, frame.fallback, context, acquired.fallback); // B1B28D
    const Word result_owner = read(result);
    void* const output = ptr(frame.output_argument);
    write(output, 0, result_owner); // B1B29A BEFORE current returned node load
    const Word result_node = read(result, 4);
    write(output, 4, result_node);
    return output;
}

void destroy_native_resource_registry_temporary_00b19dd0(void* header,
    NativeStringRawPoolContext& strings) {
    destroy_native_string_header_0041dd20(header, strings); // exact normalized29B alias
}

void* find_or_insert_native_resource_registry_value_00b1b2b0(void* tree,
    const NativeResourceRegistryValueFrame& frame, NativeResourceRegistryInsertionContext& context,
    NativeResourceRegistryValueAcquired& acquired) {
    begin(acquired.started);
    const Word name = frame.name_argument;
    Word owner = bits(tree);
    Word node = bits(lower_bound_native_resource_factory_00b19b90(tree, ptr(name))); // B1B2D2
    if (owner == 0) invalid(context); // B1B2DD
    if (node == read(tree, 4) || less(name, node + 0x0cu)) { // B1B2EE
        Word captured_data = 0;
        void* const header = const_cast<Word*>(frame.temporary_pair);
        const bool same = bits(header) == name;
        frame.temporary_pair[0] = 0;
        frame.temporary_pair[1] = 0;
        if (!same) {
            const Word length = read(ptr(name));
            resize_native_string_header_0041dd40(header, context.strings, length, true); // B1B30F
            const bool nonempty = read(ptr(name)) != 0;
            captured_data = frame.temporary_pair[1]; // capture AFTER source count test, BEFORE branch
            if (nonempty) {
                const Word count = frame.temporary_pair[0];
                const Word source = read(ptr(name), 4);
                if (count != 0) std::memmove(ptr(captured_data), ptr(source), count); // B1B326
            }
        }
        acquired.captured_temporary_data = captured_data;
        frame.temporary_pair[2] = 0;
        frame.hint.pair_argument = bits(header);
        write(&frame.hint.incoming_hint, 4, node);
        write(&frame.hint.incoming_hint, 0, owner);
        frame.hint.output_argument = bits(&frame.result);
        acquired.temporary_eh_state = 0;
        try {
            void* const result = insert_native_resource_registry_hint_00b1b0b0(
                tree, frame.hint, context, acquired.hint); // B1B346
            owner = read(result);
            node = read(result, 4);
        } catch (...) {
            acquired.temporary_eh_state = -1;
            acquired.unwind_cleanup_started = true;
            try {
                destroy_native_resource_registry_temporary_00b19dd0(header, context.strings); // CBC713
                acquired.unwind_cleanup_completed = true;
            } catch (...) {
                std::terminate(); // explicit source double-exception transport boundary
            }
            throw;
        }
        acquired.temporary_eh_state = -1;
        if (captured_data != 0) {
            const Word count = frame.temporary_pair[0] + 1u;
            acquired.normal_return_started = true;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                context.strings.actual_published_01090aa8,
                context.strings.actual_manager_publication_01090aa0); // B1B367
            return_native_string_pool_00bd1510(pool, ptr(captured_data), count,
                context.strings.actual_small_returns_disabled_01090aa4); // B1B36E
            acquired.normal_return_completed = true;
        }
    }
    if (owner == 0) invalid(context); // B1B378
    if (node == read(ptr(owner), 4)) invalid(context); // B1B382
    return ptr(node + 0x14u);
}

void register_native_resource_factory_00b1b3a0(void* registry,
    const NativeResourceRegistryRegistrationFrame& frame, NativeResourceRegistryInsertionContext& context,
    NativeResourceRegistryRegistrationAcquired& acquired) {
    begin(acquired.started);
    const Word name = frame.name_argument;
    void* const tree = at(registry, 4);
    find_native_resource_factory_00b19d60(tree, &frame.result, ptr(name), context.validation); // B1B3B4
    if (bits(tree) == 0) invalid(context); // B1B3BD
    const Word owner = read(&frame.result);
    const Word head = read(tree, 4); // BEFORE returning owner validation
    if (owner == 0 || owner != bits(tree)) invalid(context); // B1B3D2
    if (read(&frame.result, 4) != head) return;
    frame.value.name_argument = name;
    void* const value = find_or_insert_native_resource_registry_value_00b1b2b0(
        tree, frame.value, context, acquired.value); // B1B3E1
    const Word factory = frame.factory_argument;
    write(value, 0, factory);
}
} // namespace bsp
