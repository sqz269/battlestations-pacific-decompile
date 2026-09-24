#include "bsp/platform_activation.hpp"
#include "bsp/sound_system_owner.hpp"

namespace bsp {

void pause_sound_manager_00a7a480(SoundSystemOwner& owner) noexcept {
    if (owner.flag_50 == 0) {
        owner.flag_50 = 1;
        dirty_sound_classes_00a7a3f0(owner.levels, 0xffff);
    }
}

void resume_sound_manager_00a7a4a0(SoundSystemOwner& owner) noexcept {
    if (owner.flag_50 != 0) {
        owner.flag_50 = 0;
        dirty_sound_classes_00a7a3f0(owner.levels, 0xffff);
    }
}

bool handle_platform_activation_message_00bed3b0_fragment(
    PlatformActivationMessageHost& host, Win32PlatformState& active,
    HWND window, UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result) {
    if (message != WM_ACTIVATE) return false;

    auto& state = host.window_state(window); // BED3C5, captured ESI
    if (static_cast<std::uint16_t>(wparam) == 0) { // TEST BX,BX
        host.enable_xinput_00c2f172(false);
        // In this branch BL is necessarily zero, even with a nonzero HIWORD.
        const bool demote = state.fullscreen;
        state.byte_041 = false; // CMP fullscreen precedes the store at BED402
        if (demote) {
            host.set_window_pos(state.window, HWND_NOTOPMOST, 0, 0,
                state.present_width, state.present_height, SWP_FRAMECHANGED);
        }
        host.sound_pause_00a7a480();
        void* media = host.media_get_or_create_004c1710();
        host.media_pause_noop_00a4c2d0(media, 1);
        void* gui = host.gui_get_or_create_004c12b0();
        host.gui_pause_movie_descendants_00aa33a0(gui, 1);
    } else {
        host.enable_xinput_00c2f172(true);
        if (state.fullscreen) {
            host.set_window_pos(state.window, HWND_TOPMOST, 0, 0,
                state.present_width, state.present_height, SWP_FRAMECHANGED);
        }
        state.byte_040 = true;
        state.byte_041 = true;
        void* gui = host.gui_get_or_create_004c12b0();
        host.gui_pause_movie_descendants_00aa33a0(gui, 0);
        host.sound_resume_00a7a4a0();
        void* media = host.media_get_or_create_004c1710();
        host.media_pause_noop_00a4c2d0(media, 0);
        host.call_00b24fb0();
        host.set_foreground(active.window);
        host.set_focus(active.window); // reload after reentrant SetForegroundWindow
        if (active.fullscreen) {
            if (void* service = host.current_render_service_00f8d39c()) {
                host.render_service_refresh_focus_00b0d1e0(service);
            }
        }
    }
    result = host.default_message(window, message, wparam, lparam);
    return true;
}

} // namespace bsp
