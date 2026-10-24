#include "bsp/font_registry_startup.hpp"
#include "bsp/native_font_resources.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
bool valid_paths(const std::string& root, const std::string& language_path,
    std::string& error) {
    if (root.find('\0') == std::string::npos &&
        language_path.find('\0') == std::string::npos) return true;
    error = "Embedded-NUL font paths are outside the startup projection.";
    return false;
}
bool completed_resources(const std::unique_ptr<FontResources>& resources) {
    return resources && resources->gfx && resources->alpha;
}
}

FontRegistryStartup::~FontRegistryStartup() {
    // Actual resources borrow these exact descriptors. Their producer must
    // retire them and clear the associations before descriptor destruction.
    for (const auto& font : fonts_) if (font->native_resources) std::terminate();
    if (native_image_lookup_) std::terminate();
    // 00ac374a/00ac3762: destroy font owners in forward list order. Each font's
    // existing destructor clears glyphs, then GFX and alpha references.
    for (auto& font : fonts_) font.reset();
    if (publication_ && *publication_ == this) *publication_ = nullptr; // 00ac37a6
}

const FontRegistryOwnedFont* FontRegistryStartup::find_font_00ac3570(
    const std::string& name) const noexcept {
    if (native_image_lookup_selected_) {
        if (!native_image_lookup_ || !native_image_lookup_->native_registry_image()) return nullptr;
        // Read-only eight-byte NativeString input header. The lookup reads the
        // actual list and Font+0C name cells; this adapter owns no native string.
        struct NameHeader8 { std::uint32_t length; const char* data; };
        static_assert(sizeof(NameHeader8) == 8);
        const NameHeader8 incoming{static_cast<std::uint32_t>(name.size()), name.c_str()};
        const auto* selected = native_image_lookup_->find_native_font_00ac3570(&incoming);
        if (!selected) return nullptr;
        for (const auto& font : fonts_)
            if (&font->descriptor == &selected->descriptor()) return font.get();
        return nullptr;
    }
    for (const auto& font : fonts_) {
        if (font->descriptor.name.size() == name.size() &&
            (name.empty() || _stricmp(font->descriptor.name.c_str(), name.c_str()) == 0))
            return font.get();
    }
    return nullptr;
}

void FontRegistryStartup::use_native_image_lookup_00ac3570(NativeFontResourceOwners& owners) {
    if (native_image_lookup_selected_ || !owners.native_registry_image())
        throw std::logic_error("Native font lookup requires a fresh complete image registry");
    for (const auto& font : fonts_) {
        if (!font->native_resources || &owners.resolve(&font->descriptor) != font->native_resources)
            throw std::logic_error("Native font lookup requires the same complete custody bindings");
    }
    native_image_lookup_ = &owners;
    native_image_lookup_selected_ = true;
}

void FontRegistryStartup::clear_retired_image_lookup(const NativeFontResourceOwners& owners) {
    if (!native_image_lookup_selected_) return; // Fresh, never-loaded application.
    if (native_image_lookup_ != &owners || owners.native_registry_image())
        throw std::logic_error("Native font lookup cleanup requires its retired image registry");
    for (const auto& font : fonts_)
        if (font->native_resources)
            throw std::logic_error("Native font lookup cleanup requires retired custody bindings");
    native_image_lookup_ = nullptr;
    // Keep the selected arm after retirement: never fall back to host names.
}

bool FontRegistryStartup::load_lua_descriptors_00ac3910(VfsLuaScriptFiles& files,
    const LuaRuntimeGlobals& globals, const FontRegistryResourceLoader& load_resource,
    const std::string& root, const std::string& descriptor,
    const std::string& language_path, std::string& error) {
    error.clear();
    if (!load_resource) {
        error = "Font registry startup requires the native font resource load boundary.";
        return false;
    }
    for (const auto& font : fonts_) {
        if (font->native_resources) {
            error = "Cannot append semantic font resources to an actual native font registry.";
            return false;
        }
    }
    return load_lua_descriptors_with_loader(files, globals,
        [&](FontRegistryOwnedFont& font, std::string& failure) {
            std::unique_ptr<FontResources> resources;
            if (!load_resource(font.descriptor, root, language_path, resources, failure)) {
                if (failure.empty()) failure = "Cannot load font resources: " + font.descriptor.name;
                return false;
            }
            if (!completed_resources(resources)) {
                failure = "Font resource loader returned incomplete ownership: " + font.descriptor.name;
                return false;
            }
            font.resources = std::move(resources);
            return true;
        }, root, descriptor, language_path, error);
}

bool FontRegistryStartup::load_lua_native_descriptors_00ac3910(VfsLuaScriptFiles& files,
    const LuaRuntimeGlobals& globals, const FontRegistryNativeResourceLoader& load_resource,
    const std::string& root, const std::string& descriptor,
    const std::string& language_path, std::string& error) {
    error.clear();
    if (!load_resource) {
        error = "Actual font registry startup requires its native resource loader.";
        return false;
    }
    for (const auto& font : fonts_) {
        if (font->resources) {
            error = "Cannot append actual native fonts to a semantic resource registry.";
            return false;
        }
    }
    return load_lua_descriptors_with_loader(files, globals,
        [&](FontRegistryOwnedFont& font, std::string& failure) {
            NativeFontResources* resources = nullptr;
            if (!load_resource(font.descriptor, root, language_path, resources, failure)) {
                if (failure.empty()) failure = "Cannot load actual font resources: " + font.descriptor.name;
                return false;
            }
            if (!resources || !resources->live() || &resources->descriptor() != &font.descriptor) {
                failure = "Native font loader did not publish the same live descriptor association: " + font.descriptor.name;
                return false;
            }
            font.native_resources = resources;
            return true;
        }, root, descriptor, language_path, error);
}

void FontRegistryStartup::clear_retired_native_binding(const FontDescriptor& descriptor,
    const NativeFontResources& resources) {
    for (auto& font : fonts_) {
        if (&font->descriptor != &descriptor) continue;
        if (font->native_resources != &resources || !resources.retired() ||
            &resources.descriptor() != &descriptor)
            throw std::logic_error("Native font binding cleanup requires the same retired resource owner");
        font->native_resources = nullptr;
        return;
    }
    throw std::logic_error("Native font binding cleanup requires its stable owned descriptor");
}

bool FontRegistryStartup::load_lua_descriptors_with_loader(VfsLuaScriptFiles& files,
    const LuaRuntimeGlobals& globals, const OwnedFontLoader& load_resource,
    const std::string& root, const std::string& descriptor,
    const std::string& language_path, std::string& error) {
    error.clear();
    if (!valid_paths(root, language_path, error)) return false;
    if (descriptor.find('\0') != std::string::npos) {
        error = "Embedded-NUL font descriptor paths are outside the startup projection.";
        return false;
    }
    if (registry_.fonts.size() != fonts_.size()) {
        error = "The font registry compatibility view was externally modified.";
        return false;
    }

    LuaScriptRuntime runtime(files);
    PcStorageLuaOwner lua(files.owner_environment(runtime, globals));
    lua.open_storage_archive_00b6a020(1); // 00ac3941..00ac3951
    const auto outcomes = runtime.run_file(lua.storage_lua_38(), descriptor, false);
    // Fundamentals is executed by the owner from its cached source. This view
    // records successful top-level descriptor/override chunks only, not nested
    // DoFile activity (the common runtime owns that execution).
    for (const auto& outcome : outcomes)
        if (outcome.called) registry_.executed_paths.push_back(outcome.path);

    return visit_font_descriptors_lua(lua.storage_lua_38(),
        [&](FontDescriptor&& value, std::string& failure) {
            auto font = std::make_unique<FontRegistryOwnedFont>();
            font->descriptor = std::move(value);
            font->root = root;
            font->language_path = language_path;
            // Both reserves precede publication so allocation cannot leave the
            // compatibility view and owned font sequence with different sizes.
            fonts_.reserve(fonts_.size() + 1);
            registry_.fonts.push_back(font->descriptor);
            auto* appended = font.get();
            fonts_.push_back(std::move(font)); // 00ac3b92..00ac3bcc

            return load_resource(*appended, failure); // 00ac3d99, per entry
        }, error);
}

bool FontRegistryStartup::set_language_path_00ac3610(const std::string& root,
    const std::string& language_path, const FontRegistryResourceReload& reload,
    std::string& error) {
    error.clear();
    if (!reload) {
        error = "Font language changes require the native per-font reload boundary.";
        return false;
    }
    if (!valid_paths(root, language_path, error)) return false;
    for (auto& font : fonts_) {
        if (!completed_resources(font->resources)) {
            error = "Cannot reload an incompletely loaded font: " + font->descriptor.name;
            return false;
        }
        if (!reload(*font, root, language_path, error)) {
            if (error.empty()) error = "Cannot reload font resources: " + font->descriptor.name;
            return false;
        }
        if (!completed_resources(font->resources)) {
            error = "Font reload returned incomplete ownership: " + font->descriptor.name;
            return false;
        }
    }
    return true;
}

FontRegistryStartup* get_font_registry_007371d0(FontRegistryStartup*& published,
    SingletonLifetimeManager& lifetime) {
    if (!published) {
        lifetime.lock();
        struct Unlock {
            SingletonLifetimeManager& manager;
            ~Unlock() { manager.unlock(); }
        } unlock{lifetime};
        if (!published) {
            published = new FontRegistryStartup(); // native allocation10h +00ac3690
            published->publication_ = &published;
            lifetime.register_object(published);  // 00737251 publication precedes00737264
        }
    }
    return published;
}
} // namespace bsp
