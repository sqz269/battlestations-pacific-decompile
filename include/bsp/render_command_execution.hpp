#pragma once
#include "bsp/render_command_queue.hpp"
#include "bsp/instance_sort.hpp"
#include <string>

namespace bsp {
// Actual renderer state projections. The frame count and effect-owner cache are
// shared by commands and batches, not copied at construction.
struct RenderCommandExecutionState {
    const D3D9DrawState& draw;
    const std::uint32_t& active_frame; // renderer+1998
    const void*& material_effect_owner; // global0108FBF4; pass+14 effect owner identity
    const RendererSynchronization& synchronization; // global0108D6DC gate
};
bool can_execute_render_command_00b20240(const D3D9DrawState&) noexcept;
bool is_render_frame_active_00b1fe20(std::uint32_t active_frame) noexcept;

// Required actual effect virtual+14 dispatcher. Resolve it from the entry's
// retained section/material/effect identity, then execute that implementation.
// Native00B45360/00B44750 are not implemented by this interface. A callback that
// merely records a call or blindly draws one fixed pass is not their port.
class RenderBatchMaterialDispatch {
public:
    virtual ~RenderBatchMaterialDispatch() = default;
    virtual bool execute_material_entry(InstanceRenderEntry&, std::string& error) = 0;
};
// Native ECX=batch, RET8; both stack arguments are unused. Active-frame check,
// clear global effect-owner identity, then live count/list reload after each callback.
// Callback mutations must keep the current entry and remaining list valid.
// Adapter failure stops subsequent calls, retaining earlier effects; native
// has no error return. Count must fit signed32 pointer-distance arithmetic.
bool execute_render_batch_00b55550(InstanceRenderQueue&,
    const RenderCommandExecutionState&, RenderBatchMaterialDispatch&, std::string&);

// Required asynchronous job service. enqueue schedules the given actual work;
// wait(1) finishes every previously submitted preparation before returning and
// establishes visibility of their writes. No serial fallback is substituted
// for a missing service when the native two-counts>50 branch selects jobs.
class RenderCommandPreparationJobs {
public:
    virtual ~RenderCommandPreparationJobs() = default;
    virtual void enqueue(void (*work)(void*) noexcept, void* argument) noexcept = 0;
    virtual void wait(std::uint32_t mode) noexcept = 0;
};

// Required typed-owner resolution and camera/system operations. These receive
// the actual references held by RenderCommandContext. There are no default
// no-ops: camera00B71360 and system gather00B46A70 remain explicit dependencies.
class RenderCommandSceneOperations {
public:
    virtual ~RenderCommandSceneOperations() = default;
    virtual std::shared_ptr<D3D9FrameTargets> targets(RenderCommandReference*) = 0;
    virtual bool prepare_camera_00b71360(RenderCommandReference&, std::string&) = 0;
    virtual bool upload_system_prefix_00b46a70(RenderCommandReference* scene,
        RenderCommandReference& camera, std::string&) = 0;
};

// Semantic copy of optional diagnostic singleton string+684. An owner may
// replace the pointer during callbacks; scope exit reloads the pointer and
// assigns literal "X" to whichever singleton is current (00CE9A38). Native
// size-class allocator is not
// replaced with guessed globals here. It is a separate storage/ABI boundary.
struct RenderCommandDiagnosticState { std::string current; };
struct RenderCommandExecutionEnvironment {
    D3D9StateCache& renderer;
    const D3D9SurfaceBinding& default_color;
    bool manage_srgb_write; // application00F8D398
    RenderCommandExecutionState state;
    const std::vector<RenderBatchSortConfiguration>& sort_configurations;
    RenderBatchMaterialKeySource material_keys;
    RenderCommandSceneOperations& scene_operations;
    RenderBatchMaterialDispatch& material_dispatch;
    RenderCommandPreparationJobs* jobs; // required only for selected job branch
    RenderCommandDiagnosticState*& diagnostic; // nullable singleton slot
};

// Native ECX=command, RET. Read readiness, capture context camera, bind targets,
// prepare camera, upload scene constants, set optional diagnostic string, then
// the concrete renderer+114 override00B20210 does nothing (complete RET4).
// Check active frame, prepare both batches serially or via actual jobs, execute
// both in order, then reset the current diagnostic singleton to "X" on scope exit.
// Batch pointers are reloaded between calls. The captured camera must stay
// alive across callbacks, as native does not add a temporary reference.
//
// This is an orchestration fragment with REQUIRED unresolved operations above,
// not a runnable native material pipeline. Malformed bindings/adapters fail
// explicitly; COM target failures do not suppress later native stages and are
// reported after execution. C++ exceptions propagate with diagnostic cleanup.
bool execute_render_command_00b1d950_fragment(RenderCommand&,
    RenderCommandExecutionEnvironment&, std::string& error);
}
