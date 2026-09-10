// The GUI Text widget, class id 3. See docs/GUI_TEXT_WIDGET.md for the evidence
// behind every address quoted here.
#include "bsp/gui_text.hpp"

#include <cmath>
#include <cstddef>

namespace bsp {
namespace {

// The reader compares "Left" and "Top" with __stricmp and every other candidate
// through 00425850, which the layout-loader packet already established as a
// case-insensitive equality. Both are ASCII folds over the literals the binary
// holds, so one helper stands in for both.
char fold(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

bool iequals(std::string_view a, std::string_view b) noexcept {
    if (a.size() != b.size()) { return false; }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (fold(a[i]) != fold(b[i])) { return false; }
    }
    return true;
}

// 00ABA8D0 compares the newly built UTF-16 text against the stored one with the
// CRT __wcsicmp before deciding to rebuild. Only the ASCII range of that fold is
// modelled; what the CRT does above it depends on the locale and was not read.
bool wide_iequals(const std::u16string& a, const std::u16string& b) noexcept {
    if (a.size() != b.size()) { return false; }
    for (std::size_t i = 0; i < a.size(); ++i) {
        char16_t x = a[i];
        char16_t y = b[i];
        if (x >= u'A' && x <= u'Z') { x = static_cast<char16_t>(x - u'A' + u'a'); }
        if (y >= u'A' && y <= u'Z') { y = static_cast<char16_t>(y - u'A' + u'a'); }
        if (x != y) { return false; }
    }
    return true;
}

constexpr GuiTextEnumName kAlignNames[] = {
    {"Left", 0},
    {"Center", 1},
    {"Right", 2},
    {"Justified", 3},
};

constexpr GuiTextEnumName kVerticalAlignNames[] = {
    {"Top", 0},
    {"Center", 1},
    {"Bottom", 2},
};

// ---------------------------------------------------------------------------
// Reading typed values out of an evaluated page table
// ---------------------------------------------------------------------------

const std::string* table_string(const GuiTable& table, std::string_view key) {
    const GuiValue* value = table.find(key);
    if (value == nullptr || value->kind() != GuiValue::Kind::String) {
        return nullptr;
    }
    return &value->string();
}

float table_float(const GuiTable& table, std::string_view key, float fallback) {
    const GuiValue* value = table.find(key);
    if (value == nullptr || value->kind() != GuiValue::Kind::Number) {
        return fallback;
    }
    return static_cast<float>(value->number());
}

std::int32_t table_int(const GuiTable& table, std::string_view key,
    std::int32_t fallback) {
    const GuiValue* value = table.find(key);
    if (value == nullptr || value->kind() != GuiValue::Kind::Number) {
        return fallback;
    }
    return static_cast<std::int32_t>(value->number());
}

// The native reader takes a single byte. The pages write Lua booleans, but a
// number is accepted the same way the visitor's coercion would take one.
bool table_bool(const GuiTable& table, std::string_view key, bool fallback) {
    const GuiValue* value = table.find(key);
    if (value == nullptr) { return fallback; }
    if (value->kind() == GuiValue::Kind::Boolean) { return value->boolean(); }
    if (value->kind() == GuiValue::Kind::Number) { return value->number() != 0.0; }
    return fallback;
}

// GuiValueTag::Color is four consecutive floats; the script side is a table of
// four numbers. A table that cannot supply four numbers leaves the default, the
// way the native frame's staged default survives a failed read.
GuiTextColor table_color(const GuiTable& table, std::string_view key,
    const GuiTextColor& fallback) {
    const GuiValue* value = table.find(key);
    if (value == nullptr || !value->is_table()) { return fallback; }
    const GuiTable* rows = value->table();
    if (rows == nullptr || rows->array.size() < 4) { return fallback; }
    for (std::size_t i = 0; i < 4; ++i) {
        if (rows->array[i].kind() != GuiValue::Kind::Number) { return fallback; }
    }
    GuiTextColor out{};
    out.r = static_cast<float>(rows->array[0].number());
    out.g = static_cast<float>(rows->array[1].number());
    out.b = static_cast<float>(rows->array[2].number());
    out.a = static_cast<float>(rows->array[3].number());
    return out;
}

// 00ABA8D0 hands the text to the builder +FCh selects. Both call sites in this
// file go through here so the selection cannot drift.
void run_layout(GuiTextWidget& widget, GuiTextHost& host,
    const std::u16string& text) {
    if (uses_wrapped_layout_00aba8d0(widget)) {
        host.build_wrapped(text);
    } else {
        host.build_single_line(text);
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// Enumerations
// ---------------------------------------------------------------------------

const GuiTextEnumName* gui_text_align_names(std::size_t& count) noexcept {
    count = sizeof(kAlignNames) / sizeof(kAlignNames[0]);
    return kAlignNames;
}

const GuiTextEnumName* gui_text_vertical_align_names(
    std::size_t& count) noexcept {
    count = sizeof(kVerticalAlignNames) / sizeof(kVerticalAlignNames[0]);
    return kVerticalAlignNames;
}

GuiTextAlign parse_gui_text_align_00abb877(std::string_view value) noexcept {
    for (const GuiTextEnumName& row : kAlignNames) {
        if (iequals(value, row.name)) {
            return static_cast<GuiTextAlign>(row.value);
        }
    }
    return GuiTextAlign::Left;
}

GuiTextVerticalAlign parse_gui_text_vertical_align_00abb929(
    std::string_view value) noexcept {
    for (const GuiTextEnumName& row : kVerticalAlignNames) {
        if (iequals(value, row.name)) {
            return static_cast<GuiTextVerticalAlign>(row.value);
        }
    }
    return GuiTextVerticalAlign::Top;
}

std::string_view gui_text_align_name_00ab7d80(GuiTextAlign value) noexcept {
    for (const GuiTextEnumName& row : kAlignNames) {
        if (row.value == static_cast<std::int32_t>(value)) { return row.name; }
    }
    return kAlignNames[0].name;
}

std::string_view gui_text_vertical_align_name_00ab7de0(
    GuiTextVerticalAlign value) noexcept {
    for (const GuiTextEnumName& row : kVerticalAlignNames) {
        if (row.value == static_cast<std::int32_t>(value)) { return row.name; }
    }
    return kVerticalAlignNames[0].name;
}

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

void apply_shadow_preset_00ab72d0(GuiTextWidget& widget,
    std::int32_t preset) noexcept {
    if (widget.default_shadow == preset) { return; }
    widget.default_shadow = preset;
    if (preset == -1) {
        widget.shadowed = false;
        return;
    }
    widget.shadowed = true;
    widget.shadow_color = GuiTextColor{0.0f, 0.0f, 0.0f, 0.75f};
    widget.shadow_pos = GuiTextShadowPos::Behind;
    widget.shadow_offset = kGuiTextDefaultShadowOffset;
    if (preset == 1) {
        // 00AB7318: the colour becomes white with the alpha at 00CEE07C, which
        // holds the same 0.75 the black default uses.
        widget.shadow_color = GuiTextColor{1.0f, 1.0f, 1.0f, 0.75f};
    }
}

bool uses_wrapped_layout_00aba8d0(const GuiTextWidget& widget) noexcept {
    return widget.multiline;
}

std::uint32_t container_width_00aba055(const GuiTextWidget& widget) noexcept {
    const double scaled = static_cast<double>(widget.size.width) *
        kGuiTextContainerWidthScale;
    const long long truncated = static_cast<long long>(scaled);
    return static_cast<std::uint32_t>(static_cast<unsigned long long>(truncated) &
        0xFFFFFFFFull);
}

float single_line_start_x_00aba100(GuiTextAlign align, float container_width,
    float measured_width) noexcept {
    if (align == GuiTextAlign::Center) {
        return (container_width - measured_width) * 0.5f;
    }
    if (align == GuiTextAlign::Right) {
        return container_width - measured_width;
    }
    return 0.0f;
}

GuiTextBounds apply_align_to_bounds_00ab6d70(const GuiTextWidget& widget,
    const GuiTextBounds& bounds) noexcept {
    GuiTextBounds out = bounds;
    const float width = widget.measured_width /
        static_cast<float>(kGuiTextContainerWidthScale);
    switch (widget.align) {
    case GuiTextAlign::Left:
        out.right = bounds.left + width + kGuiTextBoundsPadding;
        break;
    case GuiTextAlign::Center: {
        const float midpoint = (bounds.right + bounds.left) * 0.5f;
        const float half = width * 0.5f;
        out.left = midpoint - half - kGuiTextBoundsPadding;
        out.right = midpoint + half + kGuiTextBoundsPadding;
        break;
    }
    case GuiTextAlign::Right:
        out.left = bounds.right - width - kGuiTextBoundsPadding;
        break;
    case GuiTextAlign::Justified:
        // 00AB6E17 tests only 0, 1 and 2, so mode 3 leaves the rectangle alone.
        break;
    }
    return out;
}

GuiTextLocalisationKey split_localisation_key_00a9f4b0(
    std::string_view source) noexcept {
    GuiTextLocalisationKey out{};
    out.key = source;
    if (!out.key.empty() && out.key.front() == '^') {
        out.had_caret = true;
        out.key.remove_prefix(1);
    }
    // 00A9F4B0 treats a leading '.' as a marker that suppresses the map lookup.
    // Whether the marker is also removed from the key was not established, so
    // the key is reported unchanged and the marker is reported beside it.
    out.suppresses_lookup = !out.key.empty() && out.key.front() == '.';
    return out;
}

bool source_text_changed_00abaed0(const GuiTextWidget& widget,
    std::string_view source) noexcept {
    return !iequals(widget.source, source);
}

// ---------------------------------------------------------------------------
// Routines over the host
// ---------------------------------------------------------------------------

bool set_font_by_name_00ab8c30(GuiTextWidget& widget, GuiTextHost& host,
    const std::string& name) {
    if (iequals(widget.font_name, name)) { return false; }
    widget.font_name = name;
    widget.font = host.find_font(name);
    widget.alpha_texture_scale =
        (widget.font != nullptr) ? widget.font->alpha_texture_scale : 1.0f;
    if (widget.has_cached_shader) {
        host.release_cached_shader();
        widget.has_cached_shader = false;
    }
    return true;
}

bool set_localised_source_00abaed0(GuiTextWidget& widget, GuiTextHost& host,
    const std::string& source, bool localise) {
    if (!source_text_changed_00abaed0(widget, source)) { return false; }
    widget.source = source;
    std::u16string resolved = localise ? host.resolve_localised(source)
                                       : host.widen_source(source);
    if (!wide_iequals(resolved, widget.text)) {
        widget.text = resolved;
        run_layout(widget, host, widget.text);
    }
    // 00ABBD99's tail: virtual +50h with the widget's own Color at +50h, run
    // whether or not the geometry was rebuilt.
    host.apply_color(widget.color);
    return true;
}

void set_size_and_rebuild_00abbf30(GuiTextWidget& widget, GuiTextHost& host,
    const GuiWidgetSize& size) {
    widget.size = size;
    // 00ABB1D0 copies the current text, assigns an empty string to +ECh and
    // re-submits the copy, which defeats 00ABA8D0's equality check and forces a
    // rebuild against the new width. Empty text still compares equal and is not
    // rebuilt.
    if (widget.text.empty()) { return; }
    const std::u16string saved = widget.text;
    widget.text.clear();
    widget.text = saved;
    run_layout(widget, host, widget.text);
}

void bind_gui_text_properties_00abb630(const GuiTable& table,
    GuiTextWidget& widget, GuiTextHost& host) {
    if (const std::string* font = table_string(table, "Font")) {
        set_font_by_name_00ab8c30(widget, host, *font);
    }
    widget.font_scale = table_float(table, "FontScale", widget.font_scale);
    widget.multiline = table_bool(table, "Multiline", widget.multiline);
    widget.distance_between_lines =
        table_float(table, "DistanceBetweenLines", widget.distance_between_lines);
    const std::string* default_text = table_string(table, "DefaultText");
    if (const std::string* shader = table_string(table, "ShaderName")) {
        widget.shader_name = *shader;
    }
    if (const std::string* align = table_string(table, "Align")) {
        widget.align = parse_gui_text_align_00abb877(*align);
    } else {
        widget.align = GuiTextAlign::Left;
    }
    if (const std::string* vertical = table_string(table, "VerticalAlign")) {
        widget.vertical_align = parse_gui_text_vertical_align_00abb929(*vertical);
    } else {
        widget.vertical_align = GuiTextVerticalAlign::Top;
    }
    const GuiValue* colors = table.find("MISColors");
    if (colors != nullptr && colors->is_table() && colors->table() != nullptr) {
        const GuiTable& rows = *colors->table();
        GuiTextStateColors& out = widget.state_colors;
        // The staged default under every one of the four is the lazily built
        // white at 00F8BE38, not the constructor's value.
        const GuiTextColor white{1.0f, 1.0f, 1.0f, 1.0f};
        out.normal = table_color(rows, "Normal", white);
        out.focus = table_color(rows, "Focus", white);
        out.selected = table_color(rows, "Selected", white);
        out.disabled = table_color(rows, "Disabled", white);
        widget.has_state_colors = true;
    }
    const std::int32_t preset = table_int(table, "DefaultShadow", -1);
    if (preset != -1) {
        apply_shadow_preset_00ab72d0(widget, preset);
    } else {
        widget.default_shadow = -1;
        widget.shadowed = table_bool(table, "Shadowed", false);
        if (widget.shadowed) {
            widget.shadow_color =
                table_color(table, "ShadowColor", widget.shadow_color);
            const std::string* pos = table_string(table, "ShadowPos");
            widget.shadow_pos = (pos != nullptr && iequals(*pos, "Front"))
                ? GuiTextShadowPos::Front
                : GuiTextShadowPos::Behind;
            widget.shadow_offset = table_float(table, "ShadowOffset",
                kGuiTextDefaultShadowOffset);
        }
    }
    if (default_text != nullptr) {
        // 00ABBD90 pushes the literal 1 for the flag, so an authored
        // DefaultText is always resolved through the localisation table.
        set_localised_source_00abaed0(widget, host, *default_text, true);
    }
}

}  // namespace bsp
