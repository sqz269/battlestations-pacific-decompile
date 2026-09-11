#pragma once

#include "bsp/game_settings.hpp"
#include <cstddef>
#include <string>

namespace bsp {

// Native options.txt persistence, 008d6170 and path builder008d5150.
// ABI/evidence and host projection limits: docs/SETTINGS_TEXT_PERSISTENCE.md.
struct SettingsTextHost {
    virtual ~SettingsTextHost() = default;
    // SHGetSpecialFolderPathA(nullptr, buffer, CSIDL_PERSONAL, TRUE).
    // Native assumes a valid returned directory; the host must provide one.
    virtual std::string personal_directory() = 0;
    virtual void create_directory(const std::string& path) = 0;
    virtual void* open_text_write(const std::string& path) = 0; // fopen(path,"wt")
    virtual void write_bytes(void* file, const char* data, std::size_t size) = 0;
    virtual void close_file(void* file) = 0;
};

// Concrete Windows/CRT service. An explicit personal-directory override lets
// an isolated caller use its own tree. Empty means the Windows personal folder.
class Win32SettingsTextHost final : public SettingsTextHost {
public:
    explicit Win32SettingsTextHost(std::string personal_directory_override = {});
    std::string personal_directory() override;
    void create_directory(const std::string& path) override;
    void* open_text_write(const std::string& path) override;
    void write_bytes(void* file, const char* data, std::size_t size) override;
    void close_file(void* file) override;
private:
    std::string personal_directory_override_;
};

// The native table access assumes language_index_04 is valid. The lanfile
// string is truncated at NUL, as the native stream's C-string append does.
std::string format_settings_text_008d6170(const GameSettingsBlock& settings,
    const std::vector<LanguageEntry>& languages);
std::string build_options_path_008d5150(SettingsTextHost& host);
void write_settings_text_008d6170(const GameSettingsBlock& settings,
    const std::vector<LanguageEntry>& languages, SettingsTextHost& host);

// Supplies the recovered text-persistence call for the archive writer.
// Archive section/value and keyboard methods remain required implementations.
class OptionsTextSettingsWriter : public SettingsWriter {
public:
    OptionsTextSettingsWriter(const GameSettingsBlock& settings,
        const std::vector<LanguageEntry>& languages, SettingsTextHost& host) noexcept
        : settings_(settings), languages_(languages), host_(host) {}
    void write_options_text_008d6170() final;
private:
    const GameSettingsBlock& settings_;
    const std::vector<LanguageEntry>& languages_;
    SettingsTextHost& host_;
};

} // namespace bsp
