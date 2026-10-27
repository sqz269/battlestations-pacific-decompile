// Pool packet pool01 (segment 33, 006DEE00..006EA260): nine bodies left in the progress-board pool
// after the oct24 credit audit withdrew their earlier records (metadata-only or unverified).
//
// Model as in cc13_seg33a: every routine works on the original object as raw bytes (Win32 layout,
// offsets as in the image) and reaches the engine through Pool01Host, whose methods name the image
// routine they stand for. New C++ interfaces, not drop-in binary replacements; nothing here is bound
// into bsp_game. Names are hypotheses, not recovered symbols. Marker: pool_pool01_cc13.

#ifndef BSP_POOL_POOL01_CC13_HPP
#define BSP_POOL_POOL01_CC13_HPP

#include <cstdint>

#include "bsp/cc13_seg33a.hpp"
#include "bsp/cc13_sweepA.hpp"

namespace bsp::pool_pool01_cc13 {

using cc13_sweepA::LuaObjectStorage;

struct Pool01Host {
    virtual ~Pool01Host() = default;

    // [00E19994]: the artillery sub-director bot config table (28h-byte rows).
    virtual void* artillery_bot_config_00e19994() = 0;

    // 006DFFC0 BSP_UnitInstance_DetachFromMidWorldLists, __fastcall(ECX = unit), RET
    // (cc13_seg33a::unit_instance_detach_from_mid_world_lists_006dffc0).
    virtual void unit_instance_detach_from_mid_world_lists_006dffc0(void* unit) = 0;
    // 004837D0 BSP_UnitList_EraseNode, __thiscall(ECX = list, node), RET 4.
    virtual void unit_list_erase_node_004837d0(void* list, void* node) = 0;

    // 007327B0 BSP_GunClass_ReadLuaFields, __thiscall(ECX = class, LuaObject* table), RET 4.
    virtual void gun_class_read_lua_fields_007327b0(void* cls, LuaObjectStorage* table) = 0;
    // 006E8320 BSP_BulletClass_Construct, __fastcall(ECX = class), RET.
    virtual void bullet_class_construct_006e8320(void* cls) = 0;

    // LuaPlus object wrappers (bound library calls).
    // 00B67800, __thiscall(ECX = table, out, name), RET 8: out = table[name], returns out.
    virtual LuaObjectStorage* lua_object_get_by_name_00b67800(LuaObjectStorage* table, LuaObjectStorage* out,
                                                              const char* name) = 0;
    // 00B677E0 BSP_LuaObject_ArgumentAt, __thiscall(ECX = table, out, index), RET 8: out = table[index].
    virtual LuaObjectStorage* lua_object_argument_at_00b677e0(LuaObjectStorage* table, LuaObjectStorage* out,
                                                              std::int32_t index) = 0;
    // 00B65F50, __fastcall(ECX = object), RET: an empty (nil) object.
    virtual void lua_object_construct_00b65f50(LuaObjectStorage* object) = 0;
    // 00B67690, __thiscall(ECX = destination, source), RET 4: returns destination.
    virtual LuaObjectStorage* lua_object_assign_00b67690(LuaObjectStorage* destination,
                                                         const LuaObjectStorage* source) = 0;
    // 00B67700, __fastcall(ECX = object), RET.
    virtual void lua_object_destruct_00b67700(LuaObjectStorage* object) = 0;
    // 00B65FB0, __fastcall(ECX = object), RET: AL = is nil.
    virtual bool lua_object_is_nil_00b65fb0(LuaObjectStorage* object) = 0;
    // 00B66250, __fastcall(ECX = object), RET: AL.
    virtual bool lua_object_get_boolean_00b66250(LuaObjectStorage* object) = 0;
    // 00B66270, __fastcall(ECX = object), RET: ST0 (stored by the callers as float).
    virtual double lua_object_get_number_00b66270(LuaObjectStorage* object) = 0;
    // 00B66290, __fastcall(ECX = object), RET: EAX.
    virtual std::int32_t lua_object_get_integer_00b66290(LuaObjectStorage* object) = 0;
    // 00B66330, __thiscall(ECX = object, float fallback), RET 4: ST0.
    virtual float lua_object_get_float_or_default_00b66330(LuaObjectStorage* object, float fallback) = 0;
};

// [00CE3D30] = 0.6f, the "RepeatTime" default of the bomb platform class.
inline constexpr float kBombPlatformRepeatTimeDefault_00ce3d30 = 0.6f;
// 00CFA15C "MBullet", 00CFA4B0 the bomb class vtable.
inline constexpr std::uint32_t kBulletClassNameString_00cfa15c = 0x00CFA15Cu;
inline constexpr std::uint32_t kBombClassVtable_00cfa4b0 = 0x00CFA4B0u;
inline constexpr std::int32_t kBulletClassKind_29 = 0x29;

float artillery_gunner_bot_get_bullet_throw_mul_006dee00(Pool01Host& host, void* bot);
float unit_instance_get_hull_heading_006dfd60(void* unit) noexcept;
void battleship_detach_from_world_lists_006e0060(Pool01Host& host, void* unit);
void bomb_platform_class_read_lua_fields_006e01c0(Pool01Host& host, void* cls, LuaObjectStorage* table);
void multiple_bomb_platform_class_read_lua_fields_006e02c0(Pool01Host& host, void* cls, LuaObjectStorage* table);
bool target_state_field_e0_not_positive_006e3260(const void* object) noexcept;
bool bullet_class_is_kind_of_006e8400(std::int32_t kind) noexcept;
const char* bullet_class_get_name_006e8410() noexcept;
void* bomb_class_construct_006ea260(Pool01Host& host, void* cls);

}  // namespace bsp::pool_pool01_cc13

#endif
