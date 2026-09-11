#include "bsp/language_catalog.hpp"
#include "bsp/native_text_tokens.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
bool language_descriptor_allowed_008d7bc0(const std::string& name, std::uint32_t hint_count) {
    if (hint_count != 0) return true;
    for (const char* prefix : {"lockit/english.lng", "lockit/french.lng",
        "lockit/italian.lng", "lockit/german.lng", "lockit/spanish.lng",
        "lockit/englishauthentic.lng"}) {
        if (name.compare(0, std::char_traits<char>::length(prefix), prefix) == 0) return true;
    }
    return false;
}
LanguageEntry parse_native_language_descriptor_008d7bc0(const std::vector<std::uint8_t>& bytes) {
    NativeTextTokens scanner(bytes);
    LanguageEntry entry;
    for (;;) {
        const auto key = scanner.peek_00bee8e0();
        if (scanner.eof_at_token_start() || (key.empty() && !scanner.quoted())) return entry;
        std::string* target{};
        if (_stricmp(key.c_str(), "lanfile") == 0) target = &entry.lanfile;
        else if (_stricmp(key.c_str(), "lockit_id") == 0) target = &entry.lockit_id;
        else if (_stricmp(key.c_str(), "voice_dir") == 0) target = &entry.voice_dir;
        else if (_stricmp(key.c_str(), "fontpath") == 0) target = &entry.font_path;
        else {
            // 008d8055 loops to peek without accepting an unknown key, so native
            // never advances. Explicit failure bounds this unsupported input.
            throw std::invalid_argument("Unknown language descriptor key would stall native parser: " + key);
        }
        scanner.accept_00bee800();
        bool accepted{};
        *target = scanner.read_string_00bef020(accepted);
        // A missing trailing value reaches EOF and stores the empty string.
        // An empty quoted value stays cached and stalls the next native loop.
        if (!accepted && scanner.quoted() && !scanner.eof_at_token_start())
            throw std::invalid_argument("Empty quoted language descriptor value would stall native parser");
    }
}
void build_language_catalog_008d7bc0(std::vector<LanguageEntry>& table, LanguageCatalogSource& source) {
    if (!table.empty()) return;
    const auto paths = source.enumerate_descriptors_00886280("lockit", ".lng", 0);
    for (const auto& path : paths) {
        std::vector<std::uint8_t> bytes;
        const bool opened = source.read_descriptor_00bef2e0(path, 0x32, bytes);
        if (!language_descriptor_allowed_008d7bc0(path, source.profile_hints_count_08())) continue;
        if (!opened) throw std::runtime_error("Language descriptor VFS open failed: " + path);
        table.push_back(parse_native_language_descriptor_008d7bc0(bytes));
    }
}
}
