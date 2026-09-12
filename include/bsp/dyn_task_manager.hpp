#pragma once
#include "bsp/avoid_zone_dyn_hull.hpp"
#include <cstdint>

namespace bsp {
struct alignas(4) DynTaskManagerStorage { unsigned char bytes[0x358]; };

// Complete normal-path native construction: two stack arguments, RET8 at
//00C377EB. Real Win32 CS/semaphore/events and CRT worker threads; no vtable or
//task callback is synthesized. Count is captured on entry, before native stores.
// Fresh disjoint storage and successful allocations/Win32 calls are required.
// Count0 leaves +0 (handle-array pointer) and +8 untouched; callers that will
//destroy that manager must supply +0=null. docs/DYN_TASK_MANAGER.md.
DynTaskManagerStorage* dyn_task_manager_construct_00c37740(DynTaskManagerStorage&,
    const std::uint32_t& worker_count, const AvoidZoneDynHullMemory&);

// Native ESI manager, stack count, RET4. Stops/joins existing workers, then
//creates the requested count. Does not drain pending work, close old thread
//handles, reset completion events, or change queue/counter storage.
void dyn_task_manager_set_worker_count_00c37690(DynTaskManagerStorage&,
    const std::uint32_t& worker_count, const AvoidZoneDynHullMemory&);

// Native ESI manager, EAX task-pointer vector, stack count, RET4. A task is a
//borrowed native object: [0] is a callable vtable whose first slot takes ECX
//task and no stack arguments, [4] is the writable completion-group index.
// Task methods remain external. They and their objects must outlive this call
//and must return normally. Positive count, running workers, available group
//(<100 simultaneous batches), representable queue size, and semaphore capacity
//are native preconditions. Zero count has no special success path.
void dyn_task_manager_run_batch_00c33140(DynTaskManagerStorage&, void* const* tasks,
    std::int32_t count, const AvoidZoneDynHullMemory&);

// Native stack manager, RET4 at00C41067. Stops workers, closes shutdown and
//semaphore handles, frees handle/queue arrays, and deletes the CS. The original
//does NOT close worker handles or its100 completion-event handles. This exact
//sequence preserves that ownership gap; fixture-only extra closure is separate.
// Do not call while batch submitters are active; storage is invalid afterwards.
void dyn_task_manager_destroy_00c40ff0(DynTaskManagerStorage&,
    const AvoidZoneDynHullMemory&);
} // namespace bsp
