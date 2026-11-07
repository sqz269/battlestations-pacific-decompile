#pragma once
// A base for host adapters over bsp::cc13_left55::seg56c::Left55Seg56cHost
// (include/bsp/cc13_left55_seg56c.hpp).
//
// Left55Seg56cHost carries the engine services of its segment-56 Lua bindings as pure
// virtuals, and no game host implemented it, so the bindings that take it never
// ran. This class overrides every one of them with a loud failure; an adapter
// derives from it and overrides only the services its own path reaches, each
// mapped onto the host's real objects. A service the adapter does not override
// throws std::logic_error naming the service, so a path the adapter did not read
// fails at once instead of running on an invented value.
//
// Written mechanically from the declarations in cc13_left55_seg56c.hpp (goal 5 chain 660),
// one override per pure virtual, same signature, in the header's order, as
// bsp/left55_seg56a_host_unreached.hpp does for Left55Seg56aHost. A C++ adapter base, not
// an image type: no address, no image layout. Keep it in step with Left55Seg56cHost.

#include "bsp/cc13_left55_seg56c.hpp"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace bsp::cc13_left55::seg56c {

[[noreturn]] inline void left55_seg56c_unreached(const char* service) {
    throw std::logic_error(std::string("Left55Seg56cHost service not provided by this adapter: ")
        + service);
}

// One macro body, so nothing reading the sources for implementations of a
// service (tools/wire_readiness.py among them) takes one for an implementation.
#define BSP_LEFT55_SEG56C_UNREACHED { ::bsp::cc13_left55::seg56c::left55_seg56c_unreached(__func__); }
#pragma warning(push)
#pragma warning(disable : 4100)  // the parameter names document the services
class Left55Seg56cHostUnreached : public Left55Seg56cHost {
public:
    bool entity_vcall_5c(void* entity, int class_id) override BSP_LEFT55_SEG56C_UNREACHED
    void* entity_vcall_60(void* entity) override BSP_LEFT55_SEG56C_UNREACHED
    void entity_vcall_23c(void* entity) override BSP_LEFT55_SEG56C_UNREACHED
    void entity_vcall_f4(void* entity, float value) override BSP_LEFT55_SEG56C_UNREACHED
    bool entity_subobject_310_vcall_38(void* entity) override BSP_LEFT55_SEG56C_UNREACHED
    void lua_push_c_string_00b66710(const char* text) override BSP_LEFT55_SEG56C_UNREACHED
    void push_this_table_entity_004260b0_00b663d0(std::uint16_t entity_id) override BSP_LEFT55_SEG56C_UNREACHED
    void* lua_new_table_00b67930(LuaFrame& frame) override BSP_LEFT55_SEG56C_UNREACHED
    void lua_table_set_object_at_index_00b666c0(void* table, int index, void* value) override BSP_LEFT55_SEG56C_UNREACHED
    void lua_table_set_new_table_at_index_00b66670(void* table, int index) override BSP_LEFT55_SEG56C_UNREACHED
    void* lua_table_get_by_index_00b67720(void* table, int index) override BSP_LEFT55_SEG56C_UNREACHED
    void lua_set_vector3_fields_0088ba30(void* table, const Vec3& value) override BSP_LEFT55_SEG56C_UNREACHED
    void* entity_lua_self_object_00927b40(void* entity) override BSP_LEFT55_SEG56C_UNREACHED
    void* lua_argument_object_00b677e0(int index) override BSP_LEFT55_SEG56C_UNREACHED
    void* lua_state_globals_00b67980() override BSP_LEFT55_SEG56C_UNREACHED
    void* lua_object_get_by_name_00b67800(void* object, const char* name) override BSP_LEFT55_SEG56C_UNREACHED
    void* lua_object_construct_00b65f50() override BSP_LEFT55_SEG56C_UNREACHED
    void lua_iterate_first_00b67080(void* table, void* key, void* value) override BSP_LEFT55_SEG56C_UNREACHED
    void lua_iterate_next_00b67190(void* table, void* key, void* value) override BSP_LEFT55_SEG56C_UNREACHED
    bool lua_object_is_unbound_00b66420(void* table, void* key) override BSP_LEFT55_SEG56C_UNREACHED
    std::int32_t lua_object_get_integer_00b66290(void* object) override BSP_LEFT55_SEG56C_UNREACHED
    std::string lua_object_get_string_00b662b0(void* object) override BSP_LEFT55_SEG56C_UNREACHED
    void* lua_object_to_userdata_00b662d0(void* object) override BSP_LEFT55_SEG56C_UNREACHED
    bool lua_object_is_vector3_table_0088b840(void* object) override BSP_LEFT55_SEG56C_UNREACHED
    Vec3 lua_object_read_vector3_00888760(void* object) override BSP_LEFT55_SEG56C_UNREACHED
    void lua_object_vector_push_back_00442220(std::vector<void*>& vector, void* element) override BSP_LEFT55_SEG56C_UNREACHED
    void* lua_table_ptr_field_00888d20(int index) override BSP_LEFT55_SEG56C_UNREACHED
    SceneCommandTarget read_command_target_0088a810(int index) override BSP_LEFT55_SEG56C_UNREACHED
    void objectives_refresh_00627960() override BSP_LEFT55_SEG56C_UNREACHED
    void objectives_refresh_00626c50() override BSP_LEFT55_SEG56C_UNREACHED
    void objectives_screen_set_expanded_005ed5d0(void* screen, bool expanded) override BSP_LEFT55_SEG56C_UNREACHED
    void hud_counter_set_value_00528be0(void* counter, float value) override BSP_LEFT55_SEG56C_UNREACHED
    void please_wait_screen_show_with_worker_0060d6e0(void* screen, bool show) override BSP_LEFT55_SEG56C_UNREACHED
    bool weapon_controller_any_slot_answers_007b92e0(void* plane) override BSP_LEFT55_SEG56C_UNREACHED
    void unit_send_role_transfer_0077c470(void* unit, std::int32_t role_mask, std::int32_t take) override BSP_LEFT55_SEG56C_UNREACHED
    void unit_set_health_00877b90(void* unit, float health) override BSP_LEFT55_SEG56C_UNREACHED
    void mission_entity_kill_00926d90(void* entity, std::int32_t cause) override BSP_LEFT55_SEG56C_UNREACHED
    void* unit_group_member_record_at_0070d070(void* group, std::int32_t index) override BSP_LEFT55_SEG56C_UNREACHED
    void gun_set_barrel_reload_timer_0072cf00(void* gun, std::int32_t barrel, float time, std::int32_t set_full) override BSP_LEFT55_SEG56C_UNREACHED
    void entity_pose_refresh_world_00414db0(void* entity) override BSP_LEFT55_SEG56C_UNREACHED
    void* vehicle_class_get_or_create_00964790(std::int32_t type_id, bool read_race) override BSP_LEFT55_SEG56C_UNREACHED
    bool vehicle_class_vcall_18(void* descriptor, int value) override BSP_LEFT55_SEG56C_UNREACHED
    void* scene_find_node_by_name_excluding_00b72300(void* scene, const std::string& name, const std::string& excluded_root) override BSP_LEFT55_SEG56C_UNREACHED
    void scene_node_set_visibility_factor_00b6da70(void* node, float value, bool recurse) override BSP_LEFT55_SEG56C_UNREACHED
    const void* node_name_header_00b6d800(void* node) override BSP_LEFT55_SEG56C_UNREACHED
    void* node_find_child_by_header_00b6e750(void* node, const void* name_header, bool recursive) override BSP_LEFT55_SEG56C_UNREACHED
    void* node_find_child_by_name_00b6e750(void* node, const std::string& name, bool recursive) override BSP_LEFT55_SEG56C_UNREACHED
    void shallow_water_apply_00880220(void* object, const std::string& name) override BSP_LEFT55_SEG56C_UNREACHED
    void scoring_add_party_loss_0090ed90(void* scoring, std::int32_t party, const std::string& name, std::int32_t amount) override BSP_LEFT55_SEG56C_UNREACHED
    void scoring_ignored_kill_set_insert_0063f1e0(void* tree, void* entity) override BSP_LEFT55_SEG56C_UNREACHED
    void sensitive_unit_set_insert_008caee0(void* set, void* unit) override BSP_LEFT55_SEG56C_UNREACHED
    bool trigger_list_invoke_by_name_0074fbb0(void* list, const std::string& name) override BSP_LEFT55_SEG56C_UNREACHED
    void* line_of_sight_trigger_create_00750550(void* target, float range) override BSP_LEFT55_SEG56C_UNREACHED
    void* proximity_trigger_create_007505c0(const SceneCommandTarget& target, float range) override BSP_LEFT55_SEG56C_UNREACHED
    void* squad_ordnance_trigger_create_00750660() override BSP_LEFT55_SEG56C_UNREACHED
    void trigger_set_callback_name_00750790(void* trigger, const std::optional<std::string>& optional_name, const std::string& callback) override BSP_LEFT55_SEG56C_UNREACHED
    void trigger_list_add_0074fab0(void* list, void* trigger) override BSP_LEFT55_SEG56C_UNREACHED
    void* input_manager_singleton_004bec00() override BSP_LEFT55_SEG56C_UNREACHED
    void input_set_context_level_00a933f0(void* manager, int context, int level) override BSP_LEFT55_SEG56C_UNREACHED
    void warning_manager_clear_waiting_queue_0096bf70(void* manager) override BSP_LEFT55_SEG56C_UNREACHED
    void* recon_ensure_slot_008053c0(int index) override BSP_LEFT55_SEG56C_UNREACHED
    void game_pump_front_end_frame_004cac30(void* game) override BSP_LEFT55_SEG56C_UNREACHED
    void game_destroy_world_containers_004d2b10(void* game) override BSP_LEFT55_SEG56C_UNREACHED
    void game_set_cinematic_mode_004cd0f0(void* game, bool hide, bool allow_simulation, bool p4) override BSP_LEFT55_SEG56C_UNREACHED
    void interface_get_sound_listener_0068a670(void* interface_object, ListenerMatrix& matrix, Vec3& velocity) override BSP_LEFT55_SEG56C_UNREACHED
    void* effect_manager_singleton_004d1100() override BSP_LEFT55_SEG56C_UNREACHED
    void effect_manager_update_00867ee0(void* manager, float delta, void* reference_node) override BSP_LEFT55_SEG56C_UNREACHED
    void listener_sink_vcall_04(void* object, const ListenerMatrix& matrix, Vec3 velocity) override BSP_LEFT55_SEG56C_UNREACHED
    void debug_trace_004254b0(const char* text) override BSP_LEFT55_SEG56C_UNREACHED
    bool native_string_equals_insensitive_00425850(const std::string& value, const char* text) override BSP_LEFT55_SEG56C_UNREACHED
    void sort_scored_entries_008c9e10(ScoredEntry* first, ScoredEntry* last, std::int32_t ideal) override BSP_LEFT55_SEG56C_UNREACHED
    void crt_invalid_parameter_00bf6713() override BSP_LEFT55_SEG56C_UNREACHED
};
#pragma warning(pop)
#undef BSP_LEFT55_SEG56C_UNREACHED

}  // namespace bsp::cc13_left55::seg56c
