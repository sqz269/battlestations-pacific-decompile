#pragma once

#include "bsp/native_hardware_layout_tree_insert.hpp"

#include <array>
#include <cstdint>

namespace bsp {

// Reuse the established actual8h checked-iterator and12h insertion-result
// representations. Palette NODE layout differs from the hardware-layout map:
// left0,parent4,right8,key+C,Float4+10,color20,sentinel21,padding22..23.
using PanelPaletteIterator = NativeHardwareLayoutTreeIterator;
using PanelPaletteInsertResult = NativeHardwareLayoutTreeInsertResult;
using PanelPaletteValueWords = std::array<std::uint32_t, 4>;
struct PanelPalettePair {
    std::int32_t key;
    PanelPaletteValueWords value_words;
};
static_assert(sizeof(PanelPalettePair) == 0x14);
static_assert(offsetof(PanelPalettePair, value_words) == 4);

// Actual Win32 tree header: opaque allocator/comparator word0, head pointer4,
// unsigned count8. Head stores minimum/root/maximum in its three links.
// Borrow an already initialized, valid tree and live keys/iterators. These
// functions do not construct/destroy a map or replace its native calling ABI.

// ECX=iterator, RET; checked predecessor/successor with current-field reloads
// after returning00BF6713. A tail validation return does not advance further.
void decrement_panel_palette_iterator_00449160(PanelPaletteIterator&,
    const SingletonLifetimeCallbacks&);
void increment_panel_palette_iterator_004491f0(PanelPaletteIterator&,
    const SingletonLifetimeCallbacks&);
// ECX=left iterator, right iterator* stack, RET4, bool AL.
bool equal_panel_palette_iterators_004487d0(const PanelPaletteIterator&,
    const PanelPaletteIterator&, const SingletonLifetimeCallbacks&);
// ECX=tree, node* stack, RET4. No allocation or key/value transformations.
void rotate_panel_palette_right_00448ba0(void* actual_tree, void* node) noexcept;
void rotate_panel_palette_left_00449f00(void* actual_tree, void* node) noexcept;
// Stack left/parent/right/pair*/color, RET14h, EAX=node. Fixed24h allocation;
// integer copies of all five pair words. Final two padding bytes untouched.
void* allocate_panel_palette_node_0044ab90(void* left, void* parent, void* right,
    const PanelPalettePair*, std::uint8_t color);
// ECX=tree, output*/left-byte/parent*/pair*, RET10h; output node then owner.
// Unsigned count>=0CCCCCCBh throws existing D69260 length-error transport.
PanelPaletteIterator* link_panel_palette_node_0044d2c0(void* actual_tree,
    PanelPaletteIterator* output, std::uint8_t insert_left, void* parent,
    const PanelPalettePair*);
// ECX=tree, output*/pair*, RET8. Duplicate preserves existing value/identity.
// Output owner,node,inserted byte; remaining padding untouched.
PanelPaletteInsertResult* insert_panel_palette_pair_0044e090(void* actual_tree,
    PanelPaletteInsertResult* output, const PanelPalettePair*,
    const SingletonLifetimeCallbacks&);
// ECX=tree, output*/hint-owner/hint-node/pair*, RET10h. Full checked hint paths.
PanelPaletteIterator* insert_panel_palette_hint_0044e7b0(void* actual_tree,
    PanelPaletteIterator* output, PanelPaletteIterator hint,
    const PanelPalettePair*, const SingletonLifetimeCallbacks&);

// ECX=tree, signed key* stack, RET4, EAX=actual node+10h Float4 storage.
// Native0044EC3C..64 copies FOUR UNINITIALIZED STACK WORDS on a miss. They are
// explicit defined input here, read only on absence, then copied bit-for-bit.
// No fallback color, zero initialization, float conversion or new host callback
// occurs. Loaded/assigned entries keep identity and never use the miss words.
std::array<float, 4>& panel_palette_value_0044ec00(void* actual_tree,
    const std::int32_t* key, const PanelPaletteValueWords& missing_stack_words,
    const SingletonLifetimeCallbacks&);

} // namespace bsp
