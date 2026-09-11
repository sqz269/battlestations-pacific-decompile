#include "bsp/panel_palette.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Panel palette actual tree operations require MSVC Win32.
#endif

namespace bsp {
namespace {
void* volatile& word(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(storage) + offset);
}
volatile std::uint32_t& u32(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(storage) + offset);
}
std::uint32_t read_u32(const void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(static_cast<const unsigned char*>(storage) + offset);
}
volatile std::uint8_t& byte(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint8_t*>(static_cast<unsigned char*>(storage) + offset);
}
void* volatile& left(void* node) noexcept { return word(node, 0); }
void* volatile& parent(void* node) noexcept { return word(node, 4); }
void* volatile& right(void* node) noexcept { return word(node, 8); }
void* head(void* tree) noexcept { return word(tree, 4); }
volatile std::uint32_t& count(void* tree) noexcept { return u32(tree, 8); }
volatile std::uint8_t& color(void* node) noexcept { return byte(node, 0x20); }
bool sentinel(void* node) noexcept { return byte(node, 0x21) != 0; }
std::int32_t node_key(void* node) noexcept { return static_cast<std::int32_t>(read_u32(node, 0x0c)); }
std::int32_t pair_key(const PanelPalettePair* pair) noexcept { return static_cast<std::int32_t>(read_u32(pair, 0)); }
void invalid(const SingletonLifetimeCallbacks& callbacks) { callbacks.invalid_parameter(callbacks.context); }
void* iterator_owner(const PanelPaletteIterator& value) noexcept {
    return word(const_cast<PanelPaletteIterator*>(&value), 0);
}
void* iterator_node(const PanelPaletteIterator& value) noexcept {
    return word(const_cast<PanelPaletteIterator*>(&value), 4);
}
struct CompletedLengthMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedLengthMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};
[[noreturn]] void throw_palette_length_error() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(message, "map/set<T> too long", 19);
    //0044D311 arms cleanup only after successful assignment. Same D69260
    // vtable/D83F98 throw profile as the existing owning exception transport.
    const CompletedLengthMessage completed{message};
    throw NativeHardwareLayoutTreeLengthError{message};
}
} // namespace

void decrement_panel_palette_iterator_00449160(PanelPaletteIterator& iterator,
    const SingletonLifetimeCallbacks& callbacks)
{
    if (iterator_owner(iterator) == nullptr) invalid(callbacks);
    auto* node = iterator_node(iterator);
    if (sentinel(node)) {
        node = right(node);
        word(&iterator, 4) = node;
        if (sentinel(node)) invalid(callbacks);
        return;
    }
    auto* child = left(node);
    if (!sentinel(child)) {
        auto* next = right(child);
        while (!sentinel(next)) { child = next; next = right(child); }
        word(&iterator, 4) = child;
        return;
    }
    auto* ancestor = parent(node);
    while (!sentinel(ancestor)) {
        if (iterator_node(iterator) != left(ancestor)) break;
        word(&iterator, 4) = ancestor;
        ancestor = parent(ancestor);
    }
    if (sentinel(iterator_node(iterator))) { invalid(callbacks); return; }
    word(&iterator, 4) = ancestor;
}

void increment_panel_palette_iterator_004491f0(PanelPaletteIterator& iterator,
    const SingletonLifetimeCallbacks& callbacks)
{
    if (iterator_owner(iterator) == nullptr) invalid(callbacks);
    auto* node = iterator_node(iterator);
    if (sentinel(node)) { invalid(callbacks); return; }
    auto* child = right(node);
    if (!sentinel(child)) {
        auto* next = left(child);
        while (!sentinel(next)) { child = next; next = left(child); }
        word(&iterator, 4) = child;
        return;
    }
    auto* ancestor = parent(node);
    while (!sentinel(ancestor)) {
        if (iterator_node(iterator) != right(ancestor)) break;
        word(&iterator, 4) = ancestor;
        ancestor = parent(ancestor);
    }
    word(&iterator, 4) = ancestor;
}

bool equal_panel_palette_iterators_004487d0(const PanelPaletteIterator& a,
    const PanelPaletteIterator& b, const SingletonLifetimeCallbacks& callbacks)
{
    auto* owner = iterator_owner(a);
    if (owner == nullptr || owner != iterator_owner(b)) invalid(callbacks);
    return iterator_node(a) == iterator_node(b); // reload both after handler
}

void rotate_panel_palette_right_00448ba0(void* tree, void* node) noexcept {
    auto* replacement = left(node);
    left(node) = right(replacement);
    auto* child = right(replacement); //00448BAC reload after preceding store
    if (!sentinel(child)) parent(child) = node;
    parent(replacement) = parent(node);
    auto* current_head = head(tree);
    if (node == parent(current_head)) parent(current_head) = replacement;
    else {
        auto* ancestor = parent(node);
        if (node == right(ancestor)) right(ancestor) = replacement;
        else left(ancestor) = replacement;
    }
    right(replacement) = node;
    parent(node) = replacement;
}

void rotate_panel_palette_left_00449f00(void* tree, void* node) noexcept {
    auto* replacement = right(node);
    right(node) = left(replacement);
    auto* child = left(replacement); //00449F0D and inlined0044D444 reread
    if (!sentinel(child)) parent(child) = node;
    parent(replacement) = parent(node);
    auto* current_head = head(tree);
    if (node == parent(current_head)) parent(current_head) = replacement;
    else {
        auto* ancestor = parent(node);
        if (node == left(ancestor)) left(ancestor) = replacement;
        else right(ancestor) = replacement;
    }
    left(replacement) = node;
    parent(node) = replacement;
}

void* allocate_panel_palette_node_0044ab90(void* left_node, void* parent_node,
    void* right_node, const PanelPalettePair* pair, std::uint8_t node_color)
{
    auto* node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x24, 0x24});
    if (node != nullptr) {
        left(node) = left_node;
        right(node) = right_node;
        parent(node) = parent_node;
        for (std::size_t offset = 0; offset != 0x14; offset += 4)
            u32(node, 0x0c + offset) = read_u32(pair, offset);
        color(node) = node_color;
        byte(node, 0x21) = 0;
    }
    return node;
}

PanelPaletteIterator* link_panel_palette_node_0044d2c0(void* tree,
    PanelPaletteIterator* output, std::uint8_t insert_left, void* parent_node,
    const PanelPalettePair* pair)
{
    if (count(tree) >= 0x0ccccccbu) throw_palette_length_error();
    auto* allocation_head = head(tree);
    auto* node = allocate_panel_palette_node_0044ab90(allocation_head, parent_node,
        allocation_head, pair, 0);
    auto* current_head = head(tree); //0044D34B, before increment
    count(tree) = count(tree) + 1u;
    if (parent_node == current_head) {
        parent(current_head) = node;
        left(head(tree)) = node;
        right(head(tree)) = node;
    } else if (insert_left != 0) {
        left(parent_node) = node;
        auto* current = head(tree);
        if (parent_node == left(current)) left(current) = node;
    } else {
        right(parent_node) = node;
        auto* current = head(tree);
        if (parent_node == right(current)) right(current) = node;
    }
    auto* repair = node;
    while (color(parent(repair)) == 0) {
        auto* direct_parent = parent(repair);
        auto* grandparent = parent(direct_parent);
        if (direct_parent == left(grandparent)) {
            auto* uncle = right(grandparent);
            if (color(uncle) == 0) {
                color(direct_parent) = 1;
                color(uncle) = 1;
                color(parent(parent(repair))) = 0;
                repair = parent(parent(repair));
            } else {
                if (repair == right(direct_parent)) {
                    repair = direct_parent;
                    rotate_panel_palette_left_00449f00(tree, repair);
                }
                color(parent(repair)) = 1;
                color(parent(parent(repair))) = 0;
                rotate_panel_palette_right_00448ba0(tree, parent(parent(repair)));
            }
        } else {
            auto* uncle = left(grandparent);
            if (color(uncle) == 0) {
                color(direct_parent) = 1;
                color(uncle) = 1;
                color(parent(parent(repair))) = 0;
                repair = parent(parent(repair));
            } else {
                if (repair == left(direct_parent)) {
                    repair = direct_parent;
                    rotate_panel_palette_right_00448ba0(tree, repair);
                }
                color(parent(repair)) = 1;
                color(parent(parent(repair))) = 0;
                //0044D436..472 has the same store/read order as00449F00.
                rotate_panel_palette_left_00449f00(tree, parent(parent(repair)));
            }
        }
    }
    color(parent(head(tree))) = 1;
    word(output, 4) = node;
    word(output, 0) = tree;
    return output;
}

PanelPaletteInsertResult* insert_panel_palette_pair_0044e090(void* tree,
    PanelPaletteInsertResult* output, const PanelPalettePair* pair,
    const SingletonLifetimeCallbacks& callbacks)
{
    auto* parent_node = head(tree);
    auto* node = parent(parent_node);
    std::uint8_t insert_left = 1;
    if (!sentinel(node)) {
        const auto key = pair_key(pair); // cached across callback-free descent
        do {
            parent_node = node;
            insert_left = key < node_key(node);
            node = insert_left ? left(node) : right(node);
        } while (!sentinel(node));
    }
    PanelPaletteIterator selected{tree, parent_node};
    bool insert = false;
    if (insert_left) {
        if (parent_node == left(head(tree))) insert = true;
        else decrement_panel_palette_iterator_00449160(selected, callbacks);
    }
    if (!insert) insert = node_key(iterator_node(selected)) < pair_key(pair);
    if (insert) link_panel_palette_node_0044d2c0(tree, &selected, insert_left, parent_node, pair);
    //0044E100/102/105 and0044E139/13B/13E: owner then node then byte.
    auto* owner = iterator_owner(selected);
    auto* selected_node = iterator_node(selected);
    word(output, 0) = owner;
    word(output, 4) = selected_node;
    byte(output, 8) = insert ? 1 : 0;
    return output;
}

PanelPaletteIterator* insert_panel_palette_hint_0044e7b0(void* tree,
    PanelPaletteIterator* output, PanelPaletteIterator hint,
    const PanelPalettePair* pair, const SingletonLifetimeCallbacks& callbacks)
{
    if (count(tree) == 0) return link_panel_palette_node_0044d2c0(tree, output, 1, head(tree), pair);
    auto* minimum = left(head(tree)); // captured before checked-owner callback
    if (hint.owner == nullptr || hint.owner != tree) invalid(callbacks);
    if (hint.node == minimum) {
        if (pair_key(pair) < node_key(hint.node))
            return link_panel_palette_node_0044d2c0(tree, output, 1, hint.node, pair);
    } else {
        auto* current_head = head(tree);
        if (hint.owner == nullptr || hint.owner != tree) invalid(callbacks);
        if (hint.node == current_head) {
            auto* maximum = right(head(tree));
            if (node_key(maximum) < pair_key(pair))
                return link_panel_palette_node_0044d2c0(tree, output, 0, maximum, pair);
        } else {
            auto key = pair_key(pair);
            if (key < node_key(hint.node)) {
                auto before = hint;
                decrement_panel_palette_iterator_00449160(before, callbacks);
                key = pair_key(pair);
                if (node_key(before.node) < key) {
                    if (sentinel(right(before.node)))
                        return link_panel_palette_node_0044d2c0(tree, output, 0, before.node, pair);
                    return link_panel_palette_node_0044d2c0(tree, output, 1, hint.node, pair);
                }
            }
            if (node_key(hint.node) < key) {
                PanelPaletteIterator end{tree, head(tree)};
                auto after = hint;
                increment_panel_palette_iterator_004491f0(after, callbacks);
                if (equal_panel_palette_iterators_004487d0(after, end, callbacks)
                    || pair_key(pair) < node_key(after.node)) {
                    if (sentinel(right(hint.node)))
                        return link_panel_palette_node_0044d2c0(tree, output, 0, hint.node, pair);
                    return link_panel_palette_node_0044d2c0(tree, output, 1, after.node, pair);
                }
            }
        }
    }
    PanelPaletteInsertResult result;
    insert_panel_palette_pair_0044e090(tree, &result, pair, callbacks);
    auto* owner = iterator_owner(result.iterator);
    word(output, 0) = owner;
    word(output, 4) = iterator_node(result.iterator);
    return output;
}

std::array<float, 4>& panel_palette_value_0044ec00(void* tree,
    const std::int32_t* requested_key, const PanelPaletteValueWords& missing_words,
    const SingletonLifetimeCallbacks& callbacks)
{
    auto* selected = head(tree);
    auto* node = parent(selected);
    if (!sentinel(node)) {
        const auto key = *requested_key;
        do {
            if (node_key(node) < key) node = right(node);
            else { selected = node; node = left(node); }
        } while (!sentinel(node));
    }
    auto* owner = tree;
    if (selected == head(tree) || *requested_key < node_key(selected)) {
        // All scratch bits become DEFINED inputs; source-order copies mirror
        //0044EC3C..64. No C++ uninitialized float or guessed default is used.
        PanelPalettePair pair;
        pair.value_words[0] = missing_words[0];
        pair.key = *requested_key;
        pair.value_words[2] = missing_words[2];
        pair.value_words[1] = missing_words[1];
        pair.value_words[3] = missing_words[3];
        PanelPaletteIterator output;
        insert_panel_palette_hint_0044e7b0(tree, &output, {tree, selected}, &pair, callbacks);
        owner = iterator_owner(output);
        selected = iterator_node(output);
    }
    if (owner == nullptr) invalid(callbacks);
    if (selected == head(owner)) invalid(callbacks);
    return *reinterpret_cast<std::array<float, 4>*>(static_cast<unsigned char*>(selected) + 0x10);
}

} // namespace bsp
