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
// New source interfaces cover ordinary returning calls on valid live backing.
// Cursor bounds/error contracts are those of the existing native cursor
// services. Original allocation identity, private EH/fault behavior, complete
// binary ABI, network ownership and gameplay integration remain unproved.
} // namespace bsp
