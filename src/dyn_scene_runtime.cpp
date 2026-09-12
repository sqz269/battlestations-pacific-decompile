#include "bsp/dyn_scene_runtime.hpp"
#include "bsp/dyn_collision_pass.hpp"
#include <cstring>
#include <new>

namespace bsp {
namespace {
template<class T> T& at(void* p, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset);
}
void* plus(void* p,std::size_t offset) { return static_cast<unsigned char*>(p)+offset; }
void* allocate(const AvoidZoneDynHullMemory& memory,std::size_t size) {
    void* result=memory.allocate(memory.context,size);
    if (!result) throw std::bad_alloc();
    return result;
}
} // namespace

void dyn_manifold_pool_construct_0040a1e0(DynManifoldContainerStorage& storage,
    const AvoidZoneDynHullMemory& memory) {
    void* pool=&storage;
    at<void*>(pool,0)=nullptr;
    at<std::uint32_t>(pool,4)=at<std::uint32_t>(pool,8)=0;
    at<void*>(pool,0x1c8)=plus(pool,0x10);
    at<void*>(pool,0x1cc)=nullptr;
    at<void*>(pool,0xe8)=nullptr;
    at<void*>(pool,0xec)=plus(pool,0xf0);
    at<void*>(pool,0xc)=nullptr;
    at<std::uint32_t>(pool,0x1d0)=0;

    void* page=allocate(memory,0x36b00);
    for (std::size_t i=0;i<999;++i)
        at<void*>(page,i*0xe0+0xdc)=plus(page,(i+1)*0xe0);
    at<void*>(page,0x36afc)=nullptr;
    at<void*>(pool,0xc)=page;
    // All three vector words were just initialized above. The native inlined
    // reserve's copy/free branches cannot run for this fresh constructor.
    at<std::uint32_t>(pool,8)=2;
    auto** pages=static_cast<void**>(allocate(memory,8));
    at<void*>(pool,0)=pages;
    pages[0]=page;
    at<std::uint32_t>(pool,4)=1;
}

DynSceneStorage* dyn_scene_construct_00c38070(DynSceneStorage& storage,void* world,
    const DynSceneRuntimeContext& context) {
    void* scene=&storage;
    const auto& dispatch=context.dispatch;
    std::memset(plus(scene,kDynScenePairArrayOffset),0,0x30);
    at<std::uint32_t>(scene,kDynSceneEventSpinLockOffset)=0;
    *context.general_convex_slot=dispatch.general_convex;
    at<void*>(scene,kDynSceneBodyContainerOffset)=world;
    std::memset(scene,0,0x90);

    auto** pairs=static_cast<void**>(scene);
    pairs[6]=pairs[1]=dispatch.box_sphere;
    pairs[34]=pairs[29]=dispatch.terrain_convex_mesh;
    pairs[0]=dispatch.sphere_sphere;
    pairs[7]=dispatch.box_box;
    for (const unsigned index:{10u,25u,8u,13u,14u,2u,12u,28u,24u,4u,26u,16u})
        pairs[index]=dispatch.general_convex;
    at<void*>(scene,0x9c)=nullptr;
    at<void*>(scene,0xa4)=nullptr;
    at<void*>(scene,0x94)=dispatch.box_ray;
    at<void*>(scene,0xa0)=at<void*>(scene,0x98)=dispatch.convex_ray;
    at<void*>(scene,0x90)=dispatch.sphere_ray;

    void* manager=allocate(context.memory,0x248);
    dyn_sap_manager_construct_00c36f10(manager,context.sap_manager_vtable,context.memory);
    at<void*>(scene,kDynSceneBroadPhaseOffset)=manager;
    auto* manifolds=static_cast<DynManifoldContainerStorage*>(allocate(context.memory,0x1f0));
    dyn_manifold_pool_construct_0040a1e0(*manifolds,context.memory);
    InitializeCriticalSectionAndSpinCount(&at<CRITICAL_SECTION>(manifolds,0x1d8),10000);
    at<void*>(manifolds,0x1d4)=world;

    // Assembly 00C381FC loads the global engine before storing the container,
    // then reads engine+10 and pool+4 at 00C38208/00C3820B.
    void* engine=*context.engine_slot;
    at<void*>(scene,0xb0)=manifolds;
    void* thread_pool=at<void*>(engine,0x10);
    const auto workers=at<std::uint32_t>(thread_pool,4);
    if (workers) {
        at<std::uint32_t>(scene,0xc8)=workers;
        void* tasks=allocate(context.memory,workers*kDynSceneNarrowTaskStride);
        at<void*>(scene,kDynSceneNarrowTaskArrayOffset)=tasks;
        for (std::uint32_t i=0;i<workers;++i)
            at<const void*>(tasks,i*kDynSceneNarrowTaskStride)=context.intersect_task_vtable;
    }
    at<std::uint32_t>(scene,0xc4)=workers;
    if (workers) {
        at<std::uint32_t>(scene,0xd4)=workers;
        auto** task_pointers=static_cast<void**>(allocate(context.memory,workers*4));
        at<void*>(scene,0xcc)=task_pointers;
        std::memset(task_pointers,0,workers*4);
    }
    at<std::uint32_t>(scene,0xd0)=workers;
    for (std::uint32_t i=0;i<workers;++i) {
        void* task=plus(at<void*>(scene,kDynSceneNarrowTaskArrayOffset),i*kDynSceneNarrowTaskStride);
        at<void*>(task,8)=scene;
        at<void**>(scene,0xcc)[i]=task;
    }
    // Tasks leave +4/+C/+10 untouched; the event buffer is reserved, not resized.
    at<std::uint32_t>(scene,kDynSceneEventCapacityOffset)=1000;
    at<void*>(scene,kDynSceneEventArrayOffset)=allocate(context.memory,12000);
    return &storage;
}
} // namespace bsp
