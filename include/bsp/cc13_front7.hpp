#pragma once
// Packet cc13-front7: wiring-frontier callees, round 2 (the large bodies skipped by
// cc13-front6 and the next ranked blockers of local/output/wire_readiness.json).
// docs/CC13_FRONT7.md lists every target with its ABI, evidence, the rows it
// unblocks and its uncertainty.
//
// Objects are reached through cc13_sweepA::Raw (byte offsets into the Win32
// layout). Every call the image makes goes through a host method named for its
// target; image globals are reached through global_address / global_pointer.
// Descriptive names are hypotheses, not recovered symbols. Nothing here is wired.

#include "bsp/cc13_sweepA.hpp"

#include <cstdint>

namespace bsp::cc13_front7 {

using cc13_sweepA::Raw;

// ---------------------------------------------------------------------------
// 0078CFF0: the environment's per-frame apply to a camera (ocean effects, clear
// colour, the camera's fog owner, the ocean wave field).
// ---------------------------------------------------------------------------
struct EnvironmentApplyHost {
    virtual ~EnvironmentApplyHost() = default;
    // The pointer stored in the image global dword at `address` ([00E188A8] the game).
    virtual void* global_pointer(std::uint32_t address) = 0;

    // 00BA40F0 BSP_OceanWaterParticles_Update, __thiscall(ECX = particles; camera, float clock) RET 8.
    virtual void ocean_water_particles_update_00ba40f0(void* particles, void* camera, float clock) = 0;
    // 00BA2FF0 BSP_OceanBeams_Update, __thiscall(ECX = beams; camera, float time) RET 8.
    virtual void ocean_beams_update_00ba2ff0(void* beams, void* camera, float time) = 0;
    // 004FB850 BSP_Color_ConvertFloatRgbaToArgb, __thiscall(ECX = out dword; const float* rgba) RET 4.
    virtual void color_float_rgba_to_argb_004fb850(std::uint32_t* out, const float* rgba) = 0;
    // 00B6FE50 BSP_Camera_SetClearColorWord, __thiscall(ECX = camera; dword argb) RET 4.
    virtual void camera_set_clear_color_00b6fe50(void* camera, std::uint32_t argb) = 0;
    // 00BA0090 BSP_OceanWaveField_Update, __thiscall(ECX = wave field; camera, float t) RET 8.
    virtual void ocean_wave_field_update_00ba0090(void* wave_field, void* camera, float t) = 0;

    // The camera's fog owner [camera+184h] setters, each __thiscall(ECX = owner; ...).
    // 00B84C40 BSP_SystemFogOwner_SetColor08 (const float* rgba) RET 4.
    virtual void fog_set_color_08_00b84c40(void* fog, const float* rgba) = 0;
    // 00B84FA0 BSP_SystemFogOwner_SetDirectionalColor28 (const float* rgba, int index) RET 8.
    virtual void fog_set_directional_color_28_00b84fa0(void* fog, const float* rgba, std::int32_t index) = 0;
    // 00B84C70 BSP_SystemFogOwner_SetUnderwaterColor18 (const float* rgba) RET 4.
    virtual void fog_set_underwater_color_18_00b84c70(void* fog, const float* rgba) = 0;
    // Scalar setters (float) RET 4.
    virtual void fog_set_scalar_68_00b84d00(void* fog, float value) = 0;
    virtual void fog_set_scalar_6c_00b84d10(void* fog, float value) = 0;
    virtual void fog_set_scalar_70_00b84d20(void* fog, float value) = 0;
    virtual void fog_set_scalar_74_00b84d30(void* fog, float value) = 0;
    virtual void fog_set_scalar_78_00b84d40(void* fog, float value) = 0;
    virtual void fog_set_scalar_7c_00b84d50(void* fog, float value) = 0;
    virtual void fog_set_scalar_80_00b84d60(void* fog, float value) = 0;
    virtual void fog_set_scalar_84_00b84d80(void* fog, float value) = 0;
    virtual void fog_set_scalar_88_00b84dc0(void* fog, float value) = 0;
    virtual void fog_set_scalar_8c_00b84de0(void* fog, float value) = 0;
    virtual void fog_set_scalar_90_00b84e00(void* fog, float value) = 0;
};

// 0078CFF0, __thiscall(ECX = environment; camera, float clock) RET 8.
void environment_apply_to_camera_0078cff0(Raw environment, void* camera, float clock,
                                          EnvironmentApplyHost& host);

// The VC9 checked iterator the image passes around: {_Mycont, _Ptr}.
struct MapIterator {
    void* container;
    std::uint8_t* node;
};
static_assert(sizeof(MapIterator) == 8);

// A VC9 std::vector<dword> image: +0 (not written by 0061B0F0), first, last, end.
struct DwordVectorImage {
    std::uint32_t head;
    std::uint32_t* first;
    std::uint32_t* last;
    std::uint32_t* end;
};
static_assert(sizeof(DwordVectorImage) == 16);

// ---------------------------------------------------------------------------
// 0061B0F0: the powerup-manager HUD screen's list rebuild.
// ---------------------------------------------------------------------------
struct PowerupListsHost {
    virtual ~PowerupListsHost() = default;
    // The pointer stored in the image global dword at `address` ([00E188A8] game,
    // [00E198C4] the in-game interface manager, [00F88C30] the powerup manager).
    virtual void* global_pointer(std::uint32_t address) = 0;
    // 00BF6713, the checked-iterator failure (_invalid_parameter_noinfo); returns here.
    virtual void invalid_parameter_00bf6713() = 0;
    // 00BF7FBF _stricmp, __cdecl.
    virtual std::int32_t stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // 00BF65AC operator delete, __cdecl.
    virtual void operator_delete_00bf65ac(void* block) = 0;
    // 008E9AF0 BSP_PowerUpManager_CollectActive, __thiscall(ECX = manager; seven dwords) as pushed.
    virtual void powerup_manager_collect_active_008e9af0(void* manager, void* list_184, void* list_144,
                                                         void* list_154, void* list_164, void* list_174,
                                                         std::uint32_t zero_a, std::uint32_t zero_b) = 0;
    // 00441F30 std::vector<dword>::insert(where, value), __thiscall(ECX = vector; out
    // iterator*, iterator where {container, ptr} by value, const dword*) RET 10h.
    virtual MapIterator* vector_insert_one_00441f30(DwordVectorImage* vector, MapIterator* out,
                                                    MapIterator where, const std::uint32_t* value) = 0;
    // 00614060 STL_IntMap_Find, __thiscall(ECX = map; out*, const int* key) RET 8 -> EAX = out.
    virtual MapIterator* int_map_find_00614060(void* map, MapIterator* out, const void* key) = 0;
    // 006171B0 int-keyed map operator[], __thiscall(ECX = map; const int* key) RET 4 -> value*.
    virtual std::uint32_t* int_map_subscript_006171b0(void* map, const void* key) = 0;
    // 00613900 STL_StringMapCI_LowerBound, __thiscall(ECX = map; const NativeString* key) RET 4 -> node.
    virtual std::uint8_t* string_map_ci_lower_bound_00613900(void* map, const void* key) = 0;
    // 00617870 STL_StringMapCI_Subscript, __thiscall(ECX = map; const NativeString* key) RET 4 -> value*.
    virtual std::uint32_t* string_map_ci_subscript_00617870(void* map, const void* key) = 0;
    // 006170F0 map operator[], __thiscall(ECX = map; const dword* key) RET 4 -> value*.
    virtual std::uint32_t* map_subscript_006170f0(void* map, const std::uint32_t* key) = 0;
    // 00AB64C0 BSP_GuiIcon_AddState, __thiscall(ECX = icon list; dword, const void*) RET 8 -> EAX.
    virtual std::uint32_t gui_icon_add_state_00ab64c0(void* icons, std::uint32_t id, const void* data) = 0;
    // 004F6210 std::vector<dword> copy constructor, __thiscall(ECX = dest; const vector*) RET 4.
    virtual void dword_vector_copy_construct_004f6210(DwordVectorImage* dest, const DwordVectorImage* source) = 0;
    // 0061A400 BSP_HudPowerupManagerScreen_AssignLists, __thiscall(ECX = screen+100h;
    // vector a, vector b by value) RET 20h; the callee owns and destroys both copies.
    virtual void powerup_screen_assign_lists_0061a400(void* lists, DwordVectorImage* by_value_pair) = 0;
};

// 0061B0F0, __fastcall(ECX = screen) RET.
void powerup_screen_rebuild_lists_0061b0f0(Raw screen, PowerupListsHost& host);

// ---------------------------------------------------------------------------
// 0087CA80: the damageable-class descriptor's Lua field reader.
// ---------------------------------------------------------------------------
using cc13_sweepA::LuaObjectStorage;

// The image's {length, data} string (00419CC0/00BD1510 release it to the pool).
struct NativeStringImage {
    std::uint32_t length;
    char* data;
};
static_assert(sizeof(NativeStringImage) == 8);

struct DamageableClassReadHost {
    virtual ~DamageableClassReadHost() = default;
    // The address of the image global at `address` (00F878E0, the empty C string).
    virtual void* global_address(std::uint32_t address) = 0;

    // --- CRT (bound) ---
    // 00BF6713, the checked-iterator failure (_invalid_parameter_noinfo); returns here.
    virtual void invalid_parameter_00bf6713() = 0;
    // 00BF7680 memcpy, 00BF79F0 memset, 00BF7A6A sprintf, __cdecl.
    virtual void memcpy_00bf7680(void* dest, const void* source, std::uint32_t bytes) = 0;
    virtual void memset_00bf79f0(void* dest, std::int32_t value, std::uint32_t bytes) = 0;
    virtual void sprintf_int_00bf7a6a(char* buffer, const char* format, std::int32_t value) = 0;
    // 00BF7420, the CRT ST0 -> EAX truncation (_ftol2 family); ST0 passed as a double.
    virtual std::int32_t ftol_00bf7420(double value) = 0;
    // [00CE221C] InterlockedIncrement / [00CE2220] InterlockedDecrement imports.
    virtual std::int32_t interlocked_increment_ce221c(volatile std::int32_t* count) = 0;
    virtual std::int32_t interlocked_decrement_ce2220(volatile std::int32_t* count) = 0;
    // The refcounted effect handle's vtable slot 0, __thiscall(ECX = handle), called
    // when InterlockedDecrement([handle+4]) reaches zero.
    virtual void effect_handle_vtable0(void* handle) = 0;

    // --- strings ---
    // 00438E40 BSP_NativeString_Duplicate, __fastcall(ECX = C string) -> EAX.
    virtual char* native_string_duplicate_00438e40(const char* text) = 0;
    // 0041DD40, __thiscall(ECX = string; length, char preserve) RET 8.
    virtual void native_string_resize_0041dd40(NativeStringImage* string, std::uint32_t length,
                                               char preserve) = 0;
    // 0041E870 BSP_NativeString_Assign, __thiscall(ECX = string; const char*) RET 4 (constructs).
    virtual void native_string_assign_0041e870(NativeStringImage* string, const char* text) = 0;
    // 00419CC0, the sized-storage pool singleton (EAX); the call site pushes
    // (block, length + 1, 1) before it and leaves them for 00BD1510.
    virtual void* sized_pool_00419cc0() = 0;
    // 00BD1510, __thiscall(ECX = pool; block, bytes, count) RET 0Ch.
    virtual void sized_pool_return_00bd1510(void* pool, void* block, std::uint32_t bytes,
                                            std::uint32_t count) = 0;

    // --- Lua objects ---
    // 00B67800, __thiscall(ECX = table; LuaObject* out, const char* name) RET 8 -> EAX = out.
    virtual LuaObjectStorage* lua_object_get_by_name_00b67800(LuaObjectStorage* table,
                                                              LuaObjectStorage* out,
                                                              const char* name) = 0;
    // 00B67720, __thiscall(ECX = table; LuaObject* out, int index) RET 8 -> EAX = out.
    virtual LuaObjectStorage* lua_object_get_by_index_00b67720(LuaObjectStorage* table,
                                                               LuaObjectStorage* out,
                                                               std::int32_t index) = 0;
    // 00B67700 destructor, 00B65F50 nil constructor, __thiscall(ECX = object).
    virtual void lua_object_destruct_00b67700(LuaObjectStorage* object) = 0;
    virtual void lua_object_construct_00b65f50(LuaObjectStorage* object) = 0;
    // 00B67690, __thiscall(ECX = destination; const LuaObject* source) RET 4.
    virtual void lua_object_assign_00b67690(LuaObjectStorage* destination,
                                            const LuaObjectStorage* source) = 0;
    // 00B66420, __thiscall(ECX = table; LuaObject* key) RET 4, AL = iteration ended.
    virtual bool lua_object_iteration_ended_00b66420(LuaObjectStorage* table, LuaObjectStorage* key) = 0;
    // 00B67080 / 00B67190, __thiscall(ECX = table; key*, value*) RET 8.
    virtual void lua_object_iterate_first_00b67080(LuaObjectStorage* table, LuaObjectStorage* key,
                                                   LuaObjectStorage* value) = 0;
    virtual void lua_object_iterate_next_00b67190(LuaObjectStorage* table, LuaObjectStorage* key,
                                                  LuaObjectStorage* value) = 0;
    // Predicates and getters, __thiscall(ECX = object) RET: 00B661B0 IsTable,
    // 00B66000 IsBoolean, 00B66250 GetBoolean, 00B66A60 IsInteger (AL);
    // 00B662B0 GetString (EAX); 00B66290 GetInteger (EAX); 00B66270 GetNumber (ST0).
    virtual bool lua_object_is_table_00b661b0(LuaObjectStorage* object) = 0;
    virtual bool lua_object_is_boolean_00b66000(LuaObjectStorage* object) = 0;
    virtual bool lua_object_get_boolean_00b66250(LuaObjectStorage* object) = 0;
    virtual bool lua_object_is_integer_00b66a60(LuaObjectStorage* object) = 0;
    virtual const char* lua_object_get_string_00b662b0(LuaObjectStorage* object) = 0;
    virtual std::int32_t lua_object_get_integer_00b66290(LuaObjectStorage* object) = 0;
    virtual double lua_object_get_number_00b66270(LuaObjectStorage* object) = 0;
    // 00B66330, __thiscall(ECX = object; float default) RET 4 -> ST0 (a float32 spill/reload).
    virtual float lua_object_get_float_or_default_00b66330(LuaObjectStorage* object, float fallback) = 0;
    // 00B66380, __thiscall(ECX = object; int default) RET 4 -> EAX.
    virtual std::int32_t lua_object_get_integer_or_default_00b66380(LuaObjectStorage* object,
                                                                    std::int32_t fallback) = 0;
    // 00B685C0, __thiscall(ECX = object; NativeString* out, const char* fallback) RET 8 -> EAX = out.
    virtual NativeStringImage* lua_object_string_or_default_00b685c0(LuaObjectStorage* object,
                                                                     NativeStringImage* out,
                                                                     const char* fallback) = 0;

    // --- game ---
    // 007149D0 BSP_MeshCategory_FromName, __fastcall(ECX = C string) -> EAX.
    virtual std::int32_t mesh_category_from_name_007149d0(const char* name) = 0;
    // 00870CD0 BSP_EffectHandle_Acquire, __fastcall(ECX = out handle, EDX = id; dword) RET 4 -> EAX = out.
    virtual void** effect_handle_acquire_00870cd0(void** out, std::int32_t id, std::uint32_t arg) = 0;
    // 00871BA0 BSP_EffectHandle_AcquireByName, __fastcall(ECX = out, EDX = NativeString*; dword) RET 4 -> EAX = out.
    virtual void** effect_handle_acquire_by_name_00871ba0(void** out, const NativeStringImage* name,
                                                          std::uint32_t arg) = 0;
    // 0087C870 BSP_DamageableSectionVector_Append, __thiscall(ECX = vector; const 30h record*) RET 4.
    virtual void damageable_section_vector_append_0087c870(void* vector, const void* record) = 0;
    // 0087C920, __thiscall(ECX = vector; count, 16-byte value by value) RET 14h. The image
    // writes the value's dwords +0, +4 and +0Ch only; +8 is whatever the stack slot held.
    virtual void damageable_fake_effect_vector_resize_0087c920(void* vector, std::uint32_t count,
                                                                float value_0, std::int32_t value_4,
                                                                std::int32_t value_c) = 0;
    // 004B1400 SoldierClass_GetOrLoad, __thiscall(ECX = NativeString* name) -> EAX.
    virtual void* soldier_class_get_or_load_004b1400(NativeStringImage* name) = 0;
    // 0087B750 STL_DamageableCrewTree_Subscript, __thiscall(ECX = tree; const int* key) RET 4 -> EAX.
    virtual std::uint8_t* damageable_crew_tree_subscript_0087b750(void* tree, const std::int32_t* key) = 0;
};

// 0087CA80, __thiscall(ECX = damageable-class descriptor; LuaObject* row) RET 4.
void damageable_class_read_lua_fields_0087ca80(Raw descriptor, LuaObjectStorage* row,
                                               DamageableClassReadHost& host);

}  // namespace bsp::cc13_front7
