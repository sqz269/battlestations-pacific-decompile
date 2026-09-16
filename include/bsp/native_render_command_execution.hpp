#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeRendererSynchronizationGlobals;
struct NativeRendererFrameTargetsContext;
struct NativeCameraFrameCommandContext;
struct NativeSystemConstantGatherContext;
struct NativeSystemConstantGatherFrame;
struct NativeRenderJobPublicationContext;
struct NativeRenderBatchPreparationContext;
class NativeRenderPreparationJobDispatch;
struct NativeMaterialEntryDispatchContext;
struct NativeMaterialEntryDispatchFrame;

// All providers borrow the SAME actual renderer/service/synchronization,
// string pool and raw singleton manager. Context identities remain stable;
// native publication values/profile slots remain live through callbacks.
// Optional providers are required only when their native stage is reached.
struct NativeRenderCommandExecutionContext {
    void* const volatile& actual_renderer_00f8d394;
    void* const volatile& actual_service_00f8d39c;
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc;
    NativeStringRawPoolContext& actual_strings;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    NativeRendererFrameTargetsContext* targets{};
    const NativeCameraFrameCommandContext* camera{};
    const NativeSystemConstantGatherContext* system{};
    NativeRenderJobPublicationContext* jobs{};
    NativeRenderBatchPreparationContext* preparation{};
    NativeRenderPreparationJobDispatch* preparation_dispatch{};
    NativeMaterialEntryDispatchContext* entries{};
};

// One fresh persistent system frame plus prepared pass frames for each batch.
// Prefix preimage and sampler/pass obligations survive exceptions. This entry
// does not initialize, discard, replay or retire them. Null frame is valid only
// when readiness returns false; batch frames may be null on no-selected-pass
// paths under the existing entry dispatch contract.
struct NativeRenderCommandExecutionFrame {
    NativeSystemConstantGatherFrame* system{};
    NativeMaterialEntryDispatchFrame* batches[2]{};
};

// Full B1D950[369]: native ECX actual44h command, no stack arguments, RET.
// Capture context+8 camera before target binding; current scene after camera;
// raw pooled diagnostic label; arm reset only before metadata call. Current
// sync mode and signed counts select serial or genuine job preparation; mode
// store precedes frame getter, batch reload precedes preparation-job getter.
// Execute current batches in order, then disarm before resetting to "X".
void execute_native_render_command_00b1d950(void* actual_command,
    NativeRenderCommandExecutionContext&, NativeRenderCommandExecutionFrame*);

// Source API adds borrowed context/frame. Unsupported current concrete slots
// raise contract errors. Original FH3/SEH/private-stack/register/fault ABI,
// application owner construction and gameplay validation are not established.
} // namespace bsp
