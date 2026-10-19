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
// 00737110: the same with the singleton base at +4h and vtable 00CFE9F4 at +0h (global 010904F8).
void* msar_provider_scalar_deleting_destruct_00737110(MistagHost& host, void* object, std::uint32_t flags);

}  // namespace bsp::cc13_mistag

#endif
