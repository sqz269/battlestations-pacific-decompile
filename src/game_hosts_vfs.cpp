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
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/vfs_search_defaults.hpp"
#include "bsp/winmain_startup.hpp"

namespace bsp::game {
GameVfsHost::GameVfsHost(GameHostLog& log, GameSingletonHost& singletons,
    GameNativeReadOnlyData& data, const std::filesystem::path& original_executable,
    bool hardware_probe_commit)
    : log_(log), hardware_probe_commit_(hardware_probe_commit),
      native_(std::make_unique<GameNativeVfsApplication>(log, singletons, data,
          original_executable)) {
    consumer_context_.native_access = this;
}
GameVfsHost::~GameVfsHost() = default;

GameNativeVfsRuntime& GameVfsHost::active_runtime() {
    if (native_operation_failed_)
        throw std::logic_error("Native VFS has an interrupted operation; process retention is required");
    return native_->runtime();
}

void GameVfsHost::phase2(VfsStartupState& state) {
    state.first_time_block_ran = !core_ready_;
    if (state.first_time_block_ran) {
        hardware_probe_ = std::make_unique<GameHardwareProbe>(log_, hardware_probe_commit_);
        hardware_probe_->run();
        invoke_native([&] { native_->initialize_core(); });
        core_ready_ = true;
        factories_registered_ = 3; // physical, FileStore, MPKG completed
        state.factories_registered += 2; // explicit phase-2 registrations
        char buffer[0x100]{};
        const DWORD length = GetCurrentDirectoryA(0xfa, buffer);
        if (!length || length >= 0xfa)
            throw std::runtime_error("Phase-2 current directory exceeds native buffer");
        std::string root(buffer, length);
        root.push_back('\\');
        for (const auto& mount : kVfsStartupMounts) {
            GameMountRecord record;
            record.system_path = mount.system_path == VfsStartupSystemPath::current_directory
                ? root : mount.system_path_literal;
            record.virtual_path = mount.virtual_path;
            record.priority = mount.priority;
            record.ownership = mount.ownership;
            record.device_id = mount.device_id;
            void* provider = invoke_native([&] { return active_runtime().mount(record.system_path.c_str(),
                record.virtual_path.c_str(), static_cast<std::uint32_t>(record.priority),
                record.ownership, static_cast<std::uint32_t>(record.device_id)); });
            record.status = provider ? "created" : "failed";
            mounts_.push_back(std::move(record));
            ++state.mounts_requested;
        }
        invoke_native([&] { active_runtime().scan_phase2_packages(); });
        ++package_scans_completed_;
        invoke_native([&] { active_runtime().scan_phase2_packages(); });
        ++package_scans_completed_;
    }
    invoke_native([&] { active_runtime().register_phase2_search_defaults(); });
    log_.implemented("Phase 2 actual VFS mount/scan/search", "0073d604");
    log_.notef("native VFS loose mounts=%zu fresh package scans=%zu; package entry counts unrecorded",
        mounts_.size(), package_scans_completed_);
}
void GameVfsHost::factory_tail(VfsStartupState& state, bool cached_load) {
    invoke_native([&] { active_runtime().register_archive_factory_tail(cached_load); });
    cached_load_ = cached_load;
    archive_tail_ready_ = true;
    factories_registered_ = 4;
    ++state.factories_registered;
    log_.implemented("Factory tail actual MPAK registry/cache/lock", "0073d94f");
}
void GameVfsHost::phase6(VfsStartupState& state) {
    auto manager = resource_manager_004c1400();
    state.animation_channels_parser_registered = register_type_parser_00b80a50(
        manager, animation_channels_parser_00736dd0());
    manager = resource_manager_004c1400();
    state.bone_parser_registered = register_type_parser_00b80a50(manager, bone_parser_00736ea0());
}
const GameHardwareProbeSummary& GameVfsHost::hardware_probe() const noexcept {
    static const GameHardwareProbeSummary not_run{};
    return hardware_probe_ ? hardware_probe_->summary() : not_run;
}
std::size_t GameVfsHost::registered_parsers() const noexcept {
    return resource_manager_ ? resource_manager_->parsers.size() : 0;
}
std::uint32_t GameVfsHost::failure_site() const noexcept {
    if (!core_ready_) return 0;
    const auto request_site = native_->runtime().file_store_request_failure_site();
    return request_site ? request_site : native_->runtime().name_resolution_failure_site();
}
bool GameVfsHost::exists(const std::string& name) {
    if (!core_ready_) return false;
    return invoke_native([&] { return active_runtime().exists(name.c_str()); });
}
void GameVfsHost::pump_pending() {
    invoke_native([&] { active_runtime().pump_pending(); });
}
bool GameVfsHost::resolve_existing(std::string& name) {
    if (!core_ready_) return false;
    return invoke_native([&] { return active_runtime().resolve_existing(name); });
}
bool GameVfsHost::direct_resolve(const std::string& name, std::string& output) {
    if (!core_ready_) return false;
    return invoke_native([&] { return active_runtime().direct_resolve(name, output); });
}
VfsMemoryOpen GameVfsHost::open(const std::string& name, std::uint32_t flags) {
    VfsMemoryOpen result;
    if (flags != 2 && flags != 0x32)
        return {false, {}, "Production VFS consumers require read-existing flags 2 or 0x32."};
    auto bytes = invoke_native([&] { return active_runtime().read_all(name.c_str(), flags); });
    if (!bytes) return result;
    result.provider_opened = true;
    result.stream = std::make_shared<MemoryStream>(
        memory_stream_from_complete_bytes(bytes->data(), bytes->size()));
    return result;
}
std::vector<std::string> GameVfsHost::enumerate(const std::string& directory,
    const std::string& extension, std::uint32_t flags) {
    return invoke_native([&] { return active_runtime().enumerate(directory.c_str(), extension.c_str(), flags); });
}
std::array<std::uint32_t, 5> GameVfsHost::file_date(const std::string& name) {
    return invoke_native([&] { return active_runtime().file_date(name.c_str()); });
}

VfsStartupObject GameVfsHost::resource_manager_004c1400() {
    // 004c1400 returns the global at 010901c4, constructing a 0x28-byte manager
    // through 00b81040 on the first call. That object is packet
    // `resource_manager_singleton`; the process holds the parser map at
    // manager+8h, which is the one field 00b80a50 touches, and hands back the
    // same object for both 0073db41 and 0073db55 as the native does.
    if (resource_manager_ == nullptr) {
        resource_manager_ = std::make_unique<GameResourceManager>();
    }
    log_.implemented("Phase 6 resource_manager", "004c1400");
    return resource_manager_.get();
}

VfsStartupObject GameVfsHost::animation_channels_parser_00736dd0() {
    if (animation_channels_parser_ == nullptr) {
        animation_channels_parser_ = std::make_unique<GameStructuredParser>(log_,
            kAnimationChannelsParser_00736dd0);
    }
    log_.implemented("Phase 6 animation_channels_parser", "00736dd0");
    return animation_channels_parser_.get();
}

VfsStartupObject GameVfsHost::bone_parser_00736ea0() {
    if (bone_parser_ == nullptr) {
        bone_parser_ = std::make_unique<GameStructuredParser>(log_, kBoneParser_00736ea0);
    }
    log_.implemented("Phase 6 bone_parser", "00736ea0");
    return bone_parser_.get();
}

bool GameVfsHost::register_type_parser_00b80a50(VfsStartupObject manager,
    VfsStartupObject parser) {
    // 00b80a50 appends into the parser map at manager+8h keyed by the string the
    // parser's vtable slot +4h returns. Phase 6 discards the result both times.
    auto* resource_manager = static_cast<GameResourceManager*>(manager);
    auto* structured = static_cast<GameStructuredParser*>(parser);
    if (resource_manager == nullptr || structured == nullptr) {
        log_.unimplemented("Phase 6 register_type_parser", "00b80a50");
        return false;
    }
    const bool registered = resource_manager->parsers.register_parser(*structured);
    log_.implemented("Phase 6 register_type_parser", "00b80a50");
    log_.notef("resource type parser %-18s registered=%d map=%zu",
        structured->identity().type_name, registered ? 1 : 0,
        resource_manager->parsers.size());
    return registered;
}

GameVfsProbeResult GameVfsHost::resolve_and_read(const std::string& requested) {
    GameVfsProbeResult result;
    result.requested = requested;
    if (!ready()) {
        result.error = "no provider manager";
        return result;
    }
    std::string name = requested;
    // 00bdf4c0: normalize, try the mounts directly, then the search candidates.
    if (!resolve_existing_resource_00bdf4c0_fragment(context(),
            unused_registrations_, name)) {
        result.error = "no mounted provider resolves " + requested;
        return result;
    }
    result.resolved_ok = true;
    result.resolved = name;
    // 00bdf310 with flags 2, the read-only mode the startup opens use.
    VfsMemoryOpen opened = open_resource_memory_00bdf310_fragment(context(),
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
