#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
namespace bsp {
class FontRegistryStartup;
class FingerprintPayload;
namespace game {
class GameHostLog;
class GameVfsHost;
class GameScriptHost;
class GameNativeRendererApplication;

// Application ownership for the font/payload portion of0073bae0. Uses the
// existing mounted VFS, Lua environment and canonical native renderer/cache.
// Registry rows borrow the sole native font data/image owners. Retire all Text
// and sprite borrowers, then explicitly retire fonts before device release or
// shared native drain. Keep this host/registry metadata through that drain.
// GUI scene/widget ownership remains a separate startup dependency.
class GameFontHost {
public:
    GameFontHost(GameHostLog&, GameVfsHost&, GameScriptHost&,
        GameNativeRendererApplication&);
    ~GameFontHost();
    GameFontHost(const GameFontHost&) = delete;
    GameFontHost& operator=(const GameFontHost&) = delete;
    // The two halves of0073bae0's font work, so bsp::run_gui_startup can drive them in
    // the recovered order instead of this class choosing one. initialize keeps the
    // milestone-2a entry point and is the two calls in that order.
    void initialize(const std::string& language_font_path);
    //00ac3910 with (root, descriptor, language path), the argument order the callee sees.
    void load_descriptors_00ac3910(const std::string& root, const std::string& descriptor,
        const std::string& language_font_path);
    //0073bc0d..bc17, the forced payload preload whose data pointer is discarded.
    void preload_fingerprint_payload_00be9760();
    // One successful native retirement; repeated host closure is metadata-only.
    // Never retries a failed load/retirement or retires surviving borrowers.
    void retire_native_fonts();
    bool native_operation_interrupted() const noexcept;
    FontRegistryStartup& registry() noexcept;
    const FingerprintPayload& fingerprint() const noexcept;
    // Successful DAT reads (one per completed native font) plus fingerprint
    // payload reads. Image cache/provider opens belong to the native renderer.
    std::size_t resource_opens() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
}
