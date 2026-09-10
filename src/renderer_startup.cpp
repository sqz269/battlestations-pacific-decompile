#include "bsp/renderer_startup.hpp"

namespace bsp {
namespace {
// 0073dcc4-0073dd04: allocate, then construct only when the allocation is
// non-null. Every renderer call site in these phases has this shape.
template <typename Construct>
bool allocate_and_construct(RendererStartupHost& host, std::size_t bytes, Construct construct) {
    void* const storage = host.allocate(bytes);
    if (storage == nullptr) return false;
    construct(storage);
    return true;
}
}

bool renderer_supports_shadow_maps(const RendererCapabilityRecord& caps) noexcept {
    // 00a8fe96: `cmp dword ptr [eax+28h],200h; jc` is unsigned, so the record
    // field must be at least ps_2_0. The settings tail at 008d8190 uses the same
    // comparison to force the shadow fields to zero.
    return caps.pixel_shader_version >= kShadowPixelShaderMinimum;
}

std::uint32_t shadow_map_extent_0073dccb(bool hi_res_shadow) noexcept {
    return hi_res_shadow ? 0x1000u : 0x800u;
}

ShadowFormatQuery shadow_format_query_00a8fe30(std::uint32_t format) noexcept {
    ShadowFormatQuery query{};
    query.format = format;
    return query;
}

ShadowDepthProbeResult select_shadow_depth_format_00a8fe30(
    const RendererCapabilityRecord& caps, bool df16_supported, bool d16_supported) noexcept {
    ShadowDepthProbeResult result{};
    if (!renderer_supports_shadow_maps(caps)) return result;
    if (df16_supported) {
        // 00a8fec1: the 'DF16' branch writes 17h and the FourCC and clears +0Eh.
        result.format = ShadowDepthFormat::df16;
        result.texture_kind = 0x17u;
        result.d3d_format = kFourCcDf16;
        result.plain_depth_fallback = false;
        return result;
    }
    if (d16_supported) {
        // 00a8ff0c: the D3DFMT_D16 branch writes the same 17h and sets +0Eh.
        result.format = ShadowDepthFormat::d16;
        result.texture_kind = 0x17u;
        result.d3d_format = kD3dFmtD16;
        result.plain_depth_fallback = true;
        return result;
    }
    // Neither query succeeded: 00a8fe30 falls through with +24h and +28h still
    // zero from 00a8fe7c/00a8fe7f and +0Eh still zero from base 00a8a980.
    return result;
}

D3D9StartupOptions renderer_present_request_00becee0(
    const RendererDisplaySettings& settings, HWND window) noexcept {
    D3D9StartupOptions options{};
    options.window = window;
    options.fullscreen = settings.fullscreen;
    options.width = settings.width;
    options.height = settings.height;
    // Fixed in the native prefix, not derived from the settings block.
    options.backbuffer_format = D3DFMT_A8R8G8B8;
    options.backbuffer_count = 1;
    options.multisample = static_cast<D3DMULTISAMPLE_TYPE>(settings.antialias_samples);
    options.depth_format = D3DFMT_D24S8;
    // The prefix uses only the low bit: `~(sync << 31) & 80000000h`.
    options.presentation_sync = settings.vsync ? 1u : 0u;
    options.fullscreen_refresh_rate = 0;
    return options;
}

void run_shader_preload_0073bf80(
    RendererStartupHost& host, std::vector<std::uintptr_t>& effects) {
    const std::vector<std::string> names
        = host.read_shader_preload_list(kShaderPreloadScript);
    // 0073c04b: the begin line is logged after the table is opened and the
    // iterator is created, so an empty or missing table still logs both lines.
    host.log_line(kShaderLoadingBegin);
    for (const std::string& name : names) {
        effects.push_back(host.load_effect_by_name(name));
    }
    host.log_line(kShaderLoadingEnd);
}

RendererStartupState run_renderer_startup(
    const RendererDisplaySettings& settings, RendererStartupHost& host) {
    RendererStartupState state{};

    // --- phase 4, 0073da66-0073da88 -------------------------------------
    state.renderer_constructed = allocate_and_construct(
        host, kRendererObjectSize,
        [&host](void* storage) { host.construct_renderer_00b32410(storage); });

    // --- phase 7, 0073dccb-0073dd2f -------------------------------------
    // The extent is read from the settings byte before the allocation, so it is
    // computed even when the allocation fails.
    state.shadow_extent = shadow_map_extent_0073dccb(settings.hi_res_shadow);
    state.shadow_target_constructed = allocate_and_construct(
        host, kShadowTargetSize, [&host, &state](void* storage) {
            host.construct_shadow_target_00a8fe30(
                storage, state.shadow_extent, state.shadow_extent);
        });
    if (state.shadow_target_constructed) {
        // Inside 00a8fe30: the capability gate, then at most two format queries
        // in this order. D3DFMT_D16 is asked for only after 'DF16' is refused.
        const RendererCapabilityRecord& caps = host.renderer_capabilities();
        bool df16 = false;
        bool d16 = false;
        if (renderer_supports_shadow_maps(caps)) {
            df16 = host.renderer_check_format(shadow_format_query_00a8fe30(kFourCcDf16));
            if (!df16) {
                d16 = host.renderer_check_format(shadow_format_query_00a8fe30(kD3dFmtD16));
            }
        }
        state.shadow_depth = select_shadow_depth_format_00a8fe30(caps, df16, d16);
    }

    run_shader_preload_0073bf80(host, state.preloaded_effects);

    // 0073dd17: the float is loaded from the settings block and passed to the
    // renderer gamma setter unconditionally, with no capability check here; the
    // capability bytes are tested inside 00b21960.
    state.gamma_applied = settings.gamma;
    state.gamma_call_made = true;
    host.renderer_set_gamma(settings.gamma);

    // --- phase 9 renderer resources, 0073dedf-0073df57 -------------------
    state.render_resources_constructed = allocate_and_construct(
        host, kRenderResourcesSize, [&host, &state](void* storage) {
            host.construct_render_resources_00b14a10(storage);
            // 00b14dd7 and 00b14e4c: the two fallback textures, in this order.
            state.noise_texture = host.load_texture_by_name(kDefaultNoiseTexture);
            state.black_texture = host.load_texture_by_name(kDefaultBlackTexture);
        });
    state.singleton_b3c4c0_constructed = allocate_and_construct(
        host, kRenderSingletonB3C4C0Size,
        [&host](void* storage) { host.construct_singleton_00b3c4c0(storage); });
    state.singleton_ad9f90_constructed = allocate_and_construct(
        host, kRenderSingletonAD9F90Size,
        [&host](void* storage) { host.construct_singleton_00ad9f90(storage); });

    return state;
}
}
