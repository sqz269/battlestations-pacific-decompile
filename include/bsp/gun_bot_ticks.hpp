// The gun-side AI bots: the sub-nodes the fixed-step wave runs on a gun's tick
// element, which turn the gun's fire target into a pair of angles for 0085ABA0
// and a trigger for the gun's vtable[1E8h].
//
// Evidence: docs/GUN_BOT_TICKS.md. Every routine here is a projection of one
// native body and the coverage of each is recorded in that document's routine
// table. Names are hypotheses, not recovered symbols, except the field names in
// section 4 of the document, which are string literals in the image.
//
// Reused rather than redeclared: bot_fire_target.hpp for GunAimAngles, the
// 008FEF40 debounce and the 008FFA20 hysteresis constants; unit_rudder.hpp for
// 00419010, 00438AA0 and 00438B10; gun_aiming.hpp for the gun's own offsets.
#ifndef BSP_GUN_BOT_TICKS_HPP
#define BSP_GUN_BOT_TICKS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/bot_fire_target.hpp"
#include "bsp/gun_aiming.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Addresses
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kGunBotBaseConstructAddress = 0x0072BBD0u;
inline constexpr std::uint32_t kGunBotFactoryAddress = 0x0072C6A0u;
inline constexpr std::uint32_t kGunBotAttachAddress = 0x008FBC80u;
inline constexpr std::uint32_t kGunBotIdleRestTimerAddress = 0x008FBCE0u;
inline constexpr std::uint32_t kGunBotSetFireTargetAddress = 0x006DF170u;
inline constexpr std::uint32_t kGunBotErrorRerollAddress = 0x006DEFF0u;
inline constexpr std::uint32_t kGunBotInterceptSolutionAddress = 0x00901C20u;
inline constexpr std::uint32_t kGunSolveGravityArcAddress = 0x00955630u;
inline constexpr std::uint32_t kGunBotAngleToleranceAddress = 0x006DEE40u;
inline constexpr std::uint32_t kGunSetTriggerHeldAddress = 0x0072D2C0u;
inline constexpr std::uint32_t kGunSetBotFireTargetAddress = 0x00727F10u;
inline constexpr std::uint32_t kGunSetBotSkillAddress = 0x00727E70u;
inline constexpr std::uint32_t kGunClearBotFireTargetAddress = 0x00728000u;
inline constexpr std::uint32_t kUnitGunAimMessageFilterAddress = 0x00954210u;
inline constexpr std::uint32_t kTurningGunFilterHeadingAddress = 0x0085AB50u;
inline constexpr std::uint32_t kEntityFindAncestorOfKindAddress = 0x00922E90u;

// ---------------------------------------------------------------------------
// The bot vtables. The primary table sits at bot+0h, the observer base at
// bot+1Ch. Section 2 of the document.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kGunBotBaseVtable = 0x00CFDFA0u;
inline constexpr std::uint32_t kGunBotBaseObserverVtable = 0x00CFDF88u;
inline constexpr std::uint32_t kTurretBotVtable = 0x00D18140u;     // 008FFA20
inline constexpr std::uint32_t kLeadBotVtable = 0x00D180A0u;       // 00902920
inline constexpr std::uint32_t kBallisticBotVtable = 0x00D18238u;  // 009030C0
inline constexpr std::uint32_t kTorpedoBotVtable = 0x00D182B0u;    // 008FFF20
inline constexpr std::uint32_t kMuzzleBotVtable = 0x00CFE000u;     // 006DF520

// Primary vtable slots, read from the base and every override.
inline constexpr std::size_t kGunBotSlotDestructor = 0x00;   // 0072BE20
inline constexpr std::size_t kGunBotSlotAttach = 0x04;       // 008FBC80
inline constexpr std::size_t kGunBotSlotTick = 0x0c;         // 0071C490 in the base
inline constexpr std::size_t kGunBotSlotSetSkill = 0x18;     // 0072BC80
inline constexpr std::size_t kGunBotSlotClearTarget = 0x30;  // 0072BD50
inline constexpr std::size_t kGunBotSlotSetTarget = 0x38;    // 006DF170
inline constexpr std::size_t kGunBotSlotHasTarget = 0x40;    // 0072BD10
inline constexpr std::size_t kGunBotSlotTargetEntity = 0x44; // 0072BD30

// ---------------------------------------------------------------------------
// The base instance, from 0072BBD0 and the sub-node fields 008759B0 walks.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kGunBotBaseOffSubNodePrev = 0x08;   // param_1[1]
inline constexpr std::size_t kGunBotBaseOffSubNodeNext = 0x0c;   // param_1[2]
inline constexpr std::size_t kGunBotBaseOffExpired = 0x10;       // param_1[4] byte
inline constexpr std::size_t kGunBotBaseOffEnabled = 0x11;       // set to 1
inline constexpr std::size_t kGunBotBaseOffObserverVtable = 0x1c; // param_1[7]
inline constexpr std::size_t kGunBotBaseOffClassDescriptor = 0x30; // param_1[0Ch]
inline constexpr std::size_t kGunBotBaseOffSkillIndex = 0x34;    // param_1[0Dh], default 1
inline constexpr std::size_t kGunBotBaseOffTargetRecord = 0x38;  // 00521EA0's argument
inline constexpr std::size_t kGunBotBaseOffIdleTimer = 0x54;     // param_1[15h]
inline constexpr std::size_t kGunBotBaseOffGun = 0x50;           // param_1[14h]

// The per-class cache of bot+50h. There is no single shared offset; see the
// correction in docs/GUN_BOT_TICKS.md section 9.
inline constexpr std::size_t kTurretBotOffGun = 0x68;    // 008FBEC0
inline constexpr std::size_t kLeadBotOffGun = 0x5c;      // 008FBDC0
inline constexpr std::size_t kBallisticBotOffGun = 0x68; // 008FF040
inline constexpr std::size_t kTorpedoBotOffGun = 0x58;   // 008FF310
inline constexpr std::size_t kMuzzleBotOffGun = 0x58;    // 006DF1F0

// 008FEAC0's literal property names for the 008FFA20 class.
inline constexpr std::size_t kTurretBotOffActFireState = 0x58;
inline constexpr std::size_t kTurretBotOffNextFireState = 0x59;
inline constexpr std::size_t kTurretBotOffFireDelay = 0x5c;
inline constexpr std::size_t kTurretBotOffHorzAngleError = 0x60;
inline constexpr std::size_t kTurretBotOffVertAngleError = 0x64;
inline constexpr std::size_t kTurretBotOffAimTime = 0x6c;
inline constexpr std::size_t kTurretBotOffAimPair = 0x70;
inline constexpr std::size_t kTurretBotOffLeadPoint = 0x78;
inline constexpr std::size_t kTurretBotOffAngleError = 0x84;
inline constexpr std::size_t kTurretBotOffAimPeriodMin = 0x88;
inline constexpr std::size_t kTurretBotOffAimPeriodMax = 0x8c;
inline constexpr std::size_t kTurretBotOffShootRange = 0x90;

// 006DF260's literal property names for the 006DF520 class.
inline constexpr std::size_t kMuzzleBotOffError = 0x5c;         // float2
inline constexpr std::size_t kMuzzleBotOffErrorEndHorz = 0x64;  // 006DF623's y1
inline constexpr std::size_t kMuzzleBotOffErrorEndVert = 0x68;  // 006DF651's y1
inline constexpr std::size_t kMuzzleBotOffErrorStartHorz = 0x6c; // 006DF623's y0
inline constexpr std::size_t kMuzzleBotOffErrorStartVert = 0x70; // 006DF651's y0
inline constexpr std::size_t kMuzzleBotOffCalcErrTick = 0x74;
inline constexpr std::size_t kMuzzleBotOffErrPeriod = 0x78;
inline constexpr std::size_t kMuzzleBotOffDelayedFire = 0x7c;
inline constexpr std::size_t kMuzzleBotOffFireDelayTime = 0x80;
inline constexpr std::size_t kMuzzleBotOffErrorOffset = 0x84;   // float3
inline constexpr std::size_t kMuzzleBotOffLeadPoint = 0xa8;     // float3
inline constexpr std::size_t kMuzzleBotOffLeadCountdown = 0xb4;

// ---------------------------------------------------------------------------
// Gun fields these ticks touch that gun_aiming.hpp does not already name.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kGunOffTickNode = 0x310;        // the attach argument
inline constexpr std::size_t kGunOffTickNodePayload = 0x338; // node+28h
inline constexpr std::size_t kGunOffBotSlotTurretOrLead = 0x390;
inline constexpr std::size_t kGunOffBotSlotBallistic = 0x394;
inline constexpr std::size_t kGunOffBotSlotMuzzle = 0x398;
inline constexpr std::size_t kGunOffBotSlotTorpedo = 0x39c;
inline constexpr std::size_t kGunOffBotSlotSubType8 = 0x3a0;
inline constexpr std::size_t kGunOffBotSlotMuzzleSecond = 0x3a4;
inline constexpr std::size_t kGunOffAimPoint = 0x408;        // float3, 006DFAE9
inline constexpr std::size_t kGunOffTriggerHeld = 0x454;     // 0072D2C0's store
inline constexpr std::size_t kGunOffFireStartDelay = 0x478;  // 0072D2C0's rising edge
inline constexpr std::size_t kGunVtableSlotSetTrigger = 0x1e8; // 0072D2C0
inline constexpr std::size_t kGunVtableSlotFireNow = 0x1f0;    // 006FDF60, turning classes only

inline constexpr std::size_t kEntityOffParent = 0x3c;     // 00922E90's chain
inline constexpr std::size_t kEntityOffSideIndex = 0x1ac; // 00927F10's argument
inline constexpr std::size_t kEntityVtableSlotSkill = 0x12c;
inline constexpr std::size_t kEntityVtableSlotPredictPoint = 0x100;
inline constexpr int kEntityKindUnit = 5;      // 008FBC9F, the IsKindOf(5) at attach
inline constexpr int kEntityKindTurretOwner = 0x0f; // 0072C6EB's 00922E90 argument
inline constexpr int kSideIndexNone = 8;       // the != 8 half of the AI side gate

// ---------------------------------------------------------------------------
// Constants, each with the address it was read from
// ---------------------------------------------------------------------------
inline constexpr float kGunBotFixedStep = 0.05f;              // 00875B90's argument
inline constexpr float kGunBotOneDegree = 0.0174532924f;      // 00CE3984
inline constexpr float kGunBotTenthDegree = 0.00174532935f;   // 00CF9054
inline constexpr float kGunBotGravity = 9.81000042f;          // 00CF9058, a double
inline constexpr float kGunBotQuarterPi = 0.785398185f;       // 00CEB5A8 and 00E0B588
inline constexpr float kGunBotFloatMax = 3.402823466e+38f;    // 00D7A278, a double
inline constexpr float kGunBotHalf = 0.5f;                    // 00D7A280, a double
inline constexpr float kGunBotIdleRestFraction = 0.25f;       // 00D7A348, a double
inline constexpr float kBallisticBotDepressionFloor = -0.02f; // 00D7A320
inline constexpr float kBallisticBotDepressionBias = 0.02f;   // 00D7A2F8, a double
inline constexpr float kMuzzleBotErrorPeriodMin = 3.0f;       // 00CE3854
inline constexpr float kMuzzleBotErrorPeriodMax = 8.0f;       // 00CE3918
inline constexpr float kMuzzleBotErrorStepRate = 30.0f;       // 00CE7630, a double
inline constexpr float kMuzzleBotFireDelayMax = 0.1f;         // 00D7A2F0
inline constexpr float kLeadBotSpanAtSkillZero = 1.0f;        // 00902B38's y0
inline constexpr float kLeadBotSkillSpanEnd = 6.0f;           // 00CE6630
inline constexpr float kLeadBotSpanAtSkillEnd = 0.2f;         // 00CE54A0
inline constexpr float kLeadBotErrorClampNumerator = 25.0f;   // 00CE3880, a double
inline constexpr float kLeadBotRateReversal = -0.5f;          // 00CEC9E0, a double
inline constexpr float kLeadBotRangeFraction = 0.9f;          // 00D7A390, a double
inline constexpr float kTorpedoBotRecomputePeriod = 0.2f;     // 00CE54A0
inline constexpr float kGunBotPredictDirectionScalar = 0.6f;  // 00CE3D30
inline constexpr float kUnitGunAimWindowKind12 = 0.05235987902f; // 00D1A8A0, a double
inline constexpr float kUnitGunAimWindowKind3 = 0.0349065848f;   // 00D0C26C
inline constexpr float kUnitGunAimWindowKind4 = 0.0872664675f;   // 00CEDF5C

// ---------------------------------------------------------------------------
// Which bot a weapon descriptor sub-type gets, 0072C6A0
// ---------------------------------------------------------------------------
enum class GunBotClass {
    kNone,
    kLead,      // 008FE740, tick 00902920
    kTurret,    // 008FE9F0, tick 008FFA20
    kBallistic, // 008FEFD0, tick 009030C0
    kMuzzle,    // 0072BE40, tick 006DF520
    kTorpedo,   // 008FF260, tick 008FFF20
    kSubType8,  // 008FF4A0, outside this packet
};

struct GunBotSlotAssignment {
    GunBotClass primary = GunBotClass::kNone;   // gun+390h
    GunBotClass ballistic = GunBotClass::kNone; // gun+394h
    GunBotClass muzzle = GunBotClass::kNone;    // gun+398h
    GunBotClass torpedo = GunBotClass::kNone;   // gun+39Ch
    GunBotClass sub_type_8 = GunBotClass::kNone; // gun+3A0h
    GunBotClass muzzle_second = GunBotClass::kNone; // gun+3A4h
    bool destroys_primary = false;              // 0072C7A9, the sub-type 5/6 arm
};

// 0072C6A0. `gun_is_turning` is vtable[5Ch](22h) at 0072C6DE;
// `owner_of_kind_0f` is 00922E90(gun, 0Fh) != 0 at 0072C6EB.
GunBotSlotAssignment gun_bot_slots_for_subtype_0072c6a0(int weapon_sub_type,
                                                        bool gun_is_turning,
                                                        bool owner_of_kind_0f) noexcept;

// ---------------------------------------------------------------------------
// The shared prologue
// ---------------------------------------------------------------------------

// 008FFA99 and its three siblings. The gate passes when the side index is the
// "none" value or the side's own record byte is set.
bool gun_bot_side_enabled_00927f10(int side_index, bool side_record_flag) noexcept;

// 008FFA3B..008FFA5F. The target survives only when all five hold.
struct GunBotTargetValidity {
    bool target_resolved = false;   // 00521EA0 returned non-null
    bool target_alive = false;      // target+5Dh == 0
    bool gun_present = false;       // bot+50h != 0
    bool gun_parent_present = false; // [gun+3Ch] != 0
    bool gun_parent_alive = false;  // [gun+3Ch]+5Dh == 0
    bool sides_related = false;     // 00803510([gun+54h], [target+54h])
};
bool gun_bot_target_still_valid_008ffa20(const GunBotTargetValidity& v) noexcept;

// 008FBCE0, the idle return-to-rest timer.
enum class GunBotIdleAction {
    kNone,
    kAimToRest, // 008FBD61, only when the gun answers IsKindOf(22h)
};
struct GunBotIdleTimer {
    float elapsed = 0.0f; // bot+54h
};
GunBotIdleAction gun_bot_idle_timer_008fbce0(GunBotIdleTimer& timer,
                                             bool has_target,
                                             bool side_enabled,
                                             bool gun_is_turning,
                                             float idle_limit,
                                             float dt) noexcept;

// ---------------------------------------------------------------------------
// 008FFA20, the turret bot
// ---------------------------------------------------------------------------
struct TurretBotState {
    float aim_countdown = 0.0f;    // +6Ch aimTime
    GunAimAngles aim;              // +70h, the cached pair
    float aim_error_degrees = 0.0f; // +84h angleError
    float aim_period_min = 0.0f;   // +88h
    float aim_period_max = 0.0f;   // +8Ch
    float shoot_range = 0.0f;      // +90h
    GunBotTriggerState trigger;    // +58h, +59h, +5Ch
};

// 008FFB35..008FFB62: the countdown is decremented first and the recompute runs
// only when the result is below zero.
bool gun_bot_turret_recompute_due_008ffa20(float& countdown, float dt) noexcept;

// ---------------------------------------------------------------------------
// 009030C0, the bomb and depth-charge bot
// ---------------------------------------------------------------------------

// 009032C6..009032E9. A depression steeper than -0.02 rad is halved and biased.
// A target answering IsKindOf(0Fh) flattens the shot instead (009032B5).
float gun_bot_ballistic_vertical_correction_009030c0(float vert,
                                                     bool target_is_kind_0f) noexcept;

struct BallisticBotFireInputs {
    float distance = 0.0f;     // 00901C20's outDist
    float min_range = 0.0f;    // [muzzle+58h]
    float max_range = 0.0f;    // [muzzle+60h]
    float horz_delta = 0.0f;   // 00438B10(gun+480h, h)
    float vert_delta = 0.0f;   // 00438B10(gun+484h, v)
    bool player_inhibit = false; // bit 0 of [gun+3F0h]+634h
};
bool gun_bot_ballistic_fire_009030c0(const BallisticBotFireInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 006DF520, the muzzle-solution bot
// ---------------------------------------------------------------------------
struct MuzzleBotErrorEnvelope {
    float start_horz = 0.0f; // +6Ch
    float end_horz = 0.0f;   // +64h
    float start_vert = 0.0f; // +70h
    float end_vert = 0.0f;   // +68h
};
struct MuzzleBotErrorState {
    float period = 0.0f;    // +78h, the value the countdown started from
    float countdown = 0.0f; // +74h calcErrTick
    GunAimAngles error;     // +5Ch Error
};

// 006DF623 and 006DF651. The pair is interpolated from the envelope's start to
// its end as the countdown falls from `period` to zero.
GunAimAngles gun_bot_muzzle_error_006df520(const MuzzleBotErrorEnvelope& envelope,
                                           float period, float countdown) noexcept;

// 006DEE40, __stdcall(a, b, tol) -> int, RET 0Ch.
bool gun_bot_angle_within_tolerance_006dee40(float current, float desired,
                                             float tolerance) noexcept;

// 006DFC0x. The muzzle kind at [[gun+3F8h]+34h]+8h selects which bit of the
// unit's +634h mask inhibits the shot.
int gun_bot_muzzle_inhibit_bit_006df520(int muzzle_kind) noexcept;

struct MuzzleBotDelayedFire {
    bool armed = false;  // +7Ch delayedFire
    float delay = 0.0f;  // +80h fireDelayTime
};

// 006DFB8C..006DFBE5, the arming half. `draw` is 00BD2F10(0, 0.1)'s result.
void gun_bot_muzzle_arm_fire_006df520(MuzzleBotDelayedFire& state,
                                      bool solver_ok, bool aim_accepted,
                                      float gun_horz, float gun_vert,
                                      const GunAimAngles& commanded,
                                      float draw) noexcept;

// 006DFC40..006DFC6C, the release half. Returns true on the frame the gun's
// vtable[1F0h] should be called.
bool gun_bot_muzzle_release_fire_006df520(MuzzleBotDelayedFire& state,
                                          bool inhibited, float dt) noexcept;

// ---------------------------------------------------------------------------
// 00902920, the swinging-error bot
// ---------------------------------------------------------------------------

// 00902B38: the per-skill span the random error period is scaled by.
float gun_bot_lead_error_span_00902920(float skill) noexcept;

// 00902E6D and 00902EF7: an axis is clamped once it leaves 25/distance.
float gun_bot_lead_error_limit_00902920(float distance) noexcept;

struct LeadBotFireInputs {
    float distance = 0.0f;
    float max_range = 0.0f;  // [muzzle+60h]
    bool aim_accepted = false; // 0085ABA0's answer
};
bool gun_bot_lead_fire_00902920(const LeadBotFireInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 008FFF20, the torpedo bot
// ---------------------------------------------------------------------------

// 009003C6: 0085AB50 answers FLT_MAX when no traverse window accepts the run
// heading, and the whole shot is then abandoned.
bool gun_bot_heading_accepted_008fff20(float filtered_heading) noexcept;

// 009003F8 and the vtable[1D0h](1) test that follows it.
bool gun_bot_torpedo_ready_008fff20(float filtered_heading, float gun_horz,
                                    bool gun_can_fire) noexcept;

// ---------------------------------------------------------------------------
// 00959C20, the commanded aim path
// ---------------------------------------------------------------------------
enum class UnitGunAimMessageKind {
    kNone = 0,
    kAimAtPoint = 1,      // 00959C91
    kAimAtPointWithTarget = 2,
    kAimAtDirection = 3,  // 00959F72
    kAimAtHeading = 4,    // 0095A1CC
    kAimSubType9 = 5,     // 0095A441
};

// 00954210, __cdecl(kind, device) -> bool, RET 8.
struct UnitGunAimDeviceTests {
    bool device_present = false;  // dev != 0
    bool device_ready = false;    // 00729F10(dev)
    int weapon_sub_type = 0;      // [dev+3F4h]+80h
    bool accepts_kind2_extra = false; // 005459E0(dev)
    bool accepts_kind3 = false;       // 005459B0(dev)
    bool accepts_kind5 = false;       // 0080F750(dev)
};
bool unit_gun_aim_accepts_device_00954210(UnitGunAimMessageKind kind,
                                          const UnitGunAimDeviceTests& tests) noexcept;

// The per-kind window the trigger is gated on.
float unit_gun_aim_trigger_window_00959c20(UnitGunAimMessageKind kind) noexcept;

// ---------------------------------------------------------------------------
// The ticks as sequences over an injected host
// ---------------------------------------------------------------------------

// One virtual per native call site the three complete ticks make. Nothing here
// allocates and nothing owns the gun; the host is the native side.
struct GunBotTickHost {
    virtual ~GunBotTickHost() = default;

    // -- shared prologue ----------------------------------------------------
    virtual void* resolve_fire_target_00521ea0() = 0;         // bot+38h
    virtual GunBotTargetValidity target_validity() = 0;       // the five tests
    virtual void clear_fire_target_slot38() = 0;              // bot->vtable[38h](0)
    virtual bool side_enabled_00927f10() = 0;                 // [gun+1ACh]
    virtual GunBotIdleAction run_idle_timer_008fbce0(float dt) = 0; // 008FBCE0
    virtual void* fire_target_entity_slot44() = 0;            // bot->vtable[44h]
    virtual bool gun_present() = 0;                           // the class's gun cache

    // -- 008FFA20 -----------------------------------------------------------
    virtual bool director_artillery_flag_008ffac9() = 0;      // [director+221h]
    virtual std::array<float, 3> target_pose_origin() = 0;    // 004142E0(target+0CCh)
    virtual std::array<float, 3> predict_lead_point_slot100(
        const std::array<float, 3>& direction_scalars,
        float s0, float s1, float s2, float s3) = 0;          // target->vtable[100h]
    virtual std::array<float, 3> muzzle_world_origin() = 0;   // [gun+3CCh]+120h
    virtual GunAimAngles angles_from_world_direction_008fdaf0(
        const std::array<float, 3>& world_direction) = 0;     // 00414E10 then 008FDAF0
    virtual float random_range_00bd2f10(float low, float high) = 0;
    virtual bool set_target_angles_0085aba0(const GunAimAngles& angles) = 0;
    virtual float vector_length_0042b2f0(const std::array<float, 3>& v) = 0;
    virtual float gun_horz_angle() = 0;                       // gun+480h
    virtual float gun_vert_angle() = 0;                       // gun+484h
    virtual void trigger_debounce_008fef40(bool request, float dt) = 0;

    // -- 009030C0 -----------------------------------------------------------
    virtual bool gun_is_kind5_00903154() = 0;                 // bot+50h->IsKindOf(5)
    virtual bool descriptor_flag_95h() = 0;                   // [[gun+3F4h]+95h]
    virtual BallisticBotFireInputs solve_intercept_00901c20() = 0; // 00901C20 plus the deltas
    virtual GunAimAngles ballistic_angles_009030c0() = 0;     // 0090326D then 0090327B
    virtual bool target_is_kind_0f() = 0;                     // 009032B5
    virtual void set_trigger_slot1e8(bool held) = 0;          // gun->vtable[1E8h]

    // -- 006DF520 -----------------------------------------------------------
    virtual bool has_fire_target_slot40() = 0;                // bot->vtable[40h]
    virtual void reroll_error_envelope_006deff0() = 0;        // 006DEFF0
    virtual MuzzleBotErrorEnvelope error_envelope() = 0;      // +64h..+70h
    virtual void step_error_offset_0042ac60(float rate_dt) = 0; // three 0042AC60 calls
    virtual bool solve_gravity_arc_00955630(GunAimAngles& out) = 0; // 00955630
    virtual void store_aim_point_006dfae9() = 0;              // gun+408h
    virtual bool muzzle_fire_inhibited() = 0;                 // the +634h bit
    virtual void fire_now_slot1f0() = 0;                      // gun->vtable[1F0h]
};

// 008FFA20, body 008FFA20..008FFF1F.
void gun_bot_turret_tick_008ffa20(GunBotTickHost& host, TurretBotState& state,
                                  float dt);

// 009030C0, body 009030C0..0090341D.
void gun_bot_ballistic_tick_009030c0(GunBotTickHost& host, float dt);

// 006DF520, body 006DF520..006DFC6C.
void gun_bot_muzzle_tick_006df520(GunBotTickHost& host, MuzzleBotErrorState& error,
                                  MuzzleBotDelayedFire& fire, float dt);

} // namespace bsp

#endif // BSP_GUN_BOT_TICKS_HPP
