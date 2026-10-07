#include "bsp/native_landing_queue.hpp"
#include "bsp/observer_edges.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>

namespace bsp {
namespace {
using U = std::uint32_t;
using I = std::int32_t;
I signed_bits(U bits) noexcept { I result; std::memcpy(&result, &bits, sizeof(result)); return result; }
U address(const void* p) noexcept { return reinterpret_cast<U>(p); }
template<class T> T* at(U p) noexcept { return reinterpret_cast<T*>(p); }
void enter(TrackedCriticalSection* section) {
    if (section == nullptr) return;
    ::EnterCriticalSection(&section->native);
    auto& depth = *reinterpret_cast<volatile U*>(&section->depth);
    depth = depth + 1u;
}
void leave(TrackedCriticalSection* captured) {
    if (captured == nullptr) return;
    auto& depth = *reinterpret_cast<volatile U*>(&captured->depth);
    depth = depth - 1u;
    ::LeaveCriticalSection(&captured->native);
}
void copy_record(U destination, U source) noexcept {
    auto& d = *at<volatile NativeLandingQueueRecord>(destination);
    const auto& s = *at<const volatile NativeLandingQueueRecord>(source);
    d.first_00 = s.first_00;
    d.holder_04 = s.holder_04;
    d.sequence_08 = s.sequence_08;
    d.zero_bits_0c = s.zero_bits_0c;
    d.one_bits_10 = s.one_bits_10;
}
} // namespace

NativeLandingQueueCalls::NativeLandingQueueCalls(NativeUnitHealthSetterGlobals globals,
    NativeUnitHealthRouteGlobals route, const NativeUnitHealthMessageProfile& profile,
    const volatile std::uint16_t& tick, const NativeLandingQueueContext& context) noexcept
    : NativeUnitHealthMessageSerializedCalls(globals, route, profile, tick), queue_context_(context) {}

void NativeLandingQueueCalls::append_record_006bf720(NativeLandingQueueVectorStorage& queue,
                                                   const NativeLandingQueueRecord& input) {
    register_observer_pair_00694a60(*static_cast<NativeObserverOwnerStorage*>(input.first_00),
                                   queue.observer_00, queue_context_.registration);
    volatile auto& q = queue;
    const U capacity = q.capacity_1c;
    if (q.count_18 == capacity) {
        const U grown = capacity * 2u + 2u;
        q.capacity_1c = grown;
        const U bytes = grown * 20u; // native modulo32, no saturation.
        auto* replacement = static_cast<NativeLandingQueueRecord*>(singleton_lifetime_allocate(
            {SingletonAllocationKind::object, bytes, bytes}));
        U index = 0;
        U offset = 0;
        U destination = address(replacement);
        if (q.count_18 != 0) {
            do {
                if (destination != 0) copy_record(destination, address(q.records_14) + offset);
                index = index + 1u;
                offset = offset + 20u;
                destination = destination + 20u;
            } while (index < q.count_18);
        }
        auto* const old = q.records_14;
        if (old != nullptr) singleton_lifetime_free(old);
        q.records_14 = replacement;
    }
    const U count = q.count_18;
    const U destination = address(q.records_14) + count * 20u;
    if (destination != 0) copy_record(destination, address(&input));
    q.count_18 = q.count_18 + 1u;
}

void* NativeLandingQueueCalls::queue_incoming_006c0b50(NativeLandingQueueBlockStorage& block,
                                                      void* incoming) {
    volatile auto& b = block;
    auto& q = b.queue_84;
    auto* const captured_queue_section = b.queue_section_04;
    U next_sequence = 0;
    void* retained = nullptr;
    enter(captured_queue_section);
    const U begin = address(q.records_14);
    const U initial_end = begin + q.count_18 * 20u;
    if (begin != initial_end) {
        const U end = begin + q.count_18 * 20u;
        U cursor = begin;
        do {
            const auto& record = *at<const volatile NativeLandingQueueRecord>(cursor);
            if (record.first_00 == incoming) { retained = record.holder_04; break; }
            const U sequence = record.sequence_08;
            if (signed_bits(sequence) >= signed_bits(next_sequence)) next_sequence = sequence + 1u;
            cursor = cursor + 20u;
        } while (cursor != end);
    }
    if (retained == nullptr) {
        NativeLandingQueueRecord record;
        volatile auto& r = record;
        r.holder_04 = b.holder_80;
        r.zero_bits_0c = 0;
        const U one = queue_context_.one_bits_00d7a24c;
        r.first_00 = incoming;
        r.sequence_08 = next_sequence;
        r.one_bits_10 = one;
        append_record_006bf720(block.queue_84, record);
    }
    leave(captured_queue_section);
    if (retained != nullptr) return retained;

    auto* const captured_slot_section = b.slot_section_0c;
    U slot_index = 0;
    enter(captured_slot_section);
    const U initial_count = b.slot_count_50;
    U cursor = address(b.slots_4c);
    const U initial_slot_end = cursor + initial_count * 0x58u;
    if (cursor != initial_slot_end) {
        U end;
        do {
            auto& slot = *at<volatile NativeLandingQueueSlot>(cursor);
            if (slot.first_28 == incoming) {
                const bool pending = slot.pending_34 != 0;
                slot.state_2c = 4;
                slot.timer_bits_30 = 0;
                if (pending) {
                    slot.timer_bits_30 = queue_context_.five_bits_00ce3850;
                    slot.pending_34 = 0;
                }
                auto* owner = b.owner_7c;
                notify_observer_slot0c_00696350(*static_cast<NativeObserverOwnerStorage*>(owner),
                                               4, 0, queue_context_.event_words);
                NativeLandingSlotMessage84 message; // native ordinary borrowed38h frame.
                construct_native_session_message84_006bd520(&message, &block, slot_index,
                    queue_context_.message_base, queue_context_.message84_profile, queue_context_.class_registry);
                owner = b.owner_7c;
                route_message_0077c7b0(owner, &message.base, nullptr);
                // Native producer has no ordinary scalar/destructor call here.
            }
            const U count = b.slot_count_50;
            end = address(b.slots_4c) + count * 0x58u;
            cursor = cursor + 0x58u; // old cursor, not recomputed from rebound base.
            slot_index = slot_index + 1u;
        } while (cursor != end);
    }
    leave(captured_slot_section);
    return b.holder_80;
}

void NativeLandingQueueCalls::route_message_0077c7b0(void* owner,
    NativeSessionMessageStorage* message, const void* exclude) {
    void* game = queue_context_.message_base.current_game_00e188a8;
    if (*at<const volatile U>(address(game) + 0x1fe4u) != 1) return;
    volatile auto& m = *message;
    if (exclude == nullptr) {
        const auto type = m.type_10;
        bool select_owner = type == 0x82 || type == 0x81 || type == 0xa7 || type == 0xa8 || type == 0xa9;
        if (!select_owner) {
            const volatile U* const profile = m.profile_00;
            using Query = bool(__thiscall*)(const NativeSessionMessageStorage*, U);
            select_owner = reinterpret_cast<Query>(profile[3])(message, 0x4a);
        }
        if (select_owner) exclude = m.selected_owner_14;
    }
    const auto fields = bind_route_fields(owner); // pure aliases, no value observations.
    auto* const initial_sentinel = fields.sentinel_2a8;
    auto* node = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*initial_sentinel).next_00;
    for (;;) {
        auto* const sentinel = fields.sentinel_2a8;
        if (node == sentinel) break;
        if (node == fields.sentinel_2a8) invalid_route_iterator_00bf6713();
        const auto* const compared_peer = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*node).peer_08;
        if (exclude != compared_peer) {
            if (node == fields.sentinel_2a8) invalid_route_iterator_00bf6713();
            const auto sender = fields.sender_174;
            auto* const peer = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*node).peer_08;
            *at<volatile std::uint16_t>(address(message) + 0x18u) = sender;
            void* const target = peer_target_50(peer);
            game = queue_context_.message_base.current_game_00e188a8;
            send_session_message_to_nonlocal_peer_00770b50(at<void>(address(game) + 0x1ef0u), target, message);
        }
        if (node == fields.sentinel_2a8) invalid_route_iterator_00bf6713();
        node = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*node).next_00;
    }
}

} // namespace bsp
