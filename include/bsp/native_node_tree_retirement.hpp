#pragma once
#include "bsp/native_point_light_links.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_texture_surface_getter.hpp"
#include <cstdint>

namespace bsp {
struct NativeNodeTreeRetirementFrame {
    // Actual initialized argument word pushed for B8F4C0, then passed by
    // address to byte-identical B7BED0/B7B620. Private register gaps excluded.
    volatile std::uint32_t node_argument;
};
struct NativeNodeTreeRetirementAcquired {
    bool started{}, complete{};
    std::uint32_t active_call_site{};
    bool hierarchy_unlinked{}, point_lights_cleared{}, release_flag_stored{};
    bool attachment_cleared{}, decrement_completed{}, terminal_completed{};
    std::uint32_t completed_child_calls{};
    // Fresh disjoint diagnostics; no count copy, owner admission or rollback.
    // Throwing required targets retain every preceding raw mutation.
};
struct NativeNodeTreeRetirementInvocation {
    NativeNodeTreeRetirementFrame& frame;
    NativeNodeTreeRetirementAcquired& acquired;
};
class NativeNodeTreeRetirementFrames {
public:
    virtual ~NativeNodeTreeRetirementFrames() = default;
    // Pure metadata acquisition of PREPARED persistent initialized objects.
    // No allocation/native mutation/callback; each active recursive invocation
    // gets distinct live frame/diagnostics. This is not a native call boundary.
    virtual NativeNodeTreeRetirementInvocation recursive_unregister(
        void* actual_child) = 0;
};
class NativeNodeTreeRetirementDispatch {
public:
    virtual ~NativeNodeTreeRetirementDispatch() = default;
    // Pure captured-profile lookup: borrowed actual current table, no native
    // mutation, callback, fallback or invented target.
    virtual const volatile std::uint32_t* resolve_profile(
        std::uint32_t captured_profile) const = 0;
    // Required genuine no-argument current18 call, exact captured target on
    // actual receiver. Binding supplies prepared persistent recursive frames.
    virtual void invoke_virtual18(std::uint32_t captured_target,
        void* actual_receiver) = 0;
    // Required genuine no-argument current0 terminal at observed actual+4 zero.
    // Source verifies canonical_owner borrows that SAME actual count. Provider
    // must verify actual+4 is still zero BEFORE calling the canonical terminal,
    // and honor the exact target/current profile and canonical retirement.
    // Existing camera/light terminals are admitted only in their supported
    // scene-null/cleared-root domain; NEVER use them on raw3Ch-attached nodes.
    // No trace-only/default/no-op terminal; native payload may die here.
    virtual void invoke_virtual0(std::uint32_t captured_target,
        void* actual_receiver, RenderCommandReference& canonical_owner) = 0;
};
struct NativeNodeTreeRetirementContext {
    NativePointLightLinksRuntime& point_lights;
    NativeRenderActualOwners& owners; // SAME actual+4 canonical metadata
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
    NativeNodeTreeRetirementDispatch& dispatch;
    NativeNodeTreeRetirementFrames& frames;
};

// Complete ECX actual-node/no-argument bodies; source ABI differs. Required
// targets/imports are genuine current providers, with valid nonoverflowing
// disjoint array/finite hierarchy domains. There is no native EH or rollback.
// Bindings, payloads and prepared metadata survive reached calls/failure
// disposition. No legacy scene/root host projection or full terminal closure.

// B6DFA0[105]: parent/root unlink, current profile/current18 tail invocation.
void unlink_native_node_tree_00b6dfa0(void* actual_node,
    NativeNodeTreeRetirementContext&, NativeNodeTreeRetirementAcquired&);
// B6F310[176]: actual point-light backlinks/count0, current children/current18,
// late byte44 gate, captured A0 unregister, current decrement/current0 tail.
void release_native_node_tree_00b6f310(void* actual_node,
    NativeNodeTreeRetirementFrame&, NativeNodeTreeRetirementContext&,
    NativeNodeTreeRetirementAcquired&);
// B8F4C0[43], ECX actual owner and stack node word, RET4. Captured node/A0
// match then owner+178 erase via ADDRESS OF ORIGINAL argument, clear nodeA0.
void unregister_native_node_attachment_00b8f4c0(void* actual_owner,
    const volatile std::uint32_t& node_argument);
// B6D850[52]: currentA0 unregister/clear, actual recursive children/fresh next.
void unregister_native_node_tree_00b6d850(void* actual_node,
    NativeNodeTreeRetirementFrame&, NativeNodeTreeRetirementContext&,
    NativeNodeTreeRetirementAcquired&);
} // namespace bsp
