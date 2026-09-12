// What one frame of a ship AI state step does.
//
// Packet cc_ship_ai_state_steps, worker agent/cc-ship-ai-state-steps.
// Project C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe; Ghidra
// was read-only for this packet. Every descriptive name here is a hypothesis,
// not a recovered symbol. docs/SHIP_AI_STATE_STEPS.md carries the evidence.
//
// This header builds on bsp/ship_ai_states.hpp (ShipAiControlBlock, the three
// setters, ShipAiSteeringMode, ShipAiThrottleDirection) and does not redefine
// any of its types. `blk` is brain+8h throughout, as that header records; a few
// fields here are quoted relative to `brain` because the image addresses them
// that way, and the blk-relative offset is given next to each.
#pragma once

#include <cstdint>

#include "bsp/ship_ai_states.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// 009DE050, the navigation goal setter every state step goes through
// ---------------------------------------------------------------------------
//
// __thiscall(blk)(const float* goal2d, char keep_mode, unsigned char final_leg),
// RET 0Ch, body 009DE050-009DE1A6, complete.
//
// Ten step routines call it: 009E14C0 stop, 009E1610 follow, 009E1950 land,
// 009E2020 kamikaze_attack, 009E23B0 and 009E26C0 and 009F3240 (three attackmove
// sub-states), 009E4B90, 009E59C0 moveonpath and 009E5770 movetopos. The Ghidra
// call graph lists nine; 009E59C0 has no Ghidra function so its site at 009E5AE2
// is not in it. It is not a computation shared between them: it is the one writer
// of the AI's navigation goal. It latches the goal on the block, forces Navigate
// mode, decides whether the path planner has to re-plan, and seeds the reference
// path length the turn lead tapers against.

// |goal - last goal|^2 above which the crossing state is dropped. 00D20278, a
// float32 compared against a float32 sum (009DE0CD..009DE0DB).
inline constexpr float kShipAiGoalMoveResetDistanceSq = 2500.0f;
// |goal - planned goal|^2 above which the path is re-planned. 00D21530, a
// double; the float32 sum is widened for the compare (009DE123..009DE131).
inline constexpr double kShipAiGoalReplanDistanceSq = 6400.0;

// The fields 009DE050 reads and writes. Offsets are on `blk` = brain+8h.
struct ShipAiGoalPlan {
    ShipAiSteeringMode mode{ShipAiSteeringMode::Rudder}; // +1C4h
    int throttle_hold_1c8{0};                           // +1C8h, set to 1
    ShipAiThrottleDirection requested_direction{        // +1CCh, cleared
        ShipAiThrottleDirection::Stopped};
    // +184h / +188h: the unit pose the per-frame chain leaves on the block.
    // 009DE050 only reads them, at 009DE17C and 009DE189.
    float pose_x_184{0.0f};
    float pose_z_188{0.0f};
    float goal_x_1dc{0.0f};    // +1DCh, the goal as of the last call
    float goal_z_1e0{0.0f};    // +1E0h
    bool final_leg_1e4{false}; // +1E4h, read by the navigation arm at 009EE6DE
    float planned_x_1e8{0.0f}; // +1E8h, the goal the current path was planned to
    float planned_z_1ec{0.0f}; // +1ECh
    // +1F0h, the reference path length. 009DE050 seeds it with the straight-line
    // distance from the pose to the goal; 009EE6B5..009EE6CB raises it to the
    // planner's path length, and 009EE856 tapers the turn lead against it.
    float plan_length_1f0{0.0f};
    float crossing_314{0.0f};  // +314h, zeroed when the goal jumps
    bool flag_2fd{false};      // +2FDh, zeroed with it
    bool flag_2fe{false};      // +2FEh, zeroed with it
};

struct ShipAiGoalHost {
    virtual ~ShipAiGoalHost() = default;
    // 009DE082, 009DA4E0(blk) on the branch that enters Navigate mode. The body
    // is ship_ai_clear_path_plan_009da4e0 below.
    virtual void clear_path_plan_009da4e0() = 0;
    // 009DE193, 00414C60 BSP_Vector2f_LengthWithCutoff on (goal - pose).
    virtual float planar_length_00414c60(float dx, float dz) = 0;
};

// `keep_mode` is the second argument, the middle of the three pushes. Nine of
// the ten sites were checked and every one passes zero: 009E1501 (PUSH EBX, 0),
// 009E1EAC, 009E22E1, 009E2A49, 009E4BE3, 009E5806 (PUSH EDI, 0), 009E5AD9
// (PUSH EBX, 0), and the two the decompiler shows inside 009E1610 and 009E23B0.
// 009F332B's middle push was not transcribed. The `keep_mode != 0` arm, which
// skips the re-plan and still forces Navigate mode at 009DE152, is therefore
// projected from the listing and exercised by no call site this packet read.
void ship_ai_set_navigation_goal_009de050(ShipAiGoalPlan& blk, float goal_x, float goal_z,
                                          bool keep_mode, bool final_leg,
                                          ShipAiGoalHost& host);

// ---------------------------------------------------------------------------
// 009DA4E0, the path-plan reset
// ---------------------------------------------------------------------------
//
// __fastcall(blk), RET 0, body 009DA4E0-009DA58D, complete. bsp/ship_ai_states.hpp
// declares it as ShipAiSetterHost::on_steering_mode_change_009da4e0 with "body
// unread"; this is that body. Both mode setters, 009E00A0, 009DFF40 and 009DE050
// call it.
struct ShipAiPathPlan {
    // The two owned path objects. Each is released through its own vtable slot 0
    // with argument 1 (009DA4E9, 009DA524) before the field is cleared.
    std::uint32_t path_object_244{0}; // +244h
    std::uint32_t path_object_2ac{0}; // +2ACh
    std::uint32_t cursor_240{0};      // +240h
    std::uint32_t count_248{0};       // +248h
    std::uint32_t index_258{0};       // +258h, `param_1 + 600`
    std::uint32_t value_250{0};       // +250h
    std::uint32_t limit_254{0};       // +254h, set to [00CF58EC]
    std::uint32_t value_25c{0};       // +25Ch
    std::uint32_t cursor_2a8{0};      // +2A8h
    std::uint32_t count_2b0{0};       // +2B0h
    std::uint32_t value_2b8{0};       // +2B8h
    std::uint32_t limit_2bc{0};       // +2BCh, `param_1 + 700`, set to [00CF58EC]
    std::uint32_t value_2c0{0};       // +2C0h
    std::uint32_t value_2c4{0};       // +2C4h
    bool flag_2fc{false};             // +2FCh
    bool flag_2fd{false};             // +2FDh
    bool flag_2fe{false};             // +2FEh
};

struct ShipAiPathPlanHost {
    virtual ~ShipAiPathPlanHost() = default;
    // 009DA4E9 and 009DA524: (*object)->vtable[0](1), the release of each owned
    // path object. Callee bodies unread: contract unread.
    virtual void release_path_object_vtable_0000(std::uint32_t object) = 0;
    // [00CF58EC], the value stored into +254h and +2BCh. Read once at 009DA4FB.
    virtual std::uint32_t path_limit_default_00cf58ec() = 0;
};

void ship_ai_clear_path_plan_009da4e0(ShipAiPathPlan& blk, ShipAiPathPlanHost& host);

// ---------------------------------------------------------------------------
// 009E00A0 and 009DFF40, the two "stop navigating" helpers a step calls
// ---------------------------------------------------------------------------
//
// 009DFF40: __thiscall(brain)(float heading), RET 4, body 009DFF40-009DFF91.
// 009E00A0: __thiscall(&state->owner)(), RET 0, body 009E00A0-009E0126.
// Both do what 009E0040 does (switch to Heading mode, clear the two timers, drop
// the path plan, store the heading, call 00605070) and then clear +1CCh.
// 009E00A0 additionally takes the heading from the unit instead of an argument
// and zeroes the throttle and the hold flag, so it is a full "all stop, hold the
// heading you are on".

struct ShipAiHeadingHoldHost {
    virtual ~ShipAiHeadingHoldHost() = default;
    // 009E00B2, unit->vtable[50h](): the unit's current heading. 009E00A0 only.
    virtual float unit_heading_vtable_0050() = 0;
    // 009E00E5 / 009DFF6C, 009DA4E0(blk).
    virtual void clear_path_plan_009da4e0() = 0;
    // 009E00FA / 009DFF81, 00605070 with ECX = &blk+1D8h. Body unread.
    virtual void after_heading_stored_00605070(float heading) = 0;
};

// 009DFF40. `blk` is the same object ShipAiControlBlock describes.
void ship_ai_set_heading_drop_path_009dff40(ShipAiControlBlock& blk, float heading,
                                            ShipAiHeadingHoldHost& host);
// 009E00A0.
void ship_ai_hold_heading_and_stop_009e00a0(ShipAiControlBlock& blk,
                                            ShipAiHeadingHoldHost& host);

// ---------------------------------------------------------------------------
// 009E14C0, the `stop` step
// ---------------------------------------------------------------------------
//
// __thiscall(state)(float seconds), RET 4, body 009E14C0-009E1605, complete.
// vtable 00D215C8 slot +0Ch, the state at brain+0B80h. The float argument is
// never read.

// |body-axis speed| below which the `stop` state declares the ship stopped.
// 00CE65D0, a double that is (double)0.4f, compared against a float32 magnitude
// at 009E15A1..009E15AF.
inline constexpr double kShipAiStopSpeedThreshold = 0.4;
// 00D7A208 = -0.0f. The magnitude is taken as (-0.0f - v), not fabs.
inline constexpr float kShipAiStopNegativeZero = -0.0f;

// The three fields the `stop` step writes at brain+3F4h/+3F8h/+3FCh. The image
// addresses them from `brain`, so the blk-relative offsets are 8 lower.
struct ShipAiAvoidanceRequest {
    bool enable_3f4{false};  // brain+3F4h = blk+3ECh, set to 1 on both arms
    int side_filter_3f8{-1}; // brain+3F8h = blk+3F0h, read by 009F0EA0 at 009F1052
    bool flag_3fc{false};    // brain+3FCh = blk+3F4h, read by 009DA6E0 at 009DA6E6
};

// The `stop` state object's own storage. A state object is sixteen bytes; the
// `stop` step uses the byte at +8h as a latch, not as the float the constructor
// seeded there (009F3A22 stores -99.0f, whose low byte is 0).
struct ShipAiStopStepState {
    bool making_way_08{false}; // state+8h
};

struct ShipAiStopStepHost {
    virtual ~ShipAiStopStepHost() = default;
    // 009E14D7, the unit's pose-valid byte at unit+0C8h.
    virtual bool unit_pose_valid_00c8() = 0;
    // 009E14E1, 00414DB0 BSP_EntityPose_RefreshWorld(unit).
    virtual void refresh_unit_pose_00414db0() = 0;
    // 009E14EC and 009E14F2, the unit's world position: the call takes
    // &unit+0FCh, so the y component at +100h is part of the test.
    virtual void unit_position_00fc(float& x, float& y, float& z) = 0;
    // 009E14F3, 0071C4F0 on [00E188A8]: true when the position is OUTSIDE the
    // world box (x in [+711Ch,+7128h], z in [+7130h,+7124h]). Body read.
    virtual bool position_outside_world_bounds_0071c4f0(float x, float y, float z) = 0;
    // 009E1518 and the arm at 009E14FC: 009DE050(blk, &goal, 0, 1).
    virtual void set_navigation_goal_009de050(float goal_x, float goal_z, bool keep_mode,
                                              bool final_leg) = 0;
    // 009E1534, unit->vtable[50h](): the unit's current heading.
    virtual float unit_heading_vtable_0050() = 0;
    // 009E153C, 009E0040 BSP_ShipAi_SetDesiredHeading(&state->owner, heading).
    virtual void set_desired_heading_009e0040(float heading) = 0;
    // 009E1570, 0092D730 BSP_UnitController_GetBodyAxisSpeed on [unit+1018h].
    virtual float unit_body_axis_speed_0092d730() = 0;
};

void ship_ai_stop_step_009e14c0(ShipAiStopStepState& state, ShipAiControlBlock& blk,
                                ShipAiAvoidanceRequest& request, ShipAiStopStepHost& host);

// ---------------------------------------------------------------------------
// 009E5770, the `movetopos` step
// ---------------------------------------------------------------------------
//
// __thiscall(state)(float seconds), RET 4, body 009E5770-009E59B7, complete.
// vtable 00D21628 slot +0Ch, the state at brain+0C04h, serving the authored
// command object 00E08F68 that 009DAE20 returns. The float argument is never
// read.

// The command object 009DAE20 returns (`MOV EAX,0E08F68h; RET` at 009DAE20).
// Milestone 2n's kShipAiCommandStates lists the same value for `movetopos`.
inline constexpr std::uint32_t kShipAiMoveToPosCommandObject = 0x00e08f68u;

struct ShipAiMoveToPosStepHost {
    virtual ~ShipAiMoveToPosStepHost() = default;
    // 009E579A, unit+0C8h; 009E57A5, 00414DB0 on the unit.
    virtual bool unit_pose_valid_00c8() = 0;
    virtual void refresh_unit_pose_00414db0() = 0;
    // 009E57AA / 009E57C2, unit+0FCh and unit+104h.
    virtual void unit_position_xz_00fc(float& x, float& z) = 0;
    // 009E57D0 / 009E57B4, brain+0B2Ch and brain+0B34h: the AI's goal vector.
    virtual void brain_goal_xz_0b2c(float& x, float& z) = 0;
    // 009E57ED, 0071BFF0(director, 0) with ECX = [brain+0AB8h]: slot `index` of
    // the director's command array at +1A4h, offset by 10h. Body read.
    virtual std::uint32_t director_command_slot_0071bff0(int index) = 0;
    // 009E57F4, 007ADC60 on that slot: true when the command's waypoint list is
    // absent or empty, or when its mode is 1 and the cursor is on the last leg
    // (or the first leg when the reverse byte is clear). Body read.
    virtual bool command_on_final_leg_007adc60(std::uint32_t slot) = 0;
    // 009E580F, 009DE050(blk, &goal, 0, final_leg).
    virtual void set_navigation_goal_009de050(float goal_x, float goal_z, bool keep_mode,
                                              bool final_leg) = 0;
    // 009E5821, state->vtable[2Ch](&goal): the state's own arrival test. Callee
    // body unread: contract unread.
    virtual bool state_goal_reached_vtable_002c(float goal_x, float goal_z) = 0;
    // 009E5831, [brain+0AB8h] then +54h: the command object the director holds.
    virtual std::uint32_t director_current_command_0054() = 0;
    // 009E5847, 00521EA0 BSP_CommandTarget_ResolveObject on director+58h.
    virtual std::uint32_t resolve_command_target_00521ea0() = 0;
    // 009E585F, target->vtable[5Ch](1Ch): an entity-kind predicate. Callee body
    // unread: contract unread.
    virtual bool target_is_kind_vtable_005c(std::uint32_t target, int kind) = 0;
    // 009E5869 / 009E5874, the target's own pose-valid byte and refresh.
    virtual bool target_pose_valid_00c8(std::uint32_t target) = 0;
    virtual void refresh_target_pose_00414db0(std::uint32_t target) = 0;
    // 009E5879 / 009E5887, target+0FCh and target+104h.
    virtual void target_position_xz_00fc(std::uint32_t target, float& x, float& z) = 0;
    // 009E58CE, the signed int at target+7A0h, loaded with FILD.
    virtual int target_range_07a0(std::uint32_t target) = 0;
    // 009E58D4, the float at unit+9C8h, subtracted from that range.
    virtual float unit_radius_09c8() = 0;
    // 009E58B9, 00414C60 BSP_Vector2f_LengthWithCutoff on (pos - target pos).
    virtual float planar_length_00414c60(float dx, float dz) = 0;
    // The "finished" message, 009E58E6-009E59A2. Each call site is its own
    // method because no callee body was read.
    // 009E58EF, 0041E870 BSP_NativeString_Assign(&text, "finished" at 00D09FD8).
    virtual void message_text_assign_0041e870(const char* text) = 0;
    // 009E595C, 00984300(unit, &payload, 00E08F68, &text), __stdcall RET 10h.
    virtual void post_command_message_00984300(std::uint32_t command) = 0;
    // 009E597C and 009E5983, the string buffer release pair 00419CC0 / 00BD1510.
    virtual void release_message_text_00419cc0() = 0;
    // 009E5997, 0071E430(director, 00E08F68, 1): the director is told the command
    // is done. Callee read only at its head: contract partial.
    virtual void end_command_0071e430(std::uint32_t command, int flag) = 0;
    // 009E599E, 009E00A0(&state->owner).
    virtual void hold_heading_and_stop_009e00a0() = 0;
};

// Returns true when the step reached the "finished" arm (009E58E6) and the
// command was ended; false on every early return.
bool ship_ai_movetopos_step_009e5770(ShipAiMoveToPosStepHost& host);

// ---------------------------------------------------------------------------
// 009E8820, the `attackmove` step, and 009E86F0, its sub-state selector
// ---------------------------------------------------------------------------
//
// 009E8820: __thiscall(state)(float seconds), RET 4, body 009E8820-009E88F9.
// No Ghidra function starts there; the boundary is in the doc's
// `## no_ghidra_function` table. vtable 00D219D0 slot +0Ch, the state at
// brain+0C18h, serving 00E08F78 (009E8540: `MOV EAX,0E08F78h; RET`).
//
// 009E86F0: __thiscall(state)(float seconds), RET 4, body 009E86F0-009E881A,
// complete. The selector that picks which of the five embedded sub-states runs.

// The five sub-states 009E8450 builds, as offsets on the attackmove state object.
// Each row is `state offset`, the vtable stored there, and that vtable's +0Ch.
struct ShipAiAttackMoveSubState {
    std::uint32_t state_offset; // LEA in 009E8450 / 009E86F0
    std::uint32_t vtable;
    std::uint32_t step; // vtable +0Ch
};
inline constexpr ShipAiAttackMoveSubState kShipAiAttackMoveSubStates[] = {
    {0x0008u, 0x00d21994u, 0x009f3240u}, // 009E8486, built by 009E5CA0
    {0x14C0u, 0x00d2174cu, 0x009e23b0u}, // 009E8491
    {0x14CCu, 0x00d2177cu, 0x009e26c0u}, // 009E84A7
    {0x14E0u, 0x00d217acu, 0x009f3670u}, // 009E84B7
    {0x14F4u, 0x00d2171cu, 0x007b3dd0u}, // 009E84CA, the one 009E850F starts on
};
inline constexpr int kShipAiAttackMoveSubStateCount =
    static_cast<int>(sizeof(kShipAiAttackMoveSubStates) / sizeof(kShipAiAttackMoveSubStates[0]));

// The selector's own storage on the attackmove state object.
struct ShipAiAttackMoveSelector {
    float countdown_1500{0.0f};       // +1500h, seeded negative by the constructor
    float interval_14fc{1.0f};        // +14FCh, [00D7A24C] = 1.0f at 009E84D1
    std::uint32_t current_1508{0};    // +1508h, the member the machine holds
    std::uint32_t machine_1504{0};    // +1504h, the machine object 007B6EE0 takes
};

struct ShipAiAttackMoveSelectorHost {
    virtual ~ShipAiAttackMoveSelectorHost() = default;
    // 009E8714, [brain+0B20h]: the AI's current attack target.
    virtual std::uint32_t brain_attack_target_0b20() = 0;
    // 009E8733 and 009E8799, target->vtable[5Ch](kind) with kind 8 then 1Ch.
    // Callee body unread: contract unread.
    virtual bool target_is_kind_vtable_005c(std::uint32_t target, int kind) = 0;
    // 009E873B, 00852860 with ECX = target. Body unread: contract unread.
    virtual bool call_00852860(std::uint32_t target) = 0;
    // 009E8747, the byte at brain+0B28h.
    virtual bool brain_flag_0b28() = 0;
    // 009E87BD and 009E87E3, 007B6EE0(machine at state+1504h, member):
    // __thiscall(machine)(member), RET 4, body 007B6EE0-007B6F0A. Returns at
    // once when machine+4h is already `member`; otherwise calls the old
    // member's vtable[8], stores the new one at machine+4h and calls its
    // vtable[4]. That is the same sequence 009E87EE inlines.
    virtual void set_current_substate_007b6ee0(std::uint32_t member) = 0;
    // 009E87CD, 009E85B0 with ECX = state+8h. Body unread: contract unread.
    virtual bool call_009e85b0() = 0;
    // 009E8804 and 009E8813, the inline transition on the no-target arm:
    // current->vtable[8]() then member->vtable[4](). Bodies unread.
    virtual void substate_exit_vtable_0008(std::uint32_t member) = 0;
    virtual void substate_enter_vtable_0004(std::uint32_t member) = 0;
};

// `state_base` is the address the LEAs are relative to, so the routine can name
// the sub-states the way the image does.
void ship_ai_attackmove_select_009e86f0(ShipAiAttackMoveSelector& selector,
                                        std::uint32_t state_base, float seconds,
                                        ShipAiAttackMoveSelectorHost& host);

struct ShipAiAttackMoveStepHost {
    virtual ~ShipAiAttackMoveStepHost() = default;
    // 009E8828, [brain+0AA8h]: the unit. Zero takes the run arm.
    virtual std::uint32_t brain_unit_0aa8() = 0;
    // 009E883F and 009E888C, entity->vtable[5Ch](9). Callee body unread.
    virtual bool entity_is_kind_vtable_005c(std::uint32_t entity, int kind) = 0;
    // 009E8852, [unit+284h]: the unit's group, and 009E8861, [group+4F8h], its
    // member count.
    virtual std::uint32_t unit_group_0284() = 0;
    virtual int group_member_count_04f8(std::uint32_t group) = 0;
    // 009E8873, 0070D060(group, index): __thiscall(group)(int), RET 4, body
    // 0070D060-0070D06D, returning [group + 34h*index + 18h].
    virtual std::uint32_t group_member_at_0070d060(std::uint32_t group, int index) = 0;
    // 009E88BD, unit->vtable[114h](): the director. Callee body unread.
    virtual std::uint32_t unit_director_vtable_0114() = 0;
    // 009E88C1, 0071E430(director, 00E08F78, 1).
    virtual void end_command_0071e430(std::uint32_t director, std::uint32_t command,
                                      int flag) = 0;
    // 009E88D8, 009E86F0(state, seconds).
    virtual void select_substate_009e86f0(float seconds) = 0;
    // 009E88F0, [state+1508h]->vtable[0Ch](seconds): the chosen sub-state's step.
    virtual void substate_step_vtable_000c(float seconds) = 0;
};

// Returns true when the step handed the command back (009E88A5 arm) instead of
// running the sub-state.
bool ship_ai_attackmove_step_009e8820(float seconds, ShipAiAttackMoveStepHost& host);

} // namespace bsp
