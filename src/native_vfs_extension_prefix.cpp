#include "bsp/native_vfs_extension_prefix.hpp"
#include "bsp/engine_services_cc13.hpp"
#include "bsp/wire_goal2_chain148_switches.hpp"
#include "bsp/wire_goal2_chain208_switches.hpp"
#include "bsp/wire_goal4_chain457_switches.hpp"

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
static_assert(offsetof(Header, length) == 0 && offsetof(Header, data) == 4);

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

template<bool RegisteredPair = false> struct Pair {
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
        // 00bdb7c0 calls the registered-pair destructor; OFF keeps local releases.
        if constexpr (RegisteredPair && kWireGoal2Chain148Bound) {
            if (state >= 0) {
                engine_services_cc13::destroy_string_pair_00bdb7c0(
                    engine_services_cc13::NativeStringPairView{
                        *reinterpret_cast<NativeString*>(&first),
                        *reinterpret_cast<NativeString*>(&second)}, strings);
            }
        } else {
            if (state >= 1 && second.data)
                destroy_native_string_header_0041dd20(&second, strings);
            if (state >= 0 && first.data)
                destroy_native_string_header_0041dd20(&first, strings);
        }
    }
    void construct_extension_first(const void* extension, const void* directory) {
        copy_native_string_header_00be0a30_fragment(&first, strings, extension);
        state = 0;
        copy_native_string_header_00be0a30_fragment(&second, strings, directory);
        state = 1;
    }
    // 00BE14DC CALL 00bdcbb0 on this pair. State 1 is set first: if the second copy throws,
    // the destructor's null-data guards release only the first string, as 00bdcbb0's own
    // state-0 funclet does; 00BE14E5 sets the caller's state after the return.
    void construct_with_pair_constructor(const void* extension, const void* directory) {
        state = 1;
        engine_services_cc13::construct_string_pair_00bdcbb0(
            engine_services_cc13::NativeStringPairView{
                *reinterpret_cast<NativeString*>(&first),
                *reinterpret_cast<NativeString*>(&second)},
            *static_cast<const NativeString*>(extension),
            *static_cast<const NativeString*>(directory), strings);
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

// Borrows the existing +54h tree, pooled-string domain and validation boundary.
// Directory normalization stays with the host's counted-header implementation.
struct PrefixFindAdapter final : engine_services_cc13::ExtensionPrefixFindHost {
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& callbacks;
    PrefixFindAdapter(ActualNativeStringPoolStorage& storage,
        const SingletonLifetimeCallbacks& validation) : strings(storage), callbacks(validation) {}
    void slash_terminate(NativeString& path) override {
        normalize_directory(*reinterpret_cast<Header*>(&path), strings);
    }
    void* upper_bound(engine_services_cc13::NativeContainerHeader& tree,
        const NativeString& key) override {
        return bsp::upper_bound(&tree, *reinterpret_cast<const Header*>(&key));
    }
    void* lower_bound(engine_services_cc13::NativeContainerHeader& tree,
        const NativeString& key) override {
        return lower_bound_native_physical_index_00bda260(&tree, &key);
    }
    void advance(engine_services_cc13::NativeIterator& iterator) override {
        advance_native_physical_index_00bd9860(&iterator, callbacks);
    }
    void invalid_parameter() override { callbacks.invalid_parameter(callbacks.context); }
};

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

void insert_pair(void* tree, const Pair<>& pair,
    ActualNativeStringPoolStorage& strings) {
    if constexpr (kWireGoal4Chain457Bound) {
        // bsp.py show 00BE0B90 / 00BE0370 / 00BDF700: extension-only
        // descent, native 20h node construction, then RB insertion. Borrow
        // the SAME manager+54h tree and normalized, pooled two-header pair;
        // the earlier exact-pair lookup still owns duplicate suppression.
        engine_services_cc13::InsertResult result{};
        engine_services_cc13::prefix_tree_insert_equal_00be0b90(
            *static_cast<engine_services_cc13::NativeContainerHeader*>(tree),
            result, &pair.first, strings);
    } else {
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
}
} // namespace

void register_native_vfs_extension_prefix_00be1480(void* manager,
    const void* extension, const void* directory, ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    void* tree = at(manager, 0x54);
    void* const captured_head = Tree::head(tree);
    // 00bdc3f0 calls the native pair lookup; OFF keeps the local tree scan.
    if constexpr (kWireGoal2Chain148Bound) {
        PrefixFindAdapter host(strings, callbacks);
        engine_services_cc13::NativeIterator result;
        engine_services_cc13::vfs_find_extension_prefix_pair_00bdc3f0(
            *static_cast<engine_services_cc13::NativeContainerHeader*>(tree), result,
            *static_cast<const NativeString*>(extension),
            *static_cast<const NativeString*>(directory), strings, host);
        if (result.node != captured_head) return;
    } else {
        if (find_pair(tree, extension, directory, strings, callbacks) != captured_head) return;
    }
    Pair<kWireGoal2Chain148Bound> normalized(strings);
    // 00BE14DC: 00bdcbb0 builds the extension-first pair; OFF keeps the local two-copy stand-in.
    if constexpr (kWireGoal2Chain208Bound) {
        normalized.construct_with_pair_constructor(extension, directory);
    } else {
        normalized.construct_extension_first(extension, directory);
    }
    normalized.lower_and_trail();
    Pair copied(strings);
    copied.construct_extension_first(&normalized.first, &normalized.second);
    insert_pair(tree, copied, strings);
}
} // namespace bsp
