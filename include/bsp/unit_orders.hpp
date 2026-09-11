#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/unit_forces.hpp" // kUnitOffHelmsmanThrust, kUnitOffHelmsmanToTurn,
                               // kUnitOffTurnAssistLoad, kUnitOffTurnAssistLoad2

// Producers of a unit's ordered command fields (packet unit_command_producers).
// Every offset and constant below comes from the listing of the addresses named
// in the comment on its line; see docs/UNIT_COMMAND_PRODUCERS.md. Descriptive
// names are hypotheses, not recovered symbols. These are semantic interfaces for
// MSVC Win32, not drop-in binary replacements.

namespace bsp {

// ---------------------------------------------------------------------------
// The player order queue on a unit (0080DAD0, reached from the HUD command
// screens through 00816A40).
//
// The unit carries an array of 20h-byte order slots at +838h and the index of
// the slot being filled at +97Ch. 0080DAD0 writes the incoming record into the
// indexed slot and mirrors three of its fields into a fixed "current order"
// triple at +994h/+998h/+99Ch. It does not advance the index.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kUnitOffOrderSlots = 0x838;      // 0080DAE6
inline constexpr std::size_t kUnitOrderSlotStride = 0x20;     // 0080DADD, imul by 20h
inline constexpr std::size_t kUnitOffOrderSlotIndex = 0x97C;  // 0080DAD6
inline constexpr std::size_t kUnitOffCurrentOrderParamA = 0x994; // 0080DB43
inline constexpr std::size_t kUnitOffCurrentOrderParamB = 0x998; // 0080DB34
inline constexpr std::size_t kUnitOffCurrentOrderKind = 0x99C;   // 0080DB4A

// Offsets inside one 20h-byte order slot, all relative to the slot start.
inline constexpr std::size_t kUnitOrderSlotParamA = 0x00; // 0080DAE6
inline constexpr std::size_t kUnitOrderSlotParamB = 0x04; // 0080DAF3
inline constexpr std::size_t kUnitOrderSlotActive = 0x08; // 0080DB1B, cleared to 0
inline constexpr std::size_t kUnitOrderSlotKind = 0x1C;   // 0080DB0A

// The two leading words of an order record are copied as raw 32-bit values by
// x87 loads and stores (fld/fstp at 0080DB2E..0080DB4A), so their type is not
// settled by this packet; they are carried as floats because the mirror at
// +994h/+998h is read as float by the ship-side consumers.
struct UnitOrderRecord {
    float param_a{0.0f};      // slot +00h
    float param_b{0.0f};      // slot +04h
    std::uint8_t kind{0};     // slot +1Ch, the byte the mirror at +99Ch receives
};

// The part of the unit that 0080DAD0 touches. Only the indexed slot and the
// mirror are modelled; the rest of the unit is out of scope here.
struct UnitOrderQueue {
    static constexpr std::size_t kMaxSlots = 8; // capacity of this projection only,
                                                // not a recovered native bound
    UnitOrderRecord slot[kMaxSlots]{};
    bool slot_active[kMaxSlots]{};  // slot +08h
    int slot_index{0};              // unit +97Ch
    float current_param_a{0.0f};    // unit +994h
    float current_param_b{0.0f};    // unit +998h
    std::uint8_t current_kind{0};   // unit +99Ch
};

// 0080DAD0, void __thiscall(unit, const record*), RET 4. Native operation order:
// slot.param_a, slot.param_b, slot.kind, slot.active = 0, then the mirror is
// written from the slot (not from the argument) as param_b, param_a, kind.
void publish_unit_order_0080dad0(UnitOrderQueue& queue, const UnitOrderRecord& record) noexcept;

// 00816A40, void __cdecl(a, b, c), RET 0. It builds a 20h-byte record through
// 00815440, publishes it with 0080DAD0, and when the game's session mode at
// game+1FE4h is 2 it copies the same 20h bytes into session message 8Eh and
// sends it through 0077C2A0.
inline constexpr int kUnitOrderSessionMode = 2;        // 00816A69
inline constexpr int kUnitOrderSessionMessageId = 0x8E; // 00816A76

struct UnitOrderIssueHost {
    virtual ~UnitOrderIssueHost() = default;
    // 00815440: fills the 20h-byte record from the three arguments.
    virtual UnitOrderRecord build_order_record(std::uint32_t a, std::uint32_t b,
                                               std::uint32_t c) = 0;
    // Reads the session mode at game+1FE4h (DAT_00E188A8 is the game object).
    virtual int session_mode() = 0;
    // 0075B430 + 0077C2A0: construct message 8Eh carrying the record and send it.
    virtual void send_order_message(int message_id, const UnitOrderRecord& record) = 0;
};

void issue_unit_order_00816a40(UnitOrderIssueHost& host, UnitOrderQueue& queue,
                               std::uint32_t a, std::uint32_t b, std::uint32_t c);

// ---------------------------------------------------------------------------
// The load latches at unit+102Ch and unit+1034h.
//
// 009D4FB0 and 009D4FE0 are two out-of-line copies of the same setter, one per
// field. Each is void __thiscall(unit, float request), RET 4, and raises the
// field only when the request is strictly greater than the stored value. The
// same sequence is inlined at 009DE853 (+102Ch) and six times inside 009F3F80.
// ---------------------------------------------------------------------------

// 009D4FB0: fld [ecx+102Ch]; fld [esp+4]; fcomip st,st(1); fstp st(0);
// jbe past the store; movss [ecx+102Ch], the argument. An unordered compare
// takes the jbe and leaves the field alone.
float raise_unit_load_latch_009d4fb0(float current, float request) noexcept;

// Constants the inlined copies inside 009F3F80 store instead of the request.
inline constexpr float kUnitLoadLatchHigh = 1.5f;   // 00CE380C, stored at 009F4606/009F462A
inline constexpr float kUnitLoadLatchLow = 0.5f;    // 00CE3800, stored at 009F4A51
inline constexpr double kUnitLoadLatchLowGate = 0.5;  // 00D7A280, the gate at 009F4A51
inline constexpr double kUnitLoadLatchHighGate = 1.5; // 00CE3D78, the gate at 009F4912

// 009F3F80's latch step: when the field is below the gate it becomes the level.
// Reproduces the native order, which compares first and stores a constant.
float latch_unit_load_to_level(float current, double gate, float level) noexcept;

// ---------------------------------------------------------------------------
// The script control block at unit+998h..+9A4h and the bot's intake of it.
//
// Four Lua bindings store a number straight into the block with no clamp:
// 0089D9D0 luaMW_PlaneSetYawCtrl -> +998h, 0089DB70 -> +99Ch,
// 0089DD10 luaMW_PlaneSetRollCtrl -> +9A0h, 0089DEB0 luaMW_PlaneSetPowerCtrl
// -> +9A4h. The pilot bot at 0099D300 reads all four and, for each non-zero
// value, clamps it and overrides its own plan.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kUnitOffScriptYawControl = 0x998;   // 0089DB05
inline constexpr std::size_t kUnitOffScriptPitchControl = 0x99C; // 0089DCA5
// +9A0h and +9A4h are kUnitOffHelmsmanThrust / kUnitOffHelmsmanToTurn in
// include/bsp/unit_forces.hpp; the Lua bindings 0089DD10 and 0089DEB0 write the
// same two words. They are not redefined here.

inline constexpr float kUnitControlClampLow = -1.0f; // 00D7A260
inline constexpr float kUnitControlClampHigh = 1.0f; // 00D7A24C

// 0099D57C..0099D5A0 and the three copies after it. Native order:
//   if (low <= v) { if (high < v) v = high; } else v = low;
// so an unordered compare against the low bound yields the low bound.
float clamp_unit_control_override_0099d300(float value) noexcept;

// One plan slot of the bot: a committed value, a pending value and the byte
// that says which one is current (0099B450 seeds both from one unit float and
// clears the byte; 0099D300 writes the pending value and sets it).
struct UnitPlanSlot {
    float committed{0.0f}; // +280h form
    float pending{0.0f};   // +284h form
    bool has_pending{false}; // +288h form
};

// 0099B450, void __fastcall(bot), RET 0: committed = pending = the unit float,
// has_pending = false.
void seed_unit_plan_slot_0099b450(UnitPlanSlot& slot, float unit_value) noexcept;

// The read used by 0099D300 at 0099E996 and 0099EABF.
float unit_plan_slot_value(const UnitPlanSlot& slot) noexcept;

// 0099D5A8..0099D6C0: a non-zero control word clamps and replaces the pending
// value; zero leaves the slot untouched. Returns whether the override applied.
bool apply_unit_control_override_0099d300(UnitPlanSlot& slot, float control) noexcept;

} // namespace bsp
