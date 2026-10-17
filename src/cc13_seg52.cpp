// cc13_seg52: segment 52 reconstructions (docs/CC13_SEG52.md). See include/bsp/cc13_seg52.hpp.

#include "bsp/cc13_seg52.hpp"

#include <cstring>

namespace bsp::cc13_seg52 {

namespace {
// The NativeString copy idiom every pair helper inlines: unless destination and source are the
// same object, 0041DD40(destination; source size, 1), then for a non-zero source size memcpy
// (CRT 00BF7680) of destination-size bytes from the source buffer.
void copy_native_string(Seg52Host& host, Object destination, Object source) {
    if (destination != source) {
        host.native_string_resize_0041dd40(destination, field<std::uint32_t>(source, 0x0), 1);
        if (field<std::uint32_t>(source, 0x0) != 0) {
            std::memcpy(reinterpret_cast<void*>(field<std::uintptr_t>(destination, 0x4)),
                        reinterpret_cast<const void*>(field<std::uintptr_t>(source, 0x4)),
                        field<std::uint32_t>(destination, 0x0));
        }
    }
}

// The NativeString destructor idiom: with a buffer, ReturnBlock(pool; buffer, size + 1, 1).
void release_native_string(Seg52Host& host, Object text) {
    Object buffer = field<Object>(text, 0x4);
    if (buffer != nullptr) {
        std::uint32_t size = field<std::uint32_t>(text, 0x0) + 1;
        Object pool = host.sized_storage_pool_get_singleton_00419cc0();
        host.sized_storage_pool_return_block_00bd1510(pool, buffer, size, 1);
    }
}
}  // namespace

// ---------------------------------------------------------------------------
// 0082AD20 BSP_NativeStringPair_Destruct, __fastcall(ECX pair), SEH frame (state 0 then -1):
// the second string (+8) is released first, then the first (+0). The pool call pattern is
// PUSH 1, PUSH size+1, PUSH buffer, CALL 00419CC0, MOV ECX,EAX, CALL 00BD1510.
// ---------------------------------------------------------------------------
void native_string_pair_destruct_0082ad20(Seg52Host& host, Object pair) {
    release_native_string(host, pair + 0x8);
    release_native_string(host, pair + 0x0);
}

// 0082B020 BSP_NativeStringPair_Assign, __thiscall(ECX pair; source*), RET 4 -> pair: the copy
// idiom on +0 then on +8.
Object native_string_pair_assign_0082b020(Seg52Host& host, Object pair, Object source) {
    copy_native_string(host, pair + 0x0, source + 0x0);
    copy_native_string(host, pair + 0x8, source + 0x8);
    return pair;
}

// 0082B170 BSP_NativeStringPair_Construct, __thiscall(ECX pair; first*, second*), RET 8 ->
// pair, SEH frame: +0 = {0, 0} then the copy idiom from first; +8 = {0, 0} then from second.
Object native_string_pair_construct_0082b170(Seg52Host& host, Object pair, Object first,
                                             Object second) {
    field<std::uint32_t>(pair, 0x0) = 0;
    field<std::uint32_t>(pair, 0x4) = 0;
    copy_native_string(host, pair + 0x0, first);
    field<std::uint32_t>(pair, 0x8) = 0;
    field<std::uint32_t>(pair, 0xC) = 0;
    copy_native_string(host, pair + 0x8, second);
    return pair;
}

// 0082B270 BSP_NativeStringPair_CopyConstruct, __thiscall(ECX pair; source*), RET 4 -> pair:
// 0082B170 with source+0 and source+8.
Object native_string_pair_copy_construct_0082b270(Seg52Host& host, Object pair, Object source) {
    field<std::uint32_t>(pair, 0x0) = 0;
    field<std::uint32_t>(pair, 0x4) = 0;
    copy_native_string(host, pair + 0x0, source + 0x0);
    field<std::uint32_t>(pair, 0x8) = 0;
    field<std::uint32_t>(pair, 0xC) = 0;
    copy_native_string(host, pair + 0x8, source + 0x8);
    return pair;
}

// 00835840 BSP_WeaponDirector_DetectedTarget, __fastcall(ECX director) -> EAX: JMP 00803D40
// (ECX = [[director+34h]+54h], EDX = [director+238h]).
Object weapon_director_detected_target_00835840(Seg52Host& host, Object director) {
    Object party = field<Object>(field<Object>(director, 0x34), 0x54);
    return host.recon_filter_undetected_unit_00803d40(party, field<Object>(director, 0x238));
}

// 00835F10 BSP_ShipWeaponDirector_Destruct, __fastcall(ECX director), SEH frame (state 0 ->
// -1): 006E0860(ECX = director+224h) then 0071D480(ECX = director).
void ship_weapon_director_destruct_00835f10(Seg52Host& host, Object director) {
    host.observer_member_destruct_006e0860(director + 0x224);
    host.weapon_director_base_destruct_0071d480(director);
}

// 0082EC10 BSP_ShipTrafficRecord_CopyConstructAt, __fastcall(ECX record, EDX source), SEH frame:
// with a record (the placement-new test), 0049FA70(ECX = record; source) then +4Ch copied. EAX
// is left as whatever the callee returned (no return value).
void ship_traffic_record_copy_construct_at_0082ec10(Seg52Host& host, Object record, Object source) {
    if (record != nullptr) {
        host.traffic_record_base_copy_0049fa70(record, source);
        field<std::uint32_t>(record, 0x4C) = field<std::uint32_t>(source, 0x4C);
    }
}

// 0082EC90 BSP_ShipTrafficRecord_Construct, __fastcall(ECX record) -> record: 0049F9B0 then
// +4Ch = 0.
Object ship_traffic_record_construct_0082ec90(Seg52Host& host, Object record) {
    host.traffic_record_base_construct_0049f9b0(record);
    field<std::uint32_t>(record, 0x4C) = 0;
    return record;
}

// 0084D8F0 / 0084D910 / 0084D930: the squadron command block's category filters (vtable slots
// 48h / 24h / 28h), __thiscall with ECX unused; (category), RET 4 -> EAX 1 or 0. True for
// {1, 0}, {1, 2} and {0, 3, 2} respectively (the compare order of the listing).
std::uint32_t squadron_block_allows_category48_0084d8f0(std::int32_t category) {
    return (category == 1 || category == 0) ? 1u : 0u;
}
std::uint32_t squadron_block_allows_category24_0084d910(std::int32_t category) {
    return (category == 1 || category == 2) ? 1u : 0u;
}
std::uint32_t squadron_block_allows_category28_0084d930(std::int32_t category) {
    return (category == 0 || category == 3 || category == 2) ? 1u : 0u;
}

// 0084D960 BSP_PlaneSquadronCommandBlock_Slot50, __fastcall(ECX block) -> EAX: 0 only when both
// bytes +3Dh and +3Ch are set, else 1.
std::uint32_t squadron_block_slot50_0084d960(Object block) {
    if (field<std::uint8_t>(block, 0x3D) != 0 && field<std::uint8_t>(block, 0x3C) != 0) {
        return 0;
    }
    return 1;
}

// ---------------------------------------------------------------------------
// 0084DB50 BSP_PilotOrder_ApplyAndRaiseAttackMode, __thiscall(ECX block; order), RET 4:
// 0071C130(ECX = block; order); when [block+16Ch] is the landing state object 00E08FA0,
// 007EFB60(ECX = [block+224h]); 009F6DD0(ECX = [block+38h]; order); 007ED3F0(ECX =
// [block+224h]; 1).
// ---------------------------------------------------------------------------
void pilot_order_apply_and_raise_attack_mode_0084db50(Seg52Host& host, Object block, Object order) {
    host.weapon_director_reset_completion_0071c130(block, order);
    if (field<std::uint32_t>(block, 0x16C) == 0x00E08FA0u) {
        host.plane_squadron_end_landing_007efb60(field<Object>(block, 0x224));
    }
    host.squadron_auto_target_clear_latches_009f6dd0(field<Object>(block, 0x38), order);
    host.pilot_control_set_attack_mode_007ed3f0(field<Object>(block, 0x224), 1);
}

// ---------------------------------------------------------------------------
// 008346E0 BSP_ShipUnit_BindScrewAndRudderNodes, __fastcall(ECX unit) -> AL = 1. Seven
// 0071AD50 lookups, each with ECX = [unit+360h] ? [[unit+360h]+160h] : 0 re-read per call:
// "csavar_1".."csavar_4" -> +106Ch..+1078h, "rudder" (00D06170) -> +107Ch, "rudder_1" ->
// +1080h, "rudder_2" -> +1084h; then +108Ch, +1090h, +1094h, +1098h, +1088h = 0.0f (MOVSS).
// ---------------------------------------------------------------------------
std::uint8_t ship_unit_bind_screw_and_rudder_nodes_008346e0(Seg52Host& host, Object unit) {
    static constexpr struct {
        const char* name;
        std::size_t offset;
    } kNodes[] = {{"csavar_1", 0x106C}, {"csavar_2", 0x1070}, {"csavar_3", 0x1074},
                  {"csavar_4", 0x1078}, {"rudder", 0x107C},   {"rudder_1", 0x1080},
                  {"rudder_2", 0x1084}};
    for (const auto& node : kNodes) {
        Object holder = field<Object>(unit, 0x360);
        Object model = holder != nullptr ? field<Object>(holder, 0x160) : nullptr;
        field<Object>(unit, node.offset) = host.model_find_note_node_0071ad50(model, node.name);
    }
    field<float>(unit, 0x108C) = 0.0f;
    field<float>(unit, 0x1090) = 0.0f;
    field<float>(unit, 0x1094) = 0.0f;
    field<float>(unit, 0x1098) = 0.0f;
    field<float>(unit, 0x1088) = 0.0f;
    return 1;
}

// 00835700 (FUN_00835700), __fastcall(ECX director) -> EAX = (byte +3Dh == 0) (SETE).
std::uint32_t weapon_director_move_forbidden_00835700(Object director) {
    return field<std::uint8_t>(director, 0x3D) == 0 ? 1u : 0u;
}

// 00835710 (FUN_00835710), __thiscall(ECX director; value), RET 4: vtable[40h](1), then
// vtable[44h](value != 0 ? 0 : 1).
void weapon_director_set_fire_and_move_00835710(Seg52Host& host, Object director,
                                                std::int32_t value) {
    host.director_vslot_40(director, 1);
    host.director_vslot_44(director, value != 0 ? 0u : 1u);
}

// ---------------------------------------------------------------------------
// 00846D30 BSP_Shipyard_UnlinkFromWorldLists, __fastcall(ECX shipyard): 006D3620, then the
// first node of the list world+54h (head [world+58h], next +4) whose +8 is the shipyard is
// erased (004837D0, ECX = world+54h); then the same for world+360h (head +364h); world =
// [shipyard+30h], re-read for the second list.
// ---------------------------------------------------------------------------
namespace {
void erase_first_node_for(Seg52Host& host, Object list, Object unit) {
    for (Object node = field<Object>(list, 0x4); node != nullptr; node = field<Object>(node, 0x4)) {
        if (field<Object>(node, 0x8) == unit) {
            host.unit_list_erase_node_004837d0(list, node);
            return;
        }
    }
}
}  // namespace

void shipyard_unlink_from_world_lists_00846d30(Seg52Host& host, Object shipyard) {
    host.unit_detach_from_early_world_lists_006d3620(shipyard);
    erase_first_node_for(host, field<Object>(shipyard, 0x30) + 0x54, shipyard);
    erase_first_node_for(host, field<Object>(shipyard, 0x30) + 0x360, shipyard);
}

// 0084CB30 BSP_SmallReconPlane_UnlinkFromWorldLists, __fastcall(ECX plane): 0074E610, then the
// same erase on the list [plane+30h]+114h (head +118h).
void small_recon_plane_unlink_from_world_lists_0084cb30(Seg52Host& host, Object plane) {
    host.plane_unit_detach_from_world_lists_0074e610(plane);
    erase_first_node_for(host, field<Object>(plane, 0x30) + 0x114, plane);
}

// ---------------------------------------------------------------------------
// 0084C6E0 BSP_RapidTurningGun_ApplyAimAndFireMessage, __thiscall(ECX gun; message), RET 4.
// +4D0h = message+28h; 0085B0F0(m20, m24, m20, m24) (FLD/FSTP floats from the stack map);
// with byte m3Ah clear 0072D5B0(0, 1); otherwise 0072D5B0(1, 1) when [[gun+3F4h]+80h] == 6,
// then operator new(8) (00BF681B) = {vtable 00CFE4F8, MOVSS m3Ch} replacing +41Ch (the old
// one deleted through vtable[0](1) and cleared first) when the allocation succeeded; then
// vtable[1D8h](1, m30, m34) when byte m2Ch is set else (0, 0.0f, 0.0f); [gun+3B4h]+4 = word m38h.
// ---------------------------------------------------------------------------
void rapid_turning_gun_apply_aim_and_fire_message_0084c6e0(Seg52Host& host, Object gun,
                                                           Object message) {
    field<std::uint32_t>(gun, 0x4D0) = field<std::uint32_t>(message, 0x28);
    float horz = field<float>(message, 0x20);
    float vert = field<float>(message, 0x24);
    host.turning_gun_set_angles_immediate_0085b0f0(gun, horz, vert, horz, vert);
    if (field<std::uint8_t>(message, 0x3A) == 0) {
        host.gun_select_fire_mode_0072d5b0(gun, 0, 1);
    } else {
        if (field<std::int32_t>(field<Object>(gun, 0x3F4), 0x80) == 6) {
            host.gun_select_fire_mode_0072d5b0(gun, 1, 1);
        }
        Object shot = host.operator_new_00bf681b(8);
        if (shot != nullptr) {
            field<std::uint32_t>(shot, 0x0) = 0x00CFE4F8;
            field<float>(shot, 0x4) = field<float>(message, 0x3C);
            Object old = field<Object>(gun, 0x41C);
            if (old != nullptr) {
                host.object_delete_vslot_00(old, 1);
                field<Object>(gun, 0x41C) = nullptr;
            }
            field<Object>(gun, 0x41C) = shot;
        }
    }
    if (field<std::uint8_t>(message, 0x2C) != 0) {
        host.gun_vslot_1d8(gun, 1, field<float>(message, 0x30), field<float>(message, 0x34));
    } else {
        host.gun_vslot_1d8(gun, 0, 0.0f, 0.0f);
    }
    field<std::uint16_t>(field<Object>(gun, 0x3B4), 0x4) = field<std::uint16_t>(message, 0x38);
}

// 0084C7E0 BSP_RapidTurningGun_HandleNetworkMessage, __thiscall(ECX gun; message), RET 4 -> AL:
// message byte +10h == B0h runs 0084C6E0 and returns 1; anything else JMPs to 0072D830.
bool rapid_turning_gun_handle_network_message_0084c7e0(Seg52Host& host, Object gun,
                                                       Object message) {
    if (field<std::uint8_t>(message, 0x10) != 0xB0) {
        return host.gun_handle_network_message_0072d830(gun, message);
    }
    rapid_turning_gun_apply_aim_and_fire_message_0084c6e0(host, gun, message);
    return true;
}

namespace {
TaggedValue tagged(std::uint32_t tag, const void* value) {
    return TaggedValue{tag, reinterpret_cast<std::uintptr_t>(value)};
}
}  // namespace

// ---------------------------------------------------------------------------
// 0084C640 BSP_STGun_DescribeState, __thiscall(ECX gun; writer), RET 4: 0085A130(gun, writer),
// writer[4]({0, "_singleTurningGun"} 00D0B9E4), writer[0Ch]({0, "lastFiredBarrel"}, {1,
// +4D0h}), writer[0Ch]({0, "singleShot"}, {3, byte +4D4h}), writer[8](). The byte value is
// stored with MOV byte [EAX+4],CL: the three upper bytes of that slot are uninitialised stack
// in the image and zero here.
// ---------------------------------------------------------------------------
void st_gun_describe_state_0084c640(Seg52Host& host, Object gun, Object writer) {
    host.turning_gun_debug_dump_0085a130(gun, writer);
    host.writer_open_vslot_04(writer, tagged(0, "_singleTurningGun"));
    host.writer_field_vslot_0c(writer, tagged(0, "lastFiredBarrel"),
                               TaggedValue{1, field<std::uint32_t>(gun, 0x4D0)});
    host.writer_field_vslot_0c(writer, tagged(0, "singleShot"),
                               TaggedValue{3, field<std::uint8_t>(gun, 0x4D4)});
    host.writer_close_vslot_08(writer);
}

// 0084C800 BSP_STGun_ReadArchive, __fastcall(ECX gun), SEH frame: 0085BB50 first; then on a saved
// game (record gun+C0h, record+4 == 3) the reader enters "_singleTurningGun" and reads
// lastFiredBarrel {1, &+4D0h} and singleShot {3, &+4D4h}; leave and destroy.
void st_gun_read_archive_0084c800(Seg52Host& host, Object gun) {
    host.turning_gun_load_instance_state_0085bb50(gun);
    Object record = field<Object>(gun, 0xC0);
    if (record == nullptr || field<std::int32_t>(record, 0x4) != 3) {
        return;
    }
    LuaObject data{};
    host.entity_get_saved_lua_data_009238a0(gun, &data);
    alignas(4) std::uint8_t reader[0x14]{};
    host.lua_reader_construct_004425c0(reader, &data);
    host.lua_reader_enter_key_00bd8e20(reader, tagged(0, "_singleTurningGun"));
    host.lua_reader_read_field_00bd6830(reader, tagged(0, "lastFiredBarrel"), tagged(1, gun + 0x4D0));
    host.lua_reader_read_field_00bd6830(reader, tagged(0, "singleShot"), tagged(3, gun + 0x4D4));
    host.lua_reader_leave_key_00bd7a20(reader);
    host.lua_reader_destroy_00441a20(reader);
}

// ---------------------------------------------------------------------------
// 0084F750 (FUN_0084F750), __thiscall(ECX entity; class), RET 4. class->vtable[10h]() first;
// +178h = class+18h; +17Ch/+180h = class+20h/+24h (FLD/FSTP float); the RefPtr at +184h takes
// class+28h when it differs: InterlockedIncrement(new+4) for a non-null new, then the old one is
// released (InterlockedDecrement(old+4), vtable[0] at zero); +188h..+190h = class+2Ch..+34h
// (floats); +194h = class+38h.
// ---------------------------------------------------------------------------
void stationary_apply_class_0084f750(Seg52Host& host, Object entity, Object vehicle_class) {
    host.class_vslot_10(vehicle_class);
    field<std::uint32_t>(entity, 0x178) = field<std::uint32_t>(vehicle_class, 0x18);
    field<float>(entity, 0x17C) = field<float>(vehicle_class, 0x20);
    field<float>(entity, 0x180) = field<float>(vehicle_class, 0x24);
    Object incoming = field<Object>(vehicle_class, 0x28);
    Object old = field<Object>(entity, 0x184);
    if (old != incoming) {
        field<Object>(entity, 0x184) = incoming;
        if (incoming != nullptr) {
            host.interlocked_increment_00ce221c(&field<std::int32_t>(incoming, 0x4));
        }
        if (old != nullptr && host.interlocked_decrement_00ce2220(&field<std::int32_t>(old, 0x4)) == 0) {
            host.refcounted_vslot_00(old);
        }
    }
    field<float>(entity, 0x188) = field<float>(vehicle_class, 0x2C);
    field<float>(entity, 0x18C) = field<float>(vehicle_class, 0x30);
    field<float>(entity, 0x190) = field<float>(vehicle_class, 0x34);
    field<std::uint32_t>(entity, 0x194) = field<std::uint32_t>(vehicle_class, 0x38);
}

// 00851500 BSP_StationaryClass_CreateEntity, __fastcall(ECX class) -> entity, SEH frame:
// operator new(1ACh) (00BF55BE), memset of the raw return (no null test), 00748A40(ECX =
// memory) when non-null else 0, then 0084F750(ECX = entity; class) with no null test.
Object stationary_class_create_entity_00851500(Seg52Host& host, Object vehicle_class) {
    Object memory = host.operator_new_00bf55be(0x1AC);
    std::memset(memory, 0, 0x1AC);
    Object entity = memory != nullptr ? host.stationary_prop_construct_00748a40(memory) : nullptr;
    stationary_apply_class_0084f750(host, entity, vehicle_class);
    return entity;
}

// 0084C500 BSP_Gun_ApplyAimMessageIfFunction6, __thiscall(ECX gun; flag), RET 4: when
// [[gun+3F4h]+80h] == 6, 0072D5B0(ECX = gun; 1, flag).
void gun_apply_aim_message_if_function6_0084c500(Seg52Host& host, Object gun, std::uint32_t flag) {
    if (field<std::int32_t>(field<Object>(gun, 0x3F4), 0x80) == 6) {
        host.gun_select_fire_mode_0072d5b0(gun, 1, flag);
    }
}

// ---------------------------------------------------------------------------
// 0084C920 BSP_SmallReconPlaneUnitInstance_Construct, __thiscall(ECX plane; flag), RET 4 ->
// plane: 0074E0F0(flag), then the eight vptrs 00D0BA80 (+0), 00D0BA68 (+10h), 00D0BA60 (+24h),
// 00D0BA5C (+170h), 00D0BA54 (+1E4h), 00D0BA3C (+310h), 00D0BA38 (+38Ch), 00D0B9F8 (+72Ch),
// class id +C4h = 15h and +AACh = 0Bh.
// ---------------------------------------------------------------------------
Object small_recon_plane_unit_instance_construct_0084c920(Seg52Host& host, Object plane,
                                                          std::uint32_t flag) {
    host.recon_plane_unit_instance_construct_0074e0f0(plane, flag);
    field<std::uint32_t>(plane, 0x000) = 0x00D0BA80;
    field<std::uint32_t>(plane, 0x010) = 0x00D0BA68;
    field<std::uint32_t>(plane, 0x024) = 0x00D0BA60;
    field<std::uint32_t>(plane, 0x170) = 0x00D0BA5C;
    field<std::uint32_t>(plane, 0x1E4) = 0x00D0BA54;
    field<std::uint32_t>(plane, 0x310) = 0x00D0BA3C;
    field<std::uint32_t>(plane, 0x38C) = 0x00D0BA38;
    field<std::uint32_t>(plane, 0x72C) = 0x00D0B9F8;
    field<std::int32_t>(plane, 0xC4) = 0x15;
    field<std::int32_t>(plane, 0xAAC) = 0xB;
    return plane;
}

// 0084C990 (FUN_0084C990): MOV EAX,0Bh; RET (the class's recon category slot).
std::uint32_t small_recon_plane_class_recon_category_0084c990() { return 0xB; }

// 0084CA50 BSP_SmallReconPlaneClass_CreateUnitInstance, __thiscall(ECX class; flag), RET 4,
// SEH frame: operator new(E94h), memset with no null test, 0084C920(flag) when non-null else 0,
// 009553D0(ECX = plane; class) and plane+354h = class with no null test.
Object small_recon_plane_class_create_unit_instance_0084ca50(Seg52Host& host, Object vehicle_class,
                                                            std::uint32_t flag) {
    Object memory = host.operator_new_00bf55be(0xE94);
    std::memset(memory, 0, 0xE94);
    Object plane = memory != nullptr ? host.small_recon_plane_construct_0084c920(memory, flag) : nullptr;
    host.unit_set_vehicle_class_009553d0(plane, vehicle_class);
    field<Object>(plane, 0x354) = vehicle_class;
    return plane;
}

// 0084F930 BSP_NativeString_ReleaseStorage_0084f930, __fastcall(ECX string): the NativeString
// release idiom (ReturnBlock(pool; buffer, size + 1, 1) when a buffer exists).
void native_string_release_storage_0084f930(Seg52Host& host, Object text) {
    release_native_string(host, text);
}

// 0084FA70 BSP_NativeStringPair_Construct_0084FA70, __thiscall(ECX record; NativeString*,
// const dword*), RET 8 -> record: {0, 0}, the copy idiom, then +8 = *value. (A string plus a
// dword, 0Ch bytes, not a pair of strings.)
Object string_dword_record_construct_0084fa70(Seg52Host& host, Object record, Object text,
                                              const std::uint32_t* value) {
    field<std::uint32_t>(record, 0x0) = 0;
    field<std::uint32_t>(record, 0x4) = 0;
    copy_native_string(host, record, text);
    field<std::uint32_t>(record, 0x8) = *value;
    return record;
}

// 0084FAC0 BSP_NativeStringPair_CopyConstruct_0084FAC0, __thiscall(ECX record; source*), RET 4
// -> record: {0, 0}, the copy idiom from source, +8 = source+8.
Object string_dword_record_copy_construct_0084fac0(Seg52Host& host, Object record, Object source) {
    field<std::uint32_t>(record, 0x0) = 0;
    field<std::uint32_t>(record, 0x4) = 0;
    copy_native_string(host, record, source);
    field<std::uint32_t>(record, 0x8) = field<std::uint32_t>(source, 0x8);
    return record;
}

// ---------------------------------------------------------------------------
// 0084F080 BSP_Stationaries_ReadArchive, __fastcall(ECX archive). For each stationary s of the
// list (null begin or index >= (end-begin)>>2, unsigned, ends it): archive[4]({1, i}),
// archive[10h]({0, "damage"} 00CFD7C8, {2, &s+1A0h}); x = s+150h:
//  * x > 0 (COMISS/JBE: NaN takes the other arm): m = (float)((float)(1 - x) * s+17Ch); when
//    damage >= m (FCOMIP/JC: NaN skips) damage = m;
//  * else m = s+17Ch; when damage >= m: damage = m, byte s+1A4h = 1, s->vtable[70h](1).
// archive[8]() closes every element.
// ---------------------------------------------------------------------------
void stationaries_read_archive_0084f080(Seg52Host& host, Object archive) {
    for (std::uint32_t i = 0;; ++i) {
        Object* begin = host.stationaries_begin_00f87560();
        if (begin == nullptr ||
            i >= static_cast<std::uint32_t>(
                     static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(host.stationaries_end_00f87564()) -
                                               reinterpret_cast<std::uintptr_t>(begin)) >> 2)) {
            return;
        }
        Object s = begin[i];
        host.archive_open_vslot_04(archive, TaggedValue{1, i});
        float* damage = &field<float>(s, 0x1A0);
        host.archive_read_vslot_10(archive, tagged(0, "damage"), tagged(2, damage));
        float x = field<float>(s, 0x150);
        if (x > 0.0f) {
            float remaining = static_cast<float>(1.0 - static_cast<double>(x));
            float m = static_cast<float>(static_cast<double>(remaining) *
                                         static_cast<double>(field<float>(s, 0x17C)));
            if (*damage >= m) {
                *damage = m;
            }
        } else {
            float m = field<float>(s, 0x17C);
            if (*damage >= m) {
                *damage = m;
                field<std::uint8_t>(s, 0x1A4) = 1;
                host.stationary_vslot_70(s, 1);
            }
        }
        host.archive_close_vslot_08(archive);
    }
}

// 0084F1A0 BSP_Stationaries_WriteArchive, __fastcall(ECX archive): for each stationary,
// archive[4]({1, i}), archive[0Ch]({0, "damage"}, {2, raw float bits of s+1A0h}), archive[8]().
void stationaries_write_archive_0084f1a0(Seg52Host& host, Object archive) {
    for (std::uint32_t i = 0;; ++i) {
        Object* begin = host.stationaries_begin_00f87560();
        if (begin == nullptr ||
            i >= static_cast<std::uint32_t>(
                     static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(host.stationaries_end_00f87564()) -
                                               reinterpret_cast<std::uintptr_t>(begin)) >> 2)) {
            return;
        }
        Object s = begin[i];
        host.archive_open_vslot_04(archive, TaggedValue{1, i});
        host.archive_write_vslot_0c(archive, tagged(0, "damage"),
                                    TaggedValue{2, field<std::uint32_t>(s, 0x1A0)});
        host.archive_close_vslot_08(archive);
    }
}

}  // namespace bsp::cc13_seg52