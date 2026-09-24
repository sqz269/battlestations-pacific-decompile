#pragma once
#include "bsp/native_scene_registration_gates.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_texture_surface_getter.hpp"
#include <cstdint>

namespace bsp {
class NativeNodeSceneChildDispatch {
public:
    virtual ~NativeNodeSceneChildDispatch() = default;
    // Pure metadata lookup for the captured numeric profile. Return its
    // borrowed ACTUAL current table; no callback/native mutation or fallback.
    virtual const volatile std::uint32_t* resolve_profile(
        std::uint32_t captured_profile) const = 0;
    // Required genuine reached target on actual node storage (child or self).
    // Caller just loaded its current50/54. Binding supplies initialized persistent
    // recursive frames/acquisitions and preserves the exact captured target,
    // requested identity and supplied recursion word (0 or 1). These four
    // methods dispatch children with 1; root propagation can dispatch self
    // with 0. No logical scene/no-op substitution.
    virtual void invoke_child(std::uint32_t captured_target, void* actual_child,
        std::uint32_t captured_scene, std::uint32_t recursion) = 0;
};
struct NativeNodeSceneAttachmentContext {
    NativeSceneRegistrationGateContext& gates;
    NativeRenderActualOwnerRegistry& owners;
    NativeTextureSurfaceReferenceIncrement const volatile& increment_00ce221c;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
    NativeNodeSceneChildDispatch& children;
};
struct NativeNodeSceneAttachmentFrame {
    volatile std::uint32_t scene_argument;
    volatile std::uint32_t recursion_argument;
    NativeSceneRegistrationGateInsertFrame insertion;
    NativeSceneRegistrationGateEraseFrame erasure;
};
struct NativeNodeSceneAttachmentAcquired {
    bool started{}, complete{};
    std::uint32_t active_call_site{};
    void* captured_scene{};
    void* released_scene{};
    bool scene_slot_published{}, light_slot_appended{};
    bool unregistration_completed{}, registration_completed{};
    bool increment_completed{}, decrement_completed{}, terminal_completed{};
    NativeSceneRegistryRegistrationAcquired registration;
    // Fresh, disjoint diagnostics per invocation. Flags record completed
    // calls/stores, not rollback or credits implied by a throwing callback.
    // Keep any allocated registry node and preceding native mutations for
    // explicit disposition. No destructor, admission or count copy is added.
};

// All methods consume actual node/light storage and actual3Ch scene identities,
// never host SceneResource/SceneNodeAttachment pointers. node+170 and light's
// actual12B pointer/count/capacity descriptor+178 are separate domains.
// Current imports and SAME canonical actual+4 owner registry are required;
// zero terminal validates the companion borrows that exact count before its
// genuine CURRENT0 dispatch. The canonical terminal is noexcept and requires
// the genuine observed actual count to be zero; arbitrary count-changing
// decrement continuations are outside this domain. No extra retain/admission.
// Frames are initialized stable caller preimages; contexts and Acquired remain
// live/disjoint through callbacks and explicit failure disposition. Reached
// children require genuine dispatch and their own persistent source frames.
// Nested registry valid-iterator and array nonoverflowing/disjoint-allocation
// domains apply. No native private stack/FH3/SEH compatibility or rollback.
// Do NOT install these identities into legacy logical node/light companions
// or destructors; their destruction/root-attachment closure is separate.

// B6ED80[140], ECX node; scene/recurse words, RET8. Initial170 read BEFORE
// captured request. Unregister old, reload170, publish/retain/release, register
// CURRENT170; then late recursion LOWBYTE/currenthead/current50/fresh next.
void set_native_node_scene_storage_00b6ed80(void* actual_node,
    NativeNodeSceneAttachmentFrame&, NativeNodeSceneAttachmentContext&,
    NativeNodeSceneAttachmentAcquired&);
// B6EE10[110], same ABI. Capture expected BEFORE initial170. Matching path
// unregisters then rereads170, clears BEFORE release/current0. Replacement
// installed by terminal callback survives. Recursive54 uses captured expected.
void remove_native_node_scene_storage_00b6ee10(void* actual_node,
    NativeNodeSceneAttachmentFrame&, NativeNodeSceneAttachmentContext&,
    NativeNodeSceneAttachmentAcquired&);
// B7C020[159], same ABI. Capture search begin/count BEFORE request; absent
// grows via actual B7B390, stores captured request/count++, registers then
// current increment. Duplicate still recurses. Reached append requires a
// genuine nonnull scene; no native null guard or invented fallback.
void attach_native_light_scene_storage_00b7c020(void* actual_light,
    NativeNodeSceneAttachmentFrame&, NativeNodeSceneAttachmentContext&,
    NativeNodeSceneAttachmentAcquired&);
// B7BD60[138], same ABI. Pass actual requested argument cell to B7B620,
// unregister/release captured requested; absent still recurses through54.
// The original unused PUSH ECX private word is not an added source field.
void remove_native_light_scene_storage_00b7bd60(void* actual_light,
    NativeNodeSceneAttachmentFrame&, NativeNodeSceneAttachmentContext&,
    NativeNodeSceneAttachmentAcquired&);
} // namespace bsp
