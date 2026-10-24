#pragma once
// Packet cc13-pool13 (segment 30: air operations and runways): nine bodies that
// had no credited record (metadata_only or credit-withdrawn rows). The cc9
// gameplay hosts (src/air_operations.cpp, game_hosts_units.cpp,
// game_hosts_hud.cpp) already model several of them by behaviour inside larger
// routines; those stay as they are. These are the standalone bodies.
//
// Objects are reached through byte offsets in their Win32 layout. Every call
// the image makes goes through Pool13Host, named for its target address, at the
// same point and with the same arguments. Build-tested only, not wired into a
// game host, not ABI-compatible replacements (the image passes this in ECX).
// Names are hypotheses.

#include <cstddef>
#include <cstdint>

namespace bsp::pool_pool13_cc13 {

static_assert(sizeof(void*) == 4, "these objects are modelled in their Win32 layout");

using Object = std::uint8_t*;

template <class T>
inline T& at(Object object, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(object + offset);
}

// A 14h-byte Lua object handle (opaque; 00B67700 destructs it).
struct LuaObject {
    std::uint8_t storage[0x14];
};

struct Pool13Host {
    virtual ~Pool13Host() = default;

    // 006BF230 BSP_AirOps_StockAvailable, __thiscall(block, out pair, class), RET 8,
    // EAX = out ({available, total}).
    virtual std::int32_t* air_ops_stock_available_006bf230(Object block, std::int32_t* out, Object vehicle_class) = 0;
    // 006952A0 / 00694A60 __fastcall(ECX = observed, EDX = observer link).
    virtual void observer_unregister_pair_006952a0(Object observed, Object link) = 0;
    virtual void observer_register_pair_00694a60(Object observed, Object link) = 0;
    // 0042E740 BSP_GameTuning_GetSingleton, EAX = the tuning block.
    virtual Object game_tuning_0042e740() = 0;
    // 0085F990 __fastcall(ECX = p0, EDX = p1; stack r0 g0 b0 a0 r1 g1 b1 a1), RET 20h.
    virtual void debug_draw_line_0085f990(const float* p0, const float* p1, float r0, float g0, float b0, float a0,
                                          float r1, float g1, float b1, float a1) = 0;
    // 00960230 BSP_VehicleClass_ReadLuaFields, __thiscall(class, row), RET 4.
    virtual void vehicle_class_read_lua_fields_00960230(Object vehicle_class, LuaObject* row) = 0;
    // 00B67800 __thiscall(source, out, name), RET 8, EAX = out.
    virtual LuaObject* lua_get_by_name_00b67800(LuaObject* source, LuaObject* out, const char* name) = 0;
    virtual double lua_get_number_00b66270(LuaObject* object) = 0;      // ECX = object, ST0
    virtual void lua_object_destruct_00b67700(LuaObject* object) = 0;   // ECX = object
    // 00953CC0 BSP_UnitGameObject_TickAdvance, __thiscall(sub-object, float dt), RET 4.
    virtual void unit_game_object_tick_advance_00953cc0(Object self, float dt) = 0;
    // [[unit]+1A8h](), __thiscall with no stack argument.
    virtual void unit_vslot_1a8(Object unit) = 0;
    // 006CDC70 BSP_AirOps_Update, __thiscall(block, float dt), RET 4.
    virtual void air_ops_update_006cdc70(Object block, float dt) = 0;
};

// 006BF330: available stock of one plane class in an air-ops block.
std::int32_t air_ops_free_stock_for_class_006bf330(Pool13Host& host, Object block, Object vehicle_class);
// 006BF620: the block may send planes.
bool air_ops_block_is_ready_to_send_planes_006bf620(Object block) noexcept;
// 006C2400: debug lines for the runway takeoff/touchdown distances.
void air_ops_holder_debug_draw_runway_distances_006c2400(Pool13Host& host, Object holder);
// 006C4F70: re-point an air-ops slot's observed target.
void air_ops_slot_set_target_006c4f70(Pool13Host& host, Object slot, Object target);
// 006D0B80: runway class Lua fields.
void runway_class_read_lua_fields_006d0b80(Pool13Host& host, Object runway_class, LuaObject* row);
// 006D1EF0: unit alive-and-active test (vtable slot +124h).
std::uint32_t ship_unit_is_alive_and_active_006d1ef0(Object unit) noexcept;
// 006D1F20: empty float-argument slot.
void unit_tick_no_op_float_006d1f20(Object unit, float value) noexcept;
// 006D1FA0: "shows on the minimap" slot, always true.
bool unit_shows_on_minimap_true_006d1fa0(Object unit) noexcept;
// 006D2510: airfield fixed-step advance (entered on the unit+310h sub-object).
void airfield_tick_advance_006d2510(Pool13Host& host, Object self, float dt);

}  // namespace bsp::pool_pool13_cc13
