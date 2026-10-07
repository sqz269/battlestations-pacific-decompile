#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/unit_damage.hpp"

namespace bsp {
// D2h storage used by00876D30. Constructors retain11..13 and1B; the input
// is a full DWORD even though00877B90 normally supplies a replicated byte.
struct NativeUnitHealthMessage {
    NativeSessionMessageStorage base;
    std::uint16_t sender_18;
    std::uint8_t relay_1a;
    std::uint8_t retained_1b;
    std::uint32_t health_1c;
};
static_assert(sizeof(NativeUnitHealthMessage) == 0x20);
static_assert(alignof(NativeUnitHealthMessage) == 4);
static_assert(offsetof(NativeUnitHealthMessage, sender_18) == 0x18);
static_assert(offsetof(NativeUnitHealthMessage, relay_1a) == 0x1a);
static_assert(offsetof(NativeUnitHealthMessage, health_1c) == 0x1c);

// Required release in the allocation domain that owns a deleting message.
// No default allocator/free is selected. This service and the profile must
// outlive every message using them; releasing the borrowed setter frame is
// never permitted. The service sees the root profile already stamped.
struct NativeUnitHealthMessageRelease {
    virtual ~NativeUnitHealthMessageRelease() = default;
    virtual void release_message_00bf65ac(void* allocation) = 0;
};
// Five executable Win32 source bridges in native slot order, followed by
// borrowed SOURCE metadata. This extended table is not the original game ABI.
struct NativeUnitHealthMessageProfile {
    const std::uint32_t slots[5];
    NativeUnitHealthMessageRelease* const release;
    explicit NativeUnitHealthMessageProfile(NativeUnitHealthMessageRelease&) noexcept;
};
static_assert(offsetof(NativeUnitHealthMessageProfile, slots) == 0);
static_assert(offsetof(NativeUnitHealthMessageProfile, release) == 0x14);

NativeUnitHealthMessage* construct_native_unit_health_message_00876d30(
    NativeUnitHealthMessage*, std::uint32_t health,
    const NativeSessionMessageContext&, const NativeUnitHealthMessageProfile&);
// Only accepts storage carrying the complete source D2 profile above. Captures
// its release service before root stamping, tests flags bit0, returns identity.
NativeUnitHealthMessage* delete_native_unit_health_message_0075fe50(
    NativeUnitHealthMessage*, std::uint32_t flags);
void write_native_unit_health_message_0075fd10(const NativeUnitHealthMessage*, NativeBitCursor*);
void read_native_unit_health_message_0075fd60(NativeUnitHealthMessage*, NativeSessionReadStream*);
bool native_unit_health_message_type_matches_0075fce0(std::uint32_t type);

// Opt-in abstract adapter: only the constructor becomes concrete. Table/mode
// access, health notification/provider and the complete router stay required.
// bind() gives the setter the SAME mutable game-pointer CELL used by0075B430.
// The context is borrowed, never a copied game object, owner or session value.
class NativeUnitHealthMessageConstructorCalls : public NativeUnitHealthSetterCalls {
public:
    NativeUnitHealthMessageConstructorCalls(NativeUnitHealthSetterGlobals,
        const NativeUnitHealthMessageProfile&) noexcept;
    NativeUnitHealthSetterBinding bind(NativeUnitHealthSetterFields) noexcept;
    void* construct_health_message_00876d30(NativeUnitHealthMessageFrame&,
        std::int32_t health) final;
private:
    NativeUnitHealthSetterGlobals globals_;
    const NativeUnitHealthMessageProfile& profile_;
};

// Actual borrowed native list nodes; the sentinel and reached nodes stay live.
// No owner, sentinel, peer array or links are manufactured by this interface.
struct NativeUnitHealthRoutePeerNode {
    NativeUnitHealthRoutePeerNode* next_00;
    NativeUnitHealthRoutePeerNode* previous_04;
    void* peer_08;
};
static_assert(sizeof(NativeUnitHealthRoutePeerNode) == 0x0c);
static_assert(offsetof(NativeUnitHealthRoutePeerNode, peer_08) == 8);
struct NativeUnitHealthRouteFields {
    const volatile std::int32_t& player_slot_528;
    const volatile std::uint16_t& sender_174;
    NativeUnitHealthRoutePeerNode* const volatile& sentinel_2a8;
};
struct NativeUnitHealthRouteGlobals {
    const volatile std::uint32_t& default_flags_00e0af1c;
    const volatile std::uint8_t& audit_latch_00e18db7;
};

// Opt-in D2/exact-flags4 branch of parent0077C2A0, not a generic router.
// bind()->00877B90 supplies the admitted flags4 call and borrowed D2 frame.
// Other flags are outside this API. out is untouched on every admitted path.
// Game backing must expose actual signed+5D4, +1FE4 and embedded+1EF0 fields,
// as well as the owner fields already required by the constructor. The active
// message table must remain a live translated D2 profile (queries D3/49 false).
class NativeUnitHealthMessageFlags4Calls : public NativeUnitHealthMessageConstructorCalls {
public:
    NativeUnitHealthMessageFlags4Calls(NativeUnitHealthSetterGlobals,
        NativeUnitHealthRouteGlobals, const NativeUnitHealthMessageProfile&) noexcept;
    void route_health_message_0077c2a0(void* receiver, void* message,
        std::uint32_t flags, void** clear_on_local_delivery) final;
protected:
    // Pure alias binding/access to actual backing: no value snapshots,
    // callbacks, allocation, ownership changes or FP changes.
    virtual NativeUnitHealthRouteFields bind_route_fields(void* receiver) noexcept = 0;
    virtual void* const volatile& peer_target_50(void* actual_peer) noexcept = 0;
    virtual bool call_entity_is_kind_5c(std::uint32_t entry, void* receiver,
        std::uint32_t kind) = 0;
    // Complete required operation, including native transport selection,
    // local-peer exclusion and any secondary type29/00783DC0 behavior.
    // Synchronously consume/serialize or make an owned copy; never retain or
    // release the caller's borrowed frame. No default transport/clone exists.
    virtual void send_message_to_nonlocal_peer_00770b50(void* actual_session,
        void* actual_target, NativeUnitHealthMessage* borrowed_message) = 0;
    // Reached native iterator violation service. No assumed no-return,
    // exception or recovery policy; original invalid-storage faults unproved.
    virtual void invalid_route_iterator_00bf6713() = 0;
private:
    void route_d2_flags4_0077c2a0(void* receiver, NativeUnitHealthMessage*);
    void* volatile& current_game_00e188a8_;
    NativeUnitHealthRouteGlobals route_globals_;
};
struct NativeSessionPeerSendFields {
    void* const volatile& primary_transport_188;
    void* const volatile& secondary_transport_18c;
};
struct NativeSessionPrimaryPeerFields {
    NativeUnitHealthRoutePeerNode* const volatile& sentinel_0c;
    const volatile std::uint32_t& peer_count_10;
};

// Opt-in complete ordinary00770B50 selector on the admitted health route.
// The passed session stays captured; this body does not reread the game cell.
// The primary list node+8 already IS the local target (no peer+50 lookup).
class NativeUnitHealthMessagePeerSendCalls : public NativeUnitHealthMessageFlags4Calls {
public:
    using NativeUnitHealthMessageFlags4Calls::NativeUnitHealthMessageFlags4Calls;
protected:
    // Also accepts a real translated common message header/profile. This is
    // the same complete selector, not a concrete type29 payload or factory.
    void send_session_message_to_nonlocal_peer_00770b50(void* actual_session,
        void* actual_target, NativeSessionMessageStorage* borrowed_message);
    // Pure aliases to actual backing, without early value observations,
    // callbacks, allocation, ownership changes or FP changes.
    virtual NativeSessionPeerSendFields bind_peer_send_session(void*) noexcept = 0;
    virtual NativeSessionPrimaryPeerFields bind_peer_send_primary(void*) noexcept = 0;
    // Required COMPLETE operation: actual target+4 Enter, increment+18,
    //00783C80 serialization, reload target+4, decrement+18 and Leave. No
    // invented lock/RAII, stream, success, allocator or transport policy.
    // Consume synchronously or retain an owned representation; never retain
    // or release this borrowed message/frame. All reached backing stays live.
    virtual void send_locked_session_message_00783dc0(void* actual_transport,
        void* actual_target, NativeSessionMessageStorage* borrowed_message) = 0;
private:
    void send_message_to_nonlocal_peer_00770b50(void* actual_session,
        void* actual_target, NativeUnitHealthMessage* borrowed_message) final;
};
struct TrackedCriticalSection;
// Opt-in complete ordinary00783DC0 wrapper using the canonical native lock.
// Every reached section is live and initialized; each Leave must have valid
// current-thread ownership. No null fallback, RAII or exception cleanup.
class NativeUnitHealthMessageLockedSendCalls : public NativeUnitHealthMessagePeerSendCalls {
public:
    using NativeUnitHealthMessagePeerSendCalls::NativeUnitHealthMessagePeerSendCalls;
protected:
    // Pure alias to the actual target+4 cell, without early value observation,
    // callbacks, allocation, ownership changes or FP changes.
    virtual TrackedCriticalSection* const volatile& target_critical_section_04(
        void* actual_target) noexcept = 0;
    // Required COMPLETE00783C80: actual delivery-indexed cursors, tick/prefix,
    // executable profile writer, overflow copy/rewind/flush/restore and
    // threshold effects. No default D2-only serializer or transport success.
    // Same synchronous borrowed-frame/owned-representation contract as above.
    virtual void serialize_session_message_00783c80(void* actual_transport,
        void* actual_target, NativeSessionMessageStorage* borrowed_message) = 0;
private:
    void send_locked_session_message_00783dc0(void* actual_transport,
        void* actual_target, NativeSessionMessageStorage* borrowed_message) final;
};
// Complete sole-caller helpers used by00783C80. Extraction visits absolute
// base bits MSB-first and packs MSB-first; append consumes those packed bits
// MSB-first but ORs them into the cursor's LSB bit positions. No cursor-length
// check or generic bit read/write substitution. New source interfaces.
void copy_native_message_bit_slice_00428ec0(const NativeBitCursor*, void* destination,
    std::uint32_t absolute_bit, std::uint32_t bits);
void append_native_message_msb_bits_00429540(NativeBitCursor*, const void* source,
    std::uint32_t bits);

// Complete ordinary00783C80 with actual live cursor/profile/tick bindings.
// The selected cursor stays captured across provider changes to its cell.
// Backing includes the native writable carry byte. An overflow copy's delta
// fits the actual200h-byte scratch (at most1000h bits); no clamp is introduced.
class NativeUnitHealthMessageSerializedCalls : public NativeUnitHealthMessageLockedSendCalls {
public:
    NativeUnitHealthMessageSerializedCalls(NativeUnitHealthSetterGlobals,
        NativeUnitHealthRouteGlobals, const NativeUnitHealthMessageProfile&,
        const volatile std::uint16_t& actual_tick_low_00f876b0) noexcept;
protected:
    // Pure aliases/access to actual backing, without early value snapshots,
    // callbacks, allocation, ownership changes or FP changes.
    virtual NativeBitCursor* const volatile& delivery_cursor_d40(void* actual_target,
        std::uint32_t actual_delivery) noexcept = 0;
    virtual const volatile std::uint32_t* transport_primary_table(void*) noexcept = 0;
    // Required COMPLETE current transport virtual+20 operation. Consume the
    // cursor bytes synchronously or create owned data; keep reached transport,
    // target, cursor and backing live through post-call resets. No default
    // flush/network success or retained borrowed message/cursor ownership.
    virtual void call_transport_flush_20(std::uint32_t actual_entry,
        void* actual_transport, void* actual_target, NativeBitCursor*,
        std::uint32_t actual_delivery) = 0;
private:
    void serialize_session_message_00783c80(void* actual_transport,
        void* actual_target, NativeSessionMessageStorage* borrowed_message) final;
    const volatile std::uint16_t& tick_low_00f876b0_;
};
// In the flags4 route, callbacks may replace game/sender/valid links while all
// backing stays live. Its concurrent field mutation, other message
// profiles, invalid or nonterminating traversals and fault paths are outside.
// New source interfaces cover ordinary returning calls on valid live backing.
// Cursor bounds/error contracts are those of the existing native cursor
// services. Original allocation identity, private EH/fault behavior, complete
// binary ABI, network ownership and gameplay integration remain unproved.
} // namespace bsp
