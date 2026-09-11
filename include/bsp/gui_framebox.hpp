#pragma once
#include "bsp/gui_geometry.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_texture.hpp"
#include "bsp/gui_icon_runtime.hpp"
#include <array>
#include <functional>
#include <memory>

namespace bsp {
class GuiLuaReader;

// Semantic projection, not the native 11Ch object / 2Ch state-record ABI.
// Evidence and original calling conventions: docs/GUI_FRAMEBOX_WIDGET.md.
inline constexpr std::int32_t kGuiFrameBoxTypeId = 18;
inline constexpr std::size_t kGuiFrameBoxNativeSize = 0x11c;
inline constexpr float kGuiFrameBoxDefaultX = 0.05833299830555916f;
inline constexpr float kGuiFrameBoxDefaultY = 0.07777699828147888f;
struct GuiFrameBoxState {
    std::shared_ptr<void> texture; // Owns resolve_gui_texture's returned reference.
    std::string texture_name;
    std::array<float, 2> pivot{};
    GuiWidgetSize size{};
    GuiUvRect uv{};
};
struct GuiFrameBoxWidget {
    std::int16_t current_state{-1}; // +ECh
    std::vector<GuiFrameBoxState> states; // +F0h vector, 2Ch stride
    bool has_texture{true}; // +100h
    std::string shader_name; // +104h: copy constructor deliberately clears it
    std::array<float, 2> frame_sizes_x{kGuiFrameBoxDefaultX,kGuiFrameBoxDefaultX};
    std::array<float, 2> frame_sizes_y{kGuiFrameBoxDefaultY,kGuiFrameBoxDefaultY};
};
struct GuiFrameBoxTextureServices {
    GuiTextureCallbacks callbacks;
    // REQUIRED before any resolve. Releases the acquired native reference;
    // capture an owner if the callback needs services that can otherwise die.
    std::function<void(void*)> release;
};

// 00AD2B30 has no Ghidra function: inclusive decoded range 00AD2B30..00AD2D4C.
// Native ECX widget, stack string/pivot/size, RET0Ch, low16 EAX state index.
std::int16_t gui_framebox_add_state_00ad2b30(GuiFrameBoxWidget&,
    const GuiWidgetTransform&, const std::string& texture_name,
    const float* pivot, const GuiWidgetSize* size, const GuiFrameBoxTextureServices&);

// Derived portion of native reader00AD08E0, AFTER the caller has bound the
// base properties/children. Both interfaces consume evaluated Lua values.
// States enumeration stops at the first nil positive index; it appends.
void gui_framebox_read_properties_00ad08e0(GuiFrameBoxWidget&,
    const GuiWidgetTransform&, const GuiTable&, const GuiFrameBoxTextureServices&,
    const bool& crt_sse2_conversion);
void gui_framebox_read_properties_00ad08e0(GuiFrameBoxWidget&,
    const GuiWidgetTransform&, GuiLuaReader&, const GuiFrameBoxTextureServices&);

// 00ACEB70, fastcall out/texture, RET: full texture dimensions / (960,720),
// unlike Icon's native-size rule this does not multiply by atlas UV extent.
GuiWidgetSize gui_framebox_native_size_00aceb70(std::uint32_t width,
    std::uint32_t height) noexcept;

// 00ACF9B0, ECX widget, stack stream/size, RET8; includes 00ACF260's nine
// six-vertex triangle-list writes. Order: TL,TR,BL,BR,top,left,right,bottom,center.
// Explicit y_scale is global00E1301C (installed value0.75). No clamping of
// frame widths, undersized centers, atlas UVs or division-by-zero results.
std::array<GuiQuadVertex, 54> gui_framebox_write_geometry_00acf9b0(
    const GuiFrameBoxWidget&, const GuiWidgetSize&, const GuiFrameBoxState&,
    std::uint32_t texture_width, std::uint32_t texture_height, float y_scale);

const GuiFrameBoxState& gui_framebox_state_at(const GuiFrameBoxWidget&,
    std::int16_t index); // Throws range_error where native fails range-check.
const char* gui_framebox_material_name(const GuiFrameBoxWidget&) noexcept;

// The derived state transfer of00AD27A0: strings/records copy, texture owners
// remain shared, ShaderName becomes empty. Base clone, rebuild BEFORE clearing
// model-bound byte, and the subsequent+74 must be performed by the owner.
GuiFrameBoxWidget gui_framebox_copy_state_00ad27a0(const GuiFrameBoxWidget&);

// Uses the shared actual native stream/material publication reconstructed in
// gui_icon_runtime, with FrameBox's54 vertices and18 triangle-list primitives.
// Required callbacks must retain real owner/resource identities.
struct GuiFrameBoxRuntimeServices {
    GuiGeometryRuntimeServices geometry;
    GuiFrameBoxTextureServices textures;
    std::function<std::shared_ptr<LogicalTexture>(void*)> logical_texture;
    std::function<void(const GuiWidgetPoint&)> set_position_00aa7dc0;
    std::function<void()> base_loaded78_00aa7170;
    float y_scale{0.75f};
};
void gui_framebox_constructed74_00acf8f0(GuiLayoutWidget&,
    const GuiFrameBoxRuntimeServices&);
void gui_framebox_rebuild_00ad0d80(GuiFrameBoxWidget&, GuiLayoutWidget&,
    float& overbright_94, std::int16_t state, const GuiFrameBoxRuntimeServices&);
void gui_framebox_select_state_00acf070(GuiFrameBoxWidget&, GuiLayoutWidget&,
    float& overbright_94, std::int16_t state, const GuiFrameBoxRuntimeServices&);
void gui_framebox_loaded78_00aceb50(GuiFrameBoxWidget&, GuiLayoutWidget&,
    float& overbright_94, const GuiFrameBoxRuntimeServices&);
}
