#pragma once
#include "bsp/font_resources.hpp"
#include "bsp/game_entry.hpp"
#include "bsp/vfs_lua_scripts.hpp"

namespace bsp {
class NativeFontResources;
class NativeFontResourceOwners;

// Required native00ad4c30 boundary. Bind the existing stream resource loader
// to the application's live device, D3DX imports, VFS and mip setting. A true
// result must supply actual FontResources with both retained image owners.
using FontRegistryResourceLoader = std::function<bool(const FontDescriptor&,
    const std::string& root, const std::string& language_path,
    std::unique_ptr<FontResources>&, std::string& error)>;
// Actual-resource arm. The external canonical owner collection owns the
// returned live resource; its SAME descriptor is the already appended record.
// No semantic FontResources/COM wrapper is required or manufactured. Retire
// Texts, resources and these borrowed bindings before that collection dies.
using FontRegistryNativeResourceLoader = std::function<bool(const FontDescriptor&,
    const std::string& root, const std::string& language_path,
    NativeFontResources*&, std::string& error)>;

struct FontRegistryOwnedFont {
    FontDescriptor descriptor;
    std::string root;
    std::string language_path;
    std::unique_ptr<FontResources> resources;
    NativeFontResources* native_resources{}; // Borrowed canonical association.
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
    // Image mode delegates semantic selection to the same native registry/list
    // used by Text. This aggregate only maps the selected descriptor identity
    // back to its custody row; it never compares descriptor names in this arm.
    void use_native_image_lookup_00ac3570(NativeFontResourceOwners&);
    void clear_retired_image_lookup(const NativeFontResourceOwners&);

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
    // Same stable list/descriptor and per-entry Lua order, actual owner arm.
    // Rejects mixing in semantic loaded records. Append precedes resource
    // loading; failures preserve earlier rows and the incomplete current row.
    bool load_lua_native_descriptors_00ac3910(VfsLuaScriptFiles&, const LuaRuntimeGlobals&,
        const FontRegistryNativeResourceLoader&, const std::string& root,
        const std::string& descriptor, const std::string& language_path,
        std::string& error);
    // Host association cleanup AFTER successful actual font retirement, while
    // descriptor/owner storage remains live. No native release or list erasure.
    void clear_retired_native_binding(const FontDescriptor&, const NativeFontResources&);

    // ECX registry; stack(root, language_path); native RET8. No registry-wide
    // path store: invoke required per-font reload in list order, including when
    // paths compare equal. Failure stops at that font, keeping prior changes.
    bool set_language_path_00ac3610(const std::string& root,
        const std::string& language_path, const FontRegistryResourceReload&,
        std::string& error);

private:
    using OwnedFontLoader = std::function<bool(FontRegistryOwnedFont&, std::string&)>;
    bool load_lua_descriptors_with_loader(VfsLuaScriptFiles&, const LuaRuntimeGlobals&,
        const OwnedFontLoader&, const std::string&, const std::string&,
        const std::string&, std::string&);
    friend FontRegistryStartup* get_font_registry_007371d0(
        FontRegistryStartup*&, SingletonLifetimeManager&);
    FontRegistry registry_;
    std::vector<std::unique_ptr<FontRegistryOwnedFont>> fonts_;
    FontRegistryStartup** publication_{};
    const NativeFontResourceOwners* native_image_lookup_{}; // Borrowed, no release.
    bool native_image_lookup_selected_{};
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
