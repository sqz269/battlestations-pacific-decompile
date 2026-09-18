#include "bsp/game_native_dyn_process.hpp"
#include "bsp/native_dyn_box_box.hpp"
#include "bsp/native_dyn_terrain_convex.hpp"
#include "bsp/native_dyn_primitive_dispatch.hpp"
#include "bsp/native_dyn_general_convex.hpp"
#include "bsp/native_dyn_convex_ray.hpp"
#include "bsp/native_dyn_sap_processing.hpp"
#include "bsp/native_dyn_narrow_phase.hpp"
#include "bsp/native_dyn_solver_mode0.hpp"
#include "bsp/native_dyn_solver_mode1.hpp"
#include "bsp/native_dyn_convex_pool.hpp"
#include "bsp/native_dyn_convex_support.hpp"
#include <cstdlib>
#include <mutex>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Dyn process ownership requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
void* allocate(void*,std::size_t bytes){return ::operator new(bytes);}
void release(void*,void* p){::operator delete(p);}
const AvoidZoneDynHullMemory process_memory{nullptr,&allocate,&release};
}
struct GameNativeDynProcess::Impl {
    AvoidZoneDynHullMemory memory;
    CameraAxesCrtAccess crt;
    void* engine_0109e9fc{};
    void* profile_0109e9f8{};
    void* general_convex_0109e9f4{};
    DynDispatchGlobalsStorage dispatch;
    NativeDynSapPairCalls sap_calls;
    NativeDynSapLifetimeProgress sap_progress;
    NativeDynNarrowPhaseCalls narrow_calls;
    NativeDynSolverMode0Calls solver0_calls;
    NativeDynSolverMode1Calls solver1_calls;
    DynProfileScopeContext profile_context;
    NativeDynBoxBoxRuntime boxes;
    NativeDynTerrainConvexRuntime terrain;
    NativeDynPrimitiveDispatchRuntime primitives;
    NativeDynGeneralConvexRuntime general;
    NativeDynConvexRayRuntime convex_ray;
    NativeDynSapRuntime sap;
    NativeDynIntersectTaskRuntime intersect;
    NativeDynSolverMode0Runtime solver0;
    NativeDynSolverMode1Runtime solver1;
    DynDispatchVtables tables;
    // D7A080 has one original CRT __purecall slot (BF698E). Consume the
    // platform CRT entry; constructors replace this base table before use.
    const std::uintptr_t base_task_table[1];
    DynEngineRuntimeContext engine;
    DynSceneRuntimeContext scene;
    DynWorldRuntimeContext world;
    NativeGameDynamicsContext bindings;
    std::mutex startup_mutex;
    enum class StartupState { unattempted,returned,threw };
    StartupState state{StartupState::unattempted};
    int registration_status{};
    struct ConvexBinding {
        AllocatorListDomain& list;
        const volatile std::uint32_t& conversion;
        DynConvexShapePoolStorage storage{};
        NativeDynConvexPool pool;
        NativeDynConvexShapeRuntime shape;
        DynBodyCreationContext body;
        ConvexBinding(AllocatorListDomain& l,const volatile std::uint32_t& c,
            const AvoidZoneDynHullMemory& m):list(l),conversion(c),pool(l,storage,m),
            shape(storage,c),body{m,&storage,shape.table()} {}
    };
    std::unique_ptr<ConvexBinding> convex;
    StartupState convex_state{StartupState::unattempted};
    int convex_registration_status{};

    Impl(const CameraAxesCrtAccess& c,const AvoidZoneDynHullMemory& m)
        :memory(m),crt(c),profile_context{memory,&profile_0109e9f8},
         boxes(crt),terrain(crt),primitives(crt),general(crt),convex_ray(crt),
         sap(memory,sap_calls,sap_progress),intersect(memory,narrow_calls),
         solver0(memory,solver0_calls,crt),solver1(profile_context,solver1_calls,crt),
         tables{boxes.table(),terrain.table(),primitives.sphere_sphere_table(),
             primitives.box_sphere_table(),general.table(),convex_ray.table(),
             primitives.box_ray_table(),primitives.sphere_ray_table()},
         base_task_table{reinterpret_cast<std::uintptr_t>(&_purecall)},
         engine{memory,&engine_0109e9fc,&profile_0109e9f8,"Root"},
         scene{memory,&engine_0109e9fc,&general_convex_0109e9f4,
             dyn_scene_dispatch_objects(dispatch),sap.table(),intersect.table()},
         world{&scene,base_task_table,solver0.table(),solver1.table()},
         bindings{engine,world} {
        if(!memory.allocate||!memory.release||!crt.dispatch_bypass_0109dd78||!crt.except_00c27489)
            throw std::invalid_argument("Dyn process requires actual allocator and CRT services");
        bind_dyn_dispatch_static_objects(dispatch,tables);
    }
    void require_initialized() const {
        if(state!=StartupState::returned)
            throw std::logic_error("Dyn process requires completed explicit startup");
    }
};

GameNativeDynProcess::GameNativeDynProcess(const CameraAxesCrtAccess& c,const AvoidZoneDynHullMemory& m)
    :impl_(std::make_unique<Impl>(c,m)) {}
GameNativeDynProcess::~GameNativeDynProcess()=default;

GameNativeDynProcess& game_native_dyn_process(const CameraAxesCrtAccess& c,const AvoidZoneDynHullMemory& m){
    static GameNativeDynProcess process(c,m);
    const auto& i=*process.impl_;
    if(i.memory.context!=m.context||i.memory.allocate!=m.allocate||i.memory.release!=m.release
        ||i.crt.dispatch_bypass_0109dd78!=c.dispatch_bypass_0109dd78||i.crt.except_00c27489!=c.except_00c27489)
        throw std::logic_error("Dyn process services cannot be rebound");
    return process;
}
GameNativeDynProcess& game_native_dyn_process(const CameraAxesCrtAccess& c){
    return game_native_dyn_process(c,process_memory);
}
int GameNativeDynProcess::initialize_once_00cc8950(){
    auto& i=*impl_;std::lock_guard lock(i.startup_mutex);
    if(i.state==Impl::StartupState::returned)return i.registration_status;
    if(i.state==Impl::StartupState::threw)throw std::logic_error("Dyn dispatch startup previously threw");
    i.state=Impl::StartupState::threw;
    bind_static_dyn_dispatch_globals_0109ea48(i.dispatch);
    i.registration_status=initialize_static_dyn_dispatch_00cc8950(i.tables.general_convex_00d7a1a8,i.crt,&std::atexit);
    i.state=Impl::StartupState::returned;
    return i.registration_status;
}
const NativeGameDynamicsContext& GameNativeDynProcess::dynamics(){
    auto& i=*impl_;std::lock_guard lock(i.startup_mutex);i.require_initialized();return i.bindings;
}
const DynDispatchVtables& GameNativeDynProcess::dispatch_tables(){
    auto& i=*impl_;std::lock_guard lock(i.startup_mutex);i.require_initialized();return i.tables;
}
int GameNativeDynProcess::initialize_convex_pool_once_00cc89c0(AllocatorListDomain& list,
    const volatile std::uint32_t& conversion){
    auto& i=*impl_;std::lock_guard lock(i.startup_mutex);i.require_initialized();
    if(i.convex&&(&i.convex->list!=&list||&i.convex->conversion!=&conversion))
        throw std::logic_error("Dyn convex pool services cannot be rebound");
    if(i.convex_state==Impl::StartupState::returned)return i.convex_registration_status;
    if(i.convex_state==Impl::StartupState::threw)throw std::logic_error("Dyn convex pool startup previously threw");
    i.convex_state=Impl::StartupState::threw;
    i.convex=std::make_unique<Impl::ConvexBinding>(list,conversion,i.memory);
    bind_static_native_dyn_convex_pool_0109ecf0(i.convex->pool);
    i.convex_registration_status=initialize_static_native_dyn_convex_pool_00cc89c0();
    i.convex_state=Impl::StartupState::returned;return i.convex_registration_status;
}
const DynBodyCreationContext& GameNativeDynProcess::body_creation(){
    auto& i=*impl_;std::lock_guard lock(i.startup_mutex);i.require_initialized();
    if(i.convex_state!=Impl::StartupState::returned)
        throw std::logic_error("Dyn convex bodies require completed pool startup");
    return i.convex->body;
}
} // namespace bsp::game
