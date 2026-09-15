#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-service focus requires MSVC Win32.
#endif

namespace bsp {
// B72220: ECX actual root, stack actual node, RET4. Next/previous are raw
// node+3C/+40; root head is raw+0C. Relink current neighbours/head with original
// reloads. Leave node links, parent, root identity and references unchanged.
void unlink_native_raw_root_node_00b72220(void* actual_root, void* actual_node) noexcept;

// Exact B6D890 specialization called by B4ECC0 with requested_root=0. If the
// current+A4 is already zero and parent+30 nonzero, return without recursion.
// Otherwise unlink a top-level registered node, clear+A4, recurse live+34
// children and reload each child's+3C after its call. No scene/virtual operation
// is reached with requested_root=0. Nonzero-root registration remains in the
// existing implementation and is not exposed by this raw specialization.
void clear_native_raw_node_root_00b6d890_null(void* actual_node) noexcept;

// B50010: ECX receiver; RET. Store byte+24C=1 only. "Trivial" is not a no-op.
void mark_native_render_batch_dirty_00b50010(void* actual_batch) noexcept;

// B4ECC0: ECX receiver; RET. Capture+3C before setting byte+250=1, then test
// captured root+0C. For each iteration reload current+3C/head and clear that
// node's root via B6D890(node,0); reload root/head afterward. Finish byte+251=1.
// Detaches root registration only; it neither releases nodes nor clears scenes.
void invalidate_native_render_root_chain_00b4ecc0(void* actual_owner) noexcept;

// B0D1E0: ECX render-service owner; RET. If byte+1C4, capture+20 and mark it
// when nonnull. Re-read+1C4 and then+30; tail-route to B4ECC0 when both are set.
void refresh_native_render_service_focus_00b0d1e0(void* actual_service) noexcept;

// These consume native raw pointers, including root+0C and node+A4. They do
// not reinterpret a RenderNodeRootList host view or create companion roots.
// Existing typed-node/root producers need an explicit compatible identity
// bridge before using this raw path. New C++ signatures are not original ABI.
}
