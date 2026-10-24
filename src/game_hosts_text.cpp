#include "bsp/game_hosts_text.hpp"
#include "bsp/game_native_text_application.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_gui_effect_application.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/gui_text_child_lifetime.hpp"
#include "bsp/gui_text_native_layout.hpp"
#include "bsp/native_game_grid.hpp"
#include "bsp/native_platform_load_messages.hpp"
#include "bsp/native_font_resources.hpp"
#include "bsp/game_native_gui_text_state.hpp"
#include "bsp/native_renderer_binding_getters.hpp"

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
#include "bsp/cc13_seg87.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace bsp::game {

namespace {
// Packet cc15-wB2: the geometry update's uppercase pass (00aba8d0 at 00aba90d)
// runs cc13_seg87.cpp's 00a9ec30 over the host's copy of the text, instead of the
// host's own loop over 00a9eba0. 00a9ec30 is __thiscall(ECX locale manager,
// stack string*) RET 4 and maps each code unit through 00a9eba0 with the length
// and data reloaded per unit; the host loop does the same, so ON is expected to
// change nothing. Uncertainty: the native string's capacity word and the
// terminator are not part of the view, and 00a9ec30 reads neither.
inline constexpr bool kTextUppercaseCc15Bound = false;
} // namespace

static_assert(sizeof(void*) == 4 && sizeof(int) == 4,
    "Text's explicit owning-UCRT Source profile targets MSVC Win32");
static_assert(sizeof(char16_t) == 2 && sizeof(wchar_t) == 2 &&
    (std::numeric_limits<wchar_t>::min)() == 0 &&
    (std::numeric_limits<wchar_t>::max)() == 0xffff,
    "UCRT wide copies preserve every unsigned UTF16 code unit");
static_assert(std::is_same_v<GuiNativeNameCompare, decltype(&::_stricmp)>,
    "The actual UCRT narrow comparator must have the exact Source callback ABI");

namespace {
std::wstring copy_source_ucrt_wide_units(const char16_t* text) {
    const char16_t* end = text;
    while (*end != u'\0') ++end;
    return std::wstring(text, end);
}

int compare_source_ucrt_wide_current(void*, const char16_t* left, const char16_t* right) {
    const std::wstring wide_left = copy_source_ucrt_wide_units(left);
    const std::wstring wide_right = copy_source_ucrt_wide_units(right);
    // Query no locale early: the real owning UCRT chooses its current state at
    // this final call, after both value copies and any earlier Text callbacks.
    return ::_wcsicmp(wide_left.c_str(), wide_right.c_str());
}
} // namespace

GuiNativeNameCompare source_ucrt_text_name_comparer() noexcept {
    return &::_stricmp;
}

GuiTextContentComparison source_ucrt_text_content_comparison() {
    return GuiTextContentComparison::source_current(compare_source_ucrt_wide_current, nullptr);
}

namespace {

// The image value of the mutable global 00e12fd4, read from the loaded executable:
// Borrow the same verified mutable E12FD4 backing as native Text services.
// Each layout/quad call samples its current value; no sprite-only constant or
// second GUI scale cell substitutes for the process owner.
float vertical_scale_00e12fd4() {
    return game_native_gui_text_state().vertical_scale_00e12fd4();
}

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

// The native arm borrows the sole decoded allocation, including later metric
// changes. A present but retired/mismatched native association cannot fall back
// to a semantic copy. Registry descriptors, fonts and actual images stay stable
// throughout layout/drawing; every GameTextRun must retire before its font.
const NativeFontResources* borrow_native_font(const FontRegistryOwnedFont* owned) noexcept {
    if (!owned || !owned->native_resources) return nullptr;
    const auto* native = owned->native_resources;
    return native->live() && &native->descriptor() == &owned->descriptor ? native : nullptr;
}
const FontData* borrow_font_data(const FontRegistryOwnedFont* owned) noexcept {
    if (owned == nullptr) return nullptr;
    if (const auto* native = owned->native_resources) {
        if (!native->live() || &native->descriptor() != &owned->descriptor) return nullptr;
        return native->native_image() ? nullptr : &native->data();
    }
    return owned->resources ? &owned->resources->data : nullptr;
}

IDirect3DTexture9* borrow_font_sheet(const FontRegistryOwnedFont& owned,
    std::string& error) {
    if (owned.native_resources == nullptr)
        return owned.resources && owned.resources->gfx ? owned.resources->gfx->texture()
            : nullptr;
    const auto& native = *owned.native_resources;
    if (!native.live() || &native.descriptor() != &owned.descriptor) {
        error = "font has no live canonical native resource association";
        return nullptr;
    }
    void* const raw = native.glyph(u' ').gfx_texture_18;
    if (raw == nullptr) return nullptr;
    if ((reinterpret_cast<std::uintptr_t>(raw) & 3u) != 0) {
        error = "font glyph sheet has an unaligned actual owner";
        return nullptr;
    }
    // Resolve through the SAME canonical registry before accessing the image.
    // Its companion must borrow this raw owner's live +04, never a second
    // counter. Registry lookup is metadata only and does not retain anything.
    try {
        auto& companion = native.actual_owners().resolve_actual(raw);
        auto* const actual_count = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
            static_cast<std::byte*>(raw) + 4));
        if (&companion.reference_count != actual_count || actual_count->load() <= 0) {
            error = "font glyph sheet has no live actual reference count";
            return nullptr;
        }
        std::uint32_t profile;
        std::memcpy(&profile, raw, sizeof(profile));
        if (profile != 0x00d61948u) {
            error = "the sprite text bridge requires an actual 2D glyph sheet";
            return nullptr;
        }
        // The established D61948/B3CEA0 leaf reads this owner's actual +10
        // IDirect3DTexture9*. No raw vtable is called, COM wrapper constructed,
        // AddRef performed or second font/image ownership association created.
        return static_cast<IDirect3DTexture9*>(native_texture_get_com_00b3cea0(raw));
    } catch (const std::exception& exception) {
        error = exception.what();
        return nullptr;
    }
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
        summary.vertical_scale = vertical_scale_00e12fd4();
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
        run_.font_resolved = borrow_native_font(owned_) || borrow_font_data(owned_);
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
        const auto* native = borrow_native_font(owned_);
        if (!font && !native) {
            run_.error = "no decoded font data for " + run_.font_name;
            return;
        }
        FontSingleLineParameters parameters;
        parameters.normalized_width = widget_->size.width;
        parameters.width_scale = widget_->font_scale;
        parameters.alignment = static_cast<std::uint32_t>(widget_->align);
        std::string error;
        const bool complete = native
            ? build_font_single_line_00ab9fd0_fragment(*native, transformed, parameters, single_, error)
            : build_font_single_line_00ab9fd0_fragment(*font, transformed, parameters, single_, error);
        if (!complete) {
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
        const auto* native = borrow_native_font(owned_);
        if (!font && !native) {
            run_.error = "no decoded font data for " + run_.font_name;
            return;
        }
        FontWrappedParameters parameters;
        parameters.normalized_width = widget_->size.width;
        parameters.normalized_height = widget_->size.height;
        parameters.width_scale = widget_->font_scale;
        parameters.vertical_scale = vertical_scale_00e12fd4();
        parameters.distance_between_lines = widget_->distance_between_lines;
        parameters.horizontal_alignment = static_cast<std::uint32_t>(widget_->align);
        parameters.vertical_alignment =
            static_cast<std::uint32_t>(widget_->vertical_align);
        std::string error;
        const bool complete = native
            ? build_font_wrapped_00aba270_fragment(*native, transformed, parameters, wrapped_, error)
            : build_font_wrapped_00aba270_fragment(*font, transformed, parameters, wrapped_, error);
        if (!complete) {
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
        if (owned_ == nullptr) return text;
        const auto* native = borrow_native_font(owned_);
        if (!(native ? native->uppercase_only_48() : owned_->descriptor.uppercase_only)) return text;
        run_.uppercased = true;
        std::u16string out = text;
        if constexpr (kTextUppercaseCc15Bound) {
            // The copy's length and buffer stand for the native wide string's
            // +0h / +4h; 00a9ec30 rewrites the units in place.
            NativeWideStringHeader view{static_cast<std::uint32_t>(out.size()), out.data()};
            uppercase_native_wide_string_00a9ec30(owner_.locale, owner_.runtime, view);
        } else {
            for (char16_t& unit : out) {
                unit = locale_uppercase_00a9eba0(owner_.locale, owner_.runtime, unit);
            }
        }
        owner_.log.implemented("GuiText::uppercase_text", "00a9ec30");
        return out;
    }

    const FontData* font_data() const noexcept {
        return borrow_font_data(owned_);
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
    const auto* native = borrow_native_font(owned);
    const FontData* const decoded = borrow_font_data(owned);
    if (!decoded && !native) {
        run.error = "font resources retired before glyph geometry";
        ++host.summary.runs_failed;
        return false;
    }
    run.sheet = borrow_font_sheet(*owned, run.error);
    if (run.sheet == nullptr) {
        if (run.error.empty()) run.error = "font " + run.font_name + " has no glyph sheet texture";
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
        const FontGlyphData& glyph = native ? native->glyph(placement.code_unit)
            : select_font_glyph_00ad4480(*decoded, placement.code_unit);
        FontGeometryParameters parameters;
        parameters.x = placement.x;
        parameters.y = placement.y;
        parameters.width_scale = text.font_scale;
        parameters.vertical_scale = vertical_scale_00e12fd4();
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

namespace bsp::game {
namespace {
// Actual canonical child ownership is delegated to the one deletion transport.
// Borrowed-cell mode requires its native-current comparator. Source-current
// mode owns no CRT state and uses only its explicit comparison-view callback.
class GameNativeTextContentCalls final : public GuiTextContentCalls {
public:
    GameNativeTextContentCalls(GuiTextChildDeletion& children,
        const GuiTextContentComparison& comparison,
        int (*compare)(const char16_t*, const char16_t*))
        : children_(children), compare_(compare) {
        if (!comparison.uses_source_current() && !compare_)
            throw std::invalid_argument("Native Text requires its current CRT locale comparer");
    }
    GuiTextGlyphChildCalls& glyph_child_calls() noexcept override { return children_; }
    void accept_detached_child(std::unique_ptr<GuiLayoutWidget> child) noexcept override {
        children_.accept_detached_child(std::move(child));
    }
    void delete_text_child_virtual4(GuiLayoutWidget& child, std::uint32_t flags) override {
        children_.delete_text_child_virtual4(child, flags);
    }
    int compare_current_locale_00c0392a(const char16_t* left, const char16_t* right) override {
        if (!compare_)
            throw std::logic_error("Native-current Text comparer is not bound in Source-current mode");
        return compare_(left, right);
    }
private:
    GuiTextChildDeletion& children_;
    int (*compare_)(const char16_t*, const char16_t*);
};
} // namespace

struct GameNativeTextApplication::Impl {
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameNativeTextApplicationBindings binding;
    NativeGameGridContext& grid;
    GuiTextNativeLayoutServices& layouts;
    GuiTextChildDeletion children;
    GameNativeTextContentCalls calls;
    GuiTextBufferServices buffers;
    GuiTextShaderServices shader;
    GuiTextResourceNameServices names;
    GuiTextFontNameServices font_names;
    GuiTextContentEnvironment content;
    GuiTextNonemptyEnvironment nonempty;
    GuiTextSingleLineServices single;
    GuiTextWrappedServices wrapped;
    GuiTextRuntimeContentServices runtime_content;
    GuiTextSubmitServices submit;
    GuiTextPropertyServices properties;
    GuiTextTypeDispatchServices dispatch;
    GuiTextClipRefreshServices clip;
    GuiTextRuntimeFactory text_factory;
    GuiTextGlyphChildTailServices glyph_tail;
    bool active{}; // Host callback guard, never a native Text field.
    std::uint32_t failed_entry{};
    std::exception_ptr failed;
    // Appended Source metadata. Compare only; effect metadata owns this token
    // and clears buffers.native_effect_consumer before its destruction. The
    // Text application must keep its services alive through that clearing.
    GuiTextEffectConsumer* expected_effect_consumer{};

    Impl(GameHostLog& log_in, GameNativeRendererApplication& renderer_in,
        GameNativeTextApplicationBindings b)
        : log(log_in), renderer(renderer_in), binding(b),
          grid(renderer.game_grid_context()), layouts(renderer.section_layout_services()),
          children(b.widgets, b.parenting),
          calls(children, b.content_comparison, b.compare_current_locale_00c0392a),
          buffers{b.widgets, grid.graphics.streams.geometry, b.materials, b.parenting,
              grid.strings, renderer.publication_00f8d394(), b.material_vtable_00d5e520,
              &grid.graphics},
          shader{buffers, b.configuration_0109cf04, &b.fonts},
          names{b.widgets, buffers.geometry.actual_owners()},
          font_names{names, b.font_registry, b.fonts, b.constructor.one_00d7a24c},
          content{b.locale, b.locale_runtime, b.content_comparison, buffers, calls, &b.fonts},
          nonempty{shader, b.parameters, layouts, b.style, b.fonts, b.blend_factor_00f8be54},
          single{b.fonts, grid.graphics.streams.mapping, buffers.geometry.actual_owners(),
              layouts, b.vertical_scale_00e12fd4},
          wrapped{b.fonts, grid.graphics.streams.mapping, buffers.geometry.actual_owners(),
              b.widgets, b.parenting, children, b.vertical_scale_00e12fd4,
              renderer.vertex_half_import()},
          runtime_content{content, nonempty, single, wrapped, nullptr},
          submit{runtime_content, b.default_width_00d7a260},
          properties{font_names, submit, b.crt_sse2_conversion, b.properties},
          dispatch{buffers, b.compare_names_00bf7fbf},
          clip{buffers, b.parameters, b.style.aspect_ratio_00e12fc0},
          text_factory{GuiTextRuntimeFactoryServices{b.pool, buffers, children, b.constructor,
              dispatch, properties, clip, b.bounds, b.lineage_00f8be28}},
          glyph_tail{text_factory, buffers, font_names, b.style, b.glyph_strings,
              b.mesh_vtable_00d62d60, b.glyph_shader_name_00cefd78,
              b.constructor.shadow_offset_00d5c5c0, b.constructor.one_00d7a24c,
              b.constructor.disabled_alpha_00ce3800, b.glyph_position} {
        validate_domain();
        runtime_content.glyph_children = &glyph_tail;
    }

    void validate_domain() {
        // Ready-only renderer revalidation. This does not reload, replace or
        // replay the application's stable factory/mapping/layout contexts.
        if (&renderer.game_grid_context() != &grid ||
            &renderer.section_layout_services() != &layouts ||
            buffers.native_renderer != &grid.graphics ||
            &buffers.geometry != &grid.graphics.streams.geometry ||
            &buffers.strings != &grid.strings ||
            &buffers.current_renderer_00f8d394 != &renderer.publication_00f8d394() ||
            &buffers.widgets != &binding.widgets || &buffers.parenting != &binding.parenting ||
            &buffers.materials != &binding.materials ||
            &buffers.materials.retained_owners != &buffers.geometry.actual_owners() ||
            &binding.style.actual_owners != &buffers.geometry.actual_owners() ||
            &binding.parameters.parameter_names != &grid.strings ||
            &binding.materials.parameter_names != &grid.strings ||
            &binding.materials.parameter_slots != &binding.parameters.parameter_slots ||
            static_cast<NativeStringStorage*>(&binding.glyph_strings) != &grid.strings ||
            !binding.material_vtable_00d5e520 || !binding.mesh_vtable_00d62d60 ||
            !binding.glyph_shader_name_00cefd78 ||
            &binding.constructor.shadow_color_00e12fd8[0] != &binding.properties.shadow_color_00e12fd8[0])
            throw std::invalid_argument("Native Text application requires the same live renderer, GUI, material and string owners");
        require_gui_text_native_renderer_domain(grid.graphics,
            buffers.current_renderer_00f8d394, grid.strings, buffers.geometry.actual_owners());
        if (expected_effect_consumer && buffers.native_effect_consumer != expected_effect_consumer)
            throw std::logic_error("Native Text cannot enter after its composed effect binding closed or changed");
    }

    void require_entry() {
        if (active || failed)
            throw std::logic_error("Native Text application cannot reenter or replay an interrupted operation");
        validate_domain();
    }
    GuiTextRuntimeImplementation& implementation(GuiWidgetOwner& owner) {
        if (&owner.runtime() != &binding.widgets)
            throw std::invalid_argument("Native Text belongs to a different widget runtime");
        // Factory lookup authenticates its SAME allocation/identity association;
        // checking the type or native vtable token alone would not do that.
        (void)text_factory.actual_identity(owner);
        auto* actual = dynamic_cast<GuiTextRuntimeImplementation*>(&owner.implementation());
        if (!actual || owner.text_lifetime() != &actual->lifetime())
            throw std::invalid_argument("Native Text requires its canonical factory implementation and lifetime");
        return *actual;
    }
    void record_failure(std::uint32_t entry) noexcept {
        failed_entry = entry;
        failed = std::current_exception();
    }
};

namespace {
struct GameNativeTextEntry final {
    GameNativeTextApplication::Impl& owner;
    explicit GameNativeTextEntry(GameNativeTextApplication::Impl& actual) : owner(actual) {
        owner.require_entry();
        owner.active = true;
    }
    ~GameNativeTextEntry() { owner.active = false; }
};
template<class Call>
GameNativeTextStatus native_text_submit(GameNativeTextApplication::Impl& app,
    GuiWidgetOwner& owner, std::uint32_t entry, Call&& call) {
    app.require_entry();
    auto& implementation = app.implementation(owner);
    // Pending work is resumable only through the exact native child continuation;
    // reject a new source before entering native stores or latching a failure.
    owner.require_no_active_owned_operation();
    GameNativeTextEntry guard(app);
    try {
        call(implementation);
        app.log.notef("native Text entry=%08x complete", entry);
        return GameNativeTextStatus::complete;
    } catch (const GuiTextRuntimePending&) {
        if (!implementation.has_pending_operation()) {
            app.record_failure(entry);
            throw;
        }
        app.log.notef("native Text entry=%08x retains pending native work", entry);
        return GameNativeTextStatus::pending;
    } catch (...) {
        app.record_failure(entry);
        throw;
    }
}
} // namespace

GameNativeTextApplication::GameNativeTextApplication(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameNativeTextApplicationBindings bindings)
    : impl_(std::make_unique<Impl>(log, renderer, bindings)) {}
GameNativeTextApplication::GameNativeTextApplication(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameNativeTextApplicationBindings bindings,
    GameNativeGuiEffectApplication& effects)
    : impl_(std::make_unique<Impl>(log, renderer, bindings)) {
    if (NativePlatformLoadPublication(bindings.configuration_0109cf04).identity() !=
        renderer.borrow_platform_publication_0109cf04().identity())
        throw std::invalid_argument("Native Text effect composition requires the renderer's exact platform publication cell");
    // bind validates every domain before publishing either borrowed pointer.
    // Nothing after publication can throw and destroy these borrowed services.
    effects.bind_text_effect_consumer(impl_->buffers);
    impl_->expected_effect_consumer = impl_->buffers.native_effect_consumer;
}
GameNativeTextApplication::~GameNativeTextApplication() noexcept {
    // No destructor rollback, force-unlock or disposal of a failed native frame.
    // The factory and deletion transport independently enforce the same rule.
    if (requires_process_retention()) std::terminate();
}
std::unique_ptr<GuiWidgetTypeImplementation> GameNativeTextApplication::make_type(GuiWidgetOwner& owner) {
    auto& app = *impl_;
    app.require_entry();
    if (&owner.runtime() != &app.binding.widgets ||
        &app.binding.widgets.owner(owner.layout()) != &owner ||
        owner.layout().type != GuiWidgetType::Text || owner.layout().transform.type_id != 3 ||
        owner.node_binding() || owner.text_lifetime())
        throw std::invalid_argument("Native Text factory requires its fresh primary-null canonical widget owner");
    GameNativeTextEntry guard(app);
    try {
        auto result = app.text_factory.make_type(owner);
        app.log.implemented("NativeGuiText::default_construct", "00ab9650");
        return result;
    } catch (...) {
        app.record_failure(0x00ab9650);
        throw;
    }
}
GameNativeTextStatus GameNativeTextApplication::submit_source(GuiWidgetOwner& owner,
    const std::string& source, bool localize) {
    return native_text_submit(*impl_, owner, 0x00abaed0,
        [&](GuiTextRuntimeImplementation& text) { text.submit_source_00abaed0(source, localize); });
}
GameNativeTextStatus GameNativeTextApplication::submit_utf16(GuiWidgetOwner& owner,
    std::u16string_view text) {
    return native_text_submit(*impl_, owner, 0x00ab6ab0,
        [&](GuiTextRuntimeImplementation& actual) { actual.submit_utf16_00ab6ab0(text); });
}
void GameNativeTextApplication::retire_owned_text(std::unique_ptr<GuiLayoutWidget> layout) {
    auto& app = *impl_;
    app.require_entry();
    if (!layout || layout->transform.parent)
        throw std::invalid_argument("Native Text retirement requires its sole detached layout allocation");
    auto& owner = app.binding.widgets.owner(*layout);
    (void)app.implementation(owner);
    owner.require_no_active_owned_operation();
    GameNativeTextEntry guard(app);
    auto* actual_layout = layout.get();
    app.children.accept_detached_child(std::move(layout));
    try {
        app.children.delete_text_child_virtual4(*actual_layout, 1);
    } catch (...) {
        app.record_failure(0x00ab8ee0);
        throw;
    }
}
GameNativeTextStatus GameNativeTextApplication::resume_after_completed_glyph_child(GuiWidgetOwner& owner) {
    auto& app = *impl_;
    app.require_entry();
    auto& actual = app.implementation(owner);
    auto* pending = actual.pending_content();
    if (!pending || !pending->child_tail ||
        pending->child_tail->status != GuiTextGlyphChildTailStatus::complete)
        throw std::logic_error("Native Text resume requires its same completed glyph-child tail");
    GameNativeTextEntry guard(app);
    try {
        actual.resume_after_glyph_child();
        return GameNativeTextStatus::complete;
    } catch (const GuiTextRuntimePending&) {
        if (!actual.has_pending_operation()) {
            app.record_failure(0x00aba8d0);
            throw;
        }
        return GameNativeTextStatus::pending;
    } catch (...) {
        app.record_failure(0x00aba8d0);
        throw;
    }
}
GuiTextRuntimeContentContinuation* GameNativeTextApplication::pending_content(GuiWidgetOwner& owner) {
    return impl_->implementation(owner).pending_content();
}
GuiTextRuntimeFactory& GameNativeTextApplication::factory() {
    impl_->require_entry();
    return impl_->text_factory;
}
bool GameNativeTextApplication::requires_process_retention() const noexcept {
    return impl_->active || impl_->failed || impl_->text_factory.retained_allocation_count() != 0 ||
        impl_->children.retained_storage_count() != 0 ||
        (impl_->expected_effect_consumer && impl_->buffers.native_effect_consumer);
}
std::uint32_t GameNativeTextApplication::failure_entry() const noexcept { return impl_->failed_entry; }
const std::exception_ptr& GameNativeTextApplication::failure() const noexcept { return impl_->failed; }
} // namespace bsp::game
