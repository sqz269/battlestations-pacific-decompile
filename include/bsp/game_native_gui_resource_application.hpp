#pragma once
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_render_context.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <string_view>

namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameVfsHost;
class GameNativeGuiManagerApplication;
class GameFrontendHost;

// One retained initial manager texture invocation. The result is a borrowed
// observation after reference_owned becomes false; it must never be replayed.
// Failed native/string/cache frames require process retention, not rollback.
struct GameNativeGuiResourceLoadOperation final {
    enum class Phase { entered, resolving, loading, publishing, complete, failed };
    Phase phase{Phase::entered};
    std::uint16_t manager_offset{};
    std::uint32_t native_site{};
    std::string requested_path;
    NativeString name;
    bool name_live{}, resolved{}, reference_owned{}, release_started{};
    GameNativeGuiManagerApplication* manager_owner{};
    void* actual_manager{};
    bool transfer_pending{}, manager_publication_entered{}, manager_transferred{}, publication_skipped{};
    bool old_slot_release_entered{}, old_slot_release_returned{};
    NativeTextureNameResolutionCall resolution;
    NativeTextureCacheAcquired cache;
    NativeRenderActualOwnerReleaseObservation release;
    void* result{};
    std::exception_ptr failure;
};

// Legacy entry owns the two initial AA5E20 texture references; projected
// manager slots borrow them. The separately bound actual entry transfers each
// returned credit to the actual manager once, before cleanup/observation.
// No page/GUI singleton, texture pool, cache, COM owner or reference counter is
// created here. Original AA5E20: ECX manager, RET; renderer64/B319B0: ECX renderer,
// native string* and zero callback word on stack, RET8. This is a C++ binding,
// not an original ABI entry or the complete AA5E20 manager/destructor.
// Its independent temporary-string scopes and retained failure frames are not
// the original enclosing AA5E20 stack/SEH lifetime.
//
// Bind before initial GUI startup and call in native
// whiteGui then transparent order. No reload/replacement or interrupted replay.
// Keep the SAME ready renderer/VFS/string/actual-owner domains alive through
// explicit retirement. End legacy borrowers before retire_textures(); actual
// manager-transferred references remain solely with its real AA6340 drain.
// Serialize with callbacks, frames, reset/recreation and renderer drain.
class GameNativeGuiResourceApplication final {
public:
    // Read BSP_NATIVE_GUI_RESOURCES once. Only the value "1" enables this
    // runtime route; absent/other values leave frontend binding inert.
    static bool requested() noexcept;
    GameNativeGuiResourceApplication(GameHostLog&, GameNativeRendererApplication&, GameVfsHost&);
    ~GameNativeGuiResourceApplication() noexcept;
    GameNativeGuiResourceApplication(const GameNativeGuiResourceApplication&) = delete;
    GameNativeGuiResourceApplication& operator=(const GameNativeGuiResourceApplication&) = delete;

    void* load_manager_texture_00aa5e20(std::uint16_t offset, std::string_view name);
    // Actual route: publish/hand off the exact returned credit before name
    // cleanup/logging. White VFS false skips +28; accepted null stores null.
    // Transparent releases CURRENT old+2C first. Entered failure cannot replay.
    void* load_initial_texture_into_actual_manager(GameNativeGuiManagerApplication&,
        void* captured_actual_manager, std::uint16_t offset, std::string_view name);
    // One attempt, clearing ownership before each actual+04 decrement/terminal.
    // Any interrupted retirement is retained and cannot be retried.
    void retire_textures();
    const GameNativeGuiResourceLoadOperation& operation(std::size_t) const;
    std::size_t operation_count() const noexcept;
    bool requires_process_retention() const noexcept;
    const std::exception_ptr& failure() const noexcept;
private:
    friend class GameFrontendHost;
    void require_actual_manager_domain(GameNativeGuiManagerApplication&);
    void* load_manager_texture_impl(std::uint16_t, std::string_view,
        GameNativeGuiManagerApplication*, void*);
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
