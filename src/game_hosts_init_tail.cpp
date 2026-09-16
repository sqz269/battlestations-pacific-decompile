// bsp_game.exe milestone 2c: the Init tail. See include/bsp/game_hosts_init_tail.hpp
// for the address list and the evidence.
#include "bsp/game_hosts_init_tail.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/vfs_mounts.hpp"

#include <string>

namespace bsp::game {
namespace {

// 0098d4e0's key and access mask: RegOpenKeyExA(HKLM, "SOFTWARE\Eidos\BSM_HWD",
// 0x20019) for the reads, RegCreateKeyExA with 0x20006 for the writes.
constexpr char kHardwareRegistryKey[] = "SOFTWARE\\Eidos\\BSM_HWD";
constexpr DWORD kHardwareReadAccess = 0x20019u;
constexpr DWORD kHardwareWriteAccess = 0x20006u;
// 0098d5c0 demands type 0x0b (REG_QWORD) and keeps the low dword at 0098d5e2;
// 0098d5f0 demands type 1 (REG_SZ) into a 0x400-byte buffer.
constexpr DWORD kHardwareQwordType = 0x0bu;
constexpr std::size_t kHardwareStringCapacity = 0x400;

// The read half of the probe, as bsp::probe_hardware_0073c3b0 wants it. The
// decision is not taken here: it belongs to bsp::run_hardware_probe_tail_0073c3b0,
// which models the native message, the two side effects and their ordering.
// src/app_bootstrap.cpp's own failure path joins its lines with no separator and
// models no write-back; see the corrections in docs/APP_INIT_TAIL.md.
class HardwareProbeReads final : public HardwareProbeHost {
public:
    explicit HardwareProbeReads(GameHardwareProbe& owner) : owner_(owner) {}
    bool read_hardware_qword(const std::string& value_name,
        std::uint32_t& out) const override {
        return owner_.read_stored_qword(value_name, out);
    }
    bool read_hardware_string(const std::string& value_name,
        std::string& out) const override {
        return owner_.read_stored_string(value_name, out);
    }
    std::vector<std::string> failed_requirements(const HardwareProfile& profile) const override {
        // Delegated: the four comparisons at 0073c493..0073c7a4 are
        // bsp::hardware_probe_changes_0073c3b0, which the tail runs next.
        static_cast<void>(profile);
        return {};
    }
    bool ask_write_default_options(const std::string& message) const override {
        static_cast<void>(message);
        return false;
    }
    void write_default_options_file(const std::string& path) const override {
        static_cast<void>(path);
    }

private:
    GameHardwareProbe& owner_;
};

}  // namespace

// ---------------------------------------------------------------------------
// GameHardwareProbe, 0073c3b0
// ---------------------------------------------------------------------------

GameHardwareProbe::GameHardwareProbe(GameHostLog& log, bool commit)
    : log_(log), commit_(commit) {
    summary_.commit_enabled = commit;
}

bool GameHardwareProbe::read_stored_qword(const std::string& value_name,
    std::uint32_t& out) const {
    HKEY key = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, kHardwareRegistryKey, 0, kHardwareReadAccess, &key)
        != ERROR_SUCCESS) {
        return false;
    }
    DWORD type = 0;
    std::uint64_t value = 0;
    DWORD size = sizeof(value);
    const LSTATUS status = RegQueryValueExA(key, value_name.c_str(), nullptr, &type,
        reinterpret_cast<LPBYTE>(&value), &size);
    RegCloseKey(key);
    // 0098d4e0 returns true only when the call succeeded and the type matches
    // the one in EDX; 0098d5e2 then keeps the low dword of the eight bytes.
    if (status != ERROR_SUCCESS || type != kHardwareQwordType) return false;
    out = static_cast<std::uint32_t>(value & 0xffffffffu);
    return true;
}

bool GameHardwareProbe::read_stored_string(const std::string& value_name,
    std::string& out) const {
    HKEY key = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, kHardwareRegistryKey, 0, kHardwareReadAccess, &key)
        != ERROR_SUCCESS) {
        return false;
    }
    DWORD type = 0;
    char buffer[kHardwareStringCapacity] = {};
    DWORD size = static_cast<DWORD>(sizeof(buffer));
    const LSTATUS status = RegQueryValueExA(key, value_name.c_str(), nullptr, &type,
        reinterpret_cast<LPBYTE>(buffer), &size);
    RegCloseKey(key);
    if (status != ERROR_SUCCESS || type != REG_SZ) return false;
    buffer[sizeof(buffer) - 1] = '\0';
    out.assign(buffer);
    return true;
}

std::uint32_t GameHardwareProbe::current_gpu_device_id() {
    log_.unimplemented("HardwareProbe::current_gpu_device_id", "0098c870");
    return 0;
}

std::string GameHardwareProbe::gpu_device_name(std::uint32_t id) {
    static_cast<void>(id);
    log_.unimplemented("HardwareProbe::gpu_device_name", "0098c890");
    return {};
}

std::string GameHardwareProbe::current_sound_device() {
    // 0098d3d0 is two instructions, MOV EAX,00f8a0e8 and RET: the detected name
    // is a process-wide buffer the detection object 009927d0 fills.
    log_.unimplemented("HardwareProbe::current_sound_device", "0098d3d0");
    return {};
}

std::uint32_t GameHardwareProbe::current_cpu_speed() {
    log_.unimplemented("HardwareProbe::current_cpu_speed", "0098c7f0");
    return 0;
}

std::uint32_t GameHardwareProbe::current_memory_size() {
    log_.unimplemented("HardwareProbe::current_memory_size", "0098c800");
    return 0;
}

std::string GameHardwareProbe::localized_message(std::int32_t index, const std::string& first,
    const std::string& second) {
    // 00996060 indexes the flat wide table at 00d1e918 as [language + index * 6]
    // and 00735450 formats it with _vswprintf_p_l. Neither the table nor the
    // language row is reconstructed, and phase 2 runs before the locale tables.
    static_cast<void>(first);
    static_cast<void>(second);
    log_.unimplemented("HardwareProbe::localized_message", "00996060");
    log_.notef("hardware probe message %d requested", index);
    return {};
}

bool GameHardwareProbe::ask_write_default_options(const std::string& text,
    const std::string& caption) {
    // 0073c827 is MessageBoxW(NULL, text, caption, MB_YESNO | MB_ICONEXCLAMATION).
    // A frame-limited run has no operator, so the dialog is only raised when the
    // run asked for it; otherwise the answer is the decline branch. This is the
    // milestone's decision, not recovered behaviour.
    log_.implemented("HardwareProbe::ask_write_default_options", "0073c827");
    if (!commit_) {
        log_.note("hardware probe dialog suppressed (no --hardware-probe-commit); "
            "taking the IDNO branch");
        return false;
    }
    const std::wstring wide_text(text.begin(), text.end());
    const std::wstring wide_caption(caption.begin(), caption.end());
    return MessageBoxW(nullptr, wide_text.c_str(), wide_caption.c_str(),
        MB_YESNO | MB_ICONEXCLAMATION) == IDYES;
}

void GameHardwareProbe::write_default_options_file(const std::string& path) {
    // 0098f430 is __thiscall on the detection object: fopen(path, "wb") plus the
    // defaults that object generates. The object is packet
    // `hardware_detection_object`, so there is nothing to write.
    static_cast<void>(path);
    log_.unimplemented("HardwareProbe::write_default_options_file", "0098f430");
}

void GameHardwareProbe::write_hardware_dword(const std::string& value_name,
    std::uint32_t value) {
    log_.implemented("HardwareProbe::write_hardware_dword", "0098d570");
    if (!commit_) {
        log_.notef("hardware write-back %s = %lu suppressed (no --hardware-probe-commit)",
            value_name.c_str(), static_cast<unsigned long>(value));
        return;
    }
    HKEY key = nullptr;
    DWORD disposition = 0;
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, kHardwareRegistryKey, 0, nullptr, 0,
        kHardwareWriteAccess, nullptr, &key, &disposition) != ERROR_SUCCESS) {
        log_.notef("hardware write-back %s failed to open the key", value_name.c_str());
        return;
    }
    // 0098d570 writes four bytes even though the matching read demands
    // REG_QWORD; the asymmetry is the original's.
    RegSetValueExA(key, value_name.c_str(), 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&value), sizeof(value));
    RegCloseKey(key);
}

void GameHardwareProbe::write_hardware_string(const std::string& value_name,
    const std::string& value) {
    log_.implemented("HardwareProbe::write_hardware_string", "0098d590");
    if (!commit_) {
        log_.notef("hardware write-back %s = \"%s\" suppressed (no --hardware-probe-commit)",
            value_name.c_str(), value.c_str());
        return;
    }
    HKEY key = nullptr;
    DWORD disposition = 0;
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, kHardwareRegistryKey, 0, nullptr, 0,
        kHardwareWriteAccess, nullptr, &key, &disposition) != ERROR_SUCCESS) {
        log_.notef("hardware write-back %s failed to open the key", value_name.c_str());
        return;
    }
    // 0098d590 writes strlen + 1 bytes.
    RegSetValueExA(key, value_name.c_str(), 0, REG_SZ,
        reinterpret_cast<const BYTE*>(value.c_str()),
        static_cast<DWORD>(value.size() + 1));
    RegCloseKey(key);
}

void GameHardwareProbe::run() {
    summary_.ran = true;
    HardwareProbeReads reads(*this);
    HardwareProfile profile{};
    const HardwareProbeResult gate = probe_hardware_0073c3b0(reads, profile);
    summary_.stored = profile;
    summary_.stored_values_read = (profile.cpu_speed != 0 ? 1 : 0)
        + (profile.mem_size != 0 ? 1 : 0) + (profile.gpu_device_id != 0 ? 1 : 0)
        + (profile.sound_device.empty() ? 0 : 1);
    if (gate == HardwareProbeResult::profile_incomplete) {
        // 0073c443's all-four gate: a single missing or mistyped value skips the
        // whole comparison silently, so the machine is never inspected.
        summary_.result = HardwareProbeResult::profile_incomplete;
        log_.notef("hardware profile incomplete under HKLM\\%s; the probe returns "
            "without comparing anything", kHardwareRegistryKey);
        return;
    }
    summary_.profile_complete = true;
    HardwareProbeInputs inputs;
    inputs.stored = profile;
    inputs.current.gpu_device_id = current_gpu_device_id();
    inputs.current.sound_device = current_sound_device();
    inputs.current.cpu_speed = current_cpu_speed();
    inputs.current.mem_size = current_memory_size();
    summary_.changes = hardware_probe_changes_0073c3b0(inputs).size();
    summary_.result = run_hardware_probe_tail_0073c3b0(profile, *this);
    log_.notef("hardware probe stored gpu=%lu cpu=%lu mem=%lu sound=\"%s\" changes=%zu "
        "result=%d", static_cast<unsigned long>(profile.gpu_device_id),
        static_cast<unsigned long>(profile.cpu_speed),
        static_cast<unsigned long>(profile.mem_size), profile.sound_device.c_str(),
        summary_.changes, static_cast<int>(summary_.result));
}

// ---------------------------------------------------------------------------
// GameStartupSingletonPublication, 0073fa70 and 00af06a0
// ---------------------------------------------------------------------------

GameStartupSingletonPublication::GameStartupSingletonPublication(GameHostLog& log,
    const StartupSingletonPublication& record, const char* label)
    : log_(log), record_(record), label_(label) {}

void GameStartupSingletonPublication::enter_lifetime_lock() {
    // 00415350 then the singleton lifetime manager's critical section at +10h,
    // with the depth at +18h. The section itself is the manager's, not ours.
    ++depth_;
    log_.implemented("StartupSingleton::enter_lifetime_lock", "00415350");
}

void GameStartupSingletonPublication::register_lifetime(void* instance) {
    // 00bd0c30. The native re-reads the publication global it has just written
    // rather than using the register it still holds.
    static_cast<void>(instance);
    log_.unimplemented("StartupSingleton::register_lifetime", "00bd0c30");
}

void GameStartupSingletonPublication::leave_lifetime_lock() {
    if (depth_ > 0) --depth_;
    log_.implemented("StartupSingleton::leave_lifetime_lock", "00415350+leave");
}

void GameStartupSingletonPublication::publish(void* instance) {
    published_ = instance;
    log_.implemented("StartupSingleton::publish", "0073fac6");
    log_.notef("startup singleton %s published at %08lx (base vtable %08lx)",
        label_ != nullptr ? label_ : "(unnamed)",
        static_cast<unsigned long>(record_.instance_global),
        static_cast<unsigned long>(record_.base_vtable));
}

// ---------------------------------------------------------------------------
// GameDecalTable, 00740840
// ---------------------------------------------------------------------------

GameDecalTable::GameDecalTable(GameHostLog& log, GameVfsHost& vfs) : log_(log), vfs_(vfs) {}
GameDecalTable::~GameDecalTable() = default;

void GameDecalTable::run() {
    // 0073fa70 first: the base vtable and the 00e1aea0 publication. The derived
    // vtable 00cff2a0 that 00740872 installs is packet `decal_system_runtime`.
    GameStartupSingletonPublication publication(log_, kDecalSystemBase_0073fa70,
        "decal system");
    publish_startup_singleton_00af06a0(publication, this);
    summary_.published = publication.published() == this;

    // 00740903 runs the 29-byte path scripts/datatables/decals.lua through
    // 00b69d40, which reads it out of the VFS: 00bdf4c0 resolves the name and
    // 00bdf310 opens it into memory, the same pair every other asset takes.
    std::string text;
    {
        auto* manager = vfs_.ready() ? &vfs_.context() : nullptr;
        std::string resolved = kDecalTablePath;
        if (manager == nullptr
            || !resolve_existing_resource_00bdf4c0_fragment(*manager,
                vfs_.search_registrations(), resolved)) {
            summary_.error = "cannot resolve the decal table";
            log_.notef("decal table %s not resolved", kDecalTablePath);
            log_.unimplemented("Phase 9 decal_definitions", "00740840");
            return;
        }
        VfsMemoryOpen opened = open_resource_memory_00bdf310_fragment(*manager,
            resolved, 2);
        if (!opened.provider_opened || !opened.stream || !opened.stream->fully_initialized()) {
            summary_.error = opened.error.empty() ? "cannot open the decal table" : opened.error;
            log_.notef("decal table %s not read: %s", kDecalTablePath, summary_.error.c_str());
            log_.unimplemented("Phase 9 decal_definitions", "00740840");
            return;
        }
        text.assign(reinterpret_cast<const char*>(opened.stream->data_00bef610()),
            static_cast<std::size_t>(opened.stream->size_00bef600()));
    }
    summary_.script_read = true;

    // 007408a0 builds the loader's own Lua state owner (00b66bd0) and 007408b6
    // opens it with library mask 1 (00b6a020). The repository's stock Lua 5.1.1
    // stands in for the native interpreter, as src/decal_table_probe.cpp does.
    GuiLua51Host lua;
    log_.implemented("DecalTable::construct_lua_state_owner", "007408a0");
    log_.implemented("DecalTable::open_lua_libraries", "007408b6");
    std::string error;
    if (!lua.execute_archive(text, error)) {
        summary_.error = error;
        log_.notef("decals.lua did not run: %s", error.c_str());
        log_.unimplemented("Phase 9 decal_definitions", "00740840");
        return;
    }
    summary_.script_ran = true;
    log_.implemented("DecalTable::run_script", "00740903");

    // 0109EEA4, the CRT's double-to-int selector, queried rather than assumed.
    const bool crt_sse2_conversion =
        IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE;
    definitions_ = load_decal_definitions_00740840(lua, crt_sse2_conversion);
    summary_.definitions = definitions_.size();
    log_.implemented("Phase 9 decal_definitions", "00740840");
    for (const DecalDefinition& record : definitions_) {
        // 00740454 loads decal.mvfm once per record through renderer vtable +34h,
        // 00740b90 the texture through +64h and 00740c50 the shader through +48h.
        // The renderer at 00f8d394 is another owner's, so the three loads record
        // the names the table authored instead of resolving them.
        ++summary_.textures_requested;
        ++summary_.shaders_requested;
        log_.notef("decal %-20s size=%.4f radius=%.4f maxnum=%d texture=%s shader=%s "
            "life=%.4f fade=%.4f", record.name.c_str(),
            static_cast<double>(record.size), static_cast<double>(record.radius),
            record.max_count, record.texture_name.c_str(), record.shader_name.c_str(),
            static_cast<double>(record.life_time),
            static_cast<double>(record.fade_out_time));
    }
    if (!definitions_.empty()) {
        log_.unimplemented("DecalTable::load_vertex_format", "00740454");
        log_.unimplemented("DecalTable::load_texture", "00740b90");
        log_.unimplemented("DecalTable::load_shader", "00740c50");
    }
    log_.notef("decal definitions parsed: %zu", definitions_.size());
}

}  // namespace bsp::game
