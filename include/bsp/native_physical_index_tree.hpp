#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical index trees require MSVC Win32.
#endif

namespace bsp {
class NativeStringStorage;
struct SingletonLifetimeCallbacks;

// Complete raw bodies, not typed-container projections. Tree head/count are
// DWORDs +4/+8; links +0/+4/+8, string pair +C, color/nil bytes +1C/+1D.
// Native ECX node, RET0, EAX selected node.
void* maximum_native_physical_index_node_00bd9380(void*) noexcept;
void* minimum_native_physical_index_node_00bd93a0(void*) noexcept;
// Native ECX tree, stack node, RET4. No semantic return value.
void rotate_native_physical_index_right_00bd93e0(void* tree, void* node) noexcept;
void rotate_native_physical_index_left_00bda090(void* tree, void* node) noexcept;

// 489D50[118]: native ECX pair, RET0. Release current second string then
// current first string, retaining headers. ActualNativeStringPoolStorage is
// the production binding: each nonnull return repeats the actual pool getter.
// Its inherited noexcept release means throwing-getter FH3 is not reproduced.
void destroy_native_physical_index_pair_00489d50(void* pair,
    NativeStringStorage&) noexcept;
// BDF7A0[61]: native ECX tree, stack root, RET4. Right recursion, capture left,
// pair destruction/free, then iterate captured left; no link/count clearing.
void destroy_native_physical_index_subtree_00bdf7a0(void* tree, void* root,
    NativeStringStorage&) noexcept;

// BDFD80[691], including tail through BE0032. Native ECX tree, stack
// output/owner/node, RET0C, EAX output. Input owner need not equal tree.
// Sentinel throws owning NativeHardwareLayoutInvalidIterator source transport
// after counted-message construction; original D863A8 RTTI/FH3 is not retained.
// Complete successor transplant, both balancing branches, current count read
// AFTER pair destruction/free, then owner-before-node result publication.
void* erase_native_physical_index_iterator_00bdfd80(void* tree, void* output,
    void* owner, void* node, NativeStringStorage&, const SingletonLifetimeCallbacks&);
// BE0C30[201]: native ECX tree, five stack words, RET14, EAX output.
// Complete full-range AND partial-range paths, preserving checked-owner calls
// and the first iterator's advance before erasing its captured old node.
void* erase_native_physical_index_range_00be0c30(void* tree, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringStorage&, const SingletonLifetimeCallbacks&);
// BE1700[52]: native ECX tree, RET0. Erase full range, free current head, then
// zero head/count. Tree+0 remains untouched. No null or empty-head guards.
void destroy_native_physical_index_00be1700(void* tree, NativeStringStorage&,
    const SingletonLifetimeCallbacks&);

// Explicit storage/CRT arguments make new source interfaces, not native ABI
// replacements. Coverage, native evidence and EH limits are in
// docs/NATIVE_PHYSICAL_INDEX_TREE.md. No game validation is claimed.
} // namespace bsp
