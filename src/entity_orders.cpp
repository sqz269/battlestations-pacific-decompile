// Entity order issue and the MT_COMMAND session message.
// Evidence and coverage: docs/ENTITY_ORDER_MESSAGE.md, docs/SCENE_COMMAND_TYPES.md,
// reports/entity_orders.json. Names are hypotheses, not recovered symbols.

#include "bsp/entity_orders.hpp"

#include <cstring>

namespace bsp {
namespace {

char lower_ascii(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

// The compare 00438E10 performs for the scene corpus: case-insensitive, ASCII.
bool equals_ignore_case(const char* a, const std::string& b) noexcept {
    std::size_t i = 0;
    for (; i < b.size(); ++i) {
        if (a[i] == '\0' || lower_ascii(a[i]) != lower_ascii(b[i])) {
            return false;
        }
    }
    return a[i] == '\0';
}

}  // namespace

// Constructors 00CCE500 + 40h*k, k = 0..25, in CRT table order (00CE2BEC).
// ordinal = the pre-incremented counter 00E19A68; name = vtable[4]'s literal;
// category = vtable[0Ch]; requires_target = vtable[8].
const EntityOrderCommandClass kEntityOrderCommandClasses[kEntityOrderCommandCount] = {
    {1, 0x00E08EF8u, 0x00CFB3A4u, "settarget", 1, true},
    {2, 0x00E08F00u, 0x00CFB3B8u, "cleartarget", 0, false},
    {3, 0x00E08F08u, 0x00CFB3CCu, "clearorders", 0, false},
    {4, 0x00E08F10u, 0x00CFB3E0u, "artillery", 1, true},
    {5, 0x00E08F18u, 0x00CFB400u, "torpedo", 2, true},
    {6, 0x00E08F20u, 0x00CFB41Cu, "divebomb", 2, true},
    {7, 0x00E08F28u, 0x00CFB43Cu, "levelbomb", 2, true},
    {8, 0x00E08F30u, 0x00CFB45Cu, "dropkamikaze", 2, true},
    {9, 0x00E08F38u, 0x00CFB480u, "depthcharge", 2, true},
    {10, 0x00E08F40u, 0x00CFB4BCu, "strafe", 1, true},
    {11, 0x00E08F48u, 0x00CFB4A0u, "rocket", 2, true},
    {12, 0x00E08F50u, 0x00CFB4D8u, "kamikaze", 2, true},
    {13, 0x00E08F58u, 0x00CFB4F8u, "dogfight", 1, true},
    {14, 0x00E08F60u, 0x00CFB518u, "follow", 3, true},
    {15, 0x00E08F68u, 0x00CFB534u, "moveto", 3, true},
    {16, 0x00E08F70u, 0x00CFB554u, "cruise", 3, false},
    {17, 0x00E08F78u, 0x00CFB590u, "attackmove", 2, true},
    {18, 0x00E08F80u, 0x00CFB63Cu, "moveonpath", 3, false},
    {19, 0x00E08F88u, 0x00CFB5A4u, "stop", 3, false},
    {20, 0x00E08F90u, 0x00CFB5C0u, "retreat", 3, false},
    {21, 0x00E08F98u, 0x00CFB5DCu, "returntobase", 3, false},
    {22, 0x00E08FA0u, 0x00CFB600u, "land", 3, true},
    {23, 0x00E08FA8u, 0x00CFB61Cu, "closetoship", 3, true},
    {24, 0x00E08FB0u, 0x00CFB660u, "Leave", 0, false},
    {25, 0x00E08FB8u, 0x00CFB67Cu, "disband", 0, false},
    {26, 0x00E08FC0u, 0x00CFB570u, "tutorial", 3, false},
};

const EntityOrderCommandClass* entity_order_command_class_by_name(const std::string& name) noexcept {
    for (const EntityOrderCommandClass& row : kEntityOrderCommandClasses) {
        if (equals_ignore_case(row.name, name)) {
            return &row;
        }
    }
    return nullptr;
}

const EntityOrderCommandClass* entity_order_command_class_by_ordinal(int ordinal) noexcept {
    for (const EntityOrderCommandClass& row : kEntityOrderCommandClasses) {
        if (row.ordinal == ordinal) {
            return &row;
        }
    }
    return nullptr;
}

const EntityOrderCommandClass* entity_order_command_class_by_address(
    std::uint32_t address) noexcept {
    for (const EntityOrderCommandClass& row : kEntityOrderCommandClasses) {
        if (row.object_address == address) {
            return &row;
        }
    }
    return nullptr;
}

// 0077D67D..0077D6A9. The compare against 00F89A10 is signed; the index is then
// biased by that table's own base and scaled by the 16-byte entry stride.
EntityHandleSlot entity_handle_slot_00521ea0(std::uint16_t object_id,
                                             const EntityHandleTableBases& bases) noexcept {
    const std::int32_t id = static_cast<std::int32_t>(object_id);
    EntityHandleSlot slot{};
    slot.high_table = !(id < bases.split);
    const std::int32_t base = slot.high_table ? bases.high_base : bases.low_base;
    slot.byte_offset = static_cast<std::ptrdiff_t>(id - base) *
                           static_cast<std::ptrdiff_t>(kEntityHandleEntryStride) +
                       static_cast<std::ptrdiff_t>(kEntityHandleEntryObjectOffset);
    return slot;
}

bool entity_order_target_is_object(const SceneCommandTarget& target) noexcept {
    return target.kind != 0;  // 0077D652 CMP byte [ESP+18h],BL
}

bool entity_order_resolves_target_late(int command_ordinal, int attackmove_ordinal) noexcept {
    return command_ordinal == attackmove_ordinal;  // 0077D776
}

bool entity_order_ai_group_is_notified(const void* ai_group,
                                       const void* controller,
                                       const void* controller_owner,
                                       const void* entity) noexcept {
    if (ai_group == nullptr) {
        return false;  // 0077D78F
    }
    if (controller == nullptr) {
        return true;  // 0077D799
    }
    return controller_owner == entity;  // 0077D79B CMP [EAX+0x14],EDI
}

SceneCommandTarget entity_order_retarget_0077d6ed(const SceneCommandTarget& target,
                                                  const void* retarget_object,
                                                  std::uint16_t retarget_object_id) noexcept {
    SceneCommandTarget out = target;
    out.object = const_cast<void*>(retarget_object);
    out.kind = retarget_object != nullptr ? 1u : 0u;  // 0077D724..0077D741
    out.object_id = retarget_object != nullptr ? retarget_object_id : 0u;
    // 00F87574/78/7C is the read-only zero vector; the reserved float is XORPS'd
    // at 0077D751.
    out.position[0] = 0.0f;
    out.position[1] = 0.0f;
    out.position[2] = 0.0f;
    out.reserved = 0.0f;
    // position_valid (+1h) is not touched by the retarget; it keeps the value
    // the caller's descriptor had, which the builder then copies into +24h.
    return out;
}

std::uint32_t session_message_player_slot_0075b454(int slot_index,
                                                   const std::uint32_t players[8]) noexcept {
    if (slot_index < 0 || slot_index > kSessionWorldPlayerSlotMax) {
        return 0;  // 0075B476
    }
    return players[slot_index];  // 0075B469
}

EntityOrderMessage entity_order_build_message_007798d0(std::uint8_t command_ordinal,
                                                       const SceneCommandTarget& target,
                                                       std::uint8_t flags,
                                                       std::uint32_t player_slot) noexcept {
    EntityOrderMessage message{};
    // 0075B430 with the type byte 58h, then the builder's overwrites.
    message.vtable = kEntityOrderMessageVtableAddress;  // 007798E1
    message.kind = kEntityOrderMessageKind;             // 007798DA, over the base's 3
    message.reserved_08 = 0;
    message.send_tick = 0;
    message.type = kEntityOrderMessageType;
    message.player_slot = player_slot;
    message.sender_id = 0;   // 007798E9; the router fills it
    message.relay_flag = 0;  // 007798ED
    message.audit_flag = 0;  // 007798F0
    message.command_ordinal = command_ordinal;  // 0077992F, low byte of command+4h
    message.flags = flags;                      // 007798F3
    // 007798FE reads descriptor +0h as a word: the kind byte and the
    // position_valid byte travel together.
    message.target_kind = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(target.kind) |
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(target.position_valid) << 8));
    message.target_id = target.object_id;   // 00779905
    message.target_object = target.object;  // 0077990D
    message.position[0] = target.position[0];
    message.position[1] = target.position[1];
    message.position[2] = target.position[2];
    message.trailing = target.reserved;  // 00779929
    return message;
}

EntityOrderIssueResult entity_issue_command_0077d600(EntityOrderHost& host,
                                                     void* entity,
                                                     void* command,
                                                     const SceneCommandTarget& target,
                                                     int flags) {
    EntityOrderIssueResult result{};
    // 0077D623..0077D66A: the local copy. The caller's descriptor is never
    // written, and step 9 hands the caller's copy to the AI group.
    SceneCommandTarget local = target;

    if (entity_order_target_is_object(local)) {
        void* object = host.resolve_target_object(local);  // 0077D676 / 0077D6BD
        if (object != nullptr &&
            host.object_is_kind_of(object, kEntityOrderRetargetClassId)) {  // 0077D6CB
            host.note_command_name_string(host.command_name(command));      // 0077D6DD, 0077D6E4
            void* replacement = host.retarget_object(object);               // 0077D6FA
            const std::uint16_t replacement_id =
                replacement != nullptr ? host.object_id(replacement) : 0u;  // 0077D73A
            local = entity_order_retarget_0077d6ed(local, replacement, replacement_id);
            result.retarget = EntityOrderRetargetOutcome::kRetargeted;
        } else if (object == nullptr) {
            result.retarget = EntityOrderRetargetOutcome::kUnresolvedObject;
        } else {
            result.retarget = EntityOrderRetargetOutcome::kClassRefused;
        }
    }

    const int ordinal = host.command_ordinal(command);  // 0077D773
    result.late_resolve = entity_order_resolves_target_late(ordinal, host.attackmove_ordinal());
    if (result.late_resolve) {
        host.resolve_target_object(local);  // 0077D782, the cached pointer is the point
    }

    void* ai_group = host.entity_ai_group(entity);        // 0077D787
    void* controller = host.entity_controller(entity);    // 0077D791
    void* controller_owner =
        controller != nullptr ? host.controller_owner(controller) : nullptr;  // 0077D79B
    result.ai_group_notified =
        entity_order_ai_group_is_notified(ai_group, controller, controller_owner, entity);
    if (result.ai_group_notified) {
        host.ai_group_forward_command(ai_group, command, target);  // 0077D7A7, caller's descriptor
    }

    result.message = entity_order_build_message_007798d0(
        static_cast<std::uint8_t>(static_cast<std::uint32_t>(ordinal) & 0xFFu), local,
        static_cast<std::uint8_t>(static_cast<std::uint32_t>(flags) & 0xFFu),
        host.session_player_slot());  // 0077D7BE
    host.route_message(entity, result.message);  // 0077D7D3
    return result;
}

std::vector<const EntityOrderCommandClass*> entity_order_collect_by_category_006f9110(
    EntityOrderMenuHost& host, void* entity, int category) {
    std::vector<const EntityOrderCommandClass*> out;  // 006F911B clears it first
    for (const EntityOrderCommandClass& row : kEntityOrderCommandClasses) {
        if (!host.entity_is_kind_of(entity, kEntityOrderMenuClassId)) {
            continue;  // 006F9139, re-tested per node in the native
        }
        if (row.category != category) {
            continue;  // 006F9147
        }
        if (host.entity_accepts_command_name(entity, row.name)) {  // 006F915D, 006F9164
            out.push_back(&row);                                   // 006F9172
        }
    }
    return out;
}

void entity_order_menu_set_category_00523900(EntityOrderMenu& menu,
                                             EntityOrderMenuHost& host,
                                             void* player_unit,
                                             int category) {
    menu.category = category;  // 0052390D
    menu.entries.clear();      // 00523910
    if (player_unit == nullptr) {
        menu.cursor = nullptr;  // 00523934
        return;
    }
    menu.entries = entity_order_collect_by_category_006f9110(host, player_unit, menu.category);
    menu.cursor = menu.entries.empty() ? nullptr : menu.entries.front();  // 00523928
}

}  // namespace bsp
