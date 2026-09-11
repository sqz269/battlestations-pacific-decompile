#include "bsp/gui_text_properties.hpp"
#include "bsp/gui_lua_reader.hpp"
#include "bsp/native_string_compare.hpp"
#include <cstddef>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace bsp {
static_assert(std::is_same_v<decltype(GuiTextWidget::shadowed), std::uint8_t>);
static_assert(sizeof(GuiTextColor) == 4 * sizeof(float));
static_assert(offsetof(GuiTextColor, a) == 3 * sizeof(float));
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void copy_word(float& destination, float source) noexcept {
    std::memcpy(&destination, &source, sizeof(float));
}
GuiLuaVariant default_string() noexcept {
    GuiLuaVariant value;
    value.value.text = "";
    return value;
}
GuiLuaVariant default_float(float value) noexcept {
    GuiLuaVariant result;
    result.tag = 2; result.value.number = value;
    return result;
}
GuiLuaVariant default_integer(std::int32_t value, std::int32_t tag = 1) noexcept {
    GuiLuaVariant result;
    result.tag = tag; result.value.integer = value;
    return result;
}
GuiLuaVariant default_color(const float* value) noexcept {
    GuiLuaVariant result;
    // Existing variant carries an unqualified pointer; BD61C0 only reads it.
    result.tag = 8; result.value.pointer = const_cast<float*>(value);
    return result;
}
void read_field(const GuiTable& table, const char* key, GuiLuaFieldType type,
    void* destination, const GuiLuaVariant* fallback, const bool& sse2) {
    const auto field = gui_lua_field(type, destination);
    const auto* value = table.find(key);
    const bool nil = !value || value->kind() == GuiValue::Kind::Nil;
    bool stored;
    if (nil && fallback) {
        stored = gui_lua_store_default_00bd61c0(field, *fallback);
    } else {
        const GuiValue absent;
        stored = gui_lua_store_value_00bd63b0(value ? *value : absent, field, nullptr, sse2);
    }
    if (!stored) throw std::invalid_argument("Text property has an unsupported Lua field shape");
}
void read_default(const GuiTable& table, const char* key, GuiLuaFieldType type,
    void* destination, const GuiLuaVariant& fallback, const bool& sse2) {
    read_field(table, key, type, destination, &fallback, sse2);
}
struct NameHeader { std::uint32_t length; const char* data; };
NameHeader name_header(const std::string& name) {
    static_assert(sizeof(NameHeader) == 8);
    require(name.size() <= (std::numeric_limits<std::uint32_t>::max)() &&
        name.find('\0') == std::string::npos, "Text enum requires a terminated native-length name");
    return {static_cast<std::uint32_t>(name.size()), name.c_str()};
}
GuiTextAlign parse_align(const std::string& name) {
    const auto header = name_header(name);
    if (_stricmp(name.c_str(), "Left") == 0) return GuiTextAlign::Left;
    if (equal_native_string_header_00425850(&header, "Right")) return GuiTextAlign::Right;
    if (equal_native_string_header_00425850(&header, "Center")) return GuiTextAlign::Center;
    return equal_native_string_header_00425850(&header, "Justified")
        ? GuiTextAlign::Justified : GuiTextAlign::Left;
}
GuiTextVerticalAlign parse_vertical(const std::string& name) {
    const auto header = name_header(name);
    if (_stricmp(name.c_str(), "Top") == 0) return GuiTextVerticalAlign::Top;
    if (equal_native_string_header_00425850(&header, "Bottom")) return GuiTextVerticalAlign::Bottom;
    return equal_native_string_header_00425850(&header, "Center")
        ? GuiTextVerticalAlign::Center : GuiTextVerticalAlign::Top;
}
void initialize_default_color(const GuiTextPropertyConstants& c) noexcept {
    if ((c.default_color_mask_00f8be48 & 1u) != 0) return;
    const float one = c.one_00d7a24c; // Load precedes OR and all four stores.
    c.default_color_mask_00f8be48 |= 1u;
    for (auto& channel : c.default_color_00f8be38) copy_word(channel, one);
}
void finish(GuiTextPropertiesContinuation& frame) {
    std::string{}.swap(frame.vertical_align);
    std::string{}.swap(frame.align);
    std::string{}.swap(frame.default_text);
    std::string{}.swap(frame.font);
}
void require_services(GuiTextLifetime& lifetime, GuiTextPropertyServices& services) {
    auto binding = lifetime.content_binding();
    auto& names = services.font_names.names;
    auto& content = services.submit.content;
    auto& buffers = content.nonempty.shader.buffers;
    require(binding.widget.layout().type == GuiWidgetType::Text &&
        binding.widget.layout().transform.type_id == 3 &&
        &names.widgets.owner(binding.widget.layout()) == &binding.widget &&
        &names.widgets == &buffers.widgets && &names.actual_owners == &buffers.geometry.actual_owners() &&
        &services.font_names.fonts == &content.nonempty.fonts &&
        &services.font_names.one_00d7a24c == &services.constants.one_00d7a24c,
        "Text properties must use one canonical lifetime, font and actual resource domain");
}
} // namespace

void apply_gui_text_shadow_preset_00ab72d0(GuiTextLifetime& lifetime,
    std::int32_t preset, const GuiTextPropertyConstants& c) {
    auto& text = lifetime.text();
    if (text.default_shadow == preset) return;
    text.default_shadow = preset;
    if (preset == -1) { text.shadowed = 0; return; }
    const float offset = c.shadow_offset_00d5c5c0;
    text.shadowed = 1;
    copy_word(text.shadow_color.r, c.shadow_color_00e12fd8[0]);
    copy_word(text.shadow_color.g, c.shadow_color_00e12fd8[1]);
    copy_word(text.shadow_color.b, c.shadow_color_00e12fd8[2]);
    copy_word(text.shadow_color.a, c.shadow_color_00e12fd8[3]);
    text.shadow_pos = GuiTextShadowPos::Behind;
    copy_word(text.shadow_offset, offset);
    if (preset != 1) return;
    const float one = c.one_00d7a24c;
    copy_word(text.shadow_color.r, one);
    const float alpha = c.white_shadow_alpha_00cee07c;
    copy_word(text.shadow_color.g, one);
    copy_word(text.shadow_color.b, one);
    copy_word(text.shadow_color.a, alpha);
}

GuiTextPropertiesResult read_gui_text_properties_after_base_00abb630(
    GuiTextLifetime& lifetime, const GuiTable& table, GuiTextPropertyServices& services) {
    require_services(lifetime, services);
    auto frame = std::make_unique<GuiTextPropertiesContinuation>();
    auto& text = lifetime.text();
    const auto& sse2 = services.crt_sse2_conversion;
    const auto& c = services.constants;
    read_field(table, "Font", GuiLuaFieldType::String, &frame->font, nullptr, sse2);
    set_gui_text_font_name_00ab8c30(lifetime, frame->font, services.font_names);
    read_default(table, "FontScale", GuiLuaFieldType::Float, &text.font_scale,
        default_float(c.one_00d7a24c), sse2);
    read_default(table, "Multiline", GuiLuaFieldType::Bool, &text.multiline,
        default_integer(1, 3), sse2);
    read_default(table, "DistanceBetweenLines", GuiLuaFieldType::Float,
        &text.distance_between_lines, default_float(0.0f), sse2);
    read_default(table, "DefaultText", GuiLuaFieldType::String, &frame->default_text,
        default_string(), sse2);
    // NativeABB806 writes the member directly, without AB8E70 invalidation.
    read_default(table, "ShaderName", GuiLuaFieldType::String, &text.shader_name,
        default_string(), sse2);
    read_default(table, "Align", GuiLuaFieldType::String, &frame->align, default_string(), sse2);
    text.align = parse_align(frame->align);
    read_default(table, "VerticalAlign", GuiLuaFieldType::String, &frame->vertical_align,
        default_string(), sse2);
    text.vertical_align = parse_vertical(frame->vertical_align);
    const auto* colors = table.find("MISColors");
    if (colors && colors->kind() != GuiValue::Kind::Nil) {
        require(colors->is_table() && colors->table(), "Text MISColors requires its actual table scope");
        const auto read_color = [&](const char* key, GuiTextColor& destination) {
            initialize_default_color(c); //Native repeats this gate before EACH row.
            read_default(*colors->table(), key, GuiLuaFieldType::Vec4, &destination.r,
                default_color(c.default_color_00f8be38), sse2);
        };
        read_color("Normal", text.state_colors.normal);
        read_color("Focus", text.state_colors.focus);
        read_color("Selected", text.state_colors.selected);
        read_color("Disabled", text.state_colors.disabled);
        text.has_state_colors = true; //AB BBA9, after all four writes, before scope leave.
    }
    std::int32_t preset;
    read_default(table, "DefaultShadow", GuiLuaFieldType::Int, &preset, default_integer(-1), sse2);
    if (preset != -1) {
        apply_gui_text_shadow_preset_00ab72d0(lifetime, preset, c);
    } else {
        text.default_shadow = -1;
        bool shadowed;
        read_default(table, "Shadowed", GuiLuaFieldType::Bool, &shadowed, default_integer(0, 3), sse2);
        text.shadowed = static_cast<std::uint8_t>(shadowed);
        if (text.shadowed != 0) {
            read_default(table, "ShadowColor", GuiLuaFieldType::Vec4, &text.shadow_color.r,
                default_color(c.shadow_color_00e12fd8), sse2);
            std::string position;
            read_default(table, "ShadowPos", GuiLuaFieldType::String, &position, default_string(), sse2);
            const auto header = name_header(position);
            text.shadow_pos = equal_native_string_header_00425850(&header, "Front")
                ? GuiTextShadowPos::Front : GuiTextShadowPos::Behind;
            read_default(table, "ShadowOffset", GuiLuaFieldType::Float, &text.shadow_offset,
                default_float(c.shadow_offset_00d5c5c0), sse2);
        } //Native ShadowPos temporary dies before DefaultText submission.
    }
    if (!frame->default_text.empty()) {
        auto result = submit_gui_text_source_00abaed0(lifetime, frame->default_text, true, services.submit);
        if (result.status == GuiTextSubmitStatus::pending_content) {
            require(result.pending != nullptr, "Text property submission lost its pending content frame");
            frame->submission = std::move(result.pending);
            return {GuiTextPropertiesStatus::pending_content, std::move(frame)};
        }
    }
    finish(*frame);
    return {GuiTextPropertiesStatus::complete, {}};
}

GuiTextPropertiesStatus resume_gui_text_properties_after_child(
    std::unique_ptr<GuiTextPropertiesContinuation>& pending) {
    require(pending && pending->submission, "Text properties require their pending submission frame");
    if (resume_gui_text_submit_after_child(pending->submission) == GuiTextSubmitStatus::pending_content)
        return GuiTextPropertiesStatus::pending_content;
    finish(*pending);
    pending.reset();
    return GuiTextPropertiesStatus::complete;
}
} // namespace bsp
