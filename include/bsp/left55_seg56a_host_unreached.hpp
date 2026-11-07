#pragma once
// A base for host adapters over bsp::cc13_left55::seg56a::Left55Seg56aHost
// (include/bsp/cc13_left55_seg56a.hpp).
//
// Left55Seg56aHost carries the engine services of its segment-56 Lua bindings as pure
// virtuals, and no game host implemented it, so the bindings that take it never
// ran. This class overrides every one of them with a loud failure; an adapter
// derives from it and overrides only the services its own path reaches, each
// mapped onto the host's real objects. A service the adapter does not override
// throws std::logic_error naming the service, so a path the adapter did not read
// fails at once instead of running on an invented value.
//
// Written mechanically from the declarations in cc13_left55_seg56a.hpp (goal 5 chain 655),
// one override per pure virtual, same signature, in the header's order, as
// bsp/seg56_host_unreached.hpp does for Seg56Host. A C++ adapter base, not an
// image type: no address, no image layout. Keep it in step with Left55Seg56aHost.

#include "bsp/cc13_left55_seg56a.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace bsp::cc13_left55::seg56a {

[[noreturn]] inline void left55_seg56a_unreached(const char* service) {
    throw std::logic_error(std::string("Left55Seg56aHost service not provided by this adapter: ")
        + service);
}

// One macro body, so nothing reading the sources for implementations of a
// service (tools/wire_readiness.py among them) takes one for an implementation.
#define BSP_LEFT55_SEG56A_UNREACHED { ::bsp::cc13_left55::seg56a::left55_seg56a_unreached(__func__); }
#pragma warning(push)
#pragma warning(disable : 4100)  // the parameter names document the services
class Left55Seg56aHostUnreached : public Left55Seg56aHost {
public:
    void* argument_ptr_field_00888be0(int index) override BSP_LEFT55_SEG56A_UNREACHED
    void* argument_ptr_field_00888d20(int index) override BSP_LEFT55_SEG56A_UNREACHED
    void* argument_ptr_field_00b67800_00b662d0(int index) override BSP_LEFT55_SEG56A_UNREACHED
    SceneCommandTarget read_command_target_0088a810(int index) override BSP_LEFT55_SEG56A_UNREACHED
    void push_this_table_entry_004260b0_00b663d0(std::uint16_t id) override BSP_LEFT55_SEG56A_UNREACHED
    bool entity_vcall_5c(void* entity, std::int32_t class_id) override BSP_LEFT55_SEG56A_UNREACHED
    void* entity_vcall_114(void* entity) override BSP_LEFT55_SEG56A_UNREACHED
    void entity_vcall_1f0(void* entity) override BSP_LEFT55_SEG56A_UNREACHED
    Vec3 entity_vcall_34(void* entity) override BSP_LEFT55_SEG56A_UNREACHED
    void entity_vcall_40(void* entity, const Vec3& value) override BSP_LEFT55_SEG56A_UNREACHED
    float entity_vcall_38(void* entity) override BSP_LEFT55_SEG56A_UNREACHED
    void entity_vcall_88(void* entity, const float* matrix) override BSP_LEFT55_SEG56A_UNREACHED
    void entity_sub310_vcall_0c(void* sub_object) override BSP_LEFT55_SEG56A_UNREACHED
    void entity_pose_refresh_world_00414db0(void* entity) override BSP_LEFT55_SEG56A_UNREACHED
    float* matrix_multiply_00413920(const void* left, float* out,
        const void* right) override BSP_LEFT55_SEG56A_UNREACHED
    void matrix_copy_004134f0(void* destination,
        const void* source) override BSP_LEFT55_SEG56A_UNREACHED
    void matrix_affine_inverse_00b63d50(void* out,
        const void* source) override BSP_LEFT55_SEG56A_UNREACHED
    void matrix_orthonormalize_basis_rows_0085dc80(float* matrix) override BSP_LEFT55_SEG56A_UNREACHED
    Vec3 vector3_transform_direction_0042d0d0(const Vec3& source, const void* matrix,
        std::int32_t normalize) override BSP_LEFT55_SEG56A_UNREACHED
    void entity_set_world_pose_matrix_00741e90(void* entity,
        const void* matrix) override BSP_LEFT55_SEG56A_UNREACHED
    float vector3_compass_heading_007b4e90(const Vec3& vector) override BSP_LEFT55_SEG56A_UNREACHED
    void dyn_copy_matrix43_00c336c0(float out[12],
        const float* matrix) override BSP_LEFT55_SEG56A_UNREACHED
    void dyn_body_set_transform_00c56cf0(void* body,
        const float* transform) override BSP_LEFT55_SEG56A_UNREACHED
    void* unit_parts_object_0080e490(void* unit) override BSP_LEFT55_SEG56A_UNREACHED
    void gun_compute_aim_for_target_0072c170(void* gun, std::int32_t value, void* target,
        const Vec3& point_a, const Vec3& point_b, float number, bool flag) override BSP_LEFT55_SEG56A_UNREACHED
    void route_hull_impact_message_0080ff80_0077c2a0(void* entity, std::int32_t count,
        const float point[3], int flags, int argument) override BSP_LEFT55_SEG56A_UNREACHED
    void flag_grid_set_cell_00861c20(void* grid, std::int32_t column, std::int32_t row,
        bool value) override BSP_LEFT55_SEG56A_UNREACHED
    void* spatial_index_singleton_0042e630() override BSP_LEFT55_SEG56A_UNREACHED
    bool spatial_index_query_box_0098b450(void* index, const float min[3],
        const float max[3]) override BSP_LEFT55_SEG56A_UNREACHED
    bool world_ground_height_at_00903860(void* world, const float point[3],
        float* out_height) override BSP_LEFT55_SEG56A_UNREACHED
    void* air_ops_block_006bcd20(void* entity,
        bool require_active) override BSP_LEFT55_SEG56A_UNREACHED
    void* vehicle_class_007b8a80(std::int32_t type_id) override BSP_LEFT55_SEG56A_UNREACHED
    void* vehicle_class_00964790(std::int32_t type_id,
        bool read_race) override BSP_LEFT55_SEG56A_UNREACHED
    void air_ops_update_slot_006cd350(void* block, std::int32_t slot, void* vehicle_class,
        std::int32_t count, std::int32_t equipment) override BSP_LEFT55_SEG56A_UNREACHED
    void air_ops_slot_command_006ccda0(void* block, std::int32_t slot, std::int32_t order,
        std::int32_t argument, std::int32_t owner, std::int32_t apply) override BSP_LEFT55_SEG56A_UNREACHED
    void air_ops_fill_slot_006c4780(void* block, std::int32_t slot, void* vehicle_class,
        std::int32_t count) override BSP_LEFT55_SEG56A_UNREACHED
    void air_slot_set_10_006ba4b0(void* slot_record,
        std::int32_t value) override BSP_LEFT55_SEG56A_UNREACHED
    void route_airbase_setslot_message_006bd800_0077c2a0(void* target, void* block,
        std::int32_t slot, int flags, int argument) override BSP_LEFT55_SEG56A_UNREACHED
    void air_ops_remove_stock_006c7b10(void* block, void* vehicle_class,
        std::int32_t count) override BSP_LEFT55_SEG56A_UNREACHED
    void air_ops_block_push_b8_006ca730(void* block,
        void* squadron) override BSP_LEFT55_SEG56A_UNREACHED
    void mission_entity_kill_00926d90(void* entity,
        std::int32_t cause) override BSP_LEFT55_SEG56A_UNREACHED
    void* command_registry_find_by_name_00467170(const std::string& name) override BSP_LEFT55_SEG56A_UNREACHED
    void entity_issue_command_0077d600(void* entity, void* command,
        const SceneCommandTarget& target, std::int32_t flags) override BSP_LEFT55_SEG56A_UNREACHED
    void route_message_0077c2a0(void* entity, const CommandBuildingLevelMessage& message,
        int flags, int argument) override BSP_LEFT55_SEG56A_UNREACHED
    void route_message_0077c2a0(void* entity, const StartAttackRunMessage& message, int flags,
        int argument) override BSP_LEFT55_SEG56A_UNREACHED
    void game_replace_scene_border_004ca760(void* game, const Vec3& first,
        const Vec3& second) override BSP_LEFT55_SEG56A_UNREACHED
    void weapon_director_send_subkind6_0071e150(void* director,
        bool value) override BSP_LEFT55_SEG56A_UNREACHED
    void weapon_director_send_subkind2_0071dd30(void* director,
        std::int32_t value) override BSP_LEFT55_SEG56A_UNREACHED
    void unit_request_ordnance_release_007bbba0(void* unit) override BSP_LEFT55_SEG56A_UNREACHED
    bool observer_target_within_range_0070eb00(void* member,
        float range) override BSP_LEFT55_SEG56A_UNREACHED
    float weapon_drop_fall_time_007bcc80(void* unit,
        float height) override BSP_LEFT55_SEG56A_UNREACHED
    float squadron_min_member_908_007ee150(void* squadron) override BSP_LEFT55_SEG56A_UNREACHED
    bool unit_is_local_players_role_slot_00779b60(void* unit) override BSP_LEFT55_SEG56A_UNREACHED
    bool unit_is_formation_follower_007788b0(void* unit) override BSP_LEFT55_SEG56A_UNREACHED
    void route_relocate_heading_message_007edeb0_0077c2a0(void* entity, float heading,
        std::int32_t argument, int flags, int route_argument) override BSP_LEFT55_SEG56A_UNREACHED
};
#pragma warning(pop)
#undef BSP_LEFT55_SEG56A_UNREACHED

}  // namespace bsp::cc13_left55::seg56a
