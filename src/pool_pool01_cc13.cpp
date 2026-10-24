// Pool packet pool01 (segment 33). See include/bsp/pool_pool01_cc13.hpp for the model. Each body
// carries its address, original ABI, EH states and evidence; disk listings were read whole.

#include "bsp/pool_pool01_cc13.hpp"

namespace bsp::pool_pool01_cc13 {

using cc13_seg33a::add;
using cc13_seg33a::at;
using cc13_seg33a::ptr;

// ---------------------------------------------------------------------------------------------
// 006DEE00 BSP_ArtilleryGunnerBot_GetBulletThrowMul. __fastcall(ECX = bot), RET 0, result in ST0.
// MOV EAX,[ECX+34h]; MOV ECX,[00E19994]; LEA EAX,[EAX+EAX*4]; FLD [ECX+EAX*8+30h]: the float at
// +30h of row [bot+34h] (rows 28h bytes) of the artillery sub-director bot config table, the row
// layout the cc13_seg53 getters 00861A10.. read at +0Ch..+24h. No frame, no EH. The index is not
// bounds-checked. Name from the existing ledger (the "BulletThrowMul" column); the column name is a
// hypothesis taken over from include/bsp/gun_dispersion.hpp.
// ---------------------------------------------------------------------------------------------
float artillery_gunner_bot_get_bullet_throw_mul_006dee00(Pool01Host& host, void* bot) {
    void* const table = host.artillery_bot_config_00e19994();
    const std::int32_t row = at<std::int32_t>(bot, 0x34);
    return at<float>(table, static_cast<std::size_t>(row) * 0x28u + 0x30u);
}

// ---------------------------------------------------------------------------------------------
// 006DFD60 BSP_UnitInstance_GetHullHeading. __fastcall(ECX = unit), RET 0: FLD [ECX+1050h]. The
// unit vtable slot 50h (game_hosts_units.cpp quotes the call sites 00826CDB and the multileg path).
// No frame, no EH.
// ---------------------------------------------------------------------------------------------
float unit_instance_get_hull_heading_006dfd60(void* unit) noexcept {
    return at<float>(unit, 0x1050);
}

// ---------------------------------------------------------------------------------------------
// 006E0060 BSP_Battleship_DetachFromWorldLists. __fastcall(ECX = unit), RET 0; saves ESI only, no
// EH. 006DFFC0(unit) detaches the instance from the shared lists, then the list object at
// [unit+30h]+B4h is walked from its first node (+4; node next +4, payload +8) and the first node
// naming the unit is erased by 004837D0(ECX = list, node); the walk stops there or at the end.
// The 3 bytes 006E007D..006E007F skipped by the listing are padding before the loop head.
// HOST OVERLAP: game_hosts_units.cpp world_lists_unlink_unit_00928f24 (packet
// cc9_sunk_ship_kill_depth) models the vt[134h] detach of every ship family as one host loop that
// runs today; this body is not wired in.
// ---------------------------------------------------------------------------------------------
void battleship_detach_from_world_lists_006e0060(Pool01Host& host, void* unit) {
    host.unit_instance_detach_from_mid_world_lists_006dffc0(unit);
    void* const list = add(ptr(unit, 0x30), 0xB4);
    for (void* node = ptr(list, 4); node != nullptr; node = ptr(node, 4)) {
        if (ptr(node, 8) == unit) {
            host.unit_list_erase_node_004837d0(list, node);
            return;
        }
    }
}

// ---------------------------------------------------------------------------------------------
// 006E01C0 BSP_BombPlatformClass_ReadLuaFields. __thiscall(ECX = class, LuaObject* table), RET 4;
// SEH frame (handler 00C82428), 48h bytes of locals: three 14h-byte LuaObjects.
//   007327B0(class, table): the gun class fields first.
//   state 0: "LaunchSpeed" (00CF9318), GetNumber (00B66270) -> +DCh as float (FSTP), no default.
//   state 1: "RepeatTime" (00CF930C), GetFloatOrDefault(0.6f, [00CE3D30]) -> +E0h.
//   +E4h = 0 is stored before the third lookup; state 2: "HideAmmo" (00CF9300), when not nil
//   (00B65FB0) GetBoolean (00B66250) -> +E4h.
// Each temporary is destroyed (00B67700) right after its read, state back to -1. The fields match
// gameplay_seg33_cc13::BombPlatformClassFields (+DCh/+E0h/+E4h).
// ---------------------------------------------------------------------------------------------
void bomb_platform_class_read_lua_fields_006e01c0(Pool01Host& host, void* cls, LuaObjectStorage* table) {
    host.gun_class_read_lua_fields_007327b0(cls, table);

    LuaObjectStorage launch_speed{};
    at<float>(cls, 0xDC) = static_cast<float>(
        host.lua_object_get_number_00b66270(host.lua_object_get_by_name_00b67800(table, &launch_speed, "LaunchSpeed")));
    host.lua_object_destruct_00b67700(&launch_speed);

    LuaObjectStorage repeat_time{};
    at<float>(cls, 0xE0) = host.lua_object_get_float_or_default_00b66330(
        host.lua_object_get_by_name_00b67800(table, &repeat_time, "RepeatTime"), kBombPlatformRepeatTimeDefault_00ce3d30);
    host.lua_object_destruct_00b67700(&repeat_time);

    at<std::uint8_t>(cls, 0xE4) = 0;
    LuaObjectStorage hide_ammo{};
    host.lua_object_get_by_name_00b67800(table, &hide_ammo, "HideAmmo");
    if (!host.lua_object_is_nil_00b65fb0(&hide_ammo)) {
        at<std::uint8_t>(cls, 0xE4) = host.lua_object_get_boolean_00b66250(&hide_ammo) ? 1 : 0;
    }
    host.lua_object_destruct_00b67700(&hide_ammo);
}

// ---------------------------------------------------------------------------------------------
// 006E02C0 BSP_MultipleBombPlatformClass_ReadLuaFields. __thiscall(ECX = class, LuaObject* table),
// RET 4; SEH frame (handler 00C82460), 50h bytes of locals.
//   006E01C0(class, table); +188h (platform count) = 0.
//   state 0: platforms = table["Platforms"] (00CEB734); the element object is constructed empty
//   (00B65F50), state 1.
//   Loop, index i from 1: temp = platforms.ArgumentAt(i) (00B677E0, state 2), element = temp
//   (00B67690), nil test on element (00B65FB0), temp destroyed (state 1). While not nil:
//   element["platform"] (00CE4460, state 3) GetInteger (00B66290) -> +E8h + 4*count, ++count,
//   that temporary destroyed, i + 1, next element.
//   Then the element (state 0) and platforms (state -1) are destroyed.
// The count at +188h is not bounds-checked against the +E8h array (room for 28h entries up to
// +188h); a table with more rows would write past it, as in the image.
// ---------------------------------------------------------------------------------------------
void multiple_bomb_platform_class_read_lua_fields_006e02c0(Pool01Host& host, void* cls, LuaObjectStorage* table) {
    bomb_platform_class_read_lua_fields_006e01c0(host, cls, table);
    at<std::int32_t>(cls, 0x188) = 0;

    LuaObjectStorage platforms{};
    host.lua_object_get_by_name_00b67800(table, &platforms, "Platforms");
    LuaObjectStorage element{};
    host.lua_object_construct_00b65f50(&element);

    std::int32_t index = 1;
    auto next_is_nil = [&]() {
        LuaObjectStorage temp{};
        const bool nil = host.lua_object_is_nil_00b65fb0(
            host.lua_object_assign_00b67690(&element, host.lua_object_argument_at_00b677e0(&platforms, &temp, index)));
        host.lua_object_destruct_00b67700(&temp);
        return nil;
    };
    bool nil = next_is_nil();
    while (!nil) {
        LuaObjectStorage platform{};
        const std::int32_t value =
            host.lua_object_get_integer_00b66290(host.lua_object_get_by_name_00b67800(&element, &platform, "platform"));
        std::int32_t& count = at<std::int32_t>(cls, 0x188);
        at<std::int32_t>(cls, 0xE8 + static_cast<std::size_t>(count) * 4u) = value;
        ++count;
        host.lua_object_destruct_00b67700(&platform);
        ++index;
        nil = next_is_nil();
    }
    host.lua_object_destruct_00b67700(&element);
    host.lua_object_destruct_00b67700(&platforms);
}

// ---------------------------------------------------------------------------------------------
// 006E3260 BSP_TargetState_FieldE0NotPositive. __fastcall(ECX = object), RET 0, EAX = 0/1. XORPS
// XMM0; COMISS XMM0,[ECX+E0h]; JC -> 0: returns 1 when 0 < [+E0h] is false and the compare is
// ordered, i.e. [+E0h] <= 0; a NaN sets CF and returns 0. ai_target_weights.cpp reads it as the
// bullet class "IgnitionDelay <= 0" test; the owner type is a hypothesis.
// ---------------------------------------------------------------------------------------------
bool target_state_field_e0_not_positive_006e3260(const void* object) noexcept {
    const float value = *reinterpret_cast<const float*>(static_cast<const std::uint8_t*>(object) + 0xE0);
    return value <= 0.0f;  // false for NaN, as COMISS + JC
}

// ---------------------------------------------------------------------------------------------
// 006E8400 BSP_BulletClass_IsKindOf. __thiscall(ECX = class, unused; kind), RET 4: XOR EAX; CMP
// [ESP+4],29h; SETZ AL. The bullet class (local_player_unit_lists.cpp row 29h "MBullet").
// ---------------------------------------------------------------------------------------------
bool bullet_class_is_kind_of_006e8400(std::int32_t kind) noexcept {
    return kind == kBulletClassKind_29;
}

// ---------------------------------------------------------------------------------------------
// 006E8410 BSP_BulletClass_GetName. __thiscall(ECX unused), RET 0: MOV EAX,00CFA15C ("MBullet").
// ---------------------------------------------------------------------------------------------
const char* bullet_class_get_name_006e8410() noexcept {
    return "MBullet";
}

// ---------------------------------------------------------------------------------------------
// 006EA260 BSP_BombClass_Construct. __fastcall(ECX = class), RET 0, returns the class in EAX; no
// EH. 006E8320 (bullet class base), vtable 00CFA4B0, +D4h = 0, sub-type +8 = 9 (bomb). The same
// shape as the rocket / depth charge constructors in cc13_seg33a (006EA330, 006EA3A0); the
// order of the three stores follows the listing (vtable, +D4h, +8).
// ---------------------------------------------------------------------------------------------
void* bomb_class_construct_006ea260(Pool01Host& host, void* cls) {
    host.bullet_class_construct_006e8320(cls);
    at<std::uint32_t>(cls, 0) = kBombClassVtable_00cfa4b0;
    at<std::uint32_t>(cls, 0xD4) = 0;
    at<std::uint32_t>(cls, 8) = 9;
    return cls;
}

}  // namespace bsp::pool_pool01_cc13
