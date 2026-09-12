#include "bsp/dyn_engine_runtime.hpp"
#include "bsp/dyn_task_manager.hpp"
#include "bsp/native_legacy_sbo_string.hpp"

#include <cstring>

namespace bsp {
static_assert(sizeof(DynProfileNodeStorage)==0x48);
static_assert(sizeof(DynProfileStorage)==0x9c);
static_assert(sizeof(DynEngineStorage)==0x14);
static_assert(sizeof(DynTaskManagerStorage)==0x358);
namespace {
template<class T> T& at(void* storage,std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(storage)+offset);
}
void* allocate(const DynEngineRuntimeContext& context,std::size_t bytes) {
    return context.memory.allocate(context.memory.context,bytes);
}
} // namespace

DynProfileNodeStorage* dyn_profile_node_construct_00c44000(
    DynProfileNodeStorage& node,const char* name,std::uint32_t word_2c) {
    at<std::uint32_t>(&node,4)=0;
    at<std::uint32_t>(&node,8)=0;
    at<std::uint32_t>(&node,0xc)=0;
    auto& text=at<NativeLegacySboStringStorage>(&node,0x10);
    text.capacity_18=15;
    text.length_14=0;
    text.buffer_04.inline_bytes[0]=0;
    const char* end=name;
    while (*end++) {}
    const auto length=static_cast<std::uint32_t>(end-name-1);
    native_legacy_sbo_string_assign_counted_00408720(text,name,length);
    at<std::uint32_t>(&node,0x2c)=word_2c;
    at<std::uint32_t>(&node,0x38)=0;
    at<std::uint32_t>(&node,0x3c)=0;
    at<std::uint32_t>(&node,0x40)=0;
    at<std::uint32_t>(&node,0x30)=0;
    at<std::uint32_t>(&node,0x34)=0;
    return &node;
}

DynProfileStorage* dyn_profile_construct_00c50310(
    DynProfileStorage& profile,const DynEngineRuntimeContext& context) {
    auto* node=static_cast<DynProfileNodeStorage*>(allocate(context,sizeof(DynProfileNodeStorage)));
    if (node) node=dyn_profile_node_construct_00c44000(*node,context.root_name_00d79dc8,0);
    at<DynProfileNodeStorage*>(&profile,0)=node;
    at<DynProfileNodeStorage*>(&profile,4)=node;
    at<std::uint32_t>(&profile,8)=0;
    *context.profile_slot_0109e9f8=&profile;
    std::memset(profile.bytes+0xc,0,0x90);
    return &profile;
}

DynEngineStorage* dyn_engine_construct_00c55ea0(DynEngineStorage& engine,
    const std::uint32_t* descriptor,const DynEngineRuntimeContext& context) {
    at<std::uint32_t>(&engine,0)=0;
    at<std::uint32_t>(&engine,4)=0;
    at<std::uint32_t>(&engine,8)=0;
    auto* profile=static_cast<DynProfileStorage*>(allocate(context,sizeof(DynProfileStorage)));
    if (profile) profile=dyn_profile_construct_00c50310(*profile,context);
    at<DynProfileStorage*>(&engine,0xc)=profile;
    auto* tasks=static_cast<DynTaskManagerStorage*>(allocate(context,sizeof(DynTaskManagerStorage)));
    if (tasks) {
        // Native snapshots *descriptor into EDX before entering00C37740.
        const std::uint32_t worker_count=*descriptor;
        tasks=dyn_task_manager_construct_00c37740(*tasks,worker_count,context.memory);
    }
    at<DynTaskManagerStorage*>(&engine,0x10)=tasks;
    return &engine;
}

DynEngineStorage* dyn_engine_ensure_00c55f50(
    const std::uint32_t* descriptor,const DynEngineRuntimeContext& context) {
    auto* engine=static_cast<DynEngineStorage*>(*context.engine_slot_0109e9fc);
    if (engine) return engine;
    engine=static_cast<DynEngineStorage*>(allocate(context,sizeof(DynEngineStorage)));
    if (engine) engine=dyn_engine_construct_00c55ea0(*engine,descriptor,context);
    *context.engine_slot_0109e9fc=engine;
    return engine;
}
} // namespace bsp
