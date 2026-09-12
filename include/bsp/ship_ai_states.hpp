#pragma once

#include <cstdint>

namespace bsp {
// The ship AI state family at vtable 00D21598 and the control block the three
// desired-value setters write. Semantic interfaces, not native object layouts
// and not binary replacements. Every name here is a hypothesis, not a recovered
// symbol. Native addresses, ABI, evidence and uncertainty: docs/SHIP_AI_STATES.md.
//
// Three objects appear throughout. `ai` is the object 009F50E0 receives in ECX.
// `brain` is `ai+58h`, the sub-object 009F39C0 constructs, which owns the
// embedded state objects and the unit pointer at `brain+0AA8h`. `blk` is
// `ai+60h` = `brain+8h`, the control block that every per-frame step reads and
// writes; it is what `009DBF90` / `009DFFB0` / `009E0040` reach through
// `[state+4]` (evidence 009DBF90: `MOV EAX,[ECX]; ADD EAX,8`, with ECX =
// `&state->owner` at 009E1362).

// ---------------------------------------------------------------------------
// The state family
// ---------------------------------------------------------------------------

// One entry per state object embedded in `brain` by 009F39C0. `name` is the
// string the constructor registers it under through 00411E70; it is a recovered
// literal, unlike every other name in this header.
struct ShipAiStateSlot {
    const char* name;            // the registered literal
    std::uint32_t brain_offset;  // byte offset of the state object inside brain
    std::uint32_t vtable_va;     // the vtable installed at state+0
    std::uint32_t step_va;       // vtable +0Ch, __thiscall(state)(float), RET 4
    std::uint32_t command_va;    // vtable +24h, the command object it serves
};

// 009F3A07..009F3AD8 for the offsets and vtables, 009F3ADF..009F3B60 for the
// names. `step_va` and `command_va` are read out of the vtables at 00D21598 and
// following (stride 30h, twelve slots). A zero means the slot was not read.
inline constexpr ShipAiStateSlot kShipAiStates[] = {
    {"cruise",          0x0B70u, 0x00D21598u, 0x009E1170u, 0x009DAC20u},
    {"stop",            0x0B80u, 0x00D215C8u, 0x009E14C0u, 0x009DAC80u},
    {"follow",          0x0B8Cu, 0x00D215F8u, 0x009E1610u, 0x009DADB0u},
    {"land",            0x0BE0u, 0x00D21658u, 0x009E1950u, 0x009DAF50u},
    {"movetopos",       0x0C04u, 0x00D21628u, 0x009E5770u, 0x009DAE20u},
    {"moveonpath",      0x0C0Cu, 0x00D21688u, 0x009E59C0u, 0x009DB050u},
    {"attackmove",      0x0C18u, 0x00D219D0u, 0u,          0u},
    {"sub_attack",      0x2124u, 0u,          0u,          0u},
    {"kamikaze_attack", 0x21FCu, 0x00D216B8u, 0u,          0u},
};
inline constexpr int kShipAiStateCount =
    static_cast<int>(sizeof(kShipAiStates) / sizeof(kShipAiStates[0]));

// The vtable slots this packet identified. Slots +4h, +8h, +10h, +14h, +18h,
// +1Ch, +20h, +28h and +2Ch are present in every family vtable but only +2Ch
// was traced to a call site (009E5817).
inline constexpr std::uint32_t kShipAiStateVtableStep = 0x0Cu;
inline constexpr std::uint32_t kShipAiStateVtableCommand = 0x24u;
inline constexpr std::uint32_t kShipAiStateVtableInterval = 0x28u;
inline constexpr std::uint32_t kShipAiStateVtableReached = 0x2Cu;

// ---------------------------------------------------------------------------
// 009F3DD0: which state the controller should be in
// ---------------------------------------------------------------------------

struct ShipAiSyncHost {
    virtual ~ShipAiSyncHost() = default;
    // 009F3DE2, CALL EDX = [ai+0B00h]->vtable[114h]: the unit's weapon director.
    virtual void refresh_director_vtable_0114() = 0;
    // 009F3DE6, 0071BE40: the director's current command object, or 0.
    virtual std::uint32_t director_current_command_0071be40() = 0;
    // 009F3DF3, [ai+0B00h]+184h: the player-controlled byte.
    virtual bool unit_player_controlled_0184() = 0;
    // 009F3E12, CALL EAX = [ai+2264h]->vtable[24h]: the active state's command.
    virtual std::uint32_t active_state_command_vtable24() = 0;
    // 009F3E1B, 009F3D00: install the state that serves this command object.
    virtual void select_state_for_command_009f3d00(std::uint32_t command) = 0;
};

// 009F3DD0: __fastcall(ai), RET 0, body 009F3DD0-009F3E29. Returns true when a
// different state was installed. `00E08F70` is the authored Cruise object of
// docs/CRUISE_COMMAND.md, substituted whenever the unit is player-controlled.
inline constexpr std::uint32_t kShipAiCruiseCommandObject = 0x00E08F70u;
bool ship_ai_sync_state_to_command_009f3dd0(ShipAiSyncHost& host);

// ---------------------------------------------------------------------------
// The control block `blk` and the three desired-value setters
// ---------------------------------------------------------------------------

// blk+1C4h. 009DFFB0 writes 0, 009E0040 writes 1. 009ED6B0 tests the field
// against 2 and 3 for its navigation arm; no site that writes 2 or 3 was read.
enum class ShipAiSteeringMode : int {
    Rudder = 0,
    Heading = 1,
    Navigate = 2,
    NavigateAstern = 3,
};

// blk+1CCh / blk+35Ch, the ahead/astern latch 009ED6B0 keeps.
enum class ShipAiThrottleDirection : int {
    Stopped = 0,
    Ahead = 1,
    Astern = 2,
};

// Only the fields this packet read. Offsets are relative to `blk` = brain+8h.
struct ShipAiControlBlock {
    ShipAiSteeringMode mode{ShipAiSteeringMode::Rudder}; // +1C4h
    int throttle_hold_1c8{0};                            // +1C8h
    ShipAiThrottleDirection requested_direction{          // +1CCh
        ShipAiThrottleDirection::Stopped};
    float desired_throttle{0.0f}; // +1D0h, clamped to [-1,+1] by 009DBF90
    float desired_rudder{0.0f};   // +1D4h, clamped to [-1,+1] by 009DFFB0
    float desired_heading{0.0f};  // +1D8h, stored unclamped by 009E0040
    // +360h and +368h: cleared by the two mode switches, counted down by
    // 009ED6B0, and +360h set to 1.0f when the ahead/astern latch changes.
    float timer_360{0.0f};
    float timer_368{0.0f};
    ShipAiThrottleDirection direction{ // +35Ch, the latched direction
        ShipAiThrottleDirection::Stopped};
    int direction_counter_384{0};     // +384h, zeroed on a direction change
    float direction_value_374{0.0f};  // +374h, -1.0f on a direction change
    float clamp_354{0.0f};            // +354h, raised to 2.0f then to 1.0f
    float cruise_distance_3e0{0.0f};  // +3E0h, the distance used while making way
    bool yaw_rate_subtracts_364{false}; // +364h, picks add or subtract
    bool flag_3a5{false};               // +3A5h, cleared by the 007788B0 gate
    bool early_out_3f5{false};          // +3F5h, returns from 009ED6B0
    // What 009F4D10 publishes. 009ED6B0 is the only writer this packet read.
    float heading_target_324{0.0f}; // +324h
    float distance_32c{0.0f};       // +32Ch
    float distance_330{0.0f};       // +330h
};

// 009DBF90: __thiscall(&state->owner)(float), RET 4, body 009DBF90-009DBFD0,
// complete. Clears +1C8h and +1CCh and stores the argument clamped to [-1,+1]
// (00D7A260 = -1.0f, 00D7A24C = +1.0f) at +1D0h. Both COMISS use JA, which an
// unordered compare does not take, so a NaN argument reaches MOVAPS at 009DBFBD
// and is stored unchanged.
void ship_ai_set_desired_throttle_009dbf90(ShipAiControlBlock& blk, float throttle) noexcept;

struct ShipAiSetterHost {
    virtual ~ShipAiSetterHost() = default;
    // 009DFFE3 and 009E0069, 009DA4E0(blk) on every mode change. Body unread.
    virtual void on_steering_mode_change_009da4e0() = 0;
    // 009E008D, 00605070(heading) after the heading is stored. Body unread.
    virtual void after_heading_stored_00605070(float heading) = 0;
};

// 009DFFB0: __thiscall(&state->owner)(float), RET 4, body 009DFFB0-009E0011.
// When the mode is not already Rudder it clears +360h and +368h, sets the mode
// and calls 009DA4E0; then stores the argument clamped to [-1,+1] at +1D4h.
void ship_ai_set_desired_steering_009dffb0(ShipAiControlBlock& blk, float rudder,
                                           ShipAiSetterHost& host);

// 009E0040: __thiscall(&state->owner)(float), RET 4, body 009E0040-009E0095.
// The same mode switch to Heading, then stores the argument unclamped at +1D8h
// and calls 00605070 on it.
void ship_ai_set_desired_heading_009e0040(ShipAiControlBlock& blk, float heading,
                                          ShipAiSetterHost& host);

// ---------------------------------------------------------------------------
// 009ED6B0: the direct-control arm that turns the desired values into what
// 009F4D10 publishes. Partial: only 009ED6B0-009ED9A5, the arm the three
// setters feed. See docs/SHIP_AI_STATES.md for the ranges left unprojected.
// ---------------------------------------------------------------------------

struct ShipAiDirectControlHost {
    virtual ~ShipAiDirectControlHost() = default;
    // 009ED6DE, 0080E000(seconds). Body unread: contract unread.
    virtual void prologue_0080e000(float seconds) = 0;
    // 009ED73F, 007788B0 BSP_Entity_ControllerBelongsToAnother on [blk+3FCh].
    virtual bool controller_belongs_to_another_007788b0() = 0;
    // 009ED8D1, 0092D730 on the unit's controller [unit+1018h] (009ED8CB).
    virtual float unit_body_axis_speed_0092d730() = 0;
    // 009ED8EC, FDIV [ECX+508h] with ECX = [unit+538h] (009ED8E4): a ship class
    // field used as the divisor.
    virtual float ship_class_field_0508() = 0;
    // 009ED902, FADD [EAX+9C8h]: a unit float added to the distance.
    virtual float unit_field_09c8() = 0;
    // 009ED95D, CALL EAX = unit->vtable[50h](): the unit's current heading.
    virtual float unit_heading_vtable_0050() = 0;
    // 009ED9C1, 00811940: the yaw rate for the current steering command. The
    // reconstruction of that routine is unit_current_yaw_rate_00811940 in
    // bsp/unit_rudder.hpp; this method is its value at this call site.
    virtual float unit_current_yaw_rate_00811940() = 0;
};

// The half of 009ED6B0 this packet projects (009ED6B0-009EDA25). Returns the
// routine's early-out: true means 009ED6B0 returned at 009EDA25 and the
// controller zeroes ai+0B14h. A false return does NOT mean the routine ended;
// it means the unprojected navigation arm 009EDA26-009EF228 runs next and may
// overwrite +324h, +32Ch and +330h.
bool ship_ai_direct_control_arm_009ed6b0(ShipAiControlBlock& blk, float seconds,
                                         ShipAiDirectControlHost& host);

// The two literals the arm uses, from the image.
inline constexpr double kShipAiThrottleDeadzone = 0.05;   // 00D7A270
inline constexpr float kShipAiLookAheadBonus = 2000.0f;   // 00CF0DD8

// ---------------------------------------------------------------------------
// 009F50E0: the controller's frame update
// ---------------------------------------------------------------------------

struct ShipAiControllerHost {
    virtual ~ShipAiControllerHost() = default;
    // The three gates at 009F50E4, 009F50F2 and 009F50FC.
    virtual bool unit_present_0b00() = 0;
    virtual bool unit_flag_005d() = 0;
    virtual bool unit_flag_0061() = 0;
    // 009F5106, 009F3DD0(ai).
    virtual bool sync_state_009f3dd0() = 0;
    // 009F5156, 009E0270(blk, replan). Body unread.
    virtual void pre_step_009e0270(bool replan) = 0;
    // 009F516C, 009F1420(brain, elapsed). Body unread.
    virtual void replan_prepare_009f1420(float elapsed) = 0;
    // 009F5186, [ai+2264h]->vtable[0Ch](elapsed): the active state's step.
    virtual void state_step_vtable0c(float elapsed) = 0;
    // 009F519E, [ai+2264h]->vtable[28h](): the state's replan interval.
    virtual float state_interval_vtable28() = 0;
    // 009F51AE, 009DDBC0(blk), only on a replan step. Body unread.
    virtual void replan_finish_009ddbc0() = 0;
    // 009F51B7, 009DA0D0(blk), only on a non-replan step. Body unread.
    virtual void hold_009da0d0() = 0;
    // 009F51C6 .. 009F51FA, the per-frame chain on blk. Bodies unread.
    virtual void step_009eca20(float seconds) = 0;
    virtual void step_009da6e0(float seconds) = 0;
    virtual void step_009f0ea0(float seconds) = 0;
    virtual void step_009e04e0(float seconds) = 0;
    virtual void step_009ef230() = 0;
    // 009F5209, 009ED6B0(blk, seconds): true forces the next step to replan.
    virtual bool navigate_009ed6b0(float seconds) = 0;
    // 009F5227, 009F4D10(blk, seconds): publish the order record.
    virtual void publish_009f4d10(float seconds) = 0;
    // 009F5239 and 009F5248, the two brain-level tails. Bodies unread.
    virtual void tail_009da8d0(float seconds) = 0;
    virtual void tail_009f4da0(float seconds) = 0;
};

// The two timers the update owns, ai+0B14h and ai+0B18h.
struct ShipAiControllerTimers {
    float interval_0b14{0.0f};  // seconds until the next replan step
    float elapsed_0b18{0.0f};   // seconds accumulated since the last one
};

// 009F50E0: __thiscall(ai)(float), RET 4, body 009F50E0-009F5251. No Ghidra
// function starts here; see the doc's no_ghidra_function table. Returns false
// when one of the three gates stopped the update before any host call.
// 00D0DE84 = 0.05f is the multiplier applied to the state's interval.
inline constexpr float kShipAiIntervalScale = 0.05f; // 00D0DE84
bool ship_ai_controller_step_009f50e0(ShipAiControllerTimers& timers, float seconds,
                                      ShipAiControllerHost& host);

// ---------------------------------------------------------------------------
// The movetopos arrival test, 009E58B9..009E58DE
// ---------------------------------------------------------------------------

// The pure part of the movetopos step's second arm: the planar distance from
// the unit's position (unit+0FCh, unit+104h) to the goal (brain+0B2Ch,
// brain+0B34h) against a radius built from the other entity's +7A0h integer and
// the unit's +9C8h float. `reached` is what 009E58DE decides.
struct ShipAiArrivalTest {
    float distance{0.0f};
    float radius{0.0f};
    bool reached{false};
};
ShipAiArrivalTest ship_ai_movetopos_arrival_009e58b9(float unit_x, float unit_z,
                                                     float goal_x, float goal_z,
                                                     int other_radius_07a0,
                                                     float unit_field_09c8) noexcept;

// ---------------------------------------------------------------------------
// 008162B0: is this command available to this entity against this target
// ---------------------------------------------------------------------------

struct ShipAiCommandAvailabilityHost {
    virtual ~ShipAiCommandAvailabilityHost() = default;
    // 008162BF, 00779D50(name, target). Body unread: contract unread.
    virtual bool call_00779d50() = 0;
    // 008162DB, CALL EDX = this->vtable[168h](name).
    virtual bool entity_offers_command_vtable168() = 0;
    // 008162EB 00467170 BSP_CommandRegistry_FindByName, then 008162F7
    // CALL EAX = registry->vtable[8](). Only reached when target is null.
    virtual bool registry_command_flag_00467170() = 0;
    // 0081630D (5), 0081631C (18h), 0081632B (1Ah), 0081637E (6), 0081638D (8)
    // and 008163BA (8): CALL EAX = target->vtable[5Ch](kind).
    virtual bool target_kind_vtable5c(int kind) = 0;
    // 00816337, target+5Dh.
    virtual bool target_flag_005d() = 0;
    // 00816346, 00438E10(name, "attackmove" at 00CECCA8 loaded at 0081633F):
    // nonzero when the names differ.
    virtual bool name_differs_from_attackmove_00438e10() = 0;
    // 00816359, 00803510([this+54h], [target+54h]). Body unread.
    virtual bool sides_related_00803510() = 0;
    // 0081636F, CALL EAX = this->vtable[5Ch](8).
    virtual bool self_kind_vtable5c(int kind) = 0;
    // 0081639D, 00827F70(...). Body unread.
    virtual bool call_00827f70() = 0;
    // 008163A8, 00922B10 BSP_Entity_IsAirborneTarget.
    virtual bool target_is_airborne_00922b10() = 0;
    // 008163C6, 00852820(...). Body unread.
    virtual bool call_00852820() = 0;
    // The child walk from this+48h chained through +44h: 008163E2 00465020 on
    // each child and 008163ED 0080F750 on a nonzero result. Bodies unread.
    virtual bool any_child_accepts_00465020_0080f750() = 0;
};

// 008162B0: __thiscall(entity)(const char* name, entity* target), RET 8
// (008162CD), body 008162B0-00816408. The two stack arguments are proved by the
// RET 8 and by the two loads at 008162B1 and 008162B6.
bool ship_ai_command_available_008162b0(ShipAiCommandAvailabilityHost& host,
                                        bool has_target);

}  // namespace bsp
