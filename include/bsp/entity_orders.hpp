#pragma once
// Entity order issue and the MT_COMMAND session message.
//
// Subject: 0077D600 BSP_Entity_IssueCommand, the builder 007798D0, the handle
// resolver 00521EA0, the AI-group forward 00A2BD90, and the command-type
// registry at 00E19A6C with its collector 006F9110 and menu setter 00523900.
// Evidence, addresses and coverage: docs/ENTITY_ORDER_MESSAGE.md and
// docs/SCENE_COMMAND_TYPES.md; report reports/entity_orders.json.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The 26
// command-name strings and MT_COMMAND are literals in the image.
//
// No struct, enum or k* constant declared here is declared by another header in
// include/bsp. The 0x18-byte target descriptor is bsp::SceneCommandTarget from
// scene_deferred_refs.hpp and the AI-group offsets are from lua_binding_ai.hpp;
// both are reused, not redeclared.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/lua_binding_ai.hpp"
#include "bsp/scene_deferred_refs.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The command-type objects (00E08EF8..00E08FC7)
// ---------------------------------------------------------------------------

// Each object is eight bytes: a vtable pointer and the registration ordinal the
// constructor at 00CCE500+40h*k takes from the counter 00E19A68. Nothing else
// is stored in the object; the name, the category and the target predicate are
// constant-returning vtable bodies.
inline constexpr std::size_t kEntityOrderCommandVtableOffset = 0x00;
inline constexpr std::size_t kEntityOrderCommandOrdinalOffset = 0x04;
inline constexpr std::size_t kEntityOrderCommandSize = 0x08;

inline constexpr std::uint32_t kEntityOrderCommandFirstAddress = 0x00E08EF8u;
inline constexpr int kEntityOrderCommandCount = 26;

// vtable slots the packet establishes. 00CFB378 is the base table: +4h is
// 006F7F30 "undefined command name", +8h is 006F7F40 (false) and +0Ch is
// __purecall, so a registered object always overrides all three.
inline constexpr std::size_t kEntityOrderCommandNameSlot = 0x04;
inline constexpr std::size_t kEntityOrderCommandTargetPredicateSlot = 0x08;
inline constexpr std::size_t kEntityOrderCommandCategorySlot = 0x0C;

// One row of the registry, as the 26 constructors build it. `ordinal` is the
// value the constructor stored at object+4h and is the byte the MT_COMMAND
// message carries. `requires_target` is vtable[8]: what the binary proves is
// that a true answer drops a scene record that named no target (0046ABA6);
// "requires a named target" is the reading, and it is provisional.
struct EntityOrderCommandClass {
    int ordinal;
    std::uint32_t object_address;
    std::uint32_t vtable_address;
    const char* name;
    int category;
    bool requires_target;
};

// The table in registration order, which is also the registry walk order
// because 006F7D40 appends at the tail.
extern const EntityOrderCommandClass kEntityOrderCommandClasses[kEntityOrderCommandCount];

// Lookups over that table. `by_name` is case-insensitive, which is what
// 00438E10 does for the scene corpus; it returns nullptr when nothing matches.
const EntityOrderCommandClass* entity_order_command_class_by_name(const std::string& name) noexcept;
const EntityOrderCommandClass* entity_order_command_class_by_ordinal(int ordinal) noexcept;
const EntityOrderCommandClass* entity_order_command_class_by_address(std::uint32_t address) noexcept;

// The ordinal of `attackmove`, object 00E08F78. 00E08F7C is that object's
// ordinal field and is the value 0077D776 compares against.
inline constexpr int kEntityOrderAttackMoveOrdinal = 17;

// ---------------------------------------------------------------------------
// The registry container (00E19A6C) and its nodes
// ---------------------------------------------------------------------------

inline constexpr std::uint32_t kEntityOrderRegistryCounterAddress = 0x00E19A68u;
inline constexpr std::uint32_t kEntityOrderRegistryListAddress = 0x00E19A6Cu;
inline constexpr std::size_t kEntityOrderRegistryCountOffset = 0x00;
inline constexpr std::size_t kEntityOrderRegistryHeadOffset = 0x04;  // 00E19A70
inline constexpr std::size_t kEntityOrderRegistryTailOffset = 0x08;
inline constexpr std::size_t kEntityOrderRegistryNodePrevOffset = 0x00;
inline constexpr std::size_t kEntityOrderRegistryNodeNextOffset = 0x04;
inline constexpr std::size_t kEntityOrderRegistryNodeObjectOffset = 0x08;
inline constexpr std::size_t kEntityOrderRegistryNodeSize = 0x0C;

// ---------------------------------------------------------------------------
// The handle tables (00521EA0, inlined at 0077D676)
// ---------------------------------------------------------------------------

// The two 16-byte-entry tables the descriptor's uint16 id indexes, with the
// object pointer at +0Ch. Inputs are the four globals the native reads, passed
// explicitly so the rule is pure.
struct EntityHandleTableBases {
    std::int32_t split{0};       // 00F89A10, signed compare (JGE at 0077D683)
    std::int32_t low_base{0};    // 00F89A0C
    std::int32_t high_base{0};   // 00F89A60
};
inline constexpr std::size_t kEntityHandleEntryStride = 0x10;
inline constexpr std::size_t kEntityHandleEntryObjectOffset = 0x0C;

// Which table an id lands in and at what byte offset inside it. Returns false
// when the id is below the table's own base, which the native does not check;
// the caller sees an out-of-range index rather than a silently wrapped one.
struct EntityHandleSlot {
    bool high_table{false};   // false: 00F89A54, true: 00F89AA8
    std::ptrdiff_t byte_offset{0};
};
EntityHandleSlot entity_handle_slot_00521ea0(std::uint16_t object_id,
                                             const EntityHandleTableBases& bases) noexcept;

// ---------------------------------------------------------------------------
// Pure rules of 0077D600
// ---------------------------------------------------------------------------

// 0077D652/0077D670. Only an object target reaches the retarget block.
bool entity_order_target_is_object(const SceneCommandTarget& target) noexcept;

// 0077D776. The command whose ordinal equals attackmove's is the one that gets
// its target pointer resolved and cached before the message is built.
bool entity_order_resolves_target_late(int command_ordinal,
                                       int attackmove_ordinal =
                                           kEntityOrderAttackMoveOrdinal) noexcept;

// 0077D787..0077D7A3. `controller_owner` is *(void**)(entity+284h+14h) and is
// only read when the controller pointer is non-null.
bool entity_order_ai_group_is_notified(const void* ai_group,
                                       const void* controller,
                                       const void* controller_owner,
                                       const void* entity) noexcept;

// Entity offsets this routine touches. kEntityAiGroupOffset (+16Ch) comes from
// lua_binding_ai.hpp and is not redeclared.
inline constexpr std::size_t kEntityOrderControllerOffset = 0x284;   // 0077D791
inline constexpr std::size_t kEntityOrderControllerOwnerOffset = 0x14;  // 0077D79B
inline constexpr std::size_t kEntityOrderObjectIdOffset = 0x174;     // 0077D73A
inline constexpr std::size_t kEntityOrderRetargetOffset = 0x9D4;     // 0077D6FA
inline constexpr std::size_t kEntityOrderPeerListOffset = 0x2A4;     // 0077C3D3
inline constexpr std::size_t kEntityOrderAiGroupForwardSlot = 0x24;  // 00A2BD9C

// IsKindOf class ids this packet observes. 006FE530 is the body
// (docs/UNIT_INSTANCE_UPDATE.md); the names are the query sites, not classes.
inline constexpr int kEntityOrderRetargetClassId = 0x0F;   // 0077D6C9
inline constexpr int kEntityOrderMenuClassId = 0x02;       // 006F9135
inline constexpr int kEntityOrderFlagsClassId = 0x18;      // 005252D8

// 0077D6ED..0077D74B. Applied to the local copy only; the caller's descriptor
// is untouched and is what the AI group is handed.
SceneCommandTarget entity_order_retarget_0077d6ed(const SceneCommandTarget& target,
                                                  const void* retarget_object,
                                                  std::uint16_t retarget_object_id) noexcept;

// ---------------------------------------------------------------------------
// The MT_COMMAND message (0075B430 + 007798D0)
// ---------------------------------------------------------------------------

// Type byte at +10h. 00E0AB68[0x58] is the literal "MT_COMMAND" at 00D0267C.
inline constexpr std::uint8_t kEntityOrderMessageType = 0x58;
inline constexpr std::uint32_t kEntityOrderMessageVtableAddress = 0x00D03630u;
inline constexpr std::uint32_t kSessionMessageBaseVtableAddress = 0x00D02C68u;
inline constexpr int kSessionMessageBaseKind = 3;    // 0075B436
inline constexpr int kEntityOrderMessageKind = 1;    // 007798DA

inline constexpr std::size_t kEntityOrderMessageVtableOffset = 0x00;
inline constexpr std::size_t kEntityOrderMessageKindOffset = 0x04;
inline constexpr std::size_t kEntityOrderMessageReserved08Offset = 0x08;
inline constexpr std::size_t kEntityOrderMessageSendTickOffset = 0x0C;
inline constexpr std::size_t kEntityOrderMessageTypeOffset = 0x10;
inline constexpr std::size_t kEntityOrderMessagePlayerSlotOffset = 0x14;
inline constexpr std::size_t kEntityOrderMessageSenderIdOffset = 0x18;
inline constexpr std::size_t kEntityOrderMessageRelayFlagOffset = 0x1A;
inline constexpr std::size_t kEntityOrderMessageAuditFlagOffset = 0x1C;
inline constexpr std::size_t kEntityOrderMessageCommandOrdinalOffset = 0x20;
inline constexpr std::size_t kEntityOrderMessageFlagsOffset = 0x21;
inline constexpr std::size_t kEntityOrderMessageTargetKindOffset = 0x24;
inline constexpr std::size_t kEntityOrderMessageTargetIdOffset = 0x26;
inline constexpr std::size_t kEntityOrderMessageTargetObjectOffset = 0x28;
inline constexpr std::size_t kEntityOrderMessagePositionOffset = 0x2C;
inline constexpr std::size_t kEntityOrderMessageTrailingFloatOffset = 0x38;
inline constexpr std::size_t kEntityOrderMessageEnd = 0x3C;
// 0077D600 reserves [ESP+30h]..[ESP+5Fh] for it.
inline constexpr std::size_t kEntityOrderMessageStackReservation = 0x30;

// The player-slot rule of 0075B430: the world object at 00E188A8 holds the slot
// index at +18ECh and eight pointers at +18CCh. Out-of-range gives 0.
inline constexpr std::size_t kSessionWorldPlayerSlotOffset = 0x18EC;
inline constexpr std::size_t kSessionWorldPlayerArrayOffset = 0x18CC;
inline constexpr int kSessionWorldPlayerSlotMax = 7;
std::uint32_t session_message_player_slot_0075b454(int slot_index,
                                                   const std::uint32_t players[8]) noexcept;

// The record 007798D0 writes. Field order and sizes are fixed by the builder;
// the unwritten holes at +1Dh..+1Fh and +22h..+23h are not modelled.
struct EntityOrderMessage {
    std::uint32_t vtable{kEntityOrderMessageVtableAddress};
    std::int32_t kind{kEntityOrderMessageKind};
    std::uint32_t reserved_08{0};
    std::uint32_t send_tick{0};
    std::uint8_t type{kEntityOrderMessageType};
    std::uint32_t player_slot{0};
    std::uint16_t sender_id{0};   // written by the router, 0077C40E
    std::uint8_t relay_flag{0};   // written by the router, 0077C36B
    std::uint8_t audit_flag{0};   // written by the router, 0077C398
    std::uint8_t command_ordinal{0};
    std::uint8_t flags{0};
    std::uint16_t target_kind{0};  // descriptor +0h/+1h as one word
    std::uint16_t target_id{0};
    const void* target_object{nullptr};
    float position[3]{};
    float trailing{0.0f};
};

// 007798D0. `player_slot` is what 0075B430 already resolved; the builder does
// not touch it. `command_ordinal` is the low byte of command+4h.
EntityOrderMessage entity_order_build_message_007798d0(std::uint8_t command_ordinal,
                                                       const SceneCommandTarget& target,
                                                       std::uint8_t flags,
                                                       std::uint32_t player_slot) noexcept;

// ---------------------------------------------------------------------------
// 0077D600 as a sequence over a host
// ---------------------------------------------------------------------------

// What the routine did, for this reconstruction's own reporting. The native
// reports nothing: every early exit falls through to the message build.
enum class EntityOrderRetargetOutcome {
    kPositionTarget,      // kind == 0, 0077D670
    kUnresolvedObject,    // the handle lookup left null, 0077D6B3
    kClassRefused,        // IsKindOf(0Fh) was false, 0077D6CF
    kRetargeted,          // 0077D6ED..0077D74B ran
};

struct EntityOrderIssueResult {
    EntityOrderRetargetOutcome retarget{EntityOrderRetargetOutcome::kPositionTarget};
    bool ai_group_notified{false};
    bool late_resolve{false};  // the attackmove gate at 0077D776
    EntityOrderMessage message{};
};

// One virtual per native call site inside 0077D600, in body order. There are no
// default implementations: nothing here stands in for unrecovered behaviour.
struct EntityOrderHost {
    virtual ~EntityOrderHost() = default;
    // 00521EA0 / the inline copy at 0077D676: resolve and cache the descriptor's
    // object pointer from its id. Returns null when the tables have no entry.
    virtual void* resolve_target_object(SceneCommandTarget& target) = 0;
    // Indirect vtable[5Ch] = 006FE530 on the resolved target, 0077D6CB.
    virtual bool object_is_kind_of(void* object, int class_id) = 0;
    // Indirect vtable[4] on the command, 0077D6DD.
    virtual const char* command_name(void* command) = 0;
    // 0041E870 then 00419CC0/00BD1510: the pooled string built from that name
    // and freed again at 0077D767 without being read. Modelled because the
    // allocation happens, not because a value comes back.
    virtual void note_command_name_string(const char* name) = 0;
    // *(void**)(object + 9D4h), 0077D6FA.
    virtual void* retarget_object(void* object) = 0;
    // *(std::uint16_t*)(object + 174h), 0077D73A.
    virtual std::uint16_t object_id(void* object) = 0;
    // *(int*)(command + 4h), 0077D773.
    virtual int command_ordinal(void* command) = 0;
    // [00E08F7C], the attackmove object's ordinal field, 0077D776.
    virtual int attackmove_ordinal() = 0;
    // entity+16Ch, entity+284h and *(void**)(entity+284h+14h), 0077D787..0077D79B.
    virtual void* entity_ai_group(void* entity) = 0;
    virtual void* entity_controller(void* entity) = 0;
    virtual void* controller_owner(void* controller) = 0;
    // 00A2BD90, this = the AI group, with the caller's descriptor.
    virtual void ai_group_forward_command(void* ai_group, void* command,
                                          const SceneCommandTarget& target) = 0;
    // The player slot 0075B430 reads out of the world object at 00E188A8.
    virtual std::uint32_t session_player_slot() = 0;
    // 0077C2A0, this = the entity, with the routing-flag override 0.
    virtual void route_message(void* entity, const EntityOrderMessage& message) = 0;
};

// 0077D600 BSP_Entity_IssueCommand. Native ECX = entity, three stack arguments,
// RET 0Ch, no return value; the result type exists for reporting only. `flags`
// is `int` in the native signature but only its low byte survives, at message
// +21h. Every call site that passes a constant passes 1.
EntityOrderIssueResult entity_issue_command_0077d600(EntityOrderHost& host,
                                                     void* entity,
                                                     void* command,
                                                     const SceneCommandTarget& target,
                                                     int flags);

// ---------------------------------------------------------------------------
// The command menu (006F9110, 00523900)
// ---------------------------------------------------------------------------

// Host for 006F9110. `entity_accepts_command_name` is the entity's vtable[168h]
// at 006F9164; its body was not read, so the method carries the name only.
struct EntityOrderMenuHost {
    virtual ~EntityOrderMenuHost() = default;
    virtual bool entity_is_kind_of(void* entity, int class_id) = 0;  // 006F9139
    virtual bool entity_accepts_command_name(void* entity, const char* name) = 0;  // 006F9164
};

// 006F9110: the command classes of one category that this entity accepts, in
// registry order. The IsKindOf(2) test is inside the loop in the native and
// does not depend on the node, so a false answer yields an empty result.
std::vector<const EntityOrderCommandClass*> entity_order_collect_by_category_006f9110(
    EntityOrderMenuHost& host, void* entity, int category);

// 00523900. `player_unit` is [00E188D8], the player-controlled unit
// (docs/UNIT_INSTANCE_UPDATE.md); null clears the cursor and the list.
struct EntityOrderMenu {
    int category{0};                                              // +2Ch
    std::vector<const EntityOrderCommandClass*> entries;          // +3Ch list
    const EntityOrderCommandClass* cursor{nullptr};               // +48h
};
inline constexpr std::size_t kEntityOrderMenuCategoryOffset = 0x2C;
inline constexpr std::size_t kEntityOrderMenuListOffset = 0x3C;
inline constexpr std::size_t kEntityOrderMenuHeadOffset = 0x40;
inline constexpr std::size_t kEntityOrderMenuCursorOffset = 0x48;

void entity_order_menu_set_category_00523900(EntityOrderMenu& menu,
                                             EntityOrderMenuHost& host,
                                             void* player_unit,
                                             int category);

}  // namespace bsp
