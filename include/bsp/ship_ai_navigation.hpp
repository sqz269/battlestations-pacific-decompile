#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/ship_ai_states.hpp"
#include "bsp/unit_autopilot_pair.hpp"

namespace bsp {
// The navigation arm of 009ED6B0 and the record the ship AI publishes into the
// unit. Semantic interfaces, not native object layouts. Names are hypotheses,
// not recovered symbols. Addresses, evidence, original ABI and uncertainty:
// docs/SHIP_AI_NAVIGATION_ARM.md and docs/UNIT_AI_ORDER_SLOT_READER.md.

// ---------------------------------------------------------------------------
// The 84-byte record at unit+0AECh / unit+0A98h, in full
// ---------------------------------------------------------------------------
// bsp/unit_autopilot_pair.hpp declares the three order fields and the flag as
// UnitAiOrderSlot, which is what 009F4D10 writes. This is the whole record,
// because the copy 00811D10 makes across the index flip is bounded by it: the
// copy runs 00811D14..00811D71 over +00h..+3Fh only and never touches +40h,
// +44h, +48h, +4Ch or +50h.
inline constexpr std::size_t kUnitAiOrderRecordSize = 0x54u;         // 00825F38 IMUL 0x54
inline constexpr std::size_t kUnitAiOrderRecordCopiedBytes = 0x40u;  // 00811D14..00811D71
inline constexpr std::uint32_t kUnitAiOrderPublishedSlot0 = 0x0A98u; // 00825F67 LEA +0A98h

// The two 1Ch-byte sub-records the copy shows at +08h and +24h: the second is
// the first plus 1Ch in every one of the eight copied pairs, and 0080E000
// resets both with the same five stores.
struct UnitAiOrderSubRecord {
    float value_00{0.0f};      // record +08h / +24h, the sub-record's own value
    float field_04{0.0f};      // +0Ch / +28h
    float field_08{1000.0f};   // +10h / +2Ch, reset to 1000.0f (00CE3804)
    bool flag_0c{false};       // +14h / +30h
    float field_10{0.0f};      // +18h / +34h
    float field_14{0.0f};      // +1Ch / +38h
    std::uint32_t field_18{0}; // +20h / +3Ch
};

// The whole record. +40h, +44h, +48h and +4Ch mirror UnitAiOrderSlot; they are
// named again here so the copy boundary can be expressed on one object.
struct UnitAiOrderRecord {
    float blend_00{0.0f};      // +00h, slewed toward sub_a.value_00 by 0080E000
    float timer_04{0.0f};      // +04h, counted down by 0080E000 and set by 00815F30
    UnitAiOrderSubRecord sub_a{}; // +08h..+23h
    UnitAiOrderSubRecord sub_b{}; // +24h..+3Fh
    float distance_40{0.0f};   // +40h, blk+32Ch, the distance to the waypoint
    float heading_44{0.0f};    // +44h, the limited heading target, from 00811960
    float distance_48{0.0f};   // +48h, blk+330h, the remaining path length
    bool valid_4c{false};      // +4Ch, the published flag
};

// 009F4D2F..009F4D3B and 00825F38..00825F71: the slot the AI writes this frame
// is `unit + 0AECh - 54h * index`, and the slot the promotion copies from -
// the one a reader sees, because the promotion has already flipped the index -
// is `unit + 0A98h + 54h * index`. For index in {0,1} the two are the pair.
std::uint32_t unit_ai_order_current_offset(int index) noexcept;
std::uint32_t unit_ai_order_published_offset(int index) noexcept;

// 00811D10: __thiscall(dst)(src), RET 4, body 00811D10-00811D76, complete.
// Sixteen field copies over +00h..+3Fh with no branch. The order triple, the
// flag and the unit back-pointer are deliberately outside it.
void unit_ai_order_copy_00811d10(UnitAiOrderRecord& dst, const UnitAiOrderRecord& src) noexcept;

// The rate 0080E000 slews +00h toward sub_a.value_00 with, and the value it
// resets both sub-records' field_08 to.
inline constexpr double kUnitAiOrderBlendRate = 50.0;      // 00CE3938, a double
inline constexpr float kUnitAiOrderSubRecordReset = 1000.0f; // 00CE3804

// 0080E000: __thiscall(record)(float seconds), RET 4, body 0080E000-0080E0EA,
// complete. Its only call site is 009ED6DE, the first thing 009ED6B0 does.
// While timer_04 is above zero it is decremented by `seconds`; a decrement that
// leaves it at or below zero clears both sub-records. Then +00h moves toward
// sub_a.value_00 by at most `seconds * 50.0`, and lands exactly on it when the
// remaining gap is no larger than that step.
void unit_ai_order_slot_step_0080e000(UnitAiOrderRecord& record, float seconds) noexcept;

// ---------------------------------------------------------------------------
// 009ED6B0's navigation arm, 009EDA26-009EF228
// ---------------------------------------------------------------------------
// The caller owns the gate. 009ED6B0 enters the arm only for blk+1C4h in
// {Navigate, NavigateAstern}, and 009EE756 then skips the output block while the
// mode is NavigateAstern, so what this header projects runs for Navigate alone.
// Everything projected lies in 009EE671..009EEAA2; the collision and formation
// work at 009EDA26..009EE670 and the tail at 009EEAAB..009EF228 are not
// projected. Coverage is partial; docs/SHIP_AI_NAVIGATION_ARM.md lists the
// ranges.

// blk+304h, and the code 009E3C00 hands back with the path point. The arm only
// ever uses it to one-side the heading error at 009EE916..009EE936, so the
// names say what the comparison does and claim nothing about port or starboard.
enum class ShipAiNavTurnSide : int {
    Unconstrained = 0,
    ClampNonPositive = 1, // 009EE929: a positive error is forced to zero
    ClampNonNegative = 2, // 009EE916: a negative error is forced to zero
};

// The fields of `blk` the arm uses that ShipAiControlBlock does not declare.
// Offsets are relative to blk = brain+8h, the same base ShipAiControlBlock uses.
struct ShipAiNavState {
    float longest_path_1f0{0.0f};   // +1F0h, the largest path length seen
    float goal_x_0a94{0.0f};        // +0A94h, republished every frame
    float goal_y_0a98{0.0f};        // +0A98h, always stored as zero (009EE68C)
    float goal_z_0a9c{0.0f};        // +0A9Ch
    bool goal_valid_0a90{false};    // +0A90h, set to 1 at 009EE67A
    ShipAiNavTurnSide side_304{       // +304h, the turn side the path point asks for
        ShipAiNavTurnSide::Unconstrained};
    float turn_lead_328{0.0f};      // +328h, the correction the arm applied
    float next_leg_heading_334{0.0f}; // +334h, the bearing of the following leg
    bool last_leg_338{false};       // +338h
    float look_ahead_340{0.0f};     // +340h, seeded from +3C8h at 009ED769
    float look_ahead_max_3c8{0.0f}; // +3C8h, the ceiling for +340h
    float turn_window_3d0{0.0f};    // +3D0h, the heading error the arm tolerates
    std::array<float, 2> hull_axis_19c{{0.0f, 0.0f}}; // +19Ch, the vector 009EEA1A reads
};

// What 009E3C00 hands back at 009EE550: the path point the unit steers at, the
// point after it, whether the path continues and which way the leg turns.
struct ShipAiNavWaypoint {
    float x{0.0f};
    float z{0.0f};
    float next_x{0.0f};
    float next_z{0.0f};
    bool more_path{false};   // local_34: the path continues past this point
    bool steer_enabled{false}; // the byte at [ESP+99h], 009EE801
    ShipAiNavTurnSide side{ShipAiNavTurnSide::Unconstrained}; // local_38
};

struct ShipAiNavHost {
    virtual ~ShipAiNavHost() = default;
    // 009EE6AC, 009D9E50([blk+2F4h])(&pose): the remaining length of the path.
    // Body unread: contract unread, the value is used as a distance.
    virtual float remaining_path_length_009d9e50() = 0;
    // 009EE8C7, CALL EAX = unit->vtable[50h]() on [blk+3FCh], no argument, the
    // result taken as a float. Read as the unit's own heading because the arm
    // folds it against a bearing; that reading is provisional and the slot's
    // prototype is disputed between call sites. See the follow-up
    // `unit_heading_vtable_0050` in docs/UNIT_AI_ORDER_SLOT_READER.md.
    virtual float unit_heading_vtable_0050() = 0;
};

// The two positions the arm differences: the unit's pose, read through
// [blk+184h] / [blk+188h] at 009EE702 and 009EE70F.
struct ShipAiNavPose {
    float x{0.0f};
    float z{0.0f};
};

struct ShipAiNavResult {
    float distance_to_waypoint{0.0f}; // what landed in blk+32Ch
    float heading_target{0.0f};       // what landed in blk+324h
    float path_length{0.0f};          // what landed in blk+330h
    bool bearing_taken{false};        // the 009EE7FB / 009EE809 gate opened
    bool turn_lead_applied{false};    // the 009EE964 gate opened
};

// The literals the arm uses, from the image.
inline constexpr double kShipAiNavBearingDeadzone = 0.1;      // 00D7A3A0, 009EE7EB
inline constexpr double kShipAiNavTurnLeadRange = 1000.0;     // 00CE47A0, 009EE83A
inline constexpr float kShipAiNavTurnLeadCap = 1000.0f;       // 00CE3804, 009EE824
inline constexpr double kShipAiNavPathLengthScale = 1.5;      // 00CE3D78, 009EE856
inline constexpr double kShipAiNavQuarterTurn = 1.57079637051; // 00CE3830, 009EE8B7
inline constexpr double kShipAiNavNextLegMinLength2 = 100.0;  // 00D7A220, 009EE7C3
inline constexpr float kShipAiNavCosineFloor = 0.001f;        // 00D7A23C, 009EEA64

// 009EE671..009EEAA2, the output block of the navigation arm.
// void __thiscall(blk)(float seconds) as part of 009ED6B0, RET 4.
// Writes blk+32Ch, blk+324h, blk+330h - the exact trio 009F4D10 publishes -
// plus the six nav fields above. Returns what it wrote so a caller can watch it.
ShipAiNavResult ship_ai_navigation_arm_009ee671(ShipAiControlBlock& blk,
                                                ShipAiNavState& nav,
                                                const ShipAiNavWaypoint& waypoint,
                                                const ShipAiNavPose& pose,
                                                ShipAiNavHost& host);

// 009EE848..009EE9AF on its own, so a caller can test the turn lead without a
// path. `applied` is the 009EE964 gate: the arm adds `correction` to blk+324h
// and stores it in blk+328h only while the heading error is outside blk+3D0h.
struct ShipAiTurnLead {
    bool applied{false};
    float correction{0.0f};
};
ShipAiTurnLead ship_ai_turn_lead_009ee848(float bearing, float distance, float unit_heading,
                                          const ShipAiNavState& nav,
                                          ShipAiThrottleDirection direction) noexcept;

}  // namespace bsp
