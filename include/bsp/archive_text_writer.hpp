#pragma once

#include "bsp/profile_archive.hpp"

#include <cstdio>
#include <cstdint>
#include <string>
#include <string_view>

namespace bsp {

// Addresses: 00BD8C80, 00BD6CC0, 00BD8CD0, 00BD6990, 00BD6AC0,
// 00BD6BD0, 00BD7B30, 00BD7F50, 00BD6100, 00BD49B0.
// Host interfaces for the native 8h writer (vtable00CE4104, depth+4).
// Evidence and ABI/format boundaries: docs/ARCHIVE_TEXT_WRITER.md.
struct ArchiveByteSink {
    virtual ~ArchiveByteSink() = default;
    virtual void write(std::string_view bytes) = 0;
};

// Captures actual uncompressed archive bytes. This is host-owned storage,
// not the game's compressed-block manager behind00BD49B0.
class ArchiveStringSink final : public ArchiveByteSink {
public:
    void write(std::string_view bytes) override;
    const std::string& str() const noexcept { return bytes_; }
    void clear() noexcept { bytes_.clear(); }

private:
    std::string bytes_;
};

// Shared by successive stack writers just as the native globals are shared.
// File/flag changes are observed at each emitted fragment. Owns neither FILE
// nor buffered sink. Use a binary FILE for exact LF and encoded output bytes.
// Native null-FILE routing reaches00BD49B0; the host supplies that byte sink
// explicitly. Missing sinks and failed fwrite throw rather than fake success.
struct ArchiveTextOutput {
    std::FILE* file_0109cee0{};
    bool compact_encoded_0109cee4{};
    ArchiveByteSink* buffered_0109cecc{};

    void emit(std::string_view bytes);
};

// Settings-only operations outside the writer vtable. No successful defaults.
// A profile-only writer needs no services; settings traversal requires both.
struct ArchiveSettingsServices {
    virtual ~ArchiveSettingsServices() = default;
    virtual void write_options_text_008d6170() = 0;
    virtual void write_keyboard_setup(SettingsWriter& writer) = 0;
};

// Reuses the established key and scalar value models. Settings tags0..3 are
// implemented. Native extended value tags4..8 and10 remain outside this interface.
// Destruction has no output, close, flush, or implicit end-section operation.
class ArchiveTextWriter final : public SettingsWriter, public ProfileArchiveWriter {
public:
    explicit ArchiveTextWriter(ArchiveTextOutput& output,
        ArchiveSettingsServices* settings_services = nullptr) noexcept;

    void begin_section(const char* name) override;
    void begin_section(const GuiLuaVariant& key);
    void end_section() override;
    void write_field(const char* key, const SettingsValue& value) override;
    void write_field(const GuiLuaVariant& key, const SettingsValue& value) override;
    void write_options_text_008d6170() override;
    void write_keyboard_setup() override;

    std::uint32_t depth() const noexcept { return depth_; }

private:
    std::string format_key(const GuiLuaVariant& key) const;
    void indent();
    void terminate_row();

    ArchiveTextOutput& output_;
    ArchiveSettingsServices* settings_services_;
    std::uint32_t depth_{};
};

} // namespace bsp
