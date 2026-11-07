#pragma once
// A base for host adapters over bsp::seg56_options_cc13::Seg56Host
// (include/bsp/options_seg56_cc13.hpp), with its EntityRaceHost base.
//
// Seg56Host carries 73 engine services as pure virtuals and no game host
// implemented it, so the segment-56 option bindings that take it never ran.
// This class overrides every one of them with a loud failure; an adapter
// derives from it and overrides only the services its own path reaches, each
// mapped onto the host's real objects. A service the adapter does not override
// throws std::logic_error naming the service, so a path the adapter did not
// read fails at once instead of running on an invented value.
//
// Written mechanically from the declarations in options_seg56_cc13.hpp (goal 5
// chain 649), one override per pure virtual, same signature, in the header's
// order, as bsp/fe5_host_unreached.hpp does for Fe5Host. A C++ adapter base,
// not an image type: no address, no image layout. Keep it in step with
// Seg56Host.

#include "bsp/options_seg56_cc13.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace bsp::seg56_options_cc13 {

[[noreturn]] inline void seg56_unreached(const char* service) {
    throw std::logic_error(std::string("Seg56Host service not provided by this adapter: ") + service);
}

// One macro body, so nothing reading the sources for implementations of a
// service (tools/wire_readiness.py among them) takes one for an implementation.
#define BSP_SEG56_UNREACHED { ::bsp::seg56_options_cc13::seg56_unreached(__func__); }
#pragma warning(push)
#pragma warning(disable : 4100)  // the parameter names document the services
class Seg56HostUnreached : public Seg56Host {
public:
    bool entity_is_class_vt5c(void* entity, std::int32_t class_id) override BSP_SEG56_UNREACHED
    void route_int_message_0077c2a0(void* target, const IntValueMessage& message, int flags, int out) override BSP_SEG56_UNREACHED
    void entity_set_race_vt2c(void* entity, std::int32_t dword_54, std::int32_t race, std::int32_t* out) override BSP_SEG56_UNREACHED
    void support_manager_set_banned_00a38da0(bool banned) override BSP_SEG56_UNREACHED
    void session_send_0076a9f0(const SupportManagerBanMessage& message) override BSP_SEG56_UNREACHED
    void interface_toggle_tactical_overlay_0068c0b0(void* interface_object, int argument) override BSP_SEG56_UNREACHED
    void panel_list_remove_0065bba0(void* panel, void* entity) override BSP_SEG56_UNREACHED
    void panel_list_push_back_00665ef0(void* panel, void* entity) override BSP_SEG56_UNREACHED
    bool panel_visible_006529e0(void* panel) override BSP_SEG56_UNREACHED
    void panel_set_dword_2b4_006529d0(void* panel, std::int32_t value) override BSP_SEG56_UNREACHED
    void entity_clear_failure_vt19c(void* entity, const std::string& name, LuaFrame& frame) override BSP_SEG56_UNREACHED
    void failure_block_reset_0093a390(void* failure_block) override BSP_SEG56_UNREACHED
    void entity_set_damaged_gfx_level_vt1b4(void* entity, std::int32_t level) override BSP_SEG56_UNREACHED
    void support_site_set_active_0084ce60(void* entity, bool active) override BSP_SEG56_UNREACHED
    void route_message_0077c2a0(void* entity, const SpawnpointMessage& message, int flags, int out) override BSP_SEG56_UNREACHED
    void unit_set_numbering_008761e0(void* unit, std::int32_t number) override BSP_SEG56_UNREACHED
    void unit_group_apply_formation_shape_0070efd0(void* group, std::int32_t shape) override BSP_SEG56_UNREACHED
    void unit_group_broadcast_formation_set_0077a080_0077c880(void* group) override BSP_SEG56_UNREACHED
    void plane_set_gear_mode_007b8ce0(void* plane, std::int32_t mode, std::int32_t flag) override BSP_SEG56_UNREACHED
    bool plane_controller_vt38(void* controller) override BSP_SEG56_UNREACHED
    double unit_reference_speed_0080fc30(void* unit) override BSP_SEG56_UNREACHED
    double entity_speed_vt38(void* entity) override BSP_SEG56_UNREACHED
    void panel_set_byte_8c_005f9470(void* panel, bool value) override BSP_SEG56_UNREACHED
    void* unit_parts_object_0080e490(void* unit) override BSP_SEG56_UNREACHED
    std::int32_t parts_dword_370_0092bea0(void* parts) override BSP_SEG56_UNREACHED
    std::int32_t parts_dword_374_0092be90(void* parts) override BSP_SEG56_UNREACHED
    void unit_on_breakup_00814520(void* unit) override BSP_SEG56_UNREACHED
    std::int32_t ship_count_catapulted_planes_00953920(void* ship) override BSP_SEG56_UNREACHED
    void ship_set_catapult_stock_009539a0(void* ship, std::int32_t stock) override BSP_SEG56_UNREACHED
    void entity_issue_command_0077d600(void* entity, std::uint32_t command_object, const SceneCommandTarget& target, std::int32_t flags) override BSP_SEG56_UNREACHED
    std::int32_t skill_from_crew_level_006e6210(std::int32_t crew_level) override BSP_SEG56_UNREACHED
    std::int32_t crew_level_from_skill_006e6270(std::int32_t skill) override BSP_SEG56_UNREACHED
    void entity_set_skill_vt128(void* entity, std::int32_t skill) override BSP_SEG56_UNREACHED
    std::int32_t entity_skill_vt12c(void* entity) override BSP_SEG56_UNREACHED
    void* air_ops_block_006bcd20(void* entity, bool require_active) override BSP_SEG56_UNREACHED
    std::int32_t air_ops_block_status_006bf5c0(void* block) override BSP_SEG56_UNREACHED
    void air_ops_slot_set_ready_006ba4e0(void* slot) override BSP_SEG56_UNREACHED
    std::int32_t air_ops_block_plane_limit_006ba6c0(void* block) override BSP_SEG56_UNREACHED
    void shipyard_consume_stock_008457e0(void* shipyard, std::int32_t class_id, std::int32_t count) override BSP_SEG56_UNREACHED
    void sentity_init_all_00925f20(bool flag) override BSP_SEG56_UNREACHED
    bool unit_is_formation_leader_00778890(void* unit) override BSP_SEG56_UNREACHED
    bool unit_is_formation_follower_007788b0(void* unit) override BSP_SEG56_UNREACHED
    void squadron_set_home_air_base_007f1c00(void* squadron, void* base) override BSP_SEG56_UNREACHED
    void unit_request_ordnance_release_007bbba0(void* plane) override BSP_SEG56_UNREACHED
    void squadron_reequip_all_007ed690(void* squadron, std::int32_t ammo_type) override BSP_SEG56_UNREACHED
    void squadron_reset_travel_speed_007ed590(void* squadron) override BSP_SEG56_UNREACHED
    bool unit_is_selectable_00645160(void* unit, bool flag) override BSP_SEG56_UNREACHED
    void game_set_cinematic_mode_004cd0f0(void* game, bool on, std::int32_t second, std::int32_t third) override BSP_SEG56_UNREACHED
    void* lua_table_ptr_field_00888d20(LuaArgs& args, int index) override BSP_SEG56_UNREACHED
    void* entity_object_vt140(void* entity) override BSP_SEG56_UNREACHED
    void air_ops_block_set_plane_limit_006ba6d0(void* block, std::int32_t limit) override BSP_SEG56_UNREACHED
    void weather_set_current_00445a30(void* weather_config, const std::string& name) override BSP_SEG56_UNREACHED
    bool scene_has_hidden_entity_record_00469480(void* scene_database, const std::string& name) override BSP_SEG56_UNREACHED
    bool entity_landing_traffic_active_vt210(void* entity) override BSP_SEG56_UNREACHED
    void session_send_pending_selection_0076cf30(void* session, std::int32_t slot, void* unit) override BSP_SEG56_UNREACHED
    void* entity_director_vt114(void* entity) override BSP_SEG56_UNREACHED
    void pilot_raise_attack_mode_007ed430(void* entity, std::int32_t mode) override BSP_SEG56_UNREACHED
    void string_set_insert_004d0640(void* set, const std::string& key) override BSP_SEG56_UNREACHED
    void route_damage_message_0077c2a0(void* target, const DamageSettingMessage& message, int flags, int out) override BSP_SEG56_UNREACHED
    void route_catapult_message_0077c2a0(void* target, const CatapultLaunchMessage& message, int flags, int out) override BSP_SEG56_UNREACHED
    void route_role_message_0077c2a0(void* target, const RoleTransferMessage& message, int flags, int out) override BSP_SEG56_UNREACHED
    void entity_set_control_factor_vt1a0(void* entity, const std::string& name, std::int32_t index, float value) override BSP_SEG56_UNREACHED
    void entity_get_control_factor_vt1a4(void* entity, const std::string& name, std::int32_t index, float* out) override BSP_SEG56_UNREACHED
    void* entity_path_interface_007ac9d0(void* entity) override BSP_SEG56_UNREACHED
    void land_convoy_set_path_00741160(void* convoy, void* path) override BSP_SEG56_UNREACHED
    void entity_set_failure_vt194(void* entity, const std::string& name, std::int32_t value) override BSP_SEG56_UNREACHED
    double random_uniform_00bd2f10(std::int32_t stream_ecx, float lo, float hi) override BSP_SEG56_UNREACHED
    void unit_sink_008110f0(void* unit, const float pivot[3]) override BSP_SEG56_UNREACHED
    void entity_kill_vt70(void* entity, std::int32_t mode) override BSP_SEG56_UNREACHED
    void scene_renderer_set_motion_blur_00b0ea20(void* service, const float block[8]) override BSP_SEG56_UNREACHED
    bool unit_has_drop_kamikaze_ordnance_007ed880(void* unit, std::int32_t loadout) override BSP_SEG56_UNREACHED
    void unit_send_role_transfer_0077c470(void* unit, std::uint32_t role_mask, std::int32_t take) override BSP_SEG56_UNREACHED
    void plane_set_control_enabled_007b8c30(void* plane, bool enabled) override BSP_SEG56_UNREACHED
};
#pragma warning(pop)
#undef BSP_SEG56_UNREACHED

}  // namespace bsp::seg56_options_cc13
