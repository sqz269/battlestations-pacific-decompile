// Segment 48 (007D1E50-007F8390) reference models: the plane parts. Packet
// cc13-gpr48; include/bsp/gameplay_seg48_cc13.hpp carries the per-function
// evidence and the adoption note, docs/CC13_GPR_48.md the tables.
// Reconstructed and build-tested only: not wired, not game-run.

#include "bsp/gameplay_seg48_cc13.hpp"

namespace bsp::gameplay_seg48_cc13 {
void engine_step_rpm_noop_007e23b0(float, float) noexcept {}
void engine_apply_spin_noop_007e23c0(float) noexcept {}
void engine_blur_blend_noop_007e23d0() noexcept {}
}

#include "bsp/dive_bomb_task.hpp"  // dive_bomb_interpolate_clamped_00419010
#include "bsp/plane_wanderer.hpp"   // plane_wanderer_random_direction_00439950
#include "bsp/unit_motion.hpp"     // unit_step_towards_0042ac60

#include <cmath>
#include <utility>

namespace bsp {
namespace gameplay_seg48_cc13 {

namespace {

constexpr float kOne = 1.0f;           // 00D7A24C
constexpr float kMinusOne = -1.0f;     // 00D7A260
constexpr float kZero = 0.0f;          // 00D7A218
constexpr float kTwoPiF = 6.2831855f;  // 00CE3D9C, the float upper bound of the angle draws
constexpr double kTwoPiD = 6.2831854820251465;  // 00CE3828, the fmod modulus
constexpr float kSpreadLoHi = 0.2f;    // 00CE54A0
constexpr float kSpreadHiLo = 8.0f;    // 00CE3918
constexpr float kDamageFull = 0.6f;    // 00CE3D30
constexpr float kFlickerTime = 10.0f;  // 00CE38B8
constexpr float kFlickerStep = 0.5f;   // 00CE3800
constexpr double kLevelSpeedMul = 1.0499999523162842;  // 00CF2510

float interpolate_clamped(float x0, float y0, float x1, float y1, float x) noexcept {
    return bsp::dive_bomb_interpolate_clamped_00419010(x0, y0, x1, y1, x);
}

// The image's equality tests are LAHF / TEST AH,44h: a NaN operand is unequal.
bool not_equal(float a, float b) noexcept { return !(a == b); }

float wrap_angle(float a) noexcept {
    // FLD the float sum, FLD the double 2 pi, CALL 00BF857A (fmod), FSTP float.
    return static_cast<float>(std::fmod(static_cast<double>(a), kTwoPiD));
}

void set_visible(std::vector<NodeVisibility>& out, std::uintptr_t node, float factor) {
    out.push_back(NodeVisibility{node, factor});
}

}  // namespace

// ---------------------------------------------------------------------------
// 1. Actuator animation.
// ---------------------------------------------------------------------------

float actuator_channel_anim_time_007de7e0(const ActuatorAnimChannel& channel,
                                          float clock_f876a4) noexcept {
    float v = channel.value_4;  // 007DE7E7
    // 007DE7E3 / 007DE7FE: moving, and COMISS stamp, 0.0 with JB (NaN skips).
    if (channel.moving_8 && channel.stamp_18 >= kZero) {
        const float delta = clock_f876a4 - channel.stamp_18;          // 007DE81B
        const float sign = channel.target_1 ? kOne : kMinusOne;       // 007DE80D
        const float step = channel.rate_0c * sign * delta;            // 007DE842
        v = step + channel.value_4;                                   // 007DE84E
    }
    return interpolate_clamped(0.0f, channel.anim_lo_10, 1.0f, channel.anim_hi_14, v);
}

ActuatorAnimResult actuator_block_anim_007de9a0(ActuatorAnimBlock& block,
                                                float clock_f876a4) noexcept {
    ActuatorAnimResult result;
    if (block.owner_suppressed_5d) {
        return result;  // 007DE9AD
    }
    const float old_time = block.shown_time_7c;
    float time = old_time;
    const ActuatorAnimChannel& a = block.channel_44;
    const ActuatorAnimChannel& b = block.channel_28;
    const ActuatorAnimChannel& c = block.channel_60;
    if (a.enabled_0 && not_equal(a.value_4, kOne)) {
        time = actuator_channel_anim_time_007de7e0(a, clock_f876a4);  // 007DEA14
        result.source_offset = 0x44;
    } else {
        bool done = false;
        if (b.enabled_0) {
            time = actuator_channel_anim_time_007de7e0(b, clock_f876a4);  // 007DE9EB
            result.source_offset = 0x28;
            // 007DE9F4-007DEA09: the enabled byte is re-read, then +2Ch != 0.0f.
            done = b.enabled_0 && not_equal(b.value_4, kZero);
        }
        if (!done && c.enabled_0) {
            time = actuator_channel_anim_time_007de7e0(c, clock_f876a4);  // 007DEA14
            result.source_offset = 0x60;
        }
    }
    // 007DEA2A FUCOMIP, JNP skips only an ordered equal pair.
    if (not_equal(old_time, time)) {
        block.shown_time_7c = time;  // 007DEA43
        result.pushed = true;        // [[+20h]]+24h(+24h, time, 1) at 007DEA51
        result.time = time;
    }
    return result;
}

// ---------------------------------------------------------------------------
// 2. Engines and rotors.
// ---------------------------------------------------------------------------

std::vector<float> rotor_speed_spread_007eca0e(int engine_count, const Rng& rng) {
    const float lo = rng.uniform(1, 0.0f, kSpreadLoHi);  // 007ECA23
    const float hi = rng.uniform(1, kSpreadHiLo, kOne);  // 007ECA41
    const int size = engine_count + 1;
    std::vector<float> spread;
    if (size <= 0) {
        return spread;  // 007ECA52: JLE past the fill
    }
    spread.resize(static_cast<std::size_t>(size));
    const float span = hi - lo;                                // 007ECA5B, stored as a double
    const float divisor = static_cast<float>(size - 1);        // CVTSI2SS of edi - 1
    for (int i = 0; i < size; ++i) {
        const float q = static_cast<float>(i) / divisor;       // 007ECA7A-007ECA7E
        spread[static_cast<std::size_t>(i)] = q * span + lo;   // 007ECAA3-007ECAAF
    }
    // 007ECAB5-007ECB1C: a Fisher-Yates pass from index 1 upward.
    for (std::uint32_t k = 1; k < static_cast<std::uint32_t>(size); ++k) {
        const std::uint32_t j = rng.next_u32(1) % (k + 1);    // 007ECADD, DIV
        std::swap(spread[k], spread[j]);
    }
    return spread;
}

RotorInitResult rotor_init_007ebf10(PlaneRotor& rotor, const PlaneRotor& nodes, int index,
                                    float speed_factor, const RotorTuning& tuning,
                                    const Rng& rng) {
    RotorInitResult result;
    // AND 80000001h with the negative fix-up: the C remainder of index / 2.
    rotor.spin_sign_2c = (index % 2 != 0) ? 1.0f : -1.0f;
    rotor.blur_0c = nodes.blur_0c;
    rotor.blur2_10 = nodes.blur2_10;
    rotor.still_14 = nodes.still_14;
    rotor.still2_18 = nodes.still2_18;
    rotor.still_dam_1c = nodes.still_dam_1c;
    rotor.still_dam2_20 = nodes.still_dam2_20;
    // 007EC07B-007EC0A7: (SpeedRandom * factor + 1.0) * SpeedBase, FSTP float.
    const float scaled = tuning.speed_random_168 * speed_factor;
    const float plus_one = scaled + 1.0f;
    rotor.spin_rate_30 = plus_one * tuning.speed_base_160;
    set_visible(result.visibility, rotor.still_dam_1c, 0.0f);  // 007EC0AF
    set_visible(result.visibility, rotor.still_14, 1.0f);      // 007EC0BF
    if (rotor.still_dam2_20 != 0) {
        set_visible(result.visibility, rotor.still_dam2_20, 0.0f);  // 007EC0D3
    }
    if (rotor.still2_18 != 0) {
        set_visible(result.visibility, rotor.still2_18, 0.0f);  // 007EC0E7
    }
    rotor.shown_still_24 = rotor.still_14;
    rotor.shown_still2_28 = rotor.still2_18;
    const float angle = rng.uniform(1, 0.0f, kTwoPiF);  // 007EC10F
    rotor.shown_angle_38 = angle;
    rotor.angle_3c = angle;
    rotor.stopped_9 = false;  // 007EC130; the effect at +4h is stopped and released
    return result;
}

std::vector<NodeVisibility> rotor_reset_007e1c60(PlaneRotor& rotor, const Rng& rng) {
    std::vector<NodeVisibility> out;
    set_visible(out, rotor.still_dam_1c, 0.0f);
    if (rotor.still_dam2_20 != 0) {
        set_visible(out, rotor.still_dam2_20, 0.0f);
    }
    set_visible(out, rotor.still_14, 1.0f);
    if (rotor.still2_18 != 0) {
        set_visible(out, rotor.still2_18, 0.0f);
    }
    rotor.shown_still2_28 = rotor.still2_18;
    rotor.shown_still_24 = rotor.still_14;
    const float angle = rng.uniform(1, 0.0f, kTwoPiF);  // 007E1CCF
    rotor.shown_angle_38 = angle;
    rotor.angle_3c = angle;
    rotor.stopped_9 = false;  // 007E1C00, tail-jumped at 007E1CE8
    return out;
}

std::vector<NodeVisibility> rotor_restart_007ddec0(PlaneRotor& rotor, bool running,
                                                   const RotorTuning& tuning,
                                                   const Rng& rng) {
    std::vector<NodeVisibility> out = rotor_reset_007e1c60(rotor, rng);  // vtable +8h
    rotor.rpm_34 = running ? kOne : 0.0f;                                 // 007DDEE0 / 007DDEF5
    const std::vector<NodeVisibility> blend = rotor_blur_blend_007ec3b0(rotor, tuning);
    out.insert(out.end(), blend.begin(), blend.end());
    return out;
}

std::vector<NodeVisibility> rotor_stop_engine_007ddf10(PlaneRotor& rotor) {
    std::vector<NodeVisibility> out;
    if (rotor.stopped_9) {
        return out;
    }
    rotor.stopped_9 = true;
    set_visible(out, rotor.still_14, 0.0f);      // 007DDF28
    set_visible(out, rotor.still_dam_1c, 1.0f);  // 007DDF38
    if (rotor.still2_18 != 0) {
        set_visible(out, rotor.still2_18, 0.0f);  // 007DDF4C
    }
    if (rotor.still_dam2_20 != 0) {
        set_visible(out, rotor.still_dam2_20, 1.0f);  // 007DDF60
    }
    rotor.shown_still_24 = rotor.still_dam_1c;
    rotor.shown_still2_28 = rotor.still_dam2_20;
    return out;
}

void rotor_step_rpm_007ec180(PlaneRotor& rotor, float target, float dt,
                             const RotorTuning& tuning) noexcept {
    const float max_step = tuning.rotor_speed_change_17c * dt;  // 007EC190
    const float goal = rotor.stopped_9 ? 0.0f : target;         // 007EC1AC
    rotor.rpm_34 = bsp::unit_step_towards_0042ac60(rotor.rpm_34, goal, max_step);
    const float advance = rotor.spin_rate_30 * rotor.rpm_34 * dt + rotor.angle_3c;  // 007EC1CA
    rotor.angle_3c = wrap_angle(advance);  // 007EC1E6
}

float rotor_apply_spin_007ec1f0(PlaneRotor& rotor, float dt) noexcept {
    const float advance = rotor.rpm_34 * rotor.spin_rate_30 * dt + rotor.angle_3c;  // 007EC209
    rotor.shown_angle_38 = wrap_angle(advance);  // 007EC228
    const float signed_angle = rotor.shown_angle_38 * rotor.spin_sign_2c;  // 007EC235
    return -signed_angle;  // 007EC23D FCHS
}

std::vector<NodeVisibility> rotor_blur_blend_007ec3b0(const PlaneRotor& rotor,
                                                      const RotorTuning& tuning) noexcept {
    std::vector<NodeVisibility> out;
    const float s = interpolate_clamped(tuning.still_only_speed_16c, 1.0f,
                                        tuning.blurred_only_speed_170, 0.0f, rotor.rpm_34);
    std::uintptr_t second = 0;
    if (rotor.shown_still_24 == rotor.still_dam_1c) {
        set_visible(out, rotor.still_14, 0.0f);  // 007EC400
        second = rotor.still2_18;
    } else {
        set_visible(out, rotor.still_dam_1c, 0.0f);  // 007EC40A
        second = rotor.still_dam2_20;
    }
    if (second != 0) {
        set_visible(out, second, 0.0f);  // 007EC41E
    }
    const float blur = 1.0f - s;  // DE E1 (FSUBRP): 1 - s, FSTP float
    set_visible(out, rotor.shown_still_24, s);  // 007EC430
    if (rotor.shown_still2_28 != 0) {
        set_visible(out, rotor.shown_still2_28, blur);  // 007EC452
    }
    set_visible(out, rotor.blur_0c, blur);  // 007EC470
    if (rotor.blur2_10 != 0) {
        set_visible(out, rotor.blur2_10, blur);  // 007EC486
    }
    return out;
}

void engine_list_spin_all_007de540(const std::vector<PlaneEngineSlots*>& list, float dt) {
    for (PlaneEngineSlots* e : list) {
        e->apply_spin(dt);
    }
}

void engine_list_blur_all_007de580(const std::vector<PlaneEngineSlots*>& list) {
    for (PlaneEngineSlots* e : list) {
        e->blur_blend();
    }
}

void engine_list_stop_at_007de5e0(const std::vector<PlaneEngineSlots*>& list, int index) {
    list[static_cast<std::size_t>(index)]->stop_engine();
}

void engine_list_stop_all_007de600(const std::vector<PlaneEngineSlots*>& list) {
    for (PlaneEngineSlots* e : list) {
        e->stop_engine();
    }
}

void engine_list_reset_all_007de630(const std::vector<PlaneEngineSlots*>& list) {
    for (PlaneEngineSlots* e : list) {
        e->reset();
    }
}

void engine_base_reset_007e1c00(PlaneEngineBase& engine) noexcept {
    engine.stopped_9 = false;
    engine.has_effect_4 = false;  // 00867B10 stop, release, +4h = 0
}

void engine_base_mark_stopped_007ddf00(PlaneEngineBase& engine) noexcept {
    engine.stopped_9 = true;
}

void engine_base_init_007e1ba0(PlaneEngineBase& engine) noexcept {
    engine_base_reset_007e1c00(engine);  // byte-identical body
}

void engine_base_restart_007ddeb0(PlaneEngineBase& engine, bool running) noexcept {
    static_cast<void>(running);  // RET 4; the argument is not read
    engine_base_reset_007e1c00(engine);  // CALL [vtable+8h]
}

bool engine_list_any_burning_007de5b0(const std::vector<PlaneEngineSlots*>& list) noexcept {
    for (const PlaneEngineSlots* e : list) {
        if (e->burning_8) {
            return true;
        }
    }
    return false;
}

bool engine_on_destroyed_007e23e0(PlaneEngineSlots& engine, bool plane_has_lua_self_4a4) {
    if (engine.burning_8) {
        return false;  // 007E2401
    }
    if (!engine.stopped_9) {
        engine.stop_engine();  // 007E2413, slot +0Ch
    }
    engine.burning_8 = true;  // 007E24F7
    return plane_has_lua_self_4a4;  // 007E2419-007E2428 gate the effect
}

bool engine_list_destroy_at_007eb0b0(const std::vector<PlaneEngineSlots*>& list, int index,
                                     bool plane_has_lua_self_4a4) {
    if (index < 0 || index >= static_cast<int>(list.size())) {
        return false;  // 007EB0B6 / 007EB0BB
    }
    return engine_on_destroyed_007e23e0(*list[static_cast<std::size_t>(index)],
                                        plane_has_lua_self_4a4);
}

EngineSoundCalls engine_sound_step_007de490(EngineSoundParams& sound, float dt) noexcept {
    EngineSoundCalls calls;
    if (!sound.has_sound_10) {
        return calls;
    }
    LerpValue& p = sound.pitch_14;
    const float step1 = p.rate * dt;  // 007DE4B1
    p.current = bsp::unit_step_towards_0042ac60(p.current, p.target, step1);
    LerpValue& v = sound.volume_20;
    const float step2 = v.rate * dt;  // 007DE4D9
    v.current = bsp::unit_step_towards_0042ac60(v.current, v.target, step2);
    calls.called = true;
    calls.pitch = p.current;   // 007DE4FE
    calls.volume = v.current;  // 007DE50E
    return calls;
}

// ---------------------------------------------------------------------------
// 3. The damage-effect switch.
// ---------------------------------------------------------------------------

namespace {

void set_definition_007e2600(DamageEffectSwitch& self, DamageEffectStep& step) {
    // 007E2600 stops the live effect and replaces it with one built from the
    // held definition (none releases it). Both callers (007E28CB / 007E293D)
    // pass the switch itself, so the definition tested is the held +0h.
    if (self.has_effect_4) ++step.stop_children_calls;  // 007E262F 00867B10, +9h = 1
    if (self.definition_0 >= 0) {
        ++step.effects_created;  // 007E26F2 008687C0 from the held +0h; stored at +4h
        if (self.has_effect_4) ++step.effects_released;  // 007E271F the old one released
        self.has_effect_4 = true;
        self.effect_definition_84 = self.definition_0;
    } else {
        if (self.has_effect_4) ++step.effects_released;  // 007E2774..007E2788
        self.has_effect_4 = false;  // 007E2791 +4h = 0
        self.effect_definition_84 = self.definition_0;
    }
    step.definition_set = true;
    ++step.set_definition_calls;
}

}  // namespace

DamageEffectStep damage_effect_switch_007e27b0(DamageEffectSwitch& self, float dt,
                                               float health, int list_count,
                                               const Rng& rng) {
    DamageEffectStep step;
    if (list_count <= 0) {
        return step;  // 007E27CC / 007E27DD
    }
    step.ran = true;
    const float d = 1.0f - health;  // 007E27EA DE E1: 1 - health
    self.damage_0c = d;
    const float ramp = interpolate_clamped(0.0f, 0.0f, kDamageFull, 1.0f, d);
    // FIMUL by count + 1 (the product stays in ST0), then 00BF7420 truncates.
    const float scaled = ramp * static_cast<float>(list_count + 1);
    int n = static_cast<int>(scaled);
    if (n > list_count) {
        n = list_count;  // 007E282C
    }
    if (self.state_1c == 0) {
        // 007E2953: n > 0 and the damage moved.
        if (n > 0 && not_equal(self.prev_damage_10, d)) {
            self.timer_14 = kFlickerTime;
            self.flicker_18 = kOne;
            self.state_1c = 1;
        }
        self.prev_damage_10 = self.damage_0c;  // 007E2989
        return step;
    }
    if (self.state_1c != 1) {
        self.prev_damage_10 = self.damage_0c;  // 007E2841 JNE 007E2989
        return step;
    }
    self.timer_14 = self.timer_14 - dt;              // 007E2854
    const float r = rng.uniform(0, 0.0f, 1.0f);      // 007E2862, ECX = 0
    step.drew = true;
    const float r2 = r * r;                          // 007E2881, FSTP float
    self.flicker_18 = bsp::unit_step_towards_0042ac60(self.flicker_18, r2, kFlickerStep);
    if (self.has_effect_4) {
        step.flicker_sent = true;                    // 007E28AC 007DE760(0, flicker)
    }
    if (kZero > self.timer_14) {                     // 007E28B4 COMISS, JBE
        self.state_1c = 0;
        self.definition_0 = -1;                      // 004C2C40(0)
        set_definition_007e2600(self, step);         // 007E28CE
    } else {
        int index = n - 1;                           // 007E28F7
        if (index < 0) {
            index = 0;
        } else if (index > list_count) {
            index = list_count;                      // unreachable: n <= count
        }
        self.definition_0 = index;                   // 007DDF80 then 00440580
    }
    const int live = self.has_effect_4 ? self.effect_definition_84 : -1;  // 007E291C
    if (self.definition_0 != live) {
        set_definition_007e2600(self, step);         // 007E2940
    }
    self.prev_damage_10 = self.damage_0c;            // 007E2949 / 007E298C
    return step;
}

// ---------------------------------------------------------------------------
// 4. The flight controller.
// ---------------------------------------------------------------------------

ControlStateRecord flight_controller_write_record_007d8330(
    const FlightControllerFields& ctl) noexcept {
    ControlStateRecord rec;
    rec.world_velocity_00 = ctl.world_velocity_18;
    rec.body_velocity_0c = ctl.body_velocity_3c;
    rec.angular_velocity_18 = ctl.angular_velocity_24;
    rec.dyn_b4_30 = ctl.dyn_b4;
    rec.dyn_c0_2c = ctl.dyn_c0;
    rec.turbo_25 = ctl.turbo_04;
    rec.flag05_24 = ctl.flag_05;
    rec.f_ac_28 = ctl.f_ac;
    rec.body_space_26 = false;
    return rec;
}

void flight_controller_apply_record_007db1f0(FlightControllerFields& ctl,
                                             const ControlStateRecord& rec,
                                             const FlightControllerHost& host) {
    if (!rec.body_space_26) {
        ctl.world_velocity_18 = rec.world_velocity_00;
    } else {
        ctl.world_velocity_18 = host.body_to_world_direction(rec.body_velocity_0c);
    }
    ctl.world_velocity_copy_30 = ctl.world_velocity_18;
    ctl.angular_velocity_24 = rec.angular_velocity_18;
    host.refresh_body_frame_007d9c10();
    host.update_rate_of_44_007d80c0(0.0f);
    ctl.turbo_04 = rec.turbo_25;
    ctl.flag_05 = rec.flag05_24;
    ctl.f_ac = rec.f_ac_28;
    ctl.dyn_b4 = rec.dyn_b4_30;
    ctl.dyn_c0 = rec.dyn_c0_2c;
}

void flight_controller_set_world_velocity_007d9ee0(FlightControllerFields& ctl,
                                                   const Vec3& v,
                                                   const FlightControllerHost& host) {
    ctl.world_velocity_18 = v;
    ctl.angular_velocity_24 = Vec3{};  // 00F87574..7C, the zero vector
    ctl.world_velocity_copy_30 = ctl.world_velocity_18;
    host.refresh_body_frame_007d9c10();
    host.update_rate_of_44_007d80c0(0.0f);
}

void flight_controller_reset_motion_007db2c0(FlightControllerFields& ctl,
                                             const FlightControllerHost& host) {
    if (!ctl.motion_reset_14) {
        ctl.body_angular_48 = Vec3{};
        ctl.body_velocity_3c = ctl.body_angular_48;
        ctl.vec_60 = ctl.body_velocity_3c;
        ctl.vec_54 = ctl.vec_60;
        ctl.forward_latch_7c = 0.0f;
        ctl.f_6c = 0.0f;
        ctl.forward_accel_78 = 0.0f;
        ctl.byte_94 = false;
        ctl.matrix_b0.fill(0.0f);  // 004134F0 from an all-zero stack matrix
        ctl.f_ac = 0.0f;
        ctl.f_a0 = 0.0f;
        ctl.f_a4 = 0.0f;
        host.body_to_world_velocity_007d9c80();
        ctl.world_velocity_copy_30 = ctl.world_velocity_18;
        ctl.motion_reset_14 = true;
    }
    float level_speed = 0.0f;
    if (host.class_level_flight_speed_007c47f0 &&
        host.class_level_flight_speed_007c47f0(level_speed)) {
        // 007DB40C FMUL qword then FSTP float. At 24-bit precision control the
        // image rounds the product once to 24 bits; the double product here is
        // rounded twice, which can differ only on an exact half-way tie.
        ctl.dyn_level_speed_cc =
            static_cast<float>(static_cast<double>(level_speed) * kLevelSpeedMul);
    }
}

SpeedTerms flight_controller_update_speed_terms_007d8020(const Vec3& world_velocity_18,
                                                         const Vec3& body_velocity_3c,
                                                         const Vec3& body_angular_48) noexcept {
    SpeedTerms out;
    const Vec3& v = world_velocity_18;
    // 007D802D-007D803B: (y*y + x*x) + z*z, each step rounded to 24 bits.
    const float yy_xx = v.y * v.y + v.x * v.x;
    const float s2 = yy_xx + v.z * v.z;
    float s = 0.0f;
    if (static_cast<double>(s2) > 1e-10) {  // 00CE3820, FCOMI / JBE
        s = std::sqrt(s2);                  // 00BF7030, FSTP float
    }
    out.airspeed_6c = s;
    out.body_x_over_speed_70 = body_velocity_3c.x / s;  // DE FA
    out.body_y_over_speed_74 = body_velocity_3c.y / s;  // D8 7E 40
    out.neg_body_rate_y_a8 = -0.0f - body_angular_48.y;  // 00D7A208 SUBSS
    out.body_rate_x_a4 = body_angular_48.x;
    out.body_rate_z_a0 = body_angular_48.z;
    return out;
}

float flight_forward_speed_007d99c0(float body_forward_44, bool has_parent,
                                    float parent_velocity_local_z) noexcept {
    if (!has_parent) {
        return body_forward_44;  // 007D99D4
    }
    const float z = parent_velocity_local_z;
    const float factor = (z >= kZero) ? 0.1f : 0.8f;  // 007D9A31 COMISS / JB
    return z * factor + body_forward_44;             // 007D9A58-007D9A5F
}
bool flight_controller_set_flag_94_007d8180(bool& flag_94, bool force, bool has_game,
                                            int game_mode_1fe4) noexcept {
    if (force || (has_game && (game_mode_1fe4 == 0 || game_mode_1fe4 == 1))) {
        flag_94 = true;  // 007D819F
        return true;
    }
    return false;
}

void plane_dyn_begin_step_007d7c00(PlaneDynAccumulators& dyn, const Vec3& world_velocity,
                                   const Vec3& body_velocity) noexcept {
    dyn.world_velocity_4c = world_velocity;
    dyn.body_velocity_64 = body_velocity;
    dyn.accumulators_04.fill(Vec3{});
    dyn.f_94 = 0.0f;  // 007D7D14 XORPS
}

FlightControllerDefaults flight_controller_construct_007d7ea0(float dead_meat_rotation_min,
                                                              float dead_meat_rotation_max,
                                                              const Rng& rng) {
    FlightControllerDefaults out;
    const float m = rng.uniform(1, dead_meat_rotation_min, dead_meat_rotation_max);  // 007D7F90
    float d[3] = {0.0f, 0.0f, 0.0f};
    bsp::plane_wanderer_random_direction_00439950(
        d, [&rng](float lo, float hi) { return rng.uniform(1, lo, hi); });  // 007D7F9D
    out.dead_meat_rotation_f0[0] = m * d[0];  // 007D7FBC FMUL [EAX]
    out.dead_meat_rotation_f0[1] = d[1] * m;  // 007D7FAB
    out.dead_meat_rotation_f0[2] = d[2] * m;  // 007D7FB6
    out.dead_meat_rotation_f0[3] = 0.0f;      // 007D7FDA, EBX = 0
    return out;
}

bool control_surface_step_007de2e0(ControlSurface& surface, float target, float node_ac,
                                   float& angle_y) noexcept {
    angle_y = 0.0f;
    if (!not_equal(surface.deflection_0c, target) || !(node_ac > kZero)) {
        return false;  // 007DE303 JNP / 007DE31B JBE
    }
    surface.deflection_0c =
        bsp::unit_step_towards_0042ac60(surface.deflection_0c, target, 0.05f);  // 00CE7638
    angle_y = -(surface.gain_8 * surface.deflection_0c);
    return true;
}

void plane_dyn_set_ground_contact_up_007d7d70(GroundContactUp& dyn) noexcept {
    dyn.up_98 = Vec3{0.0f, kOne, 0.0f};
    dyn.contact_a4 = true;
}


// ---------------------------------------------------------------------------
// 5. Archives and the timed flag.
// ---------------------------------------------------------------------------

ActuatorBlockArchive actuator_block_save_007deb70(const ActuatorAnimBlock& block) noexcept {
    ActuatorBlockArchive a;
    a.anim_frame = block.shown_time_7c;  // 007DEB8E
    a.gears = ActuatorChannelArchive{block.channel_28.target_1, block.channel_28.value_4};
    a.wings = ActuatorChannelArchive{block.channel_44.target_1, block.channel_44.value_4};
    a.baydoor = ActuatorChannelArchive{block.channel_60.target_1, block.channel_60.value_4};
    return a;
}

ActuatorAnimResult actuator_block_load_007dea60(ActuatorAnimBlock& block,
                                                const ActuatorBlockArchive& archive,
                                                float clock_f876a4) noexcept {
    block.shown_time_7c = archive.anim_frame;
    block.channel_28.target_1 = archive.gears.targst;
    block.channel_28.value_4 = archive.gears.curst;
    block.channel_44.target_1 = archive.wings.targst;
    block.channel_44.value_4 = archive.wings.curst;
    block.channel_60.target_1 = archive.baydoor.targst;
    block.channel_60.value_4 = archive.baydoor.curst;
    block.shown_time_7c = kMinusOne;  // 007DEB39, 00D7A260
    return actuator_block_anim_007de9a0(block, clock_f876a4);  // 007DEB3D
}

namespace {
template<class Value>
void debounce_timed_flag(Value& value, float& timer, float time_out, Value now, float dt) noexcept {
    if (now == value) {
        timer = 0.0f;  // 007EB51D
        return;
    }
    timer = timer + dt;  // 007EB4E8-007EB4F7: one float op under /fp:strict
    if (timer > time_out) {
        value = now;  // 007EB505
    }
}
}

void timed_flag_debounce_007eb4e0(TimedFlag& flag, bool now, float dt) noexcept {
    debounce_timed_flag(flag.value, flag.timer, flag.time_out, now, dt);
}

void timed_flag_debounce_007eb4e0(std::uint8_t& value, float& timer, float time_out,
                                std::uint8_t now, float dt) noexcept {
    debounce_timed_flag(value, timer, time_out, now, dt);
}

// ---- cc13-gpr48b: 007D9360 --------------------------------------------------

TrialDynamics trial_step_forces_007d9360(const Vec3& body, const Vec3& world, float angle,
                                         const TrialClass& cls, const TrialGlobals& g,
                                         const TrialCallbacks& cb) {
    TrialDynamics d;
    // 007D9408..007D945B: the squares are kept (as doubles at 24-bit precision)
    // for the second magnitude at 007D96B0.
    const float x2 = world.x * world.x;
    const float y2 = world.y * world.y;
    const float z2 = world.z * world.z;
    const float s2 = (y2 + x2) + z2;
    const float speed = (static_cast<double>(s2) > 1e-10)       // 00CE3820
        ? static_cast<float>(std::sqrt(static_cast<double>(s2))) : 0.0f;
    const float ratio = speed / cls.speed_ref_184;               // 007D946D, stored
    // 007D94F1..007D9592: thrust.
    float thrust = cls.thrust_164;
    if (0.0f > angle) {
        const float b = cb.interpolate_clamped_00419010(g.climb_blend_x0_f87408, 0.0f,
            g.climb_blend_x1_f8740c, 1.57079637f /* 00CE3C64 */, -0.0f - angle);
        const float s = static_cast<float>(std::sin(static_cast<double>(b)));
        thrust = thrust * ((g.climb_thrust_k_f87404 - 1.0f) * s + 1.0f);
    }
    float tc;
    if (0.0f > thrust) tc = 0.0f;
    else tc = (static_cast<double>(thrust) > 100.0) ? 100.0f : thrust;  // 00D7A220 / 00CE3D08
    d.thrust_24 += tc;
    // 007D959C..007D95DE: the pitch term from the BODY velocity.
    float az = body.z;
    if (!(az > 0.0f)) az = -0.0f - az;
    const float p = (1.0f > az) ? 0.0f : (-body.y) / body.z;
    // 007D95E2..007D9720.
    const float q = ratio / g.level_flight_f8732c;
    const float base = (1.0f > q) ? q * q : 1.0f;
    const float m = base * (p + 1.0f);
    float c;
    if (-2.0f > m) c = -2.0f;                  // 00CE7D7C
    else c = (m > 2.0f) ? 2.0f : m;            // 00CE3958
    const double gravity = 9.8100004196167;    // 00CF9058
    const float kc = g.accel_cheat_mul_f873fc * c;
    d.lift_20 = static_cast<float>(d.lift_20 + static_cast<double>(kc) * gravity);
    d.gravity_2c = static_cast<float>(d.gravity_2c - static_cast<double>(g.accel_cheat_mul_f873fc) * gravity);
    // 007D968B..007D97A8: drag along the world velocity.
    const float drag = cb.drag_007d9140(speed, 1.0f, angle);
    const float mag = static_cast<float>(std::sqrt(static_cast<double>((x2 + y2) + z2)));
    const float inv = (mag > 0.0f) ? 1.0f / mag : 0.0f;
    const Vec3 u{inv * world.x, world.y * inv, inv * world.z};
    d.drag_10.x += drag * u.x;
    d.drag_10.y += drag * u.y;
    d.drag_10.z += drag * u.z;
    // 007D97AF..007D9817: the aero terms.
    const float tx = -body.x * cls.aero_x_174;
    const float ty = -body.y * cls.aero_y_170;
    const float a = cb.aero_007d92b0(ratio);
    d.aero_04 += a * tx;
    d.aero_08 += a * ty;
    return d;
}

Vec3 trial_integrate_at_climb_angle_007d9360(float angle, float probe_speed, float dt, int count,
                                             const TrialClass& cls, const TrialGlobals& g,
                                             const TrialCallbacks& cb) {
    Vec3 body{0.0f, 0.0f, probe_speed};          // 007D93DC..007D93F1
    Vec3 world = cb.body_to_world(body);         // 007D93F6
    for (int i = 0; i < count; ++i) {            // 007D9400 JLE, 007D9829 SUB EBX,1
        TrialDynamics d = trial_step_forces_007d9360(body, world, angle, cls, g, cb);
        cb.integrate_007d8470(d, body, world, dt);
    }
    return body;
}

// ---- cc13-ver1 -------------------------------------------------------------

float takeoff_speed_floor_007d87c7(float body_vz, float timer_c8, float floor_cc) {
    if (!(timer_c8 > 0.0f)) return body_vz;    // 007D87CF COMISS / 007D87D2 JBE
    if (floor_cc > body_vz) return floor_cc;   // 007D87E5 FCOMIP / 007D87E9 JBE
    return body_vz;
}

float takeoff_floor_speed_007db2c0(float level_flight_speed_007c47f0) {
    // double 00CF2510 = 3FF0CCCCC0000000, the float 1.05 widened.
    return static_cast<float>(static_cast<double>(level_flight_speed_007c47f0) *
                              1.0499999523162842);
}

float rate_law_floor_factor_007da710(float forward_speed, float desc_184, float pitch_c64,
                                     float stall_range_min, float stall_range_max,
                                     float stall_off_pitch, float stall_on_pitch) {
    const float ratio = forward_speed / desc_184;                  // 007DAAC5 FDIV, FSTP float
    const float by_pitch = dive_bomb_interpolate_clamped_00419010(  // 007DAAFE
        stall_off_pitch, 1.0f, stall_on_pitch, 0.0f, pitch_c64);
    const float by_speed = dive_bomb_interpolate_clamped_00419010(  // 007DAB31
        stall_range_min, 1.0f, stall_range_max, 0.0f, ratio);
    return by_speed * by_pitch;                                     // 007DAB36
}

float rate_law_deflection_floor_007da710(int axis, float current, float sp) {
    const float magnitude = std::fabs(current);   // AND 7FFFFFFFh (007DAC12, 007DAD9A)
    if (axis == 0) return (magnitude * 1.5f) * sp;
    if (axis == 1) return magnitude * sp;
    return -1.0f;
}

float dead_meat_forward_drag_007dbc77(float dyn_0c, bool ctl_94, float lost_drag_timer_c3c,
                                      float altitude_100, float forward_speed) {
    if (!ctl_94) return dyn_0c;                          // 007DBC77
    if (!(lost_drag_timer_c3c > 1.0f)) return dyn_0c;    // 007DBC8F COMISS against XMM0 = 1.0f
    const float excess = lost_drag_timer_c3c - 1.0f;     // 007DBCA9, double 1.0, FSTP float
    // 007DBCFC: x0 -8.0f (00CE3CC8), y0 10.0f (00CE38B8), x1 -1.0f (00D7A260), y1 1.0f.
    const float depth = dive_bomb_interpolate_clamped_00419010(-8.0f, 10.0f, -1.0f, 1.0f,
                                                               altitude_100);
    const float scaled = forward_speed * depth;          // 007DBD0C, rounded at 24 bits
    const float share = excess > 1.0f ? 1.0f : excess;   // 00415510(&1.0f, &excess)
    return dyn_0c - share * scaled;                      // 007DBD21 / 007DBD28 FSUBR
}

Vec3 velocity_lowpass_007dc762(const Vec3& prev_30, const Vec3& world_velocity_18, float step) {
    if (!(step < 0.25f) && !std::isnan(step)) return world_velocity_18;   // 007DC759 JB
    const float keep = 0.25f - step;                                      // 007DC788, FSTP float
    const float x = prev_30.x * keep + world_velocity_18.x * step;        // 007DC7B3
    const float y = prev_30.y * keep + world_velocity_18.y * step;
    const float z = prev_30.z * keep + world_velocity_18.z * step;
    return Vec3{x * 4.0f, y * 4.0f, z * 4.0f};                            // 007DC7DB-007DC7F7
}

ReparentVelocity reparent_velocity_007d9ce0(const ReparentVelocity& in, bool same_parent,
                                            bool has_old_parent, bool has_new_parent,
                                            const ReparentCallbacks& cb) {
    if (same_parent) return in;                                  // 007D9CF2 CMP / 007D9CF4 JE
    ReparentVelocity out = in;
    if (has_old_parent) {
        const Vec3 r = cb.old_parent_to_world(out.velocity_18);  // 007D9D29
        const Vec3 p = cb.old_parent_velocity();                 // 007D9D4A
        out.velocity_18 = Vec3{p.x + r.x, p.y + r.y, p.z + r.z};
        out.vector_24 = cb.old_parent_to_world(out.vector_24);   // 007D9D84
    }
    if (has_new_parent) {
        const Vec3 p = cb.new_parent_velocity();                 // 007D9DAE
        out.velocity_18 = Vec3{out.velocity_18.x - p.x, out.velocity_18.y - p.y,
                               out.velocity_18.z - p.z};
        out.velocity_18 = cb.world_to_new_parent(out.velocity_18);  // 007D9E03
        out.vector_24 = cb.world_to_new_parent(out.vector_24);      // 007D9E48
    }
    out.previous_30 = out.velocity_18;
    return out;
}
}  // namespace gameplay_seg48_cc13
}  // namespace bsp
