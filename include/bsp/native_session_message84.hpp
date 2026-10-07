#pragma once

#include "bsp/native_session_message_base.hpp"
#include "bsp/object_handle_resolvers.hpp"
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {

// Actual 38h message extent. Constructor/codec stores leave every named
// retained byte unchanged. These are raw native words, not semantic IDs/classes.
struct NativeLandingSlotMessage84 {
    NativeSessionMessageStorage base;
    std::uint16_t sender_18;
    std::uint8_t relay_1a;
    std::uint8_t retained_1b;
    std::uint8_t flag_1c;
    std::uint8_t retained_1d[3];
    std::uint32_t slot_index_20;
    std::uint32_t state_24;
    std::uint32_t vehicle_class_28;
    std::uint32_t count_2c;
    std::uint32_t raw_30;
    std::uint16_t squad_id_34;
    std::uint16_t retained_36;
};
static_assert(sizeof(void*) == 4 && sizeof(bool) == 1, "Win32 source domain");
static_assert(std::is_standard_layout<NativeLandingSlotMessage84>::value);
static_assert(offsetof(NativeLandingSlotMessage84, base) == 0);
static_assert(offsetof(NativeLandingSlotMessage84, sender_18) == 0x18);
static_assert(offsetof(NativeLandingSlotMessage84, relay_1a) == 0x1a);
static_assert(offsetof(NativeLandingSlotMessage84, retained_1b) == 0x1b);
static_assert(offsetof(NativeLandingSlotMessage84, flag_1c) == 0x1c);
static_assert(offsetof(NativeLandingSlotMessage84, retained_1d) == 0x1d);
static_assert(offsetof(NativeLandingSlotMessage84, slot_index_20) == 0x20);
static_assert(offsetof(NativeLandingSlotMessage84, state_24) == 0x24);
static_assert(offsetof(NativeLandingSlotMessage84, vehicle_class_28) == 0x28);
static_assert(offsetof(NativeLandingSlotMessage84, count_2c) == 0x2c);
static_assert(offsetof(NativeLandingSlotMessage84, raw_30) == 0x30);
static_assert(offsetof(NativeLandingSlotMessage84, squad_id_34) == 0x34);
static_assert(offsetof(NativeLandingSlotMessage84, retained_36) == 0x36);
static_assert(sizeof(NativeLandingSlotMessage84) == 0x38);

// REQUIRED complete actual 00437F50 getter, including its ordinary observable
// effects. Result aliases the actual registry with DWORDs at2010h+index*4.
// No retained object, descriptor snapshot, census or replacement table.
class NativeMessage84ClassRegistryAccess {
public:
    virtual ~NativeMessage84ClassRegistryAccess() = default;
    virtual void* get_00437f50() = 0;
};

// REQUIRED actual 00BF65AC release contract for admitted allocation identities.
// No default allocator/free. Source implementations are not runtime bindings.
class NativeMessage84Release {
public:
    virtual ~NativeMessage84Release() = default;
    virtual void free_00bf65ac(void* identity) noexcept = 0;
};

// Executable SOURCE counterpart of00CF8610's five entries. Context metadata is
// outside the original profile, after its five words: this is a new SOURCE ABI.
// Profile/contexts/actual global bank references outlive all callback dispatches.
// Scalar captures release metadata before stamping the DISTINCT existing
//00CE4974 root profile, then calls required release only for flag bit0.
struct NativeLandingSlotMessage84Profile {
    const std::uint32_t slots[5];
    const ObjectHandleTables* const banks;
    NativeMessage84Release* const release;
    NativeLandingSlotMessage84Profile(const ObjectHandleTables&, NativeMessage84Release&);
};
static_assert(std::is_standard_layout<NativeLandingSlotMessage84Profile>::value);
static_assert(offsetof(NativeLandingSlotMessage84Profile, slots) == 0);
static_assert(offsetof(NativeLandingSlotMessage84Profile, banks) == 0x14);
static_assert(offsetof(NativeLandingSlotMessage84Profile, release) == 0x18);

// Complete0095B9C0 ordinary body: null descriptor ->0; otherwise actual getter,
// THEN fresh ORIGINAL descriptor+70h and actual registry+2010h+index*4 read.
// Native ECX=descriptor, RET/EAX=raw result. Added context means new SOURCE ABI.
std::uint32_t native_vehicle_class_id_0095b9c0(
    const void* actual_descriptor, NativeMessage84ClassRegistryAccess&);

// Complete006BD520 ordinary body. Native ECX=message, stack(block,index), RET8.
// Caller supplies actual block+4Ch slot-array cell and stable58h slots. After
// actual base construction/type84 and profile stamp, capture slot once. Its
// descriptor+4 may be null; a nonnull descriptor must remain live across getter.
// State+2Ch is captured before getter; slot+8/+10/+28 are fresh afterward.
// Nonnull slot+28 requires actual live squad+174h. No slot/block/owner substitute.
// Message/block/slot/descriptor/registry/contexts must be valid native addresses
// and stay live for every access; modulo32 address arithmetic is unchecked.
// Ordinary success only: allocation, private EH/fault, invalid placement,
// concurrent access and structural reentry/lifetime mutation are not admitted.
NativeLandingSlotMessage84* construct_native_session_message84_006bd520(
    NativeLandingSlotMessage84*, const void* actual_block, std::uint32_t index,
    const NativeSessionMessageContext&, const NativeLandingSlotMessage84Profile&,
    NativeMessage84ClassRegistryAccess&);

// Complete006BD600 scalar: root stamp, optional actual release, identity return
// (possibly dangling). Native ECX=this, stack flags, RET4. No separate owned
// payload destructor exists in this body. Caller supplies valid profile/lifetime.
NativeLandingSlotMessage84* delete_native_session_message84_006bd600(
    NativeLandingSlotMessage84*, std::uint32_t flags, NativeMessage84Release&) noexcept;

// Complete006BD680/006BD710. Header + flag1 + widths6/4/10/6/4 + presence1
// + optional WORD12. Absent presence clears WORD34; padding stays unchanged.
// Native ECX=message, stack cursor/stream, RET4. Valid/disjoint message, stream,
// cursor and backing (including native carry byte), ordinary helper domain;
// supplied message84 payload only. Type acceptance does not admit49/46 payloads.
void write_native_session_message84_006bd680(const NativeLandingSlotMessage84*, NativeBitCursor*);
void read_native_session_message84_006bd710(NativeLandingSlotMessage84*, NativeSessionReadStream*);

// Complete006BD5D0 accepts exactly raw84/49/46 (native RET4), without semantic
// payload admission. Complete006BD7A0 WORD34==0 is true; otherwise use actual
// signed split/modulo32 banks and entry+Ch, with no clamp or private bank.
// Source bool results model AL only, not unspecified native upper EAX bits.
bool native_session_message84_accepts_type_006bd5d0(const NativeLandingSlotMessage84*, std::uint32_t);
bool native_session_message84_is_valid_006bd7a0(const NativeLandingSlotMessage84*, const ObjectHandleTables&) noexcept;

} // namespace bsp
