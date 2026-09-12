#pragma once

#include "bsp/native_effect_jobs.hpp"
#include "bsp/point_effect_advance.hpp"
#include "bsp/effect_deletion_queue.hpp"
#include "bsp/native_render_context.hpp"

namespace bsp {

// Complete0081B010..0081B092, ECX native0Ch owning raw-pointer array, stack
// address of iterator slot, RET4. Does not change iterator. Publish/retain tail
// then release replaced owner; reload current header, release/clear current
// captured tail, decrement live count last. Valid nonempty spans required.
void erase_live_effect_reference_unordered_0081b010(NativeRenderPointerArrayStorage&,
    void** const* position, NativeRenderActualOwners&);

struct LiveEffectUpdateBindings {
    PointEffectAdvanceRuntime& advance;
    PointEffectChildRows& restart_rows;
    NativeRenderActualOwners& point_owners;
    EffectDeletionDispatch& deletion;
    volatile float& actual_delta_00f87608;
    void* volatile& actual_reference_00f8760c;
    NativeEffectJobOwnerStorage* volatile& actual_job_00f87658;
    NativeFrameJobOwnerStorage* volatile& actual_frame_0109cf08;
    SingletonLifetimeDomain& domain;
    NativeFrameJobLifetimeBindings& frame_lifetime;
};

// Complete00867EE0..00867FC6, ECX actual28h manager; stack float delta and
// original reference-node pointer; RET8. Directly executes all four recovered
// phases using the SAME actual application rows/events/pools/lifetime domains.
// First pass captures extent and skips null cells. Second pass reloads live
// end after child callbacks/erase and has no null-cell guard. Original captured
// spans/iterators and owners must survive their native last access. No implicit
// lock, retain, replacement registry or invalid-state recovery is introduced.
void update_live_effect_manager_00867ee0(NativeLiveEffectManagerStorage&, float delta,
    void* actual_reference_node, LiveEffectUpdateBindings&);

} // namespace bsp
