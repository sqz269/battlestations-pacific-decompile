#pragma once
#include "bsp/native_texture_surface_getter.hpp"
#include <cstdint>

namespace bsp {
class NativeCameraOwner;
struct CameraAxesCrtAccess;
struct NativeRenderSceneSubmissionContext;
struct NativeRenderBatchPreparationContext;
struct NativeRenderCommandStorage;
struct NativeRenderCommandQueueStorage;

// Borrow the live service, camera, command/scene/upload owners and queue
// publication already used by the application. No camera/world/queue is made.
struct NativeMainSceneSubmissionContext {
    NativeRenderSceneSubmissionContext& submission;
    NativeRenderBatchPreparationContext& queue;
    void* const volatile& actual_clock_01090ab0;
    const volatile std::uint32_t* actual_clock_profile_00d68d50;
    const CameraAxesCrtAccess& crt;
    const volatile double& actual_hysteresis_00d5e1c8;
    const volatile float& actual_clip_threshold_00cfbc84;
    const volatile float& actual_negative_one_00d7a260;
    const volatile float& actual_clip_distance_00ce3958;
    NativeTextureSurfaceReferenceIncrement const volatile& actual_increment_00ce221c;
    NativeTextureSurfaceReferenceIncrement const volatile& actual_decrement_00ce2220;
};

// External observations only. Retain this and the existing ownership domains
// after failure. append_started can be true with a partially changed queue;
// append_returned means traversal/upload returned as well as pointer append.
struct NativeMainSceneSubmissionOperation {
    void* allocation{};
    NativeRenderCommandStorage* command{};
    NativeRenderCommandQueueStorage* append_queue{};
    NativeRenderCommandQueueStorage* metadata_queue{};
    bool append_started{};
    bool append_returned{};
    bool completed{};
};

// Complete ordinary B101C0..B104F2. Original ECX service, scene/camera stack,
// RET8. New explicit source ABI. The supplied companion must be the existing
// live owner of the SAME actual camera passed to the native caller. Service
// storage extends through +24F, with actual node pointers and current fields.
//
// Full tracked-node velocity, x87 time fraction and hysteresis; B1F170 command;
// D5E13C publication; actual camera clip plane; first raw queue getter/B1EB80
// flags1; SECOND raw getter/B1CB50 metadata; actual retained-camera update.
// The clock must have the current canonical D68D50:BEE070 slot1C. The existing
// CRT handler and reached raw node/visibility and upload callees must return
// without C++ exceptions across naked frames. Native FH3/SEH, arbitrary native
// stack aliases and runtime/gameplay equivalence remain separate claims.
//
// D5E13C EXECUTION IS NOT ADMITTED by this caller. Its actual B17240/B156C0
// body and a queue execution domain admitting it are still required. Keeping
// or inspecting the submitted real command does not supply that dependency.
NativeRenderCommandStorage* submit_native_main_scene_00b101c0(void* actual_service,
    void* actual_scene, NativeCameraOwner& actual_camera,
    NativeMainSceneSubmissionContext&, NativeMainSceneSubmissionOperation&);
} // namespace bsp
