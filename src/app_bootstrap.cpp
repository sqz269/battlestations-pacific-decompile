// Bootstrap phase of cSkeletonAppMidway::Init (0073d410).
// Addresses: 00439040, 006ad0d0, 0073ce20, 0073c3b0, 008d8190, 00737c40.
// Evidence: docs/APP_INIT_BOOTSTRAP.md, reports/app_init_bootstrap.json.
//
// Reconstruction of observed behaviour. Not an ABI-compatible replacement: the
// originals are __thiscall/__fastcall methods that read and write fixed globals,
// while these take explicit arguments and return values.

#include "bsp/app_bootstrap.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

namespace bsp {
namespace {

// _splitpath splits on the last ':' for the drive and the last separator for the
// directory. 00439040 keeps drive+directory only, so a path with neither yields
// an empty result.
void split_drive_and_directory(const std::string& path, std::string& drive,
    std::string& directory)
{
    drive.clear();
    directory.clear();
    std::string::size_type start = 0;
    const std::string::size_type colon = path.find(':');
    if (colon != std::string::npos) {
        drive = path.substr(0, colon + 1);
        start = colon + 1;
    }
    const std::string::size_type slash = path.find_last_of("\\/");
    if (slash != std::string::npos && slash >= start) {
        directory = path.substr(start, slash - start + 1);
    }
}

// 0094ec70: a delimiter run produces no token, a '"' toggles an in-quotes state
// that suppresses delimiters, and the quote characters stay in the token.
std::vector<std::string> split_on(const std::string& text, const char* delimiters)
{
    std::vector<std::string> tokens;
    const std::string set(delimiters);
    bool in_quotes = false;
    std::string::size_type start = std::string::npos;
    for (std::string::size_type i = 0; i <= text.size(); ++i) {
        const char c = i < text.size() ? text[i] : '\0';
        if (i < text.size() && c == '"') {
            in_quotes = !in_quotes;
        }
        const bool is_break = i == text.size()
            || (!in_quotes && set.find(c) != std::string::npos);
        if (is_break) {
            if (start != std::string::npos) {
                tokens.push_back(text.substr(start, i - start));
                start = std::string::npos;
            }
        } else if (start == std::string::npos) {
            start = i;
        }
    }
    return tokens;
}

bool has_prefix(const std::string& token, const char* prefix)
{
    const std::string p(prefix);
    return token.size() >= p.size() && token.compare(0, p.size(), p) == 0;
}

// _atol: leading whitespace and sign, decimal digits, no error reporting.
long parse_long(const std::string& text)
{
    return std::strtol(text.c_str(), nullptr, 10);
}

std::string to_decimal(int value)
{
    return std::to_string(value);
}

} // namespace

// ---------------------------------------------------------------------------
// 00439040
// ---------------------------------------------------------------------------

std::string capture_module_directory_00439040(const std::string& module_file_name)
{
    // GetModuleFileNameA writes at most 0x104 bytes including the terminator.
    std::string path = module_file_name.substr(0,
        std::min<std::string::size_type>(module_file_name.size(), 0x103));
    // __strlwr is byte-wise tolower over the whole buffer.
    for (char& c : path) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    std::string drive;
    std::string directory;
    split_drive_and_directory(path, drive, directory);
    // strcpy(DAT_00e176c8, drive) then strcat(DAT_00e176c8, directory).
    return drive + directory;
}

// ---------------------------------------------------------------------------
// 006ad0d0
// ---------------------------------------------------------------------------

void install_object_handle_resolvers_006ad0d0(ObjectHandleResolverSlots& slots,
    const void* handle_to_object, const void* context_value,
    const void* object_to_handle)
{
    slots.handle_to_object = handle_to_object;
    slots.context_value = context_value;
    slots.object_to_handle = object_to_handle;
}

// ---------------------------------------------------------------------------
// 0073ce20
// ---------------------------------------------------------------------------

std::vector<std::string> split_command_line_0094ec70(const std::string& command_line)
{
    return split_on(command_line, " \n\r");
}

CommandLineOptions parse_command_line_0073ce20(const std::string& command_line)
{
    CommandLineOptions options;
    const std::vector<std::string> tokens = split_command_line_0094ec70(command_line);

    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];

        if (token == "cachedload") {
            options.cached_load = true;
        } else if (token == "nozip") {
            options.zip_enabled = false;
        } else if (token == "fixfps") {
            options.fixed_frame_rate = true;
        } else if (token == "1frame") {
            options.quit_requested = true;
        } else if (token == "filelog") {
            options.file_access_log = true;
        } else if (token == "freecam") {
            options.free_camera = true;
        } else if (has_prefix(token, "ip:")) {
            // strcpy into the fixed buffer at 00f1af38; no bound is checked.
            options.host_address = token.substr(3);
        } else if (has_prefix(token, "localport:")) {
            options.local_port = parse_long(token.substr(10));
        } else if (has_prefix(token, "connectport:")) {
            options.connect_port = parse_long(token.substr(12));
        } else if (has_prefix(token, "debug:")) {
            // Recognised and discarded; the native branch jumps to the loop tail.
            options.debug_prefix_seen = true;
        } else if (token == "auto") {
            // The native advances the same iterator, so the follower tokens are
            // consumed whether or not they are recognised.
            if (i + 1 >= tokens.size()) {
                break;
            }
            const std::string& mode = tokens[++i];
            if (mode == "mpak") {
                // Set before the third token is even read (0073d231).
                options.file_access_log = true;
                if (i + 1 >= tokens.size()) {
                    break;
                }
                const std::string& subject = tokens[++i];
                if (subject == "classes") {
                    options.auto_task = AutoTask::package_classes;
                } else if (subject == "scenes") {
                    options.auto_task = AutoTask::package_scenes;
                }
            } else if (mode == "test") {
                if (i + 1 >= tokens.size()) {
                    break;
                }
                options.auto_test_name = tokens[++i];
                options.auto_task = AutoTask::run_test;
            }
        } else if (token == "memlimit") {
            options.memory_limit = true;
        } else if (token == "nomemlimit") {
            options.memory_limit = false;
        } else {
            options.unrecognized.push_back(token);
        }
    }
    return options;
}

// ---------------------------------------------------------------------------
// 0073c3b0
// ---------------------------------------------------------------------------

HardwareProbeResult probe_hardware_0073c3b0(const HardwareProbeHost& host,
    HardwareProfile& profile)
{
    profile = HardwareProfile{};
    // The native sums four booleans and requires exactly 4, so a single missing
    // or mistyped value skips the whole check silently.
    int present = 0;
    present += host.read_hardware_qword("MemSize", profile.mem_size) ? 1 : 0;
    present += host.read_hardware_qword("CPUSpeed", profile.cpu_speed) ? 1 : 0;
    present += host.read_hardware_qword("GPUDeviceID", profile.gpu_device_id) ? 1 : 0;
    present += host.read_hardware_string("SoundDevice", profile.sound_device) ? 1 : 0;
    if (present != 4) {
        return HardwareProbeResult::profile_incomplete;
    }

    const std::vector<std::string> failures = host.failed_requirements(profile);
    if (failures.empty()) {
        return HardwareProbeResult::requirements_met;
    }

    std::string message;
    for (const std::string& line : failures) {
        message += line;
    }
    message += '\n';
    if (!host.ask_write_default_options(message)) {
        return HardwareProbeResult::defaults_declined;
    }
    host.write_default_options_file(kDefaultOptionsFileName);
    return HardwareProbeResult::defaults_written;
}

// ---------------------------------------------------------------------------
// 008d8190
// ---------------------------------------------------------------------------

void GameSettingsHost::log(const std::string&) const {}

const char* language_name_for_lcid_008d8190(std::uint32_t lcid)
{
    switch (lcid) {
    case 0x407: return "german";
    case 0x40a: return "spanish";
    case 0x40c: return "french";
    case 0x410: return "italian";
    default: return "english";
    }
}

namespace {

// Both index lookups walk the whole table. The no-options-file resolution lookup
// breaks on the first match; the options-file one does not, so the last match
// wins there. Both leave the index untouched when nothing matches.
int first_matching_resolution(const std::vector<Resolution>& table, int width, int height,
    int fallback_index)
{
    for (std::size_t i = 0; i < table.size(); ++i) {
        if (table[i].width == width && table[i].height == height) {
            return static_cast<int>(i);
        }
    }
    return fallback_index;
}

int last_matching_resolution(const std::vector<Resolution>& table, int width, int height,
    int fallback_index)
{
    int index = fallback_index;
    for (std::size_t i = 0; i < table.size(); ++i) {
        if (table[i].width == width && table[i].height == height) {
            index = static_cast<int>(i);
        }
    }
    return index;
}

namespace {
// 008d8190 compares each option token through 00467cc0, which calls
// BSP_CString_CompareInsensitive; the game's own writer emits `Vsync` while the
// reader literal at 00d15ef4 is `VSync`, so the comparison must fold case for a
// file the game wrote to round-trip (docs/GAME_EXECUTABLE.md, milestone 2a).
bool options_token_is(const std::string& name, const char* literal) noexcept {
    std::string::size_type i = 0;
    for (; literal[i] != '\0'; ++i) {
        if (i >= name.size()) return false;
        const unsigned char a = static_cast<unsigned char>(name[i]);
        const unsigned char b = static_cast<unsigned char>(literal[i]);
        if (std::tolower(a) != std::tolower(b)) return false;
    }
    return i == name.size();
}
}

void apply_options_token(GameSettings& settings, const std::string& name,
    const std::string& value, const std::vector<Resolution>& resolutions,
    std::vector<std::string>::size_type& consumed_extra,
    const std::string* second_value)
{
    consumed_extra = 0;
    if (options_token_is(name, "Language")) {
        settings.language = value;
    } else if (options_token_is(name, "Fullscreen")) {
        settings.fullscreen_1e = parse_long(value) != 0;
    } else if (options_token_is(name, "HiResShadow")) {
        settings.hires_shadow_1d = parse_long(value) != 0;
    } else if (options_token_is(name, "NoLOD")) {
        settings.no_lod_1c = parse_long(value) != 0;
    } else if (options_token_is(name, "Resolution")) {
        settings.width_14 = static_cast<int>(parse_long(value));
        settings.height_18 = second_value
            ? static_cast<int>(parse_long(*second_value)) : 0;
        consumed_extra = 1;
        if (first_matching_resolution(resolutions, settings.width_14,
                settings.height_18, -1) == -1) {
            settings.width_14 = kFallbackResolution.width;
            settings.height_18 = kFallbackResolution.height;
        }
        settings.resolution_index_78 = last_matching_resolution(resolutions,
            settings.width_14, settings.height_18, settings.resolution_index_78);
    } else if (options_token_is(name, "VSync")) {
        settings.vsync_60 = parse_long(value) != 0;
    } else if (options_token_is(name, "ShaderModel")) {
        settings.shader_model_88 = static_cast<int>(parse_long(value));
    } else if (options_token_is(name, "Antialias")) {
        settings.antialias_index_5c = 0;
        settings.antialias_58 = static_cast<int>(parse_long(value));
    } else if (options_token_is(name, "Clouds")) {
        settings.clouds_74 = parse_long(value) != 0;
    } else if (options_token_is(name, "Foliage")) {
        settings.foliage_86 = parse_long(value) != 0;
    } else if (options_token_is(name, "Shadow")) {
        settings.shadow_84 = parse_long(value) != 0;
        settings.shadow_85 = settings.shadow_84;
    } else if (options_token_is(name, "Reflection")) {
        settings.reflection_6c = parse_long(value) != 0;
    } else if (options_token_is(name, "TextureDetail")) {
        settings.texture_detail_68 = static_cast<int>(parse_long(value));
    } else if (options_token_is(name, "ObjectDetail")) {
        settings.object_detail_54 = static_cast<int>(parse_long(value));
    } else if (options_token_is(name, "SoundEnabled")) {
        // Recognised, its value consumed, and then discarded: the native jumps
        // straight back to the loop head with no store (008d85d9).
    } else if (options_token_is(name, "Firewall")) {
        settings.firewall_94 = parse_long(value) != 0;
    } else {
        // "Options: unknown token %s"
        settings.unknown_tokens.push_back(name);
    }
}

} // namespace

GameSettings load_game_settings_008d8190(const GameSettingsHost& host)
{
    GameSettings settings;
    const std::vector<Resolution>& resolutions = host.supported_resolutions();
    settings.shader_model_88 = host.max_shader_model();

    const std::optional<std::string> text = host.read_options_file();
    if (!text) {
        // fopen failed: derive everything from the desktop and the registry.
        settings.fullscreen_1e = true;
        const Resolution desktop = host.desktop_size();
        settings.width_14 = desktop.width;
        settings.height_18 = desktop.height;
        // The original's own spelling.
        host.log("Destop size=" + to_decimal(settings.width_14) + " "
            + to_decimal(settings.height_18));
        settings.resolution_index_78 = first_matching_resolution(resolutions,
            settings.width_14, settings.height_18, settings.resolution_index_78);
        if (const std::optional<std::uint32_t> lcid = host.read_registry_language_lcid()) {
            settings.language = language_name_for_lcid_008d8190(*lcid);
        }
        host.apply_detected_defaults(settings);
    } else {
        // The whole file is read into a memory backing and walked as tokens.
        const std::vector<std::string> tokens = split_on(*text, " \t\r\n,");
        for (std::size_t i = 0; i + 1 < tokens.size(); i += 2) {
            const std::string* second = i + 2 < tokens.size() ? &tokens[i + 2] : nullptr;
            std::vector<std::string>::size_type extra = 0;
            apply_options_token(settings, tokens[i], tokens[i + 1], resolutions,
                extra, second);
            i += extra;
        }
    }

    // Tail, run on both paths.
    if (settings.shader_model_88 < 1) {
        settings.shader_model_88 = host.max_shader_model();
    }
    settings.shader_model_88 = std::min(settings.shader_model_88, host.max_shader_model());

    const std::vector<int>& levels = host.supported_antialias_levels();
    if (!levels.empty()) {
        // No break: the last matching entry wins.
        for (std::size_t i = 0; i < levels.size(); ++i) {
            if (levels[i] == settings.antialias_58) {
                settings.antialias_index_5c = static_cast<int>(i);
            }
        }
        if (static_cast<int>(levels.size()) <= settings.antialias_index_5c) {
            settings.antialias_index_5c = static_cast<int>(levels.size()) - 1;
        }
        settings.antialias_58 = levels[static_cast<std::size_t>(settings.antialias_index_5c)];
    }
    return settings;
}

// ---------------------------------------------------------------------------
// Bootstrap order
// ---------------------------------------------------------------------------

BootstrapState run_bootstrap_0073d410(BootstrapHost& host, bool vfs_first_time,
    bool file_log_singleton_present)
{
    BootstrapState state;

    // 0073d458
    state.module_directory = capture_module_directory_00439040(host.module_file_name());
    // 0073d4bd
    host.install_object_handle_resolvers();
    state.resolvers_installed = true;
    // 0073d610, inside the DAT_0109ceec == 0 branch.
    if (vfs_first_time) {
        HardwareProfile profile;
        state.probe = probe_hardware_0073c3b0(host.hardware_probe(), profile);
        state.probe_ran = true;
    }
    // 0073d94a, after the whole VFS block.
    state.options = parse_command_line_0073ce20(host.command_line());
    // 0073d98d: the switch alone is not enough, the singleton must still be null.
    state.file_access_log_created =
        state.options.file_access_log && !file_log_singleton_present;
    // 0073daa5
    state.settings = load_game_settings_008d8190(host.game_settings());
    return state;
}

} // namespace bsp
