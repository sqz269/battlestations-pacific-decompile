#include "bsp/sound_frame_service.hpp"
#include "bsp/game_sound_runtime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <array>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
void initialize_listener(std::uint32_t one, CameraMatrix& matrix,
    std::array<float, 3>& velocity) noexcept {
    // Native MOVSS copies raw bits, including any noncanonical live one word.
    for (std::size_t i = 0; i < matrix.size(); ++i) {
        const std::uint32_t bits = i % 5 == 0 ? one : 0;
        std::memcpy(&matrix[i], &bits, sizeof bits);
    }
    const std::uint32_t zero{};
    for (auto& component : velocity) std::memcpy(&component, &zero, sizeof zero);
}
void update_current_sound(const CameraMatrix& matrix, std::array<float, 3> velocity,
    SoundFrameServiceContext& context) {
    auto* const sound = context.sound_00f8bbd8;
    if (!sound) throw std::logic_error("Sound frame service requires the current sound manager");
    // Native dispatch reads the current profile AFTER listener callbacks.
    switch (sound->native_vtable_00) {
    case 0x00d5b44c:
        update_sound_system_00a87bf0(*sound, matrix, velocity, context.update);
        return;
    case 0x00d5b000:
        update_sound_system_base_00a7e630(*sound, matrix, velocity, context.update);
        return;
    default:
        throw std::logic_error("Unreconstructed sound frame update profile");
    }
}
void write_byte(void* receiver, std::size_t offset, std::uint8_t value) noexcept {
    static_cast<unsigned char*>(receiver)[offset] = value;
}
} // namespace

SoundFrameServiceContext bind_game_sound_frame_service(game::GameSoundRuntime& core,
    void* volatile& actual_interface, InterfaceSoundListenerContext& listener) {
    return {actual_interface, listener, core.lifetime_bindings().global_00f8bbd8,
        core.update_context()};
}

void service_sound_frame_00735b50(SoundFrameServiceContext& context) {
    const auto one = context.listener.one_00d7a24c;
    auto* const actual_interface = context.interface_00e198c4;
    CameraMatrix matrix;
    std::array<float, 3> velocity;
    initialize_listener(one, matrix, velocity);
    if (actual_interface)
        get_interface_sound_listener_0068a670(actual_interface, matrix.data(),
            velocity.data(), context.listener);
    update_current_sound(matrix, velocity, context);
}

void update_published_sound_with_identity_listener(SoundFrameServiceContext& context) {
    // 004DA873..004DA905 / 004DA907..004DA999: XORPS zero and the one D7A24C
    // load fill the identity matrix and zero velocity, then [00F8BBD8] is
    // loaded and its vtable[4] called; no listener provider is consulted.
    CameraMatrix matrix;
    std::array<float, 3> velocity;
    initialize_listener(context.listener.one_00d7a24c, matrix, velocity);
    update_current_sound(matrix, velocity, context);
}

NullInterfaceSoundFrame::NullInterfaceSoundFrame(game::GameSoundRuntime& core,
    const CameraAxesCrtAccess& crt)
    : listener_{game_00e188a8_, controlled_00e188d8_, one_00d7a24c_, threshold_00ce3d64_,
          compare_00d7a218_, scale_00d7a220_, fallback_00f87574_, *this, crt},
      frame_(bind_game_sound_frame_service(core, interface_00e198c4_, listener_)) {}

const void* NullInterfaceSoundFrame::call_slot120(void*, void*) {
    throw std::logic_error("null-interface sound frame reached 0068A670 slot 120h");
}
const void* NullInterfaceSoundFrame::call_slot34(void*, void*) {
    throw std::logic_error("null-interface sound frame reached 0068A670 slot 34h");
}

void run_sound_frame_job_004bbd00(std::uint32_t, SoundFrameServiceContext& context) {
    const auto one = context.listener.one_00d7a24c;
    auto* const actual_interface = context.interface_00e198c4;
    CameraMatrix matrix;
    std::array<float, 3> velocity;
    initialize_listener(one, matrix, velocity);
    get_interface_sound_listener_0068a670(actual_interface, matrix.data(),
        velocity.data(), context.listener);
    update_current_sound(matrix, velocity, context);
}

void prepare_game_sound_transition_004cd610(void* actual_game,
    std::uint8_t change_input, SoundLoadTransitionContext& context) {
    write_byte(actual_game, 0x7184, 1);
    auto* const screen = context.movie_screen_00e18d48;
    write_byte(screen, 4, 1);
    write_byte(screen, 5, 1);
    context.host.commit_screen_visibility_004f83b0(screen);
    context.host.enter_screen_slot18(screen); // same captured ESI, not a global reload
    if (change_input) {
        void* const input = context.host.get_input_manager_004bec00();
        context.host.set_input_context_00a933f0(input, 0x10, 5);
    }
    context.host.set_cinematic_mode_004cd0f0(actual_game, 1, 0, 1);
    auto* const sleep = &Sleep; // native captures CE2230 into EDI once before loop
    CameraMatrix matrix; // native stack locals never initialized by004CD610
    std::array<float, 3> velocity;
    for (unsigned remaining = 5; remaining != 0; --remaining) {
        get_interface_sound_listener_0068a670(context.frame.interface_00e198c4,
            matrix.data(), velocity.data(), context.frame.listener);
        update_current_sound(matrix, velocity, context.frame);
        sleep(10);
    }
}
} // namespace bsp
