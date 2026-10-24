#include "bsp/game_native_mission_frame.hpp"

#include <windows.h>

#include <array>
#include <stdexcept>

namespace bsp::game {
namespace {

enum Report : std::uint32_t {
    kMissingHooks = 1u << 0,
    kWrongView = 1u << 1,
    kReentrant = 1u << 2,
    kScope = 1u << 3,
    kTerrain = 1u << 4,
    kOcean = 1u << 5,
    kWeather = 1u << 6,
    kPost = 1u << 7,
    kTonemap = 1u << 8,
};

bool environment_one(const char* name) noexcept
{
    char value[2]{};
    return GetEnvironmentVariableA(name, value, sizeof value) == 1 && value[0] == '1';
}

void report_once(std::uint32_t& reported, Report bit,
    const NativeMissionFrameHooks& hooks, const char* message)
{
    if ((reported & bit) == 0 && hooks.log) {
        reported |= bit;
        hooks.log(message);
    }
}

void report_pass_once(std::uint32_t& reported, Report bit,
    const NativeMissionFrameHooks& hooks, bool requested,
    const char* switch_name, const char* blocker)
{
    if ((reported & bit) != 0 || !hooks.log) return;
    std::string message(switch_name);
    message += requested ? "=1 no-op: blocker=" : "=0 disabled: blocker=";
    message += blocker;
    report_once(reported, bit, hooks, message.c_str());
}

class FrameHost final : public GameRenderFrameHost {
public:
    FrameHost(const NativeMissionFrameSwitches& switches,
        const NativeMissionFrameHooks& hooks, std::uint32_t& reported)
        : switches_(switches), hooks_(hooks), reported_(reported)
    {
    }

    void report_unavailable_passes()
    {
        terrain_pass();
        ocean_pass();
        weather_pass();
        post_pass();
        tonemap_pass();
    }

    bool render_queue_retained() override { return hooks_.queue_retained(); }
    void renderer_begin_frame() override
    {
        begin_attempted = true;
        begin_succeeded = hooks_.begin_and_clear();
    }

    // These native shell owners are absent from the bounded ship scene. Their
    // omission is reported once; the diagnostic overlay has its own later slot.
    void hud_render(float) override {}
    void hud_post_render() override {}
    void register_profiler_zone(const char*) override {}
    bool shadow_map_owner_present() override { return false; }
    bool render_camera_present() override { return true; }
    void update_shadow_map_view_00a8f3b0() override {}

    void enqueue_frame_job(GameRenderJob job) override
    {
        // The existing 004CA440 body enqueues exactly these two jobs.
        jobs_[job_count_++] = job;
    }
    void dispatch_frame_jobs() override
    {
        // 00BE2FA0 drains LIFO. Geometry submission is part of camera job B
        // (0068A0D0 -> 00B101C0), not the sound/listener job A.
        while (job_count_ != 0) {
            const auto job = jobs_[--job_count_];
            if (job == GameRenderJob::kCameraUpdate004b4820) {
                hooks_.update_camera();
                terrain_pass();
                ocean_pass();
                weather_pass();
                hooks_.gather_sort_submit();
            }
            // kWorldView004bbd00 submits the sound listener through 0068A670;
            // no audio owner is fabricated for this render-only adapter.
        }
    }
    void render_interface_ortho_view_0059d7b0() override {}
    void render_interface_ortho_view_00535430() override {}
    void submit_world_view_00735b50() override {}
    bool input_action_pressed(int) override { return false; }
    bool create_screenshot_directory(const std::string&) override
    {
        throw std::logic_error("native mission frame has no native screenshot directory provider");
    }
    bool front_end_preview_active() override { return false; }
    void render_front_end_preview_00503510() override {}
    void draw_gui_00aa45a0() override {}
    bool debug_overlay_present() override { return false; }
    void draw_debug_overlay_0078a430() override {}
    void execute_render_command_queue() override
    {
        hooks_.execute_commands();
        post_pass();
        tonemap_pass();
    }
    void flush_diagnostic_sink_004c14c0() override { hooks_.overlay_and_capture(); }
    void draw_profiler_overlay_00be3bf0() override {}
    void shadow_map_frame_end_00a8ac00() override {}
    bool scene_present() override { return false; }
    void scene_post_render() override {}
    void flush_render_resources_00b0d190() override {}
    void queue_camera_finalize_00b1ef60() override {}
    void profiler_set_finish_slot_color(std::uint32_t) override {}
    void profiler_begin_finish_counter() override {}
    void profiler_end_finish_counter() override {}
    void renderer_end_frame_default() override
    {
        end_attempted = true;
        presented = hooks_.end_and_present();
    }
    void renderer_end_frame_capture(const std::string&) override
    {
        throw std::logic_error("native mission frame has no native screenshot end-frame provider");
    }

    bool begin_attempted{};
    bool begin_succeeded{};
    bool end_attempted{};
    bool presented{};

private:
    // Explicit no-op visual passes. Their positions are adapter boundaries,
    // not a recovered ordering of the original main-scene multipass renderer.
    void terrain_pass()
    {
        report_pass_once(reported_, kTerrain, hooks_, switches_.terrain,
            "BSP_NATIVE_MISSION_TERRAIN", "requires retained terrain visibility nodes, "
            "33x33 patch vertex/index streams, terrain materials and native section-pool bindings "
            "(00ADD3F0/00ADE0E0/00ADC930/00AE1A10/00AE47E0)");
    }
    void ocean_pass()
    {
        report_pass_once(reported_, kOcean, hooks_, switches_.ocean,
            "BSP_NATIVE_MISSION_OCEAN", "requires retained world+A8 ocean/spectrum, "
            "projector/grid, float16 height textures, reflection/refraction targets and ocean "
            "effect/material execution providers (00B9AE00/00B9D7B0/00BA5030/00B9F0A0/00BA51C0)");
    }
    void weather_pass()
    {
        report_pass_once(reported_, kWeather, hooks_, switches_.weather,
            "BSP_NATIVE_MISSION_WEATHER", "requires retained world environment/weather, "
            "sky/cloud geometry and materials, particle emitters and main-scene execution bindings "
            "(0078CFF0/00BBCF60-00BBDEC0)");
    }
    void post_pass()
    {
        report_pass_once(reported_, kPost, hooks_, switches_.post,
            "BSP_NATIVE_MISSION_POST", "requires retained main-scene render targets, "
            "post-effect instances/materials and complete 00B156C0 continuation providers");
    }
    void tonemap_pass()
    {
        report_pass_once(reported_, kTonemap, hooks_, switches_.tonemap,
            "BSP_NATIVE_MISSION_TONEMAP", "requires retained HDR scene/luminance targets, "
            "exposure state and tone-map effect/material bindings in the 00B156C0 scene pipeline");
    }

    const NativeMissionFrameSwitches& switches_;
    const NativeMissionFrameHooks& hooks_;
    std::uint32_t& reported_;
    std::array<GameRenderJob, 2> jobs_{};
    std::size_t job_count_{};
};

} // namespace

NativeMissionFrameSwitches native_mission_frame_switches_from_environment(
    bool mission_ship_enabled) noexcept
{
    NativeMissionFrameSwitches switches;
    switches.enabled = mission_ship_enabled && environment_one("BSP_NATIVE_MISSION_FRAME");
    if (switches.enabled) {
        switches.terrain = environment_one("BSP_NATIVE_MISSION_TERRAIN");
        switches.ocean = environment_one("BSP_NATIVE_MISSION_OCEAN");
        switches.weather = environment_one("BSP_NATIVE_MISSION_WEATHER");
        switches.post = environment_one("BSP_NATIVE_MISSION_POST");
        switches.tonemap = environment_one("BSP_NATIVE_MISSION_TONEMAP");
    }
    return switches;
}

NativeMissionFrameResult NativeMissionFrameCoordinator::run(GameRenderFrameState& state,
    const NativeMissionFrameSwitches& switches, const NativeMissionFrameHooks& hooks)
{
    if (!switches.enabled) return NativeMissionFrameResult::kDisabled;
    if (running_) {
        report_once(reported_, kReentrant, hooks,
            "native mission frame blocked: reentrant adapter entry cannot begin or present twice");
        return NativeMissionFrameResult::kBlocked;
    }
    struct RunningGuard {
        bool& flag;
        explicit RunningGuard(bool& value) : flag(value) { flag = true; }
        ~RunningGuard() { flag = false; }
    } guard(running_);
    FrameHost host(switches, hooks, reported_);
    host.report_unavailable_passes();
    if (!hooks.begin_and_clear || !hooks.update_camera || !hooks.gather_sort_submit
        || !hooks.execute_commands || !hooks.overlay_and_capture || !hooks.end_and_present
        || !hooks.queue_retained || !hooks.log) {
        report_once(reported_, kMissingHooks, hooks,
            "native mission frame blocked: requires begin/clear, camera, native scene submission, "
            "queue execution, overlay/capture, end/present, queue-retention and file-log providers");
        return NativeMissionFrameResult::kBlocked;
    }
    const GameRenderViewGate gate{state.local_player_mode, state.split_view, state.game_state,
        state.interface_manager_present, state.mission_end_suspended, state.interface_level};
    if (state.game_state != static_cast<std::uint32_t>(GameStateId::kInMission)
        || select_view_path_004ca4e7(gate) != GameRenderViewPath::kJobPoolScene
        || state.screenshot.capturing) {
        report_once(reported_, kWrongView, hooks,
            "native mission frame blocked: requires the in-mission level-1 scene branch and native "
            "screenshot capture OFF; use the diagnostic overlay/capture provider for captures");
        return NativeMissionFrameResult::kBlocked;
    }

    if (!try_begin_render_frame_004c6c30(state, host)) {
        return NativeMissionFrameResult::kQueueRetained;
    }
    if (host.begin_attempted && !host.begin_succeeded) {
        // The native BeginFrame reports success even when inhibited/lost. This
        // adapter deliberately aborts the new frame when its device bridge
        // cannot continue, before any camera/submission/overlay callback.
        state.phase = GameRenderPhase::kIdle;
        return NativeMissionFrameResult::kBeginFailed;
    }
    report_once(reported_, kScope, hooks,
        "native mission frame adapter: original begin/render/finish phase bodies; native "
        "HUD/GUI/audio/shadow/profiler, world post-render, resource flush and camera-finalize "
        "shell callbacks unavailable; diagnostic overlay/capture follows native command execution");
    run_game_render(state, host);
    finish_render_frame_004ca1f0(state, host);
    if (!host.end_attempted) return NativeMissionFrameResult::kQueueRetained;
    return host.presented ? NativeMissionFrameResult::kPresented
                          : NativeMissionFrameResult::kPresentFailed;
}

} // namespace bsp::game
