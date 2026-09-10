#pragma once
// Renderer bring-up inside cSkeletonAppMidway::Init (0073d410): the phase 4
// renderer construction at 0073da88, the phase 7 renderer subsystems at
// 0073dcc4-0073dd2f and the phase 9 renderer resource objects at 0073dedf-
// 0073df57. Evidence and uncertainty are in docs/APP_INIT_RENDERER.md.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The
// native renderer object is 1D94h bytes and is not reproduced; only the fields
// these phases read or write are projected.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/d3d9_startup.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Settings the renderer phases read out of the static game-settings block at
// 00f88980. 008d8190 BSP_GameSettings_LoadFromRegistry fills it in phase 5, so
// every field below is already final when phase 6 creates the device.
// ---------------------------------------------------------------------------
struct RendererDisplaySettings {
    std::uint32_t width{640};  // +14h, 00f88994, token "Resolution" first value
    std::uint32_t height{480}; // +18h, 00f88998, token "Resolution" second value
    bool hi_res_shadow{};      // +1Dh, 00f8899d, token "HiResShadow"
    bool fullscreen{};         // +1Eh, 00f8899e, token "Fullscreen"
    // +58h, 00f889d8, token "Antialias" sample count, snapped onto the table at
    // 00f88968 by the settings loader. Forwarded unchanged as the multisample
    // type, so the D3DMULTISAMPLE_TYPE values coincide with sample counts.
    std::uint32_t antialias_samples{};
    bool vsync{}; // +60h, 00f889e0, token "VSync"
    float gamma{}; // +64h, 00f889e4, argument of renderer virtual +F0h
};

// ---------------------------------------------------------------------------
// Renderer capability record at renderer+1B18h, returned by primary virtual
// +104h (00b1ff50, a bare `lea eax,[ecx+1B18h]; ret`). 00b2c8e0 fills it from
// IDirect3D9::GetDeviceCaps(0, D3DDEVTYPE_HAL) during construction. Only the
// fields the startup path consumes are projected; offsets are relative to the
// record, not to the renderer.
// ---------------------------------------------------------------------------
struct RendererCapabilityRecord {
    std::uint32_t max_texture_width{};  // +00h (renderer+1B18h), D3DCAPS9+58h
    std::uint32_t max_texture_height{}; // +04h (renderer+1B1Ch), D3DCAPS9+5Ch
    // +28h (renderer+1B40h), LOWORD(D3DCAPS9.PixelShaderVersion), written at
    // 00b2c985. This is the field every shadow gate compares against 200h.
    std::uint32_t pixel_shader_version{};
    // +2Ch (renderer+1B44h), LOWORD(D3DCAPS9.VertexShaderVersion), written at
    // 00b2c977 and forced to 0101h at 00b2cb4b when the reported version is
    // below vs_1_1.
    std::uint32_t vertex_shader_version{};
    bool can_calibrate_gamma{};  // +3Ch (renderer+1B54h), D3DCAPS2 bit 20
    bool fullscreen_gamma{};     // +3Bh (renderer+1B53h), D3DCAPS2 bit 17
};

// Every shadow gate in the startup path is this comparison: 00a8fe30 skips the
// depth probe when it fails, and the settings tail at 008d8190 zeroes the shadow
// fields. ps_2_0 is 0200h.
inline constexpr std::uint32_t kShadowPixelShaderMinimum = 0x0200u;
bool renderer_supports_shadow_maps(const RendererCapabilityRecord& caps) noexcept;

// ---------------------------------------------------------------------------
// Shadow depth-format probe, 00a8fe30.
// ---------------------------------------------------------------------------
enum class ShadowDepthFormat {
    none, // probe found nothing; the object keeps its constructed zeros
    df16, // FourCC 'DF16' (36314644h), the fetch4-style depth texture
    d16,  // D3DFMT_D16 (50h), the plain depth surface fallback
};

// Fields 00a8fe30 writes on the shadow target it constructs.
struct ShadowDepthProbeResult {
    ShadowDepthFormat format{ShadowDepthFormat::none};
    // +24h. The native code writes the same 17h for both formats; the value is
    // an engine-side texture kind, not a D3DFORMAT.
    std::uint32_t texture_kind{};
    std::uint32_t d3d_format{}; // +28h, the FourCC or D3DFORMAT that was accepted
    // +0Eh. Base constructor 00a8a980 clears it; only the D3DFMT_D16 branch
    // sets it, so it reads "the accepted format is a plain depth format".
    bool plain_depth_fallback{};
};

// Pure form of the probe's decision. The native routine queries D3DFMT_D16 only
// after the 'DF16' query fails, so a caller that drives a real device must keep
// that order; this function is total and takes both answers.
ShadowDepthProbeResult select_shadow_depth_format_00a8fe30(
    const RendererCapabilityRecord& caps, bool df16_supported, bool d16_supported) noexcept;

// Square shadow-map extent chosen at 0073dccb-0073dce3. The native code is
// `neg dl; sbb edx,edx; and edx,800h; add edx,800h`, so it is exactly this
// two-way choice and both edges are passed the same value.
std::uint32_t shadow_map_extent_0073dccb(bool hi_res_shadow) noexcept;

// Arguments of the 'DF16' and D3DFMT_D16 queries at 00a8fead and 00a8fef5. They
// reach IDirect3D9::CheckDeviceFormat through renderer virtual +F8h (00b21ec0),
// which translates the engine resource flags into a D3DUSAGE mask.
struct ShadowFormatQuery {
    std::uint32_t adapter_format{0x16}; // D3DFMT_X8R8G8B8
    std::uint32_t engine_resource_flags{0x100}; // translated to D3DUSAGE_DEPTHSTENCIL
    std::uint32_t resource_kind{3};     // D3DRTYPE_TEXTURE
    std::uint32_t format{};             // 'DF16' then D3DFMT_D16
};
inline constexpr std::uint32_t kFourCcDf16 = 0x36314644u;
inline constexpr std::uint32_t kD3dFmtD16 = 0x50u;
ShadowFormatQuery shadow_format_query_00a8fe30(std::uint32_t format) noexcept;

// ---------------------------------------------------------------------------
// Presentation request. 00becee0 creates the HWND and forwards ten stack
// arguments to renderer virtual +4h (00b2aeb0), whose device-creation prefix is
// already reconstructed as d3d9_create_device_prefix_00b2aeb0. This builds the
// same ten arguments from the settings block, so the two halves join.
// ---------------------------------------------------------------------------
D3D9StartupOptions renderer_present_request_00becee0(
    const RendererDisplaySettings& settings, HWND window) noexcept;

// ---------------------------------------------------------------------------
// Allocation sizes the initializer passes to operator new (00bf681b) before
// each constructor. Every one of these call sites skips the constructor when
// the allocation is null, and none of them stores the returned pointer: each
// constructor publishes itself into a global.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kRendererObjectSize = 0x1D94u;      // 0073da66
inline constexpr std::size_t kShadowTargetSize = 0x2Cu;          // 0073dcd3
inline constexpr std::size_t kRenderResourcesSize = 0x6ACu;      // 0073dedf
inline constexpr std::size_t kRenderSingletonB3C4C0Size = 0x04u; // 0073df06
inline constexpr std::size_t kRenderSingletonAD9F90Size = 0x14u; // 0073df31

// ---------------------------------------------------------------------------
// State the sequence produces. This is what a caller can observe; it is not a
// native object.
// ---------------------------------------------------------------------------
struct RendererStartupState {
    bool renderer_constructed{};
    bool shadow_target_constructed{};
    std::uint32_t shadow_extent{};
    ShadowDepthProbeResult shadow_depth{};
    // Effect handles the preload appends to application+8h/+0Ch/+10h, in the
    // order shaderfx/shaderpreload.lua lists them under the key "FileNames".
    std::vector<std::uintptr_t> preloaded_effects{};
    std::uintptr_t noise_texture{};   // render resources +668h
    std::uintptr_t black_texture{};   // render resources +67Ch
    bool render_resources_constructed{};
    bool singleton_b3c4c0_constructed{};
    bool singleton_ad9f90_constructed{};
    float gamma_applied{};
    bool gamma_call_made{};
};

// ---------------------------------------------------------------------------
// Integration boundary. One method per native call site, in startup order.
// Nothing has a default implementation: none of these stands in for renderer
// behaviour that has not been recovered.
// ---------------------------------------------------------------------------
struct RendererStartupHost {
    virtual ~RendererStartupHost() = default;

    // operator new, 00bf681b. Returning null reproduces the native guard: the
    // constructor at that site is skipped and the object never exists.
    virtual void* allocate(std::size_t bytes) = 0;

    // --- phase 4, 0073da88 ---
    // 00b32410, ECX = the 1D94h allocation, plain RET. Direct3DCreate9(20h),
    // the capability gather 00b2c8e0 and the render worker all happen inside;
    // it creates no device.
    virtual void construct_renderer_00b32410(void* storage) = 0;

    // --- phase 7, 0073dcc4-0073dd2f ---
    // Renderer primary virtual +104h (00b1ff50), the capability record filled
    // during construction.
    virtual const RendererCapabilityRecord& renderer_capabilities() = 0;
    // 00a8fe30, ECX = the 2Ch allocation, RET 8. Width and height are the same
    // value at both call sites.
    virtual void construct_shadow_target_00a8fe30(
        void* storage, std::uint32_t width, std::uint32_t height) = 0;
    // Renderer primary virtual +F8h (00b21ec0), RET 10h, true when the
    // underlying IDirect3D9::CheckDeviceFormat returns a non-negative HRESULT.
    virtual bool renderer_check_format(const ShadowFormatQuery& query) = 0;

    // 0073bf80 BSP_ShaderCache_PreloadFromScript, ECX = the application.
    // Broken into its three native pieces so the loop is visible.
    virtual void log_line(const char* text) = 0; // 004254b0
    // shaderfx/shaderpreload.lua, table key "FileNames", read through the Lua
    // wrappers 00b69d40/00b67800/00b67080/00b662b0.
    virtual std::vector<std::string> read_shader_preload_list(const char* script) = 0;
    // Renderer primary virtual +48h (00b318b0), RET 4: the effect loader that
    // rewrites .mshd to .shfx and resolves through the registry at
    // renderer+1A98h. See docs/MATERIAL_DISPATCH.md.
    virtual std::uintptr_t load_effect_by_name(const std::string& name) = 0;

    // Renderer primary virtual +F0h (00b21960), RET 4, the gamma setter.
    virtual void renderer_set_gamma(float gamma) = 0;

    // --- phase 9 renderer resources, 0073dedf-0073df57 ---
    // 00b14a10, ECX = the 6ACh allocation, plain RET. Only the two texture
    // loads are modelled; the rest of the body is field initialisation.
    virtual void construct_render_resources_00b14a10(void* storage) = 0;
    // Renderer primary virtual +64h (00b319b0 BSP_Renderer_LoadTextureByName),
    // RET 8, called with the name and a zero flag.
    virtual std::uintptr_t load_texture_by_name(const std::string& name) = 0;
    // 00b3c4c0, ECX = the 4-byte allocation: base 00b61c70 publishes the object
    // into 0108fed8, then the derived vtable 00d61844 is installed.
    virtual void construct_singleton_00b3c4c0(void* storage) = 0;
    // 00ad9f90, ECX = the 14h allocation: base 00ad9d00 publishes the object
    // into 00f8c218, which the very next step at 0073df87 calls through.
    virtual void construct_singleton_00ad9f90(void* storage) = 0;
};

// Names the native code builds as sized strings; sizes are the resize arguments.
inline constexpr const char* kShaderPreloadScript = "shaderfx/shaderpreload.lua"; // 1Ah
inline constexpr const char* kShaderPreloadTableKey = "FileNames";
inline constexpr const char* kShaderLoadingBegin = "--- SHADER LOADING BEGIN ---";
inline constexpr const char* kShaderLoadingEnd = "--- SHADER LOADING END  ---";
inline constexpr const char* kDefaultNoiseTexture = "noise.dds";  // 9, 00d5e474
inline constexpr const char* kDefaultBlackTexture = "black.tga";  // 9, 00d5e468

// 0073bf80 on its own: appends one effect handle per "FileNames" entry, bracketed
// by the two log lines. The native routine writes into the application's vector at
// +8h/+0Ch/+10h and grows it by doubling through 00735ec0.
void run_shader_preload_0073bf80(
    RendererStartupHost& host, std::vector<std::uintptr_t>& effects);

// Phase 4 renderer construction, the phase 7 renderer subsystems and the phase 9
// renderer resource objects, in the order 0073d410 runs them. The phase 6 device
// creation between them is not part of this call: it happens inside the platform
// window routine 00becee0, which reaches d3d9_create_device_prefix_00b2aeb0.
RendererStartupState run_renderer_startup(
    const RendererDisplaySettings& settings, RendererStartupHost& host);
}
