#pragma once
#include "bsp/native_node_base_destruction.hpp"
#include <cstdint>

namespace bsp {
class NativeNodeParentingDispatch {
public:
    virtual ~NativeNodeParentingDispatch() = default;
    // Pure lookup of the captured numeric profile into its borrowed ACTUAL
    // current table. No native mutation, callback, unknown-profile fallback.
    virtual const volatile std::uint32_t* resolve_profile(std::uint32_t) const = 0;
    // Genuine exact current0C target; only returned AL participates. Known
    // bootstrap predicates require initialized CURRENT token/lineage storage.
    virtual std::uint8_t invoke_type_0c(std::uint32_t target, void* actual,
        std::uint32_t captured_group_token) = 0;
    // Genuine exact current1C on self or child. Binding retains prepared,
    // disjoint recursive frames/acquisitions; no logical owner conversion.
    virtual void invoke_attachment_1c(std::uint32_t target, void* actual,
        std::uint32_t captured_attachment) = 0;
};
struct NativeNodeParentingContext {
    NativeNodeBaseDestructionContext& node;
    NativeNodeParentingDispatch& dispatch;
    const volatile std::uint32_t& group_type_0109032c;
};
struct NativeNodePrependChildFrame {
    volatile std::uint32_t child_argument;
};
struct NativeNodeParentingAcquired {
    bool started{}, complete{};
    std::uint32_t active_call_site{};
};
struct NativeNodeParentingFrame {
    volatile std::uint32_t parent_argument;
    volatile std::uint32_t unregister_node_argument;
    NativeNodePrependChildFrame prepend;
    NativeNodeTreeRetirementFrame recursive_unregister;
};
struct NativeNodeSetParentAcquired : NativeNodeParentingAcquired {
    NativeNodeParentingAcquired prepend;
    NativeNodeTreeRetirementAcquired recursive_unregister;
};
struct NativeNodeAttachmentFrame {
    volatile std::uint32_t attachment_argument;
    volatile std::uint32_t unregister_node_argument;
    volatile std::uint32_t registration_node_argument;
    volatile std::uint32_t group_key_local;
    // B8F4F0 alone seeds group_key_local with actual self (PUSH ECX), then
    // writes self again on the old-attachment path. B6D7B0 leaves it untouched.
};

// Actual node prefix through174; group descriptors at178 are genuine LIVE
// SystemAmbientBacklinks12B objects over native pointer/count/capacity bytes.
// Establish that descriptor lifetime with its complete preimage preserved,
// independently of any logical group owner. Keys are opaque actual pointers:
// no SceneResource dereference, metadata pointer, count credit or admission.
// Valid arrays have nonnegative count<=capacity, accessible disjoint backing,
// representable allocation size and real singleton allocation/free provenance.
// Native overflow/null computed-copy destinations lie outside that domain.
// Context scenes/trees share SAME actual owner registry/current decrement cell.
// Frames are initialized, address-stable caller preimages, live through every
// callback; only native argument/local stores are performed. Acquisitions are
// fresh persistent diagnostics, not rollback. Failures retain prior hierarchy,
// root, scene, array and flag effects and nested callee diagnostics. Exact
// reached virtual targets require genuine bindings; no default callbacks.
// No private native stack/FH3/SEH or binary/game/application compatibility.

// 59E5E0[95], ECX descriptor/stack minimum, RET4. Normalized instruction clone
// of B7B390 (only two rel32 displacements differ, same allocator/free targets).
void reserve_native_group_pointer_array_0059e5e0(void* actual_array,
    std::int32_t captured_minimum);
// B8F460[87], ECX group/stack child, RET4. Capture child once, equality guard,
// current count/capacity reserve, CURRENT begin/count/store/count++/childA0.
void register_native_group_node_00b8f460(void* actual_group,
    const volatile std::uint32_t& child_argument);
// B6D7B0[71], ECX node/stack attachment, RET4. Incoming argument is captured
// only AFTER old unregister. Recurse through current1C/current successor.
void set_native_node_attachment_00b6d7b0(void* actual_node,
    NativeNodeAttachmentFrame&, NativeNodeParentingDispatch&,
    NativeNodeParentingAcquired&);
// B8F4F0[73], same ABI, group override. Actual-self local key/first-equal erase;
// no child recursion. Its real registration equality guard remains intact.
void set_native_group_attachment_00b8f4f0(void* actual_group,
    NativeNodeAttachmentFrame&, NativeNodeParentingAcquired&);
// B6E010[134], ECX parent/stack child, RET4. Publish30 BEFORE root propagation,
// optional current50 then current head/count/flags and exact current40.
void prepend_native_raw_child_00b6e010(void* actual_parent,
    NativeNodePrependChildFrame&, NativeNodeParentingContext&,
    NativeNodeParentingAcquired&);
// Complete B6E680[204], ECX node/stack parent, RET4. Old parent read precedes
// capture of request; captured parent profile precedes type getter, current0C
// follows it. Captured attachment survives prepend's callbacks; both reached
// world40 calls execute. Does not replace the existing null-only projection.
void set_native_raw_parent_00b6e680(void* actual_node,
    NativeNodeParentingFrame&, NativeNodeParentingContext&,
    NativeNodeSetParentAcquired&);
} // namespace bsp
