#pragma once
#include <cstdint>

// Projection of the thirteen bot task classes the factory dispatch 0099A170 builds.
// docs/BOT_TASKS.md carries the evidence; docs/ATTACK_COMMANDS.md owns the dispatch,
// the per-class constant getter at vtable +3Ch and the still-valid predicate at +40h.
//
// Every name here is a hypothesis, not a recovered symbol. Nothing in this header is a
// binary-compatible layout: the offset constants are the native ones, the structs are not.

namespace bsp {

// ---------------------------------------------------------------------------
// The task kind id, the constructor's third argument to the base 0099C6F0.
// Read from the PUSH immediately before each CALL 0099C6F0.
// ---------------------------------------------------------------------------
enum class BotTaskKind : int {
    kCloseToShip = 0x0,  // 009A274C
    kDepthCharge = 0x1,  // 009A525D
    kDogfight = 0x2,     // 009A982D
    kLand = 0x3,         // 009B325C
    kLevelBomb = 0x4,    // 009B79AD
    kDropKamikaze = 0x5, // 009ADC0D
    kDiveBomb = 0x8,     // 009C772D
    kRetreat = 0x9,      // 009C9D1D
    kStrafe = 0xA,       // 009CC24D
    kRocket = 0xB,       // 007B6A5D
    kKamikaze = 0xC,     // 009AEF0D
    kTorpedo = 0xE,      // 009D306D
    kStop = 0xF,         // 009BADE7
};

// Ids 6, 7, Dh and everything above Fh belong to the nine non-attack classes that also
// derive from 0099C6F0 (009BACB0, 009BAD30, 009BAFC0, 009BB640, 009BB910, 009BBAD0,
// 009BCE50, 009C3000, 009CF8E0). Those constructors were not read.

// ---------------------------------------------------------------------------
// Offsets in the native task object. The base class 0099C6F0 owns [0, 3F8h); the
// approach controller 009F9CE0 starts at 3F8h; state objects follow, per class.
// ---------------------------------------------------------------------------
namespace bot_task_off {
inline constexpr int kPrimaryVtable = 0x000;   // 0099C743, then the derived ctor
inline constexpr int kKindId = 0x300;          // 0099C74B, the ctor's third argument
inline constexpr int kUnitScale = 0x304;       // 0099C75D, 1.0f from 00D7A24C
inline constexpr int kInitialAngle = 0x308;    // 0099C76C, negated 00BD2F10(0.0f, 1.0f)
inline constexpr int kClassWord30C = 0x30C;    // 0099C772, the constant 00D05704
inline constexpr int kCurrentState = 0x310;    // 0099C77C zeroes it; step 7 of the ctor sets it
inline constexpr int kSubObject314 = 0x314;    // 0099C790, ctor 009FAAD0
inline constexpr int kDescriptorSpeed = 0x348; // 0099C871, desc->+230h
inline constexpr int kDescriptorClamp = 0x34C; // 0099C864, the 00CE3E20 / 00D05B50 clamp
inline constexpr int kSubObject38C = 0x38C;    // 0099C79C, ctor 009FCEF0
inline constexpr int kOwnerBot = 0x3F4;        // 0099C7CD, the ctor's second argument
inline constexpr int kApproachBase = 0x3F8;    // the approach controller's vtable

// The approach controller, written by 009F9CE0. Whole-object offsets.
inline constexpr int kUnit = 0x3FC;            // the unit, bot+50h
inline constexpr int kUnitClassBlock = 0x400;  // unit->+538h, docs/UNIT_INSTANCE_LAYOUT.md
inline constexpr int kPilotControl = 0x404;    // unit->+9D4h, what every per-tick update writes
inline constexpr int kUnitField408 = 0x408;    // unit->+DF4h
inline constexpr int kDescriptorRow = 0x40C;   // 00F8A30C + row->+34h * 248h + 0Ch
inline constexpr int kSpeedRatio = 0x41C;      // max(1.0f, classBlock->+188h / reference_speed)
inline constexpr int kNegativeOne = 0x420;     // -1.0f from 00D7A260
} // namespace bot_task_off

// The pilot control block at unit+9D4h, the only thing the per-tick update writes.
// Field meanings are read off the ten slot +54h bodies; the consumer was not read.
namespace pilot_control_off {
inline constexpr int kAltSecondCurrent = 0x37C; // tested < 0 before the second write
inline constexpr int kAltCruiseCurrent = 0x380; // tested < 0 before the cruise write
inline constexpr int kAltThirdCurrent = 0x384;  // divebomb and torpedo only
inline constexpr int kAltSecondLocked = 0x38C;  // byte, non-zero suppresses the second write
inline constexpr int kAltCruiseLocked = 0x38D;  // byte, non-zero suppresses the cruise write
inline constexpr int kAltThirdLocked = 0x38E;   // byte, divebomb and torpedo only
inline constexpr int kAltCruise = 0x394;        // the written cruising altitude
inline constexpr int kAltSecond = 0x398;        // the written second altitude
inline constexpr int kAltThird = 0x39C;         // the written third value
inline constexpr int kCruiseOneShot = 0x3A9;    // byte, always cleared by the update
inline constexpr int kSecondOneShot = 0x3AA;    // byte, always cleared by the update
inline constexpr int kThirdOneShot = 0x3AB;     // byte, always cleared by the update
inline constexpr int kDirty = 0x3AD;            // byte, set by any of the three writes
inline constexpr int kLoadoutFlag = 0x369;      // read by the break-off test at slot +1Ch
} // namespace pilot_control_off

// ---------------------------------------------------------------------------
// The 26-slot interface at 00D05708. Only the slots the packet established are named.
// ---------------------------------------------------------------------------
namespace bot_task_vtable {
inline constexpr int kScalarDeletingDtor = 0x00;
inline constexpr int kShouldBreakOff = 0x1C;   // base 0099C230, depthcharge 009A65F0
inline constexpr int kShouldAbandon = 0x38;    // the slot 0099B740 calls
inline constexpr int kCommandClass = 0x3C;     // docs/ATTACK_COMMANDS.md's constant getter
inline constexpr int kStillValid = 0x40;       // docs/ATTACK_COMMANDS.md's predicate
inline constexpr int kStepResult = 0x50;       // three-valued, depthcharge 009A5D80
inline constexpr int kUpdateCruiseProfile = 0x54; // base 0099B660 is empty; ten overrides
inline constexpr int kSlotCount = 26;
} // namespace bot_task_vtable

// The three values slot +50h returns. 009A5D80 returns kApproaching while the current
// state is moveto or follow, and otherwise kAttackingFlagged or kAttacking depending on
// the byte at +46Eh. The consumer was not read, so the two attack values are provisional.
enum class BotTaskStepResult : int {
    kAttacking = 0,
    kApproaching = 1,
    kAttackingFlagged = 2,
};

// ---------------------------------------------------------------------------
// One row per class: what the factory allocates and what the constructor writes.
// ---------------------------------------------------------------------------
struct BotTaskClassRecord {
    BotTaskKind kind;
    std::uint32_t factory;          // the 0099A170 arm
    std::uint32_t constructor;      // 0 for stop, which uses the base directly
    std::uint32_t approach_ctor;    // 0 for stop
    std::uint32_t object_size;      // the operator_new argument
    std::uint32_t primary_vtable;
    std::uint32_t secondary_vtable; // stored at +3F8h; 0 for stop
    std::uint32_t tertiary_vtable;  // 0 for stop
    int tertiary_offset;            // where the tertiary vtable goes; 0 for stop
    int moveto_state;               // whole-object offset; 0 for stop
    int follow_state;               // whole-object offset; 0 for stop
    const char* name;
};

inline constexpr int kBotTaskClassCount = 13;
extern const BotTaskClassRecord kBotTaskClasses[kBotTaskClassCount];

const BotTaskClassRecord* find_bot_task_class(BotTaskKind kind) noexcept;

// ---------------------------------------------------------------------------
// The task record this reconstruction carries. Not the native layout.
// ---------------------------------------------------------------------------
struct BotTaskRecord {
    BotTaskKind kind{BotTaskKind::kStop};
    // +3F4h and +3FCh. Opaque here: the bot and the unit are peers' contracts.
    const void* owner_bot{nullptr};
    const void* unit{nullptr};
    const void* target{nullptr};   // the latched target, at a class-specific offset
    int current_state{0};          // +310h, as a whole-object offset into the task
    float speed_ratio{1.0f};       // +41Ch
    float attack_distance{0.0f};   // the clamped field, +43Ch for depthcharge
    bool attack_flag{false};       // the byte slot +50h keys value 2 on, +46Eh for depthcharge
    bool breakoff_suppressed{false}; // the class extra byte in the slot +1Ch test
};

// ---------------------------------------------------------------------------
// The approach and exit rules, as pure functions.
// ---------------------------------------------------------------------------

// 009F9CE0's speed ratio: max(1.0f, class_block_reference_speed / tuning_reference_speed).
// A reference speed of zero would divide by zero in the native code as well; the caller
// supplies it from the tuning singleton, which never publishes zero for these rows.
float bot_task_speed_ratio(float unit_reference_speed, float tuning_reference_speed) noexcept;

// The attack-distance clamp of slot +54h, step 7: the field only ever grows.
// 009A6500 for depthcharge, 009C8920 for divebomb, 009D4A70 for torpedo, and the
// same three instructions in levelbomb, strafe, dogfight and rocket.
float bot_task_clamp_attack_distance(float current, float tuning_attack_distance,
                                     float speed_ratio) noexcept;

// Steps 4, 5 and 6 of slot +54h, for one of the three altitude channels.
// Returns true when the update writes the altitude and raises the dirty flag.
struct PilotAltitudeChannel {
    bool locked{false};   // the byte at +38Ch/+38Dh/+38Eh
    float current{0.0f};  // the float at +37Ch/+380h/+384h
    bool one_shot{false}; // the byte at +3A9h/+3AAh/+3ABh
};
bool bot_task_altitude_write_applies(const PilotAltitudeChannel& channel) noexcept;

// The break-off test of slot +1Ch, steps 2 and 5. `target_marked` is the byte at +5Dh of
// the latched target; `class_extra` is the per-class condition of step 3.
// 009A65F0 (depthcharge), 009C8A90 (divebomb), 009B8D80 (levelbomb).
bool bot_task_should_break_off(bool base_gate, bool has_target, bool target_marked,
                               bool class_extra, float distance, float tuning_safe_distance,
                               float speed_ratio) noexcept;

// The initial state selector, step 7 of every constructor: 007B8AD0 returns
// unit->+9D8h == 0, and a true result picks moveto over follow.
int bot_task_initial_state(const BotTaskClassRecord& record, bool unit_has_follow_target) noexcept;

// Slot +50h, as 009A5D80 computes it.
BotTaskStepResult bot_task_step_result(bool in_attack_state, bool attack_flag) noexcept;

// ---------------------------------------------------------------------------
// The host: one virtual method per native call site the construction and the
// per-tick update reach. No default implementations.
// ---------------------------------------------------------------------------
struct BotTaskHost {
    virtual ~BotTaskHost() = default;

    // 00BF681B operator_new at 009A6991 and the twelve sibling sites. A null result
    // skips construction entirely and the factory returns null.
    virtual void* allocate_task(std::uint32_t size) = 0;

    // 0099C6F0 at 009A5266 and the eleven sibling sites. Fills [0, 3F8h).
    virtual void construct_base(void* task, const void* bot, int kind_id) = 0;

    // The per-class approach constructor at 009A5281 and siblings, on task+3F8h.
    virtual void construct_approach(void* approach, const void* bot, const void* target) = 0;

    // 009F9CE0 at 009A1D7A, the shared head of every approach constructor.
    virtual void construct_speed_reference(void* approach, const void* unit,
                                           float reference_speed) = 0;

    // 007B8AD0 at 009A52A7, and again at the head of every slot +54h body.
    virtual bool unit_has_follow_target(const void* unit) = 0;

    // state->vtable[4]() at 009A52C7, entering the initial state.
    virtual void enter_state(void* state) = 0;

    // 009F9980 at 009A52CC, on task+3F8h with the task as the argument.
    virtual void register_approach(void* approach, void* task) = 0;

    // 0042E740 BSP_GameTuning_GetSingleton at 009A1D64 and at every constant fetch
    // inside a slot +54h body. Returns the float at the given singleton offset.
    virtual float game_tuning_float(int offset) = 0;

    // 00411E70 at the nine 009A3090 sites and the sibling registrars.
    virtual void register_state_name(const char* name, void* state) = 0;

    // The three altitude writes of slot +54h, into the pilot control block at unit+9D4h.
    virtual void write_pilot_altitude(void* pilot_control, int field_offset, float value) = 0;

    // 0099C230 at the head of every slot +1Ch override.
    virtual bool base_break_off_gate(void* task) = 0;

    // BSP_EntityPose_GetWorldPositionRefreshed and the approach's own slot 0 inside
    // 009A65F0, reduced to the distance the SafeDist test compares.
    virtual float distance_to_approach_reference(void* task) = 0;

    // task->vtable[38h]() inside 0099B740.
    virtual bool should_abandon(void* task) = 0;

    // 007ED3F0 inside 0099B740, on a true should_abandon. The sibling 007ED430(2) is
    // docs/ATTACK_COMMANDS.md's close-to-ship stop; this naming is a hypothesis.
    virtual void abandon_command(int reason) = 0;
};

// The factory, 0099A170's arm plus the allocation wrapper. Returns null when the
// allocation fails, which is what every one of the thirteen factories does.
void* bot_task_create(BotTaskHost& host, const BotTaskClassRecord& record, const void* bot,
                      const void* unit, const void* target, BotTaskRecord& out);

// Slot +54h for the classes whose bodies were read. `cruise_alt_offset` and
// `second_alt_offset` are tuning-singleton offsets; pass -1 for a channel the class
// does not write. `attack_distance_offset` is -1 for close-to-ship, which has no clamp.
struct BotTaskCruiseProfile {
    int cruise_alt_offset{-1};
    int second_alt_offset{-1};
    int third_alt_offset{-1};       // divebomb and torpedo write a constant, not a tuning row
    int attack_distance_offset{-1};
};
void bot_task_update_cruise_profile(BotTaskHost& host, const BotTaskCruiseProfile& profile,
                                    void* pilot_control, PilotAltitudeChannel& cruise_channel,
                                    PilotAltitudeChannel& second_channel,
                                    PilotAltitudeChannel& third_channel, BotTaskRecord& task);

// The tuning-singleton offsets each read class uses, from docs/GAME_TUNING_SINGLETON.md.
namespace pilot_tuning_off {
inline constexpr int kCloseToShipCruisingAlt = 0x424;   // 1200
inline constexpr int kCloseToShipDropAlt = 0x428;       // 1000
inline constexpr int kCloseToShipReferenceSpeed = 0x42C; // KMH(300)
inline constexpr int kTorpedoCruisingAlt = 0x430;       // 500
inline constexpr int kTorpedoAttackDist = 0x434;        // 2200
inline constexpr int kTorpedoSafeDist = 0x438;          // 700
inline constexpr int kTorpedoReferenceSpeed = 0x440;    // KMH(300)
inline constexpr int kLevelBombCruisingAlt = 0x444;     // 1300
inline constexpr int kLevelBombDropAlt = 0x448;         // 1300
inline constexpr int kLevelBombAttackDist = 0x44C;      // 2000
inline constexpr int kLevelBombSafeDist = 0x450;        // 1000
inline constexpr int kLevelBombReferenceSpeed = 0x458;  // KMH(300)
inline constexpr int kDepthChargeAimAltRange1 = 0x494;  // 20
inline constexpr int kDepthChargeAimAltRange2 = 0x498;  // 60
inline constexpr int kDepthChargeManeuverAltRange1 = 0x49C; // 80
inline constexpr int kDepthChargeManeuverAltRange2 = 0x4A0; // 150
inline constexpr int kDepthChargeCruisingAlt = 0x4A4;   // 700
inline constexpr int kDepthChargeFlyAboveDist = 0x4A8;  // 400
inline constexpr int kDepthChargeAttackDist = 0x4AC;    // 1800
inline constexpr int kDepthChargeSafeDist = 0x4B0;      // 250
inline constexpr int kDepthChargeReferenceSpeed = 0x4B8; // KMH(270)
inline constexpr int kDiveBombCruisingAlt = 0x4C0;      // 1300
inline constexpr int kDiveBombAttackDist = 0x4C4;       // 1100
inline constexpr int kDiveBombSafeDist = 0x4C8;         // 100
inline constexpr int kDiveBombBeginAltRange1 = 0x4CC;   // 1000
inline constexpr int kDiveBombBeginAltRange2 = 0x4D0;   // 1200
inline constexpr int kDiveBombReferenceSpeed = 0x4D8;   // KMH(280)
inline constexpr int kLandingCruisingAlt = 0x514;       // 1400
inline constexpr int kLandingReferenceSpeed = 0x52C;    // KMH(140)
inline constexpr int kDogfightCruisingAlt = 0x640;      // 1400
inline constexpr int kDogfightAttackDist = 0x644;       // 2000
inline constexpr int kDogfightReferenceSpeed = 0x650;   // KMH(300)
inline constexpr int kStrafeCruisingAlt = 0x654;        // 1000
inline constexpr int kStrafeAttackDist = 0x658;         // 2000
inline constexpr int kStrafeReferenceSpeed = 0x65C;     // KMH(280)
inline constexpr int kStrikeCruisingAlt = 0x660;        // 500, the rocket task
inline constexpr int kStrikeAttackDist = 0x664;         // 1800
inline constexpr int kStrikeReferenceSpeed = 0x668;     // KMH(280)
} // namespace pilot_tuning_off

// The four cruise profiles whose bodies were read end to end.
extern const BotTaskCruiseProfile kCloseToShipCruiseProfile; // 009A2D60
extern const BotTaskCruiseProfile kDepthChargeCruiseProfile; // 009A6500
extern const BotTaskCruiseProfile kDiveBombCruiseProfile;    // 009C8920
extern const BotTaskCruiseProfile kTorpedoCruiseProfile;     // 009D4A70

} // namespace bsp
