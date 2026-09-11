#pragma once
#include "bsp/gui_icon.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_texture.hpp"
#include "bsp/instance_geometry.hpp"
#include "bsp/material_clone.hpp"
#include <functional>
#include <memory>

namespace bsp {
class GuiWidgetOwner;
// Shared ownership projection used by the existing generated-model lifetime
// owner. GUI meshes have mesh_stream/combined_layout/section only: no instance
// stream or index buffer. These are actual renderer resources, not fixture IDs.
struct GuiGeometryMaterial {
    std::shared_ptr<MaterialCloneState> owner;
    std::int32_t order{};
    std::uint32_t queue_index{};
};
struct GuiGeometryRuntimeServices {
    IDirect3DDevice9* device{};
    D3D9StateCache* states{};
    // Native model00B74640(0). Return its SAME retained geometry owner.
    std::function<std::shared_ptr<GeneratedInstanceGeometry>()> geometry;
    // Native00B75170(0,mesh,min,max); creation is performed by the class +74.
    std::function<void(std::shared_ptr<GeneratedInstanceGeometry>, float, float)> associate_geometry;
    // Renderer+38/+5C: load actual format; create count vertices, supplied flags.
    std::function<std::shared_ptr<LogicalVertexStream>(const char*, std::uint32_t,
        std::uint32_t)> create_vertex_stream;
    // Actual compiled/effect-bound material and its actual queue metadata.
    std::function<GuiGeometryMaterial(const std::string&)> create_material;
    // Base00AA9F10 clip registration followed by00B18A40(widget,1). Required;
    // do not replace clip sources or widget lifetime with constant/no-op values.
    std::function<void(MaterialCloneState&)> register_clip_and_owner;
    std::function<void()> recompose_00aa7220;
    std::function<void()> publish_color_virtual50;
};
struct GuiGeometryMaterialChoice {
    std::string name;
    bool reuse_existing{};
};
struct GuiGeometryRebuildRequest {
    std::uint32_t vertex_count{};
    std::uint32_t primitive{};
    std::array<std::uint32_t, 4> range_words{};
    std::shared_ptr<LogicalTexture> texture;
    // Called after the stream lock, as in the native rebuild.
    std::function<GuiGeometryMaterialChoice(bool existing_section)> choose_material;
    // Runs after material/texture binding, including when +74 suppresses all
    // writes. Update the SAME layout's size/pivot here. Return false to preserve
    // position/UV bytes (invalid Icon mode still writes white vertex colors).
    std::function<bool(std::vector<GuiQuadVertex>&, bool writes_enabled)> prepare_vertices;
};
// Common retained GUI mesh publication used by Icon and FrameBox. Performs real
// logical-stream locks, declaration-selected writes, parameter/texture binding,
// layout construction and base transform/color callbacks. Throws on missing
// owner/service or unsupported format; no synthetic resources are supplied.
// New C++ semantic ABI; native00AB3CB0 ECX=this, stack short, RET4, last00AB44A9.
void rebuild_gui_geometry_00ab3cb0_fragment(GuiLayoutWidget&, float& overbright_94,
    const GuiGeometryRuntimeServices&, const GuiGeometryRebuildRequest&);

struct GuiIconTextureServices {
    GuiTextureCallbacks resolve;
    // Adopts/releases the ONE native reference returned by00AA2660 or renderer
    // +64. Must not throw. logical_texture preserves the same texture identity
    // and keeps its COM resource alive through every material/cache use.
    std::function<void(void*)> release;
    std::function<std::shared_ptr<LogicalTexture>(void*)> logical_texture;
};
struct GuiIconRuntimeServices {
    const bool& crt_sse2_conversion; // required live0109EEA4 alias
    GuiGeometryRuntimeServices geometry;
    GuiIconTextureServices textures;
    std::function<bool()> platform_allows_point_filter;
    std::function<void()> base_loaded78_00aa7170;
    std::function<void(const GuiWidgetPoint&)> set_position_00aa7dc0;
};
// Owns Icon-derived fields and every state texture reference over the existing
// base layout. Base reader/children must finish BEFORE read_properties; the
// load hook then runs base78 followed by select(0,0,1). No copy ABI is claimed.
class GuiIconRuntime {
public:
    GuiIconRuntime(GuiLayoutWidget&, float& overbright_94, GuiIconRuntimeServices);
    ~GuiIconRuntime();
    GuiIconRuntime(const GuiIconRuntime&) = delete;
    GuiIconRuntime& operator=(const GuiIconRuntime&) = delete;
    void constructed74_00ab2540();
    void read_properties_00ab3310(const GuiTable& evaluated_table);
    void loaded78_00ab10f0();
    void select_state_00ab1710(std::int16_t, std::int32_t, float);
    void rebuild_00ab3cb0(std::int16_t);
    // Complete Icon virtual+58, native ECX=this, size-pair pointer stack,
    // RET4. Base size/recompose, then current-state size/rebuild unless -1.
    // Requires this runtime's SAME retained owner, not a copied base layout.
    // The borrowed input remains live across the base recompose callback.
    void set_size58_00ab1ef0(GuiWidgetOwner&, const GuiWidgetSize&);
    const GuiIconWidget& state() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
// Uses recovered00BD63B0 conversions, integer States traversal stopping at nil,
// and indexed UV reads (missing lanes ->0). Input must be the real Lua snapshot.
// Non-table scopes and wrong-shaped Vec2 values fail explicitly; missing lanes
// in an actual Vec2 table follow the native zero conversions.
GuiIconAuthoredPage read_gui_icon_authored_page_00ab3310(
    const GuiTable&, const GuiWidgetTransform&, const bool& crt_sse2_conversion);
}
