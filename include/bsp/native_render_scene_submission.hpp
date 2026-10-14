#pragma once

#include <cstdint>

namespace bsp {
struct NativeRenderCommandActualEnvironment;
class NativeRenderCommandAssociations;
struct NativeRenderCommandQueueStorage;
struct NativeRenderCommandStorage;
struct NativeInstanceGroupUploadAccess;
struct NativeStringRawPoolContext;

// Borrow the SAME command/batch/context owner associations, string publication,
// scene traversal and instance-collection/upload domains. This record creates
// no renderer, queue, owner, table or replacement scene. Prepare associations
// before entry exactly as required by native_render_command_owner.hpp.
struct NativeRenderSceneSubmissionContext {
    NativeRenderCommandActualEnvironment& commands;
    NativeRenderCommandAssociations& associations;
    NativeInstanceGroupUploadAccess& instances;
    NativeStringRawPoolContext& strings;
};

// External acquisition observation. Supply a fresh record; retain it and the
// existing owner domains after a failure. A completed command is NOT destroyed
// if name copying, queue growth, traversal or upload throws. If construction
// throws, the outer native new-expression frees the 44h allocation only; its
// initializer's already-published context/batches retain their existing rules.
struct NativeRenderSceneSubmissionOperation {
    void* allocation{};
    NativeRenderCommandStorage* command{};
    bool appended{};
    bool completed{};
};

// Complete ordinary B1F4D0..B1F5D6. Original ECX queue, scene/camera/targets
// plus an OWNED eight-byte string by value on stack, RET14. This source API
// takes the address of that existing argument header, not a copied/retained
// string. It consumes its buffer on success and C++ failure, leaving the header
// unchanged. Do not destroy that consumed argument again. Success returns the
// newly appended actual command as a source-only observation.
NativeRenderCommandStorage* submit_native_scene_00b1f4d0(
    NativeRenderCommandQueueStorage&, void* actual_scene, void* actual_camera,
    void* actual_targets, void* actual_owned_label_header,
    NativeRenderSceneSubmissionContext&, NativeRenderSceneSubmissionOperation&);

// Complete ordinary B1F5E0..B1F6AE. Original ECX queue, scene/camera/targets/
// drawable, RET10. Append precedes visibility testing; a mask miss leaves the
// real empty command queued. A hit invokes current slot20 then B1E990 using
// the SAME actual command/context. The four canonical raw render profiles use
// the existing concrete node/camera/model/Traceline bodies; changed targets in
// those tables need the existing real nonthrowing application dispatch and
// cannot silently succeed. Other numeric profiles require a separate concrete
// mapping; this entry does not invent a table for them.
NativeRenderCommandStorage* submit_native_drawable_00b1f5e0(
    NativeRenderCommandQueueStorage&, void* actual_scene, void* actual_camera,
    void* actual_targets, void* actual_drawable,
    NativeRenderSceneSubmissionContext&, NativeRenderSceneSubmissionOperation&);

// Native-valid nonnegative array sizes/spans, returning string cleanup and
// live producer-created owners are required. New source context/operation/return
// ABI is not the original binary entry; FH3/SEH, arbitrary C++ dispatch stack
// aliasing, native fault behavior and application/game validation are separate.
} // namespace bsp
