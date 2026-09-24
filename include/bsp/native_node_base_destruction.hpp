#pragma once
#include "bsp/native_node_scene_attachment.hpp"
#include "bsp/native_node_tree_retirement.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include <cstdint>

namespace bsp {
class NativeNodeBaseWorldDispatch {
public:
    virtual ~NativeNodeBaseWorldDispatch() = default;
    // Pure metadata lookup of a captured numeric profile into its borrowed
    // actual current table. No native mutation, callback or fallback.
    virtual const volatile std::uint32_t* resolve_profile(std::uint32_t) const = 0;
    // Genuine exact reached no-argument target on actual receiver. Concrete
    // bindings may use B6DBE0/B6DBC0 in their observed profile domains; they
    // must validate current targets, including callback-mutated profiles.
    virtual void invoke_virtual40(std::uint32_t captured_target, void* actual) = 0;
    virtual void invoke_virtual3c(std::uint32_t captured_target, void* actual) = 0;
};
struct NativeNodeBaseDestructionContext {
    NativeNodeSceneAttachmentContext& scenes;
    NativeNodeTreeRetirementContext& trees;
    NativeStringRawPoolContext& strings;
    NativeNodeBaseWorldDispatch& world;
    // scenes and trees borrow SAME actual owners/current import domain.
    // Retained130 zero uses trees' canonical/current0 binding. That provider
    // must explicitly admit the retained owner's FAMILY/EXTENT, verify its
    // SAME actual+4 observed zero/current profile and retire it genuinely.
    // Do not apply node scene170/root checks to a non-node retained owner.
    // Array backing must belong to trees.point_lights' real allocation domain.
};
struct NativeNodeParentNullFrame {
    volatile std::uint32_t attachment_node_argument;
    NativeNodeTreeRetirementFrame recursive_unregister;
};
struct NativeNodeParentNullAcquired {
    bool started{}, complete{};
    std::uint32_t active_call_site{};
    NativeNodeTreeRetirementAcquired recursive_unregister;
};
struct NativeNodeBaseDestructionFrame {
    volatile std::uint32_t attachment_node_argument;
    NativeNodeParentNullFrame parent_null;
    NativeNodeSceneAttachmentFrame scene_remove;
};
struct NativeNodeBaseDestructionAcquired {
    bool started{}, complete{}, exception_cleanup_started{};
    std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{};
    void* captured_retained_owner{};
    bool retained_decrement_completed{}, retained_terminal_completed{};
    bool array_cleanup_completed{}, name_cleanup_completed{}, base_cleanup_completed{};
    NativeNodeParentNullAcquired parent_null;
    NativeNodeSceneAttachmentAcquired scene_remove;
    // Fresh disjoint persistent diagnostics, never native ownership/rollback.
    // Keep nested acquisitions available after failure; callbacks can mutate
    // storage before throwing even when the completed flag remains false.
};

// B6D940[64], ECX parent/stack captured child, RET4. Complete raw unlink leaf.
void unlink_native_raw_child_00b6d940(void* actual_parent, void* captured_child) noexcept;
// B6DBE0[25], ECX node, no args. Clear138 bits then CURRENT A0/current3C tail.
void notify_native_raw_world_00b6dbe0(void* actual_node, NativeNodeBaseWorldDispatch&);
// B6E680 REQUESTED-NULL PATH ONLY. The nonnull-request regions
// [B6E6B8,B6E6C0) and [B6E6C5,B6E6F8) are excluded; this is not a full
// reparent implementation (59 of its204 native bytes remain outside scope).
// Existing current parent==null returns immediately. Otherwise raw unlink,
// currentA0 unregister, captured root propagation, recursive unregister,
// current flags/descendants and exact current40 dispatch. Prepared frame only.
void set_native_raw_parent_null_00b6e680(void* actual_node,
    NativeNodeParentNullFrame&, NativeNodeBaseDestructionContext&,
    NativeNodeParentNullAcquired&);
// B6F3E0[23], ECX actual12B point-light descriptor, no args. Resize0 then
// capture CURRENT begin/free in SAME owned allocation domain; leave stale
// begin/capacity unchanged. No light/object destruction or metadata adoption.
void destroy_native_raw_point_light_array_00b6f3e0(void* actual_array,
    NativePointLightLinksRuntime&) noexcept;
// Complete B6F440[297], ECX node/no args, RET. Real raw normal body and
// explicit C++ projection of observed states2->1->0->-1; each normal/unwind
// cleanup is consumed BEFORE its call. No original FH3/SEH/private-stack ABI.
// Raw source performs native stamps only, never installs legacy companion
// callbacks or returns the concrete allocation. Derived terminal graph remains
// separate. Context, initialized frames and Acquired survive all callbacks and
// explicit failure disposition; no rollback of scene130/hierarchy prefixes.
void destroy_native_raw_node_base_00b6f440(void* actual_node,
    NativeNodeBaseDestructionFrame&, NativeNodeBaseDestructionContext&,
    NativeNodeBaseDestructionAcquired&);
} // namespace bsp
