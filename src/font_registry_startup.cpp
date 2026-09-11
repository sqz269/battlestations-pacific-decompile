#include "bsp/font_registry_startup.hpp"
#include <cstring>
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
    // 00ac374a/00ac3762: destroy font owners in forward list order. Each font's
    // existing destructor clears glyphs, then GFX and alpha references.
    for (auto& font : fonts_) font.reset();
    if (publication_ && *publication_ == this) *publication_ = nullptr; // 00ac37a6
}

const FontRegistryOwnedFont* FontRegistryStartup::find_font_00ac3570(
    const std::string& name) const noexcept {
    for (const auto& font : fonts_) {
        if (font->descriptor.name.size() == name.size() &&
            (name.empty() || _stricmp(font->descriptor.name.c_str(), name.c_str()) == 0))
            return font.get();
    }
    return nullptr;
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

            std::unique_ptr<FontResources> resources;
            if (!load_resource(appended->descriptor, root, language_path, resources, failure)) {
                if (failure.empty()) failure = "Cannot load font resources: " + appended->descriptor.name;
                return false;
            }
            if (!completed_resources(resources)) {
                failure = "Font resource loader returned incomplete ownership: " + appended->descriptor.name;
                return false;
            }
            appended->resources = std::move(resources); // 00ac3d99, per entry
            return true;
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
