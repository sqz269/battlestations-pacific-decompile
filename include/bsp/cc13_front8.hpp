#pragma once
// Packet cc13-front8 (marker cc13_front8): wiring-frontier callees, round 3.
// docs/CC13_FRONT8.md lists every target with its ABI, evidence, the rows it
// unblocks and its uncertainty.
//
// Objects are reached through cc13_sweepA::Raw (byte offsets into the Win32
// layout). Every call the image makes goes through a host method named for its
// target; image globals are reached through global_address / global_pointer.
// Descriptive names are hypotheses, not recovered symbols. Nothing here is wired.

#include "bsp/cc13_front7.hpp"
#include "bsp/cc13_sweepA.hpp"

#include <cstdint>

namespace bsp::cc13_front8 {

using cc13_sweepA::Raw;

// ---------------------------------------------------------------------------
// 0077B280: the replication sync scheduler (distance/interval bands -> a queue slot).
// ---------------------------------------------------------------------------
struct ReplicationSyncHost {
    virtual ~ReplicationSyncHost() = default;
    // The pointer stored in the image global dword at `address` ([00E188A8] the game).
    virtual void* global_pointer(std::uint32_t address) = 0;
    // The address of the image global at `address`: 00F87150 (byte, the local party),
    // 00F876B0 (dword tick), 00F1B050 (dword[] per-level counts), 00F57150 / 00F27150
    // (dword[level][300h] entity / record tables), 00F1B150 (byte[level][300h] bands).
    virtual void* global_address(std::uint32_t address) = 0;

    // Entity vtable slot +5Ch, __thiscall(ECX = entity; int) RET 4 -> AL.
    virtual bool entity_vslot_5c(void* entity, std::int32_t arg) = 0;
    // Entity vtable slot +15Ch, __thiscall(ECX = entity) -> AL (party).
    virtual std::uint8_t entity_vslot_15c(void* entity) = 0;
    // Vtable slot +14h of [game+2078h] (or [game+207Ch] when null),
    // __thiscall(ECX = clock; dword) RET 4 -> ST0.
    virtual double clock_vslot_14(void* clock, std::uint32_t arg) = 0;

    // 00427EB0 BSP_EntityPose_GetWorldPositionRefreshed, __fastcall(ECX = entity) -> EAX float[3].
    virtual const float* entity_world_position_00427eb0(void* entity) = 0;
    // 0042B2F0 BSP_Vector3_LengthFloatThreshold, __fastcall(ECX = float[3]) -> ST0 (a float).
    virtual float vector3_length_0042b2f0(const float* v) = 0;
    // 007F7CD0 BSP_PlayerRecord_HasRecentCameraState, __fastcall(ECX = record object) -> AL.
    virtual bool player_record_has_recent_camera_state_007f7cd0(void* object) = 0;
    // 0042AE80 BSP_Vector3f_DivideByScalarInPlace, __thiscall(ECX = float[3]; float) RET 4.
    virtual void vector3_divide_in_place_0042ae80(float* v, float s) = 0;
    // 007789D0 (STL_inst_007789d0, a clamped acos), __stdcall(float) RET 4 -> ST0.
    virtual double stl_inst_007789d0(float x) = 0;
    // 00415550 BSP_Math_MaxFloatByRef, __fastcall(ECX = const float*, EDX = const float*) -> ST0.
    virtual float max_float_by_ref_00415550(const float* a, const float* b) = 0;
    // 00419010 BSP_Math_InterpolateClamped, __stdcall(x0, y0, x1, y1, x) RET 14h -> ST0 (a float).
    virtual float interpolate_clamped_00419010(float x0, float y0, float x1, float y1, float x) = 0;
    // 00BF7420, the CRT ST0 -> EAX truncation (_ftol2 family).
    virtual std::int32_t ftol_00bf7420(double value) = 0;
};

// 0077B280, __cdecl(entity, record**, byte flag, then six {float distance, int
// interval} pairs) -> AL; 15 stack dwords. `pairs` points at the first pair's
// distance (stack argument 4); the image reads it without a bound, relying on the
// caller's last distance exceeding the measured one.
bool replication_schedule_sync_0077b280(Raw entity, void** record, std::uint8_t flag,
                                        const std::uint32_t* pairs, ReplicationSyncHost& host);

// ---------------------------------------------------------------------------
// 00B68D70: LuaObject dotted-path resolution ("a.b.3").
// ---------------------------------------------------------------------------
using cc13_front7::LuaObjectStorage;
using cc13_front7::NativeStringImage;

struct LuaDotPathHost {
    virtual ~LuaDotPathHost() = default;
    // 0108FF2C, the empty C string, by address.
    virtual void* global_address(std::uint32_t address) = 0;
    // CRT (bound): 00BF7680 memcpy, 00BF9A50 strcspn, 00BF8417 atol, 00BF7FBF _stricmp.
    virtual void memcpy_00bf7680(void* dest, const void* source, std::uint32_t bytes) = 0;
    virtual std::uint32_t strcspn_00bf9a50(const char* text, const char* set) = 0;
    virtual std::int32_t atol_00bf8417(const char* text) = 0;
    virtual std::int32_t stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // 00A675A0 lua_type, ECX = lua_State*, EDX = index -> EAX (as called here).
    virtual std::int32_t lua_type_00a675a0(void* state, std::int32_t index) = 0;

    // Strings: 0041DD40 resize, 00469840 substring (-> EAX = out), 00419CC0 pool / 00BD1510 return.
    virtual void native_string_resize_0041dd40(NativeStringImage* string, std::uint32_t length,
                                               char preserve) = 0;
    virtual NativeStringImage* native_string_substring_00469840(const NativeStringImage* string,
                                                                NativeStringImage* out, std::uint32_t start,
                                                                std::uint32_t count) = 0;
    virtual void* sized_pool_00419cc0() = 0;
    virtual void sized_pool_return_00bd1510(void* pool, void* block, std::uint32_t bytes,
                                            std::uint32_t count) = 0;

    // Lua objects.
    // 00B66FA0 BSP_LuaObject_CopyConstruct, __thiscall(ECX = destination; const LuaObject*) RET 4.
    virtual void lua_object_copy_construct_00b66fa0(LuaObjectStorage* destination,
                                                    const LuaObjectStorage* source) = 0;
    // 00B68100 BSP_LuaObject_GetByNativeString, __thiscall(ECX = table; out, const NativeString*) RET 8.
    virtual void lua_object_get_by_native_string_00b68100(LuaObjectStorage* table, LuaObjectStorage* out,
                                                          const NativeStringImage* key) = 0;
    // 00B67720, __thiscall(ECX = table; out, int index) RET 8 -> EAX = out.
    virtual LuaObjectStorage* lua_object_get_by_index_00b67720(LuaObjectStorage* table,
                                                               LuaObjectStorage* out,
                                                               std::int32_t index) = 0;
    // 00B67690, __thiscall(ECX = destination; const LuaObject*) RET 4.
    virtual void lua_object_assign_00b67690(LuaObjectStorage* destination,
                                            const LuaObjectStorage* source) = 0;
    // 00B66DE0 BSP_LuaStateOwner_ReleaseTrackedObject, __thiscall(ECX = owner; LuaObject*,
    // index, byte remove) RET 0Ch (the inlined LuaObject destructor's call).
    virtual void lua_state_owner_release_tracked_00b66de0(void* owner, LuaObjectStorage* object,
                                                          std::int32_t index, std::uint8_t remove) = 0;
};

// 00B68D70, __thiscall(ECX = LuaObject; LuaObject* out (sret), const NativeString* path,
// bool* ok) RET 0Ch -> EAX = out.
LuaObjectStorage* lua_object_resolve_dot_path_00b68d70(LuaObjectStorage* self, LuaObjectStorage* out,
                                                       const NativeStringImage* path, std::uint8_t* ok,
                                                       LuaDotPathHost& host);

// ---------------------------------------------------------------------------
// Small ranked blockers.
// ---------------------------------------------------------------------------
struct SmallBodiesHost {
    virtual ~SmallBodiesHost() = default;
    // The pointer stored in the image global dword at `address` ([00E188D8] the
    // selected unit, [0109CF04] the platform).
    virtual void* global_pointer(std::uint32_t address) = 0;
    // 00BF6713, the checked-iterator failure; returns here.
    virtual void invalid_parameter_00bf6713() = 0;
    // Unit vtable +5Ch, __thiscall(ECX = unit; int) RET 4 -> AL.
    virtual bool unit_vslot_5c(void* unit, std::int32_t arg) = 0;
    // Unit vtable +118h, __thiscall(ECX = unit; const float[3]*) RET 4.
    virtual void unit_vslot_118(void* unit, const float* position) = 0;
    // 00A675A0 lua_type, ECX = lua_State*, EDX = index -> EAX (as called here).
    virtual std::int32_t lua_type_00a675a0(void* state, std::int32_t index) = 0;
    // 00B68460 BSP_LuaType_ToName, ECX = NativeString* out, EDX = type.
    virtual void lua_type_to_name_00b68460(NativeStringImage* out, std::int32_t type) = 0;
    // 00BECE90 BSP_Platform_PopTextEvent, __thiscall(ECX = platform; byte* value, byte* category) RET 8.
    virtual void platform_pop_text_event_00bece90(void* platform, std::uint8_t* value, std::uint8_t* category) = 0;
    // Text owner vtable slot 0, __thiscall(ECX = owner; byte value, byte category) RET 8 -> AL.
    virtual bool text_owner_vslot_0(void* owner, std::uint8_t value, std::uint8_t category) = 0;
    // 00A96750 BSP_TextOwner_HandleKeyFallback, __thiscall(ECX = owner; byte value, byte category) RET 8.
    virtual void text_owner_handle_key_fallback_00a96750(void* owner, std::uint8_t value, std::uint8_t category) = 0;
};

// 00438FE0, __thiscall(ECX = destination; source*) RET 4 -> EAX = destination.
void* transfer_cstring_key_00438fe0(Raw destination, Raw source);
// 006B8B10, __fastcall(ECX = registry) -> EAX = [registry+1Ch].
std::int32_t achievement_count_006b8b10(Raw registry);
// 00BDB2E0, __stdcall(const iterator {container, node}*, int selector) RET 8 -> EAX.
void* vfs_search_group_select_value_list_00bdb2e0(void* const* iterator, std::int32_t selector,
                                                  SmallBodiesHost& host);
// 0053D230, three stack dwords (a float[3] by value) RET 0Ch; ECX unused.
void hud_free_camera_forward_to_unit_0053d230(float x, float y, float z, SmallBodiesHost& host);
// 00B68550, __thiscall(ECX = LuaObject; NativeString* out sret) RET 4 -> EAX = out.
NativeStringImage* lua_object_get_type_name_00b68550(Raw object, NativeStringImage* out, SmallBodiesHost& host);
// 00A96F40, __thiscall(ECX = text owner; dword unused) RET 4.
void text_owner_dispatch_queued_events_00a96f40(Raw owner, SmallBodiesHost& host);

struct ClearAndResizeHost {
    virtual ~ClearAndResizeHost() = default;
    // 00BF6713, the checked-iterator failure; returns here.
    virtual void invalid_parameter_00bf6713() = 0;
    // 004FCBE0 STL_CheckedDwordVector_EraseRange, __thiscall(ECX = vector; out iterator*,
    // {owner, first}, {owner, last}) RET 14h.
    virtual void dword_vector_erase_range_004fcbe0(void* vector, void** out_iterator, void* first_owner,
                                                   std::uint32_t* first, void* last_owner, std::uint32_t* last) = 0;
    // 004254B0 (TRIV_body, the log printf), __cdecl(format, ...): `count` dword arguments.
    virtual void log_004254b0(const char* format, const std::uint32_t* args, int count) = 0;
    // 004FDE20 BSP_LoadingQueue_GetInstance -> EAX; 005092E0 BSP_LoadingQueue_StopAndWait(ECX).
    virtual void* loading_queue_get_instance_004fde20() = 0;
    virtual void loading_queue_stop_and_wait_005092e0(void* queue) = 0;
    // 00504960 TMPL_RefPtrArray_Resize, __thiscall(ECX = array; int count) RET 4.
    virtual void ref_ptr_array_resize_00504960(void* array, std::int32_t count) = 0;

    // 004DA180 BSP_NativeResourceRecordVector_Reserve, __thiscall(ECX = header; int capacity) RET 4.
    virtual void resource_record_vector_reserve_004da180(void* header, std::int32_t capacity) = 0;
    // 004C3020 BSP_PooledStringList_AllocateSentinel (called with ECX = the list) -> EAX head.
    virtual void* string_list_allocate_sentinel_004c3020(void* list) = 0;
    // 004D45A0 BSP_StringAliasRecord_Destroy, __fastcall(ECX = 2Ch record).
    virtual void string_alias_record_destroy_004d45a0(void* record) = 0;
};

// 0050A950, __fastcall(ECX = tactical-library preview) RET.
void tactical_library_preview_clear_rotors_0050a950(Raw preview, ClearAndResizeHost& host);
// 004DC410, __thiscall(ECX = {data, count, capacity} header; int count) RET 4.
void particle_record_array_resize_004dc410(Raw header, std::int32_t count, ClearAndResizeHost& host);

}  // namespace bsp::cc13_front8
