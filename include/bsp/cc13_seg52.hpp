#pragma once

// cc13_seg52: segment 52 (008288D0-00851CB0: ship units, shipyard, stationaries) functions that
// had no reconstruction (docs/CC13_SEG52.md), excluding the struck cc9 ranges 00836000-00837FFF
// and 0084E000-0084EFFF. New C++ interfaces over the image's Win32 layouts; nothing here is wired
// into the game. Names are hypotheses, not recovered symbols. Every call the image makes is a
// named host hook invoked at the same point with the same arguments.

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg52 {

using Object = std::uint8_t*;

template <typename T>
T& field(Object base, std::size_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

// A tagged value as the Lua reader / field writer takes it by value: {tag, value}; tag 0 is a
// string pointer, 1 an int, 3 a byte.
struct TaggedValue {
    std::uint32_t tag;
    std::uintptr_t value;
};

struct LuaObject {
    alignas(4) std::uint8_t bytes[0x14];
};

struct Seg52Host {
    virtual ~Seg52Host() = default;

    // 0041DD40 NativeString resize, __thiscall(ECX = string {size, ptr}; size, flag), RET 8.
    virtual void native_string_resize_0041dd40(Object native_string, std::uint32_t size,
                                               std::int32_t flag) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton() -> EAX (its caller pushes ReturnBlock's three
    // arguments first); 00BD1510 BSP_SizedStoragePool_ReturnBlock, __thiscall(ECX = pool;
    // block, size, flag), RET 0Ch.
    virtual Object sized_storage_pool_get_singleton_00419cc0() = 0;
    virtual void sized_storage_pool_return_block_00bd1510(Object pool, Object block,
                                                          std::uint32_t size,
                                                          std::uint32_t flag) = 0;
    // 00803D40 BSP_Recon_FilterUndetectedUnit, __fastcall(ECX = party, EDX = unit) -> EAX.
    virtual Object recon_filter_undetected_unit_00803d40(Object party, Object unit) = 0;
    // 006E0860 BSP_ObserverMember9348_Destruct, __thiscall(ECX = member).
    virtual void observer_member_destruct_006e0860(Object member) = 0;
    // 0071D480, the weapon director base destructor body, __thiscall(ECX = director).
    virtual void weapon_director_base_destruct_0071d480(Object director) = 0;
    // 0049F9B0 / 0049FA70: the traffic record's base construct / copy construct,
    // __thiscall(ECX = record[; source]), RET 0 / RET 4.
    virtual void traffic_record_base_construct_0049f9b0(Object record) = 0;
    virtual void traffic_record_base_copy_0049fa70(Object record, Object source) = 0;
    // 0071C130 BSP_WeaponDirector_ResetCompletionState, __thiscall(ECX = director; order), RET 4.
    virtual void weapon_director_reset_completion_0071c130(Object director, Object order) = 0;
    // 007EFB60 BSP_PlaneSquadron_EndLandingAndRequestLeader, __thiscall(ECX = squadron).
    virtual void plane_squadron_end_landing_007efb60(Object squadron) = 0;
    // 009F6DD0 BSP_SquadronAutoTarget_ClearLatches, __thiscall(ECX = auto target; order), RET 4.
    virtual void squadron_auto_target_clear_latches_009f6dd0(Object auto_target, Object order) = 0;
    // 007ED3F0 BSP_PilotControl_SetAttackMode, __thiscall(ECX = squadron; mode), RET 4.
    virtual void pilot_control_set_attack_mode_007ed3f0(Object squadron, std::int32_t mode) = 0;

    // --- model nodes / unit lists ---
    // 0071AD50 BSP_Model_FindNoteNode, __thiscall(ECX = model; const char* name), RET 4 -> EAX.
    virtual Object model_find_note_node_0071ad50(Object model, const char* name) = 0;
    // 006D3620 BSP_UnitInstance_DetachFromEarlyWorldLists / 0074E610
    // BSP_PlaneUnit_DetachFromWorldLists, __thiscall(ECX = unit).
    virtual void unit_detach_from_early_world_lists_006d3620(Object unit) = 0;
    virtual void plane_unit_detach_from_world_lists_0074e610(Object unit) = 0;
    // 004837D0 BSP_UnitList_EraseNode, __thiscall(ECX = list; node), RET 4.
    virtual void unit_list_erase_node_004837d0(Object list, Object node) = 0;
    // [director]->vtable[40h] / [44h](ECX = director; byte), RET 4.
    virtual void director_vslot_40(Object director, std::uint32_t value) = 0;
    virtual void director_vslot_44(Object director, std::uint32_t value) = 0;

    // --- guns ---
    // 0085B0F0 BSP_TurningGun_SetAnglesImmediate, __thiscall(ECX = gun; 4 floats), RET 10h.
    virtual void turning_gun_set_angles_immediate_0085b0f0(Object gun, float a, float b, float c,
                                                           float d) = 0;
    // 0072D5B0 BSP_Gun_SelectFireMode, __thiscall(ECX = gun; mode, flag), RET 8.
    virtual void gun_select_fire_mode_0072d5b0(Object gun, std::uint32_t mode,
                                               std::uint32_t flag) = 0;
    // 00BF681B operator new (cdecl) -> EAX; [object]->vtable[0](ECX = object, 1): delete.
    virtual Object operator_new_00bf681b(std::uint32_t size) = 0;
    virtual void object_delete_vslot_00(Object object, std::uint32_t flags) = 0;
    // [gun]->vtable[1D8h](ECX = gun; byte flag, float a, float b), RET 0Ch.
    virtual void gun_vslot_1d8(Object gun, std::uint32_t flag, float a, float b) = 0;
    // 0072D830 BSP_Gun_HandleNetworkMessage, __thiscall(ECX = gun; message), RET 4 -> AL
    // (reached by a tail JMP).
    virtual bool gun_handle_network_message_0072d830(Object gun, Object message) = 0;
    // 0085A130 / 0085BB50: the turning gun's debug dump and saved-state reader.
    virtual void turning_gun_debug_dump_0085a130(Object gun, Object writer) = 0;
    virtual void turning_gun_load_instance_state_0085bb50(Object gun) = 0;
    // Field writer slots: [4](key), [0Ch](key, value), [8](); ECX = writer.
    virtual void writer_open_vslot_04(Object writer, TaggedValue key) = 0;
    virtual void writer_field_vslot_0c(Object writer, TaggedValue key, TaggedValue value) = 0;
    virtual void writer_close_vslot_08(Object writer) = 0;
    // The saved-state Lua reader: 009238A0, 004425C0, 00BD8E20, 00BD6830, 00BD7A20, 00441A20
    // (as bsp/cc13_seg53.hpp).
    virtual void entity_get_saved_lua_data_009238a0(Object entity, LuaObject* out) = 0;
    virtual void lua_reader_construct_004425c0(Object reader, LuaObject* data) = 0;
    virtual void lua_reader_enter_key_00bd8e20(Object reader, TaggedValue key) = 0;
    virtual void lua_reader_read_field_00bd6830(Object reader, TaggedValue key,
                                                TaggedValue value) = 0;
    virtual void lua_reader_leave_key_00bd7a20(Object reader) = 0;
    virtual void lua_reader_destroy_00441a20(Object reader) = 0;

    void gun_apply_aim_message_if_function6_0084c500(Seg52Host& host, Object gun, std::uint32_t flag);

// --- small recon plane ---
Object small_recon_plane_unit_instance_construct_0084c920(Seg52Host& host, Object plane,
                                                          std::uint32_t flag);
std::uint32_t small_recon_plane_class_recon_category_0084c990();
Object small_recon_plane_class_create_unit_instance_0084ca50(Seg52Host& host, Object vehicle_class,
                                                            std::uint32_t flag);

// --- stationaries ---
    // 00BF55BE operator new (cdecl); 00748A40 BSP_StationaryProp_Construct, __thiscall(ECX =
    // memory) -> EAX; [class]->vtable[10h](ECX = class).
    virtual Object operator_new_00bf55be(std::uint32_t size) = 0;
    virtual Object stationary_prop_construct_00748a40(Object memory) = 0;
    virtual void class_vslot_10(Object vehicle_class) = 0;
    // [00CE221C] InterlockedIncrement / [00CE2220] InterlockedDecrement (IAT); [object]->
    // vtable[0](ECX = object): the refcounted release-to-zero slot.
    virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* value) = 0;
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* value) = 0;
    virtual void refcounted_vslot_00(Object object) = 0;

    // --- small recon plane ---
    // 0074E0F0 BSP_ReconPlaneUnitInstance_Construct, __thiscall(ECX = plane; flag), RET 4.
    virtual void recon_plane_unit_instance_construct_0074e0f0(Object plane, std::uint32_t flag) = 0;
    // 0084C920 as a call target (ECX = memory; flag), RET 4 -> EAX; 009553D0
    // BSP_Unit_SetVehicleClass, __thiscall(ECX = unit; class), RET 4.
    virtual Object small_recon_plane_construct_0084c920(Object memory, std::uint32_t flag) = 0;
    virtual void unit_set_vehicle_class_009553d0(Object unit, Object vehicle_class) = 0;

    // --- the stationary list [00F87560, 00F87564) (re-read every iteration) ---
    virtual Object* stationaries_begin_00f87560() = 0;
    virtual Object* stationaries_end_00f87564() = 0;
    // Archive slots, ECX = archive: [4](key) opens an element, [10h](key, value*) reads one field,
    // [0Ch](key, value) writes one, [8]() closes; tagged values by value.
    virtual void archive_open_vslot_04(Object archive, TaggedValue key) = 0;
    virtual void archive_read_vslot_10(Object archive, TaggedValue key, TaggedValue value) = 0;
    virtual void archive_write_vslot_0c(Object archive, TaggedValue key, TaggedValue value) = 0;
    virtual void archive_close_vslot_08(Object archive) = 0;
    // [stationary]->vtable[70h](ECX = stationary; 1), RET 4: the wreck transition.
    virtual void stationary_vslot_70(Object stationary, std::uint32_t value) = 0;
};

// --- NativeString pairs {first {size, ptr}, second {size, ptr}} (10h bytes) ---
void native_string_pair_destruct_0082ad20(Seg52Host& host, Object pair);
Object native_string_pair_assign_0082b020(Seg52Host& host, Object pair, Object source);
Object native_string_pair_construct_0082b170(Seg52Host& host, Object pair, Object first,
                                             Object second);
Object native_string_pair_copy_construct_0082b270(Seg52Host& host, Object pair, Object source);

// --- ship weapon director ---
Object weapon_director_detected_target_00835840(Seg52Host& host, Object director);
void ship_weapon_director_destruct_00835f10(Seg52Host& host, Object director);

// --- ship traffic records (50h bytes) ---
void ship_traffic_record_copy_construct_at_0082ec10(Seg52Host& host, Object record, Object source);
Object ship_traffic_record_construct_0082ec90(Seg52Host& host, Object record);

// --- plane squadron command block ---
std::uint32_t squadron_block_allows_category48_0084d8f0(std::int32_t category);
std::uint32_t squadron_block_allows_category24_0084d910(std::int32_t category);
std::uint32_t squadron_block_allows_category28_0084d930(std::int32_t category);
std::uint32_t squadron_block_slot50_0084d960(Object block);
void pilot_order_apply_and_raise_attack_mode_0084db50(Seg52Host& host, Object block, Object order);

// --- ship unit ---
std::uint8_t ship_unit_bind_screw_and_rudder_nodes_008346e0(Seg52Host& host, Object unit);
std::uint32_t weapon_director_move_forbidden_00835700(Object director);
void weapon_director_set_fire_and_move_00835710(Seg52Host& host, Object director,
                                                std::int32_t value);

// --- world-list unlinks ---
void shipyard_unlink_from_world_lists_00846d30(Seg52Host& host, Object shipyard);
void small_recon_plane_unlink_from_world_lists_0084cb30(Seg52Host& host, Object plane);

// --- single turning gun (STGun / RapidTurningGun) ---
void rapid_turning_gun_apply_aim_and_fire_message_0084c6e0(Seg52Host& host, Object gun,
                                                           Object message);
bool rapid_turning_gun_handle_network_message_0084c7e0(Seg52Host& host, Object gun,
                                                       Object message);

void st_gun_describe_state_0084c640(Seg52Host& host, Object gun, Object writer);
void st_gun_read_archive_0084c800(Seg52Host& host, Object gun);

void gun_apply_aim_message_if_function6_0084c500(Seg52Host& host, Object gun, std::uint32_t flag);

// --- small recon plane ---
Object small_recon_plane_unit_instance_construct_0084c920(Seg52Host& host, Object plane,
                                                          std::uint32_t flag);
std::uint32_t small_recon_plane_class_recon_category_0084c990();
Object small_recon_plane_class_create_unit_instance_0084ca50(Seg52Host& host, Object vehicle_class,
                                                            std::uint32_t flag);

// --- stationaries ---
void native_string_release_storage_0084f930(Seg52Host& host, Object text);
Object string_dword_record_construct_0084fa70(Seg52Host& host, Object record, Object text,
                                              const std::uint32_t* value);
Object string_dword_record_copy_construct_0084fac0(Seg52Host& host, Object record, Object source);
void stationaries_read_archive_0084f080(Seg52Host& host, Object archive);
void stationaries_write_archive_0084f1a0(Seg52Host& host, Object archive);
void stationary_apply_class_0084f750(Seg52Host& host, Object entity, Object vehicle_class);
Object stationary_class_create_entity_00851500(Seg52Host& host, Object vehicle_class);

}  // namespace bsp::cc13_seg52
