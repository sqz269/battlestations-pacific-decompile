#pragma once
#include "bsp/game_settings.hpp"
#include "bsp/native_settings_loader.hpp"
#include <memory>

struct IDirect3D9;
namespace bsp {class VfsLocaleRuntime;struct NativeGameSettingsStorage;}
namespace bsp::game {
class GameHostLog;class GameVfsHost;class GameSingletonHost;
class GameNativeReadOnlyData;class GameNativeSettingsProcess;class GameNativeRendererApplication;
// Explicit source stack policy. These bytes are not recovered native stack
// identity. Actual OS/file/stream calls overwrite their native output slots.
struct GameNativeSettingsStackPolicy {
    NativeSettingsLoaderPreimages loader;
    NativeStreamTextStackPreimages scanner;
    std::array<char,260> personal_buffer;
};
// Read-only compatibility copy for the remaining projected UI/audio/locale
// consumers. The raw BCh owner is authoritative; no projected loader runs.
void copy_native_game_settings_read_view(const NativeGameSettingsStorage&,
    const std::vector<LanguageEntry>&,GameSettingsBlock&);
class GameNativeSettingsApplication final {
public:
    GameNativeSettingsApplication(GameHostLog&,GameNativeSettingsProcess&,GameSingletonHost&,
        GameVfsHost&,GameNativeRendererApplication&,GameNativeReadOnlyData&,
        const std::vector<std::string>& suffixes,std::string personal_root,
        const GameNativeSettingsStackPolicy&);
    ~GameNativeSettingsApplication();
    GameNativeSettingsApplication(const GameNativeSettingsApplication&)=delete;
    GameNativeSettingsApplication& operator=(const GameNativeSettingsApplication&)=delete;
    void load();
    bool requires_process_retention() const noexcept;
    std::uint32_t failure_site() const noexcept;
    void copy_read_view(GameSettingsBlock&) const;
    const std::vector<LanguageEntry>& language_catalog() const noexcept;
    VfsLocaleRuntime& locale_source() noexcept;
    IDirect3D9& renderer_api() const;
    const std::string& options_path() const noexcept;
    bool options_file_present() const noexcept;
private:
    struct Impl;std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
