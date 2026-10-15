#pragma once

// Segment 8 (00491170..004BA200) large leftovers that cc13-left8 did not reach:
// the scene-traffic path-runtime constructors (004A4B70 base, 004A5470,
// 004A50D0, 004A66B0), the buoy field (004A5850 and its TempAddFloatsam reader
// 004A5F30), the unit-attached traffic creation 004A6830 and the area kill
// 004A5C60. docs/CC13_LEFT8B.md lists every function with its evidence and the
// ones that were skipped.
//
// Objects are modelled in their Win32 layout through the byte-offset helpers of
// cc13_left8_seg8.hpp; Left8bHost extends Left8Host with the calls these bodies
// make that the first pass had no hook for. Every hook is invoked at the point
// the image calls the routine named in its comment.

#include <cstddef>
#include <cstdint>

#include "bsp/cc13_left8_seg8.hpp"

namespace bsp::cc13_left8b {

using cc13_left8::at;
using cc13_left8::BitRef;
using cc13_left8::LuaObject;
using cc13_left8::obj;
using cc13_left8::Object;

class Left8bHost : public cc13_left8::Left8Host {
public:
    // std::list / std::map head-node allocation (ECX = container, EAX = head).
    virtual Object list_buy_head_node_00485320(Object list) = 0;
    virtual Object list_buy_head_node_004870f0(Object list) = 0;
    // 0047D090 (ECX = &iterator {container, node}): std::map<uint32, float> ++.
    virtual void tree_iterator_increment_0047d090(Object iterator) = 0;
    // 00443880 (ECX = &iterator {container, node}): std::map<string, float> ++.
    virtual void string_float_map_iterator_increment_00443880(Object iterator) = 0;

    // Enumeration tables. 0048E960 __thiscall(ECX = [00E1867C], const char* name),
    // RET 4, EAX = table; 008F0D80 __thiscall(table, value), RET 4, EAX = name or 0.
    virtual Object enum_table_read_word_0048e960(Object tables, const char* name) = 0;
    virtual const char* enum_find_name_by_value_008f0d80(Object table, std::uint32_t value) = 0;

    // Vector helpers (ECX = vector).
    virtual void pointer_vector_push_back_00499190(Object vector, const Object* value) = 0;   // RET 4
    virtual void pointer_vector_push_back_00499210(Object vector, const Object* value) = 0;   // RET 4
    virtual void pointer_vector_push_back_00499410(Object vector, const Object* value) = 0;   // RET 4
    virtual void float_vector_push_back_0045afd0(Object vector, const float* value) = 0;      // RET 4
    virtual void native_string_vector_append_00450540(Object vector, const void* string) = 0; // RET 4
    // 0045A120 / 00496110 __thiscall(vector, &result, container, where, const T*), RET 10h.
    virtual void float_vector_insert_0045a120(Object vector, Object result, Object container, Object where,
                                              const float* value) = 0;
    virtual void pointer_vector_insert_00496110(Object vector, Object result, Object container, Object where,
                                                const Object* value) = 0;
    // 004809F0 __fastcall(ECX = float vector): divides every element by the sum.
    virtual void weights_normalize_004809f0(Object vector) = 0;
    // 004951A0 __thiscall(vector, const vector* source), RET 4: float vector assignment.
    virtual void float_vector_assign_004951a0(Object vector, Object source) = 0;
    // 00495C80 / 00495AE0 __thiscall(vector, count, value), RET 8: resize with a dword value.
    virtual void pointer_vector_resize_00495c80(Object vector, std::uint32_t count, std::uint32_t value) = 0;
    virtual void dword_vector_resize_00495ae0(Object vector, std::uint32_t count, std::uint32_t value) = 0;
    // 004A48D0 __thiscall(vector, count, vector value by value (16 bytes)), RET 14h.
    virtual void vector_vector_resize_004a48d0(Object vector, std::uint32_t count, const std::uint32_t value[4]) = 0;
    // 00427AC0 (ECX = vector of 8-byte native strings), EAX = size.
    virtual std::uint32_t native_string_vector_size_00427ac0(Object vector) = 0;

    // Classes.
    // 00964790 __fastcall(ECX = class key, DL = create flag), EAX = vehicle class.
    virtual Object vehicle_class_get_or_create_00964790(std::uint32_t key, bool create) = 0;
    virtual Object vehicle_visual_class_get_or_create_0047b3c0(Object vehicle_class) = 0;  // ECX = class
    virtual Object buoy_class_get_or_create_00475710(const void* name) = 0;               // ECX = NativeString*

    // Scene paths (ECX = path).
    virtual void camera_path_derive_knots_007af150(Object path) = 0;
    // 007B05E0 __thiscall(path, index), RET 4, ST0 = [knot[index] + 18h].
    virtual float path_knot_distance_007b05e0(Object path, std::uint32_t index) = 0;
    // 007AF740 / 007AF800 __thiscall(path, float3* out, index), RET 8, EAX = point.
    virtual const float* scene_path_point_to_world_cached_007af740(Object path, float* out, std::int32_t index) = 0;
    virtual const float* scene_path_transform_point_to_world_007af800(Object path, float* out,
                                                                      std::int32_t index) = 0;

    // Math. 00427D10 __thiscall(box {min, max}, const float3*), RET 4.
    virtual void aabb_include_point_00427d10(float* box, const float* point) = 0;
    // 00413920 __thiscall(lhs, float16* out, const float16* rhs), RET 8, EAX = out.
    virtual const float* matrix_multiply_00413920(Object lhs, float* out, const float* rhs) = 0;
    // 004142E0 __thiscall(const float3* point, float3* out, const float16* matrix), RET 8.
    virtual void vector3_transform_affine_point_004142e0(const float* point, float* out, const float* matrix) = 0;

    // Scene traffic runtime pass (__fastcall ECX = runtime; reads [runtime+16Ch]).
    virtual void traffic_runtime_pass_00487470(Object runtime) = 0;


    // 0048DC40 __thiscall(type, class, LuaObject* row), RET 8: the soldier type
    // base (reconstructed as classes_seg7_cc13 soldier_type_construct_0048dc40).
    virtual void soldier_type_construct_0048dc40(Object type, Object soldier_class, LuaObject* row) = 0;
    // 004954F0 __thiscall(vector, &result, c0, first, c1, last), RET 14h: erase a
    // native-string vector range.
    virtual void string_vector_erase_range_004954f0(Object vector, Object result, Object c0, Object first, Object c1,
                                                    Object last) = 0;

    // Landing-troop tick.
    virtual std::uint8_t global_u8(std::uint32_t address) = 0;
    virtual void landing_troop_update_004a0d20(Object troop) = 0;                       // ECX = troop
    virtual void landing_troop_spawn_missing_members_0048af00(Object troop) = 0;        // ECX = troop
    virtual void landing_troop_tick_fake_effects_0048a6d0(Object troop, float dt) = 0;  // RET 4
    virtual void landing_troop_tick_members_0048b120(Object troop, float dt) = 0;       // RET 4
    // 007B03C0 __thiscall(path, float t, float3* position, float3* forward, flag), RET 10h.
    virtual void camera_path_sample_world_007b03c0(Object path, float t, float* position, float* forward,
                                                   std::uint32_t flag) = 0;
    // Entity vtable +24Ch(float3* out, 1), RET 8, EAX = point; +118h() and +D8h().
    virtual const float* vcall_24c(Object entity, float* out, std::uint32_t one) = 0;
    virtual void vcall_118(Object entity) = 0;
    // 0048A010 __thiscall(entity, const float3* screen), RET 4, AL.
    virtual bool soldier_entity_update_clip_visibility_0048a010(Object entity, const float* screen) = 0;

    // Traffic layer rebuild.
    virtual void scene_entity_layers_reset_all_004814c0(Object traffic) = 0;  // ECX = traffic
    // 00495BA0 __thiscall(vector, count, 18h-byte box by value), RET 1Ch.
    virtual void box_vector_resize_00495ba0(Object vector, std::uint32_t count, const float fill[6]) = 0;
    // 0049DF50 __thiscall(vector<bool>, count, value), RET 8.
    virtual void bit_vector_resize_0049df50(Object vector, std::uint32_t count, std::uint32_t value) = 0;

    // Ship idle actors group. Dword vector resizes __thiscall(vector, count, value), RET 8.
    virtual void dword_vector_resize_00495770(Object vector, std::uint32_t count, std::uint32_t value) = 0;
    virtual void dword_vector_resize_004958c0(Object vector, std::uint32_t count, std::uint32_t value) = 0;
    virtual void dword_vector_resize_00495a20(Object vector, std::uint32_t count, std::uint32_t value) = 0;
    // 0049E050 __thiscall(vector, count, NativeString by value (8 bytes)), RET 0Ch.
    virtual void native_string_vector_resize_0049e050(Object vector, std::uint32_t count,
                                                      const std::uint32_t value[2]) = 0;
    // 0049E150 __thiscall(vector<bool>, &result, iterator where by value (0Ch), bool), RET 14h.
    virtual void bit_vector_insert_0049e150(Object vector, Object result, BitRef where, bool value) = 0;
    // Inserts __thiscall(vector, &result, container, where, const T*), RET 10h.
    virtual void pointer_vector_insert_00495830(Object vector, Object result, Object container, Object where,
                                                const Object* value) = 0;
    virtual void pointer_vector_insert_00495980(Object vector, Object result, Object container, Object where,
                                                const Object* value) = 0;
    virtual void int_vector_insert_00441f30(Object vector, Object result, Object container, Object where,
                                            const std::int32_t* value) = 0;

    // Plane debris spawn.
    virtual void vcall_34(Object entity, float* position) = 0;                        // vtable +34h(float3*), RET 4
    virtual bool unit_uses_small_turn_roll_limit_0047b880(Object entity) = 0;          // ECX = entity, AL
    // 00439950 (ECX = float3* out), EAX = out: a random unit direction (draws from the shared RNG).
    virtual const float* random_unit_direction_00439950(float* out) = 0;
    virtual void dword_vector_resize_00492210(Object vector, std::uint32_t count, std::uint32_t value) = 0;  // RET 8

    // Landing-lane formation. 007B04C0 __thiscall(path, float t, float3* position, a, b), RET 10h.
    virtual void camera_path_sample_point_007b04c0(Object path, float t, float* position, std::uint32_t a,
                                                   std::uint32_t b) = 0;
    // 00485920 __thiscall(list, next, prev, const value*), RET 0Ch, EAX = node; 0048FFB0 (list, count), RET 4.
    virtual Object list_buy_node_00485920(Object list, Object next, Object prev, const void* value) = 0;
    virtual void list_incsize_0048ffb0(Object list, std::uint32_t count) = 0;
};

// Path-runtime constructors (scene traffic). The base 004A4B70 is shared by the
// three derived runtimes.
Object path_runtime_construct_004a4b70(Left8bHost& host, Object runtime, Object parent, Object record);
Object scene_traffic_runtime_construct_004a50d0(Left8bHost& host, Object runtime, Object parent, Object record);
Object unit_path_runtime_construct_004a5470(Left8bHost& host, Object runtime, Object parent, Object unit,
                                            Object record);
Object landing_lane_construct_004a66b0(Left8bHost& host, Object lane, Object parent, Object unit, Object record);

// The buoy field and its TempAddFloatsam reader.
Object buoy_field_construct_004a5850(Left8bHost& host, Object field, Object parent, const float* box,
                                     Object class_names, Object weights, float gap, float deviation);
void temp_add_floatsam_004a5f30(Left8bHost& host, Object manager, LuaObject* table);

// Plane debris spawn (draws from the shared RNG 00BD2F10).
void plane_debris_spawn_0049a760(Left8bHost& host, Object spawner);

// The landing-lane formation update.
void landing_lane_update_formation_00497bb0(Left8bHost& host, Object lane, float time, bool force_visible);

// The ship idle actors group constructor.
Object ship_idle_group_construct_004a3610(Left8bHost& host, Object group, Object parent, Object unit);

// The soldier type with animation-name vectors.
Object soldier_type_with_anims_construct_0049fc80(Left8bHost& host, Object type, Object soldier_class,
                                                  LuaObject* row);

// The scene-traffic layer cells.
void traffic_layers_rebuild_0049e560(Left8bHost& host, Object traffic);

// The landing-troop group tick.
void landing_troop_tick_004a2fc0(Left8bHost& host, Object troop, float dt);

// Unit-attached traffic.
void unit_traffic_kill_in_box_004a5c60(Left8bHost& host, Object manager, std::uint32_t key, const float* box);
void unit_traffic_create_004a6830(Left8bHost& host, Object manager, Object unit);

}  // namespace bsp::cc13_left8b
