#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace bsp {
using FontScriptResolver = std::function<bool(const std::string&, std::string&, std::string&)>;
struct FontDescriptor {
    std::string name;
    float scale_ratio{1.0f};
    float alpha_texture_scale{1.0f};
    bool uppercase_only{};
    std::string data_file, gfx_file;
    std::string alpha_texture{"white.tga"};
};
struct FontRegistry {
    std::vector<FontDescriptor> fonts;
    std::vector<std::string> executed_paths;
};
// Stock Lua adapter for descriptor portion of 00ac3910. Base library, native
// PC/X360COMP/REGION inputs, DoFile and fundamentals; no native VFS ownership.
// Appends in lua_next order. Reported failure preserves the supplied registry.
// Ordinary tables only: metatables and malformed required values are rejected.
bool load_font_registry_lua(const FontScriptResolver& resolver,
    const std::string& descriptor, bool x360comp,
    const std::optional<std::string>& region, FontRegistry& output,
    std::string& error);
// Length equality followed by CRT case-insensitive comparison, first match.
// Returned pointer is invalidated by registry vector mutations.
const FontDescriptor* find_font_00ac3570(const FontRegistry& registry,
    const std::string& name) noexcept;
}
