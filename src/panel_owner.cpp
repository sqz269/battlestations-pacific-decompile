#include "bsp/panel_owner.hpp"

#include "bsp/panel_sequence.hpp"

#include <cstring>
#include <iterator>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Panel owner actual palette storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(PanelPaletteTreeStorage) == 0x0c);
static_assert(offsetof(PanelPaletteTreeStorage, head_04) == 4);
static_assert(offsetof(PanelPaletteTreeStorage, count_08) == 8);

void* volatile& word(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(storage) + offset);
}
volatile std::uint32_t& u32(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(storage) + offset);
}
volatile std::uint8_t& byte(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint8_t*>(static_cast<unsigned char*>(storage) + offset);
}
void* head(void* tree) noexcept { return word(tree, 4); }

// Native004512E0 recursively visits right, destroys the current entry before
// its key, frees the node, and walks its captured left subtree. This is reverse
// key order. Keep the CURRENT pair linked through both ownership callbacks;
// extraction before destruction would give the per-entry00451020 behavior.
void destroy_queue_projection(PanelSequenceQueue& queue, NativeStringStorage& strings) {
    while (!queue.empty()) {
        const auto current = std::prev(queue.end());
        destroy_panel_pair_00450110(const_cast<NativeString&>(current->first),
            current->second, strings);
        queue.erase(current); // library node release, no comparator/key read
    }
}
// Same right/current/left traversal of0044E4F0, with POD mapped values.
void destroy_character_projection(PanelCharacterMap& characters, NativeStringStorage& strings) {
    while (!characters.empty()) {
        const auto current = std::prev(characters.end());
        destroy_native_string_header_0041dd20(
            const_cast<NativeString*>(&current->first), strings);
        characters.erase(current);
    }
}
} // namespace

void prepare_panel_owner_allocation(VoicePanelState& owner,
    const PanelOwnerAllocationWords& words) noexcept
{
    owner.owner_native.character_allocator_04 = words.character_allocator_04;
    owner.owner_native.palette_10.allocator_00 = words.palette_allocator_10;
    owner.owner_native.queue_allocator_1c = words.queue_allocator_1c;
    std::memcpy(&owner.default_pause_30, &words.pause_bits_30, sizeof(float));
    owner.field_34 = words.state_34;
    owner.palette_10 = &owner.owner_native.palette_10;
}

void* allocate_panel_palette_sentinel_0044ab50()
{
    void* node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x24, 0x24});
    //00BF681B returns a real allocation or throws; the native's tests around
    // link construction do not provide a viable null-allocation return path.
    word(node, 0) = nullptr;
    word(node, 4) = nullptr;
    word(node, 8) = nullptr;
    byte(node, 0x20) = 1;
    byte(node, 0x21) = 0;
    return node;
}

void destroy_panel_palette_subtree_0044ac90(void* tree, void* node) noexcept
{
    while (byte(node, 0x21) == 0) {
        destroy_panel_palette_subtree_0044ac90(tree, word(node, 8));
        void* captured_left = word(node, 0); // after right subtree destruction
        singleton_lifetime_free(node);
        node = captured_left;
    }
}

void clear_panel_queue_full_range_004517e0(VoicePanelState& owner,
    NativeStringStorage& strings)
{
    destroy_queue_projection(owner.queued_1c, strings);
    //00451838 only after ALL entry callbacks, pooled releases and node frees.
    owner.field_24 = 0;
}

void clear_panel_palette_full_range_0044e150(void* tree) noexcept
{
    destroy_panel_palette_subtree_0044ac90(tree, word(head(tree), 4));
    auto* current_head = head(tree);
    word(current_head, 4) = current_head;
    current_head = head(tree);
    u32(tree, 8) = 0;
    word(current_head, 0) = current_head;
    current_head = head(tree);
    word(current_head, 8) = current_head;
}

void clear_panel_characters_full_range_0044f0a0(PanelCharacterMap& characters,
    NativeStringStorage& strings)
{
    destroy_character_projection(characters, strings);
}

VoicePanelState& construct_panel_owner_00452660(VoicePanelState& owner)
{
    owner.owner_native.vtable_00 = 0x00ce4bc0;
    // The standard character map and queue own their existing empty sentinel
    // allocations. Their native0044AB00/0044AC00 layout is not duplicated.
    auto& tree = owner.owner_native.palette_10;
    owner.palette_10 = &tree;
    tree.head_04 = allocate_panel_palette_sentinel_0044ab50();
    byte(tree.head_04, 0x21) = 1;
    auto* current_head = tree.head_04;
    word(current_head, 4) = current_head;
    current_head = tree.head_04;
    word(current_head, 0) = current_head;
    current_head = tree.head_04;
    word(current_head, 8) = current_head;
    tree.count_08 = 0;
    owner.field_24 = 0;
    //00452703/06 overwrite both header words, without destroying prior data.
    u32(&owner.current_28, 0) = 0;
    word(&owner.current_28, 4) = nullptr;
    return owner;
}

void destroy_panel_owner_00452040(VoicePanelState& owner,
    NativeStringStorage& strings)
{
    owner.owner_native.vtable_00 = 0x00ce4bc0;
    destroy_native_string_header_0041dd20(&owner.current_28, strings);
    clear_panel_queue_full_range_004517e0(owner, strings);
    auto& tree = owner.owner_native.palette_10;
    clear_panel_palette_full_range_0044e150(&tree);
    singleton_lifetime_free(tree.head_04);
    tree.head_04 = nullptr;
    tree.count_08 = 0;
    clear_panel_characters_full_range_0044f0a0(owner.characters_04, strings);
}

VoicePanelState* scalar_delete_panel_owner_00452720(VoicePanelState* owner,
    std::uint32_t flags, NativeStringStorage& strings)
{
    destroy_panel_owner_00452040(*owner, strings);
    if ((flags & 1u) != 0) {
        // Empty standard containers still own host sentinels. End their C++
        // lifetime before freeing the host-sized allocation (native38h).
        owner->~VoicePanelState();
        singleton_lifetime_free(owner);
    }
    return owner;
}

} // namespace bsp
