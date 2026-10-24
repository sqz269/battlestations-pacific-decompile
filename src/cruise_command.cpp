// The authored `Cruise` order, from the scene command record to the ordered
// throttle and steering the ship motion runs on.
//
// Evidence, addresses, original ABI, coverage and the unread boundaries:
// docs/CRUISE_COMMAND.md and reports/cruise_command.json. Every descriptive
// name is a hypothesis, not a recovered symbol.

#include "bsp/cruise_command.hpp"

#include <cmath>

namespace bsp {
namespace {

// 00835AC6..00835ADC builds the absolute value as `(r > 0.0f) ? r : (-0.0f - r)`
// with the -0.0f at 00D7A208. That is fabsf for every ordered input; a NaN takes
// the else arm and stays a NaN, which the following COMISS then treats as
// unordered, exactly as the native does.
float native_abs_00835ac6(float value) noexcept {
    return (value > 0.0f) ? value : (-0.0f - value);
}

// 009E1297 makes the body-axis speed positive by clearing the sign bit
// (AND 7FFFFFFFh) rather than by a compare, so the NaN case differs from the
// one above: the sign is cleared and the value stays a NaN.
float native_clear_sign_009e1297(float value) noexcept {
    return std::fabs(value);
}

bool is_weapon_category(int category) noexcept {
    // 0071E55F/0071E564, 00835EE1/00835EE6, 008358FB/00835900, 0071E7AA/0071E7AF:
    // every site in this packet tests the same two ids, never a range.
    return category == 1 || category == 2;
}

// 00835EC6 and 00835ECE compare the top slot's command against two objects by
// address: `land` and `follow` of the registry table in entity_orders.hpp.
constexpr std::uint32_t kLandCommandObjectAddress = 0x00e08fa0u;
constexpr std::uint32_t kFollowCommandObjectAddress = 0x00e08f60u;

} // namespace

// ---------------------------------------------------------------------------
// The command class and its two producers
// ---------------------------------------------------------------------------

// The descriptor both producers build. 0046ABE0..0046ABF8 in the scene queue and
// 008A7620..008A763C in luaMW_NavigatorCruise write the same thing: kind 0, no
// id, no object, the read-only zero vector at 00F87574 and a zero trailer.
SceneCommandTarget cruise_command_empty_target() noexcept {
    return SceneCommandTarget{};
}

// 00764D00, __stdcall(int category), RET 4, no Ghidra function
// (00764D00-00764D1D, decoded from raw bytes). Three equality tests, then 0.
bool entity_command_message_is_category_00764d00(int category) noexcept {
    return category == 0x58 || category == 0x49 || category == 0x46;
}

// 0071C900, __stdcall(int category), RET 4, no Ghidra function
// (0071C900-0071C922, decoded from raw bytes). Four equality tests, then 0.
bool gameunit_set_command_message_is_category_0071c900(int category) noexcept {
    return category == 0x5c || category == 0x59 || category == 0x49 || category == 0x46;
}

// 0071BE40, __fastcall(director), RET 0, no Ghidra function
// (0071BE40-0071BE5A, decoded from raw bytes).
std::uint32_t director_current_command_0071be40(CruiseCommandMode mode,
                                                std::uint32_t slot0_command,
                                                std::uint32_t override_command) noexcept {
    if (mode == CruiseCommandMode::QueuedSlots) {
        return slot0_command;
    }
    if (mode == CruiseCommandMode::Override) {
        return override_command;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// The cruise autopilot rules
// ---------------------------------------------------------------------------

// 00835AC0, __thiscall(director)(float rudder, float heading, float thrust),
// RET 0Ch. Coverage: complete, 00835AC0-00835B35.
CruiseAutopilotFields cruise_latch_00835ac0(float ordered_rudder,
                                            float heading_radians,
                                            float ordered_throttle) noexcept {
    CruiseAutopilotFields fields{};
    // 00835AE0/00835AE8: COMISS 0.01f against |rudder|, JBE clears the flag, so
    // the flag is set only on a strict `0.01f > |rudder|` and an unordered
    // compare clears it.
    fields.is_heading = kCruiseHeadingRudderEpsilon > native_abs_00835ac6(ordered_rudder);
    // 00835B06 stores the heading argument, 00835B25 the rudder argument; both
    // arms then store the thrust argument at +248h.
    fields.steer_or_heading = fields.is_heading ? heading_radians : ordered_rudder;
    fields.thrust = ordered_throttle;
    return fields;
}

// 00835E17..00835E58. The three arguments are the unit's live pair at the ring's
// +148h/+14Ch (unit+980h/+984h) and the heading unit->vtable[50h] returns.
CruiseAutopilotFields cruise_command_begin_00835e17(const UnitOrderRing& ring,
                                                    float heading_radians) noexcept {
    return cruise_latch_00835ac0(ring.current_param_b, heading_radians, ring.current_param_a);
}

// 009E1265..009E13B1, the AI arm of 009E1170. Coverage: complete for that arm.
CruiseOrderedValues cruise_ordered_values_009e1170(const CruiseAutopilotFields& fields,
                                                   const CruiseSpeedSetting& speed_setting,
                                                   float reference_speed,
                                                   float body_axis_speed) noexcept {
    CruiseOrderedValues out{};
    out.throttle = fields.thrust;                       // 009E126B, 008356F0
    // 009E12AC: COMISS enable against the zero at 00D7A218, JC skips the block,
    // so the override runs on `enable >= 0.0f` and an unordered compare skips it.
    if (speed_setting.enable >= 0.0f) {
        // 009E12BD..009E12D7: FDIVR of the commanded speed by 0080FC30's
        // reference speed, computed in double and stored back as a float.
        const double commanded = static_cast<double>(speed_setting.speed);
        out.throttle = static_cast<float>(commanded / static_cast<double>(reference_speed));
    }

    if (!fields.is_heading) {
        out.mode = CruiseSteerMode::Rudder;             // 009E1384
        out.steer_or_heading = fields.steer_or_heading;
        return out;
    }

    // 009E1329..009E134E. |throttle| against the double 0.05 at 00D7A270, then
    // |body-axis speed| against the 1.0f at 00D7A24C; either one takes the
    // heading arm.
    const double abs_throttle = static_cast<double>(native_clear_sign_009e1297(out.throttle));
    const float abs_speed = native_clear_sign_009e1297(body_axis_speed);
    if (abs_throttle > kCruiseHeadingThrustEpsilon || abs_speed > kCruiseHeadingSpeedEpsilon) {
        out.mode = CruiseSteerMode::Heading;            // 009E1354
        out.steer_or_heading = fields.steer_or_heading;
        return out;
    }
    out.mode = CruiseSteerMode::Straight;               // 009E1350, FLDZ
    out.steer_or_heading = 0.0f;
    return out;
}

// 0071E550, __thiscall(director)(command, target), RET 8. Coverage: complete,
// 0071E550-0071E5BA. `slot_count` is the index the scan at 0071E56B..0071E57E
// stops on, and `top_slot_category` is the category of slot_count-1's command.
bool cruise_make_room_0071e550(int incoming_category,
                               int slot_count,
                               int top_slot_category) noexcept {
    if (!is_weapon_category(incoming_category)) {
        return false;
    }
    if (slot_count <= 0) {
        return false;
    }
    return is_weapon_category(top_slot_category);
}

// 00835E90, __thiscall(director)(command), RET 4. No Ghidra function
// (00835E90-00835F02, decoded from raw bytes). Coverage: complete. `base_allows`
// is 0071C0C0's answer, read separately: false for a null command, otherwise
// "the first empty slot index is below 10".
bool cruise_command_accepted_00835e90(bool base_allows,
                                      int slot_count,
                                      std::uint32_t top_slot_command,
                                      int top_slot_category,
                                      int incoming_category) noexcept {
    if (!base_allows) {
        return false;                                   // 00835EA0
    }
    if (slot_count <= 0) {
        return true;                                    // 00835EB2
    }
    const int top_index = slot_count - 1;
    if ((top_slot_command == kLandCommandObjectAddress ||
         top_slot_command == kFollowCommandObjectAddress) &&
        top_index > 0) {
        return false;                                   // 00835ED6/00835ED8
    }
    if (!is_weapon_category(top_slot_category)) {
        return true;                                    // 00835EE9
    }
    // 00835EF4/00835EF9: a weapon command on top accepts only another weapon
    // command; anything else is refused.
    return is_weapon_category(incoming_category);
}

// ---------------------------------------------------------------------------
// The sequences
// ---------------------------------------------------------------------------

// 00816E30, __thiscall(unit)(message), RET 4 at 00817374.
// Coverage: partial. Projected here is the movement fall-through, which is what
// `cruise` (00E08F70), `stop` (00E08F88) and `moveonpath` (00E08F80) take: the
// test at 00816F5E..00816F7A is false for them, so 00816F7C..00817330 does not
// run and control reaches the null check at 00817330 and the tail at 00817334.
namespace {

// 00817330..0081735D, the tail every arm that does not return early reaches.
void entity_command_tail_00817330(CruiseCommandHost& host, std::uint32_t command,
                                  std::uint8_t flags, const SceneCommandTarget& target) {
    if (command == 0) {
        return;                                                             // 00817330
    }
    if (flags != 0) {
        host.clear_all_commands();                                          // 0081733E
    } else if (!host.command_accepted(command)) {
        return;                                                             // 0081734B/0081734F
    }
    host.director_issue_command(command, target);                           // 0081735D
}

} // namespace

void unit_apply_entity_command_00816e30(CruiseCommandHost& host,
                                        const EntityCommandMessageView& message) noexcept {
    // 00816E52..00816E96 copy the seven descriptor fields out of the message
    // into a local SceneCommandTarget; the view already carries that copy.
    const std::uint32_t command =
        host.command_object_from_message_ordinal(message.command_ordinal);  // 00816E9C
    entity_command_tail_00817330(host, command, message.flags, message.target);
}

// 0071ECF0, __thiscall(director)(command, target), RET 8 at 0071ED93.
// Coverage: complete, 0071ECF0-0071ED9A.
void director_issue_command_0071ecf0(CruiseCommandHost& host,
                                     std::uint32_t command,
                                     const SceneCommandTarget& target) noexcept {
    const std::uint32_t entity = host.endpoint_subject_vtable140();         // 0071ED19
    if (entity != 0 && host.entity_is_kind_of(entity, 2)) {                 // 0071ED32
        const std::uint32_t ai_group = host.entity_ai_group(entity);        // 0071ED38
        if (ai_group != 0 && !host.entity_controller_is_another_entity(entity)) {  // 0071ED43
            host.ai_group_forward_command(ai_group, command, target);       // 0071ED54
        }
    }
    host.make_room_for_command(command, target);                            // 0071ED62
    // 0071ED6C builds MT_GAMEUNIT_SETCMD with the flag byte 1 and 0071ED81
    // routes it with routing flags 7 through the endpoint at director+34h.
    host.route_set_command_message(command, target, 1);                     // 0071ED6C
}

// 00721A40's 5Ch arm, 00721AEE..00721B87, RET 4.
// Coverage: partial. The 5Ah, 5Bh, 5Dh, 5Eh, 5Fh and 60h arms of the same
// routine are read in the doc but not projected here.
void gameunit_apply_set_command_00721a40(CruiseCommandHost& host,
                                         int session_mode,
                                         std::uint8_t message_flag) noexcept {
    if (!host.message_is_category(kGameUnitSetCommandMessageType)) {        // 00721AEE
        return;
    }
    if (session_mode == 2) {                                                // 00721B01
        const SceneCommandTarget probe = host.message_target_descriptor();  // 00721B11
        if (probe.kind != 0) {                                              // 00721B16
            const SceneCommandTarget again = host.message_target_descriptor();  // 00721B22
            if (host.resolve_target_object(again) == 0) {                   // 00721B29
                return;                                                     // 00721B30
            }
        }
    }
    if (message_flag != 0) {                                                // 00721B36
        SceneCommandTarget target = host.message_target_descriptor();       // 00721B47
        const std::uint32_t command = host.message_command_object();        // 00721B4F
        host.director_set_command(command, target);                         // 00721B5A
        return;
    }
    const SceneCommandTarget target = host.message_target_descriptor();     // 00721B6C
    const std::uint32_t command = host.message_command_object();            // 00721B74
    host.director_queue_command(command, target);                           // 00721B7C
}

// 008358D0, __thiscall(director)(command, target), RET 8 at 0083593A.
// Coverage: complete, 008358D0-0083593C.
bool director_set_command_008358d0(CruiseCommandHost& host,
                                   std::uint32_t command,
                                   SceneCommandTarget& target,
                                   int session_mode) noexcept {
    if (!director_push_command_slot_0071e6c0(host, command, target)) {      // 008358DF
        return false;                                                       // 008358EB
    }
    if (command != 0 && is_weapon_category(host.command_category(command))) {  // 008358F9
        if (host.resolve_target_object(target) != 0 &&                      // 00835907
            (session_mode == 0 || session_mode == 1)) {                     // 0083591B/00835920
            // 00835924 pushes the force flag 1 before 00835928 resolves the
            // object again, so the setter is always called forced here.
            host.set_fire_target(host.resolve_target_object(target));       // 00835928/00835930
        }
    }
    return true;                                                            // 00835937
}

// 0071E6C0, __thiscall(director)(command, target), RET 8 at 0071E7DF.
// Coverage: complete, 0071E6C0-0071E7E1. Three callees enter as host methods
// with unread bodies: 0071D6D0, 00836040 and 006E38E0.
bool director_push_command_slot_0071e6c0(CruiseCommandHost& host,
                                         std::uint32_t command,
                                         SceneCommandTarget& target) noexcept {
    if (host.command_count() >= kDirectorCommandSlotCount) {                // 0071E6C3/0071E6C8
        return false;
    }
    // 0071E6D6..0071E6EE: walk the ten slots from director+54h with stride 1Ch
    // and stop at the first whose command pointer is null.
    int index = 0;
    while (index < kDirectorCommandSlotCount && host.slot_command(index) != 0) {
        ++index;
    }
    if (index > 0 && host.slot_target_matches(index - 1, target) &&          // 0071E70C
        host.slot_command(index - 1) == command) {                           // 0071E721
        return false;                                                        // 0071E724
    }
    if (!host.command_allowed(command, target)) {                            // 0071E72A
        return false;
    }
    // 00836040 may rewrite `target` in place before the store below sees it.
    if (host.normalize_self_target(command, target) && index >= 1) {         // 0071E73C/0071E742
        return false;
    }
    host.store_slot(index, command, target);                                 // 0071E764/0071E76C
    if (host.resolve_target_object(target) != 0) {                           // 0071E773
        host.observe_target(host.resolve_target_object(target));             // 0071E781/0071E78A
    }
    if (host.command_mode() == CruiseCommandMode::None) {                    // 0071E78F
        host.set_command_mode(CruiseCommandMode::QueuedSlots);               // 0071E795
    }
    if (command != 0 && is_weapon_category(host.command_category(command)) &&  // 0071E7A8
        host.override_command() != 0 && host.session_field_f4h_is_0_or_1()) {  // 0071E7B4/0071E7C9
        host.echo_command();                                                 // 0071E7D4
    }
    return true;                                                             // 0071E7DC
}

// 00835E0E..00835E5D, the `cruise` arm of 00835C70.
// Coverage: partial. The rest of 00835C70 (the descriptor copy at
// 00835C8B..00835CE4, the reference releases, the `follow`/`attackmove` arms at
// 00835D5A..00835E0E and the tail at 00835E5D..00835E83) is not projected.
void cruise_command_begin_00835c70(CruiseCommandHost& host,
                                   const UnitOrderRing& ring) noexcept {
    host.raise_command_stage(1);                                             // 00835E12
    const float heading = host.unit_heading();                               // 00835E46
    host.store_cruise_fields(cruise_command_begin_00835e17(ring, heading));   // 00835E58
}

// 009E1170's AI arm, 009E1265..009E13B1, RET 4.
// Coverage: partial. The two earlier arms are read in the doc but not
// projected: 009E11A9..009E1262 (the latch-and-disable arm) and
// 009E11E8..009E1262 (the player-controlled arm that forwards the ring's
// confirmed pair at +15Ch/+160h).
CruiseOrderedValues cruise_state_step_009e1170(CruiseCommandHost& host) noexcept {
    const CruiseAutopilotFields fields = host.cruise_fields();               // 009E126B
    const float speed = host.body_axis_speed();                             // 009E1282
    const CruiseSpeedSetting setting = host.speed_setting();                // 009E12A7
    // 009E12CE only runs inside the override branch, so the reference speed is
    // not fetched when the speed setting is disabled.
    const float reference =
        (setting.enable >= 0.0f) ? host.reference_speed() : 0.0f;           // 009E12CE
    const CruiseOrderedValues out =
        cruise_ordered_values_009e1170(fields, setting, reference, speed);
    switch (out.mode) {
        case CruiseSteerMode::Heading:
            host.set_desired_heading(out.steer_or_heading);                 // 009E1367
            break;
        case CruiseSteerMode::Rudder:
            host.set_desired_steering(out.steer_or_heading);                // 009E1397
            break;
        case CruiseSteerMode::Straight:
            host.set_desired_steering(0.0f);                                // 009E1397
            break;
    }
    host.set_desired_throttle(out.throttle);                                // 009E13A6
    return out;
}

// ---------------------------------------------------------------------------
// 00816E30 BSP_UnitInstance_ApplyEntityCommand (380 insns), whole body. cc13-wb2.
// ABI: __thiscall(ECX = unit; const message*), RET 4 at every exit; EDI = unit,
// EBP = the command object, BL = message +21h flags; the director is unit+738h.
// The message's +20h ordinal resolves at 00816E9C and the descriptor at
// 00816E52..00816E96 is the view's target. Each arm compares the command object
// against a registry address; the ones that do not return reach the tail
// 00817330, sometimes with a substituted command.
// EH: handler 00C90CC8, one state (0 at 0081724D, -1 at 008172F4) covering the
// raw NavPoint block while 004E5980 constructs it: not reproduced (a throwing
// constructor leaks the block here).
// Uncertainty: unit, director, objects and the NavPoint are host handles; the
// message +24h..+38h copy is the caller's; not wired (the live hosts call the
// fall-through projection unit_apply_entity_command_00816e30).
// ---------------------------------------------------------------------------
namespace {

// 00816EB2..00816ED0 and 00816F04..00816F22: resolve, then keep the object
// only when its vtable[5Ch](2) answers.
std::uint32_t resolved_kind2_object(UnitEntityCommandHost& host, const SceneCommandTarget& target) {
    const std::uint32_t object = host.resolve_target_object(target);
    if (object != 0 && host.object_is_kind_of_vcall5c(object, 2)) {
        return object;
    }
    return 0;
}

} // namespace

void unit_apply_entity_command_whole_00816e30(UnitEntityCommandHost& host,
                                              const EntityCommandMessageView& message) {
    SceneCommandTarget target = message.target;
    std::uint32_t command = host.command_object_from_message_ordinal(message.command_ordinal);
    const std::uint8_t flags = message.flags;                               // 00816EA1 BL

    if (command == kEntityCommandObject00e08f60) {                          // 00816EA6
        host.unit_request_join_formation_0077c8d0(resolved_kind2_object(host, target));
        host.hud_ship_screen_trigger_ping_0064a8e0();                       // 00816EE2
        return;
    }
    if (command == kEntityCommandObject00e08fb0) {                          // 00816EFC
        host.unit_group_release_member_0077c980(resolved_kind2_object(host, target));
        return;
    }
    if (command == kEntityCommandObject00e08fb8) {                          // 00816F41
        host.unit_group_snapshot_members_0077ca60();
        return;
    }
    if (command == kCruiseCommandObjectAddress || command == kEntityCommandObject00e08f88
        || command == kEntityCommandObject00e08f80) {                       // 00816F65..00816F83
        entity_command_tail_00817330(host, command, flags, target);
        return;
    }
    if (command == kEntityCommandObject00e08f68) {                          // 00816F89
        // 00816F91: a position descriptor keeps the command; an object whose
        // 007AC9D0 path interface exists becomes 00E08F80. 007AC9D0 receives
        // the resolve result unchecked.
        if (target.kind != 0
            && host.entity_path_interface_007ac9d0(host.resolve_target_object(target)) != 0) {
            command = kEntityCommandObject00e08f80;                         // 00816FB4
        }
        entity_command_tail_00817330(host, command, flags, target);
        return;
    }
    if (command == kEntityCommandObject00e08fa0) {                          // 00816FBE
        if (!host.unit_is_kind_of_vcall5c(0xc)) {                           // 00816FCF
            command = kEntityCommandObject00e08f78;                         // 00816FD9
        }
        entity_command_tail_00817330(host, command, flags, target);
        return;
    }
    if (command == kEntityCommandObject00e08ef8) {                          // 00816FE3
        // 00816FEB pushes the force flag 1 before 00816FF1 resolves.
        host.director_set_fire_target_00835860(host.resolve_target_object(target), 1);
        return;
    }
    if (command == kEntityCommandObject00e08f00) {                          // 00817017
        host.director_store_float_40(kEntityCommandDirectorField40Value);   // 00817029 [00CE3854]
        const std::uint32_t current = host.director_current_command_vcall2c();   // 00817041
        if (current == 0 && flags == 0) {
            return;                                                         // 0081704B
        }
        // 0081705C..0081706A: a second vtable[2Ch] call; its non-null result is
        // the byte kept in the message argument slot.
        bool handled = host.director_current_command_vcall2c() != 0;
        host.director_set_fire_target_00835860(0, 1);                       // 00817072
        if (current != 0
            && host.resolve_target_object(host.director_override_target_18c()) == current) {
            host.director_raise_override_stage_0071d9e0(2);                 // 00817098
            handled = true;                                                 // 0081709D
        }
        const int filled = host.director_filled_slot_count_0071be60();      // 008170AB
        if (filled <= 0) {
            return;                                                         // 008170B4
        }
        const int slot = filled - 1;                                        // 008170C0
        const std::uint32_t slot_command = host.slot_command(slot);         // 008170CF +54h+i*1Ch
        bool clear_slot = false;
        if (slot_command != 0 && host.command_kind_is_1_or_2_006bc030(slot_command)) {
            const std::uint32_t object = host.resolve_target_object(host.director_slot_target(slot));
            clear_slot = object == current || (current == 0 && flags != 0); // 008170F9..00817106
        }
        if (clear_slot) {
            if (slot != 0) {
                host.director_send_clear_command_slot_0071d900(slot);       // 00817117
            } else {
                host.clear_all_commands();                                  // 008171A3 0071D880
            }
            return;
        }
        if (handled || slot_command != kEntityCommandObject00e08f80) {
            return;                                                         // 00817131..00817145
        }
        const std::uint32_t path = host.director_slot_path_object_0071bfc0(slot);   // 00817152
        if (path != 0) {
            const int points = host.point_vector_size_0059cac0(host.path_object_vcall8(path));
            if (points - host.path_object_vcall14(path) > 1) {              // 00817178..0081717D
                host.path_object_vcall1c(path);                             // 00817186
                return;
            }
        }
        host.clear_all_commands();                                          // 008171A3 0071D880
        return;
    }
    if (command == kEntityCommandObject00e08f08) {                          // 008171BD
        host.director_set_fire_target_00835860(0, 1);                       // 008171CF
        if (host.unit_is_formation_follower_007788b0()) {
            return;                                                         // 008171DD
        }
        host.clear_all_commands();                                          // 008171E9 0071D880
        if (host.director_allows_fire_and_move_0080dc70()) {
            host.director_free_fire_0071bf20();                             // 00817205
        }
        return;
    }
    if (command == kEntityCommandObject00e08f78 || command == kEntityCommandObject00e08f10) {
        command = kEntityCommandObject00e08f78;                             // 00817238
        if (target.kind == 0) {
            // 00817243..00817329: a position becomes a fresh NavPoint at that
            // position, and the descriptor is rebuilt to target it.
            void* block = host.allocate_zeroed_00470b80(kEntityCommandNavPointSize);
            const std::uint32_t nav = block != nullptr
                ? host.nav_point_construct_004e5980(block, 0) : 0u;
            float identity[16] = {};
            identity[0] = identity[5] = identity[10] = identity[15] = 1.0f; // [00D7A24C]
            host.entity_vcall98(nav, 0, host.game_scene_19cc(), identity);  // 008172FF
            const std::array<float, 3> position
                = host.optional_vector3_get_or_default_0059bd20(target);    // 0081730A
            host.entity_set_world_position_006e8040(nav, position);         // 00817312
            host.entity_store_dword(nav, 0x54, 2);                          // 00817322
            host.command_target_from_entity_00464f70(target, nav, 0.0f);    // 00817329
        }
        entity_command_tail_00817330(host, command, flags, target);
        return;
    }
    entity_command_tail_00817330(host, command, flags, target);             // 0081722D
}

} // namespace bsp
