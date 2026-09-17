#include "bsp/native_renderer_constructor.hpp"
#include "bsp/native_render_entry_cache.hpp"
#include "bsp/native_renderer_base_lifetime.hpp"
#include "bsp/native_render_cache_construction.hpp"
#include "bsp/native_renderer_frame_statistics.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_shader_state_definitions.hpp"
#include "bsp/native_system_constant_registry.hpp"
#include "bsp/native_renderer_gather_capabilities.hpp"
#include "bsp/native_renderer_control_worker.hpp"
#include "bsp/native_renderer_resolution_enumeration.hpp"
#include "bsp/native_renderer_parent_member_cleanup.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include "bsp/native_vertex_declaration_cache.hpp"
#include "bsp/native_vertex_declaration_loading.hpp"
#include "bsp/native_vertex_declaration_registry_lifetime.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_effect_registry_destroy.hpp"
#include "bsp/native_renderer_cache_cleanup.hpp"
#include "bsp/native_render_resource_container_removal.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/camera_plane_initialization.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include "bsp/game_native_vfs_application.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include "bsp/native_renderer_lua_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/game_native_surface_pool.hpp"
#include "bsp/game_native_texture_pool.hpp"
#include "bsp/game_native_graphics_pools.hpp"
#include "bsp/game_native_hardware_layout_tree.hpp"
#include "bsp/native_renderer_destructor.hpp"
#include "bsp/native_renderer_cache_clear.hpp"
#include "bsp/native_renderer_query_terminal.hpp"
#include "bsp/native_renderer_default_surfaces.hpp"
#include "bsp/native_renderer_device_recreation_actual.hpp"
#include "bsp/native_system_registry_raw_terminal.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/light_type_bootstrap.hpp"
#include "bsp/game_native_mutable_crt_data.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include "bsp/native_renderer_device_startup_actual.hpp"
#include "bsp/native_renderer_reset_process.hpp"
#include "bsp/native_renderer_resource_release.hpp"
#include "bsp/native_renderer_resource_restore.hpp"
#include "bsp/native_renderer_frame_targets.hpp"
#include "bsp/native_renderer_reset_readiness.hpp"
#include "bsp/native_logical_buffer_device_save.hpp"
#include "bsp/native_logical_buffer_device_restore.hpp"
#include "bsp/native_hardware_layout_construct.hpp"
#include "bsp/native_renderer_gamma.hpp"
#include "bsp/native_crt_pow_fallback.hpp"
#include "bsp/legacy_crt_math.hpp"
#include "bsp/native_texture_2d_retained_recreation.hpp"
#include "bsp/native_cube_volume_retained_recreation.hpp"
#include "bsp/native_renderer_begin_frame.hpp"
#include "bsp/native_renderer_end_frame.hpp"
#include "bsp/native_render_queue_destruction.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_material_effect_runtime.hpp"
#include "bsp/game_native_resource_pools.hpp"
// Constructor, device startup, frame and destructor borrow one application graph.
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_renderer_scalars.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_lua_services.hpp"
#include "bsp/game_native_vertex_declarations.hpp"
#include "bsp/game_native_type_storage.hpp"
#include "bsp/native_vfs_owner_services.hpp"
#include "bsp/native_renderer_parameters.hpp"
#include "bsp/native_xlive_device_adapter.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/system_camera_axes.hpp"
#include "bsp/native_cockpit_helper_construction.hpp"
#include "bsp/native_render_resources_lifetime.hpp"
#include <array>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <new>

namespace bsp::game {
namespace {
using U=std::uint32_t;
void check(bool value,const char* message) { if(!value) throw std::runtime_error(message); }
U bits(const void* p) noexcept { return reinterpret_cast<U>(p); }
template<class T> T get(const void* p,U offset=0) noexcept {
    T value;std::memcpy(&value,static_cast<const unsigned char*>(p)+offset,sizeof value);return value;
}
void* __cdecl raw_allocate(U size) { return singleton_lifetime_allocate({SingletonAllocationKind::object,size,size}); }
void __cdecl raw_free(void* p) noexcept { singleton_lifetime_free(p); }
struct CanonicalProfiles {
    GameNativeReadOnlyData& data;
    const U* operator()(U address) const { return static_cast<const U*>(data.data_at(address,80*4)); }
};
#include "game_native_renderer_lifetime.inc"
#include "game_native_renderer_device.inc"
#include "game_native_renderer_textures.inc"
#include "game_native_renderer_camera.inc"
#include "game_native_renderer_resources.inc"
#include "game_native_renderer_frame.inc"
} // namespace

struct GameNativeRendererApplication::Impl {
    enum class Phase { prepared, constructing, constructed, starting_device, initializing_window_cache, initializing_resources, ready, rendering, draining, failed, drained };
    GameHostLog& log;
    GameSingletonHost& singletons;
    GameNativeLuaServices& lua_services;
    GameNativeVfsRawServices vfs;
    NativeStringRawPoolContext& raw;
    CanonicalProfiles profiles;
    void* volatile renderer{};
    void* volatile lua_publication{};
    void* volatile system_publication{};
    NativeShaderStateDefinitionsStorage* volatile definitions{};
    void* volatile entry_cache_publication{};
    NativeRenderEntryCacheContext entry_cache;
    volatile std::uint8_t frame_flag{},device_lost{};
    volatile U& time_bits{game_native_renderer_scalar_process().renderer_worker_time_bits_0108d6e4()};
    NativeRendererLuaOwnerContext lua;
    NativeRendererControlWorkerContext control;
    const SingletonLifetimeCallbacks& validation;
    NativeRenderActualOwnerRegistry owners;
    NativeEffectRecordStorageContext records;
    NativeEffectRegistryDestructionContext effects;
    NativeRenderResourceAccountingTables accounting;
    NativeRendererCacheCleanupContext textures;
    ResourceLoadEventHost* volatile platform_events{};
    NativeVertexDeclarationCacheContext declaration_cache;
    NativeVertexDeclarationRegistryLifetimeContext declarations;
    // Explicit readable zero preimages for source-private COM output frames.
    // Native bodies still ignore HRESULTs and keep every original write/read.
    alignas(4) std::array<unsigned char,0x10> mode{};
    alignas(4) std::array<unsigned char,0x44c> identifier{};
    alignas(4) std::array<unsigned char,0x770> gather{};
    NativeRendererConstructorContext constructor;
    DestructionGraph graph;
    DeviceGraph devices;
    TextureLoadingGraph texture_loading;
    CameraGraph cameras;
    RenderResourcesGraph resources;
    std::unique_ptr<FrameGraph> frames;
    Phase phase{Phase::prepared};
    IDirect3D9* retained_api{};
    IDirect3DDevice9* retained_device{};
    HANDLE observed_worker{};

    Impl(GameHostLog& log_,GameSingletonHost& host,GameVfsHost& files,
        GameNativeLuaServices& services,GameNativeReadOnlyData& data,
        void* const volatile& clock,const void* platform)
        : log(log_),singletons(host),lua_services(services),vfs(files.borrow_raw_services()),
          raw(files.raw_strings()),profiles{data},
          entry_cache{host.manager_publication_01090aa0(),entry_cache_publication,*profiles(0xd7a24c)},
          lua{host.manager_publication_01090aa0(),lua_publication,vfs.strings,services.bootstrap()},
          control{clock,renderer,time_bits,profiles(0xd6821c),profiles(0xd68d50),profiles(0xd5f0a8),nullptr,nullptr,nullptr},
          validation(game_native_hardware_layout_tree_process().invalid_parameters()),
          records{vfs.strings,validation,&raw_allocate,&raw_free},
          effects{records,owners,profiles(0xd5f04c),profiles(0xd5f074),profiles(0xd5e534),profiles(0xd61a00)},
          accounting{profiles(0xd61948),profiles(0xd61870),profiles(0xd618b0)},
          textures{vfs.strings,validation,owners,accounting,profiles(0xd5f038),profiles(0xd5f088)},
          declaration_cache{vfs.strings,validation,game_native_vertex_declarations_process(data).loading(),
              vfs.dates,platform_events,profiles(0xd5f060),&raw_allocate,&raw_free},
          declarations{declaration_cache,owners,profiles(0xd5f024)},
          constructor{host.manager_publication_01090aa0(),raw.actual_published_01090aa8,
              raw.actual_small_returns_disabled_01090aa4,renderer,lua_publication,definitions,system_publication,
              frame_flag,device_lost,*profiles(0xd7a24c),services.bootstrap(),declarations,textures,effects,
              mode.data(),identifier.data(),gather.data()},
          graph(constructor,vfs.strings,raw,*host.native_deletion_bindings().resource_support,owners,
              validation,accounting,data,files.native_owners().types(),files.native_types().light_types(),vfs.retained_memory),
          devices(graph,constructor,host,vfs.strings,platform),
          texture_loading(graph,devices,owners,vfs,platform_events,validation,accounting),
          cameras(graph,renderer,files.native_owners().types(),files.native_types().camera_types()),
          resources(graph,cameras,texture_loading,host,raw,vfs,owners) {
        auto& deletion=host.native_deletion_bindings();
        check(!deletion.renderer_owner && !deletion.renderer_lua_owner,"renderer lifetime already bound");
        check(!deletion.render_entry_cache,"render-entry cache lifetime already bound");
        check(!deletion.render_resources,"render-resource lifetime already bound");
        bind_native_renderer_control_worker_process_context(control);
        deletion.renderer_owner=&graph.destructor;
        deletion.renderer_lua_owner=&lua;
        deletion.render_entry_cache=&entry_cache;
        deletion.render_resources=&resources.lifetime;
    }
    ~Impl() {
        if(phase!=Phase::prepared && phase!=Phase::drained) std::terminate();
        if(observed_worker) CloseHandle(observed_worker);
        const auto device_refs=retained_device ? retained_device->Release() : 0;
        const auto api_refs=retained_api ? retained_api->Release() : 0;
        log.notef("native renderer final COM release: device=%lu api=%lu",device_refs,api_refs);
    }
};
GameNativeRendererApplication::GameNativeRendererApplication(GameHostLog& log,GameSingletonHost& host,
    GameVfsHost& files,GameNativeLuaServices& lua,GameNativeReadOnlyData& data,
    void* const volatile& clock,const void* platform)
    :impl_(std::make_unique<Impl>(log,host,files,lua,data,clock,platform)) {}
GameNativeRendererApplication::~GameNativeRendererApplication()=default;
void* GameNativeRendererApplication::construct_render_resources() {
    auto& p=*impl_;check(p.phase==Impl::Phase::ready,"render resources require ready renderer/device");
    p.phase=Impl::Phase::initializing_resources;
    try {
        auto* owner=p.resources.construct();
        p.phase=Impl::Phase::ready;
        p.log.notef("native render resources B14A10: owner=%p textures=%p cockpit=%p",owner,
            owner ? get<void*>(owner,0x34) : nullptr,owner ? get<void*>(owner,0x0c) : nullptr);
        return owner;
    } catch(...) {p.phase=Impl::Phase::failed;throw;}
}
NativeRenderResourcesLifetimeContext& GameNativeRendererApplication::render_resources_lifetime() noexcept {
    return impl_->resources.lifetime;
}
const NativeRenderResourcesConstructionAcquired& GameNativeRendererApplication::render_resources_construction() const noexcept {
    return impl_->resources.acquired;
}
NativeCameraEnvironment& GameNativeRendererApplication::camera_environment() noexcept {
    return impl_->cameras.environment;
}
const NativeNodeRawConstants& GameNativeRendererApplication::node_constants() noexcept {
    return impl_->cameras.node_constants;
}
NativeViewportRegistry& GameNativeRendererApplication::viewport_registry() noexcept {
    return impl_->cameras.viewports;
}
const NativeCockpitViewportReleaseContext& GameNativeRendererApplication::cockpit_viewport_release() noexcept {
    return impl_->cameras.viewport_release;
}
NativeTextureCacheContext& GameNativeRendererApplication::texture_cache() noexcept {
    return impl_->texture_loading.cache;
}
NativeRenderActualOwnerRegistry& GameNativeRendererApplication::actual_owners() noexcept {
    return impl_->owners;
}
void GameNativeRendererApplication::construct() {
    auto& p=*impl_;check(p.phase==Impl::Phase::prepared,"renderer constructor is once-only");
    p.phase=Impl::Phase::constructing;
    try {
        void* storage=raw_allocate(0x1d94);check(storage!=nullptr,"renderer raw allocation returned null");
        // The native object has readable byte preimages; only the constructor's
        // recovered stores initialize its state. Zero is this application's
        // explicit preimage, not a claim about the original heap's old bytes.
        ::new(static_cast<unsigned char*>(storage)+0x17c0) CameraPlaneSet;
        std::memset(storage,0,0x1d94);
        NativeLuaServiceBindings::Activation activation(p.lua_services.binding());
        construct_native_renderer_00b32410(storage,&p.constructor);
        p.retained_api=get<IDirect3D9*>(storage,0x1990);
        check(p.retained_api!=nullptr,"native renderer factory is null");p.retained_api->AddRef();
        const auto* worker=get<NativeRendererControlWorkerStorage*>(storage,0x1970);
        check(worker && DuplicateHandle(GetCurrentProcess(),worker->thread_08,GetCurrentProcess(),
            &p.observed_worker,SYNCHRONIZE,FALSE,0),"observe native renderer worker lifetime");
        p.phase=Impl::Phase::constructed;
        p.log.notef("native renderer constructed: owner=%p api=%p lua=%p worker=%lu storage=actual1d94h",
            p.renderer,p.retained_api,p.lua_publication,static_cast<unsigned long>(worker->thread_id_14));
    } catch(...) {p.phase=Impl::Phase::failed;throw;}
}
void GameNativeRendererApplication::bind_platform_services(ResourceLoadEventHost& events,
    const volatile U* online,const NativeXLiveDeviceAdapter* adapter,const XLiveLibrary& library) {
    auto& p=*impl_;check(p.phase==Impl::Phase::constructed,"renderer platform services order");
    check(!p.frames && !p.singletons.native_deletion_bindings().render_queue,"renderer frame graph already bound");
    p.frames=std::make_unique<FrameGraph>(p.graph,p.devices,p.constructor,p.singletons,
        p.vfs.strings,p.raw,p.entry_cache_publication,library,p.resources.publication);
    p.singletons.native_deletion_bindings().render_queue=&p.frames->queue_destruction;
    p.control.begin_frame=&p.frames->begin;
    p.control.end_frame_00b2f4a0=&FrameGraph::worker_finish;
    p.control.end_frame_context=&p.frames->worker_end;
    p.platform_events=&events;p.devices.recreation.actual_online_publication_00f8abe8=online;
    p.devices.recreation.online_device=adapter;
}
void GameNativeRendererApplication::create_device(const RendererInitRequest& request) {
    auto& p=*impl_;check(p.phase==Impl::Phase::constructed,"native device startup is once-only");
    check(p.devices.recreation.actual_online_publication_00f8abe8!=nullptr,"native online publication is unbound");
    p.phase=Impl::Phase::starting_device;
    try {
        NativeRendererDeviceStartupSlots slots{bits(request.window),request.fullscreen,
            static_cast<U>(request.width),static_cast<U>(request.height),request.constant_15,
            request.constant_1,request.option,request.constant_4b,request.color_depth_selector,request.constant_0};
        NativeLuaServiceBindings::Activation activation(p.lua_services.binding());
        initialize_native_renderer_device_00b2aeb0(p.renderer,slots,p.devices.startup);
        p.retained_device=get<IDirect3DDevice9*>(p.renderer,0x1a10);
        check(p.retained_device && get<void*>(p.renderer,0x197c) && get<void*>(p.renderer,0x198c),"native default surfaces missing");
        p.retained_device->AddRef();p.phase=Impl::Phase::ready;
        p.log.notef("native renderer device startup: device=%p pending=%u lost=%u render_thread=%lu focused=%d",
            p.retained_device,p.frame_flag,p.device_lost,static_cast<unsigned long>(p.devices.render_thread),
            native_platform_has_focus_00b20c50(p.devices.platform_publication));
    } catch(...) {p.phase=Impl::Phase::failed;throw;}
}
void GameNativeRendererApplication::initialize_window_render_entry_cache() {
    auto& p=*impl_;
    check(p.phase==Impl::Phase::ready && !p.entry_cache_publication,"render-entry cache startup order");
    p.phase=Impl::Phase::initializing_window_cache;
    try {
        initialize_native_window_render_entry_cache_00bed1e8_fragment(p.entry_cache);
        void* const owner=p.entry_cache_publication;
        check(owner && get<U>(owner)==0xd68cc0 && get<void*>(owner,4)
            && get<U>(owner,8)==0 && get<U>(owner,0xc)==10000 && get<void*>(owner,0x10),
            "native render-entry cache startup incomplete");
        p.phase=Impl::Phase::ready;
        p.log.notef("native render-entry cache initialized: owner=%p records=%p used=%lu capacity=%lu section=%p storage=actual14h",
            owner,get<void*>(owner,4),static_cast<unsigned long>(get<U>(owner,8)),
            static_cast<unsigned long>(get<U>(owner,0xc)),get<void*>(owner,0x10));
    } catch(...) {p.phase=Impl::Phase::failed;throw;}
}
void GameNativeRendererApplication::copy_settings_capabilities(SettingsRendererCapabilities& output) const {
    const auto& p=*impl_;check(p.phase==Impl::Phase::constructed,"capability consumer order");
    auto* resolutions=get<const Resolution*>(p.renderer,0x1c);const U count=get<U>(p.renderer,0x20);
    output.resolutions.clear();if(count)output.resolutions.assign(resolutions,resolutions+count);
    auto* samples=get<const int*>(p.renderer,0x28);const U sample_count=get<U>(p.renderer,0x2c);
    output.antialias_levels.clear();if(sample_count)output.antialias_levels.assign(samples,samples+sample_count);
    output.adapter_mode_state_19dc=get<U>(p.renderer,0x19dc);
    output.pixel_shader_version_28=get<U>(p.renderer,0x1b40);
    output.max_shader_model=get<int>(p.renderer,0x1b48);
}
void GameNativeRendererApplication::begin_frame(U clear_color) {
    auto& p=*impl_;
    check(p.phase==Impl::Phase::ready && p.frames && p.entry_cache_publication,"native frame startup order");
    p.phase=Impl::Phase::rendering;
    try {
        NativeLuaServiceBindings::Activation activation(p.lua_services.binding());
        (void)begin_native_renderer_frame_00b2b200(p.renderer,p.frames->begin);
        clear_native_renderer_00b21430(p.renderer,&p.frames->clear,0,nullptr,
            D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,&clear_color,1.0f,0);
    } catch(...) {p.phase=Impl::Phase::failed;throw;}
}
NativeRendererPresentObservation GameNativeRendererApplication::end_frame() {
    auto& p=*impl_;check(p.phase==Impl::Phase::rendering,"native frame end order");
    try {
        NativeLuaServiceBindings::Activation activation(p.lua_services.binding());
        p.frames->present={};
        end_native_renderer_frame_default_00b2f4a0(p.renderer,&p.frames->end);
        p.phase=Impl::Phase::ready;
        p.log.notef("native frame complete: frame=%lu queue=%p commands=%ld entry_used=%lu present=%d hr=%08lx clear_request=%u inhibit=%lu unavailable=%u",
            static_cast<unsigned long>(get<U>(p.renderer,0x14)),p.frames->queue,
            static_cast<long>(p.frames->queue->commands_14.count_04),
            static_cast<unsigned long>(get<U>(p.entry_cache_publication,8)),p.frames->present.returned,
            static_cast<unsigned long>(p.frames->present.result),p.frames->clear_request,
            static_cast<unsigned long>(get<U>(p.renderer,0x1d90)),get<std::uint8_t>(p.renderer,0x1d8a));
        return p.frames->present;
    } catch(...) {p.phase=Impl::Phase::failed;throw;}
}
IDirect3D9& GameNativeRendererApplication::api() const {check(impl_->retained_api!=nullptr,"native renderer API unavailable");return *impl_->retained_api;}
IDirect3DDevice9* GameNativeRendererApplication::device() const noexcept {return impl_->retained_device;}
NativeRendererParametersOwner& GameNativeRendererApplication::parameters() const {
    check(impl_->renderer!=nullptr,"native renderer parameters unavailable");
    return *reinterpret_cast<NativeRendererParametersOwner*>(static_cast<unsigned char*>(impl_->renderer)+0x1a14);
}
const D3DPRESENT_PARAMETERS& GameNativeRendererApplication::presentation() const {
    check(impl_->phase==Impl::Phase::ready,"native presentation unavailable");
    return *reinterpret_cast<const D3DPRESENT_PARAMETERS*>(static_cast<const unsigned char*>(impl_->renderer)+0x1a28);
}
bool GameNativeRendererApplication::requires_process_retention() const noexcept {
    const auto phase=impl_->phase;
    return phase!=Impl::Phase::prepared && phase!=Impl::Phase::ready && phase!=Impl::Phase::drained;
}
void GameNativeRendererApplication::drain_singletons() {
    check(!requires_process_retention(),"incomplete native renderer cannot be drained");
    NativeLuaServiceBindings::Activation activation(impl_->lua_services.binding());
    if(impl_->phase==Impl::Phase::ready)impl_->phase=Impl::Phase::draining;
    try { impl_->singletons.shutdown();after_native_drain(); }
    catch(...) {impl_->phase=Impl::Phase::failed;throw;}
}
void GameNativeRendererApplication::after_native_drain() {
    auto& p=*impl_;if(p.phase==Impl::Phase::drained || p.phase==Impl::Phase::prepared)return;
    check(!p.renderer && !p.lua_publication && !p.system_publication && !p.definitions,"native renderer children survived drain");
    check(!p.entry_cache_publication,"native render-entry cache survived drain");
    check(!p.frames || !p.frames->queue,"native render queue survived drain");
    p.resources.after_native_drain();
    check(WaitForSingleObject(p.observed_worker,0)==WAIT_OBJECT_0,"native renderer worker did not join");
    p.phase=Impl::Phase::drained;
    p.log.note("native renderer after raw drain: owner=null lua=null definitions=null system=null entry_cache=null queue=null worker=joined");
}
} // namespace bsp::game
