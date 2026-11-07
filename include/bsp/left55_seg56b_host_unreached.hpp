#pragma once
// A base for host adapters over bsp::cc13_left55::seg56b::Left55Seg56bHost
// (include/bsp/cc13_left55_seg56b.hpp), with its Left55FailureRepairEnableHost
// base.
//
// Left55Seg56bHost carries the engine services of the segment-56 Lua bindings
// 008A8480..008B0470 as pure virtuals, and no game host implemented it, so the
// bindings that take it never ran. This class overrides every one of them with
// a loud failure; an adapter derives from it and overrides only the services
// its own path reaches, each mapped onto the host's real objects. A service the
// adapter does not override throws std::logic_error naming the service, so a
// path the adapter did not read fails at once instead of running on an
// invented value.
//
// Written mechanically from the declarations in cc13_left55_seg56b.hpp (goal 5
// chain 655), one override per pure virtual, same signature, in the header's
// order, as bsp/seg56_host_unreached.hpp does for Seg56Host. A C++ adapter base,
// not an image type: no address, no image layout. Keep it in step with
// Left55Seg56bHost.

#include "bsp/cc13_left55_seg56b.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace bsp::cc13_left55::seg56b {

[[noreturn]] inline void left55_seg56b_unreached(const char* service) {
    throw std::logic_error(std::string("Left55Seg56bHost service not provided by this adapter: ")
        + service);
}

// One macro body, so nothing reading the sources for implementations of a
// service (tools/wire_readiness.py among them) takes one for an implementation.
#define BSP_LEFT55_SEG56B_UNREACHED { ::bsp::cc13_left55::seg56b::left55_seg56b_unreached(__func__); }
#pragma warning(push)
#pragma warning(disable : 4100)  // the parameter names document the services
class Left55Seg56bHostUnreached : public Left55Seg56bHost {
public:
    bool entity_vcall_5c(void* entity, int class_id) override BSP_LEFT55_SEG56B_UNREACHED
    void repair_task_set_failure_repair_enabled_00939fe0(void* task,
        bool enabled) override BSP_LEFT55_SEG56B_UNREACHED
    void entity_vcall_2c(void* entity, int party, std::int32_t field_58,
        int* zero) override BSP_LEFT55_SEG56B_UNREACHED
    Vec3 entity_vcall_34(void* entity) override BSP_LEFT55_SEG56B_UNREACHED
    float entity_vcall_38(void* entity) override BSP_LEFT55_SEG56B_UNREACHED
    void entity_vcall_d8(void* entity) override BSP_LEFT55_SEG56B_UNREACHED
    void entity_vcall_114(void* entity) override BSP_LEFT55_SEG56B_UNREACHED
    void entity_vcall_118(void* entity, const void* vector) override BSP_LEFT55_SEG56B_UNREACHED
    void entity_vcall_11c(void* entity, float radians) override BSP_LEFT55_SEG56B_UNREACHED
    void entity_vcall_120(void* entity, const float* matrix) override BSP_LEFT55_SEG56B_UNREACHED
    void* entity_vcall_140(void* entity) override BSP_LEFT55_SEG56B_UNREACHED
    void entity_vcall_178(void* entity, CommandTargetBuffer& out) override BSP_LEFT55_SEG56B_UNREACHED
    void entity_pose_refresh_world_00414db0(void* entity) override BSP_LEFT55_SEG56B_UNREACHED
    void matrix_build_orthogonal_scaled_affine_inverse_00b63d50(void* out,
        void* source) override BSP_LEFT55_SEG56B_UNREACHED
    Vec3 vector_transform_direction_0042d0d0(const Vec3& source, void* matrix,
        bool normalize) override BSP_LEFT55_SEG56B_UNREACHED
    Vec3 vector_transform_affine_point_004142e0(const Vec3& point,
        void* matrix) override BSP_LEFT55_SEG56B_UNREACHED
    float vector_length_0042b2f0(const Vec3& vector) override BSP_LEFT55_SEG56B_UNREACHED
    bool unit_is_formation_follower_007788b0(void* unit) override BSP_LEFT55_SEG56B_UNREACHED
    bool unit_is_group_leader_00778890(void* unit) override BSP_LEFT55_SEG56B_UNREACHED
    void* unit_group_member_at_0070d060(void* group, int index) override BSP_LEFT55_SEG56B_UNREACHED
    void mission_entity_kill_00926d90(void* entity, int cause) override BSP_LEFT55_SEG56B_UNREACHED
    void recon_clear_forced_level_00803b50(void* record) override BSP_LEFT55_SEG56B_UNREACHED
    void invalid_parameter_00bf6713() override BSP_LEFT55_SEG56B_UNREACHED
    std::uint32_t vector_size_0043ec50(void* vector) override BSP_LEFT55_SEG56B_UNREACHED
    int repair_category_from_name_0081ad40(const std::string& name) override BSP_LEFT55_SEG56B_UNREACHED
    void unit_set_repair_level_00827a40(void* unit, int category,
        int level) override BSP_LEFT55_SEG56B_UNREACHED
    int unit_get_repair_level_0080e130(void* unit, int category) override BSP_LEFT55_SEG56B_UNREACHED
    void* air_ops_block_006bcd20(void* entity, bool require_active) override BSP_LEFT55_SEG56B_UNREACHED
    void air_ops_block_set_orders_006bf650(void* block, bool first, bool second,
        int third) override BSP_LEFT55_SEG56B_UNREACHED
    void* game_find_border_zone_00489c40(void* game, const Vec3& key) override BSP_LEFT55_SEG56B_UNREACHED
    bool game_point_outside_map_bounds_0071c4f0(void* game,
        const Vec3& point) override BSP_LEFT55_SEG56B_UNREACHED
    void world_first_box_edge_crossing_004bc120(void* game, const Vec3& a, const Vec3& b,
        BoxEdgeCrossing& out) override BSP_LEFT55_SEG56B_UNREACHED

           void* command_target_resolve_object_00521ea0(CommandTargetBuffer& target) override BSP_LEFT55_SEG56B_UNREACHED
    std::string native_string_from_int_004260b0(int value) override BSP_LEFT55_SEG56B_UNREACHED
    void game_end_scene_004d7970(void* game, bool aborted) override BSP_LEFT55_SEG56B_UNREACHED
    void session_send_e4_004c9780_0076a9f0(void* session, void* first, int second,
        int out) override BSP_LEFT55_SEG56B_UNREACHED
    bool object_vcall_198(void* object) override BSP_LEFT55_SEG56B_UNREACHED
    void object_vcall_184(void* object) override BSP_LEFT55_SEG56B_UNREACHED
    void scoring_set_slot_ranking_00905350(void* scoring, int slot,
        int ranking) override BSP_LEFT55_SEG56B_UNREACHED
    void scoring_force_refresh_009150a0(void* scoring, int slot) override BSP_LEFT55_SEG56B_UNREACHED
    void scoring_add_action_score_0090f140(void* scoring, int slot, const std::string& key,
        int value) override BSP_LEFT55_SEG56B_UNREACHED
    void scoring_add_ship_score_0090f1a0(void* scoring, int slot, const std::string& key,
        int value) override BSP_LEFT55_SEG56B_UNREACHED
    void scoring_add_plane_score_0090f200(void* scoring, int slot, const std::string& key,
        int value) override BSP_LEFT55_SEG56B_UNREACHED
    void scoring_add_command_score_0090f260(void* scoring, int slot, const std::string& key,
        int value) override BSP_LEFT55_SEG56B_UNREACHED
    void scoring_grant_award_0090ede0(void* scoring, int slot, const std::string& token, int value,
        int flag) override BSP_LEFT55_SEG56B_UNREACHED
    bool profile_is_unlock_satisfied_007fc4c0(void* profile,
        const std::string& expression) override BSP_LEFT55_SEG56B_UNREACHED
    void route_message_0077c2a0(void* target, const PartyIdMessage& message, int flags,
        int out) override BSP_LEFT55_SEG56B_UNREACHED
    void route_message_0077c2a0(void* target, const CommandBuildingOwnerMessage& message,
        int flags, int out) override BSP_LEFT55_SEG56B_UNREACHED
    LuaRef frame_new_table_00b67930(LuaFrame& frame) override BSP_LEFT55_SEG56B_UNREACHED
    LuaRef frame_get_by_literal_00b67910(LuaFrame& frame,
        const char* name) override BSP_LEFT55_SEG56B_UNREACHED
    LuaRef lua_get_by_native_string_00b678e0(LuaRef table,
        const std::string& key) override BSP_LEFT55_SEG56B_UNREACHED
    void lua_push_value_00b663d0(LuaRef object) override BSP_LEFT55_SEG56B_UNREACHED
    void table_set_integer_at_index_00b665d0(LuaRef table, int key,
        int value) override BSP_LEFT55_SEG56B_UNREACHED
    void table_set_number_field_00b67400(LuaRef table, const std::string& key,
        float value) override BSP_LEFT55_SEG56B_UNREACHED
    void table_set_vector3_fields_0088ba30(LuaRef table,
        const Vec3& value) override BSP_LEFT55_SEG56B_UNREACHED
    void table_set_new_table_at_index_00b66670(LuaRef table, int index) override BSP_LEFT55_SEG56B_UNREACHED
    LuaRef table_get_by_index_00b67720(LuaRef table, int index) override BSP_LEFT55_SEG56B_UNREACHED
    void table_set_string_integer_00b67460(LuaRef table, const std::string& key,
        int value) override BSP_LEFT55_SEG56B_UNREACHED
    void table_set_boolean_00b673a0(LuaRef table, const std::string& key,
        bool value) override BSP_LEFT55_SEG56B_UNREACHED
    void table_set_cstring_00b66790(LuaRef table, const char* key,
        const char* value) override BSP_LEFT55_SEG56B_UNREACHED
    void* movcam_ensure_target_005cd160(void* movcam) override BSP_LEFT55_SEG56B_UNREACHED
    void* movcam_collect_variants_00887370(LuaArgs& args, int first_slot) override BSP_LEFT55_SEG56B_UNREACHED
    void movcam_set_path_walk_point_property_007a74f0(void* target, LuaArgs& args, int value_index,
        float point, bool flag, float number, const std::string& name, void* variants) override BSP_LEFT55_SEG56B_UNREACHED
    void session_broadcast_movcam_path_walk_point_property_0076d5b0(void* session, LuaArgs& args,
        int value_index, float point, bool flag, float number) override BSP_LEFT55_SEG56B_UNREACHED
};
#pragma warning(pop)
#undef BSP_LEFT55_SEG56B_UNREACHED

}  // namespace bsp::cc13_left55::seg56b
