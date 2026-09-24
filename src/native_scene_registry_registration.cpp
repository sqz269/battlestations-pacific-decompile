#include "bsp/native_scene_registry_registration.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* ptr(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(Word p, Word n = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(p + n);
}
void write(Word p, Word n, Word v) noexcept {
    *reinterpret_cast<volatile Word*>(p + n) = v;
}
Word sar3(Word v) noexcept { return static_cast<Word>(static_cast<std::int32_t>(v) >> 3); }
Word hash(Word key) noexcept {
    // C03DBE with positive divisor 1F31D: EAX signed quotient, EDX signed
    // remainder. Its negative-dividend/positive-remainder correction is
    // unreachable for this divisor. Products/subtraction wrap as DWORDs.
    const auto value = static_cast<std::int32_t>(key ^ 0xdeadbeefu);
    Word h = static_cast<Word>(value % 0x1f31d) * 0x41a7u
        - static_cast<Word>(value / 0x1f31d) * 0xb14u;
    if (static_cast<std::int32_t>(h) < 0) h += 0x7fffffffu;
    return h;
}
void invalid(NativeSceneRegistryRegistrationContext& context) {
    context.mutation.actual_00bf6713();
}
bool bad_boundary(Word vector, Word captured_begin, Word bucket) noexcept {
    return captured_begin == 0 || sar3(read(vector, 8) - captured_begin) <= bucket;
}
void check_boundary(Word vector, Word begin, Word bucket,
    NativeSceneRegistryRegistrationContext& context,
    NativeSceneRegistryRegistrationAcquired& acquired, Word site) {
    if (bad_boundary(vector, begin, bucket)) {
        acquired.active_call_site = site;
        invalid(context);
    }
}
void inserted_byte(Word output, unsigned char value) noexcept {
    *reinterpret_cast<volatile unsigned char*>(output + 8u) = value;
}
}

void* insert_native_scene_registry_00b83700(void* actual_registry,
    NativeSceneRegistryRegistrationInsertFrame& frame,
    NativeSceneRegistryRegistrationContext& context,
    NativeSceneRegistryRegistrationAcquired& acquired) {
    acquired.started = true;
    acquired.complete = false;
    auto& local = frame.local_00;
    Word registry = bits(actual_registry);
    const Word quarter_size = read(registry, 0x0c) >> 2;
    const Word active = read(registry, 0x24);
    local[0] = registry;
    if (active <= quarter_size) {
        const Word initial_begin = read(registry, 0x14);
        const Word vector = registry + 0x10u;
        const Word length = initial_begin ? sar3(read(vector, 8) - initial_begin) : 0;
        if (length - 1u <= active) {
            const Word repeated_length = initial_begin
                ? sar3(read(vector, 8) - initial_begin) : 0;
            write(registry, 0x20, repeated_length * 2u - 3u);
            const Word head = read(registry, 8);
            frame.resize.pair_argument[0] = 0;
            frame.resize.pair_argument[1] = head;
            frame.resize.pair_argument[0] = registry + 4u;
            frame.resize.requested_count = read(registry, 0x20) + 2u;
            acquired.active_call_site = 0x00b83773;
            resize_native_scene_registry_boundaries_00b83490(ptr(vector),
                frame.resize, context.mutation);
        } else {
            const Word mask = read(registry, 0x20);
            if (mask < active) write(registry, 0x20, mask * 2u + 1u);
        }
        const Word mask = read(registry, 0x20);
        Word split = read(registry, 0x24) - (mask >> 1) - 1u;
        const Word begin = read(vector, 4);
        local[1] = split;
        check_boundary(vector, begin, split, context, acquired, 0x00b837ac);
        const Word list = registry + 4u;
        Word node = read(read(vector, 4), split * 8u + 4u);
        if (list == 0) { acquired.active_call_site = 0x00b837bd; invalid(context); }
        local[2] = list;
        for (;;) {
            check_boundary(vector, read(vector, 4), split + 1u,
                context, acquired, 0x00b837dc);
            if (read(read(vector, 4), split * 8u + 12u) == node) break;
            Word owner = local[2];
            if (owner == 0) { acquired.active_call_site = 0x00b837f9; invalid(context); }
            if (node == read(owner, 4)) { acquired.active_call_site = 0x00b83803; invalid(context); }
            acquired.active_call_site = 0x00b83817;
            Word bucket = hash(read(node, 8));
            registry = local[0];
            bucket &= read(registry, 0x20);
            split = local[1];
            if (bucket == split) {
                owner = local[2];
                if (node == read(owner, 4)) { acquired.active_call_site = 0x00b8384d; invalid(context); }
                node = read(node);
                continue;
            }
            owner = local[2];
            const bool sentinel = node == read(owner, 4);
            const Word captured_head = read(list, 4);
            local[4] = owner;
            local[5] = node;
            local[7] = captured_head;
            if (sentinel) { acquired.active_call_site = 0x00b83874; invalid(context); }
            const bool same_owner = owner == list;
            local[5] = read(node);
            if (!same_owner) { acquired.active_call_site = 0x00b83884; invalid(context); }
            const Word end = local[7];
            if (local[5] != end) {
                Word index = local[1];
                for (;;) {
                    check_boundary(vector, read(vector, 4), index,
                        context, acquired, 0x00b838b3);
                    const Word current_begin = read(vector, 4);
                    if (read(current_begin, index * 8u + 4u) != node) break;
                    check_boundary(vector, current_begin, index,
                        context, acquired, 0x00b838d3);
                    const Word destination = read(vector, 4);
                    const Word pair_owner = local[4];
                    const Word pair_node = local[5];
                    write(destination, index * 8u, pair_owner);
                    write(destination, index * 8u + 4u, pair_node);
                    if (index == 0) break;
                    --index;
                }
                const Word head = read(list, 4);
                // B838F3 CMP EBX,EBX; MOV does not alter flags; JZ always
                // skips B838FE's physically present GrowListSizeChecked(0).
                Word previous = read(node, 4);
                const Word next = local[5];
                write(previous, 0, next);
                previous = read(next, 4);
                write(previous, 0, head);
                previous = read(head, 4);
                write(previous, 0, node);
                const Word next_previous = read(next, 4);
                const Word head_previous = read(head, 4);
                write(head, 4, next_previous);
                const Word node_previous = read(node, 4);
                write(next, 4, node_previous);
                write(node, 4, head_previous);
                Word current_head = read(list, 4);
                node = read(current_head, 4);
                if (node == current_head) { acquired.active_call_site = 0x00b83932; invalid(context); }
                current_head = read(list, 4);
                registry = local[0];
                index = read(registry, 0x24);
                local[9] = current_head;
                const Word boundary_begin = read(vector, 4);
                ++index;
                local[2] = list;
                check_boundary(vector, boundary_begin, index,
                    context, acquired, 0x00b8395f);
                const Word destination = read(vector, 4);
                const Word pair_head = local[9];
                write(destination, index * 8u, list);
                write(destination, index * 8u + 4u, pair_head);
            }
            registry = local[0];
            Word index = read(registry, 0x24);
            while (local[1] < index) {
                const Word head = read(list, 4);
                local[11] = head;
                check_boundary(vector, read(vector, 4), index,
                    context, acquired, 0x00b8399a);
                const Word current_begin = read(vector, 4);
                const Word pair_head = local[11];
                if (read(current_begin, index * 8u + 4u) != pair_head) break;
                check_boundary(vector, current_begin, index,
                    context, acquired, 0x00b839be);
                const Word destination = read(vector, 4);
                const Word pair_owner = local[2];
                write(destination, index * 8u, pair_owner);
                write(destination, index * 8u + 4u, node);
                --index;
            }
            const bool same = local[4] == list;
            const Word head = read(list, 4);
            if (!same) { acquired.active_call_site = 0x00b839e3; invalid(context); }
            node = local[5];
            if (node == head) break;
            const Word current_owner = local[4];
            split = local[1];
            local[2] = current_owner;
        }
        registry = local[0];
        write(registry, 0x24, read(registry, 0x24) + 1u);
    }
    acquired.active_call_site = 0x00b83a1d;
    Word bucket = hash(read(frame.key_argument));
    const Word mask = read(registry, 0x20);
    bucket &= mask;
    if (read(registry, 0x24) <= bucket) bucket += 0xffffffffu - (mask >> 1);
    const Word upper = bucket + 1u;
    check_boundary(registry + 0x10u, read(registry, 0x14), upper,
        context, acquired, 0x00b83a66);
    Word node = read(read(registry, 0x14), upper * 8u + 4u);
    const Word list = registry + 4u;
    local[1] = list;
    if (list == 0) { acquired.active_call_site = 0x00b83a7d; invalid(context); }
    Word owner = local[1];
    local[2] = owner;
    for (;;) {
        if (bad_boundary(registry + 0x10u, read(registry, 0x14), bucket)) {
            acquired.active_call_site = 0x00b83aa3; invalid(context);
            owner = local[2];
        }
        if (read(read(registry, 0x14), bucket * 8u + 4u) == node) break;
        if (owner == 0) {
            acquired.active_call_site = 0x00b83ab9; invalid(context);
            owner = local[2];
        }
        node = read(node, 4);
        if (node == read(owner, 4)) {
            acquired.active_call_site = 0x00b83aca; invalid(context);
            owner = local[2];
            if (node == read(owner, 4)) { acquired.active_call_site = 0x00b83ad8; invalid(context); }
        }
        const Word key = read(frame.key_argument);
        if (key < read(node, 8)) continue;
        if (node == read(owner, 4)) { acquired.active_call_site = 0x00b83aed; invalid(context); }
        const Word node_key = read(node, 8);
        if (node_key >= read(frame.key_argument)) {
            const Word output = frame.output_argument;
            write(output, 0, owner);
            write(output, 4, node);
            inserted_byte(output, 0);
            acquired.complete = true;
            return ptr(output);
        }
        if (node == read(owner, 4)) { acquired.active_call_site = 0x00b83b06; invalid(context); }
        node = read(node);
        break;
    }
    const Word key_source = frame.key_argument;
    const Word previous = read(node, 4);
    const Word allocation_owner = local[1]; // B83B15: original unused ECX input
    (void)allocation_owner;
    const Word captured_owner = owner; // EBX receives EDI before allocation
    local[5] = node;
    acquired.active_call_site = 0x00b83b21;
    void* const allocated = allocate_native_scene_registry_node_00b823b0(
        context.mutation.storage.actual_00bf681b, nullptr, node, previous,
        ptr(key_source));
    acquired.allocated_node = allocated;
    acquired.allocation_completed = true;
    const Word growth_list = local[1];
    acquired.active_call_site = 0x00b83b2e;
    grow_native_scene_registry_list_size_00b82d30(ptr(growth_list), 1,
        frame.list_growth, context.actual_message_00ce38f8);
    acquired.size_growth_completed = true;
    const bool has_owner = local[2] != 0; // compare BEFORE either link store
    write(node, 4, bits(allocated));
    write(read(bits(allocated), 4), 0, bits(allocated));
    acquired.node_linked = true;
    if (!has_owner) { acquired.active_call_site = 0x00b83b42; invalid(context); }
    const Word inserted = read(node, 4);
    const bool sentinel = inserted == read(captured_owner, 4);
    local[5] = inserted;
    if (sentinel) { acquired.active_call_site = 0x00b83b53; invalid(context); }
    registry = local[0];
    for (;;) {
        check_boundary(registry + 0x10u, read(registry, 0x14), bucket,
            context, acquired, 0x00b83b73);
        const Word begin = read(registry, 0x14);
        if (read(begin, bucket * 8u + 4u) != node) break;
        check_boundary(registry + 0x10u, begin, bucket,
            context, acquired, 0x00b83b91);
        const Word destination = read(registry, 0x14);
        const Word current_inserted = local[5];
        write(destination, bucket * 8u, captured_owner);
        write(destination, bucket * 8u + 4u, current_inserted);
        if (bucket == 0) break;
        --bucket;
    }
    const Word output = frame.output_argument;
    const Word current_inserted = local[5];
    write(output, 0, captured_owner);
    write(output, 4, current_inserted);
    inserted_byte(output, 1);
    acquired.complete = true;
    return ptr(output);
}

void clear_native_scene_registry_00b83be0(void* actual_registry,
    NativeSceneRegistryClearFrame& frame,
    NativeSceneRegistryRegistrationContext& context) {
    const Word registry = bits(actual_registry);
    Word head = read(registry, 8);
    Word node = read(head);
    const Word list = registry + 4u;
    write(head, 0, head);
    head = read(list, 4);
    write(head, 4, head);
    const bool empty = node == read(list, 4);
    write(list, 8, 0);
    if (!empty) {
        do {
            const Word next = read(node);
            context.mutation.storage.actual_00bf65ac(ptr(node));
            const bool done = next == read(list, 4);
            node = next;
            if (done) break;
        } while (true);
    }
    frame.pair[1] = read(list, 4);
    frame.assign.pair_source_argument = bits(frame.pair);
    frame.assign.count_argument = 9;
    frame.pair[0] = list;
    assign_native_scene_registry_boundaries_00b83560(ptr(registry + 0x10u),
        frame.assign, context.mutation);
    write(registry, 0x20, 1);
    write(registry, 0x24, 1);
}

void* erase_native_scene_registry_range_00b83d90(void* actual_registry,
    NativeSceneRegistryEraseRangeFrame& frame,
    NativeSceneRegistryRegistrationContext& context) {
    const Word registry = bits(actual_registry);
    const Word initial_first = read(read(registry, 8));
    const Word owner = frame.first_owner_argument;
    const Word list = registry + 4u;
    frame.saved_registry = registry;
    if (owner == 0 || owner != list) invalid(context);
    Word node = frame.first_node_argument;
    if (node == initial_first) {
        const Word last_owner = frame.last_owner_argument;
        const Word captured_head = read(list, 4);
        if (last_owner == 0 || last_owner != list) invalid(context);
        if (frame.last_node_argument == captured_head) {
            clear_native_scene_registry_00b83be0(ptr(frame.saved_registry),
                frame.clear, context);
            const Word head = read(list, 4);
            const Word output = frame.output_argument;
            const Word first = read(head);
            write(output, 0, list);
            write(output, 4, first);
            return ptr(output);
        }
    }
    for (;;) {
        if (owner == 0 || owner != frame.last_owner_argument) invalid(context);
        if (node == frame.last_node_argument) break;
        const Word captured = node;
        if (owner == 0) invalid(context);
        if (node == read(owner, 4)) invalid(context);
        const Word current_registry = frame.saved_registry;
        node = read(node); // next captured BEFORE entry free/callbacks
        frame.entry.node = captured;
        frame.entry.owner = owner;
        frame.entry.output = bits(&frame.first_owner_argument);
        erase_native_scene_registry_entry_00b827c0(ptr(current_registry),
            frame.entry, context.mutation.storage);
    }
    const Word output = frame.output_argument;
    write(output, 0, owner);
    write(output, 4, node);
    return ptr(output);
}

Word erase_native_scene_registry_key_00b83e50(void* actual_registry,
    NativeSceneRegistryEraseKeyFrame& frame,
    NativeSceneRegistryRegistrationContext& context) {
    const Word key = frame.key_or_count_argument;
    frame.query.key = key;
    frame.query.output = bits(frame.range);
    equal_range_native_scene_registry_00b82650(actual_registry, frame.query,
        frame.query_scratch);
    const Word last_node = frame.range[3];
    const Word last_owner = frame.range[2];
    const Word first_node = frame.range[1];
    const Word first_owner = frame.range[0];
    frame.key_or_count_argument = 0;
    frame.count.unused_tag = frame.key_or_count_argument;
    frame.count.count = bits(&frame.key_or_count_argument);
    frame.count.last_node = last_node;
    frame.count.last_owner = last_owner;
    frame.count.first_node = first_node;
    frame.count.first_owner = first_owner;
    count_native_scene_registry_range_00b820b0(frame.count);
    const Word current_first_owner = frame.range[0];
    frame.erase.last_node_argument = last_node;
    frame.erase.last_owner_argument = last_owner;
    frame.erase.first_node_argument = first_node;
    frame.erase.first_owner_argument = current_first_owner;
    frame.erase.output_argument = bits(frame.range);
    erase_native_scene_registry_range_00b83d90(actual_registry, frame.erase, context);
    return frame.key_or_count_argument;
}
} // namespace bsp
