#include "bsp/native_animation_registry.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/detail/native_tree_subtree_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native animation registry requires MSVC Win32.
#endif

namespace bsp {
namespace {
using W = std::uint32_t;
using I = std::int32_t;
using Tree = detail::TreeInsertAccess<0x18, 0x19>;
struct MirroredTree : Tree {
    static void* volatile& left(void* node) noexcept { return Tree::right(node); }
    static void* volatile& right(void* node) noexcept { return Tree::left(node); }
};
void* at(const void* p, W n = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<W>(p) + n);
}
template<class T = W> T read(const void* p, W n = 0) noexcept {
    return *static_cast<const volatile T*>(at(p, n));
}
template<class T> void write(void* p, W n, T value) noexcept {
    *static_cast<volatile T*>(at(p, n)) = value;
}
void invalid(NativeAnimationRegistryContext& context) {
    context.invalid_parameters.invalid_parameter(context.invalid_parameters.context);
}
void* allocate(W bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
void copy_x87_float(void* destination, const void* source) noexcept {
    __asm { mov eax, source }
    __asm { fld dword ptr [eax] }
    __asm { mov eax, destination }
    __asm { fstp dword ptr [eax] }
}
struct NamePair { W length; char* data; W index; };
static_assert(sizeof(NamePair) == 0xc);
struct Iterator { void* owner; void* node; };
struct InsertResult { Iterator iterator; std::uint8_t inserted; std::uint8_t padding[3]; };

void* allocate_name_head_00b77710() {
    void* const head = allocate(0x1c);
    if (head) write(head, 0, W{0});
    if (at(head, 4)) write(head, 4, W{0});
    if (at(head, 8)) write(head, 8, W{0});
    write(head, 0x18, std::uint8_t{1}); write(head, 0x19, std::uint8_t{0});
    return head;
}

void* initialize_name_node(void* node, void* left, void* parent, void* right,
    const void* pair, std::uint8_t color, NativeAnimationRegistryContext& context) {
    void* const name = at(node, 0xc);
    write(node, 0, left); write(node, 4, parent); write(node, 8, right);
    write(name, 0, W{0}); write(name, 4, W{0});
    if (name != pair) {
        resize_native_string_header_0041dd40(name, context.strings, read(pair), true);
        if (read(pair) != 0) {
            const W length = read(name);
            const void* const input = read<void*>(pair, 4);
            void* const output = read<void*>(name, 4);
            std::memcpy(output, input, length);
        }
    }
    write(node, 0x14, read(pair, 8));
    write(node, 0x18, color); write(node, 0x19, std::uint8_t{0});
    return node;
}
void* new_name_node(void* left, void* parent, void* right, const void* pair,
    std::uint8_t color, NativeAnimationRegistryContext& context) {
    void* const node = allocate(0x1c);
    if (node) {
        try { initialize_name_node(node, left, parent, right, pair, color, context); }
        catch (...) { singleton_lifetime_free(node); throw; }
    }
    return node;
}
[[noreturn]] void throw_tree_length() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15; message.length_14 = 0; message.buffer_04.inline_bytes[0] = 0;
    native_legacy_sbo_string_assign_counted_00408720(message, "map/set<T> too long", 19);
    struct Cleanup {
        NativeLegacySboStringStorage& value;
        ~Cleanup() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
    } cleanup{message};
    throw NativeHardwareLayoutTreeLengthError{message};
}
void* link_name_node(void* tree, void* output, std::uint8_t left, void* parent,
    const void* pair, NativeAnimationRegistryContext& context) {
    return detail::link_tree_node<Tree>(tree, output, left, parent, pair, 0x15555554u,
        [&context](void* a, void* b, void* c, const void* d, std::uint8_t e) {
            return new_name_node(a, b, c, d, e, context);
        },
        [](void* owner, void* node) { detail::rotate_left_inlined<Tree>(owner, node); },
        [](void* owner, void* node) { detail::rotate_left_inlined<MirroredTree>(owner, node); },
        [] { throw_tree_length(); });
}
void insert_name_pair(void* tree, InsertResult* output, const void* pair,
    NativeAnimationRegistryContext& context) {
    detail::insert_unique_tree_pair<Tree, Iterator>(tree, output, pair,
        [pair](void* node) { return less_native_string_headers_00443d00(pair, at(node, 0xc)); },
        [pair](void* node) { return less_native_string_headers_00443d00(at(node, 0xc), pair); },
        [&context](Iterator& iterator) {
            detail::decrement_tree_iterator<Tree>(&iterator, [&context] { invalid(context); });
        },
        [&context](void* owner, void* result, std::uint8_t left, void* parent, const void* value) {
            link_name_node(owner, result, left, parent, value, context);
        },
        [](InsertResult* result, void* owner, void* node, std::uint8_t inserted) {
            write(result, 4, node); write(result, 8, inserted); write(result, 0, owner);
        });
}
void release_captured_name(char* data, W length, NativeAnimationRegistryContext& context) noexcept {
    if (data) context.strings.release(data, length + 1u);
}
void clear_name_tree(void* tree, NativeAnimationRegistryContext& context) {
    // B79B00 passes the current tree's own begin/end, with no intervening
    // service call: B798C0's full-range branch is the reached specialization.
    detail::erase_tree_subtree_right_first<Tree>(Tree::parent(Tree::head(tree)),
        [](void* node) { return read<char*>(node, 0x10); },
        [&context](void* node, char* data) {
            if (data) context.strings.release(data, read(node, 0xc) + 1u);
        }, [](void* node) { singleton_lifetime_free(node); });
    auto* head = Tree::head(tree); Tree::parent(head) = head;
    head = Tree::head(tree); Tree::count(tree) = 0; Tree::left(head) = head;
    head = Tree::head(tree); Tree::right(head) = head;
}
} // namespace

void* construct_native_animation_registry_00b79a80(void* registry) {
    write(registry, 0, W{0x00ceb130}); write(registry, 4, W{1});
    write(registry, 0, W{0x00d62ef4});
    void* const head = allocate_name_head_00b77710();
    write(registry, 0xc, head); write(head, 0x19, std::uint8_t{1});
    auto* current = read<void*>(registry, 0xc); write(current, 4, current);
    current = read<void*>(registry, 0xc); write(current, 0, current);
    current = read<void*>(registry, 0xc); write(current, 8, current);
    write(registry, 0x10, W{0}); write(registry, 0x14, W{0});
    write(registry, 0x18, W{0}); write(registry, 0x1c, W{0});
    return registry;
}

std::uint32_t register_native_animation_name_00b79740(void* registry,
    const void* name, NativeAnimationRegistryContext& context) {
    void* const tree = at(registry, 8);
    Iterator found;
    find_native_file_store_name_00be5a50(tree, &found, name, context.invalid_parameters);
    void* const owner = found.owner;
    void* const head = Tree::head(tree);
    if (!owner || owner != tree) invalid(context);
    if (found.node != head) {
        if (!owner) invalid(context);
        if (found.node == Tree::head(owner)) invalid(context);
        return read(found.node, 0x14);
    }
    const W index = read(registry, 0x10);
    NamePair first{0, nullptr, index};
    if (&first != name) {
        resize_native_string_header_0041dd40(&first, context.strings, read(name), true);
        if (read(name)) std::memcpy(first.data, read<void*>(name, 4), first.length);
    }
    const W first_length = first.length;
    char* const first_data = first.data;
    try {
        NamePair second{0, nullptr, index};
        resize_native_string_header_0041dd40(&second, context.strings, first_length, true);
        char* const second_data = second.data;
        if (first_length) std::memcpy(second_data, first_data, second.length);
        InsertResult result;
        try { insert_name_pair(tree, &result, &second, context); }
        catch (...) { release_captured_name(second_data, second.length, context); throw; }
        release_captured_name(second_data, second.length, context);
    } catch (...) { release_captured_name(first_data, first_length, context); throw; }
    release_captured_name(first_data, first_length, context);
    return index;
}

void register_native_compact_track_names_00b925d0(void* item, void* registry,
    NativeAnimationRegistryContext& context) {
    W index = 0, offset = 0;
    while (static_cast<I>(index) < read<I>(item, 0x10)) {
        register_native_animation_name_00b79740(registry,
            at(read<void*>(item, 0xc), offset + 4u), context);
        ++index; offset += 0x18;
    }
}

const void* __fastcall native_animation_track_name_00b75d20(const void* track) noexcept {
    return at(track, 8);
}
void register_native_track_names_00b8a330(void* item, void* registry,
    NativeAnimationRegistryContext& context) {
    W index = 0;
    while (static_cast<I>(index) < read<I>(item, 0xc)) {
        const void* const track = read<void*>(read<void*>(item, 8), index * 4u);
        register_native_animation_name_00b79740(registry,
            native_animation_track_name_00b75d20(track), context);
        ++index;
    }
}

void reserve_native_float_array_008153e0(void* header, I capacity) {
    const I requested = capacity < 1 ? 1 : capacity;
    if (read<I>(header, 8) >= requested) return;
    void* const data = allocate(static_cast<W>(requested) * 4u);
    W index = 0;
    while (static_cast<I>(index) < read<I>(header, 4)) {
        void* const output = at(data, index * 4u);
        if (output) copy_x87_float(output, at(read<void*>(header), index * 4u));
        ++index;
    }
    singleton_lifetime_free(read<void*>(header));
    write(header, 0, data); write(header, 8, requested);
}
void resize_native_float_array_00818030(void* header, I count) {
    if (count > read<I>(header, 8)) reserve_native_float_array_008153e0(header, count);
    W index = read(header, 4);
    if (static_cast<I>(static_cast<W>(count) - index) >= 4) {
        const I limit = static_cast<I>(static_cast<W>(count) - 3u);
        do {
            for (W lane = 0; lane != 4; ++lane) {
                void* const output = at(read<void*>(header), (index + lane) * 4u);
                if (output) write(output, 0, W{0});
            }
            index += 4u;
        } while (static_cast<I>(index) < limit);
    }
    while (static_cast<I>(index) < count) {
        void* const output = at(read<void*>(header), index * 4u);
        if (output) write(output, 0, W{0});
        ++index;
    }
    while (count < read<I>(header, 4)) write(header, 4, read(header, 4) - 1u);
    write(header, 4, count);
}

void destroy_native_animation_registry_00b79b00(void* registry,
    NativeAnimationRegistryContext& context) {
    write(registry, 0, W{0x00d62ef4});
    resize_native_float_array_00818030(at(registry, 0x14), 0);
    singleton_lifetime_free(read<void*>(registry, 0x14));
    void* const tree = at(registry, 8);
    clear_name_tree(tree, context);
    singleton_lifetime_free(Tree::head(tree));
    write(tree, 4, W{0}); write(tree, 8, W{0});
    destroy_native_ref_counted_base_00bd30f0(registry);
}
void* delete_native_animation_registry_00b79ba0(void* registry, W flags,
    NativeAnimationRegistryContext& context) {
    destroy_native_animation_registry_00b79b00(registry, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(registry);
    return registry;
}
} // namespace bsp
