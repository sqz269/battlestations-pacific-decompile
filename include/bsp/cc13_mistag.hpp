// Library-tagged rows that are game code, packet cc13-mistag (marker cc13_mistag).
//
// The filter (docs/CC13_MISTAG.md) looked for library-tagged open rows in the 16 approved
// segments that call game routines, touch game globals or name game strings. This file holds
// the small ones: 23 one-instruction getters (MOV EAX, imm32; RET) and four singleton deleting
// destructors whose out-of-line base copy is not a recorded function.
//
// Model: raw bytes (Win32 layout, image offsets); image addresses are kept as 32-bit image
// values; callees go through MistagHost. A new C++ interface, not a drop-in binary
// replacement; nothing here is wired into bsp_game. Names are hypotheses.

#ifndef BSP_CC13_MISTAG_HPP
#define BSP_CC13_MISTAG_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_mistag {

// One getter: `address` is the routine, `slot` the .rdata vtable slot that holds it, `value`
// the image address returned in EAX, `text` the C string at `value` when it is one (else the
// returned object is a .data global).
struct TrivialGetter {
    std::uint32_t address;
    std::uint32_t slot;
    std::uint32_t value;
    const char* text;
};
inline constexpr std::size_t kTrivialGetterCount = 23;
extern const TrivialGetter kTrivialGetters[kTrivialGetterCount];

// The EAX of the getter at `address` (__thiscall or __cdecl, no arguments read, plain RET);
// 0 when `address` is not one of the table's routines.
std::uint32_t trivial_getter_result(std::uint32_t address) noexcept;

inline constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818;

// Part B: 119 out-of-line destructors of the one-instruction shape `MOV [ECX], vtable; RET`
// (C7 01 imm32 C3), __thiscall(ECX = object), RET. Checked by tools/cc13_mistag_partb_check.py.
// 98 store the session message root vtable 00CE4974, 14 store 00D056D0.
struct VtResetDestructor {
    std::uint32_t address;
    std::uint32_t vtable;
};
inline constexpr std::size_t kVtResetDestructorCount = 119;
extern const VtResetDestructor kVtResetDestructors[kVtResetDestructorCount];
// Runs the row at `address` on `object` ([object] = its vtable); false when `address` is not a row.
bool vt_reset_destruct(std::uint32_t address, void* object) noexcept;

// Part C: five scalar deleting destructors of the same inlined shape whose vtable store has no
// recorded out-of-line copy (rebuilt under the rule; the other 97 are bound). Bytes checked by
// tools/cc13_mistag_partc_check.py; __thiscall(ECX = object, flags), RET 4, EAX = object.
inline constexpr std::size_t kVtResetDeletingDestructorCount = 5;
extern const VtResetDestructor kVtResetDeletingDestructors[kVtResetDeletingDestructorCount];

class MistagHost {
public:
    virtual ~MistagHost() = default;
    // MOV dword [global], 0: the singleton publication slot.
    virtual void clear_global(std::uint32_t global) = 0;
    // 00BF65AC _free.
    virtual void free_00bf65ac(void* block) = 0;
    // 00BF6713: the CRT invalid-argument report a checked vector index reaches; no return.
    [[noreturn]] virtual void invalid_index_00bf6713() = 0;
    // object->vtable[50h](), __thiscall, ST0 result (stored as float at 00996211).
    virtual float object_vcall_50_angle(void* object) = 0;
    // 00438B10(left, right), two stack floats, RET 8, ST0 (bsp::wrapped_angle_subtract_00438b10).
    virtual float wrapped_angle_subtract_00438b10(float left, float right) = 0;

    // --- 0099A880 / 0099BE30 ---
    // [00F8A30C]: the pilot robots table pointer (a runtime .data dword).
    virtual std::uint32_t pilot_robots_table_00f8a30c() = 0;
    // 0072BBD0, __thiscall(ECX = bot, robots), RET 4: BSP_GunBot_ConstructBase.
    virtual void gun_bot_construct_base_0072bbd0(void* bot, std::uint32_t robots) = 0;
    // 00BF7CD1 eh_vector_constructor_iterator(first, size, count, ctor, dtor).
    virtual void eh_vector_construct_00bf7cd1(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t ctor, std::uint32_t dtor) = 0;
    // 0099B450, __thiscall(ECX = plan): BSP_PilotBot_SeedPlanSlots.
    virtual void pilot_bot_seed_plan_slots_0099b450(void* plan) = 0;

    // --- 00816B00 ---
    // 0080E240(holder), RET 4: BSP_ShipSyncState_GetOrCreate.
    virtual void* ship_sync_state_get_or_create_0080e240(void* holder) = 0;
    // 00813CA0, __thiscall(ECX = buffer; unit, state, phase, flag), returns the message.
    virtual void* unit_state_message_build_00813ca0(void* buffer, void* unit, void* state,
        std::int32_t phase, bool flag) = 0;
    // 00779FC0, __thiscall(ECX = unit; peer, message): BSP_Entity_SendMessageToPeer.
    virtual void entity_send_message_to_peer_00779fc0(void* unit, std::uint32_t peer, void* message) = 0;
    // 00779F80, __thiscall(ECX = unit; holder): BSP_NetEntityHolder_StampSyncTick.
    virtual void net_entity_holder_stamp_sync_tick_00779f80(void* unit, void* holder) = 0;
    // object->vtable[5Ch](id): IsKindOf.
    virtual bool is_kind_of_5c(void* object, std::uint32_t id) = 0;
    // 00859C60, __thiscall(ECX = unit): BSP_Unit_IsDamaged.
    virtual bool unit_is_damaged_00859c60(void* unit) = 0;
    // 0080FCF0 (bsp::cc13_census9::ship_guns_sync_construct_0080fcf0), returns the message.
    virtual void* ship_guns_sync_construct_0080fcf0(void* buffer, void* unit, std::int32_t phase,
        bool force) = 0;
    // 00BF7C6E eh_vector_destructor_iterator(first, size, count, dtor).
    virtual void eh_vector_destruct_00bf7c6e(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t dtor) = 0;
};

// --- Out-of-line MOV vtable; JMP base destructors (batch 2; rev-convoy's dropped-row scan) ---
// Each __thiscall(ECX = object), the JMP target's RET.
class MistagDtorHost {
public:
    virtual ~MistagDtorHost() = default;
    virtual void net_entity_create_message_destruct_007668e0(void* message) = 0;
    virtual void ref_counted_destroy_base_00bd30f0(void* object) = 0;
    virtual void observed_owner_base_destroy_0064a610(void* observed) = 0;   // ECX = object+2Ch
    virtual void target_observer_destruct_0042bca0(void* observer) = 0;      // ECX = object+30h
    virtual void point3_ptr_array_destruct_007b1a80(void* array) = 0;
};
void gun_create_message_destruct_00728b80(MistagDtorHost& host, void* message);      // 00CFDE1C
void effect_instance_base_destruct_00858150(MistagDtorHost& host, void* instance);   // 00D0C88C, +8h 00D0C888
void bot_approach_close_to_ship_destruct_009a1bf0(MistagDtorHost& host, void* block); // 00D1F470
void bot_approach_move_to_destruct_009be2b0(MistagDtorHost& host, void* block);       // 00D209CC
void path_component_destruct_007b1bd0(MistagDtorHost& host, void* component);         // 00D053D8
void path_component_destruct_007b1be0(MistagDtorHost& host, void* component);         // 00D053D8

// --- 0083B120: the game settings singleton destructor (batch 2, big four #1) ---
class SettingsDtorHost {
public:
    virtual ~SettingsDtorHost() = default;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void free_00bf6989(void* block) = 0;                 // thunk to _free
    [[noreturn]] virtual void invalid_index_00bf6713() = 0;
    // [CE2220] InterlockedDecrement(&object->count_4); 0 -> object->vtable[0]() (no argument).
    virtual void ref_release(void* object) = 0;
    // 0041E7A0, __thiscall(ECX = element): the 30h-byte element destructor of +740h.
    virtual void element30_destruct_0041e7a0(void* element) = 0;
    // 0041F370, __thiscall(ECX = +6B8h, 0), RET 4.
    virtual void container_6b8_resize_0041f370(void* container, std::uint32_t zero) = 0;
    // 00421470: element-range destroy, ECX = first, EDX = last, stack (vector, owner).
    virtual void destroy_range_00421470(void* vector, void* owner, void* first, void* last) = 0;
    // 00BF7C6E eh_vector_destructor_iterator(first, size, count, dtor).
    virtual void eh_vector_destruct_00bf7c6e(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t dtor) = 0;
    // MOV dword [00F8753C], 0: the BSP_GameSettings_GetSingleton publication.
    virtual void clear_game_settings_publication_00f8753c() = 0;
};
// 0083B120, __thiscall(ECX = settings), RET. Only caller the scalar deleting destructor 00424C20.
void game_settings_destruct_0083b120(SettingsDtorHost& host, void* settings);

// --- 007D03A0: the plane unit-instance destructor (batch 2, big four #2) ---
class PlaneDtorHost {
public:
    virtual ~PlaneDtorHost() = default;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void free_00bf6989(void* block) = 0;                 // thunk to _free
    // 00C34F70, __thiscall(ECX = [[00E188A8]+18h], body), RET 4: queue the physics body's release.
    virtual void queue_body_release_00c34f70(std::uint32_t body) = 0;
    // 007E2550, __thiscall(ECX = slot): the effect slot owned at +E80h (freed after).
    virtual void plane_effect_slot_destruct_007e2550(void* slot) = 0;
    // [CE2220] InterlockedDecrement(&object->count_4); 0 -> object->vtable[0]() (no argument).
    virtual void ref_release(void* object) = 0;
    // 007EC490, __thiscall(ECX = plane+C98h): BSP_PlaneFxComponent_Destruct.
    virtual void plane_fx_component_destruct_007ec490(void* block) = 0;
    // 007D7A10, __thiscall(ECX = plane+AB0h): the flight controller holder.
    virtual void owned_ptr_holder_destruct_007d7a10(void* holder) = 0;
    // 00BF7C6E eh_vector_destructor_iterator(first, size, count, dtor).
    virtual void eh_vector_destruct_00bf7c6e(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t dtor) = 0;
    // 007CD430: element-range destroy, ECX = first, EDX = last, stack (vector, owner), RET 8.
    virtual void destroy_range_007cd430(void* vector, void* owner, void* first, void* last) = 0;
    // 004C9550, __thiscall(ECX = plane+A30h, 0), RET 4: BSP_LiveEffectReferences_Resize.
    virtual void live_effect_references_resize_004c9550(void* refs, std::uint32_t size) = 0;
    // 006E25A0, __thiscall(ECX = plane+72Ch): the embedded device (vtable 00D06130).
    virtual void projectile_shot_destruct_006e25a0(void* device) = 0;
    // 00959940, __thiscall(ECX = plane): BSP_UnitGameObject_Destruct, the base.
    virtual void unit_game_object_destruct_00959940(void* unit) = 0;
};
// 007D03A0, __thiscall(ECX = plane), RET (no flags). Out-of-line destructor of the plane unit
// instance, primary vtable 00D05F20; reached from 17 deleting destructors and chunks.
void plane_unit_instance_destruct_007d03a0(PlaneDtorHost& host, void* plane);

// --- 007CFD20: the plane unit-instance constructor (batch 2, big four #3) ---
// RET forms of the callees follow from the caller's stack balance (no ADD ESP after any call).
class PlaneCtorHost {
public:
    virtual ~PlaneCtorHost() = default;
    // 0095CC90, __thiscall(ECX = plane; arg, cls), RET 8: BSP_UnitGameObject_Construct.
    virtual void unit_game_object_construct_0095cc90(void* plane, std::uint32_t arg, std::uint32_t cls) = 0;
    // 006E22D0, __thiscall(ECX = plane+72Ch): BSP_ProjectileShotBase_Construct.
    virtual void projectile_shot_base_construct_006e22d0(void* device) = 0;
    // 007C4560, __thiscall(ECX = plane+810h; plane), RET 4: BSP_PlaneWanderer_Construct.
    virtual void plane_wanderer_construct_007c4560(void* wanderer, void* plane) = 0;
    // 007BAF90, __thiscall(ECX = plane+84Ch; plane), RET 4.
    virtual void plane_init_control_block_007baf90(void* block, void* plane) = 0;
    // 00BF7CD1 eh_vector_constructor_iterator(first, size, count, ctor, dtor).
    virtual void eh_vector_construct_00bf7cd1(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t ctor, std::uint32_t dtor) = 0;
    // 007D7EA0, __thiscall(ECX = plane+AB0h; plane), RET 4: BSP_PlaneFlightController_Construct.
    virtual void plane_flight_controller_construct_007d7ea0(void* controller, void* plane) = 0;
    // 007CD520, __thiscall(ECX = plane+C98h), RET: BSP_PlaneFxComponent_Construct.
    virtual void plane_fx_component_construct_007cd520(void* block) = 0;
    // The .data byte 00F87151: a construction counter cycling 0..11.
    virtual std::uint8_t read_counter_00f87151() = 0;
    virtual void write_counter_00f87151(std::uint8_t value) = 0;
};
// 007CFD20, __thiscall(ECX = plane; arg, cls), RET 8, EAX = plane: BSP_PlaneUnitInstance_Construct.
void* plane_unit_instance_construct_007cfd20(PlaneCtorHost& host, void* plane, std::uint32_t arg,
    std::uint32_t cls);

// --- 0086C240: the Flare effect component's Lua reader (batch 2, big four #4) ---
// A LuaRef is an owned LuaObject temporary (the image's stack LuaObjects); release() is
// 00B67700 BSP_LuaObject_Destruct.
using LuaRef = void*;
class FlareReaderHost {
public:
    virtual ~FlareReaderHost() = default;
    // 00868BF0, __thiscall(ECX = component; row), RET 4: BSP_EffectComponent_ReadLuaBase.
    virtual void read_lua_base_00868bf0(void* component, LuaRef row) = 0;
    virtual LuaRef construct_00b65f50() = 0;                                  // an unbound object
    virtual LuaRef get_by_name_00b67800(LuaRef table, const char* key) = 0;
    virtual void release_00b67700(LuaRef object) = 0;
    virtual void assign_00b67690(LuaRef target, LuaRef source) = 0;
    virtual bool is_nil_00b65fb0(LuaRef object) = 0;
    virtual bool is_table_00b661b0(LuaRef object) = 0;
    virtual float get_number_00b66270(LuaRef object) = 0;                     // ST0
    virtual const char* get_string_00b662b0(LuaRef object) = 0;
    virtual bool get_boolean_or_default_00b662f0(LuaRef object, bool fallback) = 0;
    virtual float get_float_or_default_00b66330(LuaRef object, float fallback) = 0;
    // 00B67F10(ECX = object, out*), EAX = out: BSP_LuaObject_ReadPackedBgra; returns *out.
    virtual std::uint32_t read_packed_bgra_00b67f10(LuaRef object) = 0;
    // 00B67080 / 00B67190(ECX = table; key, value) and 00B66420(ECX = table; value).
    virtual void iterate_first_00b67080(LuaRef table, LuaRef key, LuaRef value) = 0;
    virtual void iterate_next_00b67190(LuaRef table, LuaRef key, LuaRef value) = 0;
    virtual bool is_unbound_00b66420(LuaRef table, LuaRef value) = 0;
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    virtual void eh_vector_construct_00bf7cd1(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t ctor, std::uint32_t dtor) = 0;
    // [00419CC0 BSP_SizedStoragePool_GetSingleton] then 00BD1120(size, 1) / 00BD1510(block, size, 1).
    virtual void* pool_allocate_00bd1120(std::uint32_t size) = 0;
    virtual void pool_return_00bd1510(void* block, std::uint32_t size) = 0;
    // 0041DD40, __thiscall(ECX = string header {length, data}; length, 1), RET 8.
    virtual void native_string_resize_0041dd40(void* header, std::uint32_t length) = 0;
    // 0086AA10 / 0086A280, __thiscall(ECX = component+28h; n), RET 4: the {data, count, capacity}
    // vector of element pointers.
    virtual void dword_vector_resize_zero_fill_0086aa10(void* vector, std::uint32_t size) = 0;
    virtual void dword_vector_reserve_0086a280(void* vector, std::uint32_t capacity) = 0;
};
// 0086C240, __thiscall(ECX = component; row), RET 4: BSP_EffectFlare_ReadLua. Slot 14h of the
// Flare component vtable 00D0D614 (docs/GAMEPLAY_EFFECT_COMPONENTS.md). Each ElementTable entry
// becomes a 60h-byte element record appended to the component's +28h vector.
void effect_flare_read_lua_0086c240(FlareReaderHost& host, void* component, LuaRef row);

// 0099A880, __thiscall(ECX = memory), RET, EAX = bot: BSP_PilotBot_Construct.
void* pilot_bot_construct_0099a880(MistagHost& host, void* bot);
// 0099BE30, __thiscall(ECX = plan, base), RET 4, EAX = plan: the pilot plan block embedded at
// task+4h (three C8h slots, then the controls; ends with 0099B450).
void* pilot_plan_construct_0099be30(MistagHost& host, void* plan, void* base);
// 00816B00, __thiscall(ECX = unit, holder, phase), RET 8: BSP_UnitInstance_SendShipSyncMessage.
// Unit primary vtable slot 188h.
void unit_send_ship_sync_message_00816b00(MistagHost& host, void* unit, void* holder, std::int32_t phase);

// 00996200, __thiscall(ECX = condition), RET, EAX = 0/1. Slot 0 of the vtable at 00D1EA30
// (Ghidra named it Unwind@00996200; it is no funclet: no parent, reached only from the slot).
// d = wrap([this+1Ch] - [this+18h]->vtable[50h]()); |d| as the image forms it (d when d > 0,
// else -0.0f - d); returns [this+20h] > |d|.
std::uint32_t angle_within_tolerance_00996200(MistagHost& host, void* condition);

// 007B1A80, __thiscall(ECX = array), RET: BSP_Point3PtrArray_Destruct. vtable 00D053A4; every
// element of the std::vector<T*> at +4h (first +8h, last +0Ch) is freed through a checked index
// and nulled; then the buffer is freed and +8h/+0Ch/+10h zeroed.
void point3_ptr_array_destruct_007b1a80(MistagHost& host, void* array);

// The singleton scalar deleting destructor shape: [global] = 0, vtable 00CE3818, _free(this)
// when flags & 1; __thiscall(ECX = object, flags), RET 4, EAX = object.
// 00735880: global 010904F8; 00735900: global 0109029C; 00866090: global 00F87658.
void* singleton_scalar_deleting_destruct_00735880(MistagHost& host, void* object, std::uint32_t flags);
void* singleton_scalar_deleting_destruct_00735900(MistagHost& host, void* object, std::uint32_t flags);
void* singleton_scalar_deleting_destruct_00866090(MistagHost& host, void* object, std::uint32_t flags);
// --- Irregular rows: five one-instruction game virtuals (each reached only from vtable slots) ---
// 009BE0E0, __thiscall(ECX = object), RET: XOR EAX,EAX (slots 00D209C0, 00D209C8).
std::uint32_t virtual_return_zero_009be0e0(void* object) noexcept;
// 009DAAA0, __thiscall(ECX = object; one stack argument), RET 4: XOR AL,AL (eight slots:
// 00D21564, 00D215C4, 00D215F4, 00D218EC, 00D2191C, 00D2194C, 00D21988, 00D219FC). Only AL is written.
std::uint8_t virtual_return_false_009daaa0(void* object, std::uint32_t argument) noexcept;
// 009DAC30, __thiscall(ECX = object), RET, ST0 = [00CE3958] = 2.0f (slot 00D215C0).
float virtual_return_two_009dac30(void* object) noexcept;
// 009DB000, __thiscall(ECX = object), RET: an empty body (slot 00D21660).
void virtual_no_op_009db000(void* object) noexcept;
// 00743E50, __thiscall(ECX = message), RET, EAX = message + 110h: slot 6 (00CFF370) of the
// LANDFORT_CREATE session message vtable 00CFF358.
void* landfort_create_message_field_110_00743e50(void* message) noexcept;

// A kVtResetDeletingDestructors row: [object] = vtable, then _free(object) when flags & 1;
// nullptr when `address` is not a row.
void* vt_reset_scalar_deleting_destruct(MistagHost& host, std::uint32_t address, void* object,
    std::uint32_t flags);
// 00737110: the same with the singleton base at +4h and vtable 00CFE9F4 at +0h (global 010904F8).
void* msar_provider_scalar_deleting_destruct_00737110(MistagHost& host, void* object, std::uint32_t flags);

}  // namespace bsp::cc13_mistag

#endif
