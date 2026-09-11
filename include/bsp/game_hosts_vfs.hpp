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
#include "bsp/game_hosts.hpp"
#include "bsp/package_scan.hpp"
#include "bsp/vfs_candidates.hpp"
#include "bsp/vfs_provider_manager.hpp"
#include "bsp/vfs_startup.hpp"

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
    GameVfsHost(GameHostLog& log, bool cached_load);
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

private:
    PackageScanCallbacks package_scan_callbacks();

    GameHostLog& log_;
    bool cached_load_{};
    std::shared_ptr<VfsProviderFactories> factories_;
    std::unique_ptr<VfsProviderManager> manager_;
    VfsCandidateRegistrations search_registrations_;
    std::vector<GameMountRecord> mounts_;
    std::vector<GameVfsProbeResult> probes_;
    std::array<PackageScanPass, 2> scans_;
    VfsStartupState state_;
    int scan_pass_{};
    // The three phase-2 factory tokens. VfsProviderFactories is one object holding the
    // physical, FileStore and MPKG factories, so the getters hand back that object and
    // register_provider_factory_00be0660 has nothing left to append; the MPAK factory
    // 00736b60 has no reconstruction at all and stays null.
    std::uint8_t factory_tokens_[3]{};
};

// Concrete GameSettingsHost for 008d8190. The options file, the registry language value and
// the desktop size are real reads; the two native capability tables are described at the
// members that supply them.
class GameSettingsBinding final : public GameSettingsHost {
public:
    explicit GameSettingsBinding(GameHostLog& log);
    ~GameSettingsBinding() override;

    std::optional<std::string> read_options_file() const override;
    std::optional<std::uint32_t> read_registry_language_lcid() const override;
    Resolution desktop_size() const override;
    const std::vector<Resolution>& supported_resolutions() const override;
    const std::vector<int>& supported_antialias_levels() const override;
    int max_shader_model() const override;
    void apply_detected_defaults(GameSettings& settings) const override;
    void log(const std::string& line) const override;

    // 008d5150 with SHGetSpecialFolderPathA(CSIDL_PERSONAL); empty when the folder is absent.
    const std::string& options_path() const noexcept { return options_path_; }
    bool options_file_present() const noexcept { return options_present_; }
    // Recognized token names whose spelling the file disagreed with; see the .cpp.
    const std::vector<std::string>& recased_tokens() const noexcept { return recased_; }

private:
    void enumerate_adapter_modes();

    GameHostLog& log_;
    std::string options_path_;
    mutable bool options_present_{};
    mutable std::vector<std::string> recased_;
    std::vector<Resolution> resolutions_;
    std::vector<int> antialias_levels_;  // stays empty; see the .cpp
    int max_shader_model_{};
};

}  // namespace bsp::game
