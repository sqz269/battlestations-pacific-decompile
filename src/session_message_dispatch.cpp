// Routing and local delivery of a session message. See
// docs/SESSION_MESSAGE_DISPATCH.md for the evidence behind every rule.
// Packet cc2_session_dispatch, read-only Ghidra analysis, 2026-09-11 UTC.

#include "bsp/session_message_dispatch.hpp"

namespace bsp {

int default_route_flags_for_mode(SessionMode mode) noexcept {
    // 0076FAD0 is the only writer of [00E0AF1C]. Mode 1 writes 1 at 0076FBAF,
    // mode 2 writes 2 at 0076FD2C, and the teardown to mode 0 writes 1 at
    // 0076FE26. There is no fourth branch: 0076FD71 leaves the word alone for
    // any other value and only stores the mode at 0076FE85.
    switch (mode) {
        case SessionMode::kClient:
            return kRouteFlagToHost;
        case SessionMode::kHost:
        case SessionMode::kLocal:
        default:
            return kRouteFlagLocal;
    }
}

SessionRouteDecision route_session_message_0077c2a0(const SessionRouteInputs& in) noexcept {
    SessionRouteDecision out;

    // 0077C2A8 and 0077C2B7: no session object, or a game state below 0Ah, and
    // the routine returns without touching the message.
    if (!in.session_object_present || in.game_state < kRouteMinimumGameState) {
        return out;
    }

    // 0077C302..0077C30A CMOVNZ: a non-zero override replaces the default.
    int flags = in.flags_override != 0 ? in.flags_override : in.default_flags;

    // 0077C30D..0077C323: exactly 4 outside a host session returns with nothing
    // sent. The test is on the whole word, not on the bit, so 5, 6 and 7 pass.
    if (flags == kRouteFlagToPeers && in.mode != SessionMode::kHost) {
        return out;
    }

    if (in.mode == SessionMode::kLocal) {
        // 0077C32F..0077C335: in a local session the flag word is replaced by 1,
        // whatever the call site asked for.
        flags = kRouteFlagLocal;
    } else if ((flags & kRouteFlagToHost) != 0 && (flags & kRouteFlagLocal) == 0) {
        // 0077C337..0077C35C. A privileged message is applied locally as well as
        // sent; on a host every message of this shape is applied locally too.
        if (in.privileged_kind || in.mode == SessionMode::kHost) {
            flags |= kRouteFlagLocal;
        }
    } else if ((flags & kRouteFlagToPeers) != 0 && in.mode == SessionMode::kClient) {
        // 0077C35E..0077C37C. A client cannot reach the peers, so the peer bit
        // becomes the host bit, plus the local bit for a privileged message.
        // NEG/SBB/NEG/ADD 2 yields 3 when the predicate answered true, else 2.
        out.mark_relay = true;  // 0077C36B msg+1Ah = 1
        flags = in.privileged_kind ? (kRouteFlagToHost | kRouteFlagLocal) : kRouteFlagToHost;
    }

    out.routed = true;
    out.flags = flags;
    // 0077C3A2 and 0077C3BE re-read the mode from the world; the three
    // destinations are independent and run in this order.
    out.send_to_host = in.mode == SessionMode::kClient && (flags & kRouteFlagToHost) != 0;
    out.send_to_peers = in.mode == SessionMode::kHost && (flags & kRouteFlagToPeers) != 0;
    out.deliver_local = (flags & kRouteFlagLocal) != 0;
    return out;
}

QueuedMessageDecision gate_queued_message_00780670(const QueuedMessageInputs& in) noexcept {
    QueuedMessageDecision out;

    // 00780675: any state but 0Dh keeps the message queued and dispatches
    // nothing. 0076C600 turns that into a drop below state 0Ah.
    if (in.game_state != kDispatchGameState) {
        out.route = QueuedMessageRoute::kNotDispatching;
        out.status = QueuedMessageStatus::kKeep;
        return out;
    }

    // 0078069F: an entity-create message is moved to the list at 00F871A0 and
    // released without being destroyed; 0077EC20 owns it from there.
    if (in.is_entity_create) {
        out.route = QueuedMessageRoute::kDeferredCreate;
        out.status = QueuedMessageStatus::kRelease;
        return out;
    }

    // 007806EE: a sub-object message needs its entity now. 0078072B holds it for
    // 600 ticks while the entity is missing, then drops it.
    if (in.is_sub_object) {
        if (in.sender_resolved) {
            out.route = QueuedMessageRoute::kSubObject;
            out.status = QueuedMessageStatus::kConsume;
        } else if (in.tick_delta <= kDispatchUnresolvedSenderAge) {
            out.route = QueuedMessageRoute::kDrop;
            out.status = QueuedMessageStatus::kConsume;
        } else {
            out.route = QueuedMessageRoute::kRetryLater;
            out.status = QueuedMessageStatus::kKeep;
        }
        return out;
    }

    // 00780751..0078076B: a message from the future waits, unless it is further
    // ahead than the window, in which case it is consumed without a receiver.
    const int window = in.is_long_window ? kDispatchTickWindowLong : kDispatchTickWindow;
    if (in.tick_delta > 0) {
        if (in.tick_delta > window) {
            out.route = QueuedMessageRoute::kDrop;
            out.status = QueuedMessageStatus::kConsume;
        } else {
            out.route = QueuedMessageRoute::kRetryLater;
            out.status = QueuedMessageStatus::kKeep;
        }
        return out;
    }

    // 0078077A: the message is due. 007807C4 holds an unresolved entity for ten
    // ticks and then drops the message.
    if (!in.sender_resolved) {
        if (in.tick_delta > kDispatchMissingEntityAge) {
            out.route = QueuedMessageRoute::kRetryLater;
            out.status = QueuedMessageStatus::kKeep;
        } else {
            out.route = QueuedMessageRoute::kDrop;
            out.status = QueuedMessageStatus::kConsume;
        }
        return out;
    }

    // 00780789: an entity that has not finished initialising keeps the message.
    if (!in.entity_ready) {
        out.route = QueuedMessageRoute::kRetryLater;
        out.status = QueuedMessageStatus::kKeep;
        return out;
    }

    // 00780797: the sequence-filtered path, or the generic entity handler.
    out.route = in.is_entity_sync ? QueuedMessageRoute::kEntitySync
                                  : QueuedMessageRoute::kEntityGeneric;
    out.status = QueuedMessageStatus::kConsume;
    return out;
}

bool MessageIsATest::answers(int id) const noexcept {
    for (int i = 0; i < count; ++i) {
        if (static_cast<int>(ids[i]) == id) {
            return true;
        }
    }
    return false;
}

EntityMessageReceiver receiver_for_message_00780120(const MessageIsATest& is_a) noexcept {
    // 00780162 .. 00780636, in the order the native ladder tests them.
    if (is_a.answers(0x4B)) return EntityMessageReceiver::kPlayerSlot;
    if (is_a.answers(0x4C)) return EntityMessageReceiver::kPlayerState;
    if (is_a.answers(0x57)) return EntityMessageReceiver::kSpawnPoint;
    if (is_a.answers(0x4E)) return EntityMessageReceiver::kDestroy;
    if (is_a.answers(0x4F)) return EntityMessageReceiver::kTeamScore;
    if (is_a.answers(0x50)) return EntityMessageReceiver::kRespawnTimer;
    if (is_a.answers(0x51)) return EntityMessageReceiver::kMissionEvent;
    if (is_a.answers(0x52)) return EntityMessageReceiver::kVoice;
    if (is_a.answers(0x97)) return EntityMessageReceiver::kCamera;
    if (is_a.answers(0x58)) return EntityMessageReceiver::kUnitCommand;
    if (is_a.answers(0x59)) return EntityMessageReceiver::kWeaponDirector;
    // 007803FD, the shared tail: 0077FE80 takes everything else.
    return EntityMessageReceiver::kSessionKindSwitch;
}

SessionRouteDecision run_route_0077c2a0(SessionMessageDispatchHost& host,
                                        const SessionRouteInputs& in,
                                        void* session,
                                        void* entity,
                                        void* message,
                                        void** out) noexcept {
    SessionRouteInputs inputs = in;

    if (!in.session_object_present || in.game_state < kRouteMinimumGameState) {
        return SessionRouteDecision{};
    }

    // 0077C2C4..0077C2FB. The discarded is-a call runs before the flags are
    // resolved and only for a class-5 entity with an assigned slot.
    if (host.entity_is_kind(entity, 0x05) && host.entity_player_slot(entity) >= 0 &&
        host.entity_is_kind(entity, 0x1C)) {
        host.message_is_a_discarded(message, 0xD3);
    }

    // 0077C344 and 0077C36F ask the predicate only on the two branches that use
    // it, so the host is not called when the flags do not reach them.
    const bool needs_predicate =
        in.mode != SessionMode::kLocal &&
        (((in.flags_override != 0 ? in.flags_override : in.default_flags) &
          (kRouteFlagToHost | kRouteFlagToPeers)) != 0);
    inputs.privileged_kind = needs_predicate && host.message_is_privileged(message);

    const SessionRouteDecision decision = route_session_message_0077c2a0(inputs);
    if (!decision.routed) {
        return decision;
    }

    if (decision.mark_relay) {
        host.mark_message_for_relay(message);
    }

    // 0077C37F: the byte at 00E18DB7 gates the audit flag entirely.
    if (in.audit_latch && host.message_is_long_window(message)) {
        host.set_message_audit_flag(message);
    }

    if (decision.send_to_host) {
        host.send_message_to_host(entity, message);
    }

    if (decision.send_to_peers) {
        for (void* peer = host.first_peer(entity); peer != nullptr;
             peer = host.next_peer(entity, peer)) {
            host.send_message_to_peer(peer, message);
        }
    }

    if (decision.deliver_local) {
        host.enqueue_local(session, message, entity);
        if (out != nullptr) {
            host.clear_caller_message_slot(out);
        }
    }
    return decision;
}

void* run_enqueue_local_0076e520(SessionMessageDispatchHost& host,
                                 void* session,
                                 void* stream,
                                 void* message,
                                 void* sender) noexcept {
    // The local path does not queue the caller's object. It writes the message
    // into a 400h-byte stack stream and rebuilds a second message from those
    // bytes, so the queued copy carries only the serialised fields plus the
    // envelope the stamp writes back.
    host.serialize_message(message, stream);
    void* copy = host.create_message_from_stream(stream);
    if (copy == nullptr) {
        return nullptr;
    }
    host.stamp_loopback_message(copy, message, sender);
    host.append_to_queue(session, copy);
    return copy;
}

int run_drain_0076c600(SessionMessageDispatchHost& host,
                       void** queue,
                       int& count,
                       int game_state) noexcept {
    // 0076C604: an empty queue returns immediately. The loop walks the array in
    // order and only advances past an entry the gate asked to keep.
    int removed = 0;
    int index = 0;
    while (index < count) {
        void* message = queue[index];
        QueuedMessageStatus status = QueuedMessageStatus::kConsume;
        if (message != nullptr) {
            status = host.gate_message(message);
        }

        if (status == QueuedMessageStatus::kKeep) {
            // 0076C669: outside the in-mission states a keep becomes a logged
            // drop, but only for a message that carries a send tick.
            if (game_state < kRouteMinimumGameState && message != nullptr &&
                host.message_send_tick(message) > 0) {
                host.log_dropped_message(message, game_state);
                status = QueuedMessageStatus::kConsume;
            } else {
                ++index;
                continue;
            }
        }

        // 0076C6AC: shift the tail down by one and drop the count.
        for (int i = index; i + 1 < count; ++i) {
            queue[i] = queue[i + 1];
        }
        --count;
        ++removed;
        if (status == QueuedMessageStatus::kConsume && message != nullptr) {
            host.destroy_message(message);
        }
    }
    return removed;
}

}  // namespace bsp
