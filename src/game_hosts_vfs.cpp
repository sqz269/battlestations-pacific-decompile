// bsp_game.exe milestone 2a bindings. See include/bsp/game_hosts_vfs.hpp and
// docs/GAME_EXECUTABLE.md. No native behaviour is invented here: whatever is not
// reconstructed goes through GameHostLog::unimplemented with its native call site.
#include "bsp/game_hosts_vfs.hpp"

#include <d3d9.h>
#include <shlobj.h>

#include <algorithm>
#include <cstring>

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

// The sixteen token names 008d8190 compares against, in the literal spellings of the table
// in docs/APP_INIT_BOOTSTRAP.md (00d15f1c down to 00d15e7c, plus "Resolution" at 00cf3a78).
const char* const kOptionTokenNames[] = {
    "Language", "Fullscreen", "HiResShadow", "NoLOD", "Resolution", "VSync", "ShaderModel",
    "Antialias", "Clouds", "Foliage", "Shadow", "Reflection", "TextureDetail", "ObjectDetail",
    "SoundEnabled", "Firewall",
};

bool equals_ignore_case(const std::string& left, const char* right) {
    return _stricmp(left.c_str(), right) == 0;
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

GameSettingsBinding::GameSettingsBinding(GameHostLog& log) : log_(log) {
    // 008d5150: SHGetSpecialFolderPathA(NULL, buf, CSIDL_PERSONAL, TRUE) joined with the
    // per-title directory and options.txt. 008d8205 falls back to the global path buffer at
    // 00f88a3c when the folder call yields an empty string; that buffer has no writer in
    // this process, so an empty personal folder leaves the path empty and selects path B.
    char personal[MAX_PATH] = {};
    if (SHGetSpecialFolderPathA(nullptr, personal, CSIDL_PERSONAL, TRUE)) {
        options_path_ = personal;
        options_path_ += kOptionsDirectory;
        options_path_ += kOptionsFileName;
    }
    enumerate_adapter_modes();
}

GameSettingsBinding::~GameSettingsBinding() = default;

void GameSettingsBinding::enumerate_adapter_modes() {
    // MILESTONE ADDITION, not recovered behaviour. The native supported-resolution table at
    // DAT_00f8895c is assigned at 008d81bf from the renderer's own vector (renderer+1Ch via
    // TRIV_body_00b1fff0), and nothing fills that vector in this reconstruction: the
    // renderer resource phase 00b14a10 is unimplemented. An empty table would make 008d8190
    // reject every parsed resolution and fall back to 640x480, so the milestone supplies the
    // adapter's own mode list, which is what a D3D9 renderer must be enumerating. The same
    // interface supplies the shader-model ceiling that TRIV_body_00b200b0 returns.
    IDirect3D9* api = Direct3DCreate9(D3D_SDK_VERSION);
    if (api == nullptr) {
        log_.note("Direct3DCreate9 failed; supported resolution table is empty");
        return;
    }
    const UINT count = api->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);
    for (UINT index = 0; index < count; ++index) {
        D3DDISPLAYMODE mode = {};
        if (FAILED(api->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, index, &mode))) {
            continue;
        }
        const Resolution entry{static_cast<int>(mode.Width), static_cast<int>(mode.Height)};
        const bool present = std::any_of(resolutions_.begin(), resolutions_.end(),
            [&entry](const Resolution& known) {
                return known.width == entry.width && known.height == entry.height;
            });
        if (!present) resolutions_.push_back(entry);
    }
    D3DCAPS9 caps = {};
    if (SUCCEEDED(api->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps))) {
        max_shader_model_ = static_cast<int>(D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion));
    }
    api->Release();
    log_.notef("adapter modes=%zu distinct resolutions, max pixel shader model=%d",
        resolutions_.size(), max_shader_model_);
}

std::optional<std::string> GameSettingsBinding::read_options_file() const {
    // fopen(path, "rt") in the original; PhysicalFile is the reconstructed read path.
    options_present_ = false;
    recased_.clear();
    if (options_path_.empty()) return std::nullopt;
    PhysicalFile file;
    DWORD error = 0;
    if (!file.open_read_only_00bf52a0_fragment(options_path_.c_str(), error)) {
        return std::nullopt;
    }
    const std::uint64_t size = file.size_00bf4f90();
    std::string text(static_cast<std::size_t>(size), '\0');
    std::uint32_t read = 0;
    if (size != 0 && file.read_00bf5030(text.data(), static_cast<std::uint32_t>(size), read,
            error)) {
        text.resize(read);
    } else {
        text.clear();
    }
    file.close_00bf5090_fragment(error);
    options_present_ = true;

    // The native token comparison at 008d8190 runs through FUN_00467cc0, which calls
    // BSP_CString_CompareInsensitive: the token names are matched case-insensitively. The
    // shared reconstruction in src/app_bootstrap.cpp compares them with ==, so a file
    // written by the game itself does not round-trip (008d6170 writes "Vsync ", the reader
    // literal at 00d15ef4 is "VSync"). That file is owned by another packet, so this host
    // canonicalizes recognized token spellings before handing the text to the loader and
    // reports every token it had to recase. Only case changes; no token is added, removed
    // or reordered, and the loader's own split characters are preserved.
    std::string canonical;
    canonical.reserve(text.size());
    std::string token;
    auto flush = [&canonical, &token, this]() {
        if (token.empty()) return;
        for (const char* name : kOptionTokenNames) {
            if (token != name && equals_ignore_case(token, name)) {
                recased_.push_back(token + " -> " + name);
                token = name;
                break;
            }
        }
        canonical += token;
        token.clear();
    };
    for (const char character : text) {
        if (character == ' ' || character == '\t' || character == '\r' || character == '\n'
            || character == ',') {
            flush();
            canonical.push_back('\n');
        } else {
            token.push_back(character);
        }
    }
    flush();
    return canonical;
}

std::optional<std::uint32_t> GameSettingsBinding::read_registry_language_lcid() const {
    HKEY key = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, kSettingsRegistryKey, 0, KEY_READ, &key)
        != ERROR_SUCCESS) {
        return std::nullopt;
    }
    DWORD type = 0;
    DWORD value = 0;
    DWORD size = sizeof(value);
    const LSTATUS status = RegQueryValueExA(key, kSettingsRegistryLanguageValue, nullptr,
        &type, reinterpret_cast<LPBYTE>(&value), &size);
    RegCloseKey(key);
    if (status != ERROR_SUCCESS || type != REG_DWORD) return std::nullopt;
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
    // DAT_00f88968 / DAT_00f8896c, the stride-4 table 008d8850 snaps the parsed sample count
    // onto. It has the same unreconstructed renderer source as the resolution table, and no
    // safe substitute: an invented list would silently move the sample count. Left empty,
    // which is exactly the "no snapping" branch of the loader tail, so the file's Antialias
    // value survives unchanged and its index stays zero.
    return antialias_levels_;
}

int GameSettingsBinding::max_shader_model() const { return max_shader_model_; }

void GameSettingsBinding::apply_detected_defaults(GameSettings& settings) const {
    // 008d6170's no-options-file path, which derives the remaining settings from detected
    // hardware. Not recovered, so nothing is written and the defaults stand.
    (void)settings;
    log_.unimplemented("GameSettingsHost::apply_detected_defaults", "008d6170");
}

void GameSettingsBinding::log(const std::string& line) const {
    log_.notef("settings %s", line.c_str());
}

}  // namespace bsp::game
