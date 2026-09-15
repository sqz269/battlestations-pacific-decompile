#include "bsp/native_soldier_loop_lengths.hpp"
#include "bsp/native_soldier_registry_lifetime.hpp"

#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"

#include <cstring>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native SoldierClass LoopLengths tree requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Access = detail::TreeInsertAccess<0x18, 0x19>;

void* at(const void* value, std::size_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(value) + offset);
}
template<class T> volatile T& field(const void* value, std::size_t offset = 0) noexcept {
    return *static_cast<volatile T*>(at(value, offset));
}
std::uint32_t string_length(const void* header) noexcept {
    return field<std::uint32_t>(header, 0);
}
char* string_data(const void* header) noexcept {
    return field<char*>(header, 4);
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    if (callbacks.invalid_parameter) callbacks.invalid_parameter(callbacks.context);
    else _invalid_parameter_noinfo();
}
bool less(const void* left_header, const void* right_header) {
    return less_native_string_headers_00443d00(left_header, right_header);
}
const void* key(const void* pair_or_node, std::size_t offset = 0) noexcept {
    return at(pair_or_node, offset);
}

void release_captured_string_block(char* captured_data, std::uint32_t size,
    NativeStringRawPoolContext& strings) {
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,
        strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, captured_data, size,
        strings.actual_small_returns_disabled_01090aa4);
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

void publish_insert_result(NativeSoldierLoopLengthInsertResult* output,
    void* owner, void* node, std::uint8_t inserted) noexcept {
    Access::word(output, 4) = node;
    Access::byte(output, 8) = inserted;
    Access::word(output, 0) = owner;
}
} // namespace

// The saved instruction comparison proves these specializations have the same
// field/load/branch/call schedule after relocating corresponding helper calls.
void* lower_bound_native_soldier_loop_lengths_00443d60(void* tree, const void* key_header) {
    return lower_bound_native_soldier_registry_004afe50(tree, key_header);
}
bool equal_native_soldier_loop_iterators_004435a0(const NativeKeyboardTreeIterator& left,
    const NativeKeyboardTreeIterator& right, const SingletonLifetimeCallbacks& calls) {
    return equal_native_soldier_registry_iterators_004af530(left, right, calls);
}
void increment_native_soldier_loop_iterator_00443880(NativeKeyboardTreeIterator& iterator,
    const SingletonLifetimeCallbacks& calls) {
    increment_native_soldier_registry_iterator_004afb50(iterator, calls);
}
void decrement_native_soldier_loop_iterator_004437f0(NativeKeyboardTreeIterator& iterator,
    const SingletonLifetimeCallbacks& calls) {
    decrement_native_soldier_registry_iterator_004afbe0(iterator, calls);
}
void rotate_native_soldier_loop_left_00443ba0(void* tree, void* node) noexcept {
    rotate_native_soldier_registry_left_004afaa0(tree, node);
}
void rotate_native_soldier_loop_right_004436e0(void* tree, void* node) noexcept {
    rotate_native_soldier_registry_right_004afaf0(tree, node);
}
void* minimum_native_soldier_loop_node_004437c0(void* node) noexcept {
    return minimum_native_soldier_registry_node_004af680(node);
}
void* maximum_native_soldier_loop_node_004437a0(void* node) noexcept {
    return maximum_native_soldier_registry_node_004af6c0(node);
}
void destroy_native_soldier_loop_subtree_00444760(void* tree, void* node,
    NativeStringRawPoolContext& strings) {
    destroy_native_soldier_registry_subtree_004b0600(tree, node, strings);
}
NativeKeyboardTreeIterator* erase_native_soldier_loop_iterator_00444490(void* tree,
    NativeKeyboardTreeIterator* output, NativeKeyboardTreeIterator input,
    NativeStringRawPoolContext& strings, const SingletonLifetimeCallbacks& calls) {
    return erase_native_soldier_registry_iterator_004b0330(tree, output, input, strings, calls);
}
NativeKeyboardTreeIterator* erase_native_soldier_loop_range_00444b10(void* tree,
    NativeKeyboardTreeIterator* output, NativeKeyboardTreeIterator first,
    NativeKeyboardTreeIterator last, NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& calls) {
    return erase_native_soldier_registry_range_004b09a0(tree, output, first, last, strings, calls);
}
void destroy_native_soldier_loop_pair_00443fb0(
    void* actual_pair, NativeStringRawPoolContext& strings) {
    destroy_native_string_header_0041dd20(actual_pair, strings);
}

void* initialize_native_soldier_loop_node_00444150(
    void* node, void* left_node, void* parent_node, void* right_node,
    const NativeSoldierLoopLengthPairStorage* pair, std::uint8_t node_color,
    NativeStringRawPoolContext& strings) {
    Access::left(node) = left_node;
    Access::parent(node) = parent_node;
    Access::right(node) = right_node;
    auto* const destination_key = at(node, 0x0c);
    field<std::uint32_t>(destination_key) = 0;
    field<char*>(destination_key, 4) = nullptr;
    if (destination_key != pair) {
        resize_native_string_header_0041dd40(
            destination_key, strings, string_length(pair), true);
        if (string_length(pair) != 0) {
            const auto copied_length = string_length(destination_key);
            auto* const source_data = string_data(pair);
            auto* const destination_data = string_data(destination_key);
            std::memmove(destination_data, source_data, copied_length);
        }
    }
    const void* source_value = at(pair, 8);
    void* destination_value = at(node, 0x14);
    __asm {
        mov eax, source_value
        mov edx, destination_value
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
    Access::color(node) = node_color;
    Access::byte(node, 0x19) = 0;
    return node;
}

void* allocate_native_soldier_loop_node_004441e0(
    void* left_node, void* parent_node, void* right_node,
    const NativeSoldierLoopLengthPairStorage* pair, std::uint8_t node_color,
    NativeStringRawPoolContext& strings) {
    void* const node = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x1c, 0x1c});
    try {
        if (node != nullptr) {
            initialize_native_soldier_loop_node_00444150(
                node, left_node, parent_node, right_node, pair, node_color, strings);
        }
    } catch (...) {
        singleton_lifetime_free(node);
        throw;
    }
    return node;
}

NativeKeyboardTreeIterator* link_native_soldier_loop_node_004442a0(
    void* actual_tree, NativeKeyboardTreeIterator* output,
    std::uint8_t insert_left, void* parent_node,
    const NativeSoldierLoopLengthPairStorage* pair,
    NativeStringRawPoolContext& strings) {
    detail::link_tree_node<Access>(actual_tree, output, insert_left,
        parent_node, pair, 0x15555554u,
        [&strings](void* left_node, void* parent, void* right_node,
            const void* value, std::uint8_t color) {
            return allocate_native_soldier_loop_node_004441e0(
                left_node, parent, right_node,
                static_cast<const NativeSoldierLoopLengthPairStorage*>(value),
                color, strings);
        },
        rotate_native_soldier_loop_left_00443ba0,
        rotate_native_soldier_loop_right_004436e0,
        throw_tree_length_error);
    return output;
}

NativeSoldierLoopLengthInsertResult* insert_native_soldier_loop_pair_004447c0(
    void* actual_tree, NativeSoldierLoopLengthInsertResult* output,
    const NativeSoldierLoopLengthPairStorage* pair,
    NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    return detail::insert_unique_tree_pair<Access, NativeKeyboardTreeIterator>(
        actual_tree, output, pair,
        [pair](void* node) { return less(key(pair), key(node, 0x0c)); },
        [pair](void* node) { return less(key(node, 0x0c), key(pair)); },
        [&callbacks](NativeKeyboardTreeIterator& iterator) {
            decrement_native_soldier_loop_iterator_004437f0(iterator, callbacks);
        },
        [&strings](void* tree, NativeKeyboardTreeIterator* iterator,
            std::uint8_t insert_left, void* parent, const void* value) {
            return link_native_soldier_loop_node_004442a0(
                tree, iterator, insert_left, parent,
                static_cast<const NativeSoldierLoopLengthPairStorage*>(value), strings);
        },
        publish_insert_result);
}

NativeKeyboardTreeIterator* insert_hint_native_soldier_loop_pair_00444910(
    void* actual_tree, NativeKeyboardTreeIterator* output,
    NativeKeyboardTreeIterator hint,
    const NativeSoldierLoopLengthPairStorage* pair,
    NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    if (Access::count(actual_tree) == 0) {
        return link_native_soldier_loop_node_004442a0(
            actual_tree, output, 1, Access::head(actual_tree), pair, strings);
    }

    void* const captured_minimum = Access::left(Access::head(actual_tree));
    void* const captured_owner = hint.owner;
    if (captured_owner == nullptr || captured_owner != actual_tree) invalid(callbacks);
    void* const captured_node = hint.node;
    if (captured_node == captured_minimum) {
        if (less(key(pair), key(captured_node, 0x0c))) {
            return link_native_soldier_loop_node_004442a0(
                actual_tree, output, 1, captured_node, pair, strings);
        }
    } else {
        // 00444992 loads this head before the second returning validation call.
        void* const captured_head = Access::head(actual_tree);
        if (captured_owner == nullptr || captured_owner != actual_tree) invalid(callbacks);
        if (captured_node == captured_head) {
            if (less(key(Access::right(Access::head(actual_tree)), 0x0c), key(pair))) {
                return link_native_soldier_loop_node_004442a0(
                    actual_tree, output, 0,
                    Access::right(Access::head(actual_tree)), pair, strings);
            }
        } else {
            if (less(key(pair), key(captured_node, 0x0c))) {
                NativeKeyboardTreeIterator predecessor{captured_owner, captured_node};
                decrement_native_soldier_loop_iterator_004437f0(predecessor, callbacks);
                if (less(key(predecessor.node, 0x0c), key(pair))) {
                    if (Access::sentinel(Access::right(predecessor.node))) {
                        return link_native_soldier_loop_node_004442a0(
                            actual_tree, output, 0, predecessor.node, pair, strings);
                    }
                    return link_native_soldier_loop_node_004442a0(
                        actual_tree, output, 1, captured_node, pair, strings);
                }
            }
            if (less(key(captured_node, 0x0c), key(pair))) {
                NativeKeyboardTreeIterator successor{captured_owner, captured_node};
                NativeKeyboardTreeIterator end{actual_tree, Access::head(actual_tree)};
                increment_native_soldier_loop_iterator_00443880(successor, callbacks);
                if (equal_native_soldier_loop_iterators_004435a0(
                        successor, end, callbacks) ||
                    less(key(pair), key(successor.node, 0x0c))) {
                    if (Access::sentinel(Access::right(captured_node))) {
                        return link_native_soldier_loop_node_004442a0(
                            actual_tree, output, 0, captured_node, pair, strings);
                    }
                    return link_native_soldier_loop_node_004442a0(
                        actual_tree, output, 1, successor.node, pair, strings);
                }
            }
        }
    }

    NativeSoldierLoopLengthInsertResult inserted;
    insert_native_soldier_loop_pair_004447c0(
        actual_tree, &inserted, pair, strings, callbacks);
    output->owner = inserted.iterator_00.owner;
    output->node = inserted.iterator_00.node;
    return output;
}

float* get_or_insert_native_soldier_loop_length_00444be0(
    void* actual_tree, const void* actual_key_header,
    NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    void* selected = lower_bound_native_soldier_loop_lengths_00443d60(
        actual_tree, actual_key_header);
    if (actual_tree == nullptr) invalid(callbacks);
    if (selected == Access::head(actual_tree) ||
        less(actual_key_header, key(selected, 0x0c))) {
        NativeSoldierLoopLengthPairStorage temporary;
        temporary.key_length_00 = 0;
        temporary.key_data_04 = nullptr;
        char* captured_data = nullptr;
        if (&temporary != actual_key_header) {
            resize_native_string_header_0041dd40(
                &temporary, strings, string_length(actual_key_header), true);
            const bool source_nonempty = string_length(actual_key_header) != 0;
            captured_data = temporary.key_data_04;
            if (source_nonempty) {
                const auto copied_length = temporary.key_length_00;
                auto* const source_data = string_data(actual_key_header);
                std::memmove(captured_data, source_data, copied_length);
            }
        }
        temporary.value_08 = 0.0f;

        NativeKeyboardTreeIterator inserted;
        try {
            insert_hint_native_soldier_loop_pair_00444910(
                actual_tree, &inserted,
                NativeKeyboardTreeIterator{actual_tree, selected},
                &temporary, strings, callbacks);
        } catch (...) {
            try {
                destroy_native_soldier_loop_pair_00443fb0(&temporary, strings);
            } catch (...) {
                std::terminate();
            }
            throw;
        }
        actual_tree = inserted.owner;
        selected = inserted.node;
        if (captured_data != nullptr) {
            release_captured_string_block(
                captured_data, temporary.key_length_00 + 1u, strings);
        }
    }
    if (actual_tree == nullptr) invalid(callbacks);
    if (selected == Access::head(actual_tree)) invalid(callbacks);
    return reinterpret_cast<float*>(at(selected, 0x14));
}
} // namespace bsp
