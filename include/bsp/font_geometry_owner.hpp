#pragma once
#include "bsp/d3d9_vertex_layout.hpp"
#include "bsp/font_geometry.hpp"
#include "bsp/font_resources.hpp"
#include "bsp/font_wrapped_layout.hpp"
#include "bsp/material_samplers.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {
enum class FontGeometryUpdate { unchanged, cleared, rebuilt, unsupported };

struct FontGeometryOwnerResources {
    std::shared_ptr<const FontResources> font;
    // Owner acquires one COM reference each and keeps logical wrappers stable.
    IDirect3DVertexShader9* vertex_shader{};
    IDirect3DPixelShader9* pixel_shader{};
    MaterialSamplerPass samplers;
    std::uint32_t pixel_usage_mask{};
};

struct FontGeometryUpdateParameters {
    std::uint8_t multiline{}; // Native +FC: zero single-line, any nonzero wrapped.
    FontSingleLineParameters single_line;
    FontWrappedParameters wrapped;
    float single_line_vertical_scale{1.0f};
    float origin_x{}, origin_y{}; // Explicit host origin, before normalization.
};

// Checked simplecolor projection used by the existing installed-font draw.
struct FontGeometryVertex {
    float x{}, y{}, z{}, u{}, v{};
    std::uint32_t color{};
};
static_assert(sizeof(FontGeometryVertex) == 24);

struct FontGeometryDrawRange {
    std::shared_ptr<LogicalVertexStream> vertices;
    std::shared_ptr<LogicalIndexStream> indices;
    std::shared_ptr<D3D9VertexLayout> layout;
    D3DPRIMITIVETYPE primitive_type{D3DPT_TRIANGLELIST};
    INT base_vertex{};
    UINT minimum_vertex{}, vertex_count{}, start_index{}, primitive_count{};
};

struct FontGeometryMetrics {
    float measured_width{}; // Native +114, reset on changed-empty input.
    std::uint32_t line_count{}; // Native +110; single-line writes zero.
    float wrapped_height{}; // Native +178; remains stale after single/empty.
    float initial_x{}; // Native +18C; remains stale after empty input.
    std::uint32_t container_width{};
    std::uint16_t height{};
    float normalized_wrapped_height{};
    float normalized_vertical_offset{};
};

struct FontGeometrySnapshot {
    // Capacity follows the stored text length; draw ranges use emitted count.
    // Empty clearing retains these last uploaded bytes and scalar placements.
    std::vector<FontGeometryVertex> vertices;
    std::vector<std::uint16_t> indices;
    std::vector<FontGlyphPlacement> placements;
    std::vector<FontWrappedLine> lines;
    FontGeometryDrawRange main;
    FontGeometryDrawRange shadow; // Same logical vertex/index/layout identities.
};

// Owning fragment of00aba8d0/00ab8400/00b865a0; not the native context ABI.
// Device and cache are borrowed, must describe the same device, and MUST outlive
// this owner (including its move destination). Font/shaders are fixed and retained.
// Constructor rejects null resources or unsupported sampler references with
// std::invalid_argument; allocation exceptions propagate. No device call there.
// Heap-owned wrappers/records keep their addresses across move operations.
class FontGeometryOwner {
public:
    FontGeometryOwner(IDirect3DDevice9& device, D3D9StateCache& cache,
        FontGeometryOwnerResources resources);
    ~FontGeometryOwner();
    FontGeometryOwner(FontGeometryOwner&&) noexcept;
    FontGeometryOwner& operator=(FontGeometryOwner&&) noexcept;
    FontGeometryOwner(const FontGeometryOwner&) = delete;
    FontGeometryOwner& operator=(const FontGeometryOwner&) = delete;

    // Already-transformed UTF16; embedded NUL is unsupported. Case-insensitive
    // equality skips parameter validation and all geometry changes. Changed
    // empty clears both ranges/width while preserving buffers and stale metrics.
    // Changed nonempty allocates fresh buffers after scalar/quad validation.
    // Guarded failure preserves the old text/geometry/metrics. No optional child
    // UI, localization, shader reload, scene callbacks or device-reset ownership.
    FontGeometryUpdate update_00aba8d0_fragment(std::u16string_view text,
        const FontGeometryUpdateParameters& parameters, std::string& error);
    // Explicit00abb1d0 behavior: rebuild current nonempty text; empty skips.
    FontGeometryUpdate rebuild_00abb1d0_fragment(
        const FontGeometryUpdateParameters& parameters, std::string& error);

    // Bind prepared shaders, active material texture slots, stream0, indices
    // and layout. Shader constants/render states/sampler states/draw stay external.
    // From bind until unbind this owner EXCLUSIVELY owns those cache slots.
    // No other user may replace them in that scope. Main/shadow differ only in
    // texture slots/range here; native shadow scene transform remains external.
    HRESULT bind(bool shadow = false);
    // Clears owned cache/device bindings; does not restore prior device state.
    // Called before replacing bound geometry and by destructor/move assignment.
    // Device/cache must still exist. Explicit call exposes best-effort HRESULT.
    HRESULT unbind() noexcept;

    // Borrowed snapshots remain valid until update/rebuild/destruction. Do not
    // mutate/copy out the logical resources for independent binding or reset.
    const FontGeometrySnapshot& geometry() const noexcept;
    const FontGeometryMetrics& metrics() const noexcept;
    std::u16string_view text() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
