// Segment 23 reconstructions, packet cc13-seg23 (docs/CC13_SEG23.md).
#include "bsp/seg23_cc13.hpp"
#include "bsp/hud_ship_screen.hpp"
#include "bsp/frontend_screens_cc13.hpp"

#include <cstring>
#include <initializer_list>
#include <string>

namespace bsp::seg23_cc13 {
namespace {

template <class T>
T read_field(const void* object, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof(value));
    return value;
}

template <class T>
void write_field(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}

std::byte* at(void* object, std::size_t offset) noexcept {
    return static_cast<std::byte*>(object) + offset;
}

// The target block the order helpers build: zero bytes/words, the zero point,
// +14h = 0.0f (005F9AA3..005F9AF4 and the copies in 005F9BB0 / 005F9C40).
CommandTarget18 zero_point_target(HudOrderCommandHost& host) {
    const std::array<float, 3> p = host.zero_point_00f87574();
    CommandTarget18 target;
    target.x = p[0];
    target.y = p[1];
    target.z = p[2];
    return target;
}

// The stack message shape shared by 005FA9F0, 005FAAE0 and 005FACB0.
void build_stack_message(StackMessage& m, std::int32_t type, std::uint32_t vtable, HudOrderCommandHost& host) {
    host.session_message_construct_base_0075b430(m.data(), type);
    write_field<std::uint16_t>(m.data(), 0x18, 0);
    write_field<std::uint8_t>(m.data(), 0x1A, 0);
    write_field<std::uint32_t>(m.data(), 0x04, 1);
    write_field<std::uint8_t>(m.data(), 0x1C, 0);
    write_field<std::uint32_t>(m.data(), 0x00, vtable);
}

}  // namespace

// ---------------------------------------------------------------------------
// Order screen command helpers
// ---------------------------------------------------------------------------

void hud_order_set_force_sellable_005f9470(void* screen, std::uint8_t value) noexcept {
    write_field<std::uint8_t>(screen, 0x8C, value);
}

void hud_order_toggle_permanent_005f9580(void* screen, HudOrderCommandHost& host) {
    write_field<std::uint8_t>(screen, 0x83, read_field<std::uint8_t>(screen, 0x83) == 0 ? 1 : 0);
    void* unit = host.controlled_unit_00e188d8();
    if (host.unit_is_kind(unit, 6)) {
        void* orders = read_field<void*>(host.controlled_unit_00e188d8(), 0x738);
        host.orders_set_mode_54(orders, read_field<std::uint8_t>(screen, 0x83) != 0 ? 1 : 0);
        return;
    }
    unit = host.controlled_unit_00e188d8();
    if (!host.unit_is_kind(unit, 0x18)) return;
    unit = host.controlled_unit_00e188d8();
    void* orders = read_field<void*>(unit, 0x348);  // read before 007EDA90 (005F95E0)
    if (host.squadron_leader_uncommitted_kamikaze_007eda90(unit)) write_field<std::uint8_t>(screen, 0x83, 0);
    host.orders_set_mode_54(orders, read_field<std::uint8_t>(screen, 0x83) != 0 ? 1 : 0);
}

bool unit_is_kind_9_45_46_005f9710(void* unit, HudOrderCommandHost& host) {
    return host.unit_is_kind(unit, 9) || host.unit_is_kind(unit, 0x45) || host.unit_is_kind(unit, 0x46);
}

void session_message_reset_vtable_005f9790(void* message) noexcept {
    write_field<std::uint32_t>(message, 0x0, kSessionMessageRootVtable);
}

bool clock_f88c28_ahead_005f9820(HudOrderCommandHost& host) {
    // FLD [00F876A4]; FLD [00F88C28]; FCOMIP; JBE -> 0. NaN compares false.
    const float clock = host.fixed_step_clock_00f876a4();
    const float other = host.clock_00f88c28();
    return other > clock;
}

bool hud_order_can_attack_move_005f9840(HudOrderCommandHost& host) {
    void* unit = host.controlled_unit_00e188d8();
    if (unit == nullptr || !host.unit_is_kind(unit, 6)) return false;
    void* target = host.orders_current_target_2c(read_field<void*>(host.controlled_unit_00e188d8(), 0x738));
    if (target == nullptr) return false;
    if (!host.director_flags_allow_0071c060(1, host.controlled_unit_00e188d8())) return false;
    return host.unit_accepts_command_16c(host.controlled_unit_00e188d8(), kAttackMoveCommandName, target);
}

void hud_order_open_submenu_005f98b0(HudOrderCommandHost& host) {
    host.frontend_set_level3_004f8670(0x3B, 0x2A, 0x49, 0);
    host.input_push_level_004d8b70(host.game_00e188a8(), 0x18, 0x0F, 0x19, 0);
    void* block = read_field<void*>(host.frontend_manager_00e198c4(), 0x58);
    write_field<std::uint8_t>(block, 0x4, 0);
}

CommandWithTarget1C* command_with_target_clear_005f9a50(CommandWithTarget1C* out, HudOrderCommandHost& host) {
    out->command = 0;
    out->target = zero_point_target(host);
    return out;
}

void hud_order_issue_return_to_base_005f9aa0(HudOrderCommandHost& host) {
    const CommandTarget18 target = zero_point_target(host);
    host.entity_issue_command_0077d600(host.controlled_unit_00e188d8(), kCommandReturnToBase, &target, 1);
    write_field<std::uint8_t>(read_field<void*>(host.frontend_manager_00e198c4(), 0x68), 0x190, 1);
}

void hud_order_issue_moveto_entity_005f9b20(void* entity, HudOrderCommandHost& host) {
    if (read_field<std::uint8_t>(entity, 0xC8) == 0) host.entity_refresh_world_pose_00414db0(entity);
    CommandTarget18 target;
    target.b1 = 1;
    target.x = read_field<float>(entity, 0xFC);
    target.y = read_field<float>(entity, 0x100);
    target.z = read_field<float>(entity, 0x104);
    host.entity_issue_command_0077d600(host.controlled_unit_00e188d8(), kCommandMoveTo, &target, 1);
}

void hud_order_release_and_return_005f9bb0(HudOrderCommandHost& host) {
    host.unit_send_role_transfer_0077c470(host.controlled_unit_00e188d8(), 0x1FF, 0);
    const CommandTarget18 target = zero_point_target(host);
    host.entity_issue_command_0077d600(host.controlled_unit_00e188d8(), kCommandReturnToBase, &target, 1);
    write_field<std::uint8_t>(read_field<void*>(host.frontend_manager_00e198c4(), 0x68), 0x190, 1);
}

void hud_order_leave_or_disband_005f9c40(HudOrderCommandHost& host) {
    std::uint32_t command;
    if (host.unit_is_formation_follower_007788b0(host.controlled_unit_00e188d8())) {
        command = kCommandLeave;
    } else if (host.unit_is_group_leader_00778890(host.controlled_unit_00e188d8())) {
        command = kCommandDisband;
    } else {
        return;
    }
    const CommandTarget18 target = zero_point_target(host);
    host.entity_issue_command_0077d600(host.controlled_unit_00e188d8(), command, &target, 1);
}

CommandWithTarget1C* hud_order_resolve_return_to_base_005f9cf0(CommandWithTarget1C* out, HudOrderCommandHost& host) {
    void* unit = host.controlled_unit_00e188d8();
    if (unit != nullptr && host.unit_is_kind(unit, 0x18)) {
        host.squadron_resolve_return_to_base_007f16d0(unit, out);
        return out;
    }
    return command_with_target_clear_005f9a50(out, host);
}

void hud_order_regroup_005fa9f0(HudOrderCommandHost& host) {
    StackMessage m{};
    build_stack_message(m, 0xBC, kSessionMessage188Vtable, host);
    write_field<std::uint8_t>(m.data(), 0x20, 0);
    host.entity_route_message_0077c2a0(host.controlled_unit_00e188d8(), m.data(), 0, 0);
    host.warning_report_confirm_00977ba0(host.warning_manager_00f8a0c4(), host.controlled_unit_00e188d8(),
                                         kRegroupWarning);
}

void hud_order_attack_005faae0(HudOrderCommandHost& host) {
    if (host.unit_is_kind(host.controlled_unit_00e188d8(), 6)) {
        void* target = host.orders_current_target_2c(read_field<void*>(host.controlled_unit_00e188d8(), 0x738));
        if (target == nullptr) return;
        host.unit_send_role_transfer_0077c470(host.controlled_unit_00e188d8(), 2, 0);
        StackMessage m{};
        build_stack_message(m, 0x57, kSessionMessage87Vtable, host);
        host.entity_route_message_0077c2a0(host.controlled_unit_00e188d8(), m.data(), 0, 0);
        CommandTarget18 scratch;
        const void* resolved = host.command_target_from_entity_00465080(&scratch, target, 0.0f);
        host.entity_issue_command_0077d600(host.controlled_unit_00e188d8(), kCommandAttackMove, resolved, 1);
        return;
    }
    if (!host.unit_is_kind(host.controlled_unit_00e188d8(), 0x18)) return;
    StackMessage m{};
    build_stack_message(m, 0xBC, kSessionMessage188Vtable, host);
    write_field<std::uint8_t>(m.data(), 0x20, 1);
    host.entity_route_message_0077c2a0(host.controlled_unit_00e188d8(), m.data(), 0, 0);
    host.warning_report_confirm_00977ba0(host.warning_manager_00f8a0c4(), host.controlled_unit_00e188d8(),
                                         kAttackWarning);
}

void hud_order_leave_unit_005facb0(HudOrderCommandHost& host) {
    if (host.controlled_unit_00e188d8() == nullptr) return;
    void* manager = host.frontend_manager_00e198c4();
    host.hud_limbo_set_state_00565fb0(read_field<void*>(manager, 0xE8), 3);
    manager = host.frontend_manager_00e198c4();
    if (read_field<std::int32_t>(manager, 0x4) == read_field<std::int32_t>(manager, 0x20) &&
        read_field<std::int32_t>(manager, 0x1C) == read_field<std::int32_t>(manager, 0x38))
        host.frontend_push_interface_request_004cc460(manager, 0x34, 0);
    StackMessage m{};
    build_stack_message(m, 0x51, kSessionMessage81Vtable, host);
    host.entity_route_message_0077c2a0(host.controlled_unit_00e188d8(), m.data(), 7, 0);
}

void* zero_six_dwords_005fd420(void* object) noexcept {
    for (std::size_t offset = 0; offset < 0x18; offset += 4) write_field<std::uint32_t>(object, offset, 0);
    return object;
}

// ---------------------------------------------------------------------------
// 005FB080, the order screen's idle slot (cc13-orderscreen)
// ---------------------------------------------------------------------------
//
// Control-flow map, from the listing (005FB080..005FC5FB; every exit is the
// shared epilogue, written here as `return`):
//   005FB09C  no controlled unit, +4h set and +84h == ECh: 00565D80, return.
//   005FB0CF  [[M]+40h]+1Ch == 1 or [M]+20h in {2Bh, 2Ch, 2Dh, 29h}: return
//             (M = [00E198C4], re-read at each use as the image does).
//   005FB10B  a squadron unit (kind 18h) with byte +3B0h: +4h clear returns,
//             else 0068AB80(1, 1) and return.
//   005FB156  action 4Bh pressed: the back/close path 005FB16B..005FB2C2,
//             every leg returns.
//   005FB2C3  action 88h pressed: the unit-request path 005FB2DB..005FB3A3,
//             returns or falls to 005FB3A4.
//   005FB3A4  +4h clear (menu closed): 005FB3AE..005FB830, open on ECh
//             (005FB4EF) or on 109h (005FB7E4); all legs return.
//   005FB831  +4h set (menu open): resume the HUD when +82h is clear, then
//             with +84h == ECh and +82h clear the stick cluster 005FB883..
//             005FBC89, the branch pick 005FBC89..005FBE06 and the branch
//             widgets 005FBE07..005FC168.
//   005FC169  pings F3h/F4h on the 109h menu; 005FC247 on release of +84h:
//             ECh issues the picked order (005FC272..005FC54A), 109h sends
//             the ping of branch +88h (005FC561..005FC5E5).
//
// x87: the six hold times are FSTP'd to float locals; every sum, quotient
// and comparison operand is float-valued (the double constants 00CE3D40,
// 00D7A3A0, 00CE65D0, 00CED6B0 are 0.8f, 0.1f, 0.4f, -0.4f widened, and
// 00D7A2B0 is 3.0). The idle slot runs on the main thread, whose precision
// control is 24 bits, so each x87 result is rounded to float here (r24); at
// 53 or 64 bits the in-ST values (the two stick sums and the /3 quotients)
// would keep more bits until their store or compare.
namespace {

float r24(double v) noexcept { return static_cast<float>(v); }

// 0042BE90 __fastcall(ECX = float*) -> ST0: x when x > 0.0f (COMISS against
// 00D7A218), else -0.0f - x (SUBSS from 00D7A208); the result is a float.
float abs_0042be90(float x) noexcept { return x > 0.0f ? x : -0.0f - x; }

void* widget_at(void* screen, std::size_t offset) noexcept { return read_field<void*>(screen, offset); }

void issue_leave_or_return_legs(void* screen, HudOrderIdleHost& host);

}  // namespace

void hud_order_idle_005fb080(void* screen, HudOrderIdleHost& host) {
    static constexpr float kWhite[4] = {1.0f, 1.0f, 1.0f, 1.0f};  // 00D7A24C x4, a stack block
    // 005FB09C..005FB0CE
    if (host.controlled_unit_00e188d8() == nullptr) {
        if (read_field<std::uint8_t>(screen, 0x04) != 0 && read_field<std::int32_t>(screen, 0x84) == kOrderActionOpen) {
            host.hud_limbo_poll_respawn_input_00565d80();
            return;
        }
    }
    // 005FB0CF..005FB10A
    {
        void* const m = host.frontend_manager_00e198c4();
        if (read_field<std::int32_t>(read_field<void*>(m, 0x40), 0x1C) == 1) return;
        const std::int32_t current = read_field<std::int32_t>(m, 0x20);
        if (current == 0x2B || current == 0x2C || current == 0x2D || current == 0x29) return;
    }
    // 005FB10B..005FB155: ECX still holds the first [00E188D8] read.
    if (void* const unit = host.controlled_unit_00e188d8();
        unit != nullptr && host.unit_is_kind(unit, 0x18) &&
        read_field<std::uint8_t>(host.controlled_unit_00e188d8(), 0x3B0) != 0) {
        if (read_field<std::uint8_t>(screen, 0x04) == 0) return;
        host.ingame_collapse_overlays_0068ab80(host.frontend_manager_00e198c4(), 1, 1);
        return;
    }

    // 005FB156..005FB2C2: action 4Bh.
    if (host.input_action_pressed_this_frame_004c43c0(0x4B)) {
        void* m = host.frontend_manager_00e198c4();
        void* const p60 = read_field<void*>(m, 0x60);
        if (read_field<std::uint8_t>(p60, 0x05) != 0 && read_field<std::uint8_t>(p60, 0x70) != 0) return;
        if (read_field<std::uint8_t>(host.game_00e188a8(), 0x19C4) != 0) {
            if (host.input_action_pressed_this_frame_004c43c0(0xD1)) return;
            m = host.frontend_manager_00e198c4();
        }
        if (!host.pointer_vector_00e18d18_is_empty_004f7230()) {
            // 006529E0 inlined (MOV AL,[ECX+4]); 00652A00 inlined ([ECX+5Ch] == 2).
            if (read_field<std::uint8_t>(read_field<void*>(m, 0xBC), 0x04) != 0 &&
                read_field<std::int32_t>(read_field<void*>(host.frontend_manager_00e198c4(), 0xBC), 0x5C) != 2) {
                void* const support = read_field<void*>(host.frontend_manager_00e198c4(), 0xBC);
                if (read_field<std::uint8_t>(support, 0x04) != 0) {
                    // 006529B0 inlined: __thiscall(support, byte) RET 4 stores +66h.
                    write_field<std::uint8_t>(read_field<void*>(host.frontend_manager_00e198c4(), 0xBC), 0x66, 1);
                }
                if (read_field<std::uint8_t>(host.menu_command_screen_00425d10(), 0x25C) != 0) return;
                host.ingame_collapse_overlays_0068ab80(host.frontend_manager_00e198c4(), 0, 1);
                return;
            }
            m = host.frontend_manager_00e198c4();  // 005FB23B
        }
        // 005FB241
        if (read_field<std::uint8_t>(host.game_00e188a8(), 0x19C4) != 0) {
            if (read_field<std::uint8_t>(read_field<void*>(m, 0xBC), 0x04) == 0 &&
                read_field<std::int32_t>(read_field<void*>(host.frontend_manager_00e198c4(), 0xBC), 0x5C) != 2) {
                m = host.frontend_manager_00e198c4();
                void* const hints = read_field<void*>(m, 0xA8);
                if (read_field<std::uint8_t>(hints, 0x05) == 0 || read_field<std::uint8_t>(hints, 0x08) == 0) {
                    host.ingame_request_scene_for_screen54_0068aa90(m);
                    return;
                }
            } else {
                m = host.frontend_manager_00e198c4();  // 005FB2A4
            }
        }
        host.ingame_leave_unit_request_level_0068ab30(m);  // 005FB2AA
        return;
    }

    // 005FB2C3..005FB3A3: action 88h.
    if (host.input_action_pressed_this_frame_004c43c0(0x88) &&
        !(read_field<std::uint8_t>(host.game_00e188a8(), 0x19C4) != 0 && host.input_action_was_pressed_004c50d0(0xC4)) &&
        unit_is_kind_9_45_46_005f9710(host.controlled_unit_00e188d8(), host)) {
        void* const unit = host.controlled_unit_00e188d8();
        const std::uint32_t slot = read_field<std::uint32_t>(host.game_00e188a8(), 0x18EC);
        if (host.unit_role_open_to_slot_0059bbd0(unit, 8, slot)) {
            if (host.unit_is_local_player_role_00927f30(unit, 0)) {
                host.ingame_collapse_overlays_0068ab80(host.frontend_manager_00e198c4(), 1, 1);
                const bool base = host.ingame_is_base_interface_0068a140(host.frontend_manager_00e198c4());
                void* const m = host.frontend_manager_00e198c4();
                if (base) {
                    host.ingame_leave_unit_request_level_0068ab30(m);  // 005FB2AC
                } else {
                    host.ingame_push_controlled_unit_request_0068aad0(m);
                }
                return;
            }
            if (read_field<std::uint8_t>(host.game_00e188a8(), 0x19C4) != 0) {
                host.ingame_collapse_overlays_0068ab80(host.frontend_manager_00e198c4(), 1, 1);
                host.ingame_push_controlled_unit_request_0068aad0(host.frontend_manager_00e198c4());
            }
        }
    }

    // 005FB3A4: menu closed.
    if (read_field<std::uint8_t>(screen, 0x04) == 0) {
        void* unit = nullptr;
        bool to_ping_test = true;  // 005FB7A3 / 005FB7A9
        if (host.input_action_pressed_this_frame_004c43c0(kOrderActionOpen) &&
            (unit = host.controlled_unit_00e188d8()) != nullptr) {
            const std::uint32_t slot = read_field<std::uint32_t>(host.game_00e188a8(), 0x18EC);
            if (host.unit_role_open_to_slot_0059bbd0(unit, 0, slot) &&
                read_field<std::uint8_t>(read_field<void*>(host.frontend_manager_00e198c4(), 0xBC), 0x04) == 0) {
                void* const m = host.frontend_manager_00e198c4();
                if (!cc13_fe1::hud_order_screen_has_pending_order_005f9410(read_field<void*>(m, 0x78)) &&
                    read_field<std::uint8_t>(read_field<void*>(m, 0x5C), 0x05) == 0) {
                    to_ping_test = false;
                }
            }
        }
        if (to_ping_test) {
            // 005FB7A3..005FB830: the ping menu.
            void* const game = host.game_00e188a8();
            if (read_field<std::uint8_t>(game, 0x19C4) == 0) return;
            if (read_field<std::int32_t>(game, 0x1FE4) == 0) return;
            if (clock_f88c28_ahead_005f9820(host)) return;
            if (!host.input_action_pressed_this_frame_004c43c0(kOrderActionPing)) return;
            write_field<std::int32_t>(screen, 0x84, kOrderActionPing);
            host.ingame_collapse_overlays_0068ab80(host.frontend_manager_00e198c4(), 0, 1);
            host.frontend_screen_mark_visible_004b6e30(screen);
            static constexpr std::int32_t kLevel3[] = {0x3B, 0};
            host.frontend_set_level3_ids_004f8670(kLevel3);
            static constexpr std::int32_t kInput[] = {0x18, 0};
            host.input_push_level3_ids_004d8b70(host.game_00e188a8(), kInput);
            return;
        }
        // 005FB42A: the first-use hint gate.
        if (host.dword_00f8898c() > 1) {
            if (!host.award_hint_has_been_shown_0068e600(kOrderFirstUseHint)) {
                host.award_hint_trigger_00690fd0(kOrderFirstUseHint, 1);
                return;
            }
            if (host.award_active_hint_equals_0068e5b0(host.award_tracker_004e1ca0(), kOrderFirstUseHint)) return;
        }
        // 005FB4EF: open the radial menu.
        write_field<std::uint8_t>(screen, 0x81, 1);
        write_field<std::uint8_t>(screen, 0x82, 0);
        write_field<float>(screen, 0x68, 0.0f);
        write_field<float>(screen, 0x6C, 0.0f);
        write_field<std::int32_t>(screen, 0x84, kOrderActionOpen);
        write_field<std::int32_t>(screen, 0x88, 0);
        host.ingame_collapse_overlays_0068ab80(host.frontend_manager_00e198c4(), 1, 0);
        host.ingame_leave_unit_request_level_0068ab30(host.frontend_manager_00e198c4());
        host.frontend_screen_close_004b6e50(read_field<void*>(host.frontend_manager_00e198c4(), 0x78));
        host.frontend_screen_mark_visible_004b6e30(screen);
        static constexpr std::int32_t kLevel3[] = {0x3B, 0};
        host.frontend_set_level3_ids_004f8670(kLevel3);
        static constexpr std::int32_t kInput[] = {0x18, 0x19, 0};
        host.input_push_level3_ids_004d8b70(host.game_00e188a8(), kInput);
        host.hud_order_hide_all_005f9490(screen);
        // 005FB571: the branches the controlled unit offers.
        if (host.unit_is_kind(host.controlled_unit_00e188d8(), 0x18)) {
            void* const squadron = host.unit_cast_to_squadron_005f9a30(host.controlled_unit_00e188d8());
            if (squadron == nullptr) return;
            if (host.director_flags_allow_0071c060(0x40, host.controlled_unit_00e188d8())) {
                host.gui_text_set_localised_abbe50(widget_at(screen, 0x58), "", 1);  // 00CE3A0C
                CommandWithTarget1C resolved;
                hud_order_resolve_return_to_base_005f9cf0(&resolved, host);
                if (resolved.command != 0) {
                    host.widget_show_34(widget_at(screen, resolved.command == kCommandReturnToBaseLanding ? 0x34 : 0x30), 1);
                }
            }
            // 005FB5EC
            if (host.director_flags_allow_0071c060(0x80, host.controlled_unit_00e188d8())) {
                host.widget_show_34(widget_at(screen, 0x20), 1);
            } else if (host.unit_lead_entity_00927880(host.controlled_unit_00e188d8()) != nullptr) {
                host.widget_show_34(widget_at(screen, 0x1C), 1);
            }
            // 005FB620: signed count at squadron+3CCh.
            if (read_field<std::int32_t>(squadron, 0x3CC) > 0 &&
                host.director_flags_allow_0071c060(0x100, host.controlled_unit_00e188d8())) {
                host.widget_show_34(widget_at(screen, 0x14), 1);
            }
            host.widget_show_34(widget_at(screen, 0x10), read_field<std::uint8_t>(screen, 0x83) == 0 ? 1 : 0);
            host.widget_show_34(widget_at(screen, 0x0C), read_field<std::uint8_t>(screen, 0x83));
            return;
        }
        // 005FB682: ships.
        if (!host.unit_is_kind(host.controlled_unit_00e188d8(), 6)) return;
        void* const ship = host.controlled_unit_00e188d8();
        if (read_field<std::uint32_t>(ship, 0x284) != 0) {
            if (host.director_flags_allow_0071c060(0x4, ship)) host.widget_show_34(widget_at(screen, 0x2C), 1);
            if (host.director_flags_allow_0071c060(0x8, host.controlled_unit_00e188d8())) {
                host.widget_show_34(widget_at(screen, 0x18), 1);
            }
        } else if (read_field<std::int32_t>(host.game_00e188a8(), 0x1FE4) != 0 ||
                   read_field<std::uint8_t>(screen, 0x8C) != 0) {
            // 005FB6F8: ECX is still the ship read at 005FB693.
            if (host.unit_port_range_test_005f98f0(ship)) host.widget_show_34(widget_at(screen, 0x28), 1);
        }
        // 005FB70D
        if (host.director_flags_allow_0071c060(0x10, host.controlled_unit_00e188d8())) {
            host.widget_show_34(widget_at(screen, read_field<std::uint8_t>(screen, 0x83) != 0 ? 0x0C : 0x10), 1);
        }
        if (hud_order_can_attack_move_005f9840(host)) {  // ECX = screen, unused
            host.widget_show_34(widget_at(screen, 0x24), 1);
            host.widget_show_34(widget_at(screen, 0x1C), 0);
            return;
        }
        if (host.unit_lead_entity_00927880(host.controlled_unit_00e188d8()) == nullptr) return;
        host.widget_show_34(widget_at(screen, 0x1C), 1);
        return;
    }

    // 005FB831: menu open.
    if (read_field<std::uint8_t>(screen, 0x82) == 0) {
        host.weapon_group_resume_00545ac0(read_field<void*>(host.frontend_manager_00e198c4(), 0x50));
        write_field<std::uint8_t>(read_field<void*>(host.frontend_manager_00e198c4(), 0x68), 0x190, 0);
        host.ship_screen_clear_timed_entry_00644240(read_field<void*>(host.frontend_manager_00e198c4(), 0x40));
    }
    if (read_field<std::int32_t>(screen, 0x84) == kOrderActionOpen && read_field<std::uint8_t>(screen, 0x82) == 0) {
        // 005FB883..005FB908: dt and the six hold times (each FSTP'd to a float local).
        const float dt = read_field<float>(host.game_00e188a8(), 0x21EC);
        const float h104 = host.input_action_hold_time_004c5070(0x104);
        const float h105 = host.input_action_hold_time_004c5070(0x105);
        const float h11c = host.input_action_hold_time_004c5070(0x11C);
        const float h11d = host.input_action_hold_time_004c5070(0x11D);
        const float h126 = host.input_action_hold_time_004c5070(0x126);
        const float h127 = host.input_action_hold_time_004c5070(0x127);
        auto set_mode = [screen](std::uint8_t a, std::uint8_t b, std::uint8_t c) {
            write_field<std::uint8_t>(screen, 0x5C, a);
            write_field<std::uint8_t>(screen, 0x5D, b);
            write_field<std::uint8_t>(screen, 0x5E, c);
        };
        // 005FB909..005FB92F: |h126| + |h127| kept in ST, FCOMIP against 0.8; JBE.
        if (r24(static_cast<double>(abs_0042be90(h126)) + abs_0042be90(h127)) > 0.8f) {
            if (h127 < 0.0f) {  // 005FB93E: COMISS 0, h127; JBE
                const float v = read_field<float>(screen, 0x64);
                set_mode(0, 1, 0);
                write_field<float>(screen, 0x68, 0.0f);
                if (0.1f > v) {  // FCOMIP 0.1 vs v; JBE -> 005FB98A
                    write_field<float>(screen, 0x60, 0.0f);
                    write_field<float>(screen, 0x64, -0.5f);  // 00CE69D0
                    write_field<float>(screen, 0x6C, -0.9f);  // 00CED6C8
                } else {
                    write_field<float>(screen, 0x64, r24(static_cast<double>(v) - dt));
                    write_field<float>(screen, 0x6C, 0.0f);
                }
            } else if (h127 > 0.0f) {  // 005FB996: COMISS h127, 0; JBE
                const float v = read_field<float>(screen, 0x64);
                set_mode(0, 1, 0);
                write_field<float>(screen, 0x68, 0.0f);
                if (v > -0.01f) {  // COMISS v, 00D7A300; JBE -> 005FB9DF
                    write_field<float>(screen, 0x60, 0.0f);
                    write_field<float>(screen, 0x64, 0.5f);   // 00CE3800
                    write_field<float>(screen, 0x6C, 0.9f);   // 00CE3860
                } else {
                    write_field<float>(screen, 0x64, r24(static_cast<double>(v) + dt));
                    write_field<float>(screen, 0x6C, 0.0f);
                }
            } else if (h126 < 0.0f) {  // 005FB9EF
                const float v = read_field<float>(screen, 0x60);
                set_mode(0, 1, 0);
                write_field<float>(screen, 0x6C, 0.0f);
                if (v > -0.01f) {  // -> 005FBA43 otherwise
                    write_field<float>(screen, 0x60, 0.5f);
                    write_field<float>(screen, 0x64, 0.0f);
                    write_field<float>(screen, 0x68, -0.9f);
                } else {
                    write_field<float>(screen, 0x68, 0.0f);
                    write_field<float>(screen, 0x60, r24(static_cast<double>(v) + dt));
                }
            } else if (h126 > 0.0f) {  // 005FBA58
                const float v = read_field<float>(screen, 0x60);
                set_mode(0, 1, 0);
                write_field<float>(screen, 0x6C, 0.0f);
                if (0.01f > v) {  // COMISS 00D7A238, v; JBE -> 005FBAAE
                    write_field<float>(screen, 0x60, -0.5f);
                    write_field<float>(screen, 0x64, 0.0f);
                    write_field<float>(screen, 0x68, 0.9f);
                } else {
                    write_field<float>(screen, 0x68, 0.0f);
                    write_field<float>(screen, 0x60, r24(static_cast<double>(v) - dt));
                }
            }
        } else {
            // 005FBAC3: L = |h11c| + |h11d| (FSTP double over the dt slot), R = |h104| + |h105| in ST.
            const float a11d = abs_0042be90(h11d);
            const float a11c = abs_0042be90(h11c);
            const float left = r24(static_cast<double>(a11c) + a11d);
            const float right = r24(static_cast<double>(abs_0042be90(h104)) + abs_0042be90(h105));
            if (left > right) {  // FCOMI ST0 = L, ST1 = R; JBE -> 005FBBF8
                set_mode(1, 0, 0);
                // DC F9 FDIV ST(1),ST(0): h11c / 3.0 kept in ST, DE C2 FADDP adds +68h, FSTP to the local.
                const float n68 = r24(static_cast<double>(r24(static_cast<double>(h11c) / 3.0)) +
                                      read_field<float>(screen, 0x68));
                write_field<float>(screen, 0x68, n68);
                // FDIVR [h11d]: h11d / 3.0, FADD +6Ch, FSTP to the local.
                const float n6c = r24(static_cast<double>(r24(static_cast<double>(h11d) / 3.0)) +
                                      read_field<float>(screen, 0x6C));
                write_field<float>(screen, 0x6C, n6c);
                if (a11c > a11d) {  // FCOMIP ST0 = |h11c|, ST1 = |h11d|; JBE -> 005FBBA8
                    if (n68 > 0.55f) {  // 00CED6C4
                        write_field<float>(screen, 0x68, 0.6f);   // 00CE3D30
                        write_field<float>(screen, 0x6C, 0.0f);
                    }
                    if (-0.55f > read_field<float>(screen, 0x68)) {  // 00CED6C0
                        write_field<float>(screen, 0x68, -0.6f);  // 00CED6BC
                        write_field<float>(screen, 0x6C, 0.0f);
                    }
                } else {
                    if (n6c > 0.55f) {
                        write_field<float>(screen, 0x6C, 0.6f);
                        write_field<float>(screen, 0x68, 0.0f);
                    }
                    if (-0.55f > read_field<float>(screen, 0x6C)) {
                        write_field<float>(screen, 0x6C, -0.6f);
                        write_field<float>(screen, 0x68, 0.0f);
                    }
                }
            } else if (right > left) {  // 005FBBF8: FXCH, FCOMI ST0 = R, ST1 = L; JBE -> 005FBC21
                write_field<float>(screen, 0x68, h104);
                set_mode(0, 0, 1);
                write_field<float>(screen, 0x6C, h105);
            } else if (left == right) {  // 005FBC21: FUCOMIP, LAHF, TEST AH,44h, JP: ordered equal only
                if (read_field<std::uint8_t>(screen, 0x5D) != 0 &&
                    0.59f > abs_0042be90(read_field<float>(screen, 0x6C)) &&  // 00CED6B8
                    0.59f > abs_0042be90(read_field<float>(screen, 0x68))) {
                    write_field<float>(screen, 0x6C, 0.0f);
                    write_field<float>(screen, 0x68, 0.0f);
                    write_field<float>(screen, 0x60, 0.0f);
                    write_field<float>(screen, 0x64, 0.0f);
                }
                if (read_field<std::uint8_t>(screen, 0x5E) != 0) {  // 005FBC79
                    write_field<float>(screen, 0x68, 0.0f);
                    write_field<float>(screen, 0x6C, 0.0f);
                }
            }
        }

        // 005FBC89..005FBE06: the highlighted branch +88h.
        if (0.4f > abs_0042be90(read_field<float>(screen, 0x6C)) &&
            0.4f > abs_0042be90(read_field<float>(screen, 0x68))) {
            write_field<std::int32_t>(screen, 0x88, 0);
        } else if (abs_0042be90(read_field<float>(screen, 0x6C)) > abs_0042be90(read_field<float>(screen, 0x68))) {
            const float v = read_field<float>(screen, 0x6C);
            if (v > 0.4f) {
                if (host.widget_is_selected_38(widget_at(screen, 0x14)) ||
                    host.widget_is_selected_38(widget_at(screen, 0x18))) {
                    write_field<std::int32_t>(screen, 0x88, 2);
                }
            } else if (-0.4f > v) {
                if (host.widget_is_selected_38(widget_at(screen, 0x1C)) ||
                    host.widget_is_selected_38(widget_at(screen, 0x20)) ||
                    host.widget_is_selected_38(widget_at(screen, 0x24))) {
                    write_field<std::int32_t>(screen, 0x88, 1);
                }
            }
        } else {
            const float v = read_field<float>(screen, 0x68);  // 005FBD79
            if (v > 0.4f) {
                if (host.widget_is_selected_38(widget_at(screen, 0x28)) ||
                    host.widget_is_selected_38(widget_at(screen, 0x2C)) ||
                    host.widget_is_selected_38(widget_at(screen, 0x30)) ||
                    host.widget_is_selected_38(widget_at(screen, 0x34))) {
                    write_field<std::int32_t>(screen, 0x88, 4);
                }
            } else if (-0.4f > v) {
                if (host.widget_is_selected_38(widget_at(screen, 0x10)) ||
                    host.widget_is_selected_38(widget_at(screen, 0x0C))) {
                    write_field<std::int32_t>(screen, 0x88, 3);
                }
            }
        }

        // 005FBE07..005FC168: the branch highlight widgets and the help line +58h.
        for (std::size_t offset = 0x44; offset <= 0x54; offset += 4) host.widget_show_34(widget_at(screen, offset), 0);
        struct Label {
            std::size_t widget;
            const char* key;
        };
        auto show_branch = [&](std::size_t highlight, std::initializer_list<Label> labels) {
            host.widget_show_34(widget_at(screen, highlight), 1);
            host.widget_set_colour_50(widget_at(screen, highlight), kWhite);
            for (const Label& label : labels) {
                if (host.widget_is_selected_38(widget_at(screen, label.widget))) {
                    host.gui_text_set_localised_abbe50(widget_at(screen, 0x58), label.key, 1);
                    return;
                }
            }
        };
        switch (read_field<std::int32_t>(screen, 0x88)) {
        case 3:
            show_branch(0x44, {{0x0C, "ingame.orders_aggressive"}, {0x10, "ingame.orders_defensive"}});
            break;
        case 4:
            show_branch(0x48, {{0x28, "ingame.orders_shipsell"}, {0x34, "ingame.orders_planelanding"},
                               {0x30, "ingame.orders_planeretreat"}, {0x2C, "ingame.orders_shipleaveformation"}});
            break;
        case 1:
            show_branch(0x4C, {{0x20, "ingame.orders_planeattack"}, {0x24, "ingame.orders_shipattack"},
                               {0x1C, "ingame.orders_shipplanemove"}});
            break;
        case 2:
            show_branch(0x50, {{0x18, "ingame.orders_shipformation"}, {0x14, "ingame.orders_planeregroup"}});
            break;
        case 0:
            host.widget_show_34(widget_at(screen, 0x44), 0);
            host.widget_show_34(widget_at(screen, 0x48), 0);
            host.widget_show_34(widget_at(screen, 0x4C), 0);
            host.widget_show_34(widget_at(screen, 0x50), 0);
            host.widget_show_34(widget_at(screen, 0x54), 1);
            host.widget_set_colour_50(widget_at(screen, 0x54), kWhite);
            host.gui_text_set_localised_abbe50(widget_at(screen, 0x58), "ingame.orders_cancel", 1);
            write_field<std::int32_t>(screen, 0x88, 0);
            break;
        default:
            break;
        }
    }

    // 005FC169..005FC246: pings on the 109h menu.
    if ((host.input_action_pressed_this_frame_004c43c0(0xF1) || host.input_action_pressed_this_frame_004c43c0(0xF2) ||
         host.input_action_pressed_this_frame_004c43c0(0xF3) || host.input_action_pressed_this_frame_004c43c0(0xF4)) &&
        read_field<std::int32_t>(screen, 0x84) == kOrderActionPing) {
        std::int32_t kind = 0;
        if (host.input_action_pressed_this_frame_004c43c0(0xF3)) {
            kind = 1;
        } else if (host.input_action_pressed_this_frame_004c43c0(0xF4)) {
            kind = 2;
        }
        if (kind != 0) {
            // The map cursor {x, y} at [[00E198C4]+54h]+554h, copied by FLD/FSTP float.
            const void* const map = read_field<void*>(host.frontend_manager_00e198c4(), 0x54);
            host.map_ping_send_local_008e2ee0(kind, {read_field<float>(map, 0x554), read_field<float>(map, 0x558)});
        }
    }

    // 005FC247: on release of the held action.
    if (!host.input_action_released_this_frame_00535ee0(read_field<std::int32_t>(screen, 0x84))) return;
    const std::int32_t action = read_field<std::int32_t>(screen, 0x84);
    if (action == kOrderActionOpen) {
        write_field<std::uint8_t>(screen, 0x81, 0);
        write_field<std::uint8_t>(screen, 0x82, 1);
        write_field<float>(screen, 0x70, 0.0f);
        host.hud_hints_dismiss_0054e610(read_field<void*>(host.frontend_manager_00e198c4(), 0xA8));
        host.award_tracker_finish_active_hint_0068e420(host.award_tracker_004e1ca0());
        if (read_field<std::int32_t>(screen, 0x88) == 3 &&
            (host.widget_is_selected_38(widget_at(screen, 0x0C)) || host.widget_is_selected_38(widget_at(screen, 0x10)))) {
            hud_order_toggle_permanent_005f9580(screen, host);
            return;
        }
        const std::int32_t branch = read_field<std::int32_t>(screen, 0x88);  // 005FC2E2
        if (branch == 1) {
            if (host.widget_is_selected_38(widget_at(screen, 0x20)) || host.widget_is_selected_38(widget_at(screen, 0x24))) {
                hud_order_attack_005faae0(host);  // ECX = screen, unused
                return;
            }
            if (!host.widget_is_selected_38(widget_at(screen, 0x1C))) return;
            // 00927880's result goes to 005F9B20 unchecked.
            hud_order_issue_moveto_entity_005f9b20(host.unit_lead_entity_00927880(host.controlled_unit_00e188d8()), host);
            return;
        }
        if (branch == 2) {
            if (host.unit_is_kind(host.controlled_unit_00e188d8(), 6) &&
                read_field<std::uint32_t>(host.controlled_unit_00e188d8(), 0x284) != 0 &&
                host.widget_is_selected_38(widget_at(screen, 0x18))) {
                host.hud_order_close_005f9520(screen);
                hud_order_open_submenu_005f98b0(host);
                return;
            }
            // 005FC3B2 / 005FC3B8
            if (!host.unit_is_kind(host.controlled_unit_00e188d8(), 0x18)) return;
            if (!host.widget_is_selected_38(widget_at(screen, 0x14))) return;
            hud_order_regroup_005fa9f0(host);
            return;
        }
        if (branch != 4) return;
        if (host.unit_is_kind(host.controlled_unit_00e188d8(), 6) &&
            read_field<std::uint32_t>(host.controlled_unit_00e188d8(), 0x284) != 0 &&
            host.widget_is_selected_38(widget_at(screen, 0x2C))) {
            if (read_field<std::uint8_t>(read_field<void*>(host.frontend_manager_00e198c4(), 0x5C), 0x04) != 0) return;
            hud_order_leave_or_disband_005f9c40(host);
            return;
        }
        issue_leave_or_return_legs(screen, host);  // 005FC459
        return;
    }
    if (action == 0xED) return;
    if (action != kOrderActionPing) return;
    // 005FC561: the ping of the highlighted branch (ECX = the branch for 1 and 2, XOR ECX for 0).
    const std::int32_t branch = read_field<std::int32_t>(screen, 0x88);
    if (branch != 1 && branch != 2 && branch != 0) return;
    const void* const map = read_field<void*>(host.frontend_manager_00e198c4(), 0x54);
    host.map_ping_send_local_008e2ee0(branch, {read_field<float>(map, 0x554), read_field<float>(map, 0x558)});
}

namespace {

// 005FC459..005FC54A, branch 4 when the ship leave/disband leg did not fire.
// Effective game mode 8 (004BCA50, called twice) with +8Ch set and [+28h]
// selected leaves the unit at once; otherwise outside mode 8 or with +8Ch set
// the port-range test 005F98F0 must also pass. Then the squadron legs:
// [+30h] release-and-return, [+34h] return-to-base.
void issue_leave_or_return_legs(void* screen, HudOrderIdleHost& host) {
    if (host.game_effective_mode_004bca50(host.game_00e188a8()) == 8 && read_field<std::uint8_t>(screen, 0x8C) != 0 &&
        host.widget_is_selected_38(widget_at(screen, 0x28))) {
        hud_order_leave_unit_005facb0(host);
        return;
    }
    // 005FC480
    if ((host.game_effective_mode_004bca50(host.game_00e188a8()) != 8 || read_field<std::uint8_t>(screen, 0x8C) != 0) &&
        host.unit_port_range_test_005f98f0(host.controlled_unit_00e188d8()) &&
        host.widget_is_selected_38(widget_at(screen, 0x28))) {
        hud_order_leave_unit_005facb0(host);
        return;
    }
    // 005FC4CF
    if (host.unit_is_kind(host.controlled_unit_00e188d8(), 0x18) && host.widget_is_selected_38(widget_at(screen, 0x30))) {
        hud_order_release_and_return_005f9bb0(host);
        return;
    }
    // 005FC509
    if (host.unit_is_kind(host.controlled_unit_00e188d8(), 0x18) && host.widget_is_selected_38(widget_at(screen, 0x34))) {
        hud_order_issue_return_to_base_005f9aa0(host);
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// Options screen
// ---------------------------------------------------------------------------

namespace {
void zero_overlapped(void* screen, std::size_t offset) noexcept {
    for (std::size_t i = 0; i < 0x1C; i += 4) write_field<std::uint32_t>(screen, offset + i, 0);
}
}  // namespace

void options_verify_entered_name_005ef550(void* screen, OptionsScreenHost& host) {
    zero_overlapped(screen, 0x2C);
    host.memset_00bf79f0(read_field<void*>(screen, 0x26C), 0, read_field<std::uint32_t>(screen, 0x70));
    // STRING_DATA {WORD size; LPWSTR text} packed at ESP+8 / ESP+0Ah.
    const auto* text = reinterpret_cast<const std::uint16_t*>(at(screen, 0x48));
    std::uint16_t count = 0;
    while (text[count] != 0) ++count;
    std::array<std::byte, 2 + sizeof(void*)> string_data{};
    write_field<std::uint16_t>(string_data.data(), 0, static_cast<std::uint16_t>(count + 1));
    write_field<const void*>(string_data.data(), 2, text);
    zero_overlapped(screen, 0x2C);
    host.xstring_verify_00a4d464(0, kOptionsVerifyLocale, 1, string_data.data(),
                                 read_field<std::uint32_t>(screen, 0x70), read_field<void*>(screen, 0x26C),
                                 at(screen, 0x2C));
    write_field<std::uint8_t>(screen, 0x265, 1);
}

void options_listener_on_focus_005ef600(void* widget, bool focused, OptionsScreenHost& host) {
    if (host.widget_kind_5c(widget) == 2) return;
    if (host.widget_kind_5c(widget) == 3) {
        host.widget_set_alpha_4c(widget, focused ? kOptionsFocusAlphaOn : kOptionsFocusAlphaOff);
        return;
    }
    if (host.widget_kind_5c(widget) == 6) host.widget_show_88(widget, focused, 0, 1.0f);
}

void options_show_keyboard_005efab0(void* screen, OptionsScreenHost& host) {
    std::uint32_t user = 0;
    if (host.xenon_has_selected_user_00a3e510(host.xenon_system_manager_00f8abe8()))
        user = host.xenon_active_user_slot_00a3eac0(host.xenon_system_manager_00f8abe8());
    zero_overlapped(screen, 0x10);
    const char* source = host.keyboard_default_text_00f88a38();
    std::uint32_t length = 0;
    while (source[length] != 0) ++length;
    std::uint32_t converted = 0;
    auto* default_text = reinterpret_cast<wchar_t*>(at(screen, 0xB0));
    host.mbstowcs_s_00bf9daa(&converted, default_text, length + 1, source, 0xFFFFFFFFu);
    write_field<std::uint16_t>(screen, 0x48, 0);
    write_field<std::uint16_t>(screen, 0x4A, 0);
    write_field<std::uint16_t>(screen, 0x4C, 0);
    host.xshow_keyboard_ui_00a4d43a(user, 0, default_text, nullptr, nullptr,
                                    reinterpret_cast<wchar_t*>(at(screen, 0x48)), 4, at(screen, 0x10));
    write_field<std::uint8_t>(screen, 0x264, 1);
}

void options_show_restart_notice_005f0090(OptionsScreenHost& host) {
    host.raise_prompt_00531b00(0, kOptionsRestartMessage, 2, 0, 0, "", 0.0f, 0, "", 1);
}

void options_listener_on_list_activate_005f0440(void* screen, void* listbox, OptionsScreenHost& host) {
    host.listbox_clear_other_two_00a9cdf0(listbox, read_field<void*>(screen, 0x7C), read_field<void*>(screen, 0x80));
    const std::int32_t row = host.listbox_selected_index_00a9c920(listbox);
    const std::int32_t page = host.options_page_00e19650();
    std::uint32_t table = 0;
    switch (page) {
    case 0: table = kHelpTablePage0; break;
    case 4: table = kHelpTablePage4; break;
    case 2: table = kHelpTablePage2; break;
    case 3: table = kHelpTablePage3; break;
    case 1: table = kHelpTablePage1; break;
    default: break;  // 7 and the rest: ""
    }
    if (table == 0) {
        host.command_bar_set_help_line_0054a0c0(host.command_bar_00e1930c(), "");
        return;
    }
    const std::string_view label = host.options_row_label_at_005f0220(table, row);
    host.command_bar_set_help_line_0054a0c0(host.command_bar_00e1930c(), label);
    if (page != 2) return;
    if (row == 2) {
        host.widget_enable_34(read_field<void*>(screen, 0x9C), 1);
    } else if (row == 3) {
        host.widget_enable_34(read_field<void*>(screen, 0x98), 1);
    } else {
        host.widget_enable_34(read_field<void*>(screen, 0x9C), 0);
        host.widget_enable_34(read_field<void*>(screen, 0x98), 0);
    }
}

void options_set_row_value_text_005f0800(void* screen, std::int32_t row, std::string_view text, bool new_content,
                                         bool damaged, OptionsScreenHost& host) {
    void* row_widget = host.listbox_row_widget_00a9be00(read_field<void*>(screen, 0x88), row);
    void* value = host.widget_find_child_00aa7e00(row_widget, kRowValueTextChild, 1);
    if (damaged) {
        host.text_set_localised_cstring_00abbe50(value, "FE.opt_damagedcontent", 1);
        return;
    }
    host.text_set_localised_source_00abaed0(value, new_content ? kNewContentText : text, 1);
}

void options_set_row_icon_005f0940(void* screen, std::int32_t row, bool shown, OptionsScreenHost& host) {
    void* row_widget = host.listbox_row_widget_00a9be00(read_field<void*>(screen, 0x88), row);
    void* group = host.widget_find_child_00aa7e00(row_widget, kRowAutoControlGroup, 1);
    void* icon = host.widget_find_child_00aa7e00(group, kRowIconChild, 1);
    host.widget_show_88(icon, shown, 0, 1.0f);
}

void options_save_input_modifiers_005f0b90(OptionsScreenHost& host) {
    if (host.settings_dirty_00f88a30() && host.xenon_has_selected_user_00a3e510(host.xenon_system_manager_00f8abe8())) {
        host.raise_prompt_00531b00(0, kOptionsSaveConfirm, 1, kPromptApplyAndSaveSettings, 0, "", 0.0f, 0, "", 1);
        return;
    }
    host.media_manager_noop_00a4c2d0(host.media_manager_get_or_create_004c1710(1));
    host.settings_apply_all_008d5b50();
    host.settings_save_to_profile_008d6c00(kStorageEmptyContinuation);
}

void options_row_show_value_arrows_005f1620(void* row, OptionsScreenHost& host) {
    write_field<std::uint8_t>(row, 0x77, 1);
    host.widget_set_listener_fields_00aa6bc0(host.widget_find_child_00aa7e00(row, kArrowLeftIcon, 1), 0, 0);
    host.widget_set_listener_fields_00aa6bc0(host.widget_find_child_00aa7e00(row, kArrowRightIcon, 1), 0, 0);
}

void options_bind_widgets_005f1cc0(void* screen, OptionsScreenHost& host) {
    auto bind = [&](std::size_t slot, std::size_t parent, std::string_view name) {
        write_field(screen, slot, host.widget_find_child_00aa7e00(read_field<void*>(screen, parent), name, 1));
    };
    bind(0x84, 0x7C, "Main_Listbox");        // 00CEB334
    bind(0x8C, 0x84, "MainListbox_Text");    // 00CEC040
    bind(0x88, 0x80, "Settings_Listbox");    // 00CEE604
    host.listbox_set_field124_00a9ceb0(read_field<void*>(screen, 0x88), kOptionsFocusAlphaOn, 0);  // [00CE3800]
    write_field<float>(read_field<void*>(screen, 0x88), 0x128, kSettingsListboxField128);
    bind(0x254, 0x80, "New_FrameBox");       // 00CEED70
    host.widget_enable_34(read_field<void*>(screen, 0x254), 0);
    bind(0x258, 0x80, "New_Text");           // 00CF39D4
    host.widget_enable_34(read_field<void*>(screen, 0x258), 0);
    bind(0x90, 0x88, "text_Group");          // 00CF285C
    bind(0x94, 0x88, "slider_Group");        // 00CEE5F0
    bind(0x98, 0x88, "toggle_chkbox_Group"); // 00CF39C0
    bind(0x9C, 0x88, "toggle_text_Group");   // 00CEE58C
    for (std::size_t slot : {0x90u, 0x94u, 0x98u, 0x9Cu}) host.widget_enable_34(read_field<void*>(screen, slot), 0);
    host.options_find_auto_control_group_005f0390(read_field<void*>(screen, 0x94), at(screen, 0x0C));
    bind(0xA0, 0x7C, "SfxVolume_Sound");     // 00CF39B0
    bind(0xA4, 0x7C, "SpeechVolume_Sound");  // 00CF399C
}

void* options_screen_scalar_delete_005f3b30(void* screen, std::uint32_t flags, OptionsScreenHost& host) {
    host.options_screen_destruct_005f1bb0(screen);
    if (flags & 1) host.free_00bf65ac(screen);
    return screen;
}

namespace {
// FLD float; FMUL double 10.0; FADD double 0.5; CALL _ftol2 (truncation).
std::int32_t slider_position(float value) noexcept {
    return static_cast<std::int32_t>(static_cast<double>(value) * kSliderScale + kSliderRound);
}

void set_slider_row(void* screen, std::int32_t row, float value, OptionsScreenHost& host) {
    const std::int32_t position = slider_position(value);
    void* row_widget = host.listbox_row_widget_00a9be00(read_field<void*>(screen, 0x88), row);
    control_slider_set_row_005f3750(row_widget, position, host);
}
}  // namespace

void options_refresh_audio_rows_005f4fa0(void* screen, OptionsScreenHost& host) {
    set_slider_row(screen, 0, read_field<float>(screen, 0xF8), host);
    set_slider_row(screen, 1, read_field<float>(screen, 0x100), host);
    set_slider_row(screen, 2, read_field<float>(screen, 0x108), host);
    set_slider_row(screen, 3, read_field<float>(screen, 0x104), host);
}

void options_refresh_game_page_rows_005f5070(void* screen, OptionsScreenHost& host) {
    set_slider_row(screen, 0, read_field<float>(screen, 0x100), host);
    set_slider_row(screen, 1, read_field<float>(screen, 0x108), host);
    set_slider_row(screen, 2, read_field<float>(screen, 0x104), host);
    const char* row3 = host.string_table_entry(kGamePageRow3Texts, read_field<std::uint8_t>(screen, 0x165));
    options_set_row_value_text_005f0800(screen, 3, row3, false, false, host);
    const char* row4 = host.string_table_entry(kGamePageRow4Texts, read_field<std::uint32_t>(screen, 0x168));
    options_set_row_value_text_005f0800(screen, 4, row4, false, false, host);
}

void bit_vector_clear_005f6190(void* vector, BitVectorClearHost& host) {
    const std::uint32_t container = host.address_of(vector);
    std::uint32_t word = read_field<std::uint32_t>(vector, 0x8);
    if (word > read_field<std::uint32_t>(vector, 0xC)) host.invalid_parameter_00bf6713();
    std::array<std::uint32_t, 3> last{0, word, 0};
    if (vector == nullptr) host.invalid_parameter_00bf6713();
    last[0] = container;
    const std::uint32_t count = read_field<std::uint32_t>(vector, 0x0);
    if (count > 0) host.bit_iterator_advance_0048d3b0(last, count);
    word = read_field<std::uint32_t>(vector, 0x8);
    if (word > read_field<std::uint32_t>(vector, 0xC)) host.invalid_parameter_00bf6713();
    const std::array<std::uint32_t, 3> first{container, word, 0};
    std::array<std::uint32_t, 3> out{};
    host.bit_vector_erase_00499700(vector, out, first, last);
}

void options_refresh_game_rows_005f4c90(void* screen, OptionsScreenHost& host) {
    auto text = [&](std::int32_t row, const char* value) {
        options_set_row_value_text_005f0800(screen, row, value, false, false, host);
    };
    auto byte_entry = [&](std::size_t offset) {
        return host.string_table_entry(kGamePageRow3Texts, read_field<std::uint8_t>(screen, offset));
    };
    text(0, host.settings_language_lockit_id_008d48c0(at(screen, 0xD8)));
    text(1, host.settings_unit_label_key_008d4260(at(screen, 0xD8)));
    text(2, byte_entry(0xE1));
    text(3, host.string_table_entry(kGamePageRow3UnitTexts, read_field<std::uint32_t>(screen, 0xE4)));
    text(4, byte_entry(0xE8));
    text(5, byte_entry(0x154));
    const float sensitivity = read_field<float>(screen, 0x158);
    const auto position = static_cast<std::int32_t>(
        (static_cast<double>(sensitivity) + static_cast<double>(sensitivity)) * kSliderScale + kSliderRound);
    void* row6 = host.listbox_row_widget_00a9be00(read_field<void*>(screen, 0x88), 6);
    control_slider_set_row_005f3750(row6, position, host);
    text(7, byte_entry(0x122));
    text(8, byte_entry(0x18A));
}

void options_refresh_control_rows_005f34d0(void* screen, OptionsScreenHost& host) {
    auto text = [&](std::int32_t row, std::uint32_t table, std::uint32_t index) {
        options_set_row_value_text_005f0800(screen, row, host.string_table_entry(table, index), false, false, host);
    };
    text(0, kGamePageRow3Texts, read_field<std::uint8_t>(screen, 0x188));
    text(1, kGamePageRow3Texts, read_field<std::uint8_t>(screen, 0x118) == 0 ? 1u : 0u);
    text(2, kControlPageTextsA, read_field<std::uint8_t>(screen, 0x11A));
    text(3, kControlPageTextsA, read_field<std::uint8_t>(screen, 0x11B));
    text(4, kControlPageTextsB, read_field<std::uint8_t>(screen, 0x119));
    text(5, kControlPageTextsB, read_field<std::uint8_t>(screen, 0x11C));
    write_field<std::uint8_t>(host.listbox_row_widget_00a9be00(read_field<void*>(screen, 0x88), 1), 0x77, 0);
    for (std::int32_t row = 2; row < read_field<std::int32_t>(read_field<void*>(screen, 0x88), 0x104); ++row) {
        const std::uint8_t arrows = read_field<std::uint8_t>(screen, 0x188) == 0 ? 1 : 0;
        write_field<std::uint8_t>(host.listbox_row_widget_00a9be00(read_field<void*>(screen, 0x88), row), 0x77, arrows);
    }
}

namespace {
// The inline NativeString equality of 005F2D82..005F2DB8: equal lengths, two
// empty strings match, otherwise __stricmp on the texts == 0.
bool native_name_equal(const void* a, const void* b, OptionsScreenHost& host) {
    const auto la = read_field<std::uint32_t>(a, 0);
    const auto lb = read_field<std::uint32_t>(b, 0);
    if (la != lb) return false;
    if (la == 0) return true;
    return host.stricmp_00bf7fbf(read_field<const char*>(a, 4), read_field<const char*>(b, 4)) == 0;
}

bool name_in_list(const void* name, const void* screen, std::size_t list, std::size_t count,
                  OptionsScreenHost& host) {
    for (std::int32_t j = 0; j < read_field<std::int32_t>(screen, count); ++j) {
        // The entries are re-read from [00F8A304] on every pass (005F2D70).
        const auto* entry = static_cast<const std::byte*>(read_field<void*>(screen, list)) + j * 8;
        if (native_name_equal(entry, name, host)) return true;
    }
    return false;
}
}  // namespace

void options_refresh_content_rows_005f2d20(void* screen, OptionsScreenHost& host) {
    for (std::int32_t row = 0; row < read_field<std::int32_t>(read_field<void*>(screen, 0x88), 0x104); ++row) {
        auto content = [&] {
            return static_cast<const std::byte*>(read_field<void*>(host.content_manager_00f8a304(), 0x4)) + row * 0x18;
        };
        const bool disabled = name_in_list(content(), screen, 0x170, 0x174, host);
        const bool is_new = name_in_list(content(), screen, 0x17C, 0x180, host);
        const char* text = host.string_table_entry(kGamePageRow3Texts, disabled ? 0u : 1u);
        const bool damaged = read_field<std::uint8_t>(content(), 0x12) != 0;
        options_set_row_value_text_005f0800(screen, row, text, is_new, damaged, host);
    }
}

void options_refresh_video_rows_005f2f20(void* screen, OptionsScreenHost& host) {
    auto text = [&](std::int32_t row, std::string_view value) {
        options_set_row_value_text_005f0800(screen, row, value, false, false, host);
    };
    auto entry = [&](std::uint32_t table, std::uint32_t index) { return host.string_table_entry(table, index); };
    const auto* mode = static_cast<const std::byte*>(host.resolution_table_00f8895c()) +
                       read_field<std::int32_t>(screen, 0x150) * 8;
    text(0, host.format_004f9a80(kResolutionFormat, read_field<std::int32_t>(mode, 0), read_field<std::int32_t>(mode, 4)));
    text(1, entry(kVideoQualityTexts, read_field<std::uint8_t>(screen, 0xF6)));
    const std::int32_t samples = read_field<std::int32_t>(screen, 0x130);
    if (samples == 0) {
        text(2, entry(kOnOffTexts, 0));
    } else {
        text(2, host.format_004f9a80(kMultisampleFormat, samples));
    }
    text(3, entry(kOnOffTexts, read_field<std::uint8_t>(screen, 0x138)));
    text(4, host.format_004f9a80(kGammaFormat, static_cast<double>(read_field<float>(screen, 0x13C))));
    text(6, entry(kVideoDetailTexts, read_field<std::uint32_t>(screen, 0x12C)));
    text(5, entry(kVideoDetailTexts, read_field<std::uint32_t>(screen, 0x140)));
    text(7, entry(kOnOffTexts, read_field<std::uint8_t>(screen, 0x15C)));
    text(8, entry(kOnOffTexts, read_field<std::uint8_t>(screen, 0x144)));
    text(9, entry(kOnOffTexts, read_field<std::uint8_t>(screen, 0x14C)));
    text(10, entry(kOnOffTexts, read_field<std::uint8_t>(screen, 0x15E)));
    text(11, entry(kOnOffTexts, read_field<std::uint8_t>(screen, 0x165)));
    text(12, entry(kGamePageRow4Texts, read_field<std::uint32_t>(screen, 0x168)));
    host.renderer_set_gamma_f0(host.renderer_00f8d394(), read_field<float>(screen, 0x13C));
}

void* options_settings_copy_005f52c0(void* dst, const void* src, SettingsCopyHost& host) {
    static constexpr std::size_t kDwords[] = {0x04, 0x0C, 0x14, 0x18, 0x20, 0x28, 0x2C, 0x30, 0x34, 0x3C, 0x50,
                                              0x54, 0x58, 0x5C, 0x64, 0x68, 0x70, 0x78, 0x80, 0x88, 0x90};
    static constexpr std::size_t kBytes[] = {0x08, 0x09, 0x10, 0x1C, 0x1D, 0x1E, 0x24, 0x38, 0x40, 0x41, 0x42,
                                             0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x60,
                                             0x6C, 0x74, 0x7C, 0x84, 0x85, 0x86, 0x8C, 0x8D, 0x94, 0x95};
    for (std::size_t offset : kDwords) write_field(dst, offset, read_field<std::uint32_t>(src, offset));
    for (std::size_t offset : kBytes) write_field(dst, offset, read_field<std::uint8_t>(src, offset));
    host.native_string_vector_copy_00543e50(at(dst, 0x98), static_cast<const std::byte*>(src) + 0x98);
    host.native_string_vector_copy_00543e50(at(dst, 0xA4), static_cast<const std::byte*>(src) + 0xA4);
    for (std::size_t offset : {0xB0u, 0xB1u, 0xB2u}) write_field(dst, offset, read_field<std::uint8_t>(src, offset));
    void* text = at(dst, 0xB4);
    const auto* source = static_cast<const std::byte*>(src) + 0xB4;
    if (text != source) {
        host.native_string_resize_0041dd40(text, read_field<std::uint32_t>(source, 0), 1);
        if (read_field<std::uint32_t>(source, 0) != 0)
            host.memcpy_00bf7680(read_field<void*>(text, 4), read_field<const void*>(source, 4),
                                 read_field<std::uint32_t>(text, 0));
    }
    return dst;
}

void options_sync_profile_values_005f6ef0(void* screen, OptionsProfileSyncHost& host) {
    void* vector = at(screen, 0xC4);
    bit_vector_clear_005f6190(vector, host);
    for (std::int32_t pass = 0; pass < 1; ++pass) {
        bool shared = false;
        if (pass == 0) {
            shared = host.input_actions_share_binding_00a92910(host.input_manager_004bec00(), 0xA4, 0x118) ||
                     host.input_actions_share_binding_00a92910(host.input_manager_004bec00(), 0xA4, 0x119);
        }
        const std::uint32_t word = read_field<std::uint32_t>(vector, 0x8);
        if (word > read_field<std::uint32_t>(vector, 0xC)) host.invalid_parameter_00bf6713();
        std::array<std::uint32_t, 3> where{0, word, 0};
        if (vector == nullptr) host.invalid_parameter_00bf6713();
        where[0] = host.address_of(vector);
        const std::uint32_t count = read_field<std::uint32_t>(vector, 0x0);
        if (count > 0) host.bit_iterator_advance_0048d3b0(where, count);
        std::array<std::uint32_t, 3> out{};
        host.bit_vector_insert_0049e150(vector, out, where, shared);
    }
}

namespace {
// "slider0" + i (004263B0) + "_Icon" (004261A0): "slider01_Icon" .. "slider09_Icon".
std::string slider_icon_name(std::int32_t i) {
    return std::string(kSliderIconPrefix) + std::to_string(i) + std::string(kSliderIconSuffix);
}
}  // namespace

std::int32_t control_slider_step_from_pointer_005f2290(void* widget, OptionsScreenHost& host) {
    void* manager = host.control_binding_manager_00f8bbf4();
    void* front = nullptr;
    const auto begin = read_field<std::uintptr_t>(manager, 0x94);
    if (begin != 0 && ((read_field<std::uintptr_t>(manager, 0x98) - begin) >> 2) != 0)
        front = read_field<void*>(reinterpret_cast<void*>(begin), 0);
    if (host.widget_kind_5c(widget) != 2 || front == nullptr) return 0;
    if (!host.control_entry_active_20(front, 0)) return 0;
    for (std::int32_t i = 1; i <= 9; ++i) {
        void* icon = host.widget_find_child_00aa7e00(widget, slider_icon_name(i), 1);
        if (!host.widget_contains_pointer_00aa6a40(icon)) continue;
        if (host.widget_state_94(icon) == 0) return i != 1 ? 1 : 0;
        if (host.widget_state_94(icon) == 1) return i != 9 ? -1 : 0;
        if (i == 1) return -1;
        return i == 9 ? 1 : 0;
    }
    return 0;
}

void control_slider_set_row_005f3750(void* row, std::int32_t position, OptionsScreenHost& host) {
    void* group = host.widget_find_child_00aa7e00(row, kRowAutoControlGroup, 1);
    for (std::int32_t i = 1; i <= 9; ++i) {
        void* icon = host.widget_find_child_00aa7e00(group, slider_icon_name(i), 1);
        const std::int32_t state = i < position ? 1 : (i == position ? 2 : 0);
        host.widget_show_88(icon, state, 0, 1.0f);
    }
}

// ---------------------------------------------------------------------------
// Multiplayer menu and pause screen pieces
// ---------------------------------------------------------------------------

void multi_main_open_friends_005e6c70(MenuMiscHost& host) {
    host.online_open_friends_154(host.online_service_00f8a2fc());
    host.set_friends_opened_00e19610(1);
    host.multi_menu_push_request_interface_00688ad0(host.multi_menu_00e198b4(), 0x10);
}

void multi_session_browser_refresh_qos_005ea810(void* browser, MenuMiscHost& host) {
    void* service = host.online_service_00f8a2fc();
    if (service == nullptr || !host.online_signed_in_198(service)) return;
    if (void* qos = read_field<void*>(browser, 0x10)) {
        host.xnet_qos_release_00a4d45e(qos);
        write_field<void*>(browser, 0x10, nullptr);
    }
    if (host.xnet_qos_service_lookup_00a4d458(0, nullptr, reinterpret_cast<void**>(at(browser, 0x10))) != 0) return;
    std::array<std::byte, 8> value{};
    std::int32_t size = 8;
    host.xnet_get_opt_00a4d452(6, value.data(), &size);
}

void hud_pause_on_widget_refresh_005fcae0(void* listener, void* widget, MenuMiscHost& host) {
    const bool on = read_field<std::uint8_t>(widget, 0xD4) != 0;
    if (host.widget_kind_5c(widget) == 3) {
        host.widget_set_alpha_4c(widget, on ? kOptionsFocusAlphaOn : kOptionsFocusAlphaOff);
        return;
    }
    if (widget == read_field<void*>(listener, 0x70)) host.scroller_end_thumb_drag_00683320(at(listener, 0x90));
    if (host.widget_kind_5c(widget) == 6) {
        host.widget_show_88(widget, read_field<std::uint8_t>(widget, 0xD4) != 0 ? 1 : 0, 0, 1.0f);
        return;
    }
    if (host.widget_kind_5c(widget) == 0x12) host.widget_check_84(widget, read_field<std::uint8_t>(widget, 0xD4) != 0);
}

void multi_lobby_select_available_scene_005e20a0(MenuMiscHost& host) {
    std::int32_t index = read_field<std::int32_t>(host.game_00e188a8(), 0x60C) % host.scene_count_00f1af2c();
    const std::int32_t two = 2;
    for (std::int32_t tried = 0; tried < host.scene_count_00f1af2c(); ++tried) {
        void* list = at(read_field<void*>(host.scene_catalog_00e198ac(), 0x5C), 0x24);
        const auto begin = read_field<std::uintptr_t>(list, 0x4);
        if (begin == 0 ||
            static_cast<std::uint32_t>(index) >=
                static_cast<std::uint32_t>(static_cast<std::int32_t>(read_field<std::uintptr_t>(list, 0x8) - begin) / 0x434))
            host.invalid_parameter_00bf6713();
        auto* entry = reinterpret_cast<std::byte*>(begin + static_cast<std::uintptr_t>(index) * 0x434);
        const std::int32_t mode = host.lobby_mode_00e19564();
        void* tree = read_field<std::uint8_t>(entry, 0x424) != 0 ? static_cast<void*>(entry + 0x428)
                                                                 : at(host.lobby_option_registry_00576b10(), 0x28);
        void* value = host.int_tree_subscript_005c6850(tree, &two, &mode);
        if (*host.map_subscript_005c49c0(value) == 1) break;
        const std::int32_t count = host.scene_count_00f1af2c();
        index = (count + index + 1) % count;
    }
    void* game = host.game_00e188a8();
    if (index == read_field<std::int32_t>(game, 0x60C)) return;
    host.set_selected_scene_00e19594(index);
    host.game_select_scene_record_004c6890(game, index);
}

void multi_mode_selector_begin_join_005eac60(void* screen, MenuMiscHost& host) {
    host.session_set_mode_0076fad0(at(host.game_00e188a8(), 0x1EF0), 1);
    void* settings = at(host.game_00e188a8(), 0x1FF0);
    if (!host.online_signed_in_198(host.online_service_00f8a2fc())) {
        write_field<std::int32_t>(screen, 0xF8, 2);
        return;
    }
    void* sessions = host.online_session_list_80(host.online_service_00f8a2fc());
    void* head = read_field<void*>(sessions, 0x4);
    if (read_field<void*>(head, 0x0) == head) host.invalid_parameter_00bf6713();  // front() of an empty list
    if (host.join_set_mode5_flag_00e08700()) host.mode_selector_set_mode_005d5660(5);
    write_field<std::int32_t>(settings, 0x38, host.game_effective_mode_004bca50(host.game_00e188a8()));
    auto max_u32 = [](std::uint32_t a, std::uint32_t b) { return a < b ? b : a; };
    write_field<std::uint32_t>(settings, 0x7C,
                               max_u32(read_field<std::uint32_t>(host.xenon_system_manager_00f8abe8(), 0x35C),
                                       read_field<std::uint32_t>(host.game_00e188a8(), 0x740)));
    write_field<std::uint32_t>(settings, 0x78,
                               max_u32(read_field<std::uint32_t>(host.xenon_system_manager_00f8abe8(), 0x358),
                                       read_field<std::uint32_t>(host.game_00e188a8(), 0x73C)));
    void* game = host.game_00e188a8();
    write_field<std::uint32_t>(settings, 0x80, read_field<std::uint32_t>(game, 0x698));
    write_field<std::uint32_t>(settings, 0x84, read_field<std::uint32_t>(game, 0x69C));
    if (host.online_create_session_16c(host.online_service_00f8a2fc(), settings))
        write_field<std::int32_t>(screen, 0xF8, 1);
}

void multi_mode_selector_widget_refresh_005ea990(void* listener, void* widget, MenuMiscHost& host) {
    if (host.widget_kind_5c(widget) == 3) {
        auto code = [&] {
            const auto* event = read_field<const std::uint8_t*>(widget, 0xF0);
            return event != nullptr ? *event : host.default_event_code_00e19618();
        };
        if (code() != 0xA2 && code() != 0xA3) return;
        host.widget_set_alpha_4c(widget, read_field<std::uint8_t>(widget, 0xD4) != 0 ? kOptionsFocusAlphaOn
                                                                                      : kOptionsFocusAlphaOff);
        return;
    }
    if (read_field<std::uint8_t>(listener, 0xD4) != 0) {
        if (widget != read_field<void*>(listener, 0xA4) && widget != read_field<void*>(listener, 0xA8)) return;
    } else if (widget == read_field<void*>(listener, 0x28) || widget == read_field<void*>(listener, 0x24)) {
        host.scroller_set_auto_scroll_00683300(at(listener, 0x30), 0, 0.0f, 0);
    } else if (widget == read_field<void*>(listener, 0x90)) {
        host.scroller_end_thumb_drag_00683320(at(listener, 0x30));
    }
    if (host.widget_kind_5c(widget) == 6) {
        host.widget_show_88(widget, read_field<std::uint8_t>(widget, 0xD4) != 0 ? 1 : 0, 0, 1.0f);
        return;
    }
    if (host.widget_kind_5c(widget) == 0x12) host.widget_check_84(widget, read_field<std::uint8_t>(widget, 0xD4) != 0);
}

void multi_lobby_update_players_title_005e4fc0(void* screen, MenuMiscHost& host) {
    const std::string count = host.lobby_settings_option_label_005e2320(0);
    const std::string text = std::string(kLobbyPlayersTitle) + count;  // 004261A0
    host.text_set_localised_source_00abaed0(read_field<void*>(screen, 0x210), text, 1);
}

void multi_mode_selector_step_option_005ecba0(void* screen, std::int32_t delta, MenuMiscHost& host) {
    if (delta == 0) return;
    const std::int32_t row = host.listbox_selected_index_00a9c920(read_field<void*>(screen, 0xA8));
    if (row == -1) return;
    const std::int32_t step = delta > 0 ? 1 : -1;
    auto field = [&](std::size_t offset) { return read_field<std::int32_t>(screen, offset); };
    auto set = [&](std::size_t offset, std::int32_t value) { write_field<std::int32_t>(screen, offset, value); };
    auto rank_cap = [&] {
        return static_cast<std::int32_t>(host.session_rank_setting_00574050(at(host.game_00e188a8(), 0x650))) - 1;
    };
    switch (row) {
    case 0:
        set(0xE4, field(0xE4) + step);
        if (field(0xE4) > 8) {
            set(0xE4, 8);
        } else if (field(0xE4) < 2) {
            set(0xE4, 2);
        }
        if (field(0xE8) >= field(0xE4)) set(0xE8, field(0xE4) - 1);
        break;
    case 1:
        set(0xE8, field(0xE8) + step);
        if (field(0xE8) >= field(0xE4)) {
            set(0xE8, field(0xE4) - 1);
        } else if (field(0xE8) < 0) {
            set(0xE8, 0);
        }
        break;
    case 2: {
        set(0xF0, field(0xF0) + step);
        std::int32_t cap = rank_cap();
        if (field(0xF0) > (field(0xEC) < cap ? field(0xEC) : cap)) {
            cap = rank_cap();
            set(0xF0, field(0xEC) < cap ? field(0xEC) : cap);
        } else if (field(0xF0) < 0) {
            set(0xF0, 0);
        }
        break;
    }
    case 3:
        set(0xEC, field(0xEC) + step);
        if (field(0xEC) > 0x13) {
            set(0xEC, 0x13);
        } else if (field(0xEC) < field(0xF0)) {
            set(0xEC, field(0xF0));
        }
        break;
    default:
        break;
    }
    host.mode_selector_refresh_server_info_005ebdd0(screen);
}

void lobby_settings_step_option_005e21b0(std::int32_t slot, MenuMiscHost& host) {
    void* game = host.game_00e188a8();
    if (read_field<std::int32_t>(game, 0x1FE4) == 2 || read_field<std::int32_t>(game, 0x5FC) == 0) return;
    const std::int32_t old_value = host.lobby_option_value_00e1955c(slot);
    const std::int32_t key = slot;
    void* record = host.lobby_option_record_00575e80(at(host.lobby_option_registry_00576b10(), 0x10), &key);
    const auto count = read_field<std::uint32_t>(record, 0x8);
    auto advance = [&] {
        const auto value = static_cast<std::uint32_t>(host.lobby_option_value_00e1955c(slot));
        const auto next = static_cast<std::int32_t>((value + count + 1) % count);
        host.set_lobby_option_value_00e1955c(slot, next);
        return next;
    };
    std::int32_t value = advance();
    void* first = host.int_tree_subscript_005c6850(host.lobby_option_table_005e1990(), &key, &value);
    if (*host.map_subscript_005c49c0(first) != 0) return;
    std::uint32_t tried = 0;
    record = host.lobby_option_record_00575e80(at(host.lobby_option_registry_00576b10(), 0x10), &key);
    if (read_field<std::uint32_t>(record, 0x8) != 0) {
        for (;;) {
            value = advance();
            void* table = nullptr;
            void* catalog = host.scene_catalog_00e198ac();
            if (catalog != nullptr && read_field<void*>(catalog, 0x5C) != nullptr) {
                void* list = at(read_field<void*>(catalog, 0x5C), 0x24);
                const std::int32_t index = host.scene_index_005d7070(host.selected_scene_00e19594());
                auto* entry = static_cast<std::byte*>(host.scene_list_at_004b4e60(list, index));
                if (read_field<std::uint8_t>(entry, 0x424) != 0) table = entry + 0x428;
            }
            if (table == nullptr) table = at(host.lobby_option_registry_00576b10(), 0x28);
            if (*host.map_subscript_005c49c0(host.int_tree_subscript_005c6850(table, &key, &value)) == 1) return;
            ++tried;
            record = host.lobby_option_record_00575e80(at(host.lobby_option_registry_00576b10(), 0x10), &key);
            if (tried >= read_field<std::uint32_t>(record, 0x8)) break;
        }
    }
    host.set_lobby_option_value_00e1955c(slot, old_value);
}

namespace {
// The empty-list guard shared by 005E7608..005E7611 / 005E7818..005E7821:
// head = list+4h, front = [head]; front == head calls 00BF6713.
void* online_front_session(MenuMiscHost& host) {
    void* list = host.online_session_list_80(host.online_service_00f8a2fc());
    void* head = read_field<void*>(list, 0x4);
    void* front = read_field<void*>(head, 0x0);
    if (front == head) host.invalid_parameter_00bf6713();
    return front;
}

void copy_session_address(void* front, MenuMiscHost& host) {
    void* profile = at(host.game_00e188a8(), 0x650);
    write_field<std::uint32_t>(profile, 0x48, read_field<std::uint32_t>(front, 0x28));
    write_field<std::uint32_t>(profile, 0x4C, read_field<std::uint32_t>(front, 0x2C));
}
}  // namespace

bool main_menu_check_live_connection_005e74b0(MenuMiscHost& host) {
    if (!host.online_connected_14(host.online_service_00f8a2fc())) {
        host.raise_prompt_00531b00(2, kCableLostLive, 2, 0, 0, "", 0.0f, 0, "", 1);
        return false;
    }
    host.online_set_live_mode_1a0(host.online_service_00f8a2fc(), 1);
    write_field<std::uint8_t>(host.online_service_00f8a2fc(), 0x4D, 0);
    host.online_reset_d4(host.online_service_00f8a2fc(), 1);
    const std::uint32_t value = host.game_value_004bc830(host.game_00e188a8());
    host.online_set_e0(host.online_service_00f8a2fc(), value);
    void* front = online_front_session(host);
    copy_session_address(front, host);
    write_field<std::uint8_t>(host.game_00e188a8(), 0x6A9, 0);
    write_field<std::uint8_t>(host.game_00e188a8(), 0x6A8, read_field<std::uint8_t>(front, 0x38));
    host.player_profile_set_display_name_007f9340(at(host.game_00e188a8(), 0x650), "");
    host.game_request_state_004d7920(host.game_00e188a8(), 6);
    host.game_request_state_004d7920(host.game_00e188a8(), 7);
    return true;
}

bool main_menu_check_live_sign_in_005e76d0(MenuMiscHost& host) {
    void* manager = host.xenon_system_manager_00f8abe8();
    if (!host.xenon_has_selected_user_00a3e510(manager) || host.xenon_sign_in_state_00a3ead0(manager) != 2) {
        host.raise_prompt_00531b00(2, kRequiresLive, 2, 0, 0, "", 0.0f, 0, "", 1);
        return false;
    }
    host.online_set_live_mode_1a0(host.online_service_00f8a2fc(), 0);
    host.online_reset_d8(host.online_service_00f8a2fc());
    host.online_refresh_38(host.online_service_00f8a2fc());
    void* front = online_front_session(host);
    copy_session_address(front, host);
    // MSVC std::string at +0Ch: the inline buffer while capacity (+20h) < 10h.
    const char* name = read_field<std::uint32_t>(front, 0x20) < 0x10
                           ? reinterpret_cast<const char*>(at(front, 0xC))
                           : read_field<const char*>(front, 0xC);
    host.player_profile_set_display_name_007f9340(at(host.game_00e188a8(), 0x650), name);
    std::uint8_t scratch = 0;  // ESP+0Bh, not initialised in the image
    void* profile = at(host.game_00e188a8(), 0x650);
    write_field<std::uint8_t>(profile, 0x59,
                              host.xenon_communication_privilege_00a3eb30(host.xenon_system_manager_00f8abe8(), &scratch));
    write_field<std::uint8_t>(host.game_00e188a8(), 0x6A8, read_field<std::uint8_t>(front, 0x38));
    host.game_request_state_004d7920(host.game_00e188a8(), 6);
    host.game_request_state_004d7920(host.game_00e188a8(), 7);
    return true;
}

void multi_main_refresh_online_state_005e7900(void* screen, MultiMainOnlineHost& host) {
    if (host.online_signed_in_198(host.online_service_00f8a2fc()) &&
        read_field<std::uint8_t>(read_field<void*>(screen, 0x24), 0x85) != 0) {
        if (read_field<std::uint8_t>(read_field<void*>(screen, 0x24), 0x77) != 0) {
            const float scale[2] = {kOptionsFocusAlphaOff, kOptionsFocusAlphaOff};  // [00D7A24C] = 1.0f
            host.widget_set_scale_48(read_field<void*>(screen, 0xE8), scale);
            host.widget_enable_34(read_field<void*>(screen, 0xD4), 0);
        }
        host.widget_focusable_60(read_field<void*>(screen, 0x20), 1);
        host.widget_enable_34(read_field<void*>(screen, 0x20), 1);
        write_field<std::uint8_t>(read_field<void*>(screen, 0x20), 0x77, 0);
        host.widget_focusable_60(read_field<void*>(screen, 0x24), 0);
        host.widget_enable_34(read_field<void*>(screen, 0x24), 0);
        write_field<std::uint8_t>(read_field<void*>(screen, 0x24), 0x77, 1);
        host.multi_main_set_buttons_visible_005e6b10(screen, 0);
        const char* help = host.help_table_entry_00e0896c(
            host.listbox_selected_data_00a9c990(read_field<void*>(screen, 0x20)));
        host.command_bar_set_help_line_0054a0c0(host.command_bar_00e1930c(), help);
        return;
    }
    host.online_set_live_mode_1a0(host.online_service_00f8a2fc(), 2);
    host.online_sign_out_dc(host.online_service_00f8a2fc());
    host.player_profile_set_display_name_007f9340(at(host.game_00e188a8(), 0x650), "");
    host.game_request_state_004d7920(host.game_00e188a8(), 9);
    host.game_request_state_004d7920(host.game_00e188a8(), 4);
    write_field<std::int32_t>(screen, 0xF4, -1);
}

void lobby_show_selection_hint_005e19e0(void* listener, void* listbox, LobbyHintHost& host) {
    void* settings = read_field<void*>(listener, 0x220);
    if (read_field<std::uint8_t>(settings, 0x85) != 0 && listbox != settings) return;
    if (listbox == read_field<void*>(listener, 0x8C) || listbox != settings) return;
    const std::int32_t row = host.listbox_selected_index_00a9c920(listbox);
    write_field<std::int32_t>(listener, 0x26C, row);
    if (row == -1) return;
    const std::int32_t kind = read_field<std::int32_t>(listener, 0x1B4 + static_cast<std::size_t>(row) * 4);
    void* text = read_field<void*>(listener, 0x224);
    if (kind == 1) {
        void* scene = host.lobby_current_scene_005d86b0(static_cast<std::byte*>(listener) - 8);
        std::string hint(read_field<const char*>(scene, 0x98 + 4), read_field<std::uint32_t>(scene, 0x98));  // 00426060
        const std::int32_t mode = host.lobby_mode_00e19564();
        if (static_cast<std::uint32_t>(mode) <= 3) {
            if (static_cast<std::uint32_t>(host.lobby_submode_00e19568()) <= 3) hint = "mp.hint_ic_desc_map";
        } else {
            switch (mode) {
            case 4: hint = "mp.hint_duel_desc_map"; break;  // 0041E350
            case 5: hint += "_escort_desc_map"; break;      // 00425E10
            case 6: hint += "_siege_desc_map"; break;
            case 7: hint += "_comp_desc_map"; break;
            default: break;
            }
        }
        host.text_set_localised_source_00abaed0(text, hint, 1);
        return;
    }
    if (kind == 0xD) {
        host.text_set_localised_cstring_00abbe50(text, "FE.multi_weather_desc", 1);
        return;
    }
    if (kind == 8 && static_cast<std::uint32_t>(host.lobby_mode_00e19564()) <= 3) {
        host.text_set_localised_cstring_00abbe50(text, "FE.multi_victorypointlimit_desc", 1);
        return;
    }
    const std::int32_t value = host.lobby_option_value_00e1955c(kind);
    void* record = host.lobby_option_record_00575e80(at(host.lobby_option_registry_00576b10(), 0x34), &kind);
    const char* const* entry = host.lobby_option_text_005732c0(record, &value);
    host.text_set_localised_cstring_00abbe50(text, *entry, 1);
}

void options_confirm_changes_005f7050(void* screen, bool video_changed, OptionsScreenHost& host) {
    const bool dirty = host.settings_dirty_00f88a30();
    const bool content_changed = read_field<std::uint8_t>(screen, 0x252) != 0;
    if (!dirty && !content_changed && !video_changed) {
        host.options_commit_changes_005f65c0(screen);
    } else if (content_changed) {
        host.raise_prompt_00531b00(0, kDlcChangedConfirm, 1, kPromptOptionsApply, 0, "", 0.0f, 0, "", 1);
    } else if (video_changed) {
        host.raise_prompt_00531b00(0, kVideoResolutionConfirm, 1, kPromptVideoConfirm, 0, "",
                                   kResolutionRevertSeconds, 0, kResolutionRevertTimer, 1);
    } else if (host.xenon_has_selected_user_00a3e510(host.xenon_system_manager_00f8abe8())) {
        host.raise_prompt_00531b00(0, kOptionsSaveConfirm, 1, kPromptOptionsApply, 0, "", 0.0f, 0, "", 1);
    } else {
        host.options_commit_changes_005f65c0(screen);
    }
    write_field<float>(screen, 0x260, kOptionsConfirmDelay);
}

namespace {
// The inline NativeString assignment (005F54B0.., 005FDD9A..): same object ->
// nothing; else resize to the source length and copy the destination length.
void assign_native_string(void* dst, const void* src, SettingsCopyHost& host) {
    if (dst == src) return;
    host.native_string_resize_0041dd40(dst, read_field<std::uint32_t>(src, 0), 1);
    if (read_field<std::uint32_t>(src, 0) != 0)
        host.memcpy_00bf7680(read_field<void*>(dst, 4), read_field<const void*>(src, 4), read_field<std::uint32_t>(dst, 0));
}
}  // namespace

void* hint_definition_assign_005fdd90(void* dst, const void* src, HintDefinitionAssignHost& host) {
    const auto* s = static_cast<const std::byte*>(src);
    for (std::size_t offset = 0; offset < 0x50; offset += 8) assign_native_string(at(dst, offset), s + offset, host);
    write_field(dst, 0x50, read_field<std::uint8_t>(src, 0x50));
    for (std::size_t entry = 0; entry < 8; ++entry)
        host.hint_entry_assign_005fd6f0(at(dst, 0x54 + entry * 0x18), s + 0x54 + entry * 0x18);
    for (std::size_t offset : {0x114u, 0x115u, 0x116u}) write_field(dst, offset, read_field<std::uint8_t>(src, offset));
    assign_native_string(at(dst, 0x118), s + 0x118, host);
    write_field(dst, 0x120, read_field<std::uint32_t>(src, 0x120));
    write_field(dst, 0x124, read_field<std::uint32_t>(src, 0x124));
    return dst;
}

// ---------------------------------------------------------------------------
// cc13-x87b. The x87 code was traced from the bytes with the stack depth at
// each step (docs/CC13_SEG23.md, "x87 bodies"). A value the image keeps in an
// x87 register is a double here and is rounded to float where the image
// stores it (the 53/64-bit result); at the main thread's 24-bit precision
// control those intermediates also round to 24 bits.
// ---------------------------------------------------------------------------
namespace {
constexpr float kAwardScrollStep = 0.004000000189989805f;   // 00CEC178 (float 3B83126Fh)
constexpr float kAwardFocusScale = 1.25f;                   // 00CF29A8 (float)
constexpr float kAwardFrameZ = -100.0f;                     // 00CE65D8 (float)
constexpr double kAwardCellWidth = 0.08437500149011612;     // 00CF29A0 (double, 0.084375f widened)
constexpr double kAwardCellHeight = 0.10277800261974335;    // 00CF2998 (double, float widened)
constexpr float kAwardNegZero = -0.0f;                      // 00D7A208 (float), SUBSS minuend
constexpr float kAwardOne = 1.0f;                           // FLD1 / 00D7A24C

// FLD (widget - frame) stored as float; 004396F0 (floor, plus 1 above a half);
// 00BF7420 _ftol2 truncates the ST0 result and EAX keeps the low dword. The
// rounded value is integral, so the truncation is exact for any in-range cell.
std::int32_t award_cell(float widget, float frame, double size) {
    const float scaled = static_cast<float>((static_cast<double>(widget) - frame) / size);
    return static_cast<std::int32_t>(bsp::ship_screen_gauge_round_004396f0(scaled));
}

// -0.0f - x by SUBSS unless x > 0 (the comparison NaN takes the SUBSS path).
float award_magnitude(float x) {
    return x > 0.0f ? x : kAwardNegZero - x;
}
}  // namespace

// 005E97D0. The kind is re-read through vtable +5Ch for each test, as the
// image does.
void multi_main_listener_on_gui_event_005e97d0(void* listener, void* widget, MultiMainEventHost& host) {
    const auto event_code = [&] {
        const auto* code = read_field<const std::uint8_t*>(widget, 0xF0);
        return code != nullptr ? *code : host.default_event_code_00e1960c();
    };
    if (host.widget_kind_5c(widget) == 3) {
        host.widget_set_alpha_4c(widget, kAwardOne);
        if (event_code() == 0xA2) {
            void* first = read_field<void*>(listener, 0x18);
            if (read_field<std::uint8_t>(first, 0x77) == 0) {
                host.listener_vslot04(at(listener, 0) - 4, widget, first);
            } else {
                void* second = read_field<void*>(listener, 0x14);
                if (read_field<std::uint8_t>(second, 0x77) == 0) {
                    host.listener_vslot04(at(listener, 0) - 4, widget, second);
                }
            }
        } else if (event_code() == 0xA3) {
            multi_main_refresh_online_state_005e7900(at(listener, 0) - 0x0C, host);
        }
    } else if (host.widget_kind_5c(widget) == 0x12) {
        if (widget == read_field<void*>(listener, 0xC0)) {
            host.scroller_begin_thumb_drag_00683a40(at(listener, 0x60));
            host.widget_vslot84(widget, 2);
        }
    } else if (host.widget_kind_5c(widget) == 6) {
        std::int32_t direction = 0;
        if (widget == read_field<void*>(listener, 0x58)) {
            direction = 2;
        } else if (widget == read_field<void*>(listener, 0x54)) {
            direction = 1;
        }
        if (direction != 0) {
            host.scroller_set_auto_scroll_00683300(at(listener, 0x60), direction, kAwardScrollStep, 1);
            host.widget_show_88(widget, 2, 0, kAwardOne);
        }
    }
    if (read_field<std::uint8_t>(widget, 0x76) == 0) return;

    void* record = host.award_table_find_record_0050fc30(at(host.award_manager_00e19900(), 0x10),
                                                         host.widget_vslot28(widget));
    alignas(4) std::byte copy[0x4C];
    host.award_record_copy_construct_0050a210(copy, record);
    const float unit_scale[2] = {kAwardOne, kAwardOne};
    host.widget_set_scale_48(read_field<void*>(listener, 0xDC), unit_scale);
    const float focus_scale[2] = {kAwardFocusScale, kAwardFocusScale};
    host.widget_set_scale_48(widget, focus_scale);
    host.widget_set_local_position_00aa7dc0(read_field<void*>(listener, 0xC8),
                                            host.widget_local_position_00aa67f0(widget));
    host.widget_set_local_z_00aa7910(read_field<void*>(listener, 0xC8), kAwardFrameZ);
    host.widget_enable_34(read_field<void*>(listener, 0xC8), 1);
    host.text_set_localised_native_00abaed0(read_field<void*>(listener, 0x3C), copy, 1);
    host.text_set_localised_native_00abaed0(read_field<void*>(listener, 0x40), copy + 0x0C, 1);

    const float widget_x = host.widget_local_position_00aa67f0(widget)[0];
    const float frame_x = host.widget_local_position_00aa67f0(read_field<void*>(listener, 0xD0))[0];
    const std::int32_t column = award_cell(widget_x, frame_x, kAwardCellWidth);
    const float widget_y = host.widget_local_position_00aa67f0(widget)[1];
    const float frame_y = host.widget_local_position_00aa67f0(read_field<void*>(listener, 0xD0))[1];
    const std::int32_t row = award_cell(widget_y, frame_y, kAwardCellHeight);
    write_field<std::int32_t>(listener, 0xD4, column);
    write_field<std::int32_t>(listener, 0xD8, row);

    // 005E9A91: the scroll offset of smallest magnitude among e, e - h and
    // (e - h) - h (e - h kept in ST for the second); ties keep the later one.
    const float resolved_y = host.widget_resolved_position_00aa6750(widget)[1];
    const float e = resolved_y - read_field<float>(listener, 0xE0);
    const double e_less = static_cast<double>(e) - kAwardCellHeight;
    const float a = static_cast<float>(e_less);
    const float b = static_cast<float>(e_less - kAwardCellHeight);
    const float magnitude_e = award_magnitude(e);   // FCOMIP e, 0; JBE
    const float magnitude_a = award_magnitude(a);   // COMISS
    float offset = e;
    if (!(magnitude_a > magnitude_e)) {             // FCOMIP; JA keeps e
        const float magnitude_b = award_magnitude(b);
        offset = magnitude_b > magnitude_a ? a : b;  // FCOMIP; JBE stores b (NaN too)
    }
    write_field<float>(listener, 0xE4, offset);
    write_field<void*>(listener, 0xDC, widget);
    write_field<std::uint8_t>(read_field<void*>(listener, 0x14), 0x77, 1);
    write_field<std::uint8_t>(read_field<void*>(listener, 0x18), 0x77, 1);
    host.award_record_destroy_00501fa0(copy);
}

namespace {
constexpr double kCheatPageRows = 16.0;        // 00CED9F8 (double)
constexpr std::uint32_t kCheatNameLimit = 0x14;

// The inlined NativeString destructor: a non-null buffer goes back to the
// sized pool with size + 1.
void release_cheat_string(const std::byte* string, PauseCheatListHost& host) {
    void* data = read_field<void*>(string, 4);
    if (data != nullptr) {
        const std::uint32_t bytes = read_field<std::uint32_t>(string, 0) + 1;
        void* pool = host.sized_pool_singleton_00419cc0(data, bytes, 1);
        host.sized_pool_return_block_00bd1510(pool, data, bytes, 1);
    }
}
}  // namespace

// 00601F80. The checked-iterator tests on the tracker map compare the
// iterator with the map it was taken from, or the node with the head the loop
// has just excluded, so they cannot fire and are omitted.
void pause_cheat_build_list_00601f80(void* screen, PauseCheatListHost& host) {
    host.listbox_clear_items_00a9bf90(read_field<void*>(screen, 0x2C));
    std::byte* names = at(screen, 0x11C);
    host.tree_erase_005fe9e0(names, read_field<void*>(read_field<void*>(names, 4), 4));
    void* head = read_field<void*>(names, 4);
    write_field<void*>(head, 4, head);
    write_field<std::uint32_t>(names, 8, 0);
    write_field<void*>(read_field<void*>(names, 4), 0, read_field<void*>(names, 4));
    write_field<void*>(read_field<void*>(names, 4), 8, read_field<void*>(names, 4));
    std::int32_t index = 0;

    // The iterator {map, node} at ESP+20h; begin is the head's left link.
    alignas(4) std::byte iterator[8];
    std::byte* tracker_map = at(host.award_tracker_004e1ca0(), 0x94);
    write_field<void*>(iterator, 0, tracker_map);
    write_field<void*>(iterator, 4, read_field<void*>(read_field<void*>(tracker_map, 4), 0));
    for (;;) {
        void* end = read_field<void*>(host.award_tracker_004e1ca0(), 0x98);
        void* node = read_field<void*>(iterator, 4);
        if (node == end) break;
        void* row = host.widget_clone_subtree_00aab4c0(read_field<void*>(screen, 0x44), 0);
        const std::byte* name = at(node, 0x0C);
        host.text_set_localised_native_00abaed0(row, name, 1);
        void* slot = host.int_string_map_subscript_005ff5d0(names, &index);
        if (slot != name) {
            host.native_string_resize_0041dd40(slot, read_field<std::uint32_t>(name, 0), 1);
            if (read_field<std::uint32_t>(name, 0) != 0) {
                host.memcpy_00bf7680(read_field<void*>(slot, 4), read_field<void*>(name, 4),
                                     read_field<std::uint32_t>(slot, 0));
            }
        }
        ++index;
        const void* wide = read_field<const void*>(row, 0xF0);
        if (wide == nullptr) wide = host.empty_wide_string_00e19688();
        alignas(4) std::byte text[8];
        host.native_string_construct_from_wide_00436630(text, wide);
        if (read_field<std::int32_t>(row, 0xEC) > static_cast<std::int32_t>(kCheatNameLimit)) {
            alignas(4) std::byte ellipsis[8] = {};
            host.native_string_resize_0041dd40(ellipsis, 3, 1);
            if (read_field<void*>(ellipsis, 4) != nullptr) {
                host.memcpy_00bf7680(read_field<void*>(ellipsis, 4), "...",  // 00CF3F98
                                     read_field<std::uint32_t>(ellipsis, 0) + 1);
            }
            alignas(4) std::byte head_text[8];
            alignas(4) std::byte joined[8];
            void* first = host.native_string_substring_00469840(text, head_text, 0, kCheatNameLimit);
            void* both = host.native_string_concat_004261a0(first, joined, ellipsis);
            host.text_set_localised_native_00abaed0(row, both, 1);
            release_cheat_string(joined, host);
            release_cheat_string(head_text, host);
            release_cheat_string(ellipsis, host);
        }
        host.listbox_insert_row_00a9d750(read_field<void*>(screen, 0x2C), row, 0, 0);
        release_cheat_string(text, host);
        host.tree_iterator_increment_004b7de0(iterator);
    }

    host.listbox_enable_paging_00a9e230(read_field<void*>(screen, 0x2C), 0x10);
    const std::int32_t rows = read_field<std::int32_t>(read_field<void*>(screen, 0x2C), 0x154);
    std::byte* thumb = at(screen, 0x94);
    if (rows <= 0x10) {
        const float width = host.widget_size_00aa6740(read_field<void*>(screen, 0x80))[0];
        write_field<float>(thumb, 4, read_field<float>(screen, 0x84));
        write_field<float>(thumb, 0, width);
    } else {
        const float width = host.widget_size_00aa6740(read_field<void*>(screen, 0x80))[0];
        // FILD rows; FDIVR 16.0; FMUL [+84h]: kept in ST, stored once.
        write_field<float>(thumb, 4, static_cast<float>(kCheatPageRows / static_cast<double>(rows) *
                                                        static_cast<double>(read_field<float>(screen, 0x84))));
        write_field<float>(thumb, 0, width);
    }
    host.widget_vslot58(read_field<void*>(screen, 0x80), reinterpret_cast<const float*>(thumb));
}

}  // namespace bsp::seg23_cc13
