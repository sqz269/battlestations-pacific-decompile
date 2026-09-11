#define _CRT_SECURE_NO_WARNINGS // Recovered CRT text-file read/write contract.
// bsp_game.exe milestone 2a bindings. See include/bsp/game_hosts_vfs.hpp and
// docs/GAME_EXECUTABLE.md. No native behaviour is invented here: whatever is not
// reconstructed goes through GameHostLog::unimplemented with its native call site.
#include "bsp/game_hosts_vfs.hpp"

#include <d3d9.h>
#include <shlobj.h>

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <utility>

#include "bsp/memory_stream.hpp"
#include "bsp/physical_file.hpp"
#include "bsp/resource_lookup.hpp"
#include "bsp/vfs_mount_registration.hpp"
#include "bsp/vfs_search_defaults.hpp"
#include "bsp/winmain_startup.hpp"

namespace bsp::game {
namespace {

const char* create_status_name(VfsProviderCreateStatus status) {
    switch (status) {
        case VfsProviderCreateStatus::created: return "created";
        case VfsProviderCreateStatus::declined: return "declined";
        case VfsProviderCreateStatus::failed: break;
    }
    return "failed";
}

const char* scan_disposition_name(PackageScanDisposition disposition) {
    switch (disposition) {
        case PackageScanDisposition::already_mounted: return "already_mounted";
        case PackageScanDisposition::mounted: return "mounted";
        case PackageScanDisposition::declined: return "declined";
        case PackageScanDisposition::failed: break;
    }
    return "failed";
}

}  // namespace

// ---------------------------------------------------------------------------
// GameVfsHost
// ---------------------------------------------------------------------------

GameVfsHost::GameVfsHost(GameHostLog& log, bool cached_load)
    : log_(log), cached_load_(cached_load) {}

GameVfsHost::~GameVfsHost() = default;

bool GameVfsHost::provider_manager_installed() {
    // DAT_0109ceec at 0073d604. One process, one Init pass, so the gate is always open.
    return manager_ != nullptr;
}

void GameVfsHost::probe_hardware_0073c3b0() {
    // 0073d610, inside the gate. The probe needs the hardware profile host; not reconstructed.
    log_.unimplemented("Phase 2 probe_hardware", "0073c3b0");
}

void GameVfsHost::construct_provider_manager_00beda60() {
    // 0073d615..0073d637: malloc(0A0h) then 00beda60. The reconstructed manager owns the
    // physical factory the native constructor registers for itself.
    factories_ = std::make_shared<VfsProviderFactories>();
    manager_ = std::make_unique<VfsProviderManager>(factories_);
    log_.implemented("Phase 2 construct_provider_manager", "00beda60");
}

void GameVfsHost::install_manager_handlers(std::uint32_t handler_90h,
    std::uint32_t handler_8ch) {
    // manager+90h and manager+8Ch at 0073d642 / 0073d652. Both targets are a single C3
    // followed by INT3 padding, so storing them cannot change behaviour and the
    // reconstructed manager carries no handler slots. Recorded with the two addresses.
    log_.implemented("Phase 2 install_manager_handlers", "0073d642");
    log_.notef("manager handlers +90h=%08x +8Ch=%08x (both verified RET)", handler_90h,
        handler_8ch);
}

VfsStartupObject GameVfsHost::file_store_factory_004fc150() {
    // The FileStore factory singleton. VfsProviderFactories holds it; the token identifies
    // the call site, it is never dereferenced.
    log_.implemented("Phase 2 file_store_factory", "004fc150");
    return &factory_tokens_[0];
}

VfsStartupObject GameVfsHost::mpkg_factory_00736a90() {
    log_.implemented("Phase 2 mpkg_factory", "00736a90");
    return &factory_tokens_[1];
}

VfsStartupObject GameVfsHost::mpak_factory_00736b60() {
    // DAT_010904d4, the .mpak provider factory. No reconstruction exists for the mpak
    // provider, so no token is produced and the registration below records the gap.
    log_.unimplemented("Factory tail mpak_factory", "00736b60");
    return nullptr;
}

void GameVfsHost::register_provider_factory_00be0660(VfsStartupObject factory) {
    // 00be0660 appends a factory to the manager list. VfsProviderFactories is a single
    // object that already holds the physical, FileStore and MPKG factories in registration
    // order, so an append has nothing to add; a null token is the unreconstructed mpak.
    if (factory == nullptr) {
        // The 0073d94f registration in the factory tail, kept under its own name so a
        // reached-but-unimplemented call is never merged into the two concrete ones.
        log_.unimplemented("Factory tail register_provider_factory", "00be0660");
        return;
    }
    log_.implemented("Phase 2 register_provider_factory", "00be0660");
}

std::string GameVfsHost::current_directory_with_separator() {
    // 0073d68d..0073d6b1: GetCurrentDirectoryA(0FAh, buf) then _strcat_s(buf, 100h, "\").
    char buffer[0x100] = {};
    const DWORD length = GetCurrentDirectoryA(0xFA, buffer);
    std::string path(buffer, length);
    if (path.empty() || path.back() != '\\') path.push_back('\\');
    log_.implemented("Phase 2 current_directory", "0073d697");
    log_.notef("mount system path %s", path.c_str());
    return path;
}

void GameVfsHost::mount_system_path_00be1890(const std::string& system_path,
    const std::string& virtual_path, std::int32_t priority, std::uint8_t ownership,
    std::int32_t device_id) {
    GameMountRecord record;
    record.system_path = system_path;
    record.virtual_path = virtual_path;
    record.priority = priority;
    record.ownership = ownership;
    record.device_id = device_id;
    if (manager_ == nullptr) {
        record.error = "no provider manager";
        mounts_.push_back(record);
        return;
    }
    std::shared_ptr<VfsProviderIdentity> identity;
    std::string error;
    const VfsProviderCreateStatus status = manager_->mount_system_path_00be1890_fragment(
        system_path, virtual_path, priority, ownership, device_id, identity, error);
    record.status = create_status_name(status);
    record.error = error;
    mounts_.push_back(record);
    log_.implemented("Phase 2 mount_system_path", "00be1890");
    log_.notef("mount %s -> \"%s\" priority=%d ownership=%u device=%d %s%s%s",
        system_path.c_str(), virtual_path.c_str(), priority,
        static_cast<unsigned>(ownership), device_id, record.status,
        error.empty() ? "" : " error=", error.c_str());
}

PackageScanCallbacks GameVfsHost::package_scan_callbacks() {
    PackageScanCallbacks callbacks;
    callbacks.enumerate = [this](const std::string& directory, const std::string& extension,
        std::uint32_t flags, std::vector<std::string>& output, std::string& error) {
        return enumerate_resources_00bdd990_fragment(manager_->context(), directory,
            extension, flags, output, error);
    };
    callbacks.already_mounted = [this](const std::string& system_name) {
        return manager_->find_system_name_00bdb120(system_name) != nullptr;
    };
    callbacks.mount = [this](const std::string& system_name, const std::string& virtual_path,
        std::int32_t priority, std::uint8_t ownership, std::int32_t device_id,
        std::string& error) {
        GameMountRecord record;
        record.system_path = system_name;
        record.virtual_path = virtual_path;
        record.priority = priority;
        record.ownership = ownership;
        record.device_id = device_id;
        record.from_package_scan = true;
        std::shared_ptr<VfsProviderIdentity> identity;
        const VfsProviderCreateStatus status = manager_->mount_system_path_00be1890_fragment(
            system_name, virtual_path, priority, ownership, device_id, identity, error);
        record.status = create_status_name(status);
        record.error = error;
        mounts_.push_back(record);
        return status == VfsProviderCreateStatus::created ? PackageMountStatus::mounted
            : status == VfsProviderCreateStatus::declined ? PackageMountStatus::declined
            : PackageMountStatus::failed;
    };
    return callbacks;
}

void GameVfsHost::mount_packages_0073cb10() {
    // 0073d881 and 0073d888 call this twice with the same ECX. Each pass queries
    // (".", "mpkg", 0) afresh, so the second sees what the first mounted.
    const int pass = scan_pass_;
    if (manager_ == nullptr || pass >= static_cast<int>(scans_.size())) {
        log_.unimplemented("Phase 2 mount_packages", "0073cb10");
        return;
    }
    ++scan_pass_;
    const bool complete = scan_packages_0073cb10_fragment(package_scan_callbacks(),
        scans_[static_cast<std::size_t>(pass)]);
    const PackageScanPass& report = scans_[static_cast<std::size_t>(pass)];
    log_.implemented("Phase 2 mount_packages", "0073cb10");
    log_.notef("package scan %d enumerated=%d entries=%zu complete=%d%s%s", pass + 1,
        report.enumerated ? 1 : 0, report.entries.size(), complete ? 1 : 0,
        report.error.empty() ? "" : " error=", report.error.c_str());
    for (const PackageScanEntry& entry : report.entries) {
        log_.notef("  package %s priority=%d %s%s%s", entry.system_name.c_str(),
            entry.priority, scan_disposition_name(entry.disposition),
            entry.error.empty() ? "" : " error=", entry.error.c_str());
    }
}

void GameVfsHost::register_resource_search_paths_00738360() {
    // 0073d894, the JNZ target of the gate: it runs on every Init pass. The reconstruction
    // covers the texture and shaderfx groups only; see docs/VFS_SEARCH_REGISTRATION.md.
    search_registrations_ = make_asset_search_registrations_00738360_fragment();
    log_.implemented("Phase 2 register_resource_search_paths", "00738360");
    log_.notef("search registrations groups=%zu extension_prefixes=%zu",
        search_registrations_.groups.size(), search_registrations_.extension_prefixes.size());
}

VfsStartupObject GameVfsHost::pak_archive_registry_00736c30() {
    log_.unimplemented("Factory tail pak_archive_registry", "00736c30");
    return nullptr;
}

void GameVfsHost::set_manager_pak_registry_00bd9230(VfsStartupObject registry) {
    // manager+88h. Without a reconstructed PAK registry there is nothing to store.
    (void)registry;
    log_.unimplemented("Factory tail set_manager_pak_registry", "00bd9230");
}

void GameVfsHost::set_manager_cached_load_00bd9f90(bool cached_load) {
    // manager+78h receives the cachedload byte DAT_00e1ae76, parsed from the command line at
    // 0073d94a. The reconstructed manager has no cached-load slot, so the flag is carried
    // here and reported; it reaches no provider in this milestone.
    cached_load_ = cached_load;
    log_.implemented("Factory tail set_manager_cached_load", "00bd9f90");
    log_.notef("cachedload=%d", cached_load ? 1 : 0);
}

void GameVfsHost::create_pak_registry_lock_00bb40b0() {
    log_.unimplemented("Factory tail create_pak_registry_lock", "00bb40b0");
}

VfsStartupObject GameVfsHost::resource_manager_004c1400() {
    log_.unimplemented("Phase 6 resource_manager", "004c1400");
    return nullptr;
}

VfsStartupObject GameVfsHost::animation_channels_parser_00736dd0() {
    log_.unimplemented("Phase 6 animation_channels_parser", "00736dd0");
    return nullptr;
}

VfsStartupObject GameVfsHost::bone_parser_00736ea0() {
    log_.unimplemented("Phase 6 bone_parser", "00736ea0");
    return nullptr;
}

bool GameVfsHost::register_type_parser_00b80a50(VfsStartupObject manager,
    VfsStartupObject parser) {
    (void)manager;
    (void)parser;
    log_.unimplemented("Phase 6 register_type_parser", "00b80a50");
    return false;
}

bool GameVfsHost::exists(const std::string& requested) {
    if (manager_ == nullptr) return false;
    return exists_resource_00bdd440_fragment(manager_->context(), requested);
}

GameVfsProbeResult GameVfsHost::resolve_and_read(const std::string& requested) {
    GameVfsProbeResult result;
    result.requested = requested;
    if (manager_ == nullptr) {
        result.error = "no provider manager";
        return result;
    }
    std::string name = requested;
    // 00bdf4c0: normalize, try the mounts directly, then the search candidates.
    if (!resolve_existing_resource_00bdf4c0_fragment(manager_->context(),
            search_registrations_, name)) {
        result.error = "no mounted provider resolves " + requested;
        return result;
    }
    result.resolved_ok = true;
    result.resolved = name;
    // 00bdf310 with flags 2, the read-only mode the startup opens use.
    VfsMemoryOpen opened = open_resource_memory_00bdf310_fragment(manager_->context(),
        name, 2);
    if (!opened.provider_opened || !opened.stream) {
        result.error = opened.error.empty() ? "resolved but no provider opened " + name
            : opened.error;
        return result;
    }
    result.opened = true;
    result.bytes = opened.stream->size_00bef600();
    if (!opened.stream->fully_initialized()) {
        result.error = "short read: stream tail uninitialized";
    }
    return result;
}

const GameVfsProbeResult& GameVfsHost::probe(const std::string& requested) {
    probes_.push_back(resolve_and_read(requested));
    const GameVfsProbeResult& result = probes_.back();
    log_.notef("vfs probe %s resolved=%d as=%s opened=%d bytes=%lld%s%s",
        result.requested.c_str(), result.resolved_ok ? 1 : 0,
        result.resolved.empty() ? "(none)" : result.resolved.c_str(),
        result.opened ? 1 : 0, static_cast<long long>(result.bytes),
        result.error.empty() ? "" : " error=", result.error.c_str());
    return result;
}

std::size_t GameVfsHost::package_entries_enumerated() const noexcept {
    std::size_t total = 0;
    for (const PackageScanPass& pass : scans_) total += pass.entries.size();
    return total;
}

std::size_t GameVfsHost::package_entries_mounted() const noexcept {
    std::size_t total = 0;
    for (const PackageScanPass& pass : scans_) {
        for (const PackageScanEntry& entry : pass.entries) {
            if (entry.disposition == PackageScanDisposition::mounted) ++total;
        }
    }
    return total;
}

// ---------------------------------------------------------------------------
// GameSettingsBinding
// ---------------------------------------------------------------------------

GameScriptHost::GameScriptHost(GameHostLog& log, VfsMountContext& mounts,
    const std::vector<std::string>& suffixes, LuaRuntimeGlobals globals)
    : globals_(std::move(globals)), files_(mounts, suffixes), runtime_(files_),
      input_(files_, runtime_, globals_) {
    // The singleton constructor loads immediately; the explicit call at
    // 0073da9b observes +4 already set and keeps the same persistent state.
    input_.load_data_tables_006a7be0();
    log.implemented("Phase 5 input_script_tables", "006ab6b0/006a7be0");
    log.notef("input scripts devices=%zu input_names=%zu controller_names=%zu "
        "presets_lua=%p data_tables_started=%d runtime_settings_loaded=%d",
        input_.settings().devices.size(), input_.settings().input_names.size(),
        input_.settings().controller_input_names.size(),
        static_cast<void*>(input_.control_presets_lua()), input_.data_tables_started() ? 1 : 0,
        input_.settings().runtime_settings_loaded ? 1 : 0);
}

void GameLocaleHost::initialize(LocaleTableSource& source, const std::string& language) {
    bool changed = false;
    std::string error;
    if (!tables_.set_language_00aa09d0(source, *this, language, changed, error))
        throw std::runtime_error("Locale language selection: " + error);
    tables_.register_table_00aa0d30("globals");
    if (!tables_.reload_00aa06d0(source, false, error))
        throw std::runtime_error("Locale table startup: " + error);
    log_.implemented("Phase 6 locale_set_language", "00aa09d0");
    log_.implemented("Phase 6 locale_register_globals", "00aa0d30");
    log_.implemented("Phase 6 locale_load_tables", "00aa06d0");
    log_.notef("locale language=%s keys=%zu files=%zu registered=%zu gui_created=%d",
        tables_.language().c_str(), tables_.size(), tables_.loaded_files().size(),
        tables_.registered_tables().size(), gui_created() ? 1 : 0);
}

GuiLocaleRefreshManager& GameLocaleHost::gui() {
    if (!gui_) gui_ = std::make_unique<GuiLocaleRefreshManager>();
    return *gui_;
}

void GameLocaleHost::refresh_locale_00aa4650() {
    gui().refresh_locale_00aa4650();
    log_.implemented("LocaleGuiRefreshHost::refresh_locale", "00aa4650");
}

GameSettingsBinding::GameSettingsBinding(GameHostLog& log, VfsMountContext& mounts,
    const VfsCandidateRegistrations& registrations, const std::vector<std::string>& suffixes,
    ProfileHintsOwner& hints, IDirect3D9& api, SettingsRendererCapabilities& capabilities,
    std::string personal_root)
    : log_(log), text_host_(std::move(personal_root)),
      locale_source_(mounts, registrations, suffixes,
          [&hints] { return static_cast<std::uint32_t>(hints.field_08); }),
      api_(api), capabilities_(capabilities) {}

void GameSettingsBinding::build_language_catalog_008d7bc0() {
    bsp::build_language_catalog_008d7bc0(languages_, locale_source_);
    log_.implemented("GameSettingsHost::build_language_catalog", "008d7bc0");
    log_.notef("settings catalog entries=%zu", languages_.size());
}

const std::vector<LanguageEntry>& GameSettingsBinding::language_catalog() const { return languages_; }

void GameSettingsBinding::copy_supported_resolutions_008d4ea0() {
    resolutions_ = settings_resolutions_00b1fff0(capabilities_);
    log_.notef("settings native resolution pairs=%zu shader ceiling=%d pixel version=0x%04x",
        resolutions_.size(), capabilities_.max_shader_model, capabilities_.pixel_shader_version_28);
}

std::optional<std::string> GameSettingsBinding::read_options_file() {
    // Preserve CRT text translation. The host keeps only actual fread bytes;
    // native's unused tail after CRLF translation has no defined byte content.
    options_present_ = false;
    options_path_ = build_options_path_008d5150(text_host_);
    std::FILE* file = std::fopen(options_path_.c_str(), "rt");
    if (!file) return std::nullopt;
    struct CloseFile { std::FILE* file; ~CloseFile() { std::fclose(file); } } close{file};
    if (std::fseek(file, 0, SEEK_END) != 0) throw std::runtime_error("Options seek failed");
    const long length = std::ftell(file);
    if (length < 0 || std::fseek(file, 0, SEEK_SET) != 0) throw std::runtime_error("Options extent failed");
    std::string text(static_cast<std::size_t>(length), '\0');
    const auto actual = std::fread(text.data(), 1, text.size(), file);
    if (std::ferror(file)) throw std::runtime_error("Options read failed");
    text.resize(actual);
    options_present_ = true;
    return text;
}

std::optional<std::uint32_t> GameSettingsBinding::read_registry_language_lcid() const {
    HKEY key = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, kSettingsRegistryKey, 0, KEY_READ, &key)
        != ERROR_SUCCESS) {
        return std::nullopt;
    }
    DWORD type = 0;
    std::array<std::uint8_t, 0x400> data{};
    DWORD size = static_cast<DWORD>(data.size());
    const LSTATUS status = RegQueryValueExA(key, kSettingsRegistryLanguageValue, nullptr,
        &type, data.data(), &size);
    RegCloseKey(key);
    if (status != ERROR_SUCCESS || type != REG_DWORD) return std::nullopt;
    if (size < sizeof(std::uint32_t))
        throw std::runtime_error("Registry language has an incomplete native DWORD");
    std::uint32_t value{};
    std::memcpy(&value, data.data(), sizeof(value));
    return value;
}

Resolution GameSettingsBinding::desktop_size() const {
    // GetDesktopWindow + GetWindowRect, the two calls of path B at 008d82c5.
    RECT rectangle = {};
    if (!GetWindowRect(GetDesktopWindow(), &rectangle)) return Resolution{};
    return Resolution{static_cast<int>(rectangle.right - rectangle.left),
        static_cast<int>(rectangle.bottom - rectangle.top)};
}

const std::vector<Resolution>& GameSettingsBinding::supported_resolutions() const {
    return resolutions_;
}

const std::vector<int>& GameSettingsBinding::supported_antialias_levels() const {
    return antialias_levels_;
}

int GameSettingsBinding::max_shader_model() const { return settings_max_shader_model_00b200b0(capabilities_); }

void GameSettingsBinding::write_options_text_008d6170(const GameSettingsBlock& settings) {
    bsp::write_settings_text_008d6170(settings, languages_, text_host_);
    log_.implemented("GameSettingsHost::write_options_text", "008d6170");
}

void GameSettingsBinding::select_shader_model_00b200c0(int selected) {
    select_settings_shader_model_00b200c0(selected);
}

std::uint32_t GameSettingsBinding::pixel_shader_version_28() const {
    return capabilities_.pixel_shader_version_28;
}

void GameSettingsBinding::rebuild_antialias_levels_00b295c0(std::uint32_t format) {
    Win32SettingsCapabilityQueries queries(api_);
    rebuild_settings_antialias_00b295c0(capabilities_, queries, format);
    log_.implemented("GameSettingsHost::rebuild_antialias_levels", "00b295c0");
    log_.notef("settings AA surface format=%u supported levels=%zu", format, capabilities_.antialias_levels.size());
}

void GameSettingsBinding::copy_supported_antialias_008d4df0() {
    antialias_levels_ = settings_antialias_levels_00b20000(capabilities_);
}

void GameSettingsBinding::log(const std::string& line) const {
    log_.notef("settings %s", line.c_str());
}

}  // namespace bsp::game
