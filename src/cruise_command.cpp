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
void unit_apply_entity_command_00816e30(CruiseCommandHost& host,
                                        const EntityCommandMessageView& message) noexcept {
    // 00816E52..00816E96 copy the seven descriptor fields out of the message
    // into a local SceneCommandTarget; the view already carries that copy.
    const std::uint32_t command =
        host.command_object_from_message_ordinal(message.command_ordinal);  // 00816E9C
    if (command == 0) {
        return;                                                             // 00817330
    }
    if (message.flags != 0) {
        host.clear_all_commands();                                          // 0081733E
    } else if (!host.command_accepted(command)) {
        return;                                                             // 0081734B/0081734F
    }
    host.director_issue_command(command, message.target);                   // 0081735D
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

} // namespace bsp
