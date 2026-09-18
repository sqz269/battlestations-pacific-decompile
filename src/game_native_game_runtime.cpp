#include "bsp/game_native_game_runtime.hpp"
#include "bsp/game_native_dyn_process.hpp"
#include "bsp/native_game_construction.hpp"
#include "bsp/native_game_lifetime.hpp"
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native game ownership requires MSVC Win32.
#endif
namespace bsp::game {
struct GameNativeGameRuntime::Impl {
    NativeGameConstructionContext construction;
    NativeGameLifetimeContext lifetime;
    const NativeDynWorldStepContext& step;
    NativeGameSourceTables tables;
    NativeGameConstructionOperation construct_operation;
    NativeGameLifetimeOperation destroy_operation;
    NativeGameStorage* owner{};
    Phase phase{Phase::fresh};
    Impl(const NativeGameConstructionContext& c,const NativeGameLifetimeContext& l,
        GameNativeDynProcess& p,const void* table)
        :construction(c),lifetime(l),step(p.world_step()),tables{table,p.contact_reports().table()} {
        const auto& dynamics=p.dynamics();
        if(&c.actual_game_00e188a8!=&l.game_00e188a8
            ||&c.actual_00e19b0c!=&l.grid_00e19b0c
            ||&c.actual_00e19b08!=&l.grid_00e19b08
            ||&c.actual_00e19b04!=&l.grid_00e19b04
            ||&c.dynamics.engine!=&dynamics.engine||&c.dynamics.world!=&dynamics.world
            ||!l.physics||&l.physics->calls!=&l.calls||&l.physics->dynamics.engine!=&dynamics.engine
            ||&l.physics->dynamics.world!=&dynamics.world)
            throw std::invalid_argument("Native game requires shared game/grid publications and canonical Dyn construction/lifetime owners");
        construction.source_tables=&tables;
        lifetime.source_primary_table=table;
    }
};
GameNativeGameRuntime::GameNativeGameRuntime(const NativeGameConstructionContext& c,
    const NativeGameLifetimeContext& l,GameNativeDynProcess& p)
    :methods_{reinterpret_cast<std::uintptr_t>(&scalar)},impl_(new Impl(c,l,p,methods_)) {
    static_assert(std::is_standard_layout_v<GameNativeGameRuntime>);
    static_assert(offsetof(GameNativeGameRuntime,methods_)==0);
}
GameNativeGameRuntime::~GameNativeGameRuntime(){
    if(impl_->phase!=Phase::fresh&&impl_->phase!=Phase::destroyed&&impl_->phase!=Phase::diagnostic_retired)
        std::terminate();
    delete impl_;
}
NativeGameStorage* GameNativeGameRuntime::construct(NativeGameStorage& game,const void* name){
    auto& i=*impl_;if(i.phase!=Phase::fresh)throw std::logic_error("Native game construction is one-shot");
    i.owner=&game;i.phase=Phase::constructing;
    try{auto* result=construct_native_game_004ddb90(game,name,i.construction,i.construct_operation);i.phase=Phase::live;return result;}
    catch(...){i.phase=Phase::failed;throw;}
}
NativeGameStorage* GameNativeGameRuntime::scalar_delete(std::uint32_t flags){
    auto& i=*impl_;if(i.phase!=Phase::live)throw std::logic_error("Native game deletion requires its live constructed owner");
    i.phase=Phase::destroying;
    try{auto* result=delete_native_game_004de270(*i.owner,flags,i.lifetime,i.destroy_operation);i.phase=Phase::destroyed;return result;}
    catch(...){i.phase=Phase::failed;throw;}
}
void* __fastcall GameNativeGameRuntime::scalar(void* game,void*,std::uint32_t flags){
    auto* runtime=reinterpret_cast<GameNativeGameRuntime*>(*static_cast<void**>(game));
    if(runtime->impl_->owner!=game)throw std::logic_error("Native game table belongs to another owner");
    return runtime->scalar_delete(flags);
}
void GameNativeGameRuntime::simulate_physics_00875e0c(float dt){
    auto& i=*impl_;if(i.phase!=Phase::live)throw std::logic_error("Native game physics requires completed construction");
    void* game=i.construction.actual_game_00e188a8;
    if(game!=i.owner)throw std::logic_error("Native game physics requires its current actual publication");
    void* world=*reinterpret_cast<void* volatile*>(static_cast<std::byte*>(game)+0x18);
    simulate_native_dyn_world_00c5c540(world,dt,i.step);
}
GameNativeGameRuntime::Phase GameNativeGameRuntime::phase() const noexcept{return impl_->phase;}
NativeGameStorage* GameNativeGameRuntime::storage() const noexcept{return impl_->owner;}
NativeGameConstructionOperation& GameNativeGameRuntime::construction_operation() noexcept{return impl_->construct_operation;}
NativeGameLifetimeOperation& GameNativeGameRuntime::lifetime_operation() noexcept{return impl_->destroy_operation;}
void GameNativeGameRuntime::acknowledge_diagnostic_cleanup(){
    auto& i=*impl_;if(i.phase!=Phase::failed)throw std::logic_error("Only a failed native game operation can be diagnostically retired");
    i.construct_operation.acknowledge_diagnostic_cleanup();i.destroy_operation.acknowledge_diagnostic_cleanup();
    i.phase=Phase::diagnostic_retired;
}
} // namespace bsp::game
