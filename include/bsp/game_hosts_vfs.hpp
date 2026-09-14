#pragma once
// bsp_game.exe milestone 2a: the phase-2 virtual file system and the phase-5 settings load,
// running for real inside the process instead of being recorded as unimplemented phases.
//
// Addresses: 0073d604..0073d899 the phase-2 block of cSkeletonAppMidway::Init, 00beda60 the
// provider manager constructor, 004fc150 / 00736a90 / 00736b60 the three provider factory
// singletons, 00be0660 factory registration, 00be1890 the mount call, 0073cb10 the package
// scan, 00738360 the resource search-path registration, 0073d94f..0073d98d the factory tail,
// 0073db41..0073db69 phase 6, 008d8190 the settings load, 008d5150 the options path.
//
// Native VFS admission and retained ownership: docs/GAME_NATIVE_VFS_ADMISSION_BJ.md.
// Resource-manager/parser hosts retain their separately documented boundaries.
//
// Evidence: docs/GAME_EXECUTABLE.md, docs/APP_INIT_VFS_SINGLETONS.md,
// docs/PROVIDER_FACTORY_STARTUP.md, docs/PACKAGE_MOUNT_STARTUP.md,
// docs/PACKAGE_SCAN_IMPLEMENTATION.md, docs/VFS_MOUNT_REGISTRATION.md,
// docs/VFS_CANDIDATE_RESOLUTION.md, docs/PHYSICAL_DIRECTORY.md, docs/APP_INIT_BOOTSTRAP.md,
// docs/APP_INIT_PLATFORM.md, docs/SETTINGS_TEXT_PERSISTENCE.md.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "bsp/app_bootstrap.hpp"
#include "bsp/app_init_tail.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_init_tail.hpp"
#include "bsp/package_scan.hpp"
#include "bsp/vfs_candidates.hpp"
#include "bsp/vfs_native_access.hpp"
#include "bsp/game_native_vfs_application.hpp"
#include "bsp/vfs_startup.hpp"
#include "bsp/vfs_locale_runtime.hpp"
#include "bsp/settings_capabilities.hpp"
#include "bsp/settings_text.hpp"
#include "bsp/input_script_startup.hpp"
#include "bsp/gui_locale_refresh.hpp"

namespace bsp::game {

// One 00be1890 request as the run performed it, in request order. Priority and ownership are
// the values the native call site pushes; `status` records returned provider or null.
struct GameMountRecord {
    std::string system_path;
    std::string virtual_path;
    std::int32_t priority{};
    std::uint8_t ownership{};
    std::int32_t device_id{-1};
    const char* status{"failed"};  // created | failed (native returned null)
    std::string error;
    bool from_package_scan{};  // true for the mounts 0073cb10 made
};

// One resolve-and-read through the mounted providers.
struct GameVfsProbeResult {
    std::string requested;   // the name handed to 00bdf4c0
    std::string resolved;    // the name 00bdf4c0 left behind on success
    bool resolved_ok{};
    bool opened{};
    std::int64_t bytes{-1};  // MemoryStream::size_00bef600 on success
    std::string error;
};

// Production source owner for the actual VFS graph. The context only adapts
// existing parser interfaces; it contains no projected providers or mounts.
class GameVfsHost final : public NativeVfsAccess {
public:
    GameVfsHost(GameHostLog&, GameSingletonHost&, GameNativeReadOnlyData&,
        const std::filesystem::path& original_executable, bool hardware_probe_commit = false);
    ~GameVfsHost() override;
    void phase2(VfsStartupState&);
    void factory_tail(VfsStartupState&, bool cached_load);
    void phase6(VfsStartupState&);
    bool ready() const noexcept { return core_ready_; }
    bool requires_process_retention() const noexcept { return native_operation_failed_; }
    std::uint32_t failure_site() const noexcept;
    VfsMountContext& context() noexcept { return consumer_context_; }
    // Unused by the native dispatch, retained only for legacy parser signatures.
    const VfsCandidateRegistrations& search_registrations() const noexcept { return unused_registrations_; }
    bool exists(const std::string&) override;
    bool resolve_existing(std::string&) override;
    bool direct_resolve(const std::string&, std::string&) override;
    VfsMemoryOpen open(const std::string&, std::uint32_t) override;
    std::vector<std::string> enumerate(const std::string&, const std::string&, std::uint32_t) override;
    std::array<std::uint32_t, 5> file_date(const std::string&) override;
    GameVfsProbeResult resolve_and_read(const std::string&);
    const GameVfsProbeResult& probe(const std::string&);
    const std::vector<GameVfsProbeResult>& probes() const noexcept { return probes_; }
    // These observations cover the three loose mount requests, not package entries.
    const std::vector<GameMountRecord>& mounts() const noexcept { return mounts_; }
    std::size_t package_scans_completed() const noexcept { return package_scans_completed_; }
    bool cached_load() const noexcept { return cached_load_; }
    const GameHardwareProbeSummary& hardware_probe() const noexcept;
    GameResourceManager* resource_manager() const noexcept { return resource_manager_.get(); }
    std::size_t registered_factories() const noexcept { return factories_registered_; }
    std::size_t registered_parsers() const noexcept;
    bool pak_registry_published() const noexcept { return archive_tail_ready_; }
    bool pak_lock_published() const noexcept { return archive_tail_ready_; }

private:
    template<class Operation> decltype(auto) invoke_native(Operation&& operation) {
        try { return operation(); }
        catch (...) { native_operation_failed_ = true; throw; }
    }
    GameNativeVfsRuntime& active_runtime();
    VfsStartupObject resource_manager_004c1400();
    VfsStartupObject animation_channels_parser_00736dd0();
    VfsStartupObject bone_parser_00736ea0();
    bool register_type_parser_00b80a50(VfsStartupObject, VfsStartupObject);
    GameHostLog& log_;
    bool hardware_probe_commit_{};
    std::unique_ptr<GameNativeVfsApplication> native_;
    VfsMountContext consumer_context_;
    VfsCandidateRegistrations unused_registrations_;
    bool core_ready_{};
    bool archive_tail_ready_{};
    bool cached_load_{};
    bool native_operation_failed_{};
    std::size_t factories_registered_{};
    std::size_t package_scans_completed_{};
    std::vector<GameMountRecord> mounts_;
    std::vector<GameVfsProbeResult> probes_;
    std::unique_ptr<GameHardwareProbe> hardware_probe_;
    std::unique_ptr<GameResourceManager> resource_manager_;
    std::unique_ptr<GameStructuredParser> animation_channels_parser_;
    std::unique_ptr<GameStructuredParser> bone_parser_;
};

// Settings startup over the retained game state, mounted catalog and recovered
// D3D9 capability operations. See docs/SETTINGS_STARTUP_OWNER.md.
// Persistent VFS, runtime, globals and input owner. The interpreter closes before
// its DoFile runtime/files are destroyed; the mounted VFS and suffix list outlive
// this aggregate. Other startup Lua consumers can borrow the same services.
class GameScriptHost {
public:
    GameScriptHost(GameHostLog&, VfsMountContext&, const std::vector<std::string>&,
        LuaRuntimeGlobals globals);
    InputScriptStartup& input() noexcept { return input_; }
    VfsLuaScriptFiles& files() noexcept { return files_; }
    LuaScriptRuntime& runtime() noexcept { return runtime_; }
    LuaRuntimeGlobals& globals() noexcept { return globals_; }
    const LuaRuntimeGlobals& globals() const noexcept { return globals_; }
private:
    LuaRuntimeGlobals globals_;
    VfsLuaScriptFiles files_;
    LuaScriptRuntime runtime_;
    InputScriptStartup input_;
};

// The locale table owner is created at0073e057. GUI state remains lazy: the
// initial setter runs before registering "globals" and does not request a GUI
// refresh. Later changes traverse the actual retained page registry.
class GameLocaleHost final : public LocaleGuiRefreshHost {
public:
    explicit GameLocaleHost(GameHostLog& log) : log_(log) {}
    void initialize(LocaleTableSource&, const std::string& language);
    void refresh_locale_00aa4650() override;
    LocaleTables& tables() noexcept { return tables_; }
    GuiLocaleRefreshManager& gui();
    bool gui_created() const noexcept { return gui_ != nullptr; }
private:
    GameHostLog& log_;
    LocaleTables tables_;
    std::unique_ptr<GuiLocaleRefreshManager> gui_;
};

// Borrows the application's renderer API and capability state for its lifetime.
class GameSettingsBinding final : public GameSettingsHost {
public:
    GameSettingsBinding(GameHostLog& log, VfsMountContext& mounts,
        const VfsCandidateRegistrations& registrations, const std::vector<std::string>& suffixes,
        ProfileHintsOwner& hints, IDirect3D9& api, SettingsRendererCapabilities& capabilities,
        std::string personal_root = {});
    GameSettingsBinding(const GameSettingsBinding&) = delete;
    GameSettingsBinding& operator=(const GameSettingsBinding&) = delete;

    void build_language_catalog_008d7bc0() override;
    const std::vector<LanguageEntry>& language_catalog() const override;
    void copy_supported_resolutions_008d4ea0() override;
    std::optional<std::string> read_options_file() override;
    std::optional<std::uint32_t> read_registry_language_lcid() const override;
    Resolution desktop_size() const override;
    const std::vector<Resolution>& supported_resolutions() const override;
    const std::vector<int>& supported_antialias_levels() const override;
    int max_shader_model() const override;
    void write_options_text_008d6170(const GameSettingsBlock& settings) override;
    void select_shader_model_00b200c0(int selected) override;
    std::uint32_t pixel_shader_version_28() const override;
    void rebuild_antialias_levels_00b295c0(std::uint32_t format) override;
    void copy_supported_antialias_008d4df0() override;
    void log(const std::string& line) const override;

    const std::string& options_path() const noexcept { return options_path_; }
    bool options_file_present() const noexcept { return options_present_; }
    VfsLocaleRuntime& locale_source() noexcept { return locale_source_; }
    IDirect3D9& renderer_api() noexcept { return api_; }

private:
    GameHostLog& log_;
    Win32SettingsTextHost text_host_;
    VfsLocaleRuntime locale_source_;
    std::vector<LanguageEntry> languages_;
    IDirect3D9& api_;
    SettingsRendererCapabilities& capabilities_;
    std::vector<Resolution> resolutions_;
    std::vector<int> antialias_levels_;
    std::string options_path_;
    bool options_present_{};
};

}  // namespace bsp::game
