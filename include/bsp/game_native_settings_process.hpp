#pragma once
#include "bsp/native_game_settings_owner.hpp"
#include "bsp/native_language_catalog_storage.hpp"
#include "bsp/native_profile_hints_owner.hpp"
#include <array>
#include <memory>

namespace bsp {class XLiveLibrary;}
namespace bsp::game {
class GameNativeReadOnlyData;
// Canonical raw BCh settings, three 0Ch vectors, and their actual CRT callbacks.
// All cells and contexts survive the application and every exit callback.
class GameNativeSettingsProcess final {
public:
    GameNativeSettingsProcess(const GameNativeSettingsProcess&)=delete;
    GameNativeSettingsProcess& operator=(const GameNativeSettingsProcess&)=delete;
    std::array<int,4> initialize_once(); // CD2D60,CD2D70,CD2D80,CD2DA0 order
    NativeGameSettingsStorage& settings();
    void* resolution_header() noexcept;
    void* antialias_header() noexcept;
    void* catalog_header() noexcept;
    void* const volatile& catalog_data() noexcept;
    const volatile std::uint32_t& catalog_count() noexcept;
    NativeLanguageCatalogAllocationCalls& catalog_calls() noexcept;
    NativeProfileHintsOwnerContext& hints() noexcept;
    NativeOnlineManagerStorage* volatile& online_00f8abe8() noexcept;
    void* volatile& game_00e188a8() noexcept;
    NativeProfileSettingsContext& profile_context() noexcept;
    void bind_profile_sdk(const XLiveLibrary*);
    const char* const volatile& scene_whitespace() noexcept;
    const char* const volatile& scene_delimiters() noexcept;
    const char* const volatile& scanner_whitespace() noexcept;
    const char* const volatile& scanner_delimiters() noexcept;
    const char* empty_00f88a3c() const noexcept;
    const char* null_integer_format_01090ab4() const noexcept;
private:
    friend GameNativeSettingsProcess& game_native_settings_process(GameNativeReadOnlyData&);
    explicit GameNativeSettingsProcess(GameNativeReadOnlyData&);
    ~GameNativeSettingsProcess();
    struct Impl;std::unique_ptr<Impl> impl_;
    static void shutdown_resolutions();
    static void shutdown_antialias();
    static void shutdown_settings();
    static void shutdown_catalog();
};
GameNativeSettingsProcess& game_native_settings_process(GameNativeReadOnlyData&);
} // namespace bsp::game
