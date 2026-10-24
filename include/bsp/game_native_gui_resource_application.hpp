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

// One retained initial manager texture invocation. The result is a borrowed
// observation after reference_owned becomes false; it must never be replayed.
// Failed native/string/cache frames require process retention, not rollback.
struct GameNativeGuiResourceLoadOperation final {
    enum class Phase { entered, resolving, loading, complete, failed };
    Phase phase{Phase::entered};
    std::uint16_t manager_offset{};
    std::uint32_t native_site{};
    std::string requested_path;
    NativeString name;
    bool name_live{}, resolved{}, reference_owned{}, release_started{};
    NativeTextureNameResolutionCall resolution;
    NativeTextureCacheAcquired cache;
    NativeRenderActualOwnerReleaseObservation release;
    void* result{};
    std::exception_ptr failure;
};

// Owns exactly the two initial AA5E20 texture references on behalf of the
// frontend's existing manager. Its manager slots only borrow those identities.
// No page/GUI singleton, texture pool, cache, COM owner or reference counter is
// created here. Original AA5E20: ECX manager, RET; renderer64/B319B0: ECX renderer,
// native string* and zero callback word on stack, RET8. This is a C++ binding,
// not an original ABI entry or the complete AA5E20 manager/destructor.
// Its independent temporary-string scopes and retained failure frames are not
// the original enclosing AA5E20 stack/SEH lifetime.
//
// Bind before initial GUI startup, with empty manager+28/+2C, and call in native
// whiteGui then transparent order. No reload/replacement or interrupted replay.
// Keep the SAME ready renderer/VFS/string/actual-owner domains alive through
// explicit retirement. End all manager borrowers before retire_textures().
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
    // One attempt, clearing ownership before each actual+04 decrement/terminal.
    // Any interrupted retirement is retained and cannot be retried.
    void retire_textures();
    const GameNativeGuiResourceLoadOperation& operation(std::size_t) const;
    std::size_t operation_count() const noexcept;
    bool requires_process_retention() const noexcept;
    const std::exception_ptr& failure() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
