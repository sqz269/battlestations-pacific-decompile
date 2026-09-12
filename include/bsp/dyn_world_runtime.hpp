#pragma once

#include "bsp/dyn_world_settings.hpp"
#include "bsp/dyn_scene_runtime.hpp"

namespace bsp {
struct alignas(4) DynWorldStorage { unsigned char bytes[0x48c]; };

// All supplied vtables are complete callable tables owned by the runtime.
// scene carries the actual borrowed engine-global slot and the same allocator
// used for the world-owned arrays. No task method or dispatch object is invented.
struct DynWorldRuntimeContext {
    const DynSceneRuntimeContext* scene;
    const void* task_vtable_00d7a080;
    const void* lcp_solver_task_vtable_00d7a088;
    const void* lcp_solver2_task_vtable_00d7a090;
};

// Complete normal-path00C41AD0 storage construction, original stack world*,
// descriptor*, EAX world*, RET8 at00C420D9 (__stdcall, not the earlier __cdecl
// hypothesis). Fresh disjoint world/descriptor storage and successful allocations
// are required. Keeps native x87 spills, exact untouched bytes, scene/task order,
// and the late shared-motion allocation. The initialized engine/task manager and
// shared dispatch objects are borrowed; their construction, running workers,
// stepping, destruction, SEH and OOM ABI are separate dependencies.
DynWorldStorage* dyn_world_storage_construct_00c41ad0(DynWorldStorage&,
    const DynWorldDescriptor&, const DynWorldRuntimeContext&);
} // namespace bsp
