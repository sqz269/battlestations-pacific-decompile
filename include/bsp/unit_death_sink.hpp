#pragma once
#include <array>
#include <cstdint>

// The death message receiver, the wreck handler behind vtable slot 7Ch and the
// sink state. docs/UNIT_DEATH_MESSAGE_AND_SINK.md, reports/unit_death_sink.json.
//
// Read-only analysis of C:/Users/sqz269/bsp.gpr, /battlestationspacific.exe.
// Every descriptive name here is a hypothesis, not a recovered symbol, except
// `sink_time`: the executable's own `_ship` diagnostic dump at 00818340 prints
// the float at unit+828h under the literal name "sinkTime" (00818354).
//
// This is a projection of the native control flow, not an ABI-compatible
// replacement. The native routines are __thiscall over objects whose full
// layout is unreconstructed; entities are carried here as opaque handles.

namespace bsp {

// Unit-instance offsets this packet establishes. Grouped in a struct so the
// member names cannot collide with another header's constants; unit_damage.hpp
// already declares +5Dh, +70h, +740h, +828h, +82Ch and +1018h under its own
// names and nothing there is repeated as a free constant.
struct UnitDeathSinkOffsets {
    static constexpr std::uint32_t kReleasedFlag = 0x5D;      // 0092639B sets it
    static constexpr std::uint32_t kDestroyedFlag = 0x60;     // 0092639E sets it
    static constexpr std::uint32_t kDestroyCause = 0x70;      // 00825093 tests == 1
    static constexpr std::uint32_t kSinkAttachment = 0x740;   // 00811126, 00825254
    static constexpr std::uint32_t kSinkTime = 0x828;         // zeroed 00811116, 0082507E
    static constexpr std::uint32_t kSinkTimeCompanion = 0x82C; // zeroed 0081111E, 00825086
    static constexpr std::uint32_t kHullAnchorPoints = 0xB68; // float3[5], stride 0Ch
    static constexpr std::uint32_t kHullAnchorStride = 0x0C;
    static constexpr std::uint32_t kBubbleTimer = 0xBC8;      // seeded 008250C8
    static constexpr std::uint32_t kMotionController = 0x1018; // 00814563, 00824FF0
    static constexpr std::uint32_t kClassDescriptor = 0x538;  // 0081456E
    static constexpr std::uint32_t kLeakManager = 0x10D4;     // 00824FE5
};

// Vehicle class descriptor offsets this packet reads. The kamikaze pair is
// already named in ship_class_fields.hpp (ShipClassDescriptorOffsets); these
// are the ones that header does not carry.
struct WreckClassOffsets {
    static constexpr std::uint32_t kHullLength = 0xA0;   // "Length"
    static constexpr std::uint32_t kHullWidth = 0xA4;    // "Width"
    static constexpr std::uint32_t kHullHeight = 0xA8;
    static constexpr std::uint32_t kBreakupSelector = 0xB0;   // 00827ACA
    static constexpr std::uint32_t kAnchorsEnabled = 0x6A8;   // 008250CE, 00825C11
    static constexpr std::uint32_t kBreakupPiecesA = 0x6E0;   // stride 14h
    static constexpr std::uint32_t kBreakupPiecesB = 0x700;   // stride 1Ch
};

// Message kinds of the two death arms of 00821E80's jump table. Kind 99h is
// already kUnitMessageKindDetachPart in unit_parts.hpp.
inline constexpr std::uint8_t kUnitMessageKindBreakup = 0x98; // 4Bh + index 4Dh
inline constexpr std::uint8_t kUnitMessageKindDeath = 0x9A;   // 4Bh + index 4Fh

// The third death slot: 00926390 (slot 74h) tail-dispatches it at 009263AE.
inline constexpr std::size_t kEntityVtableSlotOnWrecked = 0x7C;

// The cause 00926C80 stores at unit+70h for a queued destroy; 00825093 runs the
// wreck effect block only for this value.
inline constexpr int kUnitDestroyCauseQueued = 1;

// 00824FE5..00825074 constants.
inline constexpr float kWreckInertiaScale = 2.0f;     // double 2.0 at 00D7A308
inline constexpr float kWreckAngularDamping = 2.5f;   // 00CF87C8
inline constexpr float kWreckLinearDamping = 0.5f;    // 00CE3800

// 008250F0..008251CD constants.
inline constexpr int kWreckAnchorCount = 5;              // MOV EBX,5 at 008250E9
inline constexpr double kWreckAnchorLateralDivisor = 2.0;  // 00CE3DE0
inline constexpr double kWreckAnchorVerticalDivisor = 3.0; // 00D7A2B0

// 00827ACA/00827AD0: the selector compares against the double 100.0, not zero.
inline constexpr double kBreakupSelectorThreshold = 100.0; // 00D7A220

// --------------------------------------------------------------------------
// Pure rules
// --------------------------------------------------------------------------

// 00814574..0081458F. The 9Ah receiver kills the entity only when the class
// descriptor asks for kamikaze damage; both COMISS tests are strict.
bool death_message_kills_00814560(float kamikaze_damage,
                                  float kamikaze_blast_damage) noexcept;

// 00827ACA..00827ADA. True when 00827A90 sends the 9Ah death message; false
// takes the 98h breakup branch, which then has two more gates and a random roll
// this projection does not model.
bool sends_death_message_00827ada(float class_breakup_selector) noexcept;

// 008110F3..00811108. Any non-zero invincibility blocks a sink, and so does the
// released flag; 00D7A218 is 0.0f and the branch is JA.
bool sink_refused_008110f0(bool released, float invincibility) noexcept;

// The state 00824FE5..00825086 leaves on a wrecked hull. This is the whole of
// the settle and roll rule: three one-off physics writes and two cleared
// floats. Nothing in the image advances sink_time afterwards, so the descent is
// the rigid body's own response under 004462D0's buoyancy step.
struct WreckSettleState {
    std::array<float, 3> inertia{};   // 00825044
    float angular_damping{};          // 0082505C
    float linear_damping{};           // 00825074
    float sink_time{};                // 0082507E
    float sink_time_companion{};      // 00825086
};
WreckSettleState wreck_settle_00824fe5(const std::array<float, 3>& hull_inertia) noexcept;

// Hull half-extents from the class descriptor, +A0h/+A4h/+A8h.
struct ShipHullExtents {
    float length{};
    float width{};
    float height{};
};

// The (lo, hi) pairs 00824B60 passes to the random helper for one anchor slot.
// Component order is the stored order: x from the width, y from the height,
// z from the length (008250F0..008251CD).
struct WreckAnchorRanges {
    float x_lo{};
    float x_hi{};
    float y_lo{};
    float y_hi{};
    float z_lo{};
    float z_hi{};
};
WreckAnchorRanges wreck_anchor_ranges_008250f0(const ShipHullExtents& extents) noexcept;

// The same draw expressed over three unit samples in [0, 1], for a caller that
// has its own generator. Each slot starts from zero (008250F9..00825103) and is
// then incremented once, so the sum is just the draw.
std::array<float, 3> wreck_anchor_point(const WreckAnchorRanges& ranges,
                                        const std::array<float, 3>& unit_samples) noexcept;

// 00780090's arm selection, the second caller of the entity-kind dispatch.
// Every arm ends with the kind dispatch, so only the vtable calls differ.
struct MemberMessageFields {
    bool flag_20h{};       // 00780095
    bool flag_30h{};       // 007800BD
    int value_2Ch{};       // 007800D7
    std::uint32_t arg_24h{};
    std::uint32_t arg_28h{};
};
struct MemberMessagePlan {
    bool call_slot_144h{};
    bool call_slot_148h{};
    bool call_slot_150h{};
    std::uint32_t slot_150h_second_arg{}; // arg_28h, or the literal 8 at 00780100
};
MemberMessagePlan member_message_plan_00780090(const MemberMessageFields& fields) noexcept;

// --------------------------------------------------------------------------
// Hosts: one virtual method per native call site
// --------------------------------------------------------------------------

// An entity carried as an opaque handle; the native routines pass pointers.
using EntityRef = std::uint32_t;

// The unit fields the two receivers read before they call anything.
struct DyingUnit {
    EntityRef unit{};
    EntityRef parts_object{};       // unit+1018h
    EntityRef motion_controller{};  // unit+1018h, the same pointer
    EntityRef class_descriptor{};   // unit+538h
    float kamikaze_damage{};        // descriptor+510h
    float kamikaze_blast_damage{};  // descriptor+514h
};

struct UnitDeathMessageHost {
    virtual ~UnitDeathMessageHost() = default;
    // 00814569 -> 00935C70 BSP_UnitParts_DetachAllLiveParts
    virtual void detach_all_live_parts(EntityRef parts_object) = 0;
    // 00814595 -> 00926D90 BSP_MissionEntity_Kill, cause 1
    virtual void kill_entity(EntityRef unit, int cause) = 0;
    // 008145A1 tail -> 0092BD30, clears +2Ch and +30h on every hull shape
    virtual void clear_hull_shape_fields(EntityRef motion_controller) = 0;
    // 00814529 -> 0080F9F0, both descriptor piece vectors non-empty test
    virtual bool class_has_breakup_pieces(EntityRef class_descriptor) = 0;
    // 00814537/00825225 -> [00E188A8]+21D0h, the wreck manager or 0
    virtual EntityRef wreck_manager() = 0;
    // 00814547 and 0082523A -> 004A5AA0
    virtual void register_wreck(EntityRef manager, EntityRef unit) = 0;
    // 00814553 tail -> 00935D30
    virtual void breakup_parts(EntityRef motion_controller) = 0;
};

// 00814560, the kind 9Ah receiver. __thiscall(unit), RET 0: the message itself
// is never read, so nothing about the payload reaches this function.
void on_death_message_00814560(UnitDeathMessageHost& host, const DyingUnit& unit);

// 00814520, the kind 98h receiver.
void on_breakup_message_00814520(UnitDeathMessageHost& host, const DyingUnit& unit);

// The settings pair the bubble timer is seeded from, 00424C40()+64Ch/+650h.
struct WreckBubbleSettings {
    float low{};   // cfg+64Ch, the first argument at 008250BD
    float high{};  // cfg+650h, the second
};

struct WreckedUnit {
    EntityRef unit{};
    EntityRef hull_body{};       // [[unit+1018h]+2Ch]
    int destroy_cause{};         // unit+70h
    bool anchors_enabled{};      // descriptor+6A8h
    ShipHullExtents extents{};
};

struct WreckPhysicsHost {
    virtual ~WreckPhysicsHost() = default;
    // 00824FEB -> 0074EC50(&unit+10D4h)
    virtual void reset_leak_manager(EntityRef unit) = 0;
    // 00825001 -> 00C37F10
    virtual std::array<float, 3> read_body_inertia(EntityRef hull_body) = 0;
    // 00825044 -> 00C37E70 Dyn_Body_SetInertia
    virtual void set_body_inertia(EntityRef hull_body, const std::array<float, 3>& inertia) = 0;
    // 0082505C -> 00C37DE0 Dyn_Body_SetAngularDamping
    virtual void set_angular_damping(EntityRef hull_body, float rate) = 0;
    // 00825074 -> 00C37E00 Dyn_Body_SetLinearDamping
    virtual void set_linear_damping(EntityRef hull_body, float rate) = 0;
    // 0082507E and 00825086
    virtual void store_sink_fields(EntityRef unit, float sink_time, float companion) = 0;
    // 0082508E -> 00818970, releases the effect handles at +BB0h, +BC0h, +BACh
    virtual void release_attached_effects(EntityRef unit) = 0;
    // 008250BD, 0082513E, 00825186, 008251BC -> 00BD2F10
    virtual float random_range(float low, float high) = 0;
    // 008250C8
    virtual void store_bubble_timer(EntityRef unit, float seconds) = 0;
    // 0082514D, 00825196, 008251CA
    virtual void store_anchor_point(EntityRef unit, int index,
                                    const std::array<float, 3>& point) = 0;
};

// 00824FE5..0082523F, the sink block of the ship's vtable[7Ch] override. The
// effect and sound teardown before it (00824B60..00824FE4) and the epilogue
// after it are not modelled; see the doc's coverage table.
void on_wrecked_00824fe5(WreckPhysicsHost& host,
                         const WreckedUnit& wreck,
                         const WreckBubbleSettings& settings);

}  // namespace bsp
