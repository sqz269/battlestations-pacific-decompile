#include "bsp/game_hosts_text.hpp"

#include "bsp/font_data.hpp"
#include "bsp/font_geometry.hpp"
#include "bsp/font_layout.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/font_resources.hpp"
#include "bsp/font_wrapped_layout.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_fonts.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_text.hpp"
#include "bsp/locale_tables.hpp"
#include "bsp/locale_text_lookup.hpp"

#include <array>
#include <cstring>
#include <cwctype>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

// The image value of the mutable global 00e12fd4, read from the loaded executable:
// `00e12fd4  00 00 40 3f` is the float 0.75. Both 00ab98f0 and 00aba270's vertical
// metrics multiply every y by it. Nothing in the reconstructed startup writes it, so a
// cold process keeps the initialised value; a running game may not, which is why the run
// reports the number it used rather than hiding it.
constexpr float kVerticalScale00e12fd4 = 0.75f;

// The quad writer's vertex layout. It is the process's own interleaved buffer, not the
// native declaration: 00ab98f0 only needs a stride and three offsets, and the sprite
// bridge reads the four corners back out of it. See docs/FONT_GEOMETRY.md.
constexpr std::uint32_t kQuadStride = 24;
constexpr std::uint32_t kQuadPositionOffset = 0;
constexpr std::uint32_t kQuadUvOffset = 12;
constexpr std::int32_t kQuadColorOffset = 20;

FontGeometryLayout quad_layout() noexcept {
    FontGeometryLayout layout;
    layout.stride = kQuadStride;
    layout.position_offset = kQuadPositionOffset;
    layout.uv_offset = kQuadUvOffset;
    layout.packed_color_offset = kQuadColorOffset;
    return layout;
}

float vertex_float(const std::uint8_t* vertices, std::uint32_t index,
    std::uint32_t offset) noexcept {
    float value = 0.0f;
    std::memcpy(&value, vertices + static_cast<std::size_t>(index) * kQuadStride + offset,
        sizeof(value));
    return value;
}

const std::string* table_string(const GuiTable& table, const char* key) noexcept {
    const GuiValue* value = table.find(key);
    if (value == nullptr || value->kind() != GuiValue::Kind::String) return nullptr;
    return &value->string();
}

// The log line only: the resolved text is UTF-16 and the run log is ANSI.
std::string ascii_fold(const std::u16string& text, std::size_t limit) {
    std::string out;
    for (char16_t unit : text) {
        if (out.size() >= limit) break;
        out.push_back(unit >= 0x20 && unit < 0x7F ? static_cast<char>(unit) : '?');
    }
    return out;
}

// LocaleTextRuntimeHost for 00a9f4b0's two live dependencies.
//
// The native resolver reaches its Lua owner through the global at 00e1ae90 and that
// object's virtual +14h. This process builds no such owner: the page scripts each own a
// private state and none of them is the application's current context. So the '#...#'
// substitution half is the unimplemented policy rather than a guess at which state to
// read, and 00b692c0's own unsuccessful-traversal answer, "invalid", is returned. No
// shipped title or main-menu string contains a marker, so the method is never reached;
// a run that reaches it says so in the host table.
class GameLocaleTextRuntime final : public LocaleTextRuntimeHost {
public:
    explicit GameLocaleTextRuntime(GameHostLog& log) : log_(log) {}

    std::string context_string_00b692c0(const std::string& path) override {
        static_cast<void>(path);
        log_.unimplemented("LocaleText::lua_context_value", "00b692c0");
        return "invalid";
    }

    char16_t crt_uppercase_00c0391c(char16_t code_unit) override {
        log_.implemented("LocaleText::crt_uppercase", "00c0391c");
        return static_cast<char16_t>(std::towupper(static_cast<std::wint_t>(code_unit)));
    }

private:
    GameHostLog& log_;
};

}  // namespace

// ---------------------------------------------------------------------------

struct GameTextHost::Impl {
    GameHostLog& log;
    GameFontHost& fonts;
    LocaleTables& locale;
    GameLocaleTextRuntime runtime;
    GameTextSummary summary;

    Impl(GameHostLog& log_in, GameFontHost& fonts_in, LocaleTables& locale_in)
        : log(log_in), fonts(fonts_in), locale(locale_in), runtime(log_in) {
        summary.vertical_scale = kVerticalScale00e12fd4;
    }
};

namespace {

// bsp::GuiTextHost for one widget. Every method is one native call site of the Text
// class, so the order the run executes is 00abb630's order rather than one this file
// invents. The two layout results are kept here because the native builders write them
// into the text context the widget owns, which this process does not model.
class GameGuiTextBinding final : public GuiTextHost {
public:
    GameGuiTextBinding(GameTextHost::Impl& owner, GameTextRun& run)
        : owner_(owner), run_(run) {}

    const FontDescriptor* find_font(const std::string& name) override {
        // 00ab8c30: BSP_FontSystem_GetRegistry then the case-insensitive registry lookup
        // 00ac3570. A null answer is valid and leaves the alpha-texture scale at 1.0.
        owned_ = owner_.fonts.registry().find_font_00ac3570(name);
        owner_.log.implemented("GuiText::find_font", "00ab8c30");
        run_.font_name = name;
        run_.font_resolved = owned_ != nullptr && owned_->resources != nullptr;
        if (owned_ == nullptr) return nullptr;
        return &owned_->descriptor;
    }

    void release_cached_shader() override {
        // 00ab8c30's tail drops the cached font shader through its virtual +0h, and
        // 00ab8ce0 would build the replacement. No renderer material owner exists in this
        // process; the sprite bridge draws the quads with fixed-function state instead.
        owner_.log.unimplemented("GuiText::font_shader", "00ab8ce0");
    }

    std::u16string widen_source(const std::string& source) override {
        // 004c5e60 zero-extends each byte; no code page and no table lookup.
        owner_.log.implemented("GuiText::widen_source", "004c5e60");
        std::u16string text;
        text.reserve(source.size());
        for (unsigned char byte : source) text.push_back(static_cast<char16_t>(byte));
        return text;
    }

    std::u16string resolve_localised(const std::string& source) override {
        // 00a9fad0 splits on '|' and appends each piece through 00a9f4b0, which falls
        // back to the stripped key widened byte by byte when the table has no entry.
        const bool hit = owner_.locale.find_00a9ec70(source) != nullptr;
        run_.localised = true;
        run_.locale_hit = hit;
        if (hit) {
            ++owner_.summary.locale_hits;
        } else {
            ++owner_.summary.locale_misses;
        }
        LocaleTextResolver resolver(owner_.locale, owner_.runtime);
        std::u16string text = resolver.resolve(source);
        owner_.log.implemented("GuiText::resolve_localised", "00a9fad0");
        return text;
    }

    void build_single_line(const std::u16string& text) override {
        owner_.log.implemented("GuiText::build_single_line", "00ab9fd0");
        run_.wrapped = false;
        std::u16string transformed = transform(text);
        const FontData* font = font_data();
        if (font == nullptr) {
            run_.error = "no decoded font data for " + run_.font_name;
            return;
        }
        FontSingleLineParameters parameters;
        parameters.normalized_width = widget_->size.width;
        parameters.width_scale = widget_->font_scale;
        parameters.alignment = static_cast<std::uint32_t>(widget_->align);
        std::string error;
        if (!build_font_single_line_00ab9fd0_fragment(*font, transformed, parameters,
                single_, error)) {
            run_.error = error;
            return;
        }
        built_ = true;
    }

    void build_wrapped(const std::u16string& text) override {
        owner_.log.implemented("GuiText::build_wrapped", "00aba270");
        run_.wrapped = true;
        std::u16string transformed = transform(text);
        const FontData* font = font_data();
        if (font == nullptr) {
            run_.error = "no decoded font data for " + run_.font_name;
            return;
        }
        FontWrappedParameters parameters;
        parameters.normalized_width = widget_->size.width;
        parameters.normalized_height = widget_->size.height;
        parameters.width_scale = widget_->font_scale;
        parameters.vertical_scale = kVerticalScale00e12fd4;
        parameters.distance_between_lines = widget_->distance_between_lines;
        parameters.horizontal_alignment = static_cast<std::uint32_t>(widget_->align);
        parameters.vertical_alignment =
            static_cast<std::uint32_t>(widget_->vertical_align);
        std::string error;
        if (!build_font_wrapped_00aba270_fragment(*font, transformed, parameters,
                wrapped_, error)) {
            run_.error = error;
            return;
        }
        built_ = true;
    }

    void apply_color(const GuiTextColor& color) override {
        // 00ab6b50: the base colour setter 00aa6870, then the shadow alpha at +174h
        // scaled by the colour's w lane. The base setter's push into the node material is
        // the renderer's and is not performed here.
        owner_.log.implemented("GuiText::apply_color", "00ab6b50");
        run_.color[0] = color.r;
        run_.color[1] = color.g;
        run_.color[2] = color.b;
        run_.color[3] = color.a;
    }

    void bind(const GuiTable& table, GuiTextWidget& widget) {
        widget_ = &widget;
        bind_gui_text_properties_00abb630(table, widget, *this);
        owner_.log.implemented("GuiText::read_properties", "00abb630");
    }

    bool built() const noexcept { return built_; }
    const FontRegistryOwnedFont* owned() const noexcept { return owned_; }
    const FontSingleLineLayout& single() const noexcept { return single_; }
    const FontWrappedLayout& wrapped() const noexcept { return wrapped_; }

private:
    // 00aba8d0 at 00aba90d: when the font record's uppercase flag at +48h is set, the
    // copied text is uppercased in place by 00a9ec30, which maps every code unit through
    // 00a9eba0. The transform happens inside the geometry update, so it is here rather
    // than in the resolver.
    std::u16string transform(const std::u16string& text) {
        if (owned_ == nullptr || !owned_->descriptor.uppercase_only) return text;
        run_.uppercased = true;
        std::u16string out = text;
        for (char16_t& unit : out) {
            unit = locale_uppercase_00a9eba0(owner_.locale, owner_.runtime, unit);
        }
        owner_.log.implemented("GuiText::uppercase_text", "00a9ec30");
        return out;
    }

    const FontData* font_data() const noexcept {
        if (owned_ == nullptr || owned_->resources == nullptr) return nullptr;
        return &owned_->resources->data;
    }

    GameTextHost::Impl& owner_;
    GameTextRun& run_;
    GuiTextWidget* widget_{nullptr};
    const FontRegistryOwnedFont* owned_{nullptr};
    FontSingleLineLayout single_;
    FontWrappedLayout wrapped_;
    bool built_{false};
};

}  // namespace

GameTextHost::GameTextHost(GameHostLog& log, GameFontHost& fonts, LocaleTables& locale)
    : impl_(std::make_unique<Impl>(log, fonts, locale)) {}
GameTextHost::~GameTextHost() = default;

const GameTextSummary& GameTextHost::summary() const noexcept { return impl_->summary; }

bool GameTextHost::build_run(const std::string& page, const GuiLayoutWidget& widget,
    float origin_x, float origin_y, GameTextRun& run, const std::string* source_override) {
    Impl& host = *impl_;
    ++host.summary.text_widgets;
    run = GameTextRun{};
    run.page = page;
    run.key = widget.key;
    if (widget.source == nullptr) {
        run.error = "the page authored no table for this widget";
        ++host.summary.runs_failed;
        return false;
    }
    const std::string* authored = table_string(*widget.source, "DefaultText");
    if (authored == nullptr && source_override != nullptr) authored = source_override;
    if (authored == nullptr) {
        // 00abb630 only calls 00abaed0 when the key is present, so the native widget has
        // no text either: SubtitlesNormal_Text on FE_main is exactly this case.
        run.error = "no authored DefaultText";
        ++host.summary.runs_empty;
        return false;
    }
    run.source = *authored;

    // The base reader 00aaa710 has already filled the transform and the colour; the Text
    // half reads the container out of +20h/+24h and re-applies +50h through virtual +50h.
    GuiTextWidget text;
    text.size = widget.transform.size;
    text.color = GuiTextColor{widget.color[0], widget.color[1], widget.color[2],
        widget.color[3]};
    run.color[0] = widget.color[0];
    run.color[1] = widget.color[1];
    run.color[2] = widget.color[2];
    run.color[3] = widget.color[3];

    GameGuiTextBinding binding(host, run);
    binding.bind(*widget.source, text);
    if (source_override != nullptr) {
        // The run-time setter, which is what 0058C010 performs at 0058C82C on
        // the mission-detail page's briefing text: the page's authored string
        // was resolved by the loader above, and this replaces it through the
        // same 00ABAED0 the loader's own tail calls.
        run.source = *source_override;
        set_localised_source_00abaed0(text, binding, *source_override, true);
        host.log.implemented("GuiText::set_localised_source", "00abaed0");
    }
    run.resolved_ascii = ascii_fold(text.text, 96);

    if (!binding.built()) {
        if (run.error.empty()) run.error = "the layout builder produced nothing";
        ++host.summary.runs_failed;
        host.log.notef("  text %-24s %-9s FAILED %s", run.key.c_str(),
            run.font_name.c_str(), run.error.c_str());
        return false;
    }

    const FontRegistryOwnedFont* owned = binding.owned();
    const FontData& font = owned->resources->data;
    run.sheet = owned->resources->gfx ? owned->resources->gfx->texture() : nullptr;
    if (run.sheet == nullptr) {
        run.error = "font " + run.font_name + " has no glyph sheet texture";
        ++host.summary.runs_failed;
        return false;
    }

    const std::vector<FontGlyphPlacement>& placements = run.wrapped
        ? binding.wrapped().placements : binding.single().placements;
    const std::uint16_t height = run.wrapped ? binding.wrapped().height
        : binding.single().height;
    if (run.wrapped) {
        run.container_width = binding.wrapped().container_width;
        run.measured_width = binding.wrapped().measured_width;
        run.measured_height = binding.wrapped().measured_height;
        run.vertical_offset = binding.wrapped().normalized_vertical_offset;
        run.lines = binding.wrapped().lines.size();
    } else {
        run.container_width = binding.single().container_width;
        run.measured_width = binding.single().measured_width;
        run.lines = placements.empty() ? 0 : 1;
    }
    run.glyphs = placements.size();

    // One 00ab98f0 call per placement, into the process's own four-vertex scratch buffer.
    // The native writer fills the shared vertex and index objects 00ab8400 created and
    // 00ab8530 sectioned; neither exists here, so those two sites are records and the
    // corners are read straight back out of the scratch buffer for the sprite bridge.
    host.log.unimplemented("GuiText::create_glyph_buffers", "00ab8400");
    host.log.unimplemented("GuiText::ensure_draw_sections", "00ab8530");
    const FontGeometryLayout layout = quad_layout();
    std::array<std::uint8_t, kQuadStride * 4> vertices{};
    std::array<std::uint16_t, 6> indices{};
    run.quads.reserve(placements.size());
    for (const FontGlyphPlacement& placement : placements) {
        const FontGlyphData& glyph = select_font_glyph_00ad4480(font, placement.code_unit);
        FontGeometryParameters parameters;
        parameters.x = placement.x;
        parameters.y = placement.y;
        parameters.width_scale = text.font_scale;
        parameters.vertical_scale = kVerticalScale00e12fd4;
        parameters.height = height;
        parameters.quad_index = 0;
        vertices.fill(0);
        if (!write_font_quad_00ab98f0_fragment(glyph, parameters, layout, vertices.data(),
                vertices.size(), 0, indices)) {
            run.error = "the glyph quad writer rejected a placement";
            break;
        }
        GameTextQuad quad;
        quad.left = vertex_float(vertices.data(), 0, kQuadPositionOffset);
        quad.top = vertex_float(vertices.data(), 0, kQuadPositionOffset + 4);
        quad.right = vertex_float(vertices.data(), 2, kQuadPositionOffset);
        quad.bottom = vertex_float(vertices.data(), 2, kQuadPositionOffset + 4);
        if (run.wrapped) {
            // 00aba86d..00aba896 adds the vertical-alignment offset to each already
            // normalised y, after the quad write rather than before it.
            float shifted = 0.0f;
            if (apply_font_wrapped_vertical_offset_00aba860_fragment(quad.top,
                    run.vertical_offset, shifted)) {
                quad.top = shifted;
            }
            if (apply_font_wrapped_vertical_offset_00aba860_fragment(quad.bottom,
                    run.vertical_offset, shifted)) {
                quad.bottom = shifted;
            }
        }
        quad.u1 = vertex_float(vertices.data(), 0, kQuadUvOffset);
        quad.v1 = vertex_float(vertices.data(), 0, kQuadUvOffset + 4);
        quad.u2 = vertex_float(vertices.data(), 2, kQuadUvOffset);
        quad.v2 = vertex_float(vertices.data(), 2, kQuadUvOffset + 4);
        // The writer works in the text context's own space; the widget's resolved origin
        // is what places that space on screen.
        quad.left += origin_x;
        quad.right += origin_x;
        quad.top += origin_y;
        quad.bottom += origin_y;
        run.quads.push_back(quad);
    }
    if (!run.quads.empty()) {
        host.log.implemented("GuiText::write_glyph_quad", "00ab98f0");
    }
    host.summary.glyph_quads += run.quads.size();
    if (run.quads.empty()) {
        ++host.summary.runs_failed;
        return false;
    }
    ++host.summary.runs_built;
    host.log.notef("  text %-24s font=%-10s %s id=%s glyphs=%zu lines=%zu "
        "container=%u width=%.1f upper=%d color=(%.2f,%.2f,%.2f,%.2f) \"%s\"",
        run.key.c_str(), run.font_name.c_str(), run.wrapped ? "wrapped" : "single",
        run.locale_hit ? run.source.c_str() : (run.source + " (no table entry)").c_str(),
        run.quads.size(), run.lines, run.container_width,
        static_cast<double>(run.measured_width), run.uppercased ? 1 : 0,
        static_cast<double>(run.color[0]), static_cast<double>(run.color[1]),
        static_cast<double>(run.color[2]), static_cast<double>(run.color[3]),
        run.resolved_ascii.c_str());
    return true;
}

}  // namespace bsp::game
