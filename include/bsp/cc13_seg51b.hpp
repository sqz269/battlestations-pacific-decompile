#pragma once

// cc13_seg51b: the last three unreconstructed bodies of segment 51 (0081AA60-00828870), after
// cc13-gp80 named them (docs/CC13_GP_0080_0084.md, docs/CC13_SEG51B.md). New C++ interfaces over
// the image's Win32 layouts; nothing here is wired into the game. Names are hypotheses, not
// recovered symbols. Every call the image makes is a named host hook invoked at the same point
// with the same arguments; checked-iterator reports (00BF6713) are kept where the image makes them.

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp::cc13_seg51b {

using Object = std::uint8_t*;

template <typename T>
T& field(Object base, std::size_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

// The image's 8-byte native string as these bodies touch it: {size, data}.
struct NativeString {
    std::uint32_t size;
    char* data;
};

struct Seg51bHost {
    virtual ~Seg51bHost() = default;

    // 00BF6713, the checked-iterator invalid-parameter report.
    virtual void invalid_parameter_00bf6713() = 0;
    // [00CE221C] InterlockedIncrement / [00CE2220] InterlockedDecrement (IAT);
    // [object]->vtable[0](ECX = object), the refcounted release slot.
    virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* value) = 0;
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* value) = 0;
    virtual void refcounted_vslot_00(Object object) = 0;
    // 00BF55BE operator new(size) and 00BF6989 operator delete(block), cdecl.
    virtual Object operator_new_00bf55be(std::uint32_t size) = 0;
    virtual void operator_delete_00bf6989(Object block) = 0;

    // 00B6DA70 BSP_SceneNode_SetVisibilityFactor, __thiscall(ECX = node; float factor, int),
    // RET 8.
    virtual void scene_node_set_visibility_factor_00b6da70(Object node, float factor,
                                                           std::int32_t tail) = 0;
    // 00867B10 BSP_PointEffect_StopChildren, __fastcall(ECX = effect).
    virtual void point_effect_stop_children_00867b10(Object effect) = 0;
    // 004C9550 BSP_LiveEffectReferences_Resize, __thiscall(ECX = vector; count), RET 4.
    virtual void live_effect_references_resize_004c9550(Object vector, std::uint32_t count) = 0;
    // 0074D780 BSP_LiveEffectReferences_Append, __thiscall(ECX = vector; const ref*), RET 4.
    virtual void live_effect_references_append_0074d780(Object vector, Object* ref) = 0;
    // 008687C0 BSP_PointEffect_CreateWithParentMatrix, __fastcall(ECX = out ref, EDX = parent;
    // definition ref by value (the callee owns the reference), const float* matrix4x4,
    // transform word, option word, tail word) -> EAX = out.
    virtual Object* point_effect_create_with_parent_matrix_008687c0(
        Object* out, Object parent, Object definition, const float* matrix,
        std::uint32_t transform_word, std::uint32_t option_word, std::uint32_t tail_word) = 0;
    // 00815D20 BSP_SceneNode_ToggleKotelRopeNodes, __thiscall(ECX = unit; parent node,
    // byte visible), RET 8.
    virtual void scene_node_toggle_kotel_rope_nodes_00815d20(Object unit, Object parent,
                                                             std::uint8_t visible) = 0;

    // 0041DD40 BSP_NativeString_Resize, __thiscall(ECX = string; size, flag), RET 8;
    // 0041E870 BSP_NativeString_Assign, __thiscall(ECX = string; const char*), RET 4;
    // 00BF7680 memcpy (cdecl).
    virtual void native_string_resize_0041dd40(NativeString* string, std::uint32_t size,
                                               std::uint32_t flag) = 0;
    virtual void native_string_assign_0041e870(NativeString* string, const char* text) = 0;
    virtual void memcpy_00bf7680(void* dest, const void* source, std::uint32_t size) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton -> EAX, then 00BD1510
    // BSP_SizedStoragePool_ReturnBlock, __thiscall(ECX = pool; block, size, flag), RET 0Ch,
    // consuming the arguments pushed for 00419CC0 (the inlined native string release).
    virtual Object sized_storage_pool_get_singleton_00419cc0() = 0;
    virtual void sized_storage_pool_return_block_00bd1510(Object pool, Object block,
                                                          std::uint32_t size,
                                                          std::uint32_t flag) = 0;

    // [0109CEEC], the file system. vtable[8]: __thiscall(ECX = fs; const NativeString* path),
    // RET 4 -> AL (exists). vtable[4]: __thiscall(ECX = fs; const NativeString* path, mode),
    // RET 8 -> EAX = refcounted stream.
    virtual Object file_system_0109ceec() = 0;
    virtual std::uint8_t file_system_vslot_08_exists(Object fs, const NativeString* path) = 0;
    virtual Object file_system_vslot_04_open(Object fs, const NativeString* path,
                                             std::uint32_t mode) = 0;

    // Lua wrappers (__thiscall; ECX = the object named first).
    // 00B66BD0 BSP_LuaStateOwner_Construct(owner); 00B6A020 BSP_LuaStateOwner_Open(owner; flag),
    // RET 4; 00B669A0 BSP_LuaStateOwner_Close(owner); 00B69D40
    // BSP_LuaStateOwner_RunScriptWithOverrides(owner; const NativeString* path, overrides), RET 8;
    // 00B67980 BSP_LuaStateOwner_GetGlobals(owner; out), RET 4 -> EAX = out.
    virtual void lua_state_owner_construct_00b66bd0(Object owner) = 0;
    virtual void lua_state_owner_open_00b6a020(Object owner, std::uint32_t flag) = 0;
    virtual void lua_state_owner_close_00b669a0(Object owner) = 0;
    virtual void lua_state_owner_run_script_00b69d40(Object owner, const NativeString* path,
                                                     Object overrides) = 0;
    virtual Object lua_state_owner_get_globals_00b67980(Object owner, Object out) = 0;
    // 00B65F50 BSP_LuaObject_Construct(object); 00B67700 BSP_LuaObject_Destruct(object);
    // 00B67690 BSP_LuaObject_Assign(object; source), RET 4; 00B67800
    // BSP_LuaObject_GetByName(table; out, const char* name), RET 8 -> EAX = out; 00B67580
    // BSP_LuaObject_SetNewTable(table; const NativeString* name), RET 4; 00B66670
    // BSP_LuaObject_SetNewTableAtIndex(table; index), RET 4; 00B67720
    // BSP_LuaObject_GetByIndex(table; out, index), RET 8 -> EAX = out; 00B66580
    // BSP_LuaObject_SetNumericElement(table; index, float value), RET 8; 00B6A390
    // BSP_LuaObject_WriteLuaSourceText(object; stream, const NativeString* name, a, b, c, d),
    // RET 18h.
    virtual void lua_object_construct_00b65f50(Object object) = 0;
    virtual void lua_object_destruct_00b67700(Object object) = 0;
    virtual void lua_object_assign_00b67690(Object object, Object source) = 0;
    virtual Object lua_object_get_by_name_00b67800(Object table, Object out,
                                                   const char* name) = 0;
    virtual void lua_object_set_new_table_00b67580(Object table, const NativeString* name) = 0;
    virtual void lua_object_set_new_table_at_index_00b66670(Object table,
                                                            std::int32_t index) = 0;
    virtual Object lua_object_get_by_index_00b67720(Object table, Object out,
                                                    std::int32_t index) = 0;
    virtual void lua_object_set_numeric_element_00b66580(Object table, std::int32_t index,
                                                         float value) = 0;
    virtual void lua_object_write_lua_source_text_00b6a390(Object object, Object stream,
                                                           const NativeString* name,
                                                           std::uint32_t a, std::uint32_t b,
                                                           std::uint32_t c,
                                                           std::uint32_t d) = 0;
};

// 0081BA80 BSP_GunCamos_WriteEntry, __fastcall(ECX = index, EDX = const float colour[4]), plain
// RET. Unreferenced (no direct call, no absolute dword): a tool-side save of guncamos.lua.
void gun_camos_write_entry_0081ba80(Seg51bHost& host, std::int32_t index, const float* colour);

// 0081C1F0, the refcounted-pointer array push-back, __thiscall(ECX = array {data, count, cap};
// const Object* ref), RET 4. Elements are 4 bytes (EDI*4 throughout); full arrays grow to
// 2*cap+2.
void refptr_array_push_back_0081c1f0(Seg51bHost& host, Object array, const Object* ref);

// 0081C550 BSP_UnitInstance_ToggleDeckVisuals, __fastcall(ECX = unit), plain RET. Unreferenced:
// no direct call, no absolute dword and no vtable slot. Uncertain purpose: on airfields +749h is
// HangarFailure (cc13_seg30.hpp), so this may be the hangar-failure visual rather than "deck
// visuals".
void unit_instance_toggle_deck_visuals_0081c550(Seg51bHost& host, Object unit);

} // namespace bsp::cc13_seg51b
