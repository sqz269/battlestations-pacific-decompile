#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
struct IDirect3DDevice9;
namespace bsp {
class FontRegistryStartup;
class FingerprintPayload;
namespace game {
class GameHostLog;
class GameVfsHost;
class GameScriptHost;

// Application ownership for the font/payload portion of0073bae0. Uses the
// existing mounted VFS, Lua environment and renderer device. Resources close
// before their D3DX imports, device, script services and VFS. GUI scene/widget
// ownership remains a separate required startup dependency.
class GameFontHost {
public:
    GameFontHost(GameHostLog&, GameVfsHost&, GameScriptHost&,
        IDirect3DDevice9&, std::uint32_t renderer_mip_reduction);
    ~GameFontHost();
    GameFontHost(const GameFontHost&) = delete;
    GameFontHost& operator=(const GameFontHost&) = delete;
    void initialize(const std::string& language_font_path);
    FontRegistryStartup& registry() noexcept;
    const FingerprintPayload& fingerprint() const noexcept;
    std::size_t resource_opens() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
}
