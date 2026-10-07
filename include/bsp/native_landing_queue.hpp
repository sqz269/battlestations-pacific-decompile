#pragma once

#include "bsp/native_session_message84.hpp"
#include "bsp/native_unit_health_message.hpp"
#include "bsp/observer_event_producer.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual five-word records and58h slots, borrowed without construction,
// ownership or semantic translations. Fields named retained are not consumed.
struct NativeLandingQueueRecord {
    void* first_00;
    void* holder_04;
    std::uint32_t sequence_08;
    std::uint32_t zero_bits_0c;
    std::uint32_t one_bits_10;
};
struct NativeLandingQueueVectorStorage {
    NativeObserverOwnerStorage observer_00;
    std::uint32_t retained_10;
    NativeLandingQueueRecord* records_14;
    std::uint32_t count_18;
    std::uint32_t capacity_1c;
};
struct NativeLandingQueueSlot {
    std::uint32_t retained_00;
    const void* descriptor_04;
    std::uint32_t count_08;
    std::uint32_t retained_0c;
    std::uint32_t raw_10;
    std::uint8_t retained_14[0x14];
    void* first_28;
    std::uint32_t state_2c;
    std::uint32_t timer_bits_30;
    std::uint8_t pending_34;
    std::uint8_t retained_35[0x23];
};
// Actual block prefix only, not an AirOps constructor or a fabricated block.
struct NativeLandingQueueBlockStorage {
    std::uint32_t retained_00;
    TrackedCriticalSection* queue_section_04;
    std::uint32_t retained_08;
    TrackedCriticalSection* slot_section_0c;
    std::uint8_t retained_10[0x3c];
    NativeLandingQueueSlot* slots_4c;
    std::uint32_t slot_count_50;
    std::uint8_t retained_54[0x28];
    void* owner_7c;
    void* holder_80;
    NativeLandingQueueVectorStorage queue_84;
};
static_assert(sizeof(NativeLandingQueueRecord) == 0x14);
static_assert(offsetof(NativeLandingQueueRecord, holder_04) == 4);
static_assert(offsetof(NativeLandingQueueRecord, sequence_08) == 8);
static_assert(offsetof(NativeLandingQueueRecord, zero_bits_0c) == 0xc);
static_assert(offsetof(NativeLandingQueueRecord, one_bits_10) == 0x10);
static_assert(sizeof(NativeLandingQueueVectorStorage) == 0x20);
static_assert(offsetof(NativeLandingQueueVectorStorage, records_14) == 0x14);
static_assert(offsetof(NativeLandingQueueVectorStorage, count_18) == 0x18);
static_assert(offsetof(NativeLandingQueueVectorStorage, capacity_1c) == 0x1c);
static_assert(sizeof(NativeLandingQueueSlot) == 0x58);
static_assert(offsetof(NativeLandingQueueSlot, descriptor_04) == 4);
static_assert(offsetof(NativeLandingQueueSlot, count_08) == 8);
static_assert(offsetof(NativeLandingQueueSlot, raw_10) == 0x10);
static_assert(offsetof(NativeLandingQueueSlot, first_28) == 0x28);
static_assert(offsetof(NativeLandingQueueSlot, state_2c) == 0x2c);
static_assert(offsetof(NativeLandingQueueSlot, timer_bits_30) == 0x30);
static_assert(offsetof(NativeLandingQueueSlot, pending_34) == 0x34);
static_assert(offsetof(NativeLandingQueueBlockStorage, queue_section_04) == 4);
static_assert(offsetof(NativeLandingQueueBlockStorage, slot_section_0c) == 0xc);
static_assert(offsetof(NativeLandingQueueBlockStorage, slots_4c) == 0x4c);
static_assert(offsetof(NativeLandingQueueBlockStorage, slot_count_50) == 0x50);
static_assert(offsetof(NativeLandingQueueBlockStorage, owner_7c) == 0x7c);
static_assert(offsetof(NativeLandingQueueBlockStorage, holder_80) == 0x80);
static_assert(offsetof(NativeLandingQueueBlockStorage, queue_84) == 0x84);
static_assert(sizeof(NativeLandingQueueBlockStorage) == 0xa4);

struct NativeLandingQueueContext {
    NativeObserverLifetime& registration;
    ObserverEventWordDeliveryContext& event_words;
    const NativeSessionMessageContext& message_base;
    const NativeLandingSlotMessage84Profile& message84_profile;
    NativeMessage84ClassRegistryAccess& class_registry;
    // PURE aliases to actual binary32 cells, read as raw words at native sites.
    // No numeric conversion, forced1/5 constant or default tuning snapshot.
    const volatile std::uint32_t& one_bits_00d7a24c;
    const volatile std::uint32_t& five_bits_00ce3850;
};

// Connected conditional SOURCE closure: complete queue calls complete append
// and route here; route directly reuses the inherited complete00770B50 selector,
//00783DC0 lock/send and00783C80 serializer. No copied health implementation.
// Inherited globals, D2 profile and all pure actual field bindings remain real
// borrowed Source references. Its current-game CELL is the SAME message_base
// cell. Current transport+20 remains a REQUIRED complete actual operation.
// Other inherited health methods keep their separate caller domains.
//
// Ordinary native-valid success only: stable actual block/endpoints/slots,
// profiles/contexts, registry/banks, captured locks, list nodes and buffers;
// valid selected wrapping addresses; terminating traversal and disjoint live
// input record across registration/reallocation. Structural reentry invalidating
// those objects, concurrency, allocation failure, invalid placement, private
// EH/fault/ABI/network/game behavior remain unbound. Field publication changes
// may be observed while all reached backing stays live; no retention is added.
// Canonical BF55BE->BF681B and BF6989->BF65AC use the existing complete Source
// allocator/free, with native modulo32 byte arithmetic (no saturation policy).
class NativeLandingQueueCalls : public NativeUnitHealthMessageSerializedCalls {
public:
    NativeLandingQueueCalls(NativeUnitHealthSetterGlobals, NativeUnitHealthRouteGlobals,
        const NativeUnitHealthMessageProfile&, const volatile std::uint16_t& actual_tick,
        const NativeLandingQueueContext&) noexcept;

    // Native006BF720..006BF7E5: ECX actual embedded block+84, stack record,
    // RET4. Register identical-address record.first / queue.observer BEFORE
    // count/capacity; publish2*n+2 cap BEFORE alloc; five-word copies; count last.
    void append_record_006bf720(NativeLandingQueueVectorStorage&, const NativeLandingQueueRecord&);

    // Native006C0B50..006C0D1D: ECX block, stack incoming, EAX holder, RET4.
    // Cached task404 is not an input: lower caller must pass its FRESH original
    // plane9D4. First matching nonnull record4 returns captured identity; null4
    // stops scan and duplicates. Preappend holder80 differs from fresh returned80.
    // Append requires a nonnull live incoming observer-prefix identity. A null
    // token may compare on an early nonnull-record return, not become an endpoint.
    void* queue_incoming_006c0b50(NativeLandingQueueBlockStorage&, void* actual_incoming);

    // Native0077C7B0..0077C87F: ECX actual owner, stack(message,exclude), RET8.
    // Mode1 gate has NO null-game fallback. Implement all literal control flow;
    // this caller admits the actual hex84 profile/payload. Other profiles need
    // their own complete codec/provider domain;49/46 acceptance is not admission.
    // Sender/exclusion are raw identities. Send consumes/serializes the borrowed
    // frame synchronously or owns a representation; never retain/release frame.
    void route_message_0077c7b0(void* actual_owner, NativeSessionMessageStorage*, const void* exclude);
private:
    const NativeLandingQueueContext& queue_context_;
};

} // namespace bsp
