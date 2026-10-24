#pragma once
// Packet cc13-cgbuildA: seven bodies the producer tagged as compiler helpers
// (CG_array_ctor_helper / CG_vector_deleting_dtor) that cc13-cgretag (a46023c5f)
// showed to be real class constructors and destructors:
//   004A0410 / 0048DFD0  landing-troop group (vtable 00CE66E0) ctor / dtor body
//   004A41C0 / 0048F890  buoy layer scene node (vtable 00CE67AC) ctor / dtor body
//   00963600             ship class destructor body (vtables 00D1ACC4 + 00D1ACC0)
//   004042C0             SAP broad-phase manager destructor body (base stamp 00D7A0E4)
//   009E0130             ship-AI nav block destructor (vtable 00D21854; ctor 009E4330)
//
// Objects are reached through byte offsets in their Win32 layout (Object plus
// at<T>). Every call the image makes goes through a CgRebuildAHost method named
// for its target address, at the same point, in the same order and with the
// same arguments; library code (CRT free/new/memcpy/_stricmp, the EH vector
// constructor/destructor iterators, STL range destroyers and tidies, the import
// InterlockedDecrement) is bound there and not reconstructed. Inlined STL
// vector tidies are written out because they are part of these bodies.
//
// EH: every body runs under an FS:[0] frame whose state byte the image updates
// between members (the values are listed per function in the .cpp). The C++
// here does not model unwinding: if a host call throws, the partially built or
// destroyed object is not cleaned up the way the image's funclets would.
//
// Names are hypotheses, not recovered symbols. These are new C++ interfaces over
// an injected host: build-tested only, NOT wired into any game host, and not
// ABI-compatible replacements (the image passes this in ECX, these take it as a
// parameter).

#include <cstddef>
#include <cstdint>

namespace bsp::cg_rebuild_a_cc13 {

static_assert(sizeof(void*) == 4, "these objects are modelled in their Win32 layout");

using Object = std::uint8_t*;

template <class T>
inline T& at(Object object, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(object + offset);
}
inline Object obj(std::uint32_t value) noexcept {
    return reinterpret_cast<Object>(static_cast<std::uintptr_t>(value));
}

// A 14h-byte Lua object handle (opaque; 00B65F50 constructs, 00B67700 destructs).
struct LuaObject {
    std::uint8_t storage[0x14];
};

// The 8-byte native string {size, data} (0041DD40 resizes it).
struct NativeString {
    std::uint32_t size;
    char* data;
};

struct CgRebuildAHost {
    virtual ~CgRebuildAHost() = default;

    // Image globals, read at the point the image reads them.
    virtual std::uint32_t global_u32(std::uint32_t address) = 0;

    // --- CRT / compiler runtime (bound library code) ---
    virtual void crt_free_00bf65ac(void* block) = 0;   // JMP 00BF9DC8
    virtual void crt_free_00bf6989(void* block) = 0;   // JMP 00BF65AC
    virtual void crt_free_00bf9dc8(void* block) = 0;   // _free body
    virtual Object operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void crt_memcpy_00bf7680(void* destination, const void* source, std::uint32_t bytes) = 0;
    virtual int crt_stricmp_00bf7fbf(const char* left, const char* right) = 0;
    // `eh vector constructor iterator'(first, size, count, ctor, dtor), __stdcall.
    virtual void eh_vector_construct_00bf7cd1(Object first, std::uint32_t size, std::uint32_t count,
                                              std::uint32_t ctor, std::uint32_t dtor) = 0;
    // `eh vector destructor iterator'(first, size, count, dtor), __stdcall.
    virtual void eh_vector_destruct_00bf7c6e(Object first, std::uint32_t size, std::uint32_t count,
                                             std::uint32_t dtor) = 0;
    // InterlockedDecrement through the import slot 00CE2220; returns the new value.
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* addend) = 0;
    // [[object]+0](), __thiscall with no stack argument (the reference-counted
    // object's release slot, called when the count reaches 0).
    virtual void vslot0_release(Object object) = 0;
    // [[object]+0](flags), __thiscall, RET 4 (scalar deleting destructor).
    virtual void vslot0_delete(Object object, std::uint32_t flags) = 0;

    // --- native strings ---
    virtual Object string_pool_00419cc0() = 0;
    // 00BD1510 __thiscall(pool, block, bytes, count), RET 0Ch.
    virtual void string_pool_return_00bd1510(Object pool, void* block, std::uint32_t bytes, std::int32_t count) = 0;
    // 0041DD40 __thiscall(string, length, 1), RET 8.
    virtual void native_string_resize_0041dd40(NativeString* string, std::uint32_t length, std::int32_t flag) = 0;

    // --- Lua objects ---
    virtual LuaObject* lua_get_globals_00b67980(Object owner, LuaObject* out) = 0;   // ECX = owner, RET 4
    // 00B67800 __thiscall(source, out, name), RET 8, EAX = out.
    virtual LuaObject* lua_get_by_name_00b67800(LuaObject* source, LuaObject* out, const char* name) = 0;
    // 00B67720 __thiscall(table, out, index), RET 8, EAX = out.
    virtual LuaObject* lua_get_by_index_00b67720(LuaObject* table, LuaObject* out, std::int32_t index) = 0;
    virtual void lua_object_construct_00b65f50(LuaObject* object) = 0;      // ECX = object
    virtual void lua_object_destruct_00b67700(LuaObject* object) = 0;       // ECX = object
    virtual double lua_get_number_00b66270(LuaObject* object) = 0;          // ECX = object, ST0
    virtual std::int32_t lua_get_integer_00b66290(LuaObject* object) = 0;   // ECX = object
    virtual const char* lua_get_string_00b662b0(LuaObject* object) = 0;     // ECX = object
    // 00B67080 / 00B67190 __thiscall(table, key, value), RET 8.
    virtual void lua_iterate_first_00b67080(LuaObject* table, LuaObject* key, LuaObject* value) = 0;
    virtual void lua_iterate_next_00b67190(LuaObject* table, LuaObject* key, LuaObject* value) = 0;
    // 00B66420 (ECX = table, ignored; stack object), RET 4: AL = object unbound.
    virtual bool lua_is_unbound_00b66420(LuaObject* table, LuaObject* object) = 0;

    // --- scene nodes, poses, paths (segment 8 and shared) ---
    // 00496920 __thiscall(node, parent), RET 4: scene node base ctor (vtable 00CE6600).
    virtual void scene_node_construct_00496920(Object node, Object parent) = 0;
    virtual void list_node_destruct_00489dd0(Object node) = 0;              // ECX = node (base dtor)
    virtual void entity_pose_refresh_world_00414db0(Object pose) = 0;      // ECX = pose
    // 00413920 __thiscall(left, out, right), RET 8, EAX = out (4x4 multiply).
    virtual const float* matrix_multiply_00413920(const float* left, float* out, const float* right) = 0;
    virtual void matrix_copy_004134f0(float* destination, const float* source) = 0;   // ECX = dest, RET 4
    virtual Object path_object_get_entity_007aca30(Object path) = 0;        // ECX = path
    // 007AF800 __thiscall(path, out, index), RET 8, EAX = the world point.
    virtual const float* scene_path_point_to_world_007af800(Object path, float* out, std::uint32_t index) = 0;
    virtual void aabb_include_point_00427d10(float* aabb, const float* point) = 0;   // ECX = aabb, RET 4

    // --- landing-troop group (004A0410 / 0048DFD0) ---
    virtual void pod_pointer_array_reserve_00482570(Object array, std::int32_t capacity) = 0;   // RET 4
    virtual void pod_pointer_array_resize_004859e0(Object array, std::int32_t size) = 0;        // RET 4
    virtual Object soldier_class_get_or_load_004b1400(NativeString* name) = 0;   // ECX = name, RET
    // 0049FC80 __thiscall(memory, soldier class, row), RET 8, EAX = memory.
    virtual Object soldier_type_construct_0049fc80(Object memory, Object soldier_class, LuaObject* row) = 0;
    // 00964790 __fastcall(ECX = class id, DL = create flag), EAX = vehicle class.
    virtual Object vehicle_class_get_or_create_00964790(std::int32_t class_id, std::uint8_t create) = 0;
    virtual Object vehicle_visual_class_get_or_create_0047b3c0(Object vehicle_class) = 0;   // ECX
    // 0048DC40 __thiscall(memory, visual class, row), RET 8 (stamps vtable 00CE66DC).
    virtual void soldier_type_construct_from_lua_0048dc40(Object memory, Object visual_class, LuaObject* row) = 0;
    virtual void landing_troop_kill_all_members_0048c090(Object group) = 0;   // ECX = group

    // --- buoy layer node (004A41C0 / 0048F890) ---
    virtual std::uint32_t buoy_class_get_or_create_00475710(NativeString* name) = 0;   // ECX = name, RET
    virtual void traffic_layers_rebuild_0049e560(Object node) = 0;                     // ECX = node
    virtual void scene_entity_layers_reset_all_004814c0(Object node) = 0;              // ECX = node

    // --- ship class (00963600) ---
    // Range destroyers (ECX = first, EDX = last; stack: vector, owner), callee pops 8.
    virtual void destroy_range_004cc760(Object first, Object last, Object vector, Object owner) = 0;
    virtual void destroy_range_0082b5a0(Object first, Object last, Object vector, Object owner) = 0;
    virtual void destroy_range_0082c830(Object first, Object last, Object vector, Object owner) = 0;
    virtual void stl_tidy_00956150(Object container) = 0;   // ECX = container
    virtual void stl_tidy_009632c0(Object container) = 0;   // ECX = container
    virtual void stl_tidy_00963320(Object container) = 0;   // ECX = container
    virtual void pod_pointer_array_resize_00468050(Object array, std::int32_t size) = 0;      // RET 4
    virtual void points_array_reserve_0074d190(Object array, std::int32_t capacity) = 0;      // RET 4
    virtual void indexed_points_reserve_00829180(Object array, std::int32_t capacity) = 0;    // RET 4
    virtual void vehicle_class_destruct_base_0095f1c0(Object vehicle_class) = 0;              // ECX

    // --- ship-AI nav block (009E0130) ---
    virtual void observed_owner_destroy_0064a610(Object owner) = 0;                 // ECX
    virtual void path_node_observer_destruct_009d5eb0(Object observer) = 0;        // ECX
    // 006952A0 __fastcall(ECX = observed, EDX = owner).
    virtual void observer_unregister_pair_006952a0(Object observed, Object owner) = 0;
    virtual void callback_owner_destroy_00695870(Object owner) = 0;                 // ECX
    virtual void avoid_zone_segment_list_clear_004158a0(Object list) = 0;           // ECX
};

// 004A0410: landing-troop group constructor. Returns group.
Object landing_troop_group_construct_004a0410(CgRebuildAHost& host, Object group, Object parent, Object pad,
                                              Object base, Object ship);
// 0048DFD0: landing-troop group destructor body.
void landing_troop_group_destruct_0048dfd0(CgRebuildAHost& host, Object group);
// 004A41C0: buoy layer node constructor. Returns node.
Object buoy_layer_node_construct_004a41c0(CgRebuildAHost& host, Object node, Object parent);
// 0048F890: buoy layer node destructor body.
void buoy_layer_node_destruct_0048f890(CgRebuildAHost& host, Object node);
// 00963600: ship class destructor body (chains to 0095F1C0).
void ship_class_destruct_00963600(CgRebuildAHost& host, Object ship_class);
// 004042C0: SAP broad-phase manager destructor body.
void sap_broad_phase_manager_destruct_004042c0(CgRebuildAHost& host, Object manager);
// 009E0130: ship-AI nav block destructor (brain+8h).
void ship_ai_nav_block_destruct_009e0130(CgRebuildAHost& host, Object block);

}  // namespace bsp::cg_rebuild_a_cc13
