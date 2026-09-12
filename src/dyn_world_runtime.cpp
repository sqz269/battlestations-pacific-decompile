#include "bsp/dyn_world_runtime.hpp"
#include "bsp/dyn_body_creation.hpp"

#include <cstring>
#include <initializer_list>
#include <limits>
#include <new>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(DynWorldDescriptor)==0x40);
static_assert(offsetof(DynWorldDescriptor,solver_word_20)==0x20);
static_assert(offsetof(DynWorldDescriptor,solver_float_3c)==0x3c);
namespace {
template<class T> T& at(void* p,std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset);
}
void* plus(void* p,std::size_t offset) {
    return static_cast<unsigned char*>(p)+offset;
}
void copy_word(void* destination,const void* source) {
    std::memcpy(destination,source,4);
}
void copy_x87_float(void* destination,const void* source) {
    __asm {
        mov eax,source
        fld dword ptr [eax]
        mov eax,destination
        fstp dword ptr [eax]
    }
}
void* allocate(const AvoidZoneDynHullMemory& memory,std::uint32_t count,std::uint32_t stride) {
    if (count>(std::numeric_limits<std::uint32_t>::max)()/stride)
        throw std::length_error("native Dyn allocation extent overflows Win32 size");
    void* result=memory.allocate(memory.context,static_cast<std::size_t>(count)*stride);
    if (!result) throw std::bad_alloc();
    return result;
}
void resize_tasks(void* vector,std::uint32_t requested,const void* base_vtable,
    const void* derived_vtable,const AvoidZoneDynHullMemory& memory) {
    auto& data=at<void*>(vector,0);
    auto& count=at<std::uint32_t>(vector,4);
    auto& capacity=at<std::uint32_t>(vector,8);
    if (count<requested) {
        if (capacity<requested) {
            capacity=requested;
            void* replacement=allocate(memory,requested,0x18);
            for (std::uint32_t i=0;i<count;++i) {
                void* target=plus(replacement,i*0x18);
                void* source=plus(data,i*0x18);
                at<const void*>(target,0)=base_vtable;
                copy_word(plus(target,4),plus(source,4));
                at<const void*>(target,0)=derived_vtable;
                copy_word(plus(target,8),plus(source,8));
                copy_word(plus(target,0xc),plus(source,0xc));
                copy_word(plus(target,0x10),plus(source,0x10));
                copy_x87_float(plus(target,0x14),plus(source,0x14));
            }
            if (data) memory.release(memory.context,data);
            data=replacement;
        }
        for (std::uint32_t i=count;i<requested;++i)
            at<const void*>(data,i*0x18)=derived_vtable;
    }
    count=requested;
}
void resize_task_pointers(void* vector,std::uint32_t requested,const AvoidZoneDynHullMemory& memory) {
    auto& data=at<void**>(vector,0);
    auto& count=at<std::uint32_t>(vector,4);
    auto& capacity=at<std::uint32_t>(vector,8);
    if (count<requested) {
        if (capacity<requested) {
            capacity=requested;
            auto* replacement=static_cast<void**>(allocate(memory,requested,4));
            for (std::uint32_t i=0;i<count;++i) replacement[i]=data[i];
            if (data) memory.release(memory.context,data);
            data=replacement;
        }
        for (std::uint32_t i=count;i<requested;++i) data[i]=nullptr;
    }
    count=requested;
}
void* engine_pointer(void* const* global_engine) {
    if (!global_engine || !*global_engine)
        throw std::invalid_argument("Dyn world requires its initialized engine global");
    return *global_engine;
}
std::uint32_t worker_count(void* engine) {
    void* manager=at<void*>(engine,0x10);
    if (!manager) throw std::invalid_argument("Dyn world requires its actual task manager");
    return at<std::uint32_t>(manager,4);
}
} // namespace

DynWorldStorage* dyn_world_storage_construct_00c41ad0(DynWorldStorage& storage,
    const DynWorldDescriptor& descriptor,const DynWorldRuntimeContext& context) {
    if (!context.scene || !context.task_vtable_00d7a080
        || !context.lcp_solver_task_vtable_00d7a088 || !context.lcp_solver2_task_vtable_00d7a090)
        throw std::invalid_argument("Dyn world requires complete native scene and task inputs");
    void* world=&storage;
    const auto* desc=reinterpret_cast<const unsigned char*>(&descriptor);
    copy_x87_float(plus(world,0),desc);
    copy_word(plus(world,4),desc+4);
    copy_word(plus(world,8),desc+8);
    copy_word(plus(world,0xc),desc+0xc);
    copy_word(plus(world,0x10),desc+0x20);
    copy_x87_float(plus(world,0x14),desc+0x28);
    copy_x87_float(plus(world,0x18),desc+0x2c);
    copy_x87_float(plus(world,0x1c),desc+0x30);
    copy_x87_float(plus(world,0x20),desc+0x34);
    copy_x87_float(plus(world,0x28),desc+0x3c);
    copy_word(plus(world,0x34),desc+0x10);
    copy_word(plus(world,0x38),desc+0x24);
    copy_x87_float(plus(world,0x3c),desc+0x14);
    copy_x87_float(plus(world,0x40),desc+0x18);
    copy_word(plus(world,0x44),desc+0x1c);
    const auto& scene_context=*context.scene;
    const auto& memory=scene_context.memory;
    dyn_body_pool_construct_00409170(plus(world,0x4c),memory);
    dyn_body_pool_construct_00409170(plus(world,0x170),memory);
    dyn_motion_pool_construct_00409450(plus(world,0x294),memory);
    for (std::size_t offset : {0x438u,0x43cu,0x440u,0x44cu,0x450u,0x454u})
        at<std::uint32_t>(world,offset)=0;
    at<void*>(world,0x448)=world;
    for (std::size_t offset=0x45c;offset<=0x488;offset+=4)
        at<std::uint32_t>(world,offset)=0;
    auto* scene=static_cast<DynSceneStorage*>(allocate(memory,1,sizeof(DynSceneStorage)));
    at<DynSceneStorage*>(world,0x444)=dyn_scene_construct_00c38070(*scene,world,scene_context);
    void* first_engine=engine_pointer(scene_context.engine_slot);
    at<std::uint32_t>(world,0x48)=0;
    at<std::uint32_t>(world,0x2c)=0;
    at<std::uint32_t>(world,0x24)=0;
    const std::uint32_t first_count=worker_count(first_engine);
    resize_tasks(plus(world,0x45c),first_count,context.task_vtable_00d7a080,
        context.lcp_solver_task_vtable_00d7a088,memory);
    resize_task_pointers(plus(world,0x468),first_count,memory);
    const std::uint32_t second_count=worker_count(engine_pointer(scene_context.engine_slot));
    resize_tasks(plus(world,0x474),second_count,context.task_vtable_00d7a080,
        context.lcp_solver2_task_vtable_00d7a090,memory);
    resize_task_pointers(plus(world,0x480),second_count,memory);
    // Native engine configuration stays fixed while a world is constructed.
    // A concurrent shrink would make the original loop overrun its second array.
    if (second_count<first_count)
        throw std::invalid_argument("Dyn task count shrank during world construction");
    for (std::uint32_t i=0;i<at<std::uint32_t>(world,0x460);++i) {
        void* first=plus(at<void*>(world,0x45c),i*0x18);
        at<void*>(first,8)=world;
        at<void**>(world,0x468)[i]=first;
        void* second=plus(at<void*>(world,0x474),i*0x18);
        at<void*>(second,8)=world;
        at<void**>(world,0x480)[i]=second;
    }
    dyn_world_allocate_shared_motion_00c41fa4(world,memory);
    return &storage;
}
} // namespace bsp
