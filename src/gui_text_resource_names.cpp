#include "bsp/gui_text_resource_names.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void require_owner(GuiTextLifetime& lifetime, GuiTextResourceNameServices& services) {
    auto binding = lifetime.content_binding();
    require(binding.widget.layout().type == GuiWidgetType::Text &&
        binding.widget.layout().transform.type_id == 3 &&
        &services.widgets.owner(binding.widget.layout()) == &binding.widget &&
        &services.widgets.environment().models.retained_owners == &services.actual_owners,
        "Text resource names require the same canonical Text and actual resource domain");
}
void require_name(const std::string& name) {
    require(name.size() <= (std::numeric_limits<std::uint32_t>::max)() &&
        name.find('\0') == std::string::npos,
        "Text resource name requires a null-free native DWORD-length string");
}
bool equal_name(const std::string& current, const std::string& incoming) {
    // Nonowning header views borrow the canonical string bytes solely for
    // the existing00435C40 comparison; no second cached name is introduced.
    struct Header { std::uint32_t length; const char* data; };
    static_assert(sizeof(Header) == 8, "Text native name comparison requires Win32");
    const Header left{static_cast<std::uint32_t>(current.size()), current.c_str()};
    const Header right{static_cast<std::uint32_t>(incoming.size()), incoming.c_str()};
    return equal_native_string_headers_00435c40(&left, &right);
}
void invalidate_shader(GuiTextLifetime& lifetime, NativeRenderActualOwners& owners) {
    void* const captured = lifetime.cached_shader_slot_1ec();
    if (!captured) return;
    release_native_render_actual_owner(owners, captured);
    //8CC5/8EC7 stores after terminal callback, even if it published another value.
    lifetime.cached_shader_slot_1ec() = nullptr;
    lifetime.text().has_cached_shader = false;
}
} // namespace

void set_gui_text_shader_name_00ab8e70(GuiTextLifetime& lifetime,
    const std::string& name, GuiTextResourceNameServices& services) {
    require_owner(lifetime, services);
    require_name(name);
    auto& current = lifetime.text().shader_name;
    if (&current != &name) current = name; // Native header identity skips copy only.
    invalidate_shader(lifetime, services.actual_owners);
}

bool set_gui_text_font_name_00ab8c30(GuiTextLifetime& lifetime,
    const std::string& name, GuiTextFontNameServices& services) {
    require_owner(lifetime, services.names);
    require_name(name);
    auto& text = lifetime.text();
    require_name(text.font_name);
    if (equal_name(text.font_name, name)) return false;
    if (&text.font_name != &name) text.font_name = name;
    // Getter callbacks see the changed name. The subsequent lookup reads the
    // LIVE stored member, not the earlier source argument or compatibility copy.
    auto* registry = get_font_registry_007371d0(services.published_registry_00f8bf44,
        services.singleton_lifetime);
    require(registry != nullptr, "Text font lookup requires its actual registry owner");
    const auto* found = registry->find_font_00ac3570(text.font_name);
    if (found) {
        const auto& actual = services.fonts.resolve(&found->descriptor);
        require(&actual.descriptor() == &found->descriptor &&
            &actual.actual_owners() == &services.names.actual_owners,
            "Text font lookup must resolve the same actual font and image owner domain");
    }
    //AB8C86 publishes borrowed font BEFORE reading its live+1C scalar.
    text.font = found ? &found->descriptor : nullptr;
    if (text.font) {
        std::memcpy(&text.alpha_texture_scale, &text.font->alpha_texture_scale, sizeof(float));
    } else {
        const float one = services.one_00d7a24c;
        std::memcpy(&text.alpha_texture_scale, &one, sizeof(float));
    }
    invalidate_shader(lifetime, services.names.actual_owners);
    return true;
}
} // namespace bsp
