#pragma once
// The unit command controller at unit+738h, the object the weapon bindings call
// the weapon director. Established in docs/WEAPON_DIRECTOR.md; every name here
// is a hypothesis, not a recovered symbol, except the six field names that come
// from literal strings the native property dump 008362A0 passes.
//
// Producer: 00810F60 allocates 250h bytes at 00810F85, constructs them with
// 008366D0 at 00810FA0 and stores the result at 00810FA9 into unit+738h.
#include <cstddef>
#include <cstdint>

#include "bsp/unit_weapons.hpp" // FireStance, NativeHandle, kSessionRouteChannel

namespace bsp {

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------
inline constexpr std::size_t kDirectorSize = 0x250;              // 00810F80
inline constexpr std::size_t kDirectorOffPrimaryVtable = 0x00;   // 00836717
inline constexpr std::size_t kDirectorOffSecondaryVtable = 0x1c; // 0083671D
inline constexpr std::size_t kDirectorOffSessionEndpoint = 0x34; // 0071DAA2, 0071E026
inline constexpr std::size_t kDirectorOffSubobject38h = 0x38;    // 00836777, a 40h-byte object
inline constexpr std::size_t kDirectorOffAllowFire = 0x3c;       // 00836216
inline constexpr std::size_t kDirectorOffAllowMove = 0x3d;       // 0071D5E4
inline constexpr std::size_t kDirectorOffCommandSlots = 0x54;    // 00720CD0's clear loop
inline constexpr std::size_t kDirectorCommandSlotStride = 0x1c;  // the loop's -7 dword step
inline constexpr int kDirectorCommandSlotCount = 10;             // the loop runs index 9 down to 0
inline constexpr std::size_t kDirectorOffSubKind2Dword = 0x1cc;  // 0071C1E0, the only non-boolean
inline constexpr std::size_t kDirectorOffArtilleryFlag = 0x220;  // 0071C231, sub-kind 3
inline constexpr std::size_t kDirectorOffSubKind4Flag = 0x221;   // 0071C246
inline constexpr std::size_t kDirectorOffTorpedoFlag = 0x222;    // 0071C25B, sub-kind 5
inline constexpr std::size_t kDirectorOffSubKind6Flag = 0x223;   // 0071C270
inline constexpr std::size_t kDirectorOffTargetReference = 0x224; // vtable 00CF938C, base ctor
inline constexpr std::size_t kDirectorOffFireTarget = 0x238;     // 008364E0 reads, 00836230 clears
inline constexpr std::size_t kDirectorOffTargetChangeGate = 0x23c; // 00835860
inline constexpr std::size_t kDirectorOffBaseConstructorByte = 0x23d; // 008363E0, read at 008355F0
inline constexpr std::size_t kDirectorOffTorpedoAvoidance = 0x240;      // "torpedoAvoidance"
inline constexpr std::size_t kDirectorOffShipCollisionAvoidance = 0x241; // "shipCollisionAvoidance"
inline constexpr std::size_t kDirectorOffLandCollisionAvoidance = 0x242; // "landCollisionAvoidance"
inline constexpr std::size_t kDirectorOffCruiseIsHeading = 0x243;       // "cruiseIsHeading"
inline constexpr std::size_t kDirectorOffCruiseSteerOrHeading = 0x244;  // "cruiseSteerOrHeading"
inline constexpr std::size_t kDirectorOffCruiseThrust = 0x248;          // "cruiseThrust"
inline constexpr std::size_t kDirectorOffOwnerUnit = 0x24c;             // 0083674D

inline constexpr std::uint32_t kDirectorVtableDerived = 0x00d09f58; // 00836717
inline constexpr std::uint32_t kDirectorVtableBase = 0x00d09ec0;    // 008363E0
inline constexpr std::size_t kDirectorVtableSlotStanceAllowsFire = 0x24; // 0071D560
inline constexpr std::size_t kDirectorVtableSlotStanceAllowsMove = 0x28; // 0071D580
inline constexpr std::size_t kDirectorVtableSlotFireTarget = 0x2c;       // 008364E0
inline constexpr std::size_t kDirectorVtableSlotApplyMessage = 0x38;     // 00835640
inline constexpr std::size_t kDirectorVtableSlotSendAllowFire = 0x40;    // 0071DA50
inline constexpr std::size_t kDirectorVtableSlotSendAllowMove = 0x44;    // 0071DAD0
inline constexpr std::size_t kDirectorVtableSlotIssueCommand = 0x58;     // 00720CD0
inline constexpr std::size_t kDirectorVtableSlotStoreAllowFire = 0x64;   // 00836210
inline constexpr std::size_t kDirectorVtableSlotStoreAllowMove = 0x68;   // 0071D5E0

// The fields this packet established. Everything else in the 250h bytes stays
// out: this is a projection, not the native record.
struct WeaponDirectorState {
    bool allow_fire{true};  // +3Ch, set to 1 by 008363E0 when both class tests fail
    bool allow_move{true};  // +3Dh, same site
    std::uint32_t sub_kind_2_value{0};       // +1CCh
    bool artillery_flag{false};              // +220h
    bool sub_kind_4_flag{false};             // +221h
    bool torpedo_flag{false};                // +222h
    bool sub_kind_6_flag{false};             // +223h
    NativeHandle fire_target{0};             // +238h, a raw entity pointer
    bool target_change_gate{false};          // +23Ch
    bool torpedo_avoidance{true};            // +240h, 00836724
    bool ship_collision_avoidance{true};     // +241h, 0083672A
    bool land_collision_avoidance{true};     // +242h, 00836730
    bool cruise_is_heading{false};           // +243h, 00836736
    float cruise_steer_or_heading{0.0f};     // +244h, 0083673D
    float cruise_thrust{0.0f};               // +248h, 00836745
    NativeHandle owner_unit{0};              // +24Ch, 0083674D
};

// ---------------------------------------------------------------------------
// The stance rules, recovered from the predicate bodies
// ---------------------------------------------------------------------------
// 0071D560 and 0071D580 have no Ghidra function; both were decoded from the raw
// bytes and both ignore ECX, taking the stance at [ESP+4] and returning in AL,
// RET 4. They confirm the script-table decoding that unit_weapons.hpp carries as
// a contract: the bodies agree with it exactly.
bool director_stance_allows_fire_0071d560(FireStance stance) noexcept;
bool director_stance_allows_move_0071d580(FireStance stance) noexcept;

// ---------------------------------------------------------------------------
// The kind 5Ah command message
// ---------------------------------------------------------------------------
// Ten senders share one message shape. The sub-kind at +20h selects the field
// the receiver writes; the value at +24h is always the payload.
enum class DirectorCommandSubKind : int {
    AllowFire = 0,               // 0071DA50, vtable[40h]
    AllowMove = 1,               // 0071DAD0, vtable[44h]
    SubKind2Dword = 2,           // 0071DD30
    ArtilleryEnable = 3,         // 0071DFD0
    SubKind4 = 4,                // 0071E050
    TorpedoEnable = 5,           // 0071E0D0
    SubKind6 = 6,                // 0071E150
    TorpedoAvoidance = 7,        // 00835940
    ShipCollisionAvoidance = 8,  // 008359C0
    LandCollisionAvoidance = 9,  // 00835A40
};

inline constexpr std::uint32_t kDirectorCommandMessageVtable = 0x00cfd9c4; // 0071DA92
inline constexpr std::uint32_t kDirectorTargetMessageVtable = 0x00d02e98;  // 00835782
inline constexpr int kDirectorTargetMessageKind = 0x5e;                    // 0083575A
inline constexpr int kDirectorTargetClassTestId = 2;                       // 008357A2

struct DirectorCommandMessage {
    int base_kind{0};             // 5Ah, pushed to 0075B430
    std::uint32_t vtable{0};      // +00h
    int header_dword{1};          // +04h, the literal 1 every sender writes
    int sub_kind{0};              // +20h
    std::uint32_t value{0};       // +24h
};

struct DirectorTargetMessage {
    int base_kind{kDirectorTargetMessageKind}; // 5Eh
    std::uint32_t vtable{0};                   // +00h
    int header_dword{1};                       // +04h
    std::uint16_t target_id{0};                // +20h, the target's +174h, 0 when null
    bool target_passes_class_test{false};      // +22h, target->vtable[5Ch](2)
    bool force{false};                         // +23h
};

// ---------------------------------------------------------------------------
// Host boundary
// ---------------------------------------------------------------------------
// One virtual per native call site, in the order docs/WEAPON_DIRECTOR.md lists
// them. Nothing here has a default body: none of these stands in for
// unrecovered game behaviour, and the session, the entity reference and the
// command system stay contracts.
struct WeaponDirectorHost {
    virtual ~WeaponDirectorHost() = default;

    // -- construction -------------------------------------------------------
    virtual NativeHandle allocate_director(std::size_t size) = 0;     // 00BF681B at 00810F85
    virtual NativeHandle unit_session_endpoint(NativeHandle unit) = 0; // unit vtable[60h] at 00836700
    virtual void construct_command_array(NativeHandle endpoint) = 0;  // 00720180 at 00836403
    virtual NativeHandle construct_subobject_38h() = 0;               // 009F6A20 at 0083676A
    // The two class questions 008363E0 asks the endpoint before defaulting the
    // two permissions to true. Ids 0Bh and 9; what they name is unread.
    virtual bool endpoint_class_test(NativeHandle endpoint, int class_id) = 0; // vtable[5Ch]

    // -- stance -------------------------------------------------------------
    virtual bool director_stance_allows_fire(FireStance stance) = 0; // vtable[24h] at 0071BE8F
    virtual bool director_stance_allows_move(FireStance stance) = 0; // vtable[28h] at 0071BE9D
    virtual void send_allow_fire(bool allowed) = 0;                  // vtable[40h] at 0071BEAF
    virtual void send_allow_move(bool allowed) = 0;                  // vtable[44h] at 0071BEBD

    // -- session ------------------------------------------------------------
    virtual void session_message_construct_base(int kind) = 0;  // 0075B430 at 0071DA71
    virtual void session_route_message(const DirectorCommandMessage& message,
                                       int channel,
                                       int flags) = 0;          // 0077C2A0 at 0071DAB1
    virtual void session_route_target_message(const DirectorTargetMessage& message,
                                              int channel,
                                              int flags) = 0;   // 0077C2A0 from 00835860

    // -- target -------------------------------------------------------------
    virtual std::uint16_t entity_id(NativeHandle entity) = 0;            // entity+174h at 0083578A
    virtual bool target_class_test(NativeHandle target, int class_id) = 0; // vtable[5Ch] at 008357A4
    virtual void release_target_reference(NativeHandle target) = 0;      // 006952A0 BSP_Observer_UnregisterPair at 0083622B

    // -- commands -----------------------------------------------------------
    virtual bool command_slot_occupied(int index) = 0; // the loop test in 00720CD0
    virtual void clear_command_slot(int index) = 0;    // 00720850 from 00720CD0
    virtual void issue_command(std::uint32_t descriptor,
                               bool has_target,
                               std::uint16_t target_id,
                               const float position[3]) = 0; // vtable[60h] from 00720CD0
};

// ---------------------------------------------------------------------------
// Routines
// ---------------------------------------------------------------------------
// 008366D0 and 008363E0: the field state a freshly constructed director holds.
WeaponDirectorState construct_director_008366d0(WeaponDirectorHost& host,
                                                NativeHandle unit);

// 0071BE80: ask both questions, then apply both answers. The two questions come
// first: neither answer is applied before the second is asked.
void set_fire_stance_0071be80(WeaponDirectorHost& host, FireStance stance);

// 0071BED0, 0071BF20, 0071BF70: the same four virtuals with a literal stance.
void hold_fire_0071bed0(WeaponDirectorHost& host);
void free_fire_0071bf20(WeaponDirectorHost& host);
void free_attack_0071bf70(WeaponDirectorHost& host);

// 0071DA50, 0071DAD0, 0071DFD0, 0071E0D0 and the six siblings: one builder.
DirectorCommandMessage director_command_message_0071da50(DirectorCommandSubKind sub_kind,
                                                         bool value);

// 00835640 then 0071C1E0: the receiving side. The derived override takes the
// three avoidance sub-kinds and tail-calls the base for everything else.
void apply_command_message_00835640(WeaponDirectorHost& host,
                                    WeaponDirectorState& state,
                                    const DirectorCommandMessage& message);

// 00836210 (vtable[64h]) and 0071D5E0 (vtable[68h]).
void store_allow_fire_00836210(WeaponDirectorHost& host,
                               WeaponDirectorState& state,
                               bool allowed);
void store_allow_move_0071d5e0(WeaponDirectorState& state, bool allowed) noexcept;

// 008364E0: two instructions, MOV EAX,[ECX+238h]; RET.
NativeHandle fire_target_008364e0(const WeaponDirectorState& state) noexcept;

// 00835860's gate, split out so the rule is testable on its own.
bool accepts_fire_target_00835860(const WeaponDirectorState& state, bool force) noexcept;

// 00835860 and its builder 00835740.
DirectorTargetMessage build_target_message_00835740(WeaponDirectorHost& host,
                                                    NativeHandle target,
                                                    bool force);
void set_fire_target_00835860(WeaponDirectorHost& host,
                              const WeaponDirectorState& state,
                              NativeHandle target,
                              bool force);

// 00720CD0 (vtable[58h]): clear every occupied command slot, then issue one.
// 00E08F60 is the `follow` command singleton (category 3), so 00720CD0 issues a follow
// order (docs/COMMAND_CLASSES.md, packet cc2_director_commands); the constant was first
// named as an attack descriptor from the call site alone.
inline constexpr std::uint32_t kDirectorFollowCommandDescriptor = 0x00e08f60;
void issue_target_command_00720cd0(WeaponDirectorHost& host,
                                   NativeHandle target,
                                   const float position[3]);

} // namespace bsp
