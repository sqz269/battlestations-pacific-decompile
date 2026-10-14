// Packet cc13-seg56: script bindings of segment 56. See include/bsp/options_seg56_cc13.hpp and
// docs/CC13_SEG56.md for addresses, ABI and evidence. Comments cite the instruction each
// statement stands for.
#include "bsp/options_seg56_cc13.hpp"

namespace bsp::seg56_options_cc13 {
namespace {

void* game(ImageAccess& image)
{
    return image.global_ptr(kGameGlobal);
}

void* support_panel(ImageAccess& image)
{
    return image.read_ptr(image.global_ptr(kInGameInterfaceGlobal), 0xBCu);
}

int send_support_manager_ban(bool banned, ImageAccess& image, Seg56Host& host)
{
    host.support_manager_set_banned_00a38da0(banned); // MOV CL,BL / XOR CL,CL, CALL 00A38DA0
    if (image.read_i32(game(image), kSessionModeOffset) == 1) { // CMP [EDX+1FE4h],EBX(1)
        SupportManagerBanMessage message;
        message.banned = banned; // byte +18h
        host.session_send_0076a9f0(message);
    }
    return 0;
}

int set_plane_gears(LuaArgs& args, ImageAccess& image, Seg56Host& host, std::int32_t flag)
{
    void* squadron = args.entity_at(0);
    const std::int32_t mode = args.get_integer(1);
    // CMP [EDI+3CCh],ESI / JLE at the top, re-read at the bottom of every pass.
    for (std::int32_t slot = 0; slot < image.read_i32(squadron, 0x3CCu); ++slot) {
        // CMP ESI,4 / JA: slots past the fifth pass a null plane (XOR ECX,ECX).
        void* plane = static_cast<std::uint32_t>(slot) <= 4u
            ? image.read_ptr(squadron, 0x3D0u + 4u * static_cast<std::uint32_t>(slot))
            : nullptr;
        host.plane_set_gear_mode_007b8ce0(plane, mode, flag); // PUSH flag, PUSH EBP(mode)
    }
    return 0;
}

int spawnpoint(LuaArgs& args, Seg56Host& host, bool active)
{
    void* entity = args.entity_at(0);
    if (args.count() == 1) {                                // CMP EAX,1 / JNZ
        host.support_site_set_active_0084ce60(entity, active);
        return 0;
    }
    SpawnpointMessage message;
    message.argument_1 = args.get_integer(1); // read before 0075B430
    message.active = active;
    host.route_message_0077c2a0(entity, message, kSpawnpointRouteFlags, 0);
    return 0;
}

void* air_slot(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    void* block = host.air_ops_block_006bcd20(args.entity_at(0), false); // XOR DL,DL
    const std::int32_t index = args.get_integer(1);
    // SUB ESI,1 / IMUL ESI,ESI,58h / [EDI+4Ch] + ESI.
    auto* base = static_cast<std::uint8_t*>(image.read_ptr(block, 0x4Cu));
    return base + static_cast<std::int32_t>((index - 1) * 0x58);
}

} // namespace

// ---------------------------------------------------------------------------
// Support manager and interface glue
// ---------------------------------------------------------------------------

int lua_bann_support_manager(ImageAccess& image, Seg56Host& host)
{
    return send_support_manager_ban(true, image, host);
}

int lua_permit_support_manager(ImageAccess& image, Seg56Host& host)
{
    return send_support_manager_ban(false, image, host);
}

int lua_show_hide_support_manager(ImageAccess& image, Seg56Host& host)
{
    host.interface_toggle_tactical_overlay_0068c0b0(image.global_ptr(kInGameInterfaceGlobal), 0); // PUSH EBP(0)
    return 0;
}

int lua_support_manager_enable(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    const bool enable = args.get_boolean(1);
    if (enable) {
        host.panel_list_remove_0065bba0(support_panel(image), entity);    // 0088CD08
    } else {
        host.panel_list_push_back_00665ef0(support_panel(image), entity); // 0088CD1B
    }
    return 0;
}

int lua_is_sm_visible(LuaResults& results, ImageAccess& image, Seg56Host& host)
{
    results.push_boolean(host.panel_visible_006529e0(support_panel(image)));
    return 1;
}

int lua_unlock_all_mission()
{
    return 0; // 0088D0D9: the frame is opened and the count returned, nothing else
}

int lua_set_bpc_soda_squad_merete_hack(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    const std::int32_t value = args.get_integer(0);
    host.panel_set_dword_2b4_006529d0(support_panel(image), value);
    return 0;
}

// ---------------------------------------------------------------------------
// Failures, damage visuals, spawnpoints, numbering, formations
// ---------------------------------------------------------------------------

int lua_clear_failure(LuaArgs& args, LuaFrame& frame, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    const std::string name = args.get_string(1); // 0041E870 into a stack native string
    host.entity_clear_failure_vt19c(entity, name, frame); // CALL [vtable+19Ch](&name, &frame)
    return frame.result_count();
}

int lua_clear_all_ship_failure(LuaArgs& args, Seg56Host& host)
{
    auto* entity = static_cast<std::uint8_t*>(args.entity_at(0));
    host.failure_block_reset_0093a390(entity + 0xA20); // LEA ECX,[ESI+0A20h]
    return 0;
}

int lua_set_damaged_gfx_level(LuaArgs& args, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    host.entity_set_damaged_gfx_level_vt1b4(entity, args.get_integer(1));
    return 0;
}

int lua_get_damaged_gfx_level(LuaArgs& args, LuaResults& results, ImageAccess& image)
{
    results.push_number(image.read_i32(args.entity_at(0), 0x358u));
    return 1;
}

int lua_activate_spawnpoint(LuaArgs& args, Seg56Host& host)
{
    return spawnpoint(args, host, true);
}

int lua_deactivate_spawnpoint(LuaArgs& args, Seg56Host& host)
{
    return spawnpoint(args, host, false);
}

int lua_set_numbering(LuaArgs& args, Seg56Host& host)
{
    void* unit = args.entity_at(0);
    host.unit_set_numbering_008761e0(unit, args.get_integer(1));
    return 0;
}

int lua_set_formation_shape(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    void* group = image.read_ptr(args.entity_at(0), 0x284u);
    if (group == nullptr) { // CMP ESI,EBP / JZ 00890176
        return 0;
    }
    host.unit_group_apply_formation_shape_0070efd0(group, args.get_integer(1));
    host.unit_group_broadcast_formation_set_0077a080_0077c880(group);
    return 0;
}

int lua_get_formation_shape(LuaArgs& args, LuaResults& results, ImageAccess& image)
{
    void* group = image.read_ptr(args.entity_at(0), 0x284u);
    results.push_number(group != nullptr ? image.read_i32(group, 0x500u) : -1); // EDI = -1
    return 1;
}

// ---------------------------------------------------------------------------
// Planes, ships, submarines
// ---------------------------------------------------------------------------

int lua_set_plane_gears(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    return set_plane_gears(args, image, host, 0); // PUSH 0 at 008904A1
}

int lua_force_plane_gears(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    return set_plane_gears(args, image, host, 1); // PUSH 1 at 008909A1
}

int lua_get_plane_surely_landed(LuaArgs& args, LuaResults& results, ImageAccess& image)
{
    void* plane = args.entity_at(0);
    bool landed = false;
    if (image.read_u8(plane, 0x904u) != 0) {
        const std::int32_t state = image.read_i32(plane, 0x900u);
        landed = state == 2 || state == 1;
    }
    results.push_boolean(landed);
    return 1;
}

int lua_get_plane_landing_aborted(LuaArgs& args, LuaResults& results, ImageAccess& image, Seg56Host& host)
{
    auto* plane = static_cast<std::uint8_t*>(args.entity_at(0));
    bool aborted = false;
    if (image.read_u8(plane, 0x904u) == 0) {
        const std::int32_t state = image.read_i32(plane, 0x900u);
        aborted = state == 2 || state == 4;
    }
    if (!aborted) {
        aborted = host.plane_controller_vt38(plane + 0x72C); // LEA ECX,[ESI+72Ch], CALL [vtable+38h]
    }
    results.push_boolean(aborted);
    return 1;
}

int lua_get_ship_max_speed(LuaArgs& args, LuaResults& results, Seg56Host& host)
{
    results.push_float(static_cast<float>(host.unit_reference_speed_0080fc30(args.entity_at(0))));
    return 1;
}

int lua_get_ship_speed(LuaArgs& args, LuaResults& results, Seg56Host& host)
{
    results.push_float(static_cast<float>(host.entity_speed_vt38(args.entity_at(0)))); // FSTP float [ESP]
    return 1;
}

int lua_set_force_sellable_units(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    const bool value = args.get_boolean(0);
    host.panel_set_byte_8c_005f9470(image.read_ptr(image.global_ptr(kInGameInterfaceGlobal), 0x64u), value);
    return 0;
}

int lua_break_ship(LuaArgs& args, Seg56Host& host)
{
    void* unit = args.entity_at(0);
    if (host.parts_dword_370_0092bea0(host.unit_parts_object_0080e490(unit)) != 0
        && host.parts_dword_374_0092be90(host.unit_parts_object_0080e490(unit)) != 0) {
        host.unit_on_breakup_00814520(unit);
    }
    return 0;
}

int lua_get_num_catapulted(LuaArgs& args, LuaResults& results, Seg56Host& host)
{
    void* ship = args.entity_at(0);
    std::int32_t count = 0; // PUSH EBP
    if (ship != nullptr && host.entity_is_class_vt5c(ship, 6)) {
        count = host.ship_count_catapulted_planes_00953920(ship);
    }
    results.push_number(count);
    return 1;
}

int lua_set_catapult_stock(LuaArgs& args, Seg56Host& host)
{
    void* ship = args.entity_at(0);
    if (ship != nullptr && host.entity_is_class_vt5c(ship, 5)) {
        host.ship_set_catapult_stock_009539a0(ship, args.get_integer(1));
    }
    return 0;
}

int lua_set_pump(LuaArgs& args)
{
    static_cast<void>(args.entity_at(0));   // 008932E7, result dropped
    static_cast<void>(args.get_boolean(1)); // 00893317, result dropped
    return 0;
}

int lua_get_submarine_air_supply(LuaArgs& args, LuaResults& results, ImageAccess& image)
{
    results.push_float(image.read_f32(args.entity_at(0), 0x127Cu));
    return 1;
}

int lua_set_submarine_air_supply(LuaArgs& args, ImageAccess& image)
{
    void* submarine = args.entity_at(0);
    const float value = static_cast<float>(args.get_number(1)); // FSTP float [ESP+10h]
    image.write_f32(submarine, 0x127Cu, value);
    return 0;
}

int lua_force_submarine_periscope(LuaArgs& args, ImageAccess& image)
{
    void* submarine = args.entity_at(0);
    image.write_u8(submarine, 0x1235u, args.get_boolean(1) ? 1 : 0);
    return 0;
}

int lua_submarine_attack(LuaArgs& args, LuaCommandTargetSource& targets, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    const SceneCommandTarget target = lua_read_command_target(targets, 1);
    host.entity_issue_command_0077d600(entity, kCommandObjectAttackMoveSeg56, target, 1);
    return 0;
}

int lua_pilot_torpedo(LuaArgs& args, LuaCommandTargetSource& targets, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    const SceneCommandTarget target = lua_read_command_target(targets, 1);
    host.entity_issue_command_0077d600(entity, kCommandObjectPilotTorpedo, target, 1);
    return 0;
}

int lua_set_crew_level(LuaArgs& args, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    const std::int32_t level = args.get_integer(1);
    host.entity_set_skill_vt128(entity, host.skill_from_crew_level_006e6210(level));
    return 0;
}

int lua_get_crew_level(LuaArgs& args, LuaResults& results, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    results.push_number(host.crew_level_from_skill_006e6270(host.entity_skill_vt12c(entity)));
    return 1;
}

int lua_get_skill_level(LuaArgs& args, LuaResults& results, Seg56Host& host)
{
    results.push_number(host.entity_skill_vt12c(args.entity_at(0)));
    return 1;
}

int lua_get_air_base_status(LuaArgs& args, LuaResults& results, Seg56Host& host)
{
    void* block = host.air_ops_block_006bcd20(args.entity_at(0), false);
    results.push_number(host.air_ops_block_status_006bf5c0(block));
    return 1;
}

int lua_get_air_base_slot_status(LuaArgs& args, LuaResults& results, ImageAccess& image, Seg56Host& host)
{
    results.push_number(image.read_i32(air_slot(args, image, host), 0x2Cu));
    return 1;
}

int lua_set_air_base_slot_ready(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    host.air_ops_slot_set_ready_006ba4e0(air_slot(args, image, host));
    return 0;
}

// ---------------------------------------------------------------------------
// Shipyards, units, squadrons, navigator flags
// ---------------------------------------------------------------------------

int lua_remove_shipyard_stock(LuaArgs& args, Seg56Host& host)
{
    void* shipyard = args.entity_at(0);
    const std::int32_t class_id = args.get_integer(1);
    const std::int32_t count = args.get_integer(2);
    host.shipyard_consume_stock_008457e0(shipyard, class_id, count); // PUSH EBP(count), PUSH EDI(class)
    return 0;
}

int lua_get_air_base_plane_limit(LuaArgs& args, LuaResults& results, Seg56Host& host)
{
    void* block = host.air_ops_block_006bcd20(args.entity_at(0), false);
    results.push_number(host.air_ops_block_plane_limit_006ba6c0(block));
    return 1;
}

int lua_inferior_is_disabled(LuaArgs& args, LuaResults& results, ImageAccess& image)
{
    results.push_boolean(image.read_u8(args.entity_at(0), 0x720u) != 0);
    return 1;
}

int lua_clear_wait(LuaArgs& args, ImageAccess& image)
{
    image.write_u8(args.entity_at(0), 0x1DCu, 0);
    return 0;
}

int lua_init_all(Seg56Host& host)
{
    host.sentity_init_all_00925f20(false); // XOR CL,CL
    return 0;
}

int lua_is_formation_leader(LuaArgs& args, LuaResults& results, Seg56Host& host)
{
    results.push_boolean(host.unit_is_formation_leader_00778890(args.entity_at(0)));
    return 1;
}

int lua_is_formation_follower(LuaArgs& args, LuaResults& results, Seg56Host& host)
{
    results.push_boolean(host.unit_is_formation_follower_007788b0(args.entity_at(0)));
    return 1;
}

int lua_set_beach_fight(LuaArgs& args)
{
    static_cast<void>(args.entity_at(0));   // 0089AEAF, result dropped
    static_cast<void>(args.get_integer(1)); // 0089AEDE, result dropped
    return 0;
}

int lua_set_capture_value(LuaArgs& args, ImageAccess& image)
{
    void* entity = args.entity_at(0);
    image.write_i32(entity, 0x7A4u, args.get_integer(1));
    return 0;
}

int lua_get_command_building_level(LuaArgs& args, LuaResults& results, ImageAccess& image)
{
    results.push_number(image.read_i32(args.entity_at(0), 0x770u));
    return 1;
}

int lua_get_squadron_landed(LuaArgs& args, LuaResults& results, ImageAccess& image)
{
    results.push_boolean(image.read_u8(args.entity_at(0), 0x3B0u) != 0);
    return 1;
}

int lua_plane_set_target_roll(LuaArgs& args, ImageAccess& image)
{
    void* plane = args.entity_at(0);
    image.write_f32(plane, 0x9A8u, static_cast<float>(args.get_number(1)));
    return 0;
}

int lua_plane_set_target_pitch(LuaArgs& args, ImageAccess& image)
{
    void* plane = args.entity_at(0);
    image.write_f32(plane, 0x9ACu, static_cast<float>(args.get_number(1)));
    return 0;
}

int lua_squadron_set_home_base(LuaArgs& args, Seg56Host& host)
{
    void* squadron = args.entity_at(0);
    void* base = args.entity_at(1);
    host.squadron_set_home_air_base_007f1c00(squadron, base);
    return 0;
}

int lua_squadron_set_old_style_bombing(LuaArgs& args, ImageAccess& image)
{
    void* squadron = args.entity_at(0);
    image.write_u8(squadron, 0x3A8u, args.get_boolean(1) ? 1 : 0);
    return 0;
}

int lua_plane_force_release(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    auto* plane = static_cast<std::uint8_t*>(args.entity_at(0));
    const std::uint32_t index = image.global_u16(0x00F876B8u); // MOVZX EDX,word [00F876B8]
    if (image.read_u8(plane, 0x9C0u + index * 8u) != 0) {      // CMP byte [ESI+EDX*8+9C0h],0
        host.unit_request_ordnance_release_007bbba0(plane);
    }
    return 0;
}

int lua_plane_change_ammo_type(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    const std::int32_t ammo_type = args.get_integer(0);
    host.squadron_reequip_all_007ed690(image.global_ptr(0x00E188D8u), ammo_type);
    return 0;
}

int lua_squadron_set_ai_retreat_enabled(LuaArgs& args, ImageAccess& image)
{
    void* squadron = args.entity_at(0);
    image.write_u8(squadron, 0x368u, args.get_boolean(1) ? 0 : 1); // SETZ: stores "disabled"
    return 0;
}

int lua_squadron_enable_vehicle_avoidance(LuaArgs& args, ImageAccess& image)
{
    void* squadron = args.entity_at(0);
    const bool enable = args.get_boolean(1);
    const std::int32_t flags = image.read_i32(squadron, 0x3A4u);
    // OR EAX,2 / AND EAX,0FDh (25 FD 00 00 00 at 008A0157: the AND also clears bits 8..31).
    image.write_i32(squadron, 0x3A4u, enable ? (flags | 0x2) : (flags & 0xFD));
    return 0;
}

int lua_squadron_enable_gunfire_avoidance(LuaArgs& args, ImageAccess& image)
{
    void* squadron = args.entity_at(0);
    const bool enable = args.get_boolean(1);
    const std::int32_t flags = image.read_i32(squadron, 0x3A4u);
    // OR EAX,4 / AND EAX,0FBh (25 FB 00 00 00 at 008A0307: the AND also clears bits 8..31).
    image.write_i32(squadron, 0x3A4u, enable ? (flags | 0x4) : (flags & 0xFB));
    return 0;
}

int lua_squadron_enable_player_takeoff(LuaArgs& args, ImageAccess& image)
{
    image.write_u8(args.entity_at(0), 0x360u, 1); // MOV [ESI+360h],BL with EBX = 1
    return 0;
}

int lua_squadron_disable_player_takeoff(LuaArgs& args, ImageAccess& image)
{
    image.write_u8(args.entity_at(0), 0x360u, 0);
    return 0;
}

int lua_squadron_restore_travel_speed(LuaArgs& args, Seg56Host& host)
{
    host.squadron_reset_travel_speed_007ed590(args.entity_at(0));
    return 0;
}

int lua_squadron_set_drop_all_bomb_percent(LuaArgs& args, ImageAccess& image)
{
    void* squadron = args.entity_at(0);
    image.write_f32(squadron, 0x390u, static_cast<float>(args.get_number(1)));
    return 0;
}

int lua_navigator_land_attacker_stop(LuaArgs& args, ImageAccess& image)
{
    image.write_global_u8(0x00E0E2FCu, args.get_boolean(0) ? 0 : 1); // SETZ DL
    return 0;
}

int lua_navigator_allow_max_depth(LuaArgs& args, ImageAccess& image)
{
    void* navigator = image.read_ptr(args.entity_at(0), 0x73Cu); // read before argument 1
    image.write_u8(navigator, 0x20u, args.get_boolean(1) ? 1 : 0);
    return 0;
}

int lua_navigator_force_move_close_to_target(LuaArgs& args, ImageAccess& image)
{
    void* navigator = image.read_ptr(args.entity_at(0), 0x73Cu);
    image.write_u8(navigator, 0x21u, args.get_boolean(1) ? 0 : 1); // SETZ AL
    return 0;
}

int lua_is_unit_selectable(LuaArgs& args, LuaResults& results, Seg56Host& host)
{
    results.push_boolean(host.unit_is_selectable_00645160(args.entity_at(0), true)); // MOV DL,1
    return 1;
}

int lua_set_repair_effectivity(LuaArgs& args, ImageAccess& image)
{
    void* unit = args.entity_at(0);
    image.write_f32(unit, 0xA48u, static_cast<float>(args.get_number(1)));
    return 0;
}

int lua_pause(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    const bool on = args.get_boolean(0);
    host.game_set_cinematic_mode_004cd0f0(game(image), on, 0, 1); // PUSH 1, PUSH EBP(0), PUSH EAX
    return 0;
}

// ---------------------------------------------------------------------------
// Added by cc13-seg56b
// ---------------------------------------------------------------------------

int lua_set_plane_yaw_control(LuaArgs& args, ImageAccess& image)
{
    void* plane = args.entity_at(0);
    image.write_f32(plane, 0x998u, static_cast<float>(args.get_number(1))); // 0089DB05
    return 0;
}

int lua_set_plane_pitch_control(LuaArgs& args, ImageAccess& image)
{
    void* plane = args.entity_at(0);
    image.write_f32(plane, 0x99Cu, static_cast<float>(args.get_number(1))); // 0089DCA5
    return 0;
}

int lua_set_plane_roll_control(LuaArgs& args, ImageAccess& image)
{
    void* plane = args.entity_at(0);
    image.write_f32(plane, 0x9A0u, static_cast<float>(args.get_number(1))); // 0089DE45
    return 0;
}

int lua_set_plane_power_control(LuaArgs& args, ImageAccess& image)
{
    void* plane = args.entity_at(0);
    image.write_f32(plane, 0x9A4u, static_cast<float>(args.get_number(1))); // 0089DFE5
    return 0;
}

int lua_squadron_enable_terrain_avoidance(LuaArgs& args, ImageAccess& image)
{
    void* squadron = args.entity_at(0);
    const bool enable = args.get_boolean(1);
    const std::int32_t flags = image.read_i32(squadron, 0x3A4u);
    // OR EAX,1 / AND EAX,0FEh (25 FE 00 00 00 at 0089FFA7: the AND also clears bits 8..31).
    image.write_i32(squadron, 0x3A4u, enable ? (flags | 0x1) : (flags & 0xFE));
    return 0;
}

int lua_squadron_set_force_release(LuaArgs& args, ImageAccess& image)
{
    void* squadron = args.entity_at(0);
    image.write_u8(squadron, 0x3AEu, args.get_boolean(1) ? 1 : 0); // 008A29FC
    return 0;
}

int lua_set_torpedo_swim_depth(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    void* object = host.lua_table_ptr_field_00888d20(args, 0); // 008A280F, not 00888AA0
    const float depth = static_cast<float>(args.get_number(1)); // FSTP float [ESP+10h]
    image.write_f32(object, 0x47Cu, depth);                      // 008A2861
    return 0;
}

int lua_remove_untouchable_unit(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    image.write_u8(host.entity_object_vt140(entity), 0x1D4u, 0); // CALL [vtable+140h], 008AC3D6
    return 0;
}

int lua_set_air_base_plane_limit(LuaArgs& args, Seg56Host& host)
{
    void* block = host.air_ops_block_006bcd20(args.entity_at(0), false); // XOR DL,DL
    const std::int32_t limit = args.get_integer(1);
    host.air_ops_block_set_plane_limit_006ba6d0(block, limit);          // 00897878
    return 0;
}

int lua_set_device_reload_time_mul(LuaArgs& args, ImageAccess& image)
{
    image.write_global_f32(0x00E08178u, static_cast<float>(args.get_number(0))); // 008C15B4
    return 0;
}

int lua_set_weather(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    const std::string name = args.get_string(0); // 0041E870 into the by-value argument
    host.weather_set_current_00445a30(image.read_ptr(game(image), 0x21C4u), name);
    return 0;
}

int lua_find_hidden_entity(LuaArgs& args, LuaResults& results, ImageAccess& image, Seg56Host& host)
{
    const std::string name = args.get_string(0);
    results.push_boolean(host.scene_has_hidden_entity_record_00469480(image.global_ptr(0x00E18680u), name));
    return 1;
}

int lua_is_landing_traffic_active(LuaArgs& args, LuaResults& results, Seg56Host& host)
{
    results.push_boolean(host.entity_landing_traffic_active_vt210(args.entity_at(0))); // 0089B060
    return 1;
}

int lua_multi_select_unit(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    const std::int32_t slot = args.get_integer(0); // read before the entity
    void* unit = args.entity_at(1);
    auto* session = static_cast<std::uint8_t*>(game(image)) + 0x1EF0; // ADD ECX,1EF0h
    host.session_send_pending_selection_0076cf30(session, slot, unit);
    return 0;
}

int lua_pilot_close_to_ship(LuaArgs& args, LuaCommandTargetSource& targets, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    const SceneCommandTarget target = lua_read_command_target(targets, 1);
    host.entity_issue_command_0077d600(entity, kCommandObjectCloseToShip, target, 1); // 008A4AB7
    return 0;
}

int lua_pilot_stop_close_to_ship(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    void* director = host.entity_director_vt114(entity);
    // CMP [EAX+54h],00E08FA8: the director's current command object is closetoship.
    if (static_cast<std::uint32_t>(image.read_i32(director, 0x54u)) == kCommandObjectCloseToShip) {
        host.pilot_raise_attack_mode_007ed430(entity, 2); // PUSH EBX(2), ECX = the entity
    }
    return 0;
}

int lua_scoring_grant_unlock(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    const std::string name = args.get_string(0); // 0041E870 into a stack native string
    auto* set = static_cast<std::uint8_t*>(game(image)) + 0x6CC; // ADD ECX,6CCh
    host.string_set_insert_004d0640(set, name);                   // 008CBA4C
    return 0;
}
// ---------------------------------------------------------------------------
// Added by cc13-seg56c
// ---------------------------------------------------------------------------

namespace {

int set_damage_setting(LuaArgs& args, Seg56Host& host, std::int32_t selector)
{
    void* entity = args.entity_at(0);
    const float value = static_cast<float>(args.get_number(1)); // FSTP float [ESP+10h]
    bool flag = false;                                          // XOR BL,BL
    if (args.count() == 3) {                                    // CMP EAX,3 / JNZ
        flag = args.get_boolean(2);
    }
    DamageSettingMessage message;
    message.value = value;
    message.flag_cleared = !flag; // TEST BL,BL / SETZ CL
    message.selector = selector;
    host.route_damage_message_0077c2a0(entity, message, 7, 0);
    return 0;
}

// The inline id -> object lookup PilotGunFire makes (008A522D..008A5262): ids below the
// count [00F89A10] index the first table ([00F89A54], base id [00F89A0C]), the rest the
// second ([00F89AA8], base id [00F89A60]); 10h-byte records, object at +0Ch.
void* object_from_handle_inline(ImageAccess& image, std::uint16_t id)
{
    const std::int32_t value = id;
    std::int32_t index = 0;
    std::uint8_t* table = nullptr;
    if (value < image.global_i32(0x00F89A10u)) { // CMP EAX,[00F89A10] / JGE
        index = value - image.global_i32(0x00F89A0Cu);
        table = static_cast<std::uint8_t*>(image.global_ptr(0x00F89A54u));
    } else {
        index = value - image.global_i32(0x00F89A60u);
        table = static_cast<std::uint8_t*>(image.global_ptr(0x00F89AA8u));
    }
    return image.read_ptr(table + index * 0x10, 0xCu); // SHL EAX,4 / [EAX+ECX+0Ch]
}

} // namespace

int lua_set_fire_damage(LuaArgs& args, Seg56Host& host)
{
    return set_damage_setting(args, host, 0); // MOV [ESP+50h],EBP with EBP = 0
}

int lua_set_water_damage(LuaArgs& args, Seg56Host& host)
{
    return set_damage_setting(args, host, 1); // MOV [ESP+50h],EBP with EBP = 1
}

int lua_set_motion_blur_params(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    const float a0 = static_cast<float>(args.get_number(0)); // FSTP float [ESP+1Ch]
    const float a1 = static_cast<float>(args.get_number(1)); // FSTP float [ESP+10h]
    const float a2 = static_cast<float>(args.get_number(2)); // FSTP float [ESP+18h]
    float block[8];
    block[0] = 1.0f; // [00D7A24C] = 3F800000
    block[1] = 0.5f; // [00CE3800] = 3F000000
    block[2] = a0;
    // DE E2 FSUBRP ST2,ST0 (a1 - a0), DE F9 FDIVP (/ a2), rounded once by the FSTP float.
    block[3] = static_cast<float>((static_cast<double>(a1) - static_cast<double>(a0)) / static_cast<double>(a2));
    block[4] = a2;
    block[5] = 0.0f;
    block[6] = 0.0f;
    block[7] = 0.0f;
    host.scene_renderer_set_motion_blur_00b0ea20(image.global_ptr(0x00F8D39Cu), block); // 0088E738
    return 0;
}

int lua_set_control_factor(LuaArgs& args, Seg56Host& host)
{
    const std::string name = args.get_string(1); // 0041E870 into a stack native string
    std::int32_t index = -1;                     // OR EDI,0FFFFFFFFh
    float value = 0.0f;
    if (args.count() == 3) {
        value = static_cast<float>(args.get_number(2));
    } else {
        index = args.get_integer(2);
        value = static_cast<float>(args.get_number(3));
    }
    void* entity = args.entity_at(0); // read last (0088ED13)
    host.entity_set_control_factor_vt1a0(entity, name, index, value); // 0088ED42
    return 0;
}

int lua_get_control_factor(LuaArgs& args, LuaResults& results, Seg56Host& host)
{
    const std::string name = args.get_string(1);
    const std::int32_t index = args.count() == 2 ? 0 : args.get_integer(2); // XOR EDI,EDI
    void* entity = args.entity_at(0);
    float out; // [ESP+18h], not initialised by the native
    host.entity_get_control_factor_vt1a4(entity, name, index, &out); // 0088EF69
    results.push_float(out);                                        // 0088EF77
    return 1;
}

int lua_ship_use_catapult(LuaArgs& args, Seg56Host& host)
{
    void* object = host.lua_table_ptr_field_00888d20(args, 0); // 00891E58
    float speed = 0.0f;                                        // XORPS, 00891E76
    if (args.count() == 2 && args.is_number(1)) {              // CMP EAX,EBX(2); 00B66050
        speed = static_cast<float>(args.get_number(1));
    }
    CatapultLaunchMessage message;
    message.speed = speed;
    host.route_catapult_message_0077c2a0(object, message, 2, 0); // PUSH EBP(0), PUSH EBX(2)
    return 0;
}

int lua_get_draught(LuaArgs& args, LuaResults& results, ImageAccess& image)
{
    void* klass = image.read_ptr(args.entity_at(0), 0x538u);
    float best = 0.0f; // XORPS, 008937C2
    // The size (end - begin) / 24h is recomputed every pass (008937D0..008937F9); a null
    // begin ends the loop. The checked-iterator tests (00BF6713) cannot fire while i < size.
    for (std::uint32_t i = 0;; ++i) {
        auto* begin = static_cast<std::uint8_t*>(image.read_ptr(klass, 0x52Cu));
        if (begin == nullptr) {
            break;
        }
        auto* end = static_cast<std::uint8_t*>(image.read_ptr(klass, 0x530u));
        const std::int32_t size = static_cast<std::int32_t>(end - begin) / 0x24;
        if (i >= static_cast<std::uint32_t>(size)) { // CMP EDI,EAX / JNC
            break;
        }
        const float value = image.read_f32(begin + i * 0x24u, 0x14u);
        if (value > best) { // FCOMIP / JBE skips equal and unordered
            best = value;
        }
    }
    results.push_float(best);
    return 1;
}

int lua_land_convoy_set_path(LuaArgs& args, Seg56Host& host)
{
    void* convoy = host.lua_table_ptr_field_00888d20(args, 0);
    void* entity = args.entity_at(1);
    host.land_convoy_set_path_00741160(convoy, host.entity_path_interface_007ac9d0(entity));
    return 0;
}

int lua_pilot_bomb(LuaArgs& args, LuaCommandTargetSource& targets, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    const SceneCommandTarget target = lua_read_command_target(targets, 1);
    std::uint32_t command = kCommandObjectDropKamikaze;
    if (!host.unit_has_drop_kamikaze_ordnance_007ed880(entity, 0)) { // PUSH EBP(0)
        command = host.entity_is_class_vt5c(entity, 0x10) ? kCommandObjectBombKind10 : kCommandObjectBomb;
    }
    host.entity_issue_command_0077d600(entity, command, target, 1); // 008A507F
    return 0;
}

int lua_pilot_gun_fire(LuaArgs& args, LuaCommandTargetSource& targets, ImageAccess& image, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    SceneCommandTarget target = lua_read_command_target(targets, 1);
    void* object = nullptr;               // kind 0: XOR ESI,ESI
    if (target.kind != 0) {
        object = target.object;
        if (object == nullptr) {          // resolve from the id and write it back (008A5264)
            object = object_from_handle_inline(image, target.object_id);
            target.object = object;
        }
    }
    // The class tests run on the target object unguarded, as the native does (008A5268).
    std::uint32_t command = kCommandObjectGunFireShipOrBuilding;
    if (!host.entity_is_class_vt5c(object, 0x18) && !host.entity_is_class_vt5c(object, 0x0F)) {
        void* lead_plane = image.read_ptr(entity, 0x3D0u); // [EDI+3D0h]
        command = host.entity_is_class_vt5c(lead_plane, 0x17) ? kCommandObjectGunFirePlaneKind17
                                                               : kCommandObjectGunFire;
    }
    host.entity_issue_command_0077d600(entity, command, target, 1); // 008A52B6
    return 0;
}

int lua_set_player_control(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    void* object = host.lua_table_ptr_field_00888d20(args, 0);
    host.unit_send_role_transfer_0077c470(object, 2, 1); // PUSH 1, PUSH EBX(2)
    if (object != nullptr && host.entity_is_class_vt5c(object, 0x18)) {
        host.plane_set_control_enabled_007b8c30(image.read_ptr(object, 0x3D0u), true);
    }
    return 0;
}

int lua_enter_player_to_role(LuaArgs& args, Seg56Host& host)
{
    void* object = host.lua_table_ptr_field_00888d20(args, 0);
    RoleTransferMessage message;
    message.argument_1 = args.get_integer(1);
    message.argument_2 = args.get_integer(2);
    host.route_role_message_0077c2a0(object, message, 0, 0); // PUSH 0, PUSH 0
    return 0;
}

int lua_set_race(LuaArgs& args, ImageAccess& image, Seg56Host& host)
{
    const std::int32_t race = args.get_integer(1); // read before the entity (008A8824)
    void* entity = args.entity_at(0);
    if (host.entity_is_class_vt5c(entity, 2)) {    // PUSH EBX(2)
        IntValueMessage message;
        message.type = 0x80;
        message.vtable = 0x00D032FCu;
        message.value = race;
        host.route_int_message_0077c2a0(entity, message, 7, 0);
    } else {
        std::int32_t out = 0; // MOV [ESP+24h],EBP before the call
        host.entity_set_race_vt2c(entity, image.read_i32(entity, 0x54u), race, &out);
    }
    return 0;
}

int lua_set_dead_meat(LuaArgs& args, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    if (host.entity_is_class_vt5c(entity, 0x0F)) {
        host.entity_set_failure_vt194(entity, kDeadMeatFailureName, 0); // 008AC8FB
    } else if (host.entity_is_class_vt5c(entity, 6)) {
        // FLD1 / [00D7A260] = -1.0f as hi / lo, ECX = 1, result FSTP float.
        const float pivot[3] = {static_cast<float>(host.random_uniform_00bd2f10(1, -1.0f, 1.0f)), 0.0f, 0.0f};
        host.unit_sink_008110f0(entity, pivot); // 008AC96A
    } else {
        host.entity_kill_vt70(entity, 1); // 008AC97A
    }
    return 0;
}

int lua_set_repair_priority(LuaArgs& args, Seg56Host& host)
{
    void* entity = args.entity_at(0);
    IntValueMessage message;
    message.type = 0xA0;
    message.vtable = 0x00CF5C38u;
    message.value = args.get_integer(1);
    host.route_int_message_0077c2a0(entity, message, 7, 0); // 008AD866
    return 0;
}

int lua_force_multi_score_send(ImageAccess& image)
{
    void* game_object = game(image);
    if (image.read_i32(game_object, kSessionModeOffset) == 1) { // CMP [EAX+1FE4h],EBX(1)
        void* world = image.read_ptr(game_object, 0x19CCu);
        void* node = image.read_ptr(world, 0x49Cu);
        std::int32_t count = image.read_i32(world, 0x498u);
        while (count > 0) { // CMP ECX,EBP / JLE, then SUB ECX,1 ... JNZ
            --count;
            image.write_u8(image.read_ptr(node, 0x8u), 0x310u, 1); // 008AFF95
            node = image.read_ptr(node, 0x4u);
        }
    }
    return 0; // XOR EAX,EAX: no call frame in this native
}
} // namespace bsp::seg56_options_cc13
