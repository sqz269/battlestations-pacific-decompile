#pragma once
#include "bsp/font_resources.hpp"
#include "bsp/game_entry.hpp"
#include "bsp/vfs_lua_scripts.hpp"

namespace bsp {

// Required native00ad4c30 boundary. Bind the existing stream resource loader
// to the application's live device, D3DX imports, VFS and mip setting. A true
// result must supply actual FontResources with both retained image owners.
using FontRegistryResourceLoader = std::function<bool(const FontDescriptor&,
    const std::string& root, const std::string& language_path,
    std::unique_ptr<FontResources>&, std::string& error)>;

struct FontRegistryOwnedFont {
    FontDescriptor descriptor;
    std::string root;
    std::string language_path;
    std::unique_ptr<FontResources> resources;
};

// Native00ad51d0 checks saved root/extra, then destructively clears old glyphs
// and images before loading changed paths. The supplied callback owns that
// behavior and updates saved paths; no initial-load-as-reload default exists.
using FontRegistryResourceReload = std::function<bool(FontRegistryOwnedFont&,
    const std::string& root, const std::string& language_path, std::string& error)>;

// Owning C++ startup composition of00ac3690/00ac3910. Direct construction is
// suitable for application RAII; use the separate getter only with a real
// singleton lifetime manager. Native layout, pool and intrusive ABI omitted.
class FontRegistryStartup {
public:
    FontRegistryStartup() = default;
    ~FontRegistryStartup();
    FontRegistryStartup(const FontRegistryStartup&) = delete;
    FontRegistryStartup& operator=(const FontRegistryStartup&) = delete;

    // Compatibility view for GuiStartupHost. The caller must not mutate it;
    // actual font objects/resources are owned by this aggregate.
    FontRegistry& registry() noexcept { return registry_; }
    const FontRegistry& registry() const noexcept { return registry_; }
    const std::vector<std::unique_ptr<FontRegistryOwnedFont>>& fonts() const noexcept {
        return fonts_;
    }
    // Equal byte length and CRT case-insensitive comparison; first match.
    // Font identity remains stable across appends. A failed appended font has
    // null resources; callers must check it. Valid until owner destruction.
    const FontRegistryOwnedFont* find_font_00ac3570(const std::string&) const noexcept;

    // ECX registry; stack(root, descriptor, language_path); native RET0Ch.
    // Fresh mask1 Lua owner with cached fundamentals, runtime DoFile and VFS
    // overrides; closes after iteration. Resource load occurs per lua_next
    // entry, after append and before advancing to the next key. Repeated calls
    // append. Earlier fonts survive a later false return or C++ exception.
    // Ordinary-table conversion completes before append (host safety boundary);
    // native appends before reading Data/GFX/AlphaTexture. A factory failure
    // leaves an owned descriptor with null resources, safely destructible here.
    bool load_lua_descriptors_00ac3910(VfsLuaScriptFiles&, const LuaRuntimeGlobals&,
        const FontRegistryResourceLoader&, const std::string& root,
        const std::string& descriptor, const std::string& language_path,
        std::string& error);

    // ECX registry; stack(root, language_path); native RET8. No registry-wide
    // path store: invoke required per-font reload in list order, including when
    // paths compare equal. Failure stops at that font, keeping prior changes.
    bool set_language_path_00ac3610(const std::string& root,
        const std::string& language_path, const FontRegistryResourceReload&,
        std::string& error);

private:
    friend FontRegistryStartup* get_font_registry_007371d0(
        FontRegistryStartup*&, SingletonLifetimeManager&);
    FontRegistry registry_;
    std::vector<std::unique_ptr<FontRegistryOwnedFont>> fonts_;
    FontRegistryStartup** publication_{};
};

// Getter007371d0: cdecl/no args natively, EAX registry, plain RET. `published`
// projects DAT_00f8bf44. Double check under the supplied real lifetime lock,
// construct an empty registry, publish before registering. The manager must
// dispatch deletion of this concrete owner; its destructor clears publication,
// matching00ac37a6. The published pointer storage must outlive the owner. Caller
// must unregister before deleting outside teardown. No private fake manager.
// Host allocation throws instead of native null allocation/registration path.
FontRegistryStartup* get_font_registry_007371d0(FontRegistryStartup*& published,
    SingletonLifetimeManager&);

} // namespace bsp
