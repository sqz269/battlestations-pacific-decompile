#pragma once

#include "bsp/d3d9_startup.hpp"
#include "bsp/settings_capabilities.hpp"

#include <cstdint>
#include <vector>

namespace bsp {

// Semantic members of each native stride-12 format record. The native final
// DWORD has three uninitialized high bytes; they are deliberately not exposed.
struct RendererTextureFormatCapabilities {
    D3DFORMAT format{};
    bool render_target{};
    bool depth_stencil{};
    bool dynamic{};
    bool post_pixel_shader_blending{};
};

// C++ state, not renderer object layout. Offset suffixes refer to native fields.
// Mapped limits/vectors start at observed constructor zeroes. Query records and
// validity-gated flags use C++ initialization and become valid after their call.
struct RendererCapabilities {
    D3DCAPS9 device_caps{};
    D3DADAPTER_IDENTIFIER9 constructor_identifier{};
    D3DADAPTER_IDENTIFIER9 gather_identifier{};
    bool constructor_identifier_valid{};
    bool gathered{};
    bool description_contains_nvidia_1d88{};
    bool description_matches_legacy_set_1d89{};
    std::uint32_t max_texture_width_1b18{};
    std::uint32_t max_texture_height_1b1c{};
    std::uint32_t max_volume_extent_1b20{};
    std::uint32_t max_anisotropy_1b24{};
    std::uint32_t max_simultaneous_textures_1b28{};
    std::uint32_t shader_texture_limit_1b2c{}; // 4 or 8; consumer meaning provisional
    bool shader_2_or_3_flag_1b30{};           // consumer meaning provisional
    std::uint32_t effective_vertex_shader_version_1b44{};
    std::uint32_t max_vertex_shader_constants_1b4c{};
    bool instancing_flag_1b50{};             // always cleared after INST probe
    bool user_clip_planes_1b51{};
    bool stream_offset_1b52{};
    bool fullscreen_gamma_1b53{};
    bool calibrate_gamma_1b54{};
    bool atoc_1b55{};                        // successful probe sets; failure retains
    std::vector<std::uint32_t> declaration_types_1b5c;
    // Only native resource-type index 3 (D3DRTYPE_TEXTURE) is populated.
    std::vector<RendererTextureFormatCapabilities> texture_formats_1b68;
    bool software_vertex_processing_1b74{};
};

// Constructor fragment 00b32861..00b32898 in 00b32410, thiscall renderer, RET.
// GetAdapterIdentifier(0,0), then case-sensitive Description contains NVIDIA.
// Call after resolution enumeration and before gather. Borrows api, no AddRef.
HRESULT query_renderer_adapter_identifier_00b32410(RendererCapabilities& state,
    IDirect3D9& api);

// 00b2c8e0, thiscall renderer, RET; complete capability query/decision sequence
// through 00b2d8dd. Uses GetDeviceCaps(0,HAL), ATOC, a second identifier query,
// native shader/decl-type rules, conditional INST, and 57 texture-format probes.
// Updates the settings projection from the SAME caps query. Appends to vectors.
// Native ignores caps/identifier failures and reads uninitialized stack memory.
// This API returns their failing HRESULT and leaves both output states unchanged.
// Copying into temporary C++ state also defers native publications and preserves
// outputs if allocation throws; native allocation ordering/EH are not reproduced.
// Unterminated successful adapter descriptions return E_INVALIDARG. Other format
// probes accept strictly HRESULT==0; they do not abort the gather.
HRESULT gather_renderer_capabilities_00b2c8e0(RendererCapabilities& state,
    SettingsRendererCapabilities& settings, IDirect3D9& api);

} // namespace bsp
