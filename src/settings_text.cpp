#define _CRT_SECURE_NO_WARNINGS // Preserve the recovered fopen/fwrite/fclose sequence.
#include "bsp/settings_text.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <ShlObj.h>
#include <cstdio>
#include <stdexcept>
#include <utility>

namespace bsp {

Win32SettingsTextHost::Win32SettingsTextHost(std::string personal_directory_override)
    : personal_directory_override_(std::move(personal_directory_override)) {}

std::string Win32SettingsTextHost::personal_directory()
{
    if (!personal_directory_override_.empty()) return personal_directory_override_;
    char path[MAX_PATH];
    if (!SHGetSpecialFolderPathA(nullptr, path, CSIDL_PERSONAL, TRUE))
        throw std::runtime_error("SHGetSpecialFolderPathA could not resolve the personal directory");
    return path;
}

void Win32SettingsTextHost::create_directory(const std::string& path)
{
    (void)CreateDirectoryA(path.c_str(), nullptr); // native ignores the result
}

void* Win32SettingsTextHost::open_text_write(const std::string& path)
{
    return std::fopen(path.c_str(), "wt");
}

void Win32SettingsTextHost::write_bytes(void* file, const char* data, std::size_t size)
{
    (void)std::fwrite(data, 1, size, static_cast<std::FILE*>(file));
}

void Win32SettingsTextHost::close_file(void* file)
{
    (void)std::fclose(static_cast<std::FILE*>(file));
}

std::string format_settings_text_008d6170(const GameSettingsBlock& settings,
    const std::vector<LanguageEntry>& languages)
{
    const auto& video = settings.options_file;
    const auto& language = languages[static_cast<std::size_t>(settings.gameplay.language_index_04)].lanfile;
    std::string text = "Language ";
    text.append(language.c_str());
    text += '\n';
    const auto integer = [&text](const char* key, int value) {
        text += key;
        text += std::to_string(value); // stream default %d at00ce3a34
        text += '\n';
    };
    integer("Fullscreen ", video.fullscreen_1e);
    text += "Resolution " + std::to_string(video.width_14) + " "
        + std::to_string(video.height_18) + "\n";
    integer("Vsync ", video.vsync_60);
    integer("ShaderModel ", video.shader_model_88);
    integer("Antialias ", video.antialias_58);
    integer("Clouds ", video.clouds_74);
    integer("Foliage ", video.foliage_86);
    integer("Shadow ", video.shadow_84);
    integer("Reflection ", video.reflection_6c);
    integer("TextureDetail ", video.texture_detail_68);
    integer("ObjectDetail ", video.object_detail_54);
    integer("SoundEnabled ", settings.audio.enabled_24);
    integer("Firewall ", video.firewall_94);
    integer("HardwareReported ", settings.presentation.hardware_reported_95);
    return text;
}

std::string build_options_path_008d5150(SettingsTextHost& host)
{
    std::string path = host.personal_directory();
    path += "\\Battlestations-Pacific";
    host.create_directory(path);
    path += "\\options.txt";
    return path;
}

void write_settings_text_008d6170(const GameSettingsBlock& settings,
    const std::vector<LanguageEntry>& languages, SettingsTextHost& host)
{
    // All values are formatted before folder resolution and opening the file.
    const std::string text = format_settings_text_008d6170(settings, languages);
    const std::string path = build_options_path_008d5150(host);
    void* file = host.open_text_write(path);
    if (file != nullptr) {
        host.write_bytes(file, text.data(), text.size());
        host.close_file(file);
    }
}

void OptionsTextSettingsWriter::write_options_text_008d6170()
{
    write_settings_text_008d6170(settings_, languages_, host_);
}

} // namespace bsp
