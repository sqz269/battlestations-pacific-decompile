#include "bsp/platform_window.hpp"

#include <cstddef>

namespace bsp {
namespace {

// 00becee0 keeps the class name and the window title in one string: the same
// pointer reaches lpszClassName, lpClassName and lpWindowName. A null string
// data pointer falls back to the shared empty byte DAT_0109db8d.
const char* class_and_title(const char* name) noexcept {
    return name ? name : "";
}

// The native subtractions at 00bed0c5/00bed0d1 are plain 32-bit wraparound.
std::int32_t rect_extent(LONG high, LONG low) noexcept {
    const auto bits = static_cast<std::uint32_t>(high) - static_cast<std::uint32_t>(low);
    return static_cast<std::int32_t>(bits);
}

} // namespace

void construct_win32_platform_00becda0(Win32PlatformState& state,
    void* text_queue_sentinel) noexcept {
    // Base 00be2ac0 runs first and publishes the object into DAT_0109cf04; that
    // registration is not modelled here. The vtable pointer is recorded as an
    // address-shaped value only, never dereferenced.
    state.vtable = nullptr;
    state.name.clear();
    state.fullscreen = false;
    state.widescreen = false;
    state.active_aspect = 0.0f;
    state.exact_1280x720 = false;
    state.color_depth = 0;
    state.x = 0;
    state.y = 0;
    state.present_width = 0;
    state.present_height = 0;
    state.window = nullptr;
    state.requested_width = 0;
    state.requested_height = 0;
    state.desktop_aspect = 0.0f;
    state.byte_040 = false;
    state.byte_041 = false;
    state.frames_enabled = false;
    state.loop_finished = false;
    state.byte_044 = false;
    state.application = nullptr;
    state.power_scheme = 0;
    state.byte_170 = false;
    state.text_queue = text_queue_sentinel;
    state.text_queue_tail = nullptr;
    state.close_requested = false;
    state.exit_requested = false;
}

float aspect_ratio_x87(std::int32_t width, std::int32_t height) noexcept {
    // FILD width; FIDIV height computes in the x87 intermediate precision, then
    // FSTP/FLD on a float slot rounds the result to float before it is stored
    // and compared. A double intermediate reproduces the default 53-bit control
    // word; a single-rounding float divide would not.
    const double quotient = static_cast<double>(width) / static_cast<double>(height);
    return static_cast<float>(quotient);
}

bool configure_platform_window_00becee0(PlatformWindowHost& host,
    const PlatformWindowRequest& request, Win32PlatformState& state,
    RendererInitRequest& renderer) noexcept {
    // 00becee9..00becf07: requested size and application land before anything
    // else, and an existing window is stopped through vtable +8.
    state.requested_width = request.width;
    state.requested_height = request.height;
    state.application = request.application;
    if (state.window) {
        host.stop_existing_window(state);
    }

    const char* const name = class_and_title(request.name);

    // 00becf29..00becf9d: class style 0x2000, 24 window-extra bytes, arrow
    // cursor. MOV dword ptr [ESP+0x38],0x2000 at 00becf36 writes the style
    // member of the WNDCLASSA that 00becf83 passes to RegisterClassA, and
    // 0x2000 is CS_BYTEALIGNWINDOW; CS_GLOBALCLASS would be 0x4000.
    WNDCLASSA window_class{};
    window_class.style = CS_BYTEALIGNWINDOW;
    window_class.lpfnWndProc = request.procedure;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 24;
    window_class.hInstance = request.instance;
    window_class.hIcon = nullptr;
    window_class.hCursor = host.load_arrow_cursor();
    window_class.hbrBackground = nullptr;
    window_class.lpszMenuName = nullptr;
    window_class.lpszClassName = name;
    if (!host.register_class(window_class)) {
        return false;
    }

    // 00becfa4..00becfc6: the requested size is treated as a client rectangle
    // and grown for WS_CAPTION only. Extended styles are applied afterwards and
    // the rectangle is never recomputed for them.
    RECT rectangle{0, 0, request.width, request.height};
    host.adjust_window_rect(rectangle, WS_CAPTION, FALSE);
    const std::int32_t outer_width = rect_extent(rectangle.right, rectangle.left);
    const std::int32_t outer_height = rect_extent(rectangle.bottom, rectangle.top);

    state.window = host.create_window(0, name, name, WS_CAPTION, request.x, request.y,
        outer_width, outer_height, request.instance, &state);
    if (!state.window) {
        return false;
    }

    // 00bed028..00bed07d: fullscreen takes the unadjusted requested size at 0,0
    // with HWND_TOPMOST; windowed keeps the adjusted size at the same position.
    if (request.fullscreen) {
        host.set_window_long(state.window, GWL_STYLE, static_cast<LONG>(WS_POPUP));
        host.set_window_long(state.window, GWL_EXSTYLE, WS_EX_TOPMOST);
        host.set_window_pos(state.window, HWND_TOPMOST, 0, 0,
            request.width, request.height, SWP_FRAMECHANGED);
    } else {
        host.set_window_long(state.window, GWL_STYLE, WS_CAPTION);
        host.set_window_long(state.window, GWL_EXSTYLE,
            WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE);
        host.set_window_pos(state.window, nullptr, 0, 0,
            outer_width, outer_height, SWP_FRAMECHANGED);
    }

    // 00bed088..00bed0b3: the title argument is copied into the member string.
    state.name.assign(name);

    // 00bed0b3..00bed149: desktop aspect always, active aspect from the desktop
    // in fullscreen and from the requested size otherwise.
    RECT desktop{0, 0, 0, 0};
    host.desktop_client_rect(desktop);
    const float desktop_aspect = aspect_ratio_x87(
        rect_extent(desktop.right, desktop.left),
        rect_extent(desktop.bottom, desktop.top));
    state.present_width = request.width;
    state.present_height = request.height;
    state.fullscreen = request.fullscreen;
    state.desktop_aspect = desktop_aspect;
    state.active_aspect = request.fullscreen
        ? desktop_aspect
        : aspect_ratio_x87(request.width, request.height);
    state.widescreen = state.active_aspect > widescreen_threshold_00d5bd98;
    state.exact_1280x720 = request.width == 0x500 && request.height == 0x2d0;

    // 00bed15c..00bed1a5: windowed reads the real colour depth and keeps the
    // requested position; fullscreen substitutes the settings selector and 0,0.
    if (request.fullscreen) {
        state.color_depth = request.color_depth_selector ? 1 : 0;
        state.x = 0;
        state.y = 0;
    } else {
        state.color_depth = host.window_color_depth(state.window);
        state.x = request.x;
        state.y = request.y;
    }

    // 00bed1a7..00bed1b8: frames are enabled before the window becomes visible.
    state.frames_enabled = true;
    host.show_window(state.window, SW_SHOWNORMAL);

    renderer.window = state.window;
    renderer.fullscreen = request.fullscreen;
    renderer.width = state.present_width;
    renderer.height = state.present_height;
    renderer.option = request.renderer_option;
    renderer.color_depth_selector = request.color_depth_selector;
    renderer.requested = true;
    return true;
}

bool initialize_save_storage_00beb2c0(SaveStorageHost& host,
    SaveStorageRoots& roots) noexcept {
    roots.save_directory.clear();
    roots.search_pattern.clear();

    std::string personal;
    if (!host.special_folder_path(5, personal)) {
        return false;
    }
    // 00beb3bf: the special-folder path is the left operand, so the product
    // directory is <personal> + "\Battlestations-Pacific".
    std::string path = personal;
    path += "\\Battlestations-Pacific";
    host.create_directory(path);

    // 00beb440..00beb4b5: "\save" is appended in place and the same string is
    // used for the second CreateDirectoryA.
    path += "\\save";
    host.create_directory(path);

    // 00beb4bf..00beb538: the member at +530 receives that path, and the member
    // at +538 is that member concatenated with "\*".
    roots.save_directory = path;
    roots.search_pattern = roots.save_directory;
    roots.search_pattern += "\\*";
    return true;
}

void construct_allocation_stats_00be2900(AllocationStatsState& state) noexcept {
    // Base 00be2750 publishes the object into DAT_0109cefc first.
    state.vtable = nullptr;
    state.budget = 0x40000000u;
    state.startup_allocated = 0;
}

void set_file_manager_provider_policy_00bd9230(FileManagerKnobs& knobs,
    const void* policy) noexcept {
    knobs.provider_policy = policy;
}

void set_file_manager_cached_load_00bd9f90(FileManagerKnobs& knobs,
    bool cached_load) noexcept {
    knobs.cached_load = cached_load;
}

bool create_shared_lock_00bb40b0(SharedLock*& slot, SharedLock& storage) noexcept {
    // 00bd1860 allocates 0x1c bytes, calls InitializeCriticalSection and zeroes
    // the depth counter at +18. The caller owns the storage here; the native
    // routine never frees it either.
    InitializeCriticalSection(&storage.section);
    storage.depth = 0;
    slot = &storage;
    return true;
}

bool construct_frame_clock_singleton_00bedfb0(FrameClock& clock) noexcept {
    // 00bedfd2..00be0043 in native order. Each timestamp frequency is set to 1
    // and every counter to zero; +70 and +78 are deliberately left alone.
    clock.accumulated = 0.0f;
    clock.updates = 0;
    clock.start = ClockTimestamp{0, 1};
    clock.current = ClockTimestamp{0, 1};
    clock.previous = ClockTimestamp{0, 1};
    clock.interval = ClockTimestamp{0, 1};
    clock.pause_snapshot = ClockTimestamp{0, 1};
    clock.paused = false;
    clock.fixed_counter = false;
    return initialize_frame_clock_00bedbd0(clock);
}

} // namespace bsp
