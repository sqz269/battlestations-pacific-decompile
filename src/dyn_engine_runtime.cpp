#include "bsp/dyn_engine_runtime.hpp"
#include "bsp/dyn_task_manager.hpp"
#include "bsp/native_legacy_sbo_string.hpp"

#include <cstring>
#include <stdexcept>

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
// 0040A900..0040A90D: EAX actual vector, RET. Capture its CURRENT data once,
// call the actual BF6989 provider only when nonzero, and leave fields alone.
void release_profile_vector_storage_0040a900(
    void* actual_vector, const AvoidZoneDynHullMemory& vector_memory) {
    void* const data = *reinterpret_cast<void* volatile*>(actual_vector);
    if (data) vector_memory.release(vector_memory.context,data);
}

void initialize_profile_node_vector(DynProfileNodeStorage& node) {
    at<std::uint32_t>(&node,4)=0;
    at<std::uint32_t>(&node,8)=0;
    at<std::uint32_t>(&node,0xc)=0;
}

DynProfileNodeStorage* construct_profile_node_payload(
    DynProfileNodeStorage& node,const char* name,std::uint32_t word_2c) {
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
} // namespace

DynProfileNodeStorage* dyn_profile_node_construct_00c44000(
    DynProfileNodeStorage& node,const char* name,std::uint32_t word_2c) {
    initialize_profile_node_vector(node);
    return construct_profile_node_payload(node,name,word_2c);
}

DynProfileNodeStorage* dyn_profile_node_construct_00c44000(
    DynProfileNodeStorage& node,const char* name,std::uint32_t word_2c,
    const AvoidZoneDynHullMemory& vector_memory) {
    initialize_profile_node_vector(node);
    // E04014/map E0400C: state0 starts at C4402B AFTER vector zeroing.
    try {
        return construct_profile_node_payload(node,name,word_2c);
    } catch (...) {
        // CC81E7 reads the original node argument, adds4 and tail-jumps A900.
        release_profile_vector_storage_0040a900(
            static_cast<unsigned char*>(static_cast<void*>(&node))+4,vector_memory);
        throw;
    }
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

DynProfileStorage* dyn_profile_construct_00c50310(
    DynProfileStorage& profile,const DynEngineRuntimeContext& context,
    DynProfileRootPartialNodeDelete delete_partial_00bf65ac) {
    if (!delete_partial_00bf65ac)
        throw std::invalid_argument("Dyn profile Root requires a matching raw-node partial deleter");
    // Native C50328 allocation precedes state0 C50335. Never catch it here.
    auto* const allocation=static_cast<DynProfileNodeStorage*>(
        allocate(context,sizeof(DynProfileNodeStorage)));
    try {
        auto* node=allocation;
        if (node) node=dyn_profile_node_construct_00c44000(
            *node,context.root_name_00d79dc8,0,context.memory);
        at<DynProfileNodeStorage*>(&profile,0)=node;
        at<DynProfileNodeStorage*>(&profile,4)=node;
        at<std::uint32_t>(&profile,8)=0;
        *context.profile_slot_0109e9f8=&profile;
        std::memset(profile.bytes+0xc,0,0x90);
        return &profile;
    } catch (...) {
        // E041C8 ->CC8363: free captured[EBP-10] only. Root's state spans the
        // publication/tail too; unlike C50390 it has no earlier state reset.
        delete_partial_00bf65ac(allocation);
        throw;
    }
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

DynEngineStorage* dyn_engine_construct_qualified_00c55ea0(DynEngineStorage& engine,
    const volatile std::uint32_t* descriptor,const DynEngineRuntimeContext& context,
    DynProfileRootPartialNodeDelete delete_partial_00bf65ac) {
    if (!delete_partial_00bf65ac)
        throw std::invalid_argument("Dyn engine requires a matching raw-allocation partial deleter");
    at<std::uint32_t>(&engine,0)=0;
    at<std::uint32_t>(&engine,4)=0;
    at<std::uint32_t>(&engine,8)=0;
    // Native E042B4 state0 begins before the 9Ch allocator, after initial zeros.
    // These are source catch extents; /EHsc may hoist its physical guard.
    try {
        auto* const profile_allocation=static_cast<DynProfileStorage*>(
            allocate(context,sizeof(DynProfileStorage)));
        auto* profile=profile_allocation;
        try {
            if (profile) profile=dyn_profile_construct_00c50310(
                *profile,context,delete_partial_00bf65ac);
        } catch (...) {
            // CC83ED: captured 9Ch only, then the outer current-vector action.
            delete_partial_00bf65ac(profile_allocation);
            throw;
        }
        // Source profile catch has ended. Native state1 demotes BEFORE this
        // store; /EHsc may delay physical demotion to the next throwing call.
        at<DynProfileStorage*>(&engine,0xc)=profile;
        auto* const task_allocation=static_cast<DynTaskManagerStorage*>(
            allocate(context,sizeof(DynTaskManagerStorage)));
        try {
            auto* tasks=task_allocation;
            if (tasks) {
                // C55F16: one live descriptor DWORD AFTER the allocator callback
                // and within the acquired task scope. Null task skips this read.
                const std::uint32_t worker_count=*descriptor;
                tasks=dyn_task_manager_construct_qualified_00c37740(
                    *tasks,worker_count,context.memory);
            }
            at<DynTaskManagerStorage*>(&engine,0x10)=tasks;
            return &engine;
        } catch (...) {
            // CC83F8: captured 358h only; completed profile/Root are not retired.
            delete_partial_00bf65ac(task_allocation);
            throw;
        }
    } catch (...) {
        // CC83E5 ->409B10 is the SAME current-data operation as existing A900.
        release_profile_vector_storage_0040a900(&engine,context.memory);
        throw;
    }
}

DynEngineStorage* dyn_engine_ensure_qualified_00c55f50(
    const volatile std::uint32_t* descriptor,const DynEngineRuntimeContext& context,
    DynProfileRootPartialNodeDelete delete_partial_00bf65ac) {
    auto* const existing=static_cast<DynEngineStorage*>(
        *static_cast<void* volatile*>(context.engine_slot_0109e9fc));
    if (existing) return existing;
    if (!delete_partial_00bf65ac)
        throw std::invalid_argument("Dyn engine cold ensure requires a matching raw-allocation partial deleter");
    // Native state-1 at C55F74: allocator exceptions have no captured 14h action.
    auto* const allocation=static_cast<DynEngineStorage*>(
        allocate(context,sizeof(DynEngineStorage)));
    try {
        auto* engine=allocation;
        if (engine) engine=dyn_engine_construct_qualified_00c55ea0(
            *engine,descriptor,context,delete_partial_00bf65ac);
        *static_cast<void* volatile*>(context.engine_slot_0109e9fc)=engine;
        return engine;
    } catch (...) {
        // CC840D: captured 14h only. Never clear/reload/free current publication.
        delete_partial_00bf65ac(allocation);
        throw;
    }
}
} // namespace bsp
