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
// Callbacks may replace game/sender/valid links while all reached backing stays
// live. Concurrent mutation between ordinary field observations, other message
// profiles, invalid or nonterminating traversals and fault paths are outside.
// New source interfaces cover ordinary returning calls on valid live backing.
// Cursor bounds/error contracts are those of the existing native cursor
// services. Original allocation identity, private EH/fault behavior, complete
// binary ABI, network ownership and gameplay integration remain unproved.
} // namespace bsp
