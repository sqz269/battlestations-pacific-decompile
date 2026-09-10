#include "bsp/render_command_execution.hpp"
#include <array>
#include <exception>
#include <limits>

namespace bsp {
bool can_execute_render_command_00b20240(const D3D9DrawState& state) noexcept {
    return state.inhibit == 0 && !state.device_lost;
}
bool is_render_frame_active_00b1fe20(std::uint32_t active_frame) noexcept {
    return active_frame != 0;
}
bool execute_render_batch_00b55550(InstanceRenderQueue& queue,
    const RenderCommandExecutionState& state, RenderBatchMaterialDispatch& dispatch,
    std::string& error) {
    error.clear();
    if (!is_render_frame_active_00b1fe20(state.active_frame)) return true;
    state.material_effect_owner = nullptr;
    for (std::size_t index = 0; index < queue.entries().size(); ++index) {
        if (queue.entries().size() > static_cast<std::size_t>(INT32_MAX) / 4) {
            error = "Render batch count exceeds native signed pointer-distance domain"; return false;
        }
        auto* entry = queue.entries()[index];
        if (!entry || !entry->section) {
            error = "Render batch has no live entry/section"; return false;
        }
        if (!dispatch.execute_material_entry(*entry, error)) return false;
    }
    return true;
}
namespace {
struct BatchPreparationJob {
    RenderCommandBatch* batch{};
    const RenderCommandExecutionEnvironment* environment{};
    bool succeeded{};
    std::string error;
    static void run(void* opaque) noexcept {
        auto& job = *static_cast<BatchPreparationJob*>(opaque);
        try {
            job.succeeded = prepare_render_batch_00b51df0(job.batch->entries,
                job.batch->preparation_mode, job.environment->sort_configurations,
                job.environment->material_keys, job.error);
        } catch (const std::exception& exception) { job.error = exception.what(); }
        catch (...) { job.error = "Render batch preparation raised an exception"; }
    }
};
struct DiagnosticScope {
    RenderCommandDiagnosticState*& current;
    ~DiagnosticScope() { if (current) current->current.assign("X", 1); }
};
}
bool execute_render_command_00b1d950_fragment(RenderCommand& command,
    RenderCommandExecutionEnvironment& environment, std::string& error) {
    error.clear();
    if (!can_execute_render_command_00b20240(environment.state.draw)) return true;
    if (!command.context || !command.context->camera) {
        error = "Render command requires its actual context and camera"; return false;
    }
    auto* camera = command.context->camera; // Native captures this before target binding.
    const auto target_result = environment.renderer.bind_frame_targets_00b24e70(
        environment.scene_operations.targets(command.context->target),
        environment.default_color, environment.manage_srgb_write);
    if (!environment.scene_operations.prepare_camera_00b71360(*camera, error)
        || !environment.scene_operations.upload_system_prefix_00b46a70(command.scene, *camera, error))
        return false;
    if (environment.diagnostic) {
        if (command.diagnostic.size())
            environment.diagnostic->current.assign(command.diagnostic.data(), command.diagnostic.size());
        else environment.diagnostic->current.clear();
    }
    DiagnosticScope scope{environment.diagnostic};
    // Concrete renderer virtual+114=00B20210 is RET4. The ignored command+1C
    // argument has no storage projection because this override cannot read it.
    if (is_render_frame_active_00b1fe20(environment.state.active_frame)) {
        if (!command.batches[0] || !command.batches[1]) {
            error = "Render command requires both actual batches"; return false;
        }
        const bool parallel = !environment.state.synchronization.enabled
            && command.batches[0]->entries.entries().size() > 50
            && command.batches[1]->entries.entries().size() > 50;
        if (parallel) {
            if (!environment.jobs) {
                error = "Native command branch requires a preparation job service"; return false;
            }
            std::array<BatchPreparationJob, 2> jobs;
            for (std::uint32_t index = 0; index < 2; ++index) {
                auto* batch = command.batches[index];
                if (!batch) {
                    // Do not abandon any queued work holding stack arguments.
                    environment.jobs->wait(1);
                    error = "Command callback removed a required batch"; return false;
                }
                batch->preparation_mode = index;
                jobs[index].batch = batch; jobs[index].environment = &environment;
                environment.jobs->enqueue(BatchPreparationJob::run, &jobs[index]);
            }
            environment.jobs->wait(1);
            for (const auto& job : jobs) if (!job.succeeded) { error = job.error; return false; }
        } else {
            for (std::uint32_t index = 0; index < 2; ++index) {
                auto* batch = command.batches[index];
                if (!batch) { error = "Command callback removed a required batch"; return false; }
                if (!prepare_render_batch_00b51df0(batch->entries, index,
                    environment.sort_configurations, environment.material_keys, error)) return false;
            }
        }
        for (std::uint32_t index = 0; index < 2; ++index) {
            auto* batch = command.batches[index];
            if (!batch) { error = "Command callback removed a required batch"; return false; }
            if (!execute_render_batch_00b55550(batch->entries, environment.state,
                environment.material_dispatch, error)) return false;
        }
    }
    if (FAILED(target_result)) { error = "Render command target surface binding failed"; return false; }
    return true;
}
}
