#pragma once
// A Seg29UHost whose every service traps (std::abort). Callers derive from it
// and override only the boundaries the body they run actually reaches, so a
// path this process has not bound fails loudly instead of answering with an
// unrelated placeholder. Moved verbatim from the units host's
// LandingAssignmentLookupHost (goal 5 chain 628), which now derives from it.
#include "bsp/cc13_seg29U.hpp"

#include <cstdlib>

namespace bsp::cc13_seg29U {

class Seg29UTrapHost : public Seg29UHost {
    using NativeStr = bsp::cc13_seg29U::NativeStr;
public:
    void* global_pointer(std::uint32_t) override { std::abort(); }
    void* global_address(std::uint32_t) override { std::abort(); }
    void invalid_parameter_00bf6713() override { std::abort(); }
    void free_00bf65ac(void*) override { std::abort(); }
    void* operator_new_00bf681b(std::uint32_t) override { std::abort(); }
    void entity_pose_refresh_00414db0(void*) override { std::abort(); }
    void* matrix_multiply_00413920(void*,  void*,  void*) override { std::abort(); }
    void matrix_copy_004134f0(void*,  void*) override { std::abort(); }
    void matrix_inverse_00b63d50(void*,  void*) override { std::abort(); }
    void* transform_point_004142e0(void*,  void*,  void*) override { std::abort(); }
    void object_vslot_04(void*,  std::uint32_t,  void*) override { std::abort(); }
    void path_derive_knots_007af150(void*) override { std::abort(); }
    void game_21d0_register_0048ca60(void*,  void*,  std::int32_t) override { std::abort(); }
    void observer_unregister_pair_006952a0(void*,  void*) override { std::abort(); }
    void callback_owner_destroy_00695870(void*) override { std::abort(); }
    void game_entity_destruct_009287b0(void*) override { std::abort(); }
    void entity_default_slot_a4_009295b0(void*) override { std::abort(); }
    NativeStr* native_string_from_int_004260b0(NativeStr*,  std::int32_t) override { std::abort(); }
    void native_string_resize_0041dd40(NativeStr*,  std::uint32_t,  char) override { std::abort(); }
    void memcpy_00bf7680(void*,  const void*,  std::uint32_t) override { std::abort(); }
    void native_string_concat_004261a0(const NativeStr*,  NativeStr*,  const NativeStr*) override { std::abort(); }
    void* sized_pool_00419cc0() override { std::abort(); }
    void sized_pool_return_00bd1510(void*,  void*,  std::uint32_t,  std::uint32_t) override { std::abort(); }
    void* property_bag_find_008f2260(void*,  const char*) override { std::abort(); }
    const char* property_string_value_008f0df0(void*) override { std::abort(); }
    void* entity_registry_find_00925a90(void*,  const char*) override { std::abort(); }
    bool entity_is_kind_of_5c(void*,  std::int32_t) override { std::abort(); }
    void list_push_back_006acd80(void*,  void* const*) override { std::abort(); }
    void object_vslot_08_float(void*,  float) override { std::abort(); }
    std::int32_t object_vslot_0c(void*) override { std::abort(); }
    double object_vslot_10_x87(void*) override { std::abort(); }
    void* object_vslot_18(void*) override { std::abort(); }
    const float* object_vslot_34(void*,  float*) override { std::abort(); }
    void object_vslot_44(void*,  float*) override { std::abort(); }
    void* object_vslot_114(void*) override { std::abort(); }
    void enter_critical_section(void*) override { std::abort(); }
    void leave_critical_section(void*) override { std::abort(); }
    void free_00bf6989(void*) override { std::abort(); }
    void session_message_construct_base_0075b430(void*,  std::uint32_t) override { std::abort(); }
    void session_route_message_0077c2a0(void*,  void*,  std::uint32_t,  std::uint32_t) override { std::abort(); }
    std::uint32_t vehicle_class_registry_id_0095b9c0(void*) override { std::abort(); }
    std::uint32_t current_command_0071be40(void*) override { std::abort(); }
    void* active_target_descriptor_0071eb60(void*,  void*) override { std::abort(); }
    const float* target_descriptor_position_006f7dd0(void*) override { std::abort(); }
    float vector3_length_0042b2f0(const float*) override { std::abort(); }
    std::int32_t squadron_airborne_count_007ee5c0(void*) override { std::abort(); }
    void observer_ref_destruct_00653400(void*) override { std::abort(); }
    void observer_member_cleanup_00653390(void*) override { std::abort(); }
    float add_wrapped_angle_00438aa0(float,  float) override { std::abort(); }
    double sqrt_00bf7030(double) override { std::abort(); }
    double atan2_00bf701a(double,  double) override { std::abort(); }
    void format_to_buffer32_00467310(char*,  const char*,  std::int32_t) override { std::abort(); }
    void vehicle_class_mark_party_required_0095ba60(void*,  std::uint32_t) override { std::abort(); }
    void register_hidden_stock_unit_0046be90(void*,  void*) override { std::abort(); }
    void* landing_vector_at_006ba7b0(void*,  std::int32_t) override { std::abort(); }
};

}  // namespace bsp::cc13_seg29U
