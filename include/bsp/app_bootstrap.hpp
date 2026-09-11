#pragma once

#include "bsp/object_handle_resolvers.hpp"

// Bootstrap phase of cSkeletonAppMidway::Init (0073d410).
//
// Addresses: 00439040, 006ad0d0, 0073ce20, 0073c3b0, 008d8190, 00737c40.
// Evidence: docs/APP_INIT_BOOTSTRAP.md, reports/app_init_bootstrap.json.
//
// Every name below is a hypothesis, not a recovered symbol. These interfaces are
// reconstructions of observed behaviour with explicit arguments; they are not
// drop-in binary replacements and do not reproduce the native ABI. Platform work
// the originals perform inline (GetModuleFileNameA, RegOpenKeyExA, fopen,
// GetWindowRect, MessageBoxW) is injected through the host interfaces so that no
// reconstruction reads the real registry or the real filesystem.

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bsp {

struct GameSettingsBlock;
struct LanguageEntry;

// ---------------------------------------------------------------------------
// 00439040 - module path capture
// ---------------------------------------------------------------------------

// GetModuleFileNameA(NULL, buf, 0x104), __strlwr, _splitpath(buf, drive, dir,
// fname, ext), then strcpy(DAT_00e176c8, drive) followed by strcat(.., dir).
// Only drive+directory survive; the file name and extension are discarded.
// The native buffer is 0x104 bytes and the result is lower-cased.
std::string capture_module_directory_00439040(const std::string& module_file_name);

// ---------------------------------------------------------------------------
// 006ad0d0 - three callback slots, registered through 00bd4fc0
// ---------------------------------------------------------------------------

// The three callbacks and their typed registration now live in
// object_handle_resolvers.hpp.006AD0D0 installs reconstructed callable bodies;
// the actual object table owners/population remain separate dependencies.

// ---------------------------------------------------------------------------
// 0073ce20 - the command line switch table
// ---------------------------------------------------------------------------

enum class AutoTask {
    none,
    package_classes, // "auto mpak classes"
    package_scenes,  // "auto mpak scenes"
    run_test,        // "auto test <name>"
};

// One field per global the native parser writes. Defaults are the values the
// image carries before Init runs, not values the parser itself establishes.
struct CommandLineOptions {
    // DAT_00e1ae76, read at 0073d975 and handed to the VFS provider manager.
    bool cached_load = false;
    // DAT_00e0a538, initialised to 1 in .data and only ever cleared by "nozip".
    // No reader for this byte exists anywhere in the image.
    bool zip_enabled = true;
    // DAT_00e1ae81, read at 0073daaa to gate the timer-service vtable+0x24 call.
    bool fixed_frame_rate = false;
    // DAT_00e1ae75, the shared quit-requested flag read by 00737b4e/00737b65 in
    // BSP_Application_RunFrame; "1frame" raises it before the first frame runs.
    bool quit_requested = false;
    // DAT_00e1ae77, read at 0073d98d to gate the files.txt access log.
    bool file_access_log = false;
    // DAT_00e1ae80, read and cleared by the camera routine 0068c1f0.
    bool free_camera = false;
    // DAT_00f1af38, filled by strcpy from the token tail after "ip:".
    std::string host_address;
    // DAT_00f1af30 = atol(token + 10), read by the network routine 0076fad0.
    long local_port = 0;
    // DAT_00f1af34 = atol(token + 12). No reader found in the image.
    long connect_port = 0;
    // app+0x1a, forwarded late in Init to game+0x719d. The parser writes it only
    // when "memlimit" or "nomemlimit" appears, so absence is distinguishable.
    std::optional<bool> memory_limit;
    AutoTask auto_task = AutoTask::none;
    std::string auto_test_name;
    // "debug:" is matched on its six-character prefix and then discarded; the
    // native jump target is the loop tail. Recorded here so callers can see the
    // switch was recognised.
    bool debug_prefix_seen = false;
    // Tokens the switch table does not claim. The native parser ignores them.
    std::vector<std::string> unrecognized;
};

// 0094ec70: split on the delimiter set " \n\r" (00cff164). Empty fields are not
// produced; the native tokenizer yields a vector<std::string> of 0x1c-byte
// elements walked with a checked iterator.
std::vector<std::string> split_command_line_0094ec70(const std::string& command_line);

// 0073ce20, __thiscall(this) with RET 0. The native reads its input from the
// duplicated command line at DAT_00e1ae78 rather than from an argument; the
// string is passed explicitly here. "auto" consumes one or two following tokens
// from the same iterator, so the switch table is not purely per-token.
CommandLineOptions parse_command_line_0073ce20(const std::string& command_line);

// ---------------------------------------------------------------------------
// 0073c3b0 - the hardware probe
// ---------------------------------------------------------------------------

// HKLM key read by 0098d4e0 on behalf of the probe. Written by the separate
// hardware-detection pass, not by the game.
inline constexpr char kHardwareRegistryKey[] = "SOFTWARE\\Eidos\\BSM_HWD";

struct HardwareProfile {
    // CPUSpeed, MemSize and GPUDeviceID are read as REG_QWORD (type 11) into an
    // eight-byte buffer; only the low dword is kept (0098d5c0).
    std::uint32_t cpu_speed = 0;
    std::uint32_t mem_size = 0;
    std::uint32_t gpu_device_id = 0;
    // SoundDevice is read as REG_SZ (type 1) into a 0x400-byte buffer (0098d5f0).
    std::string sound_device;
};

enum class HardwareProbeResult {
    profile_incomplete, // one of the four values is missing or mistyped
    requirements_met,   // all four present, nothing to warn about
    defaults_written,   // warning shown, user answered Yes, options file written
    defaults_declined,  // warning shown, user answered No
};

class HardwareProbeHost {
public:
    virtual ~HardwareProbeHost() = default;
    // 0098d5c0 -> 0098d4e0 with expected type REG_QWORD. False when the value is
    // absent or carries a different type; the probe treats both the same way.
    virtual bool read_hardware_qword(const std::string& value_name,
        std::uint32_t& out) const = 0;
    // 0098d5f0 -> 0098d4e0 with expected type REG_SZ.
    virtual bool read_hardware_string(const std::string& value_name,
        std::string& out) const = 0;
    // The four requirement comparisons at 0073c493-0073c7ff are not recovered.
    // The host supplies the already-localised failure lines; an empty result
    // means the machine passed and no message box is shown.
    virtual std::vector<std::string> failed_requirements(
        const HardwareProfile& profile) const = 0;
    // MessageBoxW(NULL, text, caption, MB_YESNO | MB_ICONEXCLAMATION); true for
    // IDYES (6). Every other return value is treated as a decline.
    virtual bool ask_write_default_options(const std::string& message) const = 0;
    // 0098f430: fopen(path, "wb") and write the generated option text.
    virtual void write_default_options_file(const std::string& path) const = 0;
};

// Native message layout at 0073c7ff: L"%s%s%s%s\n%s" over the four per-check
// lines plus a trailing localized line. Reconstructed as newline-joined text.
inline constexpr char kDefaultOptionsFileName[] = "options.txt";

HardwareProbeResult probe_hardware_0073c3b0(const HardwareProbeHost& host,
    HardwareProfile& profile);

// ---------------------------------------------------------------------------
// 008d8190 - game settings
// ---------------------------------------------------------------------------

// Read only when the options file cannot be opened.
inline constexpr char kSettingsRegistryKey[] = "SOFTWARE\\Eidos\\Battlestations Pacific";
inline constexpr char kSettingsRegistryLanguageValue[] = "language";

struct Resolution {
    int width = 0;
    int height = 0;
};

// Fields of the static settings object at 00f88980, by native offset. Only the
// members this routine writes are modeled.
struct GameSettings {
    int width_14 = 0;
    int height_18 = 0;
    bool no_lod_1c = false;
    bool hires_shadow_1d = false;
    bool fullscreen_1e = false;
    int object_detail_54 = 0;
    int antialias_58 = 0;
    int antialias_index_5c = 0;
    bool vsync_60 = false;
    int texture_detail_68 = 0;
    bool reflection_6c = false;
    bool clouds_74 = false;
    int resolution_index_78 = 0;
    bool shadow_84 = false;
    bool shadow_85 = false;
    bool foliage_86 = false;
    int shader_model_88 = 0;
    bool firewall_94 = false;
    std::string language;
    // Tokens that reached the "Options: unknown token %s" diagnostic.
    std::vector<std::string> unknown_tokens;
};

// 640x480, the literal pair stored at 008d841f when the parsed resolution is not
// in the supported table.
inline constexpr Resolution kFallbackResolution{0x280, 0x1e0};

// LCID switch at 008d8236. Any value outside the table yields "english".
const char* language_name_for_lcid_008d8190(std::uint32_t lcid);

class GameSettingsHost {
public:
    virtual ~GameSettingsHost() = default;
    virtual void build_language_catalog_008d7bc0() = 0;
    virtual const std::vector<LanguageEntry>& language_catalog() const = 0;
    // Native copies the renderer table into the persistent settings table.
    virtual void copy_supported_resolutions_008d4ea0() = 0;
    // 008d5150 builds <CSIDL_PERSONAL>\...\options.txt; 008d8205 falls back to
    // the global path buffer at 00f88a3c when that string is empty. An empty
    // optional stands for the fopen(path, "rt") failure that selects the
    // registry path below.
    virtual std::optional<std::string> read_options_file() = 0;
    // RegOpenKeyExA(HKEY_LOCAL_MACHINE, kSettingsRegistryKey, 0, KEY_READ) then
    // RegQueryValueExA(kSettingsRegistryLanguageValue). The value is used only
    // when the query succeeds and the type is REG_DWORD.
    virtual std::optional<std::uint32_t> read_registry_language_lcid() const = 0;
    // GetDesktopWindow + GetWindowRect.
    virtual Resolution desktop_size() const = 0;
    // The stride-8 table at DAT_00f8895c with count DAT_00f88960.
    virtual const std::vector<Resolution>& supported_resolutions() const = 0;
    // The stride-4 table at DAT_00f88968 with count DAT_00f8896c, used at
    // 008d8850 to snap the parsed antialias sample count onto a supported one.
    virtual const std::vector<int>& supported_antialias_levels() const = 0;
    // 00b200b0, the renderer's game-specific shader ceiling (one or two). Read
    // to seed +0x88, repair a value below one, and clamp the selected value.
    virtual int max_shader_model() const = 0;
    // Missing input file: persist the retained object BEFORE the common tail.
    virtual void write_options_text_008d6170(const GameSettingsBlock& settings) = 0;
    virtual void select_shader_model_00b200c0(int selected) = 0;
    // Renderer virtual+104 returns the capability record; +28 is the low
    // word of PixelShaderVersion, not the constant-buffer limit.
    virtual std::uint32_t pixel_shader_version_28() const = 0;
    virtual void rebuild_antialias_levels_00b295c0(std::uint32_t format) = 0;
    virtual void copy_supported_antialias_008d4df0() = 0;
    // TRIV_body_004254b0, including the original's "Destop size=%d %d" typo.
    virtual void log(const std::string& line) const;
};

// 008d8190, __fastcall(this) with RET 0; the native `this` is the static object
// at 00f88980. Updates the actual retained object, including its language index.
// AA enumeration uses format21 or113 according to byte+8c. An empty AA table
// or negative final index is outside the native valid domain and throws.
// Evidence and host adaptation limits: docs/SETTINGS_STARTUP_OWNER.md.
void load_game_settings_008d8190(GameSettingsBlock& settings, GameSettingsHost& host);

// ---------------------------------------------------------------------------
// 00737c40 and the bootstrap order
// ---------------------------------------------------------------------------

// 00737c40, __thiscall(this, native string by value) with RET 8. Allocates four
// bytes, chains to the base constructor 007374a0 and stores the vtable
// 00cfeae0. The object is never stored in a global and never freed; the base
// constructor is what registers it. Subject string: "files.txt" (00e0a534).
inline constexpr char kFileAccessLogName[] = "files.txt";

// Ordered record of what the bootstrap phase did, with the guard values the
// later phases of Init read back.
struct BootstrapState {
    std::string module_directory;     // DAT_00e176c8
    bool resolvers_installed = false; // DAT_0109ced4/ced8/cedc
    HardwareProbeResult probe = HardwareProbeResult::profile_incomplete;
    bool probe_ran = false;           // false when the first-time gate skipped it
    CommandLineOptions options;
    bool file_access_log_created = false; // 00737c40 actually constructed
    GameSettings settings;
};

class BootstrapHost {
public:
    virtual ~BootstrapHost() = default;
    // GetModuleFileNameA(NULL, ..) at 00439040.
    virtual std::string module_file_name() const = 0;
    // 006ad0d0.
    virtual void install_object_handle_resolvers() = 0;
    // The string BSP_NativeString_Duplicate stores in DAT_00e1ae78 at 0073d945.
    virtual std::string command_line() const = 0;
    virtual HardwareProbeHost& hardware_probe() = 0;
    virtual GameSettingsHost& game_settings() = 0;
    virtual GameSettingsBlock& retained_game_settings() = 0;
};

// The bootstrap subset of Init in native order. `vfs_first_time` mirrors the
// DAT_0109ceec == 0 guard that decides whether the hardware probe runs, and
// `file_log_singleton_present` mirrors DAT_0109cee8 != 0, which suppresses the
// access log even when the switch asked for it.
BootstrapState run_bootstrap_0073d410(BootstrapHost& host, bool vfs_first_time,
    bool file_log_singleton_present);

} // namespace bsp
