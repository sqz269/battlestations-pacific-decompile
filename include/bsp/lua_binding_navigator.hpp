// The mission Lua bindings that order the mission's ships.
//
// Packet cc_lua_navigator, worktree agent/cc-lua-navigator. Ghidra was read-only for this
// packet. Every name here is a hypothesis, not a recovered symbol.
//
// docs/GAME_EXECUTABLE.md milestone 2l runs usn_2_java's own order function and watches it
// address 21 of the 32 created instances through ten bindings that are all host records, so
// no order reaches a ship. Eight of those ten rows are this packet's subject:
//
//   00895250 SetSkillLevel (15 calls)      008a30d0 NavigatorAttackMove (6)
//   00899d10 JoinFormation (14)            008ad330 RepairEnable (6)
//   00897a50 SetInvincible (10, already    008ab850 SetRoleAvailable (4)
//            named by packet cc2_unit_     008a2f20 NavigatorMoveToRange (1)
//            damage; not redone here)      0089a8b0 SetFireTarget (6, packet cc2_weapons)
//
// Every one is `__fastcall(lua_State* in ECX)`, returns its result count in EAX, and shares
// the machine prologue/epilogue of docs/MISSION_LUA_MACHINE.md: a one-time `luakod` category
// string, BSP_LuaStateOwner_ConstructBorrowed (00B66C00), BSP_LuaObject_OpenCallFrame
// (00B679B0), indexed argument reads through 00B677E0, then BSP_LuaObject_ResultCount
// (00B66400) and BSP_LuaStateOwner_Close (00B669A0). This header models only what each
// binding does between them. All eight return whatever 00B66400 answers, which for a handler
// that pushes nothing is zero; none of the eight pushes a result.
//
// No struct, enum or k* constant declared here is declared by another header in include/bsp.
// The 0x18-byte command descriptor is `bsp::SceneCommandTarget` from
// include/bsp/scene_deferred_refs.hpp and the command-object addresses are the
// `kCommandObject*` constants of include/bsp/unit_commanded_speed.hpp; both are included and
// reused rather than restated.

#ifndef BSP_LUA_BINDING_NAVIGATOR_HPP
#define BSP_LUA_BINDING_NAVIGATOR_HPP

#include <cstdint>

#include "bsp/scene_deferred_refs.hpp"
#include "bsp/unit_commanded_speed.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// What the navigator bindings actually issue
// ---------------------------------------------------------------------------
//
// 008A30D0 and 008A2F20 are the same routine with one constant changed. Each reads Lua
// argument 0 as the acting entity (00888AA0), reads Lua argument 1 into a SceneCommandTarget
// through 0088A810, and calls BSP_Entity_IssueCommand (0077D600) with a fixed command object
// and the constant flags 1. Neither reads a third argument: 008A30D0's call frame touches
// indices 0 and 1 only (008A31AD, 008A31E6) and so does 008A2F20's (008A2FFD, 008A3036).
//
// Correction to the packet brief's expectation: `NavigatorMoveToRange` takes no range. The
// Lua name promises one and the binding has no argument for it; the range, if any, belongs to
// the `moveto` command class, not to the call.
//
// 008A2BC0 `NavigatorMoveToPos` and 008A2D70 `NavigatorDirectMoveToRange` are the same
// routine again, with the same command object as 008A2F20 (PUSH 0xe08f68 at 008A2D10 and
// 008A2EC0). The three differ only in the length passed to the dead error-prefix string
// (0x20 at 008A2BE5, 0x28 at 008A2D95, 0x22 at 008A2F45) and in the one-time `luakod` guard
// global. At the native level the three Lua names are one behaviour.
inline constexpr std::uint8_t kNavigatorIssueFlags = 1;  // 008A323E, 008A3069

// The two command objects the four navigator bindings name, as bsp::kCommandObject* values:
//   attackmove  kCommandObjectAttackMove  0x00E08F78, registry ordinal 17, category 2
//   moveto      0x00E08F68,               registry ordinal 15, category 3
// docs/SCENE_COMMAND_TYPES.md's table of 26. `moveto` has no kCommandObject* constant yet, so
// this packet adds one under the same naming rule.
inline constexpr std::uint32_t kCommandObjectMoveTo = 0x00e08f68u;  // 008A3070, 008A2D10, 008A2EC0

// ---------------------------------------------------------------------------
// 0088A810: the Lua-to-SceneCommandTarget reader
// ---------------------------------------------------------------------------
//
// `__fastcall(ECX = SceneCommandTarget* out, EDX = const LuaObject*)`, body 0088A810-0088A8F3,
// RET 0 (no stack arguments), returns the out pointer in EAX. 17 callers, all mission Lua
// bindings that take a target.
//
// It asks the Lua value for the field named by the literal at 00CE59B4, whose bytes are
// 49 44 00 = "ID" (00B67910 at 0088A83B), and tests it with 00B65FB0 at 0088A84A:
//
//   ID present  -> 00888AA0 resolves the value's `Ptr` to an entity (0088A855). The position
//                  becomes the zero vector at 00F87574 (0088A85C..0088A87E), `object` the
//                  resolved pointer (0088A886). When the pointer is non-null, `kind` becomes
//                  1 and `object_id` the uint16 at entity+174h (0088A88B..0088A895). When it
//                  is null the record falls through to the shared tail and stays kind 0 with
//                  object_id 0 and position_valid 0 -- a target that resolves to nothing
//                  becomes the origin, not an error.
//   ID absent   -> 00888760 reads the value as a Vector3 into the position (0088A8A1..
//                  0088A8BB), `position_valid` becomes 1 (0088A8A9), `object` 0 (0088A8BE).
//
// Either way the tail at 0088A8C1 clears `object_id` and `kind`, and 0088A8C7 clears the
// +14h reserved float with XORPS. `position_valid` is the only field the two branches leave
// different, which is what makes scene_deferred_refs.hpp's provisional reading of +1h hold
// here too: 1 means "the position fields were authored", 0 means "they are filler".
//
// One method per native call site 0088A810 makes. `index` follows
// mission_binding_argument_slot: index 0 is Lua slot 1.
class LuaCommandTargetSource {
public:
    virtual ~LuaCommandTargetSource() = default;
    // 00B67910 at 0088A83B fetching "ID", then 00B65FB0 at 0088A84A.
    virtual bool argument_id_field_is_nil(int index) = 0;
    // 00888AA0 at 0088A855. Null when the table carries no usable `Ptr`.
    virtual void* argument_entity(int index) = 0;
    // 00888760 at 0088A8A1. False when the value is not a readable Vector3, in which case the
    // native still copies whatever 00888760 left in its output buffer; the reconstruction
    // leaves the position at zero instead of reproducing uninitialised stack.
    virtual bool argument_vector3(int index, float out[3]) = 0;
    // The uint16 at entity+174h, read at 0088A88E. Declared here rather than taken from the
    // navigator host because 0088A810 has 17 callers and is not a navigator routine.
    virtual std::uint16_t entity_object_id(void* entity) = 0;
};

// 0088A810 as a sequence over that source. Pure apart from the four host reads.
SceneCommandTarget lua_read_command_target(LuaCommandTargetSource& source, int index);

// ---------------------------------------------------------------------------
// The dead error-prefix prologue
// ---------------------------------------------------------------------------
//
// Each of the eight opens by building a std::string of a fixed length through
// BSP_NativeString_Resize (0041DD40), memcpy-ing a `luaMW_<name> failed:` literal into it
// (00BF7680), and immediately returning the block to the pool (00419CC0 + 00BD1510). The
// length is exactly strlen of that literal: 0x21 for "luaMW_NavigatorAttackMove failed:",
// 0x22 for "luaMW_NavigatorMoveToRange failed:", 0x28 for the DirectMoveToRange spelling,
// 0x1B for JoinFormation and SetSkillLevel, 0x1E for SetRoleAvailable, 0x1A for
// RepairEnable. The string never escapes; it is the
// residue of an inlined error-report helper whose reporting arm the optimiser removed.
//
// Correction, decompiler artifact: Ghidra reports "Removing unreachable block" for the memcpy
// arm of 008A30D0 (008A3110), 008A2F20, 008A2BC0, 008A2D70 and 00899D10. Those blocks are
// reachable. The caller zeroes the string object's two words before the call
// (008A30FB/008A30FF), Ghidra's prototype for 0041DD40 does not model the write through ECX,
// so it constant-folds the following `CMP EDI,EBP` to always-equal. 008AD330 has the same
// idiom with a different stack layout and decompiles with the memcpy visible, which is the
// cross-check. Nothing in the block affects the binding's result, so no routine here models
// it; it is recorded so a later reader does not treat the pseudocode as complete.

// ---------------------------------------------------------------------------
// 0077C8D0: the formation join, and which ship becomes the follower
// ---------------------------------------------------------------------------
//
// `__thiscall(entity /*ECX*/, void* other)`, body 0077C8D0-0077C97B, RET 4. Four callers:
// 00816E30 BSP_UnitInstance_ApplyEntityCommand, 00899D10 JoinFormation, 009483D0, 00A10DC0.
//
// JoinFormation passes the entity of Lua argument 0 in ECX (00899E59, ESI from 00899E10) and
// the entity of Lua argument 1 on the stack (00899E58, EDI from 00899E41). 0077C8D0 then asks
// `entity->vtable[16Ch]("follow", other)` (0077C8F2..0077C8FE; the literal 0x00CFB52C is the
// bytes 66 6f 6c 6c 6f 77 00, "follow", which sits immediately after the `follow` command
// class's vtable 00CFB518). So the question asked is "may *this* entity follow *other*", and
// the answer settles the direction: **Lua argument 0 is the follower, Lua argument 1 is the
// leader.** The message built below carries argument 1's object id as its only entity field,
// which is the same reading from the other side.
//
// When the predicate answers false the routine does nothing at all (0077C902 jumps to the
// epilogue). When it answers true:
//   - entity+1ACh is read and, when it is 0..7 unsigned (0077C90A CMP 7 / JA), passed to
//     00905300 with ECX = [00E188A8]+21A0h (0077C910..0077C91B).
//   - a session message of type 76h is built by BSP_SessionMessage_ConstructBase (0075B430 at
//     0077C926) and given vtable 00D02D30 at +0h, 1 at +4h, 0 at +18h/+1Ah/+1Ch and the
//     leader's uint16 id at +20h (0077C92B..0077C951). +0h/+4h/+18h match
//     include/bsp/entity_orders.hpp's message base; +20h is this type's own field.
//   - BSP_Session_RouteMessage (0077C2A0) routes it with this = the follower, route-flags
//     override 7 and out 0 (0077C956..0077C964).
//
// The binding therefore never issues a `follow` command itself. Vtable slot 16Ch is a
// predicate: for MDestroyer (vtable 00CFC3D0, confirmed because +160h there is 00816E30) it
// is 008162B0, whose body 008162B0-00816408 returns bool and dispatches only IsKindOf and
// name-comparison helpers. What turns the accepted request into an order is the delivery of
// the type-76h message, which this packet did not read.
inline constexpr std::uint8_t kFormationJoinMessageType = 0x76;      // 0077C920
inline constexpr std::uint32_t kFormationJoinMessageVtable = 0x00d02d30u;  // 0077C949
inline constexpr int kFormationJoinRouteFlags = 7;                   // 0077C957
inline constexpr std::size_t kFormationJoinMessageLeaderIdOffset = 0x20;  // 0077C951
inline constexpr const char kFormationFollowCommandName[] = "follow";     // 00CFB52C
inline constexpr std::size_t kUnitVtableSlotCommandIsAvailable = 0x16c;   // 0077C8F2
// entity+1ACh, the index 00905300 counts against. Bounded to 0..7 by 0077C90A, which is the
// player-slot range elsewhere in the corpus; the reading of the field is provisional.
inline constexpr std::size_t kEntityOffRouteSlot = 0x1ac;  // 0077C904
inline constexpr int kEntityRouteSlotMax = 7;              // 0077C90A

// 00905300, body 00905300-00905319, `__thiscall(base /*ECX*/, int slot)`, RET 4: adds 1 to
// the dword at base + slot*284h + 170h and returns its address. What the counter means was
// not read; only the arithmetic is established here.
inline constexpr std::size_t kSlotCounterStride = 0x284;  // 00905304
inline constexpr std::size_t kSlotCounterOffset = 0x170;  // 0090530A

// ---------------------------------------------------------------------------
// The three small bindings
// ---------------------------------------------------------------------------
//
// 00895250 SetSkillLevel, body 00895250-008953EF. Reads Lua argument *1* first as an integer
// through 00B66290 (00895351), then Lua argument 0 as the entity (00895381), then calls
// `entity->vtable[128h](level)` with this = the entity (00895398..008953A2). No clamping, no
// range test, no message: the value the script passed reaches the virtual unchanged.
inline constexpr std::size_t kUnitVtableSlotSetSkillLevel = 0x128;  // 0089539A

// 008AD330 RepairEnable, body 008AD330-008AD534. Reads Lua argument 0 as the entity
// (008AD42E), asks `entity->vtable[5Ch](6)` (IsKindOf; vtable read 008AD448, PUSH 6 at
// 008AD44B), reads Lua argument 1 as a boolean through 00B66250 (008AD46D), and then
// branches on the IsKindOf answer:
//   false -> writes the byte at entity+378h directly (008AD4E8).
//   true  -> builds a session message of type 9Fh (0075B430 at 008AD494) with vtable 00D03360
//            at +0h, 1 at +4h, 0 at +18h/+1Ah and the boolean at +1Ch, and routes it through
//            0077C2A0 with this = the entity, flags 7 and out 0 (008AD4CD).
// The local write and the message are exclusive arms, so a reconstruction that models only
// one of them is partial. What class id 6 means was not read; 008AD330 is the only evidence
// here and it only shows that the answer selects local write versus routed message.
inline constexpr std::size_t kEntityOffRepairEnabled = 0x378;         // 008AD4E8
inline constexpr int kRepairEnableIsKindOfClassId = 6;                // 008AD44B, PUSH 6
inline constexpr std::uint8_t kRepairEnableMessageType = 0x9f;        // 008AD48B
inline constexpr std::uint32_t kRepairEnableMessageVtable = 0x00d03360u;  // 008AD4AF
inline constexpr std::size_t kRepairEnableMessageValueOffset = 0x1c;  // 008AD4B7
inline constexpr int kRepairEnableRouteFlags = 7;                     // 008AD4BC

// 008AB850 SetRoleAvailable, body 008AB850-008ABABF. Three Lua arguments, and argument 0 is
// read with BSP_LuaTable_GetPtrField (00888D20 at 008AB953), not with 00888AA0: 00888D20
// fetches the table's `Ptr` and converts it without the entity-table validation 00888AA0
// applies, so a caller can pass any table that carries a `Ptr`. Arguments 1 and 2 are
// integers through 00B66290. Then:
//   session mode == 0        -> `owner->vtable[148h](role, value)` (008ABA51).
//   otherwise, mode < 4    -> message type 4Ch, vtable 00D02CA4, role at +1Ch and value at
//                             +20h, routed with this = owner, flags 5, out 0 (008ABA2F).
//   otherwise, mode >= 4   -> `owner->vtable[148h](role, value)` and, when value < 8,
//                             BSP_Game_AssignPartyPlayerSlots(0) (008ABA60), the second
//                             vtable[148h] call being 008ABA73.
// The mode comes from BSP_Game_GetEffectiveGameMode (004BCA50 at 008AB9DE) with
// this = [00E188A8], and the flag itself is read at 008AB9D1.
//
// Correction to this packet's first reading: [00E188A8+1FE4h] is the *session mode*, not a
// campaign flag. include/bsp/unit_damage.hpp, include/bsp/unit_order_record.hpp and
// include/bsp/unit_state_message.hpp already declare it that way (0 local, 2 client), so this
// header reuses that reading and restates no constant for it.
inline constexpr std::size_t kRoleOwnerVtableSlotSetRoleAvailable = 0x148;  // 008ABA47
inline constexpr int kSetRoleAvailableRoutedModeLimit = 4;                  // 008AB9E3
inline constexpr int kSetRoleAvailableLocalSessionMode = 0;                 // 008AB9D1
inline constexpr int kSetRoleAvailablePartyReassignLimit = 8;               // 008ABA57
inline constexpr std::uint8_t kSetRoleAvailableMessageType = 0x4c;          // 008AB9EC
inline constexpr std::uint32_t kSetRoleAvailableMessageVtable = 0x00d02ca4u;
inline constexpr std::size_t kSetRoleAvailableMessageRoleOffset = 0x1c;
inline constexpr std::size_t kSetRoleAvailableMessageValueOffset = 0x20;
inline constexpr int kSetRoleAvailableRouteFlags = 5;                       // 008ABA25

// Which arm a run took, so a probe can report the branch instead of guessing it.
enum class RepairEnableArm {
    kLocalFieldWrite,  // IsKindOf(6) false, 008AD4E8
    kRoutedMessage,    // IsKindOf(6) true, 008AD48B
};

enum class SetRoleAvailableArm {
    kDirectCall,          // session mode == 0, 008ABA51
    kRoutedMessage,       // networked, effective mode < 4, 008ABA2F
    kDirectCallWithParty, // networked, effective mode >= 4, 008ABA60
};

// Whether 0077C8D0 got past its predicate.
enum class FormationJoinOutcome {
    kRefused,   // vtable[16Ch] answered false, 0077C902
    kRequested, // counter bumped and the type-76h message routed
};

// ---------------------------------------------------------------------------
// The host: one pure-virtual method per native call site
// ---------------------------------------------------------------------------
//
// Nothing here stands in for unrecovered game behaviour. A host that cannot perform a step
// records it; the reconstruction never invents the effect.
class LuaBindingNavigatorHost {
public:
    virtual ~LuaBindingNavigatorHost() = default;

    // --- shared argument reads -------------------------------------------------
    // 00888AA0 at 008A31CF / 008A301F / 00899E10 / 00899E41 / 00895381 / 008AD42E: the entity
    // behind an entity table, null when the table is not one.
    virtual void* argument_entity(int index) = 0;
    // 00B66290 at 00895351 / 008AB984 / 008AB9B4: Lua integer, __ftol truncation.
    virtual int argument_integer(int index) = 0;
    // 00B66250 at 008AD46D: Lua boolean.
    virtual bool argument_boolean(int index) = 0;
    // 00888D20 at 008AB953: the table's `Ptr`, unvalidated.
    virtual void* argument_ptr_field(int index) = 0;

    // --- 008A30D0 / 008A2F20 / 008A2BC0 / 008A2D70 ---------------------------
    // 0077D600 at 008A3273 and 008A3077. __thiscall(entity, command object, descriptor,
    // flags); docs/ENTITY_ORDER_MESSAGE.md owns its body.
    virtual void entity_issue_command(void* entity, std::uint32_t command_object,
                                      const SceneCommandTarget& target, int flags) = 0;

    // --- 00899D10 through 0077C8D0 -------------------------------------------
    // entity->vtable[16Ch] at 0077C8F8, called with ("follow", other). For MDestroyer the
    // concrete callee is 008162B0, a bool-returning availability predicate.
    virtual bool entity_command_is_available(void* entity, const char* command_name,
                                             void* target) = 0;
    // The dword at entity+1ACh, read at 0077C904.
    virtual int entity_route_slot(void* entity) = 0;
    // 00905300 at 0077C91B, with ECX = [00E188A8]+21A0h.
    virtual void slot_counter_increment(int slot) = 0;
    // The uint16 at entity+174h, read at 0077C92B.
    virtual std::uint16_t entity_object_id(void* entity) = 0;
    // 0075B430(76h) at 0077C926 + 0077C2A0 at 0077C964.
    virtual void session_route_formation_message(void* follower,
                                                 std::uint16_t leader_object_id) = 0;

    // --- 00895250 -------------------------------------------------------------
    // entity->vtable[128h] at 0089539A.
    virtual void entity_set_skill_level(void* entity, int level) = 0;

    // --- 008AD330 -------------------------------------------------------------
    // entity->vtable[5Ch] at 008AD448, the IsKindOf the corpus uses everywhere; called with 6.
    virtual bool entity_is_kind_of(void* entity, int class_id) = 0;
    // The byte write at entity+378h, 008AD4E8.
    virtual void entity_set_repair_enabled_field(void* entity, bool enabled) = 0;
    // 0075B430(9Fh) at 008AD494 + 0077C2A0 at 008AD4CD.
    virtual void session_route_repair_enable_message(void* entity, bool enabled) = 0;

    // --- 008AB850 -------------------------------------------------------------
    // The session mode at [00E188A8]+1FE4h, read at 008AB9D1.
    virtual int game_session_mode() = 0;
    // 004BCA50 at 008AB9DE, this = [00E188A8].
    virtual int game_effective_game_mode() = 0;
    // owner->vtable[148h], read at 008ABA47/008ABA69 and called at 008ABA51/008ABA73.
    virtual void role_owner_set_role_available(void* owner, int role, int value) = 0;
    // 0075B430(4Ch) at 008AB9EE + 0077C2A0 at 008ABA2F.
    virtual void session_route_role_message(void* owner, int role, int value) = 0;
    // 004C3840 at 008ABA60, this = [00E188A8].
    virtual void game_assign_party_player_slots(int value) = 0;
};

// ---------------------------------------------------------------------------
// The routines
// ---------------------------------------------------------------------------
//
// Each returns the binding's Lua result count, which for all eight is 0: none pushes a value
// above its arguments, so 00B66400's `lua_gettop - base` is zero.

// 008A30D0 NavigatorAttackMove: issue kCommandObjectAttackMove.
int lua_binding_navigator_attack_move(LuaCommandTargetSource& targets,
                                      LuaBindingNavigatorHost& host);

// 008A2F20 NavigatorMoveToRange, and byte-identically 008A2BC0 NavigatorMoveToPos and
// 008A2D70 NavigatorDirectMoveToRange: issue kCommandObjectMoveTo.
int lua_binding_navigator_move_to(LuaCommandTargetSource& targets,
                                  LuaBindingNavigatorHost& host);

// 00899D10 JoinFormation: argument 0 follows argument 1.
int lua_binding_join_formation(LuaBindingNavigatorHost& host);

// 0077C8D0 on its own, so the unit-command path that also calls it can reuse the sequence.
FormationJoinOutcome entity_join_formation(LuaBindingNavigatorHost& host, void* follower,
                                           void* leader);

// 00895250 SetSkillLevel.
int lua_binding_set_skill_level(LuaBindingNavigatorHost& host);

// 008AD330 RepairEnable. `arm_out` reports which of the two exclusive arms ran.
int lua_binding_repair_enable(LuaBindingNavigatorHost& host, RepairEnableArm& arm_out);

// 008AB850 SetRoleAvailable. `arm_out` reports which of the three arms ran.
int lua_binding_set_role_available(LuaBindingNavigatorHost& host, SetRoleAvailableArm& arm_out);

}  // namespace bsp

#endif  // BSP_LUA_BINDING_NAVIGATOR_HPP
