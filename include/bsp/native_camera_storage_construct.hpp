#pragma once
#include "bsp/native_camera_pose_storage.hpp"
#include "bsp/native_camera_storage_lifetime.hpp"
#include "bsp/native_node_construction.hpp"
#include "bsp/native_viewport_owner.hpp"

namespace bsp {
struct NativeCameraStorageConstructContext {
    const NativeNodeRawConstants& node_constants;
    NativeViewportRawEnvironment& viewport;
    NativeCameraPoseDispatch& pose_dispatch;
    const NativeCameraLookAtBindings& look_at;
    NativeCameraStorageLifetimeContext& lifetime;
    const volatile std::uint32_t& hundred_00ce3d08;
    const volatile std::uint32_t& scalar_00ce7d20;
    const volatile std::uint32_t& scalar_00d0c5f8;
    const volatile std::uint32_t& scalar_00d5bd98;
    const volatile std::uint32_t& scalar_00ce77fc;
    const volatile std::uint32_t& scalar_00ce3c88;
    // SAME one cell in node/viewport/look_at, and SAME raw strings, registry,
    // imports and world dispatcher throughout construction/failure/lifetime.
};
struct NativeCameraStorageConstructFrame {
    volatile std::uint32_t* scratch; // six live words at original entry E-24h
    volatile std::uint32_t* pushed_words; // two words E-40h and E-3Ch
    // pushed_words[1] is reused by base-name/allocation/origin/dimensions/
    // depth calls and finally the pose target argument; [0] is pose eye.
    NativeCameraPoseFrame pose; // pose entry E-44h, views may overlap dead cells
    NativeNodeBaseDestructionFrame& cleanup_node;
    // Immutable metadata stays disjoint. All selected backing has initialized
    // live DWORD lifetimes/stable addresses; no overlapping aggregate frames.
    // Saved-register/return/EH gaps and private provider stacks are excluded.
};
struct NativeCameraStorageConstructAcquired {
    bool started{}, complete{}, node_complete{}, planes_live{};
    bool viewport_complete{}, viewport_published{}, exception_cleanup_started{};
    bool allocation_cleanup_complete{}, retained_cleanup_complete{}, base_cleanup_complete{};
    std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{};
    void* viewport_allocation{};
    void* viewport_result{};
    void* allocation_cleanup_pointer{};
    NativeNodeBaseDestructionAcquired cleanup_node;
    // Fresh disjoint persistent diagnostics; not a second owner or rollback.
    // State0/2 failure does NOT release the published viewport. Keep its real
    // allocation/reference and all context/frame state for explicit disposition.
};

// Complete604B normal B71A80, ECX fresh actual camera, stack name header,
// EAX same, RET4. Requires aligned actual45Ch storage and no conflicting live
// tail objects. B6F5A0 establishes the real atomic+4 prefix; byte-preserving
// memcpy starts implicit CameraPlaneSet lifetime at+2F4 before genuine B659D0.
// The SAME mutable incoming word first contains the actual name header, then
// allocation result and finally color-byte scratch. It may alias native data
// within the documented explicit-view domain; it is never replaced by a copy.
void* construct_native_camera_storage_00b71a80(void* actual_camera,
    std::size_t actual_slot_bytes, volatile std::uint32_t& name_allocation_color_argument,
    const NativeCameraStorageConstructFrame&, NativeCameraStorageConstructContext&,
    NativeCameraStorageConstructAcquired&);

// Reuses real CRT new-handler allocation/free with native/host size34h. A null
// allocation is not a successful domain: native continues to unconditional
// viewport calls. Raw renderer binding admits actual extent/current target;
// B1FF60 only returns actual+1A14 and establishes no renderer/COM lifetime.
// No fog is constructed: +184 is cleared. Source C++ exception projection
// consumes the three native cleanup states, but is not native FH3/SEH proof.
// Success satisfies the full-construction precondition of the existing camera
// companion only with its SAME real pool slot/domain and persistent lifetime
// bindings. Companion admission/creator ownership are explicit caller actions.
} // namespace bsp
