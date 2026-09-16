#pragma once
#include "bsp/native_renderer_generated_model.hpp"
#include "bsp/native_system_constant_gather.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/native_material_pass_execution.hpp"
#include "bsp/native_material_constant_build.hpp"
#include "bsp/native_camera_reference.hpp"
#include "bsp/native_traceline_render.hpp"
#include <optional>

namespace bsp {
struct NativeRenderEntryCacheContext;
using NativeDebugRecord40Atomic = long (__stdcall*)(volatile long*);

// An explicit application binding for the opaque record+00 current virtual0.
// The numeric profile/target are identities, never callable game addresses.
// invoke must execute that genuine terminal on this actual object, including
// its native effects. A no-op, semantic release or substitute owner is invalid.
// Validation happens only AFTER the native decrement returns zero. Unsupported
// identity leaves the decremented count and all earlier effects in place.
struct NativeDebugRecord40Terminal {
    std::uint32_t profile_token;
    const volatile std::uint32_t* actual_profile;
    std::uint32_t expected_virtual0;
    void* context;
    void (*invoke)(void* context, void* actual_object);
};
struct NativeDebugRecords40VertexContext {
    const volatile double* actual_two_00d7a308;
    const volatile float* actual_depth_00ce3800;
    NativeDebugRecord40Atomic const volatile* actual_decrement_00ce2220;
    const NativeDebugRecord40Terminal* terminals;
    std::uint32_t terminal_count;
};

// Internal parent fragment B2B72F..B2B8CD, not a separate original function.
// ECX actual renderer, EDX locked output, stack explicit borrowed context/RET4.
// Reads actual +1D18/+1D1C. Six 10h vertices/60h output bytes per 28h record;
// every x87 operation, float32 spill, current color read and terminal reload
// remains ordered. Raw valid output/source aliasing is retained. Aliasing the
// new private scratch/context or source call frame is outside this interface.
void __fastcall write_native_debug_records40_vertices_00b2b741(void* actual_renderer,
    void* actual_vertices, const NativeDebugRecords40VertexContext*);

struct NativeRendererDebugRecords40Context {
    NativeRendererGeneratedModelContext& generated;
    const NativeRendererRawModelBinding& raw_models;
    NativeLogicalBufferMappingContext& mapping;
    const NativeTracelineRenderAccess& render_entry;
    const NativeSystemConstantGatherContext& gather;
    NativeMaterialPassExecutionContext& pass;
    // Caller must already bind THIS cameras.pool_0108ffb0 through the existing
    // static-pool adapter. B71930/B71350 use that private canonical binding;
    // camera scalar cleanup uses this environment's pool. No read-only identity
    // accessor exists, so prepare cannot verify that equality or establish it.
    NativeCameraEnvironment& cameras;
    NativeViewportRegistry& viewports;
    NativeRenderEntryCacheContext& entry_cache;
    NativeDebugRecords40VertexContext vertices;
    NativeDebugRecord40Atomic const volatile& actual_increment_00ce221c;
};

// One caller-owned persistent frame per invocation. prepare reserves only host
// registration credits before native effects. Existing camera companions refer
// to the actual allocated slot and SAME node, viewport and render-entry domains.
// No application pool/cache/producer is created. The generated, gather, pass
// and constant frames are distinct and never replayed or implicitly reset.
class NativeRendererDebugRecords40Frame final {
public:
    enum class Phase { fresh, preparing, prepared, running, complete, failed, retired };
    explicit NativeRendererDebugRecords40Frame(const void* gather_prefix_preimage)
        : gather{gather_prefix_preimage}, pass(constants) {}
    ~NativeRendererDebugRecords40Frame() noexcept;
    NativeRendererDebugRecords40Frame(const NativeRendererDebugRecords40Frame&) = delete;
    NativeRendererDebugRecords40Frame& operator=(const NativeRendererDebugRecords40Frame&) = delete;
    void prepare(NativeRendererDebugRecords40Context&);
    void cancel_preparation() noexcept;
    // Caller proves every surviving native creator and host borrow is settled.
    // This retires host storage only; it never performs native cleanup/release.
    void retire_after_host_quiescence() noexcept;
    Phase phase() const noexcept { return phase_; }
    NativeRendererGeneratedModelAcquired cached_model;
    NativeSystemConstantGatherFrame gather;
    NativeMaterialConstantBuildFrame constants;
    NativeMaterialPassExecutionFrame pass;
    std::uint32_t reached_callsite{};
    std::uint32_t captured_lock_vertices{};
    void* locked_stream{};
    void* locked_output{};
    void* camera_slot{};
    NativeCameraOwner* camera_owner{};
    NativeCameraReference* camera_reference{};
    NativeViewportOwner* retained_constructor_viewport{};
    void* captured_entry_cache{};
    void* entry{};
    bool camera_slot_return_started{};
    bool camera_creator_release_started{};
    bool entry_increment_started{};
private:
    friend void __fastcall draw_native_renderer_debug_records40_00b2b580(void*,
        NativeRendererDebugRecords40Context*, NativeRendererDebugRecords40Frame*);
    static void retire_camera(void*, NativeCameraReference&) noexcept;
    void settle_admissions() noexcept;
    void construct_camera();
    NativeRendererDebugRecords40Context* context_{};
    Phase phase_{Phase::fresh};
    SceneAttachmentRuntime::BindingAdmission camera_scene_;
    GeneratedModelLifetimeRuntime::BindingAdmission camera_lifetime_;
    NativeViewportRegistry::Storage viewport_record_;
    NativeViewportRegistry::Admission viewport_admission_;
    std::optional<NativeCameraOwner> camera_owner_storage_;
    std::optional<NativeCameraReference> camera_reference_storage_;
    bool camera_registered_{};
    bool camera_retired_{};
    NativeString camera_name_;
};

// Complete B2B580..B2BB8C, original ECX renderer/plain RET. New source adds EDX
// context and stacked persistent frame. Initially zero count reads no binding.
// Nonzero input needs a prepared frame. The original count*6 lock request is
// retained separately from the current count*2 section primitive count.
// Construction EH cleans reached local strings and returns the camera raw slot
// only while its allocation/constructor state is active. Later failures never
// unlock, undo an entry increment, release a camera/model or drain records.
// Record+00 clears only after the reached terminal returns. Source-domain
// admissions, private stack, FH3/SEH and incidental-register ABI differ from the
// original. Active producer/application binding and gameplay are unvalidated.
void __fastcall draw_native_renderer_debug_records40_00b2b580(void* actual_renderer,
    NativeRendererDebugRecords40Context*, NativeRendererDebugRecords40Frame*);
} // namespace bsp
