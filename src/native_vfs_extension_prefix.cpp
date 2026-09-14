#include "bsp/native_vfs_extension_prefix.hpp"

#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_physical_index_tree.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
using Tree = detail::TreeInsertAccess<0x1c, 0x1d>;
struct Header { std::uint32_t length; char* data; };
static_assert(sizeof(Header) == 8);

void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
template<class T> volatile T& field(const void* base, std::uint32_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(at(base, offset));
}
int compare(const Header& left, const Header& right) noexcept {
    if (left.length == 0) return -(right.length != 0);
    if (right.length == 0) return 1;
    return _stricmp(left.data, right.data);
}
Header& header(void* base, std::uint32_t offset) noexcept {
    return *static_cast<Header*>(at(base, offset));
}
const Header& header(const void* base, std::uint32_t offset) noexcept {
    return *static_cast<const Header*>(at(base, offset));
}

// Native normalizes a counted directory after lowercasing. Empty directories
// retain the original unsafe data[length-1] access; startup supplies nonempty.
void normalize_directory(Header& value, ActualNativeStringPoolStorage& strings) {
    for (std::uint32_t i = 0; i < value.length; ++i)
        if (value.data[i] == '\\') value.data[i] = '/';
    const char* current = value.data ? value.data : "";
    if (current[value.length - 1] != '/') {
        const auto old_length = value.length;
        resize_native_string_header_0041dd40(&value, strings, old_length + 1u, true);
        value.data[old_length] = '/';
    }
}

struct Pair {
    Header first{};
    Header second{};
    ActualNativeStringPoolStorage& strings;
    int state{-1};
    bool directory_first{};
    explicit Pair(ActualNativeStringPoolStorage& storage) : strings(storage) {}
    ~Pair() noexcept {
        if (directory_first) {
            // BDC3F0 builds directory then extension and unwinds them in
            // reverse order, unlike BDCBB0's extension-first pair.
            if (first.data) destroy_native_string_header_0041dd20(&first, strings);
            if (second.data) destroy_native_string_header_0041dd20(&second, strings);
            return;
        }
        if (state >= 1 && second.data)
            destroy_native_string_header_0041dd20(&second, strings);
        if (state >= 0 && first.data)
            destroy_native_string_header_0041dd20(&first, strings);
    }
    void construct_extension_first(const void* extension, const void* directory) {
        copy_native_string_header_00be0a30_fragment(&first, strings, extension);
        state = 0;
        copy_native_string_header_00be0a30_fragment(&second, strings, directory);
        state = 1;
    }
    void lower_and_trail() {
        lowercase_native_string_header_004bcc00(&first);
        lowercase_native_string_header_004bcc00(&second);
        normalize_directory(second, strings);
    }
};

// BDC3F0 performs its own temporary copies in directory, extension order.
// Upper bound is captured before lower bound; both compare extension alone.
void* upper_bound(void* tree, const Header& key) noexcept {
    void* candidate = Tree::head(tree);
    void* current = Tree::parent(candidate);
    while (!Tree::sentinel(current)) {
        if (compare(key, header(current, 0x0c)) < 0) {
            candidate = current;
            current = Tree::left(current);
        } else {
            current = Tree::right(current);
        }
    }
    return candidate;
}

void* find_pair(void* tree, const void* extension, const void* directory,
    ActualNativeStringPoolStorage& strings, const SingletonLifetimeCallbacks& callbacks) {
    Pair query(strings);
    query.directory_first = true;
    // Keep the native copy order even though the final pair layout is ext/dir.
    copy_native_string_header_00be0a30_fragment(&query.second, strings, directory);
    query.state = 1; // First header is empty; release second on later failure.
    copy_native_string_header_00be0a30_fragment(&query.first, strings, extension);
    query.lower_and_trail();
    void* const end = upper_bound(tree, query.first);
    void* cursor = lower_bound_native_physical_index_00bda260(tree, &query.first);
    while (cursor != end && cursor != Tree::head(tree)) {
        const Header& current = header(cursor, 0x14);
        if (current.length == query.second.length &&
            (current.length == 0 || _stricmp(current.data, query.second.data) == 0))
            return cursor;
        void* iterator[2]{tree, cursor};
        advance_native_physical_index_00bd9860(iterator, callbacks);
        cursor = iterator[1];
    }
    return Tree::head(tree);
}

void* allocate_node(void* left, void* parent, void* right, const void* pair,
    std::uint8_t color, ActualNativeStringPoolStorage& strings) {
    void* node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x20, 0x20});
    Tree::left(node) = left;
    Tree::parent(node) = parent;
    Tree::right(node) = right;
    field<std::uint32_t>(node, 0x0c) = 0;
    field<char*>(node, 0x10) = nullptr;
    int state{-1};
    try {
        copy_native_string_header_00be0a30_fragment(at(node, 0x0c), strings, pair);
        state = 0;
        field<std::uint32_t>(node, 0x14) = 0;
        field<char*>(node, 0x18) = nullptr;
        copy_native_string_header_00be0a30_fragment(at(node, 0x14), strings, at(pair, 8));
        Tree::color(node) = color;
        Tree::byte(node, 0x1d) = 0;
        return node;
    } catch (...) {
        if (state >= 0) destroy_native_string_header_0041dd20(at(node, 0x0c), strings);
        singleton_lifetime_free(node);
        throw;
    }
}

void insert_pair(void* tree, const Pair& pair,
    ActualNativeStringPoolStorage& strings) {
    void* parent = Tree::head(tree);
    void* current = Tree::parent(parent);
    std::uint8_t insert_left = 1;
    while (!Tree::sentinel(current)) {
        parent = current;
        insert_left = static_cast<std::uint8_t>(
            compare(pair.first, header(current, 0x0c)) < 0);
        current = insert_left ? Tree::left(current) : Tree::right(current);
    }
    std::byte result[12]; // BE0B90 publishes owner,node,success byte.
    detail::link_tree_node<Tree>(tree, result, insert_left, parent, &pair.first,
        0x0ffffffeu,
        [&](void* left, void* parent_node, void* right, const void* source,
            std::uint8_t color) {
            return allocate_node(left, parent_node, right, source, color, strings);
        },
        [](void* owner, void* node) { rotate_native_physical_index_left_00bda090(owner, node); },
        [](void* owner, void* node) { rotate_native_physical_index_right_00bd93e0(owner, node); },
        [] { throw std::length_error("map/set<T> too long"); });
}
} // namespace

void register_native_vfs_extension_prefix_00be1480(void* manager,
    const void* extension, const void* directory, ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    void* tree = at(manager, 0x54);
    void* const captured_head = Tree::head(tree);
    if (find_pair(tree, extension, directory, strings, callbacks) != captured_head) return;
    Pair normalized(strings);
    normalized.construct_extension_first(extension, directory);
    normalized.lower_and_trail();
    Pair copied(strings);
    copied.construct_extension_first(&normalized.first, &normalized.second);
    insert_pair(tree, copied, strings);
}
} // namespace bsp
