#pragma once

#include "bsp/interface_sound_listener.hpp"
#include "bsp/sound_system_update.hpp"
#include <cstdint>

namespace bsp {
namespace game { class GameSoundRuntime; }

// Borrow the actual interface publication, actual game/camera/unit bindings in
// listener, and the existing canonical sound owner/update services. No semantic
// InGameInterfaceManager may be passed as the actual native interface receiver.
// The sound publication is reloaded AFTER the listener call on every pass.
struct SoundFrameServiceContext {
    void* volatile& interface_00e198c4;
    InterfaceSoundListenerContext& listener;
    SoundSystemOwner* volatile& sound_00f8bbd8;
    SoundSystemUpdateContext& update;
};

// Pure composition: borrows core's existing publication and update context,
// which supplies its same FrameClock through BEE050/current+20. These callers
// never advance the clock, synthesize dt, or construct another sound manager.
SoundFrameServiceContext bind_game_sound_frame_service(game::GameSoundRuntime&,
    void* volatile& actual_interface_00e198c4, InterfaceSoundListenerContext&);

// 00735B50..00735C20 inclusive, complete. Native has no effective inputs, RET;
// callers may load ECX, which this body ignores. Identity/+zero are initialized
// from the one live D7A24C word. ONLY this entry skips listener on null interface.
void service_sound_frame_00735b50(SoundFrameServiceContext&);

// 004BBD00..004BBDCF inclusive, complete. ECX job object unused, one UNUSED
// STACK DWORD, RET4. The decompiler's EDX input is spurious; 68A670 ignores it.
// Same initialized local buffers, but an unconditional native listener call.
void run_sound_frame_job_004bbd00(std::uint32_t unused_job_argument,
    SoundFrameServiceContext&);

// Required non-sound operations of 004CD610, whose callee bodies were inspected.
// Actual receivers retain their CURRENT native virtual dispatch. Implementers
// must provide those operations; no generic scene/input substitute is supplied.
class SoundLoadTransitionHost {
public:
    virtual ~SoundLoadTransitionHost() = default;
    // ECX captured raw screen, RET. Enumerate slot24 children and apply its
    // current+5 byte via child slot34; retain existing allocation/EH behavior.
    virtual void commit_screen_visibility_004f83b0(void* actual_screen) = 0;
    // ECX SAME captured screen, current vtable+18, no arguments/RET. The
    // constructed CEAE94 movie profile points to the one-byte RET at004F8940.
    virtual void enter_screen_slot18(void* actual_screen) = 0;
    // No arguments/RET, EAX actual24h input manager from F8BBF8; lazy creation
    // and shared singleton registration belong to the existing input provider.
    virtual void* get_input_manager_004bec00() = 0;
    // ECX actual24h input, stack index/value, RET8: write vector+10[index],
    // recompute unsigned maximum+20 starting1, then A93020's 30h-record walk.
    virtual void set_input_context_00a933f0(void* actual_input,
        std::int32_t index, std::uint32_t value) = 0;
    // ECX raw game, three stack-byte values, RET0C. Existing cinematic-mode
    // body includes sound-class and input effects; all three bytes are kept.
    virtual void set_cinematic_mode_004cd0f0(void* actual_game,
        std::uint8_t first, std::uint8_t second, std::uint8_t third) = 0;
};

struct SoundLoadTransitionContext {
    void* volatile& movie_screen_00e18d48;
    SoundLoadTransitionHost& host;
    SoundFrameServiceContext& frame;
};

// 004CD610..004CD6CC inclusive, complete control flow through explicit required
// dependencies. ECX actual game (at least +7184), stack byte, RET4. Publishes
// +7184 and captured movie-screen+4/+5, commits/enters, optionally changes input,
// sets cinematic(1,0,1), then five listener/update/Sleep(10) passes. The two
// output buffers are initially UNWRITTEN and reused; no null-interface arm.
// 004CD66D..66F is unreachable LEA ECX,[ECX] alignment, not an omitted branch.
// These C++ entries add contexts and are not original ABI replacements.
void prepare_game_sound_transition_004cd610(void* actual_game,
    std::uint8_t change_input, SoundLoadTransitionContext&);

} // namespace bsp
