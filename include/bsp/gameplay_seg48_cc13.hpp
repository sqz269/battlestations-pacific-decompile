#pragma once

// Segment 48 (007D1E50-007F8390) reference models: the plane parts. Packet
// cc13-gpr48; docs/CC13_GPR_48.md, docs/CC13_GP_007A_007F.md.
//
// These are semantic interfaces read from the disk bytes (Capstone disasm-raw,
// x87 read from the opcode bytes) and the Ghidra pseudocode, not native layouts
// or ABI-compatible replacements. Field names are hypotheses; the "_NNN" suffix
// is the native byte offset. Every engine service the image calls is a host
// callback, so nothing here invents a global or stubs a call. Gameplay runs at
// 24-bit x87 precision (docs/CC13_FPU_PRECISION.md), so float arithmetic is
// modelled in float where the image rounds to float. Build-tested only; not
// wired, not game-run.
//
// ADOPTION NOTE (cc9 terms; the full table is in docs/CC13_GPR_48.md section 2):
// - Replaces: nothing the hosts run today except where named. The actuator
//   animation pick (007DE9A0) sits beside the host's actuator step
//   (src/game_hosts_units.cpp:1600, bsp::PlaneActuatorBlock, 007DE3A0) and only
//   chooses which channel's time the plane's part animation shows. The rotor,
//   engine sound and damage-effect models replace nothing: the hosts build no
//   plane effects component (007EC5A0 is HOST-OMITS in
//   docs/CC13_RNG_STREAM_CENSUS.md 10.4). The flight-controller record pair
//   (007D8330 / 007DB1F0) is the multiplayer control-state message, which the
//   single-player hosts never send.
// - Inputs: the actuator channels of unit+DECh and the fixed-step clock
//   [00F876A4]; GameTuning Rotor/* (+160h..+17Ch); the class engine count
//   (+140h) and no-rotor flag (+144h); the unit's health (00923BE0) and the
//   class damage-effect list (+224h); dt.
// - RNG, all through the host's Rng callback, in image order:
//   * 007ECA23 then 007ECA41: U(0, 0.2) then U(8, 1), STREAM 1, once per plane
//     effects load (007EC5A0); then `count` NextU32 draws on STREAM 1 (007ECADD,
//     the Fisher-Yates shuffle of the rotor speed spread).
//   * 007EC10F: U(0, 2 pi), STREAM 1, once per rotor Init (007EBF10).
//   * 007E1CCF: U(0, 2 pi), STREAM 1, once per rotor Reset (007E1C60), which
//     Restart (007DDEC0) and the engine list's ResetAll (007DE630, from
//     007B97F0 RestoreFullHealth) reach.
//   * 007E2862: U(0, 1), STREAM 0, once per call of the damage-effect switch
//     (007E27B0) while its state is 1, i.e. every plane tick for 10 s after the
//     plane's damage changes. Presentation on the gameplay stream: the RNG rule
//     applies (a presentation stream, or OFF with a comment).
// - OFF-switch prediction: OFF changes nothing. ON, the stream-1 draws are the
//   only gameplay-visible effect of the rotor models: every plane spawn shifts
//   stream-1 consumers by 2 + count + rotors draws, and a full-health restore
//   by one per rotor. The damage-effect switch, if bound on stream 0, shifts
//   every later stream-0 consumer from the first hit on a plane with a class
//   damage-effect list.

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace bsp {
namespace gameplay_seg48_cc13 {

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// The image's random service. stream is the ECX value: 0 for the gameplay
// stream, 1 for the second thread stream. uniform is 00BD2F10
// BSP_Random_UniformFloatRange(lo, hi); next_u32 is 00BD2FC0
// BSP_RandomThreads_NextU32. The host decides which generator answers.
struct Rng {
    std::function<float(int stream, float lo, float hi)> uniform;
    std::function<std::uint32_t(int stream)> next_u32;
};

// ---------------------------------------------------------------------------
// 1. The actuator block's part animation (gear, wings, bay doors).
// ---------------------------------------------------------------------------

// One channel of the block at unit+DECh, as 007DE7E0 reads it. The block holds
// three: +28h, +44h, +60h. The first four fields are the ones cc9's
// bsp::PlaneActuatorChannel names; +10h/+14h are the animation time range of
// this channel and +18h the fixed-step clock at the last step (007DE274
// copies [00F876A4] there with MOVSS; 007DE7E0 does float arithmetic on it).
struct ActuatorAnimChannel {
    bool enabled_0{false};
    bool target_1{false};      // the commanded end: true is 1.0f, false 0.0f
    float value_4{0.0f};       // the position in [0, 1]
    bool moving_8{false};
    float rate_0c{0.0f};       // per second
    float anim_lo_10{0.0f};    // animation time at value 0
    float anim_hi_14{0.0f};    // animation time at value 1
    float stamp_18{-1.0f};     // the clock at the last step; < 0 means never
};

// 007DE7E0 BSP_ActuatorChannel_ComputeBlend, __fastcall(ECX = channel), result
// in ST0. A moving channel with stamp_18 >= 0.0f (COMISS; a NaN stamp skips)
// is extrapolated from its last step to the current clock:
//   v = float(float(rate * (target ? 1 : -1) * float(clock - stamp)) + value)
// with no clamp. The result is 00419010 BSP_Math_InterpolateClamped(0, lo, 1,
// hi, v), a stdcall (RET 14h) returning ST0.
float actuator_channel_anim_time_007de7e0(const ActuatorAnimChannel& channel,
                                          float clock_f876a4) noexcept;

struct ActuatorAnimBlock {
    ActuatorAnimChannel channel_28{};   // cc9's channel_b, the gear (007B8860)
    ActuatorAnimChannel channel_44{};   // cc9's channel_a (PartAnims.Wings)
    ActuatorAnimChannel channel_60{};   // cc9's channel_c, the ordnance bay
    float shown_time_7c{0.0f};          // the last time pushed to the animation
    bool owner_suppressed_5d{false};    // [block+1Ch]+5Dh; set: nothing runs
};

struct ActuatorAnimResult {
    bool pushed{false};      // the animation slot +24h was called
    float time{0.0f};        // the new shown_time_7c when pushed
    int source_offset{0};    // 0x44, 0x28 or 0x60: the channel whose time won;
                             // 0 when no channel was evaluated
};

// 007DE9A0 BSP_ActuatorBlock_Step, __fastcall(ECX = block), plain RET.
// Bytes 007DE9A0-007DEA5E. The pick, read from the jumps (each equality test is
// LAHF / TEST AH,44h / JP, so a NaN counts as "not equal"):
//   if +44h enabled and +48h != 1.0f      -> time of +44h
//   else { if +28h enabled                -> time of +28h, and done unless
//                                            +2Ch == 0.0f;
//          if +60h enabled                -> time of +60h }
// With no channel evaluated the time stays shown_time_7c. When the time
// differs from shown_time_7c (FUCOMIP; NaN differs) the block stores it and
// calls [[block+20h]]+24h(block+24h, time, 1). Mutates block.shown_time_7c.
ActuatorAnimResult actuator_block_anim_007de9a0(ActuatorAnimBlock& block,
                                                float clock_f876a4) noexcept;

// ---------------------------------------------------------------------------
// 2. Engines and rotors (vtables 00D08640 base, 00D0865C rotor).
// ---------------------------------------------------------------------------

// GameTuning "Rotor/..." fields (docs/GAME_TUNING_SINGLETON.md rows +160..+17C).
struct RotorTuning {
    float speed_base_160{6.2831855f};       // Rotor/SpeedBase, DEG(360)
    float speed_random_168{0.1f};           // Rotor/SpeedRandom
    float still_only_speed_16c{0.1f};       // Rotor/StillOnlySpeed
    float blurred_only_speed_170{0.3f};     // Rotor/BlurredOnlySpeed
    float rotor_speed_change_17c{0.2f};     // Rotor/RotorSpeedChange
};

// The rotor object (operator new(40h) at 007ECBBD). Scene nodes are opaque
// handles; 0 is "absent". The second-model nodes (+10h, +18h, +20h) exist only
// when the visual class field +160h is non-null.
struct PlaneRotor {
    bool burning_8{false};          // read by 007DE5B0; written by 007E24F7
    bool stopped_9{false};          // 007DDF10 / 007EC130
    std::uintptr_t blur_0c{0};      // "rotor<i>"
    std::uintptr_t blur2_10{0};
    std::uintptr_t still_14{0};     // "rotor_still<i>"
    std::uintptr_t still2_18{0};
    std::uintptr_t still_dam_1c{0}; // "rotor_still_dam<i>"
    std::uintptr_t still_dam2_20{0};
    std::uintptr_t shown_still_24{0};   // +14h, or +1Ch once stopped
    std::uintptr_t shown_still2_28{0};  // +18h, or +20h once stopped
    float spin_sign_2c{1.0f};
    float spin_rate_30{0.0f};       // radians per second at rpm 1
    float rpm_34{0.0f};             // 0..1
    float shown_angle_38{0.0f};
    float angle_3c{0.0f};
};

// One scene-node visibility write, 00B6DA70(node, factor, 0) in call order.
struct NodeVisibility {
    std::uintptr_t node{0};
    float factor{0.0f};
};

// 007EC5A0's speed spread, bytes 007ECA0E-007ECB1C. lo = U1(0, 0.2) and
// hi = U1(8, 1) (00CE54A0 = 0.2f, 00CE3918 = 8.0f, FLD1), in that order; then
// count + 1 values spread[i] = float(float(float(i) / float(count)) * (hi - lo)
// + lo); then for k = 1 .. count: j = NextU32(1) % (k + 1), swap(spread[k],
// spread[j]). Engine i is initialised with spread[i]. count is class+140h; with
// count 0 the one value is 0/0, a NaN, and no engine reads it.
std::vector<float> rotor_speed_spread_007eca0e(int engine_count, const Rng& rng);

struct RotorInitResult {
    std::vector<NodeVisibility> visibility;
};

// 007EBF10 BSP_PlaneRotor_Init, slot 0 of 00D0865C, __thiscall(this, model,
// model2, index, speed_factor), RET 10h (the call at 007ECC85 pushes spread[i],
// the index or -1 when class+140h < 2, the visual field +160h, then
// 00476B90's result). The node lookups (BSP_Model_FindNoteNode on "rotor",
// "rotor_still", "rotor_still_dam" with the index appended when >= 0) are the
// caller's: pass the resolved handles in `nodes` (only the node fields are
// read). Scalar effects:
//   spin_sign_2c = +1.0f when index % 2 != 0 (C remainder, so -1 gives +1),
//                  else -1.0f;
//   spin_rate_30 = float(float(SpeedRandom * speed_factor + 1.0) * SpeedBase);
//   still_dam 0, still 1, still_dam2 0, still2 0; shown = (still, still2);
//   shown_angle_38 = angle_3c = U1(0, 2 pi) (007EC10F, 00CE3D9C);
//   stopped_9 = false; the point effect at +4h is stopped and released.
RotorInitResult rotor_init_007ebf10(PlaneRotor& rotor, const PlaneRotor& nodes,
                                    int index, float speed_factor,
                                    const RotorTuning& tuning, const Rng& rng);

// 007E1C60 BSP_PlaneRotor_Reset, slot +8h of 00D0865C. Visibility (bytes
// 007E1C61-007E1CA7): still_dam 0, still_dam2 0 (if present), still 1, still2 0
// (if present); shown = (still, still2);
// shown_angle_38 = angle_3c = U1(0, 2 pi) (007E1CCF); then the base reset
// 007E1C00 (stopped_9 = false, point effect stopped).
std::vector<NodeVisibility> rotor_reset_007e1c60(PlaneRotor& rotor, const Rng& rng);

// 007DDEC0 BSP_PlaneRotor_Restart, slot +4h, __thiscall(this, bool running),
// RET 4: Reset (vtable +8h, one stream-1 draw), rpm_34 = running ? 1.0f : 0.0f,
// then the blur blend (vtable +18h). The visibility of both calls is returned.
std::vector<NodeVisibility> rotor_restart_007ddec0(PlaneRotor& rotor, bool running,
                                                   const RotorTuning& tuning,
                                                   const Rng& rng);

// 007DDF10 BSP_PlaneRotor_StopEngine, slot +0Ch. Once (stopped_9 clear), from
// the bytes 007DDF13-007DDF6E: stopped_9 = true; still 0, still_dam 1, still2 0
// (if present), still_dam2 1 (if present); shown = (still_dam, still_dam2). The
// blur nodes are not written here (the Ghidra pseudocode's argument order is
// garbled; the listing is authoritative).
std::vector<NodeVisibility> rotor_stop_engine_007ddf10(PlaneRotor& rotor);

// 007EC180 BSP_PlaneRotor_StepRpm, slot +10h, __thiscall(this, float target,
// float dt), RET 8. rpm_34 = 0042AC60(rpm, stopped ? 0 : target,
// float(RotorSpeedChange * dt)); then angle_3c = float(fmod(float(spin_rate *
// rpm * dt + angle), 2 pi)) with the modulus the double at 00CE3828.
void rotor_step_rpm_007ec180(PlaneRotor& rotor, float target, float dt,
                             const RotorTuning& tuning) noexcept;

// 007EC1F0 BSP_PlaneRotor_ApplySpin, slot +14h, __thiscall(this, float dt),
// RET 4: shown_angle_38 = float(fmod(float(rpm * spin_rate * dt + angle_3c),
// 2 pi)); the blur nodes are rotated about z by -(shown_angle * spin_sign)
// (BSP_Matrix_BuildRotationZ). Returns that z angle. The node matrix writes
// and the trailing vtable +18h call (the blur blend) are the caller's.
float rotor_apply_spin_007ec1f0(PlaneRotor& rotor, float dt) noexcept;

// 007EC3B0 BSP_PlaneRotor_UpdateBlurBlend, slot +18h. s = 00419010(
// StillOnlySpeed, 1, BlurredOnlySpeed, 0, rpm_34). The still node that is not
// shown gets 0 (still when shown is still_dam, else still_dam), and its second
// model node too; then shown_still_24 = s, shown_still2_28 = 1 - s (sic: the
// second model's still gets the blur weight), blur_0c = 1 - s,
// blur2_10 = 1 - s.
std::vector<NodeVisibility> rotor_blur_blend_007ec3b0(const PlaneRotor& rotor,
                                                      const RotorTuning& tuning) noexcept;

// The engine interface both vtables implement (00D08640 base, 0Ch bytes;
// 00D0865C rotor, 40h bytes), as the list loops below call it. Slot 0 (Init)
// is called once by 007ECC85 and is not part of this interface. The base's
// +10h/+14h/+18h are the empty bodies 007E23B0 (RET 8), 007E23C0 (RET 4) and
// 007E23D0 (RET), which have no Ghidra function.
class PlaneEngineSlots {
public:
    bool burning_8{false};  // set once by 007E24F7 (OnDestroyed); 0 from 007ECBD6
    bool stopped_9{false};  // the base's +9h; the rotor's PlaneRotor::stopped_9
    virtual ~PlaneEngineSlots() = default;
    virtual void restart(bool running) = 0;           // +4h: 007DDEB0 / 007DDEC0
    virtual void reset() = 0;                         // +8h: 007E1C00 / 007E1C60
    virtual void stop_engine() = 0;                   // +0Ch: 007DDF00 / 007DDF10
    virtual void step_rpm(float target, float dt) = 0;  // +10h: (base) / 007EC180
    virtual void apply_spin(float dt) = 0;            // +14h: (base) / 007EC1F0
    virtual void blur_blend() = 0;                    // +18h: (base) / 007EC3B0
};

// The list at unit+C98h (+8h data, +0Ch count), walked in index order.
// 007DE540 BSP_PlaneEngineList_StepAll, __thiscall(list, float dt), RET 4:
// slot +14h(dt) on each. (The name predates this reading: the slot it calls
// is the spin, not the rpm step.)
void engine_list_spin_all_007de540(const std::vector<PlaneEngineSlots*>& list, float dt);
// 007DE580 BSP_PlaneEngineList_UpdateAll: slot +18h on each.
void engine_list_blur_all_007de580(const std::vector<PlaneEngineSlots*>& list);
// 007DE5E0 BSP_PlaneEngineList_StopEngineAt, __thiscall(list, int i), RET 4:
// slot +0Ch on list[i], unchecked.
void engine_list_stop_at_007de5e0(const std::vector<PlaneEngineSlots*>& list, int index);
// 007DE600 BSP_PlaneEngineList_StopAll: slot +0Ch on each.
void engine_list_stop_all_007de600(const std::vector<PlaneEngineSlots*>& list);
// 007DE630 BSP_PlaneEngineList_ResetAll: slot +8h on each. For rotors this is
// 007E1C60, one stream-1 draw per rotor.
void engine_list_reset_all_007de630(const std::vector<PlaneEngineSlots*>& list);

// The base engine's own slots. 007DDEB0 (+4h, RET 4) calls its own +8h and
// ignores the argument; 007DDF00 (+0Ch) sets +9h; 007E1C00 (+8h) and 007E1BA0
// (slot 0, the Init of a base engine, whose arguments it ignores) both clear
// +9h and stop and release the point effect at +4h.
struct PlaneEngineBase {
    bool stopped_9{false};
    bool has_effect_4{false};
};
void engine_base_reset_007e1c00(PlaneEngineBase& engine) noexcept;
void engine_base_mark_stopped_007ddf00(PlaneEngineBase& engine) noexcept;
// 007E1BA0, slot 0 of 00D08640: the same body as 007E1C00.
void engine_base_init_007e1ba0(PlaneEngineBase& engine) noexcept;
// 007DDEB0, slot +4h of 00D08640: its own slot +8h (007E1C00), argument unread.
void engine_base_restart_007ddeb0(PlaneEngineBase& engine, bool running) noexcept;

// 007DE5B0 BSP_PlaneEngineList_AnyBurning, __fastcall(ECX = list at
// unit+C98h): true when any engine's byte +8h is set. 007CAF10 asks it before
// raising "enginefire" (docs/GUN_AIM_TERMS.md).
bool engine_list_any_burning_007de5b0(const std::vector<PlaneEngineSlots*>& list) noexcept;

// 007E23E0 BSP_PlaneEngine_OnDestroyed, __thiscall(engine, plane, index),
// RET 8. Only while burning_8 is clear: slot +0Ch (stop) unless stopped_9;
// with a plane whose +4A4h (the Lua self) is set, a point effect of the class
// definition +220h is created at the engine's point (returned true); then
// burning_8 = true (007E24F7, on every path through the body).
bool engine_on_destroyed_007e23e0(PlaneEngineSlots& engine, bool plane_has_lua_self_4a4);

// 007EB0B0 BSP_PlaneEngineList_DestroyEngineAt, __thiscall(list, int i),
// RET 4: for 0 <= i < count, 007E23E0(list[i], list+4h (the plane), i). The
// only caller is 007D0B80's message-101 ("enginefire") arm.
bool engine_list_destroy_at_007eb0b0(const std::vector<PlaneEngineSlots*>& list, int index,
                                     bool plane_has_lua_self_4a4);

// 007DE490 BSP_PlaneEngineSound_StepParams, __thiscall(this, float dt), RET 4.
// Runs only with a sound at +10h. Two rate-limited parameters:
//   p1 = 0042AC60(p1 +14h, target +1Ch, float(rate +18h * dt));
//   p2 = 0042AC60(p2 +20h, target +28h, float(rate +24h * dt));
// then sound slot +10h(p1) and slot +0Ch(p2).
struct EngineSoundParams {
    bool has_sound_10{false};
    float p1_14{0.0f};
    float rate1_18{0.0f};
    float target1_1c{0.0f};
    float p2_20{0.0f};
    float rate2_24{0.0f};
    float target2_28{0.0f};
};
struct EngineSoundCalls {
    bool called{false};
    float slot10_value{0.0f};
    float slot0c_value{0.0f};
};
EngineSoundCalls engine_sound_step_007de490(EngineSoundParams& sound, float dt) noexcept;

// ---------------------------------------------------------------------------
// 3. The damage-effect switch, 007E27B0 BSP_Plane_UpdateDamageEffects.
// ---------------------------------------------------------------------------

// The object at unit+E80h. Effect definitions are indices into the class list
// [unit+538h]+224h (begin +228h, end +22Ch); -1 is "none".
struct DamageEffectSwitch {
    int definition_0{-1};        // the definition held (+0h, a refcounted ptr)
    bool has_effect_4{false};    // the live effect at +4h is non-null
    int effect_definition_84{-1};// [+4h]+84h, the effect's own definition
    float damage_0c{0.0f};       // 1 - health, this call
    float prev_damage_10{0.0f};
    float timer_14{0.0f};
    float flicker_18{0.0f};
    int state_1c{0};
};

struct DamageEffectStep {
    bool ran{false};                 // the class list is non-empty
    bool drew{false};                // the stream-0 draw happened
    bool flicker_sent{false};        // 007DE760(effect, 0, flicker)
    bool definition_set{false};      // 007E2600 was called
    int set_definition_calls{0};
};

// __thiscall(this, float dt), RET 4. Bytes 007E27B0-007E2994.
//   d = float(1 - health); damage_0c = d;
//   n = trunc(00419010(0, 0, 0.6, 1, d) * (count + 1)) (FIMUL, then
//       00BF7420), n = min(n, count);
//   state 0: when n > 0 and prev_damage != d: timer = 10.0f (00CE38B8),
//            flicker = 1.0f, state = 1;
//   state 1: timer = float(timer - dt); r = U0(0, 1) (STREAM 0, 007E2862);
//            flicker = 0042AC60(flicker, float(r * r), 0.5f);
//            with an effect: 007DE760(effect, 0, flicker);
//            timer >= 0 (COMISS 0 > timer is false): definition = list[max(n-1,0)];
//            timer < 0: state = 0, definition = none, 007E2600 (one call);
//            then when definition != effect_definition_84: 007E2600 again;
//   prev_damage = d on every path past the list test.
// The model records that 007E2600 ran; it sets effect_definition_84 to the
// definition it installs, which is what 007E2600 hands the new effect.
DamageEffectStep damage_effect_switch_007e27b0(DamageEffectSwitch& self, float dt,
                                               float health, int list_count,
                                               const Rng& rng);

// ---------------------------------------------------------------------------
// 4. The flight controller's state record and resets.
// ---------------------------------------------------------------------------

// The controller fields these functions touch (ctl = unit+AB0h; finding 8 of
// docs/CC13_GP_007A_007F.md). dyn_* fields live in the object at ctl+10h.
struct FlightControllerFields {
    bool flag_04{false};
    bool flag_05{false};
    bool motion_reset_14{false};
    Vec3 world_velocity_18{};
    Vec3 angular_velocity_24{};
    Vec3 world_velocity_copy_30{};
    Vec3 body_velocity_3c{};      // +44h is the body forward speed
    Vec3 body_angular_48{};
    Vec3 vec_54{};
    Vec3 vec_60{};
    float f_6c{0.0f};
    float forward_accel_78{0.0f};
    float forward_latch_7c{0.0f};
    bool byte_94{false};
    float f_a0{0.0f};
    float f_a4{0.0f};
    float f_ac{0.0f};
    std::array<float, 16> matrix_b0{};
    Vec3 dyn_b4{};
    float dyn_c0{0.0f};
    float dyn_level_speed_cc{0.0f};
};

// The 3Ch-byte control-state record (PILOT_COMMAND_PATH.md: 007C2970 builds
// it on the stack for the plane control-state message).
struct ControlStateRecord {
    Vec3 world_velocity_00{};
    Vec3 body_velocity_0c{};
    Vec3 angular_velocity_18{};
    bool flag05_24{false};
    bool flag04_25{false};
    bool body_space_26{false};   // apply: rebuild the velocity from +0Ch
    float f_ac_28{0.0f};
    float dyn_c0_2c{0.0f};
    Vec3 dyn_b4_30{};
};

// 007D8330 BSP_PlaneFlightController_WriteStateRecord, __thiscall(ctl, rec*),
// RET 4: clears +2Ch, +28h, +25h, +24h, +26h, then copies (the order of the
// copies has no observable effect).
ControlStateRecord flight_controller_write_record_007d8330(
    const FlightControllerFields& ctl) noexcept;

// What 007DB1F0 / 007D9EE0 / 007DB2C0 call on the controller.
struct FlightControllerHost {
    // 0042D0D0 BSP_Vector3f_TransformDirectionOptionalNormalize(out, v,
    // [ctl+8]+74h, 0): the body-to-world rotation of the unit.
    std::function<Vec3(const Vec3& body)> body_to_world_direction;
    std::function<void()> refresh_body_frame_007d9c10;
    std::function<void(float dt)> update_rate_of_44_007d80c0;
    std::function<void()> body_to_world_velocity_007d9c80;
    // 007C47F0 on [ctl+8]+538h; absent when ctl+10h, ctl+8h or the class is null.
    std::function<bool(float& level_speed)> class_level_flight_speed_007c47f0;
};

// 007DB1F0 BSP_PlaneFlightController_ApplyStateRecord, __thiscall(ctl, rec*),
// RET 4. world velocity = body_space_26 ? body_to_world(rec+0Ch) : rec+00h;
// +30h = +18h; angular = rec+18h; 007D9C10; 007D80C0(0); then +4h, +5h, +ACh
// and the dyn fields from the record.
void flight_controller_apply_record_007db1f0(FlightControllerFields& ctl,
                                             const ControlStateRecord& rec,
                                             const FlightControllerHost& host);

// 007D9EE0 BSP_PlaneFlightController_SetWorldVelocity, __thiscall(ctl,
// const Vec3*), RET 4: world velocity = v, angular = the zero vector
// 00F87574, +30h = +18h, 007D9C10, 007D80C0(0). (The ctl+0Ch refresh from
// unit+538h when null is the host's.)
void flight_controller_set_world_velocity_007d9ee0(FlightControllerFields& ctl,
                                                   const Vec3& v,
                                                   const FlightControllerHost& host);

// 007DB2C0 BSP_PlaneFlightController_ResetMotion, __fastcall(ctl), plain RET.
// Unless motion_reset_14: +48h = zero, +3Ch = +48h, +60h = +3Ch, +54h = +60h,
// +7Ch = +6Ch = +78h = 0, +94h = 0, the 4x4 at +B0h = all zero (a zero
// matrix, not identity), +ACh = +A0h = +A4h = 0, 007D9C80, +30h = +18h,
// motion_reset_14 = true. Then, on every call with the dyn object, unit and
// class present: dyn+CCh = float(007C47F0() * 1.05) (the double 00CF2510,
// 1.0499999523162842).
void flight_controller_reset_motion_007db2c0(FlightControllerFields& ctl,
                                             const FlightControllerHost& host);

// 007D7D70 BSP_PlaneDyn_SetGroundContactUp, __fastcall(ECX = dyn): dyn+98h..A0h
// = (0, 1, 0) and dyn+A4h byte = 1.
struct GroundContactUp {
    Vec3 up_98{};
    bool contact_a4{false};
};
void plane_dyn_set_ground_contact_up_007d7d70(GroundContactUp& dyn) noexcept;

}  // namespace gameplay_seg48_cc13
}  // namespace bsp
