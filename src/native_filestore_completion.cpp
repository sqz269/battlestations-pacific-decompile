#include "bsp/native_filestore_completion.hpp"

#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_filestore_factory.hpp"
#include "bsp/native_filestore_open.hpp"
#include "bsp/native_filestore_pending_tree.hpp"
#include "bsp/native_filestore_request.hpp"
#include "bsp/native_filestore_resident_insert.hpp"
#include "bsp/native_filestore_subtree.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstddef>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native FileStore completion requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(const void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
volatile std::uint32_t& word(const void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(p, offset));
}
void* volatile& link(const void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<void* volatile*>(at(p, offset));
}
bool nil(const void* p) noexcept {
    return *static_cast<const volatile unsigned char*>(at(p, 0x19)) != 0;
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}
void construct_key(void* output, const void* source, NativeStringStorage& strings) {
    const bool same = output == source;
    word(output) = 0;
    word(output, 4) = 0;
    if (!same) {
        resize_native_string_header_0041dd40(output, strings, word(source), true);
        if (word(source) != 0) {
            const auto count = word(output);
            auto* const from = link(source, 4);
            auto* const to = link(output, 4);
            if (count) std::memmove(to, from, count); // Original BF7680 permits overlap.
        }
    }
}
void release_stream(void* stream, NativeAdoptedSubstreamDispatch& dispatch) {
    if (stream && InterlockedDecrement(static_cast<volatile LONG*>(at(stream, 4))) == 0) {
        const auto table = word(stream);
        const auto target = word(reinterpret_cast<void*>(table));
        dispatch.source_zero_reference(target, stream, table);
    }
}
struct NameCleanup {
    void* header;
    NativeStringStorage& strings;
    ~NameCleanup() noexcept { destroy_native_string_header_0041dd20(header, strings); }
};
struct StreamCleanup {
    void* stream;
    NativeAdoptedSubstreamDispatch& dispatch;
    bool armed{true};
    ~StreamCleanup() noexcept { if (armed) release_stream(stream, dispatch); }
    void finish() { armed = false; release_stream(stream, dispatch); }
};
struct PairCleanup {
    void* pair;
    NativeFileStoreCompletionContext& context;
    bool armed{true};
    ~PairCleanup() noexcept {
        if (armed) destroy_native_file_store_payload_00be5d70(pair, context.strings, context.streams);
    }
    void finish() {
        armed = false;
        destroy_native_file_store_payload_00be5d70(pair, context.strings, context.streams);
    }
};
void publish_insert(void* output, const NativeFileStoreNameIterator& iterator,
    unsigned char inserted) noexcept {
    auto* const owner = iterator.owner_00;
    auto* const node = iterator.node_04;
    link(output, 4) = node;
    *static_cast<volatile unsigned char*>(at(output, 8)) = inserted;
    link(output) = owner;
}
} // namespace

void* construct_native_file_store_resident_pair_00be6090(void* output,
    const void* name, const void* stream_cell, NativeStringStorage& strings) {
    construct_key(output, name, strings);
    link(output, 8) = nullptr;
    auto* const stream = link(stream_cell);
    if (stream) {
        link(output, 8) = stream;
        InterlockedIncrement(static_cast<volatile LONG*>(at(stream, 4)));
    }
    return output;
}
void* copy_native_file_store_resident_pair_00be6250(void* output,
    const void* pair, NativeStringStorage& strings) {
    construct_key(output, pair, strings);
    link(output, 8) = nullptr;
    auto* const stream = link(pair, 8);
    if (stream) {
        link(output, 8) = stream;
        InterlockedIncrement(static_cast<volatile LONG*>(at(stream, 4)));
    }
    return output;
}
void destroy_native_file_store_resident_pair_00be5cf0(void* pair,
    NativeStringStorage& strings, NativeAdoptedSubstreamDispatch& dispatch) {
    // Independent BE5CF0 listing has the same member/call/clear/EH schedule.
    destroy_native_file_store_payload_00be5d70(pair, strings, dispatch);
}
void* insert_native_file_store_resident_pair_00be7340(void* tree, void* output,
    const void* pair, NativeStringStorage& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    auto* parent = link(tree, 4);
    auto* node = link(parent, 4);
    unsigned char insert_left = 1;
    while (!nil(node)) {
        parent = node;
        insert_left = less_native_string_headers_00443d00(pair, at(node, 0x0c)) ? 1 : 0;
        node = link(node, insert_left ? 0 : 8);
    }
    NativeFileStoreNameIterator iterator{tree, parent};
    if (insert_left) {
        if (parent == link(link(tree, 4))) {
            link_native_file_store_resident_node_00be6cf0(tree, &iterator, 1, parent, pair, strings);
            publish_insert(output, iterator, 1);
            return output;
        }
        decrement_native_file_store_resident_iterator_00be4d20(&iterator, callbacks);
    }
    auto* const candidate = iterator.node_04;
    if (!less_native_string_headers_00443d00(at(candidate, 0x0c), pair)) {
        publish_insert(output, iterator, 0);
        return output;
    }
    link_native_file_store_resident_node_00be6cf0(tree, &iterator, insert_left, parent, pair, strings);
    publish_insert(output, iterator, 1);
    return output;
}
void add_native_file_store_file_00be7760(void* store, const void* name,
    void* stream, NativeFileStoreCompletionContext& context) {
    alignas(4) std::byte normalized[8];
    copy_construct_native_resource_path_header_00bee780(normalized, name, context.strings);
    const NameCleanup name_cleanup{normalized, context.strings};
    auto* const tree = at(store, 0x14);
    NativeFileStoreNameIterator found;
    find_native_file_store_open_name_00be5e90(tree, &found, normalized, context.invalid_parameters);
    auto* const found_owner = found.owner_00;
    auto* const found_node = found.node_04;
    auto* const found_head = link(tree, 4); // BE77AD precedes returning CRT BE77B6.
    if (!found_owner || found_owner != tree) invalid(context.invalid_parameters);
    if (found_node != found_head) {
        // BE77C1 reads current data; fallback0109DB6C is only passed to RET4254B0.
        auto* const data = link(normalized, 4);
        (void)data;
        return;
    }
    InterlockedIncrement(static_cast<volatile LONG*>(at(stream, 4)));
    StreamCleanup retained{stream, context.streams};
    alignas(4) std::byte first[12], second[12], result[12];
    void* stream_cell = stream;
    construct_native_file_store_resident_pair_00be6090(first, normalized, &stream_cell, context.strings);
    PairCleanup first_cleanup{first, context};
    copy_native_file_store_resident_pair_00be6250(second, first, context.strings);
    PairCleanup second_cleanup{second, context};
    insert_native_file_store_resident_pair_00be7340(tree, result, second, context.strings,
        context.invalid_parameters);
    second_cleanup.finish();
    first_cleanup.finish();
    retained.finish();
}
void complete_native_file_store_file_00be78b0(void* store,
    const void* first, const void* second, void* stream,
    NativeFileStoreCompletionContext& context) {
    alignas(4) std::byte normalized[8];
    copy_construct_native_resource_path_header_00bee780(normalized, first, context.strings);
    const NameCleanup name_cleanup{normalized, context.strings};
    auto* const tree = at(store, 0x20);
    NativeFileStoreNameIterator found, erased;
    find_native_file_store_pending_name_00be5f00(tree, &found, normalized, context.invalid_parameters);
    auto* const found_owner = found.owner_00;
    if (!found_owner) invalid(context.invalid_parameters);
    auto* const found_node = found.node_04; // BE7904 follows the first CRT call.
    if (found_node == link(found_owner, 4)) invalid(context.invalid_parameters);
    auto* const erase_owner = found.owner_00; // BE7912 reload; ESI node stays captured.
    const auto callback = word(found_node, 0x14);
    erase_native_file_store_pending_iterator_00be6a20(tree, &erased,
        erase_owner, found_node, context.strings, context.invalid_parameters);
    add_native_file_store_file_00be7760(store, normalized, stream, context);
    context.completion.invoke_00be7942(callback, first, second);
}
void dispatch_native_file_store_completion_00be7b20(void* stream,
    const void* first, const void* second, NativeFileStoreFactoryContext& factory,
    NativeFileStoreCompletionContext& context) {
    auto* const current_factory = get_native_filestore_factory_004fc150(factory);
    auto* const current_store = link(current_factory, 8);
    complete_native_file_store_file_00be78b0(current_store, first, second, stream, context);
}
} // namespace bsp
