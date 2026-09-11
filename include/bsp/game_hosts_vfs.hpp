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
// Nothing here is a new reconstruction. GameVfsHost is an integration binding that satisfies
// bsp::VfsStartupHost by driving the reconstructed VFS types (bsp::VfsProviderFactories,
// bsp::VfsProviderManager, the mount registration, the package scan and the search-path
// registration); GameSettingsBinding satisfies bsp::GameSettingsHost with the real options
// file. Whatever is still unrecovered goes through GameHostLog::unimplemented with its
// native call site, exactly as in milestone 1.
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
#include "bsp/vfs_provider_manager.hpp"
#include "bsp/vfs_startup.hpp"
#include "bsp/vfs_locale_runtime.hpp"
#include "bsp/settings_capabilities.hpp"
#include "bsp/settings_text.hpp"
#include "bsp/input_script_startup.hpp"
#include "bsp/gui_locale_refresh.hpp"

namespace bsp::game {

// One 00be1890 request as the run performed it, in request order. Priority and ownership are
// the values the native call site pushes; `status` is the manager's own three-way result.
struct GameMountRecord {
    std::string system_path;
    std::string virtual_path;
    std::int32_t priority{};
    std::uint8_t ownership{};
    std::int32_t device_id{-1};
    const char* status{"failed"};  // created | declined | failed
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

// Concrete VfsStartupHost. Every method is one native call site of the phase-2 block, the
// factory tail or phase 6; the ones backed by a reconstructed VFS type run for real, the rest
// take the unimplemented-host policy. The object owns the provider manager for the whole run,
// so the later milestones read assets through the same mounts startup created.
class GameVfsHost final : public VfsStartupHost {
public:
    // The mount system path is whatever GetCurrentDirectoryA returns at 0073d697. bsp_game
    // sets the process current directory from --game-root before Init, so this stays the
    // recovered call rather than an injected path.
    // hardware_probe_commit lets the phase-2 probe raise its message box and
    // perform its registry write-back; see include/bsp/game_hosts_init_tail.hpp.
    GameVfsHost(GameHostLog& log, bool cached_load, bool hardware_probe_commit = false);
    ~GameVfsHost() override;

    // ---- phase 2, 0073d604..0073d899 ----
    bool provider_manager_installed() override;
    void probe_hardware_0073c3b0() override;
    void construct_provider_manager_00beda60() override;
    void install_manager_handlers(std::uint32_t handler_90h, std::uint32_t handler_8ch) override;
    VfsStartupObject file_store_factory_004fc150() override;
    VfsStartupObject mpkg_factory_00736a90() override;
    VfsStartupObject mpak_factory_00736b60() override;
    void register_provider_factory_00be0660(VfsStartupObject factory) override;
    std::string current_directory_with_separator() override;
    void mount_system_path_00be1890(const std::string& system_path,
        const std::string& virtual_path, std::int32_t priority, std::uint8_t ownership,
        std::int32_t device_id) override;
    void mount_packages_0073cb10() override;
    void register_resource_search_paths_00738360() override;

    // ---- factory tail, 0073d94f..0073d98d ----
    VfsStartupObject pak_archive_registry_00736c30() override;
    void set_manager_pak_registry_00bd9230(VfsStartupObject registry) override;
    void set_manager_cached_load_00bd9f90(bool cached_load) override;
    void create_pak_registry_lock_00bb40b0() override;

    // ---- phase 6, 0073db41..0073db69 ----
    VfsStartupObject resource_manager_004c1400() override;
    VfsStartupObject animation_channels_parser_00736dd0() override;
    VfsStartupObject bone_parser_00736ea0() override;
    bool register_type_parser_00b80a50(VfsStartupObject manager,
        VfsStartupObject parser) override;

    // What the later milestones use. 00bdf4c0 resolves the name through the mounts and the
    // search groups, then 00bdf310 opens it into memory. Read-only; nothing is cached.
    GameVfsProbeResult resolve_and_read(const std::string& requested);
    // resolve_and_read, kept in the run record and logged.
    const GameVfsProbeResult& probe(const std::string& requested);
    const std::vector<GameVfsProbeResult>& probes() const noexcept { return probes_; }
    // 00bdd440 without opening a stream, for a cheap existence answer.
    bool exists(const std::string& requested);

    bool ready() const noexcept { return manager_ != nullptr; }
    const std::vector<GameMountRecord>& mounts() const noexcept { return mounts_; }
    const std::array<PackageScanPass, 2>& package_scans() const noexcept { return scans_; }
    // Providers enumerated by the two 0073cb10 passes, before the already-mounted filter.
    std::size_t package_entries_enumerated() const noexcept;
    std::size_t package_entries_mounted() const noexcept;
    const VfsStartupState& startup_state() const noexcept { return state_; }
    bool cached_load() const noexcept { return cached_load_; }
    VfsProviderManager* manager() const noexcept { return manager_.get(); }
    const VfsCandidateRegistrations& search_registrations() const noexcept { return search_registrations_; }

    // ---- milestone 2c ----
    const GameHardwareProbeSummary& hardware_probe() const noexcept;
    // The manager the two 004c1400 fetches return; the same object both times.
    GameResourceManager* resource_manager() const noexcept { return resource_manager_.get(); }
    // Factories appended by 00be0660, in call order: physical, FileStore, MPKG
    // then the MPAK singleton the factory tail adds.
    std::size_t registered_factories() const noexcept { return registered_factories_.size(); }
    std::size_t registered_parsers() const noexcept;
    bool pak_registry_published() const noexcept { return pak_registry_published_; }
    bool pak_lock_published() const noexcept { return pak_lock_ != nullptr; }

private:
    PackageScanCallbacks package_scan_callbacks();

    GameHostLog& log_;
    bool cached_load_{};
    bool hardware_probe_commit_{};
    std::shared_ptr<VfsProviderFactories> factories_;
    std::unique_ptr<VfsProviderManager> manager_;
    VfsCandidateRegistrations search_registrations_;
    std::vector<GameMountRecord> mounts_;
    std::vector<GameVfsProbeResult> probes_;
    std::array<PackageScanPass, 2> scans_;
    VfsStartupState state_;
    int scan_pass_{};
    // The four factory tokens. VfsProviderFactories is one object holding the physical,
    // FileStore and MPKG factories, so those getters hand back that object; the MPAK
    // factory 00736b60 is the 8-byte singleton of kMpakProviderFactory_00736b60, whose
    // Create 00bb83a0 is packet `mpak_provider_create` and is not reconstructed.
    std::uint8_t factory_tokens_[4]{};
    // Milestone 2c: the factory tail 0073d94f-0073d98d and phase 6.
    std::unique_ptr<GameHardwareProbe> hardware_probe_;
    std::vector<const void*> registered_factories_;
    PakArchiveRegistryState pak_registry_{};
    bool pak_registry_published_{};
    FileManagerKnobs manager_knobs_{};
    SharedLock pak_lock_storage_{};
    SharedLock* pak_lock_{};
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
