#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native platform focus owners require MSVC Win32.
#endif

namespace bsp {
class NativeRenderActualOwners;

// Stable bindings to the application's actual cells and canonical references.
// GUI storage is88h, media storage18h. No semantic GUI/media owner is allocated.
struct NativePlatformFocusOwnersContext {
    void* volatile& actual_manager_01090aa0;
    void* volatile& actual_gui_00f8bc5c;
    void* volatile& actual_media_00f8aef8;
    const volatile std::uint32_t& actual_half_00ce3800;
    const volatile std::uint32_t& actual_one_00d7a24c;
    NativeRenderActualOwners& actual_resources;
};

// Complete allocation leaves: native no consumed arguments, EAX allocation,
// RET. Self-link list next/previous only; tree links=0, byte14=1/15=0. Preserve
// untouched payload and original pointer tests/wrapping, including null faults.
void* allocate_native_gui_list_sentinel_00aa2820(); //14h;26 native bytes
void* allocate_native_gui_tree_node_00aa2920();     //18h;55 native bytes
void* allocate_native_media_list_sentinel_00a4c4a0(); //0Ch;26 native bytes

// AA0FF0/A4C3F0: native ECX owner, RET. Clear CURRENT publication first, then
// reset ORIGINAL owner toCE3818. No unregister or owner-free operation.
void destroy_native_gui_base_00aa0ff0(void* actual_owner,
    NativePlatformFocusOwnersContext& context) noexcept;
void destroy_native_media_base_00a4c3f0(void* actual_owner,
    NativePlatformFocusOwnersContext& context) noexcept;

// 41DA80: native ECX raw pointer slot, RET. Capture, decrement actual+4, dispatch
// current virtual0 through canonical ownership only on zero; then clear slot.
// A null initial pointer leaves the slot untouched. The canonical terminal
// callback is nonthrowing; missing identities/profiles are never a no-op.
void release_native_gui_resource_slot_0041da80(void* actual_slot,
    NativeRenderActualOwners& actual_resources);

// 4C8020: native ECX raw10h vector header, RET. Free captured +4 when nonnull,
// then zero current +4/+8/+C in order; preserve header+0.
void destroy_native_gui_page_vector_004c8020(void* actual_header) noexcept;

// AA22C0: native ECX tree (forwarded), stack node, RET4. Recursively erase right
// subtree, capture left, free old node, then iterate left; byte15 ends traversal.
void erase_native_gui_tree_subtree_00aa22c0(void* actual_tree, void* actual_node);

// AA5260: native ECX tree header (unused00/head04/count08), RET. Its fixed
// AA49E0 arguments select the whole-range branch for valid nonconcurrent
// storage. Erase root, reset sentinel links/count in original order, free current
// head, clear head/count. This specializes that fixed call; arbitrary AA49E0
// iterator inputs and its single-node erase branch are not exposed or claimed.
void destroy_native_gui_tree_00aa5260(void* actual_tree);

// Complete AA5D70[169] and A4C5A0[96], native ECX owner, EAX original owner,
// RET. Preserve all unassigned bytes. GUI state3 unwind releases+2C, destroys
// vector+14 and tree+8, then clears publication viaAA0FF0. Media's sole cleanup
// clears publication viaA4C3F0. The literal loads precede subsequent stores.
void* construct_native_gui_manager_00aa5d70(void* actual_88h_owner,
    NativePlatformFocusOwnersContext& context);
void* construct_native_media_manager_00a4c5a0(void* actual_18h_owner,
    NativePlatformFocusOwnersContext& context);

// Complete4C12B0[192]/4C1710[189], native no consumed args, EAX publication,
// RET. Fast return uses first captured value; slow path rechecks under first
// manager's section, constructs/registers, and rereads publication after Leave.
// Allocation cleanup is disarmed BEFORE publication/registration. Constructor
// failure frees captured owner after constructor cleanup, then unwinds guard.
void* get_native_gui_manager_004c12b0(NativePlatformFocusOwnersContext& context);
void* get_native_media_manager_004c1710(NativePlatformFocusOwnersContext& context);

// Complete native RET4[3]; no ECX read and no side effects. Its caller pushes
// this argument BEFORE the no-argument media getter, so that getter still runs.
void __stdcall ignore_native_media_pause_00a4c2d0(std::uint32_t pause);

// Source context arguments are new interfaces, not original FH3/SEH/ABI.
// Numeric profiles retain identity, not callable host vtables. C++ cleanup
// follows reviewed states; a second escaping cleanup exception terminates.
// Hardware faults, invalid storage and concurrent mutation are unproved. These
// are constructor/getter operations and their cleanup dependencies, not complete
// populated GUI/media destruction, page activation or application shutdown.
} // namespace bsp
