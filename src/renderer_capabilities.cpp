#include "bsp/renderer_capabilities.hpp"

#include <array>
#include <cstddef>
#include <cstring>
#include <utility>

namespace bsp {
namespace {

// Stack baseline at 00b2c8ff is ESP+1F4h. These checks bind the assembly's
// stack loads to the SDK layout, including loads made after argument pushes.
static_assert(offsetof(D3DCAPS9, Caps2) == 0x0c);
static_assert(offsetof(D3DCAPS9, DevCaps) == 0x1c);
static_assert(offsetof(D3DCAPS9, MaxTextureWidth) == 0x58);
static_assert(offsetof(D3DCAPS9, MaxTextureHeight) == 0x5c);
static_assert(offsetof(D3DCAPS9, MaxVolumeExtent) == 0x60);
static_assert(offsetof(D3DCAPS9, MaxAnisotropy) == 0x6c);
static_assert(offsetof(D3DCAPS9, MaxSimultaneousTextures) == 0x98);
static_assert(offsetof(D3DCAPS9, MaxUserClipPlanes) == 0xa4);
static_assert(offsetof(D3DCAPS9, VertexShaderVersion) == 0xc4);
static_assert(offsetof(D3DCAPS9, MaxVertexShaderConst) == 0xc8);
static_assert(offsetof(D3DCAPS9, PixelShaderVersion) == 0xcc);
static_assert(offsetof(D3DCAPS9, DevCaps2) == 0xd4);
static_assert(offsetof(D3DCAPS9, DeclTypes) == 0xec);
static_assert(offsetof(D3DADAPTER_IDENTIFIER9, Description) == 0x200);

struct FormatProbe {
    std::uint32_t format;
    bool render_target;
    bool depth_stencil;
    bool dynamic;
    bool unused_fourth_native_flag;
};

// 00b2ceba..00b2d70f builds 57 stride-8 entries on the native stack.
constexpr std::array<FormatProbe, 57> format_probes{{
    {0x32495441, true, false, true, true},
    {0x14, true, true, true, true},
    {0x15, true, false, false, false}, {0x16, true, false, false, false},
    {0x17, true, false, false, false}, {0x18, true, false, false, false},
    {0x19, true, false, false, false}, {0x1a, true, false, false, false},
    {0x1b, true, false, false, false}, {0x1c, true, false, false, false},
    {0x1d, true, false, false, false}, {0x1e, true, false, false, false},
    {0x1f, false, false, false, false},
    {0x20, true, false, false, false}, {0x21, true, false, false, false},
    {0x22, true, false, false, false}, {0x23, true, false, false, false},
    {0x24, true, false, false, false}, {0x28, true, false, false, false},
    {0x29, true, false, false, false}, {0x32, true, false, false, false},
    {0x33, true, false, false, false}, {0x34, true, false, false, false},
    {0x3c, true, false, false, false}, {0x3d, true, false, false, false},
    {0x3e, true, false, false, false}, {0x3f, true, false, false, false},
    {0x40, true, false, false, false}, {0x43, true, false, false, false},
    {0x59565955, true, false, false, false},
    {0x47424752, true, false, false, false},
    {0x32595559, true, false, false, false},
    {0x42475247, true, false, false, false},
    {0x31545844, true, false, false, false},
    {0x32545844, true, false, false, false},
    {0x33545844, true, false, false, false},
    {0x34545844, true, false, false, false},
    {0x35545844, true, false, false, false},
    {0x46, true, true, false, false}, {0x47, true, true, false, false},
    {0x49, true, true, false, false}, {0x4b, true, true, false, false},
    {0x4d, true, true, false, false}, {0x4f, true, true, false, false},
    {0x50, true, true, false, false}, {0x52, true, true, false, false},
    {0x53, true, true, false, false}, {0x51, true, false, false, false},
    {0x6e, true, false, false, false},
    {0x3154454d, true, false, false, false},
    {0x6f, true, false, false, false}, {0x70, true, false, false, false},
    {0x71, true, false, false, false}, {0x72, true, false, false, false},
    {0x73, true, false, false, false}, {0x74, true, false, false, false},
    {0x75, true, false, false, false}
}};

HRESULT get_identifier(IDirect3D9& api, D3DADAPTER_IDENTIFIER9& identifier) {
    const HRESULT result = api.GetAdapterIdentifier(0, 0, &identifier);
    if (FAILED(result)) return result;
    return std::memchr(identifier.Description, '\0', sizeof(identifier.Description))
        ? S_OK : E_INVALIDARG;
}

bool check_format(IDirect3D9& api, DWORD usage, D3DRESOURCETYPE type,
    std::uint32_t format) {
    return api.CheckDeviceFormat(0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
        usage, type, static_cast<D3DFORMAT>(format)) == D3D_OK;
}

} // namespace

HRESULT query_renderer_adapter_identifier_00b32410(RendererCapabilities& state,
    IDirect3D9& api) {
    D3DADAPTER_IDENTIFIER9 identifier{};
    const HRESULT result = get_identifier(api, identifier);
    if (FAILED(result)) return result;
    state.constructor_identifier = identifier;
    state.constructor_identifier_valid = true;
    state.description_contains_nvidia_1d88 =
        std::strstr(identifier.Description, "NVIDIA") != nullptr;
    return S_OK;
}

HRESULT gather_renderer_capabilities_00b2c8e0(RendererCapabilities& state,
    SettingsRendererCapabilities& settings, IDirect3D9& api) {
    D3DCAPS9 caps{};
    const HRESULT caps_result = api.GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps);
    if (FAILED(caps_result)) return caps_result;

    RendererCapabilities next = state;
    next.device_caps = caps;
    next.max_texture_width_1b18 = caps.MaxTextureWidth;
    next.max_texture_height_1b1c = caps.MaxTextureHeight;
    next.max_volume_extent_1b20 = caps.MaxVolumeExtent;
    next.fullscreen_gamma_1b53 = (caps.Caps2 & 0x20000) != 0;
    next.calibrate_gamma_1b54 = (caps.Caps2 & 0x100000) != 0;
    next.effective_vertex_shader_version_1b44 = caps.VertexShaderVersion & 0xffff;
    const std::uint32_t pixel_version = caps.PixelShaderVersion & 0xffff;

    if (check_format(api, 0, D3DRTYPE_SURFACE, 0x434f5441)) next.atoc_1b55 = true;
    const HRESULT identifier_result = get_identifier(api, next.gather_identifier);
    if (FAILED(identifier_result)) return identifier_result;
    const char* const description = next.gather_identifier.Description;
    next.description_matches_legacy_set_1d89 = std::strstr(description, "8800")
        || std::strstr(description, "8600") || std::strstr(description, "8200")
        || std::strstr(description, "ATI");

    if (pixel_version <= 0x104) {
        next.shader_texture_limit_1b2c = 4;
        next.shader_2_or_3_flag_1b30 = false;
    } else if (pixel_version == 0x200 || pixel_version == 0x300) {
        next.shader_texture_limit_1b2c = 8;
        next.shader_2_or_3_flag_1b30 = true;
    }
    next.max_vertex_shader_constants_1b4c = caps.MaxVertexShaderConst;
    next.max_simultaneous_textures_1b28 = caps.MaxSimultaneousTextures;
    next.max_anisotropy_1b24 = caps.MaxAnisotropy;
    next.user_clip_planes_1b51 = caps.MaxUserClipPlanes != 0;
    next.software_vertex_processing_1b74 = (caps.DevCaps & 0x10000) == 0
        || (caps.VertexShaderVersion & 0xffff) < 0x101;
    if (next.software_vertex_processing_1b74)
        next.effective_vertex_shader_version_1b44 = 0x101;

    for (std::uint32_t type = 0; type < 5; ++type)
        next.declaration_types_1b5c.push_back(type);
    constexpr std::array<std::uint32_t, 9> optional_types{{5, 8, 9, 10, 11, 12, 13, 14, 15}};
    for (std::uint32_t bit = 0; bit < optional_types.size(); ++bit)
        if ((caps.DeclTypes & (1u << bit)) != 0)
            next.declaration_types_1b5c.push_back(optional_types[bit]);

    // Native sets the flag on ps>=3 or INST success, then unconditionally clears
    // it at 00b2ceae. The conditional query still has to occur.
    if (pixel_version < 0x300)
        (void)check_format(api, 0, D3DRTYPE_SURFACE, 0x54534e49);
    next.instancing_flag_1b50 = false;
    next.stream_offset_1b52 = (caps.DevCaps2 & 1) != 0;

    for (const FormatProbe& probe : format_probes) {
        if (!check_format(api, 0, D3DRTYPE_TEXTURE, probe.format)) continue;
        RendererTextureFormatCapabilities entry;
        entry.format = static_cast<D3DFORMAT>(probe.format);
        if (probe.render_target)
            entry.render_target = check_format(api, 1, D3DRTYPE_TEXTURE, probe.format);
        if (probe.depth_stencil)
            entry.depth_stencil = check_format(api, 2, D3DRTYPE_TEXTURE, probe.format);
        if (probe.dynamic)
            entry.dynamic = check_format(api, 0x200, D3DRTYPE_TEXTURE, probe.format);
        entry.post_pixel_shader_blending = check_format(api, 0x80001,
            D3DRTYPE_TEXTURE, probe.format);
        next.texture_formats_1b68.push_back(entry);
    }

    next.gathered = true;
    state = std::move(next);
    settings.pixel_shader_version_28 = pixel_version;
    settings.max_shader_model = pixel_version < 0x200 ? 1 : 2;
    return S_OK;
}

} // namespace bsp
