#pragma once
// A SweepCHost whose every service traps (std::abort). Callers derive from it
// and override only the boundaries the body they run actually reaches, so a
// path this process has not bound fails loudly instead of answering with an
// unrelated placeholder. Generated from the pure virtuals of
// bsp/cc13_sweepC.hpp (SweepCHost and its five bases), goal 5 chain 628.
#include "bsp/cc13_sweepC.hpp"

#include <cstdlib>

namespace bsp::cc13_sweepC {

#pragma warning(push)
#pragma warning(disable : 4100)  // the trapped services name their parameters
class SweepCTrapHost : public SweepCHost {
public:
    void sound_stream_select_row_00a86710(void* stream, const void* name) override { std::abort(); }
    void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t keep) override { std::abort(); }
    void session_message_construct_base_0075b430(void* message, std::uint32_t kind) override { std::abort(); }
    std::uint32_t image_dword(std::uint32_t address) override { std::abort(); }
    void* camera_pool_allocate_00b71930(std::uint32_t size) override { std::abort(); }
    void* camera_construct_00b71a80(void* block, std::uint32_t arg) override { std::abort(); }
    void invalid_iterator_00bf6713() override { std::abort(); }
    bool filter_accepts_vslot_00(void* filter, std::uint32_t parameter) override { std::abort(); }
    std::uint32_t* vector_at_009658b0(void* vector, std::uint32_t index) override { std::abort(); }
    void enter_critical_section(void* section) override { std::abort(); }
    void leave_critical_section(void* section) override { std::abort(); }
    void* operator_new_00bf55be(std::uint32_t bytes) override { std::abort(); }
    void operator_delete_00bf6989(void* block) override { std::abort(); }
    float fixed_step_clock_00f876a4() override { std::abort(); }
    void observer_register_pair_00694a60(void* subject, void* member) override { std::abort(); }
    void observer_unregister_pair_006952a0(void* subject, void* member) override { std::abort(); }
    void observer_notify_slot_00696350(void* object, int edx_slot, std::uint32_t arg) override { std::abort(); }
    void* list_buy_node_006bfbd0(void* list, void* next, void* prev, const void* value) override { std::abort(); }
    void list_incsize_006c89d0(void* list, std::uint32_t count) override { std::abort(); }
    void list_remove_006c48f0(void* list, const void* value) override { std::abort(); }
    void* landing_slot_message_construct_006bd520(void* message, void* block, int slot) override { std::abort(); }
    void session_owner_broadcast_0077c7b0(void* owner, void* message, std::uint32_t peer) override { std::abort(); }
    void session_route_message_0077c2a0(void* owner, void* message, std::uint32_t channel, std::uint32_t peer) override { std::abort(); }
    void air_ops_add_stock_006ca770(void* block, void* vehicle_class, std::int32_t count) override { std::abort(); }
    void archive_write_vslot_0c(void* archive, ArchiveArg key, ArchiveArg value) override { std::abort(); }
    void archive_read_vslot_10(void* archive, ArchiveArg key, ArchiveArg value) override { std::abort(); }
    void follow_observer_member_destroy_006cdd70(void* member) override { std::abort(); }
    std::int32_t air_ops_slot_command_006ccda0(void* block, std::int32_t slot, std::int32_t order, void* target, std::int32_t player, std::uint32_t apply) override { std::abort(); }
    void air_ops_apply_slot_setup_006c47f0(void* block, void* message) override { std::abort(); }
    void air_ops_launch_slot_006ca8e0(void* block, std::int32_t slot, std::uint8_t flag) override { std::abort(); }
    void air_ops_apply_slot_record_006c83b0(void* block, void* message) override { std::abort(); }
    void air_ops_launch_start_006c7490(void* block, std::int32_t slot, std::uint32_t arg) override { std::abort(); }
    void* message_resolve_unit_006bdbf0(void* message) override { std::abort(); }
    void object_vslot_14(void* object, void* arg) override { std::abort(); }
    void* vehicle_class_get_007b8a80(std::uint32_t class_id) override { std::abort(); }
    void* site_vslot_4c(void* site) override { std::abort(); }
    void* elevator_message_construct_00758b90(void* message, std::uint32_t flag, void* plane) override { std::abort(); }
    void* operator_new_00bf681b(std::uint32_t bytes) override { std::abort(); }
    void* critical_section_create_00bd1860() override { std::abort(); }
    void* list_head_004870f0(void* container) override { std::abort(); }
    void* list_head_00659350(void* container) override { std::abort(); }
    void* list_head_006bfa20(void* container) override { std::abort(); }
    void* tree_head_006bfd60(void* container) override { std::abort(); }
    void observer_list_construct_006c4fa0(void* object) override { std::abort(); }
    void unit_game_object_construct_0095cc90(void* unit, std::uint32_t flag, std::uint32_t zero) override { std::abort(); }
    void collision_node_init_004e6480(void* node) override { std::abort(); }
    void unit_set_vehicle_class_009553d0(void* unit, void* descriptor) override { std::abort(); }
    float unit_health_00923be0(void* unit) override { std::abort(); }
    void air_ops_block_tick_006ba670(void* block, float dt) override { std::abort(); }
    void unit_set_float_00779c60(void* unit, float value) override { std::abort(); }
    void land_unit_reset_00878340(void* unit) override { std::abort(); }
    void object_vslot_1b8(void* object) override { std::abort(); }
    void airfield_rebuild_aim_points_006d2980(void* airfield) override { std::abort(); }
    void unit_detach_00928570(void* unit) override { std::abort(); }
    void list_erase_node_004837d0(void* list, void* node) override { std::abort(); }
    float motion_body_axis_speed_0092d730(void* controller) override { std::abort(); }
    std::uint32_t director_current_command_0071be40(void* director) override { std::abort(); }
    bool object_is_kind_of_vslot_5c(void* object, std::uint32_t kind) override { std::abort(); }
    void node_release_0085ad00(void* node) override { std::abort(); }
    void* motion_controller_construct_00939cb0(void* controller, void* unit) override { std::abort(); }
    void object_vslot_18(void* object, std::uint32_t arg) override { std::abort(); }
    void unit_set_skill_level_009565a0(void* unit, std::uint32_t level) override { std::abort(); }
    void part_set_controlled_00815370(void* part, float value) override { std::abort(); }
    void wake_point_at_distance_00810630(void* wake, float distance, void* a, void* b, void* c) override { std::abort(); }
    std::int32_t unit_torpedo_stock_part_00810e90(void* unit) override { std::abort(); }
    void motion_controller_on_killed_0092bd30(void* controller) override { std::abort(); }
    void ship_stop_wreck_effects_00818970(void* ship) override { std::abort(); }
    void unit_on_killed_base_0095d400(void* unit) override { std::abort(); }
    void* object_vslot_2c(void* object) override { std::abort(); }
    void entity_pose_refresh_world_00414db0(void* entity) override { std::abort(); }
    void* nearest_landing_site_006f2c30(void* position, std::uint32_t party, std::uint32_t kind, void* out) override { std::abort(); }
    void gun_sync_record_stamp_00782790(void* record, std::uint32_t tick) override { std::abort(); }
    void object_vslot_18c(void* object, void* arg) override { std::abort(); }
    float object_vslot_50_float(void* object) override { std::abort(); }
    std::uint32_t object_vslot_00_noarg(void* object) override { std::abort(); }
    const float* motion_angular_rate_0092d700(void* controller, void* scratch) override { std::abort(); }
    const float* motion_velocity_0092d6a0(void* controller, void* scratch) override { std::abort(); }
    void session_message_write_header_0075b480(void* message, void* stream) override { std::abort(); }
    void session_message_read_header_0075b4c0(void* message, void* stream) override { std::abort(); }
    void bit_write_float_004295c0(void* stream, float value, std::uint32_t a, std::uint32_t b, float range, std::uint32_t bits) override { std::abort(); }
    void bit_read_float_004293f0(void* stream, float* out, std::uint32_t a, std::uint32_t b, float range, std::uint32_t bits) override { std::abort(); }
    void bit_write_bool_004290b0(void* stream, std::uint8_t value) override { std::abort(); }
    void bit_read_bool_00428d70(void* stream, std::uint8_t* out) override { std::abort(); }
    void unit_get_property_base_00779bb0(void* unit, void* lua, void* key) override { std::abort(); }
    void lua_push_integer_00b664b0(void* lua, std::int32_t value) override { std::abort(); }
    double object_vslot_1c_double(void* object) override { std::abort(); }
    double object_vslot_20_double(void* object) override { std::abort(); }
    std::int32_t interlocked_decrement_00ce2220(std::int32_t* counter) override { std::abort(); }
    void refcounted_destroy_vslot_00(void* object) override { std::abort(); }
    void member_destroy_004b7ef0(void* member) override { std::abort(); }
    void* ai_globals_owner_004c1c50() override { std::abort(); }
    void ai_globals_load_00a335d0(void* owner) override { std::abort(); }
    void* lua_object_to_entity_00888aa0(void* lua_object) override { std::abort(); }
    void lua_object_destroy_00b67700(void* lua_object) override { std::abort(); }
    std::uint32_t xlive_user_index_00a3eac0(void* xlive) override { std::abort(); }
    bool xlive_signed_in_00a3e510(void* xlive) override { std::abort(); }
    std::uint32_t xlive_check_privilege_00a4d5e4(std::uint32_t user, std::uint32_t a, std::uint32_t b, std::uint32_t* out) override { std::abort(); }
    bool object_vslot_24_xuid(void* object, std::uint32_t lo, std::uint32_t hi) override { std::abort(); }
    void session_set_participant_policy_00771630(void* session, std::uint32_t lo, std::uint32_t hi, std::uint32_t value) override { std::abort(); }
    void sound_stream_set_paused_00a85c40(void* stream, std::uint8_t paused) override { std::abort(); }
    std::uint32_t sound_stream_draw_debug_00a869b0(void* stream, std::uint32_t x, std::uint32_t y) override { std::abort(); }
    void sound_stream_request_stop_00a86f40(void* stream) override { std::abort(); }
    const void* sound_stream_name_00a77ff0(void* stream, void* out_string) override { std::abort(); }
    void log_printf_004254b0(const char* format, const char* text) override { std::abort(); }
    void string_pool_release_00419cc0_00bd1510(void* data, std::uint32_t size) override { std::abort(); }
    void eh_vector_destructor_iterator_00bf7c6e(void* array, std::uint32_t size, std::uint32_t count, std::uint32_t destructor) override { std::abort(); }
    void operator_delete_00bf65ac(void* block) override { std::abort(); }
    void dialog_manager_destroy_00a780b0(void* manager) override { std::abort(); }
    void dialog_channel_records_resize_00a78ec0(void* records, std::uint32_t count) override { std::abort(); }
    void input_backend_base_destroy_00a90e00(void* backend) override { std::abort(); }
    void input_action_poll_00a92370(void* action) override { std::abort(); }
    void input_control_update_00a91a50(void* control, float dt, std::uint32_t a, std::uint32_t b) override { std::abort(); }
    std::uint8_t image_byte(std::uint32_t address) override { std::abort(); }
    void set_image_byte(std::uint32_t address, std::uint8_t value) override { std::abort(); }
    void gui_type_base_init_00aa8200(std::uint32_t descriptor) override { std::abort(); }
    void type_base_init_00bea780(std::uint32_t descriptor) override { std::abort(); }
    void* type_id_counter_006fac20() override { std::abort(); }
    void gui_widget_pool_free_00aa75f0(std::uint32_t pool, void* block) override { std::abort(); }
    void* gui_widget_pool_allocate_00aa7a80(std::uint32_t pool, std::uint32_t size) override { std::abort(); }
    void node_assign_name_00b6f960(void* node, const void* name) override { std::abort(); }
    void gui_icon_construct_00ab5c60(void* icon) override { std::abort(); }
    void* matrix_compose_00b64470(void* out, const float* translation, const float* first, const float* second) override { std::abort(); }
    void object_vslot_38(void* object, void* matrix) override { std::abort(); }
    void object_vslot_88(void* object) override { std::abort(); }
    void object_vslot_80(void* object, std::int32_t state) override { std::abort(); }
    void gui_widget_position_changed_00aa7220(void* widget) override { std::abort(); }
    std::int32_t interlocked_increment_00ce221c(std::int32_t* counter) override { std::abort(); }
    void native_pool_return_00b17af0(std::uint32_t pool, void* slot) override { std::abort(); }
    void resource_item_array_reserve_00b872f0(void* array, std::int32_t n) override { std::abort(); }
    void hierarchy_item_array_reserve_00b87350(void* array, std::int32_t n) override { std::abort(); }
    void resource_item_array_resize_00b873c0(void* array, std::int32_t n) override { std::abort(); }
    void hierarchy_item_array_resize_00b87410(void* array, std::int32_t n) override { std::abort(); }
    void resource_name_copy_00b887d0(void* dst, const void* src) override { std::abort(); }
    void* resource_instance_construct_00b89f20(void* block, void* resource) override { std::abort(); }
    std::uint32_t object_vslot_00(void* object, std::uint32_t arg) override { std::abort(); }
};
#pragma warning(pop)

}  // namespace bsp::cc13_sweepC
