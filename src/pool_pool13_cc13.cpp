// Packet cc13-pool13: see include/bsp/pool_pool13_cc13.hpp. Build-tested only;
// nothing here is wired into a game host.
#include "bsp/pool_pool13_cc13.hpp"

namespace bsp::pool_pool13_cc13 {
namespace {

// The three function-local static colours of 006C2400 ({r, g, b, a}), each
// filled from [00D7A24C] (1.0f) and 0.0f under its own guard bit (00E19914,
// 00E19928, 00E1993C) on first entry; the image keeps them at 00E19904,
// 00E19918 and 00E1992C. Modelled as constants.
constexpr float kWhite[4] = {1.0f, 1.0f, 1.0f, 1.0f};
constexpr float kRed[4] = {1.0f, 0.0f, 0.0f, 1.0f};
constexpr float kGreen[4] = {0.0f, 1.0f, 0.0f, 1.0f};

struct Point {
    float x, y, z;
};

// One 0085F990 call: both endpoints built in stack slots, the colour passed
// twice (start and end colour).
void line(Pool13Host& host, const Point& p0, const Point& p1, const float (&colour)[4]) {
    host.debug_draw_line_0085f990(&p0.x, &p1.x, colour[0], colour[1], colour[2], colour[3], colour[0], colour[1],
                                  colour[2], colour[3]);
}

}  // namespace

// ---------------------------------------------------------------------------
// 006BF330 BSP_AirOps_FreeStockForClass. 9 instructions.
// ABI: __thiscall(ECX = block; stack class), RET 4, EAX = available.
// Body: 006BF230(ECX = block, &pair (8 bytes on the stack), class), then
// EAX = [returned pointer] (the pair's first word, the available count).
// No EH frame. Callers 006CCDA0, 006CD350.
// ---------------------------------------------------------------------------
std::int32_t air_ops_free_stock_for_class_006bf330(Pool13Host& host, Object block, Object vehicle_class) {
    std::int32_t pair[2];
    return *host.air_ops_stock_available_006bf230(block, pair, vehicle_class);
}

// ---------------------------------------------------------------------------
// 006BF620 BSP_AirOpsBlock_IsReadyToSendPlanes. 15 instructions.
// ABI: __thiscall(ECX = block), RET, AL = result.
// Ready when bytes +1Ch and +1Dh are clear, the owner [+7Ch] is set, the
// owner's byte +5Dh is clear and dword +38h is 0, tested in that order.
// Uncertainty: only AL is defined; on the true path the upper bytes of EAX
// hold the owner pointer's, on the false paths whatever EAX held (callers
// 00895D20 and 0089E3C0 are Lua bindings that read AL).
// src/air_operations.cpp models the same test inline for the live host.
// ---------------------------------------------------------------------------
bool air_ops_block_is_ready_to_send_planes_006bf620(Object block) noexcept {
    if (at<std::uint8_t>(block, 0x1C) != 0) return false;
    if (at<std::uint8_t>(block, 0x1D) != 0) return false;
    Object owner = at<Object>(block, 0x7C);
    if (owner == nullptr) return false;
    if (at<std::uint8_t>(owner, 0x5D) != 0) return false;
    return at<std::uint32_t>(block, 0x38) == 0;
}

// ---------------------------------------------------------------------------
// 006C2400 BSP_AirOpsHolder_DebugDrawRunwayDistances. 1610 instructions.
// ABI: __fastcall(ECX = holder), RET (callers 009B35E0, 009CFA10 tail JMP).
// No EH frame. The holder carries a 4x4 world matrix at +8h (right +8h,
// forward +28h, position +38h; the up row +18h is not read), the runway width
// at +B0h and length at +B4h.
// Evidence: the whole listing run through a symbolic x87/stack executor that
// decodes every register-form x87 arithmetic instruction from its bytes (the
// DC/DE direction is not taken from the listing text), executing both arms of
// the one branch; the 12 trailing calls are identical on both arms. The
// pseudocode drops every endpoint.
// Order: colours (static guards); half_length = 0.5 x [+B4h] (stored),
// half_width = [+B0h] x 0.5 (stored, double constant 00D7A280 = 0.5);
// tuning = 0042E740() twice, TakeoffDist +508h and TouchDownDist +50Ch read
// once each. When [+B4h] > TakeoffDist + TouchDownDist (FCOMIP/JBE: false on
// equal or unordered) five white lines are drawn: the runway centre line
// between the two distance marks and the outline of that span. Then seven
// green lines (the runway outline and the takeoff mark) and five red lines
// (the touchdown mark, its outline). Each endpoint coordinate is the binary's
// exact operation tree; every x87 result is stored to a float slot before
// reuse, so every operation is written as a float operation.
// Precision: the main thread runs with x87 precision control at 24 bits, so
// each x87 operation rounds to a float mantissa; float arithmetic here matches
// that except for the x87 extended exponent range (overflow/underflow of
// intermediates). The one unstored intermediate is the comparison's sum.
// Uncertainty: the colour statics are modelled as constants (their first-call
// initialisation and the guard bits are not written).
// ---------------------------------------------------------------------------
void air_ops_holder_debug_draw_runway_distances_006c2400(Pool13Host& host, Object holder) {
    auto h = [holder](std::size_t offset) { return at<float>(holder, offset); };
    const float length = h(0xB4);
    const float half_width = h(0xB0) * 0.5f;
    const float half_length = 0.5f * length;
    const float takeoff_distance = at<float>(host.game_tuning_0042e740(), 0x508);
    const float touchdown_distance = at<float>(host.game_tuning_0042e740(), 0x50C);
    if (length > touchdown_distance + takeoff_distance) {
        // 006C266D
        line(host, {(h(0x28) * (half_length - takeoff_distance)) + h(0x38),
                    h(0x3C) + (h(0x2C) * (half_length - takeoff_distance)),
                    h(0x40) + ((half_length - takeoff_distance) * h(0x30))},
             {h(0x38) - ((half_length - touchdown_distance) * h(0x28)),
              h(0x3C) - (h(0x2C) * (half_length - touchdown_distance)),
              h(0x40) - ((half_length - touchdown_distance) * h(0x30))}, kWhite);
        // 006C27E9
        line(host, {(h(0x38) + ((half_length - takeoff_distance) * h(0x28))) + (h(0x8) * half_width),
                    (h(0x3C) + (h(0x2C) * (half_length - takeoff_distance))) + (h(0xC) * half_width),
                    (h(0x40) + ((half_length - takeoff_distance) * h(0x30))) + (half_width * h(0x10))},
             {(h(0x38) - ((half_length - touchdown_distance) * h(0x28))) + (h(0x8) * half_width),
              (h(0x3C) - (h(0x2C) * (half_length - touchdown_distance))) + (h(0xC) * half_width),
              (h(0x40) - ((half_length - touchdown_distance) * h(0x30))) + (half_width * h(0x10))}, kWhite);
        // 006C2969
        line(host, {((h(0x28) * (half_length - takeoff_distance)) + h(0x38)) - (h(0x8) * half_width),
                    (h(0x3C) + (h(0x2C) * (half_length - takeoff_distance))) - (h(0xC) * half_width),
                    (h(0x40) + ((half_length - takeoff_distance) * h(0x30))) - (half_width * h(0x10))},
             {(h(0x38) - ((half_length - touchdown_distance) * h(0x28))) - (h(0x8) * half_width),
              (h(0x3C) - (h(0x2C) * (half_length - touchdown_distance))) - (h(0xC) * half_width),
              (h(0x40) - ((half_length - touchdown_distance) * h(0x30))) - (half_width * h(0x10))}, kWhite);
        // 006C2AE1
        line(host, {(((half_length - takeoff_distance) * h(0x28)) + h(0x38)) + (h(0x8) * half_width),
                    (h(0x3C) + (h(0x2C) * (half_length - takeoff_distance))) + (h(0xC) * half_width),
                    (h(0x40) + ((half_length - takeoff_distance) * h(0x30))) + (half_width * h(0x10))},
             {(((half_length - takeoff_distance) * h(0x28)) + h(0x38)) - (h(0x8) * half_width),
              (h(0x3C) + (h(0x2C) * (half_length - takeoff_distance))) - (h(0xC) * half_width),
              (h(0x40) + ((half_length - takeoff_distance) * h(0x30))) - (half_width * h(0x10))}, kWhite);
        // 006C2C59
        line(host, {(h(0x38) - ((half_length - touchdown_distance) * h(0x28))) + (h(0x8) * half_width),
                    (h(0x3C) - (h(0x2C) * (half_length - touchdown_distance))) + (h(0xC) * half_width),
                    (h(0x40) - ((half_length - touchdown_distance) * h(0x30))) + (half_width * h(0x10))},
             {(h(0x38) - ((half_length - touchdown_distance) * h(0x28))) - (h(0x8) * half_width),
              (h(0x3C) - (h(0x2C) * (half_length - touchdown_distance))) - (h(0xC) * half_width),
              (h(0x40) - ((half_length - touchdown_distance) * h(0x30))) - (half_width * h(0x10))}, kWhite);
    }
    // 006C2D60
    line(host, {h(0x38) + (h(0x28) * half_length),
                h(0x3C) + (h(0x2C) * half_length),
                h(0x40) + (half_length * h(0x30))},
         {(h(0x8) * half_width) + (h(0x38) + ((half_length - half_width) * h(0x28))),
          (h(0x3C) + (h(0x2C) * (half_length - half_width))) + (h(0xC) * half_width),
          (h(0x40) + ((half_length - half_width) * h(0x30))) + (h(0x10) * half_width)}, kGreen);
    // 006C2E61
    line(host, {h(0x38) + (h(0x28) * half_length),
                h(0x3C) + (h(0x2C) * half_length),
                h(0x40) + (half_length * h(0x30))},
         {(h(0x38) + ((half_length - half_width) * h(0x28))) - (h(0x8) * half_width),
          (h(0x3C) + (h(0x2C) * (half_length - half_width))) - (h(0xC) * half_width),
          (h(0x40) + ((half_length - half_width) * h(0x30))) - (half_width * h(0x10))}, kGreen);
    // 006C2F26
    line(host, {h(0x38) + (h(0x28) * half_length),
                h(0x3C) + (h(0x2C) * half_length),
                h(0x40) + (half_length * h(0x30))},
         {h(0x38) + ((half_length - takeoff_distance) * h(0x28)),
          h(0x3C) + (h(0x2C) * (half_length - takeoff_distance)),
          h(0x40) + ((half_length - takeoff_distance) * h(0x30))}, kGreen);
    // 006C308B
    line(host, {(h(0x38) + (h(0x28) * half_length)) + (h(0x8) * half_width),
                (h(0x3C) + (h(0x2C) * half_length)) + (h(0xC) * half_width),
                (h(0x40) + (half_length * h(0x30))) + (half_width * h(0x10))},
         {(h(0x38) + (h(0x28) * (half_length - takeoff_distance))) + (h(0x8) * half_width),
          (h(0x3C) + (h(0x2C) * (half_length - takeoff_distance))) + (h(0xC) * half_width),
          (h(0x40) + ((half_length - takeoff_distance) * h(0x30))) + (half_width * h(0x10))}, kGreen);
    // 006C31F0
    line(host, {((h(0x28) * half_length) + h(0x38)) - (h(0x8) * half_width),
                (h(0x3C) + (h(0x2C) * half_length)) - (h(0xC) * half_width),
                (h(0x40) + (half_length * h(0x30))) - (half_width * h(0x10))},
         {(h(0x38) + (h(0x28) * (half_length - takeoff_distance))) - (h(0x8) * half_width),
          (h(0x3C) + (h(0x2C) * (half_length - takeoff_distance))) - (h(0xC) * half_width),
          (h(0x40) + ((half_length - takeoff_distance) * h(0x30))) - (half_width * h(0x10))}, kGreen);
    // 006C3347
    line(host, {((h(0x28) * half_length) + h(0x38)) + (h(0x8) * half_width),
                (h(0x3C) + (h(0x2C) * half_length)) + (h(0xC) * half_width),
                (h(0x40) + (half_length * h(0x30))) + (half_width * h(0x10))},
         {((h(0x28) * half_length) + h(0x38)) - (h(0x8) * half_width),
          (h(0x3C) + (h(0x2C) * half_length)) - (h(0xC) * half_width),
          (h(0x40) + (h(0x30) * half_length)) - (half_width * h(0x10))}, kGreen);
    // 006C34A8
    line(host, {(((half_length - takeoff_distance) * h(0x28)) + h(0x38)) + (h(0x8) * half_width),
                (h(0x3C) + (h(0x2C) * (half_length - takeoff_distance))) + (h(0xC) * half_width),
                (h(0x40) + ((half_length - takeoff_distance) * h(0x30))) + (half_width * h(0x10))},
         {(((half_length - takeoff_distance) * h(0x28)) + h(0x38)) - (h(0x8) * half_width),
          (h(0x3C) + (h(0x2C) * (half_length - takeoff_distance))) - (h(0xC) * half_width),
          (h(0x40) + ((half_length - takeoff_distance) * h(0x30))) - (half_width * h(0x10))}, kGreen);
    // 006C357A
    line(host, {h(0x38) - (h(0x28) * half_length),
                h(0x3C) - (h(0x2C) * half_length),
                h(0x40) - (half_length * h(0x30))},
         {h(0x38) - (h(0x28) * (half_length - touchdown_distance)),
          h(0x3C) - (h(0x2C) * (half_length - touchdown_distance)),
          h(0x40) - ((half_length - touchdown_distance) * h(0x30))}, kRed);
    // 006C36DF
    line(host, {(h(0x38) - (h(0x28) * half_length)) + (h(0x8) * half_width),
                (h(0x3C) - (h(0x2C) * half_length)) + (h(0xC) * half_width),
                (h(0x40) - (half_length * h(0x30))) + (half_width * h(0x10))},
         {(h(0x38) - (h(0x28) * (half_length - touchdown_distance))) + (h(0x8) * half_width),
          (h(0x3C) - (h(0x2C) * (half_length - touchdown_distance))) + (h(0xC) * half_width),
          (h(0x40) - ((half_length - touchdown_distance) * h(0x30))) + (half_width * h(0x10))}, kRed);
    // 006C3840
    line(host, {(h(0x38) - (h(0x28) * half_length)) - (h(0x8) * half_width),
                (h(0x3C) - (h(0x2C) * half_length)) - (h(0xC) * half_width),
                (h(0x40) - (half_length * h(0x30))) - (half_width * h(0x10))},
         {(h(0x38) - ((half_length - touchdown_distance) * h(0x28))) - (h(0x8) * half_width),
          (h(0x3C) - (h(0x2C) * (half_length - touchdown_distance))) - (h(0xC) * half_width),
          (h(0x40) - ((half_length - touchdown_distance) * h(0x30))) - (half_width * h(0x10))}, kRed);
    // 006C3997
    line(host, {(h(0x38) - (h(0x28) * half_length)) + (h(0x8) * half_width),
                (h(0x3C) - (h(0x2C) * half_length)) + (h(0xC) * half_width),
                (h(0x40) - (half_length * h(0x30))) + (half_width * h(0x10))},
         {(h(0x38) - (h(0x28) * half_length)) - (h(0x8) * half_width),
          (h(0x3C) - (h(0x2C) * half_length)) - (h(0xC) * half_width),
          (h(0x40) - (h(0x30) * half_length)) - (half_width * h(0x10))}, kRed);
    // 006C3AF8
    line(host, {(h(0x38) - ((half_length - touchdown_distance) * h(0x28))) + (h(0x8) * half_width),
                (h(0x3C) - (h(0x2C) * (half_length - touchdown_distance))) + (h(0xC) * half_width),
                (h(0x40) - ((half_length - touchdown_distance) * h(0x30))) + (half_width * h(0x10))},
         {(h(0x38) - ((half_length - touchdown_distance) * h(0x28))) - (h(0x8) * half_width),
          (h(0x3C) - (h(0x2C) * (half_length - touchdown_distance))) - (h(0xC) * half_width),
          (h(0x40) - ((half_length - touchdown_distance) * h(0x30))) - (half_width * h(0x10))}, kRed);
}

// ---------------------------------------------------------------------------
// 006C4F70 BSP_AirOpsSlot_SetTarget. 20 instructions.
// ABI: __thiscall(ECX = slot; stack target), RET 4. No EH frame.
// The slot's observer link is at +38h and its target at +4Ch (link+14h).
// When the target changes: 006952A0(ECX = old, EDX = link) if there was one,
// store the new target, 00694A60(ECX = new, EDX = link) if it is set. An
// unchanged target does nothing. src/air_operations.cpp keeps slot+4Ch for the
// live host without the observer registration.
// ---------------------------------------------------------------------------
void air_ops_slot_set_target_006c4f70(Pool13Host& host, Object slot, Object target) {
    Object link = slot + 0x38;
    Object current = at<Object>(link, 0x14);
    if (current == target) return;
    if (current != nullptr) host.observer_unregister_pair_006952a0(current, link);
    at<Object>(link, 0x14) = target;
    if (target != nullptr) host.observer_register_pair_00694a60(target, link);
}

// ---------------------------------------------------------------------------
// 006D0B80 BSP_RunwayClass_ReadLuaFields. 42 instructions.
// ABI: __thiscall(ECX = class; stack row), RET 4. No direct caller (a vtable
// slot of the runway class).
// Order: 00960230(class, row) (vehicle class fields); +138h = "RunwayWidth"
// (00CF8AE8); +13Ch = "RunwayLength" (00CF8AD8); each FSTP float of the
// 00B66270 ST0 result, the temporary destructed after each.
// EH: state 0 over the first temporary, 1 over the second, -1 between.
// Unwinding is not modelled. src/vehicle_class_fields.cpp lists the runway
// class among its table-driven readers for the live host.
// ---------------------------------------------------------------------------
void runway_class_read_lua_fields_006d0b80(Pool13Host& host, Object runway_class, LuaObject* row) {
    host.vehicle_class_read_lua_fields_00960230(runway_class, row);
    LuaObject width;    // [ESP+8h]
    at<float>(runway_class, 0x138) =
        static_cast<float>(host.lua_get_number_00b66270(host.lua_get_by_name_00b67800(row, &width, "RunwayWidth")));
    host.lua_object_destruct_00b67700(&width);
    LuaObject length;   // [ESP+1Ch]
    at<float>(runway_class, 0x13C) =
        static_cast<float>(host.lua_get_number_00b66270(host.lua_get_by_name_00b67800(row, &length, "RunwayLength")));
    host.lua_object_destruct_00b67700(&length);
}

// ---------------------------------------------------------------------------
// 006D1EF0 ShipUnit_IsAliveAndActive_vt124. 12 instructions.
// ABI: __thiscall(ECX = unit), RET, EAX = 1 or 0 (the whole register).
// 1 when byte +5Ch is set and +5Dh, +60h, +5Eh are clear, tested in that order.
// game_hosts_hud.cpp reproduces the same test for the live minimap.
// ---------------------------------------------------------------------------
std::uint32_t ship_unit_is_alive_and_active_006d1ef0(Object unit) noexcept {
    if (at<std::uint8_t>(unit, 0x5C) == 0) return 0;
    if (at<std::uint8_t>(unit, 0x5D) != 0) return 0;
    if (at<std::uint8_t>(unit, 0x60) != 0) return 0;
    if (at<std::uint8_t>(unit, 0x5E) != 0) return 0;
    return 1;
}

// ---------------------------------------------------------------------------
// 006D1F20 BSP_UnitTick_NoOpFloat. 1 instruction: RET 4.
// ABI: __thiscall(ECX = unit; stack one dword, a float by its slot's callers),
// RET 4. Does nothing.
// ---------------------------------------------------------------------------
void unit_tick_no_op_float_006d1f20(Object, float) noexcept {}

// ---------------------------------------------------------------------------
// 006D1FA0 BSP_Unit_ShowsOnMinimap_True. 2 instructions: MOV AL,1; RET.
// ABI: __thiscall(ECX = unit), RET, AL = 1 (the upper bytes of EAX are not
// written). include/bsp/hud_minimap.hpp records the same value as data.
// ---------------------------------------------------------------------------
bool unit_shows_on_minimap_true_006d1fa0(Object) noexcept { return true; }

// ---------------------------------------------------------------------------
// 006D2510 BSP_AirField_TickAdvance. 19 instructions.
// ABI: __thiscall(ECX = self = unit+310h, the fixed-step sub-object; stack
// float dt), RET 4. No EH frame.
// Order: 00953CC0(self, dt); [[unit]+1A8h](ECX = unit = self-310h); when
// byte [unit+5Dh] is clear, 006CDC70(ECX = self+41Ch = unit+72Ch (the air-ops
// block), dt). dt is passed on through FLD/FSTP float (a bit copy except
// that an SNaN would be quieted).
// HOST OVERLAP: game_hosts_units.cpp runs this sequence today for airfields
// (its motion dispatch entry 006D2510); this body is not switched in.
// ---------------------------------------------------------------------------
void airfield_tick_advance_006d2510(Pool13Host& host, Object self, float dt) {
    host.unit_game_object_tick_advance_00953cc0(self, dt);
    Object unit = self - 0x310;
    host.unit_vslot_1a8(unit);
    if (at<std::uint8_t>(unit, 0x5D) == 0) {
        host.air_ops_update_006cdc70(self + 0x41C, dt);
    }
}

}  // namespace bsp::pool_pool13_cc13
