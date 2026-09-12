#pragma once

namespace bsp {
class NativeStringStorage;
struct SingletonLifetimeCallbacks;

// Actual 1Ch FileStore pending nodes: links +0/+4/+8, string +0C/+10,
// unowned payload +14, color +18, nil +19. Producer BE5690/BE63E0.
// These complete source interfaces add explicit services; original FH3/SEH,
// original CRT exception identity, binary replacement and gameplay are unproved.
// Native ECX node, EAX selected node, RET.
void* maximum_native_file_store_pending_node_00be4a70(void* node) noexcept;
void* minimum_native_file_store_pending_node_00be4a90(void* node) noexcept;
// Native ECX tree (head+4/count+8), stack node, RET4.
void rotate_native_file_store_pending_right_00be4ad0(void* tree, void* node) noexcept;
void rotate_native_file_store_pending_left_00be5140(void* tree, void* node) noexcept;

// Complete BE6A20..BE6CEB, including reachable successor transplantation and
// post-free continuation. Native ECX tree; stack output, owner, node; RET0Ch.
// Return output containing the advanced by-value iterator. Sentinel throws the
// existing owning NativeHardwareLayoutInvalidIterator source transport.
// Erase frees only the original node/key; it leaves payload+14 unowned.
void* erase_native_file_store_pending_iterator_00be6a20(void* tree, void* output,
    void* iterator_owner, void* iterator_node, NativeStringStorage&,
    const SingletonLifetimeCallbacks&);

// Complete BE7580..BE7648: fast full range and checked partial loop. Native
// ECX tree; output, first owner/node, last owner/node on stack; RET14h.
void* erase_native_file_store_pending_range_00be7580(void* tree, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringStorage&, const SingletonLifetimeCallbacks&);

// Complete BE7650..BE7683 including raw tail: erase full range, free CURRENT
// sentinel, zero head/count. Native ECX tree; RET; no semantic return.
void destroy_native_file_store_pending_tree_00be7650(void* tree,
    NativeStringStorage&, const SingletonLifetimeCallbacks&);

// Complete duplicate destruction entry BE7A70..BE7AA3. Provider state2's
// CC6E09 tail transfer supplies provider+20. Same full-range/free/reset
// schedule as BE7650; native ECX tree, RET. Source interface only.
void destroy_native_file_store_pending_tree_unwind_00be7a70(void* tree,
    NativeStringStorage&, const SingletonLifetimeCallbacks&);
} // namespace bsp

