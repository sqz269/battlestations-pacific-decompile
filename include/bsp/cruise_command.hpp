#pragma once
// The authored `Cruise` order: from the scene's command record to the ordered
// throttle and steering the ship motion runs on.
//
// Subject: the command-type object 00E08F70 (`cruise`, ordinal 16), the unit's
// MT_COMMAND apply 00816E30 (MDestroyer vtable slot 160h), the weapon-director
// hop 0071ECF0 -> 0071E550 -> 0071C830, the MT_GAMEUNIT_SETCMD receive
// 00721A40 -> 008358D0 -> 0071E6C0 (the command-slot push), the cruise latch
// 00835AC0 reached from 00835C70, and the per-step cruise rule 009E1170.
//
// Evidence, addresses, coverage and the unread boundaries: docs/CRUISE_COMMAND.md
// and reports/cruise_command.json.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The
// literals in the image are the command name `cruise`, the message names
// MT_COMMAND, MT_GAMEUNIT_SETCMD and MT_GAMEUNIT_CLEARCMD, and the three field
// names cruiseIsHeading / cruiseSteerOrHeading / cruiseThrust that the native
// property dump 008362A0 passes.
//
// No struct, enum or k* name declared here is declared by another header in
// include/bsp. The 0x18-byte target descriptor is bsp::SceneCommandTarget from
// scene_deferred_refs.hpp; the director offsets, its vtable constants and
// bsp::WeaponDirectorState are from weapon_director.hpp; the order ring is
// bsp::UnitOrderRing from unit_state_message.hpp; the command-class table is
// from entity_orders.hpp. All four are reused, not redeclared.

#include <cstddef>
#include <cstdint>

#include "bsp/entity_orders.hpp"
#include "bsp/scene_deferred_refs.hpp"
#include "bsp/unit_state_message.hpp"
#include "bsp/weapon_director.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The `cruise` command class
// ---------------------------------------------------------------------------

// Row 16 of the registry table in entity_orders.hpp: object 00E08F70, vtable
// 00CFB554, name getter 006F8A30 returning the literal "cruise", category 3
// (movement), target predicate false. Nothing else is stored in the object, so
// the command carries no parameters at all: both producers below pass an empty
// descriptor.
inline constexpr int kCruiseCommandOrdinal = 16;
inline constexpr std::uint32_t kCruiseCommandObjectAddress = 0x00e08f70u;
inline constexpr int kCruiseCommandCategory = 3;

// The two producers of a `Cruise` with no target, and they agree exactly.
//   0046AC0B  the scene deferred-reference queue, from the authored `Command`
//             property that 00469610 queued at load.
//   008A7710  luaMW_NavigatorCruise 008A75C0, one Lua argument (the entity).
// Both reach 0077D600 with command = 00E08F70, flags = 1 and a descriptor whose
// kind is 0 and whose position is the read-only zero vector at 00F87574.
inline constexpr std::uint8_t kCruiseCommandIssueFlags = 1;
SceneCommandTarget cruise_command_empty_target() noexcept;

// ---------------------------------------------------------------------------
// The transport hops
// ---------------------------------------------------------------------------

// MT_GAMEUNIT_SETCMD, message type 5Ch, name literal 00D0262C through the type
// table at 00E0AB68. Built by 0071C830 with vtable 00CFD9EC. Its payload is the
// MT_COMMAND payload with the two leading bytes swapped: +20h is the caller's
// flag and +21h the command ordinal, where 007798D0 writes the ordinal at +20h
// and the flags at +21h.
inline constexpr std::uint8_t kGameUnitSetCommandMessageType = 0x5c;
inline constexpr std::uint32_t kGameUnitSetCommandMessageVtable = 0x00cfd9ecu;
// MT_GAMEUNIT_CLEARCMD, type 5Dh, literal 00D02614, vtable 00CFD9D8, built by
// 0071D880 with +20h = 1 and +24h = -1, which is "clear every slot".
inline constexpr std::uint8_t kGameUnitClearCommandMessageType = 0x5d;
inline constexpr std::uint32_t kGameUnitClearCommandMessageVtable = 0x00cfd9d8u;
inline constexpr std::int32_t kGameUnitClearCommandAllSlots = -1;

// The category ids the two classes answer true for, from their vtable[0Ch]
// bodies 00764D00 (MT_COMMAND) and 0071C900 (MT_GAMEUNIT_SETCMD). Neither
// answers 48h, so neither takes the entity-sync branch of 00780670; both answer
// 49h, which widens that routine's tick window from 3Ch to 1770h ticks.
inline constexpr int kSessionCategoryWideTickWindow = 0x49;   // 00780670
inline constexpr int kSessionCategoryEntityCommand = 0x58;    // 00780607
inline constexpr int kSessionCategoryGameUnitMessage = 0x59;  // 00780636
bool entity_command_message_is_category_00764d00(int category) noexcept;
bool gameunit_set_command_message_is_category_0071c900(int category) noexcept;

// The dispatch slot 00780120 uses once a message answers 58h: the unit virtual
// at primary-vtable slot 160h, which for MDestroyer (vtable 00CFC3D0) is
// 00816E30. Slot 59h goes to 00721A40 instead, through the one-word wrapper
// 00778820 builds around unit->vtable[114h]() (the weapon director).
inline constexpr std::size_t kUnitVtableSlotApplyEntityCommand = 0x160;  // 00816E30
inline constexpr std::size_t kUnitVtableSlotWeaponDirector = 0x114;      // 0080E150
inline constexpr std::size_t kUnitVtableSlotHeading = 0x50;              // 00835E46
inline constexpr std::size_t kUnitVtableSlotCurrentCommand = 0x174;      // 0080E1B0
inline constexpr std::size_t kUnitVtableSlotCurrentCommandTarget = 0x178; // 00812CD0

// The ordinal-to-object decode. 0077A050 reads the byte at message +20h and
// 007216D0 the byte at +21h; both then walk the registry list at 00E19A70 and
// return the first object whose +4h ordinal matches, or null. 0FFh is the
// "no command" ordinal 0071C830 writes when its command argument is null.
inline constexpr std::uint8_t kCruiseCommandNoOrdinal = 0xff;

// ---------------------------------------------------------------------------
// The director's command slots and the two command modes
// ---------------------------------------------------------------------------

// 0071E6C0 writes the command pointer at director + 54h + i*1Ch and assigns the
// 0x18-byte descriptor at director + 58h + i*1Ch, so one slot is a command and
// a SceneCommandTarget packed into 1Ch bytes. kDirectorOffCommandSlots,
// kDirectorCommandSlotStride and kDirectorCommandSlotCount come from
// weapon_director.hpp and are not redeclared.
inline constexpr std::size_t kCruiseSlotCommandOffset = 0x00;  // 0071E764
inline constexpr std::size_t kCruiseSlotTargetOffset = 0x04;   // 0071E768

struct CruiseCommandSlot {
    std::uint32_t command{0};    // slot +00h, the command-type object address
    SceneCommandTarget target{}; // slot +04h..+1Bh
};

// director+30h, written to 1 by 0071E6C0 the first time a slot is filled and
// read by 0071BE40: 1 means the current command is slot 0 (director+54h), 2
// means the override pair at director+188h/+18Ch, anything else means none.
inline constexpr std::size_t kCruiseDirectorOffCommandMode = 0x30;      // 0071E78F
inline constexpr std::size_t kCruiseDirectorOffOverrideCommand = 0x188; // 0071BE51
inline constexpr std::size_t kCruiseDirectorOffOverrideTarget = 0x18c;  // 00835C92
inline constexpr std::size_t kCruiseDirectorOffCommandStage = 0x48;     // 0071D810
enum class CruiseCommandMode : int {
    None = 0,
    QueuedSlots = 1,   // 0071BE48 returns director+54h
    Override = 2,      // 0071BE51 returns director+188h
};
// 0071BE40, __fastcall(director), returns the current command object or null.
std::uint32_t director_current_command_0071be40(CruiseCommandMode mode,
                                                std::uint32_t slot0_command,
                                                std::uint32_t override_command) noexcept;

// Vtable slots of 00D09F58 this packet reads. weapon_director.hpp already names
// 24h, 28h, 2Ch, 38h, 40h, 44h, 58h, 64h and 68h; these five are new. Slot 58h
// is 00720CD0, the clear-and-issue helper; the plain command setter is 60h.
inline constexpr std::size_t kCruiseDirectorVtableSlotMakeRoom = 0x30;      // 0071E550
inline constexpr std::size_t kCruiseDirectorVtableSlotAcceptCommand = 0x34; // 00835E90
inline constexpr std::size_t kCruiseDirectorVtableSlotSupersedes = 0x14;    // 00836040
inline constexpr std::size_t kCruiseDirectorVtableSlotSetCommand = 0x60;    // 008358D0
inline constexpr std::size_t kCruiseDirectorVtableSlotBeginCommand = 0x78;  // 00835C70
inline constexpr std::size_t kCruiseDirectorVtableSlotStep = 0x7c;          // 00836920

// ---------------------------------------------------------------------------
// The cruise autopilot fields and their two pure rules
// ---------------------------------------------------------------------------

// 00835AC0 constants. 00D7A208 is -0.0f, so `(r > 0) ? r : (-0.0f - r)` is |r|;
// 00D7A238 is 0.01f; 00D7A270 is the double 0.05; 00D7A24C is 1.0f; 00D7A218 is
// zero. All four are read straight out of the image.
inline constexpr float kCruiseHeadingRudderEpsilon = 0.01f;    // 00835AE0
inline constexpr double kCruiseHeadingThrustEpsilon = 0.05;    // 009E131D
inline constexpr float kCruiseHeadingSpeedEpsilon = 1.0f;      // 009E1347
inline constexpr float kCruiseSpeedSettingInactive = -1.0f;    // 009E11C6, 00D7A260

// 00835AC0, __thiscall(director)(float rudder, float heading, float thrust),
// RET 0Ch. It writes all three cruise fields and nothing else:
//   cruiseIsHeading      = |rudder| < 0.01f
//   cruiseSteerOrHeading = cruiseIsHeading ? heading : rudder
//   cruiseThrust         = thrust
// One field holds either a rudder or a heading, which is what the literal name
// cruiseSteerOrHeading says.
struct CruiseAutopilotFields {
    bool is_heading{false};        // director +243h
    float steer_or_heading{0.0f};  // director +244h
    float thrust{0.0f};            // director +248h
};
CruiseAutopilotFields cruise_latch_00835ac0(float ordered_rudder,
                                            float heading_radians,
                                            float ordered_throttle) noexcept;

// 00835E17..00835E58, the `cruise` arm of the begin-command routine 00835C70.
// The arguments it hands 00835AC0 are the unit's live pair and its heading:
//   rudder  = ring +14Ch (unit+984h), the ring's current ordered rudder
//   heading = unit->vtable[50h](), the current heading
//   thrust  = ring +148h (unit+980h), the ring's current ordered throttle
// So `Cruise` means "hold what you are doing now": it captures the standing
// order rather than carrying one.
CruiseAutopilotFields cruise_command_begin_00835e17(const UnitOrderRing& ring,
                                                    float heading_radians) noexcept;

// The commanded-speed pair at *(unit+73Ch) +24h / +28h. The enable is a float:
// 009E12AC takes the override branch when it is >= 0.0f, and 009E11C6 disables
// it by storing -1.0f. Its producer is not read by this packet.
struct CruiseSpeedSetting {
    float speed{0.0f};                            // *(unit+73Ch) +24h
    float enable{kCruiseSpeedSettingInactive};    // *(unit+73Ch) +28h
};

// What the cruise state asks of the ship each step.
enum class CruiseSteerMode : int {
    Rudder = 0,   // 009E1384: 009DFFB0 with cruiseSteerOrHeading
    Heading = 1,  // 009E1354: 009E0040 with cruiseSteerOrHeading
    Straight = 2, // 009E1350: 009DFFB0 with a literal zero
};
struct CruiseOrderedValues {
    float throttle{0.0f};
    CruiseSteerMode mode{CruiseSteerMode::Rudder};
    float steer_or_heading{0.0f};
};

// 009E1170's AI arm, 009E1265..009E13B1. Pure in its four inputs:
//   throttle = cruiseThrust, replaced by speed / reference_speed when the speed
//              setting is enabled;
//   !cruiseIsHeading            -> Rudder  with cruiseSteerOrHeading
//   cruiseIsHeading and moving  -> Heading with cruiseSteerOrHeading
//   cruiseIsHeading and stopped -> Straight (a literal zero steer)
// "moving" is |throttle| > 0.05 or |body-axis speed| > 1.0f; the speed is made
// positive at 009E1297 (AND 7FFFFFFFh) before the compare.
CruiseOrderedValues cruise_ordered_values_009e1170(const CruiseAutopilotFields& fields,
                                                   const CruiseSpeedSetting& speed_setting,
                                                   float reference_speed,
                                                   float body_axis_speed) noexcept;

// 0071E550, director vtable[30h], __thiscall(director)(command, target),
// RET 8. It pushes nothing: it drops the top queued command when both it and
// the incoming command are weapon commands, so the incoming one has room.
// Returns true when it dropped one. `slot_count` is the index of the first
// empty slot, which is the loop at 0071E56B..0071E57E.
bool cruise_make_room_0071e550(int incoming_category,
                               int slot_count,
                               int top_slot_category) noexcept;

// 00835E90, director vtable[34h], __thiscall(director)(command), RET 4, no
// Ghidra function (00835E90-00835F02, decoded from raw bytes). The gate
// 00816E30 applies when the message's flags byte is zero. `base_allows` is
// 0071C0C0's answer, __thiscall(director)(command): false for a null command,
// otherwise "the first empty slot index is below 10", i.e. there is room.
// 00E08FA0 is `land` and 00E08F60 is `follow`.
bool cruise_command_accepted_00835e90(bool base_allows,
                                      int slot_count,
                                      std::uint32_t top_slot_command,
                                      int top_slot_category,
                                      int incoming_category) noexcept;

// ---------------------------------------------------------------------------
// The host boundary
// ---------------------------------------------------------------------------
// One pure virtual per native call site, in the order docs/CRUISE_COMMAND.md
// lists them. Nothing has a default body: the session, the entity handle
// tables, the AI group and the ship AI's own controller stay contracts.
struct CruiseCommandHost {
    virtual ~CruiseCommandHost() = default;

    // -- 00816E30, the unit's MT_COMMAND apply -----------------------------
    // 0077A050 at 00816E9C: the message's +20h ordinal to a registry object.
    virtual std::uint32_t command_object_from_message_ordinal(std::uint8_t ordinal) = 0;
    // 0071D880 at 0081733E: build MT_GAMEUNIT_CLEARCMD (+20h = 1, +24h = -1)
    // and route it; the slots are cleared on the receive side, not here.
    virtual void clear_all_commands() = 0;
    // director vtable[34h] = 00835E90 at 0081734B, only when flags == 0.
    virtual bool command_accepted(std::uint32_t command) = 0;
    // 0071ECF0 at 0081735D.
    virtual void director_issue_command(std::uint32_t command,
                                        const SceneCommandTarget& target) = 0;

    // -- 0071ECF0, the director hop ----------------------------------------
    // [director+34h]->vtable[140h] at 0071ED19. The callee is an indirect slot
    // on the session endpoint and its body was not read (contract: unread); what
    // 0071ECF0 does with the result is ask it IsKindOf(2) and read its +16Ch, so
    // it is an entity. The name says where it comes from, not what it does.
    virtual std::uint32_t endpoint_subject_vtable140() = 0;
    // entity vtable[5Ch] at 0071ED32, class id 2.
    virtual bool entity_is_kind_of(std::uint32_t entity, int class_id) = 0;
    // entity +16Ch, read at 0071ED38 and used as ECX at 0071ED54.
    virtual std::uint32_t entity_ai_group(std::uint32_t entity) = 0;
    // 007788B0 at 0071ED43, __fastcall(entity), RET 0, body 007788B0-007788C7:
    // ctrl = [entity+284h]; return ctrl != 0 && [ctrl+14h] != entity. The
    // forward is skipped when it is true. entity+284h is the controller
    // back-pointer of docs/ENTITY_ORDER_MESSAGE.md and its +14h the owner.
    virtual bool entity_controller_is_another_entity(std::uint32_t entity) = 0;
    // 00A2BD90 at 0071ED54.
    virtual void ai_group_forward_command(std::uint32_t ai_group,
                                          std::uint32_t command,
                                          const SceneCommandTarget& target) = 0;
    // director vtable[30h] = 0071E550 at 0071ED62.
    virtual bool make_room_for_command(std::uint32_t command,
                                       const SceneCommandTarget& target) = 0;
    // 0071C830 at 0071ED6C, then 0077C2A0 at 0071ED81 with routing flags 7 and
    // ECX = director+34h.
    virtual void route_set_command_message(std::uint32_t command,
                                           const SceneCommandTarget& target,
                                           std::uint8_t flag) = 0;

    // -- 00721A40, the MT_GAMEUNIT_SETCMD receive --------------------------
    // message vtable[0Ch] at 00721AEE and the sibling category tests.
    virtual bool message_is_category(int category) = 0;
    // 00721030 at 00721B11 / 00721B22 / 00721B47: rebuild the descriptor.
    virtual SceneCommandTarget message_target_descriptor() = 0;
    // 00521EA0 at 00721B29: the mode-2 drop when the target cannot resolve.
    virtual std::uint32_t resolve_target_object(const SceneCommandTarget& target) = 0;
    // 007216D0 at 00721B4F: the +21h ordinal to a registry object.
    virtual std::uint32_t message_command_object() = 0;
    // director vtable[60h] = 008358D0 at 00721B5A.
    virtual bool director_set_command(std::uint32_t command,
                                      const SceneCommandTarget& target) = 0;
    // 0071E7F0 at 00721B7C, the flag == 0 arm. Its body is unread.
    virtual void director_queue_command(std::uint32_t command,
                                        const SceneCommandTarget& target) = 0;

    // -- 008358D0 and 0071E6C0, the slot push ------------------------------
    // command vtable[0Ch] at 008358F9, 0071E7A8 and 0071E59B.
    virtual int command_category(std::uint32_t command) = 0;
    // 00835930: 00835860 with force = 1.
    virtual void set_fire_target(std::uint32_t target_object) = 0;
    // 0071D780 at 0071E6C3, __fastcall(director), RET 0, body 0071D780-0071D807:
    // walks the slots and returns the index of the first empty one, except that a
    // moveonpath slot (00E08F80) whose path object at director+1A4h+i*4 reports a
    // non-empty point vector contributes that point count instead of 1. The queue
    // is full at 10.
    virtual int command_count() = 0;
    // The slot's command pointer, director + 54h + i*1Ch. A field read, not a
    // call: 0071E6E0 for the empty-slot scan and 0071E721 for the duplicate
    // test, and 0071E593 / 00835EC3 read the same word.
    virtual std::uint32_t slot_command(int slot_index) = 0;
    // 0071E200 at 0071E70C, __thiscall(slotTarget)(otherTarget), body
    // 0071E200-0071E2D9: both descriptors resolve to the same object and their
    // positions are within a squared distance of 1.0, where a descriptor whose
    // position_valid byte is clear contributes the zero vector at 00F87574.
    virtual bool slot_target_matches(int slot_index, const SceneCommandTarget& target) = 0;
    // 0071D6D0 at 0071E72A, __thiscall(director)(command, target), body
    // 0071D6D0-0071D772: when the command's vtable[8] requires a target and either
    // the descriptor's position_valid byte is clear or the category is 1 or 2, the
    // target must resolve and must not carry the +5Dh flag; torpedo (00E08F18) and
    // moveonpath (00E08F80) add their own tests through 009229F0 and 007AC9D0,
    // whose bodies are unread.
    virtual bool command_allowed(std::uint32_t command, const SceneCommandTarget& target) = 0;
    // director vtable[14h] = 00836040 at 0071E73C, __thiscall(director)(command,
    // target), body 00836040-008360BC. It is not a supersede test: when the
    // descriptor names a target, that target resolves to the director's own
    // endpoint at director+34h, and the command is cruise or stop, it REWRITES
    // the descriptor in place to an empty one (kind 0, id 0, object 0, the zero
    // vector at 00F87574, zero trailer) and returns 1. It returns 0 and writes
    // nothing otherwise. The reference is what lets it rewrite.
    virtual bool normalize_self_target(std::uint32_t command, SceneCommandTarget& target) = 0;
    // 0071E764 and 0071DB50 at 0071E76C: the two halves of one slot.
    virtual void store_slot(int slot_index, std::uint32_t command,
                            const SceneCommandTarget& target) = 0;
    // 00694A60 at 0071E78A, with EDX = director+1Ch.
    virtual void observe_target(std::uint32_t target_object) = 0;
    // 0071E78F: director+30h, set to QueuedSlots only when it was None.
    virtual CruiseCommandMode command_mode() = 0;
    virtual void set_command_mode(CruiseCommandMode mode) = 0;
    // director+188h at 0071E7B4, then 006E38E0 at 0071E7C9 with
    // ECX = [00E188A8]+1EF0h, body 006E38E0-006E38F7: x = [this+0F4h]; return
    // x == 0 || x == 1. Then 0071E2E0 at 0071E7D4, body unread.
    virtual std::uint32_t override_command() = 0;
    virtual bool session_field_f4h_is_0_or_1() = 0;
    virtual void echo_command() = 0;

    // -- 00835C70's cruise arm ---------------------------------------------
    // 0071D810 at 00835E12 with stage 1: director+48h only ever rises.
    virtual void raise_command_stage(int stage) = 0;
    // unit vtable[50h] at 00835E46.
    virtual float unit_heading() = 0;
    // 00835AC0 at 00835E58.
    virtual void store_cruise_fields(const CruiseAutopilotFields& fields) = 0;

    // -- 009E1170's cruise step --------------------------------------------
    // 008356F0 at 009E126B, 008356C0 at 009E130E, 008356D0 at 009E138C and
    // 008356E0 at 009E135C: the three director getters, all one instruction.
    virtual CruiseAutopilotFields cruise_fields() = 0;
    // 0092D730 at 009E1282, ECX = unit+1018h.
    virtual float body_axis_speed() = 0;
    // 0080FC30 at 009E12CE.
    virtual float reference_speed() = 0;
    // *(unit+73Ch) +24h / +28h, read at 009E12A7 and 009E12BD.
    virtual CruiseSpeedSetting speed_setting() = 0;
    // 009DFFB0 at 009E1397, 009E0040 at 009E1367 and 009DBF90 at 009E13A6. All
    // three are __thiscall(state)(float) with RET 4 and all three work on the
    // control block at [state]+8, which is what their names describe:
    //   009DFFB0 sets the block's mode word +1C4h to 0 and stores the value
    //            clamped to [-1,+1] (00D7A260, 00D7A24C) at +1D4h;
    //   009E0040 sets the same mode word to 1 and stores the value unclamped at
    //            +1D8h, then calls 00605070 on it (body unread);
    //   009DBF90 clears +1C8h and +1CCh and stores the value clamped to [-1,+1]
    //            at +1D0h.
    // Both mode switches first zero +360h and +368h. How the block reaches
    // unit+0FC4h / unit+0FDCh is contract: unread.
    virtual void set_desired_steering(float rudder) = 0;
    virtual void set_desired_heading(float heading_radians) = 0;
    virtual void set_desired_throttle(float throttle) = 0;
};

// ---------------------------------------------------------------------------
// The sequences
// ---------------------------------------------------------------------------

// 00816E30, __thiscall(unit)(message), RET 4. Projected for the movement
// commands only: `cruise`, `stop` and `moveonpath` skip every branch of
// 00816EA6..00817330 and fall straight through to the tail at 00817334.
// Coverage is partial; docs/CRUISE_COMMAND.md lists the unprojected ranges.
struct EntityCommandMessageView {
    std::uint8_t command_ordinal{0};   // +20h
    std::uint8_t flags{0};             // +21h
    SceneCommandTarget target{};       // +24h..+38h
};
void unit_apply_entity_command_00816e30(CruiseCommandHost& host,
                                        const EntityCommandMessageView& message) noexcept;

// 0071ECF0, __thiscall(director)(command, target), RET 8. Complete,
// 0071ECF0-0071ED9A.
void director_issue_command_0071ecf0(CruiseCommandHost& host,
                                     std::uint32_t command,
                                     const SceneCommandTarget& target) noexcept;

// 00721A40's 5Ch arm, 00721AEE..00721B87. `session_mode` is [00E188A8]+1FE4h;
// only mode 2 runs the resolve-or-drop test at 00721B0A.
void gameunit_apply_set_command_00721a40(CruiseCommandHost& host,
                                         int session_mode,
                                         std::uint8_t message_flag) noexcept;

// 008358D0, __thiscall(director)(command, target), RET 8, returns whether the
// push happened. Complete, 008358D0-0083593C.
// The descriptor is taken by reference because 00836040, reached through
// 0071E6C0, rewrites it in place for a self-targeted cruise or stop.
bool director_set_command_008358d0(CruiseCommandHost& host,
                                   std::uint32_t command,
                                   SceneCommandTarget& target,
                                   int session_mode) noexcept;

// 0071E6C0, __thiscall(director)(command, target), RET 8. Complete,
// 0071E6C0-0071E7E1.
bool director_push_command_slot_0071e6c0(CruiseCommandHost& host,
                                         std::uint32_t command,
                                         SceneCommandTarget& target) noexcept;

// 00835E0E..00835E5D, the `cruise` arm of 00835C70.
void cruise_command_begin_00835c70(CruiseCommandHost& host,
                                   const UnitOrderRing& ring) noexcept;

// 009E1170's AI arm, 009E1265..009E13B1.
CruiseOrderedValues cruise_state_step_009e1170(CruiseCommandHost& host) noexcept;

} // namespace bsp
