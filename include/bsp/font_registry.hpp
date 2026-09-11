#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>

struct lua_State;

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
using FontDescriptorVisitor = std::function<bool(FontDescriptor&&, std::string&)>;
// Descriptor conversion only, in the supplied interpreter. Visits each entry
// before advancing lua_next; restores the Lua stack on return or C++ exception.
// Callback failures keep prior visitor effects. Ordinary-table/schema guards
// are host safety boundaries; native malformed-input behavior is not reproduced.
bool visit_font_descriptors_lua(lua_State*, const FontDescriptorVisitor&, std::string& error);
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
