#include "bsp/native_damageable_crew_tree.hpp"

#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native damageable crew-tree insertion requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Access = detail::TreeInsertAccess<0x54, 0x55>;

void invalid(const SingletonLifetimeCallbacks& callbacks) {
    if (callbacks.invalid_parameter) callbacks.invalid_parameter(callbacks.context);
    else _invalid_parameter_noinfo();
}

std::int32_t key(const void* pair_or_node, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::int32_t*>(
        static_cast<const std::uint8_t*>(pair_or_node) + offset);
}

struct CompletedLengthMessage {
    NativeLegacySboStringStorage& storage;
    ~CompletedLengthMessage() noexcept {
        native_legacy_sbo_string_destroy_004072d0(storage);
    }
};

[[noreturn]] void throw_tree_length_error() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(
        message, "map/set<T> too long", 19);
    const CompletedLengthMessage completed{message};
    throw NativeHardwareLayoutTreeLengthError{message};
}

void publish_insert_result(NativeDamageableCrewTreeInsertResult* output,
    void* owner, void* node, std::uint8_t inserted) noexcept {
    Access::word(output, 0) = owner;
    Access::word(output, 4) = node;
    Access::byte(output, 8) = inserted;
}
} // namespace

bool equal_native_damageable_crew_iterators_00876340(
    const NativeKeyboardTreeIterator& left,
    const NativeKeyboardTreeIterator& right,
    const SingletonLifetimeCallbacks& callbacks) {
    if (left.owner == nullptr || left.owner != right.owner) invalid(callbacks);
    return left.node == right.node;
}

void rotate_native_damageable_crew_right_00876630(
    void* tree, void* node) noexcept {
    auto* const replacement = Access::left(node);
    Access::left(node) = Access::right(replacement);
    auto* const moved_child = Access::right(replacement);
    if (!Access::sentinel(moved_child)) Access::parent(moved_child) = node;
    Access::parent(replacement) = Access::parent(node);
    auto* const head = Access::head(tree);
    if (node == Access::parent(head)) {
        Access::parent(head) = replacement;
    } else {
        auto* const current_parent = Access::parent(node);
        if (node == Access::right(current_parent)) Access::right(current_parent) = replacement;
        else Access::left(current_parent) = replacement;
    }
    Access::right(replacement) = node;
    Access::parent(node) = replacement;
}

void rotate_native_damageable_crew_left_008773a0(
    void* tree, void* node) noexcept {
    detail::rotate_left_inlined<Access>(tree, node);
}

void decrement_native_damageable_crew_iterator_008767a0(
    NativeKeyboardTreeIterator& iterator,
    const SingletonLifetimeCallbacks& callbacks) {
    detail::decrement_tree_iterator<Access>(
        &iterator, [&callbacks] { invalid(callbacks); });
}

void* allocate_native_damageable_crew_tree_node_00877fe0(
    void* left, void* parent, void* right,
    const NativeDamageableCrewTreePairStorage* pair, std::uint8_t color) {
    void* const node = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x58, 0x58});
    if (node != nullptr) {
        Access::left(node) = left;
        Access::right(node) = right;
        Access::parent(node) = parent;
        void* destination = static_cast<std::uint8_t*>(node) + 0x0c;
        const void* source = pair;
        __asm {
            mov edi, destination
            mov esi, source
            mov ecx, 0x12
            rep movsd
        }
        Access::color(node) = color;
        Access::byte(node, 0x55) = 0;
    }
    return node;
}

NativeKeyboardTreeIterator* link_native_damageable_crew_tree_node_0087a5f0(
    void* tree, NativeKeyboardTreeIterator* output,
    std::uint8_t insert_left, void* parent,
    const NativeDamageableCrewTreePairStorage* pair) {
    detail::link_tree_node<Access>(tree, output, insert_left, parent, pair,
        0x038e38e2u,
        [](void* left, void* parent, void* right, const void* value,
            std::uint8_t color) {
            return allocate_native_damageable_crew_tree_node_00877fe0(
                left, parent, right,
                static_cast<const NativeDamageableCrewTreePairStorage*>(value),
                color);
        },
        rotate_native_damageable_crew_left_008773a0,
        rotate_native_damageable_crew_right_00876630,
        throw_tree_length_error);
    return output;
}

NativeDamageableCrewTreeInsertResult* insert_native_damageable_crew_tree_pair_0087adb0(
    void* tree, NativeDamageableCrewTreeInsertResult* output,
    const NativeDamageableCrewTreePairStorage* pair,
    const SingletonLifetimeCallbacks& callbacks) {
    return detail::insert_unique_tree_pair<Access, NativeKeyboardTreeIterator>(
        tree, output, pair,
        [pair](void* node) { return key(pair) < key(node, 0x0c); },
        [pair](void* node) { return key(node, 0x0c) < key(pair); },
        [&callbacks](NativeKeyboardTreeIterator& iterator) {
            decrement_native_damageable_crew_iterator_008767a0(iterator, callbacks);
        },
        [](void* tree, NativeKeyboardTreeIterator* iterator,
            std::uint8_t insert_left, void* parent, const void* value) {
            return link_native_damageable_crew_tree_node_0087a5f0(
                tree, iterator, insert_left, parent,
                static_cast<const NativeDamageableCrewTreePairStorage*>(value));
        },
        publish_insert_result);
}

NativeKeyboardTreeIterator* insert_hint_native_damageable_crew_tree_pair_0087b260(
    void* tree, NativeKeyboardTreeIterator* output,
    NativeKeyboardTreeIterator position,
    const NativeDamageableCrewTreePairStorage* pair,
    const SingletonLifetimeCallbacks& callbacks) {
    if (Access::count(tree) == 0) {
        return link_native_damageable_crew_tree_node_0087a5f0(
            tree, output, 1, Access::head(tree), pair);
    }

    void* const minimum = Access::left(Access::head(tree));
    if (position.owner == nullptr || position.owner != tree) invalid(callbacks);
    if (position.node == minimum) {
        if (key(pair) < key(position.node, 0x0c)) {
            return link_native_damageable_crew_tree_node_0087a5f0(
                tree, output, 1, position.node, pair);
        }
    } else {
        if (position.owner == nullptr || position.owner != tree) invalid(callbacks);
        if (position.node == Access::head(tree)) {
            void* const maximum = Access::right(Access::head(tree));
            if (key(maximum, 0x0c) < key(pair)) {
                return link_native_damageable_crew_tree_node_0087a5f0(
                    tree, output, 0, maximum, pair);
            }
        } else {
            if (key(pair) < key(position.node, 0x0c)) {
                NativeKeyboardTreeIterator previous = position;
                decrement_native_damageable_crew_iterator_008767a0(previous, callbacks);
                if (key(previous.node, 0x0c) < key(pair)) {
                    if (Access::sentinel(Access::right(previous.node))) {
                        return link_native_damageable_crew_tree_node_0087a5f0(
                            tree, output, 0, previous.node, pair);
                    }
                    return link_native_damageable_crew_tree_node_0087a5f0(
                        tree, output, 1, position.node, pair);
                }
            }
            if (key(position.node, 0x0c) < key(pair)) {
                const NativeKeyboardTreeIterator end{tree, Access::head(tree)};
                NativeKeyboardTreeIterator next = position;
                advance_native_class_frame_008772b0(&next, callbacks);
                if (equal_native_damageable_crew_iterators_00876340(
                        next, end, callbacks) ||
                    key(pair) < key(next.node, 0x0c)) {
                    if (Access::sentinel(Access::right(position.node))) {
                        return link_native_damageable_crew_tree_node_0087a5f0(
                            tree, output, 0, position.node, pair);
                    }
                    return link_native_damageable_crew_tree_node_0087a5f0(
                        tree, output, 1, next.node, pair);
                }
            }
        }
    }

    NativeDamageableCrewTreeInsertResult inserted;
    insert_native_damageable_crew_tree_pair_0087adb0(
        tree, &inserted, pair, callbacks);
    output->owner = inserted.iterator_00.owner;
    output->node = inserted.iterator_00.node;
    return output;
}

NativeDamageableCrewMappedStorage* get_or_insert_native_damageable_crew_mapped_0087b750(
    void* tree, const std::int32_t* sought,
    const SingletonLifetimeCallbacks& callbacks) {
    void* candidate = Access::head(tree);
    void* node = Access::parent(candidate);
    while (!Access::sentinel(node)) {
        if (key(node, 0x0c) < *sought) {
            node = Access::right(node);
        } else {
            candidate = node;
            node = Access::left(node);
        }
    }

    if (candidate == Access::head(tree) || *sought < key(candidate, 0x0c)) {
        std::uint32_t uninitialized_mapped[17];
        NativeDamageableCrewTreePairStorage pair;
        pair.key_00 = *sought;
        void* source = uninitialized_mapped;
        void* destination = pair.mapped_04.bytes_00;
        __asm {
            mov esi, source
            mov edi, destination
            mov ecx, 0x11
            rep movsd
        }
        NativeKeyboardTreeIterator inserted;
        insert_hint_native_damageable_crew_tree_pair_0087b260(
            tree, &inserted, NativeKeyboardTreeIterator{tree, candidate},
            &pair, callbacks);
        tree = inserted.owner;
        candidate = inserted.node;
    }

    if (tree == nullptr) invalid(callbacks);
    if (candidate == Access::head(tree)) invalid(callbacks);
    return reinterpret_cast<NativeDamageableCrewMappedStorage*>(
        static_cast<std::uint8_t*>(candidate) + 0x10);
}

} // namespace bsp
