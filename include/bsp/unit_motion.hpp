#pragma once
#include "bsp/unit_timer_pose.hpp"
#include <cstddef>
#include <cstdint>

// The three timed sub-updates of the vehicle base update, docs/UNIT_TIMED_SUBUPDATES.md.
//
// 008255B0 (docs/UNIT_INSTANCE_UPDATE.md, step 11) calls three __thiscall(this, float)
// routines back to back at 00825D4D..00825D83, each RET 4, each receiving the same
// scaled frame delta: 008252C0, 00956600, 00834E90.
//
// None of the three integrates a position or a heading. The unit's transform is owned
// by the physics body reached through the controller at +1018h; these three read that
// body (0092D730) and drive presentation and bookkeeping from it. The rules below are
// the native expressions in the native operation order; the x87 sequences keep their
// intermediates wider than float, which the double-typed temporaries model.
//
// Nothing here is a drop-in binary replacement, and the descriptive names are
// hypotheses rather than recovered symbols.

namespace bsp {

// ---------------------------------------------------------------------------
// Shared scalar primitives.
// ---------------------------------------------------------------------------

// 0042AC60, __thiscall(float* state, float target, float max_step), RET 8. A rate
// limiter: it snaps when the remaining distance is under one step, otherwise it moves
// exactly one step. The distance test is strict (max_step > |state - target|) and the
// direction test is target > state, both taken from FCOMI/FCOMIP at 0042AC8A/0042ACA6.
float unit_step_towards_0042ac60(float state, float target, float max_step) noexcept;

// 00415690, __fastcall(float* value, const float* low, const float* high), RET 4.
// Raises to low first, then lowers to high; both tests are strict.
float unit_clamp_00415690(float value, float low, float high) noexcept;

// ---------------------------------------------------------------------------
// Speed sources the sub-updates read.
// ---------------------------------------------------------------------------

// 0080FC30, float __thiscall(this), RET 0. The reference speed: the instance scalar at
// +9C0h times either a gameplay scale from 008E6430(4, this) or the literal 1.0f at
// 00D7A24C when the byte at 00E0C978 is clear or [DAT_00F88C30 + 0B8h] is null.
inline constexpr float kUnitReferenceSpeedUnscaled = 1.0f; // 00D7A24C
float unit_reference_speed_0080fc30(float base_speed, float gameplay_scale) noexcept;

// 0092D730, float __thiscall(controller), RET 0. Dots the physics body's linear
// velocity (00C31F40) with the third 12-byte row of the object 00C32000 returns, i.e.
// the signed speed along one body axis. The accumulation order is native: the y and x
// products are added first, then the z product.
struct UnitBodyAxisSpeedInputs {
    float velocity[3]{0.0f, 0.0f, 0.0f}; // 00C31F40's out vector
    float axis[3]{0.0f, 0.0f, 0.0f};     // [00C32000() + 18h .. +20h]
};
float unit_forward_speed_0092d730(const UnitBodyAxisSpeedInputs& in) noexcept;

// 00923BE0, float __thiscall(this), RET 0. The health read the damage scan compares
// against: zero while the +5Dh gate is set, otherwise the virtual at vtable +110h
// floored at zero. The floor also writes the zero back to +164h.
struct UnitHealthRead {
    float value{0.0f};
    bool wrote_zero_to_164{false};
};
UnitHealthRead unit_health_00923be0(bool gate_5d, float virtual_health) noexcept;

// ---------------------------------------------------------------------------
// 008252C0, the engine-audio parameter update.
// ---------------------------------------------------------------------------

// The trait ids the routine passes to IsKindOf (vtable +5Ch) at 008252DE and 008252F5.
inline constexpr int kUnitAudioTraitSecondary = 0x0E; // 008252DE
inline constexpr int kUnitAudioTraitTertiary = 0x08;  // 008252F5

// Byte offsets of the three per-category settings blocks inside the object 00424C40
// returns. Only +8 within a block is read here; the stride between them is 0x24.
inline constexpr std::size_t kUnitAudioSettingsDefault = 0x5BC;   // 00825305
inline constexpr std::size_t kUnitAudioSettingsSecondary = 0x5E0; // 008252E8
inline constexpr std::size_t kUnitAudioSettingsTertiary = 0x604;  // 008252FD
inline constexpr std::size_t kUnitAudioSettingsRateField = 0x008;  // 0082530B

// Instance fields 008252C0 owns, beyond the layout in include/bsp/unit_instance.hpp.
inline constexpr std::size_t kUnitOffEngineAudioGate = 0x9C4;     // 008252CA
inline constexpr std::size_t kUnitOffThrottle = 0x980;            // 00825331
inline constexpr std::size_t kUnitOffSteering = 0x984;            // 00834ECF
inline constexpr std::size_t kUnitOffReferenceSpeedBase = 0x9C0;  // 0080FC58
inline constexpr std::size_t kUnitOffRpmEmitterC = 0xBB4;         // 008253B2
inline constexpr std::size_t kUnitOffRpmEmitterB = 0xBB8;         // 0082538D
inline constexpr std::size_t kUnitOffRpmEmitterA = 0xBBC;         // 00825367
inline constexpr std::size_t kUnitOffAudioStopTarget = 0xBC0;     // 00825420
inline constexpr std::size_t kUnitOffRpmSmoothed = 0xBC4;         // 00825319

// Which settings block 008252C0 selects. The secondary trait wins over the tertiary.
std::size_t unit_audio_settings_block_008252de(bool is_secondary_trait,
                                               bool is_tertiary_trait) noexcept;

// 0082530B: the step the smoother is allowed this frame.
float unit_audio_rpm_step_0082530b(float settings_rate, float scaled_delta) noexcept;

// 00825311..0082535D: the smoother target. The simulate gate at +5Dh forces zero;
// otherwise it is the throttle magnitude, produced by clearing the sign bit.
float unit_audio_rpm_target_00825311(bool gate_5d, float throttle) noexcept;

// 008253C6..0082540C: the "param00" value. The gate forces zero; otherwise the native
// code inline-remaps the forward speed from [0, reference] onto [0, 1], which is a
// plain divide with no clamp and no zero guard.
float unit_audio_param00_008253c6(bool gate_5d, float reference_speed,
                                  float forward_speed) noexcept;

struct UnitEngineAudioState {
    bool enabled{false};        // +9C4h
    bool gate_5d{false};        // +5Dh
    float throttle{0.0f};       // +980h
    float rpm_smoothed{0.0f};   // +BC4h
    bool has_emitter_a{false};  // +BBCh
    bool has_emitter_b{false};  // +BB8h
    bool has_emitter_c{false};  // +BB4h
    bool has_stop_target{false};// +BC0h
};

// The literals the emitters are addressed with, at 00D08688 and 00D09998.
extern const char kUnitAudioParamRpm[];     // "rpm"
extern const char kUnitAudioParamGeneric[]; // "param00"

struct UnitEngineAudioHost {
    virtual ~UnitEngineAudioHost() = default;
    // 00424C40 then the byte reads at +5BCh/+5E0h/+604h plus kUnitAudioSettingsRateField.
    virtual float settings_rate(std::size_t block_offset) = 0;
    // The instance vtable +5Ch, with kUnitAudioTraitSecondary then kUnitAudioTraitTertiary.
    virtual bool is_kind_of(int class_id) = 0;
    // 0080FC30 on the instance and 0092D730 on the controller at +1018h.
    virtual float reference_speed() = 0;
    virtual float forward_speed() = 0;
    // vtable +1Ch on the emitters at +BBCh, +BB8h and +BB4h: set one named scalar.
    virtual void set_emitter_parameter(std::size_t emitter_offset, const char* name,
                                       float value) = 0;
    // vtable +0Ch on the object at +BC0h, called with 0.0f only while the gate is set.
    virtual void stop_audio_target(float value) = 0;
};

// 008252C0, __thiscall(this, float), RET 4.
void unit_update_engine_audio_008252c0(UnitEngineAudioState& state,
                                       UnitEngineAudioHost& host, float scaled_delta);

// ---------------------------------------------------------------------------
// 00956600, the scalar timers.
// ---------------------------------------------------------------------------

// The reload the damage scan writes back into +6D8h at 009566BD, and the rate the fade
// at +2F4h moves at, from the double at 00D7A280.
inline constexpr float kUnitDamageScanPeriod = 0.2f;   // 00CE54A0
inline constexpr double kUnitFadeRatePerSecond = 0.5;  // 00D7A280
inline constexpr float kUnitFadeVisibleThreshold = 0.5f; // 00CE3800
inline constexpr float kUnitFadeCeiling = 1.0f;          // 00D7A24C, the high bound 00415690 gets
inline constexpr int kUnitFadeThresholdGameMode = 9;   // 00956ABA

// Instance fields 00956600 owns.
inline constexpr std::size_t kUnitOffDamageScanMark = 0x364;  // 009566FE
inline constexpr std::size_t kUnitOffFadeTarget = 0x2F8;      // 00956A0E
inline constexpr std::size_t kUnitOffAnimationGate = 0x70C;   // 009567F5 region
inline constexpr std::size_t kUnitOffAnimationOwner = 0x360;  // 00956860 region
inline constexpr std::size_t kUnitOffAnimationExtra = 0x714;  // 00956880 region

// Descriptor fields at +354h that the damage scan walks.
inline constexpr std::size_t kUnitDescriptorDamageVector = 0x008; // begin +0Ch, end +10h
inline constexpr std::size_t kUnitDescriptorAnchorArray = 0x028;  // 009567B6
inline constexpr std::size_t kUnitDescriptorAnchorCount = 0x02C;  // 009566E8
inline constexpr std::size_t kUnitDamageRecordStride = 0x010;     // 009566B4, SAR 4

// One 16-byte record of the descriptor's damage table.
struct UnitDamageRecord {
    float health_threshold{0.0f}; // +0h, 0095674C
    int announce_id{-1};          // +4h, gate for the vtable +34h call at 0095685C
    int anchor_index{-1};         // +8h, index into the +28h float3 array
    bool has_effect{false};       // +0Ch, the ref-counted effect template
};

// 00956704: the whole scan only runs on a frame where health fell.
bool unit_damage_scan_runs_00956704(float health, float previous_health) noexcept;

// 0095674C..0095678E: a record fires when the health that just passed crossed it.
bool unit_damage_record_fires_0095674c(float health, float previous_health,
                                       float threshold) noexcept;

// 009569F8..00956A9D: the fade at +2F4h chases +2F8h at kUnitFadeRatePerSecond. Rising
// is floored at zero then capped at the target; falling is clamped into [target, 1].
// An exactly equal pair is left untouched.
float unit_step_fade_009569f8(float fade, float target, float scaled_delta) noexcept;

// 00956AA3..00956B25: the value handed to 00B6DA70 on the scene node at +4A4h.
float unit_visibility_factor_00956aa3(int game_mode, bool mission_reveal_byte,
                                      bool global_intensity_override,
                                      bool local_intensity_override, float fade) noexcept;

struct UnitTimerState {
    float age{0.0f};              // +524h
    float clamped_countdown{0.0f};// +728h
    float damage_scan_timer{0.0f};// +6D8h
    float damage_scan_mark{0.0f}; // +364h
    float fade{0.0f};             // +2F4h
    float fade_target{0.0f};      // +2F8h
    bool local_intensity_override{false}; // +2F0h
    bool animation_gate{false};   // +70Ch
    bool animation_extra{false};  // +714h
    bool has_scene_node{false};   // +4A4h
};

struct UnitTimerHost {
    virtual ~UnitTimerHost() = default;
    // Whether the descriptor at +354h holds a non-empty damage table.
    virtual std::size_t damage_record_count() = 0;
    virtual UnitDamageRecord damage_record(std::size_t index) = 0;
    virtual std::size_t descriptor_anchor_count() = 0;
    // 00923BE0 on the instance.
    virtual float health() = 0;
    // Borrow the actual descriptor +28h array entry, selected before pose refresh.
    // Required identity lookup, with no point copy, transform, allocation, fallback,
    // or owner mutation. Storage must survive the subsequent pose refresh.
    virtual const std::array<float, 3>& descriptor_anchor(std::size_t anchor_index) = 0;
    // The instance vtable +34h then 0049C940, behind announce_id >= 0. XYZ is the
    // captured stack point at 0095686B..00956870; native owner services remain bound.
    virtual void announce_damage_record(std::size_t index,
        const std::array<float, 3>& world_point) = 0;
    // 00440490 / 008689C0 / 004845D0 / 00440A30: actual effect ownership boundary.
    // Pass the same captured XYZ to 008689C0 with the native transform flag zero.
    virtual void spawn_damage_effect(std::size_t index,
        const std::array<float, 3>& world_point) = 0;
    // 00956818..00956880: the animation owner chain behind the +70Ch byte.
    virtual bool animation_target_ready() = 0;
    virtual void animation_pre_step_00b78670() = 0;
    virtual void animation_step() = 0;
    // 004BCA50 on DAT_00E188A8 and the byte at [DAT_00E188A8 + 61Dh].
    virtual int effective_game_mode() = 0;
    virtual bool mission_reveal_byte() = 0;
    // DAT_00F87152.
    virtual bool global_intensity_override() = 0;
    // 00B6DA70, BSP_SceneNode_SetVisibilityFactor, on the node at +4A4h.
    virtual void set_visibility_factor(float value) = 0;
};

// 00956600, __thiscall(this, float), RET 4. New C++ interface. actual_pose must
// borrow this same unit's +3C/+74/+C8/+CC/+10C fields; it must not be a pose copy.
// Only its anchor/fallback point branch is closed here; other timers retain the
// existing scalar projection and required host services. docs/UNIT_TIMER_POSE.md.
void unit_update_timers_00956600(UnitTimerState& state, PoseRefreshView& actual_pose,
    UnitTimerHost& host, float scaled_delta);

// ---------------------------------------------------------------------------
// 00834E90, the propeller and steering-node update.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kUnitPropellerCount = 4;   // 00835535
inline constexpr std::size_t kUnitSteeringNodeCount = 3; // 00834F22, 00834FEB, 008350A3

// Instance fields 00834E90 owns.
inline constexpr std::size_t kUnitOffPropellerNodes = 0x106C;   // [ESI-20h] in the loop
inline constexpr std::size_t kUnitOffSteeringNodes = 0x107C;    // 00834F22
inline constexpr std::size_t kUnitOffSteeringAngle = 0x1088;    // 00834EEF
inline constexpr std::size_t kUnitOffPropellerRates = 0x108C;   // 00835161
inline constexpr std::size_t kUnitOffCavitationHandles = 0xB44; // [ESI-548h]
inline constexpr std::size_t kUnitOffThrottleAlternate = 0xFC4; // 008350C4
inline constexpr std::size_t kUnitOffSteeringAlternate = 0xFDC; // 00834EC5
inline constexpr std::size_t kUnitOffPropellerLoad = 0x1030;    // 00835104

// Class-block fields at +538h the routine reads.
inline constexpr std::size_t kUnitClassPropellerGain = 0x6A0;    // 008350F8
inline constexpr std::size_t kUnitClassPropellerIdle = 0x69C;    // 00835116
inline constexpr std::size_t kUnitClassCavitationEffect = 0x6A4; // 008352C3

// Literals, all from the listing.
inline constexpr double kUnitSteeringBias = -1.0;                    // 00D7A250
inline constexpr double kUnitSteeringBlend = 0.78539818525314331;    // 00CEDCD0, float pi/4
inline constexpr double kUnitPropellerLoadScale = 1.2000000476837158; // 00CEC160
inline constexpr double kUnitPropellerThrottleGain = 4.0;            // 00D7A328
inline constexpr float kUnitPropellerRateFloor = -1.0f;              // 00D7A260
inline constexpr float kUnitPropellerRateCeiling = 1.0f;             // 00D7A24C
inline constexpr double kUnitPropellerRateSlew = 5.0;                // 00D7A370
inline constexpr float kUnitCavitationRateThreshold = 0.01f;         // 00D7A238
inline constexpr float kUnitPropellerMirror = -0.0f;                 // 00D7A208

// 00834ED7..00834F0F: the target the shared steering angle at +1088h chases, stepped at
// exactly the frame delta. The bias and blend are the two doubles named above; the
// expression is literally (s - bias) - blend * (s - bias).
float unit_steering_angle_target_00834ed7(float steering_source) noexcept;

// 008350F2..00835175: the load-dependent term every propeller target shares.
float unit_propeller_base_rate_008350f2(float throttle, float steering, float load,
                                        float class_gain, float class_idle) noexcept;

// 008351A2 and 008351F5: a propeller whose scene-node local x is positive keeps the
// base term; the others take its mirror, produced as -0.0f minus the term.
float unit_propeller_side_term_008351f5(float base_rate, bool node_local_x_positive) noexcept;

// 00835203..00835285: the per-propeller target and the slew applied to it.
float unit_propeller_target_rate_00835220(float class_gain, float throttle,
                                          float side_term) noexcept;
float unit_propeller_rate_slew_00835203(float scaled_delta) noexcept;

// 0083541F..0083544E: the angle the delta rotation is built from. Even indices spin one
// way and odd indices the other, and the whole product is negated after rounding.
float unit_propeller_spin_delta_0083541f(std::size_t index, float rate,
                                         float scaled_delta) noexcept;

// 008351C9 / 0083529F: the cavitation effect follows a hysteresis-free threshold on the
// magnitude of the rate, sampled before and after the slew.
bool unit_cavitation_active_008351c9(float rate) noexcept;

struct UnitPropellerState {
    float throttle{0.0f};             // +980h
    float throttle_alternate{0.0f};   // +FC4h
    float steering{0.0f};             // +984h
    float steering_alternate{0.0f};   // +FDCh
    bool use_alternate_inputs{false}; // +61h, tested at 00834EBE and 008350BD
    float load{0.0f};            // +1030h
    float steering_angle{0.0f};  // +1088h
    float rates[kUnitPropellerCount]{};        // +108Ch
    bool has_propeller_node[kUnitPropellerCount]{}; // +106Ch
    bool cavitation_live[kUnitPropellerCount]{};    // +B44h
    bool has_steering_node[kUnitSteeringNodeCount]{}; // +107Ch
};

struct UnitPropellerClassBlock {
    float propeller_gain{0.0f};   // [+538h]+6A0h
    float propeller_idle{0.0f};   // [+538h]+69Ch
    bool has_cavitation_effect{false}; // [+538h]+6A4h
};

struct UnitPropellerHost {
    virtual ~UnitPropellerHost() = default;
    // 0092D730 on the controller at +1018h. The result is discarded at 00834EBC; the
    // call is kept because it also clamps and caches on the controller.
    virtual float sample_controller_speed() = 0;
    // 00B6E0A0 on a scene node: its local position. Only x is read in the loop.
    virtual void steering_node_local_position(std::size_t index, float out[3]) = 0;
    virtual float propeller_node_local_x(std::size_t index) = 0;
    // 00467050 with (0, angle, 0) then the node's vtable +38h, for a steering node.
    virtual void set_steering_node_transform(std::size_t index, float angle,
                                             const float local_position[3]) = 0;
    // 00B64780 with one angle, 00B6DB60 for the node's current local matrix, 00413920
    // (BSP_Matrix_Multiply4x4, delta on the left), then the node's vtable +38h.
    virtual void spin_propeller_node(std::size_t index, float angle_delta) = 0;
    // 008687C0 with the stack identity frame then 004845D0 into the +B44h slot.
    virtual void start_cavitation_effect(std::size_t index) = 0;
    // 00867B10, handle->byte_9 = 1, then the ref-count release of the +B44h slot.
    virtual void stop_cavitation_effect(std::size_t index) = 0;
    // 00834820, 00834CC0 and 00834A70, in that order, each __thiscall(this, delta).
    virtual void sub_update_00834820(float scaled_delta) = 0;
    virtual void sub_update_00834cc0(float scaled_delta) = 0;
    virtual void sub_update_00834a70(float scaled_delta) = 0;
};

// 00834E90, __thiscall(this, float), RET 4.
void unit_update_propellers_00834e90(UnitPropellerState& state,
                                     const UnitPropellerClassBlock& class_block,
                                     UnitPropellerHost& host, float scaled_delta);

} // namespace bsp
