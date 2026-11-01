#pragma once
#include <memory>
#include <cstdint>
#include <exception>

namespace bsp {
class NativeRenderActualOwners;
struct NativeStringRawPoolContext;
struct NativePlatformFocusOwnersContext;
struct NativeGuiPageLifetimeHost;
class NativeGuiSelectedManagerDeleteAttempt;
}

namespace bsp::game {
class GameHostLog;
class GameSingletonHost;
class GameNativeReadOnlyData;
class GameFrontendHost;
class GameNativeGuiPageApplication;
class GameNativeGuiResourceApplication;

// cc15-gui4 P1, GUI Step 4: the warm actual 88h GUI manager. Enabled.
// ON builds the image's manager through the complete raw 004C12B0 getter
// (constructor 00AA5D70, vtable 00D5BFCC) at 0073BC19, its first call inside
// 0073BAE0, publishes it in this application's 00F8BC5C cell, registers it with
// the shared 01090AA0 manager through 00BD0C30, and binds the native_gui
// deletion arm so the shared raw drain runs 00AA6540 -> 00AA6340.
//
// Unbound owner split while the sprite bridge still draws: the bridge's
// GameFrontendHost ManagerState stays the 004C12B0 stand-in for every hosted
// caller (resource slots, ready flag, pointer slot). The native block owns its
// constructor state (tree +0C, page vector +14, string list +34/+38, cursor
// +5C/+60, pointer byte +48, +6C); nothing reads it until the Page chain (P3)
// borrows context(). Its resource slots and page vector stay empty, so the
// drain's +28/+2C releases are null-slot no-ops and no page virtual runs.
// A separately cold once-bound Source141 frontend/Page tuple can transport
// actual resources instead. It changes no default selector/Host activation;
// private sequence history grants no Native birth or whole-lifetime proof.
//
// 00F8BC5C reader census (2026-10-28, for P3 to re-check before it goes ON):
// image absolute-dword hits are only in 004C12B0 (5), 00736250 (5), 00AA0FF0,
// 00AA1F70 and 00AA6340. Every other reader goes through 004C12B0 (307 call
// sites in 192 functions). In bsp_game every hosted 004C12B0 call resolves to
// gui_manager_get_or_create_004c12b0 (game_hosts_frontend.cpp), which returns
// the bridge ManagerState unless Source141 is explicitly bound; the reconstructed callers' host
// hooks have no production implementation; Lua IsGUIActive (008CA010) uses
// the bridge's widget lookup. No cc9 file reads the cell or the stand-in.
inline constexpr bool kGuiManagerCc15Bound = true;

class GameNativeGuiManagerApplication {
public:
    // Pure host binding custody, not a native retain. Declare the provider
    // before this token so reverse destruction drops the token first. Both
    // survive the shared drain and after_singleton_drain(). Dropping the token
    // before that acknowledgement or destroying its manager first terminates.
    // The caller must retain the provider object for the whole token lifetime.
    class PageLifetimeBinding final {
    public:
        ~PageLifetimeBinding() noexcept;
        PageLifetimeBinding(const PageLifetimeBinding&) = delete;
        PageLifetimeBinding& operator=(const PageLifetimeBinding&) = delete;
    private:
        friend class GameNativeGuiManagerApplication;
        explicit PageLifetimeBinding(GameNativeGuiManagerApplication& owner) noexcept : owner_(&owner) {}
        GameNativeGuiManagerApplication* owner_;
    };

    GameNativeGuiManagerApplication(GameHostLog&, GameSingletonHost&, GameNativeReadOnlyData&,
        NativeRenderActualOwners&, NativeStringRawPoolContext&);
    ~GameNativeGuiManagerApplication();
    GameNativeGuiManagerApplication(const GameNativeGuiManagerApplication&) = delete;
    GameNativeGuiManagerApplication& operator=(const GameNativeGuiManagerApplication&) = delete;

    // 0073BC19: the raw getter. The first call constructs, publishes and
    // registers; later calls take its fast arm.
    void* get_004c12b0();
    // The same actual cells for later borrowers; valid until after the drain.
    NativePlatformFocusOwnersContext& context() noexcept;
    // Bind once, after the manager getter returns and before its page vector
    // has allocated storage. The borrowed provider implements the two CURRENT
    // page virtual calls separately. Unbound calls retain their rejection.
    // No page load, native call, table replacement or ownership transfer occurs.
    std::unique_ptr<PageLifetimeBinding> bind_page_lifetime_host(NativeGuiPageLifetimeHost&);
    // Explicit cold metadata preparation only; no default caller or Native effect.
    void prepare_cold_selected_manager_delete();
    // Reuse that SAME cold attempt after the Page's Font services are prepared.
    void bind_page_font_retirement(GameNativeGuiPageApplication&);
    bool requires_process_retention() const noexcept;
    // After the shared raw drain: the drain's 00AA0FF0 must have cleared the cell.
    void after_singleton_drain();

private:
    friend class GameFrontendHost;
    friend class GameNativeGuiResourceApplication;
    // Private Source transport history, never public birth/readiness authority.
    // Only the reached frontend/resource operations may write these known slots.
    void bind_frontend_transport(GameFrontendHost&, GameNativeGuiPageApplication&,
        GameNativeGuiResourceApplication&, NativeStringRawPoolContext&);
    void* begin_initial_resources(GameFrontendHost&);
    void admit_initial_texture(GameNativeGuiResourceApplication&, void*, std::uint16_t);
    void complete_initial_texture(void*, std::uint16_t, void*, bool publish) noexcept;
    void store_initial_widget(GameFrontendHost&, void*, std::uint16_t, void*);
    void finish_initial_resources(GameFrontendHost&, void*);
    void admit_pointer_operation(GameFrontendHost&, void*);
    static void store_enabled_48(void*, std::uint8_t) noexcept;
    static void* current_resource_slot(void*, std::uint16_t) noexcept;
    void fail_frontend_transport(std::exception_ptr) noexcept;
    void release_page_lifetime_binding(PageLifetimeBinding&) noexcept;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
