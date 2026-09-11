#pragma once
// The GUI Text widget, class id 3, instance size 1F4h, vtable 00D5C6C8.
//
// Addresses: 00ab9650 (constructor and defaults), 00abb630 (property reader,
//            vtable +18h), 00ab7b60 (describer, vtable +1Ch), 00ab8c30 (font by
//            name), 00ab72d0 (DefaultShadow expander), 00abaed0 (set localised
//            source), 00ab6d70 (alignment applied to a bounds rectangle,
//            vtable +64h), 00abbf30 (vtable +58h), 00ab6ad0/00ab6b50
//            (vtable +4Ch/+50h).
// Supporting addresses read but not owned: 00aa9390 (base constructor),
//            00aaa710 (base property reader), 00aaaed0 (base describer),
//            00aba8d0 (geometry update), 00ab9fd0 / 00aba270 (the two font
//            layout builders), 00ac3570 (font registry lookup), 00a9fad0 /
//            00a9f4b0 (localisation), 00aa6870 / 00aa6980 / 00aa7970 (base
//            colour, alpha and size setters).
//
// Evidence: docs/GUI_TEXT_WIDGET.md, reports/gui_text_widget.json. The layout
// and drawing themselves belong to the font packets; nothing here re-implements
// them. Names are hypotheses, not recovered symbols.
#include "bsp/font_registry.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_widget.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace bsp {

// ---------------------------------------------------------------------------
// Constants read out of the binary
// ---------------------------------------------------------------------------

// 00AA1380 loads it into ECX before the allocator call; the class table in
// gui_layout_loader.hpp carries the same figure.
inline constexpr std::uint32_t kGuiTextInstanceSize = 0x1F4;

// Installed by 00AB9650; the state-color override at +80h is 00AB7200.
inline constexpr std::uint32_t kGuiTextVtable = 0x00D5C6C8;

// 00ABA055 and 00ABA5FC multiply the widget's normalised Size.x by this double
// before truncating it to the integer container width the builders use.
inline constexpr double kGuiTextContainerWidthScale = 960.0;

// The double at 00D5C5C8, the horizontal padding 00AB6D70 adds outside the
// measured text when it rewrites a bounds rectangle.
inline constexpr float kGuiTextBoundsPadding = 0.011994361877441406f;

// The default at 00D5C5C0, also the value 00AB72D0 installs for every preset.
inline constexpr float kGuiTextDefaultShadowOffset = 0.05f;

// gui_layout_loader.hpp lists GuiValueTag 1 as unrecovered. DefaultShadow is
// the property that recovers it: the reader and the describer both push tag 1
// with a signed 32-bit field and the default 0xFFFFFFFF.
inline constexpr GuiValueTag kGuiValueTagInt = static_cast<GuiValueTag>(1);

// ---------------------------------------------------------------------------
// The two enumerations, as the data the reader and the describer agree on
// ---------------------------------------------------------------------------

// +100h. The reader compares "Left" with __stricmp and the rest through the
// case-insensitive helper 00425850; an unmatched or absent value leaves Left.
enum class GuiTextAlign : std::int32_t {
    Left = 0,       // 00CE92E4
    Center = 1,     // 00D5C67C
    Right = 2,      // 00CE92DC
    Justified = 3,  // 00D5C670
};

// +104h. Same comparison shape, "Top" first.
enum class GuiTextVerticalAlign : std::int32_t {
    Top = 0,     // "Top"
    Center = 1,  // "Center"
    Bottom = 2,  // "Bottom"
};

// +160h. 00ABBCFD sets 1 exactly when the authored string matches "Front";
// every other value, the absent key included, leaves 0.
enum class GuiTextShadowPos : std::int32_t {
    Behind = 0,
    Front = 1,
};

// One row of the enum tables. Exposed as data so the doc and the code cannot
// drift apart, and because the describer reads the same rows backwards.
struct GuiTextEnumName {
    std::string_view name;
    std::int32_t value;
};

const GuiTextEnumName* gui_text_align_names(std::size_t& count) noexcept;
const GuiTextEnumName* gui_text_vertical_align_names(std::size_t& count) noexcept;

// The reader's comparison chains. Order matters only because the native code
// stops at the first match; the strings are distinct, so any order agrees.
GuiTextAlign parse_gui_text_align_00abb877(std::string_view value) noexcept;
GuiTextVerticalAlign parse_gui_text_vertical_align_00abb929(
    std::string_view value) noexcept;

// 00AB7D80 and 00AB7DE0, the describer's inverse maps. Both are total: a value
// outside the table takes the same branch the native `else` takes.
std::string_view gui_text_align_name_00ab7d80(GuiTextAlign value) noexcept;
std::string_view gui_text_vertical_align_name_00ab7de0(
    GuiTextVerticalAlign value) noexcept;

// ---------------------------------------------------------------------------
// The instance
// ---------------------------------------------------------------------------

// A four-float colour lane. The native fields are four consecutive floats read
// and written with GuiValueTag::Color.
struct GuiTextColor {
    float r{0.0f};
    float g{0.0f};
    float b{0.0f};
    float a{0.0f};
};

// +11Ch..+158h, the four colours under the "MISColors" sub-table.
// 00AB7200 selects these rows, with owner+77h forcing the disabled row.
// The defaults are the constructor's stores.
struct GuiTextStateColors {
    GuiTextColor normal{0.7f, 0.7f, 0.7f, 1.0f};    // +11Ch, 00CE3E18 x3
    GuiTextColor focus{1.0f, 1.0f, 1.0f, 1.0f};     // +12Ch
    GuiTextColor selected{1.0f, 1.0f, 1.0f, 1.0f};  // +13Ch
    GuiTextColor disabled{0.0f, 0.0f, 0.0f, 0.5f};  // +14Ch, 00CE3800
};

// The fields of the Text extension that this packet establishes, +ECh..+1F3h
// over the widget base 00AA9390 builds. Not the whole object: the geometry,
// material and drawable fields between +178h and +1EFh belong to the font
// packets and are documented in FONT_CONTEXT_OWNERSHIP.md.
struct GuiTextWidget {
    // The base fields this packet reads. `size` projects +20h/+24h, the
    // container the builders measure against; `color` projects the four-float
    // "Color" property at +50h, which 00ABAED0 re-applies through virtual +50h.
    GuiWidgetSize size{};
    GuiTextColor color{1.0f, 1.0f, 1.0f, 1.0f};

    // +ECh, the current UTF-16 text. The builders walk it; 00ABB1D0 blanks it
    // to force a rebuild.
    std::u16string text{};

    // +F4h, the narrow source the last set-text call cached. 00ABAED0 compares
    // against it case-insensitively and returns early on a match, so this is
    // the widget's re-layout key, not its displayed text.
    std::string source{};

    // +FCh, "Multiline". The constructor stores 1, so wrapping is the default
    // and the key is only ever authored to turn it off.
    bool multiline{true};

    GuiTextAlign align{GuiTextAlign::Left};                            // +100h
    GuiTextVerticalAlign vertical_align{GuiTextVerticalAlign::Top};    // +104h

    // +108h, the borrowed registry record 00AB8C30 stored. Null until a font
    // resolves; the pointer is invalidated by registry mutations.
    const FontDescriptor* font{nullptr};

    float distance_between_lines{0.0f};  // +10Ch, "DistanceBetweenLines"

    // +110h and +114h, what the last layout produced: the line count and the
    // measured width in font units. Divide the width by
    // kGuiTextContainerWidthScale to compare it against a normalised size.
    std::int32_t line_count{0};
    float measured_width{0.0f};

    bool has_state_colors{false};  // +118h, set to 1 by the MISColors descend
    GuiTextStateColors state_colors{};

    // 00AB6C30 stores the caller's raw low byte, including noncanonical true.
    std::uint8_t shadowed{0};                                   // +15Ch
    GuiTextShadowPos shadow_pos{GuiTextShadowPos::Behind};      // +160h
    float shadow_offset{kGuiTextDefaultShadowOffset};           // +164h
    GuiTextColor shadow_color{0.0f, 0.0f, 0.0f, 0.75f};         // +168h

    std::string shader_name{};  // +1C0h, "ShaderName"
    std::string font_name{};    // +1C8h, "Font"

    // +1D0h, "DefaultShadow". -1 means the key was absent, which is the only
    // state in which the explicit shadow keys are read at all.
    std::int32_t default_shadow{-1};

    // +1D4h, the font record's +1Ch copied on every successful lookup, 1.0
    // when the lookup failed. FONT_CONTEXT_MATERIAL_BINDINGS.md pairs it with
    // the base widget's +94h to feed cOverbrightAlphatex.
    float alpha_texture_scale{1.0f};

    float font_scale{1.0f};  // +1D8h, "FontScale", an extra horizontal scale

    // +1ECh, the cached font shader. 00AB8C30 releases and clears it whenever
    // the font name changes. Modelled as a flag because the renderer object is
    // not this packet's to own.
    bool has_cached_shader{false};
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 00AB72D0, __thiscall(this, int), RET 4. Returns early when the preset is
// unchanged. -1 clears the shadow; every other preset enables it with the black
// default, and only 1 overwrites the colour with white. Presets 0 and 2 are
// therefore indistinguishable, which is what the shipped pages rely on.
void apply_shadow_preset_00ab72d0(GuiTextWidget& widget,
    std::int32_t preset) noexcept;

// 00ABA8D0's builder choice: +FCh zero selects the single-line builder
// 00AB9FD0, nonzero the wrapped builder 00ABA270.
bool uses_wrapped_layout_00aba8d0(const GuiTextWidget& widget) noexcept;

// The integer container width both builders derive from the widget's Size.x:
// multiply by 960 under x87 truncation to a signed 64-bit temporary, keep the
// low dword, and read that dword back as unsigned when converting to float
// again. The unsigned reading is why this returns an unsigned value.
std::uint32_t container_width_00aba055(const GuiTextWidget& widget) noexcept;

// The single-line start offset, 00ABA100..00ABA148. Left and Justified start at
// zero; Right subtracts the measured width from the container width and Center
// halves that difference. Widths are in font units, as the native code holds
// them.
float single_line_start_x_00aba100(GuiTextAlign align, float container_width,
    float measured_width) noexcept;

// A bounds rectangle in normalised widget units, the shape 00AB6D70 rewrites.
struct GuiTextBounds {
    float left{0.0f};
    float top{0.0f};
    float right{0.0f};
    float bottom{0.0f};
};

// 00AB6D70's horizontal half, __thiscall(this, float*, float*, float*, float*),
// RET 10h. The measured width is normalised by 960 and padded by the double at
// 00D5C5C8. Left moves the right edge, Right moves the left edge, and Center
// spreads the padded width around the midpoint of the two. Justified leaves the
// rectangle untouched, because the native code tests only 0, 1 and 2. The
// vertical half dispatches into 00AB6BD0 and is not reconstructed here.
GuiTextBounds apply_align_to_bounds_00ab6d70(const GuiTextWidget& widget,
    const GuiTextBounds& bounds) noexcept;

// 00A9F4B0's prefix handling. One leading '^' is stripped before anything else;
// a '.' that leads what remains marks the key as one whose map lookup is
// suppressed. What the '^' means to the resolver is still open, so this reports
// the split rather than claiming a behaviour.
struct GuiTextLocalisationKey {
    std::string_view key;         // the remainder after the stripped prefixes
    bool had_caret{false};        // a '^' was present and removed
    bool suppresses_lookup{false};  // the remainder began with '.'
};
GuiTextLocalisationKey split_localisation_key_00a9f4b0(
    std::string_view source) noexcept;

// 00ABAED0's early-out: the cached narrow source is compared with
// BSP_NativeString_EqualsInsensitive, so only a case-insensitively different
// key re-runs the resolution and the layout.
bool source_text_changed_00abaed0(const GuiTextWidget& widget,
    std::string_view source) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

// One method per native call site that leaves this packet. There are no default
// implementations: nothing here stands in for the font, localisation or render
// code the class calls into.
struct GuiTextHost {
    virtual ~GuiTextHost() = default;

    // 00AB8C30: BSP_FontSystem_GetRegistry followed by 00AC3570. The result is
    // a borrowed pointer the widget stores at +108h; null is a valid answer and
    // leaves the alpha-texture scale at 1.0.
    virtual const FontDescriptor* find_font(const std::string& name) = 0;

    // 00AB8C30's tail: InterlockedDecrement on the cached shader at +1ECh and
    // its virtual +0h when the count reaches zero.
    virtual void release_cached_shader() = 0;

    // 00ABAED0 with the flag clear: 004C5E60 widens the bytes with no table
    // lookup.
    virtual std::u16string widen_source(const std::string& source) = 0;

    // 00ABAED0 with the flag set: 00A9FAD0 splits the source on '|' with
    // _strcspn and resolves each piece through 00A9F4B0.
    virtual std::u16string resolve_localised(const std::string& source) = 0;

    // 00ABA8D0 with +FCh clear: the single-line builder 00AB9FD0.
    virtual void build_single_line(const std::u16string& text) = 0;

    // 00ABA8D0 with +FCh set: the wrapped builder 00ABA270.
    virtual void build_wrapped(const std::u16string& text) = 0;

    // 00ABAED0's tail, virtual +50h with the widget's own Color at +50h. Its
    // Text override 00AB6B50 calls the base setter 00AA6870 and then scales the
    // shadow alpha at +174h by the colour's w lane.
    virtual void apply_color(const GuiTextColor& color) = 0;
};

// 00AB8C30, __thiscall(this, const NativeString*), RET 4. Returns false and
// touches nothing when the name matches the cached one case-insensitively.
// Otherwise it caches the name, resolves the record, copies its scale ratio
// into +1D4h and drops the cached shader. It deliberately does not re-run the
// layout: the text is only rebuilt on the next text or size change.
bool set_font_by_name_00ab8c30(GuiTextWidget& widget, GuiTextHost& host,
    const std::string& name);

// 00ABAED0, __thiscall(this, const NativeString*, bool), RET 8. Caches the
// source, resolves it through the host according to the flag, hands the result
// to the builder +FCh selects, and finishes by re-applying the widget colour.
// Returns false when the cached source already matched.
bool set_localised_source_00abaed0(GuiTextWidget& widget, GuiTextHost& host,
    const std::string& source, bool localise);

// 00ABBF30, __thiscall(this, size), RET 4: the base size setter 00AA7970
// followed by 00ABB1D0, which blanks the UTF-16 text and re-submits the saved
// copy so the wrap is recomputed against the new width.
void set_size_and_rebuild_00abbf30(GuiTextWidget& widget, GuiTextHost& host,
    const GuiWidgetSize& size);

// 00ABB630, __thiscall(this, visitor*), RET 4, minus the 00AAA710 half the
// layout-loader packet already models. Reads the Text keys out of an evaluated
// page table in the native order, so Font is resolved before DefaultText and
// DefaultShadow is expanded before the explicit shadow keys are considered.
// An authored DefaultText is always localised: the native call site pushes the
// literal 1 for the flag.
void bind_gui_text_properties_00abb630(const GuiTable& table,
    GuiTextWidget& widget, GuiTextHost& host);

}  // namespace bsp
