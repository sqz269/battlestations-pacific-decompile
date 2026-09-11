#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/unit_motion.hpp" // unit_step_towards_0042ac60
#include "bsp/unit_orders.hpp" // kUnitOffOrderSlots, kUnitOrderSlotStride

// The receive side of session message 8Ch, MT_SHIP_SYNC, and the order ring it
// feeds. Every offset and constant below comes from the listing of the address
// named on its line; see docs/UNIT_STATE_MESSAGE.md for the pipeline, the ABIs
// and the uncertainties. Descriptive names are hypotheses, not recovered
// symbols. These are semantic interfaces for MSVC Win32, not drop-in binary
// replacements.

namespace bsp {

// ---------------------------------------------------------------------------
// The message class: 4Ch bytes, vtable 00D02D94, built by 00813CA0.
// ---------------------------------------------------------------------------

// Names from the table at 00E0AB68 indexed by the type byte at msg+10h.
inline constexpr int kSessionMessageShipSync = 0x8C;      // 00D02158 "MT_SHIP_SYNC"
inline constexpr int kSessionMessageShipGunsSync = 0x8D;  // 00D02144 "MT_SHIP_GUNS_SYNC"
inline constexpr int kSessionMessageShipCreate = 0x8A;    // 00D02180 "MT_SHIP_CREATE"
inline constexpr int kSessionMessageMothershipCreate = 0x8B; // 00D02168

inline constexpr std::size_t kUnitStateMessageSize = 0x4C; // 00769380, PUSH 4Ch

// The quantized field widths and ranges, identical in 00813DC0 and 00813EF0.
inline constexpr float kUnitStateMessageHeadingRange = 3.14159265f; // 00D7A264
inline constexpr int kUnitStateMessageHeadingBits = 16;             // 00813F3D
inline constexpr float kUnitStateMessageYawRateRange = 1.5f;        // 00CE380C
inline constexpr int kUnitStateMessageYawRateBits = 10;             // 00813F58
inline constexpr float kUnitStateMessagePositionRange = 24000.0f;   // 00D02F64
inline constexpr int kUnitStateMessagePositionBits = 22;            // 00813F22
inline constexpr float kUnitStateMessageVelocityRange = 50.0f;      // 00D09290
inline constexpr int kUnitStateMessageVelocityBits = 12;            // 00813F73
inline constexpr float kUnitStateMessageOrderRange = 2.0f;          // 00CE3958
inline constexpr int kUnitStateMessageOrderBits = 10;               // 00813FA9

// Twenty ticks a second. 00D0DE84, the multiplier the apply uses on the tick age.
inline constexpr float kUnitStateMessageTickSeconds = 0.05f;

// The gate at 00816CCC: the apply runs only when unit+BCCh <= this.
inline constexpr float kUnitStateMessageApplyGate = 1.0f; // 00D7A24C

// The decoded payload. The Y components are built by 00813CA0 but never
// serialized (00813DC0 sends X and Z only), so after a round trip they hold the
// constructor's zero; they are kept here because the builder writes them.
struct UnitStateMessage {
    int send_tick{0};           // +0Ch, the tick the sender stamped
    std::uint8_t type{0};       // +10h
    int entity_id{0};           // +14h
    std::uint16_t sender_id{0}; // +18h
    float heading{0.0f};        // +20h, unit vtable[50h]
    float yaw_rate{0.0f};       // +24h, 0092D700(scratch)+4
    float position_x{0.0f};     // +28h, unit+6A4h
    float position_y{0.0f};     // +2Ch, unit+6A8h, not transmitted
    float position_z{0.0f};     // +30h, unit+6ACh
    float velocity_x{0.0f};     // +34h, 0092D6A0(scratch)+0
    float velocity_y{0.0f};     // +38h, not transmitted
    float velocity_z{0.0f};     // +3Ch, 0092D6A0(scratch)+8
    float ordered_throttle{0.0f}; // +40h, unit+980h at 00813D4B
    float ordered_rudder{0.0f};   // +44h, unit+984h at 00813D54
    std::uint8_t flooded{0};      // +48h, unit+110Ch below 00CE3800
    std::uint8_t collision_group{0}; // +49h, [unit+1018h]+64h
    std::uint8_t class_byte{0};      // +4Ah, unit+118Ch when the class query is 2
};

// ---------------------------------------------------------------------------
// The order ring embedded in the unit at +838h. 168h bytes, ten 20h-byte slots
// followed by two cursors, the live pair, a kind byte, two slew limits and the
// confirmed triple. unit_orders.hpp carries a narrower eight-slot projection of
// the same object under the name UnitOrderQueue; the native slot count is ten.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kUnitOrderRingSlotCount = 10; // CMP EAX,0Ah at 00812FDE
inline constexpr std::size_t kUnitOffOrderRing = kUnitOffOrderSlots; // 838h

// Ring-relative offsets; add 838h for the unit-relative address.
inline constexpr std::size_t kUnitOrderRingReadCursor = 0x140;   // unit+978h
inline constexpr std::size_t kUnitOrderRingWriteCursor = 0x144;  // unit+97Ch
inline constexpr std::size_t kUnitOrderRingCurrentA = 0x148;     // unit+980h
inline constexpr std::size_t kUnitOrderRingCurrentB = 0x14C;     // unit+984h
inline constexpr std::size_t kUnitOrderRingCurrentKind = 0x150;  // unit+988h
inline constexpr std::size_t kUnitOrderRingSlewA = 0x154;        // unit+98Ch
inline constexpr std::size_t kUnitOrderRingSlewB = 0x158;        // unit+990h
inline constexpr std::size_t kUnitOrderRingConfirmedA = 0x15C;   // unit+994h
inline constexpr std::size_t kUnitOrderRingConfirmedB = 0x160;   // unit+998h
inline constexpr std::size_t kUnitOrderRingConfirmedKind = 0x164; // unit+99Ch

// Constructor constants, all from 00812D40.
inline constexpr float kUnitOrderRingBoundHigh = 2.0f;  // 00CE3958
inline constexpr float kUnitOrderRingBoundLow = -2.0f;  // 00CE7D7C
inline constexpr float kUnitOrderRingSlewADefault = 8.0f; // 00CE3918, 00812F84
inline constexpr float kUnitOrderRingSlewBDefault = 2.0f; // 00CE3958, 00812F8C

// DAT_00E0B51C, written with 4 by the ring constructor at 00812F50. It is both
// the initial write cursor and the cap on the back-fill depth in 00812FA0.
inline constexpr int kUnitOrderRingSyncLagTicks = 4;

// One 20h-byte slot, the record 00815440 builds.
struct UnitOrderRingSlot {
    float param_a{0.0f};  // +00h
    float param_b{0.0f};  // +04h
    // +08h. 1 means the slot is an extrapolated copy of its predecessor, 0 that
    // it carries an order the game actually issued or received.
    bool predicted{true};
    float param_a_high{kUnitOrderRingBoundHigh}; // +0Ch
    float param_a_low{kUnitOrderRingBoundLow};   // +10h
    float param_b_high{kUnitOrderRingBoundHigh}; // +14h
    float param_b_low{kUnitOrderRingBoundLow};   // +18h
    std::uint8_t kind{0};                        // +1Ch
};

struct UnitOrderRing {
    UnitOrderRingSlot slot[kUnitOrderRingSlotCount]{};
    int read_cursor{0};                      // +140h
    int write_cursor{kUnitOrderRingSyncLagTicks}; // +144h
    float current_param_a{0.0f};             // +148h, unit+980h
    float current_param_b{0.0f};             // +14Ch, unit+984h
    std::uint8_t current_kind{0};            // +150h
    float slew_a{kUnitOrderRingSlewADefault}; // +154h
    float slew_b{kUnitOrderRingSlewBDefault}; // +158h
    float confirmed_param_a{0.0f};           // +15Ch, unit+994h
    float confirmed_param_b{0.0f};           // +160h, unit+998h
    std::uint8_t confirmed_kind{0};          // +164h
};

// 00812D40, void __thiscall(ring), RET 0. Ten unrolled slot initializers, then
// the cursors, the live pair and the two slew limits. +150h and +15Ch..+164h are
// left alone; in the game they are already zero from the unit's bulk clear, and
// this projection's defaults match that.
void construct_unit_order_ring_00812d40(UnitOrderRing& ring) noexcept;

// 00812FA0, void __thiscall(ring, float a, float b, uint8 kind, int age_ticks),
// RET 10h. Back-dates one order over the last min(age_ticks, sync_lag) slots up
// to and including the write cursor, marking each authoritative. Touches no
// cursor, no bound and neither live field.
void backfill_unit_order_ring_00812fa0(UnitOrderRing& ring, float param_a, float param_b,
                                       std::uint8_t kind, int age_ticks,
                                       int sync_lag_ticks = kUnitOrderRingSyncLagTicks) noexcept;

// 0080D9B0 and 0080DA00, both void __thiscall(ring, float), RET 4. Each fills
// the pending span from the read cursor to the write cursor inclusive and then
// writes the live field directly, bypassing the slew limit. 00825F20 inlines
// both at 008266CE and 00826710.
void set_unit_order_ring_param_a_0080d9b0(UnitOrderRing& ring, float value) noexcept;
void set_unit_order_ring_param_b_0080da00(UnitOrderRing& ring, float value) noexcept;

// The clamp shape 00813020 uses on a slot before stepping toward it:
//   if (low <= v) { if (high < v) v = high; } else v = low;
// An unordered comparison against the low bound yields the low bound, so a NaN
// slot value becomes the low bound rather than propagating.
float clamp_unit_order_ring_slot_00813020(float value, float low, float high) noexcept;

// 00813020, void __thiscall(ring, float dt), called once a frame from 00825F20.
// This is what writes unit+980h and unit+984h: it steps the live pair toward the
// clamped slot under the read cursor at slew * dt through 0042AC60, publishes
// the confirmed triple when that slot is authoritative, copies the write slot
// forward as a predicted slot, and advances the cursors. A networked client
// (session mode 2) advances the read cursor by one and so keeps the write cursor
// ahead; otherwise the read cursor jumps to the old write cursor.
inline constexpr int kUnitOrderRingClientSessionMode = 2; // game+1FE4h, 00813198
void tick_unit_order_ring_00813020(UnitOrderRing& ring, float dt, int session_mode) noexcept;

// ---------------------------------------------------------------------------
// The apply, 00816C80, unit primary vtable slot 18Ch.
// ---------------------------------------------------------------------------

// The 24h-byte stack record 00816C80 builds and 0092F2E0 copies into the
// controller at +40h..+60h. The two zero floats are written by the prologue and
// never overwritten; the trailing byte is set to 1 immediately before the call.
struct UnitNetworkMotionState {
    float velocity_x{0.0f}; // +00h
    float velocity_z{0.0f}; // +04h
    float position_x{0.0f}; // +08h
    float position_z{0.0f}; // +0Ch
    float heading{0.0f};    // +10h
    float yaw_rate{0.0f};   // +14h
    float reserved_18{0.0f}; // +18h, 00816CF9
    float reserved_1c{0.0f}; // +1Ch, 00816CF3
    std::uint8_t valid{1};   // +20h, 00816DB6
};

// 00816CE1..00816DBB. Dead reckoning: the sender's state advanced by the age of
// the message. The heading goes through BSP_Math_AddWrappedAngle, supplied by
// the host because its wrap constants belong to another packet.
UnitNetworkMotionState dead_reckon_ship_sync_00816c80(const UnitStateMessage& message,
                                                      int age_ticks,
                                                      float wrapped_heading) noexcept;

// The unresolved native calls of 00816C80, one method per call site.
struct ShipSyncApplyHost {
    virtual ~ShipSyncApplyHost() = default;
    // 00816C97, msg->vtable[0Ch](8Dh). True only for MT_SHIP_GUNS_SYNC.
    virtual bool message_is_category_0075a660(int category) = 0;
    // 00816CA0, this->00813950(msg): the per-turret gun sync fan-out.
    virtual void apply_gun_sync_00813950() = 0;
    // 00816CB1/00816CC1, the byte at [unit+1018h]+64h.
    virtual std::uint8_t controller_collision_group() = 0;
    // 00816CC7, 0092BD70(ECX = unit+1018h, byte): stores the byte and rebuilds
    // the physics collision filter.
    virtual void set_controller_collision_group_0092bd70(std::uint8_t group) = 0;
    // 00816CCC, the float at unit+BCCh compared against 1.0f.
    virtual float apply_gate_value() = 0;
    // 00816D97, BSP_Math_AddWrappedAngle(heading, yaw_rate * seconds).
    virtual float add_wrapped_angle_00438aa0(float angle, float delta) = 0;
    // 00816DBB, 0092F2E0(ECX = unit+1018h, &state).
    virtual void apply_network_motion_state_0092f2e0(const UnitNetworkMotionState& state) = 0;
    // 00816DC4, 00927F30(unit, role): unit[1ACh + role*4] == game+18ECh, and
    // that slot must be 7 or below.
    virtual bool is_local_player_role_00927f30(int role) = 0;
    // 00816DF7, unit+10D0h = 0.0f.
    virtual void clear_unit_10d0() = 0;
    // 00816E0D, [unit+170h]->vtable[0]().
    virtual int unit_class_query() = 0;
    // 00816E17, unit+118Ch = the message's class byte.
    virtual void set_unit_class_byte(std::uint8_t value) = 0;
};

// 00816C80, void __thiscall(unit, message*), RET 4. current_tick is
// DAT_00F876B0; the dispatcher 00780670 has already proven
// message.send_tick <= current_tick, so the age is not negative.
inline constexpr int kUnitStateMessageLocalRole = 1; // 00816DC0, PUSH 1
void apply_ship_sync_message_00816c80(ShipSyncApplyHost& host, UnitOrderRing& ring,
                                      const UnitStateMessage& message, int current_tick);

} // namespace bsp
