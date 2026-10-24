#pragma once

#include "bsp/game_render_frame.hpp"

#include <cstdint>
#include <functional>

namespace bsp::game {

// Diagnostic composition switches, not recovered game fields. The environment
// helper enables the frame only when both the caller's BSP_NATIVE_MISSION_SHIP
// master and BSP_NATIVE_MISSION_FRAME=1 are set. Every field defaults OFF.
struct NativeMissionFrameSwitches {
    bool enabled{};
    bool terrain{};
    bool ocean{};
    bool weather{};
    bool post{};
    bool tonemap{};
};

NativeMissionFrameSwitches native_mission_frame_switches_from_environment(
    bool mission_ship_enabled) noexcept;

// All callbacks borrow the primary application's existing device, scene,
// camera, command queue and retained execution contexts. No callback may start
// another frame. begin_and_clear owns the sole BeginScene and clear;
// end_and_present owns the sole EndScene/Present and reports its actual result.
// In particular, begin_and_clear must not query presentation() while rendering.
struct NativeMissionFrameHooks {
    std::function<bool()> begin_and_clear;
    std::function<void()> update_camera;
    // Existing native scene submission performs traversal, grouping/sorting,
    // upload and queue insertion. Do not substitute a direct diagnostic draw.
    std::function<void()> gather_sort_submit;
    std::function<void()> execute_commands;
    std::function<void()> overlay_and_capture;
    std::function<bool()> end_and_present;
    // The same retained-queue predicate is read by original begin and finish.
    std::function<bool()> queue_retained;
    // File logger supplied by the application; never a console sink.
    std::function<void(const char*)> log;
};

enum class NativeMissionFrameResult {
    kDisabled,
    kBlocked,
    kQueueRetained,
    kBeginFailed,
    kPresented,
    kPresentFailed,
};

// Adapter over the existing 004C6C30/004CA440/004CA1F0 reconstructed bodies.
// The caller supplies the one actual GameRenderFrameState; this class neither
// copies it nor declares a second phase field. Keep it alive across retained
// queues. It must describe the in-mission level-1 scene branch, with capture
// disabled (the separate diagnostic capture hook remains available).
//
// This is a bounded scene adapter. Native HUD/GUI/audio/shadow/profiler and
// world cleanup providers are excluded explicitly in OCT24_RENDER_D_FRAME.md.
class NativeMissionFrameCoordinator final {
public:
    NativeMissionFrameResult run(GameRenderFrameState& state,
        const NativeMissionFrameSwitches& switches, const NativeMissionFrameHooks& hooks);

private:
    std::uint32_t reported_{};
    bool running_{};
};

} // namespace bsp::game
