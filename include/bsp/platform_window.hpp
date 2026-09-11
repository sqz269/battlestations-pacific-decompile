#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdint>
#include <string>

#include "bsp/frame_clock.hpp"

// Platform phase of cSkeletonAppMidway::Init (0073d410), covering the object
// construction at 0073d899..0073d988 and the window configuration reached
// through platform vtable +4 at 0073dc25. Evidence: docs/APP_INIT_PLATFORM.md.
//
// Every structure here is a typed projection of a native object, not its ABI.
// Native field offsets are quoted per member so the projection stays auditable;
// none of these types are drop-in binary replacements. Window class registration
// and CreateWindowEx themselves already exist as the audited native fragment
// create_platform_window_00becee0_fragment in bsp/win32_window.hpp; this header
// adds the surrounding sequence and routes creation through an injected host so
// the whole routine can run without a real window.

namespace bsp {

// Projection of the 0x184-byte object allocated at 0073d8d1 and constructed by
// 00becda0. Only fields written by 00becda0 or 00becee0 appear; bytes whose
// role is not recovered keep an offset-derived name.
struct Win32PlatformState {
    const void* vtable{};        // +000, native &PTR_LAB_00d68cc4
    std::string name;            // +004 length / +008 data, native cNativeString
    bool fullscreen{};           // +00c
    bool widescreen{};           // +00d, active_aspect > widescreen_threshold
    float active_aspect{};       // +010
    bool exact_1280x720{};       // +014
    std::int32_t color_depth{};  // +018, see configure_platform_window_00becee0
    std::int32_t x{};            // +01c
    std::int32_t y{};            // +020
    std::int32_t present_width{};    // +024
    std::int32_t present_height{};   // +028
    bool settings_changed_2c{};      // +02c, mouse cooperative-mode refresh flag
    HWND window{};               // +030
    std::int32_t requested_width{};  // +034
    std::int32_t requested_height{}; // +038
    float desktop_aspect{};      // +03c
    bool byte_040{};             // +040, zeroed by 00becda0, role unrecovered
    bool byte_041{};             // +041, zeroed by 00becda0, role unrecovered
    bool frames_enabled{};       // +042, gates the frame slot in 00bec1a0
    bool loop_finished{};        // +043, set on loop exit by 00bec1a0
    bool byte_044{};             // +044, zeroed by 00becda0, role unrecovered
    void* application{};         // +048, the cSkeletonAppMidway object
    std::uint32_t power_scheme{};// +04c, GetActivePwrScheme output
    // +050..+0df original POWER_POLICY copy, +0e0..+16f modified copy.
    bool byte_170{};             // +170, zeroed by 00becda0, role unrecovered
    void* text_queue{};          // +178, sentinel from 00bec710
    void* text_queue_tail{};     // +17c
    bool close_requested{};      // +180, set by WM_CLOSE in 00bed3b0
    bool exit_requested{};       // +181, loop exit flag read by 00bec1a0
};

// 4/3, native float constant DAT_00d5bd98 read at 00bed129.
inline constexpr float widescreen_threshold_00d5bd98 = 1.3333334f;

// 00becda0, __fastcall(ECX=this), returns this, RET. Zero-initializes the state
// above and installs the text-queue sentinel produced by 00bec710. The caller
// supplies that sentinel; this routine never allocates it.
void construct_win32_platform_00becda0(Win32PlatformState& state,
    void* text_queue_sentinel) noexcept;

// The eleven native arguments of 00becee0 as assembled at 0073dbc7..0073dc25.
// Each member records the global the call site reads.
struct PlatformWindowRequest {
    const char* name{};              // arg 2, temporary "Battlestations Pacific"
    bool fullscreen{};               // arg 3, settings byte 00f8899e
    bool color_depth_selector{};     // arg 4, settings byte 00f889e0 != 0
    std::int32_t x{};                // arg 5, 00e1ae88
    std::int32_t y{};                // arg 6, 00e1ae8c
    std::int32_t width{};            // arg 7, settings dword 00f88994
    std::int32_t height{};           // arg 8, settings dword 00f88998
    std::uint32_t renderer_option{}; // arg 9, 00f889d8, forwarded to the renderer
    void* application{};             // arg 10, the application object
    HINSTANCE instance{};            // arg 11, 00e1ae7c, WinMain's HINSTANCE
    WNDPROC procedure{};             // native window thunk 00bec3b0
};

// Arguments 00becee0 assembles for the renderer singleton call at 00bed1b8
// (DAT_00f8d394 virtual +4). Recorded, never invoked: device creation belongs
// to the renderer packet.
struct RendererInitRequest {
    HWND window{};
    bool fullscreen{};
    std::int32_t width{};
    std::int32_t height{};
    std::uint32_t constant_15{0x15};
    std::uint32_t constant_1{1};
    std::uint32_t option{};
    std::uint32_t constant_4b{0x4b};
    bool color_depth_selector{};
    std::uint32_t constant_0{0};
    bool requested{false};
};

// Injected Win32 surface. One method per native call in 00becee0, in call
// order, so the routine runs against a recording double with no real window.
struct PlatformWindowHost {
    PlatformWindowHost() = default;
    PlatformWindowHost(const PlatformWindowHost&) = delete;
    PlatformWindowHost& operator=(const PlatformWindowHost&) = delete;
    virtual ~PlatformWindowHost() = default;
    // Native vtable +8 on the platform object, invoked only when an HWND exists.
    virtual void stop_existing_window(Win32PlatformState& state) = 0;
    virtual HCURSOR load_arrow_cursor() = 0;
    virtual ATOM register_class(const WNDCLASSA& window_class) = 0;
    virtual BOOL adjust_window_rect(RECT& rectangle, DWORD style, BOOL menu) = 0;
    virtual HWND create_window(DWORD extended_style, const char* class_name,
        const char* title, DWORD style, int x, int y, int width, int height,
        HINSTANCE instance, void* parameter) = 0;
    virtual void set_window_long(HWND window, int index, LONG value) = 0;
    virtual void set_window_pos(HWND window, HWND insert_after, int x, int y,
        int width, int height, UINT flags) = 0;
    virtual BOOL desktop_client_rect(RECT& rectangle) = 0;
    // GetDC + GetDeviceCaps(BITSPIXEL) + ReleaseDC on the created window.
    virtual int window_color_depth(HWND window) = 0;
    virtual void show_window(HWND window, int command) = 0;
};

// 00becee0, __cdecl with eleven stack arguments including the explicit platform
// pointer; the caller executes ADD ESP,2Ch and the callee RET. Reproduces the
// native body from 00becee9 up to the renderer call, then records the renderer
// arguments instead of issuing them. Returns false when class registration or
// window creation fails; the native code ignores those Win32 results, so a
// false return is a host addition and the state is left as far as it got.
//
// Not reproduced: the 0x14-byte allocation at 00bed1e2 (00bec870), the
// 00bec3e0(10000) call, the power-scheme save/override and the trailing
// SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, 0). Those touch machine-wide
// settings and stay out of the host.
bool configure_platform_window_00becee0(PlatformWindowHost& host,
    const PlatformWindowRequest& request, Win32PlatformState& state,
    RendererInitRequest& renderer) noexcept;

// Aspect helper isolated because the native code is x87: FILD/FIDIV in the
// current precision mode, FSTP to a float slot and FLD back, so the quotient is
// rounded to the intermediate precision and then to float. Reproduced with a
// double intermediate; see docs/APP_INIT_PLATFORM.md for the residual risk.
float aspect_ratio_x87(std::int32_t width, std::int32_t height) noexcept;

// 00beb2c0, __fastcall(ECX=this), returns this, RET. Object is 0x540 bytes
// (allocation at 0073d8f8); only its two trailing strings are recovered.
struct SaveStorageRoots {
    std::string save_directory;   // +530, <personal>\Battlestations-Pacific\save
    std::string search_pattern;   // +538, the same path plus "\*"
};

struct SaveStorageHost {
    SaveStorageHost() = default;
    SaveStorageHost(const SaveStorageHost&) = delete;
    SaveStorageHost& operator=(const SaveStorageHost&) = delete;
    virtual ~SaveStorageHost() = default;
    // SHGetSpecialFolderPathA(nullptr, buffer, folder, TRUE); folder is 5,
    // CSIDL_PERSONAL. False leaves the buffer contents unspecified, which the
    // native code also tolerates because it reads the stack buffer regardless.
    virtual bool special_folder_path(int folder, std::string& path) = 0;
    virtual bool create_directory(const std::string& path) = 0;
};

// Builds both strings and issues the two CreateDirectoryA calls in native order:
// the product directory first, then its save subdirectory. Returns false only
// when the special-folder query fails, in which case the native code would carry
// an unspecified buffer forward; the host stops instead.
bool initialize_save_storage_00beb2c0(SaveStorageHost& host,
    SaveStorageRoots& roots) noexcept;

// 00be2900, __fastcall(ECX=this), returns this, RET. 0xc-byte singleton
// published into DAT_0109cefc by base 00be2750. Read by the allocation report
// 00be3fd0, which prints field +8 as "Alloc at startup:%8dK".
struct AllocationStatsState {
    const void* vtable{};            // +0, native &PTR_FUN_00d685f4
    std::uint32_t budget{};          // +4, native constant 0x40000000
    std::uint32_t startup_allocated{}; // +8
};
void construct_allocation_stats_00be2900(AllocationStatsState& state) noexcept;

// Two out-of-line setters on the file/VFS manager singleton DAT_0109ceec,
// called at 0073d970 and 0073d983. Both are __thiscall with one stack argument
// and RET 4. Field roles are inferred from the call sites only.
struct FileManagerKnobs {
    const void* provider_policy{}; // +88, the 0x1c-byte singleton from 00736c30
    bool cached_load{};            // +78, the -cachedload switch, DAT_00e1ae76
};
void set_file_manager_provider_policy_00bd9230(FileManagerKnobs& knobs,
    const void* policy) noexcept;
void set_file_manager_cached_load_00bd9f90(FileManagerKnobs& knobs,
    bool cached_load) noexcept;

// 00bb40b0, __cdecl void(void), RET. Publishes one process-wide lock into
// DAT_010904e0, consumed by the case-insensitive registry at 00bb82f0/00bb83a0.
// The native lock block is 0x1c bytes: CRITICAL_SECTION plus a depth counter at
// +18 (00bd1860). Returns false when the native allocation would have failed.
struct SharedLock {
    CRITICAL_SECTION section{};
    std::int32_t depth{};
};
bool create_shared_lock_00bb40b0(SharedLock*& slot, SharedLock& storage) noexcept;

// 00bedfb0, __fastcall(ECX=this), returns this, RET. The 0x80-byte object
// allocated at 0073d48f is the frame clock already reconstructed in
// bsp/frame_clock.hpp: every field this constructor writes lands on a FrameClock
// member at the same offset, and base 00bede00 publishes it into DAT_01090ab0.
// The constructor leaves the two fixed-step fields at +70 and +78 untouched, so
// this routine does not write them either; it ends with the native tail call to
// 00bedbd0. Returns what initialize_frame_clock_00bedbd0 returns.
bool construct_frame_clock_singleton_00bedfb0(FrameClock& clock) noexcept;

} // namespace bsp
