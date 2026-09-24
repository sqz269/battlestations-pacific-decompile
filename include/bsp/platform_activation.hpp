#pragma once

#include "bsp/platform_window.hpp"
#include <cstdint>

namespace bsp {

struct SoundSystemOwner;
// Production typed owner adapters for the complete A7A480/A7A4A0 bodies.
// These update the canonical owner flag and its existing levels collection;
// SoundSystemOwner is not reinterpreted as the native 178h object.
void pause_sound_manager_00a7a480(SoundSystemOwner&) noexcept;
void resume_sound_manager_00a7a4a0(SoundSystemOwner&) noexcept;

// Call boundaries for the WM_ACTIVATE arm only (BED3F3..BED507) of BED3B0.
// All providers are required. Each method with a callee address represents one
// native call. Publication reads belong to the stated site, not construction
// of this host: earlier calls can replace singleton owners by re-entry.
struct PlatformActivationMessageHost : PlatformFocusMessageHost {
    virtual void enable_xinput_00c2f172(bool enabled) = 0;
    virtual void sound_pause_00a7a480() = 0;  // ECX = current F8BBD8
    virtual void sound_resume_00a7a4a0() = 0; // ECX = current F8BBD8
    virtual void* media_get_or_create_004c1710() = 0;
    // Native A4C2D0 is exactly RET4, with no receiver/argument read. Its
    // preceding getter still executes. The explicit receiver records EAX->ECX.
    virtual void media_pause_noop_00a4c2d0(void* receiver, std::uint32_t pause) = 0;
    virtual void* gui_get_or_create_004c12b0() = 0;
    virtual void gui_pause_movie_descendants_00aa33a0(
        void* receiver, std::uint32_t pause) = 0;
    // ECX = current F8D394. Body is gated by 108D4BB, calls B22030 at
    // receiver+1A98, receiver virtual+120, then B21F70 at receiver+1A74.
    // This packet leaves that renderer-specific contract external.
    virtual void call_00b24fb0() = 0;
    // This read is not a native call; it occurs only after active.fullscreen
    // is reread following SetFocus. Null suppresses the B0D1E0 call.
    virtual void* current_render_service_00f8d39c() = 0;
    virtual void render_service_refresh_focus_00b0d1e0(void* receiver) = 0;
};

// Partial projection of BED3B0: complete WM_ACTIVATE arm and its common
// DefWindowProc continuation only. Other messages return false without effects.
// Native takes five stack slots (explicit receiver, HWND, message, WPARAM,
// LPARAM), RET14h. This typed interface does not reproduce that ABI or FH3.
// The captured window-extra receiver and explicit active receiver are distinct.
bool handle_platform_activation_message_00bed3b0_fragment(
    PlatformActivationMessageHost& host, Win32PlatformState& active,
    HWND window, UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result);

} // namespace bsp
