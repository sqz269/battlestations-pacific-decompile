#include "bsp/native_filestore_request.hpp"

#include "bsp/native_filestore_open.hpp"
#include "bsp/native_filestore_pending_tree.hpp"
#include "bsp/native_filestore_subtree.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_string.hpp"

#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native FileStore requests require MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(const void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* volatile& link(const void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<void* volatile*>(at(p, offset));
}
volatile std::uint32_t& word(const void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(p, offset));
}
volatile unsigned char& byte(const void* p, std::uint32_t offset) noexcept {
    return *static_cast<volatile unsigned char*>(at(p, offset));
}
void* head(void* tree) noexcept { return link(tree, 4); }
void* parent(void* node) noexcept { return link(node, 4); }
volatile unsigned char& color(void* node) noexcept { return byte(node, 0x18); }
bool nil(void* node) noexcept { return byte(node, 0x19) != 0; }
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}
void copy_construct_key(void* output, const void* source, NativeStringStorage& strings) {
    const bool same = output == source;
    word(output) = 0;
    word(output, 4) = 0;
    if (!same) {
        resize_native_string_header_0041dd40(output, strings, word(source), true);
        if (word(source) != 0) {
            const auto count = word(output);
            auto* const from = link(source, 4);
            auto* const to = link(output, 4);
            // BF7680 supports overlap; preserve all current field reads.
            if (count != 0) std::memmove(to, from, count);
        }
    }
}
struct CompletedLengthMessage {
    NativeLegacySboStringStorage& storage;
    ~CompletedLengthMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(storage); }
};
[[noreturn]] void throw_length_error() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = 0;
    native_legacy_sbo_string_assign_counted_00408720(message, "map/set<T> too long", 19);
    const CompletedLengthMessage completed{message};
    throw NativeHardwareLayoutTreeLengthError{message};
}
void publish_insert(void* output, const NativeFileStoreNameIterator& iterator,
    unsigned char inserted) noexcept {
    auto* const owner = iterator.owner_00;
    auto* const node = iterator.node_04;
    link(output, 4) = node;
    byte(output, 8) = inserted;
    link(output) = owner;
}
// All RequestFile log sites target RET-only4254B0. Its call arguments still
// read a current name data pointer; no host log or callback is invented.
void read_log_name(const void* name) noexcept {
    auto* const data = link(name, 4);
    (void)data;
}
} // namespace

void* lower_bound_native_file_store_pending_name_00be5530(void* tree, const void* name) {
    auto* candidate = head(tree);
    auto* node = parent(candidate);
    while (!nil(node)) {
        if (less_native_string_headers_00443d00(at(node, 0x0c), name)) {
            node = link(node, 8);
        } else {
            candidate = node;
            node = link(node);
        }
    }
    return candidate;
}
void* upper_bound_native_file_store_pending_name_00be5630(void* tree, const void* name) {
    auto* candidate = head(tree);
    auto* node = parent(candidate);
    while (!nil(node)) {
        if (less_native_string_headers_00443d00(name, at(node, 0x0c))) {
            candidate = node;
            node = link(node);
        } else {
            node = link(node, 8);
        }
    }
    return candidate;
}
void* find_native_file_store_pending_name_00be5f00(void* tree, void* output,
    const void* name, const SingletonLifetimeCallbacks& callbacks) {
    auto* node = lower_bound_native_file_store_pending_name_00be5530(tree, name);
    if (!tree) invalid(callbacks);
    if (node == head(tree) || less_native_string_headers_00443d00(name, at(node, 0x0c))) {
        node = head(tree);
    }
    link(output) = tree;
    link(output, 4) = node;
    return output;
}
void decrement_native_file_store_pending_iterator_00be4db0(void* iterator,
    const SingletonLifetimeCallbacks& callbacks) {
    if (!link(iterator)) invalid(callbacks);
    auto* node = link(iterator, 4);
    if (nil(node)) {
        auto* const maximum = link(node, 8);
        link(iterator, 4) = maximum;
        if (nil(maximum)) invalid(callbacks);
        return;
    }
    auto* child = link(node);
    if (!nil(child)) {
        child = maximum_native_file_store_pending_node_00be4a70(child);
        link(iterator, 4) = child;
        return;
    }
    auto* ancestor = parent(node);
    while (!nil(ancestor) && link(iterator, 4) == link(ancestor)) {
        link(iterator, 4) = ancestor;
        ancestor = parent(ancestor);
    }
    if (nil(link(iterator, 4))) {
        invalid(callbacks);
        return;
    }
    link(iterator, 4) = ancestor;
}
void distance_native_file_store_pending_iterators_00be5750(void* first_owner,
    void* first_node, void* last_owner, void* last_node, volatile std::uint32_t* count,
    std::uint32_t ignored, const SingletonLifetimeCallbacks& callbacks) {
    (void)ignored;
    NativeFileStoreNameIterator first{first_owner, first_node};
    for (;;) {
        if (!first.owner_00 || first.owner_00 != last_owner) invalid(callbacks);
        if (first.node_04 == last_node) return;
        *count = *count + 1u;
        advance_native_file_store_pending_iterator_00be4e40(&first, callbacks);
    }
}
void* construct_native_file_store_pending_pair_00be6120(void* output,
    const void* name, const volatile std::uint32_t* callback, NativeStringStorage& strings) {
    copy_construct_key(output, name, strings);
    word(output, 8) = *callback;
    return output;
}
void* copy_native_file_store_pending_pair_00be62e0(void* output,
    const void* pair, NativeStringStorage& strings) {
    copy_construct_key(output, pair, strings);
    word(output, 8) = word(pair, 8);
    return output;
}
void* construct_native_file_store_pending_node_00be63e0(void* node, void* left,
    void* parent_node, void* right, const void* pair, std::uint8_t node_color,
    NativeStringStorage& strings) {
    link(node) = left;
    link(node, 4) = parent_node;
    link(node, 8) = right;
    copy_construct_key(at(node, 0x0c), pair, strings);
    word(node, 0x14) = word(pair, 8);
    color(node) = node_color;
    byte(node, 0x19) = 0;
    return node;
}
void* allocate_native_file_store_pending_node_00be6630(void* left, void* parent_node,
    void* right, const void* pair, std::uint8_t node_color, NativeStringStorage& strings) {
    auto* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x1c, 0x1c});
    try {
        if (node) construct_native_file_store_pending_node_00be63e0(
            node, left, parent_node, right, pair, node_color, strings);
    } catch (...) {
        // E01584 catch-all -> BE66A2..BE66B6 frees and rethrows. State1's
        // CC6CE0 placement cleanup calls RET-only401130: no key destructor.
        singleton_lifetime_free(node);
        throw;
    }
    return node;
}
void* link_native_file_store_pending_node_00be6ee0(void* tree, void* output,
    std::uint8_t insert_left, void* parent_node, const void* pair, NativeStringStorage& strings) {
    if (word(tree, 8) >= 0x15555554u) throw_length_error();
    auto* const sentinel = head(tree);
    auto* const inserted = allocate_native_file_store_pending_node_00be6630(
        sentinel, parent_node, sentinel, pair, 0, strings);
    auto* const current_head = head(tree);
    word(tree, 8) = word(tree, 8) + 1u;
    if (parent_node == current_head) {
        link(current_head, 4) = inserted;
        link(head(tree)) = inserted;
        link(head(tree), 8) = inserted;
    } else if (insert_left) {
        link(parent_node) = inserted;
        auto* const current = head(tree);
        if (parent_node == link(current)) link(current) = inserted;
    } else {
        link(parent_node, 8) = inserted;
        auto* const current = head(tree);
        if (parent_node == link(current, 8)) link(current, 8) = inserted;
    }
    auto* node = inserted;
    while (color(parent(node)) == 0) {
        auto* const parent_node_now = parent(node);
        auto* const grandparent = parent(parent_node_now);
        if (parent_node_now == link(grandparent)) {
            auto* const uncle = link(grandparent, 8);
            if (color(uncle) == 0) {
                color(parent_node_now) = 1;
                color(uncle) = 1;
                color(parent(parent(node))) = 0;
                node = parent(parent(node));
            } else {
                if (node == link(parent_node_now, 8)) {
                    node = parent_node_now;
                    rotate_native_file_store_pending_left_00be5140(tree, node);
                }
                color(parent(node)) = 1;
                color(parent(parent(node))) = 0;
                rotate_native_file_store_pending_right_00be4ad0(tree, parent(parent(node)));
            }
        } else {
            auto* const uncle = link(grandparent);
            if (color(uncle) == 0) {
                color(parent_node_now) = 1;
                color(uncle) = 1;
                color(parent(parent(node))) = 0;
                node = parent(parent(node));
            } else {
                if (node == link(parent_node_now)) {
                    node = parent_node_now;
                    rotate_native_file_store_pending_right_00be4ad0(tree, node);
                }
                color(parent(node)) = 1;
                color(parent(parent(node))) = 0;
                // BE7056..BE7094 is the same left rotation inline.
                rotate_native_file_store_pending_left_00be5140(tree, parent(parent(node)));
            }
        }
    }
    color(parent(head(tree))) = 1;
    link(output, 4) = inserted;
    link(output) = tree;
    return output;
}
void* insert_native_file_store_pending_pair_00be7460(void* tree, void* output,
    const void* pair, NativeStringStorage& strings, const SingletonLifetimeCallbacks& callbacks) {
    auto* candidate = head(tree);
    auto* node = parent(candidate);
    bool insert_left = true;
    while (!nil(node)) {
        candidate = node;
        insert_left = less_native_string_headers_00443d00(pair, at(node, 0x0c));
        node = link(node, insert_left ? 0u : 8u);
    }
    NativeFileStoreNameIterator iterator{tree, candidate};
    if (insert_left) {
        if (candidate == link(head(tree))) {
            link_native_file_store_pending_node_00be6ee0(tree, &iterator, 1, candidate, pair, strings);
            publish_insert(output, iterator, 1);
            return output;
        }
        decrement_native_file_store_pending_iterator_00be4db0(&iterator, callbacks);
    }
    auto* const previous = iterator.node_04;
    if (!less_native_string_headers_00443d00(at(previous, 0x0c), pair)) {
        publish_insert(output, iterator, 0);
        return output;
    }
    link_native_file_store_pending_node_00be6ee0(tree, &iterator,
        static_cast<std::uint8_t>(insert_left), candidate, pair, strings);
    publish_insert(output, iterator, 1);
    return output;
}
std::uint32_t erase_native_file_store_pending_name_00be79c0(void* tree,
    const void* name, NativeStringStorage& strings, const SingletonLifetimeCallbacks& callbacks) {
    auto* const last = upper_bound_native_file_store_pending_name_00be5630(tree, name);
    if (!tree) invalid(callbacks);
    auto* const first = lower_bound_native_file_store_pending_name_00be5530(tree, name);
    if (!tree) invalid(callbacks);
    volatile std::uint32_t count = 0;
    distance_native_file_store_pending_iterators_00be5750(tree, first, tree, last, &count, 0, callbacks);
    NativeFileStoreNameIterator ignored;
    erase_native_file_store_pending_range_00be7580(tree, &ignored, tree, first,
        tree, last, strings, callbacks);
    return count;
}
void notify_native_vfs_request_failure_00bd9e30(void* manager,
    std::uint32_t discarded, NativeFileStoreRequestDispatch& dispatch) {
    (void)discarded;
    const auto target = word(manager, 0x90);
    dispatch.invoke_manager_failure_callback(target, manager);
}
struct NativeFileStoreRequestAcquired::Impl {
    NativeFileStoreRequestPhase phase = NativeFileStoreRequestPhase::fresh;
    std::uint32_t active = 0;
    std::uint32_t failed_at = 0;
    std::uint32_t resolved[2]{};
    std::uint32_t first_pair[3]{};
    std::uint32_t second_pair[3]{};
    std::uint32_t insertion_output[3]{};
    std::uint32_t callback = 0;
    bool resolved_armed = false;
    bool first_armed = false;
    bool second_armed = false;
    NativeFileStoreNameIterator iterator{};
};
NativeFileStoreRequestAcquired::NativeFileStoreRequestAcquired() : impl_(std::make_unique<Impl>()) {}
NativeFileStoreRequestAcquired::~NativeFileStoreRequestAcquired() {
    if (impl_->phase != NativeFileStoreRequestPhase::fresh &&
        impl_->phase != NativeFileStoreRequestPhase::complete) std::terminate();
}
NativeFileStoreRequestPhase NativeFileStoreRequestAcquired::phase() const noexcept { return impl_->phase; }
std::uint32_t NativeFileStoreRequestAcquired::active_call_site() const noexcept { return impl_->active; }
std::uint32_t NativeFileStoreRequestAcquired::failure_site() const noexcept { return impl_->failed_at; }

bool request_native_file_store_00be7cd0(void* store, const void* original_name,
    std::uint32_t callback_address, NativeFileStoreRequestContext& context,
    NativeFileStoreRequestAcquired& acquired) {
    auto& frame = *acquired.impl_;
    if (frame.phase != NativeFileStoreRequestPhase::fresh) {
        throw std::logic_error("Native FileStore request frame cannot replay");
    }
    auto* const resolved = frame.resolved;
    auto& iterator = frame.iterator;
    frame.callback = callback_address;
    const auto finish = [&](bool result) {
        frame.active = 0x00be7df7;
        destroy_native_string_header_0041dd20(resolved, context.strings);
        frame.resolved_armed = false;
        frame.active = 0;
        frame.phase = NativeFileStoreRequestPhase::complete;
        return result;
    };
    try {
        frame.phase = NativeFileStoreRequestPhase::normalizing;
        frame.active = 0x00be7d04;
        copy_construct_native_resource_path_header_00bee780(resolved, original_name, context.strings);
        frame.resolved_armed = true;
        frame.phase = NativeFileStoreRequestPhase::resolving;
        frame.active = 0x00be7d1c;
        if (!context.dispatch.resolve_existing_name_00bdf4c0(context.actual_published_0109ceec, resolved)) {
            read_log_name(original_name);
            return finish(false);
        }
        frame.phase = NativeFileStoreRequestPhase::resident;
        frame.active = 0x00be7d9b;
        auto* const resident_tree = at(store, 0x14);
        find_native_file_store_open_name_00be5e90(resident_tree, &iterator, resolved,
            context.invalid_parameters);
        auto* const resident_node = iterator.node_04;
        auto* const resident_head = head(resident_tree);
        if (!iterator.owner_00 || iterator.owner_00 != resident_tree) {
            frame.active = 0x00be7db0;
            invalid(context.invalid_parameters);
        }
        if (resident_node != resident_head) {
            read_log_name(resolved);
            return finish(true);
        }
        frame.phase = NativeFileStoreRequestPhase::pending;
        frame.active = 0x00be7e21;
        auto* const pending_tree = at(store, 0x20);
        find_native_file_store_pending_name_00be5f00(pending_tree, &iterator, resolved,
            context.invalid_parameters);
        auto* const pending_head = head(pending_tree);
        if (!iterator.owner_00 || iterator.owner_00 != pending_tree) {
            frame.active = 0x00be7e35;
            invalid(context.invalid_parameters);
        }
        if (iterator.node_04 != pending_head) {
            read_log_name(resolved);
            return finish(true);
        }
        frame.phase = NativeFileStoreRequestPhase::inserting;
        frame.active = 0x00be7e66;
        construct_native_file_store_pending_pair_00be6120(frame.first_pair, resolved,
            &frame.callback, context.strings);
        frame.first_armed = true;
        frame.active = 0x00be7e75;
        copy_native_file_store_pending_pair_00be62e0(frame.second_pair, frame.first_pair, context.strings);
        frame.second_armed = true;
        frame.active = 0x00be7e8b;
        insert_native_file_store_pending_pair_00be7460(pending_tree, frame.insertion_output,
            frame.second_pair, context.strings, context.invalid_parameters);
        frame.second_armed = false;
        destroy_native_string_header_0041dd20(frame.second_pair, context.strings);
        frame.first_armed = false;
        destroy_native_string_header_0041dd20(frame.first_pair, context.strings);
        frame.phase = NativeFileStoreRequestPhase::submitting;
        frame.active = 0x00be7eef;
        if (context.dispatch.open_file_overlapped_00bdda10(context.actual_published_0109ceec,
                resolved, original_name, 0x00be7b20u, 2u)) return finish(true);
        read_log_name(original_name);
        frame.phase = NativeFileStoreRequestPhase::rollback;
        frame.active = 0x00be7f1d;
        erase_native_file_store_pending_name_00be79c0(pending_tree, resolved,
            context.strings, context.invalid_parameters);
        frame.phase = NativeFileStoreRequestPhase::notifying;
        frame.active = 0x00be7f2a;
        notify_native_vfs_request_failure_00bd9e30(context.actual_published_0109ceec,
            0xffffffffu, context.dispatch);
        return finish(false);
    } catch (...) {
        // An interrupted native provider/resolver can retain these actual
        // headers. Keep their exact completed/partial state until the owner
        // diagnoses that boundary; do not speculate about native FH3 cleanup.
        frame.failed_at = frame.active;
        frame.phase = NativeFileStoreRequestPhase::failed;
        throw;
    }
}
} // namespace bsp
