#pragma once
#include <cstdint>
#include <memory>

struct IDirect3D9;
struct IDirect3DDevice9;
struct _D3DPRESENT_PARAMETERS_;
namespace bsp {
struct RendererInitRequest;
struct NativeRendererParametersOwner;
struct SettingsRendererCapabilities;
struct ResourceLoadEventHost;
class NativeXLiveDeviceAdapter;
class XLiveLibrary;
struct NativeRendererPresentObservation;
struct NativeTextureCacheContext;
struct NativeTextureReloadOutputs;
struct NativeTextureReloadAcquired;
struct NativeMaterialTextureRefreshContext;
struct NativeMaterialTextureNameLiterals;
struct NativeEffectRegistryDestructionContext;
class NativeRenderActualOwnerRegistry;
struct NativeCameraEnvironment;
struct NativeNodeRawConstants;
class NativeViewportRegistry;
struct NativeCockpitViewportReleaseContext;
struct NativeRenderResourcesLifetimeContext;
class NativeRenderResourcesConstructionAcquired;
struct NativeShaderBinaryCacheContext;
struct NativeShaderBinaryCacheOperation;
struct NativeShaderDescriptorStorage;
struct NativeShaderDescriptorReadContext;
class NativeShaderDescriptorReadOperation;
class NativeLuaServiceBindings;
struct NativeGameGridContext;
class NativeShadowDepthTargetContext;
class GuiTextNativeLayoutServices;
class NativeD3dx9Float16Import;
struct NativeMeshBufferReadContext;
struct NativeMeshMetadataReadContext;
struct NativeMeshTextureFieldContext;
struct NativeMeshLightingConstants;
struct NativeInstanceGeneratorContext;
class NativePlatformLoadPublication;
struct NativeFrameClockPublicationContext;
struct NativeRenderResourceInitArguments;
}
namespace bsp::game {
struct GameNativeMaterialCompilerOwners;
struct GameNativeMaterialCompilerSources;
struct GameNativeMaterialEffectOwners;
struct GameNativeRenderResourceProviders;
class GameSingletonHost;
class GameVfsHost;
class GameNativeReadOnlyData;
class GameNativeLuaServices;
class GameHostLog;
class GameInputRuntime;
// Stable metadata over the original VFS reader and this renderer's existing
// geometry/cache domains. Borrowing invokes no native field reader. Retain the
// application through every consumer; entered operations require their own
// persistent failure frames and native payload retirement before shared drain.
struct GameNativeMeshFieldServices {
    NativeMeshBufferReadContext& buffers;
    NativeMeshMetadataReadContext& metadata;
    NativeMeshTextureFieldContext& textures;
    const NativeMeshLightingConstants& lighting;

    GameNativeMeshFieldServices(NativeMeshBufferReadContext& actual_buffers,
        NativeMeshMetadataReadContext& actual_metadata, NativeMeshTextureFieldContext& actual_textures,
        const NativeMeshLightingConstants& actual_lighting) noexcept
        : buffers(actual_buffers), metadata(actual_metadata), textures(actual_textures), lighting(actual_lighting) {}
    GameNativeMeshFieldServices(const GameNativeMeshFieldServices&) = delete;
    GameNativeMeshFieldServices& operator=(const GameNativeMeshFieldServices&) = delete;
};
// Retain this source graph before native construction and through the actual
// singleton drain. Native failures/constructor-only owners require process
// retention; B32920 requires a completed device/default-surface lifetime.
class GameNativeRendererApplication final {
public:
    // Legacy projected-focus entry. Retains the original pointer-value API and
    // its renderer-owned focus cell; this is not an actual184h platform binding.
    GameNativeRendererApplication(GameHostLog&, GameSingletonHost&, GameVfsHost&,
        GameNativeLuaServices&, GameNativeReadOnlyData&, void* const volatile& clock,
        const void* actual_platform_window_focus);
    // Borrow the caller's SAME publication cell through device reset/loading.
    // Keep that cell and every reached receiver alive through renderer drain.
    // For actual platform callers borrow the mutable0109CF04 cell used by raw
    // construction; this interface does not install a game-process publication,
    // create a platform/window, or admit the complete native message handler.
    GameNativeRendererApplication(GameHostLog&, GameSingletonHost&, GameVfsHost&,
        GameNativeLuaServices&, GameNativeReadOnlyData&, void* const volatile& clock,
        NativePlatformLoadPublication actual_platform_0109cf04);
    ~GameNativeRendererApplication();
    GameNativeRendererApplication(const GameNativeRendererApplication&) = delete;
    GameNativeRendererApplication& operator=(const GameNativeRendererApplication&) = delete;
    void construct();
    void create_device(const RendererInitRequest&);
    // BECEE0's BED1E8..BED222 fragment, after actual device startup. Retain
    // the registered cache and its source bindings through the shared drain.
    void initialize_window_render_entry_cache();
    void bind_platform_services(ResourceLoadEventHost&, const volatile std::uint32_t* online,
        const NativeXLiveDeviceAdapter*, const XLiveLibrary&);
    // Full native begin/clear/end around the current frontend draw. The color
    // is a host choice; the raw frame providers retain their native branches.
    void begin_frame(std::uint32_t clear_color);
    NativeRendererPresentObservation end_frame();
    void copy_settings_capabilities(SettingsRendererCapabilities&) const;
    // The loader reads CURRENT renderer fields and rebuilds its raw AA table.
    void* const volatile& publication_00f8d394() noexcept;
    IDirect3D9& api() const;
    IDirect3DDevice9* device() const noexcept;
    NativeRendererParametersOwner& parameters() const;
    const _D3DPRESENT_PARAMETERS_& presentation() const;
    // The actual renderer's loading domain. Keep acquisition frames alive and
    // release ordinary returned references before the shared renderer drain;
    // its own fallback reference follows the native cache cleanup schedule.
    NativeTextureCacheContext& texture_cache() noexcept;
    // Ready-only captured slot+08 dispatch for this application's canonical
    // D61948 owners and B3FA90 target. Borrow the SAME loading graph; caller
    // supplies explicit output preimages and retains the acquired frame.
    // This does not bind other resource profiles or the full activation scan.
    void reload_activation_texture_2d(void* actual_resource,
        std::uint32_t captured_slot_08, NativeTextureReloadOutputs,
        NativeTextureReloadAcquired&);
    // Ready-only B19000 domain: SAME texture cache, actual owners, CURRENT
    // F8D434 publication and mapped renderer profile. The caller supplies the
    // actual F8D438 mutable/CE3A0C literal bindings and keeps them alive.
    NativeMaterialTextureRefreshContext material_texture_refresh_context(
        const NativeMaterialTextureNameLiterals&);
    // Legacy pointer-value entry only, with its original ready-phase guard.
    // Its +30/+41 focus storage is not a whole native platform. Borrowed-entry
    // callers use borrow_platform_publication_0109cf04 instead; this accessor
    // rejects them rather than returning a fabricated const-pointer cell.
    const void* const volatile& loading_platform_focus_publication();
    // Metadata borrow only; no phase transition, receiver read or native call.
    // Copies preserve the external cell identity. The legacy entry returns its
    // explicitly projected focus cell and gains no actual184h-owner admission.
    NativePlatformLoadPublication borrow_platform_publication_0109cf04() const noexcept;
    NativeRenderActualOwnerRegistry& actual_owners() noexcept;
    // Borrowed until renderer teardown. Callers retain their actual grid
    // storage, operation records and explicit descriptor preimages, and must
    // release grids before draining this renderer. Uses the same VFS strings,
    // declaration cache, stream owners and live process vector as the graph.
    NativeGameGridContext& game_grid_context() noexcept;
    // Ready-only admission/view of this renderer's SAME verified read-only
    // D5E520 {BD30E0,B194B0} profile and canonical geometry/actual-owner domain.
    // No native work, pool, cache or callable table is created. Retain this
    // renderer/read-only-data owner through every material's final release,
    // interrupted invocation and shared drain. New C++ view, not Original ABI.
    const volatile std::uint32_t* borrow_gui_material_destruction_profile_00d5e520();
    // Ready-only borrowed section-layout/vertex-reader prerequisites. These
    // use game_grid_context()'s SAME graphics/string/owner domain and the live
    // renderer publication. Retain this application through every borrower;
    // serialize uses with startup, device recreation, frames and native drain.
    // Retire caller-owned layout references before the shared renderer drain.
    // Interrupted factory/registration state or surviving layout companions
    // require process retention. Successful live layouts remain usable; the
    // retained service and interrupted acquisition must not be replaced/replayed.
    GuiTextNativeLayoutServices& section_layout_services();
    const NativeD3dx9Float16Import& vertex_half_import() const;
    // Explicit ready-only metadata borrow, revalidating the original VFS and
    // renderer identities each time. Do not retarget its reader or contexts.
    // No mesh/subset/item parser, native acquisition or cleanup is activated.
    const GameNativeMeshFieldServices& borrow_mesh_field_services();
    // Ready-only metadata inspection of the SAME graphics/layout/process and
    // mapped-data domain. Retain this application and its original providers.
    // End ALL borrowed views/copies before frame entry, reset/recreation, shared
    // drain or provider destruction; serialize uses and reborrow afterward.
    // Const is shallow: do not mutate referents/serial or copy for activation.
    // No Native construction, factory/finalizer, registration, setter, release
    // or mutable service use is permitted through this metadata borrow. It adds
    // no Owners, acquisition tracking or retention enforcement. The cached plain
    // context remains stable; every permitted borrow revalidates its identities.
    const NativeInstanceGeneratorContext& borrow_instance_generator_context();
    // ON-only diagnostic model composition may activate this SAME context.
    // Borrow only in the ready phase; retain all entered generator/subset
    // operations and this application through retirement or process retention.
    // Serialize loading with frames, reset/recreation and native drain. This
    // does not authorize mutable activation through the metadata-only borrow.
    NativeInstanceGeneratorContext& mission_instance_generator_context();
    // Explicit diagnostic composition. OFF leaves native owners, logs and
    // simulation untouched. The captured startup providers survive raw drain.
    static bool mission_ship_requested() noexcept;
    void configure_mission_ship(GameInputRuntime&, const NativeFrameClockPublicationContext&,
        NativeRenderResourceInitArguments);
    bool mission_ship_enabled() const noexcept;
    void prepare_mission_ship();
    void draw_mission_ship(std::uint64_t mission_frame, float simulated_seconds);
    // 00737F30's 737F80..737FB0 loop over the startup Application's preload rows,
    // when the mission preload borrowed that object (kApplicationCc15Bound).
    void release_application_preload_rows();
    //73BF80's cache bracket over canonical process cells and this SAME VFS.
    // Keep it alive through material compilation, release before singleton drain.
    // Full preload loop remains separate; these entrypoints perform real I/O.
    void create_shader_cache();
    void release_shader_cache();
    // Retained lifecycle metadata remains readable after entered failure or
    // normal release. Cache/allocation pointers are diagnostic identities after
    // retirement; they do not grant another owner or permit replay/rebinding.
    const NativeShaderBinaryCacheOperation* shader_cache_creation_operation() const noexcept;
    bool shader_cache_released() const noexcept;
    NativeShaderBinaryCacheContext& shader_cache_context() noexcept;
    // Same compiler and secondary-pass construction/copy/registration domains.
    // No effect/program load graph is created. The caller keeps persistent
    // native operation frames and this application alive through failed work
    // and genuine owner retirement; no implicit cleanup or replay is supplied.
    GameNativeMaterialCompilerOwners material_compiler_owners();
    GameNativeMaterialCompilerSources material_compiler_sources();
    // Ready-only SAME lifetime/creator-registration prerequisites for genuine
    // effect loading and NativeChildren. No cache/compiler/sampler graph or
    // native load is created. Keep this application and all retained failed
    // frames alive until native owners, bindings and borrowed uses are retired.
    GameNativeMaterialEffectOwners material_effect_owners();
    // Ready-only SAME renderer effect-record/string/array/profile/retirement
    // domain. Borrowing performs no load or cache mutation. Keep this renderer
    // and any new loading composition alive through the actual native drain.
    NativeEffectRegistryDestructionContext& effect_registry_domain();
    // Full B43B00 over a caller-owned initialized110h descriptor. The caller
    // keeps descriptor/name/operation alive and retires descriptor children
    // before this graph. Unknown native stack inputs are never synthesized.
    NativeShaderDescriptorReadContext& shader_descriptor_reader() noexcept;
    // Ready-only borrow of the SAME retained service bundle used by this
    // descriptor reader's bootstrap/files and native renderer Lua entries.
    // Activate it around synchronous borrowed-reader execution; retain this
    // renderer/application Lua owner through any interrupted Lua state.
    NativeLuaServiceBindings& lua_service_binding();
    void read_shader_descriptor(NativeShaderDescriptorStorage&,const void* actual_name,
        std::uint32_t generation,NativeShaderDescriptorReadOperation&);
    // One camera domain shares the application's pool, type counter, node
    // lifetimes and current raw renderer. Retire native cameras/viewports and
    // forget quiescent construction records before destroying this graph.
    NativeCameraEnvironment& camera_environment() noexcept;
    const NativeNodeRawConstants& node_constants() noexcept;
    NativeViewportRegistry& viewport_registry() noexcept;
    const NativeCockpitViewportReleaseContext& cockpit_viewport_release() noexcept;
    // Full B14A10 after device startup; preserves allocation preimages.
    // Caller must complete the later native resource initialization before
    // ordinary destruction (B14A10 leaves +70 unwritten). Normal startup is
    // still gated on that B107F0 integration; this is the shared provider API.
    void* construct_render_resources();
    NativeRenderResourcesLifetimeContext& render_resources_lifetime() noexcept;
    // Ready-phase borrowed raw factory contexts, retained by this application.
    // Caller retains each acquired frame and disposes survivors before drain;
    // failed attempts may require process retention. Does not call B107F0 or
    // admit raw owners into the service's optional direct-terminal domain.
    GameNativeRenderResourceProviders render_resource_providers();
    // Ready-only borrowed context over the canonical F8BBF0/current imports
    // and this application's actual producers and singleton deletion binding.
    // Does not allocate/construct a target. Callers supply genuine storage and
    // complete the final D5B5E8 owner before normal drain; partial base profiles
    // remain unsupported. Failed operation records force process retention.
    NativeShadowDepthTargetContext& shadow_depth_target_context();
    const NativeRenderResourcesConstructionAcquired& render_resources_construction() const noexcept;
    bool requires_process_retention() const noexcept;
    void drain_singletons();
    void after_native_drain();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
