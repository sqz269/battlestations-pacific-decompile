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

// The listener-free update the session teardown 004DA780 makes twice
// (004DA873..004DA905 and 004DA907..004DA999): 00735B50's identity matrix and
// zero velocity from the live D7A24C word, then the current [00F8BBD8]
// vtable[4] dispatch. It never reads the interface publication. Not a native
// entry of its own; both sites are inline in 004DA780.
void update_published_sound_with_identity_listener(SoundFrameServiceContext&);

// A SoundFrameServiceContext whose [00E198C4] publication is null, which is
// the native state outside a mission: the word is BSS until 0068CC70
// publishes it (0068CCFE), and the teardowns that end a mission delete the
// manager and store null (004DAB94 in 004DA780, 004DA6B3 in 004DA650). With
// that word 00735B50 skips 0068A670, so this binding's game/controlled
// publications are never read and its 0068A670 dispatches throw if reached.
// The constants are the PE words 0068A670 reads (bsp.py ghidra bytes):
// 00D7A24C 3F800000h, 00CE3D64 461C4000h (10000.0f), 00D7A218 0.0f,
// 00D7A220 4059000000000000h (100.0); 00F87574 is BSS (three zero words).
// Borrows the started core and CRT access for its whole lifetime.
class NullInterfaceSoundFrame final : public InterfaceSoundListenerCalls {
public:
    NullInterfaceSoundFrame(game::GameSoundRuntime& core, const CameraAxesCrtAccess& crt);
    NullInterfaceSoundFrame(const NullInterfaceSoundFrame&) = delete;
    NullInterfaceSoundFrame& operator=(const NullInterfaceSoundFrame&) = delete;
    SoundFrameServiceContext& context() noexcept { return frame_; }
    const void* call_slot120(void* actual_object, void* scratch64) override;
    const void* call_slot34(void* actual_object, void* scratch12) override;

private:
    void* volatile interface_00e198c4_{nullptr};
    void* volatile game_00e188a8_{nullptr};
    void* volatile controlled_00e188d8_{nullptr};
    const volatile std::uint32_t one_00d7a24c_{0x3f800000u};
    const volatile float threshold_00ce3d64_{10000.0f};
    const volatile float compare_00d7a218_{0.0f};
    const volatile double scale_00d7a220_{100.0};
    const volatile float fallback_00f87574_[3]{0.0f, 0.0f, 0.0f};
    InterfaceSoundListenerContext listener_;
    SoundFrameServiceContext frame_;
};

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
