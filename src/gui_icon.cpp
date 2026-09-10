#include "bsp/gui_icon.hpp"

#include <cmath>
#include <cstring>

namespace bsp {
namespace {

// The two exact-comparison patterns 00AB16A9..00AB16ED test. The native uses
// UCOMISS with the LAHF/TEST AH,44h/JP idiom, so an unordered operand takes the
// "not equal" branch; == on float reproduces that, NaN included.
bool is_one(float value) noexcept { return value == 1.0f; }
bool is_zero(float value) noexcept { return value == 0.0f; }

// 00AB1810 and 00AB1836: the extent is taken through AND 7FFFFFFFh on the
// stored float, not through fabs on the register, so a negative zero difference
// becomes positive zero and a NaN keeps its payload.
float sign_masked(float value) noexcept {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    bits &= 0x7FFFFFFFu;
    float result = 0.0f;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

// 00AB17C8 and 00AB17ED: FILD of a dword followed by a conditional add of
// 4294967296.0 when the value read as signed was negative. That is the standard
// unsigned-to-float sequence, so the dimensions are unsigned.
double unsigned_to_double(std::uint32_t value) noexcept {
    return static_cast<double>(value);
}

std::size_t state_count(const GuiIconWidget& icon) noexcept {
    return icon.states.size();
}

} // namespace

void gui_icon_set_resolved_uv_00ab1680(
    GuiIconState& state, const GuiUvRect& atlas_uv) noexcept {
    state.resolved_uv = atlas_uv;
    if (is_one(state.authored_uv.top) && is_zero(state.authored_uv.bottom)) {
        const float top = state.resolved_uv.top;
        state.resolved_uv.top = state.resolved_uv.bottom;
        state.resolved_uv.bottom = top;
    }
    if (is_one(state.authored_uv.left) && is_zero(state.authored_uv.right)) {
        const float left = state.resolved_uv.left;
        state.resolved_uv.left = state.resolved_uv.right;
        state.resolved_uv.right = left;
    }
}

GuiWidgetSize gui_icon_native_size_00ab17b0(std::uint32_t texture_width,
    std::uint32_t texture_height, const GuiUvRect& uv) noexcept {
    const float width_scale = static_cast<float>(
        unsigned_to_double(texture_width) / kGuiLogicalPageWidth);
    const float height_scale = static_cast<float>(
        unsigned_to_double(texture_height) / kGuiLogicalPageHeight);
    GuiWidgetSize size{};
    size.width = sign_masked(uv.right - uv.left) * width_scale;
    size.height = sign_masked(uv.bottom - uv.top) * height_scale;
    return size;
}

bool gui_icon_state_differs_from_native_size_00ab24b0(const GuiIconState& state,
    std::uint32_t texture_width, std::uint32_t texture_height) noexcept {
    const GuiWidgetSize native = gui_icon_native_size_00ab17b0(
        texture_width, texture_height, state.resolved_uv);
    if (kGuiIconMinPointFilterWidth > native.width) {
        return true;
    }
    if (static_cast<double>(sign_masked(state.size.width - native.width))
        > kGuiIconNativeSizeEpsilon) {
        return true;
    }
    if (static_cast<double>(sign_masked(state.size.height - native.height))
        > kGuiIconNativeSizeEpsilon) {
        return true;
    }
    return false;
}

bool gui_icon_prefers_bilinear_filter_00ab2600(const GuiIconWidget& icon,
    const GuiWidgetTransform& widget, bool platform_allows_point_filter,
    bool state_differs_from_native_size) noexcept {
    if (!icon.has_texture) {
        return false; // 00AB2607
    }
    if (!icon.shader_name.empty()) {
        return false; // 00AB2610
    }
    if (!platform_allows_point_filter) {
        return true; // 00AB261B, the byte at +14h of 0109CF04
    }
    if (widget.rotate != 0.0f) {
        return true; // 00AB2622
    }
    if (widget.scale_x != 1.0f || widget.scale_y != 1.0f) {
        return true; // 00AB263C and 00AB264A
    }
    return state_differs_from_native_size; // 00AB266D
}

const char* gui_icon_material_name(
    const GuiIconWidget& icon, bool prefers_bilinear_filter) noexcept {
    if (!icon.shader_name.empty()) {
        return icon.shader_name.c_str();
    }
    if (!icon.has_texture) {
        return kGuiIconFadeMaterial;
    }
    return prefers_bilinear_filter ? kGuiIconDefaultMaterial
                                   : kGuiIconPointMaterial;
}

float gui_icon_clamp_partial_ratio_00ab1710(float ratio) noexcept {
    if (0.0f > ratio) {
        return 0.0f; // 00AB171C
    }
    if (ratio > 1.0f) {
        return 1.0f; // 00AB1729
    }
    return ratio; // NaN reaches here because neither JA is taken
}

bool gui_icon_select_state_00ab1710(GuiIconWidget& icon, std::int16_t index,
    std::int32_t partial_display_type, float ratio) noexcept {
    const float clamped = gui_icon_clamp_partial_ratio_00ab1710(ratio);
    bool rebuild = false;
    if (index != icon.current_state) {
        const bool in_range = static_cast<std::size_t>(
            static_cast<std::int32_t>(index)) < state_count(icon)
            && static_cast<std::int32_t>(index) >= 0;
        // 00AB1765 and 00AB176E: an index change forces the rebuild when the
        // index is usable, and also when the widget carries no texture at all.
        rebuild = in_range || !icon.has_texture;
    }
    if (!rebuild) {
        // 00AB1770 and 00AB1786, reached both for an unchanged index and for a
        // changed one the two tests above rejected.
        rebuild = partial_display_type != icon.partial_display_type
            || !(clamped == icon.partial_display_ratio);
    }
    if (!rebuild) {
        return false;
    }
    icon.partial_display_ratio = clamped; // 00AB1795
    icon.partial_display_type = partial_display_type; // 00AB179D
    return true;
}

const GuiIconState* gui_icon_state_at(
    const GuiIconWidget& icon, std::int16_t index) noexcept {
    // 00AB3D4E: the sign-extended index is compared unsigned against the count,
    // so -1 is 0FFFFFFFFh here and never selects a record.
    const std::uint32_t unsigned_index =
        static_cast<std::uint32_t>(static_cast<std::int32_t>(index));
    if (icon.states.empty() || unsigned_index >= state_count(icon)) {
        return nullptr;
    }
    return &icon.states[unsigned_index];
}

bool gui_icon_build_quad_setup(const GuiIconWidget& icon,
    const GuiWidgetTransform& widget, GuiIconQuadSetup& setup) noexcept {
    setup = GuiIconQuadSetup{};
    setup.quad.mode = static_cast<std::uint32_t>(icon.partial_display_type);
    setup.quad.ratio = icon.partial_display_ratio;
    setup.quad.crop = icon.crop;
    if (!icon.has_texture) {
        // 00AB3D34: no lookup, the identity rectangle and the widget's size.
        setup.quad.uv = GuiUvRect{};
        setup.size = widget.size;
        setup.quad.width = widget.size.width;
        setup.quad.height = widget.size.height;
        return true;
    }
    const GuiIconState* state = gui_icon_state_at(icon, icon.current_state);
    if (state == nullptr) {
        return false; // the native __report_rangecheckfailure at 00AB3D5E
    }
    setup.quad.uv = state->resolved_uv; // 00AB3D8B..00AB3D93
    setup.writes_widget_pivot = true;   // 00AB3DBE, into widget +18h/+1Ch
    setup.pivot_x = state->pivot_x;
    setup.pivot_y = state->pivot_y;
    setup.size = state->size;           // 00AB3DC8, into widget +20h/+24h
    setup.quad.width = state->size.width;
    setup.quad.height = state->size.height;
    return true;
}

std::int16_t gui_icon_add_state_00ab66c0(GuiIconWidget& icon,
    const GuiWidgetTransform& widget, const std::string& texture_name,
    const GuiUvRect& authored_uv, const float* pivot, const GuiWidgetSize* size,
    GuiIconHost& host) {
    GuiIconState state{};

    // 00AB66E0 and 00AB670F: a null pointer takes the widget's own pair.
    state.pivot_x = pivot != nullptr ? pivot[0] : widget.pivot_x;
    state.pivot_y = pivot != nullptr ? pivot[1] : widget.pivot_y;
    GuiWidgetSize requested = size != nullptr ? *size : widget.size;

    state.texture_name = texture_name; // 00AB67D8, a deep copy
    state.authored_uv = authored_uv;   // 00AB680F..00AB6833

    // 00AB67B5: the rectangle handed to the resolve is the identity one, not
    // the authored one.
    GuiUvRect atlas_uv{};

    if (icon.delayed_texture_load) {
        // 00AB68D5 -> 00AB3B30. The delayed record keeps the identity
        // rectangle the zero-initialisation left, because this path never
        // reaches 00AB1680.
        state.load_pending = true;
        state.flag_3d = false;
        state.texture = host.load_placeholder_texture(kGuiIconDelayedPlaceholder);
    } else {
        state.texture = host.resolve_texture(
            state.texture_name, atlas_uv, requested, 1.0f); // 00AB6859
        gui_icon_set_resolved_uv_00ab1680(state, atlas_uv); // 00AB6886
        if (requested.width == 0.0f && requested.height == 0.0f) {
            // 00AB68AC: the resolve left the size alone, so take the texture's
            // own extent over the rectangle it produced.
            requested = gui_icon_native_size_00ab17b0(
                host.texture_width(state.texture),
                host.texture_height(state.texture), state.resolved_uv);
        }
    }

    state.size = requested; // 00AB68E8 and 00AB68FD
    icon.states.push_back(std::move(state)); // 00AB6903
    // 00AB6908..00AB691F: the count recomputed from the pointers, minus one.
    return static_cast<std::int16_t>(state_count(icon) - 1);
}

void gui_icon_apply_authored_page_00ab3310(GuiIconWidget& icon,
    const GuiWidgetTransform& widget, const GuiIconAuthoredPage& page,
    GuiIconHost& host) {
    icon.dynamic_vb = page.dynamic_vb;                       // 00AB337D
    icon.has_texture = page.has_texture;                     // 00AB33C3
    icon.shader_name = page.shader_name;                     // 00AB3406
    icon.partial_display_type = page.partial_display_type;   // 00AB3449
    icon.partial_display_ratio = page.partial_display_ratio; // 00AB34A2
    icon.delayed_texture_load = page.delayed_texture_load;   // 00AB34E7
    icon.auto_rotate = page.auto_rotate;                     // 00AB3531
    for (const GuiIconAuthoredState& authored : page.states) {
        const float pivot[2] = {authored.pivot_x, authored.pivot_y};
        gui_icon_add_state_00ab66c0(icon, widget, authored.texture,
            authored.uv, pivot, &authored.size, host); // 00AB37E3
    }
}

} // namespace bsp
