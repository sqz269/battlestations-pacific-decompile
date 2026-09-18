#ifndef BSP_TORPEDO_TASK_ARM_HPP
#define BSP_TORPEDO_TASK_ARM_HPP

// The torpedo bot task, from its per-tick arm 009D4850 to the ordnance release
// request 007BBBA0. docs/TORPEDO_TASK_ARM.md carries the evidence;
// docs/BOT_TASKS.md owns the task object and the thirteen classes, and
// docs/BOT_TASK_STATES.md owns the state family this header extends.
//
// Every name here is a hypothesis, not a recovered symbol. Nothing is a
// binary-compatible layout: the offset constants are the native ones, the
// structs are not.
//
// Contracts named but not reconstructed: the device at unit+DECh that 007BBBA0
// flags and the projectile spawn behind it; the approach update 009D3420; the
// bearing/range producers 009D1500 and 009D1360; the aim tick 009D15F0
// (009D15F0-009D2377, 3464 bytes, analyzed for its writes only); the steering
// helper 009F9E40; the follow base tick 009C1FD0.

#include "bsp/bot_task_states.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The seven torpedo states, as whole-object offsets from the task. 009D4030
// and 009D3F60 compare exactly these pointers; docs/BOT_TASKS.md tabulates the
// same seven from the registrar 009D2DA0.
// ---------------------------------------------------------------------------
enum class TorpedoState : int {
    kNone = 0,
    kMoveTo = 0x544,
    kFollow = 0x580,
    kDone = 0x618,
    kAttackRun = 0x6B4,
    kGoAway = 0x6D8,
    kAim = 0x710,
    kPrepare = 0x740,
};

// ---------------------------------------------------------------------------
// Task fields the torpedo rules read. Whole-object offsets.
// ---------------------------------------------------------------------------
namespace torpedo_task_off {
inline constexpr int kCurrentState = 0x2E4 + 0x2C;  // 0x310, task->+310h
inline constexpr int kPlanStepScratch = 0x49C;      // the arm sets 0FFh at 009D4865
inline constexpr int kPlanStepResult = 0x2E4;       // copied back at 009D4905
inline constexpr int kApproach = 0x3F8;             // the approach controller
inline constexpr int kUnit = 0x3FC;                 // the ECX of every 007BBBA0
inline constexpr int kPilotControlBlock = 0x404;    // approach->+0Ch, unit+9D4h
inline constexpr int kRoundsPending = 0x424;        // the arm's manual passthrough budget
inline constexpr int kEngageTarget = 0x4C4;         // 009D3210 step 1
inline constexpr int kEngageRange = 0x484;          // 009D3210 step 3
inline constexpr int kEngageRangeLimit = 0x488;     // 009D3210 step 3
inline constexpr int kAimFlag = 0x529;              // attackrun -> aim, and 009D3210
inline constexpr int kAttackFlag = 0x52A;           // the arming gate and goaway -> done
// prepare (+740h) + the follow drop gate (+98h). The task is 7DCh, so this is
// its last dword: 009D49C2 writes it as [ESI+7D8h].
inline constexpr int kPrepareDropTimer = 0x740 + 0x98;
}  // namespace torpedo_task_off

// ---------------------------------------------------------------------------
// Approach fields the torpedo rules read, approach-relative (task+3F8h).
// ---------------------------------------------------------------------------
namespace torpedo_approach_off {
inline constexpr int kMoveToRangeSpread = 0x74;   // 009D48AF
inline constexpr int kMoveToRangeBase = 0x78;     // 009D48AC
inline constexpr int kSpeedLow = 0x7C;            // chosen when +134h >= 15.0
inline constexpr int kSpeedHigh = 0x80;           // chosen otherwise
inline constexpr int kRangeMetric = 0x90;         // 009D3150 compares against it
inline constexpr int kRunInHeading = 0x94;        // the attackrun heading base
inline constexpr int kWeaponSelect = 0xA4;        // set to 3 by 009D2720
inline constexpr int kSurfaceRunner = 0x132;      // byte, gates 009D31B0 and 009D3150
inline constexpr int kSpeedSwitch = 0x134;        // compared against the double 15.0
}  // namespace torpedo_approach_off

// ---------------------------------------------------------------------------
// The constants the release rules read, by address. Values from the image.
// ---------------------------------------------------------------------------
namespace torpedo_constant {
inline constexpr float kSpeedSwitchThreshold = 15.0f;    // 00CF3F20, a double
inline constexpr float kCommittedThrottle = 0.3f;        // 00CE69C8
inline constexpr float kDisarmed = -1.0f;                // 00D7A260
inline constexpr float kExitDropThreshold = 0.0f;        // 00D7A218
inline constexpr float kAbortTimeToTarget = 2.5f;        // 00CF87C8
inline constexpr float kAbortBearing = 1.3962635f;       // 00CF8858, 80 degrees
inline constexpr float kCloseTimeToTarget = 1.5f;        // 00CE380C
inline constexpr float kConeNearTime = 0.5f;             // 00CE3800
inline constexpr float kConeNearAngle = 1.0471976f;      // 00D05AAC, 60 degrees
inline constexpr float kConeFarTime = 1.2f;              // 00CE3814
inline constexpr float kConeFarAngle = 0.2617994f;       // 00D05AA8, 15 degrees
inline constexpr float kMaxBankAtRelease = 0.52359879f;  // 00CEC724, 30 degrees
// The gated helper 009D25A0 uses a second cone over the same time metric.
inline constexpr float kHelperConeNearTime = 1.0f;       // the 3F800000 immediate
inline constexpr float kHelperConeNearAngle = 1.0471976f;  // 00D05AAC
inline constexpr float kHelperConeFarTime = 2.0f;        // 00CE3958
inline constexpr float kHelperConeFarAngle = 0.52359879f;  // 00CEC724
inline constexpr float kHelperMaxBank = 0.61086529f;     // 00CE74F4, 35 degrees
// 009D49A0 arms the prepare countdown with this. 00CE3D34 was not decoded to a
// value here; the host supplies it. See docs/TORPEDO_TASK_ARM.md.
}  // namespace torpedo_constant

// ---------------------------------------------------------------------------
// 009D31D0, the in-attack-state test. __thiscall(task, state), the torpedo
// analogue of the depth charge's 009A5420. Five pointer compares, no call.
// ---------------------------------------------------------------------------
bool torpedo_in_attack_state_009d31d0(TorpedoState state) noexcept;

// ---------------------------------------------------------------------------
// 009D3210, the engaged predicate. The torpedo analogue of the depth charge's
// inline `task->+46Dh != 0 || (ctl->+370h == 2 && task->+48Ch != 0)`.
// ---------------------------------------------------------------------------
struct TorpedoEngagedInputs {
    bool aim_flag_529 = false;         // task->+529h
    bool has_engage_target_4c4 = false;  // task->+4C4h != 0
    int pilot_control_mode_370 = 0;    // ctl->+370h, the three-valued mode
    bool unit_has_no_follow_target = false;  // 007B8AD0(unit)
    float engage_range_484 = 0.0f;     // task->+484h
    float engage_range_scale = 1.0f;   // 00D05AC8
    float engage_range_limit_488 = 0.0f;  // task->+488h
};
bool torpedo_engaged_009d3210(const TorpedoEngagedInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009D3F60, the entry chooser. The analogue of the depth charge's 009A57D0.
// ---------------------------------------------------------------------------
struct TorpedoEntryInputs {
    int pilot_control_mode_370 = 0;  // ctl->+370h
    bool attack_flag_52a = false;    // task->+52Ah
    bool control_flag_369 = false;   // ctl->+369h
    bool global_e17bf2 = false;      // the byte at 00E17BF2
    bool aim_flag_529 = false;       // task->+529h
};
TorpedoState torpedo_entry_state_009d3f60(const TorpedoEntryInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009D4030, the transition rule, called once per tick from the arm at 009D48DE
// before the current state's own tick. Returns kNone to stay put.
//
// The four predicates it consults are supplied by value, because three of them
// (009D31B0, 009D3150, task->vtable[1Ch] == 009D4C10) read objects this rule
// does not otherwise touch.
// ---------------------------------------------------------------------------
struct TorpedoTransitionInputs {
    TorpedoState current = TorpedoState::kNone;
    TorpedoEngagedInputs engaged;
    TorpedoEntryInputs entry;
    bool unit_has_no_follow_target = false;  // 007B8AD0(unit)
    bool should_break_off = false;           // task->vtable[1Ch], 009D4C10
    bool aim_hold_009d31b0 = false;          // 009D31B0: aim->+2Ch when approach->+132h
    bool goaway_done_009d3150 = false;       // 009D3150
};
TorpedoState torpedo_next_state_009d4030(const TorpedoTransitionInputs& in) noexcept;

// ---------------------------------------------------------------------------
// The goaway state, task+6D8h (approach+2E0h), vtable 00D212E0.
// 009D2ECC-009D2F09 constructs it inline in the registrar 009D2DA0 and writes
// +4h, +8h, +Ch, +10h, +14h, +18h, +1Ch, +20h, +28h, +30h and +34h. It does NOT
// write +24h, and the vtable's slot +8h (009D0C00) is a bare RET. The producer
// of +24h is slot +4h, the enter 009D0D90, which stores it at 009D0E37. An
// exhaustive disassembly of 009D0000-009D5000 finds exactly one non-ESP store
// to a +24h slot in the whole torpedo band, and that is it.
// ---------------------------------------------------------------------------
namespace torpedo_goaway {
inline constexpr float kDistanceJitterLo = 1.0f;   // FLD1 at 009D0E27
inline constexpr float kDistanceJitterHi = 1.15f;  // 00D20CE4, 009D0E15
inline constexpr float kSideLeft = -1.0f;          // 00D7A260, 009D0DA9
inline constexpr float kSideRight = 1.0f;          // 00D7A24C, 009D0DB3
inline constexpr float kOrderedScale = 0.4f;       // 00CE65D0, 009D3183
}  // namespace torpedo_goaway

struct TorpedoGoAwayEnterInputs {
    // 0042E740()+438h, `Pilot/Torpedo/SafeDist`, default 700.
    float safe_distance_438 = 0.0f;
    // 007B5BE0(target) when approach+CCh answers vtable[5Ch](5): the larger of
    // the target's +444h/+448h extents. 0 when there is no such target.
    float target_extent = 0.0f;
    bool has_extent_target = false;         // 009D0DDF, 009D0DEE
    // BSP_Random_UniformFloatRange(1.0, 1.15) at 009D0E2C.
    float distance_jitter = 1.0f;
    // [00F876B0] & 1 at 009D0D98: the alternating break-off side.
    bool side_bit = false;
};

struct TorpedoGoAwayState {
    float break_off_distance_24 = 0.0f;  // 009D0E37
    float break_off_side_2c = 1.0f;      // 009D0DBC
};

// 009D0D90-009D0E3A, the goaway enter (vtable 00D212E0 slot +4h).
TorpedoGoAwayState torpedo_goaway_enter_009d0d90(
    const TorpedoGoAwayEnterInputs& in) noexcept;

struct TorpedoGoAwayCompleteInputs {
    float range_90 = 0.0f;               // approach+90h, 009D318F
    float break_off_distance_24 = 0.0f;  // goaway+24h, 009D3151
    bool has_ordnance_132 = false;       // approach+132h, 009D3173
    bool control_flag_369 = false;       // ctl+369h, 009D315C
    bool global_e17bf2 = false;          // [00E17BF2], 009D316A
};

// 009D3150-009D31A5. True when the aircraft has opened past its break-off
// distance. When both the control flag and the global are set the distance is
// scaled by 0.4 and a missing ordnance byte refuses outright.
bool torpedo_goaway_complete_009d3150(
    const TorpedoGoAwayCompleteInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009D49A0, task vtable slot +24h: the arming entry. BSP_PilotBot_Tick walks
// the bot's task vector at 0099AF90-0099AFAF and calls this slot on each task,
// stopping at the first that returns true and spending one unit->+C58h.
//
// This is the single gate that raises prepare->+98h, the countdown the
// done/prepare tick and its exit slot both drop on.
// ---------------------------------------------------------------------------
struct TorpedoArmInputs {
    bool attack_flag_52a = false;
    TorpedoState current = TorpedoState::kNone;
    TorpedoEngagedInputs engaged;
    float arm_countdown_00ce3d34 = 0.0f;  // the value 009D49A0 stores
};
struct TorpedoArmResult {
    bool consumed = true;        // the return value; true spends one unit->+C58h
    bool armed = false;          // prepare->+98h was written
    float prepare_timer = 0.0f;  // the value written when armed
    bool fell_through_to_0099b6a0 = false;  // the base slot +24h ran instead
};
TorpedoArmResult torpedo_arm_drop_009d49a0(const TorpedoArmInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009D25A0, the gated release helper. __thiscall(state, bool force), RET 4.
// Called once, from the done/prepare tick at 009D27A6 with force = 0.
// The release site inside it is 009D26F8; the forced one is 009D25B3.
// ---------------------------------------------------------------------------
struct TorpedoReleaseGateInputs {
    bool force = false;
    float time_to_target = 0.0f;  // 009D1500(approach)
    float bearing_error = 0.0f;   // |SubtractWrappedAngle(bearing, heading)|
    float unit_bank_c68 = 0.0f;   // unit->+C68h
};
bool torpedo_release_gate_009d25a0(const TorpedoReleaseGateInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009D2570, the done/prepare exit slot (+8h of vtable 00D21320). Leaving the
// state while the countdown is still armed forces the drop.
// ---------------------------------------------------------------------------
bool torpedo_exit_drop_009d2570(float drop_timer_98) noexcept;

// ---------------------------------------------------------------------------
// 009D2720, the done/prepare tick (+Ch of vtable 00D21320), 009D2720-009D2CF1
// inclusive, RET 4. This is where the release range is established.
// ---------------------------------------------------------------------------
enum class TorpedoDoneTickOutcome : int {
    kIdle = 0,        // no target, or the countdown is disarmed and the gates refused
    kSteerToTarget,   // 009D29B7, the approach's own steering helper
    kHoldHeading,     // 009D2990, cmd->+2C4h = 0 and cmd->+2CCh = 1
    kReleaseTimeout,  // 009D27A6 -> 009D26F8: the countdown expired
    kReleaseInCone,   // 009D2938: close, inside the cone, wings level enough
    kReleaseAbort,    // 009D29CB: the solution decayed, drop anyway
};
struct TorpedoDoneTickInputs {
    float dt = 0.0f;
    float drop_timer_98 = -1.0f;   // state->+98h
    bool has_target = false;       // ctl->+3D0h != 0
    float time_to_target = 0.0f;   // 009D1500(approach)
    float bearing_error = 0.0f;    // |SubtractWrappedAngle(bearing, heading)|
    float unit_bank_c68 = 0.0f;    // unit->+C68h
};
struct TorpedoDoneTickResult {
    TorpedoDoneTickOutcome outcome = TorpedoDoneTickOutcome::kIdle;
    bool released = false;
    bool consumed_round = false;  // approach->+2Ch -= 1
    float drop_timer_98 = -1.0f;  // the value left in state->+98h
    bool wrote_committed_command = false;  // weapon select 3 and throttle 0.3
};
TorpedoDoneTickResult torpedo_done_prepare_tick_009d2720(
    const TorpedoDoneTickInputs& in) noexcept;

// ---------------------------------------------------------------------------
// The arm's own two arithmetic steps, 009D4874-009D48CF.
// ---------------------------------------------------------------------------
float torpedo_arm_speed_009d4886(float speed_switch_134, float speed_low_7c,
                                 float speed_high_80) noexcept;
float torpedo_arm_move_to_range_009d48ac(float range_base_78,
                                         float range_spread_74) noexcept;

// ---------------------------------------------------------------------------
// 009D48F8-009D495B, the manual-release passthrough at the tail of the arm.
// The torpedo list omits `turnto`, which the class does not have.
// ---------------------------------------------------------------------------
bool torpedo_manual_passthrough_009d4956(int rounds_pending_424, bool has_unit,
                                         bool device_requests_release,
                                         TorpedoState current) noexcept;

// ---------------------------------------------------------------------------
// The host. One virtual per native call site the arm sequence reaches, on top
// of the shared BotTaskStateHost of include/bsp/bot_task_states.hpp.
// ---------------------------------------------------------------------------
struct TorpedoTaskHost : BotTaskStateHost {
    // 009D3420 at 009D486F, ECX = task+3F8h, RET 4. The torpedo approach
    // update, the analogue of the depth charge's 009A5F50. 009D3420-009D3E3F,
    // 2592 bytes. contract: unread body.
    virtual void update_torpedo_approach_009d3420(void* approach, float dt) = 0;

    // 009D4030 at 009D48DE, ECX = task, RET 4. Reconstructed above; the host
    // supplies the predicate values the rule cannot compute.
    virtual TorpedoTransitionInputs read_transition_inputs(void* task) = 0;
    virtual void set_state(void* task, TorpedoState next) = 0;

    // 009D1500 at 009D289A and 009D160A, ECX = approach. The time-to-target
    // metric every cone is keyed on: 009D2A44-009D2A52 recomputes it inline as
    // planar distance / ((approach->+14h)->+8h * approach->+24h).
    virtual float time_to_target_009d1500(void* approach) = 0;

    // The bearing chain at 009D27EB-009D2890: approach->vtable[0] for the
    // target point, LIBCRT_atan2, unit->vtable[50h] for the heading and
    // BSP_Math_SubtractWrappedAngle, then the |x| fold at 009D2882.
    virtual float bearing_error_to_target(void* approach, void* unit) = 0;

    // unit->+C68h, the bank the two release cones cap.
    virtual float unit_bank_c68(const void* unit) = 0;

    // 009D1360 at 009D27D1, ECX = approach. contract: unread body.
    virtual void approach_committed_hook_009d1360(void* approach) = 0;

    // 009C1FD0 at 009D2731, ECX = state. The follow base's own tick, which the
    // done/prepare tick runs before anything else. contract: unread body.
    virtual void follow_base_tick_009c1fd0(void* state, float dt) = 0;

    // 009F9E40 at 009D29B7 with the target point from approach->vtable[0].
    // The steering helper the tick uses while the countdown is above 1.0f.
    virtual void steer_toward_target_009f9e40(void* approach) = 0;

    // approach->+1Ch->+40h = 0 at 009D2770 and 009D2B34, alongside the
    // weapon selector. contract: unread field.
    virtual void clear_approach_1c_field40(void* approach) = 0;

    // prepare->+98h, the countdown. 009D49A0 raises it; 009D2720 spends it.
    virtual float read_drop_timer(void* prepare_state) = 0;
    virtual void write_drop_timer(void* prepare_state, float value) = 0;

    // ctl->+3D0h != 0, the target the done/prepare tick needs.
    virtual bool pilot_control_has_target(void* pilot_control_block) = 0;

    // task->+424h, the manual-release budget.
    virtual int rounds_pending(void* task) = 0;
    virtual void spend_pending_round(void* task) = 0;

    // task->+49Ch at 009D4865 and the copy back to task->+2E4h at 009D4905.
    virtual void set_plan_step_scratch(void* task, int value) = 0;
    virtual void commit_plan_step_result(void* task) = 0;

    // 009D4886/009D4890 and 009D48AC, read off the approach by the arm itself.
    virtual float approach_speed_for_arm(void* approach) = 0;
    virtual float approach_move_to_range_for_arm(void* approach) = 0;

    // task+544h, the moveto state 009BDE80 is called on at 009D48CF.
    virtual void* move_to_state(void* task) = 0;
};

// ---------------------------------------------------------------------------
// The context the arm sequence works over.
// ---------------------------------------------------------------------------
struct TorpedoTaskContext {
    void* task = nullptr;
    void* approach = nullptr;
    void* unit = nullptr;
    void* command_block = nullptr;       // approach->+18h == task+4h
    void* pilot_control_block = nullptr;  // approach->+0Ch == unit+9D4h
    void* state = nullptr;               // task->+310h, the object
    TorpedoState current = TorpedoState::kNone;
    void* prepare_state = nullptr;       // task+740h
};

// ---------------------------------------------------------------------------
// 009D4850, task vtable slot +64h, the per-tick arm. Body 009D4850-009D4965
// inclusive, RET 4, `void __thiscall(task, float dt)`.
// ---------------------------------------------------------------------------
struct TorpedoArmTickResult {
    TorpedoState state_before = TorpedoState::kNone;
    TorpedoState state_after = TorpedoState::kNone;
    bool transitioned = false;
    TorpedoDoneTickResult done_tick;   // valid when the state was done or prepare
    bool ran_done_tick = false;
    bool manual_release = false;       // the 009D4956 passthrough fired
    int releases = 0;                  // 007BBBA0 requests this tick
    float move_to_range = 0.0f;
    float move_to_speed = 0.0f;
};
TorpedoArmTickResult torpedo_task_arm_009d4850(TorpedoTaskHost& host,
                                               TorpedoTaskContext& ctx,
                                               float dt);

}  // namespace bsp

#endif  // BSP_TORPEDO_TASK_ARM_HPP
