#pragma once
// Packet cc13-seg2: segment 2 (0042A920..0043E4D0) leaves. Camera mover field
// setters, the observer-reference setters, vector and matrix helpers, the
// vehicle class registry and profile manager lifetime, and the sized-pool
// string releases. docs/CC13_SEG2.md lists every function with its ABI.
//
// Objects are reached through Raw (byte offsets into the Win32 layout); every
// call the image makes goes through a Seg2Host method named for its target.
// Descriptive names are hypotheses, not recovered symbols.

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_sys {
struct LazySingletonHost;
}

namespace bsp::profile_seg2_cc13 {

static_assert(sizeof(void*) == 4, "segment 2 objects are modelled in their Win32 layout");

// Raw view of an image object: at<T>(off) is the field at that byte offset.
struct Raw {
    std::uint8_t* p;
    template <class T>
    T& at(std::size_t off) const noexcept { return *reinterpret_cast<T*>(p + off); }
    void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};

// A 4x4 float matrix as the image stores it (16 floats, rows of four).
struct Mat16 {
    float m[16];
};

struct Seg2Host {
    virtual ~Seg2Host() = default;
    // Image globals (dword storage at a fixed address) and image data addresses.
    virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
    virtual void* image_address(std::uint32_t address) = 0;
    // CRT and kernel imports.
    virtual void free_00bf6989(void* block) = 0;                       // _free
    virtual long interlocked_decrement_ce2220(long* addend) = 0;      // [00CE2220]
    // Observer pairs: ECX = observed, EDX = the observer reference sub-object.
    virtual void observer_unregister_006952a0(void* observed, void* ref) = 0;
    virtual void observer_register_00694a60(void* observed, void* ref) = 0;
    // Virtual slots, ECX = object.
    virtual bool object_is_class_5c(void* object, std::int32_t class_id) = 0;  // vtable +5Ch(id)
    virtual void* object_slot_04(void* object) = 0;                            // vtable +4h()
    virtual void object_slot_00(void* object) = 0;                             // vtable +0h(), no args
    virtual bool object_slot_e8(void* object) = 0;                             // vtable +E8h(), tail jump
    virtual void entity_slot_dc(void* entity, float value) = 0;                // vtable +DCh(float)
    // Transform node: 00B6DB70 rebuilds the world matrix (ECX = node).
    virtual void transform_refresh_world_00b6db70(Raw node) = 0;
    // 00414DB0, ECX = entity: refresh the pose before reading it.
    virtual void entity_pose_refresh_00414db0(Raw entity) = 0;
    // 00BF701A _CIatan2 on (ST1 = y, ST0 = x).
    virtual double ciatan2_00bf701a(double y, double x) = 0;
    // Matrix builders: __fastcall(ECX = out, EDX = &angle), EAX = out.
    virtual void build_rotation_x_00b64640(Mat16& out, const float& angle) = 0;
    virtual void build_rotation_y_00b646e0(Mat16& out, const float& angle) = 0;
    virtual void build_rotation_z_00b64780(Mat16& out, const float& angle) = 0;
    // 004134F0, ECX = destination, stack source, RET 4: sequential FLD/FSTP copy.
    virtual void matrix_copy_004134f0(Raw destination, const Mat16& source) = 0;
    // 0085DC80, ECX = matrix: re-orthonormalise the basis rows.
    virtual void matrix_orthonormalize_0085dc80(Raw matrix) = 0;
    // Camera copy bodies (__thiscall(this, other), RET 4).
    virtual void ship_camera_copy_state_0042dc20(Raw camera, Raw other) = 0;
    virtual void plane_chase_copy_if_same_target_0042cd50(Raw camera, Raw other) = 0;
    virtual void submarine_copy_if_same_target_0042dfd0(Raw camera, Raw other) = 0;
    // 00798D70, ECX = holder: the camera a class-54h holder carries.
    virtual void* holder_camera_00798d70(void* holder) = 0;
    // 00424C40: the game settings singleton.
    virtual Raw game_settings_00424c40() = 0;
    // Lua owners.
    virtual void lua_state_owner_close_00b65e80(Raw owner) = 0;   // ECX = this+38h (tail jump)
    virtual void lua_object_destruct_00b67700(Raw object) = 0;    // ECX = this+4
    // Vehicle class registry vectors (__thiscall, RET 4).
    virtual void party_rows_reserve_00435960(Raw vector, std::uint32_t count) = 0;
    virtual void party_rows_resize_004359e0(Raw vector, std::uint32_t count) = 0;
    virtual void registry_cache_resize_00437bd0(Raw vector, std::uint32_t count) = 0;
    // Sized string pool: 00419CC0 returns the pool, 00BD1510 (ECX = pool) takes the block back.
    virtual void* string_pool_00419cc0() = 0;
    virtual void string_pool_return_00bd1510(void* pool, void* block, std::uint32_t bytes,
                                             std::int32_t count) = 0;
};

// Converts an image dword holding a pointer into a Raw view.
inline Raw raw_from_u32(std::uint32_t value) noexcept {
    return Raw{reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(value))};
}

// --- camera mover fields -------------------------------------------------
void camera_mover_set_camera_node_0042a920(Raw mover, void* node) noexcept;
void camera_set_shake_max_0042a940(Raw camera, float value) noexcept;
void camera_set_shake_base_0042a970(Raw camera, float value) noexcept;
void ship_camera_on_observed_released_0042a9d0(Raw camera, void* observed) noexcept;
void plane_chase_camera_on_observed_released_0042aad0(Raw camera, void* observed) noexcept;
float entity_get_heading_0042ba40(Raw entity, Seg2Host& host);
void camera_mover_get_default_extent_0042ba70(float out[2]) noexcept;
float camera_mover_return_ten_0042ba90() noexcept;
bool camera_mover_forward_to_target_slot_e8_0042bad0(Raw mover, Seg2Host& host);
void camera_mover_clear_byte_c_0042d800(Raw object) noexcept;
void ship_captain_camera_copy_state_if_same_ship_0042df80(Raw camera, Raw other, Seg2Host& host);
void plane_chase_camera_adopt_from_0042cea0(Raw camera, void* other, Seg2Host& host);
void ship_captain_camera_copy_state_from_camera_0042eac0(Raw camera, void* other, Seg2Host& host);
void submarine_camera_adopt_from_0042eb90(Raw camera, void* other, Seg2Host& host);
void entity_set_ref_target_and_offset_0042e0e0(Raw entity, void* target, const float offset[3],
                                               Seg2Host& host);
void camera_mover_set_target_0042f650(Raw mover, void* target, Seg2Host& host);
void camera_mover_set_target_and_mode_00432e60(Raw mover, void* target, std::uint32_t mode,
                                               Seg2Host& host);

// --- trivial virtual bodies ----------------------------------------------
bool entity_hit_not_handled_0042baf0(void* hit) noexcept;
bool entity_aim_offset_still_valid_0042bb20(void* a, void* b, void* c) noexcept;
bool return_false_0042bb40() noexcept;
void tick_element_place_step_pose_0042bb70(void* arg) noexcept;
void tick_element_tick_advance_0042bb80(void* arg) noexcept;
void tick_element_commit_step_pose_0042bb90() noexcept;
void tick_element_unused_slot_0042bba0(void* arg) noexcept;
bool tick_element_predicate_0042bbb0() noexcept;

// --- vectors and matrices ------------------------------------------------
void vector3_divide_scalar_to_0042ae20(const float v[3], float out[3], float s) noexcept;
float* vector3_scale_in_place_0042ae50(float v[3], float s) noexcept;
float* vector3_add_in_place_0042aeb0(float v[3], const float w[3]) noexcept;
float* vector3_subtract_in_place_0042aee0(float v[3], const float w[3]) noexcept;
float vector3_dot_0042b220(const float a[3], const float b[3]) noexcept;
void matrix_set_rotation_x_0042b3c0(Raw matrix, float angle, Seg2Host& host);
void matrix_set_rotation_y_0042b3f0(Raw matrix, float angle, Seg2Host& host);
void matrix_set_rotation_z_0042b420(Raw matrix, float angle, Seg2Host& host);
Raw matrix_set_rotation_x_negated_0042b450(Raw matrix, float angle, Seg2Host& host);
Raw matrix_set_rotation_y_negated_0042b490(Raw matrix, float angle, Seg2Host& host);
Raw matrix_assign_0042b680(Raw matrix, const Mat16& source, Seg2Host& host);
Raw matrix_orthonormalize_return_this_0042b7f0(Raw matrix, Seg2Host& host);
Raw matrix_set_identity_0042d510(Raw matrix, Seg2Host& host);
Raw matrix_set_identity_0042e8c0(Raw matrix, Seg2Host& host);
Raw matrix_set_forward_up_orthonormalize_0042d5a0(Raw matrix, const float forward[3], const float up[3],
                                                  Seg2Host& host);
float* transform_get_world_matrix_0042aff0(Raw node, Seg2Host& host);
void transform_get_world_position_0042d210(Raw node, float out[3], Seg2Host& host);

// --- observer references and casts ---------------------------------------
void observer_ref_set_0042d870(Raw ref, void* observed, Seg2Host& host);
Raw observer_ref_set_0042e700(Raw ref, void* observed, Seg2Host& host);
Raw observer_ref_set_0042e800(Raw ref, void* observed, Seg2Host& host);
Raw observer_ref_set_0042eca0(Raw ref, void* observed, Seg2Host& host);
void observer_ref_on_observed_released_0042ec20(Raw ref, void* released, Seg2Host& host);
void observer_ref_on_observed_released_0042ec50(Raw ref, void* released, Seg2Host& host);
void* class_46_dynamic_cast_0042e840(void* object, Seg2Host& host);
const char* unit_instance_name_or_fallback_0042e950(Raw unit, Seg2Host& host);

// --- singletons, registry and profile manager ----------------------------
void global_config_owner_destruct_singleton_base_0042ac00(Raw object, Seg2Host& host);
void profile_manager_owner_destruct_singleton_base_00435750(Raw object, Seg2Host& host);
void vehicle_class_registry_destruct_singleton_base_00435770(Raw object, Seg2Host& host);
void lua_script_holder_close_00435850(Raw holder, Seg2Host& host);
void profile_manager_destruct_004358c0(Raw manager, Seg2Host& host);
void vehicle_class_registry_reset_index_tables_004356f0(Raw registry) noexcept;
Raw vehicle_class_registry_construct_00437e10(Raw registry, Seg2Host& host);
void vehicle_class_registry_destruct_00437ea0(Raw registry, Seg2Host& host);
void registry_cache_destroy_00437dc0(Raw vector, Seg2Host& host);
void registry_cache_destroy_00437df0(Raw vector, Seg2Host& host);
void party_rows_destroy_00436370(Raw vector, Seg2Host& host);
void party_rows_destroy_00436460(Raw vector, Seg2Host& host);
// The two lazy getters share the cc13-sys4 body (bsp::cc13_sys::lazy_singleton_get).
void* vehicle_class_registry_get_00437f50(void*& global_00f8a0b8, cc13_sys::LazySingletonHost& host);
void* game_tuning_get_0042e740(void*& global_00f87440, cc13_sys::LazySingletonHost& host);
void scoped_byte_override_f8d394_construct_0043ab50(Raw scope, std::uint8_t value, Seg2Host& host);
void scoped_byte_override_f8d394_destruct_0043ab80(Raw scope, Seg2Host& host);

// --- sized-pool string storage releases ----------------------------------
// The string is {length at +0, data at +4} relative to `string`.
void native_string_destroy_storage_00431f20(Raw string, Seg2Host& host);
void native_string_destroy_storage_00431f40(Raw string, Seg2Host& host);
void native_string_at_0c_destroy_storage_00436490(Raw owner, Seg2Host& host);
void native_string_at_8_destroy_storage_0043bae0(Raw owner, Seg2Host& host);
void native_string_at_8_destroy_storage_0043d090(Raw owner, Seg2Host& host);

}  // namespace bsp::profile_seg2_cc13
