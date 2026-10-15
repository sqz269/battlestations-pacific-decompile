#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstddef>
#include <cstdint>
#include <string>

#include "bsp/frame_clock.hpp"
#include "bsp/platform_loop.hpp"

// Platform phase of cSkeletonAppMidway::Init (0073d410), covering the object
// construction at 0073d899..0073d988 and the window configuration reached
// through platform vtable +4 at 0073dc25. Evidence: docs/APP_INIT_PLATFORM.md.
//
// The platform object is a typed projection, not its complete native ABI.
// Native field offsets are quoted per member so the projection stays auditable;
// none of these types are drop-in binary replacements. Window class registration
// and CreateWindowEx themselves already exist as the audited native fragment
// create_platform_window_00becee0_fragment in bsp/win32_window.hpp; this header
// adds the surrounding sequence and routes creation through an injected host so
// the whole routine can run without a real window.

namespace bsp {

// Plain native storage: no default member writes, constructors, destructor or
// host vtable. BECDA0 constructs the actual allocation before it is consumed.
// Standalone compatibility views value-initialize their own unregistered bytes.
struct NativePlatformWindowFocusStorage {
    const void* profile_00;
    std::uint32_t title_length_04;
    char* title_data_08;
    bool fullscreen_0c;
    bool widescreen_0d;
    std::byte untouched_0e[2];
    float active_aspect_10;
    bool exact_1280x720_14;
    std::byte untouched_15[3];
    std::int32_t color_depth_18;
    std::int32_t x_1c;
    std::int32_t y_20;
    std::int32_t present_width_24;
    std::int32_t present_height_28;
    bool settings_changed_2c;
    std::byte untouched_2d[3];
    HWND window_30;
    std::int32_t requested_width_34;
    std::int32_t requested_height_38;
    float desktop_aspect_3c;
    bool byte_040;
    bool active_41;
    bool frames_enabled_42;
    bool loop_finished_43;
};
struct NativePlatformObjectStorage {
    NativePlatformWindowFocusStorage window;
    bool clipboard_requested_44;
    std::byte untouched_45[3];
    void* application_48;
    std::uint32_t power_scheme_4c;
    std::byte power_policies_50[0x120];
    bool text_enabled_170;
    std::byte untouched_171[3];
    std::uint32_t queue_unused_174;
    void* queue_sentinel_178;
    std::uint32_t queue_count_17c;
    bool close_requested_180;
    bool exit_requested_181;
    std::byte untouched_182[2];
};
static_assert(sizeof(HWND) == 4);
static_assert(sizeof(NativePlatformWindowFocusStorage) == 0x44);
static_assert(offsetof(NativePlatformWindowFocusStorage, present_height_28) == 0x28);
static_assert(offsetof(NativePlatformWindowFocusStorage, window_30) == 0x30);
static_assert(offsetof(NativePlatformWindowFocusStorage, active_41) == 0x41);
static_assert(offsetof(NativePlatformWindowFocusStorage, frames_enabled_42) == 0x42);
static_assert(offsetof(NativePlatformObjectStorage, power_policies_50) == 0x50);
static_assert(offsetof(NativePlatformObjectStorage, text_enabled_170) == 0x170);
static_assert(offsetof(NativePlatformObjectStorage, queue_sentinel_178) == 0x178);
static_assert(offsetof(NativePlatformObjectStorage, queue_count_17c) == 0x17c);
static_assert(offsetof(NativePlatformObjectStorage, exit_requested_181) == 0x181);
static_assert(sizeof(NativePlatformObjectStorage) == 0x184);

struct NativeStringRawPoolContext;

// Borrow the native title header. Only a standalone compatibility projection
// owns a std::string; actual-owner writes use its same raw pool/manager cells.
// Destruction never returns the native title: BE2B10 owns that operation.
class PlatformWindowName final {
public:
    PlatformWindowName(NativePlatformWindowFocusStorage&, NativeStringRawPoolContext*) noexcept;
    void assign(const char*);
    void clear() { assign(""); }
    const char* c_str() const noexcept;
    std::size_t size() const noexcept;
    bool empty() const noexcept { return size() == 0; }
    operator std::string() const { return std::string(c_str(), size()); }
    void copy_projection_from(const PlatformWindowName&);
    void move_projection_from(PlatformWindowName&) noexcept;
private:
    void publish_projection_header() noexcept;
    NativePlatformWindowFocusStorage& header_;
    NativeStringRawPoolContext* strings_;
    std::string compatibility_name_;
};

class PlatformWindowStorageBinding {
protected:
    PlatformWindowStorageBinding() noexcept;
    PlatformWindowStorageBinding(NativePlatformObjectStorage&, NativeStringRawPoolContext&) noexcept;
    NativePlatformObjectStorage& storage() noexcept { return *storage_; }
    const NativePlatformObjectStorage& storage() const noexcept { return *storage_; }
    NativeStringRawPoolContext* native_strings() const noexcept { return strings_; }
private:
    // Unused/unregistered for actual-owner views; never another native owner.
    NativePlatformObjectStorage projection_storage_;
    NativePlatformObjectStorage* storage_;
    NativeStringRawPoolContext* strings_{};
};

// The C++ view has a new Source ABI. Every exposed scalar is a reference to
// its one backing cell; actual-owner views never initialize or copy raw bytes.
// Their source metadata must outlive every callback but stop reading fields
// before canonical native deletion frees the actual owner. Do not copy/move
// an actual-owner view. Standalone projection copies retain independent cells.
struct Win32PlatformFields : private PlatformWindowStorageBinding, PlatformLoopState {
    Win32PlatformFields() noexcept;
    Win32PlatformFields(NativePlatformObjectStorage&, NativeStringRawPoolContext&) noexcept;
    Win32PlatformFields(const Win32PlatformFields&);
    Win32PlatformFields(Win32PlatformFields&&) noexcept;
    Win32PlatformFields& operator=(const Win32PlatformFields&);
    Win32PlatformFields& operator=(Win32PlatformFields&&) noexcept;
    NativePlatformObjectStorage* actual_native_object() noexcept {
        return native_strings() ? &storage() : nullptr;
    }
    NativePlatformWindowFocusStorage& native_window_focus() noexcept { return storage().window; }
    const NativePlatformWindowFocusStorage& native_window_focus() const noexcept { return storage().window; }
    const void*& vtable{storage().window.profile_00};
    PlatformWindowName name{storage().window, native_strings()};
    bool& fullscreen{storage().window.fullscreen_0c};
    bool& widescreen{storage().window.widescreen_0d};
    float& active_aspect{storage().window.active_aspect_10};
    bool& exact_1280x720{storage().window.exact_1280x720_14};
    std::int32_t& color_depth{storage().window.color_depth_18};
    std::int32_t& x{storage().window.x_1c};
    std::int32_t& y{storage().window.y_20};
    std::int32_t& present_width{storage().window.present_width_24};
    std::int32_t& present_height{storage().window.present_height_28};
    bool& settings_changed_2c{storage().window.settings_changed_2c};
    HWND& window{storage().window.window_30};
    std::int32_t& requested_width{storage().window.requested_width_34};
    std::int32_t& requested_height{storage().window.requested_height_38};
    float& desktop_aspect{storage().window.desktop_aspect_3c};
    bool& byte_040{storage().window.byte_040};
    bool& byte_041{storage().window.active_41};
    bool& byte_044{storage().clipboard_requested_44};
    void*& application{storage().application_48};
    std::uint32_t& power_scheme{storage().power_scheme_4c};
    bool& byte_170{storage().text_enabled_170};
    void*& text_queue{storage().queue_sentinel_178};
    std::uint32_t& text_queue_count{storage().queue_count_17c};
    bool& close_requested{storage().close_requested_180};
};

struct Win32PlatformState : Win32PlatformFields {
    Win32PlatformState() = default;
    Win32PlatformState(NativePlatformObjectStorage& owner, NativeStringRawPoolContext& strings) noexcept
        : Win32PlatformFields(owner, strings) {}
    Win32PlatformState(const Win32PlatformState&) = default;
    Win32PlatformState(Win32PlatformState&&) noexcept = default;
    Win32PlatformState& operator=(const Win32PlatformState&) = default;
    Win32PlatformState& operator=(Win32PlatformState&&) noexcept = default;
};

struct PlatformFocusMessageHost {
    virtual ~PlatformFocusMessageHost() = default;
    virtual Win32PlatformState& window_state(HWND window) = 0; // GetWindowLongA(window, 0)
    virtual void store_window_state(HWND window, Win32PlatformState& state) = 0; // SetWindowLongA
    virtual void set_window_pos(HWND window, HWND insert_after, int x, int y,
        int width, int height, UINT flags) = 0;
    virtual void set_focus(HWND window) = 0;
    virtual void set_foreground(HWND window) = 0;
    virtual LRESULT default_message(HWND window, UINT message, WPARAM wparam, LPARAM lparam) = 0;
};
// Returns false without effects for messages outside this fragment. WM_SIZE
// preserves both receiver identities and reloads the active HWND after SetFocus.
// WM_ACTIVATE (audio/media/GUI/renderer calls) remains outside this fragment.
bool handle_platform_focus_message_00bed3b0_fragment(PlatformFocusMessageHost& host,
    Win32PlatformState& active, HWND window, UINT message, WPARAM wparam,
    LPARAM lparam, LRESULT& result) noexcept;

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
