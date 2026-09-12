#include "bsp/entity_event_queues.hpp"

// docs/ENTITY_EVENT_QUEUES.md. Read from the raw listing: the non-returning
// _free annotation truncates the pseudocode of 00926700 and 009273A0.

namespace bsp {

// ---------------------------------------------------------------------------
// The deferred hit-event queue
// ---------------------------------------------------------------------------

bool deferred_event_should_dispatch(bool subject_present, const SceneNodeFlags& subject) noexcept
{
    // 00926738 TEST ECX,ECX, then 0092673F and 00926745: both bytes must be
    // clear. +5Eh is `destroyed` and +5Fh `removed` in bsp/hit_narrowphase.hpp;
    // docs/UNIT_DAMAGE_AND_DEATH.md calls the same bytes release-requested and
    // killed. They are the two 009263C0 and 00926D90 set.
    return subject_present && !subject.destroyed && !subject.removed;
}

void drain_deferred_entity_events_00926700(DeferredEntityEventHost& host)
{
    // 00926700: an empty queue still clears the byte at 009267BB.
    while (host.deferred_queue_size() != 0) {
        // 00926714, list.back().
        const DeferredEventBack front = host.deferred_queue_back();

        if (deferred_event_should_dispatch(front.subject != nullptr, front.subject_flags)) {
            // 00926750, 009239A0(ECX = subject, node+8h, node+5Ch).
            host.dispatch_hit_009239a0(front.subject, front.record, front.direction);
        }

        // 0092675B, list.back() a second time. The native re-reads the sentinel
        // rather than reusing the node above, so an event a handler queued
        // during the dispatch is what gets popped and destroyed here, and the
        // dispatched node survives to be dispatched again. Asking the host
        // again reproduces that; do not hoist the first result into the pop.
        const DeferredEventBack popped = host.deferred_queue_back();
        host.deferred_queue_pop_back_and_destroy(popped);
    }

    // 009267BB.
    host.set_deferred_pending_flag_00e18684(false);
}

// ---------------------------------------------------------------------------
// The two pending entity lists
// ---------------------------------------------------------------------------

int mission_entity_kill_cause_00926d90(int cause) noexcept
{
    // 00926DAD CMP EBX,0x7 / 00926DB5 MOV ESI,2 / 00926DBA JZ: the move to ESI
    // is skipped when the cause is 7, so 7 alone keeps the literal 2.
    return cause == kMissionEntityKillCauseRemap ? kMissionEntityKillCauseRemapped : cause;
}

void remove_killed_entity_009263c0(PendingEntityQueueHost& host, void* entity)
{
    // 009274A1..009274C3 (009263C3..009263E5): the render handle is fetched,
    // and when it is non-null a second call is compared with the controlled
    // unit's. Both calls are in the listing; both are made here.
    if (host.render_handle_18h(entity) != nullptr) {
        if (host.render_handle_18h(entity) == host.controlled_unit_handle_00e188dc()) {
            host.clear_controlled_unit_handle_004bca80();
        }
    }

    // 009274C8 (009263EA): an entity already torn down skips the rest.
    SceneNodeFlags flags = host.scene_node_flags(entity);
    if (flags.destroyed) { return; }

    // 009274CE..009274DA (009263F0..009263FD). scene_node_remove_009263c0 in
    // bsp/hit_narrowphase.hpp is the same four stores behind the same guard.
    scene_node_remove_009263c0(flags);
    host.store_scene_node_flags(entity, flags);

    // 009274DE..00927510 (00926401, 00925C40): the observer count is sampled
    // under the 00694280 lock and the notification runs outside it.
    if (host.sample_has_observers_00925c40(entity)) {
        host.notify_observers_00696330(entity);
    }

    // 0092751F (00926406, a tail-jump there).
    host.on_entity_killed_80h(entity);
}

void flush_pending_entity_queues_009273a0(PendingEntityQueueHost& host)
{
    // 009273C0: the outer loop runs until both globals are empty, so entities
    // a handler queues are flushed by this same call.
    while (host.pending_destroy_count() != 0 || host.pending_kill_count() != 0) {
        // 009273DF and 009273F1: both copies are taken before either clear.
        host.copy_pending_lists_00926fa0();

        // 009273F6 and 00927435.
        host.clear_pending_lists();

        // 00927471..0092748E, in arrival order.
        const std::size_t destroyed = host.destroy_copy_count();
        for (std::size_t i = 0; i < destroyed; ++i) {
            host.on_entity_destroyed_74h(host.destroy_copy_at(i));
        }

        // 00927496..00927537, in arrival order.
        const std::size_t killed = host.kill_copy_count();
        for (std::size_t i = 0; i < killed; ++i) {
            remove_killed_entity_009263c0(host, host.kill_copy_at(i));
        }

        // 0092753C..009275B3 destroy both copies; 009275BA jumps back.
    }
}

// ---------------------------------------------------------------------------
// World expiry
// ---------------------------------------------------------------------------

EntityExpiryStep entity_expiry_step_00903620(int counter) noexcept
{
    EntityExpiryStep step;

    // 00903623 TEST EAX,EAX / 00903625 JLE: zero or negative is untouched, so
    // an unmarked entity is never aged and never becomes the resume anchor.
    if (counter <= 0) {
        step.action = EntityExpiryAction::kSkip;
        step.counter = counter;
        return step;
    }

    // 00903627 ADD EAX,0x1 / 0090362D MOV [ESI+0x6C],EAX: the increment is
    // stored back whether or not the entity is released.
    step.counter = counter + 1;

    // 0090362A CMP EAX,0x3 / 00903630 JL.
    step.action = step.counter < kEntityExpiryReleaseValue ? EntityExpiryAction::kAge
                                                           : EntityExpiryAction::kRelease;
    return step;
}

void destroy_child_subtree_009035e0(WorldExpiryHost& host, void* node)
{
    // 009035E3..009035FC: recurse on the first child while any child is left.
    // The count falls because the destructor unlinks the child; that is a
    // contract this packet did not read.
    while (host.child_count(node) != 0) {
        destroy_child_subtree_009035e0(host, host.first_child(node));
    }

    // 00903606, vtable[0](1).
    host.destroy_entity_vtable0(node);
}

void release_expired_world_objects_00903610(WorldExpiryHost& host)
{
    // 00903613..00903617.
    void* entity = host.world_chain_head();

    // EDI at 0090361A: the last entity that survived a pass with a live
    // counter. Only the kAge branch sets it (00903660), so after a release the
    // walk resumes ahead of that entity and re-examines any unmarked ones in
    // between; they are no-ops.
    void* anchor = nullptr;

    while (entity != nullptr) {
        const EntityExpiryStep step = entity_expiry_step_00903620(host.expiry_counter(entity));

        if (step.action == EntityExpiryAction::kSkip) {
            // 00903662, without touching the anchor.
            entity = host.next_in_world_chain(entity);
            continue;
        }

        host.store_expiry_counter(entity, step.counter);

        if (step.action == EntityExpiryAction::kAge) {
            // 00903660 then 00903662.
            anchor = entity;
            entity = host.next_in_world_chain(entity);
            continue;
        }

        // 00903632..00903644: every child subtree first.
        while (host.child_count(entity) != 0) {
            destroy_child_subtree_009035e0(host, host.first_child(entity));
        }

        // 0090364E, vtable[0](1). The entity leaves the chain here, which is
        // why the resume point below cannot be entity->next.
        host.destroy_entity_vtable0(entity);

        // 00903650..0090365E.
        entity = anchor != nullptr ? host.next_in_world_chain(anchor) : host.world_chain_head();
    }
}

}  // namespace bsp
