#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS string tree requires MSVC Win32.
#endif

namespace bsp {
class NativeStringStorage;
struct SingletonLifetimeCallbacks;

// Actual raw Win32 tree: opaque DWORD+0, head+4, unsigned count+8.
// Actual 18h node: left/parent/right +0/+4/+8, string length/data +C/+10,
// color+14 (red0/black1), sentinel+15. Iterator = owner/node DWORDs+0/+4.
// Producers: 4C26B0 allocation and BE1DC0 manager+6C sentinel initialization.
// Names are descriptive hypotheses. All nine bodies are complete source
// reconstructions; explicit providers change the original calling conventions.

// 4B9FD0[28],4BE360[27]: native ECX node, EAX selected node, RET0.
void* maximum_native_vfs_string_node_004b9fd0(void* node) noexcept;
void* minimum_native_vfs_string_node_004be360(void* node) noexcept;

// 4BF130[78],4BF180[82]: native ECX tree, stack node, RET4.
void rotate_left_native_vfs_string_tree_004bf130(void* tree, void* node) noexcept;
void rotate_right_native_vfs_string_tree_004bf180(void* tree, void* node) noexcept;

// 4BE730[99]: native ECX iterator, RET0 or BF6713 tail. Missing owner calls
// the returning validation service, then reloads node. Sentinel validation
// returns immediately when its handler returns; no advancement is added.
void increment_native_vfs_string_iterator_004be730(void* iterator,
    const SingletonLifetimeCallbacks&);

// 4CEC60[82]: native ECX tree, stack node, RET4. Right recursion, capture
// current left before string release, free node, iterate captured left.
// Does not reset the tree. Uses malloc-paired singleton_lifetime_free.
void erase_native_vfs_string_subtree_004cec60(void* tree, void* node,
    NativeStringStorage&) noexcept;

// 4CF8A0[716], through raw tail4CFB32..4CFB6B: native ECX tree, stack
// output/input-owner/input-node, EAX output, RET0C. Complete successor,
// transplant, extrema, color repair, payload/free, current count/output order.
// Nil input throws the established owning source out_of_range transport;
// owner/tree equality is not required. Output may alias raw tree/node storage.
void* erase_native_vfs_string_iterator_004cf8a0(void* tree, void* output,
    void* input_owner, void* input_node, NativeStringStorage&,
    const SingletonLifetimeCallbacks&);

// 4D1A50[201]: native ECX tree, stack output,first-owner/node,last-owner/node,
// EAX output, RET14. Full-range subtree/reset and complete checked partial loop.
void* erase_native_vfs_string_range_004d1a50(void* tree, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringStorage&, const SingletonLifetimeCallbacks&);

// 4D74A0[52], through raw tail4D74C4..4D74D3: native ECX tree, RET0.
// Full current range, free current head, then clear current head/count.
void destroy_native_vfs_string_tree_004d74a0(void* tree, NativeStringStorage&,
    const SingletonLifetimeCallbacks&);

// Production passes ActualNativeStringPoolStorage and the application's shared
// SingletonLifetimeCallbacks. Every string release repeats the real getter and
// return operation. Its existing noexcept release boundary terminates on pool
// recreation failure; native FH3/SEH/catch ABI and exact stack/register identity
// are not reproduced. No private pool, cached publication or replacement map.
} // namespace bsp
