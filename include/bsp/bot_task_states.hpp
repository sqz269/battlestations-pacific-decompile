#pragma once
#include <cstdint>

// Projection of the bot task state objects behind BotTask+310h.
// docs/BOT_TASK_STATES.md carries the evidence; docs/BOT_TASKS.md owns the task object,
// the thirteen classes, the 26-slot task interface and the state-name tables.
//
// Every name here is a hypothesis, not a recovered symbol. Nothing in this header is a
// binary-compatible layout: the offset constants are the native ones, the structs are not.
//
// Contracts named but not reconstructed: the pilot bot itself (0099D300 PilotBot_PlanControls
// and 009998A0, docs/UNIT_COMMAND_PRODUCERS.md) consumes the command block this header writes;
// the unit motion controller behind unit+9D4h; the device at unit+DECh that 007BBBA0 flags and
// the projectile spawn behind it (docs/PROJECTILE_KINDS.md); the effects, sound and session
// singletons. None of them is modelled here.

namespace bsp {

// ---------------------------------------------------------------------------
// The state record. The base is six words; +18h onwards belongs to the derived
// class. 009C2AC0, 009A36A0 and 009D06D0 write the base identically, and the
// five depth-charge states 009A4DC0 builds inline at 009A4EC1..009A4FC4 agree.
// ---------------------------------------------------------------------------
namespace bot_task_state_off {
inline constexpr int kVtable = 0x00;          // every constructor
inline constexpr int kOwnerApproach = 0x04;   // 009C2AD4, 009A36C6, 009D06E6, 009A4EC1
inline constexpr int kReserved08 = 0x08;      // zeroed; contract: unread
inline constexpr int kReserved0C = 0x0C;      // zeroed; contract: unread
inline constexpr int kReserved10 = 0x10;      // zeroed; contract: unread
inline constexpr int kReserved14 = 0x14;      // zeroed; contract: unread
inline constexpr int kBaseSize = 0x18;

// The attackrun derived shape, 009A36A0 (depth charge) and 009D06D0 (torpedo).
inline constexpr int kAttackRunPeriod = 0x18;        // 1.0f from 00D7A24C
inline constexpr int kAttackRunCountdown = 0x1C;     // -(00BD2F10(0.0f, 1.0f)), a random stagger
inline constexpr int kAttackRunHeadingOffset = 0x20; // re-rolled on every expiry

// The depth-charge aim shape, built inline at 009A4F01..009A4F20.
inline constexpr int kAimPullOutFlag = 0x18;       // byte, set by 009A4588
inline constexpr int kAimByte19 = 0x19;            // byte; contract: unread
inline constexpr int kAimReleaseCooldown = 0x1C;   // float, decremented by the tick

// The moveto/follow shape, 009C2AC0 and 009C2980.
inline constexpr int kMoveToObserverVtable = 0x18; // a second vtable, 00D20AD4 for moveto
inline constexpr int kMoveToTarget = 0x2C;         // param_3, also the observer registration
inline constexpr int kMoveToNearRange = 0x30;      // param_4
inline constexpr int kMoveToFarRange = 0x34;       // param_5
inline constexpr int kMoveToMode = 0x38;           // param_6
inline constexpr int kFollowDropGate = 0x98;       // 009A4E9C / 009D2F83; the exit-slot drop gate
inline constexpr int kFollowField9C = 0x9C;        // zeroed by the same writers
}  // namespace bot_task_state_off

// ---------------------------------------------------------------------------
// The state interface. Seven slots, +0h..+18h; the moveto family adds +1Ch.
// Read from 00D1F64C, 00D1F668, 00D1F684, 00D1F6A0, 00D1F6D8, 00D1F6FC,
// 00D212C4, 00D212E0, 00D212FC, 00D21320 and 00D20AEC.
// ---------------------------------------------------------------------------
namespace bot_task_state_vtable {
inline constexpr int kScalarDeletingDtor = 0x00;
inline constexpr int kEnter = 0x04;             // 009A56C0 calls it on the incoming state
inline constexpr int kExit = 0x08;              // 009A56C0 calls it on the outgoing state
inline constexpr int kTick = 0x0C;              // __thiscall(this, float dt), RET 4
inline constexpr int kPredicate10 = 0x10;       // base 007B3DE0 returns true; contract: unread
inline constexpr int kSlot14 = 0x14;            // base 007B3DF0 is empty; contract: unread
inline constexpr int kPredicate18 = 0x18;       // base 007B45E0 returns false; contract: unread
inline constexpr int kSetDesiredSpeed = 0x1C;   // moveto/follow only, 009C1850
inline constexpr int kBaseSlotCount = 7;
inline constexpr int kMoveToSlotCount = 8;
}  // namespace bot_task_state_vtable

// ---------------------------------------------------------------------------
// The approach controller the states are embedded in, at BotTask+3F8h.
// The first eleven words come from the shared head 009F9CE0 (docs/BOT_TASKS.md);
// +18h..+20h are filled afterwards by 009F9980, and the rest is per class.
// ---------------------------------------------------------------------------
namespace bot_approach_off {
inline constexpr int kVtable = 0x00;             // four slots at 00D21C74
inline constexpr int kUnit = 0x04;               // the ECX of every 007BBBA0 release
inline constexpr int kUnitClassBlock = 0x08;     // unit->+538h
inline constexpr int kPilotControlBlock = 0x0C;  // unit->+9D4h, the altitude limits
inline constexpr int kCommandBlock = 0x18;       // 009F9980: task+4h, the per-tick output
inline constexpr int kTaskSub314 = 0x1C;         // 009F9980: task+314h
inline constexpr int kTaskSub38C = 0x20;         // 009F9980: task+38Ch
inline constexpr int kSpeedRatio = 0x24;         // max(1.0f, classBlock->+188h / reference speed)

// Depth-charge fields the release rule reads. 009A3390 writes +34h, +3Ch and +58h.
inline constexpr int kRoundsRemaining = 0x2C;    // decremented by both release rules
inline constexpr int kAimAltSpread = 0x34;       // 00BD2F10(0, AimAltRange/2 - AimAltRange/1)
inline constexpr int kAimAltBase = 0x3C;         // Pilot/DepthCharge/AimAltRange/1 = 20
inline constexpr int kRunInHeading = 0x4C;       // the run-in heading the attackrun offsets
inline constexpr int kPullOutMetric = 0x50;      // compared against 00CE380C by the aim tick
inline constexpr int kReleaseBase = 0x58;        // the constant 00CE38C8
inline constexpr int kReleaseBias = 0x5C;        // per tick, written by 009A5F50; contract: unread
inline constexpr int kRunInParam = 0x60;         // per tick; every interpolation is keyed on it
inline constexpr int kHasOrdnance = 0x74;        // byte, from BSP_WeaponController_HasDepthChargeOrdnance
inline constexpr int kArmRequest = 0x76;         // byte, set by the aim tick inside the drop band
inline constexpr int kWeaponSelect = 0x78;       // set to 3 alongside the arm request
inline constexpr int kLatchedTarget = 0x94;      // the release chain's first test
inline constexpr int kStateRegistry = 0xFC;      // the {name, state} vector 00411E70 appends to
}  // namespace bot_approach_off

// ---------------------------------------------------------------------------
// The command block at BotTask+4h, which approach->+18h points at. This is what
// 0099D300 PilotBot_PlanControls consumes; that routine is a peer's contract.
// ---------------------------------------------------------------------------
namespace pilot_command_off {
inline constexpr int kFloat278 = 0x278;       // 009A3770 writes 00CE6650, 009D07B0 writes 1.0f
inline constexpr int kFlag27C = 0x27C;        // byte, set to 1 with the above
inline constexpr int kWord2A8 = 0x2A8;        // both attackrun ticks write 0
inline constexpr int kFlag2AC = 0x2AC;        // byte, set to 1 with the above
inline constexpr int kSpeedOverride = 0x2B0;  // byte, cleared when a speed is written
inline constexpr int kDesiredSpeed = 0x2B4;   // 009C1850 and 009A4010
inline constexpr int kWord2BC = 0x2BC;        // cleared by the no-target branch of 009C18C0
inline constexpr int kDesiredHeading = 0x2C0; // an absolute wrapped angle
inline constexpr int kWord2C4 = 0x2C4;        // cleared by the no-target branch of 009C18C0
inline constexpr int kInterp2C8 = 0x2C8;      // 009A4010 only
inline constexpr int kHeadingMode = 0x2CC;    // 1 with no target, 2 with a heading
inline constexpr int kMode2D0 = 0x2D0;        // 2, written by 009C18C0
inline constexpr int kSpeedValid = 0x2D8;     // 1 from the speed writers, 0 from attackrun
}  // namespace pilot_command_off

// ---------------------------------------------------------------------------
// The depth-charge state set. Whole-object offsets from BotTask, as
// docs/BOT_TASKS.md records them; 009A5870 compares these pointers.
// ---------------------------------------------------------------------------
enum class DepthChargeState : int {
    kUnchanged = -1,
    kMoveTo = 0,     // task+508h
    kFollow,         // task+544h
    kDone,           // task+5DCh
    kAttackRun,      // task+67Ch
    kGoAway,         // task+6A0h
    kAim,            // task+6C4h
    kPrepare,        // task+6E4h
    kLeave,          // task+784h
    kTurnTo,         // task+7A0h
    kCount
};

// The task-relative offset of each depth-charge state object.
int depth_charge_state_offset(DepthChargeState state);

// The registered name of each depth-charge state, the literal 009A3090 passes to
// 00411E70. Debug text only: no code looks a state up by name.
const char* depth_charge_state_name(DepthChargeState state);

// True for the seven states 009A5420 accepts, i.e. everything but moveto and follow.
bool depth_charge_is_attack_state(DepthChargeState state);

// ---------------------------------------------------------------------------
// The transition rule, as pure functions over explicit inputs.
// ---------------------------------------------------------------------------

// The inputs 009A5870 and 009A57D0 read, all off the task, its approach and the
// pilot control block. Names are hypotheses.
struct DepthChargeTransitionInputs {
    DepthChargeState current = DepthChargeState::kMoveTo;
    int control_mode = 0;          // ctl->+370h; 0, 2 and "other" are the three cases
    bool flag_46C = false;         // task->+46Ch, read by 009A57D0 and by the goaway branch
    bool flag_46D = false;         // task->+46Dh, the "engaged" half and the attackrun exit
    bool has_latched_target = false; // task->+48Ch != 0
    bool unit_has_no_follow_target = true; // 007B8AD0(unit), i.e. unit->+9D8h == 0
    bool should_break_off = false; // task->vtable[+1Ch](), 009A65F0 for this class
    bool approach_has_ordnance = false; // approach->+74h
    bool aim_pull_out = false;     // the aim state's +18h byte
    float leave_countdown = 0.0f;  // the leave state's +18h float
    bool turn_to_aim_ready = false;   // 009A5610(); contract: unread body
    bool turn_to_give_up = false;     // 009A5390(); contract: unread body
    bool go_away_finished = false;    // 009A53B0(); contract: unread body
};

// 009A57D0, complete. The entry into the attack chain.
DepthChargeState depth_charge_entry_state(const DepthChargeTransitionInputs& in);

// 009A5870, complete. Returns kUnchanged when the rule leaves the state alone.
DepthChargeState depth_charge_next_state(const DepthChargeTransitionInputs& in);

// ---------------------------------------------------------------------------
// The run-in and release rules, as pure functions.
// ---------------------------------------------------------------------------

// 009A3770 and 009D07B0 head: the attackrun heading is re-rolled when the
// countdown expires, and the countdown is refilled by one period.
bool attack_run_should_reroll(float countdown, float dt);
float attack_run_next_countdown(float period, float countdown, float dt);

// 009A3FA0, complete: the depth-charge release window.
bool depth_charge_in_release_window(float run_in_param, float release_base, float release_bias);

// 009A45A3: the aim tick only arms inside a band below the aim altitude.
bool depth_charge_in_drop_band(float altitude, float release_base, float release_bias,
                               double band_00ce4d70);

// 009A45B8..009A4619, complete: every term of the depth-charge release conjunction.
struct DepthChargeReleaseInputs {
    bool has_latched_target = false;  // approach->+94h != 0
    bool target_engageable = false;   // target->+5Dh == 0
    bool has_ordnance = false;        // approach->+74h != 0
    float release_cooldown = 0.0f;    // the aim state's +1Ch; must be strictly negative
    bool in_release_window = false;   // 009A3FA0
    float unit_altitude = 0.0f;       // BSP_EntityPose_GetWorldPositionRefreshed(unit).y
    float aim_altitude = 0.0f;        // approach->+3Ch + approach->+34h
    double altitude_margin = 0.0;     // 00CE3DD8
};
bool depth_charge_should_release(const DepthChargeReleaseInputs& in);

// 009A48D0 and 009D2570, complete: leaving done or prepare drops when the gate
// has been raised above 00D7A218. The constructor leaves it at -1.0f.
bool prepare_done_exit_drops(float drop_gate, double threshold_00d7a218);

// 009C18C0 steps 1 and 5, complete for the arithmetic they contain.
bool move_to_arrived(float dx, float dz, double threshold_00ce3820);
float move_to_desired_speed(float dx, float dz, double threshold_00ce3820);
float move_to_target_altitude(float far_range, float target_altitude, float near_range);

// 009A66C0 step 7: the manual-release passthrough fires only outside the four
// states that own the release themselves.
bool manual_release_allowed(DepthChargeState current);

// ---------------------------------------------------------------------------
// The host. One virtual per native call site the sequences below reach.
// Pattern: include/bsp/app_frame.hpp and include/bsp/bot_tasks.hpp.
// ---------------------------------------------------------------------------
struct BotTaskStateHost {
    virtual ~BotTaskStateHost() = default;

    // 00411E70 at 009A30A8 and the eight siblings, ECX = approach+FCh.
    // The body is one 00411D90 push_back of an eight-byte {name, state} pair.
    virtual void register_state_name(void* registry, const char* name, void* state) = 0;

    // state->vtable[+8h] at 009A56DB, on the outgoing state.
    virtual void exit_state(void* state) = 0;

    // state->vtable[+4h] at 009A52C7 and 009A56EA, on the incoming state.
    virtual void enter_state(void* state) = 0;

    // state->vtable[+Ch] at 009A6741, on task->+310h.
    virtual void tick_state(void* state, float dt) = 0;

    // state->vtable[+1Ch] inside 009C18C0, the moveto family's speed slot.
    virtual void set_desired_speed(void* state, float speed) = 0;

    // 009A5F50 at 009A66DF, on task+3F8h. Refreshes the target, the ordnance flag
    // at approach+74h and the run-in values at approach+5Ch/+60h. coverage: partial.
    virtual void update_approach(void* approach, float dt) = 0;

    // 009BDE80 at 009A671A, on the moveto state. contract: unread body.
    virtual void refresh_move_to_ranges(void* move_to_state, float near_range, float far_range,
                                        float third) = 0;

    // 007B8AD0 at 009A52A7 and inside 009A5870. Returns unit->+9D8h == 0, so the
    // Ghidra name BSP_Unit_HasFollowTarget on it is inverted.
    virtual bool unit_has_no_follow_target(const void* unit) = 0;

    // task->vtable[+1Ch] inside 009A5870; 009A65F0 for the depth charge.
    virtual bool should_break_off(void* task) = 0;

    // (*(unit+72Ch))->vtable[+38h] at 009A6771. contract: unread body.
    virtual bool manual_release_requested(void* unit) = 0;

    // 007BBBA0 at 009A4620, 009A48EA, 009A67AB, 009D258A and 009D25B3, ECX = unit.
    // Raises dev->+11h on unit->+DECh and increments unit->+C20h; it spawns nothing.
    virtual void request_ordnance_release(void* unit) = 0;

    // 00427EB0 BSP_EntityPose_GetWorldPositionRefreshed at 009A45FB, ECX = unit.
    virtual float unit_altitude(const void* unit) = 0;

    // 00BD2F10 at 009A4646 with ECX = 1, and in the three state constructors.
    // The distribution is unread; only the two bounds are established.
    virtual float random_between(float low, float high) = 0;

    // 007F0280 inside 009A3770 and 009D07B0. The depth charge passes 1 as the last
    // argument and the torpedo 0. contract: unread body.
    virtual float sample_heading_offset(void* unit, int variant) = 0;

    // BSP_Math_AddWrappedAngle inside 009A3770, 009A4010 and 009D07B0.
    virtual float add_wrapped_angle(float base, float delta) = 0;

    // 009FBA50 inside 009C18C0, 009A3770, 009A4010 and 009D07B0. contract: unread body.
    virtual void command_altitude_and_throttle(void* approach, float altitude, float a, float b,
                                               float throttle) = 0;

    // The command block writes, all on approach->+18h == task+4h.
    virtual void write_command_word(void* command_block, int offset, int value) = 0;
    virtual void write_command_byte(void* command_block, int offset, unsigned char value) = 0;
    virtual void write_command_float(void* command_block, int offset, float value) = 0;

    // approach->+76h and +78h, the arm request and weapon selector of the aim tick.
    virtual void set_arm_request(void* approach, bool armed) = 0;
    virtual void set_weapon_selector(void* approach, int selector) = 0;

    // approach->+2Ch, decremented by both release rules.
    virtual void consume_round(void* approach) = 0;
};

// ---------------------------------------------------------------------------
// The sequences, one per read state body. Each is the native order of the call
// sites above; nothing here is binary compatible.
// ---------------------------------------------------------------------------

// A view of the objects a sequence needs, gathered so the sequences stay free of
// pointer arithmetic on the native layouts.
struct BotTaskStateContext {
    void* task = nullptr;
    void* approach = nullptr;
    void* unit = nullptr;
    void* command_block = nullptr;  // approach->+18h
    void* state = nullptr;          // task->+310h
};

// 009A56C0, complete.
void set_state(BotTaskStateHost& host, BotTaskStateContext& ctx, void* next);

// 009A66C0, the depth-charge slot +64h, complete.
void depth_charge_tick(BotTaskStateHost& host, BotTaskStateContext& ctx,
                       const DepthChargeTransitionInputs& in, float dt, int rounds_remaining,
                       void* const state_objects[static_cast<int>(DepthChargeState::kCount)]);

// 009A3770 and 009D07B0, the two attackrun ticks. They differ only in constants,
// which is why one sequence carries both through `variant`.
struct AttackRunTickInputs {
    float period = 1.0f;
    float countdown = 0.0f;
    float heading_offset = 0.0f;
    float run_in_heading = 0.0f;  // approach->+4Ch for the depth charge, +94h for the torpedo
    float altitude = 0.0f;
    float throttle = 0.0f;
    float command_278 = 0.0f;     // 00CE6650 for the depth charge, 1.0f for the torpedo
    int variant = 1;              // the last argument to 007F0280
};
// Returns the countdown and heading offset the state keeps.
struct AttackRunTickResult {
    float countdown = 0.0f;
    float heading_offset = 0.0f;
};
AttackRunTickResult attack_run_tick(BotTaskStateHost& host, BotTaskStateContext& ctx,
                                    const AttackRunTickInputs& in, float dt);

// 009A4010, the release tail only. coverage: partial, the heading and throttle
// arithmetic of 009A4010-009A4587 is not reproduced.
struct DepthChargeAimTickResult {
    bool pull_out = false;
    bool armed = false;
    bool released = false;
    float release_cooldown = 0.0f;
};
DepthChargeAimTickResult depth_charge_aim_tick_release(BotTaskStateHost& host,
                                                       BotTaskStateContext& ctx,
                                                       const DepthChargeReleaseInputs& release,
                                                       float pull_out_metric,
                                                       double pull_out_threshold_00ce380c,
                                                       double drop_band_00ce4d70,
                                                       float release_cooldown, float dt);

// 009A48D0 and 009D2570, complete. Returns true when the exit dropped a round.
bool prepare_done_exit(BotTaskStateHost& host, BotTaskStateContext& ctx, float drop_gate,
                       double threshold_00d7a218);

// 009C18C0, the no-target branch and the speed slot. coverage: partial.
void move_to_tick(BotTaskStateHost& host, BotTaskStateContext& ctx, bool has_target, float dx,
                  float dz, double arrive_threshold_00ce3820);

}  // namespace bsp
