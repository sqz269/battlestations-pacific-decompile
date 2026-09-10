#include "bsp/render_tail.hpp"
#include "bsp/particle_clock_lifetime.hpp"

namespace bsp {
namespace {
void forward_particle_time(ParticleTimeSink& sink, const float& time_ms) {
    float forwarded;
    const auto* source = &time_ms;
    //00B19A33/00B19A3D: raw clock storage precedes this per-call x87
    // load/spill. Signaling NaNs quiet here and update the x87 status.
    __asm {
        mov eax, source
        fld dword ptr [eax]
        fstp forwarded
    }
    sink.set_time(forwarded);
}
}

float particle_clock_time_004e538e(float global_time) noexcept
{
    // FLD dword [00f876a4]; FMUL qword [00ce47a0]; FSTP dword [ESP+28h]. The
    // product is formed in x87 double precision and rounded once on the store.
    return static_cast<float>(static_cast<double>(global_time) * kParticleClockTimeScale);
}

void set_particle_clock_time_00b19a10(ParticleClock& clock, float time_ms)
{
    auto* const owned_begin = clock.owned_records;
    const auto* input = &time_ms;
    auto* output = &clock.shader_time;
    __asm {
        mov eax, input
        mov edx, output
        movss xmm0, dword ptr [eax]
        movss dword ptr [edx], xmm0
    }
    if (owned_begin != nullptr) {
        // ESI advances independently; only the end is recomputed from the
        // reloaded native +8/+C after each virtual call. Reallocation during a
        // callback has the same native invalidation risk, not index semantics.
        auto cursor = reinterpret_cast<std::uintptr_t>(owned_begin);
        auto end = cursor + clock.sink_count * kParticleClockRecordStride;
        while (cursor != end) {
            forward_particle_time(*reinterpret_cast<ParticleClockOwnedRecord*>(cursor)->sink_28,time_ms);
            end = reinterpret_cast<std::uintptr_t>(clock.owned_records)
                + clock.sink_count * kParticleClockRecordStride;
            cursor += kParticleClockRecordStride;
        }
        return;
    }
    // Retained compatibility path for earlier borrowed sink projections.
    if (clock.sinks == nullptr) {
        return;
    }
    for (std::uint32_t index = 0; index < clock.sink_count; ++index) {
        ParticleTimeSink* sink = clock.sinks[index];
        if (sink != nullptr) {
            forward_particle_time(*sink,time_ms);
        }
    }
}

FoliageBuildResult build_foliage_visible_set_00af0c50(FoliageGroupManager& manager,
    FoliageGroup* groups, std::uint32_t group_count, const FoliageCameraPair& cameras,
    std::uint32_t camera_valid_flags, FoliageBuildHost& host)
{
    FoliageBuildResult result{};

    manager.frame_counter += 1;
    manager.current_camera = cameras.render_camera;
    host.reset_group_scratch();

    // First pass 00af0c91..00af0d7a: decide visibility and size the buffer.
    for (std::uint32_t index = 0; index < group_count; ++index) {
        FoliageGroup& group = groups[index];
        if (group.description_needs_render_mode
            && !host.camera_render_mode_set(cameras.render_camera)) {
            continue;
        }
        group.passes_group_test = host.group_passes_test(group, cameras.render_camera);
        if (!group.passes_group_test) {
            continue;
        }
        group.primary_cull = host.cull_group(cameras.render_camera, group);
        if (host.debug_capture_enabled()) {
            host.debug_submit(group, cameras.render_camera, group.primary_cull);
        }
        bool visible = false;
        if (group.primary_cull != kFrustumRejectCode) {
            visible = true;
        } else if (group.description_allows_secondary) {
            group.secondary_cull = host.cull_group(cameras.secondary_camera, group);
            visible = group.secondary_cull != kFrustumRejectCode;
        }
        group.visible = visible;
        host.set_group_visible(group, visible ? 1.0f : 0.0f);
        if (visible) {
            result.pending_quads += group.quad_count;
        }
    }

    // 00af0d80..00af0dbe: a buffer is locked only when something is visible and
    // the manager owns a buffer source.
    result.buffer_bytes = result.pending_quads * 4u;
    if (result.pending_quads != 0 && manager.dynamic_buffer_owner != nullptr) {
        manager.locked_buffer = host.lock_dynamic_buffer(result.buffer_bytes);
        result.buffer_locked = true;
    }

    if ((camera_valid_flags & kCameraWorldMatrixValidBit) == 0) {
        host.refresh_camera_world_matrix(cameras.render_camera);
    }
    host.refresh_camera_view_matrices(cameras.render_camera);

    // Second pass 00af0df0..00af0e31: groups that survived but hold no geometry
    // are queued for a build, then the queue is flushed once.
    for (std::uint32_t index = 0; index < group_count; ++index) {
        FoliageGroup& group = groups[index];
        if (group.lod_scale == kFoliageGroupInactiveLodScale) {
            continue;
        }
        if (group.quad_count == 0) {
            host.request_group_build(group);
        }
    }
    host.flush_group_builds();

    // Third pass 00af0e60..00af0e9b: fill the locked buffer and count the quads.
    for (std::uint32_t index = 0; index < group_count; ++index) {
        FoliageGroup& group = groups[index];
        if (group.lod_scale == kFoliageGroupInactiveLodScale) {
            continue;
        }
        if (group.quad_count != 0) {
            host.fill_group_quads(group, cameras.render_camera);
        }
        result.visible_quads += group.quad_count;
    }

    if (manager.dynamic_buffer_owner != nullptr) {
        if (result.buffer_locked) {
            host.unlock_dynamic_buffer();
            manager.locked_buffer = nullptr;
            result.buffer_locked = false;
        }
        result.device_vertex_count = result.visible_quads * 4u;
        result.device_triangle_count = result.visible_quads * 2u;
        host.publish_device_counts(result.device_vertex_count, result.device_triangle_count);
    }
    return result;
}

SoundRequestDecision apply_sound_request_00941140(SoundRequestQueue& queue,
    bool slot_one_condition)
{
    SoundRequestDecision decision{};
    if (queue.forced_slot != kSoundRequestNone) {
        decision.apply = true;
        decision.slot = queue.forced_slot;
    } else {
        for (int slot = kSoundRequestSlotCount - 1; slot >= 0; --slot) {
            if (queue.requested[static_cast<std::size_t>(slot)] == 0) {
                continue;
            }
            if (slot == 1 && !slot_one_condition) {
                slot = kSoundRequestFallbackSlot;
            }
            decision.apply = true;
            decision.slot = slot;
            break;
        }
    }
    queue.requested.fill(0);
    queue.forced_slot = kSoundRequestNone;
    return decision;
}

GuiVisibilityDecision decide_gui_visibility_004c6c70(const GuiVisibilityInputs& in)
{
    GuiVisibilityDecision decision{};

    bool wants_gui = in.loading_screen_active || in.menu_screen_active
        || in.other_screen_active || in.game_state == 2;

    bool not_cinematic = true;
    if (in.hud_present) {
        wants_gui = wants_gui || in.hud_dialog_flag || in.base_screen_active
            || in.hud_page_a_flag || in.hud_page_b_flag || in.loading_screen_sub_flag
            || (in.hud_page_c_flag && in.hud_page_c_second_flag && !in.hud_page_b_suppress);
        not_cinematic = !in.cinematic_flag;
    }

    const bool no_modal = !in.modal_present || !in.modal_flag;
    bool enabled = (wants_gui && not_cinematic && no_modal) || in.menu_command_flag;
    if (enabled && in.gui_suppressed) {
        enabled = false;
    }
    decision.enabled = enabled;

    if (!in.hud_present) {
        return decision; // 00aa0e50 is never reached without the HUD object
    }

    bool pointer = (in.hud_page_a_flag || in.hud_page_b_flag) && !in.cinematic_flag
        && !(in.modal_present && in.modal_flag);
    if (pointer && !in.gui_suppressed) {
        decision.applies_pointer = true; // 004c6df8 branches straight to the call
        decision.pointer_visible = true;
        return decision;
    }
    if (pointer) {
        pointer = false; // suppressed, then the enabled test decides
    }
    if (enabled) {
        return decision;
    }
    decision.applies_pointer = true;
    decision.pointer_visible = pointer;
    return decision;
}

RenderTailResult run_render_tail(const RenderTailInputs& inputs,
    const RenderTailCounters& counters, RenderTailHost& host)
{
    RenderTailResult result{};

    if (inputs.simulation_ran) {
        host.set_particle_clock_time(particle_clock_time_004e538e(inputs.global_time));
        result.particle_step_ran = true;
    } else {
        host.update_without_simulation(); // 004e53b4
    }

    if (inputs.game_state == kInMissionGameState) {
        host.set_foliage_shader_time(inputs.foliage_shader_time);
        const FoliageCameraPair cameras = host.select_foliage_cameras();
        host.build_foliage_visible_set(cameras);
        result.foliage_step_ran = true;
    }

    host.apply_gui_visibility();
    if (inputs.menu_interface_present) {
        host.update_menu_interface(inputs.raw_delta);
    }
    host.update_multiplayer_interface();

    // 004e5439..004e5488. The latch is cleared after every service call, and the
    // loop keeps draining while the service reports more work.
    bool pending = host.service_menu_requests();
    ++result.requests_serviced;
    host.clear_request_latch();
    while (pending) {
        host.tick_peer_session();
        host.update_without_simulation();
        pending = host.service_menu_requests();
        ++result.requests_serviced;
        ++result.drain_iterations;
        host.clear_request_latch();
    }
    pending = false; // 004e548a, the redundant clear the compiler kept

    host.apply_sound_request();
    host.update_frontend_screens();

    host.profiler_end_counter(counters.game_counter);
    if (!pending) {
        host.profiler_begin_counter(counters.render_counter);
        host.game_render();
        host.profiler_end_counter(counters.render_counter);
        result.render_block_ran = true;
    }
    host.finish_render_frame();
    return result;
}
}
